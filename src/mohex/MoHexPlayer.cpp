//----------------------------------------------------------------------------
/** @file MoHexPlayer.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "SgTimer.h"
#include "SgUctTreeUtil.h"

#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"
#include "VCSet.hpp"
#include "HexUctUtil.hpp"
#include "HexUctKnowledge.hpp"
#include "HexUctSearch.hpp"
#include "HexUctPolicy.hpp"
#include "MoHexPlayer.hpp"
#include "PlayerUtils.hpp"
#include "SequenceHash.hpp"
#include "Time.hpp"
#include "VCUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

MoHexPlayer::MoHexPlayer()
    : BenzenePlayer(),
      m_shared_policy(),
      m_search(new HexThreadStateFactory(&m_shared_policy), 
               HexUctUtil::ComputeMaxNumMoves()),
      m_backup_ice_info(true),
      m_max_games(500000),
      m_max_time(10),
      m_reuse_subtree(false)
{
    LogFine() << "--- MoHexPlayer" << '\n';
}

MoHexPlayer::~MoHexPlayer()
{
}

void MoHexPlayer::CopySettingsFrom(const MoHexPlayer& other)
{
    SetBackupIceInfo(other.BackupIceInfo());
    Search().SetLockFree(other.Search().LockFree());
    Search().SetLiveGfx(other.Search().LiveGfx());
    Search().SetRave(other.Search().Rave());
    Search().SetBiasTermConstant(other.Search().BiasTermConstant());
    Search().SetExpandThreshold(other.Search().ExpandThreshold());
    Search().SetLiveGfxInterval(other.Search().LiveGfxInterval());
    SetMaxGames(other.MaxGames());
    SetMaxTime(other.MaxTime());
    Search().SetMaxNodes(other.Search().MaxNodes());
    Search().SetNumberThreads(other.Search().NumberThreads());
    Search().SetPlayoutUpdateRadius(other.Search().PlayoutUpdateRadius());
    Search().SetRaveWeightFinal(other.Search().RaveWeightFinal());
    Search().SetRaveWeightInitial(other.Search().RaveWeightInitial());
    Search().SetTreeUpdateRadius(other.Search().TreeUpdateRadius());
    Search().SetKnowledgeThreshold(other.Search().KnowledgeThreshold());
}

//----------------------------------------------------------------------------

HexPoint MoHexPlayer::search(HexBoard& brd, 
                             const Game& game_state,
			     HexColor color,
                             const bitset_t& given_to_consider,
                             double time_remaining,
                             double& score)
{
   
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(!brd.isGameOver());

    double start = Time::Get();
    PrintParameters(color, time_remaining);

    // Do presearch and abort if win found.
    SgTimer timer;
    bitset_t consider = given_to_consider;
    PointSequence winningSequence;
    if (PerformPreSearch(brd, color, consider, winningSequence))
    {
	LogInfo() << "Winning sequence found before UCT search!" << '\n'
		  << "Sequence: " << winningSequence[0] << '\n';
        score = IMMEDIATE_WIN;
	return winningSequence[0];
    }
    timer.Stop();
    double elapsed = timer.GetTime();
    time_remaining -= elapsed;
    LogInfo() << "Time for PreSearch: " << Time::Formatted(elapsed) << '\n';

    // Create the initial state data
    HexUctSharedData data;
    data.root_to_play = color;
    GameUtil::HistoryToSequence(game_state.History(), data.game_sequence);
    data.root_last_move_played = LastMoveFromHistory(game_state.History());
    data.root_stones = HexUctStoneData(brd);
    data.root_consider = consider;
    m_search.SetSharedData(&data);
    
    // Reuse the old subtree
    SgUctTree* initTree = 0;
    if (m_reuse_subtree)
        initTree = TryReuseSubtree(game_state);

    // Compute timelimit for this search. Use the minimum of the time
    // left in the game and the m_max_time parameter.
    double timelimit = std::min(time_remaining, m_max_time);
    if (timelimit < 0) 
    {
        LogWarning() << "***** timelimit < 0!! *****" << '\n';
        timelimit = m_max_time;
    }
    LogInfo() << "timelimit: " << timelimit << '\n';

    brd.ClearPatternCheckStats();
    int old_radius = brd.updateRadius();
    brd.setUpdateRadius(m_search.TreeUpdateRadius());

    // Do the search
    std::vector<SgMove> sequence;
    std::vector<SgMove> rootFilter;
    m_search.SetBoard(brd);
    score = m_search.Search(m_max_games, timelimit, sequence,
                            rootFilter, initTree, 0);

    brd.setUpdateRadius(old_radius);

    double end = Time::Get();

    // Output stats
    std::ostringstream os;
    os << '\n';
#if COLLECT_PATTERN_STATISTICS
    os << m_shared_policy.DumpStatistics() << '\n';
    os << brd.DumpPatternCheckStats() << '\n';
#endif
    os << "Elapsed Time   " << Time::Formatted(end - start) << '\n';
    m_search.WriteStatistics(os);
    os << "Score          " << score << "\n"
       << "Sequence      ";
    for (int i=0; i<(int)sequence.size(); i++) 
        os << " " << HexUctUtil::MoveString(sequence[i]);
    os << '\n';
    
    LogInfo() << os.str() << '\n';

#if 0
    if (m_save_games) {
        std::string filename = "uct-games.sgf";
        uct.SaveGames(filename);
        LogInfo() << "Games saved in '" << filename << "'." << '\n';
    }
#endif

    // Return move recommended by HexUctSearch
    if (sequence.size() > 0) 
        return static_cast<HexPoint>(sequence[0]);

    // It is possible that HexUctSearch only did 1 rollout (probably
    // because it ran out of time to do more); in this case, the move
    // sequence is empty and so we give a warning and return a random
    // move.
    LogWarning() << "**** HexUctSearch returned empty sequence!" << '\n'
		 << "**** Returning random move!" << '\n';
    return BoardUtils::RandomEmptyCell(brd);
}

/** Returns INVALID_POINT if history is empty, otherwise last move
    played to the board, ie, skips swap move.
*/
HexPoint MoHexPlayer::LastMoveFromHistory(const GameHistory& history)
{
    HexPoint lastMove = INVALID_POINT;
    if (!history.empty()) 
    {
	lastMove = history.back().point();
	if (lastMove == SWAP_PIECES) 
        {
            HexAssert(history.size() == 2);
            lastMove = history.front().point();
	}
    }
    return lastMove;
}

/** Does a 1-ply search.

    For each move that in the consider set, if the move is a win,
    returns true and the move. If the move is a loss, prune it out of
    the consider set if there are non-losing moves in the consider
    set.  If all moves are losing, perform no pruning, search will
    resist.

    Returns true if there is a win, false otherwise. 

    @todo Is it true that MoHex will resist in the strongest way
    possible?
*/
bool MoHexPlayer::PerformPreSearch(HexBoard& brd, HexColor color, 
                                   bitset_t& consider, 
                                   PointSequence& winningSequence)
{
   
    bitset_t losing;
    HexColor other = !color;
    PointSequence seq;
    bool foundWin = false;
    for (BitsetIterator p(consider); p && !foundWin; ++p) 
    {
        brd.PlayMove(color, *p);
        seq.push_back(*p);

        // Found a winning move!
        if (PlayerUtils::IsLostGame(brd, other))
        {
            winningSequence = seq;
            brd.UndoMove();
            LogInfo() << "Found win: " << seq[0] << '\n';
            break;
        }	

        if (PlayerUtils::IsWonGame(brd, other))
            losing.set(*p);

        seq.pop_back();
        brd.UndoMove();
    }

    // Abort if we found a one-move win
    if (foundWin)
        return true;

    // Backing up cannot cause this to happen, right? 
    HexAssert(!PlayerUtils::IsDeterminedState(brd, color));

    // Use the backed-up ice info to shrink the moves to consider
    if (m_backup_ice_info) 
    {
        bitset_t new_consider 
            = PlayerUtils::MovesToConsider(brd, color) & consider;

        if (new_consider.count() < consider.count()) 
        {
            consider = new_consider;       
            LogFine() << "$$$$$$ new moves to consider $$$$$$" 
                      << brd.printBitset(consider) << '\n';
        }
    }

    // Subtract any losing moves from the set we consider, unless all of them
    // are losing (in which case UCT search will find which one resists the
    // loss well).
    if (losing.any()) 
    {
	if (BitsetUtil::IsSubsetOf(consider, losing)) 
        {
	    LogInfo() << "************************************" << '\n'
                      << " All UCT root children are losing!!" << '\n'
                      << "************************************" << '\n';
	} 
        else 
        {
            LogFine() << "Removed losing moves: " 
		      << brd.printBitset(losing) << '\n';
	    consider = consider - losing;
	}
    }
    HexAssert(consider.any());
    LogInfo()<< "Moves to consider:" << '\n' 
             << brd.printBitset(consider) << '\n';
    return false;
}

void MoHexPlayer::PrintParameters(HexColor color, double remaining)
{
    LogInfo() << "--- MoHexPlayer::Search() ---" << '\n'
	      << "Color: " << color << '\n'
	      << "MaxGames: " << m_max_games << '\n'
	      << "MaxTime: " << m_max_time << '\n'
	      << "NumberThreads: " << m_search.NumberThreads() << '\n'
	      << "MaxNodes: " << m_search.MaxNodes()
	      << " (" << sizeof(SgUctNode)*m_search.MaxNodes() << " bytes)" 
	      << '\n'
	      << "TimeRemaining: " << remaining << '\n';
}

/** @bug CURRENTLY BROKEN!  
    
    Need to do a few things before subtrees can be reused:
    
    1) After extracting the relevant subtree, must copy over fillin
    states from old hashmap to new hashmap.

    2) Deal with new root-state knowledge. SgUctSearch has a
    rootfilter that prunes moves in the root and is applied when it is
    passed an initial tree, so we can use that to apply more pruning
    to the new root node. Potential problem if new root knowledge adds
    moves that weren't present when knowledge was computed in the
    tree, but I don't think this would happen very often (or matter).
*/
SgUctTree* MoHexPlayer::TryReuseSubtree(const Game& game)
{
    LogSevere() << "\"param_mohex reuse_subtree\" is currently broken!" << '\n'
                << "Please see MoHexPlayer::TryReuseSubtree() in "
                << "the documentation." << '\n';
    abort();

    const StoneBoard& lastPosition = m_search.LastPositionSearched();

    /** @todo This is evil. Come up with better way to determine
        if a search has been done before, other than checking if
        lastPosition is an undefined board. 
    */
    if (!&lastPosition.Const())
        return 0;

    LogInfo() << "Old Search Position:" << lastPosition << '\n';

    GameHistory gameSeq;
    if (!GameUtil::SequenceFromPosition(game, lastPosition, gameSeq))
        return 0;

    // Ensure alternating colors p
    for (std::size_t i = 1; i < gameSeq.size(); ++i)
        if (gameSeq[i].color() == gameSeq[i-1].color())
            return 0;

    LogInfo() << "Moves played:";            
    std::vector<SgMove> sequence;
    for (std::size_t i = 0; i < gameSeq.size(); ++i)
    {
        LogInfo() << ' ' << gameSeq[i].point();
        sequence.push_back(gameSeq[i].point());
    }
    LogInfo() << '\n';
    
    SgUctTree& tree = m_search.GetTempTree();
    SgUctTreeUtil::ExtractSubtree(m_search.Tree(), tree, sequence, true, 10.0);

    /** @todo Must traverse tree and copy fillin data from old
        hashmap to new hashmap.
    */

    std::size_t newTreeNodes = tree.NuNodes();
    std::size_t oldTreeNodes = m_search.Tree().NuNodes();
    if (oldTreeNodes > 1 && newTreeNodes > 1)
    {
        float reuse = static_cast<float>(newTreeNodes) / oldTreeNodes;
        int reusePercent = static_cast<int>(100 * reuse);
        LogInfo() << "HexUctPlayer: Reusing " << newTreeNodes
                  << " nodes (" << reusePercent << "%)\n";
    }
    return &tree;
}

//----------------------------------------------------------------------------

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

namespace {

//----------------------------------------------------------------------------

/** Performs the one-ply pre-search. */
void ComputeSharedData(bool backupIceInfo, 
                       HexBoard& brd, HexColor color,
                       bitset_t& consider,
                       HexUctSharedData& data,
                       HexPoint& oneMoveWin)
{
    // For each 1-ply move that we're told to consider:
    // 1) If the move gives us a win, no need for UCT - just use this move
    // 2) If the move is a loss, note this fact - we'll likely prune it later
    // 3) Compute the 2nd-ply moves to consider so that only reasonable
    //    opponent replies are considered
    // 4) Store the state of the board fill-in to shorten rollouts and 
    //    improve their accuracy

    bitset_t losing;
    data.root_to_play = color;
    data.root_stones = HexUctStoneData(brd);

    HexColor other = !color;
    PointSequence seq;
    for (BitsetIterator p(consider); p; ++p) 
    {
        brd.PlayMove(color, *p);
        seq.push_back(*p);

        // found a winning move!
        if (PlayerUtils::IsLostGame(brd, other))
        {
	    oneMoveWin = *p;
            brd.UndoMove();
            LogInfo() << "Found win: " << oneMoveWin << '\n';
            break;
        }	

        data.stones.put(SequenceHash::Hash(seq), HexUctStoneData(brd));

        if (PlayerUtils::IsWonGame(brd, other))
            losing.set(*p);

        seq.pop_back();
        brd.UndoMove();
    }

    // Abort out if we found a one-move win
    if (oneMoveWin != INVALID_POINT) 
        return;

    // Backing up cannot cause this to happen, right? 
    HexAssert(!PlayerUtils::IsDeterminedState(brd, color));

    // Use the backed-up ice info to shrink the moves to consider
    if (backupIceInfo) 
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
    HexAssert(oneMoveWin == INVALID_POINT);
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

    // Add the appropriate children to the root of the UCT tree
    HexAssert(consider.any());
    LogInfo()<< "Moves to consider:" << '\n' 
             << brd.printBitset(consider) << '\n';
    data.root_consider = consider;
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

MoHexPlayer::MoHexPlayer()
    : BenzenePlayer(),
      m_shared_policy(),
      m_search(new HexThreadStateFactory(&m_shared_policy), 
               HexUctUtil::ComputeMaxNumMoves()),
      m_backup_ice_info(true),
      m_max_games(500000),
      m_max_time(9999999),
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

    HexPoint lastMove = INVALID_POINT;
    if (!game_state.History().empty()) {
	lastMove = game_state.History().back().point();
	if (lastMove == SWAP_PIECES) {
	  HexAssert(game_state.History().size() == 2);
	  lastMove = game_state.History().front().point();
	}
    }

    LogInfo() << "--- MoHexPlayer::Search()" << '\n'
	      << "Color: " << color << '\n'
	      << "MaxGames: " << m_max_games << '\n'
	      << "MaxTime: " << m_max_time << '\n'
	      << "NumberThreads: " << m_search.NumberThreads() << '\n'
	      << "MaxNodes: " << m_search.MaxNodes()
	      << " (" << sizeof(SgUctNode)*m_search.MaxNodes() << " bytes)" 
	      << '\n'
	      << "TimeRemaining: " << time_remaining << '\n';

    // Create the initial state data
    HexPoint oneMoveWin = INVALID_POINT;
    HexUctSharedData data;
    data.root_last_move_played = lastMove;
    bitset_t consider = given_to_consider;

    SgTimer timer;
    timer.Start();
    ComputeSharedData(m_backup_ice_info, brd, color, 
                      consider, data, oneMoveWin);
    m_search.SetSharedData(&data);
    timer.Stop();
    double elapsed = timer.GetTime();
    time_remaining -= elapsed;
    
    LogInfo() << "Time to compute initial data: " 
	      << Time::Formatted(elapsed) << '\n';
    
    // If a winning VC is found after a one ply move, no need to search
    if (oneMoveWin != INVALID_POINT) {
	LogInfo() << "Winning VC found before UCT search!" << '\n'
		  << "Sequence: " << HexPointUtil::toString(oneMoveWin) 
		  << '\n';
        score = IMMEDIATE_WIN;
	return oneMoveWin;
    }

    double timelimit = std::min(time_remaining, m_max_time);
    if (timelimit < 0) {
        LogWarning() << "***** timelimit < 0!! *****" << '\n';
        timelimit = 30;
    }
    LogInfo() << "timelimit: " << timelimit << '\n';

    brd.ClearPatternCheckStats();
    int old_radius = brd.updateRadius();
    brd.setUpdateRadius(m_search.TreeUpdateRadius());

    SgUctTree* initTree = 0;
    if (m_reuse_subtree)
        initTree = TryReuseSubtree(game_state);

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

/** @bug CURRENTLY BROKEN!  
    
    Need to do a few things before subtrees can be reused:
    1) Switch sequence used to compute fillin hash to use the
    entire game sequence.
    2) After extracting the relevant subtree, must copy over
    fillin states from old hashmap to new hashmap. 
    3) Deal with new root-state knowledge. SgUctSearch has
    a rootfilter that prunes moves in the root and is applied
    when it is passed an initial tree, so we can use that
    to apply more pruning to the new root node. Potential problem
    if new root knowledge adds moves that weren't present when
    knowledge was computed in the tree, but I don't think
    this would happen very often (or matter). 
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

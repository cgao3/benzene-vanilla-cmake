//----------------------------------------------------------------------------
/** @file MoHexPlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"
#include "SgUctTreeUtil.h"

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "VCSet.hpp"
#include "MoHexUtil.hpp"
#include "MoHexPriorKnowledge.hpp"
#include "MoHexSearch.hpp"
#include "MoHexPlayoutPolicy.hpp"
#include "MoHexPlayer.hpp"
#include "EndgameUtil.hpp"
#include "Resistance.hpp"
#include "SequenceHash.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

/** Returns true if one is a prefix of the other. */
bool IsPrefixOf(const MoveSequence& a, const MoveSequence& b)
{
    for (std::size_t i = 0; i < a.size() && i < b.size(); ++i)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}


void SortConsiderSet(const bitset_t& consider, const Resistance& resist,
                     std::vector<HexPoint>& moves)
{
    std::vector<std::pair<HexEval, HexPoint> > mvsc;
    for (BitsetIterator it(consider); it; ++it) 
        mvsc.push_back(std::make_pair(-resist.Score(*it), *it));
    stable_sort(mvsc.begin(), mvsc.end());
    moves.clear();
    for (std::size_t i = 0; i < mvsc.size(); ++i)
        moves.push_back(mvsc[i].second);
}                             

}

//----------------------------------------------------------------------------

MoHexPlayer::MoHexPlayer()
    : BenzenePlayer(),
      m_shared_policy(),
      m_search(new HexThreadStateFactory(&m_shared_policy), 
               MoHexUtil::ComputeMaxNumMoves()),
      m_backup_ice_info(true),
      m_max_games(99999999),
      m_max_time(10),
      m_useTimeManagement(false),
      m_reuse_subtree(false),
      m_ponder(false),
      m_performPreSearch(true)
{
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
    Search().SetNumberPlayouts(other.Search().NumberPlayouts());
    SetMaxGames(other.MaxGames());
    SetMaxTime(other.MaxTime());
    SetPerformPreSearch(other.PerformPreSearch());
    SetUseTimeManagement(other.UseTimeManagement());
    Search().SetMaxNodes(other.Search().MaxNodes());
    Search().SetNumberThreads(other.Search().NumberThreads());
    Search().SetPlayoutUpdateRadius(other.Search().PlayoutUpdateRadius());
    Search().SetRandomizeRaveFrequency
        (other.Search().RandomizeRaveFrequency());
    Search().SetRaveWeightFinal(other.Search().RaveWeightFinal());
    Search().SetRaveWeightInitial(other.Search().RaveWeightInitial());
    Search().SetWeightRaveUpdates(other.Search().WeightRaveUpdates());
    Search().SetTreeUpdateRadius(other.Search().TreeUpdateRadius());
    Search().SetKnowledgeThreshold(other.Search().KnowledgeThreshold());
}

//----------------------------------------------------------------------------

HexPoint MoHexPlayer::Search(const HexState& state, const Game& game,
                             HexBoard& brd, const bitset_t& given_to_consider,
                             double maxTime, double& score)
{
    BenzeneAssert(!brd.GetGroups().IsGameOver());
    HexColor color = state.ToPlay();   
   
    SgTimer totalElapsed;
    PrintParameters(color, maxTime);

    // Do presearch and abort if win found. Allow it to take 20% of
    // total time.
    SgTimer timer;
    bitset_t consider = given_to_consider;
    PointSequence winningSequence;
    if (m_performPreSearch && PerformPreSearch(brd, color, consider, 
                                               maxTime * 0.2, winningSequence))
    {
	LogInfo() << "Winning sequence found before UCT search!\n"
		  << "Sequence: " << winningSequence[0] << '\n';
        score = IMMEDIATE_WIN;
	return winningSequence[0];
    }
    timer.Stop();
    LogInfo() << "Time for PreSearch: " << timer.GetTime() << "s\n";

    maxTime -= timer.GetTime();
    maxTime = std::max(1.0, maxTime);
        
    // Create the initial state data
    MoHexSharedData data(m_search.FillinMapBits());
    data.gameSequence = game.History();
    data.rootState = HexState(brd.GetPosition(), color);
    data.rootConsider = consider;
    
    // Reuse the old subtree
    SgUctTree* initTree = 0;
    if (m_reuse_subtree)
    {
        MoHexSharedData oldData(m_search.SharedData());
        initTree = TryReuseSubtree(oldData, data);
        if (!initTree)
            LogInfo() << "No subtree to reuse.\n";
    }
    m_search.SetSharedData(data);

    brd.GetPatternState().ClearPatternCheckStats();
    int old_radius = brd.GetPatternState().UpdateRadius();
    brd.GetPatternState().SetUpdateRadius(m_search.TreeUpdateRadius());

    // Do the search
    std::vector<SgMove> sequence;
    std::vector<SgMove> rootFilter;
    m_search.SetBoard(brd);
    score = m_search.Search(m_max_games, maxTime, sequence,
                            rootFilter, initTree, 0);

    brd.GetPatternState().SetUpdateRadius(old_radius);

    // Output stats
    std::ostringstream os;
    os << '\n';
#if COLLECT_PATTERN_STATISTICS
    {
        MoHexState& thread0 
            = dynamic_cast<MoHexState&>(m_search.ThreadState(0));
        MoHexPolicy* policy = dynamic_cast<MoHexPolicy*>(thread0.Policy());
        if (policy)
            os << policy->DumpStatistics() << '\n';
    }
#endif
    os << "Elapsed Time   " << totalElapsed.GetTime() << "s\n";
    m_search.WriteStatistics(os);
    os << "Score          " << std::setprecision(2) << score << "\n"
       << "Sequence      ";
    for (std::size_t i = 0; i < sequence.size(); i++)
        os << " " << MoHexUtil::MoveString(sequence[i]);
    os << '\n';
    LogInfo() << os.str() << '\n';

#if 0
    if (m_save_games) 
    {
        std::string filename = "uct-games.sgf";
        uct.SaveGames(filename);
        LogInfo() << "Games saved in '" << filename << "'.\n";
    }
#endif

    // Return move recommended by MoHexSearch
    if (sequence.size() > 0) 
        return static_cast<HexPoint>(sequence[0]);

    // It is possible that MoHexSearch did only 1 simulation (probably
    // because it ran out of time to do more); in this case, the move
    // sequence is empty and so we give a warning and return a random
    // move.
    LogWarning() << "**** MoHexSearch returned empty sequence!\n"
		 << "**** Returning random move!\n";
    return BoardUtil::RandomEmptyCell(brd.GetPosition());
}

/** Does a 1-ply search.

    For each move in the consider set, if the move is a win, returns
    true and the move. If the move is a loss, prune it out of the
    consider set if there are non-losing moves in the consider set.
    If all moves are losing, perform no pruning, search will resist.

    Returns true if there is a win, false otherwise. 

    @todo Is it true that MoHex will resist in the strongest way
    possible?
*/
bool MoHexPlayer::PerformPreSearch(HexBoard& brd, HexColor color, 
                                   bitset_t& consider, double maxTime, 
                                   PointSequence& winningSequence)
{
   
    bitset_t losing;
    HexColor other = !color;
    PointSequence seq;
    bool foundWin = false;

    SgTimer elapsed;
    Resistance resist;
    resist.Evaluate(brd);
    std::vector<HexPoint> moves;
    SortConsiderSet(consider, resist, moves);
    for (std::size_t i = 0; i < moves.size() && !foundWin; ++i) 
    {
        if (elapsed.GetTime() > maxTime)
        {
            LogInfo() << "PreSearch: max time reached "
                      << '(' << i << '/' << moves.size() << ").\n";
            break;
        }
        brd.PlayMove(color, moves[i]);
        seq.push_back(moves[i]);
        if (EndgameUtil::IsLostGame(brd, other)) // Check for winning move
        {
            winningSequence = seq;
            foundWin = true;
        }	
        else if (EndgameUtil::IsWonGame(brd, other))
            losing.set(moves[i]);
        seq.pop_back();
        brd.UndoMove();
    }

    // Abort if we found a one-move win
    if (foundWin)
        return true;

    // Backing up cannot cause this to happen, right? 
    BenzeneAssert(!EndgameUtil::IsDeterminedState(brd, color));

    // Use the backed-up ice info to shrink the moves to consider
    if (m_backup_ice_info) 
    {
        bitset_t new_consider 
            = EndgameUtil::MovesToConsider(brd, color) & consider;

        if (new_consider.count() < consider.count()) 
        {
            consider = new_consider;       
            LogFine() << "$$$$$$ new moves to consider $$$$$$" 
                      << brd.Write(consider) << '\n';
        }
    }

    // Subtract any losing moves from the set we consider, unless all of them
    // are losing (in which case UCT search will find which one resists the
    // loss well).
    if (losing.any()) 
    {
	if (BitsetUtil::IsSubsetOf(consider, losing)) 
	    LogInfo() << "************************************\n"
                      << " All UCT root children are losing!!\n"
                      << "************************************\n";
        else 
        {
            LogFine() << "Removed losing moves: " << brd.Write(losing) << '\n';
	    consider = consider - losing;
	}
    }

    BenzeneAssert(consider.any());
    LogInfo() << "Moves to consider:\n" << brd.Write(consider) << '\n';
    return false;
}

void MoHexPlayer::PrintParameters(HexColor color, double timeForMove)
{
    LogInfo() << "--- MoHexPlayer::Search() ---\n"
	      << "Color: " << color << '\n'
	      << "MaxGames: " << m_max_games << '\n'
	      << "NumberThreads: " << m_search.NumberThreads() << '\n'
	      << "MaxNodes: " << m_search.MaxNodes()
	      << " (" << sizeof(SgUctNode)*m_search.MaxNodes() << " bytes)\n" 
	      << "TimeForMove: " << timeForMove << '\n';
}

/** Extracts relevant portion of old tree for use in upcoming search.
    Returns valid pointer to new tree on success, 0 on failure. */
SgUctTree* MoHexPlayer::TryReuseSubtree(const MoHexSharedData& oldData,
                                        MoHexSharedData& newData)
{
    // Must have knowledge on to reuse subtrees, since root has fillin
    // knowledge which affects the tree below.
    if (m_search.KnowledgeThreshold().empty())
    {
        LogInfo() << "ReuseSubtree: knowledge is off.\n";
        return 0;
    }
    // Board size must be the same. This also catches the case where
    // no previous search has been performed.
    const StoneBoard& oldPosition = oldData.rootState.Position();
    const StoneBoard& newPosition = newData.rootState.Position();
    if (&oldPosition.Const() == 0)
        return 0;
    if (oldPosition.Width() != newPosition.Width() ||
        oldPosition.Height() != newPosition.Height())
        return 0;

    const MoveSequence& oldSequence = oldData.gameSequence;
    const MoveSequence& newSequence = newData.gameSequence;
    LogInfo() << "Old: " << oldSequence << '\n';
    LogInfo() << "New: "<< newSequence << '\n';
    if (oldSequence.size() > newSequence.size())
    {
        LogInfo() << "ReuseSubtree: Backtracked to an earlier state.\n";
        return 0;
    }
    if (!IsPrefixOf(oldSequence, newSequence))
    {
        LogInfo() << "ReuseSubtree: Not a continuation.\n";
        return 0;
    }

    const HexState& oldState = oldData.rootState;
    const HexState& newState = newData.rootState;
    bool samePosition = (oldSequence == newSequence && oldState == newState
                         && oldData.rootConsider == newData.rootConsider);
    if (samePosition)
        LogInfo() << "ReuseSubtree: in same position as last time!\n";

    // If no old knowledge for the new root in the old tree, then we
    // cannot reuse the tree (since the root is given its knowledge
    // and using this knowledge would require pruning the trees under
    // the root's children). 
    if (!samePosition)
    {
        StoneBoard oldPosition;
        if (!oldData.stones.Get(SequenceHash::Hash(newSequence), oldPosition))
        {
            LogInfo() << "ReuseSubtree: No knowledge for state in old tree.\n";
            return 0;
        }
        // Check that the old knowledge is equal to the new knowledge
        // in the would-be root node.
        if (!(oldPosition == newPosition))
        {
            LogInfo() << "Old fillin data does not match data for new root!\n";
            return 0;
        }
    }

    // Ensure alternating colors and extract suffix
    MoveSequence suffix;
    std::vector<SgMove> sequence;
    for (std::size_t i = oldSequence.size(); i < newSequence.size(); ++i)
    {
        if (i && newSequence[i-1].Color() == newSequence[i].Color())
        {
            LogInfo() << "ReuseSubtree: Colors do not alternate.\n";
            return 0;
        }
        suffix.push_back(newSequence[i]);
        sequence.push_back(newSequence[i].Point());
    }
    LogInfo() << "MovesPlayed: " << suffix << '\n';
    
    // Extract the tree
    SgUctTree& tree = m_search.GetTempTree();
    SgUctTreeUtil::ExtractSubtree(m_search.Tree(), tree, sequence, true, 10.0);

    std::size_t newTreeNodes = tree.NuNodes();
    std::size_t oldTreeNodes = m_search.Tree().NuNodes();

    if (oldTreeNodes > 1 && newTreeNodes > 1)
    {
        // Fix root's children to be those in the consider set
        std::vector<SgMove> moves;
        for (BitsetIterator it(newData.rootConsider); it; ++it)
            moves.push_back(static_cast<SgMove>(*it));
        tree.SetChildren(0, tree.Root(), moves);

        float reuse = float(newTreeNodes) / float(oldTreeNodes);
        int reusePercent = static_cast<int>(100 * reuse);
        LogInfo() << "MoHexPlayer: Reusing " << newTreeNodes
                  << " nodes (" << reusePercent << "%)\n";

        MoveSequence moveSequence = newSequence;
        CopyKnowledgeData(tree, tree.Root(), newData.rootState.ToPlay(),
                          moveSequence, oldData, newData);
        float kReuse = float(newData.stones.Count()) 
            / float(oldData.stones.Count());
        int kReusePercent = static_cast<int>(100 * kReuse);
        LogInfo() << "MoHexPlayer: Reusing " 
                  << newData.stones.Count() << " knowledge states ("
                  << kReusePercent << "%)\n";
        return &tree;
    }
    return 0;
}

void MoHexPlayer::CopyKnowledgeData(const SgUctTree& tree,
                                    const SgUctNode& node,
                                    HexColor color, MoveSequence& sequence,
                                    const MoHexSharedData& oldData,
                                    MoHexSharedData& newData) const
{
    // This check will fail in the root if we are reusing the
    // entire tree, so only do it when not in the root.
    if (sequence != oldData.gameSequence)
    {
        SgHashCode hash = SequenceHash::Hash(sequence);
        StoneBoard stones;
        if (!oldData.stones.Get(hash, stones))
            return;
        newData.stones.Add(hash, stones);
    }
    if (!node.HasChildren())
        return;
    for (SgUctChildIterator it(tree, node); it; ++it)
    {
        sequence.push_back(Move(color, static_cast<HexPoint>((*it).Move())));
        CopyKnowledgeData(tree, *it, !color, sequence, oldData, newData);
        sequence.pop_back();
    }
}

//----------------------------------------------------------------------------

void MoHexPlayer::FindTopMoves(int num, const HexState& state, 
                               const Game& game, HexBoard& brd, 
                               const bitset_t& given_to_consider,
                               double maxTime, 
                               std::vector<HexPoint>& moves,
                               std::vector<double>& scores)
{
    bitset_t consider = given_to_consider;
    while (consider.any() && num-- > 0)
    {
        double score;
        HexPoint move = Search(state, game, brd, consider, maxTime, score);
        consider.reset(move);
        moves.push_back(move);
        scores.push_back(score);
    }
}

//----------------------------------------------------------------------------

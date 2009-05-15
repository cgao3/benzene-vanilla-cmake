//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "Misc.hpp"
#include "PlayerUtils.hpp"
#include "SequenceHash.hpp"
#include "WolvePlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

WolvePlayer::WolvePlayer()
    : BenzenePlayer(),
      m_tt(16), 
      m_plywidth(),
      m_search_depths(),
      m_panic_time(240)
{
    LogFine() << "--- WolvePlayer" << '\n';

    m_plywidth.push_back(20);
    m_plywidth.push_back(20);
    m_plywidth.push_back(20);
    m_plywidth.push_back(20);

    m_search_depths.push_back(1);
    m_search_depths.push_back(2);
    m_search_depths.push_back(4);

    m_search.SetTT(&m_tt);
}

WolvePlayer::~WolvePlayer()
{
}

//----------------------------------------------------------------------------

HexPoint WolvePlayer::search(HexBoard& brd, 
                             const Game& game_state,
			     HexColor color, 
                             const bitset_t& consider,
                             double max_time,
                             double& score)
{
    UNUSED(game_state);
    std::vector<int> search_depths = m_search_depths;

    // If low on time, set a maximum search depth of 2.
    if (max_time < m_panic_time)
    {
	std::vector<int> new_search_depths;
	for (std::size_t i = 0; i < search_depths.size(); ++i)
	    if (search_depths[i] <= 2)
		new_search_depths.push_back(search_depths[i]);
	search_depths = new_search_depths;
	LogWarning() << "############# PANIC MODE #############" << '\n';
    }

    m_search.SetRootMovesToConsider(consider);
    LogInfo() << "Using consider set:" << brd.printBitset(consider) << '\n'
	      << "Plywidths: " << MiscUtil::PrintVector(m_plywidth) << '\n'
	      << "Depths: " << MiscUtil::PrintVector(m_search_depths) << '\n';

    std::vector<HexPoint> PV;
    score = m_search.Search(brd, color, m_plywidth, search_depths, -1, PV);

    LogInfo() << m_search.DumpStats() << '\n';

    HexAssert(PV.size() > 0);
    return PV[0];
}

//----------------------------------------------------------------------------

WolveWorkThread::WolveWorkThread(HexBoard& brd, 
                                 boost::barrier& start,
                                 boost::barrier& completed,
                                 const WolveThreadAction& action,
                                 const HexPoint& move,
                                 const HexColor& color)
    : m_brd(brd),
      m_start(start),
      m_completed(completed),
      m_action(action),
      m_move(move),
      m_color(color)
{
}

void WolveWorkThread::operator()()
{
    bool finished = false;
    while (true)
    {
        m_start.wait();

        switch(m_action) {
        case WOLVE_THREAD_QUIT:
            finished = true;
            break;

        case WOLVE_THREAD_UNDO:
            m_brd.UndoMove();
            break;

        case WOLVE_THREAD_PLAY:
            m_brd.PlayMove(m_color, m_move);
            break;
        }

        if (finished) break;

        m_completed.wait();
    }

    LogInfo() << "Exiting..." << '\n';
}

//----------------------------------------------------------------------------

WolveSearch::WolveSearch()
    : m_no_fillin_board(),

      m_varTT(16),           // 16bit variation trans-table

      m_use_threads(false),
      m_backup_ice_info(true),

      m_start_threads(3),
      m_threads_completed(3)
{
}

WolveSearch::~WolveSearch()
{
}

void WolveSearch::OnStartSearch()
{
    m_varTT.clear();  // *MUST* clear old variation TT!
    m_consider.clear();

    if (!m_no_fillin_board.get() ||
        m_no_fillin_board->width() != m_brd->width() || 
        m_no_fillin_board->height() != m_brd->height())
    {
        LogInfo() << "Wolve: Creating new board..." << '\n';
        m_no_fillin_board.reset
            (new HexBoard(m_brd->width(), m_brd->height(), 
                          m_brd->ICE(), m_brd->Builder().Parameters()));
        int sf = m_brd->Cons(BLACK).SoftLimit(VC::FULL);
        int ss = m_brd->Cons(BLACK).SoftLimit(VC::SEMI);
        m_no_fillin_board->Cons(BLACK).SetSoftLimit(VC::FULL, sf);
        m_no_fillin_board->Cons(BLACK).SetSoftLimit(VC::SEMI, ss);
        m_no_fillin_board->Cons(WHITE).SetSoftLimit(VC::FULL, sf);
        m_no_fillin_board->Cons(WHITE).SetSoftLimit(VC::SEMI, ss);
    }

    SetupNonFillinBoard();
    StartThreads();
}

void WolveSearch::EnteredNewState()
{
}

HexEval WolveSearch::Evaluate()
{
    Resistance resist;
    ComputeResistance(resist);

    HexEval score = (m_toplay == BLACK) 
        ?  resist.Score() 
        : -resist.Score();

    LogFine() << "Score for " << m_toplay
             << ": " << score << '\n';

    return score;
}

void WolveSearch::GenerateMoves(std::vector<HexPoint>& moves)
{
    HexAssert(BitsetUtil::IsSubsetOf(m_brd->getEmpty(), 
                                     m_no_fillin_board->getEmpty()));

    Resistance resist;
    ComputeResistance(resist);

    // Get moves to consider:
    //   1) from the variation tt, if this variation has been visited before.
    //   2) from the passed in consider set, if at the root.
    //   3) from computing it ourselves.
    bitset_t consider;

    VariationInfo varInfo;
    if (m_varTT.get(SequenceHash::Hash(m_sequence), varInfo))
    {
        LogFine() << "Using consider set from TT." << '\n'
		  << HexPointUtil::ToPointListString(m_sequence) << '\n'
		  << m_brd << '\n';
        consider = varInfo.consider;
    } 
    else if (m_current_depth == 0)
    {
        LogFine() << "Using root consider set." << '\n';
        consider = m_rootMTC;
    }
    else 
    {
        LogFine() << "Computing our own consider set." << '\n';
        consider = PlayerUtils::MovesToConsider(*m_brd, m_toplay);
    }

    m_consider.push_back(consider);
    HexAssert((int)m_consider.size() == m_current_depth+1);

    // order the moves
    moves.clear();
    std::vector<std::pair<HexEval, HexPoint> > mvsc;
    for (BitsetIterator it(consider); it; ++it) {

        // prefer the best move from the TT if possible
        HexEval score = (*it == m_tt_bestmove) 
            ? 10000
            : resist.Score(*it);

        mvsc.push_back(std::make_pair(-score, *it));
    }

    /** @note to ensure we are deterministic, we must use stable_sort. */
    stable_sort(mvsc.begin(), mvsc.end());
    
    for (unsigned i=0; i<mvsc.size(); ++i) {
        moves.push_back(mvsc[i].second);
    }
}

void WolveSearch::ExecuteMove(HexPoint move)
{
    if (m_use_threads)
    {
        m_threadAction = WOLVE_THREAD_PLAY;
        m_threadMove = move;
        m_threadColor = m_toplay;
        m_start_threads.wait();
        m_threads_completed.wait();
    } 
    else
    {
        m_brd->PlayMove(m_toplay, move);
        m_no_fillin_board->PlayMove(m_toplay, move);
    }
}

void WolveSearch::UndoMove(HexPoint move)
{
    UNUSED(move);

    if (m_use_threads)
    {
        m_threadAction = WOLVE_THREAD_UNDO;
        m_start_threads.wait();
        m_threads_completed.wait();
    } 
    else
    {
        m_brd->UndoMove();
        m_no_fillin_board->UndoMove();
    }
}

void WolveSearch::AfterStateSearched()
{
    if (m_backup_ice_info) 
    {
        // store new consider set in varTT
        bitset_t old_consider = m_consider[m_current_depth];
        bitset_t new_consider 
            = PlayerUtils::MovesToConsider(*m_brd, m_toplay) & old_consider;
        hash_t hash = SequenceHash::Hash(m_sequence);
        m_varTT.put(VariationInfo(hash, m_current_depth, new_consider));
    }

    m_consider.pop_back();
}

void WolveSearch::OnSearchComplete()
{
    StopThreads();
}

//----------------------------------------------------------------------------

void WolveSearch::SetupNonFillinBoard()
{
    m_no_fillin_board->startNewGame();
    m_no_fillin_board->setColor(BLACK, m_brd->getBlack());
    m_no_fillin_board->setColor(WHITE, m_brd->getWhite());    
    m_no_fillin_board->setPlayed(m_brd->getPlayed());
    // do not use ICE or decomps on our board
    m_no_fillin_board->SetUseICE(false);
    m_no_fillin_board->SetUseDecompositions(false);
    m_no_fillin_board->ComputeAll(m_toplay, 
                                  // note this does not really matter
                                  // since we aren't using ice anyway!
                                  HexBoard::DO_NOT_REMOVE_WINNING_FILLIN);
}

void WolveSearch::ComputeResistance(Resistance& resist)
{
    // augment the no-fillin-board's connections with those of
    // the filled-in board.
    AdjacencyGraph graphs[BLACK_AND_WHITE];
    ResistanceUtil::AddAdjacencies(*m_no_fillin_board, graphs);
    ResistanceUtil::AddAdjacencies(*m_brd, graphs);
    resist.Evaluate(*m_no_fillin_board, graphs);
}

void WolveSearch::StartThreads()
{
    if (!m_use_threads)
        return;

    LogInfo() << "WolveSearch::StartThreads()" << '\n';

    m_thread[0].reset(new boost::thread(WolveWorkThread(*m_brd, 
                                                        m_start_threads,
                                                        m_threads_completed,
                                                        m_threadAction,
                                                        m_threadMove,
                                                        m_threadColor)));
    
    m_thread[1].reset(new boost::thread(WolveWorkThread(*m_no_fillin_board, 
                                                        m_start_threads,
                                                        m_threads_completed,
                                                        m_threadAction,
                                                        m_threadMove,
                                                        m_threadColor)));
}

void WolveSearch::StopThreads()
{
    if (!m_use_threads)
        return;

    LogInfo() << "WolveSearch::StopThreads()" << '\n';

    m_threadAction = WOLVE_THREAD_QUIT;
    m_start_threads.wait();
    
    m_thread[0]->join();
    m_thread[1]->join();

    m_thread[0].reset();
    m_thread[1].reset();
}

//----------------------------------------------------------------------------

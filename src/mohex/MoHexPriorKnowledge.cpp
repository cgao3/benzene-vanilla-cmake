//----------------------------------------------------------------------------
/** @file MoHexPriorKnowledge.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "MoHexThreadState.hpp"
#include "MoHexPriorKnowledge.hpp"
#include "MoHexSearch.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

MoHexPriorKnowledge::MoHexPriorKnowledge(const MoHexThreadState& state) 
    : m_state(state)
{
}

MoHexPriorKnowledge::~MoHexPriorKnowledge()
{
}

void MoHexPriorKnowledge::ProcessPosition(std::vector<SgUctMoveInfo>& moves,
                                          const bool doPruning)
{
    if (m_state.Search().ProgressiveBiasConstant() == 0.0f)
        return;

    bitset_t safe, pruned;
    double TotalGamma = 0;
    double MoveGamma[BITSETSIZE];
    const MoHexBoard& board = m_state.GetMoHexBoard();
    const MoHexPatterns& patterns = m_state.Search().GlobalPatterns();
    for (std::size_t i = 0; i < moves.size(); )
    {
        int type, killer;
        HexPoint move = (HexPoint)moves[i].m_move;
        double gamma = patterns.GetGammaFromBoard(board, 12, move, 
                                                  m_state.ColorToPlay(),
                                                  &type, &killer);
        if (doPruning && type && !safe.test(move))
        {
            if (type == 1) 
            {
                pruned.set(move);
                std::swap(moves[i], moves.back());
                moves.pop_back();
                continue;
            }
            else if (type == 2)
            {
                HexPoint killerMove = board.Const().PatternPoint(move, killer);
                if (board.GetColor(killerMove) != EMPTY)
                {
                    LogSevere() << board.Write() << '\n';
                    LogSevere() << "move=" << move << '\n';
                    LogSevere() << "killer=" << killerMove << ' ' << killer << '\n';
                    LogSevere() << "gamma=" << gamma << '\n';
                    throw BenzeneException("Killer not empty!!!\n");
                }
                if (!pruned.test(killerMove))
                {
                    safe.set(killerMove);
                    pruned.set(move);
                    if (moves.size() == 1)
                    {
                        LogSevere() << board.Write() << '\n';
                        LogSevere() << board.Write(pruned) << '\n';
                        LogSevere() << board.Write(safe) << '\n';
                        LogSevere() << "move=" << move << '\n';
                        LogSevere() << "killer=" << killerMove << '\n';
                        throw BenzeneException("Pruned dominated to empty!!\n");
                    }
                    std::swap(moves[i], moves.back());
                    moves.pop_back();
                    continue;
                }
            }
        }

        MoveGamma[(int)move] = gamma;
        TotalGamma += gamma;
        ++i;
    }
    if (TotalGamma == 0)
        return;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        double gamma = MoveGamma[(int)moves[i].m_move];
        double prob = gamma / TotalGamma;
        moves[i].m_prior = (float)prob;
	moves[i].m_raveValue = 0.5f;
	moves[i].m_raveCount = 8;
    }
}

//----------------------------------------------------------------------------

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
    float totalGamma = 0.0f;
    float moveGamma[BITSETSIZE];
    MoHexPatterns::Data data;
    const HexColor toPlay = m_state.ColorToPlay();
    const MoHexBoard& board = m_state.GetMoHexBoard();
    const MoHexPatterns& patterns = m_state.Search().GlobalPatterns();
    for (std::size_t i = 0; i < moves.size(); )
    {
        const HexPoint move = (HexPoint)moves[i].m_move;
        data.type = 0;
        data.gamma = 1.0f;
        patterns.Match(board, 12, move, toPlay, &data);

        if (doPruning && data.type && !safe.test(move))
        {
            if (data.type == 1) // can remove with no checks
            {
                pruned.set(move);
                std::swap(moves[i], moves.back());
                moves.pop_back();
                continue;
            }
            else if (data.type == 2)  // dominated
            {
                const HexPoint killerMove
                    = board.Const().PatternPoint(move, data.killer);
                if (board.GetColor(killerMove) != EMPTY)
                {
                    LogSevere() << board.Write() << '\n';
                    LogSevere() << "move=" << move << '\n';
                    LogSevere() << "killer=" << killerMove << ' ' 
                                << data.killer << '\n';
                    LogSevere() << "gamma=" << data.gamma << '\n';
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

        moveGamma[move] = data.gamma;
        totalGamma += data.gamma;
        ++i;
    }
    if (totalGamma < 1e-6)
        return;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        const float gamma = moveGamma[moves[i].m_move];
        const float prob = gamma / totalGamma;
        moves[i].m_prior = prob;
	moves[i].m_raveValue = 0.5f;
	moves[i].m_raveCount = 8;
    }
}

//----------------------------------------------------------------------------

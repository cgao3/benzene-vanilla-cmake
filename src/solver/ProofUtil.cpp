//----------------------------------------------------------------------------
/** @file ProofUtil.cpp
 */
//----------------------------------------------------------------------------

#include "Hex.hpp"
#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "DfsSolver.hpp"
#include "ProofUtil.hpp"
#include "SortedSequence.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

bitset_t ProofUtil::MaximumProofSet(const HexBoard& brd, HexColor toPlay)
{
    return brd.GetState().GetEmpty()
        | brd.GetState().GetPlayed(toPlay)
        | brd.GetInferiorCells().DeductionSet(toPlay);
}

bool ProofUtil::ShrinkProof(bitset_t& proof, const StoneBoard& board, 
                            HexColor loser, const ICEngine& ice)
{
    StoneBoard brd(board.Width(), board.Height());
    PatternState pastate(brd);
    Groups groups;

    // Give loser all cells outside proof
    bitset_t cells_outside_proof = (~proof & brd.Const().GetCells());
    brd.AddColor(loser, cells_outside_proof);

    // Give winner only his stones inside proof; 
    HexColor winner = !loser;
    brd.AddColor(winner, board.GetPlayed(winner) & proof);
    pastate.Update();
    GroupBuilder::Build(brd, groups);

    // Compute fillin and remove captured cells from the proof
    InferiorCells inf;
    ice.ComputeFillin(loser, groups, pastate, inf, 
                      HexColorSetUtil::Only(loser));
    HexAssert(inf.Captured(winner).none());

    bitset_t filled = inf.Dead() | inf.Captured(loser);
    bitset_t shrunk_proof = proof - filled;
    bool shrunkTheProof = shrunk_proof.count() < proof.count();
    proof = shrunk_proof;
    return shrunkTheProof;
}

//----------------------------------------------------------------------------

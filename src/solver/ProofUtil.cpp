//----------------------------------------------------------------------------
/** @file ProofUtil.cpp */
//----------------------------------------------------------------------------

#include "Hex.hpp"
#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "DfsSolver.hpp"
#include "ProofUtil.hpp"
#include "SortedSequence.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

bitset_t ProofUtil::MaximumProofSet(const HexBoard& brd, HexColor toPlay)
{
    return brd.GetPosition().GetEmpty()
      | brd.GetPosition().GetColor(toPlay); // includes the fillin
}

bitset_t ProofUtil::InitialProofForOpponent(const HexBoard& brd, 
                                            HexColor toPlay)
{
    // Add opponent played stones and deduction set.
    const InferiorCells& inf = brd.GetInferiorCells();
    bitset_t proof = brd.GetPosition().GetPlayed(!toPlay);
    proof |= inf.Fillin(!toPlay);

    // Add all semi-connections from the mustplay.
    proof |= brd.Cons(!toPlay).GetSmallestSemisUnion();

    // Add reversers.
    // TODO: Currently, we just add the first reverser: we should see if
    // any reverser is already in the proof, since then we wouldn't need
    // to add one.
    for (BitsetIterator p(inf.SReversible()); p; ++p) 
    {
        const std::set<HexPoint>& reversers = inf.SReversers(*p);
        proof.set(*reversers.begin());
    }
    for (BitsetIterator p(inf.Vulnerable()); p; ++p) 
    {
        const std::set<HexPoint>& killers = inf.Killers(*p);
        proof.set(*killers.begin());
    }
    return proof;
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
    brd.AddColor(winner, board.GetColor(winner) & proof);
    pastate.Update();
    GroupBuilder::Build(brd, groups);

    // Remove stones that the opponent could fillin from the proof
    InferiorCells inf;
    ice.ComputeFillin(groups, pastate, inf, loser,
		      ICEngine::MONOCOLOR);
    BenzeneAssert(inf.Fillin(winner).none());

    bitset_t shrunk_proof = proof - inf.Fillin(loser);
    bool shrunk_the_proof = shrunk_proof.count() < proof.count();
    proof = shrunk_proof;
    return shrunk_the_proof;
}

//----------------------------------------------------------------------------

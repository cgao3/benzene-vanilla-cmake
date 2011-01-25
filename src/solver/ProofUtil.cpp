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
        | brd.GetPosition().GetPlayed(toPlay)
        | brd.GetInferiorCells().DeductionSet(toPlay);
}

bitset_t ProofUtil::InitialProofForOpponent(const HexBoard& brd, 
                                            HexColor toPlay)
{
    // Add opponent played stones and deduction set.
    const InferiorCells& inf = brd.GetInferiorCells();
    bitset_t proof = brd.GetPosition().GetPlayed(!toPlay);
    proof |= inf.DeductionSet(!toPlay);

    // Add all semi-connections from the mustplay.
    const VCList& lst = brd.Cons(!toPlay).GetList(VC::SEMI, 
                                          HexPointUtil::colorEdge1(!toPlay),
                                          HexPointUtil::colorEdge2(!toPlay));
    const bool useGreedy = brd.Builder().Parameters().use_greedy_union;
    proof |= useGreedy ? lst.GetGreedyUnion() : lst.GetUnion();

    // Add reversable reversers. 
    // The carriers do NOT need to be included in the proof, since
    // they are captured by the (losing) player, not his opponent (for
    // whom we are building the proof set).
    // TODO: Currently, we just add the first reverser: we should see
    // if any reverser is already in the proof, since then we wouldn't
    // need to add one.
    for (BitsetIterator p(inf.Reversible()); p; ++p) 
    {
        const std::set<HexPoint>& reversers = inf.Reversers(*p);
        proof.set(*reversers.begin());
    }
    // Add vulnerable killers and their carriers.
    // TODO: Currently, we just add the first killer: we should see if
    // any killer is already in the proof, since then we wouldn't need
    // to add one.
    for (BitsetIterator p(inf.Vulnerable()); p; ++p) 
    {
        const std::set<VulnerableKiller>& killers = inf.Killers(*p);
        proof.set((*killers.begin()).killer());
        proof |= ((*killers.begin()).carrier());
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
    brd.AddColor(winner, board.GetPlayed(winner) & proof);
    pastate.Update();
    GroupBuilder::Build(brd, groups);

    // Compute fillin and remove captured cells from the proof
    InferiorCells inf;
    ice.ComputeFillin(loser, groups, pastate, inf, 
                      HexColorSetUtil::Only(loser));
    BenzeneAssert(inf.Captured(winner).none());

    bitset_t filled = inf.Dead() | inf.Captured(loser);
    bitset_t shrunk_proof = proof - filled;
    bool shrunkTheProof = shrunk_proof.count() < proof.count();
    proof = shrunk_proof;
    return shrunkTheProof;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file ProofUtil.hpp
 */
//----------------------------------------------------------------------------

#ifndef PROOFUTIL_HPP
#define PROOFUTIL_HPP

#include "Hex.hpp"
#include "BenzeneSolver.hpp"
#include "StoneBoard.hpp"
#include "ICEngine.hpp"
#include "SolverDB.hpp"
#include "SortedSequence.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Deduce equivalent states, etc. */
namespace ProofUtil 
{
    /** Compute the maximum possible proof. */
    bitset_t MaximumProofSet(const HexBoard& brd, HexColor toPlay);

    /** The proof for a losing state is the union the proofs for each
        move. This function returns the proof defined by all opponent
        semi-connctions as well as any ice deductions. 
        @note The proof is for the opponent of toPlay.
        @todo Argh, document this better. */
    bitset_t InitialProofForOpponent(const HexBoard& brd, HexColor toPlay);

    /** Gives all cells outside of the proof to loser, computes fillin
        using ice, removes any cell in proof that is filled-in. 
        Returns true if proof was shrunk. */
    bool ShrinkProof(bitset_t& proof, const StoneBoard& board,
                     HexColor loser, const ICEngine& ice);

    /** Computes and stores in db the transpostions of this proof on
        the given boardstate. Returns number of db entries
        successfully added or updated. */
    template<class HASH, class DB, class DATA>
    int StoreTranspositions(SolverDB<HASH,DB,DATA>& db, const DATA& data,
                            const HexState& state, const bitset_t& proof, 
                            HexColor winner);

    /** Computes and stores in db the flipped transpostions of this
        proof on the given boardstate. Returns number of db entries
        successfully added or updated. */
    template<class HASH, class DB, class DATA>
    int StoreFlippedStates(SolverDB<HASH,DB,DATA>& db, const DATA& data,
                           const HexState& state, const bitset_t& proof, 
                           HexColor winner);
}

//----------------------------------------------------------------------------

template<class HASH, class DB, class DATA>
int ProofUtil::StoreTranspositions(SolverDB<HASH,DB,DATA>& db, 
                                   const DATA& data, const HexState& state,
                                   const bitset_t& proof, HexColor winner)
{
    boost::function_requires< HasFlagsConcept<DATA> >();
    const StoneBoard& brd = state.Position();

    // Number of non-fillin game stones played
    std::size_t numBlack 
        = (brd.GetPlayed(BLACK) & brd.Const().GetCells()).count();
    std::size_t numWhite 
        = (brd.GetPlayed(WHITE) & brd.Const().GetCells()).count();
    BenzeneAssert(numBlack + numWhite == brd.NumStones());

    // Loser can use all his stones as well as all those outside the proof
    HexColor loser = !winner;
    bitset_t outside = (~proof & brd.GetEmpty()) 
        | (brd.GetColor(loser) & brd.Const().GetCells());

    // Winner can use all of his stones
    bitset_t winners = brd.GetColor(winner) & brd.Const().GetCells();

    // Store the players' stones as lists of sorted indices  
    std::vector<HexPoint> black, white;
    std::vector<HexPoint>& lose_list = (loser == BLACK) ? black : white;
    std::vector<HexPoint>& winn_list = (loser == BLACK) ? white : black;
    BitsetUtil::BitsetToVector(outside, lose_list);
    BitsetUtil::BitsetToVector(winners, winn_list);

    BenzeneAssert(black.size() >= (unsigned)numBlack);
    BenzeneAssert(white.size() >= (unsigned)numWhite);

    // Write each transposition 
    int count = 0;
    HexState myState(state);
    StoneBoard& board = myState.Position();
    SortedSequence bseq(black.size(), numBlack);
    while (!bseq.finished()) 
    {
        SortedSequence wseq(white.size(), numWhite);
        while (!wseq.finished()) 
        {
            // Convert the indices into cells
            board.StartNewGame();
            for (std::size_t i = 0; i < numBlack; ++i)
                board.PlayMove(BLACK, black[bseq[i]]);
            for (std::size_t i = 0; i < numWhite; ++i)
                board.PlayMove(WHITE, white[wseq[i]]);

            // Mark state as transposition if the current one is not
            // the original.
            DATA ss(data);
            if (board.Hash() != brd.Hash())
                ss.m_flags |= SolverDataFlags::TRANSPOSITION;
            db.Put(myState, ss);
            ++count;
            
            ++wseq;
        }
        ++bseq;
    }
    return count;
}

template<class HASH, class DB, class DATA>
int ProofUtil::StoreFlippedStates(SolverDB<HASH,DB,DATA>& db, const DATA& data,
                                  const HexState& state, const bitset_t& proof,
                                  HexColor winner)
{
    boost::function_requires< HasFlagsConcept<DATA> >();
    boost::function_requires< HasMirrorConcept<DATA> >();
    const StoneBoard& brd = state.Position();

    // Start by computing the flipped board position.
    // This involves mirroring the stones and *flipping their colour*.
    bitset_t flippedBlack = BoardUtil::Mirror(brd.Const(), 
                    brd.GetPlayed(WHITE) & brd.Const().GetCells());
    bitset_t flippedWhite = BoardUtil::Mirror(brd.Const(),
                    brd.GetPlayed(BLACK) & brd.Const().GetCells());
    StoneBoard flippedBrd(brd.Width(), brd.Height());
    flippedBrd.AddColor(BLACK, flippedBlack);
    flippedBrd.AddColor(WHITE, flippedWhite);
    flippedBrd.SetPlayed(flippedBlack | flippedWhite);

    
    HexColor flippedWinner = !winner;
    bitset_t flippedProof = BoardUtil::Mirror(brd.Const(), proof);
    bitset_t flippedOutside = (~flippedProof & flippedBrd.GetEmpty());
    
    // Determine what stones we can add or remove.
    bool canAddFlippedBlack = false;
    bitset_t flippedBlackToAdd;
    bool canRemoveFlippedWhite = false;
    bitset_t flippedWhiteToRemove;
    // To switch player to move (while keeping parity valid), we must
    // either add one stone to flippedBlack or else delete 1 stone
    // from flippedWhite. Note that we can always add winner stones or
    // delete loser stones without changing the value, and we can add
    // loser stones if the proof set does not cover all empty cells.
    if (flippedWinner == BLACK) 
    {
	canAddFlippedBlack = true;
	flippedBlackToAdd = flippedBrd.GetEmpty();
	canRemoveFlippedWhite = true;
	flippedWhiteToRemove = flippedWhite;
    } 
    else 
    {
	BenzeneAssert(flippedWinner == WHITE);
	canAddFlippedBlack = flippedOutside.any();
	flippedBlackToAdd = flippedOutside;
    }
    BenzeneAssert(canAddFlippedBlack != flippedBlackToAdd.none());
    BenzeneAssert(BitsetUtil::IsSubsetOf(flippedBlackToAdd,flippedBrd.GetEmpty()));
    BenzeneAssert(canRemoveFlippedWhite != flippedWhiteToRemove.none());
    BenzeneAssert(BitsetUtil::IsSubsetOf(flippedWhiteToRemove,flippedWhite));
    
    // Now we can create and store the desired flipped states.
    DATA ss(data);
    ss.Mirror(brd.Const());
    ss.m_flags |= SolverDataFlags::TRANSPOSITION;
    ss.m_flags |= SolverDataFlags::MIRROR_TRANSPOSITION;
    
    int count = 0;
    if (canAddFlippedBlack) 
    {
	for (BitsetIterator i(flippedBlackToAdd); i; ++i) 
        {
	    flippedBrd.PlayMove(BLACK, *i);
	    BenzeneAssert(!state.ToPlay() == flippedBrd.WhoseTurn());
	    db.Put(HexState(flippedBrd, WHITE), ss);
	    flippedBrd.UndoMove(*i);
            ++count;
	}
    }
    if (canRemoveFlippedWhite) 
    {
	for (BitsetIterator i(flippedWhiteToRemove); i; ++i) 
        {
	    flippedBrd.UndoMove(*i);
	    BenzeneAssert(!state.ToPlay() == flippedBrd.WhoseTurn());
	    db.Put(HexState(flippedBrd, WHITE), ss);
	    flippedBrd.PlayMove(WHITE, *i);
            ++count;
	}
    }
    return count;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PROOFUTIL_HPP

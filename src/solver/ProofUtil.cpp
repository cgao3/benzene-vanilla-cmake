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

/** Dumps some debug output. */
#define PRINT_OUTPUT 0

//----------------------------------------------------------------------------

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

int ProofUtil::StoreTranspositions(DfsDB& db, const StoneBoard& brd, 
                                   const DfsData& state, const bitset_t& proof)
{
    int numstones = brd.NumStones();
    int numblack = (numstones + 1) / 2;
    int numwhite = numstones / 2;
    HexAssert(numblack + numwhite == numstones);

    // Find color of losing/winning players
    HexColor toplay = brd.WhoseTurn();
    HexColor other = !toplay;
    HexColor loser = (state.m_win) ? other : toplay;
    HexColor winner = (state.m_win) ? toplay : other;

    // Loser can use his stones as well as all those outside the proof
    bitset_t outside = (~proof & brd.GetEmpty()) 
        | (brd.GetColor(loser) & brd.Const().GetCells());

    // Winner can use his stones
    // @todo fix this so only relevant stones to the proof are used.
    bitset_t winners = brd.GetColor(winner) & brd.Const().GetCells();

    // store the players' stones as lists of sorted indices  
    std::vector<HexPoint> black, white;
    std::vector<HexPoint>& lose_list = (loser == BLACK) ? black : white;
    std::vector<HexPoint>& winn_list = (loser == BLACK) ? white : black;
    BitsetUtil::BitsetToVector(outside, lose_list);
    BitsetUtil::BitsetToVector(winners, winn_list);

    HexAssert(black.size() >= (unsigned)numblack);
    HexAssert(white.size() >= (unsigned)numwhite);

#if 0
    LogInfo() << "[" << numstones << "]" 
	      << " StoreTranspositions\n"
	      << brd.Write(proof & brd.getEmpty()) << '\n'
	      << "Winner: " << winner << '\n'
	      << "Black positions: " << black.size() << '\n'
	      << HexPointUtil::ToString(black)<< '\n'
	      << "White positions: " << white.size() << '\n'
	      << HexPointUtil::ToString(white)<< '\n'
	      << "outside proof:\n" << brd.Write(outside) << '\n';
#endif

    // write each transposition 
    int count = 0;
    StoneBoard board(brd.Width(), brd.Height());
    SortedSequence bseq(black.size(), numblack);
    while (!bseq.finished()) 
    {
        SortedSequence wseq(white.size(), numwhite);
        while (!wseq.finished()) 
        {
            // convert the indices into cells
            board.StartNewGame();
            for (int i = 0; i < numblack; ++i)
                board.PlayMove(BLACK, black[bseq[i]]);
            for (int i = 0; i < numwhite; ++i)
                board.PlayMove(WHITE, white[wseq[i]]);

            // mark state as transposition if the current one is not
            // the original.
            DfsData ss(state);
            if (board.Hash() != brd.Hash())
                ss.m_flags |= DfsData::FLAG_TRANSPOSITION;
            
            // do the write; this handles replacing only larger
            // proofs, etc.
            count += db.Put(board, ss);
            
            ++wseq;
        }
        ++bseq;
    }
    return count;
}

int ProofUtil::StoreFlippedStates(DfsDB& db, const StoneBoard& brd,
                                  const DfsData& state, const bitset_t& proof)
{
    // Start by computing the flipped board position.
    // This involves mirroring the stones and *flipping their colour*.
    bitset_t flippedBlack = BoardUtils::Mirror(brd.Const(), 
                    brd.GetPlayed(WHITE) & brd.Const().GetCells());
    bitset_t flippedWhite = BoardUtils::Mirror(brd.Const(),
                    brd.GetPlayed(BLACK) & brd.Const().GetCells());
    StoneBoard flippedBrd(brd.Width(), brd.Height());
    flippedBrd.AddColor(BLACK, flippedBlack);
    flippedBrd.AddColor(WHITE, flippedWhite);
    flippedBrd.SetPlayed(flippedBlack | flippedWhite);
#if PRINT_OUTPUT
    LogInfo() << "Original Board:" << brd << '\n'
              << "Flipped Board:" << flippedBrd << '\n';
#endif
    
    // Find color of winning player in *flipped state*
    /** @todo Ensure position reachable in a normal game! */
    HexColor toPlay = brd.WhoseTurn();
    HexColor flippedWinner = (state.m_win) ? !toPlay : toPlay;
#if PRINT_OUTPUT
    LogInfo() << "Normal winner: "
	     << (state.win ? toPlay : !toPlay) << '\n';
    LogInfo() << "Flipped winner: "
	     << flippedWinner << '\n';
#endif
    
    // Find empty cells outside the flipped proof, if any
    bitset_t flippedProof = BoardUtils::Mirror(brd.Const(), proof);
    bitset_t flippedOutside = (~flippedProof & flippedBrd.GetEmpty());
#if PRINT_OUTPUT
    LogInfo() << "Flipped proof:"
              << flippedBrd.Write(flippedProof) << '\n';
#endif
    
    // We need to determine what stones we can add or remove.
    bool canAddFlippedBlack = false;
    bitset_t flippedBlackToAdd;
    bool canRemoveFlippedWhite = false;
    bitset_t flippedWhiteToRemove;
    // To switch player to move (while keeping parity valid), we must either
    // add 1 stone to flippedBlack or else delete 1 stone from flippedWhite.
    // Note that we can always add winner stones or delete loser stones
    // without changing the value, and we can add loser stones if the proof
    // set does not cover all empty cells.
    if (flippedWinner == BLACK) 
    {
	canAddFlippedBlack = true;
	flippedBlackToAdd = flippedBrd.GetEmpty();
	canRemoveFlippedWhite = true;
	flippedWhiteToRemove = flippedWhite;
    } 
    else 
    {
	HexAssert(flippedWinner == WHITE);
	canAddFlippedBlack = flippedOutside.any();
	flippedBlackToAdd = flippedOutside;
    }
    HexAssert(canAddFlippedBlack != flippedBlackToAdd.none());
    HexAssert(BitsetUtil::IsSubsetOf(flippedBlackToAdd,flippedBrd.GetEmpty()));
    HexAssert(canRemoveFlippedWhite != flippedWhiteToRemove.none());
    HexAssert(BitsetUtil::IsSubsetOf(flippedWhiteToRemove,flippedWhite));
    
    // Now we can create and store the desired flipped states.
    // Note that numstates and nummoves are approximations.
    DfsData ss;
    ss.m_win = state.m_win;
    ss.m_flags = state.m_flags 
        | DfsData::FLAG_TRANSPOSITION 
        | DfsData::FLAG_MIRROR_TRANSPOSITION;
    ss.m_numstates = state.m_numstates;
    ss.m_nummoves = state.m_nummoves;
    ss.m_bestmove = BoardUtils::Mirror(brd.Const(), state.m_bestmove);
    //ss.proof = flippedProof;
    
    int count = 0;
    if (canAddFlippedBlack) 
    {
#if PRINT_OUTPUT
	LogInfo() << "Add-Black Flips:" << '\n';
#endif
	for (BitsetIterator i(flippedBlackToAdd); i; ++i) 
        {
	    flippedBrd.PlayMove(BLACK, *i);
	    HexAssert(!toPlay == flippedBrd.WhoseTurn());
//             HexAssert(!ss.winners_stones.test(*i));
//             if (flippedWinner == BLACK) {
//                 ss.proof.set(*i);
//             }
#if PRINT_OUTPUT
	    LogInfo() << flippedBrd << '\n';
#endif
	    count += db.Put(flippedBrd, ss);
	    //ss.proof = flippedProof;
	    flippedBrd.UndoMove(*i);
	}
    }
    if (canRemoveFlippedWhite) 
    {
#if PRINT_OUTPUT
	LogInfo() << "Remove-White Flips:\n";
#endif
	for (BitsetIterator i(flippedWhiteToRemove); i; ++i) 
        {
	    flippedBrd.UndoMove(*i);
	    HexAssert(!toPlay == flippedBrd.WhoseTurn());
#if PRINT_OUTPUT
	    LogInfo() << flippedBrd << '\n';
#endif
	    count += db.Put(flippedBrd, ss);
	    flippedBrd.PlayMove(WHITE, *i);
	}
    }
    return count;
}

//----------------------------------------------------------------------------

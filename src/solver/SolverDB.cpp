//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include <cstring>
#include <cstdio>

#include "Time.hpp"
#include "Hex.hpp"
#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "SolverDB.hpp"
#include "SortedSequence.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Dumps some debug output. */
#define PRINT_OUTPUT 0

//----------------------------------------------------------------------------

SolverDB::SolverDB()
{
}

SolverDB::~SolverDB()
{
}

//----------------------------------------------------------------------------


void SolverDB::open(int width, int height, int maxstones, int transtones,
                    const std::string& filename)
    throw(HexException)
{
    m_settings = Settings(width, height, transtones, maxstones);

    if (!m_db.Open(filename))
        throw HexException("Could not open database file!");

    // Load settings from database and ensure they match the current
    // settings.  
    char key[] = "settings";
    Settings temp;
    if (m_db.Get(key, strlen(key)+1, &temp, sizeof(temp))) 
    {
        LogInfo() << "Database exists." << '\n';
        if (m_settings != temp) 
        {
            LogInfo() << "Settings do not match!" << '\n'
		      << "DB: " << temp.toString() << '\n'
		      << "Current: " << m_settings.toString() << '\n';
            throw HexException("Settings do not match db settings!");
        } 
    } 
    else 
    {
        // Read failed: this is a new database. Store the settings.
        LogInfo() << "New database!" << '\n';
        if (!m_db.Put(key, strlen(key)+1, &m_settings, sizeof(m_settings)))
            throw HexException("Could not write to database!");
    }
    LogInfo() << "Settings: " << m_settings.toString() << '\n';
}

void SolverDB::open(int width, int height, const std::string& filename)
    throw(HexException)
{
    if (!m_db.Open(filename))
        throw HexException("Could not open database!");

    // Load settings from database
    char key[] = "settings";
    if (m_db.Get(key, strlen(key)+1, &m_settings, sizeof(m_settings))) 
    {
        LogInfo() << "Settings: " << m_settings.toString() << '\n';
        if (m_settings.width != width || m_settings.height != height)
            throw HexException("Dimensions do not match!");
    }
    else
        throw HexException("Could not read from database!");
}

void SolverDB::close()
{
    m_db.Close();
}

//----------------------------------------------------------------------------

bool SolverDB::get(const StoneBoard& brd, SolvedState& state)
{
    int count = brd.NumStones();
    if (0 < count && count <= m_settings.maxstones) {

        // check if exact boardstate exists in db
        if (m_db.Get(brd.Hash(), state)) 
        {
            m_stats.gets++;
            m_stats.saved += state.numstates;
           
            state.numstones = brd.NumStones();
            return true;
        }

        // check if rotated boardstate exists in db
        StoneBoard rotated_brd(brd);
        rotated_brd.RotateBoard();
        if (m_db.Get(rotated_brd.Hash(), state)) 
        {
            m_stats.gets++;
            m_stats.saved += state.numstates;

            // rotate data so it matches the given board
            state.proof = BoardUtils::Rotate(brd.Const(), state.proof);
            state.winners_stones = BoardUtils::Rotate(brd.Const(), 
                                                      state.winners_stones);
            state.bestmove = BoardUtils::Rotate(brd.Const(), 
                                                state.bestmove);

            state.numstones = brd.NumStones();
            return true;
        }
    }
    return false;
}

bool SolverDB::check(const StoneBoard& brd)
{
    int count = brd.NumStones();
    if (0 < count && count <= m_settings.maxstones) {
        if (m_db.Exists(brd.Hash()))
            return true;

        StoneBoard rotated_brd(brd);
        rotated_brd.RotateBoard();
        if (m_db.Exists(rotated_brd.Hash()))
            return true;
    }
    return false;
}

int SolverDB::write(const StoneBoard& brd, const SolvedState& state)
{
    int count = brd.NumStones();
    if (0 < count && count <= m_settings.maxstones) {
        
        SolvedState old_state;
        bool old_exists = get(brd, old_state);

        if (old_exists && old_state.win != state.win) {
            LogSevere()
                     << "old win = " << old_state.win << '\n'
                     << "new win = " << state.win << '\n'
                     << "old_proof = " 
                     << brd.Write(old_state.proof & brd.GetEmpty())
                     << '\n'
                     << "new_proof = " 
                     << brd.Write(state.proof & brd.GetEmpty()) 
                     << '\n';
            HexAssert(false);
        }

        // do not overwrite a proof unless the new one is smaller
        if (old_exists && old_state.proof.count() <= state.proof.count())
            return 0;

        // track the shrinkage
        if (old_exists) {
            m_stats.shrunk++;
            m_stats.shrinkage += 
                old_state.proof.count() - state.proof.count();
        }
        
        if (m_db.Put(brd.Hash(), state)) {
            m_stats.writes++;
            return 1;
        }
    }
    return 0;
}

int SolverDB::put(const StoneBoard& brd, const SolvedState& state)
{
    int count = brd.NumStones();
    if (0 < count && count <= m_settings.maxstones) {

        int wrote = write(brd, state);
        if (count <= m_settings.trans_stones) {
            wrote += SolverDBUtil::StoreTranspositions(*this, brd, state);
            wrote += SolverDBUtil::StoreFlippedStates(*this, brd, state);
        }
	
        if (wrote) {
            m_stats.puts++;
        }
        return wrote;
    }
    return 0;
}

//----------------------------------------------------------------------------

int SolverDBUtil::StoreTranspositions(SolverDB& db, 
                                      const StoneBoard& brd, 
                                      const SolvedState& state)
{
    int numstones = brd.NumStones();
    int numblack = (numstones+1) / 2;
    int numwhite = numstones / 2;
    HexAssert(numblack + numwhite == numstones);

    // Find color of losing/winning players
    HexColor toplay = brd.WhoseTurn();
    HexColor other = !toplay;
    HexColor loser = (state.win) ? other : toplay;
    HexColor winner = (state.win) ? toplay : other;

    // Loser can use his stones as well as all those outside the proof
    bitset_t outside = (~state.proof & brd.GetEmpty()) 
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
	      << " StoreTranspositions" << '\n'
	      << brd.Write(state.proof & brd.getEmpty()) << '\n'
	      << "Winner: " << winner << '\n'
	      << "Black positions: " << black.size() << '\n'
	      << HexPointUtil::ToPointListString(black)<< '\n'
	      << "White positions: " << white.size() << '\n'
	      << HexPointUtil::ToPointListString(white)<< '\n'
	      << "outside proof:" << '\n'
	      << brd.Write(outside) << '\n';
#endif

    // write each transposition 
    int count=0;
    StoneBoard board(brd.Width(), brd.Height());
    SortedSequence bseq(black.size(), numblack);
    while (!bseq.finished()) {
        SortedSequence wseq(white.size(), numwhite);
        while (!wseq.finished()) {

            // convert the indices into cells
            board.StartNewGame();
            for (int i=0; i<numblack; ++i) {
                board.PlayMove(BLACK, black[bseq[i]]);
            }
            for (int i=0; i<numwhite; ++i) {
                board.PlayMove(WHITE, white[wseq[i]]);
            }

            // mark state as transposition if the current one is not
            // the original.
            SolvedState ss(state);
            if (board.Hash() != brd.Hash())
                ss.flags |= SolvedState::FLAG_TRANSPOSITION;
            
            // do the write; this handles replacing only larger
            // proofs, etc.
            count += db.write(board, ss);
            
            ++wseq;
        }
        ++bseq;
    }
    return count;
}


int SolverDBUtil::StoreFlippedStates(SolverDB& db, 
                                     const StoneBoard& brd,
                                     const SolvedState& state)
{
    // Start by computing the flipped board position.
    // This involves mirroring the stones and *flipping their colour*.
    bitset_t flippedBlack = BoardUtils::Mirror(brd.Const(), 
                    brd.GetWhite() & brd.GetPlayed() & brd.Const().GetCells());
    bitset_t flippedWhite = BoardUtils::Mirror(brd.Const(),
                    brd.GetBlack() & brd.GetPlayed() & brd.Const().GetCells());
    StoneBoard flippedBrd(brd.Width(), brd.Height());
    flippedBrd.AddColor(BLACK, flippedBlack);
    flippedBrd.AddColor(WHITE, flippedWhite);
    flippedBrd.SetPlayed(flippedBlack | flippedWhite);
#if PRINT_OUTPUT
    LogInfo() << "Original Board:" << brd << '\n'
	     << "Flipped Board:" << flippedBrd << '\n';
#endif
    
    // Find color of winning player in *flipped state*
    HexColor toPlay = brd.WhoseTurn();
    HexColor flippedWinner = (state.win) ? !toPlay : toPlay;
#if PRINT_OUTPUT
    LogInfo() << "Normal winner: "
	     << (state.win ? toPlay : !toPlay) << '\n';
    LogInfo() << "Flipped winner: "
	     << flippedWinner << '\n';
#endif
    
    // Find empty cells outside the flipped proof, if any
    bitset_t flippedProof = BoardUtils::Mirror(brd.Const(), state.proof);
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
    if (flippedWinner == BLACK) {
	canAddFlippedBlack = true;
	flippedBlackToAdd = flippedBrd.GetEmpty();
	canRemoveFlippedWhite = true;
	flippedWhiteToRemove = flippedWhite;
    } else {
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
    SolvedState ss;
    ss.win = state.win;
    ss.flags = state.flags 
        | SolvedState::FLAG_TRANSPOSITION 
        | SolvedState::FLAG_MIRROR_TRANSPOSITION;
    ss.numstates = state.numstates;
    ss.nummoves = state.nummoves;
    ss.bestmove = BoardUtils::Mirror(brd.Const(), state.bestmove);
    ss.proof = flippedProof;
    ss.winners_stones = (flippedWinner == BLACK) ? flippedBlack : flippedWhite;
    
    int count = 0;
    if (canAddFlippedBlack) {
#if PRINT_OUTPUT
	LogInfo() << "Add-Black Flips:" << '\n';
#endif
	for (BitsetIterator i(flippedBlackToAdd); i; ++i) {
	    flippedBrd.PlayMove(BLACK, *i);
	    HexAssert(!toPlay == flippedBrd.WhoseTurn());
	    HexAssert(!ss.winners_stones.test(*i));
	    if (flippedWinner == BLACK) {
		ss.winners_stones.set(*i);
		ss.proof.set(*i);
	    }
#if PRINT_OUTPUT
	    LogInfo() << flippedBrd << '\n';
#endif
	    count += db.write(flippedBrd, ss);
	    ss.proof = flippedProof;
	    ss.winners_stones.reset(*i);
	    flippedBrd.UndoMove(*i);
	}
    }
    if (canRemoveFlippedWhite) {
#if PRINT_OUTPUT
	LogInfo() << "Remove-White Flips:" << '\n';
#endif
	for (BitsetIterator i(flippedWhiteToRemove); i; ++i) {
	    flippedBrd.UndoMove(*i);
	    HexAssert(!toPlay == flippedBrd.WhoseTurn());
#if PRINT_OUTPUT
	    LogInfo() << flippedBrd << '\n';
#endif
	    count += db.write(flippedBrd, ss);
	    flippedBrd.PlayMove(WHITE, *i);
	}
    }
    return count;
}

//----------------------------------------------------------------------------

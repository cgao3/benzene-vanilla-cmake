//----------------------------------------------------------------------------
// $Id: PatternBoard.hpp 1897 2009-02-05 01:43:36Z broderic $
//----------------------------------------------------------------------------

#ifndef PATTERNBOARD_HPP
#define PATTERNBOARD_HPP

#include "Hex.hpp"
#include "GroupBoard.hpp"
#include "Pattern.hpp"
#include "HashedPatternSet.hpp"

//----------------------------------------------------------------------------

/** Instance of a pattern matching a subset of the board.  */
class PatternHit
{
public:

    /** Creates an instance with a single encoded move1, and
        move2 is empty. */
    PatternHit(const Pattern* pat, HexPoint move);

    /** Creates an instance with a set of encoded move1, and
        move2 is empty. */
    PatternHit(const Pattern* pat, const std::vector<HexPoint>& moves1);

    /** Creates an instance with two sets of encoded moves. */
    PatternHit(const Pattern* pat, 
               const std::vector<HexPoint>& moves1,
               const std::vector<HexPoint>& moves2);

    /** Returns the pattern. */
    const Pattern* pattern() const;

    /** Returns the set of moves the pattern encodes. */
    const std::vector<HexPoint>& moves1() const;
    const std::vector<HexPoint>& moves2() const;

private:

    const Pattern* m_pattern;

    std::vector<HexPoint> m_moves1;

    std::vector<HexPoint> m_moves2;
};

inline PatternHit::PatternHit(const Pattern* pat, HexPoint move)
    : m_pattern(pat),
      m_moves1(1, move),
      m_moves2()
{
}

inline PatternHit::PatternHit(const Pattern* pat, 
                              const std::vector<HexPoint>& moves1)
    : m_pattern(pat),
      m_moves1(moves1),
      m_moves2()
{
}

inline PatternHit::PatternHit(const Pattern* pat, 
                              const std::vector<HexPoint>& moves1,
                              const std::vector<HexPoint>& moves2)
    : m_pattern(pat),
      m_moves1(moves1),
      m_moves2(moves2)
{
}

inline const Pattern* PatternHit::pattern() const
{
    return m_pattern;
}

inline const std::vector<HexPoint>& PatternHit::moves1() const
{
    return m_moves1;
}

inline const std::vector<HexPoint>& PatternHit::moves2() const
{
    return m_moves2;
}

/** Vector of PatternHits. */
typedef std::vector<PatternHit> PatternHits;

//----------------------------------------------------------------------------

/** Data used for pattern matching. */
class PatternMatcherData
{
public:

    /** Returns instance for given board. */
    static const PatternMatcherData* Get(const ConstBoard* brd);

    /** Board data is defined on. */
    const ConstBoard* brd;

    /** For x: slice in which y resides. */
    int played_in_slice[BITSETSIZE][BITSETSIZE];

    /** For x: godel in the slice in which y resides. */
    int played_in_godel[BITSETSIZE][BITSETSIZE];

    /** For x, edge y, slice s: set of godels edge hits. */
    int played_in_edge[BITSETSIZE][4][Pattern::NUM_SLICES];

    /** Maps a cell's (slice,godel) to a point. */
    HexPoint inverse_slice_godel[BITSETSIZE][Pattern::NUM_SLICES][32];

private:

    /** Constructor. */
    PatternMatcherData(const ConstBoard* brd);

    void Initialize();
};

//----------------------------------------------------------------------------

/** Performs pattern matching on a StoneBoard.
 
    Before trying to match a pattern, update() must be called to
    calculate the pattern matching information.  Once update() has been
    called, update(cell) may be used to update the pattern matching
    given the single move of color to cell.
    
    Any method in StoneBoard that changes the color of a cell will
    invalidate the pattern matching information; update(), or
    update(cell) for each cell that changed, must be called beforehand
    or the pattern matching methods will return unpredicable results.
  */
class PatternBoard : public GroupBoard
{
public:

    /** Creates a rectangular board. */
    PatternBoard(int width, int height);
    
    /** Destructor. */
    virtual ~PatternBoard();

    //-----------------------------------------------------------------------

    /** Sets the distance to which we update pattern info from the
        last played cell; used in update(cell). Default is
        Pattern::MAX_EXTENSION. */
    void setUpdateRadius(int radius);

    /** Returns the update radius. See setUpdateRadius(). */
    int updateRadius() const;

    //-----------------------------------------------------------------------

    /** Update only the ring godels of the neighbours of cell. */
    void updateRingGodel(HexPoint cell);

    /** Computes the pattern checking information for this board
        state.  Calls update(cell) for each occupied cell. */
    void update();

    /** Updates the pattern checking information only for the given
        move.  Sweeps over all cells updateRadius() distance from
        cell. */
    void update(HexPoint cell);

    /** Calls update(cell) for each move in changed, each of which
        must correspond to an occupied cell. */
    void update(const bitset_t& changed);

    //-----------------------------------------------------------------------

    /** Checks the pre-rotated pattern against the board. Returns true
        if it matches.  Pattern encoded moves are stored in moves. */
    bool checkRotatedPattern(HexPoint cell, 
                             const RotatedPattern& rotpat,
                             std::vector<HexPoint>& moves1,
                             std::vector<HexPoint>& moves2) const;

    //-----------------------------------------------------------------------

    /** Option controlling pattern matching behavoir at a cell. */
    typedef enum 
    {
        /** Stops the search after first hit. */
        STOP_AT_FIRST_HIT, 

        /** Continues search after first hit, storing all results. */
        MATCH_ALL 

    } MatchMode;

    /** Matches the hashed patterns at the specified cell, storing hit
        information in hits, using the given matching mode. */
    void matchPatternsOnCell(const HashedPatternSet& patset, 
                             HexPoint cell, MatchMode mode,
                             PatternHits& hits) const;

    /** Matches the hashed patterns on the consider set, returning a
        set of cells where at least one pattern matched. Note that
        hits must be large enough that it can be indexed by each cell
        in consider. Matching mode refers to a single cell, not the
        search as a whole; that is, a hit on cell A does not abort the
        entire search, it only moves the search on to the remaining
        cells.
        
        @todo Can we switch hits to be a map instead of a vector?
        Will a map be too slow?
    */
    bitset_t matchPatternsOnBoard(const bitset_t& consider, 
                                  const HashedPatternSet& patset, 
                                  MatchMode mode, 
                                  std::vector<PatternHits>& hits) const;

    /** Matches the hashed patterns on the given consider set,
        returning a set of cells where at least one pattern
        matched. For each cell, the search is aborted after the first
        match. No information on the hits is returned. This is a
        convience method. */
    bitset_t matchPatternsOnBoard(const bitset_t& consider,
                                  const HashedPatternSet& patset) const;
        
    //-----------------------------------------------------------------------

    /** Reset the pattern checking statistics. */
    void ClearPatternCheckStats();

    /** Return a string containing the pattern checking statistics. */
    std::string DumpPatternCheckStats();

protected:

    /** Clears pattern info. */
    virtual void clear();

private:

    /** Clears current pattern matching info. */
    void clearGodels();

    /** No assignment of PatternBoards allowed! */
    void operator=(const PatternBoard& other);

    //-----------------------------------------------------------------------

    /** Returns the HexPoint of the position (slice, bit) centered on
        cell and rotated by angle. */
    HexPoint getRotatedMove(HexPoint cell, int slice, int bit, int angle) const;

    /** Returns true if pattern's slices rotated by angle match the
        board when pattern is centered at cell. */
    bool checkRotatedSlices(HexPoint cell, const Pattern& pat, int angle) const;
    bool checkRotatedSlices(HexPoint cell, const RotatedPattern& rotpat) const;
        
    /** Returns true if the pattern's ring godel matches the board. */
    bool checkRingGodel(HexPoint cell, const Pattern& pattern, int angle) const;
    bool checkRingGodel(HexPoint cell, const RotatedPattern& rotpat) const;

    //-----------------------------------------------------------------------

    /** Pattern checking stats. */
    struct Statistics
    {
        /** Number of pattern checks. */
        u64 pattern_checks;

        /** Number of calls to checkRingGodel(). */
        u64 ring_checks;

        /** Number of slice checks. */
        u64 slice_checks;
    };

    //-----------------------------------------------------------------------

    /** See updateRadius() */
    int m_update_radius;

    int m_slice_godel[BITSETSIZE][BLACK_AND_WHITE][Pattern::NUM_SLICES];

    RingGodel m_ring_godel[BITSETSIZE];

    mutable Statistics m_statistics;

    //-----------------------------------------------------------------------

    const PatternMatcherData* m_data;
};

inline void PatternBoard::setUpdateRadius(int radius)
{
    HexAssert(1 <= radius);
    HexAssert(radius <= Pattern::MAX_EXTENSION);
    m_update_radius = radius;
}

inline int PatternBoard::updateRadius() const
{
    return m_update_radius;
}

//----------------------------------------------------------------------------

#endif // PATTERNBOARD_HPP

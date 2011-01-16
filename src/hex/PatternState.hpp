//----------------------------------------------------------------------------
/** @file PatternState.hpp */
//----------------------------------------------------------------------------

#ifndef PATTERNSTATE_HPP
#define PATTERNSTATE_HPP

#include "Hex.hpp"
#include "HashedPatternSet.hpp"
#include "Pattern.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

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
    const Pattern* GetPattern() const;

    /** Returns the set of moves the pattern encodes. */
    const std::vector<HexPoint>& Moves1() const;

    const std::vector<HexPoint>& Moves2() const;

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

inline const Pattern* PatternHit::GetPattern() const
{
    return m_pattern;
}

inline const std::vector<HexPoint>& PatternHit::Moves1() const
{
    return m_moves1;
}

inline const std::vector<HexPoint>& PatternHit::Moves2() const
{
    return m_moves2;
}

//----------------------------------------------------------------------------

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

    /** Returns the HexPoint of the position (slice, bit) centered on cell
        and rotated by angle. */
    HexPoint GetRotatedMove(HexPoint cell, int slice, 
                            int bit, int angle) const;

private:
    /** Constructor. */
    PatternMatcherData(const ConstBoard* brd);

    void Initialize();
};

//----------------------------------------------------------------------------

/** Tracks pattern state info on a board. */
class PatternState
{
public:
    /** Track the pattern state on the given board. */
    explicit PatternState(StoneBoard& brd);

    ~PatternState();

    //-----------------------------------------------------------------------

    /** Returns board state is tracking. */
    const StoneBoard& Board() const;

    /** Returns board state is tracking. */
    StoneBoard& Board();

    /** Copies state from other. */
    void CopyState(const PatternState& other);

    /** Sets the distance to which we update pattern info from the
        last played cell; used in Update(cell). Default is
        Pattern::MAX_EXTENSION. */
    int UpdateRadius() const;

    /** See SetUpdateRadius(). */
    void SetUpdateRadius(int radius);

    /** Computes the pattern checking information for this board
        state.  Calls Update(cell) for each occupied cell. */
    void Update();

    /** Updates the pattern checking information only for the given
        move.  Sweeps over all cells updateRadius() distance from
        cell. */
    void Update(HexPoint cell);

    /** Calls Update(cell) for each move in changed, each of which
        must correspond to an occupied cell. */
    void Update(const bitset_t& changed);

    /** Update only the ring godels of the neighbours of cell. */
    void UpdateRingGodel(HexPoint cell);

    //-----------------------------------------------------------------------

    /** @name Pattern Matching */

    // @{

    /** Options controlling pattern matching behavoir at a cell. */
    typedef enum 
    {
        /** Stops the search after first hit. */
        STOP_AT_FIRST_HIT, 

        /** Continues search after first hit, storing all results. */
        MATCH_ALL 
    } MatchMode;

    //-----------------------------------------------------------------------

    /** Matches the hashed patterns at the specified cell, storing hit
        information in hits, using the given matching mode. */
    void MatchOnCell(const HashedPatternSet& patset, 
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
    bitset_t MatchOnBoard(const bitset_t& consider, 
                          const HashedPatternSet& patset, 
                          MatchMode mode, 
                          std::vector<PatternHits>& hits) const;

    /** Matches the hashed patterns on the given consider set,
        returning a set of cells where at least one pattern
        matched. For each cell, the search is aborted after the first
        match. No information on the hits is returned. This is a
        convience method. */
    bitset_t MatchOnBoard(const bitset_t& consider,
                          const HashedPatternSet& patset) const;
    
    // @}

    //-----------------------------------------------------------------------

    /** Reset the pattern checking statistics. */
    void ClearPatternCheckStats();

    /** Return a string containing the pattern checking statistics. */
    std::string DumpPatternCheckStats() const;

private:
    /** Pattern checking statistics. */
    struct Statistics
    {
        /** Number of pattern checks. */
        size_t pattern_checks;

        /** Number of calls to checkRingGodel(). */
        size_t ring_checks;

        /** Number of slice checks. */
        size_t slice_checks;
    };

    StoneBoard& m_brd;

    const PatternMatcherData* m_data;

    /** See UpdateRadius() */
    int m_update_radius;

    int m_slice_godel[BITSETSIZE][BLACK_AND_WHITE][Pattern::NUM_SLICES];

    RingGodel m_ring_godel[BITSETSIZE];

    mutable Statistics m_statistics;

    /** Non-assignable. */
    void operator=(const PatternState& other);
    
    /** Non-copyable. */
    PatternState(const PatternState& other);

    void ClearGodels();

    bool CheckRotatedSlices(HexPoint cell, 
                            const Pattern& pat, int angle) const;

    bool CheckRotatedSlices(HexPoint cell, 
                            const RotatedPattern& rotpat) const;
        
    bool CheckRingGodel(HexPoint cell, 
                        const Pattern& pattern, int angle) const;

    bool CheckRingGodel(HexPoint cell, 
                        const RotatedPattern& rotpat) const;

    bool CheckRotatedPattern(HexPoint cell, 
                             const RotatedPattern& rotpat,
                             std::vector<HexPoint>& moves1,
                             std::vector<HexPoint>& moves2) const;
};

inline const StoneBoard& PatternState::Board() const
{
    return m_brd;
}

inline StoneBoard& PatternState::Board()
{
    return m_brd;
}

inline void PatternState::SetUpdateRadius(int radius)
{
    BenzeneAssert(1 <= radius);
    BenzeneAssert(radius <= Pattern::MAX_EXTENSION);
    m_update_radius = radius;
}

inline int PatternState::UpdateRadius() const
{
    return m_update_radius;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PATTERNSTATE_HPP

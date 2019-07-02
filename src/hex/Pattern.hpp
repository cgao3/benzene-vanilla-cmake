//----------------------------------------------------------------------------
/** @file Pattern.hpp */
//----------------------------------------------------------------------------

#ifndef PATTERN_H
#define PATTERN_H

#include "Hex.hpp"
#include "RingGodel.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** @page patternencoding Pattern Encoding

    Each pattern is a type, followed by a colon, followed by six
    slices, followed by an optional weight (the weight parameter is
    used only for certain types of patterns).
    
    @verbatim

       pattern = type : slice; slice; slice; slice; slice; slice; weight
    @endverbatim

    The six slices form a fan around the center cell. If the pattern
    is rotated 60 degrees, the first slice will map onto the second
    slice, the second onto the third, etc.  This allows the patterns
    to be easily rotated on the hex board.

    Each slice extends out by MAX_EXTENSION cells. If MAX_EXTENSION=7,
    then the slices would be laid out like this:

    @verbatim
                                  | 
                                  |   slice 1     27  
                      slice 2     |            20 26
                                  |         14 19 25
                                  |       9 13 18 24
                                  |     5 8 12 17 23 <-- slice 0
                                      2 4 7 11 16 22
                 21 15 10 6 3 1 0 * 0 1 3 6 10 15 21
                 22 16 11 7 4 2  
     slice 3 --> 23 17 12 8 5    |
                 24 18 13 9      |   
                 25 19 14        |     slice 5
                 26 20           |
                 27     slice 4  |
                                 |
    @endverbatim
    
    Each slice is composed of five comma separated features. 

    @verbatim

       slice = feature, feature, feature, feature, feature
    @endverbatim

    Each feature is a 32-bit integer used as a bitmask where the set
    bits denote cells int which that feature is "on".

    - CELLS the cells used in the slice.  
    - BLACK the black stones in the slice.
    - WHITE the white stones in the slice. 
    - MARKED1 first set of marked cells in the slice. 
    - MARKED1 second set of marked cells in the slice.

    All features must be a subset of CELLS. BLACK, WHITE, MARKED1 and
    MARKED2 must all be pairwise disjoint.

    For example, let s be a slice in which CELLS=7, BLACK=4, WHITE=1,
    MARKED1=0, and MARKED2=0. Then this slice uses cells 0, 1 and 2;
    cell 0 contains a white stone, cell 1 is empty, and cell 2
    contains a black stone.
*/

/** @page patternfiles Pattern Files

    A pattern file is a text file encoding multiple patterns. In
    addition to the raw pattern data, it also stores names and
    mirroring information for each pattern.
    
    Pattern names are assumed to come before the encoding and are
    between '[' and '/' characters (this comes from Jack's pattern
    file format).

    There may also be a comment between the '/' and the closing ']',
    used for precomputation.
    If a (either-)fillin pattern contains the comment "deduce-only",
    it means that it has not to be included in the final file, because
    it would not be rigorous (see comment for fillin patterns), but one
    can use it to deduce inferiority or reversibility results.

    A mirrored copy of a pattern is stored if two names are
    encountered before the pattern string.  No checking is done to
    determine if a mirror is really necessary.

    The pattern encoding is detected by any character in the first
    column and is assumed to occupy exactly a single line.

    So, to create your own pattern file, use something like this:

    @verbatim
        |  ...
        |
        |  [name1/]
        |  [name2/]
        | pattern encoding;
        |
        |  ...
    @endverbatim

    Notice the names are between [ and / symbols and do not start
    in the first column.  The pattern encoding, however, does
    start in the first column (and is the ONLY thing starting in the 
    first column).

    If you want, you can add a picture of the pattern. Here is 
    an example pattern from our pattern file

    @verbatim
        |
        |               W
	|              * ^
	|             W W W                          [capt-1-3/]
	|
	|f:1,0,0,1,0;1,0,1,0,0;0,0,0,0,0;0,0,0,0,0;1,0,1,0,0;5,0,5,0,0;
        |
    @endverbatim

    This defines a fillin pattern with name "capt-1-3" and no comment.

    Lines not starting in the first column and not containing a name
    between a pair of '[' and '/' are simply ignored.
*/

/** Patterns on a Hex board. 
    Patterns are centered around a cell, and are encoded such that they
    can be rotated with minimal computation.  
    @see @ref patternencoding
    @see @ref patternfiles */
class Pattern
{
public:
    /** This sets how far out patterns are allowed to extend. Value
        must be >= 1 and <= 7. */
    static const int MAX_EXTENSION = 3;

    /** Patterns should not have any cell of moves1 further that this.
	If they do, they may not always be detected, but nothing will
	fail. */
    static const int MAX_EXTENSION_MOVES1 = 1;

    //-----------------------------------------------------------------------

    /** Pattern encodes a move. */
    static const int HAS_EMPTY     = 0x01;
    static const int HAS_MOVES1    = 0x02;
    static const int HAS_MOVES2    = 0x04;

    /** Pattern has a weight (used by MOHEX patterns). */
    static const int HAS_WEIGHT    = 0x08;

    //-----------------------------------------------------------------------

    /** @name Pattern Types.
        The pattern type typically denotes the status of the cell at
        the center of the pattern.
    */

    // @{

    /** Unknown type. Set in Pattern(), but should not appear in a
        defined pattern. */
    static const char UNKNOWN = ' ';
    
    /** Marks that the cell the pattern is centered on can be
        fillined by either player (typically, a dead cell). */
    static const char EITHER_FILLIN = 'e';

    /** Marks that the cell the pattern is centered on can be 
        white-fillined (typically, a captured cell).
        Often, other nearby cells can also be fillined, they
        are in MARKED1.
	Finally, for some patterns, a simple fillin may create
	more wining moves for white, making it unrigorous if we are
	unlucky with the pruning due to inferiority. Thus, some
	cells may be in MARKED2, and black-coloring them solves
	this problem. These cells have to be captured after the
	white-coloring, else an important optimisation with mode
	MONOCOLOR_USING_CAPTURE will fail, @see ComputePatternFillin. */
    static const char WHITE_FILLIN = 'f';

    /** Marks that the cell the pattern is centerd on is
        strong reversed by black playing in in MARKED2, ie
	after black's move white's stone could be removed.
	Often, other neaby cells are reversed by the same cell,
	they are in MARKED1. */
    static const char WHITE_STRONG_REVERSIBLE = 's';

    /** Marks that the cell the pattern is centerd on is
        threat reversed by black playing in in MARKED2, ie
	after not only is (not strong) reverses it, but also
	there is a move ouside the reverse pattern such that,
	if white keeps playing in the reverse pattern, black
	can black-colors everything. */
    static const char WHITE_THREAT_REVERSIBLE = 't';

    /** Marks that the cell the pattern is centered on is
        white-inferior to the cell in MARKED2.
	Often, other nearby cells are inferior to the same cell,
	they are in MARKED1. */
    static const char WHITE_INFERIOR = 'i';

    /** Marks that the cell the pattern is centered on is
        white-reversible to the cell in MARKED2 (ie, after
	white plays in the first, black should respond in the
	second). */
    static const char WHITE_REVERSIBLE = 'r';

    /** Patterns used by VCS. */
    static const char VC_CAPTURE = 'c';

  /* The two following types are currently unused, all the code
     corresponding to them is commented. */

    /** A mohex pattern.  These patterns are used during the random
        playout phase of an UCT search. */
    //static const char MOHEX = 'm';

    /** A mohex pattern. These patterns are used during the random
        playout phase of an UCT search. */
    //static const char SHIFT = 's';

    // @} // @name

    //-----------------------------------------------------------------------

    /** Number of triangular slices.  Each slice is rooted at a
        neighbour of the center cell.  Should be 6 (one for each
        direction in HexDirection). */
    static const int NUM_SLICES = 6;

    /** Info stored in each slice. */
    static const int FEATURE_CELLS   = 0;
    static const int FEATURE_BLACK   = 1;
    static const int FEATURE_WHITE   = 2;
    static const int FEATURE_MARKED1 = 3;
    static const int FEATURE_MARKED2 = 4;
    static const int NUM_FEATURES    = 5;

    /** A slice is simply an array of features. */
    typedef int slice_t[NUM_FEATURES];

    //------------------------------------------------------------

    /** Creates an empty pattern. Type is set to UNKNOWN. */
    Pattern();

    /** Returns a string of the pattern in encoded form. 
        @see @ref patternencoding
    */
    std::string Serialize() const;

    /** Parses a pattern from an encoded string. 
        @see @ref patternencoding
        @return true if pattern was parsed without error.
    */
    bool Unserialize(std::string code);
    
    /** Returns the pattern's flags. */
    int GetFlags() const;

    /** Returns the pattern's name. */
    std::string GetName() const;

    /** Returns the pattern's comment. */
    std::string GetComment() const;

    /** Sets the name of this pattern. */
    void SetName(const std::string& s);

    /** Sets the comment of this pattern. */
    void SetComment(const std::string& s);

    /** Returns the pattern's type. */
    char GetType() const;

    /** Returns the list of (slice, bit) pairs for moves defined in
        the field. */  
    const std::vector<std::pair<int, int> >& GetEmpty() const;
  
    const std::vector<std::pair<int, int> >& GetMoves1() const;

    const std::vector<std::pair<int, int> >& GetMoves2() const;

    /** Gets the weight for this pattern. */
    int GetWeight() const;

    /** Gets the extension radius of this pattern. */
    int Extension() const;

    /** Returns pointer to pattern's slice data. */
    const slice_t* GetData() const;

    /** Returns the ring godel of this pattern rotated
        by angle slices. */
    const PatternRingGodel& RingGodel(int angle) const;

    /** Flip the pattern's colors. */
    void FlipColors();

    /** Mirrors pattern along x/y diagonal. */
    void Mirror(); 

    /** Parses patterns from a stream.
        Stream must be formatted as in @ref patternfiles.
        @throws BenzeneException on error. */
    static void LoadPatternsFromStream(std::istream& f,
				       std::vector<Pattern>& out);

    /** Load patterns from a file.
        File must be formatted as in @ref patternfiles.
        @throws BenzeneException on error. */
    static void LoadPatternsFromFile(const char *filename,
				     std::vector<Pattern>& out);

private:
    /** Pattern type. */
    char m_type;

    /** Name of the pattern. */
    std::string m_name;

    /** Comment (for precomputation) */
    std::string m_comment;

    /** Flags. */
    int m_flags;

    /** (slice, bit) pairs of cells in not FEATURE_BLACK
        nor FEATURE_WHITE. */
    std::vector<std::pair<int, int> > m_empty;

    /** (slice, bit) pairs of cells in FEATURE_MARKED1. */
    std::vector<std::pair<int, int> > m_moves1;

    /** (slice, bit) pairs of cells in FEATURE_MARKED2. */
    std::vector<std::pair<int, int> > m_moves2;

    /** MoHex pattern weight. */
    int m_weight;

    /** Data for each slice. */
    slice_t m_slice[NUM_SLICES];

    /** How far out the pattern extends. */
    int m_extension;

    /** One RingGodel for each rotation of the pattern. */
    PatternRingGodel m_ring_godel[NUM_SLICES];

    void ComputeRingGodel();

    bool CheckSliceIsValid(const slice_t& slice) const;
};

/** Sends output of serialize() to the stream. */
inline std::ostream& operator<<(std::ostream &os, const Pattern& p)
{
    os << p.Serialize();
    return os;
}

inline std::string Pattern::GetName() const
{
    return m_name;
}

inline std::string Pattern::GetComment() const
{
    return m_comment;
}

inline int Pattern::GetFlags() const
{
    return m_flags;
}

inline char Pattern::GetType() const
{
    return m_type;
}

inline const Pattern::slice_t* Pattern::GetData() const
{
    return m_slice;
}

inline void Pattern::SetName(const std::string& s) 
{
    m_name = s; 
}

inline void Pattern::SetComment(const std::string& s) 
{
    m_comment = s; 
}

inline const std::vector<std::pair<int, int> >& Pattern::GetEmpty() const
{
    BenzeneAssert(m_flags & HAS_EMPTY);
    return m_empty;
}

inline const std::vector<std::pair<int, int> >& Pattern::GetMoves1() const
{
    BenzeneAssert(m_flags & HAS_MOVES1);
    return m_moves1;
}

inline const std::vector<std::pair<int, int> >& Pattern::GetMoves2() const
{
    BenzeneAssert(m_flags & HAS_MOVES2);
    return m_moves2;
}

inline int Pattern::GetWeight() const
{
    BenzeneAssert(m_flags & HAS_WEIGHT);
    return m_weight;
}

inline int Pattern::Extension() const
{
    return m_extension;
}

inline const PatternRingGodel& Pattern::RingGodel(int angle) const
{
    return m_ring_godel[angle];
}

//----------------------------------------------------------------------------

/** Vector of patterns. */
typedef std::vector<Pattern> PatternSet;

//----------------------------------------------------------------------------

/** Utilities on Patterns. */
namespace PatternUtil
{
    /** Computes how far out this godel code extends from the center point
        of the pattern. */
    int GetExtensionFromGodel(int godel);
}

//----------------------------------------------------------------------------

/** A (pattern, angle) pair. */
class RotatedPattern
{
public:
    RotatedPattern(const Pattern* pat, int angle);

    const Pattern* GetPattern() const;

    int Angle() const;

private:
    const Pattern* m_pattern;

    int m_angle;
};

inline RotatedPattern::RotatedPattern(const Pattern* pat, int angle)
    : m_pattern(pat), 
      m_angle(angle)
{
}

inline const Pattern* RotatedPattern::GetPattern() const
{
    return m_pattern;
}

inline int RotatedPattern::Angle() const
{
    return m_angle;
}

//----------------------------------------------------------------------------

/** List of RotatedPatterns. */
typedef std::vector<RotatedPattern> RotatedPatternList;

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PATTERN_H

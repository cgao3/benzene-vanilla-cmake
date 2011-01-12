//----------------------------------------------------------------------------
/** @file RingGodel.hpp */
//----------------------------------------------------------------------------

#ifndef RING_GODEL_HPP
#define RING_GODEL_HPP

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Base Ring godel class. */
class RingGodel
{
public:
    /** Constructor. */
    RingGodel();

    /** Initializes godel with given value: use only if you know what
        you are doing! */
    RingGodel(int value);

    /** Destructor. */
    virtual ~RingGodel();

    //------------------------------------------------------------------------

    /** Adds BLACK, WHITE or EMPTY to slice in ring_godel. */
    void AddColorToSlice(int slice, HexColor color);

    /** Removes a color from a slice: color can be BLACK, WHITE, EMPTY. */
    void RemoveColorFromSlice(int slice, HexColor color);

    /** Sets the color of slice: color can be BLACK, WHITE, EMPTY. */
    void SetSliceToColor(int slice, HexColor color);

    /** Sets the godel to have all empty slices. */
    virtual void SetEmpty();

    /** Returns the index of this ring_godel; use to hash into arrays. */
    std::size_t Index() const;

    /** Returns godel as an integer. */
    int Value() const;

    //------------------------------------------------------------------------

    /** Returns the valid ring godels. */
    static const std::vector<RingGodel>& ValidGodels();

protected:

    /** The actual godel value. */
    int m_value;

    /** Number of bits to use for each slice in the ring godel. */
    static const int BITS_PER_SLICE = 3;
    
    /** Sould be 1<<(BITS_PER_SLICE) - 1. */
    static const int SLICE_MASK = 7;

    /** Score for color in a slice. */
    static int Score(HexColor color);

    /** Adjusts a score by the slice. */
    static int AdjustScoreBySlice(int score, int slice);

private:
    struct GlobalData
    {
        /** Value of empty ring godel. */
        int empty;

        /** Scores adjusted for each slice.*/
        std::vector<int> color_slice_score[BLACK_WHITE_EMPTY];

        /** Mask for each slice. */
        std::vector<int> mask_slice_score;
        
        GlobalData();
    };

    static GlobalData& GetGlobalData();

    struct ValidGodelData
    {
        std::vector<RingGodel> valid_godel;

        std::vector<std::size_t> godel_to_index;

        ValidGodelData();
    };

    static ValidGodelData& GetValidGodelData();
};

inline int RingGodel::Value() const
{
    return m_value;
}

inline int RingGodel::AdjustScoreBySlice(int score, int slice)
{
    return score << (slice * BITS_PER_SLICE);
}

inline int RingGodel::Score(HexColor color)
{
    switch(color)
    {
    case EMPTY: return 1;
    case BLACK: return 2;
    case WHITE: return 4;
    }
    BenzeneAssert(false);
    return 0;
}

//----------------------------------------------------------------------------

/** Standard RingGodel with an added mask to use when checking if two
    RingGodels match. */
class PatternRingGodel : public RingGodel
{
public: 
    /** Constructs pattern ring godel with empty mask. */
    PatternRingGodel();

    /** Destructor. */
    virtual ~PatternRingGodel();

    /** Sets the godel and mask to empty. */
    virtual void SetEmpty();

    /** Adds the given slice to the mask. */
    void AddSliceToMask(int slice);
    
    /** Returns true if we match godel on our mask.  For a match to
        occur, each corresponding slice in godel must be a superset of
        the slice in this godel. If godel has BW, then B or W or BW
        will match it, but if we have BW, only BW in godel will
        match. */
    bool MatchesGodel(const RingGodel& godel) const;

private:
    int m_mask;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // RING_GODEL_HPP

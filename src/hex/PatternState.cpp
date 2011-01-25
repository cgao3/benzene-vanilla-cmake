//----------------------------------------------------------------------------
/** @file PatternState.cpp */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "PatternState.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

const PatternMatcherData* PatternMatcherData::Get(const ConstBoard* brd)
{
    static std::vector<const PatternMatcherData*> data;
    for (std::size_t i = 0; i < data.size(); ++i) 
        if (*brd == *data[i]->brd)
            return data[i];
    LogFine() << "Data does not exist. Creating...\n";
    data.push_back(new PatternMatcherData(brd));
    return data.back();
}

PatternMatcherData::PatternMatcherData(const ConstBoard* brd)
    : brd(brd)
{
    Initialize();
}

/** For each cell, store the slice and godel for every other cell. */
void PatternMatcherData::Initialize()
{
    LogFine() << "PatternMatcherData::Initialize " 
	      << "(" << brd->Width() << " x " << brd->Height() <<")\n";
    memset(played_in_slice, 0, sizeof(played_in_slice));
    memset(played_in_godel, 0, sizeof(played_in_godel));
    memset(played_in_edge,  0, sizeof(played_in_edge));
    memset(inverse_slice_godel, 0, sizeof(inverse_slice_godel));
    for (BoardIterator ip1 = brd->Interior(); ip1; ++ip1)
    {
        int x, y;
        HexPoint p1 = *ip1;
        HexPointUtil::pointToCoords(p1, x, y);
        for (int s = 0; s < Pattern::NUM_SLICES; s++) 
        {
            int fwd = s;
            int lft = (s + 2) % NUM_DIRECTIONS;
            int x1 = x + HexPointUtil::DeltaX(fwd); 
            int y1 = y + HexPointUtil::DeltaY(fwd);
            for (int i = 1, g = 0; i <= Pattern::MAX_EXTENSION; i++) 
            {
                int x2 = x1;
                int y2 = y1;
                for (int j = 0; j < i; j++) 
                {
                    // handle obtuse corner: both colors get it. 
                    if (x2 == -1 && y2 == brd->Height()) 
                    {
                        // southwest obtuse corner
                        played_in_edge[p1][SOUTH - FIRST_EDGE][s] 
                            |= (1 << g);
                        played_in_edge[p1][WEST - FIRST_EDGE][s]
                            |= (1 << g);
                    } 
                    // handle obtuse corner: both colors get it. 
                    else if (x2 == brd->Width() && y2 == -1) 
                    {
                        // northeast obtuse corner
                        played_in_edge[p1][NORTH - FIRST_EDGE][s] 
                            |= (1 << g);
                        played_in_edge[p1][EAST - FIRST_EDGE][s]
                            |= (1 << g);
                    } 
                    else 
                    {
                        // handle all valid interior cells and edges
                        HexPoint p2 = BoardUtil::CoordsToPoint(*brd, x2, y2);
                        if (p2 != INVALID_POINT) 
                        {
                            if (HexPointUtil::isEdge(p2)) 
                            {
                                played_in_edge[p1][p2 - FIRST_EDGE][s] 
                                    |= (1 << g);
                            } 
                            else 
                            {
                                played_in_slice[p1][p2] = s;
                                played_in_godel[p1][p2] = (1 << g);
                            }
                            inverse_slice_godel[p1][s][g] = p2;
                        }
                    }
                    x2 += HexPointUtil::DeltaX(lft);
                    y2 += HexPointUtil::DeltaY(lft);
                    g++;
                }
                x1 += HexPointUtil::DeltaX(fwd);
                y1 += HexPointUtil::DeltaY(fwd);
            }
        }
    }
}

HexPoint PatternMatcherData::GetRotatedMove(HexPoint cell, int slice, 
                                            int bit, int angle) const
{
    slice = (slice + 6 - angle) % Pattern::NUM_SLICES;
    return inverse_slice_godel[cell][slice][bit];
}

//----------------------------------------------------------------------------

PatternState::PatternState(StoneBoard& brd)
    : m_brd(brd),
      m_data(PatternMatcherData::Get(&m_brd.Const())),
      m_update_radius(Pattern::MAX_EXTENSION)
{
    ClearGodels();
}

PatternState::~PatternState()
{
}

//----------------------------------------------------------------------------

void PatternState::UpdateRingGodel(HexPoint cell)
{
    BenzeneAssert(m_brd.Const().IsCell(cell));
    HexColor color = m_brd.GetColor(cell);
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    // NOTE: if Pattern::NUM_SLICES != 6, this won't work!! This also
    // relies on the fact that slice 3 is opposite 0, 4 opposite 1,
    // etc.
    BenzeneAssert(Pattern::NUM_SLICES == 6);
    for (int opp_slice = 3, slice = 0; slice < Pattern::NUM_SLICES; ++slice) 
    {
        HexPoint p = m_data->inverse_slice_godel[cell][slice][0];
        m_ring_godel[p].AddColorToSlice(opp_slice, color);
        m_ring_godel[p].RemoveColorFromSlice(opp_slice, EMPTY);
        if (++opp_slice == Pattern::NUM_SLICES) opp_slice = 0;
    }
}

void PatternState::Update(HexPoint cell)
{
    if (HexPointUtil::isSwap(cell)) 
        return;
    BenzeneAssert(m_brd.Const().IsLocation(cell));
    int r = m_update_radius;
    HexColor color = m_brd.GetColor(cell);
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    if (HexPointUtil::isEdge(cell)) 
        goto handleEdge;
    for (BoardIterator p = m_brd.Const().Nbs(cell, r); p; ++p) 
    {
        int slice = m_data->played_in_slice[*p][cell];
        int godel = m_data->played_in_godel[*p][cell];
        m_slice_godel[*p][color][slice] |= godel;
        // Update *p's ring godel if we played next to it
        if (godel == 1)
        {
            m_ring_godel[*p].AddColorToSlice(slice, color);
            m_ring_godel[*p].RemoveColorFromSlice(slice, EMPTY);
        }
    }
    return;

 handleEdge:
    int edge = cell - FIRST_EDGE;
    for (BoardIterator p = m_brd.Const().Nbs(cell, r); p; ++p) 
    {
        for (int slice = 0; slice < Pattern::NUM_SLICES; ++slice)
        {
            int godel = m_data->played_in_edge[*p][edge][slice];
            m_slice_godel[*p][color][slice] |= godel;
            // Update *p's ring godel if we played next to it.
            // Must use AddColorToSlice instead of SetSliceToColor
            // because the obtuse corner is both black and white.
            if ((godel & 1) == 1)
            {
                m_ring_godel[*p].AddColorToSlice(slice, color);
                m_ring_godel[*p].RemoveColorFromSlice(slice, EMPTY);
            }
        }            
    }
}

void PatternState::Update(const bitset_t& changed)
{
    for (BitsetIterator p(changed); p; ++p) 
    {
        BenzeneAssert(m_brd.IsOccupied(*p));
        Update(*p);
    }
}

void PatternState::Update()
{
    ClearGodels();
    for (BitsetIterator p(m_brd.GetBlack() | m_brd.GetWhite()); p; ++p) 
        Update(*p);
}

void PatternState::ClearGodels()
{
    memset(m_slice_godel, 0, sizeof(m_slice_godel));
    for (BoardIterator p(m_brd.Const().Interior()); p; ++p)
        m_ring_godel[*p].SetEmpty();
}

void PatternState::CopyState(const PatternState& other)
{
    BenzeneAssert(m_brd.Const() == other.m_brd.Const());
    m_update_radius = other.m_update_radius;
    memcpy(m_slice_godel, other.m_slice_godel, sizeof(m_slice_godel));
    memcpy(m_ring_godel, other.m_ring_godel, sizeof(m_ring_godel));
}

//----------------------------------------------------------------------------

void PatternState::MatchOnCell(const HashedPatternSet& patset, 
                               HexPoint cell, MatchMode mode,
                               PatternHits& hits) const
{
    const RingGodel& ring_godel = m_ring_godel[cell];
    const RotatedPatternList& rlist = patset.ListForGodel(ring_godel);
    RotatedPatternList::const_iterator it = rlist.begin();
    for (; it != rlist.end(); ++it) 
    {
        std::vector<HexPoint> moves1, moves2;
        if (CheckRotatedPattern(cell, *it, moves1, moves2)) 
        {
            hits.push_back(PatternHit(it->GetPattern(), moves1, moves2));
            if (mode == STOP_AT_FIRST_HIT)
                break;
        }
    }
}

bitset_t PatternState::MatchOnBoard(const bitset_t& consider, 
                                    const HashedPatternSet& patset, 
                                    MatchMode mode, 
                                    std::vector<PatternHits>& hits) const
{
    bitset_t ret;
    bitset_t lookat = consider & Board().Const().GetCells();
    for (BitsetIterator p(lookat); p; ++p)
    {
        MatchOnCell(patset, *p, mode, hits[*p]);
        if (!hits[*p].empty())
            ret.set(*p);
    }
    return ret;
}

bitset_t PatternState::MatchOnBoard(const bitset_t& consider, 
                                    const HashedPatternSet& patset) const
{
    bitset_t ret;    
    bitset_t lookat = consider & Board().Const().GetCells();
    for (BitsetIterator p(lookat); p; ++p) 
    {
        PatternHits hits;
        MatchOnCell(patset, *p, STOP_AT_FIRST_HIT, hits);
        if (!hits.empty())
            ret.set(*p);
    }
    return ret;
}

//-----------------------------------------------------------------------------

/** Checks the pre-rotated pattern against the board. Returns true if
    it matches.  Pattern encoded moves are stored in moves. */
bool PatternState::CheckRotatedPattern(HexPoint cell, 
                                       const RotatedPattern& rotpat,
                                       std::vector<HexPoint>& moves1,
                                       std::vector<HexPoint>& moves2) const
{
    BenzeneAssert(m_brd.Const().IsCell(cell));
    m_statistics.pattern_checks++;
    bool matches = CheckRingGodel(cell, rotpat);
    const Pattern* pattern = rotpat.GetPattern();
    if (matches && pattern->Extension() > 1)
        matches = CheckRotatedSlices(cell, rotpat);
    if (matches) 
    {
        if (pattern->GetFlags() & Pattern::HAS_MOVES1) 
        {
            for (unsigned i = 0; i < pattern->GetMoves1().size(); ++i) 
            {
                int slice = pattern->GetMoves1()[i].first;
                int bit = pattern->GetMoves1()[i].second;
                moves1.push_back(m_data->GetRotatedMove(cell, slice, bit, 
                                                        rotpat.Angle()));
            }
        }
        if (pattern->GetFlags() & Pattern::HAS_MOVES2) 
        {
            for (unsigned i = 0; i < pattern->GetMoves2().size(); ++i) 
            {
                int slice = pattern->GetMoves2()[i].first;
                int bit = pattern->GetMoves2()[i].second;
                moves2.push_back(m_data->GetRotatedMove(cell, slice, bit, 
                                                        rotpat.Angle()));
            }
        }
        return true;
    }
    return false;
}

/** Convenience method. */
bool PatternState::CheckRotatedSlices(HexPoint cell, 
                                      const RotatedPattern& rotpat) const
{
    return CheckRotatedSlices(cell, *rotpat.GetPattern(), rotpat.Angle());
}

/** Returns true if pattern's slices rotated by angle match the board
    when pattern is centered at cell. */
bool PatternState::CheckRotatedSlices(HexPoint cell, const Pattern& pattern,
                                      int angle) const
{
    const int *gb = m_slice_godel[cell][BLACK];
    const int *gw = m_slice_godel[cell][WHITE];
    const Pattern::slice_t* pat = pattern.GetData();
    bool matches = true;
    for (int i = 0; matches && i < Pattern::NUM_SLICES; ++i) 
    {
        m_statistics.slice_checks++;
        int j = (angle + i) % Pattern::NUM_SLICES;
        int black_b = gb[i] & pat[j][Pattern::FEATURE_CELLS];
        int white_b = gw[i] & pat[j][Pattern::FEATURE_CELLS];
        int empty_b = black_b | white_b;
        int black_p = pat[j][Pattern::FEATURE_BLACK];
        int white_p = pat[j][Pattern::FEATURE_WHITE];
        int empty_p = pat[j][Pattern::FEATURE_CELLS] - 
                      pat[j][Pattern::FEATURE_BLACK] -
                      pat[j][Pattern::FEATURE_WHITE];

        // black cells on the board must be a superset of the
        // black cells in the pattern since the obtuse corner
        // is both black and white. 
        // TODO: all cells to be black/white/empty (ie, dead cells),
        // in which case we would use ((empty_b & empty_p) != empty_p)
        if (empty_b & empty_p)
            matches = false;
        else if ((black_b & black_p) != black_p)
            matches = false;
        else if ((white_b & white_p) != white_p)
            matches = false;
    }
    return matches;
}

/** Returns true if the pattern's ring godel matches the board. */
bool PatternState::CheckRingGodel(HexPoint cell, 
                                  const RotatedPattern& rotpat) const
{
    return CheckRingGodel(cell, *rotpat.GetPattern(), rotpat.Angle());
}

/** Returns true if the pattern's ring godel matches the board. */
bool PatternState::CheckRingGodel(HexPoint cell, 
                                  const Pattern& pattern, int angle) const
{
    m_statistics.ring_checks++;
    return pattern.RingGodel(angle).MatchesGodel(m_ring_godel[cell]);
}

//----------------------------------------------------------------------------

void PatternState::ClearPatternCheckStats()
{
    m_statistics.pattern_checks = 0;
    m_statistics.ring_checks = 0;
    m_statistics.slice_checks = 0;
}

std::string PatternState::DumpPatternCheckStats() const
{
    std::ostringstream os;
    os << std::endl;
    os << "    Patterns Checked: " << m_statistics.pattern_checks << '\n';
    os << " Ring Godels Checked: " << m_statistics.ring_checks << '\n';
    os << "      Slices Checked: " << m_statistics.slice_checks << '\n';
    os << " Avg Rings Per Check: " 
       << std::setprecision(4) 
       << static_cast<double>(m_statistics.ring_checks) 
        / static_cast<double>(m_statistics.pattern_checks) << '\n';
    os << "Avg Slices Per Check: " 
       << std::setprecision(4) 
       << static_cast<double>(m_statistics.slice_checks)
        / static_cast<double>(m_statistics.pattern_checks) << '\n';
    return os.str();
}

//----------------------------------------------------------------------------

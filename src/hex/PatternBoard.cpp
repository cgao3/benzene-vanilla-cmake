//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"
#include "PatternBoard.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

const PatternMatcherData* PatternMatcherData::Get(const ConstBoard* brd)
{
    static std::vector<const PatternMatcherData*> data;
    for (std::size_t i=0; i<data.size(); ++i) 
    {
        if (*brd == *data[i]->brd)
            return data[i];
    }

    LogFine() << "Data does not exist. Creating..." << '\n';

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
	      << "(" << brd->width() << " x " << brd->height() <<")" << '\n';

    memset(played_in_slice, 0, sizeof(played_in_slice));
    memset(played_in_godel, 0, sizeof(played_in_godel));
    memset(played_in_edge,  0, sizeof(played_in_edge));
    memset(inverse_slice_godel, 0, sizeof(inverse_slice_godel));

    for (BoardIterator ip1=brd->Interior(); ip1; ++ip1) 
    {
        int x, y;
        HexPoint p1 = *ip1;
        HexPointUtil::pointToCoords(p1, x, y);

        for (int s=0; s<Pattern::NUM_SLICES; s++) 
        {
            int fwd = s;
            int lft = (s + 2) % NUM_DIRECTIONS;
        
            int x1 = x + HexPointUtil::DeltaX(fwd); 
            int y1 = y + HexPointUtil::DeltaY(fwd);

            for (int i=1, g=0; i<=Pattern::MAX_EXTENSION; i++) 
            {
                int x2 = x1;
                int y2 = y1;
                for (int j=0; j<i; j++) 
                {
                    // handle obtuse corner: both colors get it. 
                    if (x2 == -1 && y2 == brd->height()) 
                    {
                        // southwest obtuse corner
                        played_in_edge[p1][SOUTH - FIRST_EDGE][s] 
                            |= (1 << g);
                        played_in_edge[p1][WEST - FIRST_EDGE][s]
                            |= (1 << g);
                    } 
                    // handle obtuse corner: both colors get it. 
                    else if (x2 == brd->width() && y2 == -1) 
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
                        HexPoint p2 
                            = BoardUtils::CoordsToPoint(*brd, x2, y2);

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

//----------------------------------------------------------------------------

PatternBoard::PatternBoard(int width, int height)
    : GroupBoard(width, height),
      m_update_radius(Pattern::MAX_EXTENSION),
      m_data(PatternMatcherData::Get(&this->Const()))
{
}

PatternBoard::~PatternBoard()
{
}

//----------------------------------------------------------------------------

void PatternBoard::updateRingGodel(HexPoint cell)
{
    HexAssert(isCell(cell));
    HexColor color = getColor(cell);
    HexAssert(HexColorUtil::isBlackWhite(color));

    /** @note if Pattern::NUM_SLICES != 6, this won't work!! 
        This also relies on the fact that slice 3 is opposite 0, 4 opposite 1,
        etc. */
    for (int opp_slice=3, slice=0; slice<Pattern::NUM_SLICES; ++slice) 
    {
        HexPoint p = m_data->inverse_slice_godel[cell][slice][0];
        m_ring_godel[p].AddColorToSlice(opp_slice, color);
        if (++opp_slice == Pattern::NUM_SLICES) opp_slice = 0;
    }
}

void PatternBoard::update(HexPoint cell)
{
    if (HexPointUtil::isSwap(cell)) 
        return;

    HexAssert(isLocation(cell));

    int r = m_update_radius;
    HexColor color = getColor(cell);
    HexAssert(HexColorUtil::isBlackWhite(color));

    if (HexPointUtil::isEdge(cell)) 
        goto handleEdge;
    
    for (BoardIterator p = Const().Nbs(cell, r); p; ++p) 
    {
        int slice = m_data->played_in_slice[*p][cell];
        int godel = m_data->played_in_godel[*p][cell];
        m_slice_godel[*p][color][slice] |= godel;

        // update *p's ring godel if we played next to it
        if (godel == 1)
            m_ring_godel[*p].AddColorToSlice(slice, color);
    }
    return;

 handleEdge:

    int edge = cell - FIRST_EDGE;
    for (BoardIterator p = Const().Nbs(cell, r); p; ++p) 
    {
        for (int slice=0; slice<Pattern::NUM_SLICES; slice++) 
        {
            int godel = m_data->played_in_edge[*p][edge][slice];
            m_slice_godel[*p][color][slice] |= godel;

            // update *p's ring godel if we played next to it
            if ((godel & 1) == 1) 
                m_ring_godel[*p].AddColorToSlice(slice, color);
        }            
    }
}

void PatternBoard::update(const bitset_t& changed)
{
    for (BitsetIterator p(changed); p; ++p) 
    {
        HexAssert(isOccupied(*p));
        update(*p);
    }
}

void PatternBoard::update()
{
    clearGodels();
    for (BitsetIterator p(getBlack() | getWhite()); p; ++p) 
        update(*p);
}

//----------------------------------------------------------------------------

void PatternBoard::matchPatternsOnCell(const HashedPatternSet& patset, 
                                       HexPoint cell, MatchMode mode,
                                       PatternHits& hits) const
{
    const RingGodel& ring_godel = m_ring_godel[cell];
    const RotatedPatternList& rlist = patset.ListForGodel(ring_godel);
    RotatedPatternList::const_iterator it = rlist.begin();
    for (; it != rlist.end(); ++it) 
    {
        std::vector<HexPoint> moves1, moves2;
        if (checkRotatedPattern(cell, *it, moves1, moves2)) 
        {
            hits.push_back(PatternHit(it->pattern(), moves1, moves2));
            if (mode == STOP_AT_FIRST_HIT)
                break;
        }
    }
}

bitset_t PatternBoard::matchPatternsOnBoard(const bitset_t& consider, 
                                            const HashedPatternSet& patset, 
                                            MatchMode mode, 
                                            std::vector<PatternHits>& hits) 
                                            const 
{
    bitset_t ret;    
    for (BitsetIterator p(consider & Const().getCells()); p; ++p) 
    {
        matchPatternsOnCell(patset, *p, mode, hits[*p]);
        if (!hits[*p].empty())
            ret.set(*p);
    }
    return ret;
}

bitset_t PatternBoard::matchPatternsOnBoard(const bitset_t& consider, 
                                            const HashedPatternSet& patset)
                                            const
{
    bitset_t ret;    
    for (BitsetIterator p(consider & Const().getCells()); p; ++p) 
    {
        PatternHits hits;
        matchPatternsOnCell(patset, *p, STOP_AT_FIRST_HIT, hits);
        if (!hits.empty())
            ret.set(*p);
    }
    return ret;
}

//-----------------------------------------------------------------------------

HexPoint PatternBoard::getRotatedMove(HexPoint cell, 
                                      int slice, int bit, int angle) const
{
    slice = (slice + 6 - angle) % Pattern::NUM_SLICES;
    return m_data->inverse_slice_godel[cell][slice][bit];
}

bool PatternBoard::checkRotatedSlices(HexPoint cell, 
                                      const RotatedPattern& rotpat) const
{
    return checkRotatedSlices(cell, *rotpat.pattern(), rotpat.angle());
}

bool PatternBoard::checkRotatedSlices(HexPoint cell, 
                                      const Pattern& pattern,
                                      int angle) const
{
    const int *gb = m_slice_godel[cell][BLACK];
    const int *gw = m_slice_godel[cell][WHITE];
    const Pattern::slice_t* pat = pattern.getData();

    bool matches = true;
    for (int i=0; matches && i<Pattern::NUM_SLICES; ++i) {
        m_statistics.slice_checks++;
        
        int j = (angle+i)%Pattern::NUM_SLICES;
     
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

        // @todo all cells to be black/white/empty (ie, dead cells),
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

bool PatternBoard::checkRingGodel(HexPoint cell, 
                                  const RotatedPattern& rotpat) const
{
    return checkRingGodel(cell, *rotpat.pattern(), rotpat.angle());
}

bool PatternBoard::checkRingGodel(HexPoint cell, 
                                  const Pattern& pattern,
                                  int angle) const
{
    m_statistics.ring_checks++;
    return pattern.RingGodel(angle).MatchesGodel(m_ring_godel[cell]);
}

bool PatternBoard::checkRotatedPattern(HexPoint cell, 
                                       const RotatedPattern& rotpat,
                                       std::vector<HexPoint>& moves1,
                                       std::vector<HexPoint>& moves2) const
{
    HexAssert(isCell(cell));

    m_statistics.pattern_checks++;

    bool matches = checkRingGodel(cell, rotpat);

    const Pattern* pattern = rotpat.pattern();
    if (matches && pattern->extension() > 1) {
        matches = checkRotatedSlices(cell, rotpat);
    }

    if (matches) {
        if (pattern->getFlags() & Pattern::HAS_MOVES1) {
            for (unsigned i=0; i<pattern->getMoves1().size(); ++i) {
                int slice = pattern->getMoves1()[i].first;
                int bit = pattern->getMoves1()[i].second;
                moves1.push_back(getRotatedMove(cell, slice, 
                                                  bit, rotpat.angle()));
            }
        }

        if (pattern->getFlags() & Pattern::HAS_MOVES2) {
            for (unsigned i=0; i<pattern->getMoves2().size(); ++i) {
                int slice = pattern->getMoves2()[i].first;
                int bit = pattern->getMoves2()[i].second;
                moves2.push_back(getRotatedMove(cell, slice, 
                                                bit, rotpat.angle()));
            }
        }

        return true;
    }
    return false;
}

//----------------------------------------------------------------------------

void PatternBoard::ClearPatternCheckStats()
{
    m_statistics.pattern_checks = 0;
    m_statistics.ring_checks = 0;
    m_statistics.slice_checks = 0;
}

std::string PatternBoard::DumpPatternCheckStats()
{
    std::ostringstream os;
    os << std::endl;
    os << "    Patterns Checked: " << m_statistics.pattern_checks << std::endl;
    os << " Ring Godels Checked: " << m_statistics.ring_checks << std::endl;
    os << "      Slices Checked: " << m_statistics.slice_checks << std::endl;
    os << " Avg Rings Per Check: " 
       << std::setprecision(4) 
       << (double)m_statistics.ring_checks/m_statistics.pattern_checks
       << std::endl;
    os << "Avg Slices Per Check: " 
       << std::setprecision(4) 
       << (double)m_statistics.slice_checks/m_statistics.pattern_checks
       << std::endl;
    return os.str();
}

void PatternBoard::clearGodels()
{
    memset(m_slice_godel, 0, sizeof(m_slice_godel));
    for (BoardIterator p(Interior()); p; ++p)
        m_ring_godel[*p].SetEmpty();
}

//----------------------------------------------------------------------------

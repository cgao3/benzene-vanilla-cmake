//----------------------------------------------------------------------------
/** @file VCPattern.cpp */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "Misc.hpp"
#include "StoneBoard.hpp"
#include "VCPattern.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Tools for constructing patterns. */
namespace
{

/** The start/end of a ladder pattern. */
struct BuilderPattern
{
    bitset_t black, empty;
    HexPoint endpoint;
    int height;
    
    BuilderPattern(const bitset_t& b, const bitset_t& em, HexPoint e, int h)
        : black(b), empty(em), endpoint(e), height(h)
    { };
};

/** Shifts a BuilderPattern in the given direction. Returns true
    if pattern remains inside the board, false otherwise. */
bool ShiftBuilderPattern(BuilderPattern& pat, HexDirection dir,
                         const StoneBoard& brd)
{
    bitset_t black, empty, bad;
    HexPoint endpoint = BoardUtil::PointInDir(brd.Const(), pat.endpoint, dir);
    if (!BoardUtil::ShiftBitset(brd.Const(), pat.black, dir, black)) 
        return false;
    if (!BoardUtil::ShiftBitset(brd.Const(), pat.empty, dir, empty)) 
        return false;
    pat = BuilderPattern(black, empty, endpoint, pat.height);
    return true;
}

//---------------------------------------------------------------------------

/** Rotates pattern on given board. */
VCPattern RotatePattern(const VCPattern& pat, const StoneBoard& brd)
{
    HexPoint endpoint1 = BoardUtil::Rotate(brd.Const(), pat.Endpoint(0));
    HexPoint endpoint2 = BoardUtil::Rotate(brd.Const(), pat.Endpoint(1));
    bitset_t must = BoardUtil::Rotate(brd.Const(), pat.MustHave());
    bitset_t oppt = BoardUtil::Rotate(brd.Const(), pat.NotOpponent());
    return VCPattern(endpoint1, endpoint2, must, oppt);
}

/** Mirrors pattern on given board. */
VCPattern MirrorPattern(const VCPattern& pat, const StoneBoard& brd)
{
    HexPoint endpoint1 = BoardUtil::Mirror(brd.Const(), pat.Endpoint(0));
    HexPoint endpoint2 = BoardUtil::Mirror(brd.Const(), pat.Endpoint(1));
    bitset_t must = BoardUtil::Mirror(brd.Const(), pat.MustHave());
    bitset_t oppt = BoardUtil::Mirror(brd.Const(), pat.NotOpponent());
    return VCPattern(endpoint1, endpoint2, must, oppt);
}

/** Applies the reverse-mapping; used to reverse the direction of
    ladder vcs. Returns INVALID_POINT if this point would be reversed
    off the board. */
HexPoint ReversePoint(HexPoint point, const StoneBoard& brd)
{
    if (HexPointUtil::isEdge(point)) 
        return point;
    int x, y;
    HexPointUtil::pointToCoords(point, x, y);
    x = (brd.Width() - 1 - x) + (brd.Height() - 1 - y);
    if (x >= brd.Width()) 
        return INVALID_POINT;
    return HexPointUtil::coordsToPoint(x, y);
}

/** Reverses bitset using ReversePoint(). Returns true if all points
    were reversed successfully, false otherwise. */
bool ReverseBitset(const bitset_t& bs, const StoneBoard& brd, bitset_t& out)
{
    out.reset();
    for (BitsetIterator p(bs); p; ++p) 
    {
        HexPoint rp = ReversePoint(*p, brd);
        if (rp == INVALID_POINT) 
            return false;
        out.set(rp);
    }
    return true;
}

/** Reverses a pattern situated in the bottom left corner. */
bool ReversePattern(VCPattern& pat, const StoneBoard& brd)
{
    do 
    {
        bitset_t must, oppt, badp;
        if (ReverseBitset(pat.MustHave(), brd, must) 
            && ReverseBitset(pat.NotOpponent(), brd, oppt))
        {
            HexPoint endpoint1 = ReversePoint(pat.Endpoint(0), brd);
            HexPoint endpoint2 = ReversePoint(pat.Endpoint(1), brd);
            pat = VCPattern(endpoint1, endpoint2, must, oppt);
            return true;
        }
    } while (pat.ShiftPattern(DIR_EAST, brd));
    return false;
}

//---------------------------------------------------------------------------

/** Shifts pattern in given direction until it goes off the board. */
void ShiftAndAdd(const VCPattern& pat, HexDirection dir, 
                 StoneBoard& brd, std::vector<VCPattern>& out)
{
    VCPattern spat(pat);
    do {
        out.push_back(spat);
    } while (spat.ShiftPattern(dir, brd));
}

/** Shifts pattern in first direction, rotates, then shifts in second
    direction. */
void RotateAndShift(const VCPattern& pat, StoneBoard& brd,
                    HexDirection d1, HexDirection d2, 
                    std::vector<VCPattern>& out)
{
    ShiftAndAdd(pat, d1, brd, out);
    ShiftAndAdd(RotatePattern(pat, brd), d2, brd, out);
}

/** Calls RotateAndShift() on the original and mirrored versions of
    pattern, then again on the reversed and mirrored-reversed versions
    of the pattern. */
void ProcessPattern(const VCPattern& pat, StoneBoard& brd,
                    std::vector<VCPattern> out[BLACK_AND_WHITE])
{
    RotateAndShift(pat, brd, DIR_EAST, DIR_WEST, out[BLACK]);
    RotateAndShift(MirrorPattern(pat, brd), brd, DIR_SOUTH, 
                   DIR_NORTH, out[WHITE]);
    VCPattern rpat(pat);
    ReversePattern(rpat, brd);
    RotateAndShift(rpat, brd, DIR_WEST, DIR_EAST, out[BLACK]);
    RotateAndShift(MirrorPattern(rpat, brd), brd, DIR_NORTH, 
                   DIR_SOUTH, out[WHITE]);
}

} // annonymous namespace

//----------------------------------------------------------------------------

VCPattern::VCPatternSetMap& VCPattern::GetConstructed(HexColor color)
{
    static GlobalData data;
    return data.constructed[color];
}

const VCPatternSet& 
VCPattern::GetPatterns(int width, int height, HexColor color)
{
    std::pair<int, int> key = std::make_pair(width, height);
    /** Return an empty list for all non-square boards. */
    if (width != height) 
    {
        width = 0;
        height = 1;
        VCPatternSetMap::iterator it = GetConstructed(color).find(key);
        if (it == GetConstructed(color).end()) 
        {
            VCPatternSet empty;
            GetConstructed(BLACK)[key] = empty;
            GetConstructed(WHITE)[key] = empty;
        }
    }
    else
    {
        VCPatternSetMap::iterator it = GetConstructed(color).find(key);
        if (it == GetConstructed(color).end()) 
            CreatePatterns(width, height);
    }
    return GetConstructed(color)[key];
}

void VCPattern::CreatePatterns(int width, int height)
{
    LogFine() << "VCPattern::CreatePatterns(" 
              << width << ", " << height << ")\n";
    std::ifstream fin;
    try {
        std::string file = MiscUtil::OpenFile("vc-patterns.txt", fin);
        LogConfig() << "VCPattern: loading pattern templates from '" 
                    << file << "'.\n";
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "VCPattern: " << e.what();
    }
    std::vector<VCPattern> out[BLACK_AND_WHITE];
    std::vector<BuilderPattern> start, end;
    std::vector<VCPattern> complete;
    int numConstructed = 0;
    int numComplete = 0;
    std::string line;
    while (true) 
    {
        int patternHeight;
        std::string tmp, name, type;
        std::vector<std::string> carrier;
        std::istringstream is;
        if (!getline(fin, line)) 
            break;
        if (line == "") 
            break;
        is.str(line);
        is >> name;
        is >> name;
        is.clear();
        
        getline(fin, line);
        is.str(line);
        is >> type;
        is >> type;
        is.clear();

        getline(fin, line);
        is.str(line);
        is >> tmp;
        is >> patternHeight;
        is.clear();

        while (true) 
        {
            getline(fin, line);
            if (line == "") 
                break;
            carrier.push_back(line);
        }
        BenzeneAssert(!carrier.empty());
        BenzeneAssert(patternHeight == -1 
                      || patternHeight <= (int)carrier.size());

        // abort if pattern too large for board
        if ((int)carrier.size() > height) 
            continue;
        
        int row = height - 1;
        int numcol = -1;
        HexPoint endpoint = SOUTH;
        bitset_t black, empty;
        bool patternFits = true;
        for (int i = static_cast<int>(carrier.size() - 1); i >= 0; --i) 
        {
            is.clear();
            is.str(carrier[i]);

            std::string sym;
            int col = 0;
            while (is >> sym) 
            {
                if (col >= width) 
                {
                    patternFits = false;
                    break;
                }
                HexPoint p = HexPointUtil::coordsToPoint(col, row);
                switch(sym[0]) 
                {
                case '*': empty.set(p); break;
                case 'E': endpoint = p; empty.set(p); break;
                case 'B': black.set(p); break;
                case '.': break;
                default: BenzeneAssert(false);
                }
                col++;
            }
            if (!patternFits)
                break;
            if (numcol == -1) 
                numcol = col;
            BenzeneAssert(numcol == col);
            row--;
        }

        // abort if pattern too large for board 
        if (!patternFits)
            continue;

        StoneBoard sb(width, height);
        sb.StartNewGame();

        if (type == "complete") 
        {
            numComplete++;
            VCPattern pat(endpoint, SOUTH, black, empty);
            complete.push_back(pat);
        } 
        else if (type == "start") 
        {
            if (endpoint == SOUTH)
                throw BenzeneException("Start pattern with no endpoint!");
            start.push_back(BuilderPattern(black, empty, endpoint, 
                                           patternHeight));
        } 
        else if (type == "end") 
        {
            end.push_back(BuilderPattern(black, empty, endpoint, 
                                         patternHeight));
        }
    }
    fin.close();

    // Build ladder patterns by combining start and end patterns
    LogFine() << "Combining start(" << start.size()
              << ") and end(" << end.size() << ")...\n";
    StoneBoard sb(width, height);
    for (std::size_t s = 0; s < start.size(); ++s) 
    {
        const BuilderPattern& st = start[s];
        for (std::size_t e = 0; e < end.size(); ++e) 
        {
            const BuilderPattern& en = end[e];
            if (en.height < st.height) 
                continue;
            // Shift end pattern until it does not hit start pattern
            sb.StartNewGame();
            BuilderPattern bp(en);
            int col = 0;
            bool onBoard = true;
            while (onBoard) 
            {
		if (((bp.empty|bp.black) & (st.empty|st.black)).none()) 
                    break;
                onBoard = ShiftBuilderPattern(bp, DIR_EAST, sb);
                col++;
            }
            if (!onBoard) 
                continue;

            int startCol = col;
            while (onBoard) 
            {
                bitset_t empty = st.empty | bp.empty;
                bitset_t black = st.black | bp.black;
                for (int i = startCol; i < col; ++i) 
                {
                    for (int j = 0; j < st.height; ++j) 
                    {
                        HexPoint p 
                            = HexPointUtil::coordsToPoint(i, height-1-j);
                        empty.set(p);
                    }
                }
                // No point creating a pattern with adjacent
                // endpoints, since the add will always fail.
                if (!sb.Const().Adjacent(st.endpoint, bp.endpoint))
                {
                    VCPattern pat(st.endpoint, bp.endpoint, black, empty);
                    complete.push_back(pat);
                    numConstructed++;
                }
                onBoard = ShiftBuilderPattern(bp, DIR_EAST, sb);
                col++;
            }
        }
    }
    LogFine() << "Constructed " << numConstructed << ".\n" 
	      << "Parsed " << numComplete << " complete.\n";
    LogFine() << "Translating, rotating, mirroring...\n";
    for (std::size_t i=0; i<complete.size(); ++i)
        ProcessPattern(complete[i], sb, out);
    LogFine() << out[BLACK].size() << " total patterns\n";
    GetConstructed(BLACK)[std::make_pair(width, height)] = out[BLACK];
    GetConstructed(WHITE)[std::make_pair(width, height)] = out[WHITE];
    LogFine() << "Done.\n";
}

//----------------------------------------------------------------------------

VCPattern::VCPattern(HexPoint end1, HexPoint end2, const bitset_t& must_have, 
                     const bitset_t& not_oppt)
    : m_must_have(must_have), 
      m_not_oppt(not_oppt), 
      m_end1(end1), 
      m_end2(end2)
{
}

VCPattern::~VCPattern()
{
}

//----------------------------------------------------------------------------

bool VCPattern::Matches(HexColor color, const StoneBoard& brd) const
{
    bitset_t my_color = brd.GetColor(color) & brd.Const().GetCells();
    bitset_t op_color = brd.GetColor(!color) & brd.Const().GetCells();
    return ((m_not_oppt & op_color).none() 
            && BitsetUtil::IsSubsetOf(m_must_have, my_color));
}

//----------------------------------------------------------------------------

bool VCPattern::ShiftPattern(HexDirection dir, const StoneBoard& brd)
{
    bitset_t must, oppt, bad;
    if (!BoardUtil::ShiftBitset(brd.Const(), MustHave(), dir, must)) 
        return false;
    if (!BoardUtil::ShiftBitset(brd.Const(), NotOpponent(), dir, oppt)) 
        return false;
    HexPoint endpoint1 = BoardUtil::PointInDir(brd.Const(), Endpoint(0), dir);
    HexPoint endpoint2 = BoardUtil::PointInDir(brd.Const(), Endpoint(1), dir);
    m_end1 = endpoint1;
    m_end2 = endpoint2;
    m_must_have = must;
    m_not_oppt = oppt;
    return true;
}

//----------------------------------------------------------------------------

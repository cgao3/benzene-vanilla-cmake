//----------------------------------------------------------------------------
/** @file MoHexPatterns.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgHash.h"
#include "Misc.hpp"
#include "MoHexPatterns.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

std::string MoHexPatterns::Statistics::ToString() const
{
    std::ostringstream os;
    os << "GlobalPatterns:\n";
    os << "6PatternHit         "  << hit6 << '\n';
    os << "6PatternMiss        "  << miss6 << '\n';
    os << "12PatternHit        "  << hit12 << '\n';
    os << "12PatternMiss       "  << miss12 << '\n';
    return os.str();
}
        
//----------------------------------------------------------------------------

uint64_t  MoHexPatterns::m_zobrist[2][MoHexPatterns::MAX_INDEX][6];
HexDirection MoHexPatterns::m_direction[MoHexPatterns::MAX_INDEX];

MoHexPatterns::MoHexPatterns()
    : m_table(new Data[TABLE_SIZE])
{
}

MoHexPatterns::~MoHexPatterns()
{
    delete[] m_table;
}

void MoHexPatterns::InitializeDirection()
{
    m_direction[1]  = DIR_NORTH;
    m_direction[2]  = DIR_NORTH_EAST;
    m_direction[3]  = DIR_WEST;
    m_direction[4]  = DIR_EAST;
    m_direction[5]  = DIR_SOUTH_WEST;
    m_direction[6]  = DIR_SOUTH;

    m_direction[7]  = DIR_NORTH_EAST;
    m_direction[8]  = DIR_EAST;
    m_direction[9]  = DIR_NORTH;
    m_direction[10] = DIR_SOUTH;
    m_direction[11] = DIR_WEST;
    m_direction[12] = DIR_SOUTH_WEST;

    m_direction[13] = DIR_NORTH;
    m_direction[14] = DIR_NORTH_EAST;
    m_direction[15] = DIR_WEST;
    m_direction[16] = DIR_EAST;
    m_direction[17] = DIR_SOUTH_WEST;
    m_direction[18] = DIR_SOUTH;

}

void MoHexPatterns::InitializeZobrist()
{
    int old_seed = SgRandom::Global().Seed();
    SgRandom::Global().SetSeed(1);
    for (size_t i = 0; i < MAX_INDEX; i++) 
	for (int j = 0; j < 6; j++) 
        {
            uint64_t a = SgRandom::Global().Int();
            uint64_t b = SgRandom::Global().Int();
	    m_zobrist[0][i][j] = (a << 32) | b;
        }
    SgRandom::SetSeed(old_seed);    

    for (int i = 0; i < 6; i++)
    {
	m_zobrist[1][13][i] = m_zobrist[0][15][i];
	m_zobrist[1][ 7][i] = m_zobrist[0][11][i];
	m_zobrist[1][14][i] = m_zobrist[0][17][i];
	m_zobrist[1][ 9][i] = m_zobrist[0][ 9][i];
	m_zobrist[1][ 1][i] = m_zobrist[0][ 3][i];
	m_zobrist[1][ 2][i] = m_zobrist[0][ 5][i];
        m_zobrist[1][ 8][i] = m_zobrist[0][12][i];
	m_zobrist[1][15][i] = m_zobrist[0][13][i];
	m_zobrist[1][ 3][i] = m_zobrist[0][ 1][i];
	m_zobrist[1][ 4][i] = m_zobrist[0][ 6][i];
	m_zobrist[1][16][i] = m_zobrist[0][18][i];
	m_zobrist[1][11][i] = m_zobrist[0][ 7][i];
	m_zobrist[1][ 5][i] = m_zobrist[0][ 2][i];
        m_zobrist[1][ 6][i] = m_zobrist[0][ 4][i];
	m_zobrist[1][10][i] = m_zobrist[0][10][i];
	m_zobrist[1][17][i] = m_zobrist[0][14][i];
	m_zobrist[1][12][i] = m_zobrist[0][ 8][i];
	m_zobrist[1][18][i] = m_zobrist[0][16][i];
    }
}


void MoHexPatterns::Rotate(int pattern[])
{
    int temp = pattern[1];

    pattern[1] = pattern[3];
    pattern[3] = pattern[5];
    pattern[5] = pattern[6];
    pattern[6] = pattern[4];
    pattern[4] = pattern[2];
    pattern[2] = temp;

    temp = pattern[7];
    pattern[7] = pattern[9];
    pattern[9] = pattern[11];
    pattern[11] = pattern[12];
    pattern[12] = pattern[10];
    pattern[10] = pattern[8];
    pattern[8] = temp;

    temp = pattern[13];
    pattern[13] = pattern[15];
    pattern[15] = pattern[17];
    pattern[17] = pattern[18];
    pattern[18] = pattern[16];
    pattern[16] = pattern[14];
    pattern[14] = temp;
}

uint64_t MoHexPatterns::ComputeKey(int size, int pattern[])
{
    uint64_t key = 0;
    for (int i = 1; i <= size; i++)
	key ^= m_zobrist[0][i][pattern[i]];
    return key;
}

inline void MoHexPatterns::GetKeyFromBoard(uint64_t *key_6, uint64_t *key_12, 
                                           uint64_t *key_18, const int size, 
                                           const StoneBoard& board, 
                                           const HexPoint point, 
                                           const HexColor toPlay) const
{
    *key_6 = 0;
    *key_12 = 0;
    *key_18 = 0;
    const ConstBoard& cbrd = board.Const();
    for (int i = 1; i <= 6; i++)
    {
	const HexPoint n = cbrd.PointInDir(point, m_direction[i], toPlay);
        const HexColor color = board.GetColor(n);
        if (FIRST_CELL <= n) // interior cell
	{
	    if (color == EMPTY)
	 	*key_6 ^= m_zobrist[(int)toPlay][i][0];
	    else if (color == toPlay)
		*key_6 ^= m_zobrist[(int)toPlay][i][1];
	    else
		*key_6 ^= m_zobrist[(int)toPlay][i][2];
	}
	else // edge
	{
	    if (color == toPlay)
		*key_6 ^= m_zobrist[(int)toPlay][i][3];
	    else
		*key_6 ^= m_zobrist[(int)toPlay][i][4];
	}
    }

    if (size >= 12)
    {
	*key_12 = *key_6;
	for (int i = 1; i <= 6; i++)
	{
	    const HexPoint n = cbrd.PointInDir(point, m_direction[i], toPlay);
	    if (n < FIRST_CELL) // edge
	    {
	        if (board.GetColor(n) == toPlay)
		    *key_12 ^= m_zobrist[(int)toPlay][i + 6][3];
	        else
		    *key_12 ^= m_zobrist[(int)toPlay][i + 6][4];
	    }
	    else
	    {
		const HexPoint m 
                    = cbrd.PointInDir(n, m_direction[i + 6], toPlay);
                const HexColor color = board.GetColor(m);
		if (FIRST_CELL <= m) // interior
		{
		    if (color == EMPTY)
			*key_12 ^= m_zobrist[(int)toPlay][i + 6][0];
		    else if (color == toPlay)
		        *key_12 ^= m_zobrist[(int)toPlay][i + 6][1];
		    else
		        *key_12 ^= m_zobrist[(int)toPlay][i + 6][2];
		}
		else // edge
		{
		    if (color == toPlay)
		        *key_12 ^= m_zobrist[(int)toPlay][i + 6][3];
		    else 
		        *key_12 ^= m_zobrist[(int)toPlay][i + 6][4];
		}
	    }
	}
    }
   
    if (size >= 18)
    {
	*key_18 = *key_12;
	for (int i = 1; i <= 6; i++)
	{
	    const HexPoint n = cbrd.PointInDir(point, m_direction[i], toPlay);
	    if (n < FIRST_CELL) // edge
	    {
		if (board.GetColor(n) == toPlay)
		    *key_18 ^= m_zobrist[(int)toPlay][i + 12][3];
		else
		    *key_18 ^= m_zobrist[(int)toPlay][i + 12][4];
	    }
	    else // interior
	    {
		const HexPoint m 
                    = cbrd.PointInDir(n, m_direction[i + 12], toPlay);
                const HexColor color = board.GetColor(m);
                if (FIRST_CELL <= m) // interior
		{
		    if (color == EMPTY)
		        *key_18 ^= m_zobrist[(int)toPlay][i + 12][0];
		    else if (color == toPlay)
		        *key_18 ^= m_zobrist[(int)toPlay][i + 12][1];
		    else
		        *key_18 ^= m_zobrist[(int)toPlay][i + 12][2];
		}
	        else // edge
		{
		    if (color == toPlay)
		        *key_18 ^= m_zobrist[(int)toPlay][i + 12][3];
	 	    else
		        *key_18 ^= m_zobrist[(int)toPlay][i + 12][4];
		}
            }
	}
    }
}

double MoHexPatterns::GetGammaFromBoard(const StoneBoard& board, 
                                        HexPoint point, HexColor toPlay,
                                        bool *isBadPattern) const
{
    *isBadPattern = false;
    double gamma = 1.0f;
    uint64_t key_6, key_12, key_18;
    GetKeyFromBoard(&key_6, &key_12, &key_18, 12, board, point, toPlay);
    gamma = QueryHashtable(key_12, isBadPattern);
    if (gamma == 1.0f)
    {
        gamma = QueryHashtable(key_6, isBadPattern);
        if (gamma == 1.0f)
            m_stats.miss6++;
        else   
            m_stats.hit6++;
    }
    else
        m_stats.hit12++;
    return gamma;
}

double MoHexPatterns::QueryHashtable(uint64_t key, bool *isBadPattern) const
{
    *isBadPattern = false;
    const uint64_t mask = (uint64_t)(TABLE_SIZE - 1);
    uint64_t index = key & mask;
    for (;;)
    {
	if (m_table[index].key == 0)
	    return 1.0f;
	else if (m_table[index].key == key)
	{
            *isBadPattern = m_table[index].bad;
	    return m_table[index].gamma;
	}
        index++;
        index &= mask;
    }
    return 1.0f;
}

bool MoHexPatterns::InsertHashTable(uint64_t key, double gamma, bool bad)
{
    const uint64_t mask = (uint64_t)(TABLE_SIZE - 1);
    uint64_t index = key & mask;
    for (;;)
    {
        if (m_table[index].key == 0)
	{
            m_table[index].key = key;
            m_table[index].gamma = gamma;
            m_table[index].bad = bad;
	    return true;
	}
	else if (m_table[index].key == key)
	{
	    return false;
	}
        index++;
        index &= mask;
    }
    return false;
}

void MoHexPatterns::ReadPatterns(std::string filename)
{
    for (size_t i = 0; i < TABLE_SIZE; ++i)
    {
        m_table[i].key = 0;
        m_table[i].gamma = 1.0f;
        m_table[i].bad = false;
    }

    std::ifstream ins;
    std::string fname = MiscUtil::OpenFile(filename, ins);
    FILE *stream = fopen(fname.c_str(), "r");
    if (stream == NULL)
        throw BenzeneException() << "Failed to open '" << fname << "'\n";

    double gamma;
    int A;
    int W;
    char temp[128];
    int pattern[MAX_INDEX];
    int Count[MAX_INDEX] = {0};

    if (!fscanf(stream,"%d\n", &A))
        throw BenzeneException() << "Error parsing number of patterns\n";

    size_t badPatternCount = 0;
    double LargestGamma = 0;
    double SmallestGamma = 9999.0f;
    size_t HashTableEntryCount = 0;

    while (!feof(stream))
    {
        if (fscanf(stream, " %lf %d %d %s ", &gamma, &W, &A, temp) != 4)
            throw BenzeneException() << "Error parsing pattern\n";

	int size = (int)strlen(temp);
	for (int i = 1; i <= size; i++)
        {
            if (temp[i-1] == '5'){
                temp[i-1] = '3';
            }
	    pattern[i] = (int)temp[i - 1] - 48;
        }

 	Count[size]++;

	if (gamma > LargestGamma)
	    LargestGamma = gamma;
	if (gamma < SmallestGamma)
	    SmallestGamma = gamma;

        bool bad = false;
        if ((A >= 10000 && W == 0) ||
            (A >= 10000 && (double)W / (double)A <= 0.0001))
        {
            bad = true;
            badPatternCount++;
        }

	for (int i = 1; i <= 6; i++)
	{
	    if (InsertHashTable(ComputeKey(size, pattern), gamma, bad))
	        HashTableEntryCount++;
            if (HashTableEntryCount > TABLE_SIZE / 4)
                throw BenzeneException("Table too small!\n");
	    for (int j = 1; j <= 3; j++)
	        Rotate(pattern);
	}
    }

    LogInfo() << "GlobalPatterns:\n";
    for (size_t i = 0; i < MAX_INDEX; i++)
	if (Count[i] > 0)
	    LogInfo() << "size " << i << "         =  " << Count[i] << '\n';
    LogInfo() << "HashTableEntryCount  = " << HashTableEntryCount << '\n';
    LogInfo() << "BadPatternCount      = " << badPatternCount << '\n';
    LogInfo() << "LargestGamma         = " << LargestGamma << '\n';
    LogInfo() << "SmallestGamma        = " << SmallestGamma << '\n';
    fclose(stream);
}

//----------------------------------------------------------------------------

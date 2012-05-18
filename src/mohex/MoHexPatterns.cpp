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


void MoHexPatterns::Rotate(int pattern[], int* killer)
{
    static const int rot[] = 
        {  0, 
           3,  1,  5,  2,  6,  4,
           9,  7, 11,  8, 12, 10,
          15, 13, 17, 14, 18, 16
        };
    static const int backrot[] =
        {  0,
           2,  4,  1,  6,  3,  5,
           8, 10,  7, 12,  9, 11,
          14, 16, 13, 18, 15, 17            
        };

    *killer = backrot[ *killer ];

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

std::string MoHexPatterns::ShowPattern6(const int p[], const int e[])
{
    UNUSED(e);
    static const int index[] = { -1,  1, -1,  2, -1, -2, 
                                  3, -1, -4, -1,  4, -2, 
                                 -1,  5, -1,  6, -1, -2, 
                                 -3 }; 
    std::ostringstream os;
    os << '\n';
    for (int i = 0; ; ++i) 
    {
        LogInfo() << i << '\n';
        if (index[i] == -3)
            break;
        else if (index[i] == -2)
            os << '\n';
        else if (index[i] == -1)
            os << ' ';
        else if (index[i] == -4)
            os << '+';
        else
        {
            switch(p[index[i]])
            {
            case 0: os << '+'; break;
                //case 1: os << (e[index[i]] ? 'B' : 'b'); break;
                //case 2: os << (e[index[i]] ? 'W' : 'w'); break;
            case 1: os << 'b'; break;
            case 2: os << 'w'; break;
            case 3: os << '#'; break;
            case 4: os << '%'; break;
            default: os << '!'; break;
            }
        }
    }
    return os.str();
}

std::string MoHexPatterns::ShowPattern12(const int p[], const int e[])
{
    UNUSED(e);
    static const int index[] = { -1, -1, -1,  7, -1, -1, -1, -2,
                                  9, -1,  1, -1,  2, -1,  8, -2, 
                                 -1,  3, -1, -4, -1,  4, -1, -2, 
                                 11, -1,  5, -1,  6, -1, 10, -2, 
                                 -1, -1, -1, 12, -1, -1, -1, -2,
                                 -3 }; 
    std::ostringstream os;
    os << '\n';
    for (int i = 0; ; ++i) 
    {
        if (index[i] == -3)
            break;
        else if (index[i] == -2)
            os << '\n';
        else if (index[i] == -1)
            os << ' ';
        else if (index[i] == -4)
            os << '+';
        else
        {
            switch(p[index[i]])
            {
            case 0: os << '+'; break;
                //case 1: os << (e[index[i]] ? 'B' : 'b'); break;
                //case 2: os << (e[index[i]] ? 'W' : 'w'); break;
            case 1: os << 'b'; break;
            case 2: os << 'w'; break;
            case 3: os << '#'; break;
            case 4: os << '%'; break;
            default: os << '!'; break;
            }
        }
    }
    return os.str();
}

std::string MoHexPatterns::ShowPattern(int size, const int p[], const int e[])
{
    if (size == 6)
        return ShowPattern6(p, e);
    else if (size == 12)
        return ShowPattern12(p, e);
    return "-";
}

uint64_t MoHexPatterns::ComputeKey(int size, int pattern[])
{
    uint64_t key = 0;
    for (int i = 1; i <= size; i++)
	key ^= m_zobrist[0][i][pattern[i]];
    return key;
}

inline void MoHexPatterns
::GetKeyFromBoardBlackToPlay(uint64_t *key, const int size, 
                             const MoHexBoard& board, 
                             const HexPoint point) const
{
    key[0] = key[1] = key[2] = 0;
    static const int sizes[] = { 6, 12, 18, 9999 };
    const ConstBoard& cbrd = board.Const();
    for (int i = 1, r = 0; ; )
    {
        for (int j = 0; j < 6; ++i, ++j)
        {
            const HexPoint n = cbrd.PatternPoint(point, i, BLACK);
            switch(board.GetColor(n))
            {
            case EMPTY: 
                key[r] ^= m_zobrist[BLACK][i][0];
                break;
                
            case BLACK:
                key[r] ^= m_zobrist[BLACK][i][ 1 + (FIRST_CELL <= n ? 0: 2) ];
                break;
                
            case WHITE:
                key[r] ^= m_zobrist[BLACK][i][ 2 + (FIRST_CELL <= n ? 0: 2) ];
                break;
            }
        }
        if (size < sizes[++r])
            break;
        key[r] = key[r - 1];
    }
}

inline void MoHexPatterns
::GetKeyFromBoardWhiteToPlay(uint64_t *key, const int size, 
                             const MoHexBoard& board, 
                             const HexPoint point) const
{
    key[0] = key[1] = key[2] = 0;
    static const int sizes[] = { 6, 12, 18, 9999 };
    const ConstBoard& cbrd = board.Const();
    for (int i = 1, r = 0; ; )
    {
        for (int j = 0; j < 6; ++i, ++j)
        {
            const HexPoint n = cbrd.PatternPoint(point, i, WHITE);
            switch(board.GetColor(n))
            {
            case EMPTY: 
                key[r] ^= m_zobrist[WHITE][i][0];
                break;
                
            case BLACK:
                key[r] ^= m_zobrist[WHITE][i][ 2 + (FIRST_CELL <= n ? 0: 2) ];
                break;
                
            case WHITE:
                key[r] ^= m_zobrist[WHITE][i][ 1 + (FIRST_CELL <= n ? 0: 2) ];
                break;
            }
        }
        if (size < sizes[++r])
            break;
        key[r] = key[r - 1];
    }
}

inline void MoHexPatterns::GetKeyFromBoard(uint64_t *key, const int size, 
                                           const MoHexBoard& board, 
                                           const HexPoint point, 
                                           const HexColor toPlay) const
{
    if (toPlay == BLACK)
        GetKeyFromBoardBlackToPlay(key, size, board, point);
    else
        GetKeyFromBoardWhiteToPlay(key, size, board, point);
}

#if 1
inline void MoHexPatterns::GetKeyFromBoardOld(uint64_t *key_6, uint64_t *key_12, 
                                           uint64_t *key_18, const int size, 
                                           const MoHexBoard& board, 
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
#endif

float MoHexPatterns::GetGammaFromBoard(const MoHexBoard& board, int size,
                                        HexPoint point, HexColor toPlay) const
{
    uint64_t key[3];
    GetKeyFromBoard(key, size, board, point, toPlay);
#if 0
    {
        uint64_t key_6a, key_12a, key_18a;
        GetKeyFromBoardOld(&key_6a, &key_12a, &key_18a, size, board, 
                           point, toPlay);        
        if ((key_6a != key[0]) ||
            (key_12a != key[1]))
        {
            LogInfo() << board.Write() << "p=" << point << '\n';
            LogInfo() << key_6a << ' ' << key[0] << '\n';
            LogInfo() << key_12a << ' ' << key[1] << '\n';
            throw BenzeneException("Keys differ!\n");
        }
    }
#endif
    const Data* data;
    switch(size)
    {
    case 12:
        if ((data = QueryHashtable(key[1])) != NULL)
        {
            m_stats.hit12++;
            return data->gamma;
        }
        m_stats.miss12++;

    case 6:
        if ((data = QueryHashtable(key[0])) != NULL)
        {
            m_stats.hit6++;
            return data->gamma;
        }
        m_stats.miss6++;
    }
    return 1.0f;
}

void MoHexPatterns::Match(const MoHexBoard& board, int size,
                          HexPoint point, HexColor toPlay,
                          MoHexPatterns::Data* ret) const
{
    uint64_t key[3];
    GetKeyFromBoard(key, size, board, point, toPlay);
#if 0
    {
        uint64_t key_6a, key_12a, key_18a;
        GetKeyFromBoardOld(&key_6a, &key_12a, &key_18a, size, board, 
                           point, toPlay);        
        if ((key_6a != key[0]) ||
            (key_12a != key[1]))
        {
            LogInfo() << board.Write() << "p=" << point << '\n';
            LogInfo() << key_6a << ' ' << key[0] << '\n';
            LogInfo() << key_12a << ' ' << key[1] << '\n';
            throw BenzeneException("Keys differ!\n");
        }
    }
#endif

    static const int mirror[] = 
        {  0,
           3,  5,  1,  6,  2,  4,
          11, 12,  9, 10,  7,  8,
          15, 17, 13, 18, 14, 16
        };

    const MoHexPatterns::Data* data;
    switch(size)
    {
    case 12:
        if ((data = QueryHashtable(key[1])) != NULL)
	{
            m_stats.hit12++;
            *ret = *data;
            if (toPlay == WHITE)
                ret->killer = mirror[ ret->killer ];
            return;
	}
        m_stats.miss12++;

    case 6:
        if ((data = QueryHashtable(key[0])) != NULL)
	{
            m_stats.hit6++;
            *ret = *data;
            if (toPlay == WHITE)
                ret->killer = mirror[ ret->killer ];
            return;
        }
        m_stats.miss6++;
    }
    return;
}


const MoHexPatterns::Data* MoHexPatterns::QueryHashtable(uint64_t key) const
{
    static const uint64_t mask = (uint64_t)(TABLE_SIZE - 1);
    uint64_t index = key & mask;
    for (;;)
    {
	if (m_table[index].key == 0)
            return NULL;
	else if (m_table[index].key == key)
            return &m_table[index];
        index++;
        index &= mask;
    }
    return NULL;
}

bool MoHexPatterns::InsertHashTable(uint64_t key, float gamma, 
                                    int type, int killer)
{
    const uint64_t mask = (uint64_t)(TABLE_SIZE - 1);
    uint64_t index = key & mask;
    for (;;)
    {
        if (m_table[index].key == 0)
	{
            m_table[index].key = key;
            m_table[index].gamma = gamma;
            m_table[index].type = type;
            m_table[index].killer = killer;
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
        m_table[i].type = 0;
        m_table[i].killer = 0;
    }

    int count[MAX_INDEX] = {0};
    size_t prunableCount = 0;
    float largestGamma = 0;
    float smallestGamma = 9999.0f;
    size_t hashTableEntryCount = 0;

    std::ifstream ins;
    MiscUtil::OpenFile(filename, ins);
    {
        int A;
        ins >> A; // skip number of patterns in file
    }
    while (ins.good()) 
    {
        std::string line;
        if (!std::getline(ins, line))
            break;
        if (line.size() < 5)
            continue;

        float gamma;
        int A, W, type, killer;
        char temp[128];
        std::istringstream ss(line);
    
        ss >> gamma;
        ss >> W;
        ss >> A;
        ss >> temp;
        ss >> type;
        ss >> killer;

	int size = (int)strlen(temp);
        int pattern[MAX_INDEX];
        int edge[MAX_INDEX];

	for (int i = 1; i <= size; i++)
        {
            if (temp[i-1] == '5')
                temp[i-1] = '3';
	    pattern[i] = (int)temp[i - 1] - 48;
        }

 	count[size]++;

	if (gamma > largestGamma)
	    largestGamma = gamma;
	if (gamma < smallestGamma)
	    smallestGamma = gamma;

        if (type == 2) 
        {
            if (pattern[killer] != 0) {
                LogInfo() << ShowPattern(size, pattern, edge) << '\n';
                LogInfo() << "killer=" << killer << '\n';
                throw BenzeneException("Bad killer!\n");
            }
        }
	for (int i = 1; i <= 2; i++)
	{
            uint64_t key = ComputeKey(size, pattern);
	    if (InsertHashTable(key, gamma, type, killer))
            {
	        hashTableEntryCount++;
                if (type)
                    prunableCount++;
            }
            if (hashTableEntryCount > TABLE_SIZE / 4)
                throw BenzeneException("Table too small!\n");
	    for (int j = 1; j <= 3; j++)
	        Rotate(pattern, &killer);
	}
    }

    for (size_t i = 0; i < MAX_INDEX; i++)
	if (count[i] > 0)
	    LogInfo() << "size " << i << "         =  " << count[i] << '\n';
    LogInfo() << "HashTableEntryCount  = " << hashTableEntryCount << '\n';
    LogInfo() << "PrunableCount        = " << prunableCount << '\n';
    LogInfo() << "LargestGamma         = " << largestGamma << '\n';
    LogInfo() << "SmallestGamma        = " << smallestGamma << '\n';
}

//----------------------------------------------------------------------------

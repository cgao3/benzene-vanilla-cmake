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

uint64_t MoHexPatterns::m_zobrist[MoHexPatterns::MAX_INDEX][6];
uint64_t MoHexPatterns::m_zobrist_size[5];

MoHexPatterns::MoHexPatterns()
    : m_table(2)
{
    m_table[0] = new Data[TABLE_SIZE];
    m_table[1] = new Data[TABLE_SIZE];
}

MoHexPatterns::~MoHexPatterns()
{
    delete[] m_table[0];
    delete[] m_table[1];
}

uint64_t MoHexPatterns::RandomHash()
{
    uint64_t a = SgRandom::Global().Int();
    uint64_t b = SgRandom::Global().Int();
    return (a << 32) | b;
}

void MoHexPatterns::InitializeZobrist()
{
    int old_seed = SgRandom::Global().Seed();
    SgRandom::Global().SetSeed(1);
    for (size_t i = 0; i < MAX_INDEX; i++) 
	for (int j = 0; j < 6; j++) 
	    m_zobrist[i][j] = RandomHash();
    for (size_t i = 0; i < 5; ++i)
        m_zobrist_size[i] = RandomHash();
    SgRandom::SetSeed(old_seed);
}

int MoHexPatterns::Mirror(int loc)
{
    static const int mirror[] = 
        {  0,
           3,  5,  1,  6,  2,  4,
          11, 12,  9, 10,  7,  8,
          15, 17, 13, 18, 14, 16
        };
    return mirror[loc];
}

void MoHexPatterns::MirrorAndFlipPattern(int size, int pattern[], int* killer)
{
    static const int flip[] = { 0, 2, 1, 4, 3, 5 };

    *killer = Mirror(*killer);
    for (int i = 1; i <= size; ++i)
        pattern[i] = flip[ pattern[i] ];

    int temp[MAX_INDEX];
    for (int i = 1; i <= size; ++i)
        temp[i] = pattern[ Mirror(i) ];

    for (int i = 1; i <= size; ++i)
        pattern[i] = temp[i];
}

void MoHexPatterns::RotatePattern(int size, int pattern[], int* killer)
{
    UNUSED(size);
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

uint64_t MoHexPatterns::ComputeKey(int size, const int pattern[])
{
    uint64_t key = (size == 6) ? m_zobrist_size[0] : m_zobrist_size[1];
    for (int i = 1; i <= size; i++)
        if (pattern[i])
            key ^= m_zobrist[i][pattern[i]];
    return key;
}

void MoHexPatterns::GetKeyFromBoard(uint64_t *key, const int size, 
                                    const MoHexBoard& board, 
                                    const HexPoint point, 
                                    const HexColor toPlay)
{
    UNUSED(toPlay);
    key[0] = m_zobrist_size[0];
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
                break;
                
            case BLACK:
                key[r] ^= m_zobrist[i][ (FIRST_CELL <= n ? 1: 3) ];  
                break;
                
            case WHITE:
                key[r] ^= m_zobrist[i][ (FIRST_CELL <= n ? 2: 4) ];
                break;
            }
        }
        if (size < sizes[++r])
            break;
        key[r] = key[r - 1] ^ m_zobrist_size[r - 1] ^ m_zobrist_size[r];
    }
}

void MoHexPatterns::MatchWithKeys(const uint64_t* keys, const int size, 
                                  const HexColor toPlay, const Data** ret) const
{
    const Data* table = m_table[toPlay];
    switch(size)
    {
    case 12:
        if ((*ret = QueryHashtable(table, keys[1])) != NULL)
	{
            m_stats.hit12++;
            return;
	}
        m_stats.miss12++;

    case 6:
        if ((*ret = QueryHashtable(table, keys[0])) != NULL)
	{
            m_stats.hit6++;
            return;
        }
        m_stats.miss6++;
    }
}

void MoHexPatterns::MatchWithKeysBoth(const uint64_t* keys, 
                                      const HexColor toPlay, 
                                      const Data** ret) const
{
    const Data* table = m_table[toPlay];
    if ((*ret = QueryHashtable(table, keys[1])) != NULL)
    {
        m_stats.hit12++;
        return;
    }
    m_stats.miss12++;
    if ((*ret = QueryHashtable(table, keys[0])) != NULL)
    {
        m_stats.hit6++;
        return;
    }
    m_stats.miss6++;
}

void MoHexPatterns::Match(const MoHexBoard& board, int size,
                          HexPoint point, HexColor toPlay,
                          const Data** ret) const
{
    uint64_t keys[3];
    GetKeyFromBoard(keys, size, board, point, toPlay);
    MatchWithKeys(keys, size, toPlay, ret);
}

float MoHexPatterns::GammaFromKeysBoth(const uint64_t* keys,
                                       const HexColor toPlay) const
{
    const Data* table = m_table[toPlay];
    const Data* data;
    if ((data = QueryHashtable(table, keys[1])) != NULL)
    {
        m_stats.hit12++;
        return data->gamma;
    }
    m_stats.miss12++;
    if ((data = QueryHashtable(table, keys[0])) != NULL)
    {
        m_stats.hit6++;
        return data->gamma;
    }
    m_stats.miss6++;
    return 1.0f;
}

inline const MoHexPatterns::Data* 
MoHexPatterns::QueryHashtable(const Data* table, uint64_t key) const
{
    static const uint64_t mask = (uint64_t)(TABLE_SIZE - 1);
    uint64_t index = key & mask;
    for (;;)
    {
	if (table[index].key == key)
            return &table[index];
	if (table[index].key == 0)
            return NULL;
        index++;
        index &= mask;
    }
    return NULL;
}

bool MoHexPatterns::InsertHashTable(Data* table, uint64_t key, float gamma, 
                                    int type, int killer, int id)
{
    const uint64_t mask = (uint64_t)(TABLE_SIZE - 1);
    uint64_t index = key & mask;
    for (;;)
    {
        if (table[index].key == 0)
	{
            table[index].key = key;
            table[index].gamma = gamma;
            table[index].type = type;
            table[index].killer = killer;
            table[index].id = id;
	    return true;
	}
	else if (table[index].key == key)
	{
            if (table[index].gamma == gamma) 
                // Duplicate: can happen because pattern is the same under
                // rotation.
                return false;
            // Gammas can be different for the same key because of how we
            // handle the obtuse corner (ie, always setting it to black). 
            // This causes patterns that were trained as different patterns
            // to be indistinguishable here.
            if ((table[index].type > 0) != (type > 0))
                throw BenzeneException("Prunable classification mismatch.");

            // If non-prunable: take largest gamma
            if (type == 0 && gamma > table[index].gamma)
                table[index].gamma = gamma;
	    return false;
	}
        index++;
        index &= mask;
    }
    return false;
}

float MoHexPatterns::DefaultGammaFunction(int type, float gamma)
{
    UNUSED(type);
    return gamma;
}

void MoHexPatterns::ReadPatterns(std::string filename, bool makeMirrorComplete,
                                 float (*GammaFunction)(int type, float gamma))
{
    for (size_t i = 0; i < TABLE_SIZE; ++i)
    {
        m_table[BLACK][i].key = 0;
        m_table[BLACK][i].gamma = 1.0f;
        m_table[BLACK][i].otherGamma =  1.0f;
        m_table[BLACK][i].localGamma = -1.0f;
        m_table[BLACK][i].type = 0;
        m_table[BLACK][i].killer = 0;
        m_table[BLACK][i].id = -1;

        m_table[WHITE][i].key = 0;
        m_table[WHITE][i].gamma = 1.0f;
        m_table[WHITE][i].otherGamma =  1.0f;
        m_table[WHITE][i].localGamma = -1.0f;
        m_table[WHITE][i].type = 0;
        m_table[WHITE][i].killer = 0;
        m_table[WHITE][i].id = -1;
    }

    int count[MAX_INDEX] = {0};
    size_t tableEntries[2] = { 0, 0 };
    size_t prunableCount[2] = { 0, 0 };
    float largestGamma = 0;
    float smallestGamma = 9999.0f;

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
            char c = temp[i-1];
            if (c == '5')
                c = '3';
	    pattern[i] = (int)c - 48;
        }
 	count[size]++;

	if (gamma > largestGamma)
	    largestGamma = gamma;
	if (gamma < smallestGamma)
	    smallestGamma = gamma;

        if (type == 3) 
        {
            if (pattern[killer] != 0) {
                LogInfo() << ShowPattern(size, pattern, edge) << '\n';
                LogInfo() << "killer=" << killer << '\n';
                throw BenzeneException("Bad killer!\n");
            }
        }

        // Change gamma based on supplied gamma function
        gamma = GammaFunction(type, gamma);

        // Add to black table
	for (int i = 1; i <= 2; i++)
	{
            uint64_t key = ComputeKey(size, pattern);
            int id = (int)m_patterns.size();
	    if (InsertHashTable(m_table[BLACK], key, gamma, type, killer, id))
            {
                m_patterns.push_back(Pattern(size, pattern));
	        tableEntries[BLACK]++;
                if (type)
                    prunableCount[BLACK]++;
            }
            if (tableEntries[BLACK] > TABLE_SIZE / 4)
                throw BenzeneException("Black table too small!\n");
	    for (int j = 1; j <= 3; j++)
	        RotatePattern(size, pattern, &killer);
	}

        // Add to white table, after flipping colors and mirroring
	for (int i = 1; i <= size; i++)
        {
            char c = temp[i-1];
            if (c == '5')
                // NOTE: set it to '4' so it gets flipped and 
                // mirrored to a black cell. This ensures the obtuse
                // corner is always black
                c = '4';
	    pattern[i] = (int)c - 48;
        }
        MirrorAndFlipPattern(size, pattern, &killer);
	for (int i = 1; i <= 2; i++)
	{
            uint64_t key = ComputeKey(size, pattern);
            int id = (int)m_patterns.size();
	    if (InsertHashTable(m_table[WHITE], key, gamma, type, killer, id))
            {
	        tableEntries[WHITE]++;
                m_patterns.push_back(Pattern(size, pattern));
                if (type)
                    prunableCount[WHITE]++;
            }
            if (tableEntries[WHITE] > TABLE_SIZE / 4)
                throw BenzeneException("White table too small!\n");
	    for (int j = 1; j <= 3; j++)
	        RotatePattern(size, pattern, &killer);
	}
    }

    // Pre-compute the gamma for the opposite color
    size_t mirrorsAdded[2] = { 0, 0 };
    if (makeMirrorComplete) 
    {
        for (BWIterator c; c; ++c)
        {
            const HexColor color = *c;
            const HexColor other = !color;
            Data* gt = m_table[color];
            for (size_t i = 0; i < TABLE_SIZE; ++i)
            {        
                const uint64_t key = gt[i].key;
                const float gamma = gt[i].gamma;
                if (key == 0)
                    continue;
                const Pattern& cp = m_patterns[gt[i].id];
                const Data* data = QueryHashtable(m_table[other], key);
                if (data == NULL)
                {
                    // mirror of this pattern was not encoded for this color
                    // create a 'fake' entry for opponent
                    float fakeGamma = 1.0f;
                    int fakeType = 0;
                    int fakeKiller = 0;
                    if (cp.size == 12)
                    {
                        // try using the 6-pattern
                        uint64_t key6 = ComputeKey(6, cp.pattern);
                        data = QueryHashtable(m_table[other], key6);
                        if (data != NULL)
                        {
                            fakeGamma = data->gamma;
                            fakeType = data->type;
                            fakeKiller = data->killer;
                        }
                    }
                    InsertHashTable(m_table[other], key, fakeGamma, fakeType,
                                    fakeKiller, gt[i].id);
                    data = QueryHashtable(m_table[other], key);
                    const_cast<Data*>(data)->otherGamma = gamma;
                    gt[i].otherGamma = fakeGamma;
                    mirrorsAdded[other]++;
                    tableEntries[other]++;
                }
                else 
                {
                    // const Pattern& lp = m_patterns[data->id];
                    // LogInfo() << "===== Match ======="
                    //           << ShowPattern(cp.size, cp.pattern, cp.pattern)
                    //           << ShowPattern(lp.size, lp.pattern, lp.pattern) 
                    //           << '\n' << gt[i].gamma << ' ' << data->gamma << '\n';
                    gt[i].otherGamma = data->gamma;
                }
            }
        }
    }

    LogInfo() << "Size            = ";
    for (size_t i = 0; i < MAX_INDEX; i++)
	if (count[i] > 0)
	    LogInfo() << count[i] << ' ';
    LogInfo() << '\n';
    LogInfo() << "TableEntries    = " << tableEntries[BLACK]
              << ' ' << tableEntries[WHITE] << '\n';
    LogInfo() << "MirrorsAdded    = " << mirrorsAdded[BLACK] 
              << ' ' << mirrorsAdded[WHITE] << '\n';
    LogInfo() << "PrunableCount   = " << prunableCount[BLACK]
              << ' ' << prunableCount[WHITE] << '\n';
    LogInfo() << "LargestGamma    = " << largestGamma << '\n';
    LogInfo() << "SmallestGamma   = " << smallestGamma << '\n';
}

//----------------------------------------------------------------------------

void MoHexPatterns::AddLocalToGlobal(MoHexPatterns& global,
                                     MoHexPatterns& local)
{
    for (BWIterator c; c; ++c)
    {
        Data* gt = global.m_table[*c];
        for (size_t i = 0; i < TABLE_SIZE; ++i)
        {        
            uint64_t key = gt[i].key;
            if (key == 0 || gt[i].type != 0)
                continue;
            const Pattern& gp = global.m_patterns[gt[i].id];
            const Data* data = local.QueryHashtable(local.m_table[*c], key);
            if (data == NULL)
            {
                // 12 pattern did not match, try the 6 pattern
                uint64_t key6 = ComputeKey(6, gp.pattern);
                data = local.QueryHashtable(local.m_table[*c], key6);
            }
            if (data != NULL) 
            {
                //const Pattern& lp = local.m_patterns[data->id];
                // LogInfo() << "===== Match ======="
                //           << ShowPattern(gp.size, gp.pattern, gp.pattern)
                //           << ShowPattern(lp.size, lp.pattern, lp.pattern) 
                //           << '\n' << "localGamma=" << data->gamma << '\n';
                
                // Take max local gamma in case of duplicates
                gt[i].localGamma = std::max(gt[i].localGamma, data->gamma);
            }
        }
    }
}

//----------------------------------------------------------------------------

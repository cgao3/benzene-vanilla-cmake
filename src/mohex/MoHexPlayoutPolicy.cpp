//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"
#include "SgHash.h"
#include "MoHexPlayoutPolicy.hpp"
#include "Misc.hpp"
#include "PatternState.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

// TODO: refactor this stuff into its own class?

// Local patterns
size_t LocalPatterns_6PatternHit = 0;
size_t LocalPatterns_6PatternMiss = 0;
size_t LocalPatterns_12PatternHit = 0;

// Global patterns
size_t GlobalPatterns_6PatternHit = 0;
size_t GlobalPatterns_6PatternMiss = 0;
size_t GlobalPatterns_12PatternHit = 0;     
size_t GlobalPatterns_18PatternHit = 0;
size_t GlobalPatterns_BadPattern_Tree = 0;
size_t GlobalPatterns_BadPattern_Playout = 0;      

double GlobalPatterns_LargestGamma = 0;
double GlobalPatterns_SmallestGamma = 9999.0f;

void InitializeDirection();
void InitializeZobrist();

void LocalPatterns_ReadPatterns();
void GlobalPatterns_ReadPatterns();
void ReadGammas();

double LocalPatterns_GetGammaFromBoard(const StoneBoard& board, 
                                       HexPoint point, HexColor toPlay,
                                       bool *isBadPattern);

double GlobalPatterns_GetGammaFromBoard(const StoneBoard& board, 
                                        HexPoint point, HexColor toPlay,
                                        bool *isBadPattern);

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

/** Shuffle a vector with the given random number generator. 
    @todo Refactor this out somewhere. */
template<typename T>
void ShuffleVector(std::vector<T>& v, SgRandom& random)
{
    for (int i = static_cast<int>(v.size() - 1); i > 0; --i) 
    {
        int j = random.Int(i+1);
        std::swap(v[i], v[j]);
    }
}

/** Returns true 'percent' of the time. */
bool PercentChance(int percent, SgRandom& random)
{
    if (percent >= 100) 
        return true;
    unsigned int threshold = random.PercentageThreshold(percent);
    return random.RandomEvent(threshold);
}

//----------------------------------------------------------------------------

} // annonymous namespace

//----------------------------------------------------------------------------

std::string MoHexPlayoutPolicyStatistics::ToString() const
{
    std::ostringstream os;
    os << "Playout Statistics:\n"
       << "Total          " << totalMoves << '\n'
       << "Pattern        " << patternMoves << " ("
       << std::setprecision(3) << double(patternMoves) * 100.0 
        / double(totalMoves) << "%)\n" 
       << "Random         " << randomMoves << " ("
       << std::setprecision(3) << double(randomMoves) * 100.0 
        / double(totalMoves) << "%)";
    return os.str();
}

//----------------------------------------------------------------------------

MoHexSharedPolicy::MoHexSharedPolicy()
    : m_config()
{
    ///
    // MOVE AJA PATTERNS OUT OF HERE!!!!
    ///
    InitializeZobrist();
    InitializeDirection();
    //LocalPatterns_ReadPatterns();
    GlobalPatterns_ReadPatterns();
    //ReadGammas();
}

MoHexSharedPolicy::~MoHexSharedPolicy()
{
}

//----------------------------------------------------------------------------

MoHexPlayoutPolicy::MoHexPlayoutPolicy(MoHexSharedPolicy* shared)
    : m_shared(shared)
{
}

MoHexPlayoutPolicy::~MoHexPlayoutPolicy()
{
}

//----------------------------------------------------------------------------

void MoHexPlayoutPolicy::InitializeForSearch()
{
}

void MoHexPlayoutPolicy::InitializeForPlayout(const StoneBoard& brd)
{
    BitsetUtil::BitsetToVector(brd.GetEmpty(), m_moves);
    ShuffleVector(m_moves, m_random);
}

HexPoint MoHexPlayoutPolicy::GenerateMove(const HexState& state, 
                                          HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    const MoHexPlayoutPolicyConfig& config = m_shared->Config();
    MoHexPlayoutPolicyStatistics& stats = m_shared->Statistics();
    // Patterns applied probabilistically (if heuristic is turned on)
    if (config.patternHeuristic 
        && PercentChance(config.patternCheckPercent, m_random))
    {
        move = GeneratePatternMove(state, lastMove);
    }
    // Select random move if no move was selected by the heuristics
    if (move == INVALID_POINT) 
    {
	stats.randomMoves++;
        move = GenerateRandomMove(state.Position());
    } 
    else 
        stats.patternMoves++;
    BenzeneAssert(state.Position().IsEmpty(move));
    stats.totalMoves++;
    return move;
}

//--------------------------------------------------------------------------

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove(const StoneBoard& brd)
{
    HexPoint ret = INVALID_POINT;
    while (true) 
    {
	BenzeneAssert(!m_moves.empty());
        ret = m_moves.back();
        m_moves.pop_back();
        if (brd.IsEmpty(ret))
            break;
    }
    return ret;
}

/** Checks the save-bridge pattern. */
HexPoint MoHexPlayoutPolicy::GeneratePatternMove(const HexState& state, 
                                                 HexPoint lastMove)
{
    const StoneBoard& brd = state.Position();
    const HexColor toPlay = state.ToPlay();
    HexPoint p[6];
    for (int i = 0; i < 6; ++i)
        p[i] = brd.Const().PointInDir(lastMove, (HexDirection)i);
    // State machine: s is number of cells matched.
    // In clockwise order, need to match CEC, where C is the color to
    // play and E is an empty cell. We start at a random direction and
    // stop at first match, which handles the case of multiple bridge
    // patterns occuring at once.
    // We use 'brd.IsColor(cell, color)' because it is a single 
    // bitset check. brd.IsEmpty() will check against two bitsets,
    // so we avoid calling that here.
    // TODO: speed this up?
    int s = 0;
    const int k = m_random.Int(6);
    HexPoint ret = INVALID_POINT;
    for (int j = 0; j < 8; ++j)
    {
        const int i = (j + k) % 6;
        bool mine = brd.IsColor(p[i], toPlay);
        if (s == 0)
        {
            if (mine) s = 1;
        }
        else if (s == 1)
        {
            if (mine) s = 1;
            else if (brd.IsColor(p[i], !toPlay)) s = 0;
            else 
            {
                s = 2;
                ret = p[i];
            }
        }
        else if (s == 2)
        {
            if (mine) return ret; // matched!!
            else s = 0;
        }
    }
    return INVALID_POINT;
}

//----------------------------------------------------------------------------

#define LocalPatterns_HashTableSize (1 << 20)

uint64_t  PatternZobrist[2][100][6];
uint64_t  LocalPatterns_HashTable_Key[LocalPatterns_HashTableSize];
double    LocalPatterns_HashTable_Gamma[LocalPatterns_HashTableSize];
bool      LocalPatterns_HashTable_BadPattern[LocalPatterns_HashTableSize];

HexDirection PatternDirection[100];

void InitializeDirection()
{
    PatternDirection[1]  = DIR_NORTH;
    PatternDirection[2]  = DIR_NORTH_EAST;
    PatternDirection[3]  = DIR_WEST;
    PatternDirection[4]  = DIR_EAST;
    PatternDirection[5]  = DIR_SOUTH_WEST;
    PatternDirection[6]  = DIR_SOUTH;

    PatternDirection[7]  = DIR_NORTH_EAST;
    PatternDirection[8]  = DIR_EAST;
    PatternDirection[9]  = DIR_NORTH;
    PatternDirection[10] = DIR_SOUTH;
    PatternDirection[11] = DIR_WEST;
    PatternDirection[12] = DIR_SOUTH_WEST;

    PatternDirection[13] = DIR_NORTH;
    PatternDirection[14] = DIR_NORTH_EAST;
    PatternDirection[15] = DIR_WEST;
    PatternDirection[16] = DIR_EAST;
    PatternDirection[17] = DIR_SOUTH_WEST;
    PatternDirection[18] = DIR_SOUTH;

}

void LocalPatterns_InitializeHashTable()
{
    for (int i = 0; i < LocalPatterns_HashTableSize; i++)
    {
	LocalPatterns_HashTable_Key[i] = 0;
	LocalPatterns_HashTable_Gamma[i] = 1.0f;
        LocalPatterns_HashTable_BadPattern[i] = false;
    }

}

void InitializeZobrist()
{
    int old_seed = SgRandom::Global().Seed();
    SgRandom::Global().SetSeed(1);
    for (int i = 0; i <= 100; i++) 
	for (int j = 0; j <= 5; j++) 
        {
            uint64_t a = SgRandom::Global().Int();
            uint64_t b = SgRandom::Global().Int();
	    PatternZobrist[0][i][j] = (a << 32) | b;
        }
    SgRandom::SetSeed(old_seed);    

    for (int i = 0; i <= 5; i++)
    {
	PatternZobrist[1][13][i] = PatternZobrist[0][15][i];
	PatternZobrist[1][ 7][i] = PatternZobrist[0][11][i];
	PatternZobrist[1][14][i] = PatternZobrist[0][17][i];
	PatternZobrist[1][ 9][i] = PatternZobrist[0][ 9][i];
	PatternZobrist[1][ 1][i] = PatternZobrist[0][ 3][i];
	PatternZobrist[1][ 2][i] = PatternZobrist[0][ 5][i];
        PatternZobrist[1][ 8][i] = PatternZobrist[0][12 ][i];
	PatternZobrist[1][15][i] = PatternZobrist[0][13][i];
	PatternZobrist[1][ 3][i] = PatternZobrist[0][ 1][i];
	PatternZobrist[1][ 4][i] = PatternZobrist[0][ 6][i];
	PatternZobrist[1][16][i] = PatternZobrist[0][18][i];
	PatternZobrist[1][11][i] = PatternZobrist[0][ 7][i];
	PatternZobrist[1][ 5][i] = PatternZobrist[0][ 2][i];
        PatternZobrist[1][ 6][i] = PatternZobrist[0][ 4][i];
	PatternZobrist[1][10][i] = PatternZobrist[0][10 ][i];
	PatternZobrist[1][17][i] = PatternZobrist[0][14][i];
	PatternZobrist[1][12][i] = PatternZobrist[0][ 8][i];
	PatternZobrist[1][18][i] = PatternZobrist[0][16][i];
    }
}

uint64_t LocalPatterns_ComputeKey(int size, int pattern[100])
{
    uint64_t key = 0;
    for (int i = 1; i <= size; i++)
	key ^= PatternZobrist[0][i][pattern[i]];
    return key;
}

double LocalPatterns_QueryHashtable(uint64_t key, bool *isBadPattern);
inline double GlobalPatterns_QueryHashtable(uint64_t key, bool *isBadPattern);

inline void LocalPatterns_GetKeyFromBoard(uint64_t *key_6, uint64_t *key_12, 
                                          uint64_t *key_18, int size, 
                                          const StoneBoard& board, 
                                          HexPoint point, HexColor toPlay)
{
    *key_6 = 0;
    *key_12 = 0;
    *key_18 = 0;

    for (int i = 1; i <= 6; i++)
    {
	HexPoint n = BoardUtil::PointInDir(board.Const(), point, 
                                           PatternDirection[i]);
        if (! HexPointUtil::isEdge(n))
	{
	    if (!HexPointUtil::isInteriorCell(n))
	    {
		*key_6 ^= PatternZobrist[(int)toPlay][i][5];
		continue;
	    }
	    if (board.GetColor(n) == EMPTY)
	    {
	 	*key_6 ^= PatternZobrist[(int)toPlay][i][0];
	    }
	    else if (board.GetColor(n) == toPlay)
	    {
		*key_6 ^= PatternZobrist[(int)toPlay][i][1];
	    }
	    else if (board.GetColor(n) == !toPlay)
	    {
		*key_6 ^= PatternZobrist[(int)toPlay][i][2];
	    }
	}
	else
	{
	    if (board.GetColor(n) == toPlay)
	    {
		*key_6 ^= PatternZobrist[(int)toPlay][i][3];
	    }
	    else if (board.GetColor(n) == !toPlay)
	    {
		*key_6 ^= PatternZobrist[(int)toPlay][i][4];
	    }
	}

    }

    if (size >= 12)
    {
	*key_12 = *key_6;

	for (int i = 1; i <= 6; i++)
	{
	    HexPoint n = BoardUtil::PointInDir(board.Const(), point, 
                                               PatternDirection[i]);
	    if (HexPointUtil::isEdge(n))
	    {
	        if (board.GetColor(n) == toPlay)
		    *key_12 ^= PatternZobrist[(int)toPlay][i + 6][3];
	        else if (board.GetColor(n) == !toPlay)
		    *key_12 ^= PatternZobrist[(int)toPlay][i + 6][4];
	    }
	    else
	    {
		if (! HexPointUtil::isInteriorCell(n))
		{
		    *key_12 ^= PatternZobrist[(int)toPlay][i + 6][5];
		    continue;
		}
		
		HexPoint m = BoardUtil::PointInDir(board.Const(), n, 
                                                PatternDirection[i + 6]);
		
		if (! HexPointUtil::isEdge(m))
		{
		    if (! HexPointUtil::isInteriorCell(m))
		    {
			*key_12 ^= PatternZobrist[(int)toPlay][i + 6][5];
			continue;
		    }
		    if (board.GetColor(m) == EMPTY)
			*key_12 ^= PatternZobrist[(int)toPlay][i + 6][0];
		    else if (board.GetColor(m) == toPlay)
		        *key_12 ^= PatternZobrist[(int)toPlay][i + 6][1];
		    else if (board.GetColor(m) == !toPlay)
		        *key_12 ^= PatternZobrist[(int)toPlay][i + 6][2];
		}
		else
		{
		    if (board.GetColor(m) == toPlay)
		        *key_12 ^= PatternZobrist[(int)toPlay][i + 6][3];
		    else if (board.GetColor(m) == !toPlay)
		        *key_12 ^= PatternZobrist[(int)toPlay][i + 6][4];
		}
	    }
	}
    }
   
    if (size >= 18)
    {
	*key_18 = *key_12;
	for (int i = 1; i <= 6; i++)
	{
	    HexPoint n = BoardUtil::PointInDir(board.Const(), point, 
                                               PatternDirection[i]);
	    if (HexPointUtil::isEdge(n))
	    {
		if (board.GetColor(n) == toPlay)
		    *key_18 ^= PatternZobrist[(int)toPlay][i + 12][3];
		else if (board.GetColor(n) == !toPlay)
		    *key_18 ^= PatternZobrist[(int)toPlay][i + 12][4];
	    }
	    else
	    {
		if (! HexPointUtil::isInteriorCell(n))
		{
		    *key_18 ^= PatternZobrist[(int)toPlay][i + 12][5];
		    continue;
		}

		HexPoint m = BoardUtil::PointInDir(board.Const(), n, 
                                             PatternDirection[i + 12]);
	    
	    	if (! HexPointUtil::isEdge(m))
		{
		    if (! HexPointUtil::isInteriorCell(m))
		    {
		        *key_18 ^= PatternZobrist[(int)toPlay][i + 12][5];
		        continue;
		    }

		    if (board.GetColor(m) == EMPTY)
		        *key_18 ^= PatternZobrist[(int)toPlay][i + 12][0];
		
		    else if (board.GetColor(m) == toPlay)
		        *key_18 ^= PatternZobrist[(int)toPlay][i + 12][1];
		    
		    else if (board.GetColor(m) == !toPlay)
		        *key_18 ^= PatternZobrist[(int)toPlay][i + 12][2];
		
		}
	        else
		{
		    if (board.GetColor(m) == toPlay)
		        *key_18 ^= PatternZobrist[(int)toPlay][i + 12][3];
	
	 	    else if (board.GetColor(m) == !toPlay)
		        *key_18 ^= PatternZobrist[(int)toPlay][i + 12][4];
		}
            }
	}
    }
}

double LocalPatterns_GetGammaFromBoard(const StoneBoard& board, HexPoint point,
                                       HexColor toPlay, bool *isBadPattern)
{
    double gamma = 1.0f;
    uint64_t key_6, key_12, key_18;
    
    LocalPatterns_GetKeyFromBoard(&key_6, &key_12, &key_18, 12, board, 
                                  point, toPlay);

    gamma = LocalPatterns_QueryHashtable(key_12, isBadPattern);

    if (gamma == 1.0f)
    {
	gamma = LocalPatterns_QueryHashtable(key_6, isBadPattern);
	if (gamma == 1.0f)
	{
	    LocalPatterns_6PatternMiss++;
	}
	else
	    LocalPatterns_6PatternHit++;
    }
    else
        LocalPatterns_12PatternHit++;

    return gamma;
}

double GlobalPatterns_GetGammaFromBoard(const StoneBoard& board, 
                                        HexPoint point, HexColor toPlay,
                                        bool *isBadPattern)
{
    *isBadPattern = false;
    double gamma = 1.0f;

    uint64_t key_6, key_12, key_18;
    LocalPatterns_GetKeyFromBoard(&key_6, &key_12, &key_18, 12, board, 
                                  point, toPlay);
    //LocalPatterns_GetKeyFromBoard(&key_6, &key_12, &key_18, 18, board, point, toPlay);

    //gamma = GlobalPatterns_QueryHashtable(key_18, isBadPattern);

    if (gamma == 1.0f)
    {
        gamma = GlobalPatterns_QueryHashtable(key_12, isBadPattern);
	
	if (gamma == 1.0f)
	{
	    gamma = GlobalPatterns_QueryHashtable(key_6, isBadPattern);
    
    	    if (gamma == 1.0f)
	    {
	        GlobalPatterns_6PatternMiss++;
	        //return GlobalPatterns_SmallestGamma;
	    }
	    else   
            {
	        GlobalPatterns_6PatternHit++;
 	    }
	}
	else
	    GlobalPatterns_12PatternHit++;
    }
    else
    {
        GlobalPatterns_18PatternHit++;
    }
    return gamma;
}

void PatternRotation(int pattern[100])
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

double LocalPatterns_QueryHashtable(uint64_t key, bool *isBadPattern)
{
    *isBadPattern = false;

    int index = key & (uint64_t)(LocalPatterns_HashTableSize - 1);

    for (;;)
    {
	if (LocalPatterns_HashTable_Key[index] == 0)
	    return 1.0f;

	else if (LocalPatterns_HashTable_Key[index] == key)
	{
	    if (LocalPatterns_HashTable_BadPattern[index])
	        *isBadPattern = true;

	    return LocalPatterns_HashTable_Gamma[index];
	}
        if (index < LocalPatterns_HashTableSize - 1)
	    index ++;
	else 
	    index = 0;
    }
    return 1.0f;
}

bool LocalPatterns_InsertHashTable(uint64_t key, double gamma, int A, int W, 
                                   int pattern[100])
{
    bool TableIsFull = false;

    int index = key & (uint64_t)(LocalPatterns_HashTableSize - 1);

    for (;;)
    {
	if (LocalPatterns_HashTable_Key[index] == 0)
	{
	    LocalPatterns_HashTable_Key[index] = key;
	    LocalPatterns_HashTable_Gamma[index] = gamma;
	    if (W == 0)
		LocalPatterns_HashTable_BadPattern[index] = true;
            return true;
 	}
	else if (LocalPatterns_HashTable_Key[index] == key)
	{
	    return false;
	}
        
	if (index < LocalPatterns_HashTableSize - 1)
	    index++;
	else
	{
	    if (TableIsFull == false)
		TableIsFull = true;
	    else
	    {
		LogInfo() << "LocalPatterns_InsertHashTable(): TableIsFull\n";
		return false;
	    }
	    index = 0;
	}
    }
    return false;
}

void LocalPatterns_ReadPatterns()
{

    LocalPatterns_InitializeHashTable();

    std::ifstream ins;
    std::string fname 
        = MiscUtil::OpenFile("mohex-local-pattern-gamma.txt", ins);
    FILE *stream = fopen(fname.c_str(),"r");
    if (stream == NULL)
    {
        LogWarning() << "Failed to read '" << fname << "'\n";
	return;
    }

    double gamma;
    int A;
    int W;
    char temp[128];
    int pattern[100];
    int LocalPatternsCount[100] = {0};
    int LocalPatterns_HashTableEntryCount = 0;

    fscanf(stream,"%d\n", &A);
   
    while (! feof(stream))
    {
        fscanf(stream,"%lf %d %d", &gamma, &W, &A);


        fscanf(stream,"%s\n", temp);
        
	int size = strlen(temp);

	for (int i = 1; i <= size; i++)
            pattern[i] = (int)temp[i - 1] -48;

        LocalPatternsCount[size]++;

        // Insert the pattern
        for (int i = 1; i <= 6; i++)
        {    	   
	    if (LocalPatterns_InsertHashTable
                (LocalPatterns_ComputeKey(size, pattern), gamma, A, W, pattern))
                LocalPatterns_HashTableEntryCount++;

	    for (int j = 1; j <= 3; j++)
	        PatternRotation(pattern); 
        }
    }

    LogInfo() << "LocalPatterns:\n";
    for (int i = 0; i < 100; i++)
    {
        if (LocalPatternsCount[i] > 0)
	    LogInfo() << "size " << i << "         =  " 
                      << LocalPatternsCount[i] << "\n";
    }
    LogInfo() << "LocalPatterns_HashTableEntryCount   = " 
              << LocalPatterns_HashTableEntryCount << "\n";
    fclose(stream);
}

#define      GlobalPatterns_HashTableSize (1 << 20)

uint64_t     GlobalPatterns_HashTable_Key[GlobalPatterns_HashTableSize];
double       GlobalPatterns_HashTable_Gamma[GlobalPatterns_HashTableSize];
bool         GlobalPatterns_HashTable_BadPattern[GlobalPatterns_HashTableSize];

void GlobalPatterns_InitializeHashTable()
{
    for (int i = 0; i < GlobalPatterns_HashTableSize; i++)
    {
	GlobalPatterns_HashTable_Key[i] = 0;
	GlobalPatterns_HashTable_Gamma[i] = 1.0f;
	GlobalPatterns_HashTable_BadPattern[i] = false;
    }
}

inline double GlobalPatterns_QueryHashtable(uint64_t key, bool *isBadPattern)
{
    int index = key & (uint64_t)(GlobalPatterns_HashTableSize - 1);

    for (;;)
    {
	if (GlobalPatterns_HashTable_Key[index] == 0)
	    return 1.0f;

	else if (GlobalPatterns_HashTable_Key[index] == key)
	{
	    *isBadPattern = GlobalPatterns_HashTable_BadPattern[index];
	    return GlobalPatterns_HashTable_Gamma[index];
	}

	if (index < GlobalPatterns_HashTableSize - 1)
	    index++;
	else 
	    index = 0;
    }

    return 1.0f;
}

int GlobalPatterns_BadPatternCount = 0;

bool GlobalPatterns_InsertHashTable(uint64_t key, double gamma, int A, 
                                    int W, int pattern[100])
{
    bool TableIsFull = false;

    int index = key & (uint64_t)(GlobalPatterns_HashTableSize - 1);

    for (;;)
    {
        if (GlobalPatterns_HashTable_Key[index] == 0)
	{
	    GlobalPatterns_HashTable_Key[index] = key;
	    GlobalPatterns_HashTable_Gamma[index] = gamma;
	   
	    if ((A >= 10000 && W == 0) ||
	        (A >= 10000 && (double)W / (double)A <= 0.0001))
	    {
		GlobalPatterns_HashTable_BadPattern[index] = true;
	        GlobalPatterns_BadPatternCount++;
	    }
	    return true;
	}
	else if (GlobalPatterns_HashTable_Key[index] == key)
	{
	    return false;
	}

	if (index < LocalPatterns_HashTableSize - 1)
	    index++;
	else
	{
	    if (TableIsFull == false)
		TableIsFull = true;
	    else
	    {
		LogInfo() << "GlobalPatterns_InsertHashTable(): TableIsFull\n";
		return false;
	    }
            index = 0;
	}
    }
    return false;
}

void GlobalPatterns_ReadPatterns()
{
    GlobalPatterns_InitializeHashTable();

    std::ifstream ins;
    std::string fname 
        = MiscUtil::OpenFile("mohex-global-pattern-gamma.txt", ins);
    FILE *stream = fopen(fname.c_str(),"r");
    if (stream == NULL)
    {
        LogWarning() << "Failed to read '" << fname << "'\n";
	return;
    }

    int GlobalPatterns_HashTableEntryCount = 0;
    double gamma;
    int A;
    int W;
    char temp[128];
    int pattern[100];
    int GlobalPatternsCount[100] = {0};

    fscanf(stream,"%d\n", &A);

    while (! feof(stream))
    {
        fscanf(stream,"%lf %d %d", &gamma, &W, &A);

	fscanf(stream,"%s\n", temp);
	
	int size = (int)strlen(temp);

	for (int i = 1; i <= size; i++)
	    pattern[i] = (int)temp[i - 1] -48;


 	GlobalPatternsCount[size]++;

	if (gamma > GlobalPatterns_LargestGamma)
	    GlobalPatterns_LargestGamma = gamma;
	if (gamma < GlobalPatterns_SmallestGamma)
	    GlobalPatterns_SmallestGamma = gamma;

	for (int i = 1; i <= 6; i++)
	{
	    if (GlobalPatterns_InsertHashTable
                (LocalPatterns_ComputeKey(size, pattern), gamma, A, W, pattern))
	        GlobalPatterns_HashTableEntryCount++;

	    for (int j = 1; j <= 3; j++)
	        PatternRotation(pattern);
	}
    }

    LogInfo() << "GlobalPatterns:\n";
    for (int i = 0; i < 100; i++)
    {
	if (GlobalPatternsCount[i] > 0)
	    LogInfo() << "size " << i << "         =  " 
                      << GlobalPatternsCount[i] << "\n";
    }
    LogInfo() << "GlobalPatterns_HashTableEntryCount  = " 
                << GlobalPatterns_HashTableEntryCount << "\n";
    LogInfo() << "GlobalPatterns_BadPatternCount      = " 
                << GlobalPatterns_BadPatternCount << "\n";
    LogInfo() << "GlobalPatterns_LargestGamma         = " 
                << GlobalPatterns_LargestGamma << "\n";
    LogInfo() << "GlobalPatterns_SmallestGamma        = " 
                << GlobalPatterns_SmallestGamma << "\n";
    fclose(stream);
}


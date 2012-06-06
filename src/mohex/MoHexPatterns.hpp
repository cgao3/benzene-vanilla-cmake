//----------------------------------------------------------------------------
/** @file MoHexPatterns.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPATTERNS_HPP
#define MOHEXPATTERNS_HPP

#include "MoHexBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class MoHexPatterns
{
public:

    struct Statistics
    {
        size_t hit6, hit12, hit18;
        size_t miss6, miss12, miss18;
        Statistics() : hit6(0), hit12(0), hit18(0), 
                       miss6(0), miss12(0), miss18(0) { }
        std::string ToString() const;
    };

    struct Data
    {
        uint64_t key;
        float gamma;
        int type;
        int killer;
        int id;
    };

    struct Pattern
    {
        int size;
        int pattern[20];
        Pattern(int s, int p[20])
            : size(s)
        {
            memcpy(pattern, p, sizeof(pattern));
        }
    };

    static const size_t TABLE_SIZE = 1 << 19; // 512k slots
    static const size_t MAX_INDEX = 20;
    static uint64_t m_zobrist[MAX_INDEX][6];
    static uint64_t m_zobrist_size[5];

    MoHexPatterns();

    ~MoHexPatterns();

    static float DefaultGammaFunction(int type, float gamma);

    void ReadPatterns(std::string filename,
                      float (*GammaFunction)(int type, float gamma)
                      =&DefaultGammaFunction
                      );

    static void GetKeyFromBoard(uint64_t *key, const int size, 
                                const MoHexBoard& board, 
                                const HexPoint point, 
                                const HexColor toPlay);

    /** Computes pattern key centered on point and looks up paterrn.
        Stores pointer to pattern info in ret on a hit, stores NULL
        on a miss. */
    void Match(const MoHexBoard& board, int size, 
               HexPoint point, HexColor toPlay, const Data** ret) const;

    /** Uses pre-computed key to lookup pattern. */
    void MatchWithKeys(const uint64_t* keys, const int size,
                       const HexColor toPlay, const Data** ret) const;

    /** Faster version of MatchWithKeys(). */
    void MatchWithKeysBoth(const uint64_t* keys, const HexColor toPlay, 
                           const Data** ret) const;
   
    float GammaFromKeysBoth(const uint64_t* keys,
                            const HexColor toPlay) const;

    Statistics GetStatistics() const;

    static void InitializeZobrist();

private:
    static uint64_t RandomHash();
    static int Mirror(int loc);
    static void MirrorAndFlipPattern(int size, int pattern[], int* killer);
    static void RotatePattern(int size, int pattern[], int* killer);
    static uint64_t ComputeKey(int size, int pattern[]);

    std::vector<Data*> m_table;
    std::vector<Pattern> m_patterns;

    mutable Statistics m_stats;

    const Data* QueryHashtable(const Data* table, uint64_t key) const;
    bool InsertHashTable(Data* table, uint64_t key, float gamma, 
                         int type, int killer, int id); 

    static std::string ShowPattern6(const int p[], const int e[]);
    static std::string ShowPattern12(const int p[], const int e[]);
    static std::string ShowPattern(int size, const int p[], const int e[]);
};

inline MoHexPatterns::Statistics MoHexPatterns::GetStatistics() const
{
    return m_stats;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPATTERNS_HPP

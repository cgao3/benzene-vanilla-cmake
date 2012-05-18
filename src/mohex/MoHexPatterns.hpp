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
    };

    MoHexPatterns();

    ~MoHexPatterns();

    void ReadPatterns(std::string filename);

    /** Returns gamma of pattern that matches. 
        If no pattern matches, return 1.0f. */
    float GetGammaFromBoard(const MoHexBoard& board, int size, 
                            HexPoint point, HexColor toPlay) const;

    /** Fills 'ret' with info of pattern that matches.
        Does not touch 'ret' if no pattern matches. If matching
        pattern has a killer, killer is mirrored if toPlay is
        WHITE. */
    void Match(const MoHexBoard& board, int size, 
               HexPoint point, HexColor toPlay, Data* ret) const;

    Statistics GetStatistics() const;

    static void InitializeDirection();

    static void InitializeZobrist();

private:
    static const size_t TABLE_SIZE = 1 << 20;
    static const size_t MAX_INDEX = 20;
    static uint64_t  m_zobrist[2][MAX_INDEX][6];
    static HexDirection m_direction[MAX_INDEX];

    static void Rotate(int pattern[], int* killer);
    static uint64_t ComputeKey(int size, int pattern[]);

    Data* m_table;

    mutable Statistics m_stats;

    void GetKeyFromBoard(uint64_t *key, const int size, 
                         const MoHexBoard& board, 
                         const HexPoint point, const HexColor toPlay) const;

    void GetKeyFromBoardBlackToPlay(uint64_t *key, const int size, 
                                    const MoHexBoard& board, 
                                    const HexPoint point) const;
    void GetKeyFromBoardWhiteToPlay(uint64_t *key, const int size, 
                                    const MoHexBoard& board, 
                                    const HexPoint point) const;
    void GetKeyFromBoardOld(uint64_t *key_6, uint64_t *key_12, 
                            uint64_t *key_18, const int size, 
                            const MoHexBoard& board, 
                            const HexPoint point, const HexColor toPlay) const;

    const Data* QueryHashtable(uint64_t key) const;
    bool InsertHashTable(uint64_t key, float gamma, int type, int killer); 

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

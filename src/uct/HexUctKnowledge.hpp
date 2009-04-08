//----------------------------------------------------------------------------
// $Id: HexUctKnowledge.hpp 1898 2009-02-05 02:01:04Z broderic $
//----------------------------------------------------------------------------

#ifndef HEXUCTKNOWLEDGE_H
#define HEXUCTKNOWLEDGE_H

#include "SgMove.h"
#include "SgUctSearch.h"

#include "HexUctState.hpp"

//----------------------------------------------------------------------------

class HexUctPriorKnowledge : public SgUctPriorKnowledge
{
public:
    // Constructor and Destructor.
    HexUctPriorKnowledge(const HexUctState& state);

    ~HexUctPriorKnowledge();
    
    // Loads patterns used for move analysis.
    void LoadPatterns(const std::string& config_dir);
    
    // Methods to analyze position and produce initial values based on
    // positive/negative attributes.
    void ProcessPosition(bool& deepenTree);

    void InitializeMove(SgMove move, float& value, float& count);

private:
    // Loads patterns used to determine if a response is good/bad.
    void LoadGoodPatterns(const std::string& filename);
    void LoadBadPatterns(const std::string& filename);
    
    // Access to UCT state.
    const HexUctState& m_state;
    
    // Pattern data to analyze move responses.
    std::vector<Pattern> m_good_patterns[BLACK_AND_WHITE];
    HashedPatternSet m_hash_good_patterns[BLACK_AND_WHITE];
    std::vector<Pattern> m_bad_patterns[BLACK_AND_WHITE];
    HashedPatternSet m_hash_bad_patterns[BLACK_AND_WHITE];
    
    // Indicates whether or not we use good/bad init patterns.
    bool m_use_good;
    bool m_use_bad;
    
    // Stores likely good/bad responses.
    bitset_t m_goodResponses;
    bitset_t m_badResponses;
};

//----------------------------------------------------------------------------

class HexUctPriorKnowledgeFactory : public SgUctPriorKnowledgeFactory
{
public:
    // Constructor and Destructor.
    HexUctPriorKnowledgeFactory(const std::string& config_dir);
    ~HexUctPriorKnowledgeFactory();
    
    // Creation of knowledge class used for node initialization.
    SgUctPriorKnowledge* Create(SgUctThreadState& state);

private:
    // Allows PriorKnowledge classes to find init pattern files.
    std::string m_config_dir;
};

//----------------------------------------------------------------------------

#endif // HEXUCTKNOWLEDGE_H

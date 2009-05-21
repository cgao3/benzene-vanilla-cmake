//----------------------------------------------------------------------------
/** @file HexUctKnowledge.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "HexUctKnowledge.hpp"
#include "PatternState.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexUctPriorKnowledge::HexUctPriorKnowledge(const HexUctState& state) 
    : m_state(state),
      m_use_good(false),
      m_use_bad(false)
{
}

HexUctPriorKnowledge::~HexUctPriorKnowledge()
{
}

void HexUctPriorKnowledge::ProcessPosition(bool& deepenTree)
{
    UNUSED(deepenTree);

    // Get previous move and current board state.
    HexPoint prevMove = m_state.GetLastMovePlayed();
    HexColor toPlay = m_state.GetColorToPlay();
    const PatternState& pastate = m_state.GetPatternState();
    
    // Among all cells, compute which moves are bad (dead/vulnerable)
    if (m_use_bad) 
    {
	m_badResponses = pastate.MatchOnBoard(pastate.Board().getEmpty(),
                                              m_hash_bad_patterns[toPlay]);
	//LogSevere() << "BAD:"
        //         << brd.Write(m_badResponses) << '\n';
    }
    
    // Among cells close to the last move played, determine which of
    // these are likely to be a strong response.
    if (m_use_good) 
    {
	m_goodResponses.reset();
	if (prevMove == INVALID_POINT) return;
	
	PatternHits hits;
	pastate.MatchOnCell(m_hash_good_patterns[toPlay], prevMove,
                            PatternState::MATCH_ALL, hits);
	for (unsigned i = 0; i < hits.size(); ++i) 
        {
	    HexPoint p = hits[i].moves1()[0];
	    m_goodResponses.set(p);
	}
	//LogSevere() << "GOOD:"
        //         << brd.Write(m_goodResponses) << '\n';
    }
}

void HexUctPriorKnowledge::InitializeMove(SgMove move, float& value,
                                          float& count)
{
    count = 0.0;
    if (move < 0 || move >= BITSETSIZE) {
	// If not a HexPoint, cannot analyze, so do not initialize.
	return;
    } else if (m_use_bad && m_badResponses.test(move)) {
	// If the move is bad, prevents its exploration.
	value = 0.0f;
	count = 11222;
    } else if (m_use_good && m_goodResponses.test(move)) {
	// If the response is likely to be good, ensure it is explored.
	value = 1.0f;
	count = 7;
    }
}

void HexUctPriorKnowledge::LoadPatterns(const std::string& config_dir)
{
    LoadGoodPatterns(config_dir + "mohex-init-good-patterns.txt");
    LoadBadPatterns(config_dir + "mohex-init-bad-patterns.txt");
}

void HexUctPriorKnowledge::LoadGoodPatterns(const std::string& filename)
{
    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromFile(filename.c_str(), patterns);
    LogInfo() << "HexUctPriorKnowledge: Read " << patterns.size()
	      << " patterns from '" << filename << "'." << '\n';
    
    // Can only load patterns once!
    HexAssert(m_good_patterns[BLACK].empty());
    
    for (std::size_t i = 0; i < patterns.size(); ++i) {
        Pattern p = patterns[i];
        
        switch(p.getType()) {
        case Pattern::MOHEX:
            m_good_patterns[BLACK].push_back(p);
            p.flipColors();
            m_good_patterns[WHITE].push_back(p);
            break;
        default:
            LogWarning() << "Pattern type = " << p.getType() << '\n';
            HexAssert(false);
        }
    }
    
    // create the hashed pattern sets for fast checking
    for (BWIterator color; color; ++color) {
        m_hash_good_patterns[*color].hash(m_good_patterns[*color]);
    }
}

void HexUctPriorKnowledge::LoadBadPatterns(const std::string& filename)
{
    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromFile(filename.c_str(), patterns);
    LogInfo() << "HexUctPriorKnowledge: Read " << patterns.size()
	      << " patterns from '" << filename << "'." << '\n';
    
    // Can only load patterns once!
    HexAssert(m_bad_patterns[BLACK].empty());
    
    for (std::size_t i=0; i < patterns.size(); ++i) {
        Pattern p = patterns[i];
        
        switch(p.getType()) {
        case Pattern::MOHEX:
            m_bad_patterns[BLACK].push_back(p);
            p.flipColors();
            m_bad_patterns[WHITE].push_back(p);
            break;
        default:
            LogWarning() << "Pattern type = " << p.getType() << '\n';
            HexAssert(false);
        }
    }
    
    // create the hashed pattern sets for fast checking
    for (BWIterator color; color; ++color) {
        m_hash_bad_patterns[*color].hash(m_bad_patterns[*color]);
    }
}

//----------------------------------------------------------------------------

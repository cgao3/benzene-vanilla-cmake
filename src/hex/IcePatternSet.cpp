//----------------------------------------------------------------------------
/** @file IcePatternSet.cpp */
//----------------------------------------------------------------------------

#include "IcePatternSet.hpp"
#include "Misc.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

IcePatternSet::IcePatternSet()
{
}

IcePatternSet::~IcePatternSet()
{
}

void IcePatternSet::LoadPatterns(std::string name)
{
    std::ifstream inFile;
    try {
        std::string file = MiscUtil::OpenFile(name, inFile);
        LogConfig() << "IcePatternSet: reading from '" << file << "'.\n";
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "IcePatternSet: " << e.what();
    }
    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromStream(inFile, patterns);
    LogConfig() << "IcePatternSet: parsed " << patterns.size() 
                << " patterns.\n";
    for (std::size_t i = 0; i < patterns.size(); i++) 
    {
        Pattern p(patterns[i]);
        switch(p.GetType()) {
        case Pattern::EITHER_FILLIN:
            m_e_fillin.push_back(p);
	    p.FlipColors();
	    m_e_fillin.push_back(p);
            break;

        case Pattern::WHITE_FILLIN:
            m_fillin[WHITE].push_back(p);
	    if (p.GetEmpty().size() == p.GetMoves1().size())
	        m_captured[WHITE].push_back(p);
            p.FlipColors();
            m_fillin[BLACK].push_back(p);
            if (p.GetEmpty().size() == p.GetMoves1().size())
	        m_captured[BLACK].push_back(p);
	    break;

        case Pattern::WHITE_STRONG_REVERSIBLE:
            m_s_reversible[WHITE].push_back(p);
	    if (p.GetEmpty().size() == 1) // the only one is the reverser
	        m_vulnerable[WHITE].push_back(p);
            p.FlipColors();
            m_s_reversible[BLACK].push_back(p);
	    if (p.GetEmpty().size() == 1)
	        m_vulnerable[BLACK].push_back(p);
            break;

	case Pattern::WHITE_THREAT_REVERSIBLE:
            m_t_reversible[WHITE].push_back(p);
            p.FlipColors();
            m_t_reversible[BLACK].push_back(p);
            break;

        case Pattern::WHITE_INFERIOR:
            m_inferior[WHITE].push_back(p);
            p.FlipColors();
            m_inferior[BLACK].push_back(p);
            break;

	case Pattern::WHITE_REVERSIBLE:
            m_reversible[WHITE].push_back(p);
            p.FlipColors();
            m_reversible[BLACK].push_back(p);
            break;

        default:
            throw BenzeneException() << "IcePatternSet: unknown pattern type " 
                                     << "'" << p.GetType() << "'\n";
        }
    }

    m_hashed_e_fillin.Hash(m_e_fillin);
    for (BWIterator it; it; ++it) 
    {
        m_hashed_fillin[*it].Hash(m_fillin[*it]);
        m_hashed_s_reversible[*it].Hash(m_s_reversible[*it]);
	m_hashed_t_reversible[*it].Hash(m_t_reversible[*it]);
        m_hashed_inferior[*it].Hash(m_inferior[*it]);
	m_hashed_captured[*it].Hash(m_captured[*it]);
	m_hashed_vulnerable[*it].Hash(m_vulnerable[*it]);
	m_hashed_reversible[*it].Hash(m_reversible[*it]);
    }
}

//----------------------------------------------------------------------------

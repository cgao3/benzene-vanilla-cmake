//----------------------------------------------------------------------------
/** @file IcePatternSet.cpp
 */
//----------------------------------------------------------------------------

#include "IcePatternSet.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

IcePatternSet::IcePatternSet()
{
}

IcePatternSet::~IcePatternSet()
{
}

void IcePatternSet::LoadPatterns(const boost::filesystem::path& file)
{
    boost::filesystem::path normalizedFile = file;
    normalizedFile.normalize();
    std::string nativeFile = normalizedFile.native_file_string();

    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromFile(nativeFile.c_str(), patterns);

    LogFine() << "IcePatternSet: "
	      << "Read " << patterns.size() << " patterns " 
	      << "from '" << nativeFile << "'." << '\n';

    for (std::size_t i = 0; i < patterns.size(); i++) 
    {
        Pattern p(patterns[i]);

        switch(p.getType()) {
        case Pattern::DEAD:
            m_dead.push_back(p);
            break;

        case Pattern::CAPTURED:
            // WHITE is first!!
            m_captured[WHITE].push_back(p);  
            p.flipColors();
            m_captured[BLACK].push_back(p);
            break;

        case Pattern::PERMANENTLY_INFERIOR:
            // WHITE is first!!
            m_permanently_inferior[WHITE].push_back(p);  
            p.flipColors();
            m_permanently_inferior[BLACK].push_back(p);
            break;

        case Pattern::VULNERABLE:
            m_vulnerable[BLACK].push_back(p);
            p.flipColors();
            m_vulnerable[WHITE].push_back(p);
            break;

        case Pattern::REVERSIBLE:
            m_reversible[BLACK].push_back(p);
            p.flipColors();
            m_reversible[WHITE].push_back(p);
            break;

        case Pattern::DOMINATED:
            m_dominated[BLACK].push_back(p);
            p.flipColors();
            m_dominated[WHITE].push_back(p);
            break;

        default:
            LogSevere() << "Pattern type = " << p.getType() << '\n';
            HexAssert(false);
        }
    }

    m_hashed_dead.hash(m_dead);
    for (BWIterator it; it; ++it) 
    {
        m_hashed_captured[*it].hash(m_captured[*it]);
        m_hashed_permanently_inferior[*it].hash(m_permanently_inferior[*it]);
        m_hashed_vulnerable[*it].hash(m_vulnerable[*it]);
        m_hashed_reversible[*it].hash(m_reversible[*it]);
        m_hashed_dominated[*it].hash(m_dominated[*it]);
    }
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file IcePatternSet.cpp */
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

        switch(p.GetType()) {
        case Pattern::DEAD:
            m_dead.push_back(p);
            break;

        case Pattern::CAPTURED:
            // WHITE is first!!
            m_captured[WHITE].push_back(p);  
            p.FlipColors();
            m_captured[BLACK].push_back(p);
            break;

        case Pattern::PERMANENTLY_INFERIOR:
            // WHITE is first!!
            m_permanently_inferior[WHITE].push_back(p);  
            p.FlipColors();
            m_permanently_inferior[BLACK].push_back(p);
            break;

        case Pattern::MUTUAL_FILLIN:
            // BLACK is first; should maybe be WHITE to match other fillin?
            m_mutual_fillin[BLACK].push_back(p);
            p.FlipColors();
            m_mutual_fillin[WHITE].push_back(p);
            break;

        case Pattern::VULNERABLE:
            m_vulnerable[BLACK].push_back(p);
            p.FlipColors();
            m_vulnerable[WHITE].push_back(p);
            break;

        case Pattern::REVERSIBLE:
            m_reversible[BLACK].push_back(p);
            p.FlipColors();
            m_reversible[WHITE].push_back(p);
            break;

        case Pattern::DOMINATED:
            m_dominated[BLACK].push_back(p);
            p.FlipColors();
            m_dominated[WHITE].push_back(p);
            break;

        default:
            LogSevere() << "Pattern type = " << p.GetType() << '\n';
            BenzeneAssert(false);
        }
    }

    m_hashed_dead.Hash(m_dead);
    for (BWIterator it; it; ++it) 
    {
        m_hashed_captured[*it].Hash(m_captured[*it]);
        m_hashed_permanently_inferior[*it].Hash(m_permanently_inferior[*it]);
        m_hashed_mutual_fillin[*it].Hash(m_mutual_fillin[*it]);
        m_hashed_vulnerable[*it].Hash(m_vulnerable[*it]);
        m_hashed_reversible[*it].Hash(m_reversible[*it]);
        m_hashed_dominated[*it].Hash(m_dominated[*it]);
    }
}

//----------------------------------------------------------------------------

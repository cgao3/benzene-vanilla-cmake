//----------------------------------------------------------------------------
/** @file CacheBook.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgHash.h"
#include "CacheBook.hpp"
#include "Misc.hpp"
#include <boost/filesystem/path.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

/** Loads cached opening moves from WolveCacheBook.txt in share directory. */
CacheBook::CacheBook()
{
    std::ifstream inFile;
    try {
        std::string file = MiscUtil::OpenFile("wolve-cache-book.txt", inFile);
        LogConfig() << "CacheBook: reading from '" << file << "'.\n";
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "CacheBook: " << e.what();
    }
    ParseFile(inFile);
    LogConfig() << "CacheBook: contains " << Size() << " entries.\n";
}

void CacheBook::ParseFile(std::ifstream& inFile)
{
    int size;
    std::string line;
    while (true) 
    {
        // Read line, and quit once end of file
        getline(inFile, line);
        if (inFile.eof())
            break;
        // Commented lines are ignored
        if (0 < line.size() && '#' == line[0])
            continue;
        // Parse line
        std::istringstream in(line);
        in >> size;
        std::vector<HexPoint> variation = ReadPoints(in);
        std::vector<HexPoint> moves = ReadPoints(in);
        if (0 == variation.size() || 1 != moves.size()) 
        {
            LogWarning() << "CacheBook: error parsing: '" << line << "'\n";
            continue;
        }
        // Add corresponding entry (if not redundant)
        HexState hs(size);
        for (size_t i = 0; i < variation.size(); ++i)
            hs.PlayMove(variation[i]);
        if (!Exists(hs))
            operator[](hs) = moves[0];
    }
}

std::vector<HexPoint> CacheBook::ReadPoints(std::istringstream& in) const
{
    std::vector<HexPoint> result;
    while (true) 
    {
        std::string s;
        in >> s;
        if (!in || s == "|")
            break;
        HexPoint p = HexPointUtil::FromString(s);
        if (INVALID_POINT == p) 
        {
            result.clear();
            return result;
        }
        result.push_back(p);
    }
    return result;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file CacheBook.cpp
*/
//----------------------------------------------------------------------------

#include "CacheBook.hpp"
#include "boost/filesystem/path.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Loads cached opening moves from WolveCacheBook.txt in share directory. */
CacheBook::CacheBook()
{
#ifdef ABS_TOP_SRCDIR
    boost::filesystem::path normalizedFile =
        boost::filesystem::path(ABS_TOP_SRCDIR)
        / "share" / "WolveCacheBook.txt";
    normalizedFile.normalize();
    std::string nativeFile = normalizedFile.native_file_string();
    std::ifstream f(nativeFile.c_str());
    if (!f.is_open())
        LogWarning() << "Could not open cache book file for reading: '"
                     << nativeFile << "'" << '\n';
    else
        ParseFile(f);
#else
    LogWarning() << "**** NO CACHE BOOK LOADED ***" << '\n';
#endif
}

void CacheBook::ParseFile(std::ifstream& inFile)
{
    int size;
    std::string line;
    while (!inFile.eof()) {
        getline(inFile, line);
        if (line.size() < 9)
            break;
        std::istringstream in(line);
        in >> size;
        std::vector<HexPoint> variation = ReadPoints(in);
        std::vector<HexPoint> moves = ReadPoints(in);
        
        // add entry
        HexState hs(size);
        for (size_t i = 0; i < variation.size(); ++i)
            hs.PlayMove(variation[i]);
        if (!Exists(hs))
            operator[](hs) = moves[0];
    }
    LogInfo() << "CacheBook: contains " << Size() << " entries.\n";
}

std::vector<HexPoint> CacheBook::ReadPoints(std::istringstream& in) const
{
    std::vector<HexPoint> result;
    while (true) {
        std::string s;
        in >> s;
        if (!in || s == "|")
            break;
        HexPoint p = HexPointUtil::FromString(s);
        if (INVALID_POINT != p)
            result.push_back(p);
    }
    return result;
}

//----------------------------------------------------------------------------

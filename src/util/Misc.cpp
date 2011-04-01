//----------------------------------------------------------------------------
/** @file Misc.cpp */
//----------------------------------------------------------------------------

#include "BenzeneException.hpp"
#include "Misc.hpp"
#include <fstream>
#include <boost/filesystem/path.hpp>

using namespace benzene;
using namespace boost::filesystem;

//----------------------------------------------------------------------------

namespace
{

/** Directory of executable.
    Can be set with MiscUtil::FindProgramDir() and is used in
    MiscUtil::OpenFile() */
path programDir;

}

//----------------------------------------------------------------------------

void MiscUtil::FindProgramDir(int argc, char* argv[])
{
    if (argc == 0 || argv == 0)
        return;
    programDir = path(argv[0], native).branch_path();
}

void MiscUtil::WordToBytes(unsigned word, byte* out)
{
    for (int i = 0; i < 4; i++) 
    {
        out[i] = static_cast<byte>(word & 0xff);
        word >>= 8;
    }
}

unsigned MiscUtil::BytesToWord(const byte* bytes)
{
    unsigned ret = 0;
    for (int i = 3; i >= 0; i--) 
    {
        ret <<= 8;
        ret |= bytes[i];
    }
    return ret;
}

int MiscUtil::NumBytesToHoldBits(int bits)
{
    return (bits + 7) / 8;
}

std::string MiscUtil::OpenFile(std::string name, std::ifstream& f)
{
#ifndef ABS_TOP_SRCDIR
    #error ABS_TOP_SRCDIR not defined!
#endif
#ifndef DATADIR
    #error ABS_TOP_SRCDIR not defined!
#endif
    std::string programDirFile;
    std::string absFile;
    std::string dataFile;
    {
        path p = programDir / name;
        p.normalize();
        programDirFile = p.native_file_string();
        f.open(programDirFile.c_str());
        if (f.is_open())
            return programDirFile;
    }
    {
        path p = boost::filesystem::path(ABS_TOP_SRCDIR) / "share" / name;
        p.normalize();
        absFile = p.native_file_string();
        f.open(absFile.c_str());
        if (f.is_open())
            return absFile;
    }
    {
        path p = boost::filesystem::path(DATADIR) / name;
        p.normalize();
        dataFile = p.native_file_string();
        f.open(dataFile.c_str());
        if (f.is_open())
            return dataFile;
    }
    throw BenzeneException() << "Could not find '" << name << "'. Tried \n"
                             << "\t'" << programDirFile << "' and\n"
                             << "\t'" << absFile << "' and\n"
                             << "\t'" << dataFile << "'.";
}

//----------------------------------------------------------------------------


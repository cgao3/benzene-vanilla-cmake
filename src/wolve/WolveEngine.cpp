//----------------------------------------------------------------------------
// $Id: WolveEngine.cpp 1888 2009-02-01 01:04:41Z broderic $
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "Misc.hpp"
#include "WolveEngine.hpp"
#include "WolvePlayer.hpp"

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

template<typename TYPE>
void ParseDashSeparatedString(const std::string& str, std::vector<TYPE>& out)
{
    // remove the '-' separators
    std::string widths(str);
    for (std::size_t i=0; i<widths.size(); ++i)
        if (widths[i] == '-') widths[i] = ' ';

    // parse the ' ' separated widths
    std::istringstream is;
    is.str(widths);
    while (is)
    {
        TYPE j;
        if (is >> j)
            out.push_back(j);
    }
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

WolveEngine::WolveEngine(std::istream& in, std::ostream& out,
                         int boardsize, BenzenePlayer& player)
    : BenzeneHtpEngine(in, out, boardsize, player)
{
    RegisterCmd("param_wolve", &WolveEngine::WolveParam);
}

WolveEngine::~WolveEngine()
{
}

//----------------------------------------------------------------------------

void WolveEngine::RegisterCmd(const std::string& name,
                              GtpCallback<WolveEngine>::Method method)
{
    Register(name, new GtpCallback<WolveEngine>(this, method));
}

//----------------------------------------------------------------------------

void WolveEngine::WolveParam(HtpCommand& cmd)
{
    
    WolvePlayer* wolve = BenzeneHtpEngine::GetInstanceOf<WolvePlayer>(&m_player);

    if (!wolve)
        throw HtpFailure("No Wolve instance!");
    WolveSearch& search = wolve->Search();

    if (cmd.NuArg() == 0) 
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << search.BackupIceInfo() << '\n'
            << "[bool] use_guifx "
            << search.GuiFx() << '\n'
            << "[bool] use_threads "
            << search.UseThreads() << '\n'
            << "[string] ply_width " 
            << MiscUtil::PrintVector(wolve->PlyWidth()) << '\n'
            << "[string] search_depths "
            << MiscUtil::PrintVector(wolve->SearchDepths()) << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            search.SetBackupIceInfo(cmd.BoolArg(1));
        else if (name == "ply_width")
        {
            std::vector<int> plywidth;
            ParseDashSeparatedString(cmd.Arg(1), plywidth);
            wolve->SetPlyWidth(plywidth);
        } 
        else if (name == "search_depths")
        {
            std::vector<int> depths;
            ParseDashSeparatedString(cmd.Arg(1), depths);
            wolve->SetSearchDepths(depths);
        }
        else if (name == "use_guifx")
            search.SetGuiFx(cmd.BoolArg(1));
        else if (name == "use_threads")
            search.SetUseThreads(cmd.BoolArg(1));
        else
            throw HtpFailure("Unknown argument!");
    }
}

//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void WolveEngine::InitPonder()
{
}

void WolveEngine::Ponder()
{
}

void WolveEngine::StopPonder()
{
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------

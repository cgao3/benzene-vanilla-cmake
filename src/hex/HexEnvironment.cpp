//----------------------------------------------------------------------------
/** @file HexEnvironment.cpp */
//----------------------------------------------------------------------------

#include <boost/format.hpp>
#include "HexEnvironment.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexEnvironment::HexEnvironment(int width, int height)
    : brd(0)
{
    brd.reset(new HexBoard(width, height, ice, buildParam));
}

void HexEnvironment::NewGame(int width, int height)
{
    if (brd->GetPosition().Width() != width 
        && brd->GetPosition().Height() != height)
    {
        /** @todo Make board resizable? Until then, make sure all
            HexBoard parameters are copied here! */
        bool use_vcs = brd->UseVCs();
        bool use_ice = brd->UseICE();
        bool use_dec = brd->UseDecompositions();
        bool backup  = brd->BackupIceInfo();
        brd.reset(new HexBoard(width, height, ice, buildParam));
        brd->SetUseVCs(use_vcs);
        brd->SetUseICE(use_ice);
        brd->SetUseDecompositions(use_dec);
        brd->SetBackupIceInfo(backup);
    }
    brd->GetPosition().StartNewGame();
}

HexBoard& HexEnvironment::SyncBoard(const StoneBoard& board)
{
    brd->GetPosition().SetPosition(board);
    return *brd.get();
}

//----------------------------------------------------------------------------

HexEnvironmentCommands::HexEnvironmentCommands(HexEnvironment& env)
    : m_env(env)
{
}

HexEnvironmentCommands::~HexEnvironmentCommands()
{
}

void HexEnvironmentCommands::Register(GtpEngine& e, const std::string name)
{
    Register(e, "param_"+ name+ "_ice", &HexEnvironmentCommands::ParamICE);
    Register(e, "param_"+ name+ "_vc", &HexEnvironmentCommands::ParamVC);
    Register(e, "param_"+ name+ "_board", &HexEnvironmentCommands::ParamBoard);
}

void HexEnvironmentCommands::Register(GtpEngine& engine, 
                                      const std::string& command,
                           GtpCallback<HexEnvironmentCommands>::Method method)
{
    engine.Register(command, 
                    new GtpCallback<HexEnvironmentCommands>(this, method));
}

void HexEnvironmentCommands::AddAnalyzeCommands(HtpCommand& cmd,
                                                const std::string& name)
{
    std::string capName(name);
    capName[0] = std::toupper(name[0]);
    cmd << (boost::format("param/%1% ICE Param/param_%2%_ice\n") 
            % capName % name);
    cmd << (boost::format("param/%1% VC Param/param_%2%_vc\n") 
            % capName % name);
    cmd << (boost::format("param/%1% Board Param/param_%2%_board\n") 
            % capName % name);
}

void HexEnvironmentCommands::ParamICE(HtpCommand& cmd)
{
    ICEngine& ice = m_env.ice; 
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] backup_opponent_dead "
            << ice.BackupOpponentDead() << '\n'
            << "[bool] find_all_pattern_dominators "
            << ice.FindAllPatternDominators() << '\n'
            << "[bool] find_all_pattern_killers "
            << ice.FindAllPatternKillers() << '\n'
            << "[bool] find_permanently_inferior "
            << ice.FindPermanentlyInferior() << '\n'
            << "[bool] find_presimplicial_pairs " 
            << ice.FindPresimplicialPairs() << '\n'
            << "[bool] find_three_sided_dead_regions "
            << ice.FindThreeSidedDeadRegions() << '\n'
            << "[bool] iterative_dead_regions "
            << ice.IterativeDeadRegions() << '\n'
            << "[bool] use_hand_coded_patterns "
            << ice.UseHandCodedPatterns() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_opponent_dead")
            ice.SetBackupOpponentDead(cmd.Arg<bool>(1));
        else if (name == "find_all_pattern_dominators")
            ice.SetFindAllPatternDominators(cmd.Arg<bool>(1));
        else if (name == "find_all_pattern_killers")
            ice.SetFindAllPatternKillers(cmd.Arg<bool>(1));
        else if (name == "find_permanently_inferior")
            ice.SetFindPermanentlyInferior(cmd.Arg<bool>(1));
        else if (name == "find_presimplicial_pairs")
            ice.SetFindPresimplicialPairs(cmd.Arg<bool>(1));
        else if (name == "find_three_sided_dead_regions")
            ice.SetFindThreeSidedDeadRegions(cmd.Arg<bool>(1));
        else if (name == "iterative_dead_regions")
            ice.SetIterativeDeadRegions(cmd.Arg<bool>(1));
        else if (name == "use_hand_coded_patterns")
            ice.SetUseHandCodedPatterns(cmd.Arg<bool>(1));
        else
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw HtpFailure() << "Expected 0 or 2 arguments";
}

void HexEnvironmentCommands::ParamVC(HtpCommand& cmd)
{
    HexBoard& brd = *m_env.brd; 
    VCBuilderParam& param = brd.Builder().Parameters();
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] abort_on_winning_connection "
            << param.abort_on_winning_connection << '\n'
            << "[bool] and_over_edge "
            << param.and_over_edge << '\n'
            << "[bool] use_greedy_union "
            << param.use_greedy_union << '\n'
            << "[bool] use_patterns "
            << param.use_patterns << '\n'
            << "[bool] use_non_edge_patterns "
            << param.use_non_edge_patterns << '\n'
            << "[string] max_ors "
            << param.max_ors << '\n'
            << "[string] softlimit_full "
            << brd.Cons(BLACK).SoftLimit(VC::FULL) << '\n'
            << "[string] softlimit_semi "
            << brd.Cons(BLACK).SoftLimit(VC::SEMI) << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "abort_on_winning_connection")
            param.abort_on_winning_connection = cmd.Arg<bool>(1);
        else if (name == "and_over_edge")
            param.and_over_edge = cmd.Arg<bool>(1);
        else if (name == "use_greedy_union")
            param.use_greedy_union = cmd.Arg<bool>(1);
        else if (name == "use_patterns")
            param.use_patterns = cmd.Arg<bool>(1);
        else if (name == "use_non_edge_patterns")
            param.use_non_edge_patterns = cmd.Arg<bool>(1);
        else if (name == "max_ors")
            param.max_ors = cmd.ArgMin<int>(1, 1);
        else if (name == "softlimit_full")
        {
            int limit = cmd.ArgMin<int>(1, 0);
            brd.Cons(BLACK).SetSoftLimit(VC::FULL, limit);
            brd.Cons(WHITE).SetSoftLimit(VC::FULL, limit);
        }
        else if (name == "softlimit_semi")
        {
            int limit = cmd.ArgMin<int>(1, 0);
            brd.Cons(BLACK).SetSoftLimit(VC::SEMI, limit);
            brd.Cons(WHITE).SetSoftLimit(VC::SEMI, limit);
        }
        else
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw HtpFailure() << "Expected 0 or 2 arguments";
}

void HexEnvironmentCommands::ParamBoard(HtpCommand& cmd)
{
    HexBoard& brd = *m_env.brd; 
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << brd.BackupIceInfo() << '\n'
            << "[bool] use_decompositions "
            << brd.UseDecompositions() << '\n'
            << "[bool] use_ice "
            << brd.UseICE() << '\n'
            << "[bool] use_vcs "
            << brd.UseVCs() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            brd.SetBackupIceInfo(cmd.Arg<bool>(1));
        else if (name == "use_decompositions")
            brd.SetUseDecompositions(cmd.Arg<bool>(1));
        else if (name == "use_ice")
            brd.SetUseICE(cmd.Arg<bool>(1));
        else if (name == "use_vcs")
            brd.SetUseVCs(cmd.Arg<bool>(1));
        else 
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw HtpFailure() << "Expected 0 or 2 arguments";
}

//----------------------------------------------------------------------------

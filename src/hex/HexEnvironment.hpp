//----------------------------------------------------------------------------
/** @file HexEnvironment.hpp */
//----------------------------------------------------------------------------

#ifndef HEXENVIRONMENT_HPP
#define HEXENVIRONMENT_HPP

#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Groups a HexBoard, ICEngine, and VCBuilderParam objects,
    which correspond to a set of parameters that can be changed.
*/
struct HexEnvironment
{
    HexEnvironment(int width, int height);

    void NewGame(int width, int height);

    HexBoard& SyncBoard(const StoneBoard& brd);

    ICEngine ice;

    VCBuilderParam buildParam;

    boost::scoped_ptr<HexBoard> brd;
    
};

//----------------------------------------------------------------------------

/** HTP Commands for an environment. */
class HexEnvironmentCommands
{
public:
    HexEnvironmentCommands(HexEnvironment& env);

    ~HexEnvironmentCommands();

    void Register(GtpEngine& engine, const std::string name);

    void AddAnalyzeCommands(HtpCommand& cmd, const std::string& name);

private:

    HexEnvironment& m_env;

    void ParamICE(HtpCommand& cmd);
    void ParamVC(HtpCommand& cmd);
    void ParamBoard(HtpCommand& board);

    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<HexEnvironmentCommands>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXENVIRONMENT_HPP

//----------------------------------------------------------------------------
/** @file CommonHtpEngine.hpp */
//----------------------------------------------------------------------------

#ifndef COMMONHTPENGINE_H
#define COMMONHTPENGINE_H

#include "BenzenePlayer.hpp"
#include "DfpnCommands.hpp"
#include "HexEnvironment.hpp"
#include "HexHtpEngine.hpp"
#include "DfsSolver.hpp"
#include "DfsCommands.hpp"
#include "SolverDB.hpp"
#include "VCCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** HTP engine with commands for stuff common to all UofA Hex
    players. */
class CommonHtpEngine: public HexHtpEngine
{
public:
    CommonHtpEngine(int boardsize); 
    
    ~CommonHtpEngine();

    /** @page benzenehtpenginecommands CommonHtpEngine Commands
        - @link CmdLicense() @c benzene-license @endlink
        - @link CmdGroupGet() @c group-get @endlink
        - @link CmdHandbookAdd() @c handbook-add @endlink
        - @link CmdComputeInferior() @c compute-inferior @endlink
        - @link CmdComputeFillin() @c compute-fillin @endlink
        - @link CmdComputeVulnerable() @c compute-vulnerable @endlink
        - @link CmdComputeReversible() @c compute-reversible @endlink
        - @link CmdComputeDominated() @c compute-dominated @endlink
        - @link CmdComputeDominatedOnCell() @c compute-dominated-cell @endlink
        - @link CmdFindCombDecomp() @c find-comb-decomp @endlink
        - @link CmdFindSplitDecomp() @c find-split-decomp @endlink
        - @link CmdEncodePattern() @c encode-pattern @endlink
        - @link CmdEvalTwoDist() @c eval-twod @endlink
        - @link CmdEvalResist() @c eval-resist @endlink
        - @link CmdEvalResistCells() @c eval-resist-cells @endlink
    */

    /** @name Command Callbacks */
    // @{
    // The callback functions are documented in the cpp file

    void CmdAnalyzeCommands(HtpCommand& cmd);
    void CmdLicense(HtpCommand& cmd);
    void CmdGroupGet(HtpCommand& cmd);
    void CmdHandbookAdd(HtpCommand& cmd);
    void CmdComputeInferior(HtpCommand& cmd);
    void CmdComputeFillin(HtpCommand& cmd);
    void CmdComputeVulnerable(HtpCommand& cmd);
    void CmdComputeReversible(HtpCommand& cmd);
    void CmdComputeDominated(HtpCommand& cmd);
    void CmdComputeDominatedOnCell(HtpCommand& cmd);
    void CmdFindCombDecomp(HtpCommand& cmd);
    void CmdFindSplitDecomp(HtpCommand& cmd);
    void CmdEncodePattern(HtpCommand& cmd);
    void CmdEvalTwoDist(HtpCommand& cmd);
    void CmdEvalResist(HtpCommand& cmd);
    void CmdEvalResistCells(HtpCommand& cmd);

    // @} // @name

protected:

    /** Player's environment. */
    HexEnvironment m_pe;

    /** Solver's environment. */
    HexEnvironment m_se;

    DfsSolver m_dfsSolver;

    DfpnSolver m_dfpnSolver;

    boost::scoped_ptr<DfsHashTable> m_dfsHashTable;

    boost::scoped_ptr<DfpnHashTable> m_dfpnHashTable;

    boost::scoped_ptr<DfsDB> m_dfsDB;

    boost::scoped_ptr<DfpnDB> m_dfpnDB;

    SolverDBParameters m_dfsParam;

    SolverDBParameters m_dfpnParam;

    SolverDB<DfsHashTable, DfsDB, DfsData> m_dfsPositions;

    SolverDB<DfpnHashTable, DfpnDB, DfpnData> m_dfpnPositions;

    HexEnvironmentCommands m_playerEnvCommands;

    HexEnvironmentCommands m_solverEnvCommands;

    VCCommands m_vcCommands;

    DfsCommands m_dfsSolverCommands;

    DfpnCommands m_dfpnSolverCommands;

    bool m_useParallelSolver;

    virtual void NewGame(int width, int height);
    
    void ParamPlayer(BenzenePlayer* player, HtpCommand& cmd);

private:

    void RegisterCmd(const std::string& name,
                     GtpCallback<CommonHtpEngine>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // COMMONHTPENGINE_HPP

//----------------------------------------------------------------------------
/** @file PerfectPlayer.hpp
 */
//----------------------------------------------------------------------------

#ifndef PERFECTPLAYER_HPP
#define PERFECTPLAYER_HPP

#include "DfsSolver.hpp"
#include "HexBoard.hpp"
#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Player using Solver to generate moves. Works best on boards 7x7
    and smaller.

    @note This player is currently not used!!
*/
class PerfectPlayer : public BenzenePlayer
{
public:

    explicit PerfectPlayer(DfsSolver* solver);

    virtual ~PerfectPlayer();
    
    /** Returns "perfect". */
    std::string Name() const;

    SolverDB* DB();

    void SetDB(SolverDB* db);
    
protected:

    /** Generates a move in the given gamestate using DfsSolver. */
    virtual HexPoint Search(HexBoard& brd, const Game& game_state,
			    HexColor color, const bitset_t& consider,
                            double max_time, double& score);

    DfsSolver* m_solver;

    SolverDB* m_db;

private:

    bool find_db_move(StoneBoard& brd, HexColor color, 
                      HexPoint& move_to_play, double& score) const;

    void solve_new_state(HexBoard& brd, HexColor color, 
                         HexPoint& move_to_play, double& score) const;
};

inline std::string PerfectPlayer::Name() const
{
    return "perfect";
}

inline SolverDB* PerfectPlayer::DB() 
{
    return m_db;
}

inline void PerfectPlayer::SetDB(SolverDB* db)
{
    m_db = db;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXSOLVERPLAYER_HPP

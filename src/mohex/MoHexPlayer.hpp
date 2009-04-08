//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef MOHEXPLAYER_HPP
#define MOHEXPLAYER_HPP

#include "BenzenePlayer.hpp"
#include "HexUctSearch.hpp"
#include "HexUctPolicy.hpp"

//----------------------------------------------------------------------------

/** Player using UCT to generate moves. */
class MoHexPlayer : public BenzenePlayer
{
public:

    /** Constructor. */
    MoHexPlayer();

    /** Destructor. */
    virtual ~MoHexPlayer();

    /** Returns "mohex". */
    std::string name() const;

    /** Returns the search. */
    HexUctSearch& Search();

    /** Returns the search. */
    const HexUctSearch& Search() const;

    /** Copy settings from other player. */
    void CopySettingsFrom(const MoHexPlayer& other);

    //-----------------------------------------------------------------------

    /** @name Parameters */
    // @{

    bool BackupIceInfo() const;

    void SetBackupIceInfo(bool enable);

    /** Max number of games to play. */
    int MaxGames() const;

    /** See MaxGames() */
    void SetMaxGames(int games);

    /** Maximum time to spend on search (in seconds). */
    double MaxTime() const;

    /** See MaxTime() */
    void SetMaxTime(double time);

    // @}

protected:

    /** Generates a move in the given gamestate using uct. */
    virtual HexPoint search(HexBoard& brd, const Game& game_state,
			    HexColor color, const bitset_t& consider,
                            double time_remaining, double& score);

    /** Does the search. */
    HexPoint Search(HexBoard& b, HexColor color, HexPoint lastMove,
                    const bitset_t& given_to_consider, 
                    double time_remaining, double& score);

    HexUctSharedPolicy m_shared_policy;
    
    HexUctSearch m_search;
   
    bool m_backup_ice_info;

    /** See MaxGames() */
    int m_max_games;

    /** See MaxTime() */
    double m_max_time;
};

inline std::string MoHexPlayer::name() const
{
    return "mohex";
}

inline HexUctSearch& MoHexPlayer::Search()
{
    return m_search;
}

inline const HexUctSearch& MoHexPlayer::Search() const
{
    return m_search;
}

inline bool MoHexPlayer::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void MoHexPlayer::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

inline int MoHexPlayer::MaxGames() const
{
    return m_max_games;
}

inline void MoHexPlayer::SetMaxGames(int games)
{
    m_max_games = games;
}

inline double MoHexPlayer::MaxTime() const
{
    return m_max_time;
}

inline void MoHexPlayer::SetMaxTime(double time)
{
    m_max_time = time;
}

//----------------------------------------------------------------------------

#endif // MOHEXPLAYER_HPP

//----------------------------------------------------------------------------
/** @file JYPlayer.hpp */
//----------------------------------------------------------------------------
#ifndef JYPLAYER_HPP
#define JYPLAYER_HPP

#include "BenzenePlayer.hpp"
#include "JYPattern.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** JY.
    Uses Jing Yang's pattern file to generate moves. */
class JYPlayer : public BenzenePlayer
{
public:
    explicit JYPlayer();

    virtual ~JYPlayer();
  
    /** Returns "JY". */
    std::string Name() const;

    std::string m_pattern_file;

    /** read lines in the pattern file into a vector of strings **/
    std::vector<std::string> m_vc_str;
    std::unordered_map<int, JYPattern> m_all_patterns;

    int m_boardsize;
    std::vector<JYPattern> m_cur_pattern_list;
    std::vector<std::vector<JYPattern> > m_prev_pattern_list_stack;
    bool m_is_rotate180;
    //-----------------------------------------------------------------------

    void LoadPatterns(std::string s);
    void SetPatternFile(std::string s);
    std::string GetPatternFile();

    void ProcessDecompose(JYPattern &pattern_to_decompose, int bn);
    HexPoint JYGenMove(HexPoint last_point);
    HexPoint JYSearch(const HexState& state, const Game& game);

private:
    void ParsePatterns();
    virtual HexPoint Search(const HexState& state, const Game& game,
                            HexBoard& brd, const bitset_t& consider,
                            double maxTime, double& score);



};

inline std::string JYPlayer::Name() const
{
    return "JYPattern Player";
}

inline void JYPlayer::SetPatternFile(std::string pattern_file){
    m_pattern_file=pattern_file;
}

inline std::string JYPlayer::GetPatternFile(){
    return m_pattern_file;
}

//----------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // JYPLAYER_HPP

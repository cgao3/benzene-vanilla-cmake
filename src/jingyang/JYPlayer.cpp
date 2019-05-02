//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchControl.h"
#include "SgSearchValue.h"

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "VCS.hpp"
#include "HexEval.hpp"
#include "Misc.hpp"
#include "SequenceHash.hpp"
#include "JYPlayer.hpp"

using namespace benzene;

    std::string str_strip(std::string &line){
        int j=line.size()-1;
        while(j>=0 && line[j]!='"') j--;
        assert(line[0]=='"' && line[j]=='"');
        std::string line2=line.substr(1, j-1);
        return line2;
    }

    std::vector<std::string> split(std::string str, char delimiter) {
        std::vector<std::string> ret;
        std::stringstream ss(str); // Turn the string into a stream.
        std::string tok;
        while(std::getline(ss, tok, delimiter)) {
            ret.push_back(tok);
        }
        return ret;
    }

    HexPoint RN1GlobalPoint(int local, int boardsize){
        int mid=boardsize*boardsize/2;
        int global=local;
        if(local>mid) global +=1; //because the center is occupied by black's first move
        //however, minus 1 is required since JY uses index starting from 1 not 0
        global -=1;
        char c1=global%boardsize + 'a', c2=global/boardsize+'1';
        std::string s;
        s.push_back(c1);
        s.push_back(c2);
        return HexPointUtil::FromString(s);

    }
    void move_next(std::vector<std::string> &vc_str, int &idx, std::vector<std::string> &tokens){
        tokens=split(vc_str[idx], ' ');
        idx++;
    }

    void print_tokens(std::vector<std::string> &toks){
        for(size_t i=0;i<toks.size();i++){
            std::cout<<toks[i]<<" ";
        }
        std::cout<<std::endl;
    }

JYPlayer::JYPlayer() : BenzenePlayer(), m_is_rotate180(false), m_boardsize(9)
{

}

JYPlayer::~JYPlayer()
{
}

void JYPlayer::LoadPatterns(std::string jy_pattern_file){
    m_pattern_file=jy_pattern_file;
    assert(m_pattern_file.size()>0);
    std::string line;
    std::ifstream infile(m_pattern_file.c_str());
    std::getline(infile, line);
    std::vector<std::string> tokens;
    tokens=split(line, ' ');
    if(line.find("BOARDSIZE")!=std::string::npos){
        m_boardsize=std::stoi(tokens[1]);
        LogInfo()<<"boardsize: "<<m_boardsize<<"\n";
    }
    while(std::getline(infile, line)){
        if (line.size()==0 || line[0]!='"')
            continue;
        m_vc_str.push_back(str_strip(line));
    }
    ParsePatterns();
}

//constructs patterns from text
void JYPlayer::ParsePatterns()
{
    std::string line;
    std::vector<std::string> tokens;
    size_t i, j, k;
    int idx = 0;
    int local, global;
    while (idx < m_vc_str.size())  {
        move_next(m_vc_str, idx, tokens);
        if (tokens[0] == "RN") {
            //a new pattern
            JYPattern pattern;
            int RN = std::stoi(tokens[1]);
            pattern.RN = RN;
            move_next(m_vc_str, idx, tokens); assert(tokens[0] == "BT");
            pattern.BT = std::stoi(tokens[1]);
            for (int bn = 0; bn < pattern.BT; bn++) {
                move_next(m_vc_str, idx, tokens); assert(tokens[0] == "BN");
                JYBranch branch;
                branch.BN=std::stoi(tokens[1]);
                //WM
                move_next(m_vc_str, idx, tokens); assert(tokens[0] == "WM");
                std::vector<JYHexPoint> WM;
                for (j = 2; j < tokens.size(); j++) { // index 1 is total num WM, ignore
                    local = std::stoi(tokens[j]);
                    global=-1;
                    if(RN==1) {
                        global= static_cast<int>(RN1GlobalPoint(local, m_boardsize));
                        //LogInfo()<<"local: "<<local<<" global: "<<global<<"\n";
                    }
                    JYHexPoint wm_point(local, global);
                    WM.push_back(wm_point);
                }
                //BM
                move_next(m_vc_str, idx, tokens); assert(tokens[0] == "BM");
                local = std::stoi(tokens[1]);
                global=-1;
                if(RN==1) {
                    global= static_cast<int>(RN1GlobalPoint(local, m_boardsize));
                }
                JYHexPoint BM(local, global);
                //decompose
                move_next(m_vc_str, idx, tokens); assert(tokens[0] == "ND");
                JYDecompose decompose;
                int ND = std::stoi(tokens[1]);
                decompose.ND=ND;
                if (ND!=0) {
                    move_next(m_vc_str, idx, tokens); assert(tokens[0] == "PS");
                    std::vector<int> new_RNs;
                    for (j = 1; j < tokens.size(); j++) {
                        new_RNs.push_back(std::stoi(tokens[j]));
                    }
                    decompose.PS = new_RNs;
                    std::vector<std::vector<int> > PP_list;
                    for (k = 0; k < ND; k++) {
                        move_next(m_vc_str, idx, tokens);assert(tokens[0] == "PP");
                        std::vector<int> PP;
                        for (j = 1; j < tokens.size(); j++) {
                            PP.push_back(std::stoi(tokens[j]));
                        }
                        PP_list.push_back(PP);
                    }
                    decompose.PPs = PP_list;
                }
                branch.WM=WM;
                branch.BM=BM;
                branch.decompose=decompose;
                pattern.branchs.push_back(branch);
            }
            m_all_patterns[RN]=pattern;
        }
    }
    JYPattern p1=m_all_patterns[1];//starting from pattern 1.
    m_cur_pattern_list.push_back(p1);
    LogInfo()<<"Num of loaded patterns: "<<m_all_patterns.size()<<"\n";
    LogInfo()<<"Current pattern list:{";
    for (JYPattern &p: m_cur_pattern_list){
        LogInfo()<<p.RN<<",";
    }
    LogInfo()<<"}\n";
}

void JYPlayer::ProcessDecompose(JYPattern &pattern_to_decompose, int bn){
    JYPattern &pattern = pattern_to_decompose;
    LogInfo()<<"Pattern to decompose:"<<pattern.RN<<" BN:"<<bn<<" Decompos into:{";
    if (pattern.branchs[bn].decompose.ND == 0){
        LogInfo()<<" }\n";
        return;
    }
    std::unordered_map<int,int> local_to_global;
    if (pattern.RN==1){
        //since RN=1 contains only half empty cells on board
        int mid_point=m_boardsize*m_boardsize/2;
        for(int i=1;i<=m_boardsize*m_boardsize;i++){
            if(i==mid_point) continue;//center is occupied by first black move
            int local=i;
            if(i>mid_point) local=i-1;
            int global= static_cast<int>(RN1GlobalPoint(local, m_boardsize));
            local_to_global[local]=global;
        }
    } else{
        for(int i=0;i<pattern.BT;i++){
            for(size_t j=0;j<pattern.branchs[i].WM.size();j++){
                local_to_global[pattern.branchs[i].WM[j].m_local] = pattern.branchs[i].WM[j].m_global;
            }
        }
    }

    JYDecompose &decompose=pattern.branchs[bn].decompose;
    LogInfo()<<"\n";
    for(int i=0; i<decompose.ND; i++){
        for(size_t j=0;j<decompose.PPs[i].size();j++){
            LogInfo()<<decompose.PPs[i][j]<<" ";
        }
        LogInfo()<<"\n";
    }
    LogInfo()<<"}\n";
    for(int i=0;i<decompose.ND;i++){
        int ch_id=decompose.PPs[i][0];
        std::unordered_map<int,int> child_local_to_pa_local;
        for(size_t j=1;j<decompose.PPs[i].size();j++){
            int ch_local=j, pa_local=decompose.PPs[i][j];
            child_local_to_pa_local[ch_local]=pa_local;
        }
        JYPattern ch_pattern=m_all_patterns[ch_id];
        for(int j=0;j<ch_pattern.BT;j++){
            int l;
            JYBranch &branch=ch_pattern.branchs[j];
            for(size_t k=0;k<branch.WM.size();k++){
                l=branch.WM[k].m_local;
                branch.WM[k].m_global= local_to_global[child_local_to_pa_local[l]];
            }
            l=branch.BM.m_local;
            branch.BM.m_global=local_to_global[child_local_to_pa_local[l]];
        }
        m_cur_pattern_list.push_back(ch_pattern);
    }
}

HexPoint JYPlayer::JYGenMove(HexPoint last_point){
    //genmove black move by computer
    m_prev_pattern_list_stack.push_back(m_cur_pattern_list);
    JYPattern pattern;
    LogInfo()<<"last point:"<<last_point<<"\n";
    int white_point = static_cast<int>(last_point);
    for(size_t index=0;index<m_cur_pattern_list.size(); index++) {
        pattern = m_cur_pattern_list[index];
        for(int i=0;i<pattern.BT;i++){
            for(size_t j=0;j<pattern.branchs[i].WM.size();j++){
                //find the white move
                if(pattern.branchs[i].WM[j].m_global == white_point ){
                    JYHexPoint BM=pattern.branchs[i].BM;
                    std::vector<JYPattern>::iterator it=m_cur_pattern_list.begin() + index;
                    m_cur_pattern_list.erase(it);
                    ProcessDecompose(pattern, i);
                    return static_cast<HexPoint>(BM.m_global);
                }
            }
        }
    }
    //white move didn't find, so play a move in the working_patterns
    LogInfo()<<"Out of pattern_list coverage, select the first brach of the last pattern "<<pattern.RN<<"\n";
    int bn=0;
    JYHexPoint BM=pattern.branchs[bn].BM;
    m_cur_pattern_list.pop_back();
    ProcessDecompose(pattern, bn);
    return static_cast<HexPoint>(BM.m_global);
}

//---------------------------------------------------------------------
/** Generates a move. */
HexPoint JYPlayer::JYSearch(const HexState &state, const Game &game)
{
    int center=m_boardsize*m_boardsize/2;
    int x=center%m_boardsize, y=center/m_boardsize;
    char c1=x+'a', c2='1'+y;
    std::string s;
    s.push_back(c1);
    s.push_back(c2);
    HexPoint mid_point=HexPointUtil::FromString(s);;
    if(game.History().size()==0){
        return mid_point;
    }
    Move last_move=game.History().back();
    if(last_move.Color() == HexColor::BLACK){
        return BoardUtil::RandomEmptyCell(state.Position());
    }
    assert(last_move.Color() == HexColor::WHITE);
    HexPoint white_point=last_move.Point();
    if(game.History().size()==2 && last_move.Point()>mid_point){
        //after first white move
        m_is_rotate180=true;
    }

    if (m_is_rotate180){
        white_point=BoardUtil::Rotate(state.Position().Const(), white_point);
        LogInfo()<<"after rotation last move:"<<white_point<<"\n";
    }
    HexPoint ret_point=JYGenMove(white_point);
    if(m_is_rotate180){
        ret_point=BoardUtil::Rotate(state.Position().Const(), ret_point);
    }
    return ret_point;
}

HexPoint JYPlayer::Search(const HexState &state, const Game &game, HexBoard &brd, const bitset_t &consider,
                          double maxTime, double &score) {
    return BoardUtil::RandomEmptyCell(state.Position());
}
#include<vector>
#include<string>
#include<unordered_map>
#include<cassert>
#include<iostream>
#include<fstream>
#include<sstream>
#include<cassert>

#ifndef BENZENE_CMAKE_JYPATTERN_HPP
#define BENZENE_CMAKE_JYPATTERN_HPP

class JYHexPoint {
public:
    JYHexPoint(){

    }
    JYHexPoint(int local, int global){
        m_local=local;
        m_global=global;
    }
    int m_local; //[1, ]
    int m_global; //Hexpoint used in benzene
};

struct JYDecompose {
    int ND;//num of decompositions
    std::vector<int> PS; //each newly produced pattern
    std::vector<std::vector<int> > PPs; //each new pattern contains a subset of parent pattern moves
};

struct JYBranch {
    int BN; //branch no.
    std::vector<JYHexPoint> WM; //all white's possible move
    JYHexPoint BM; //black's counter move
    JYDecompose decompose;
};

struct JYPattern{
    int RN; //Rule no. i.e. Pattern No.
    int BT; //brach total
    std::vector<JYBranch> branchs;
};

std::string str_strip(std::string &line);

std::vector<std::string> split(std::string str, char delimiter);

void move_next(std::vector<std::string> &vc_str, int &idx, std::vector<std::string> &tokens);

void print_tokens(std::vector<std::string> &toks);

benzene::HexPoint RN1GlobalPoint(int local, int boardsize);

#endif //BENZENE_CMAKE_JYPATTERN_HPP

#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>
#include<unordered_map>
#include<queue>
#include<set>
#include<map>
#include<stack>
#include<cassert>
#include<algorithm>

int BOARDSIZE = 9;

using namespace std;




string strip(string &line){
    int j=line.size()-1;
    while(j>=0 && line[j]!='"') j--;
    assert(line[0]=='"' && line[j]=='"');
    string line2=line.substr(1, j-1);
    return line2;
}

vector<string> split(string str, char delimiter) {
    vector<string> ret;
    stringstream ss(str); // Turn the string into a stream.
    string tok;
    while(getline(ss, tok, delimiter)) {
        ret.push_back(tok);
    }
    return ret;
}

int cell_to_point(string move){
    int x=move[0]-'a';
    int y=move[1]-'1';
    return y*BOARDSIZE + x; //point integer starts from 0
}

int reflect_point(int point){
    return BOARDSIZE*BOARDSIZE-1-point;
}

string point_to_cell(int point){
    string ret;
    int x,y;
    x=point%BOARDSIZE;
    y=point/BOARDSIZE;
    char m1='a'+x;
    char m2='1'+y;
    ret.push_back(m1);
    ret.push_back(m2);
    return ret;
}

string point_to_benzene_cell(int point){
    //in benzene local point 1 refers to a1 but in this program local 1 refers to b1.
    //This functions outputs a cell normalized to benzene
    return point_to_cell(point - 1);
}

bool empty(vector<string>&board, int point){
    int x,y;
    x=point%BOARDSIZE;
    y=point/BOARDSIZE;   
    return board[y][x] == '.';
}

struct PMOVE {
    int local_move; //[1, ]
    int global_move;//in the original board, i.e., in range [0, boardsize**2)
};

struct Pattern {
    int RN;//rule number, i.e., pattern index
    int BT;//branch total
    vector<int> vc_BN;
    vector< vector<PMOVE> > vc_WMs; //for each branch, white move list
    vector<PMOVE> vc_BM;
    //for each branch, for any move in white move list, black's counter move
    //Note the pair means local point move and its corresponding global point move
    //global point move means the move in the original board, i.e., in [0, boardsize*boardisze)
    vector<int> vc_ND; //for each branch, number of decompositions after white's play and black's response
    vector< vector<int> > vc_PSs;//for each branch, list of decomposed pattern IDs
    vector< vector< vector<int> > > vc_vc_PPs; //for each branch, for each decomposed pattern, moves to update
};

/*
 * print the current pattern list, which always covers the whole board
 * regions out of coveraged of those pattern list are not of interest, since
 * if white plays there, a random counter move can be responded by black
 */
std::string PrintCurrentPatternList(vector<string>&board, vector<Pattern>&working_patterns,bool reflect)
{

    //benzene local is 1 greater than main.cxx local
    std::ostringstream os;
    Pattern cur_pattern = working_patterns[working_patterns.size() - 1];
    if(cur_pattern.RN==1 && BOARDSIZE == 9){
        //since RN=1 contains only half empty cells on board
        int mid_point=(BOARDSIZE * BOARDSIZE)/2 ;
        for(int i=0;i<BOARDSIZE*BOARDSIZE;i++){
            if(i==mid_point) continue;//center is occupied by first black move
            int local= (i > mid_point) ? i-1 : i;
            string hexpoint = point_to_cell((i > mid_point) ? local + 1 : local);
            os <<' '<<hexpoint<<' '<<local + 1<<"@1";
        }
        return os.str();
    }

    
    Pattern pattern;
    for(int index=0;index<working_patterns.size(); index++) {
        pattern=working_patterns[index];
        for(int i=0;i<pattern.vc_WMs.size();i++){
            for(int j=1;j<pattern.vc_WMs[i].size();j++){
                int global = pattern.vc_WMs[i][j].global_move;
                int local = pattern.vc_WMs[i][j].local_move;
                string hexPoint= point_to_cell((reflect) ? reflect_point(global) : global) ;
                os << ' '<<hexPoint<<' '<<local<<'@'<<pattern.RN;
            }
        }
    }

    return os.str();
}

std::string PrintCurrentBranchList(vector<string>&board, vector<Pattern>&working_patterns,bool reflect)
{
    std::ostringstream os;
    Pattern pattern;
    for(int index = 0; index < working_patterns.size(); index++){
        pattern = working_patterns[index];
        for(int i=0;i<pattern.BT;i++){
            for(int j=1;j<pattern.vc_WMs[i].size();j++){
                int global = pattern.vc_WMs[i][j].global_move;
                int local = pattern.vc_WMs[i][j].local_move;
                string hexPoint= point_to_cell((reflect) ? reflect_point(global) : global) ;
                os << ' '<<hexPoint<<' ' <<pattern.RN << "@" <<pattern.vc_BN[i];
                if (pattern.RN == 1 && BOARDSIZE == 9){
                    hexPoint= point_to_cell((!reflect) ? reflect_point(global) : global) ;
                    os << ' '<<hexPoint<<' ' <<pattern.RN << "@" <<pattern.vc_BN[i];
                }
            }
        }
    }

    return os.str();
}


std::string PrintCurrentBlackMoves(vector<string>&board, vector<Pattern>&working_patterns,bool reflect)
{
    //Create mapping from black hexpoint to all white hexpoints s.t. the black hexpoint is a winning response to all white hexpoints.

    //First create the mapping to ensure it can be printed in order without repititions   
    std::map<int,std::vector<int>> bmToWM;
    std::ostringstream os;
    Pattern pattern;
    for(int index = 0; index < working_patterns.size(); index++){
        pattern = working_patterns[index];
        for(int i=0;i<pattern.BT;i++){
            for(int j=1;j<pattern.vc_WMs[i].size();j++){
                if(bmToWM.count(pattern.vc_BM[i].global_move) == 0){
                    bmToWM.insert(std::pair<int,std::vector<int>>(pattern.vc_BM[i].global_move, {pattern.vc_WMs[i][j].global_move}));
                }
                else{
                    bmToWM.at(pattern.vc_BM[i].global_move).push_back(pattern.vc_WMs[i][j].global_move);
                }
            }
        }
    }
    
    //Format is = bm wm,...,wm .... bm wm,....,wm

    for(auto const &kvp: bmToWM){
        string hexPoint= point_to_cell((reflect) ? reflect_point(kvp.first) : kvp.first) ;

        os << hexPoint;

        for(int i = 0; i < kvp.second.size(); i++){
            string hexPoint= point_to_cell((reflect) ? reflect_point(kvp.second[i]) : kvp.second[i]) ;
            os << ((i == 0) ? ' ' : ',') << hexPoint;
        }

        os << ' ';
        //Handle first move case
        if (working_patterns[working_patterns.size() - 1].RN == 1 && BOARDSIZE == 9){
            hexPoint=point_to_cell((!reflect) ? reflect_point(kvp.first) : kvp.first) ;
            os << hexPoint;
            for(int i = 0; i < kvp.second.size(); i++){
                string hexPoint = point_to_cell((!reflect) ? reflect_point(kvp.second[i]) : kvp.second[i]) ;                
                os << ((i == 0) ? ' ' : ',') << hexPoint;
            }
            os << ' ';
        }
    }

    return os.str();
}


void next(vector<string> &vc_str, int &idx, vector<string> &tokens){
    tokens=split(vc_str[idx], ' ');
    idx++;
}

PMOVE init_PMOVE(int m, int RN){
    PMOVE p;
    p.local_move=m;
    p.global_move=-1;//will calculate later
    if(RN==1){
        p.global_move=m;
        if(m<=BOARDSIZE*BOARDSIZE/2)
            p.global_move=m-1;
    }
    return p;
}

void print_tokens(vector<string> &toks){
    for(int i=0;i<toks.size();i++){
        cout<<toks[i]<<" ";
    }
    cout<<endl;
}

void parse_patterns(vector<string> &vc_str, unordered_map<int, Pattern> &patterns){
    string line;
    vector<string> tokens;
    int i,j,k;
    int idx=0;
    int m;
    while( idx < vc_str.size() ){
        next(vc_str, idx, tokens);
        if(tokens[0]=="RN"){
            //a new pattern
            Pattern pattern;
            pattern.RN=stoi(tokens[1]);
            int RN=pattern.RN;
            next(vc_str, idx, tokens);
            assert(tokens[0]=="BT");
            pattern.BT=stoi(tokens[1]);
            for(int bn=0;bn<pattern.BT;bn++) {
                next(vc_str, idx, tokens); assert(tokens[0] == "BN");
                pattern.vc_BN.push_back(stoi(tokens[1]));
                next(vc_str, idx, tokens); assert(tokens[0] == "WM");
                //print_tokens(tokens);
                vector<PMOVE> WMs;
                for (j = 1; j < tokens.size(); j++) {
                    m = stoi(tokens[j]);
                    WMs.push_back(init_PMOVE(m, RN));
                }
                pattern.vc_WMs.push_back(WMs);
                next(vc_str, idx, tokens); assert(tokens[0] == "BM");
                //print_tokens(tokens);
                m = stoi(tokens[1]);
                pattern.vc_BM.push_back(init_PMOVE(m, RN));
                next(vc_str, idx, tokens); assert(tokens[0] == "ND");
                int ND = stoi(tokens[1]);
                pattern.vc_ND.push_back(ND);
                //print_tokens(tokens);
                if (ND==0){
                    vector<int> PSs;
                    pattern.vc_PSs.push_back(PSs);
                    vector<vector<int>> vcs;
                    pattern.vc_vc_PPs.push_back(vcs);
                    continue;
                }
                next(vc_str, idx, tokens); assert(tokens[0] == "PS");
                vector<int> PSs;
                for (j = 1; j < tokens.size(); j++) {
                    m = stoi(tokens[j]);
                    PSs.push_back(m);
                }
                pattern.vc_PSs.push_back(PSs);
                vector<vector<int>> vcs;
                for (k = 0; k < ND; k++) {
                    next(vc_str, idx, tokens);
                    assert(tokens[0] == "PP");
                    vector<int> vc;
                    for (j = 1; j < tokens.size(); j++) {
                        m = stoi(tokens[j]);
                        vc.push_back(m);
                    }
                    vcs.push_back(vc);
                }
                pattern.vc_vc_PPs.push_back(vcs);
            }
            patterns[pattern.RN]=pattern;
        }

     }
}

void add_new_patterns(unordered_map<int, Pattern>& all_patterns, Pattern &cur_pattern, 
int bn, vector<Pattern>& working_patterns){
    int i,j,k;
    unordered_map<int,int> local_global;
    for(i=0;i<cur_pattern.vc_WMs.size();i++){
        for(j=1;j<cur_pattern.vc_WMs[i].size();j++){
            local_global[cur_pattern.vc_WMs[i][j].local_move]=cur_pattern.vc_WMs[i][j].global_move;
        }
    }
    if(cur_pattern.RN==1){
        for(i=1;i<=80;i++){
            if(i<=40)
                local_global[i]=i-1;
            else local_global[i]=i;
        }
    }
    cerr<<"cur pattern:"<<cur_pattern.RN<<" bn:"<<bn<<" decompos:"<<cur_pattern.vc_PSs[bn].size()<<endl;
    if (cur_pattern.vc_ND[bn]==0) return; //nothing to add
    cerr<<"\ndecomposed into: "<<cur_pattern.vc_vc_PPs[bn].size()<<"\n";
    for(i=0;i<cur_pattern.vc_vc_PPs[bn].size();i++){
        for(j=0;j<cur_pattern.vc_vc_PPs[bn][i].size();j++){
            cerr<<cur_pattern.vc_vc_PPs[bn][i][j]<<" ";
        }
        cerr<<endl;
    }
    int ND=cur_pattern.vc_vc_PPs[bn].size();
    for(i=0;i<ND;i++){
       int ch_id=cur_pattern.vc_vc_PPs[bn][i][0];
        unordered_map<int,int> ch_pa;
        for(j=1;j<cur_pattern.vc_vc_PPs[bn][i].size();j++){
           int m=cur_pattern.vc_vc_PPs[bn][i][j];
           ch_pa[j]=local_global[m];
       }
        Pattern p=(all_patterns[ch_id]);
        for(j=0;j<p.vc_WMs.size();j++){
            int l;
            for(k=1;k<p.vc_WMs[j].size();k++){
                l=p.vc_WMs[j][k].local_move;
                p.vc_WMs[j][k].global_move=ch_pa[l];
            }
            l=p.vc_BM[j].local_move;
            p.vc_BM[j].global_move=ch_pa[l];
        }
        working_patterns.push_back(p);
    }
}

string genmove(string white_move, vector<Pattern>& working_patterns, unordered_map<int, Pattern>& all_patterns){
    //genmove black move by computer
    //first transoform white's move into local move
    int white=cell_to_point(white_move);
    int i,j,k, index;
    int BM=-1;
    string black_move="";
    cerr<<"white move: "<<white_move<<" =>"<<white<<endl;
    Pattern pattern;
    for(index=0;index<working_patterns.size(); index++) {
        pattern=working_patterns[index];
        for(i=0;i<pattern.vc_WMs.size();i++){
            for(j=1;j<pattern.vc_WMs[i].size();j++){
                //find the white move
                //cerr<<pattern.vc_WMs[i][j].local_move<<"=>"<<pattern.vc_WMs[i][j].global_move<<"; ";
                if(pattern.vc_WMs[i][j].global_move==white){
                    BM=pattern.vc_BM[i].global_move;
                    add_new_patterns(all_patterns, pattern, i, working_patterns);
                    vector<Pattern>::iterator it=working_patterns.begin();
                    it +=index;
                    working_patterns.erase(it);
                    black_move=point_to_cell(BM);
                    return black_move;
                }
            }
        }
    }
    //white move didn't find, so play a move in the working_patterns
    cerr<<"no matching white move, select the last pattern "<<pattern.RN<<"\n";
    BM=pattern.vc_BM[0].global_move;
    working_patterns.pop_back();
    add_new_patterns(all_patterns, pattern,0, working_patterns);
    black_move=point_to_cell(BM);
    return black_move;
}

void showboard(const vector<string> &board){
    int boardsize=board.size();
    int i,j,k;
    for(i=0;i<boardsize;i++){
        char c='a'+i;
        cout<<' '<<c<<' ';
    }
    cout<<endl;
    for(i=0;i<=boardsize;i++){
        if(i>=1)
            cout<<' ';
        for(k=0;k<i-1;k++)
            cout<<' ';
        if(i==boardsize){
            cout<<' '<<' ';
            for(j=0;j<boardsize;j++){
                char c='a'+j;
                cout<<c<<' '<<' ';
            }
            cout<<endl;
            continue;
        }
        cout<<(i+1)<<"\\";
        for(j=0;j<boardsize;j++){
            if(j<=boardsize-2){
                char c=board[i][j];
                if(c>='a'&&c<='z') c=toupper(c);
                cout<<c<<' '<<' ';
            }
            if(j==boardsize-1){
                char c=board[i][j];
                if(c>='a'&&c<='z') c=toupper(c);
                cout<<c<<'\\'<<(i+1);
            }
        }
        cout<<endl;
    }
}



void play(vector<string> &board, char color, string move){
    int x=move[0]-'a';
    int y=move[1]-'1';
    board[y][x]=color;
}

void gtp_loop(vector<string>&board, unordered_map<int, Pattern> &all_patterns){
    string text;
    int mid_point = ((BOARDSIZE - 1) / 2 ) * BOARDSIZE + (BOARDSIZE / 2);

    play(board, 'b', point_to_cell(mid_point));
    int toplay=1;
    string white_move="";
    vector<Pattern> working_patterns;
    stack<vector<Pattern>> previous_working_patterns; 
    Pattern p = all_patterns[1];
    working_patterns.push_back(p);
    stack<int> history;
    bool reflect=false;
    bool can_reflect= (BOARDSIZE == 9);
    int point;
    while(true){
        getline(cin, text);
        transform(text.begin(), text.end(), text.begin(), ::tolower);
        if(text.find("quit")!=string::npos) break;
        if(text.find("genmove b")!=string::npos || text.find("genmove black")!=string::npos){
            if(working_patterns.size() == 0){
                cout << "= invalid\n" << endl;
                continue;
            }

            vector<string> tokens=split(text, ' ');
            previous_working_patterns.push(working_patterns);
            string black_move=genmove(white_move, working_patterns, all_patterns);            
            point=cell_to_point(black_move);
            history.push(point);
            if(reflect && can_reflect){
                point = reflect_point(point);
                black_move=point_to_cell(point);
            }            
            play(board, 'b', black_move);
            toplay=1-toplay;
            cerr<<"Reflect:"<<reflect<<" Patterns now:";
            for(auto &p2: working_patterns)
                cerr<<p2.RN<<" ";
            cerr<<endl;
            cout<<"= "<<black_move<<'\n'<<endl;
        }
        if(text.find("play ")!=string::npos){
            vector<string> tokens=split(text, ' ');
            if(tokens[1][0]=='B'||tokens[1][0]=='b'){
                cout<<"= \n"<<endl;
            }
            white_move=tokens[2];
            if(white_move.size()!=2){
                cerr<<"wrong move\n";
                continue;
            }
            point=cell_to_point(white_move);
            if(point<0 || point>BOARDSIZE*BOARDSIZE-1){
                cerr<<"move out of range\n";
                continue;
            }
            cerr << "white move: " << point << "\n" << endl;
            if(!empty(board,point)){
                cerr<<"occupied cell\n";
                continue;
            }
            play(board, tokens[1][0], white_move);
            if(history.size() == 0 && point>BOARDSIZE*BOARDSIZE/2){
                cerr << "reflecting\n";
                reflect=true && can_reflect;
            }
            
            if(reflect){
                point=reflect_point(point);//symmetric move
                white_move=point_to_cell(point);
            }
            history.push(point);
            toplay=1-toplay;
            cout<<"= \n"<<endl;
        }
        if(text.find("showboard")!=string::npos){
            cout<<"= \n"<<endl;
            int r=rand();
            cout<<' '<<std::hex<<r<<endl;
            showboard(board);
            cout<<endl;
        }
        if(text.find("version")!=string::npos){
            cout<<"= 1.0\n"<<endl;
        }
        if(text.find("name")!=string::npos){
            cout<<"= jingyang\n"<<endl;
        }
        if(text.find("hexgui-analyze_commands")!=string::npos){
            cout<<"= \n"<<endl;
        }
        if(text.find("boardsize")!=string::npos){
            cout<<"= \n"<<endl;
        }
        if(text.find("show_jypattern_list")!=string::npos){
            if(working_patterns.size() == 0){
                cout << "= \n" << endl;
                continue;
            }
            std::ostringstream os;
            os << "= " << PrintCurrentPatternList(board,working_patterns,reflect) << "\n"; 
            string out = os.str();
            cout << out << endl;
        }
        if(text.find("show_jybranch_list") != string::npos){
            if(working_patterns.size() == 0){
                cout << "= \n" << endl;
                continue;
            }
            cout << "= " << PrintCurrentBranchList(board,working_patterns,reflect) << "\n" << endl; 
        }
        if(text.find("show_jyblackmoves_list") != string::npos){
            if(working_patterns.size() == 0){
                cout << "= \n" << endl;
                continue;
            }
            cout << "= " << PrintCurrentBlackMoves(board,working_patterns,reflect) << "\n" << endl; 
        }
        if(text.find("clear_board") != string::npos){
            for(int i=0;i<BOARDSIZE;i++){
                for(int j=0;j<BOARDSIZE;j++){
                    board[i][j] = '.';
                }
            }            
            white_move="";
            working_patterns.clear();
            while(!previous_working_patterns.empty()){
                previous_working_patterns.pop();
            }
            Pattern p = all_patterns[1];
            working_patterns.push_back(p);
            while(!history.empty()){
                history.pop();
            }
            reflect=false;
            point = 0;
            play(board, 'b', point_to_cell(mid_point));
            toplay = 1;
            cout << "= \n" << endl;
        }
        if(text.find("undo") != string::npos){
            if(history.size() > 0){
                if(toplay==1){
                    working_patterns = previous_working_patterns.top();
                    previous_working_patterns.pop();
                    toplay = 0;
                }
                else if(toplay == 0){
                    toplay = 1;
                }
                string lastmove = point_to_cell((reflect) ? reflect_point(history.top()) : history.top());
                cerr << "Undoing: " << lastmove << "\n" << endl;
                play(board,'.',lastmove);
                history.pop();
                cerr << "size: " << history.size() << "\n" << endl;
                if(history.size() == 0 && reflect){
                    cerr << "Unreflecting\n" << endl;
                    reflect = false;
                }
            }
            cout << "= \n" << endl;
        }
    }
}

int main(int argc, char *argv[]){
    if (argc<2){
        cout<<"usage: ./program jingyang_pattern.txt board_dim"<<endl;
        return 1;
    }
    string file_name=string(argv[1]);
    string line;
    ifstream infile(file_name.c_str());
    vector<string> vc_str;
    while(getline(infile, line)){
        if (line[0] == '#'){
            BOARDSIZE = stoi(split(line,' ')[1]);
            cerr << "Found boardsize:" << BOARDSIZE << "\n" << endl;
        }

        if (line.size()==0 || line[0]!='"')
            continue;
        vc_str.push_back(strip(line));
    }
    cerr<<vc_str.size()<<" lines in total"<<endl;
    unordered_map<int, Pattern> patterns;
    parse_patterns(vc_str, patterns);
    vector<string> board;
    board.resize(BOARDSIZE);
    for(int i=0;i<BOARDSIZE;i++){
        for(int j=0;j<BOARDSIZE;j++){
            board[i].push_back('.');
        }
    }
    gtp_loop(board, patterns);
    return 0;
}

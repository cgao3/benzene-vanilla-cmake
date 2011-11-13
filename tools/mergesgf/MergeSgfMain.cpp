//----------------------------------------------------------------------------
/** @file MergeSgfMain.cpp

    (NOTE: The code in this file was originally written by Markus
    Enzenberger for Project Explorer. )

    Merge SGF files to a single tree.

    Usage:

    mergesgf [-output merged.sgf] game.sgf [...]

    Description:
    Statistics about the game results are computed and stored as a
    comment in the nodes.

    NEED TO DO THE FOLLOWING: Merges SGF files to a single tree. The
    game-moves are transformed into a normalized forms to merge
    rotated/mirrored openings into the same subtree.

    Options:
    -output Filename for the resulting merged SGF file (default merged.sgf)
    -help   Print help and exit
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include "SgCmdLineOpt.h"
#include "SgDebug.h"
#include "SgException.h"
#include "SgGameReader.h"
#include "SgInit.h"
#include "SgNode.h"
#include "SgPoint.h"
#include "SgStatistics.h"

#include "Hex.hpp"
#include "HexProp.hpp"
#include "HexSgUtil.hpp"

using namespace std;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

struct Node
{
    SgPoint m_move;

    SgStatisticsBase<float,size_t> m_blackWins;

    vector<Node*> m_children;

    Node();

    Node(SgPoint move);

    void DeleteTree();
};

Node::Node()
    : m_move(SG_NULLMOVE)
{
}

Node::Node(SgPoint move)
    : m_move(move)
{
}

void Node::DeleteTree()
{
    for (vector<Node*>::iterator it = m_children.begin();
         it != m_children.end(); ++it)
        (*it)->DeleteTree();
    delete this;
}

//----------------------------------------------------------------------------

/** Hold a tree and delete automatically when out of scope. */
struct TreeHolder
{
    SgNode* m_root;

    TreeHolder(SgNode* root);

    ~TreeHolder();
};

TreeHolder::TreeHolder(SgNode* root)
    : m_root(root)
{
}

TreeHolder::~TreeHolder()
{
    m_root->DeleteTree();
    m_root = 0;
}

//----------------------------------------------------------------------------

string g_output;

vector<string> g_files;

int g_boardSize = -1;

//----------------------------------------------------------------------------

void AddMoves(Node* root, const vector<SgPoint>& moves, bool blackWin);
bool GetBlackWin(const SgNode* node);
int GetBoardSize(const SgNode* node);
vector<SgPoint> GetMoves(const SgNode* node);
vector<SgPoint> Normalize(const vector<SgPoint>& moves);
string PointToSgfString(SgPoint p);

void AddFile(Node* root, const string& filename)
{
    SgDebug() << "Adding file " << filename << '\n';
    ifstream in(filename.c_str());
    if (! in)
        throw SgException("Could not read file");
    SgGameReader reader(in);
    TreeHolder tree(reader.ReadGame());
    const SgNode* gameRoot = tree.m_root;
    if (gameRoot == 0)
        throw SgException("No game in file");
    int boardSize = GetBoardSize(gameRoot);
    if (g_boardSize < 0)
        g_boardSize = boardSize;
    else if (boardSize != g_boardSize)
        throw SgException("Games have different board sizes");
    bool blackWin = GetBlackWin(gameRoot);
    vector<SgPoint> moves = GetMoves(gameRoot);
    // TODO: ACTUALLY DO THIS!!
    //moves = Normalize(moves);
    AddMoves(root, moves, blackWin);
}

void AddMoves(Node* root, const vector<SgPoint>& moves, bool blackWin)
{
    Node* node = root;
    node->m_blackWins.Add(blackWin);
    for (vector<SgPoint>::const_iterator it = moves.begin();
         it != moves.end(); ++it)
    {
        SgPoint move = *it;
        Node* child = 0;
        for (vector<Node*>::const_iterator it2 = node->m_children.begin();
             it2 != node->m_children.end(); ++it2)
            if ((*it2)->m_move == move)
            {
                child = *it2;
                break;
            }
        if (child == 0)
        {
            child = new Node(move);
            node->m_children.push_back(child);
        }
        child->m_blackWins.Add(blackWin);
        node = child;
    }
}

string BlackWinsString(const Node* node)
{
    ostringstream out;
    size_t count = node->m_blackWins.Count();
    float mean = node->m_blackWins.Mean();
    out << static_cast<int>(100 * mean) << "% (" << count << ')';
    return out.str();
}

vector<SgPoint> GetMoves(const SgNode* root)
{
    vector<SgPoint> moves;
    SgBlackWhite toPlay = SG_BLACK;
    const SgNode* node = root;
    while (true)
    {
        if (node->HasProp(SG_PROP_ADD_BLACK)
            || node->HasProp(SG_PROP_ADD_WHITE)
            || node->HasProp(SG_PROP_ADD_EMPTY))
            throw SgException("File must not contain setup properties");
        if (node->HasProp(SG_PROP_MOVE))
        {
            SgPropMove* prop =
                dynamic_cast<SgPropMove*>(node->Get(SG_PROP_MOVE));
            if (prop->Player() != toPlay)
                throw SgException("File contains non-alternating moves");
            moves.push_back(prop->Value());
            toPlay = SgOppBW(toPlay);
        }
        if (! node->HasSon())
            break;
        node = node->LeftMostSon();
    }
    return moves;
}

bool GetBlackWin(const SgNode* node)
{
    if (! node->HasProp(SG_PROP_RESULT))
        throw SgException ("File has no result property");
    SgPropText* prop = dynamic_cast<SgPropText*>(node->Get(SG_PROP_RESULT));
    string result = prop->Value();
    if (result.find("B+") != string::npos)
        return true;
    if (result.find("W+") != string::npos)
        return false;
    throw SgException("Unknown format of result property");
}

int GetBoardSize(const SgNode* node)
{
    if (! node->HasProp(SG_PROP_SIZE))
        return 19;
    SgPropInt* prop = dynamic_cast<SgPropInt*>(node->Get(SG_PROP_SIZE));
    return prop->Value();
}

string GetLabel(size_t i)
{
    if (i <= static_cast<size_t>('Z' - 'A'))
    {
        string result;
        result = static_cast<char>('A' + i);
        return result;
    }
    else
    {
        ostringstream result;
        result << i;
        return result.str();
    }
}

bool IsCountGreater(const Node* node1, const Node* node2)
{
    return (node1->m_blackWins.Count() > node2->m_blackWins.Count());
}

vector<SgPoint> Normalize(const vector<SgPoint>& moves)
{
    // TODO: Convert this function to Hex!!
    SG_ASSERT(g_boardSize > 0);
    vector<SgPoint> result(moves);
    for (int rot = 0; rot < 8; ++rot)
    {
        vector<SgPoint> rotated(moves);
        for (vector<SgPoint>::iterator it = rotated.begin();
             it != rotated.end(); ++it)
            *it = SgPointUtil::Rotate(rot, *it, g_boardSize);
        for (size_t i = 0; i < moves.size(); ++i)
        {
            if (rotated[i] < result[i])
                result = rotated;
            if (rotated[i] != result[i])
                break;
        }
    }
    return result;
}

void ParseOptions(int argc, char** argv)
{
    SgCmdLineOpt cmdLineOpt;
    vector<string> specs;
    specs.push_back("output:");
    specs.push_back("help");
    cmdLineOpt.Parse(argc, argv, specs);
    if (cmdLineOpt.Contains("help"))
    {
        cout <<
            "Usage: mergesgf [Options] game.sgf [...]\n"
            "Options:\n"
            "  -output  Filename for merged file (default merged.sgf)\n"
            "  -help    print usage and exit\n";
        exit(0);
    }
    g_output = cmdLineOpt.GetString("output", "merged.sgf");
    g_files = cmdLineOpt.GetArguments();
    if (g_files.size() == 0)
        throw SgException("No filename given");
}

string PointToSgfString(SgPoint p)
{
    SG_ASSERT(g_boardSize > 0);
    benzene::HexPoint hp 
        = benzene::HexSgUtil::SgPointToHexPoint(p, g_boardSize);
    return benzene::HexPointUtil::ToString(hp);
}

void SaveNode(ostream& out, const Node* node, SgBlackWhite toPlay,
              bool isRoot)
{
    SgPoint move = node->m_move;
    if (! isRoot)
        out << ";";
    if (move != SG_NULLMOVE)
    {
        out << (toPlay == SG_BLACK ? 'B' : 'W') << '['
            << PointToSgfString(move) << ']';
        toPlay = SgOppBW(toPlay);
    }
    vector<Node*> children = node->m_children;
    sort(children.begin(), children.end(), IsCountGreater);
    bool hasNonPassMoves =
        (children.size() > 1
         || (children.size() == 1 && children[0]->m_move != SG_PASS));
    if (hasNonPassMoves)
    {
        out << "LB";
        for (size_t i = 0; i < children.size(); ++i)
        {
            const Node* childNode = children[i];
            SgPoint childMove = childNode->m_move;
            if (childMove != SG_PASS)
                out << '[' << PointToSgfString(childMove) << ':'
                    << GetLabel(i) << "]";
        }
        out << '\n';
    }

    out << "C[" << BlackWinsString(node) << "\n\n";
    for (size_t i = 0; i < children.size(); ++i)
    {
        const Node* childNode = children[i];
        out << GetLabel(i) << " (" 
            << PointToSgfString(childNode->m_move) << "): "
            << BlackWinsString(childNode) << '\n';
    }
    out << "]\n";
    for (vector<Node*>::const_iterator it = children.begin();
         it != children.end(); ++it)
    {
        out << "(\n";
        SaveNode(out, *it, toPlay, false);
        out << ")\n";
    }
}

void SaveTree(const Node* root)
{
    ofstream out(g_output.c_str());
    out << "(;FF[4]SZ[" << g_boardSize << "]AP[mergesgf]\n";
    SaveNode(out, root, SG_BLACK, true);
    out << ")\n";
    if (! out)
        throw SgException("Write error");
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        ParseOptions(argc, argv);
        SgInit();
        benzene::HexProp::Init();
        Node* root = new Node();
        for (vector<string>::const_iterator it = g_files.begin();
             it != g_files.end(); ++it)
            AddFile(root, *it);
        SaveTree(root);
        root->DeleteTree();
        SgFini();
    }
    catch (const SgException& e)
    {
        SgDebug() << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------

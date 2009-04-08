//----------------------------------------------------------------------------
// $Id: HexSgUtil.cpp 1994 2009-04-06 00:57:12Z broderic $
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgNode.h"
#include "SgProp.h"
#include "SgGameWriter.h"

#include "Hex.hpp"
#include "HexProp.hpp"
#include "HexSgUtil.hpp"

//----------------------------------------------------------------------------

SgPoint HexSgUtil::HexPointToSgPoint(HexPoint p, int height)
{
    int c, r;
    HexPointUtil::pointToCoords(p, c, r);
    return SgPointUtil::Pt(1+c, height - r);
}


HexPoint HexSgUtil::SgPointToHexPoint(SgPoint p, int height)
{
    int c = SgPointUtil::Col(p);
    int r = SgPointUtil::Row(p);
    return HexPointUtil::coordsToPoint(c-1, height - r);
}


SgBlackWhite HexSgUtil::HexColorToSgColor(HexColor color)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    if (color == BLACK) return SG_BLACK;
    return SG_WHITE;
}


HexColor HexSgUtil::SgColorToHexColor(SgBlackWhite player)
{
    HexAssert(player == SG_BLACK || player == SG_WHITE);
    if (player == SG_BLACK) return BLACK;
    return WHITE;
}

SgList<SgPoint> HexSgUtil::BitsetToSgList(const bitset_t& b, int height)
{
    SgList<SgPoint> ret;
    std::vector<HexPoint> hp;
    BitsetUtil::BitsetToVector(b, hp);
    for (unsigned i=0; i<hp.size(); ++i) {
        ret.Append(HexSgUtil::HexPointToSgPoint(hp[i], height));
    }
    return ret;
}

void HexSgUtil::AddMoveToNode(SgNode* node, HexColor color, 
                              HexPoint cell, int height)
{
    SgPoint sgcell = HexSgUtil::HexPointToSgPoint(cell, height); 
    SgBlackWhite sgcolor = HexSgUtil::HexColorToSgColor(color);
    HexPropUtil::AddMoveProp(node, sgcell, sgcolor);
}

bool HexSgUtil::NodeHasSetupInfo(SgNode* node)
{
    if (node->HasProp(SG_PROP_ADD_BLACK)) return true;
    if (node->HasProp(SG_PROP_ADD_WHITE)) return true;
    if (node->HasProp(SG_PROP_ADD_EMPTY)) return true;
    if (node->HasProp(SG_PROP_PLAYER)) return true;
    return false;
}

void HexSgUtil::SetPositionInNode(SgNode* node, 
                                  const StoneBoard& brd, 
                                  HexColor color)
{
    int height = brd.height();
    SgList<SgPoint> blist = 
        HexSgUtil::BitsetToSgList(brd.getBlack()&brd.getCells(), height);
    SgList<SgPoint> wlist = 
        HexSgUtil::BitsetToSgList(brd.getWhite()&brd.getCells(), height);
    SgList<SgPoint> elist = 
        HexSgUtil::BitsetToSgList(brd.getEmpty()&brd.getCells(), height);

    SgPropPlayer *pprop = new SgPropPlayer(SG_PROP_PLAYER);
    SgPropAddStone *bprop = new SgPropAddStone(SG_PROP_ADD_BLACK);
    SgPropAddStone *wprop = new SgPropAddStone(SG_PROP_ADD_WHITE);
    SgPropAddStone *eprop = new SgPropAddStone(SG_PROP_ADD_EMPTY);

    pprop->SetValue(HexSgUtil::HexColorToSgColor(color));    
    bprop->SetValue(blist);
    wprop->SetValue(wlist);
    eprop->SetValue(elist);

    node->Add(pprop);
    node->Add(bprop);
    node->Add(wprop);
    node->Add(eprop);
}

void HexSgUtil::GetSetupPosition(const SgNode* node, 
                                 int height, 
                                 std::vector<HexPoint>& black,
                                 std::vector<HexPoint>& white,
                                 std::vector<HexPoint>& empty)
{
    black.clear();
    white.clear();
    empty.clear();

    if (node->HasProp(SG_PROP_ADD_BLACK)) {
        SgPropPointList* prop = (SgPropPointList*)node->Get(SG_PROP_ADD_BLACK);
        SgList<SgPoint>& lst = prop->Value();
        for (int i=1; i<=lst.Length(); ++i) {
            HexPoint cell = HexSgUtil::SgPointToHexPoint(lst[i], height);
            black.push_back(cell);
        }
    }

    if (node->HasProp(SG_PROP_ADD_WHITE)) {
        SgPropPointList* prop = (SgPropPointList*)node->Get(SG_PROP_ADD_WHITE);
        SgList<SgPoint>& lst = prop->Value();
        for (int i=1; i<=lst.Length(); ++i) {
            HexPoint cell = HexSgUtil::SgPointToHexPoint(lst[i], height);
            white.push_back(cell);
        }
    }

    if (node->HasProp(SG_PROP_ADD_EMPTY)) {
        SgPropPointList* prop = (SgPropPointList*)node->Get(SG_PROP_ADD_EMPTY);
        SgList<SgPoint>& lst = prop->Value();
        for (int i=1; i<=lst.Length(); ++i) {
            HexPoint cell = HexSgUtil::SgPointToHexPoint(lst[i], height);
            empty.push_back(cell);
        }
    }
}

void HexSgUtil::SetPositionInBoard(const SgNode* node, StoneBoard& brd)
{
    std::vector<HexPoint> black, white, empty;
    GetSetupPosition(node, brd.height(), black, white, empty);

    brd.startNewGame();
    for (unsigned i=0; i<black.size(); ++i) {
        brd.playMove(BLACK, black[i]);
    }

    for (unsigned i=0; i<white.size(); ++i) {
        brd.playMove(WHITE, white[i]);
    }
}


bool HexSgUtil::WriteSgf(SgNode* tree, 
                         const std::string& application,
                         const char* filename, 
                         int boardsize)
{

    // set the boardsize property
    tree->Add(new SgPropInt(SG_PROP_SIZE, boardsize));

    // @note 11 is the sgf gamenumber for Hex
    std::ofstream f(filename);
    if (f) {
        SgGameWriter sgwriter(f);
        sgwriter.WriteGame(*tree, true, 0, application, 11, boardsize);
        f.close();
    } else {
        LogWarning() << "Could not open '" << filename << "' "
                 << "for writing!" << '\n';
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------

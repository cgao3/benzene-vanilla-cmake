#include "SgSystem.h"
#include "SgRandom.h"

#include "config.h"
#include "sixplayer.h"
#include "circuit.h"
#include "misc.h"

#include <cassert>
#include <algorithm>
#include <cmath>

using std::endl;
using std::cout;

#if 0

static int vvv = 0;

static int countSemis(const Connector &c)
{
  int r = 0;
  const Connector::DualBatchMap &m = c.connections();
  Connector::DualBatchMap::const_iterator cur = m.begin();
  Connector::DualBatchMap::const_iterator end = m.end();
  while(cur != end) {
    r += (*cur).second.semiBatch().size();
    ++cur;
  }
  return r;
}

static int countConns(const Connector &c)
{
  int r = 0;
  const Connector::DualBatchMap &m = c.connections();
  Connector::DualBatchMap::const_iterator cur = m.begin();
  Connector::DualBatchMap::const_iterator end = m.end();
  while(cur != end) {
    r += (*cur).second.connBatch().size();
    ++cur;
  }
  return r;
}

static void print(const Batch &b)
{
  cout << "(Batch:";
  vector<Carrier> v;
  Batch::Iterator cur = b.begin();
  Batch::Iterator end = b.end();
  while(cur != end) {
    v.push_back(*cur);
    ++cur;
  }
//   sort(v.begin(), v.end());
  for(unsigned i = 0; i < v.size(); ++i) {
    cout << v[i];
    cout << " ";
  }
  cout << ")" << endl;
}

static void print(const DualBatch &db)
{
  cout << "(DualBatch :x " << *db.minGroup()
       << " :y " << *db.maxGroup() << endl;
  cout << " :conns ";
  print(db.connBatch());
  cout << " :semis ";
  print(db.semiBatch());
  cout << ")" << endl;
}

static void print(const Grouping &gi)
{
  int i;
  for(i = 0; i < gi.size(); i++) {
    const Group &g = *gi[i];
    cout << g;
    cout << endl;
  }
}

static void print(const Connector &c, bool dump = false)
{
  cout << "connWinner=" << c.connWinner() << endl;
  cout << "nConn=" << countConns(c)
       << ", nSemi=" << countSemis(c)
       << endl;
  if(dump) {
    cout << c.board();
    cout << c.grouping();
    //print(c._queue);
    const Connector::DualBatchMap &m = c.connections();
    Connector::DualBatchMap::const_iterator cur = m.begin();
    Connector::DualBatchMap::const_iterator end = m.end();
    while(cur != end) {
      print((*cur).second);
      ++cur;
    }
  }
}

class ColoredLimiter : public Connector::DualBatchLimiter
{
public:
  ColoredLimiter(Poi<DualBatchLimiter> emptyEmpty,
                 Poi<DualBatchLimiter> emptyColor,
                 Poi<DualBatchLimiter> colorColor)
    : _emptyEmpty(emptyEmpty),
      _emptyColor(emptyColor),
      _colorColor(colorColor)
  {
  }

  virtual void init(DualBatch &db)
  {
    if(db.minGroup()->mark() == HEX_MARK_EMPTY &&
       db.maxGroup()->mark() == HEX_MARK_EMPTY) {
      (*_emptyEmpty).init(db);
    } else if(db.minGroup()->mark() != HEX_MARK_EMPTY &&
              db.maxGroup()->mark() != HEX_MARK_EMPTY) {
      (*_colorColor).init(db);
    } else {
      (*_emptyColor).init(db);
    }
  }

  virtual std::string to_string() const
  {
    std::ostringstream os;
    os << "(ColoredLimiter "
       << (*_emptyEmpty).to_string() << " "
       << (*_emptyColor).to_string() << " "
       << (*_colorColor).to_string()
       << ")";
    return os.str();
  }

  virtual ~ColoredLimiter()
  {
  }
private:
  Poi<DualBatchLimiter> _emptyEmpty, _emptyColor, _colorColor;
};

static Poi<Connector::DualBatchLimiter>
createColoredLimiter(Poi<Connector::DualBatchLimiter> ee,
                     Poi<Connector::DualBatchLimiter> ec,
                     Poi<Connector::DualBatchLimiter> cc)
{
  return Poi<Connector::DualBatchLimiter>(new ColoredLimiter(ee, ec, cc));
}

#endif

static Poi<Connector::DualBatchLimiter>
createSoftLimiter(unsigned smc, unsigned hmc, unsigned sms, unsigned hms)
{
  return Poi<Connector::DualBatchLimiter>
    (new Connector::SoftLimiter(smc, hmc, sms, hms));
}

//
//
//

SixPlayer::TaskTracker::TaskTracker(SlicedTask *t, Connector *c,
                                    Connector **activeConnector)
{
  _task = t;
  _connector = c;
  _activeConnector = activeConnector;
}

void SixPlayer::TaskTracker::doSlice()
{
  if(_activeConnector)
    *_activeConnector = _connector;
  if(_task)
    _task->doSlice();
  if(_activeConnector)
    *_activeConnector = 0;
}

//
//
//


static const char *noSwapsFor11x11 = "A1 A5 A6 A7 A8 A9 A10 B6 B7 \
 J1 I1 H1 G1 F1 E1 D1 C1 \
 A4 B5 A3 B4 B1 B8 B9 B1";

static vector<HexField> parseFieldList(const char *s, const HexBoard &b)
{
  vector<HexField> r;
  for(int i = 0; i < (signed)strlen(s); i++) {
    if(!isspace(s[i])) {
      int x = tolower(s[i++]) - 'a';
      int y = atoi(s + i) - 1;
      r.push_back(b.coords2Field(x, y));
      for(;(i < (signed)strlen(s)) && (!isspace(s[i])); i++) {
      }
    }
  }
  return r;
}

static HexField getFieldForOnlyMove(const HexBoard &b)
{
  assert(b.nMark() == 1);
  for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b.size(); f++) {
    if(b.get(f) != HEX_MARK_EMPTY) {
      return f;
    }
  }
  assert(0);
  // suppress warning
  return 0;
}

static HexField mirrorField(HexField f, const HexBoard &b)
{
  int x, y;
  b.field2Coords(f, &x, &y);
  return b.coords2Field(b.xs() - x - 1, b.ys() - y - 1);
}

static double swapValueFor11x11(const HexBoard &b)
{
  assert((b.xs() == 11) && (b.ys() == 11));
  vector<HexField> noSwaps = parseFieldList(noSwapsFor11x11, b);
  HexField f = getFieldForOnlyMove(b);
  HexField g = mirrorField(f, b);
  HexField a2 = b.coords2Field(0, 1);
  HexField b1 = b.coords2Field(1, 0);
  HexField k1 = b.coords2Field(10, 0);
  if((f == a2)  || (g == a2)){
    return 0.2;
  } else if((f == k1) || (g == k1)) {
    return 0.5;
  } else if((f == b1) || (g == b1)) {
    return -0.1;
  } else if((find(noSwaps.begin(), noSwaps.end(), f) != noSwaps.end()) ||
            (find(noSwaps.begin(), noSwaps.end(), g) != noSwaps.end())) {
    return -1.0;
  } else {
    return 1.0;
  }
}

static double swapValue(const HexBoard &b)
{
  assert(b.nMark() == 1);
  if((b.xs() == 11) && (b.ys() == 11)) {
    return swapValueFor11x11(b);
  } else {
    HexField f = getFieldForOnlyMove(b);
    int x, y;
    b.field2Coords(f, &x, &y);
    int xDist = MIN(x, b.xs() - x - 1);
    int yDist = MIN(y, b.ys() - y - 1);
    int cMin = MIN(xDist, yDist);
    int cMax = MAX(xDist, yDist);
    if(cMin > 0) {
      return 1.0;
    }
    if(cMax == 0) {
      return -1.0;
    }
    return 0.;
  }
}

static HexMove makeSafeOpeningMoveFor11x11(const HexBoard &b, HexMark mark)
{
    int n = SgRandom::Global().Int(3);
  if(n == 0)
    return HexMove(mark, b.coords2Field(0, 1));
  else if(n == 1)
    return HexMove(mark, b.coords2Field(1, 0));
  else
    return HexMove(mark, b.coords2Field(0, 2));
}

static HexMove makeSafeOpeningMove(const HexBoard &b, HexMark mark)
{
  assert(b.nMark() == 0);
  if((b.xs() == 11) && (b.ys() == 11)) {
    return makeSafeOpeningMoveFor11x11(b, mark);
  } else {
    vector<HexMove> moves;
    for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b.size(); f++) {
      HexBoard b2(b);
      b2.set(f, mark);
      if(swapValue(b2) == 0.) {
        moves.push_back(HexMove(mark, f));
      }
    }
    return moves.front();
  }
}

static HexMove highestEnergy(HexMark mark, const Carrier &c,
                             const Grouping &g,
                             const Circuit &cond)
{
  DBG << "Winning Connection: " << c << endl;
  bool found = 0;
  double max = 0.;
  int maxField = -1;
  const vector<int> &fields = c.fields();
  for(unsigned i = 0; i < fields.size(); i++) {
    double e = cond.energy(g.groupIndex(g(fields[i])));
    if(!found || e > max) {
      found = 1;
      max = e;
      maxField = fields[i];
    }
  }
  return HexMove(mark, maxField);
}

SixPlayer::SixPlayer(Level level, bool allowResign, SlicedTask *task)
{
  _level = level;
  _task = task;
  _activeConnector = 0;
  _cancelMove = false;
  _thinking = false;
  _resignAllowed = allowResign;
}

void SixPlayer::init(const HexGame *g, HexMark yourMark)
{
  assert(!_thinking);
  _game = g;
  _myMark = yourMark;
  _vert = 0;
  _hori = 0;
  _cancelMove = false;
}

void SixPlayer::played(const HexMove &m)
{
  if(m.isNormal() || m.isSwap()) {
    if(!_vert.null())
      (*_vert).move(m, true, false);
    if(!_hori.null())
      (*_hori).move(m, true, false);
  }
}

void SixPlayer::cancelMove()
{
  if(_thinking) {
    DBG << "Cancelling move ..." << endl;
    _cancelMove = true;
    if(_activeConnector)
      _activeConnector->stop();
  }
}

void SixPlayer::allowResign(bool allow)
{
  _resignAllowed = allow;
}

double SixPlayer::evalPos(const Circuit &vertCond,
                          const Circuit &,
                          const Circuit &horiCond,
                          const Circuit &,
                          HexMark m)
{
  assert(m == HEX_MARK_VERT || m == HEX_MARK_HORI);
  double rb = vertCond.resistance();
  double rw = horiCond.resistance();
//   double orb= oldVertCond.resistance();
//   double orw = oldHoriCond.resistance();
  double r;
//   DBG << "MARK=" << m << endl;
//   DBG << "ORB=" << orb << " RB=" << rb << endl;
//   DBG << "ORW=" << orw << " RW=" << rw << endl;
//   DBG << " RB=" << rb << ", RW=" << rw << endl;
  if(m == HEX_MARK_VERT) {
    r = rw / rb;
  } else {
    r = rb / rw;
  }
  return log(r);
//   if(m == HEX_MARK_VERT) {
//     r = rw - rb;
//   } else {
//     r = rb - rw;
//   }
//   return r;
}

double SixPlayer::evalPotentialWinner(const Connector &vert,
                                      const Connector &hori,
                                      HexMark mark,
                                      HexMove *bestMove,
                                      unsigned depth)
{
  assert(vert.winner() == HEX_MARK_EMPTY &&
         hori.winner() == HEX_MARK_EMPTY);
  assert(vert.connWinner() == HEX_MARK_EMPTY ||
         hori.connWinner() == HEX_MARK_EMPTY);
  assert(vert.connWinner() != HEX_MARK_EMPTY ||
         hori.connWinner() != HEX_MARK_EMPTY);
  assert(vert.connWinner() == HEX_MARK_EMPTY ||
         vert.connWinner() == HEX_MARK_VERT);
  assert(hori.connWinner() == HEX_MARK_EMPTY ||
         hori.connWinner() == HEX_MARK_HORI);
  if(bestMove) {
    Carrier wc;
    Poi<Circuit> cond;
    const Grouping *grouping;
    if(vert.connWinner() == HEX_MARK_VERT) {
      wc = vert.winningConnCarrier();
      grouping = &vert.grouping();
      cond = new Circuit(vert, *_conductance);
      _nCond++;
    } else {
      wc = hori.winningConnCarrier();
      grouping = &hori.grouping();
      cond = new Circuit(hori, *_conductance);
      _nCond++;
    }
    *bestMove = highestEnergy(mark, wc, *grouping, *cond);
  }
  int lengthOfConn;
  if(vert.connWinner() == HEX_MARK_VERT) {
    lengthOfConn = vert.winningConnCarrier().size();
  } else {
    lengthOfConn = hori.winningConnCarrier().size();
  }
  if(vert.connWinner() == mark || hori.connWinner() == mark) {
    return 1000. * (10 - depth) - lengthOfConn;
  } else {
    return -1000. * (10 - depth) + lengthOfConn;
  }
}

void SixPlayer::generateMoves(const HexGame &game,
                              unsigned depth,
                              const Connector &vert,
                              const Circuit &vertCond,
                              const Connector &hori,
                              const Circuit &horiCond,
                              HexMark mark, vector<Move> *moves)
{
  const HexBoard &b = vert.board();
  const Connector &me = ((mark == HEX_MARK_VERT) ? vert : hori);
  const Connector &opp = ((mark == HEX_MARK_VERT) ? hori : vert);
  const Circuit &myCond = ((mark == HEX_MARK_VERT) ? vertCond : horiCond);
  const Circuit &oppCond = ((mark == HEX_MARK_VERT) ? horiCond : vertCond);
  const Grouping &myG = me.grouping();
  const Grouping &oppG = opp.grouping();
  HexMark semiWinner = opp.semiWinner();
  Carrier criticalPath;
  if(semiWinner != HEX_MARK_EMPTY) {
    criticalPath = opp.criticalPath();
    if(criticalPath.empty()) {
      criticalPath = opp.winningSemiCarrier();
    }
  }
  {
    HexMove swapMove = HexMove::createSwap(game.next());
    if(game.isValidMove(swapMove))
      (*moves).push_back(Move(swapMove));
  }
  for(HexField f = 0; f < b.size(); f++) {
    if((b.get(f) == HEX_MARK_EMPTY) &&
       !myG(f).null() && !myG.uselessFields().has(f)) {
      if((semiWinner == HEX_MARK_EMPTY) || criticalPath.has(f)) {
        HexMove m(mark, f);
        double myE = (myG(f).null() ?
                      0. : myCond.energy(myG.groupIndex(myG(f))));
        double oppE = (oppG(f).null() ?
                       0. : oppCond.energy(oppG.groupIndex(oppG(f))));
        (*moves).push_back(Move(m, myE + oppE));
      }
    }
  }
  if((*moves).empty()) {
    if(_resignAllowed || depth > 0) {
      (*moves).push_back(Move(HexMove::createResign(mark), 0));
    } else {
      for(HexField f = 0; f < b.size(); f++) {
        if(b.get(f) == HEX_MARK_EMPTY) {
          if((semiWinner == HEX_MARK_EMPTY) || criticalPath.has(f)) {
            (*moves).push_back(Move(HexMove(mark, f), 0));
            return;
          }
        }
      }
    }
  }
}

bool SixPlayer::tryToCut(const Connector &vert,
                         const Connector &hori,
                         HexMark mark,
                         HexMove *bestMove, double alpha,
                         unsigned depth,
                         double *value)
{
  if(vert.winner() != HEX_MARK_EMPTY || hori.winner() != HEX_MARK_EMPTY) {
    if(vert.winner() == mark || hori.winner() == mark) {
      *value = 1000. * (10 - depth);
    } else {
      *value = -1000. * (10 - depth);
    }
    return true;
  } else if(vert.connWinner() != HEX_MARK_EMPTY ||
            hori.connWinner() != HEX_MARK_EMPTY) {
    *value = evalPotentialWinner(vert, hori, mark, bestMove, depth);
    return true;
  } else if(bestMove == 0 &&
            ((vert.semiWinner() == mark || hori.semiWinner() == mark))) {
    // the player to move already has a winning semi connection
    int lengthOfConn;
    if(mark == HEX_MARK_VERT) {
      lengthOfConn = vert.winningSemiCarrier().size() - 1;
    } else {
      lengthOfConn = hori.winningSemiCarrier().size() - 1;
    }
    *value = 1000. * (10 - depth) - lengthOfConn;
    return true;
  } else if(1000. * (10 - (depth + 1)) < alpha) {
    // if the parent node has a win (this node doesn't) and 
    // we are already at the depth of that win,
    // then make a cut
    *value = -alpha;
    return true;
  } else {
    return false;
  }
}

void SixPlayer::move(const Connector &oldVert, const Connector &oldHori,
                     Move *m)
{
  assert((*m).vert.null() == (*m).hori.null());
  if((*m).vert.null()) {
    // FIXME: move for mark of m first and stop if it's a win
    (*m).vert = new Connector(oldVert);
    (*m).hori = new Connector(oldHori);
    TaskTracker vertTracker(_task, &*(*m).vert, &_activeConnector);
    TaskTracker horiTracker(_task, &*(*m).hori, &_activeConnector);
    (*(*m).vert).setTask(&vertTracker);
    (*(*m).hori).setTask(&horiTracker);
    if((*m).move.isSwap() || (*m).move.isNormal()) {
      if(!_cancelMove) {
        (*(*m).vert).move((*m).move);
        _nMove++;
        if(!_cancelMove) {
          (*(*m).hori).move((*m).move);
          _nMove++;
        }
      }
      (*(*m).vert).setTask(0);
      (*(*m).hori).setTask(0);
    }
  }
}

double SixPlayer::eval(const HexGame &game,
                       const Connector &vert, const Circuit &oldVertCond,
                       const Connector &hori, const Circuit &oldHoriCond,
                       HexMove *bestMove, double alpha, double beta,
                       const vector<unsigned> &widths, unsigned depth)
{
//   DBG << "A=" << alpha << "\tB=" << beta << std::endl;
  assert(alpha < beta);
  HexMark mark = game.next();
  double r;
  _nNode++;
  assert(vert.winner() == hori.winner());
  if(!tryToCut(vert, hori, mark, ((depth == 0) ? bestMove : 0),
               alpha, depth, &r)) {
    Circuit vertCond(vert, *_conductance);
    _nCond++;
    Circuit horiCond(hori, *_conductance);
    _nCond++;
    if(depth >= widths.size()) {
      r = evalPos(vertCond, oldVertCond, horiCond, oldHoriCond, mark);
      return r;
//     } else if((depth + 1 == widths.size()) &&
//               ((r = evalPos(vertCond, oldVertCond,
//                             horiCond, oldHoriCond, mark))
//                >= beta)) {
//       // we are already too good and it will only get better if we make a move
//       DBG << "QUICK CUT AT " << depth << std::endl;
//       return r;
    } else {
      vector<Move> moves;
      generateMoves(game, depth, vert, vertCond, hori, horiCond, mark, &moves);
      sort(moves.begin(), moves.end());
      if(depth == 0 && moves.size() == 1) {
        DBG << "Playing only move: ";
        *bestMove = moves[0].move;
        (*_game).printMove(DBG, moves[0].move);
        DBG << endl;
        return 0.;
      }
//       if(depth == 0) {
//         DBG << "PREORDERING ..." << endl;
//         if(moves.size() > widths[depth])
//           moves.resize(widths[depth]);
//         for(unsigned i = 0; i < moves.size(); i++) {
//           move(vert, hori, &moves[i]);
//           moves[i].value = evalPos(*moves[i].vert, oldVertCond,
//                                    *moves[i].hori, oldHoriCond,
//                                    mark);
//         }
//         DBG << "PREORDERING DONE" << endl;
//         sort(moves.begin(), moves.end());
//       }
      bool found = false;
      if(depth == 0 && !moves.empty())
        _candidateMove = moves[0].move;
      unsigned bestMovePos = 0;
      for(unsigned i = 0; i < moves.size() && i < widths[depth] &&
            alpha < beta; i++) {

        if(bestMove) {
          for(unsigned j = 0; j < depth; j++)
            DBG << "  ";
          (*_game).printMove(DBG, moves[i].move);
          DBG << endl;
        }

//         if(depth != 0)
        move(vert, hori, &moves[i]);
        if(_cancelMove)
          return 0.;
        double v;
        {
          HexMove bbb;
          HexGame newGame(game);
          newGame.play(moves[i].move, 0L);
//           print(*moves[i].vert, true);
//           print(*moves[i].hori, false);
          v = -eval(newGame,
                    *moves[i].vert, vertCond,
                    *moves[i].hori, horiCond,
                    &bbb, -beta, -alpha,
                    widths, depth + 1);
        }
        // we don't need the connectors anymore
        moves[i].vert = 0;
        moves[i].hori = 0;

        if(bestMove) {
          for(unsigned j = 0; j < depth; j++)
            DBG << "  ";
          DBG << "(";
          (*_game).printMove(DBG, moves[i].move);
          DBG << " " << v << ")" << endl;
        }

        if(!found) {
          found = true;
          *bestMove = moves[i].move;
          bestMovePos = i;
        }
        if(v > alpha) {
          alpha = v;
          *bestMove = moves[i].move;
          bestMovePos = i;

          if(bestMove) {
            *bestMove = moves[i].move;
            for(unsigned j = 0; j < depth; j++)
              DBG << "  ";
            DBG << "Best move=";
            (*_game).printMove(DBG, *bestMove);
            DBG << "(" << v << ")" << endl;
          }

          if(depth == 0)
            _candidateMove = moves[i].move;
        }
      }
      assert(found);
      DBG << "(depth " << depth << " " <<  "bestmovepos " << bestMovePos << ")"
          << endl;
      return alpha;
    }
  }
  return r;
}

pair<bool, HexMove> SixPlayer::play()
{
  assert(!_thinking);
  _thinking = true;
  _candidateMove = HexMove();
  pair<bool, HexMove> r;
  switch(_level) {
  case 0:
    r = beginnerPlay();
    break;
  case 1:
    r = intermediatePlay();
    break;
  case 2:
    r = advancedPlay();
    break;
  case 3:
    r = expertPlay();
    break;
  default:
    assert(0);
  }
  _thinking = false;
  if(_cancelMove) {
    // let's make sure that we don't return a move if we're asked to cancel it
    r = pair<bool, HexMove>(false, HexMove());
  }
  return r;
}

void
SixPlayer::updateConnectors(const Poi<Connector::DualBatchLimiter> &limiter,
                            unsigned mio, bool ue, bool ip)
{
  assert(_vert.null() == _hori.null());
  assert(_vert.null() ||
         ((*_vert).limiter() == (*_hori).limiter() &&
          (*_vert).maxInOrRule() == (*_hori).maxInOrRule() &&
          (*_vert).useEdgePivot() == (*_hori).useEdgePivot() &&
          (*_vert).includePivotInCarrier() ==
          (*_hori).includePivotInCarrier()));
  if(_vert.null() ||
     (*_vert).limiter() != limiter ||
     (*_vert).maxInOrRule() != mio || (*_vert).useEdgePivot() != ue ||
     (*_vert).includePivotInCarrier() != ip) {
    _vert = new Connector(limiter, mio, ue, ip);
    _hori = new Connector(limiter, mio, ue, ip);
    _vertTracker = TaskTracker(_task, &*_vert, &_activeConnector);
    _horiTracker = TaskTracker(_task, &*_hori, &_activeConnector);
    (*_vert).setTask(&_vertTracker);
    (*_hori).setTask(&_horiTracker);
    DBG << "Initilizing connectors ..." << endl;
    (*_vert).init(_game->board(), HEX_MARK_VERT, false);
    (*_hori).init(_game->board(), HEX_MARK_HORI, false);
  }
}

const int oneMinute = 60000;

long SixPlayer::usedTime()
{
  if(_myMark == HEX_MARK_VERT)
    return _game->vertClockTotal();
  else
    return _game->horiClockTotal();
}

long SixPlayer::remainingTime()
{
  return 30 * oneMinute - usedTime();
}

pair<bool, HexMove> SixPlayer::beginnerPlay()
{
  _conductance = new Circuit::DualBatchConductance(1.0, 1.0);
  int n = _game->board().nMark();
  vector<unsigned> widths;
  if(n < 4) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(MAXINT, MAXINT, 15, MAXINT);
    widths.push_back(10);
    return commonPlay(l, 4, true, false, widths);
  } else if(n < 18) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(15, MAXINT, 7, MAXINT);
    widths.push_back(4);
    widths.push_back(4);
    return commonPlay(l, 4, true, false, widths);
  } else {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(3, MAXINT, 7, MAXINT);
    widths.push_back(4);
    widths.push_back(4);
    return commonPlay(l, 3, true, false, widths);
  }
}

pair<bool, HexMove> SixPlayer::intermediatePlay()
{
  _conductance = new Circuit::DualBatchConductance(1.0, 2.0);
  int n = _game->board().nMark();
  vector<unsigned> widths;
  if(n < 4) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(MAXINT, MAXINT, 15, MAXINT);
    widths.push_back(20);
    return commonPlay(l, 4, true, false, widths);
  } else if(n < 18) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(15, MAXINT, 7, MAXINT);
    widths.push_back(8);
    widths.push_back(8);
    return commonPlay(l, 4, true, false, widths);
  } else {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(3, MAXINT, 7, MAXINT);
    widths.push_back(8);
    widths.push_back(8);
    return commonPlay(l, 3, true, false, widths);
  }
}

pair<bool, HexMove> SixPlayer::advancedPlay()
{
  _conductance = new Circuit::DualBatchConductance(1.0, 12.0);
  int n = _game->board().nMark();
  vector<unsigned> widths;
  if(n < 4) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(MAXINT, MAXINT, 15, MAXINT);
    widths.push_back(30);
    return commonPlay(l, 4, true, false, widths);
  } else if(n < 18) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(15, MAXINT, 7, MAXINT);
    widths.push_back(20);
    widths.push_back(15);
    return commonPlay(l, 4, true, false, widths);
  } else {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(3, MAXINT, 7, MAXINT);
    widths.push_back(20);
    widths.push_back(15);
    return commonPlay(l, 3, true, false, widths);
  }
}

pair<bool, HexMove> SixPlayer::expertPlay()
{
  int n = _game->board().nMark();
  vector<unsigned> widths;
  _conductance = new Circuit::DualBatchConductance(1.0, 48.0);
#ifdef OLYMPICS
  if (remainingTime() < 2 * oneMinute) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(MAXINT, MAXINT, 30, MAXINT);
    widths.push_back(10);
    widths.push_back(10);
    return commonPlay(l, 4, false, false, widths);
  } else if(remainingTime() < 8 * oneMinute) {
    static Poi<Connector::DualBatchLimiter> l =
      createSoftLimiter(7, MAXINT, 15, MAXINT);
    widths.push_back(15);
    widths.push_back(10);
    return commonPlay(l, 4, true, false, widths);
  } else
#endif
  {
    if(n < 4) {
      static Poi<Connector::DualBatchLimiter> l =
        createSoftLimiter(MAXINT, MAXINT, 50, MAXINT);
      widths.push_back(30);
      return commonPlay(l, 4, true, false, widths);
    } else if(n < 10) {
      static Poi<Connector::DualBatchLimiter> l =
        createSoftLimiter(MAXINT, MAXINT, 40, MAXINT);
      widths.push_back(20);
      widths.push_back(15);
      return commonPlay(l, 4, true, false, widths);
    } else {
      static Poi<Connector::DualBatchLimiter> l =
        createSoftLimiter(10, MAXINT, 25, MAXINT);
      widths.push_back(20);
      widths.push_back(15);
      return commonPlay(l, 4, true, false, widths);
    }
  }
}

pair<bool, HexMove>
SixPlayer::commonPlay(const Poi<Connector::DualBatchLimiter> &limiter,
                      unsigned mio, bool ue, bool ip,
                      const vector<unsigned> &widths)
{
  pair<bool, HexMove> r(false, HexMove());
  if(_cancelMove)
    return r;
  HexMove m;
  _nNode = _nMove = _nCond = 0;
  if(_game->swappable() && ((*_game).board().nMark() == 0)) {
    m = makeSafeOpeningMove((*_game).board(), _game->next());
  } else if(_game->swappable() && ((*_game).board().nMark() == 1) &&
            swapValue((*_game).board()) > 0.) {
    m = HexMove::createSwap(_myMark);
  } else {
    DBG << "Widths:";
    for(unsigned i = 0; i < widths.size(); i++) {
      DBG << " " << widths[i];
    }
    DBG << ", Connector: " << (*limiter) << "," << mio << "," << ue
        << "," << ip << " "
        << *_conductance << endl;
    updateConnectors(limiter, mio, ue, ip);
    if(_cancelMove)
      return r;
    (*_vert).calc();
    if(_cancelMove)
      return r;
    (*_hori).calc();
    if(_cancelMove)
      return r;
    Circuit vertCond(*_vert, *_conductance);
    _nCond++;
    Circuit horiCond(*_hori, *_conductance);
    _nCond++;
    double v = eval(*_game, *_vert, vertCond, *_hori, horiCond,
                    &m, -500, 500, widths, 0);
    if(_resignAllowed && v < -9000) {
      m = HexMove::createResign(m.mark());
    }
    DBG << "v=" << v << endl;
    if(_cancelMove)
      return r;
  }

  DBG << "nNode=" << _nNode << ",nMove=" << _nMove << ",nCond=" << _nCond
      << endl;
  DBG << "Playing: ";
  (*_game).printMove(DBG, m);
  DBG << endl;
  return pair<bool, HexMove>(true, m);
}

HexMove SixPlayer::candidateMove() const
{
  if(_thinking)
    return _candidateMove;
  else
    return HexMove();
}

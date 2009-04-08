#include "hexmatch.h"
#include "poi.h"
#include "misc.h"

#include <sys/time.h>
#include <cassert>

#include <algorithm>

HexMatch::HexMatch(const HexGame &g, const Poi<HexPlayer> &vert,
                   const Poi<HexPlayer> &hori)
  : _game(g),  _status(MATCH_OFF)
{
  setVerticalPlayer(vert);
  setHorizontalPlayer(hori);
  if(_game.winner() != HEX_MARK_EMPTY)
    _status = MATCH_FINISHED;
  resetClock();
  connect(&_timer, SIGNAL(timeout()), this, SLOT(timerDone()));
}

void HexMatch::setVerticalPlayer(const Poi<HexPlayer> &vert)
{
  if(_vert != vert) {
    _vert = vert;
    _vertNew = true;
    emit signalPlayerChange();
  }
}

void HexMatch::setHorizontalPlayer(const Poi<HexPlayer> &hori)
{
  if(_hori != hori) {
    _hori = hori;
    _horiNew = true;
    emit signalPlayerChange();
  }
}

const HexGame &HexMatch::game() const
{
  return _game;
}

long HexMatch::vertClockTotal() const
{
  long r = _game.vertClockTotal();
  if(_game.next() == HEX_MARK_VERT)
    r += _millisecondsMove;
  return r;
}

long HexMatch::horiClockTotal() const
{
  long r = _game.horiClockTotal();
  if(_game.next() == HEX_MARK_HORI)
    r += _millisecondsMove;
  return r;
}

bool HexMatch::doSome()
{
  assert(!_vert.null());
  assert(!_hori.null());
  assert(_game.winner() == HEX_MARK_EMPTY);
  assert(_status != MATCH_FINISHED);
  setStatus(MATCH_ON);
  if(_vertNew) {
    (*_vert).init(&_game, HEX_MARK_VERT);
    _vertNew = false;
  }
  if(_horiNew) {
    (*_hori).init(&_game, HEX_MARK_HORI);
    _horiNew = false;
  }
  pair<bool, HexMove> move;
  bool valid = false;
  while(!valid) {
    if(_game.next() == HEX_MARK_VERT) {
      move = (*_vert).play();
    } else {
      move = (*_hori).play();
    }
    valid = !move.first || _game.isValidMove(move.second);
  }
  if(move.first) {
    clockOff();
    DBG << "Playing: ";
    _game.printMove(DBG, move.second);
    DBG << std::endl;
    bool wasChanged = _game.isChanged();
    bool wasBranched = _game.isBranched();
    _game.play(move.second, _millisecondsMove);
    (*_vert).played(move.second);
    (*_hori).played(move.second);
    if(_game.winner() != HEX_MARK_EMPTY) {
      setStatus(MATCH_FINISHED);
    } else {
      resetClock();
      clockOn();
    }
    if(!wasChanged || (!wasBranched && _game.isBranched()))
      emit signalChangedGameStatus();
    emit signalClockChange();
    emit signalPositionChange();
  }
  return move.first;
}

void HexMatch::on()
{
  if(_status == MATCH_OFF)
    setStatus(MATCH_ON);
}

void HexMatch::off()
{
  if(_status == MATCH_ON)
    setStatus(MATCH_OFF);
}

HexMatch::Status HexMatch::status() const
{
  return _status;
}

void HexMatch::setStatus(Status status)
{
  if(_status != status) {
    _status = status;
    if(status == MATCH_ON)
      clockOn();
    else
      clockOff();
    emit signalStatusChange();
  }
}

void HexMatch::setChanged(bool isChanged)
{
  if(_game.isChanged() != isChanged) {
    _game.setChanged(isChanged);
    emit signalChangedGameStatus();
  }
}

void HexMatch::setBranched(bool isBranched)
{
  if(_game.isBranched() != isBranched) {
    _game.setBranched(isBranched);
    emit signalChangedGameStatus();
  }
}

bool HexMatch::canBack() const
{
  return _game.canBack();
}

void HexMatch::back()
{
  _game.back();
  _vertNew = true;
  _horiNew = true;
  setStatus(MATCH_OFF);
  resetClock();
  emit signalClockChange();
  emit signalPositionChange();
}

void HexMatch::backAll()
{
  while(_game.canBack()) {
    _game.back();
  }
  _vertNew = true;
  _horiNew = true;
  setStatus(MATCH_OFF);
  resetClock();
  emit signalClockChange();
  emit signalPositionChange();
}

bool HexMatch::canForward() const
{
  return _game.canForward();
}

void HexMatch::forward()
{
  _game.forward();
  _vertNew = true;
  _horiNew = true;
  if(_game.winner() == HEX_MARK_EMPTY)
    setStatus(MATCH_OFF);
  else
    setStatus(MATCH_FINISHED);
  resetClock();
  emit signalClockChange();
  emit signalPositionChange();
}

void HexMatch::forwardAll()
{
  while(_game.canForward()) {
    _game.forward();
  }
  _vertNew = true;
  _horiNew = true;
  if(_game.winner() == HEX_MARK_EMPTY)
    setStatus(MATCH_OFF);
  else
    setStatus(MATCH_FINISHED);
  resetClock();
  emit signalClockChange();
  emit signalPositionChange();
}

void HexMatch::resetClock()
{
  _millisecondsMove = 0;
}

void HexMatch::clockOn()
{
  assert(_status == MATCH_ON);
  _timer.start(250);
  _startClock = clock();
  gettimeofday(&_startTime, 0);
}

void HexMatch::clockOff()
{
  if(_timer.isActive()) {
    _timer.stop();
    struct timeval endTime;
    gettimeofday(&endTime, 0);
    _millisecondsMove += 1000 * (endTime.tv_sec - _startTime.tv_sec) +
      (endTime.tv_usec - _startTime.tv_usec) / 1000;
  }
}

void HexMatch::timerDone()
{
  clockOff();
  clockOn();
  emit signalClockChange();
}

ostream &operator <<(ostream &os, const HexMatch &b)
{
  os << b.game();
  return os;
}

#include "hexmatch.moc"

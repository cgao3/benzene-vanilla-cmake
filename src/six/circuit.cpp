#include "circuit.h"
#include "lssolve.h"

#include <cassert>
#include <cmath>

Circuit::DualBatchConductance::DualBatchConductance(double emptyEmpty,
                                                    double emptyColor)
  : _emptyEmpty(emptyEmpty), _emptyColor(emptyColor)
{
}

double Circuit::DualBatchConductance::conductance(const DualBatch &db) const
{
  if(db.connBatch().empty()) {
    return 0.;
//   } else if((db.minGroup()->edge() &&
//              (db.maxGroup()->mark() == HEX_MARK_EMPTY)) ||
//             (db.maxGroup()->edge() &&
//              (db.minGroup()->mark() == HEX_MARK_EMPTY))) {
//     return _emptyColor;
  } else if((db.minGroup()->mark() == HEX_MARK_EMPTY &&
             db.maxGroup()->mark() != HEX_MARK_EMPTY) ||
            (db.minGroup()->mark() != HEX_MARK_EMPTY &&
             db.maxGroup()->mark() == HEX_MARK_EMPTY)) {
    return _emptyColor;
  } else if(db.minGroup()->mark() == HEX_MARK_EMPTY &&
            db.maxGroup()->mark() == HEX_MARK_EMPTY) {
      return _emptyEmpty;
  } else {
    return 0.;
  }
}

std::ostream &operator <<(std::ostream &os,
                          const Circuit::DualBatchConductance &dbc)
{
  os << "(DualBatchConductance "
     << dbc._emptyEmpty << " "
     << dbc._emptyColor << ")";
  return os;
}

//
//
//

#define INVALID_OFFSET -1234567

static Vec<int> makeOffsets(const Grouping &g, int ground, int *nGroups)
{
  const int n = g.size();
  Vec<int> r(n);
  int offset = 0;
  *nGroups = 0;
  for(int i = 0; i < n; i++) {
    if((i == ground) ||
       (!(*g[i]).edge() && (*g[i]).mark() != HEX_MARK_EMPTY)) {
      offset--;
      r[i] = INVALID_OFFSET;
    } else {
      (*nGroups)++;
      r[i] = offset;
    }
  }
  return r;
}

Circuit::Circuit(const Connector &c, const DualBatchConductance &dbc)
{
  const Grouping &g = c.grouping();
  Grouping::GroupIndex ground =
    g.groupIndex(((g.mark() == HEX_MARK_VERT) ?
                  g(HexBoard::TOP_EDGE) : g(HexBoard::LEFT_EDGE)));
  int n;
  Vec<int> offsets = makeOffsets(g, ground, &n);
  Mat<double> G(n, n);
  Vec<double> groundG(n);
  Vec<double> I(n);
  G = 0.;
  groundG = 0.;
  I = 0.;

  Grouping::GroupIndex phase =
    g.groupIndex(((g.mark() == HEX_MARK_VERT) ?
                  g(HexBoard::BOTTOM_EDGE) : g(HexBoard::RIGHT_EDGE)));
  int phaseIndex = phase + offsets[phase];
  // apply current to phase only
  I[phaseIndex] = 1;

  Grouping::GroupIndex k, j;
  int ki, ji;
  double conductance;
  Connector::DualBatchMap::const_iterator cur = c.connections().begin();
  Connector::DualBatchMap::const_iterator end = c.connections().end();
  while(cur != end) {
    k = ki = g.groupIndex((*cur).first.minGroup());
    j = ji = g.groupIndex((*cur).first.maxGroup());
    assert(k != j);
    if(((k == ground) || (offsets[k] != INVALID_OFFSET)) &&
       ((j == ground) || (offsets[j] != INVALID_OFFSET))) {
      ki = k + offsets[k];
      ji = j + offsets[j];
      conductance = dbc.conductance((*cur).second);
      if(conductance != 0.) {
        // apply the element's stamp
        if(k != ground)
          G(ki, ki) += conductance;
        else
          groundG[ji] += conductance;
        if(j != ground)
          G(ji, ji) += conductance;
        else
          groundG[ki] += conductance;
        if(k != ground && j != ground) {
          G(ki, ji) -= conductance;
          G(ji, ki) -= conductance;
        }
      }
    }
    ++cur;
  }
  const Vec<double> &Y = lsSolve(G, I);
  _resistance = fabs(Y[phaseIndex]);
  assert(_resistance >= 0.);
  // Now, let the energy of a group be the sum of current flowing
  // between the group and its neighbours.
  {
    _energy.setSize(g.size());
    _energy = 0.;
    for(k = 0; k < g.size(); k++) {
      if((*g[k]).mark() == HEX_MARK_EMPTY) {
        if(offsets[k] != INVALID_OFFSET) {
          assert(k != ground);
          ki = k + offsets[k];
          double sum = fabs(groundG[ki] * Y[ki]);
          for(ji = 0; ji < n; ji++) {
            if(ji != ki)
              sum +=  fabs(G(ki, ji) * (Y[ki] - Y[ji]));
          }
          _energy[k] = sum;
        }
        // This catches NaN, too.
        if(!(_energy[k] >= 0.))
          std::cout << _energy[k] << std::endl;
        assert(_energy[k] >= 0.);
      } else {
        _energy[k] = -1.;
      }
    }
  }
}

double Circuit::resistance() const
{
  assert(_resistance >= 0.);
  return _resistance;
}

double Circuit::energy(Grouping::GroupIndex gi) const
{
  assert(gi >= 0 && gi < _energy.size());
  assert(_energy[gi] >= 0.);
  return _energy[gi];
}

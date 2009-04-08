// Yo Emacs, this -*- C++ -*-
#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "connector.h"
#include "vec.h"

/**
 * An electrical circuit where nodes are @ref Group instances and
 * wires are connections.
 *
 * The circuit is defined the following way:
 * A wire is added between two groups if and only if there is a virtual
 * connection between them. Between two groups there is at most one wire.
 *
 * The position evaluation is based on calculating the resistence
 * between between edges of the same color. An electrical circuit is
 * set up where the nodes are the groups of cells of the same color
 * and standalone empty cells.
 *
 * A wire is put between two nodes iff there is a virtual connection
 * between the corresponding groups. Position evalution is very
 * tolerant to the choice of resistence values. Still, they affect it
 * fundemantally.
 *
 * In earlier versions of Six it proved to be hard to come up with
 * good resistence values for the different kinds of wires:
 * black-to-black, black-to-empty and empty-to-empty. Obviously,
 * empty-to-empty connections should have resistence than
 * black-to-empty and black-to-black should have the least resistence.
 *
 * While Six calculates all kinds of connections, it creates wires for
 * empty-to-empty (R=1), edge-to-empty (R=1/2) connections only.
 *
 * For the vertical player the group of TOP_EDGE is the ground
 * and 1 unit of current is applied to the group of BOTTOM_EDGE.
 *
 * Resistance is calculated according to the Kirchhoff rules using
 * Nodal Analysis (see @ref http://www.ecs.soton.ac.uk/~mz/CctSim/).
 *
 * Energy level of a group is the sum of unsigned current that flows
 * through the wires connecting it to its neighbours. The higher the
 * energy of an empty group the more promising candidate it is for a
 * move.
 */
class Circuit
{
public:
  class DualBatchConductance
  {
  public:
    DualBatchConductance(double emptyEmpty = 1.0, double emptyColor = 2.0);
    virtual ~DualBatchConductance() {};
    virtual double conductance(const DualBatch &) const;
    friend std::ostream &operator <<(std::ostream &,
                                     const DualBatchConductance &);
  private:
    double _emptyEmpty;
    double _emptyColor;
  };

  /**
   * Calculates conductance for the circuit defined by groups
   * and virtual connections in connector <code>c</code>.
   */
  Circuit(const Connector &, const DualBatchConductance & =
          DualBatchConductance(1.0, 2.0));

  /**
   * The resistance between the edges of connector's mark of interest.
   */
  double resistance() const;

  /**
   * Returns the energy level of the empty group <code>gi</code>.
   */
  double energy(Grouping::GroupIndex) const;
private:
  double _resistance;
  Vec<double> _energy;
};

#endif

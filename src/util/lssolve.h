// Yo Emacs, this -*- C++ -*-
#ifndef LSSOLVE_H
#define LSSOLVE_H

#include "vec.hpp"
#include "mat.hpp"

/**
 * Solver for linear equation system.
 *
 * @return x where Ax=b
 */
Vec<double> lsSolve(const Mat<double> &A, const Vec<double> &b);

#endif

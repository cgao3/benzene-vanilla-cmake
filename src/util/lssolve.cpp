/**
 * This stuff is shamelessly ripped from IT++.
 * Original copyright notice follows.
 *
 *---------------------------------------------------------------------------*
 *                                   IT++			             *
 *---------------------------------------------------------------------------*
 * Copyright (c) 1995-2001 by Tony Ottosson, Thomas Eriksson, Pål Frenger,   *
 * Tobias Ringström, and Jonas Samuelsson.                                   *
 *                                                                           *
 * Permission to use, copy, modify, and distribute this software and its     *
 * documentation under the terms of the GNU General Public License is hereby *
 * granted. No representations are made about the suitability of this        *
 * software for any purpose. It is provided "as is" without expressed or     *
 * implied warranty. See the GNU General Public License for more details.    *
 *---------------------------------------------------------------------------*/

#include "mat.hpp"
#include "vec.hpp"

#include <cassert>
#include <cmath>

static void lu(const Mat<double> &X, Mat<double> &L, Mat<double> &U,
               Vec<int> &p)
{
  assert(X.ys() == X.xs());
  
  int u, k, i, j, n = X.ys();
  double Umax;

  // temporary matrix
  U = X;

  p.setSize(n);
  L.setSize(n, n);

  for(k = 0; k < n - 1; k++) {
    // determine u.  Alt. u=max_index(abs(U(k,n-1,k,k)));
    u = k;
    Umax = fabs(U(k, k));
    for(i = k + 1; i < n; i++) {
      if(fabs(U(k, i)) > Umax) {
        Umax=fabs(U(k, i));
        u=i;
      }
    }
    U.swapRows(k, u);
    p(k) = u;

    if(U(k, k) != 0.) {
      //U(k+1,n-1,k,k)/=U(k,k);
      for(i = k + 1; i < n; i++)
        U(k, i) /= U(k, k);
      // Should be:  U(k+1,n-1,k+1,n-1)-=U(k+1,n-1,k,k)*U(k,k,k+1,n-1);
      // but this is too slow.
      // Instead work directly on the matrix data-structure.
      double *iPos = U.data() + (k + 1) * U.xs();
      double *kPos = U.data() + k * U.xs();
      for(i = k + 1; i < n; i++) {
        for(j = k + 1; j < n; j++) {
          *(iPos + j) -= *(iPos + k) * *(kPos + j);
        }
        iPos += U.xs();
      }
    }
    
  }
  
  p(n - 1) = n - 1;
  
  // Set L and reset all lower elements of U.
  // set all lower triangular elements to zero
  for(i = 0; i < n; i++) {
    L(i, i) = 1.;
    for(j = i + 1; j < n; j++) {
      L(i, j) = U(i, j);
      U(i, j) = 0;
      L(j, i) = 0;
    }
  }
}

static void interchangePermutations(Vec<double> &b, const Vec<int> &p)
{
  assert(b.size() == p.size());
  double temp;
  
  for(int k = 0; k < b.size(); k++) {
    SWAP(b(k), b(p(k)), temp);
  }
}

static void forwardSubstitution(const Mat<double> &L, const Vec<double> &b,
                                Vec<double> &x)
{
  assert(L.ys() == L.xs() && L.xs() == b.size() && b.size() == x.size());
  int n = L.ys(), i, j, iPos;
  double temp;

  x(0) = b(0) / L(0, 0);
  for(i = 1; i < n; i++) {
    // Should be: x(i)=((b(i)-L(i,i,0,i-1)*x(0,i-1))/L(i,i))(0);
    // but this is to slow.
    iPos = i * L.xs();
    temp = 0;
    for(j = 0; j < i; j++) {
      temp += L.data()[iPos + j] * x(j);
    }
    x(i) = (b(i) - temp) / L.data()[iPos + i];
  }
}

static void backwardSubstitution(const Mat<double> &U, const Vec<double> &b,
                                 Vec<double> &x)
{
  assert(U.ys() == U.xs() && U.xs() == b.size() && b.size() == x.size());
  int n = U.ys(), i, j, iPos;
  double temp;

  x(n - 1) = b(n - 1) / U(n - 1, n - 1);
  if(std::isnan(x(n - 1)))
     x(n - 1) = 0.;
  for(i = n - 2; i >= 0; i--) {
    // Should be: x(i)=((b(i)-U(i,i,i+1,n-1)*x(i+1,n-1))/U(i,i))(0);
    // but this is too slow.
    temp = 0;
    iPos = i * U.xs();
    for(j = i + 1; j < n; j++) {
      temp += U.data()[iPos + j] * x(j);
    }
    x(i) = (b(i) - temp) / U.data()[iPos + i];
    if(std::isnan(x(i)))
       x(i) = 0.;
  }
}

static Vec<double> lsSolve(const Mat<double> &L, const Mat<double> &U,
                           const Vec<double> &b)
{
  Vec<double> x(L.ys());
  // Solve Ly=b, Here y=x
  forwardSubstitution(L, b, x);
  // Solve Ux=y, Here x=y
  backwardSubstitution(U, x, x);
  return x;
}

Vec<double> lsSolve(const Mat<double> &A, const Vec<double> &b)
{
  Mat<double> L, U;
  Vec<int> p;
  Vec<double> btemp(b);

  lu(A, L, U, p);
  interchangePermutations(btemp, p);
  return lsSolve(L, U, btemp);
}

/* 
 *  The SvLis Geometric Modelling Kernel
 *  ------------------------------------
 *
 *  Copyright (C) 1993, 1997, 1998, 2000 
 *  University of Bath & Information Geometers Ltd
 *
 *  http://www.bath.ac.uk/
 *  http://www.inge.com/
 *
 *  Principal author:
 *
 *     Adrian Bowyer
 *     Department of Mechanical Engineering
 *     Faculty of Engineering and Design
 *     University of Bath
 *     Bath BA2 7AY
 *     U.K.
 *
 *     e-mail: A.Bowyer@bath.ac.uk
 *        web: http://www.bath.ac.uk/~ensab/
 *
 *   SvLis is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   Licence as published by the Free Software Foundation; either
 *   version 2 of the Licence, or (at your option) any later version.
 *
 *   SvLis is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public Licence for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   Licence along with svLis; if not, write to the Free
 *   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA,
 *   or see
 *
 *      http://www.gnu.org/
 * 
 * =====================================================================
 *
 *  Andy's Real POlynomial Root Solver.  C Version
 *  Andrew Wallis 1994 (from Fortran Version 2.3.1 (1985))
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 15 March 1993
 * This version: 23 September 2000
 *
 */

//
// ---- This routine finds the real roots of a Polynomial that
// ---- lie within a given search-space. The algorithm is based
// ---- on two facts.
//
// 1) All candidate roots of a polynomial are located between
//    a pair of stationary points for the polynomial where the value of
//    the polynomial at each of the max/min are of opposite signs.
//
// 2) The staionary points of a polynomial are located at the
//    roots of its derivative.
//
// ---- Also :-
//
//
// 3) If R'(t) has no roots, then R(t) has a slope of constant sign
// 4) If R(t) has two adjacent stationary points with the same sign
//    then there can be no roots between them.
//
// ---- The algorithm is as follows :-
//
// * From the origional polynomial, create all the diferentiated
//   polynomials down to order 2.
//
// * Initialise the test-point list to contain the points on the ray
//   where it crosses the near and far sides of the object-space.
//
// * Find the roots of the 2nd order equation.
//
// * There will be either 0,1 or 2 real roots.
// * If there are no real roots then from (3) above, the
//   3rd order polynomial is of constant sign slope. Add
//   no points to the test-point list
// * If there is 1 real root then the 3rd order polynomial
//   contains a point of inflexion at that point. Add no
//   points to the test-point list.
// * If there are 2 real roots then evaluate the origional
//   polynomial for these points and add them to the test-point
//   list.
//
//     1 * Increment the current polynomial level
//
// * Take each pair of test-points and use the biscector method
//   and the Regula Falsi method
//   to find the root of the current polynomial. These points
//   form the new test-point list.
//
// * If the current polynomial level is less than the order of the
//   origional polynomial then goto 1
//
// * Test-point list contains the real roots of the origional
//  polynomial.
//
//

#define MAX_DEGREE 12

// #include <unistd.h> // Commented out by Adrian

#include "svlis.h"
#include "polynml.h"
#include "arpors.h"
#if macintosh
 #pragma export on
#endif

static sv_integer debug = 0;


// ==== Evaluates a  polynomial at a point,

static
inline double
evpoly(const double& t,	// Point at which to evaluate polynomialA 
       double *coeffs,	// Array of caoefficients
       sv_integer deg)		// Degree of polynomial
{
   double result = coeffs[deg];
   while(--deg >= 0)
      result=result*t+coeffs[deg];

   return result;
}

sv_integer
arpors(polynomial poly,			// Polynomial for which roots are required
       sv_interval rootfinding_range,	// Range within which to find roots
       sv_integer max_iterations,		// Maximum number of iterations to perform at any stage
       sv_real tol_f,			// Accuracy value for allowable zeros as fraction of
                                        // range between starting points for any pair of test points
       sv_real tol_t,			// Accuracy value for roots as difference in T within interval
       double *roots			// Array of roots in acending order
    )
{
// The function returns the numner of roots found, or nrgative value for error.  Error codes are:
//   -1 = First term in poly has zero coeff.
//   -2 = Iteration count exceeded.
//   -3 = Initial point classification failed.
//
// An array of coefficients.  Each polynomial is stored with its terms in order, starting with the constant term.
// Polynomials start at elements 0, MAX_DEGREE, MAX_DEGREE*2, ...

   double coefficients[MAX_DEGREE*MAX_DEGREE];

   sv_integer quadratic;		// Index of start of quadratic polynomial in the coefficients[] array
   sv_integer current_poly_index;	// Index of start of current polynomial in the coefficients[] array
   sv_integer degree;			// Degree of the current polynomial
   sv_integer maxterm;			// Index +1 of max term in the current polynomial
   double lo_pt, hi_pt;		// Low and high bounds for current biscection/RF chop
   double lo_val, hi_val;	// Polynomial value at lo_pt and hi_pt
   double test_pt, test_val;	// New point and poly value generated by chop
   double next_lo_pt, next_lo_val;
   sv_integer iteration_count;		// Number of iterations for current chop

   sv_integer term, ipol, i, ipt;	// Loop counters
   double coeff, total_coeffs;	// Temp variables
   double bsm4ac, denom;
   double tp1, tp2;
   double scale_factor;
   
   double rtol;			// Accuracy limit for polynomial values for current polynomial

   sv_integer num_test_points;		// Number of test points (start points for biscection or RF)
   sv_integer num_new_test_points;	// Number of test points for next polynomial

   sv_integer bisect;			// Flag to say if next chop is to be bisection or RF
   sv_integer lo_val_gt_0;		// Flag to say that lo_val > 0.0
   sv_integer hi_val_gt_0;		// Flag to say that hi_val > 0.0

// ---- Check for error type -1

   degree = poly.degree();

   if(poly.get_coeff(degree) == 0.0) {
      return -1;
   }
//
// ---- Check polynomial order > 2.
//
   if(degree > 2) {
//
// ---- Form coefficients of normalised differentiated polynomials, down to order 2.
//
      total_coeffs=0.0;
      for(term=0; term<=degree; term++) {
	 coeff=poly.get_coeff(term);
	 total_coeffs += coeff*coeff;
      }
      scale_factor=1.0/sqrt(total_coeffs);
      for(term=0; term<=degree; term++)
	 coefficients[term]=poly.get_coeff(term)*scale_factor;

      for(ipol=1; ipol <= degree-2; ipol++) {
	 total_coeffs=0.0;
	 maxterm = degree-ipol;
	 for(term=0; term<=maxterm; term++) {
	    coeff=coefficients[(ipol-1)*MAX_DEGREE+term+1]*(double)(term+1);
	    coefficients[ipol*MAX_DEGREE+term]=coeff;
	    total_coeffs += coeff*coeff;
	 }
	 scale_factor=1.0/sqrt(total_coeffs);
	 for(term=0; term<=maxterm; term++)
	    coefficients[ipol*MAX_DEGREE+term] *= scale_factor;
      }

      if(debug) {
	 cout << "rootfinding range is [" << rootfinding_range.lo() << ", " << rootfinding_range.hi() << "]\n";
	 cout << "max iteration count is " << max_iterations << "   tol_f = " << tol_f << "   tol_t = " << tol_t << "\n";
	 cout << "polynomial list is:\n";
	 for(ipol=0; ipol <= degree-2; ipol++) {
	    cout << "poly # " << ipol << " (starting at " << ipol*MAX_DEGREE << "  : ";
	    maxterm = degree-ipol;
	    for(term=0; term<=maxterm; term++)
	       cout << coefficients[ipol*MAX_DEGREE+term] << "  ";
	    cout << "\n";
	 }
      }

//
// ---- Find roots of quadratic.
//
      quadratic = (degree-2)*MAX_DEGREE;
      bsm4ac = coefficients[quadratic+1]*coefficients[quadratic+1] - 4.0*coefficients[quadratic]*coefficients[quadratic+2];

// ---- Check for 0,1 or 2 real roots.

      if(bsm4ac < 0.0) {
// ---- 2 Complex roots.
	 roots[0]=rootfinding_range.lo();
	 roots[1]=rootfinding_range.hi();
	 num_test_points=2;

      } else if(bsm4ac == 0.0) {
// ---- 1 real root.
	 roots[0]=rootfinding_range.lo();
	 roots[1]=-coefficients[quadratic+1]/(2.0*coefficients[quadratic+2]);
	 roots[2]=rootfinding_range.hi();
	 num_test_points=3;

      } else {
// ---- 2 real roots.
// ---- Add roots to test-point list so that points are in
// ---- increasing order of magnitude.
	 bsm4ac=sqrt(bsm4ac);
	 denom=2.0*coefficients[quadratic+2];
	 if(denom > 0.0){
	    tp1=(-coefficients[quadratic+1]-bsm4ac)/denom;
	    tp2=(-coefficients[quadratic+1]+bsm4ac)/denom;
	 } else {
	    tp1=(-coefficients[quadratic+1]+bsm4ac)/denom;
	    tp2=(-coefficients[quadratic+1]-bsm4ac)/denom;
	 }

	 if(tp2 < rootfinding_range.lo()) {
	    roots[0]=rootfinding_range.lo();
	    roots[1]=rootfinding_range.hi();
	    num_test_points=2;

	 } else if((tp1 < rootfinding_range.lo()) && (tp2 < rootfinding_range.hi())) {
	    roots[0]=rootfinding_range.lo();
	    roots[1]=tp2;
	    roots[2]=rootfinding_range.hi();
	    num_test_points=3;

        } else if((tp1 < rootfinding_range.lo()) && (tp2 > rootfinding_range.hi())) {
	   roots[0]=rootfinding_range.lo();
	   roots[1]=rootfinding_range.hi();
	   num_test_points=2;

        } else if((tp1 > rootfinding_range.lo()) && (tp2 < rootfinding_range.hi())) {
	   roots[0]=rootfinding_range.lo();
	   roots[1]=tp1;
	   roots[2]=tp2;
	   roots[3]=rootfinding_range.hi();
	   num_test_points=4;

        } else if((tp1 > rootfinding_range.lo()) && (tp2 > rootfinding_range.hi())) {
	   roots[0]=rootfinding_range.lo();
	   roots[1]=tp1;
	   roots[2]=rootfinding_range.hi();
	   num_test_points=3;

        } else if(tp1 > rootfinding_range.hi()) {
	   roots[0]=rootfinding_range.lo();
	   roots[1]=rootfinding_range.hi();
	   num_test_points=2;

        } else {
	   return -3;
	}
      }
//
      bisect = 1;	// Start with bisection

// ---- Set index of current polynomial for be that of degree 3.

      current_poly_index = (degree-3)*MAX_DEGREE;
      degree = 3;

      while(1) {

// ---- Calcualate tolerance in P(t) for current polynomial.  ******** Fortran version used (num_test_points-1) here !!!

	 rtol=0.0;
	 for(i=0; i<num_test_points; i++)
	 	rtol += fabs(evpoly(roots[i], &(coefficients[current_poly_index]), degree));
         rtol=tol_f*rtol/num_test_points; // AB - changed to avoid /0 31/12/95

	 if(debug) {
	    cout << "current_poly_index = " << current_poly_index << "     degree = " << degree << "\n";
	    cout << "num_test_points = " << num_test_points << "\n";
	    for(i=0; i<num_test_points; i++) {
	       cout << "test point [" << i << "] = " << roots[i] << "     P(test point) =  ";
	       cout << evpoly(roots[i], &(coefficients[current_poly_index]), degree) <<"\n";
	    }
	 }

// ---- Initialise new test-point list.

	 num_new_test_points=0;

// ---- Pick start point ( = min of rootfinding_range.lo() and first point in list).

	 if(roots[0] > rootfinding_range.lo()) {
	    next_lo_pt = rootfinding_range.lo();
	    ipt = 0;
	 } else {
	    next_lo_pt = roots[0];
	    ipt = 1;
	 }
	 next_lo_val = evpoly(next_lo_pt, &(coefficients[current_poly_index]), degree);

// ---- Take each pair of test points in turn. If they have the same sign
// ---- of polynomial value then do nothing, else apply alternately Regulai
// ---- Falsi and Biscection methods to obtain roots.

	 while(ipt<num_test_points) {
	    lo_pt = next_lo_pt;
	    hi_pt = roots[ipt];

	    lo_val = next_lo_val;
	    hi_val = evpoly(hi_pt, &(coefficients[current_poly_index]), degree);

	    next_lo_pt = hi_pt;
	    next_lo_val = hi_val;

	    lo_val_gt_0 = lo_val > 0.0;
	    hi_val_gt_0 = hi_val > 0.0;

	    if(lo_val_gt_0 != hi_val_gt_0) {
	       iteration_count=0;		// ---- Zero iteration count.

// ---- Calc estimated zero crossing and polynomial value at that point.

	       while(1) {
		  if(bisect)
		     test_pt=(lo_pt+hi_pt)*0.5;
		  else
		     test_pt=(hi_pt*lo_val - lo_pt*hi_val)/(lo_val-hi_val);
		  
		  bisect = ~bisect;

// ---- If not within accuracy limit replace point of same sign with this point.

		  if((hi_pt-lo_pt) <= tol_t)
		     break;

		  test_val = evpoly(test_pt, &(coefficients[current_poly_index]), degree);
		  
		  if(fabs(test_val) <= rtol)
		     break;

		  if((test_val > 0.0) == lo_val_gt_0) {
		     lo_pt = test_pt;
		     lo_val = test_val;
		  } else {
		     hi_pt = test_pt;
		     hi_val = test_val;
		  }

		  if(iteration_count++ > max_iterations)
		     return-2; // ---- Iteration count exceded
	       }

// ---- Zero within accuracy limit - Add point to new point list.

	       roots[num_new_test_points++]=test_pt;
	    }
	    ipt++;
	 }

// ---- All roots of this order polynomial have been found.  If order is not that of origional
// ---- polynomial then form test point list and repeat.

	 if(current_poly_index == 0)
	    break;

// ---- Add on upper limit for t if last point within bounds.

	 if((num_new_test_points == 0) || (roots[num_new_test_points-1] < rootfinding_range.hi())) {
	    roots[num_new_test_points++]=rootfinding_range.hi();
	 }
	 num_test_points=num_new_test_points;

	 current_poly_index -= MAX_DEGREE;
	 degree++;
      }

// ---- Roots are roots of origional polynomial.

      return num_new_test_points;

   } else {
      // ---- Polynomial is order 2 - solve explicetly.

      sv_real a = poly.get_coeff(2);
      sv_real b = poly.get_coeff(1);
      sv_real c = poly.get_coeff(0);
   
      bsm4ac = b*b - 4.0*a*c;

      // ---- Check for 0,1 or 2 real roots.

      if(bsm4ac < 0.0) {
	 return 0;	// ---- 2 Complex roots.

      } else if(bsm4ac == 0.0) {
	 // ---- 1 real root.

	 roots[0] = -b/(2.0*a);
	 if((roots[0] < rootfinding_range.lo()) || (roots[0] > rootfinding_range.hi()))
	    return 0;
	 else
	    return 1;

      } else {
	 // ---- 2 roots, return those in bounds.

	 bsm4ac = sqrt(bsm4ac);
	 denom=2.0*a;
	 if(denom > 0.0){
	    lo_pt=(-b-bsm4ac)/denom;
	    hi_pt=(-b+bsm4ac)/denom;
	 } else {
	    lo_pt=(-b+bsm4ac)/denom;
	    hi_pt=(-b-bsm4ac)/denom;
	 }
//
// Possible returns
//
// case 1    L H |     |
// case 2      L | H   |
// case 3      L |     | H
// case 4        | L H |
// case 5        | L   | H
// case 6        |     | L H
//
	 if(hi_pt < rootfinding_range.lo()) {
	    return 0;				// CASE 1

	 } else if(lo_pt < rootfinding_range.lo()) {
	    if(hi_pt < rootfinding_range.hi()) {
	       roots[0] = hi_pt;			// CASE 2
	       return 1;
	    } else {
	       return 0;				// CASE 3
	    }
	 } else if(lo_pt < rootfinding_range.hi()) {
	    if(hi_pt < rootfinding_range.hi()) {
	       roots[0] = lo_pt;			// CASE 4
	       roots[1] = hi_pt;
	       return 2;
	    } else {
	       roots[0] = lo_pt;			// CASE 5
	       return 1;
	    }
	 } else {
	    return 0;				// CASE 6
	 }
      }
   }
}
#if macintosh
 #pragma export off
#endif


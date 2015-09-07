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
 * SvLis RAYTRACER - univariate polynomial function file
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: May 1994
 * This version: 8 March 2000
 *
 */


#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "interval.h"
#include "polynml.h"
#if macintosh
 #pragma export on
#endif


// constructors moved from polynomials.h due to loops GMB 27-7-94

   polynomial::polynomial()
   {
      sv_integer i;
      length = DEF_LEN;
      list = new sv_real[length];
      for(i=0;i<length;i++)
         list[i] = 0.0;
      max_index=-1;
   }

   polynomial::polynomial(sv_integer len)
   {
      sv_integer i;
      length = len;
      list = new sv_real[length];
      for(i=0;i<length;i++)
         list[i] = 0.0;
      max_index=-1;
   }

   polynomial::polynomial(sv_real v)
   {
      sv_integer i;
      length = DEF_LEN;
      list = new sv_real[length];
      list[0] = v;
      for(i=1;i<length;i++)
         list[i] = 0.0;
      max_index=0;
   }

   // Copy constructor
   polynomial::polynomial(const polynomial& src)
   {
      if(src.length) {
         length = src.length;
         max_index = src.max_index;
         list = new sv_real[length];
         sv_integer i=0;
         while(i<=max_index)
	{
            list[i] = src.list[i]; // BUG 2, fixed by AB 13-9-95
		i++;
	}
         while(i<length)
	{
            list[i] = 0.0;
		i++;
	}
      } else {
         list = 0;
         length = 0;
         max_index=-1;
      }
   }


// Assignment operator

polynomial
polynomial::operator=(const polynomial& src)
{
   if( this != &src) {	// Check for "dest=src"

      // Free any storage previously used by target of assignment

      if(list) delete[] list;

      // Copy src to target

      if(src.length) {
	 length = src.length;
	 max_index = src.max_index;
	 list = new sv_real[length];
	 sv_integer i=0;
	 while(i<=max_index) {
	    list[i] = src.list[i];
	    i++;
	 }
	 while(i<length)
	    list[i++] = 0.0;
      } else {
	 list = 0;
	 length = 0;
	 max_index=-1;
      }
   }
   return *this;
}

// Set a coefficient, allowing the polynomial to grow

void polynomial::set_coeff(const sv_integer& index, const sv_real& v)
{
      if((index < 0))
	 svlis_error("set_coeff", "invalid polynomial index", SV_WARNING);

	sv_real* new_list;
	sv_integer i;

	if(index >= length)
	{
		new_list = new sv_real[index+DEF_LEN];
		for(i = 0; i <= max_index; i++) new_list[i] = list[i];
		for(i = max_index+1; i < index+DEF_LEN; i++) new_list[i] = 0;
		delete [] list;
		list = new_list;
		length = index+DEF_LEN;
	}

      if(max_index < index) max_index = index;
      list[index] = v;
}

// Arithmetic operators

polynomial
operator+(const polynomial& a, const polynomial& b)
{
   sv_integer length = a.length < b.length ? b.length : a.length;

   polynomial result(length);

   // Combine lists

   if(result.length) {
      if(a.max_index < b.max_index)
	 result.max_index = b.max_index;
      else
	 result.max_index = a.max_index;

      sv_integer i=0;

      if(a.max_index < b.max_index) {
	 while(i<=a.max_index) {
	    result.list[i] = a.list[i]+b.list[i]; i++;
	 }

	 while(i<result.length) {
	    result.list[i] = b.list[i]; i++;
	 }
      } else {
	 while(i<=b.max_index) {
	    result.list[i] = a.list[i]+b.list[i]; i++;
	 }

	 while(i<result.length) {
	    result.list[i] = a.list[i]; i++;
	 }
      }
   }
	 
   return result;
}


polynomial
operator-(const polynomial& a, const polynomial& b)
{
   sv_integer length = a.length < b.length ? b.length : a.length;

   polynomial result(length);

   // Combine lists

   if(result.length) {
      if(a.max_index < b.max_index)
	 result.max_index = b.max_index;
      else
	 result.max_index = a.max_index;

      sv_integer i=0;

      if(a.max_index < b.max_index) {
	 while(i<=a.max_index) {
	    result.list[i] = a.list[i]-b.list[i]; i++;
	 }

	 while(i<result.length) {
	    result.list[i] = -(b.list[i]); i++;
	 }
      } else {
	 while(i<=b.max_index) {
	    result.list[i] = a.list[i]-b.list[i]; i++;
	 }

	 while(i<result.length) {
	    result.list[i] = a.list[i]; i++;
	 }
      }
   }
	 
   return result;
}


polynomial
operator*(const polynomial& a, const polynomial& b)
{
   sv_integer length = a.length+b.max_index;

   polynomial result(length);

   // Combine lists

   if(result.length) {
      result.max_index = a.max_index+b.max_index;
      sv_integer i;
      for(i=0;i<result.length;i++)
	 result.list[i] = 0.0;

      sv_integer b_index=0;
      while(b_index <= b.max_index) {
	 sv_real b_coeff = b.list[b_index];
	 for(i=0;i<=a.max_index;i++)
	    result.list[i+b_index] += a.list[i]*b_coeff;
	 b_index++;
      }
   } 
   return result;
}


// Debug print

void
debug_print_polynomial(const polynomial& poly,
		       char* msg)
{
   sv_integer i;

   cout << "----- polynomial ";
   if(msg) cout << msg;
//   cout << " (at " << (sv_integer)&poly << ") has length = " << poly.length << ", max-index = " << poly.max_index << "\n";
// GMB 06-12-94
   cout << " (at " << (sv_integer)&poly << ") has length = " << poly.length << ", max-index = " << poly.max_index << "\n";
   cout << "list is:\n";

   for(i=0;i<poly.length;i++)
      cout << poly.list[i] << "\n";

   cout << "-----\n\n";
}

// Low-degree closed-form solutions for roots (up to degree 4).
// NB coincident roots are returned twice.
// Added by Adrian....

// Deal with a quadratic

sv_integer quadratic(double* c, double* roots, double tol)
{
	sv_integer result = 0;
	double b24ac = c[1]*c[1] - 4*c[0];
	if (b24ac < -tol)
	{
		result = 0;
	} else if (b24ac < tol)
	{
		result = 2;
		roots[0] = -0.5*c[1];
		roots[1] = roots[0];
	} else
	{
		result = 2;
		b24ac = sqrt(b24ac);
		roots[0] = -0.5*(c[1] + b24ac);
		roots[1] = -0.5*(c[1] - b24ac);
	}
	return(result);
}

// Sort roots - nasty bubble cos there aren't many 

void sortroots(double* r, sv_integer n)
{
	double t;
	for(sv_integer i = 0; i < n-1; i++)
		for(sv_integer j = i; j < n; j++)
			if(r[j] < r[i])
			{
				t = r[j];
				r[j] = r[i];
				r[i] = t;
			}
}

// Deal with a cubic

// See Abramowitz & Stegun: Handbook of Mathematical
// Functions, Dover, SBN 486-61272-4, page 17.

sv_integer cubic(double* c, double* roots, double tol)
{
	sv_integer result = 1;

	const double third = 1.0/3.0;
	double c22 = c[2]*c[2];
	double q = c[1]*third - c22/9;
	double r = c[1]*c[2]/6 - c[0]*0.5 - c22*c[2]/27;
	double q3 = q*q*q;
	double r2 = r*r;
	double q3pr2 = q3 + r2;
	sv_complex cpa, cpb, cp1, cp2;

	if (q3pr2 < -tol)
	{
		result = 3;
		cpa = sqrt(sv_complex(q3pr2,0));
		cpb = sv_complex(r,0);
		cp1 = pow(cpb+cpa,third);
		cp2 = pow(cpb-cpa,third);
		cpa = cp1 + cp2;
		cpb = cp1 - cp2;
		cp1 = sv_complex(0,0.5*sqrt(3.0))*cpb;
		q3 = c[2]*third;
		q = -0.5*cpa.r - q3;
		roots[0] = q - cp1.r;
		roots[1] = q + cp1.r;
		roots[2] = cpa.r - q3;
		sortroots(roots,3);
	} else if (q3pr2 < tol)
	{
		result = 3;
		if(r < 0)
			r2 = -pow(-r,third);
		else
			r2 = pow(r,third);
		q3 = -c[2]*third;
		roots[0] = -r2 + q3;
		roots[1] = roots[0];		
		roots[2] = 2*r2 + q3;
		sortroots(roots,3);
	} else
	{
		result = 1;
		q3pr2 = sqrt(q3pr2);
		r2 = r+q3pr2;
		if(r2 < 0)
			r2 = -pow(-r2,third);
		else
			r2 = pow(r2,third);
		q3 = r-q3pr2;
		if(q3 < 0)
			q3 = -pow(-q3,third);
		else
			q3 = pow(q3,third);
		roots[0] = r2 + q3 - c[2]*third;
	}

	return(result);
}

// Deal with a quartic - Neumark's algorithm
// see http://linus.socs.uts.edu.au/~don/pubs/solving.html

sv_integer quartic(double* q, double* roots, double tol)
{
	sv_integer result = 0;

	double a = q[3];
	double b = q[2];
	double c = q[1];
	double d = q[0];

// Reduced cubic is: y^3 - 2by^2 + (ac + b^2 - 4d)y + (a^2 d - abc + c^2)) = 0 

	double cb[4];
	cb[3] = 1;
	cb[2] = -2*b;
	cb[1] = a*c + b*b - 4*d;
	cb[0] = a*a*d - a*b*c + c*c;

	double r[3];

	sv_integer crts = cubic(cb, r, tol);

	double y = r[0];

	double g1 = a*0.5;
	double g2 = sqrt(a*a - 4*y);
	double h1 = (b - y)*0.5;
	double h2 = (a*h1 - c)/g2;
	g2 = g2*0.5;

	cb[2] = 1;
	cb[1] = g1 + g2;
	cb[0] = h1 + h2;
	result = quadratic(cb, roots, tol);
	cb[1] = g1 - g2;
	cb[0] = h1 - h2;
	result = result + quadratic(cb, &(roots[result]), tol);

	if (result) sortroots(roots, result);
	return(result);
}

// Get real roots of low-degree polynomials analytically
// The function returns the number of roots found, or negative value for error.  
// Error codes are:
//	-1: degree > 4

sv_integer
low_d_roots(const polynomial& poly,	// Polynomial for which roots are required
       double tol,			// Accuracy value
       double *roots			// Array of roots in acending order
    )
{
	sv_integer result = -1;
	sv_integer deg = poly.degree();
	if (deg > 4) return(result);
	sv_integer i;

// Local copy of coefficients; leading coefficient = 1

	double c[5];
	double lti = poly.get_coeff(deg);

	if(lti == 0)
		svlis_error("low_d_roots",
			"polynomial has leading coefficient of 0",
			SV_WARNING);
	else
		lti = 1.0/lti;

// Make the leading coefficient 1

	for(i = 0; i < deg; i++)
		c[i] = poly.get_coeff(i)*lti;
	c[deg] = 1;

	switch(deg)
	{
	case -1:
		svlis_error("low_d_roots","polynomial has no coefficients",
			SV_WARNING);
		break; 

	case 0:
		result = 0;
		break;

	case 1:
		roots[0] = -c[0];
		result = 1;
		break;

	case 2:
		result = quadratic(c, roots, tol);
		break;

	case 3:
		result = cubic(c, roots, tol);
		break;

	case 4:
		result = quartic(c, roots, tol);
		break;

	default:
		break;
	}

	return(result);
}


// Value member functions (should use Horner form)

sv_real polynomial::value(sv_real t) const
{
	sv_real result = get_coeff(0);
	sv_real tt = 1;
	for(int i = 1; i <= degree(); i++)
	{
		tt = tt*t;
		result = result + tt*get_coeff(i);
	}
	return(result);
}

sv_interval polynomial::value(const sv_interval& a) const
{
	sv_interval result = sv_interval(get_coeff(0),get_coeff(0));
	for(int i = 1; i <= degree(); i++)
	{
		result = result + pow(a,i)*get_coeff(i);
	}
	return(result);
}


// Differentiate a polynomial

polynomial derivative(const polynomial& p)
{
	polynomial result;

	for(int i = 0; i < p.degree(); i++)
	{
		result.set_coeff(i,(i+1)*p.get_coeff(i+1));
	}
	return(result);
}
#if macintosh
 #pragma export off
#endif

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
 * SvLis - univariate polynomials
 * 
 * Andy Wallis
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


#ifndef SVLIS_POLYNOMIAL
#define SVLIS_POLYNOMIAL


// Let's have our own weedy complex class...

struct sv_complex
{
	sv_real r, i;
	sv_complex() { r = 0; i = 0; }
	sv_complex(sv_real rr, sv_real ii) { r = rr; i = ii; }
	sv_complex(sv_real rr) {r = rr; i = 0;}
};

inline sv_complex operator+(const sv_complex& a, const sv_complex& b)
{
	return(sv_complex(a.r + b.r, a.i + b.i));
}

inline sv_complex operator-(const sv_complex& a, const sv_complex& b)
{
	return(sv_complex(a.r - b.r, a.i - b.i));
}

inline sv_complex operator-(const sv_complex& a)
{
	return(sv_complex(-a.r, -a.i));
}


inline sv_complex operator*(const sv_complex& a, const sv_complex& b)
{
	return(sv_complex(a.r*b.r - a.i*b.i, a.r*b.i + b.r*a.i));
}

inline sv_complex operator/(const sv_complex& a, const sv_complex& b)
{
        sv_real d = b.r*b.r + b.i*b.i;
	if(d == 0)
	{
	  svlis_error("sv_complex operator/","division by 0", SV_WARNING);
	  return(a);
        }
        return(sv_complex( (b.r*a.r + b.i*a.i)/d,
			  (b.r*a.i - b.i*a.r)/d));
}

inline sv_complex pow(const sv_complex& a, sv_real p)
{
	sv_real theta = p*atan2(a.i,a.r);
	sv_real r = pow((double)sqrt(a.r*a.r + a.i*a.i), (double) p);
	return(sv_complex(r*cos(theta), r*sin(theta)));
}

inline sv_complex sqrt(const sv_complex& a)
{
    return(pow(a, 0.5));
}

// Absolute value of a double
/*
inline double abs(double a)
{
        return( (a >= 0) ? a : -a );
}
*/
#define DEF_LEN 5

class polynomial
{
   sv_real *list;	// List of real coefficients. Element n is coeff for term in t^n
   sv_integer length;
   sv_integer max_index;

 public:

// Constructors and Destructor

// moved to polynomials.cxx due to inline loops GMB 27-7-94

     polynomial();
     polynomial(sv_integer);
     polynomial(sv_real);
     polynomial(const polynomial&);

// Destructor - free all storage for list

   ~polynomial() { if(list) delete[] list; }

// Assignment operator

   polynomial operator=(const polynomial&);

// Member access functions

   sv_integer degree(void) const { return max_index; }
   
// consts added by Adrian

   sv_real get_coeff(const sv_integer& index) const { return list[index]; }  
   
   void set_coeff(const sv_integer&, const sv_real&);

// Arithmetic functions

   friend polynomial operator+(const polynomial&, const polynomial&);
   friend polynomial operator-(const polynomial&, const polynomial&);
   friend polynomial operator*(const polynomial&, const polynomial&);

// Value member functions

   sv_real value(sv_real) const;
   sv_interval value(const sv_interval&) const;

// Debug print

   friend void debug_print_polynomial(const polynomial&, char*);
};

// Differentiate a polynomial

polynomial derivative(const polynomial&);

// Closed-form solutions up to degree 4

sv_integer low_d_roots(const polynomial &, double, double*);

#endif

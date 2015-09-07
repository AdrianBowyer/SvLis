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
 * SvLis - This defines the interval and box classes.
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 8 March 1993
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_INTERVAL
#define SVLIS_INTERVAL

// Interval arithmetic structure

// Note this is not a field as it contains no multiplicative inverse;
// Svlis doesn't need this, and it makes things a lot simpler.

// An interval with its bounds the wrong way is empty.

struct sv_interval
{
	sv_real lo, hi;

	sv_interval(sv_real a=1.0, sv_real b=0.0)
	{
		lo = a;
		hi = b;
	}

	int empty() const { return(lo > hi); }
	
// Membership test value represented by an interval

	mem_test member() const
	{
		mem_test result = SV_SOLID;

		if ( lo > 0.0 ) 
			result = SV_AIR;
		else
			if (hi >= 0.0) result = SV_SURFACE;

		return(result);
	}
	
	// Membership test of a value in an interval

	mem_test member(sv_real a) const
	{
		mem_test result = SV_AIR;
    	
		if(a >= lo)
		{
			if (a <= hi)
			{
				if ((a == lo) || (a == hi))
					result = SV_SURFACE;
				else
					result = SV_SOLID;
			}
		}
		return(result);
	}
	
// Is one interval contained in another ?

	int inside(const sv_interval& b) const
	{
		return((lo >= b.lo) && (hi <= b.hi));
	}
	
// Unique tag

	sv_integer tag() const;	
};

// Intersection and union of two intervals

extern sv_interval operator&(const sv_interval&, const sv_interval&);
extern sv_interval operator|(const sv_interval&, const sv_interval&);

// Intervals the same? 

extern prim_op same(const sv_interval&, const sv_interval&);

// Raise an interval to a power 

extern sv_interval pow(const sv_interval&, sv_integer);

// Sin of an interval

extern sv_interval sin(const sv_interval&);

// I/O

extern ostream& operator<<(ostream&, const sv_interval&);
extern void write(ostream&, const sv_interval&, sv_integer);
extern istream& operator>>(istream&, sv_interval&);
extern void read(istream&, sv_interval&);
extern void read1(istream&, sv_interval&);

// Intervals and reals

inline sv_interval operator+(const sv_interval& a, sv_real b)
{
	return(sv_interval(a.lo + b, a.hi + b));
}
inline sv_interval operator+(sv_real a, const sv_interval& b) 
    {return(b + a);}

inline sv_interval operator-(const sv_interval& a, sv_real b)
{
	return(sv_interval(a.lo - b, a.hi - b));
}
inline sv_interval operator-(sv_real a, const sv_interval& b)
{
	return(sv_interval(a - b.hi, a - b.lo));
}

inline sv_interval operator*(const sv_interval& a, sv_real b)
{
	if (b > 0.0)
		return(sv_interval(a.lo*b, a.hi*b));
	else
		return(sv_interval(a.hi*b, a.lo*b));
}
inline sv_interval operator*(sv_real a, const sv_interval& b) 
    {return(b*a);}

inline sv_interval operator/(const sv_interval& a, sv_real b)
{
	sv_interval result = a;

	if (b == 0.0)
		svlis_error("sv_interval::operator/","division by 0", 
		    SV_WARNING);
	else
		result = result*(1/b);
	return(result);
} 

// Monadic minus

inline sv_interval operator-(const sv_interval& a)
{
	return(sv_interval(-a.hi, -a.lo));
}

// Interval arithmetic

inline sv_interval operator+(const sv_interval& a, const sv_interval& b)
{
	return(sv_interval(a.lo + b.lo, a.hi + b.hi));
}

inline sv_interval operator-(const sv_interval& a, const sv_interval& b)
{
	return(sv_interval(a.lo - b.hi, a.hi - b.lo));
}

inline sv_interval operator*(const sv_interval& a, const sv_interval& b)
{
	sv_real c = a.lo*b.lo;
	sv_real d = c;
	sv_real q = a.lo*b.hi;
	sv_real r = a.hi*b.lo;
	sv_real s = a.hi*b.hi;

	if (q < c) c = q;
	if (r < c) c = r;
	if (s < c) c = s;

	if (q > d) d = q;
	if (r > d) d = r;
	if (s > d) d = s;

	return(sv_interval(c,d));
}

// Interval division is not defined, as Svlis does not support rationals

// Absolute value of an interval

inline sv_interval abs(const sv_interval& a)
{
	sv_interval result = a;
	sv_real p;

	if (a.lo < 0.0)
	{
		if (a.hi <= 0.0)
		{
			result.lo = -a.hi;
			result.hi = -a.lo;
		} else
		{
			result.lo = 0.0;
			if ( (p = -a.lo) > a.hi ) result.hi = p;
		}
	}
	return(result);
}

// Sign of an interval

inline sv_interval sign(const sv_interval& a)
{
	return( sv_interval(sign(a.lo), sign(a.hi)) );
}

// Max and min

inline sv_interval max(const sv_interval& a, const sv_interval& b)
{
	sv_interval result = b;
	if (a.lo > b.lo) result.lo = a.lo;
	if (a.hi > b.hi) result.hi = a.hi;
	return(result);
}

inline sv_interval min(const sv_interval& a, const sv_interval& b)
{
	sv_interval result = b;
	if (a.lo < b.lo) result.lo = a.lo;
	if (a.hi < b.hi) result.hi = a.hi;
	return(result);
}

inline sv_interval cos(const sv_interval& u) { return(sin(u + (sv_real)(0.5*M_PI))); }

// e^interval and log OF A POSITIVE interval

inline sv_interval exp(const sv_interval& a)
{
	return(sv_interval((sv_real)exp(a.lo), (sv_real)exp(a.hi)));
}

inline sv_interval log(const sv_interval& a)
{
	sv_interval result = a;

	if (a.lo <= 0.0)
		svlis_error("log(sv_interval)","interval not positive",
		    SV_WARNING);
	else
		result = sv_interval((sv_real)log(a.lo), (sv_real)log(a.hi));

	return(result);
}


// Signed square root

inline sv_interval s_sqrt(const sv_interval& a)
{
	return(sv_interval(s_sqrt(a.lo), s_sqrt(a.hi)));
}

// sv_interval
// ***********************************************************************

// A 3-D box comprising three intervals


struct sv_box
{
	sv_interval xi, yi, zi;

// Make a null box

	sv_box() { }

// Make a box from 3 intervals

	sv_box(sv_interval a, sv_interval b, sv_interval c)
	{
		xi = a;
		yi = b;
		zi = c;
	}

// Make a box from 2 points

	sv_box(sv_point low, sv_point high)
	{
		xi = sv_interval(low.x,high.x);
		yi = sv_interval(low.y,high.y);
		zi = sv_interval(low.z,high.z);
	}
	
// The volume of a box

	sv_real vol() const
	{
		sv_real x = xi.hi - xi.lo;	
		sv_real y = yi.hi - yi.lo;	
		sv_real z = zi.hi - zi.lo;
		return(x*y*z);
	}

// The squared length of the box's diagonal

	sv_real diag_sq() const
	{
		sv_real x = xi.hi - xi.lo;	
		sv_real y = yi.hi - yi.lo;	
		sv_real z = zi.hi - zi.lo;
		return(x*x + y*y + z*z);
	}

// The middle of a box

	sv_point centroid() const
	{
		return( sv_point( (xi.lo + xi.hi)/2,(yi.lo + yi.hi)/2,
			(zi.lo + zi.hi)/2 ) );
	}

// Membership test of a point in a box

	mem_test member(const sv_point& p) const
	{
		mem_test result = SV_AIR;
		mem_test x, y, z;

		if ((x = xi.member(p.x)) != SV_AIR)
		{
			if ((y = yi.member(p.y)) != SV_AIR)
			{
				if ((z = zi.member(p.z)) != SV_AIR)
				{
					if( (x == SV_SURFACE) || (y == SV_SURFACE) || (z == SV_SURFACE) )
						result = SV_SURFACE;
					else
						result = SV_SOLID;
				}
			}
		}
		return(result);
	}
	
// Is one box in another?

	int inside(const sv_box& b) const
	{
		return(xi.inside(b.xi) && yi.inside(b.yi) && zi.inside(b.zi));
	}

	
// The lexical order of a box's vertices

	sv_point corner(sv_integer) const;
	
// Empty box?

	int empty() const
	{
		return(xi.empty() && yi.empty() && zi.empty());
	} 

               
// norm of a box returns a box that is as if every point in the box
// were normalized.  Note - the box must not contain the origin.

	sv_box norm(const sv_box& a)
	{
		sv_interval denom = s_sqrt(pow(a.xi, 2) + pow(a.yi, 2) + pow(a.zi, 2));
		sv_interval recip = sv_interval(1/denom.hi, 1/denom.lo);
		return(sv_box(a.xi*recip, a.yi*recip, a.zi*recip));
	}

// Unique tag

	sv_integer tag() const;
};

// The lexical order of a box's edges (corner pairs);

extern void box_edge(sv_integer, sv_integer*, sv_integer*);

// The vertices of the tetrahedral decomposition of a box
// sv_tet_vert[i][j] gives the jth vertex of the ith tet
// 0 <= i < 6, 0 <= j < 4
 
extern sv_integer sv_tet_vert(sv_integer, sv_integer);

// The edges of the tetrahedral decomposition
// sv_tet_edge[i][j] gives the jth edge of the ith tet
// 0 <= i < 6, 0 <= j < 6

extern sv_integer sv_tet_edge(sv_integer, sv_integer);

// The lookup table for primitive/tetrahedron polygons

extern sv_integer sv_tet_pol(sv_integer, sv_integer);
extern sv_integer sv_v_count(sv_integer);

// Find the part of a line that cuts a box

extern sv_interval line_box(const sv_line&, const sv_box&);

// Compute the part of a parametric interval on a line inside a plane

extern sv_interval l_int_plane(const sv_line&, sv_plane&, const sv_interval&);

extern bsp_type box_spread(const sv_box&, sv_point*, sv_point*, sv_real*, sv_real*);


// I/O

extern ostream& operator<<(ostream&, const sv_box&);
extern void write(ostream&, const sv_box&, sv_integer);
extern istream& operator>>(istream&, sv_box&);
extern void read(istream&, sv_box&);
extern void read1(istream&, sv_box&);

// To do arithmetic on boxes, just do the arithmetic on their intervals
// Everywhere a real is needed for an interval, a point is needed here

inline sv_box operator-(const sv_box& a)
{
	return(sv_box(-a.xi, -a.yi, -a.zi));
}

// Adding or subtracting a point affects a translation

inline sv_box operator+(const sv_box& a, const sv_point& b)
{
	return(sv_box(a.xi+b.x,a.yi+b.y,a.zi+b.z));
}
inline sv_box operator+(const sv_point& a, const sv_box& b) { return(b + a); }

inline sv_box operator-(const sv_box& a, const sv_point& b)
{
	return(a + (-b));
}
inline sv_box operator-(const sv_point& a, const sv_box& b) { return(a + (-b)); }

inline sv_box operator*(const sv_point& a, const sv_box& b)
{
	return(sv_box(a.x*b.xi, a.y*b.yi, a.z*b.zi));
}
inline sv_box operator*(const sv_box& a, const sv_point& b) { return(b*a); }

inline sv_box operator/(const sv_box& a, const sv_point& b)
{
	return(sv_box(a.xi/b.x, a.yi/b.y, a.zi/b.z));
}

inline sv_box operator+(const sv_box& a, const sv_box& b)
{
	return(sv_box(a.xi + b.xi, a.yi + b.yi, a.zi + b.zi));
}

inline sv_box operator-(const sv_box& a, const sv_box& b)
{
	return(sv_box(a.xi - b.xi, a.yi - b.yi, a.zi - b.zi));
}

inline sv_box operator*(const sv_box& a, const sv_box& b)
{
	return(sv_box(a.xi*b.xi, a.yi*b.yi, a.zi*b.zi));
}

// Absolute value of a box

inline sv_box abs(const sv_box& a)
{
	return(sv_box(abs(a.xi), abs(a.yi), abs(a.zi)));
}

// Sign of a box

inline sv_box sign(const sv_box& a)
{
	return(sv_box(sign(a.xi), sign(a.yi), sign(a.zi)));
}

// Max and min

inline sv_box max(const sv_box& a, const sv_box& b)
{
	return(sv_box(max(a.xi, b.xi), max(a.yi, b.yi), max(a.zi, b.zi)));
}

inline sv_box min(const sv_box& a, const sv_box& b)
{
	return(sv_box(min(a.xi, b.xi), min(a.yi, b.yi), min(a.zi, b.zi)));
}

// sin, cos and e^ (well, why not?)

inline sv_box sin(const sv_box& a)
{
	return(sv_box(sin(a.xi), sin(a.yi), sin(a.zi)));
}

inline sv_box cos(const sv_box& a)
{
	return(sv_box(cos(a.xi), cos(a.yi), cos(a.zi)));
}

inline sv_box exp(const sv_box& a)
{
	return(sv_box(exp(a.xi), exp(a.yi), exp(a.zi)));
}

inline sv_box log(const sv_box& a)
{
	return(sv_box(log(a.xi), log(a.yi), log(a.zi)));
}

inline sv_box s_sqrt(const sv_box& a)
{
	return(sv_box(s_sqrt(a.xi), s_sqrt(a.yi), s_sqrt(a.zi)));
}


// Raise a box to a power (well, why not)^2

inline sv_box pow(const sv_box& a, sv_integer b)
{
	return(sv_box(pow(a.xi,b), pow(a.yi, b), pow(a.zi, b)));
}

// Boxes the same?

inline prim_op same(const sv_box& a, const sv_box& b)
{
	prim_op x = same(a.xi, b.xi);
	prim_op y = same(a.yi, b.yi);
	prim_op z = same(a.zi, b.zi);

	if( (x == SV_PLUS) && (y == SV_PLUS) && (z == SV_PLUS) ) 
		return(SV_PLUS);
	if( (x == SV_COMP) && (y == SV_COMP) && (z == SV_COMP) ) 
		return(SV_COMP);
	return(SV_ZERO);
}

// Intersection of two boxes (n.b.  SOME dimensions may be empty, some not)

inline sv_box operator&(const sv_box& a, const sv_box& b)
{
	return(sv_box(a.xi&b.xi, a.yi&b.yi, a.zi&b.zi));
}

// Union of two boxes

inline sv_box operator|(const sv_box& a, const sv_box& b)
{
	return(sv_box(a.xi|b.xi, a.yi|b.yi, a.zi|b.zi));
}

// Plane range function can be defined now we know what an interval is

inline sv_interval sv_plane::range(const sv_box& b) const
{
	return(this->normal.x*b.xi + this->normal.y*b.yi + 
		this->normal.z*b.zi + this->d);
}

// Sv_box
// **********************************************************************

#endif

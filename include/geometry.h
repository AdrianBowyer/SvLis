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
 * SvLis - This defines all the geometrical structures that svLis uses.
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 4 March 1993
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_GEOMETRY
#define SVLIS_GEOMETRY

// point: a 3D vector

struct sv_point
{
	sv_real x, y, z;

// Constructor defaults to the origin.

	sv_point() {x=0; y=0; z=0;}
	sv_point(sv_real a, sv_real b, sv_real c) {x=a; y=b; z=c;}

// Modulus and normalization

	sv_real mod() const;
	sv_point norm() const;
	
// Transformations

	sv_point spin(const sv_line&, sv_real) const;
	sv_point mirror(const sv_plane&) const;
	sv_point scale(const sv_point&, sv_real) const;
	sv_point scale(const sv_line&, sv_real) const;

// Unique tag

	sv_integer tag() const;
};


// External and inline definitions for all the point procedures

// I/O

extern ostream& operator<<(ostream&, const sv_point&);
extern void write(ostream&, const sv_point&, sv_integer);
extern istream& operator>>(istream&, sv_point&);
extern void read(istream&, sv_point&);
extern void read1(istream&, sv_point&);

// Monadic minus, diadic + and -

inline sv_point operator-(const sv_point& a)
{
	return(sv_point(-a.x, -a.y, -a.z));
}

inline sv_point operator+(const sv_point& a, const sv_point& b)
{
	return(sv_point(a.x + b.x, a.y + b.y, a.z + b.z));
}

inline sv_point operator-(const sv_point& a, const sv_point& b)
{
	return(sv_point(a.x - b.x, a.y - b.y, a.z - b.z));
}

// Multiply/divide by a real

inline sv_point operator*(const sv_point& a, sv_real b)
{
	return(sv_point(a.x*b, a.y*b, a.z*b));
}
inline sv_point operator*(sv_real a, const sv_point& b) {return(b*a);}

inline sv_point operator/(const sv_point& a, sv_real b)
{
	sv_point result = a;

	if (b == 0)
		svlis_error("sv_point::operator/","division by 0",SV_WARNING);
	else
	{
		result.x = a.x/b;
		result.y = a.y/b;
		result.z = a.z/b;
	}
        return(result);
}

// Scalar and vector product

inline sv_real operator*(const sv_point& a, const sv_point& b)
{
	return(a.x*b.x + a.y*b.y + a.z*b.z);
}

inline sv_point operator^(const sv_point& a, const sv_point& b)
{
	return(sv_point(a.y*b.z - b.y*a.z,
		b.x*a.z - a.x*b.z,
		a.x*b.y - b.x*a.y));
}

// Do mod now we know how to do *

inline sv_real sv_point::mod() const
{
	return((sv_real)sqrt( (*this)*(*this) ));
}

// finds the squared distance between two points

inline sv_real dist_2(const sv_point& p1, const sv_point& p2) 
{   
    return  (((p1.x - p2.x)*(p1.x - p2.x)) + 
	     ((p1.y - p2.y)*(p1.y - p2.y)) +
	     ((p1.z - p2.z)*(p1.z - p2.z)));	
}


// Generate a unit-length point at right angles to another

inline sv_point right(const sv_point& a)
{
  if(fabs(a.x) < 0.1)
    return((a^SV_X).norm());
  else
    return((a^SV_Y).norm());
}

// Set up a set of orthogonal axes with p as one (w)

inline void sv_axes(const sv_point& p, sv_point& u, sv_point& v, sv_point& w)
{
        w = p.norm();
        u = right(w);
        v = w^u;
}

// Two points the same?

inline prim_op same(const sv_point& a, const sv_point& b)
{
	if ((a - b).mod() <= sv_same_tol) return(SV_PLUS);
	if ((a + b).mod() <= sv_same_tol) return(SV_COMP);
	return(SV_ZERO);
}

// sv_point
// **************************************************************************

// sv_line: a normalized parametric straight line in space.
// The line passes through the point origin and goes in direction.

struct sv_line
{
	sv_point direction, origin;

	sv_line() { direction = SV_Z; }	// Null constructor

	sv_line(const sv_point& a, const sv_point& b) 
			{direction = a.norm(); origin = b;}

// Transformations

	sv_line spin(const sv_line&, sv_real) const;
	sv_line mirror(const sv_plane&) const;
	sv_line scale(const sv_point&, sv_real) const;
	sv_line scale(const sv_line&, sv_real) const;

// Parametric point

        sv_point point(sv_real t) const { return(origin + direction*t); }

// Unique tag

	sv_integer tag() const;
};

// I/O

extern ostream& operator<<(ostream&, const sv_line&);
extern void write(ostream&, const sv_line&, sv_integer);
extern istream& operator>>(istream&, sv_line&);
extern void read(istream&, sv_line&);
extern void read1(istream&, sv_line&);

// Monadic minus changes the direction

inline sv_line operator-(const sv_line& a)
{
	return(sv_line(-a.direction, a.origin));
}

// Adding a point effects a translation

inline sv_line operator+(const sv_line& a, const sv_point& b)
{
	return(sv_line(a.direction,a.origin + b));
}

// Is this sensible?

inline sv_line operator+(const sv_point& a, const sv_line& b)
{
	return(b+a);
}
inline sv_line operator-(const sv_line& a, const sv_point& b)
{
	return(a + (-b));
}

// NB (point - line) really doesn't make sense

// Retained for backwards compatibility

inline sv_point line_point(const sv_line& a, sv_real t)
{
	return(a.point(t));
}

// Two lines the same?  

// If the answer is:

// SV_ZERO - no
// SV_PLUS - yes
// SV_TIMES - coincide, but with different origins
// SV_COMP - going oposite ways

inline prim_op same(const sv_line& a, const sv_line& b)
{
	prim_op r1 = same(a.direction, b.direction);
	if (!r1) return(SV_ZERO);
	prim_op r2 = same(a.origin, b.origin);
	if (r2 == SV_PLUS) return(r1);
	sv_point k = a.origin - b.origin;
	if(same(fabs(k*a.direction),1)) return(SV_TIMES);
	return(SV_ZERO);
}

// sv_line
// **************************************************************************

// sv_plane: a flat plane in space.
// The plane is:

//    normal.x * X  +  normal.y * Y + normal.z * Z + d = 0

struct sv_plane
{
        sv_point normal;
        sv_real d;
        
        sv_plane() { normal = SV_Z; d = 0; }	// Null constructor

// Constructor takes a normal vector (a) and a point through which
// the plane is to pass (b).

	sv_plane(const sv_point& a, const sv_point& b)
	{
		normal = a.norm();
		d = -b*normal;
	}
	
// Constructor takes three points through which the plane passes.

	sv_plane(const sv_point& a, const sv_point& b,  const sv_point& c)
	{
		normal = ((b - a)^(c - a)).norm();
		d = -a*normal;
	}

// Constructor for when we know the normal and d

	sv_plane(const sv_point& n, sv_real dd)
	{
		normal = n;
		d = dd;
		sv_real div = normal.mod();
		if (div == 0.0)
			svlis_error("sv_plane(sv_point, sv_real)",
			    "zero-length normal",SV_WARNING);
		else
		{
			div = 1/div;
			normal = normal*div;
			d = d*div;
		}
	}
		

// Constructor for when we know the coefficients

	sv_plane(sv_real a, sv_real b, sv_real c, sv_real dd)
	{
		sv_point n = sv_point(a,b,c);
		*this = sv_plane(n,dd);
	}
	
// Signed distance of a sv_point from a plane, and value of a box

	sv_real value(const sv_point& p) const
	{
		return(p*normal + d);
	}

	sv_interval range(const sv_box&) const;
	
// Transformations

	sv_plane spin(const sv_line&, sv_real) const;
	sv_plane mirror(const sv_plane&) const;
	sv_plane scale(const sv_point&, sv_real) const;
	sv_plane scale(const sv_line&, sv_real) const;

// Intersection with a line

        sv_point point(const sv_line& l);

// Unique tag

	sv_integer tag() const;
};

// plane inlines and externs

// I/O

extern ostream& operator<<(ostream&, const sv_plane&);
extern void write(ostream&, const sv_plane&, sv_integer);
extern istream& operator>>(istream&, sv_plane&);
extern void read(istream&, sv_plane&);
extern void read1(istream&, sv_plane&);

// Line of intersection between two planes

extern sv_line planes_line(const sv_plane&, const sv_plane&, flag_val& flag);

// Backwards compatibility

inline sv_line planes_line(const sv_plane& a, const sv_plane& b)
{
  flag_val flag;
  sv_line result = planes_line(a,b,flag);
  set_svlis_flag(flag);
  return(result);
}

// Intersection between a line and a plane

extern sv_real plane_line_t(const sv_plane&, const sv_line&, flag_val& flag);

// Backwards compatibility

inline sv_real plane_line_t(const sv_plane& a, const sv_line& b)
{
  flag_val flag;
  return(plane_line_t(a,b,flag));
}

// Intersection of three planes

extern sv_point planes_point(const sv_plane&, const sv_plane&, 
    const sv_plane&, flag_val& flag);

// Backwards compatibility

inline sv_point planes_point(const sv_plane& a, const sv_plane& b, const sv_plane& c)
{
  flag_val flag;
  return(planes_point(a,b,c,flag));
}

// Monadic - means complement

inline sv_plane operator-(const sv_plane& a)
{
	return(sv_plane(-a.normal,-a.d));
}

// As usual, adding a point effects a translation

inline sv_plane operator+(const sv_plane& a, const sv_point& b)
{
	return(sv_plane(a.normal,b - a.d*a.normal));
}
inline sv_plane operator+(const sv_point& a, const sv_plane& b)
{
	return(b + a);
}
inline sv_plane operator-(const sv_plane& a, const sv_point& b)
{
	return(a + (-b));
}

inline sv_point sv_plane::point(const sv_line& l)
{
	return(l.point(plane_line_t(*this,l)));
}

// Retained for backwards compatibility

inline sv_point plane_line(const sv_plane& f, const sv_line& l)
{
	return(line_point(l,plane_line_t(f,l)));
}

// Two planes the same?

inline prim_op same(const sv_plane& a, const sv_plane& b)
{
	prim_op result = same(a.normal, b.normal);
	if (!result) return(SV_ZERO);
	
	prim_op r2 = same(a.d, b.d);
	if (!r2) return(SV_ZERO);
	if (r2 != result) return(SV_ZERO);
	return(result);
}


// sv_plane
// **************************************************************************

#endif

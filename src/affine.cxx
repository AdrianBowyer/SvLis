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
 *   SVLIS interval procedures
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
 * This version: 23 September 2000
 *
 */

#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "interval.h"
#if macintosh
 #pragma export on
#endif

// I/O

void write(ostream& s, const sv_interval& i, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_INTERVAL, 0, 0);
	put_token(s, SVT_OB, 0, 0);
	put_token(s, SVT_REAL, 0, i.a0);
	s << ", ";
	put_token(s, SVT_REAL, 0, i.od);
	s << ", ";
	put_token(s, SVT_REAL, 0, i.ev);
	s << ", ";
	put_token(s, SVT_INTEGER, i.empt, 0);
	put_token(s, SVT_CB, 0, 0);
}

ostream& operator<<(ostream& s, const sv_interval& i)
{
	write(s, i, 0);
	return(s);
}

istream& operator>>(istream& s, sv_interval& i)
{
	if(get_read_version() == SV_VER)
		read(s, i);
	else
	{
		if(get_read_version() != (SV_VER-1))
			svlis_error("operator>> sv_i",
			"file version is too old",
			SV_WARNING);
		read1(s, i);
	}
	return(s);
}

void write(ostream& s, const sv_box& b, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_BOX, 0, 0);
	put_token(s, SVT_OB, 0, 0);
	write(s, b.xi, 0);
	s << ", ";
	write(s, b.yi, 0);
	s << ", ";
	write(s, b.zi, 0);
	put_token(s, SVT_CB, 0, 0);
}

ostream& operator<<(ostream& s, const sv_box& b)
{
	write(s, b, 0);
	return(s);
}

istream& operator>>(istream& s, sv_box& b)
{
	if(get_read_version() == SV_VER)
		read(s, b);
	else
	{
		if(get_read_version() != (SV_VER-1))
			svlis_error("operator>> sv_box",
			"file version is too old",
			SV_WARNING);
		read1(s, b);
	}
	return(s);
}

// Unique tag

sv_integer sv_interval::tag() const { return(SVT_F*SVT_INTERVAL); }

// Intervals the same?

prim_op same(const sv_interval& aa, const sv_interval& bb)
{
        sv_i a = aa.interval();
        sv_i b = bb.interval();

	if( (fabs(a.lo - b.lo) < sv_same_tol) && 
		(fabs(a.hi - b.hi) < sv_same_tol) ) return(SV_PLUS);
	if( (fabs(a.lo + b.hi) < sv_same_tol) && 
		(fabs(a.hi + b.lo) < sv_same_tol) ) return(SV_COMP);
	return(SV_ZERO);
}

// Raise and interval to an integer power.  

sv_interval pow(const sv_interval& a, sv_integer i)
{
  sv_interval result = sv_interval(1,1);
  for(sv_integer k = 0; k < i; k++) result = result*a;
  result.empt = 0; // defensive
  return(result);

#if 0
	if (i < 0)
	{
		svlis_error("sv_i::operator^","negative exponent",SV_WARNING);
		return(a);
	}

	if (i%2)
	{
		return(sv_i(pow(a.lo,i),pow(a.hi,i)));
	} else
	{
		if ((a.lo < 0.0) && (a.hi < 0.0)) 
			return(sv_i(pow(a.hi,i),pow(a.lo,i)));
		if ((a.lo < 0.0) && (a.hi >= 0.0))
			return(sv_i(0,pow(max(-a.lo, a.hi), i)));
		return(sv_i(pow(a.lo,i),pow(a.hi,i)));
	}
#endif
}


sv_i sin(const sv_i& uu)
{
/*
 * This function is passed an interval containing two angles in radians, it
 * then outputs the corresponding sin values.
 * There are a number of special cases these are when the radian values straddle
 * M_PI/2 and 3/2M_PI. In these cases the values closest to zero is keep along  
 * with the value one or -one depending upon the peak crossed.
 * For consistancy if the values traverse both values then one value is set to
 * one and the other -one.
 * This function assumes that the angles are in increasing order.
 */
sv_i u = uu;
sv_i v;
sv_real x;

// First test if interval is greater than a revolution.

   if ((u.hi - u.lo) >= (2*M_PI)) return (sv_i(-1.0, 1.0));


// Get in range

   while(u.hi > 2.0*M_PI) u = u - 2.0*M_PI;
   while(u.lo < 0.0) u = u + 2.0*M_PI;

 /* test if both intervals cross both peaks */

 if( ((u.lo <= 0.5*M_PI) && (u.hi >= 1.5*M_PI)) ||
	((u.lo <= 1.5*M_PI) && (u.hi >= 2.5*M_PI))||
        ((u.lo <= 2.5*M_PI) && (u.hi >= 3.5*M_PI))  )
              return (sv_i(-1.0, 1.0));
 else
  {
   /* test if interval crosses M_PI/2 or 2.5*M_PI */
   if( ((u.lo <= 0.5*M_PI) && (u.hi >= 0.5*M_PI)) ||
		((u.lo <= 2.5*M_PI) && (u.hi >= 2.5*M_PI)) )
     {
      v.lo = (sv_real)sin(u.lo);
      if( (x = (sv_real)sin(u.hi)) < v.lo ) v.lo = x;
      v.hi = 1.0;
     }
   else
     {
      /* test if interval crosses 3/2*M_PI or 3.5*M_PI*/
      if( ((u.lo <= 1.5*M_PI) && (u.hi >= 1.5*M_PI)) ||
		((u.lo <= 3.5*M_PI) && (u.hi >= 3.5*M_PI)) )
        {
         v.hi = (sv_real)sin(u.lo);
         if( (x = (sv_real)sin(u.hi)) > v.hi ) v.hi = x;
         v.lo = -1.0;
        }
      else
        {
         /* else compute sin values */
         v.lo = (sv_real)sin(u.lo);
         v.hi = (sv_real)sin(u.hi);
	 if (v.lo > v.hi)
	 {
		x = v.lo;
		v.lo = v.hi;
		v.hi = x;
	 } 
        }
     }
  }
 return(v);
}


// Compute the (possibly null) intersection between two intervals

sv_interval operator&(const sv_interval& a, const sv_interval& b)
{
	if (a.empty() || b.empty()) return(a);

	sv_i result;	// Null constructor makes an empty interval

	if (a.lo() > b.hi()) return(sv_interval(result));
	if (b.lo() > a.hi()) return(sv_interval(result));
	if (b.lo() <= a.lo())
		result.lo = a.lo();
	else 
		result.lo = b.lo();
	if (b.hi() >= a.hi())
		result.hi = a.hi();
	else
		result.hi = b.hi();
	return(sv_interval(result));
}

// Compute the union of two intervals - this is defined to be the whole
// range if the two are disjoint (i.e. there are no intervals with holes in).

sv_interval operator|(const sv_interval& a, const sv_interval& b)
{
	sv_i result;	// Null constructor makes an empty interval

	if(a.empty())
	{
		if(b.empty())
			return(sv_interval(result));
		else
			return(b);
	} else
	{
		if(b.empty()) return(a);
		result = a.interval();
		if (b.lo() < a.lo()) result.lo = b.lo();
		if (b.hi() > a.hi()) result.hi = b.hi();
	}
	return(sv_interval(result));
}

// sv_interval
//**********************************************************************

// Boxes


// Unique tag

sv_integer sv_box::tag() const { return(SVT_F*SVT_BOX); }

// The lexical order of a box's corners

sv_point sv_box::corner(sv_integer i) const
{
	switch(i)
	{
	case 0:	return( sv_point(this->xi.lo(), this->yi.lo(), this->zi.lo()) );
	case 1:	return( sv_point(this->xi.lo(), this->yi.lo(), this->zi.hi()) );
	case 2:	return( sv_point(this->xi.lo(), this->yi.hi(), this->zi.lo()) );
	case 3:	return( sv_point(this->xi.lo(), this->yi.hi(), this->zi.hi()) );
	case 4:	return( sv_point(this->xi.hi(), this->yi.lo(), this->zi.lo()) );
	case 5:	return( sv_point(this->xi.hi(), this->yi.lo(), this->zi.hi()) );
	case 6:	return( sv_point(this->xi.hi(), this->yi.hi(), this->zi.lo()) );
	case 7:	return( sv_point(this->xi.hi(), this->yi.hi(), this->zi.hi()) );
	default:
		svlis_error("box_corner","silly corner number",SV_WARNING);
	}
	return(SV_OO);
}

// The lexical order of a box's edges (corner pairs)
// 0-11 are the axis-parallel edges, 12-18 are the
// edges of the internal tetrahedral decomposition

static const sv_integer edge0[19] = {0,4,6,2,0,1,5,4,0,2,3,1,
							0,0,3,4,1,1,0};
static const sv_integer edge1[19] = {1,5,7,3,2,3,7,6,4,6,7,5,
							3,6,6,7,7,4,7};

void box_edge(sv_integer i, sv_integer* c0, sv_integer* c1)
{
	*c0 = edge0[i];
	*c1 = edge1[i];
	return;
}

// The 4 corners of each of the six internal tetrahedra

static const sv_integer stv[6][4] = {
			0,2,3,6,
			1,4,7,5,
			0,4,6,7,
			0,1,4,7,
			0,3,7,6,
			0,1,7,3
};

sv_integer sv_tet_vert(sv_integer i, sv_integer j) { return(stv[i][j]);}

// The six edges of each of the six internal tetrahedra

static const sv_integer ste[6][6] = {
			4,12,13,3,9,14,
			17,16,11,15,1,6,
			8,13,18,7,15,2,
			0,8,18,17,16,15,
			12,18,13,10,14,2,
			0,18,12,16,5,10
};

sv_integer sv_tet_edge(sv_integer i, sv_integer j) { return(ste[i][j]);}

// The 16 possible tetrahedron patterns for a tet polygon

static const sv_integer svc[16] = {0,3,3,4,3,4,4,3,3,4,4,3,4,3,3,0};

sv_integer sv_v_count(sv_integer i) { return(svc[i]);}

static const sv_integer stp[16][4] = {
			-1,-1,-1,-1,
			2,5,4,-1,
			1,3,5,-1,
			1,3,4,2,
			0,4,3,-1,
			2,5,3,0,
			0,4,5,1,
			0,2,1,-1,
			0,1,2,-1,
			0,1,5,4,
			0,3,5,2,
			0,3,4,-1,
			1,2,4,3,
			1,5,3,-1,
			2,4,5,-1,
			-1,-1,-1,-1
};

sv_integer sv_tet_pol(sv_integer i, sv_integer j) { return(stp[i][j]);}

// Compute the parametric interval of a line in a box

sv_interval line_box(const sv_line& l, const sv_box& b)
{
	sv_interval tx;
	sv_interval ty;
	sv_interval tz;
	sv_interval t;
	sv_integer i = 0;


	if (l.direction.x != 0.0)
	{
 		tx = (b.xi - l.origin.x)/l.direction.x;
		i = i|1;
	}
	if (l.direction.y != 0.0)
	{
 		ty = (b.yi - l.origin.y)/l.direction.y;
		i = i|2;
	}
	if (l.direction.z != 0.0)
	{
 		tz = (b.zi - l.origin.z)/l.direction.z;
		i = i|4;
	}

	switch(i)
	{
	case 7: t = tx & ty & tz;
		break;

	case 0: svlis_error("line_box","line has no gradient",SV_WARNING);
		break;

	case 1: if( (b.zi.member(l.origin.z) != SV_AIR) &&
			(b.yi.member(l.origin.y) != SV_AIR) )
				t = tx;
		break;

	case 2: if( (b.zi.member(l.origin.z) != SV_AIR) &&
			(b.xi.member(l.origin.x) != SV_AIR) )
				t = ty;
		break;

	case 3: if( b.zi.member(l.origin.z) != SV_AIR )
			t = tx & ty;
		break;

	case 4: if( (b.xi.member(l.origin.x) != SV_AIR) &&
			(b.yi.member(l.origin.y) != SV_AIR) )
				t = tz;
		break;

	case 5: if( (b.yi.member(l.origin.y) != SV_AIR) )
			t = tz & tx;
		break;

	case 6: if( (b.xi.member(l.origin.x) != SV_AIR) )
			t = tz & ty;
		break;

	default:
		svlis_error("line_box", "the | operator doesn't work on integers",SV_CORRUPT);
	}

	return(sv_interval(t));
}

// Compute the part of a parametric interval on a line inside a plane

sv_interval l_int_plane(const sv_line& l, sv_plane& f, const sv_interval& i)
{
	sv_i result = i.interval();
	flag_val flag;

	if (i.empty()) return(sv_interval(result));

	sv_real t = plane_line_t(f,l, flag);
	if (flag) return(result);

	sv_point l_start = line_point(l,i.lo());
	sv_point l_end = line_point(l,i.hi());
	if (f.value(l_start) < f.value(l_end))
		result.hi = min(t,i.hi());
	else
		result.lo = max(t,i.lo());

	return(sv_interval(result));
}


// Work out the `principal components' of a box that (usually) contains
// a range of grad vectors. 

// *major is the major axis measured along *ma, *minor is the minor axis
// BSP_C on return means that nothing useful is returned as the centroid of
// the box is the origin.
// BSP_S means that the box contains the origin, but everything will be set.
// BSP_OK means everythin will be set
// BSP_DUD means an error has occured.

bsp_type box_spread(const sv_box& b, sv_point* cen, sv_point* ma, sv_real* major, 
		sv_real* minor)
{
	*cen = b.centroid();
	sv_real c_len = cen->mod();

	if (c_len == 0.0) return(BSP_C);

	sv_point p,q,u,v,s;
	sv_real su2 = 0.0;
	sv_real sv2 = 0.0;
	sv_real suv = 0.0;
	sv_real u_c, v_c, ab, apb, root;

	c_len = 1/c_len;

	sv_axes(*cen, u, v, s);

	for (short i = 0; i < 8; i++)
	{
		q = b.corner(i);
		u_c = u*q;
		v_c = v*q;
		su2 = su2 + u_c*u_c;
		sv2 = sv2 + v_c*v_c;
		suv = suv + u_c*v_c;
	}

	c_len = c_len*c_len;
	su2 = c_len*su2;
	sv2 = c_len*sv2;
	suv = c_len*suv;

	apb = su2 + sv2;
	ab = su2*sv2;
	root = apb*apb - 4*(ab - suv*suv);

	if (root < 0.0)
	{
		if (root > -QUITE_SMALL*ab) 
			root = 0.0;
		else
		{
			svlis_error("box_spread","imaginary roots", SV_WARNING);
			return(BSP_DUD);
		}
	}

	root = (sv_real)sqrt(root);

// Bigger and smaller eigenvalue

	*major = (apb + root)/2;
	*minor = (apb - root)/2;

	if (*minor < 0.0)
	{
		if (*minor > -QUITE_SMALL*root) 
			*minor = 0.0;
		else
		{
			svlis_error("box_spread","negative eigenvalue",SV_WARNING);
			return(BSP_DUD);
		}
	}


// Bigger eigenvector

	sv_real b1 = su2 + suv - *major;
	sv_real b2 = suv + sv2 - *major;
	b1 = b1*b1;
	b2 = b2*b2;
	b1 = b1 + b2;
	if (b1 == 0)
	{
		b1 = 1;	// Bit of a hack
		b2 = 0;
	}
	b1 = b2/b1;
	b2 = (sv_real)sqrt(1 - b1);
	b1 = (sv_real)sqrt(b1);
	*ma = b1*u + b2*v;

	if ( (b.xi.member() == SV_SURFACE) && (b.yi.member() == SV_SURFACE) &&
			(b.zi.member() == SV_SURFACE) )
		return(BSP_S);
	else
		return(BSP_OK);	
}

#if macintosh
 #pragma export off
#endif


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
 *   SvLis geometry procedures
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

void write(ostream& s, const sv_point& p, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_POINT, 0, 0);
	put_token(s, SVT_OB, 0, 0);
	put_token(s, SVT_REAL, 0, p.x);
	s << ", ";
	put_token(s, SVT_REAL, 0, p.y);
	s << ", ";
	put_token(s, SVT_REAL, 0, p.z);
	put_token(s, SVT_CB, 0, 0);
}

ostream& operator<<(ostream& s, const sv_point& p)
{
	write(s,p,0);
	return(s);
}

istream& operator>>(istream& s, sv_point& p)
{
	if(get_read_version() == SV_VER) 
		read(s,p);
	else
	{
		if(get_read_version() != (SV_VER-1))
			svlis_error("operator>> sv_point",
			"file version is too old",
			SV_WARNING);
		read1(s,p);
	}

	return(s);
}

void write(ostream& s, const sv_line& l, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_LINE, 0, 0);
	put_token(s, SVT_OB, 0, 0);
	write(s, l.direction, 0);
	s << ", ";
	write(s, l.origin, 0);
	put_token(s, SVT_CB, 0, 0);
}

ostream& operator<<(ostream& s, const sv_line& l)
{
	write(s, l, 0);
	return(s);
}

istream& operator>>(istream& s, sv_line& l)
{
	if(get_read_version() == SV_VER)
		read(s, l);
	else
	{
		if(get_read_version() != (SV_VER-1))
			svlis_error("operator>> sv_line",
			"file version is too old",
			SV_WARNING);
		read1(s, l);
	}
	return(s);
}

void write(ostream& s, const sv_plane& f, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_PLANE, 0, 0);
	put_token(s, SVT_OB, 0, 0);
	write(s, f.normal, 0);
	s << ", ";
	put_token(s, SVT_REAL, 0, f.d);
	put_token(s, SVT_CB, 0, 0);
}

ostream& operator<<(ostream& s, const sv_plane& f)
{
	write(s, f, 0);
	return(s);
}

istream& operator>>(istream& s, sv_plane& f)
{
	if(get_read_version() == SV_VER)
		read(s, f);
	else
	{
		if(get_read_version() != (SV_VER-1))
			svlis_error("operator>> sv_plane",
			"file version is too old",
			SV_WARNING);
		read1(s, f);
	}
	return(s);
}

// Normalization of a point

sv_point sv_point::norm() const
{
	sv_real b = this->mod();
	sv_point result = *this;

	if (b == 0.0)
	{
		svlis_error("sv_point::norm()",
			"attempt to normalize zero-size point", SV_WARNING);
	}
	else
		result = result/b;
	return(result);
}

// Rotate one point (p) about another (a)

sv_point pp_spin(const sv_point& a, const sv_point& p, sv_real angle)
{
	sv_point axis, u, v;
	
	sv_axes(a, u, v, axis);

// Find the components of the new point in those coordinates

     sv_real along = axis*p;
     sv_real u_comp = u*p;
     sv_real v_comp = v*p;

// Rotate the point (along stays unaltered) in the (u,v) plane.

     sv_real s = (sv_real)sin(angle);
     sv_real c = (sv_real)cos(angle);
     u = u*(c*u_comp - s*v_comp);
     v = v*(s*u_comp + c*v_comp);

	 return(along*axis + u + v);
}

// Reflect a point (p) in an origin plane perpendicular to another (m)

sv_point pp_mirror(const sv_point& m, const sv_point& p)
{
	sv_point u, v, w;
	
	sv_axes(m, u, v, w);

// Find the components of the new point in those coordinates

	sv_real along = w*p;
	sv_real u_comp = u*p;
	sv_real v_comp = v*p;
	return(u*u_comp + v*v_comp - w*along);
}

// Point transformations

sv_point sv_point::spin(const sv_line& l, sv_real angle) const
{
	return(l.origin + pp_spin(l.direction, *this - l.origin, angle));
}

sv_point sv_point::mirror(const sv_plane& m) const
{
	return(*this - 2*m.value(*this)*m.normal);  // Work it out!
}

sv_point sv_point::scale(const sv_point& c, sv_real s) const
{
	return(c + (*this - c)*s);
}

sv_point sv_point::scale(const sv_line& s_ax, sv_real s) const
{
        sv_point v = *this - s_ax.origin;
	v = (v*s_ax.direction)*s*s_ax.direction;
	return(*this + v);
}

// Unique tag

sv_integer sv_point::tag() const
{
	return(SVT_F*SVT_POINT);
}
 
// Line transformations

sv_line sv_line::spin(const sv_line& l, sv_real angle) const
{
     return( sv_line( pp_spin(l.direction,this->direction,angle),
     	this->origin.spin(l,angle) ) );
}

sv_line sv_line::mirror(const sv_plane& m) const
{
	return( sv_line( pp_mirror(m.normal, this->direction),
		this->origin.mirror(m) ) );
}

// NB line scaling leaves the direction vector normalized

sv_line sv_line::scale(const sv_point& c, sv_real s) const
{
	sv_line q;
	q.direction = this->direction;      
	q.origin = q.origin.scale(c, s);
	return(q);
}

sv_line sv_line::scale(const sv_line& s_ax, sv_real s) const
{
	sv_line q;
	sv_point p = this->origin + this->direction*10;
	p = p.scale(s_ax, s);      
	q.origin = this->origin.scale(s_ax, s);
	q.direction = (p - q.origin).norm();
	return(q);
}

// Unique tag

sv_integer sv_line::tag() const
{
	return(SVT_F*SVT_LINE);
}

// Plane transformations

sv_plane sv_plane::spin(const sv_line& l, sv_real angle) const
{

// Translate the plane so that the origin of the line is a local origin

     sv_plane q = *this - l.origin;
     sv_point new_norm = pp_spin(l.direction, q.normal, angle);
     return(sv_plane(new_norm, q.d) + l.origin);
}

sv_plane sv_plane::mirror(const sv_plane& m) const
{
	sv_point n = pp_mirror(m.normal, this->normal);
	sv_point in = -this->d*(this->normal);
	in = in.mirror(m);
	return(sv_plane(n,in));	
}

// NB plane scaling leaves the normal vector normalized

sv_plane sv_plane::scale(const sv_point& c, sv_real s) const
{
	return(sv_plane(this->normal, this->d + (s - 1)*(this->value(c))));
}

sv_plane sv_plane::scale(const sv_line& s_ax, sv_real s) const
{
        sv_point a, b, c;
	sv_axes(this->normal, b, c, a);
	a = -a*this->d;
	b = a + b*10;
	c = a + c*10;
	a = a.scale(s_ax, s);
	b = b.scale(s_ax, s);
	c = c.scale(s_ax, s);
	return(sv_plane(a, b, c));
}

// Unique tag

sv_integer sv_plane::tag() const
{
	return(SVT_F*SVT_PLANE);
}

// Line of intersection between two planes; the line returned has its
// point nearest (0,0,0) as its origin.

// see A Programmer's Geometry p 114

sv_line planes_line(const sv_plane& a, const sv_plane& b, flag_val& flag)
{

	flag = NO_FLAG;
	set_svlis_flag(flag);
	sv_point p1 = a.normal;
	sv_point p2 = b.normal;
	sv_point g = p1^p2;
	sv_real det = g*g;
	if (det == 0.0)
	{
		flag = NO_CROSS;
		set_svlis_flag(flag);
		return(sv_line(SV_X,SV_OO));
	}
	det = 1/det;
	sv_real d1 = a.d;
	sv_real d2 = b.d;
	sv_real dc = d1*p2.z - p1.z*d2;
	sv_real db = d1*p2.y - p1.y*d2;
	sv_real ad = p1.x*d2 - p2.x*d1;

	sv_point ori = sv_point((g.y*dc - g.z*db), -(g.x*dc + g.z*ad),
			(g.x*db + g.y*ad));
	ori = ori*det;
	set_svlis_flag(flag);
	return(sv_line(g,ori));
}

// Intersection between a line and a plane

sv_real plane_line_t(const sv_plane& f, const sv_line& l, flag_val& flag)
{
	sv_real denom = f.normal*l.direction;

	if (denom == 0.0)
	{
		flag = NO_CROSS;
		set_svlis_flag(flag);
		return(0.0);
	}else
		flag = NO_FLAG;
	set_svlis_flag(flag);
	return(-(f.normal*l.origin + f.d)/denom);
}


// Intersection of three planes

// See A Programmer's Geometry, p112

sv_point planes_point(const sv_plane& f, const sv_plane& g, const sv_plane& h, flag_val& flag)
{
	sv_point p = (g.normal)^(h.normal);

	sv_real det = f.normal.x*p.x - f.normal.y*p.y + f.normal.z*p.z;

	if (det == 0.0)
	{
		flag = NO_CROSS;
		set_svlis_flag(flag);
		return(p);
	}else
		flag = NO_FLAG;

	set_svlis_flag(flag);

	det = 1/det;

	sv_real dc = g.d*h.normal.z - h.d*g.normal.z;
	sv_real db = g.d*h.normal.y - h.d*g.normal.y;
	sv_real ad = g.normal.x*h.d - h.normal.x*g.d;

	sv_real x = (f.normal.y*dc - f.d*p.x - f.normal.z*db)*det;
	sv_real y = (f.d*p.y - f.normal.x*dc - f.normal.z*ad)*det;
	sv_real z = (f.normal.y*ad + f.normal.x*db - f.d*p.z)*det;

	return(sv_point(x,y,z));
}
#if macintosh
 #pragma export off
#endif

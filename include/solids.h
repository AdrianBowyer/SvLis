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
 * SvLis - commonly-used solids
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 16 March 1993
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_SOLIDS
#define SVLIS_SOLIDS

// The common solids

// A tetrahedron with vertices at four points v[0...3]
// The source for this is in set.cxx

extern sv_set tetrahedron(sv_point v[]);

//  This returns a cuboid aligned with the coordinate axes.  Its
//  `bottom left-hand' point is at low, its `top right-hand' at
//  high.

inline sv_set cuboid(const sv_point& l, const sv_point& h)
{
	sv_set low, high;
    
	low = sv_set(sv_primitive(sv_plane(-SV_X,l))) & sv_set(sv_primitive(
	    sv_plane(-SV_Y,l))) & sv_set(sv_primitive(sv_plane(-SV_Z,l)));
	high = sv_set(sv_primitive(sv_plane(SV_X,h))) & sv_set(sv_primitive(
	    sv_plane(SV_Y,h))) & sv_set(sv_primitive(sv_plane(SV_Z,h)));
	return (low & high);     
}

inline sv_set cuboid(const sv_box& b)
{

// This makes a cuboid out of a box

	sv_point l = b.corner(0);
	sv_point h = b.corner(7);

	return(cuboid(l,h));
}

// Note all the hacking about needed in here . . .

inline sv_set thin_cuboid(const sv_point& l, const sv_point& h)
{
	sv_primitive p0x = sv_primitive(sv_plane(-SV_X,l));
	sv_set s0x = sv_set(p0x);
	sv_primitive p0y = sv_primitive(sv_plane(-SV_Y,l));
	sv_set s0y = sv_set(p0y);
	sv_primitive p0z = sv_primitive(sv_plane(-SV_Z,l));
	sv_set s0z = sv_set(p0z);
	sv_primitive p1x = sv_primitive(sv_plane(SV_X,h));
	sv_set s1x = sv_set(p1x);
	sv_primitive p1y = sv_primitive(sv_plane(SV_Y,h));
	sv_set s1y = sv_set(p1y);
	sv_primitive p1z = sv_primitive(sv_plane(SV_Z,h));
	sv_set s1z = sv_set(p1z);

	sv_set result = ( sv_set(abs(p0x)) | sv_set(abs(p1x)) ) & s0y & s1y & s0z & s1z;
	result = result | (   ( sv_set(abs(p0y)) | sv_set(abs(p1y)) )
			& s0x & s1x & s0z & s1z   );
	result = result | (   ( sv_set(abs(p0z)) | sv_set(abs(p1z)) )
			& s0x & s1x & s0y & s1y   );

	return(result);
}

inline sv_set thin_cuboid(const sv_box& b)
{
	sv_point l = b.corner(0);
	sv_point h = b.corner(7);

	return(thin_cuboid(l,h));
}


//  This returns an infinitely long cylinder aligned with the line axis
//   and of radius radius.

inline sv_set cylinder(const sv_line& axis, sv_real radius)
{
	return(sv_set(p_cylinder(axis,radius)));
}

inline sv_set thin_cylinder(const sv_line& axis, sv_real radius)
{
	return(sv_set(abs(p_cylinder(axis,radius))));
}


//  This returns a cone aligned with axis, and with apex at the origin of
//  axis.  The cone points in the direction of axis's direction vector.
//  The included angle of the cone is angle.

inline sv_set cone(const sv_line& axis, sv_real angle)
{
	sv_plane chop = sv_plane(axis.direction, axis.origin);
	return(sv_set(p_cone(axis,angle)) & sv_set(sv_primitive(chop)));
}

inline sv_set thin_cone(const sv_line& axis, sv_real angle)
{
	sv_plane chop = sv_plane(axis.direction, axis.origin);
	return(sv_set(abs(p_cone(axis,angle))) & sv_set(sv_primitive(chop)));
}


// A sphere . . .

inline sv_set sphere(const sv_point& centre, sv_real radius)
{
	return(sv_set(p_sphere(centre,radius)));
}

inline sv_set thin_sphere(const sv_point& centre, sv_real radius)
{
	return(sv_set(abs(p_sphere(centre,radius))));
}


//  This returns a torus with its major circle in a plane through the
//  origin of the line axis and perpendicular to axis.  tor_rad is the
//  major radius, tor_sect_rad the minor.

inline sv_set torus(const sv_line& axis, sv_real tor_rad, sv_real tor_sect_rad)
{
	return(sv_set(p_torus(axis,tor_rad,tor_sect_rad)));
}

inline sv_set thin_torus(const sv_line& axis, sv_real tor_rad, sv_real tor_sect_rad)
{
	return(sv_set(abs(p_torus(axis,tor_rad,tor_sect_rad))));
}

//  This returns a cyclide with its major circle in a plane through the
//  origin of the line axis and perpendicular to axis.
//  sym is a vector that defines the plane of symmetry - it _must_ not
//  be parallel to the axis.  
//  a is the major cyclide radius, m the minor +/- c.


inline sv_set cyclide(const sv_line& axis, const sv_point& sym, sv_real a, sv_real m, sv_real c)
{
	return(sv_set(p_cyclide(axis,sym,a,m,c)));
}

inline sv_set thin_cyclide(const sv_line& axis, const sv_point& sym, sv_real a, sv_real m, sv_real c)
{
	return(sv_set(abs(p_cyclide(axis,sym,a,m,c))));
}

#endif

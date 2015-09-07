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
 * SvLis - user-defined hard-coded primitives
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 9 August 1993
 * This version: 8 March 2000
 *
 */


/*
 *  This file is the interface between svLis and any half-spaces that
 *  the user wants to code up in C++.  The example is a sinusoidal
 *  wave pattern.
 *
 */

#include "svlis.h"
#if macintosh
 #pragma export on
#endif

#define SIN_SHEET 2000

// Input and Output

sv_primitive read_user(istream& s, sv_integer up)
{
	sv_primitive result;

	switch (up)
	{
	default:
		svlis_error("read_user","attempt to read undefined primitive", SV_CORRUPT);
	}

	return(result);
}

void write_user(ostream& s, sv_integer up)
{
	switch (up)
	{
	default:
		svlis_error("write_user","attempt to write undefined primitive", SV_WARNING);
	}
}


// Return the bogus degree value

sv_integer degree_user(sv_integer up)
{
	return(2); // well, why not?
}

// Translate a primitive

sv_primitive translate_user(sv_integer up, const sv_point& q)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("translate_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Rotate about a line

sv_primitive spin_user(sv_integer up, const sv_line& l, sv_real angle)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("spin_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Mirror in a plane

sv_primitive mirror_user(sv_integer up, const sv_plane& m)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("mirror_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Scale

sv_primitive scale_user(sv_integer up, const sv_point& c, sv_real s)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("scale_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

sv_primitive scale_user(sv_integer up, const sv_line& c, sv_real s)
{
	sv_primitive result;

	switch(up)
	{
	default:
		svlis_error("scale_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Value of a primitive for a point

sv_real value_user(sv_integer up, const sv_point& q)
{
	sv_real result = 0.0;

	switch(up)
	{
	case SIN_SHEET:
		return(q.z - (sv_real)sin(q.x));

	case SIN_SHEET+1:
		return((sv_real)-cos(q.x));

	case SIN_SHEET+2:
		return(0.0);

	case SIN_SHEET+3:
		return(1.0);

	case SIN_SHEET+4:
		return((sv_real)sin(q.x) - q.z);

	case SIN_SHEET+5:
		return((sv_real)cos(q.x));

	case SIN_SHEET+6:
		return(0.0);

	case SIN_SHEET+7:
		return(-1.0);

	default:
		svlis_error("value_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Value of a box in a primitive

sv_interval range_user(sv_integer up, const sv_box& b)
{
	sv_interval result;

	switch(up)
	{
	case SIN_SHEET:
		return(b.zi - sin(b.xi));

	case SIN_SHEET+1:
		return(-cos(b.xi));

	case SIN_SHEET+2:
		return(sv_interval(0.0,0.0));

	case SIN_SHEET+3:
		return(sv_interval(1.0,1.0));

	case SIN_SHEET+4:
		return(sin(b.xi) - b.zi);

	case SIN_SHEET+5:
		return(cos(b.xi));

	case SIN_SHEET+6:
		return(sv_interval(0.0,0.0));

	case SIN_SHEET+7:
		return(sv_interval(-1.0,-1.0));

	default:
		svlis_error("range_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

// Complement a primitive

sv_primitive complement_user(sv_integer up)
{
	sv_primitive result;

	switch(up)
	{
	case SIN_SHEET:
		return(sv_primitive(SIN_SHEET+4, SIN_SHEET+5, SIN_SHEET+6,
			SIN_SHEET+7));
	default:
		svlis_error("complement_user","dud tag supplied",SV_WARNING);
	}

	return(result);
}

#if macintosh
 #pragma export off
#endif












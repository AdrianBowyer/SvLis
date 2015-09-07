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
 * SvLis - model divider decision procedures
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 5 March 1993
 * This version: 23 September 2000
 *
 */

#include "svlis.h"
#if macintosh
 #pragma export on
#endif

/* You need to supply the divider with two functions which are below:
 *
 *	sv_real user_little_box();
 *
 *		This should return the volume of the smallest box that 
 *		you want to create.  This might be, say, 0.0001 times
 *		the volume of the box around the original model.  You
 *		can get that original model using
 *
 *			sv_model m = root_model();
 *
 *	sv_integer user_low_contents();
 *
 *		This should return a count of primitives considered low
 *		enough for a box containing them not to be divided
 *		further.  3 is a sensible value.  Using that Svlis
 *		will (more or less) divide down until boxes contain
 *		no more than corners of objects.
 *
 */

static sv_real volume_factor = SV_SMALL_BOX;

void set_small_volume(sv_real f) { volume_factor = SV_SMALL_BOX*f; }

sv_real get_small_volume() { return(volume_factor/SV_SMALL_BOX); }

static sv_integer low_contents = SV_LOW_CONTENTS;

void set_low_contents(sv_integer i) { low_contents = i; }
 
sv_real user_little_box()
{
	sv_box b = root_model().box();
	return(volume_factor*b.vol());
}

sv_integer user_low_contents() { return(low_contents); }

// Smart division trategies:

//	SV_MIN_MIN: minimise the minimum box contents - gives concentrations of complexity
//			and large amounts of empty boxes
//	SV_MIN_MAX: minimize the maximum box contents - evens out complexity

static sv_integer smart_strat = SV_MIN_MIN;

void set_smart_strategy(sv_integer s) { smart_strat = s; }

sv_integer get_smart_strategy() { return(smart_strat); }

// Divide each box along its longest side until the contents are <= low_contents
// or the volume is <=  SV_SMALL_BOX*volume_factor

void dumb_decision(const sv_model& m, sv_integer level, void* vp, mod_kind* k, sv_real* c, 
		sv_model* c_1, sv_model* c_2)
{

	sv_box mb = m.box();
	sv_set_list sl = m.set_list();
	sv_integer dont_divide = 1;
	sv_integer contents;

// Check each set in the list to see if it has enough contents to make 
// further division needed.

	while(sl.exists() && dont_divide)
	{
		contents = sl.set().contents();
		if (contents > user_low_contents()) dont_divide = 0;
		sl = sl.next();
	}

// If all the sets were simple enough, don't divide further.

	if(dont_divide)
	{
		*k = LEAF_M;
		return;
	}

// We may need to divide further.
// How big is the box?

// If the box is too small, don't divide it further

	if (mb.vol() < user_little_box())
	{
		*k = LEAF_M;
		return;
	}

// Divide the box in half along its longest edge.

	sv_real x = mb.xi.hi() - mb.xi.lo();
	sv_real y = mb.yi.hi() - mb.yi.lo();
	sv_real z = mb.zi.hi() - mb.zi.lo();

	if (x > y)
	{
		if (z > x)
		{
			*k = Z_DIV;
			*c = mb.zi.lo() + z/2;
		} else
		{
			*k = X_DIV;
			*c = mb.xi.lo() + x/2;
		}
	} else
	{
		if (z > y)
		{
			*k = Z_DIV;
			*c = mb.zi.lo() + z/2;
		} else
		{
			*k = Y_DIV;
			*c = mb.yi.lo() + y/2;
		}
	}
	return;
}

// Do all possible even divisions and choose the one that minimises
// the maximum contents

void smart_decision(const sv_model& m, sv_integer level, void* vp, mod_kind* k, sv_real* c, 
		sv_model* c_1, sv_model* c_2)
{

	sv_box mb = m.box();
	sv_set_list sl = m.set_list();
	sv_integer dont_divide = 1;
	sv_integer contents;

// Check each set in the list to see if it has enough contents to make 
// further division needed.

	while(sl.exists() && dont_divide)
	{
		contents = sl.set().contents();
		if (contents > user_low_contents()) dont_divide = 0;
		sl = sl.next();
	}

// If all the sets were simple enough, don't divide further.

	if(dont_divide)
	{
		*k = LEAF_M;
		return;
	}

// We may need to divide further.
// How big is the box?

// If the box is too small, don't divide it further

	if (mb.vol() < user_little_box())
	{
		*k = LEAF_M;
		return;
	}

// Divide in each direction in turn

	sl = m.set_list();
	sv_real swell = 1 + get_swell_fac();

// X_DIV

	sv_interval x = mb.xi;
	sv_interval y = mb.yi;
	sv_interval z = mb.zi;

	sv_real x_cut = (x.hi() - x.lo())*0.5;

	sv_interval i_part = sv_interval(x.lo(), x.lo() + x_cut*swell);
        sv_box b_part = sv_box(i_part, y, z);
	sv_model x_1 = sv_model(sl, b_part, m);
	sv_integer x_1c = x_1.set_list().contents();

	i_part = sv_interval(x.hi() - x_cut*swell, x.hi());
	b_part = sv_box(i_part, y, z);
	sv_model x_2 = sv_model(sl, b_part, m);
	sv_integer x_2c = x_2.set_list().contents();

// If we've hit a 0 return immediately

	if((!x_1c) || (!x_2c))
	{
		*c_1 = x_1;
		*c_2 = x_2;
		*c = x.lo() + x_cut;
		*k = X_DIV;
		return;
	}

// Y_DIV

	sv_real y_cut = (y.hi() - y.lo())*0.5;

	i_part = sv_interval(y.lo(), y.lo() + y_cut*swell);
        b_part = sv_box(x, i_part, z);
	sv_model y_1 = sv_model(sl, b_part, m);
	sv_integer y_1c = y_1.set_list().contents();

	i_part = sv_interval(y.hi() - y_cut*swell, y.hi());
	b_part = sv_box(x, i_part, z);
	sv_model y_2 = sv_model(sl, b_part, m);
	sv_integer y_2c = y_2.set_list().contents();

// If we've hit a 0 return immediately

	if((!y_1c) || (!y_2c))
	{
		*c_1 = y_1;
		*c_2 = y_2;
		*c = y.lo() + y_cut;
		*k = Y_DIV;
		return;
	}

// Z_DIV

	sv_real z_cut = (z.hi() - z.lo())*0.5;

	i_part = sv_interval(z.lo(), z.lo() + z_cut*swell);
        b_part = sv_box(x, y, i_part);
	sv_model z_1 = sv_model(sl, b_part, m);
	sv_integer z_1c = z_1.set_list().contents();

	i_part = sv_interval(z.hi() - z_cut*swell, z.hi());
	b_part = sv_box(x, y, i_part);
	sv_model z_2 = sv_model(sl, b_part, m);
	sv_integer z_2c = z_2.set_list().contents();

// If we've hit a 0 return immediately

	if((!z_1c) || (!z_2c))
	{
		*c_1 = z_1;
		*c_2 = z_2;
		*c = z.lo() + z_cut;
		*k = Z_DIV;
		return;
	}


// Divide the box to give the minimum maximum contents, or the
// minimum minimum contents.

	sv_integer xm;
	sv_integer ym;
	sv_integer zm;
	
	switch(smart_strat)
	{
	case SV_MIN_MIN:
		xm = min(x_1c, x_2c);
		ym = min(y_1c, y_2c);
		zm = min(z_1c, z_2c);
		break;

	case SV_MIN_MAX:
		xm = max(x_1c, x_2c);
		ym = max(y_1c, y_2c);
		zm = max(z_1c, z_2c);
		break;

	default:
		svlis_error("smart_decision","dud smart strategy", SV_WARNING);
	}

	if (xm > ym)
	{
		if (ym > zm)
		{
			*c_1 = z_1;
			*c_2 = z_2;
			*k = Z_DIV;
			*c = z.lo() + z_cut;
		} else
		{
			if ( (ym == zm) && (y_cut < z_cut) )
			{
				*c_1 = z_1;
				*c_2 = z_2;
				*k = Z_DIV;
				*c = z.lo() + z_cut;
				return;
			}

			*c_1 = y_1;
			*c_2 = y_2;
			*k = Y_DIV;
			*c = y.lo() + y_cut;
		}
	} else
	{
		if (xm > zm)
		{
			*c_1 = z_1;
			*c_2 = z_2;
			*k = Z_DIV;
			*c = z.lo() + z_cut;
		} else
		{
			if( (xm == zm) && (x_cut < z_cut) )
			{
				*c_1 = z_1;
				*c_2 = z_2;
				*k = Z_DIV;
				*c = z.lo() + z_cut;
				return;
			}

			if( (xm == ym) && (x_cut < y_cut) )
			{
				*c_1 = y_1;
				*c_2 = y_2;
				*k = Y_DIV;
				*c = y.lo() + y_cut;
				return;
			}

			*c_1 = x_1;
			*c_2 = x_2;
			*k = X_DIV;
			*c = x.lo() + x_cut;
		}
	}
	return;
}

#if macintosh
 #pragma export off
#endif

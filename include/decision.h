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
 * SvLis - Functions to take division decisions, provide constants 
 * and so on
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 22 March 1993
 * This version: 8 March 2000
 *
 */

#ifndef SVLIS_DECISIONS
#define SVLIS_DECISIONS

// See decision.cxx in the sv_user directory

extern sv_integer user_low_contents();
extern sv_real user_little_box();
extern void set_small_volume(sv_real);
extern void set_low_contents(sv_integer);
extern sv_real get_small_volume();

extern void dumb_decision(const sv_model&, sv_integer, void*,  
    mod_kind*, sv_real*, sv_model*, sv_model*);

enum sm_strat
{
	SV_MIN_MIN,
	SV_MIN_MAX
};

extern void smart_decision(const sv_model&, sv_integer, void*,
    mod_kind*, sv_real*, sv_model*, sv_model*);

extern void set_smart_strategy(sv_integer);

extern sv_integer get_smart_strategy();

#endif

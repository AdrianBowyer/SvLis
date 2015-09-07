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
 * SvLis - error processing
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 2 February 1995
 * This version: 8 March 2000
 *
 */


/*
 *
 *  This file contains the error procedure that Svlis calls for all errors
 *  which it detects.  You can edit this procedure so that you can intercept
 *  these errors and do whatever you like with them.  As supplied this file
 *  allows you to set the level of the errors that will be reported, and
 *  then calls sv_error_int, which actually reports the error.
 *  sv_error_int always returns.
 *
 */


#include "sv_std.h"
#include "enum_def.h"
#include "sums.h"
#include "flag.h"
#if macintosh
 #pragma export on
#endif

// Default reporting level is SV_WARNING

sv_err lev = SV_WARNING;

// User's error-intercept procedure

void svlis_error(char* p, char* mess, sv_err l)
{
	if (l < lev) return;
	sv_error_int(p, mess, l);
}
#if macintosh
 #pragma export off
#endif


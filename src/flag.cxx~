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
 * Svlis flag.cxx file
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 5 May 1993
 * This version: 8 March 2000
 *
 */

#include "sv_std.h"
#include "enum_def.h"
#include "sums.h"
#include "flag.h"
#if macintosh
 #pragma export on
#endif

// There are many occasions when an operation may `fail', though it is
// perfectly legitimate to attempt it (such as trying to find the line
// of intersection between two parallel planes).  Svlis uses a flagging 
// mechanism to record this.  This file, and flag.h, are the flag 
// mechanism

flag_val svlis_flag;

// This is the standard minimal internal error-reporting procedure 
// which the user may call from svlis_error.

void sv_error_int(char* p, char* mess, sv_err l)
{	
	cerr << "SvLis ";
	switch(l)
	{
	case SV_DEBUG: cerr << "debug message"; break;
	case SV_COMMENT: cerr << "comment message"; break;
	case SV_WARNING: cerr << "warning message"; break;
	case SV_FATAL: cerr << "fatal error"; break;
	case SV_CORRUPT: cerr << "corruption error"; break;
	default: cerr << "end-of-the-world error";
	}
	cerr << " from procedure " << p << ": " << mess << "." << SV_EL;
}
#if macintosh
 #pragma export off
#endif
 

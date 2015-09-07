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
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   Licence as published by the Free Software Foundation; either
 *   version 2 of the Licence, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public Licence for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   Licence along with this library; if not, write to the Free
 *   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA,
 *   or see
 *
 *      http://www.gnu.org/
 * 
 * =====================================================================
 *
 * Initializing and terminating functions.
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 15 March 1993
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
 
 
int out_of_store(int dummy) 
{ 
	svlis_error("could be anywhere. . .", "run out of memory", SV_FATAL); 
	return(0); 
} 
 
void svlis_init() 
{ 
#if macintosh
        MaxApplZone();
        SIOUXSettings.autocloseonquit=false; 
        SIOUXSettings.showstatusline = true; 
        SIOUXSettings.toppixel=400; 
        SIOUXSettings.leftpixel=400; 
        cout << "SvLis: starting..." << SV_EL << SV_EL;
#endif 
//	licence_check(); 
//	_set_new_handler(_PNH &out_of_store); 
} 

int svlis_end(int i)
{
    char dummy;
 
    cout << "SvLis: type any character to finish: "; 
    cin >> dummy;

// SvLis has left the building...

    return(i);
}

#if macintosh 
 #pragma export off 
#endif 

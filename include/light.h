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
 * SvLis - light sources header file
 *
 * Andy Wallis
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: May 1994 
 * This version: 8 March 2000
 *
 */

#ifndef SVLIS_LIGHT
#define SVLIS_LIGHT

// The light class stores lighting parameters that are used to 
// generate an image


class sv_lightsource
{
   light_type tp;
   sv_point loc;		// For POINT_SOURCE only
   sv_point col;		// RGB intensities of the light
   sv_point dir;		//
   sv_real angle_p;		// power to raise (direction . vector-from-surface-point) to (for POINT_SOURCE only)

 public:

// Constructors and Destructor for a light

   sv_lightsource();
   sv_lightsource(const sv_lightsource &);		// Copy constructor
   ~sv_lightsource();

// Assignment operator

   sv_lightsource operator=(const sv_lightsource &);

// Member-setting functions

   void type(light_type type);
   void location(sv_point location);
   void colour(sv_point colour);
   void direction(sv_point direction);
   void angle_power(sv_real angle_power);

// Member accessing functions

   light_type type(void) const;
   sv_point location(void) const;
   sv_point colour(void) const;
   sv_point direction(void) const;
   sv_real angle_power(void) const;

   sv_integer tag() const {return(SVT_F*SVT_LSOURCE);}

// Debug print for light

   friend void debug_print_lightsource(const sv_lightsource& light, char* msg);
};


struct sv_light_list {
   sv_lightsource *source;
   char *name;
   sv_light_list* next;

   sv_integer tag() const {return(SVT_F*SVT_LLIST);}
 
};

#endif

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
 * SvLis  RAYTRACER - light function file
 *
 *	Andy Wallis
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: July 1994
 * This version: 8 March 2000
 *
 */

// The light class stores light parameters that are used to generate an image


#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "light.h"
#if macintosh
 #pragma export on
#endif

// Constructors

sv_lightsource::sv_lightsource()
{
   tp = PARALLEL;
   col = sv_point(1,1,1);
   dir = -SV_Z;
   loc = SV_OO;
   angle_p = 0.3;
}


// Copy constructor

sv_lightsource::sv_lightsource(const sv_lightsource &src)
{
   tp = src.tp;
   loc = src.loc;
   col = src.col;
   dir = src.dir;
   angle_p = src.angle_p;
}

// Destructor (nothing to do here)

sv_lightsource::~sv_lightsource()
{
   ;
}


// Assignment operator

sv_lightsource
sv_lightsource::operator=(const sv_lightsource &src)
{
// Do we need an a = a fix in here? AB
   tp = src.tp;
   loc = src.loc;
   col = src.col;
   dir = src.dir;
   angle_p = src.angle_p;

   return *this;
}


// Member-setting functions

void
sv_lightsource::type(light_type t)
{
   this->tp = t;
}

void
sv_lightsource::location(sv_point location)
{
   this->loc = location;
}

void
sv_lightsource::colour(sv_point colour)
{
   this->col = colour;
}

void
sv_lightsource::direction(sv_point direction)
{
   this->dir = direction.norm();
}

void
sv_lightsource::angle_power(sv_real angle_power)
{
   this->angle_p = angle_power;
}


// Member accessing functions

light_type
sv_lightsource::type() const
{
   return this->tp;
}

sv_point
sv_lightsource::location() const
{
   return this->loc;
}

sv_point
sv_lightsource::colour() const
{
   return this->col;
}

sv_point
sv_lightsource::direction() const
{
   return this->dir;
}

sv_real
sv_lightsource::angle_power() const
{
   return this->angle_p;
}


// Debug print for lightsource

void
debug_print_lightsource(const sv_lightsource& light,
			char* msg)
{
   cout << "----- lightsource";
   if(msg) cout << msg;
//   cout << " (at " << &light << ") is:\n";
// GMB 06-12-94
   cout << " (at " << (sv_integer)&light << ") is:\n";
   switch(light.tp) {
    case PARALLEL:
      cout << "type PARALLEL\n";
      break;

    case POINT_SOURCE:
      cout << "type: POINT_SOURCE\n";
      break;

    default:
      cout << "type: *** INVALID ***\n";
      break;
   }
		   
   cout << "location: " << light.loc.x << ", " << light.loc.y << ", " << light.loc.z << "\n";
   cout << "colour: " << light.col.x << ", " << light.col.y << ", " << light.col.z << "\n";
   cout << "direction: " << light.dir.x << ", " << light.dir.y << ", " << light.dir.z << "\n";
   cout << "angle_power: " << light.angle_p << "\n";
   cout << "-----\n\n";
}

#if macintosh
 #pragma export off
#endif


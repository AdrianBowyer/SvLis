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
 * SvLis RAYTRACER - sv_view function file
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



// The sv_view class stores view parameters that are used to generate an image

#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "view.h"
#if macintosh
 #pragma export on
#endif

// Constructors

sv_view::sv_view()
{
   eye = sv_point(1,1,1);
   cen = SV_OO;
   up = SV_Z;
   lens_a = 37*M_PI/180;	// 37 degrees
}


// Copy constructor

sv_view::sv_view(const sv_view &src)
{
   eye = src.eye;
   cen = src.cen;
   up = src.up;
   lens_a = src.lens_a;
}

// Destructor (nothing to do here)

sv_view::~sv_view()
{
   ;
}


// Assignment operator

sv_view
sv_view::operator=(const sv_view &src)
{
   eye = src.eye;
   cen = src.cen;
   up = src.up;
   lens_a = src.lens_a;

   return *this;
}


// Member-setting functions

void
sv_view::eye_point(sv_point eyepoint)
{
   this->eye = eyepoint;
}

void
sv_view::centre(sv_point c)
{
   this->cen = c;
}

void
sv_view::vertical_dir(sv_point upvec)
{
   this->up = upvec;
}

void
sv_view::view_vector(sv_point vv)
{
   this->cen = this->eye+vv;
}

void
sv_view::lens_angle(sv_real angle)
{
   this->lens_a = angle;
}


// Member accessing functions

sv_point
sv_view::eye_point() const
{
   return this->eye;
}

sv_point
sv_view::centre() const
{
   return this->cen;
}

sv_point
sv_view::up_vector() const
{
   return this->up;
}

sv_point
sv_view::view_vector() const
{
   return this->cen - this->eye;
}

sv_real
sv_view::lens_angle() const
{
   return this->lens_a;
}


// Debug print for view

void
debug_print_view(const sv_view& vew,
		 char* msg)
{
   cout << "----- view";
   if(msg) cout << msg;
//   cout << " (at " << &vew << ") is:\n";
// GMB 06-12-94
   cout << " (at " << (int)&vew << ") is:\n";
   cout << "eye-point: " << vew.eye.x << ", " << vew.eye.y << ", " << vew.eye.z << "\n";
   cout << "centre: " << vew.cen.x << ", " << vew.cen.y << ", " << vew.cen.z << "\n";
   cout << "up-dir: " << vew.up.x << ", " << vew.up.y << ", " << vew.up.z << "\n";
   cout << "lens angle: " << vew.lens_a << "\n";
   cout << "-----\n\n";
}
#if macintosh
 #pragma export off
#endif

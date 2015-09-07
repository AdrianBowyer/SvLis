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
 * SvLis - RAYTRACER - view header file
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


// The view class stores view parameters that are used to generate an image

#ifndef SVLIS_VIEW
#define SVLIS_VIEW

class sv_view
{
   sv_point eye;	// The location of the eye-point
   sv_point cen;	// Centre of interest
   sv_point up;		// A vector that will point vertically up in the image
   sv_real lens_a;	// The lens-angle for the view in radians
   
 public:

// Constructors and Destructor for a view

   sv_view();
   sv_view(const sv_view&);		// Copy constructor
   ~sv_view();

// Assignment operator

   sv_view operator=(const sv_view&);

// Member-setting functions

   void eye_point(sv_point eye);
   void centre(sv_point centre);
   void vertical_dir(sv_point up);
   void view_vector(sv_point vv);
   void lens_angle(sv_real angle);

// Member accessing functions

   sv_point eye_point(void) const;
   sv_point centre(void) const;
   sv_point up_vector(void) const;
   sv_point view_vector(void) const;
   sv_real lens_angle(void) const;

   sv_integer tag() const { return(SVT_F*SVT_VIEW); }

// Debug print for view

   friend void debug_print_view(const sv_view& vew, char* msg);
};

#endif

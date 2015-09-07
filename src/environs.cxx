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
 * SVLIS RAYTRACER - surrounding colouring functions
 *
 *  Andy Wallis - August 1994
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 1 August 1994
 * This version: 8 March 2000
 *
 */

//
//  Name changed from surroundings to environs (8 char)
//  Adrian Bowyer 2/10/95
//

#include "svlis.h"
#include "view.h"
#include "light.h"
#include "ivallist.h"
#include "raytrace.h"
#include "shade.h"
#include "environs.h"
#if macintosh
 #pragma export on
#endif

static sv_point background_colour = SV_OO;	// this is NOT normalised !!!

static sv_point horizon_colour;
static sv_integer horizon_colour_set = 0;

static sv_point ground_colour;
static sv_integer ground_colour_set = 0;

static sv_point overhead_colour;
static sv_integer overhead_colour_set = 0;


void
set_background_colour(sv_real r,
		      sv_real g,
		      sv_real b,
		      sv_real i)
{
   background_colour = sv_point(r,g,b);
   if(background_colour.mod() != 0.0)
      background_colour = background_colour.norm()*i;

   if(!horizon_colour_set)
      set_horizon_colour(r,g,b,i);

   if(!ground_colour_set)
      set_ground_colour(r,g,b,i);

   if(!overhead_colour_set)
      set_overhead_colour(r,g,b,i);
}


sv_point
get_background_colour()
{
   return background_colour;
}


void
set_horizon_colour(sv_real r,
		   sv_real g,
		   sv_real b,
		   sv_real i)
{
   horizon_colour = sv_point(r,g,b);
   if(horizon_colour.mod() != 0.0)
      horizon_colour = horizon_colour.norm()*i;
   horizon_colour_set = 1;
}

void
set_ground_colour(sv_real r,
		  sv_real g,
		  sv_real b,
		  sv_real i)
{
   ground_colour = sv_point(r,g,b);
   if(ground_colour.mod() != 0.0)
      ground_colour = ground_colour.norm()*i;
   ground_colour_set = 1;
}

void
set_overhead_colour(sv_real r,
		    sv_real g,
		    sv_real b,
		    sv_real i)
{
   overhead_colour = sv_point(r,g,b);
   if(overhead_colour.mod() != 0.0)
      overhead_colour = overhead_colour.norm()*i;
   overhead_colour_set = 1;
}

sv_point get_horizon_colour() {return(horizon_colour);}
sv_point get_ground_colour() {return(ground_colour);}
sv_point get_overhead_colour() {return(overhead_colour);}

sv_point
surroundings_colour(sv_point ray_dir)
{
   sv_point result = SV_OO;
   sv_real h;

   // If the surrounding colours are set, then use them to generate a colour

   if(ray_dir.z < 0.0) 
   {
      if(ground_colour_set)
	 result = ground_colour;
   } else 
   {
      if(horizon_colour_set && overhead_colour_set) {	
	 result = horizon_colour*(1 - ray_dir.z) + overhead_colour*ray_dir.z;
      } else if(horizon_colour_set) {
	 result = horizon_colour;
      } else if(overhead_colour_set) {
	 result = overhead_colour;
      }
   }

   if(mist_active())
   {
	h = get_haze();
	result = (1-h)*result + h*get_mist();
   }

   return result;
}
#if macintosh
 #pragma export off
#endif


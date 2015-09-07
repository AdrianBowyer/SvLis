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
 * SvLis Generate a raytraced picture of a model
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
 * This version: 23 September 2000
 *
 */


#include "svlis.h"

#if macintosh
 #pragma export on
#endif

sv_integer
generate_picture(const sv_model& modl,
		 const sv_view& view_params,
		 const sv_light_list& light_list,
		 sv_picture& picture_params,
		 sv_real progress_report_step,
		 void report_procedure(sv_real percent))
{
   sv_integer ix, iy, xdif, ydif;
   sv_point ray_origin;
   sv_point ray_dir;
   sv_line ray;
   sv_interval interval;
   sv_real t;
   sv_set hit_surf;
   sv_point hit_point;
   sv_point pix_col;
   sv_pixel pixel_colour;


   // Generate vectors that are horizontal and vertical in the screen plane
   // The magnitude of the vectors is such that they represent the incremental
   // vector between adjacent pixels in the picture being generated

   sv_integer x_res = picture_params.x_resolution();
   sv_integer y_res = picture_params.y_resolution();
   sv_real la = view_params.lens_angle();

   sv_point view_vector = view_params.view_vector();
   sv_real view_const = tan((double)(la/2.0))*view_vector.mod() / (sv_real(x_res)/2.0);
   sv_point screen_h = (view_vector ^ view_params.up_vector()).norm() * view_const;
   sv_point screen_v = (view_vector ^ screen_h).norm() * view_const;
   
   ray_origin = view_params.eye_point();

   // Loop for each pixel

//   sv_integer report_interval_line_count = max(1,sv_integer(sv_real(y_res)*progress_report_step/100.0));
// GMB 06-12-94
   sv_integer report_interval_line_count = max(1,long(sv_real(y_res)*progress_report_step/100.0));
   sv_integer next_report_line = report_interval_line_count;
   sv_integer missed;
   ydif = -y_res/2;
   for(iy=0;iy<y_res;iy++) {
      xdif = -x_res/2;      
      for(ix=0;ix<x_res;ix++) {
	 // ***************************** Do we really need to normalise the ray vector?
	 ray_dir = (view_vector + xdif*screen_h + ydif*screen_v).norm();
	 missed=1;
	 ray = sv_line(ray_dir, ray_origin);
	 interval = line_box(ray, modl.box()); 
	 if(!interval.empty()) { // AB 15/5/96
	    if(interval.lo() < 0.0) interval = sv_interval(0.0, interval.hi());
	    hit_surf = modl.fire_ray(ray, interval, &t);
	    if(hit_surf.exists()) {
	       hit_point = line_point(ray,t);
	       pix_col = shade(modl, ray_dir, hit_surf, hit_point, 
		view_params, light_list, (sv_real)1.0,t);
	       missed=0;
	    }
	 }
	 if(missed)
	    pix_col = surroundings_colour(ray_dir);	// Colour using surrounding sphere

	 pixel_colour = sv_pixel(pix_col);
	 picture_params.pixel(ix,iy,pixel_colour);
	 xdif++;
      }

      if(iy == next_report_line) {
	 next_report_line += report_interval_line_count;
	 report_procedure((sv_real(iy)*100.0/sv_real(y_res)));
      }
      ydif++;
   }


   report_procedure(100.0);
   return 1;
}



void
dummy_picture_report(sv_real)
{
   ;
}

sv_integer
generate_picture(const sv_model& modl,
		 const sv_view& view_params,
		 const sv_light_list& light_list,
		 sv_picture& picture_params)
{
   return generate_picture(modl, view_params, light_list, picture_params, 0, dummy_picture_report);
}
#if macintosh
 #pragma export off
#endif

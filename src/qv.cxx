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
 * Quick view raytrace
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


//
// Rectangle list consists of a ring buffer containing records of regions of the picture waiting
// to be (re)processed.
//
// Free list consists of a singly-linked-list of unused entries

#define INITIAL_RECTLIST_SIZE 1000
#define RECTLIST_GROWTH_INC    500

#define DEBUG 0

struct qv_list_entry {
   qv_rectangle rect;
   qv_list_entry *prev;				// Pointer to previous entry (not set for freelist)
   qv_list_entry *next;				// Pointer to next entry (or NULL if this is the tail of freelist)
};

static qv_list_entry qv_rectlist[INITIAL_RECTLIST_SIZE];
static int qv_rectlist_is_empty;		// Non-zero if list is empty

static qv_list_entry *qv_rectlist_ptr;		// Points to next entry in list
static qv_list_entry *qv_freelist;		// Points to entry at head of freelist

void
init_qv_rectlist()
{
   // Initialise rectangle-list.  All rectangles are in the free list

   int i;
   for(i=0; i<INITIAL_RECTLIST_SIZE; i++)
      qv_rectlist[i].next = &(qv_rectlist[i+1]);
   qv_rectlist[INITIAL_RECTLIST_SIZE-1].next = 0;

   qv_freelist = &(qv_rectlist[0]);
   qv_rectlist_is_empty = 1;
}



void
add_qv_rect(qv_rectangle rect)
{
   // get next entry from freelist

   if(qv_freelist == 0) {
      svlis_error("add_qv_rect","freelist is empty! - picture will be corrupt",SV_WARNING);
      return;
   }

   qv_list_entry *new_rect_entry = qv_freelist;
   qv_freelist = qv_freelist->next;

   if(qv_rectlist_is_empty) {
      // New entry is the only entry in the rectlist
      qv_rectlist_is_empty = 0;
      qv_rectlist_ptr = new_rect_entry;
      new_rect_entry->prev = new_rect_entry;
      new_rect_entry->next = new_rect_entry;
   } else {
      // Add new entry to the tail of the rectlist      
      qv_list_entry *qv_rectlist_tail = qv_rectlist_ptr->prev;

      new_rect_entry->next = qv_rectlist_tail->next;
      new_rect_entry->prev = qv_rectlist_tail;
      qv_rectlist_tail->next = new_rect_entry;
      qv_rectlist_ptr->prev = new_rect_entry;
   }

   // Add record.

   new_rect_entry->rect = rect;

#if DEBUG
   cout << "add_qv_rect: " << rect.x_pos << ", " << rect.y_pos << ", " << rect.width << ", " << rect.height;
   cout << ", " << rect.step << ", " << rect.prev_step << ", " << rect.nice << "\n";
   print_qv_rectlist();
#endif
}

sv_integer
get_qv_rect(qv_rectangle& rect)
{
   // Check for empty list.

   if(qv_rectlist_is_empty)
      return 0;

   // Loop through list decrementing niceness until entry with niceness of 0 is found.

   qv_list_entry *rect_entry = qv_rectlist_ptr;
   while(rect_entry->rect.nice > 0) {
      rect_entry->rect.nice--;
      rect_entry = rect_entry->next;
   }

   // Set empty flag if required

   if(rect_entry->next == rect_entry)
      qv_rectlist_is_empty = 1;

   // Remove entry from list and add to freelist

   rect_entry->prev->next = rect_entry->next;
   rect_entry->next->prev = rect_entry->prev;
   qv_rectlist_ptr = rect_entry->next;
   rect_entry->next = qv_freelist;
   qv_freelist = rect_entry;


   // Copy data to arg and return

   rect = rect_entry->rect;

#if DEBUG
   cout << "get_qv_rect: " << rect.x_pos << ", " << rect.y_pos << ", " << rect.width << ", " << rect.height;
   cout << ", " << rect.step << ", " << rect.prev_step << "\n";
   print_qv_rectlist();
#endif
   return 1;
}



void
print_qv_rectlist()
{
   if(qv_rectlist_is_empty)
      cout << "qv_rectlist is empty\n";
   else {
      qv_list_entry *rect_entry = qv_rectlist_ptr;
      do {
	 cout << "Rectangle entry at " << rect_entry << ": ";
	 cout << rect_entry->rect.x_pos << ", " << rect_entry->rect.y_pos << ", ";
	 cout << rect_entry->rect.width << ", " << rect_entry->rect.height << ", ";
	 cout << rect_entry->rect.step << ", " << rect_entry->rect.prev_step << ", " << rect_entry->rect.nice << ", ";
	 cout << rect_entry->prev << ", " << rect_entry->next << "\n";

	 rect_entry = rect_entry->next;
      }
      while(rect_entry != qv_rectlist_ptr);
   }

   cout << "\n";
}

// Limit on the maximum number of rays cast for a rectangle (not counting mirrors, shadows etc)
static sv_integer max_rays_per_rectangle = 64;

//
// Niceness values are set to be:
//
// Rays hit different surfaces:				 0
// All rays hit same surface with different colours:	 5
// All rays hit same surface with same colour:		20
//

sv_integer
generate_quickview_picture(const sv_model& modl,
			   const sv_view& view_params,
			   const sv_light_list& light_list,
			   sv_picture& picture_params,
			   sv_real progress_report_step,
			   void report_procedure(sv_real percent))
{
   sv_integer x_start, y_start;
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
   qv_rectangle rectangle;
   sv_integer j,k;

   sv_integer first_ray_for_this_rectangle;
   sv_integer rect_x_max_plus_1, rect_y_max_plus_1;
   sv_set test_set;
   sv_pixel test_colour;
   /* Not needed? sv_pixel background_colour = picture_params.get_pixel(0,0); */
   sv_integer width_lo, width_hi, height_lo, height_hi;

   // Generate vectors that are horizontal and vertical in the screen plane
   // The magnitude of the vectors is such that they represent the incremental
   // vector between adjacent pixels in the picture being generated

   sv_integer pic_width = picture_params.x_resolution();
   sv_integer pic_height = picture_params.y_resolution();
   sv_real la = view_params.lens_angle();

   sv_point view_vector = view_params.view_vector();
   sv_real view_const = tan((double)(la/2.0))*view_vector.mod() / (sv_real(pic_width)/2.0);
   sv_point screen_h = (view_vector ^ view_params.up_vector()).norm() * view_const;
   sv_point screen_v = (view_vector ^ screen_h).norm() * view_const;
   
   ray_origin = view_params.eye_point();

   // Decide on number of pixels for each report step

   sv_integer report_step_count = sv_integer(sv_real(pic_width*pic_height)*progress_report_step/100.0);
   sv_integer next_report_count = report_step_count;
   sv_integer pixel_count = 0;

   sv_integer missed;

   // Pick initial step - must be a power of 2
   sv_integer initial_step = 1;
   while(pic_width*pic_height/(initial_step*initial_step) > max_rays_per_rectangle)
      initial_step *= 2;
 
   init_qv_rectlist();		// Initialise rectangle-list to be empty.

   rectangle.x_pos = 0;
   rectangle.y_pos = 0;
   rectangle.width = pic_width;
   rectangle.height = pic_height;
   rectangle.step = initial_step;
   rectangle.prev_step = pic_width + pic_height;
   rectangle.nice = 0;
   add_qv_rect(rectangle);

   // Process first pixel
   ydif = -pic_height/2;
   xdif = -pic_width/2;
   ray_dir = (view_vector + xdif*screen_h + ydif*screen_v).norm();
   missed=1;
   ray = sv_line(ray_dir, ray_origin);
   interval = line_box(ray, modl.box());
   if(interval.lo() < 0.0) interval = sv_interval(0.0, interval.hi()); 
   if(!interval.empty()) { // Is this right?  AB.  interval != SV_AIR???
       hit_surf = modl.fire_ray(ray, interval, &t);
       if(hit_surf.exists()) {
 	 hit_point = line_point(ray,t);
	 pix_col = shade(modl, ray_dir, hit_surf, hit_point, view_params, 
		light_list, (sv_real)1.0, t);
	 missed=0;
      }
   }

   if(missed)
      pix_col = surroundings_colour(ray_dir);		// Colour using surrounding sphere

   pixel_colour = sv_pixel(pix_col);
   picture_params.picture_to_colour(pixel_colour);
   
   // Get record from rectangle list.

   while(get_qv_rect(rectangle)) {
      first_ray_for_this_rectangle = 1;
      rect_x_max_plus_1 = rectangle.x_pos+rectangle.width;
      rect_y_max_plus_1 = rectangle.y_pos+rectangle.height;

      x_start = rectangle.x_pos;
      while(x_start%rectangle.step != 0) x_start++;
      y_start = rectangle.y_pos;
      while(y_start%rectangle.step != 0) y_start++;

      for(iy=y_start; iy<rectangle.y_pos+rectangle.height; iy+=rectangle.step)
	 for(ix=x_start; ix<rectangle.x_pos+rectangle.width; ix+=rectangle.step) {
	    if((iy%rectangle.prev_step != 0) || (ix%rectangle.prev_step != 0)) {

	       ydif = iy - pic_height/2;
	       xdif = ix - pic_width/2;
	       ray_dir = (view_vector + xdif*screen_h + ydif*screen_v).norm();

	       hit_surf = sv_set();
	       missed=1;
	       ray = sv_line(ray_dir, ray_origin);
	       interval = line_box(ray, modl.box()); 
		if(interval.lo() < 0.0) interval = sv_interval(0.0, interval.hi());
	       if(!interval.empty()) { // ??? != SV_AIR ??? AB
		  hit_surf = modl.fire_ray(ray, interval, &t);

		  if(hit_surf.exists()) {
		     hit_point = line_point(ray,t);
		     pix_col = shade(modl, ray_dir, hit_surf, hit_point, 
			view_params, light_list, (sv_real)1.0, t);
		     missed=0;
		  }
	       }

	       if(missed)
		  pix_col = surroundings_colour(ray_dir);	// Colour using surrounding sphere

	       pixel_colour = sv_pixel(pix_col);
	       
//	       for(j=ix; j<min(ix+rectangle.step,rect_x_max_plus_1);j++)
// GMB 06-12-94
	       for(j=ix; j<min((long)(ix+rectangle.step),(long)rect_x_max_plus_1);j++)
//		  for(k=iy; k<min(iy+rectangle.step,rect_y_max_plus_1);k++)
// GMB 06-12-94
		  for(k=iy; k<min((long)(iy+rectangle.step),(long)rect_y_max_plus_1);k++)
		     picture_params.pixel(j,k,pixel_colour);

	       if(first_ray_for_this_rectangle) { // Set initial values.
		  test_colour = pixel_colour;
		  test_set = hit_surf;
		  rectangle.nice = 20;
		  first_ray_for_this_rectangle = 0;

	       } else { // Compare values.
		  if(hit_surf != test_set)
		     rectangle.nice=0;
		  else
		     if((test_colour.r != pixel_colour.r) ||
			(test_colour.g != pixel_colour.g) ||
			(test_colour.b != pixel_colour.b))
//			rectangle.nice=min(rectangle.nice,5);
// GMB 06-12-94
			rectangle.nice=min((long)rectangle.nice,5);
	       }
	       pixel_count++;
	    }
	 }

      // Report and update screen if required

      if(pixel_count >= next_report_count) {
	 next_report_count += report_step_count;
	 report_procedure((sv_real(pixel_count)*100.0/sv_real(pic_width*pic_height)));
      }

      // Add son rectangle(s) if not at final resolution.

      if(rectangle.step > 1) {
	 // Check to see if rectangle requires splitting.

	 if(((rectangle.width*rectangle.height)/(rectangle.step*rectangle.step)) > max_rays_per_rectangle) {

	    // Be careful here to handle odd width or height properly!

	    rectangle.prev_step = rectangle.step;
	    rectangle.step /= 2;
	    width_lo = rectangle.width/2;
	    width_hi = rectangle.width - width_lo;
	    height_lo = rectangle.height/2;
	    height_hi = rectangle.height - height_lo;

	    rectangle.width = width_lo;
	    rectangle.height = height_lo;
	    add_qv_rect(rectangle);

	    rectangle.x_pos += width_lo;
	    rectangle.width = width_hi;
	    add_qv_rect(rectangle);

	    rectangle.y_pos += height_lo;
	    rectangle.height = height_hi;
	    add_qv_rect(rectangle);

	    rectangle.x_pos -= width_lo;
	    rectangle.width = width_lo;
	    add_qv_rect(rectangle);

	 } else {
	    rectangle.prev_step = rectangle.step;
	    rectangle.step /= 2;
	    add_qv_rect(rectangle);
	 }
      }
   }

   report_procedure(100.0);
   return 1;
}
#if macintosh
 #pragma export off
#endif








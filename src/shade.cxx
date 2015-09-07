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
 * SvLis shade.c++	- calculate the colour of a raytraced pixel
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
 * First version: 15 March 1993
 * This version: 23 September 2000
 *
 */

//      Much modified by Adrian 29/12/95


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


#define SMALL_RAY_INCREMENT 1.0e-4

static sv_point ambient_colour(0.577, 0.577, 0.577);	// Default ambient colour (white)
static sv_real ambient_coeff = 0.2;			// Default ambient coefficient

static sv_point air_distance_attenuation(0.7, 0.7, 0.7);	
static sv_real max_air_dist = 10000000.0;
static sv_real haze = 0.5;
static sv_integer mist_def = 0;

void set_mist(const sv_point& ada) {air_distance_attenuation = ada; mist_def = 1;}
void set_back(sv_real b) {max_air_dist = b;}
void set_haze(sv_real h) {haze = h;}
sv_real get_back() {return max_air_dist; }
sv_real get_haze() {return haze;}
sv_point get_mist() {return air_distance_attenuation; }
sv_integer mist_active() { return mist_def; }

// these are used to limt the number of reflections! - we could do this better!

#define ATTENUATION_LIMIT (0.1)
#define ATTENUATION_COLOUR SV_OO


static sv_integer debug = 0;
static sv_surface stag;

static void
get_surface_attributes(const sv_set& hit_surface,
		       const sv_point& hit_point,
		       const sv_point& surface_normal,
		       sv_real *diffuse_refl_coeff,
		       sv_point *diffuse_colour,
		       sv_real *specular_refl_coeff,
		       sv_point *specular_colour,
		       sv_real *specular_refl_exp,
		       sv_real *transmission_coeff,
		       sv_integer *shad,
		       sv_integer *fog,
		       sv_picture** texture)
{
// Get surface parameters from surface attribute


   sv_attribute set_attribs = hit_surface.attribute();
   sv_attribute surf_attrib;
   sv_surface* surf;

   if((surf_attrib = set_attribs.tag_find(-stag.tag())).exists())
   {
	surf = (sv_surface*)(surf_attrib.user_attribute()->pointer);
   } else
	surf = new sv_surface();

// Note defaults are from the sv_surface constructor

   *diffuse_refl_coeff = surf->diffuse_coeff(); 
   *diffuse_colour = surf->diffuse_colour();
   *specular_refl_coeff = surf->specular_coeff();
   *specular_colour = surf->specular_colour();
   *specular_refl_exp = surf->specular_angle_power();
   *transmission_coeff = surf->transmission();
   *shad = surf->shadow();
   *fog = surf->mist();
   *texture = surf->texture();

   // If image has an image-map, overwrite the diffuse colour with one from the image
 
   if(*texture)
   {
	sv_integer u,v;
	get_tex_map(hit_surface, *surf, hit_point, surface_normal, u, v);

// Check if point is within map range - tile map if reguired

	sv_integer map_u_res = (*texture)->x_resolution();
	sv_integer map_v_res = (*texture)->y_resolution();

	if((u < 0) || (u >= map_u_res))
	{
		if(surf->map_type() == SV_TILED)
		{
			while(u < 0) u += map_u_res; // Is this needed?
			u = u % map_u_res;
		} else
			return;
	}	

	if((v < 0) || (v >= map_v_res))
	{
		if(surf->map_type() == SV_TILED)
		{
			while(v < 0) v += map_v_res; // Is this needed?
			v = v % map_v_res;
		} else
		return;
	}

	sv_pixel pixel_colour = (*texture)->pixel(u,v);
	diffuse_colour->x = (sv_real)pixel_colour.r/255.0;
	diffuse_colour->y = (sv_real)pixel_colour.g/255.0;
	diffuse_colour->z = (sv_real)pixel_colour.b/255.0;

// If it's (nearly) black and that means transparent...

	if (surf->map_0_xmit() && (diffuse_colour->mod() < 0.001) )  // Hack
		*transmission_coeff = 1;
    }

// Solid texture?

    if(surf->map_type() == SV_SOLID_TEX)
    {
	get_solid_tex(hit_surface, *surf, hit_point, surface_normal, diffuse_colour);
	if (surf->map_0_xmit() && (diffuse_colour->mod() < 0.001) )
		*transmission_coeff = 1;
    }
}


sv_point
shade(const sv_model&  modl,
      const sv_point ray_dir,
      const sv_set hit_surface,
      const sv_point& pnt,
      const sv_view& view_params,
      const sv_light_list& light_list,
      const sv_real& attenuation,
      const sv_real& dist)
{
   // wot about over or under flows? ;

// Pick up the scale length (AB)

	sv_real sri = modl.box().vol();
	sri = pow((double)sri,(double)0.333333)*SMALL_RAY_INCREMENT;

   // If attenuation is too great, then return

   if(attenuation < ATTENUATION_LIMIT) {
      if(debug)
	 cout << "shade() - attenuation limit reached\n";
      return ATTENUATION_COLOUR;
   }

   sv_real diffuse_refl_coeff;			// Diffuse reflection coefficient [0 ... 1]
   sv_point diffuse_colour;			// Diffuse surface colour (RGB)
   sv_real specular_refl_coeff;			// Specular reflection coefficient [0 ... 1]
   sv_point specular_colour;			// Specular surface colour (RGB)
   sv_real specular_angle_power;			// power to raise ..
   sv_real transmission_coeff;
   sv_integer shad;				// Cast shadows?
   sv_integer fog;				// In the fog?
   sv_picture* tex;				// Texture map

   sv_line new_ray;		// Reflections and shadows
   sv_point new_eye_point;
   sv_interval interval;
   sv_real t;
   sv_set hit_surf;

   sv_primitive prim = hit_surface.primitive();
   sv_point surface_normal = prim.grad(pnt);
   surface_normal = surface_normal.norm();				// Get surface normal vector at hit point

   if(debug)
      cout << "pnt =          " << pnt << "\n";
   
   // Get surface parameters from surface attribute

   get_surface_attributes(hit_surface, pnt, surface_normal, &diffuse_refl_coeff, &diffuse_colour,
			  &specular_refl_coeff, &specular_colour, &specular_angle_power,
			  &transmission_coeff, &shad, &fog, &tex);


   // Calculate shade based on all lights for this surface

   // First set ambient light contribution to colour

   sv_point colour = ambient_colour*ambient_coeff;
   colour.x *= diffuse_colour.x;
   colour.y *= diffuse_colour.y;
   colour.z *= diffuse_colour.z;

   sv_point light_vector;				// vector from light source to surface point

//   sv_light_list *light_list_ptr = &light_list;
// GMB 06-12-94
   const sv_light_list *light_list_ptr = &light_list;
   while(light_list_ptr) {
      // Get info this this light
      light_type type = light_list.source->type();
      sv_point light_loc = light_list.source->location();
      sv_point light_col = light_list.source->colour();
      sv_point light_dir = light_list.source->direction();
      sv_real light_pwr = light_list.source->angle_power();

      switch(type) {
       case POINT_SOURCE:
	 light_vector = (pnt - light_loc).norm();
	 break;

       case PARALLEL:
	 light_vector = light_dir;
	 break;

       default:
	 svlis_error("shade()","Invalid light type", SV_WARNING);
	 break;
      }

// Cast shadows added by AB

// Can the dog see the rabbit?

	if(shad)
	{
      		new_ray = sv_line(-light_vector, pnt);

      // Move away from surface slightly

//     		new_eye_point = line_point(new_ray,sri);
		new_eye_point = pnt + surface_normal*sri;
      		new_ray = sv_line(-light_vector, new_eye_point);

      		interval = line_box(new_ray, modl.box()); 
      		if(interval.empty())
	 	   svlis_error("shade()",
			"Shadow ray does not intersect object-space",
			SV_WARNING);
	  	else 
		{
	 		if(interval.lo() < 0.0) interval = sv_interval(0.0, interval.hi());
	 		hit_surf = modl.fire_ray(new_ray, interval, &t);
		}
	 	shad = hit_surf.exists();
	}
      
      // Add in diffuse reflection contribution to colour

      if(!shad)
      {
        sv_real cosine = -light_vector * surface_normal;
	cosine = max(0.0, min(cosine, 1.0));
        sv_point difuse_contrib = light_col*diffuse_refl_coeff*cosine;
        difuse_contrib.x *= diffuse_colour.x;
        difuse_contrib.y *= diffuse_colour.y;
        difuse_contrib.z *= diffuse_colour.z;
        colour = colour + difuse_contrib;

      // Add in specular reflection contribution to colour

        sv_point reflected_vector;
        if(specular_refl_coeff > 0.0) 
	{
         // Form vector that is reflection of light vector
         if(light_vector * surface_normal < -0.999) 
	 {
            reflected_vector = surface_normal;
         } else 
	 {
            sv_point tmpvec1 = surface_normal ^ light_vector;
            sv_point tmpvec2 = tmpvec1 ^ surface_normal;
         
            reflected_vector = 
               ( (-light_vector*surface_normal)*surface_normal + 
               	(light_vector*tmpvec2)*tmpvec2 ).norm();
         }

	 cosine = max(0.0, min(-ray_dir*reflected_vector, 1.0));
	 sv_point specular_contrib = light_col*specular_refl_coeff*pow((double)cosine,(double)specular_angle_power);
	 specular_contrib.x *= specular_colour.x;
	 specular_contrib.y *= specular_colour.y;
	 specular_contrib.z *= specular_colour.z;
	 colour = colour + specular_contrib;

	}
      }

      light_list_ptr = light_list_ptr->next;
   }


   // Allow for attenuation

   colour = colour*attenuation;

   // Now add in contribution for reflected ray

   sv_point new_ray_dir;

   if(specular_refl_coeff > 0.0) {  // Change to 0.00001 - AB
      // Form vector that is reflection of current ray vector
      if(ray_dir * surface_normal < -0.999) {
	 new_ray_dir = surface_normal;
      } else {
	 sv_point tmpvec1 = surface_normal ^ ray_dir;
	 sv_point tmpvec2 = (tmpvec1 ^ surface_normal).norm();
         
	 new_ray_dir = 
	    ( (-ray_dir*surface_normal)*surface_normal + (ray_dir*tmpvec2)*tmpvec2 ).norm();

	 if(debug) {
	    cout << "attenuation    = " << attenuation << "\n";
	    cout << "ray_dir        = " << ray_dir.x << ", " << ray_dir.y << ", " << ray_dir.z << "\n";
	    cout << "surface_normal = " << surface_normal.x << ", " << surface_normal.y << ", " << surface_normal.z << "\n";
	    cout << "tmpvec1        = " << tmpvec1.x << ", " << tmpvec1.y << ", " << tmpvec1.z << "\n";
	    cout << "tmpvec2        = " << tmpvec2.x << ", " << tmpvec2.y << ", " << tmpvec2.z << "\n";
	    cout << "new_ray_dir    = " << new_ray_dir.x << ", " << new_ray_dir.y << ", " << new_ray_dir.z << "\n";
	 }
      }

//      new_ray = sv_line(new_ray_dir, pnt);

      // Move away from surface slightly

      new_eye_point = pnt + surface_normal*sri;
      new_ray = sv_line(new_ray_dir, new_eye_point);

      interval = line_box(new_ray, modl.box()); 
      if(interval.empty())
	 	svlis_error("shade()","Reflected ray does not intersect object-space",SV_WARNING);
	  else {
	 if(interval.lo() < 0.0) interval = sv_interval(0.0, interval.hi());
	 hit_surf = modl.fire_ray(new_ray, interval, &t);
	 
	 if(hit_surf.exists()) {
	    sv_point hit_point = line_point(new_ray,t);

	    if(debug)
	       cout << " secondary ray hit: t = " << t << "\n";

	    colour = colour + shade(modl, new_ray_dir, hit_surf, hit_point, 
		view_params, light_list, attenuation*specular_refl_coeff,t);
	 } else {
	    // Reflected ray misses
	    colour = colour + attenuation*surroundings_colour(new_ray_dir);
	 }
      }
   }

#if 0
   // Now add in contribution for transmitted (refracted) ray

   if(transmission_coeff > 0.0) {
      // Form vector that is refracted current ray vector
#if 0
      if(ray_dir * surface_normal < -0.999) {
	 new_ray_dir = surface_normal;
      } else {
	 sv_point tmpvec1 = surface_normal ^ ray_dir;
	 sv_point tmpvec2 = (tmpvec1 ^ surface_normal).norm();
         
	 new_ray_dir = 
	    ((-ray_dir*surface_normal)*surface_normal + (ray_dir*tmpvec2)*tmpvec2).norm();

//	 if(debug) {
//	 cout << "ray_dir        = " << ray_dir.x << ", " << ray_dir.y << ", " << ray_dir.z << "\n";
//	 cout << "surface_normal = " << surface_normal.x << ", " << surface_normal.y << ", " << surface_normal.z << "\n";
//	 cout << "tmpvec1        = " << tmpvec1.x << ", " << tmpvec1.y << ", " << tmpvec1.z << "\n";
//	 cout << "tmpvec2        = " << tmpvec2.x << ", " << tmpvec2.y << ", " << tmpvec2.z << "\n";
//	 cout << "new_ray_dir    = " << new_ray_dir.x << ", " << new_ray_dir.y << ", " << new_ray_dir.z << "\n";
//       }
      }
#else
      new_ray_dir = ray_dir;
#endif
      sv_line new_ray = sv_line(new_ray_dir, pnt);

      // Move away from surface slightly

      sv_point new_eye_point = line_point(new_ray,sri);
      new_ray = sv_line(new_ray_dir, new_eye_point);

      interval interval = line_box(new_ray, m_box(modl)); 
      if(interval != SV_AIR) {
	 sv_real t;
	 if(interval.lo() < 0.0) interval = sv_interval(0.0, interval.hi());
	 sv_set hit_surf = modl.fire_ray(new_ray, interval, &t);
	 
	 // This should reach the next surface (from the solid side)

	 // do lots of checks etc 
	 if(hit_surf.exists()) {
	    sv_point hit_point = line_point(new_ray,t);
	    colour = colour + shade(modl, new_ray_dir, hit_surf, hit_point, view_params, light_list,
				    attenuation*specular_refl_coeff,t);
	 }
      }
   }

#endif

// Fog?

   sv_real d;

   if(fog && (dist > 0.0) && mist_active())
   {
	d = dist/max_air_dist;
	if(d > 1.0) d = 1.0;
	d = d*haze;
	colour = (1 - d)*colour + d*air_distance_attenuation;
   }

   // Limit range for colour

   if(colour.x > 1.0) colour.x = 1.0;
   if(colour.y > 1.0) colour.y = 1.0;
   if(colour.z > 1.0) colour.z = 1.0;

   return colour;
}



void
set_ambient_light_level(sv_point colour,
			sv_real intensity)
{
   ambient_colour = colour.norm();
   ambient_coeff = intensity;
}
#if macintosh
 #pragma export off
#endif

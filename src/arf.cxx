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
 * arf.c++    Andy's Root Finder - Andy Wallis
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


// #include <unistd.h>  // Commented out by Adrian

#include "svlis.h"
#include "polynml.h"
#include "arf.h"
#if macintosh
 #pragma export on
#endif

#define DEBUG		0		// Non-zero to enable debuging

#define MAX_ROOTS 100

static sv_integer num_roots;
static sv_real roots[MAX_ROOTS];
static sv_integer too_many_roots;

sv_integer
arf(const sv_line& ray,			// Ray to intersect with primitive
    const sv_primitive& prim,		// Primitive for which roots are required
    sv_interval rootfinding_range,		// Range within which to find roots
    sv_real tol_t,				// Accuracy value for roots as difference in T within interval
    const sv_integer max_root_count,		// maximum number of roots to find
    double *returned_roots		// Array of roots in acending order
    )
{
   // The function returns the number of roots found, or nrgative value for error.  Error codes are:
   //   -1 = Too many roots (returned_roots array will still be filled in up to element `max_root_count-1'
   //   -2 = Too many roots (returned_roots array will still be filled in up to element `MAX_ROOTS'

   num_roots = 0;
   too_many_roots = 0;

#if DEBUG
   cout << "arf: range = " << rootfinding_range.lo << ", " << rootfinding_range.hi() <<
      "  ( = [" << ray.origin+ray.direction*rootfinding_range.lo << " , " <<
	  ray.origin+ray.direction*rootfinding_range.hi() << "])\n";
#endif

   arf_r(ray, prim, rootfinding_range, tol_t);

   // Copy roots to user array

   sv_integer i;
//   sv_integer n = min(max_root_count, num_roots);
// GMB 27-7-94
   sv_integer n = min((long)max_root_count, num_roots);

   for(i=0;i<n;i++)
      returned_roots[i] = roots[i];

   // Set return status and return

   if(too_many_roots) {
      if(max_root_count > MAX_ROOTS)
	 return -2;
      else
	 return -1;
   } else {
      if(num_roots > max_root_count)
	 return -1;
      else
	 return num_roots;	 
   }
}

// Adrian's version of arf_r

void
arf_r(const sv_line& ray,			// Ray to intersect with primitive
      const sv_primitive& prim,		// Primitive for which roots are required
      sv_interval t_range,			// Range within which to find roots
      sv_real tol_t)			// Accuracy value for roots as difference in T within interval
{
	sv_point lo = ray.point(t_range.lo());
	sv_point hi = ray.point(t_range.hi());
	sv_box b;

	b.xi = sv_interval(min(lo.x, hi.x), max(lo.x, hi.x));
	b.yi = sv_interval(min(lo.y, hi.y), max(lo.y, hi.y));
	b.zi = sv_interval(min(lo.z, hi.z), max(lo.z, hi.z));

	sv_interval pr = prim.range(b);

	switch(pr.member())
	{
	case SV_SOLID:
	case SV_AIR:
		return;

	case SV_SURFACE:
		{
			sv_real del = t_range.hi() - t_range.lo();
			sv_real mid = (t_range.lo() + t_range.hi())/2;
			if(del > tol_t)
			{
				arf_r(ray, prim, sv_interval(t_range.lo(), mid), tol_t);
				arf_r(ray, prim, sv_interval(mid, t_range.hi()), tol_t);
			} else
			{
				sv_real lov = prim.value(lo);
				sv_real hiv = prim.value(hi);
				if(lov*hiv > 0) return;
				if(num_roots >= MAX_ROOTS)
				{
	       				svlis_error("arf_r", "too many roots", SV_WARNING);
	       				too_many_roots = 1;
	       				return;
	    			}
				sv_real dv = hiv - lov;
				sv_real t = t_range.lo() - del*lov/dv;
	    			roots[num_roots++] = t;
			}
		}
		break;

	default:
		svlis_error("arf_r","dud interval member value",SV_CORRUPT);
	}
}

#if 0
void
arf_r(const sv_line& ray,			// Ray to intersect with primitive
      const sv_primitive& prim,		// Primitive for which roots are required
      sv_interval t_range,			// Range within which to find roots
      sv_real tol_t)			// Accuracy value for roots as difference in T within interval
{
   if(too_many_roots) return;

#if DEBUG
   cout << "arf_r: range = " << t_range.lo() << ", " << t_range.hi() << "   num_roots = " << num_roots;
#endif

//   sv_interval values = rangeX(prim,ray,range);

   sv_point point_l = ray.origin + t_range.lo()*ray.direction;
   sv_point point_h = ray.origin + t_range.hi()*ray.direction;

   sv_point min_point;
   sv_point max_point;
   
   if(point_l.x < point_h.x) {
      min_point.x = point_l.x; max_point.x = point_h.x;
   } else {
      min_point.x = point_h.x; max_point.x = point_l.x;
   }  

   if(point_l.y < point_h.y) {
      min_point.y = point_l.y; max_point.y = point_h.y;
   } else {
      min_point.y = point_h.y; max_point.y = point_l.y;
   }  

   if(point_l.z < point_h.z) {
      min_point.z = point_l.z; max_point.z = point_h.z;
   } else {
      min_point.z = point_h.z; max_point.z = point_l.z;
   }  

   sv_interval values = prim.range(sv_box(min_point, max_point));

#if DEBUG
   cout << "   values = " << values.lo() << ", " << values.hi() << "\n";
#endif

   if((values.lo() < 0.0) && (values.hi() > 0.0)) {
      sv_real mid_point = (t_range.lo() + t_range.hi())/2.0;
      if(t_range.hi() - t_range.lo() <= tol_t) {
	 // We may have a root, check if it is for real
	 sv_real value_lo = prim.value(point_l);
	 sv_real value_hi = prim.value(point_h);

	 if(((value_lo <= 0.0) && (value_hi > 0.0)) ||
	    ((value_lo > 0.0) && (value_hi <= 0.0))) {
	    if(num_roots >= MAX_ROOTS) {

	       svlis_error("arf_r", "too many roots", SV_WARNING);
	       too_many_roots = 1;
	       return;
	    }
	    roots[num_roots++] = mid_point;
	 }
      } else {
	 arf_r(ray, prim, sv_interval(t_range.lo(), mid_point), tol_t);
	 arf_r(ray, prim, sv_interval(mid_point, t_range.hi()), tol_t);
      }
   }
}

#endif

#if macintosh
 #pragma export off
#endif


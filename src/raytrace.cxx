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
 * raytrace.c++ - top-level code for tracing a ray against a model
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

// #include <unistd.h>  // Commented out by Adrian

#include "svlis.h"
#include "polynml.h"
#include "ivallist.h"
#include "raytrace.h"
#include "arpors.h"
#include "arf.h"
#if macintosh
 #pragma export on
#endif

#define LARGE 99999999.9		// we MUST be able to do better than this- see AB about this
					// and the use of infinity as an interval limit

#define CACHEING	0		// Non-zero to use root cache (recomended value: 1)
#define DEBUG		0		// Non-zero to enable debugging
#define USE_LINE_BOX	0		// Non-zero to use standard line-box tests (recomended value: 0)

static sv_integer current_ray_number = 0;

#if DEBUG
//static sv_integer debug_ray_number = 140*200+80;
//static sv_integer debug_ray_number = 127*200+5;
static sv_integer debug_ray_number = -1;

void
set_debug_ray_number(sv_integer raynum)
{
   debug_ray_number = raynum;
   cout << "Setting debug ray number to " << debug_ray_number << "\n";
}

sv_integer
get_debug_ray_number()
{
   return debug_ray_number;
}
#endif

static sv_integer max_iteration_count = 200;
static double arpors_tol_f = 0.00001;
static double arpors_tol_t = 0.00001;
static double arf_tol_t = 0.001;


#if CACHEING
// Cached root-finding data.
// NB: This can only be used if we assume that we are finding ALL roots for a primitive each time
//     we first come across it for a ray

struct root_data {
   sv_integer ray_number;
   sorted_interval_list roots;
};

static sv_integer cache_size;
static root_data* cache;



sv_integer
set_prim_flags(sv_integer count,
	       sv_set s)
{
   // Walk the tree setting primitive flags
   // Also count the number of primitives
   // NB: If a primitive occurs twice, it will be counted twice!

   sv_primitive prim;

   switch(s.contents()) {
    case SV_EVERYTHING:
    case SV_NOTHING:
      break;

    case 1:
      // Leaf node - set primitive flags
      prim = s.primitive();
      prim.set_flags_priv(count);
      count++;
      break;

    default:
      switch(s.op()) {
       case SV_UNION:
       case SV_INTERSECTION:
	 count = set_prim_flags(count,s.child_1());
	 count = set_prim_flags(count,s.child_2());
	 break;

       default:	
	 svlis_error("set_prim_flags",
		"unknown operator", SV_CORRUPT)
	 return(count);
	 break;
      }
   }
   return count;
}
   



sv_integer init_raytrace_cache(sv_set& ss)
{
   sv_integer i;

   current_ray_number = 0;

   // Set primitive flags

   sv_integer primitive_count = set_prim_flags(0, s);

   if((cache = new root_data[primitive_count]) == NULL)
      return -1;

   cache_size = primitive_count;

   // Clear cache
   for(i=0;i<cache_size;i++)
      cache[i].ray_number = -1;

   svlis_error("init_raytrace_cache",
	"Using rootfinding cache", SV_COMMENT);

   return 0;
}

void
destroy_raytrace_cache()
{
   delete[] cache;
}

#else

sv_integer
init_raytrace_cache(sv_set &s)
{
   current_ray_number = 0;
   svlis_error("init_raytrace_cache",
	"Not using rootfinding cache",SV_COMMENT);
   return 0;
}

void
destroy_raytrace_cache()
{
   ;
}

#endif


#if ~USE_LINE_BOX
enum ray_direction {
   Zero,
   Positive,
   Negative};

static ray_direction ray_x_dir;
static ray_direction ray_y_dir;
static ray_direction ray_z_dir;
#endif



//
// Fire a ray into a model and report what it hits
// (not only used to create images)
//

sv_set						// return set that was hit by ray
sv_model::fire_ray(
	 const sv_line& ray,			// ray to fire
	 const sv_interval& ray_param_interval,		// parameter range that is of interest
	 /* Returns */
	 sv_real*	hit_ray_param) const			// parametric value at intersection
{
	sv_model mod = *this;
   current_ray_number++;

#if DEBUG
   if((debug_ray_number >= 0) && (current_ray_number != debug_ray_number)) {
      return sv_set();
   } else {
      cout << "current_ray_number = " << current_ray_number << "\n";
   }
#endif

#if ~USE_LINE_BOX
   // Set ray direction flags

   if(ray.direction.x < 0.0)
      ray_x_dir = Negative;
   else if(ray.direction.x > 0.0)
      ray_x_dir = Positive;
   else
      ray_x_dir = Zero;

   if(ray.direction.y < 0.0)
      ray_y_dir = Negative;
   else if(ray.direction.y > 0.0)
      ray_y_dir = Positive;
   else
      ray_y_dir = Zero;

   if(ray.direction.z < 0.0)
      ray_z_dir = Negative;
   else if(ray.direction.z > 0.0)
      ray_z_dir = Positive;
   else
      ray_z_dir = Zero;
#endif

#if DEBUG
   sv_set result = ray_model_test(mod, ray, ray_param_interval.hi(), 
	ray_param_interval, hit_ray_param);
   sv_point hit_point;

   if((debug_ray_number >= 0) && (current_ray_number == debug_ray_number)) {
      if(result.exists()) {
	 hit_point = line_point(ray,*hit_ray_param);
	 cout << "Hit: t = " << *hit_ray_param << " = (" << hit_point.x << ","
	    << hit_point.y << "," << hit_point.z << ")\n";
      } else {
	 cout << "Miss\n";
      }
   }
   return result;
#else
   return ray_model_test(mod, ray, ray_param_interval.hi(), 
	ray_param_interval, hit_ray_param);
#endif
}



sv_set						// return set that was hit by ray
ray_model_test(const sv_model& mod,		// model to fire ray into
	       const sv_line& ray,			// ray to fire
	       const sv_real& rootfinding_tmax,	// the max t value to find roots for
	       const sv_interval& valid_model_interval, // the limits within which the model is valid
	       // Returns
	       sv_real*	hit_ray_param)		// parametric value at intersection
{
   // NB: `rootfinding_tmax' should always be >=  far limit of `line_box(ray, m_box(mod))'

   sv_set hit_surface;

#if DEBUG
   if(current_ray_number == debug_ray_number) {
      cout << "ray_model_test(): valid_model_interval = [" << valid_model_interval.lo() << ',' << valid_model_interval.hi() << "]";
      cout << "        tmax = " << rootfinding_tmax << "\n";
      cout << "                  box limits are: (" << mod.box().xi.lo() << "," << mod.box().yi.lo() << "," << mod.box().zi.lo();
      cout << ") to (" << mod.box().xi.hi() << "," << mod.box().yi.hi() << "," << mod.box().zi.hi() << ")\n";
   }
#endif


   if(mod.kind() == LEAF_M) {			// At leaf node in model tree
      // We know that ray intersects this box since test was done one level up!
      return ray_leaf_node_test(mod.set_list(), ray, rootfinding_tmax, valid_model_interval, hit_ray_param);

   } else {					// Not leaf-node in model, so recurse for children
      sv_model child_1_model = mod.child_1();
      sv_model child_2_model = mod.child_2();

#if USE_LINE_BOX
      sv_interval child_1_valid_int = line_box(ray, child_1_model.box()) & valid_model_interval;
      sv_interval child_2_valid_int = line_box(ray, child_2_model.box()) & valid_model_interval;

#else
      sv_interval child_1_valid_int;
      sv_interval child_2_valid_int;

      switch(mod.kind()) {
       case X_DIV:
	 switch(ray_x_dir) {
	  case Positive:
	    child_1_valid_int = sv_interval(valid_model_interval.lo(),
                  min(valid_model_interval.hi(), (child_1_model.box().xi.hi() - ray.origin.x)/ray.direction.x));

            child_2_valid_int = sv_interval(max(valid_model_interval.lo(), (child_2_model.box().xi.lo() - ray.origin.x)/ray.direction.x),
	          valid_model_interval.hi());
	    break;

	 case Negative:
	   child_1_valid_int = sv_interval(max(valid_model_interval.lo(), (child_1_model.box().xi.hi() - ray.origin.x)/ray.direction.x),
	           valid_model_interval.hi());

	   child_2_valid_int = sv_interval(valid_model_interval.lo(),
                  min(valid_model_interval.hi(), (child_2_model.box().xi.lo() - ray.origin.x)/ray.direction.x));
	    break;

	  default:
	    // ray is parallel to division plane
	    child_1_valid_int = line_box(ray, child_1_model.box());
	    child_2_valid_int = line_box(ray, child_2_model.box());
	    break;
	 }
	 break;

       case Y_DIV:
	 switch(ray_y_dir) {
	  case Positive:
	    child_1_valid_int = sv_interval(valid_model_interval.lo(),
                       min(valid_model_interval.hi(), (child_1_model.box().yi.hi() - ray.origin.y)/ray.direction.y));

            child_2_valid_int = sv_interval(max(valid_model_interval.lo(), (child_2_model.box().yi.lo() - ray.origin.y)/ray.direction.y),
	               valid_model_interval.hi());
	    break;

	 case Negative:
	   child_1_valid_int = sv_interval(max(valid_model_interval.lo(), (child_1_model.box().yi.hi() - ray.origin.y)/ray.direction.y),
	              valid_model_interval.hi());

	   child_2_valid_int = sv_interval(valid_model_interval.lo(),
                      min(valid_model_interval.hi(), (child_2_model.box().yi.lo() - ray.origin.y)/ray.direction.y));
	    break;

	  default:
	    // ray is parallel to division plane
	    child_1_valid_int = line_box(ray, child_1_model.box());
	    child_2_valid_int = line_box(ray, child_2_model.box());
	    break;
	 }
	 break;

       case Z_DIV:
	 switch(ray_z_dir) {
	  case Positive:
	    child_1_valid_int = sv_interval(valid_model_interval.lo(),
                     min(valid_model_interval.hi(), (child_1_model.box().zi.hi() - ray.origin.z)/ray.direction.z));

            child_2_valid_int = sv_interval(max(valid_model_interval.lo(), (child_2_model.box().zi.lo() - ray.origin.z)/ray.direction.z),
	             valid_model_interval.hi());
	    break;

	 case Negative:
	   child_1_valid_int = sv_interval(max(valid_model_interval.lo(), (child_1_model.box().zi.hi() - ray.origin.z)/ray.direction.z),
	            valid_model_interval.hi());

	   child_2_valid_int = sv_interval(valid_model_interval.lo(),
                    min(valid_model_interval.hi(), (child_2_model.box().zi.lo() - ray.origin.z)/ray.direction.z));
	    break;

	  default:
	    // ray is parallel to division plane
	    child_1_valid_int = line_box(ray, child_1_model.box());
	    child_2_valid_int = line_box(ray, child_2_model.box());
	    break;
	 }
	 break;

       default:
       	svlis_error("ray_model_test", "invalid model kind", SV_CORRUPT);
	 break;
      }
#endif

      if(child_1_valid_int.empty() && child_2_valid_int.empty())
	 return hit_surface;	// empty set!

      if(child_1_valid_int.empty())
	 return ray_model_test(child_2_model, ray, rootfinding_tmax, child_2_valid_int, hit_ray_param);

      if(child_2_valid_int.empty())
	 return ray_model_test(child_1_model, ray, rootfinding_tmax, child_1_valid_int, hit_ray_param);

      if(child_1_valid_int.lo() < child_2_valid_int.lo()) {
	 hit_surface = ray_model_test(child_1_model, ray, rootfinding_tmax, child_1_valid_int, hit_ray_param);
	 if(hit_surface.exists())
	    return hit_surface;
	 else
	    return ray_model_test(child_2_model, ray, rootfinding_tmax, child_2_valid_int, hit_ray_param);

      } else {
	 hit_surface = ray_model_test(child_2_model, ray, rootfinding_tmax, child_2_valid_int, hit_ray_param);
	 if(hit_surface.exists())
	    return hit_surface;
	 else
	    return ray_model_test(child_1_model, ray, rootfinding_tmax, child_1_valid_int, hit_ray_param);
      }	
   }
}



//
// Generate intersections between ray and primitive (within given ray interval)
//

sv_set							// return set that was hit by ray
ray_leaf_node_test(const sv_set_list &sets,		// set_list to test ray against
		   const sv_line& ray,			// ray to test
		   const sv_real& rootfinding_tmax,	// the max t value to find roots for
		   const sv_interval& valid_model_interval,// the limits within which the model is valid
		   /* Returns */
		   sv_real*	hit_ray_param)		// parametric value at intersection
{
   sv_set slo,shi,result;
   sv_interval i;

   // Find all roots (within the ray parameter interval) for THE FIRST SET in this leaf node

   sorted_interval_list solid_int_list;

   sv_set_list tmp_sets = sets;

// Added by AB - assume set lists are unioned.

   while(tmp_sets.exists()) {
   solid_int_list = solid_int_list | ray_set_intersection_test(tmp_sets.set(), ray, 
	valid_model_interval.lo(), rootfinding_tmax);
      tmp_sets = tmp_sets.next();
   }


// Added by AB - are we starting in solid?

	interval_list_entry* ile = solid_int_list.entry();
	int in_solid = 0;
	while(ile)
	{
		if( ile->intrval.member(valid_model_interval.lo()) == SV_SOLID)
		{
			in_solid = 1;
			ile = 0;
		} else
			ile = ile->next;
	}
 
  // Generate the intersection of the interval with the sub-space for this node

   solid_int_list = solid_int_list & 
	sorted_interval_list(valid_model_interval, slo, shi);

#if DEBUG
   if(current_ray_number == debug_ray_number) {
      debug_print_sil(solid_int_list, "ray_leaf_node_test:\n");
   }
#endif

// If starting in solid - force return of first exit point 
// by chucking the head away.

// Get the first intersection

    solid_int_list.remove_from_head(&i,&slo,&shi);
    if(in_solid)
    {
	result = shi;
	*hit_ray_param = i.hi();
    } else
    {
	result = slo;
	*hit_ray_param = i.lo();
    }   
    return(result);
}


sorted_interval_list
ray_set_intersection_test(const sv_set& set_to_test,
			  const sv_line& ray,
			  const sv_real& rootfinding_tmin,
			  const sv_real& rootfinding_tmax)
{
   // Walk over the set tree and, for each set leaf-node, generate a sorted-interval-list of
   // solid.  Combine these using the set theory.
   // The combined sorted-interval-list is returned
   // NB: The sorted-interval-list for each set leaf-node is cached locally at that node
   //     OR SHOULD IT BE THE PRIMITIVE FOR THAT NODE?

   sv_set no_set;

   switch(set_to_test.contents()) {
    case SV_EVERYTHING:
#if DEBUG
      if(current_ray_number == debug_ray_number)
	 cout << "ray_set_intersection_test(): EVERYTHING -  returns [-LARGE,LARGE]\n";
#endif
      return sorted_interval_list(sv_interval(-LARGE,LARGE),
	no_set,no_set);

    case SV_NOTHING:
#if DEBUG
      if(current_ray_number == debug_ray_number)
	 cout << "ray_set_intersection_test(): NOTHING -  returns NULL sil\n";
#endif
      return sorted_interval_list();

    case 1:
#if DEBUG
      if(current_ray_number == debug_ray_number)
	 cout << "ray_set_intersection_test(): Single HS\n";
#endif
//      return ray_test(set_to_test, ray, rootfinding_tmin, rootfinding_tmax);
// GMB 06-12-94
      return ray_test((sv_set)set_to_test, ray, rootfinding_tmin, 
	rootfinding_tmax);

    default:
      switch(set_to_test.op()) {
       case SV_UNION:
#if DEBUG
      if(current_ray_number == debug_ray_number)
	 cout << "ray_set_intersection_test(): SV_UNION\n";
#endif
	 return ray_set_intersection_test(set_to_test.child_1(), ray, rootfinding_tmin, rootfinding_tmax) |
	    ray_set_intersection_test(set_to_test.child_2(), ray, rootfinding_tmin, rootfinding_tmax);

       case SV_INTERSECTION:
#if DEBUG
      if(current_ray_number == debug_ray_number)
	 cout << "ray_set_intersection_test(): SV_INTERSECTION\n";
#endif
	 return ray_set_intersection_test(set_to_test.child_1(), ray, rootfinding_tmin, rootfinding_tmax) &
	    ray_set_intersection_test(set_to_test.child_2(), ray, rootfinding_tmin, rootfinding_tmax);

       default:	
	 svlis_error("ray_set_intersection_test","unknown operator", SV_CORRUPT);
	 break;
      }
   }
   return sorted_interval_list();  // Error return added by Adrian
}


//
// Return a list of solid intervals along the ray for the primitive
//
// Take a primitive formed from the arithmetic combination of planar
// half-spaces and a line and return the coefficients
// of the univariate polynomial that results from the substitution of
// the line into the arithmetic half-space.  The coefficients of the
// univariate polynomial are stored in `t_coeffs[]'.  The functions
// this polynomial

polynomial
get_t_coefficients(const sv_line& l,
		   const sv_primitive& p)
{
   sv_plane f;
   polynomial c_1, c_2;
   sv_integer lp, ilp;

   switch(p.kind()) {
    case SV_REAL:
      return polynomial(p.real());

    case SV_PLANE:
      f = p.plane();
      c_1.set_coeff(0,f.normal*l.origin + f.d);
      c_1.set_coeff(1,l.direction*f.normal);
      return c_1;

    case SV_CYLINDER:
    case SV_SPHERE:
    case SV_CONE:
    case SV_TORUS:
    case SV_CYCLIDE:
    case SV_GENERAL:
      switch(p.op()) {
       case SV_PLUS:
	 return (get_t_coefficients(l, p.child_1()) + 
		get_t_coefficients(l, p.child_2()));

       case SV_MINUS:
	 return (get_t_coefficients(l, p.child_1()) - 
		get_t_coefficients(l, p.child_2()));

       case SV_TIMES:
	 return (get_t_coefficients(l, p.child_1()) * 
		get_t_coefficients(l, p.child_2()));

       case SV_POW:
	 c_1 = get_t_coefficients(l, p.child_1());
	 c_2 = get_t_coefficients(l, p.child_2());
	 if(c_2.degree() == 0) // Changed by AB -> any non -ve int power
	 {
		lp = round(c_2.get_coeff(0));
		c_2 = polynomial((sv_real)1.0);
		for(ilp = 0; ilp < lp; ilp++) c_2 = c_2*c_1;
	        return (c_2);
	 }
	 else 
	 {
	 	svlis_error("get_t_coefficients", "dud POW operator", SV_CORRUPT);
	 }
	 break;

	case SV_COMP: // Hack 0 - ...
		return(polynomial((float)0) - get_t_coefficients(l, p.child_1()));

	case SV_ABS:
		return(get_t_coefficients(l, p.child_1()));

       case SV_DIVIDE:
       default:
       		svlis_error("get_t_coefficients", "dud operator", SV_CORRUPT);
      }
      break;

    default:
      svlis_error("get_t_coefficients", "called for hard-coded primitive", SV_FATAL);
	break;
   }
   return(c_1); // Error return added by Adrian
}



sv_integer
prim_is_polynomial(const sv_primitive& p)
{
   sv_integer result = 0;

   switch(p.kind()) {
    case SV_REAL:
    case SV_PLANE:
    case SV_CYLINDER:
    case SV_SPHERE:
    case SV_CONE:
//    case SV_TORUS:
//    case SV_CYCLIDE:
      result = 1;
      break;

    case SV_GENERAL:
      switch(p.op()) {
       case SV_PLUS:
       case SV_MINUS:
       case SV_TIMES:
	 result = prim_is_polynomial(p.child_1()) && prim_is_polynomial(p.child_2());
	 break;

       case SV_POW:
	 if(prim_is_polynomial(p.child_1()) &&
	    (p.child_2().kind() == SV_REAL) // &&  -  com out byAB 
	    // (round(p.child_2().real()) == 2)
		)
	    result = 1;
	 break;

       case SV_COMP:
       case SV_ABS:
	result = prim_is_polynomial(p.child_1());
	 break;

       case SV_DIVIDE:
       default:
	 break;
      }
      break;

    default:
      break;
   }
   return result;
}

#define MAX_ROOTS 100

sorted_interval_list
ray_test(const sv_set& set_leaf_node,	// NB: this better be a leaf-node!!
	 const sv_line& ray,
	 const sv_real& rootfinding_tmin,
	 const sv_real& rootfinding_tmax)
{
   sv_primitive prim;
   sv_set no_set;
   sv_real tt;
   sv_integer poly_prim;
   polynomial t_coeffs;

   sv_integer nroots;
   double roots[MAX_ROOTS];

   sv_real test_t;
   sv_point test_point;
   sv_integer i;
   flag_val flag;

   sorted_interval_list result;

   if(set_leaf_node.contents() != 1) {
      svlis_error("ray_test", "non-leaf set_leaf_node", SV_FATAL);
   }

   prim = set_leaf_node.primitive();

#if CACHEING
   sv_integer cache_index = prim.flags();
   if(cache[cache_index].ray_number == current_ray_number) {
      result = cache[cache_index].roots;

#if DEBUG
      if(current_ray_number == debug_ray_number) {
	 cout << "Reading cache entry [" << cache_index << "] (ray# = " << current_ray_number << ")\n";
	 debug_print_sil(result,"");
	 cout << '\n';
      }
#endif

   } else {
#endif

      switch(prim.kind()) {
       case SV_REAL:
	   svlis_error("ray_test", "ray cast into an SV_REAL", SV_WARNING);
	 break;

       case SV_PLANE:
	 tt = plane_line_t(prim.plane(), ray, flag);
	 if (!flag) {
	    // root is valid, so make an interval
	    if((prim.value(ray.origin) > 0.0) == (tt > 0.0))
	       result = sorted_interval_list(sv_interval(tt,LARGE),
		set_leaf_node,no_set);
	    else
	       result = sorted_interval_list(sv_interval(-LARGE,tt),
		no_set,set_leaf_node);
	 } else {
	    // No valid roots - ray must be parallel to plane
	    if(prim.value(ray.origin) < 0.0)
	       result = sorted_interval_list(sv_interval(-LARGE,LARGE),
			no_set,no_set);
	 }
	 break;

       case SV_CYLINDER:
       case SV_SPHERE:
       case SV_CONE:
       case SV_TORUS:
       case SV_CYCLIDE:
       case SV_GENERAL:
	 poly_prim = prim_is_polynomial(prim);
	 if(poly_prim)
	    t_coeffs = get_t_coefficients(ray, prim);

//  --- AB: solve directly for degree < 4

	if(poly_prim && (t_coeffs.degree() <= 4))
		nroots = low_d_roots(t_coeffs, 1.0e-10, roots);
	else {
	    if(poly_prim) {
	       // Use ARPORS to find roots

	       nroots = arpors(t_coeffs,sv_interval(rootfinding_tmin, rootfinding_tmax),
			       max_iteration_count, arpors_tol_f, arpors_tol_t,
			       roots);
#if DEBUG
	       if(current_ray_number == debug_ray_number) {
		  cout << "arpors returns nroots = " << nroots << "\n";
		  if(nroots > 0) {
		     cout << "Roots are:\n";
		     for(i=0;i<nroots;i++)
			cout << "          " << roots[i] << "\n";
		  }
	       }
#endif
	       if(nroots < 0)
		  svlis_error("ray_test","arpors returns error status", 
			SV_WARNING);
	    } else {
	       // use ARF to find roots

	       nroots = arf(ray,prim,sv_interval(rootfinding_tmin, rootfinding_tmax),
			    arf_tol_t, MAX_ROOTS, roots);
#if DEBUG
	       if(current_ray_number == debug_ray_number) {
		  cout << "arf returns nroots = " << nroots << "\n";
		  if(nroots > 0) {
		     cout << "Roots are:\n";
		     for(i=0;i<nroots;i++)
			cout << "          " << roots[i] << "\n";
		  }
	       }
#endif
	       if(nroots < 0)
		  svlis_error("ray_test","arf returns error status",
			SV_WARNING);
	    }
	}

	    if(nroots == 0) {
	       // No roots - decide if air or solid
	       if(prim.value(line_point(ray,(rootfinding_tmin+rootfinding_tmax)/2.0)) < 0.0)
		  result = sorted_interval_list(sv_interval(-LARGE,LARGE),
			no_set,no_set);
	       else
		  result = sorted_interval_list(sv_interval(-LARGE,-LARGE+1),
			no_set,no_set);
	    } else if(nroots > 0) {
	       // Now generate interval list
	       if(nroots > 1) {
		  test_t = (roots[0]+roots[1])*0.5;
		  test_point = ray.origin+ray.direction*test_t;
		  if(prim.value(test_point) > 0.0) {
		     result = sorted_interval_list(sv_interval(-LARGE,
			roots[0]),no_set,set_leaf_node);
		     i = 1;
		  } else {
		     result = sorted_interval_list(sv_interval(roots[0],roots[1]),
			set_leaf_node,set_leaf_node);
		     i = 2;
		  }
		  roots[nroots] = LARGE;
		  while(i<nroots) {
		    if(i+1 != nroots)
		     result.add_to_tail(sv_interval(roots[i],roots[i+1]),
			set_leaf_node,set_leaf_node);
		    else
     		     result.add_to_tail(sv_interval(roots[i],roots[i+1]),
			set_leaf_node,no_set);
		     i+=2;
		  }

	       } else {
		  // Single root

		  if((prim.value(ray.origin) > 0.0) == (roots[0] > 0.0))
		     result = sorted_interval_list(sv_interval(roots[0],LARGE),
			set_leaf_node,no_set);
		  else
		     result = sorted_interval_list(sv_interval(-LARGE,roots[0]),
			no_set, set_leaf_node);
	       }
	    }
	//XXXX }
	 break;

       default:
       svlis_error("ray_test", "ray cast into a user-defined primitive", SV_WARNING);
      }

#if CACHEING
      // Remember this result
      cache_index = prim.flags();
      cache[cache_index].roots = result;
      cache[cache_index].ray_number = current_ray_number;

#if DEBUG
      if(current_ray_number == debug_ray_number) {
	 cout << "Setting cache entry [" << cache_index << "] (ray# = "
	    << current_ray_number << ")\n";
	 debug_print_sil(cache[cache_index].roots,"");
	 cout << '\n';
      }
#endif
   }
#endif

#if DEBUG
   if(current_ray_number == debug_ray_number) {
      cout << "ray_test: prim = " << (void*)&prim << "\n";
      debug_print_sil(result, "");
   }
#endif

   return result;
}
#if macintosh
 #pragma export off
#endif






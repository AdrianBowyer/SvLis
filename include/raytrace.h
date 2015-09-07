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
 * SvLis - raytracing header file
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


#ifndef SVLIS_RAYTRACE
#define SVLIS_RAYTRACE

sv_set ray_model_test(const sv_model&, const sv_line&, const sv_real&, 
    const sv_interval&, sv_real*);
sv_set ray_leaf_node_test(const sv_set_list&, const sv_line& , const sv_real& , 
	const sv_interval&, sv_real*);
sorted_interval_list ray_set_intersection_test(const sv_set&, const sv_line&, 
	const sv_real&, const sv_real&);
sorted_interval_list ray_test(const sv_set&, const sv_line&, const sv_real&, const sv_real&);
sv_integer init_raytrace_cache(sv_set&);
void destroy_raytrace_cache(void);
sv_integer prim_is_polynomial(const sv_primitive&);
void set_debug_ray_number(sv_integer);
sv_integer get_debug_ray_number(void);

#endif

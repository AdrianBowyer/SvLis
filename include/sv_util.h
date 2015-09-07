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
 * SvLis - utilities: a file of useful things
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 9 June 1995
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_UTILITIES
#define SVLIS_UTILITIES
 
// A file of useful things to do with Svlis

// Integral properties of the set in a model

extern void integral(const sv_model&, sv_real, sv_real&, sv_point&, sv_point&, sv_point&); 
extern sv_real area(const sv_model&);
extern sv_real area(const sv_model&,  const sv_set&);

// Statistics from the integral function
//  the total number of random points

extern sv_integer integral_points();

// Set and return an absolute fix on the number of
// points to use for a Monte Carlo - 0 disables

void constant_work(sv_integer i);
sv_integer constant_work();

// Force the system to use a specific number of points
// per unit volume (-1 disables)

void integral_points(sv_real);

// Generate a random point uniformly inside a box

extern sv_point ran_point(const sv_box&);

// Minimum and maximum squared distance between two boxes

extern sv_interval dist_2(const sv_box&, const sv_box&);

// Newton-Raphson for a primitive

extern sv_point newton(const sv_primitive&, sv_point, sv_real);

// Use Newton-Raphson to find a corner for a set

sv_point corner(const sv_set& s, sv_point p, sv_real accy);

// Use Newton-Raphson to find a point on an edge for a set

sv_point edge(const sv_set& s, sv_point p, sv_real accy);

// Move a distance (roughly) r along an edge in (roughly) direction d

sv_point edge_step(const sv_set&, sv_point, const sv_point&, 
    sv_real, sv_real);

// Find a zero of a primitive along the line between two points

sv_point binary_chop(const sv_primitive& p, sv_point p0, sv_real v0,
		sv_point p1,  sv_real v1, sv_real accy);

// returns a face of a box as planes

sv_plane box_face(const sv_box&, sv_integer);

// makes a wireframe box from a svlis box

sv_set wireframe_box(const sv_box&);

// Return the crikliness of a FACETED model

sv_real crinkliness(const sv_model&);

// Slice a dimension out of a svLis box

sv_box slice(const sv_box&, const mod_kind, const sv_real);

// Slice a dimension out of a svLis plane

sv_primitive slice(const sv_plane&, const mod_kind, const sv_real);

// Slice a dimension out of a svLis primitive

sv_primitive slice(const sv_primitive&, const mod_kind, const sv_real);

// Slice a dimension out of a svLis set

sv_set slice(const sv_set&, const mod_kind, const sv_real);

// Slice a dimension out of a svLis set list

sv_set_list slice(const sv_set_list&, const mod_kind, const sv_real);

// Slice a dimension out of a svLis model

sv_model slice(const sv_model&, const mod_kind, const sv_real);

// Procedure to rewrite a svLis set tree
// in the Disjunctive Form.

sv_set disjunctive_form(const sv_set&);

// Put a set into the disjunctive form and pull out
// the products into a list.

sv_set_list list_products(const sv_set&);

// Approximations to the natural quadrics as polyhedra
// These are useful as the faceter renders convex polyhedra
// very quiclky, so they can be used as approximations
// to set things up

// Set and recover the polygonal accuracy

void polygon_approximation(sv_real);
sv_real polygon_approximation();
void polygon_faces(sv_integer);
sv_integer polygon_faces();

// What N-gon is needed to approcimate a circle
// to the desired accuracy?

sv_integer poly_count();

// Cylinder, cone, and sphere

sv_set poly_cylinder(const sv_line&, sv_real);

sv_set poly_cone(const sv_line&, sv_real);

sv_set poly_sphere(const sv_point&, sv_real);

// Whether to use Niederreiter (1) or uniform (0) values
// for random box points

void sv_niederreiter_ran(int);

#endif





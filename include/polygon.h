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
 * SvLis - sv_p_gon: a closed loop of points representing a polygon
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
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_POLYGON
#define SVLIS_POLYGON

// sv_p_gon - a closed linked-list loop of points representing a polygon in space
// (this will usually, though not necessarily, lie in a plane), a polyline
// (in which case the linked list - though closed at the end - is considered
// open and the last link not used), or a point set.

struct sv_p_gon
{
	sv_point p;		// The point
	sv_point g;		// the surface grad at that point
	short edge;		// what sort of edge leaves this point
	sv_p_gon_kind kind;	// What the polygon is.
	sv_p_gon* next;		// Link to the next one
	
	sv_integer tag();
};

// I/O

extern istream& operator>>(istream&, sv_p_gon_kind&);
extern ostream& operator<<(ostream&, sv_p_gon*);
extern void write(ostream&, sv_p_gon*,  sv_integer);
extern istream& operator>>(istream&, sv_p_gon**);
extern void read(istream&, sv_p_gon**);
extern void read1(istream&, sv_p_gon**);
extern void read(istream&, sv_p_gon_kind&);
extern void read1(istream&, sv_p_gon_kind&);

// Initialize a polygon with a single point

inline sv_p_gon* first_point(const sv_point& q, sv_p_gon_kind k)
{
	sv_p_gon* res = new sv_p_gon;
	res->p = q;
	res->next = res;	// Loops back to itself
	res->edge = 0;
	res->kind = k;
	return(res);
}

// Add a point in the linked list after pg; returns pointer to added point
// The resulting edge is visible.

inline sv_p_gon* add_edge(sv_p_gon* pg, const sv_point& q)
{
	sv_p_gon* res = new sv_p_gon;
	res->p = q;
	res->edge = 0; 
	res->kind = pg->kind;
	res->next = pg->next;
	pg->next = res;
	return(res);
}

// Make the edge starting at pg (in)visible

inline void make_invisible(sv_p_gon* pg)
{
	if(pg)
		pg->edge = pg->edge | INVISIBLE; 
}
inline void make_visible(sv_p_gon* pg) 
{
	if(pg)
		pg->edge = pg->edge & (~INVISIBLE);
}

// return the number of vertices

extern sv_integer p_gon_vertex_count(sv_p_gon*);

// Set the grad values from a parent primitive

extern void set_p_gon_grad(sv_p_gon*, const sv_primitive&);

// delete flagged points after clipping

extern sv_p_gon* tidy_p_gon(sv_p_gon*);

// Throw away a whole polygon

extern void kill_p_gon(sv_p_gon*);

// Copy a whole polygon

extern sv_p_gon* copy_p_gon(sv_p_gon*);

// Copy a whole polygon changing its kind

extern sv_p_gon* copy_p_gon(sv_p_gon*, sv_p_gon_kind);

// This chops the corners off the polygon that lie outside a plane

extern sv_p_gon* p_gon_chop(sv_p_gon*, const sv_plane&);

// Work out the mean triangle normal vector of a polygon

extern sv_point p_gon_tri_norm(sv_p_gon*);

// Work out the approximate area of a polygon

inline sv_real p_gon_area(sv_p_gon* pg)
{
	return((p_gon_tri_norm(pg).mod())*0.5);
}

// Retained for backwards compatibility

inline void set_user_facet_fac(sv_real a) { set_small_volume(a); }
inline sv_real get_user_facet_fac() { return(get_small_volume()); }

// Compute the polygon that a box carves out of a primitive

extern sv_p_gon* prim_box_p_gon(const sv_primitive&, const sv_box&, const sv_point&);

//***************************************************************

// Decomposition of a box into tetrahedra containing polygons
// The lookup tables for this class are in interval.h (with other stuff
// for boxes).

class sv_box_tets
{
private:
    sv_set s;		// The leaf set for which these are the facets
    mem_test diff_sign; // Box air, solid, or contains surface?
    sv_p_gon* pg[6];       // One polygon for each of the 6 tets
    sv_p_gon* pgn[6];      // ... plus ones for notching

public:

// Null constructor
    
    sv_box_tets()
    {
	for(int i = 0; i < 6; i++) 
	{
	  pg[i] = 0;
	  pgn[i] = 0;
        }
	diff_sign = SV_AIR;
    }

// Build from a set and box with a given accuracy
    
    sv_box_tets(const sv_set&, const sv_box&, sv_real, sv_integer);

// Delete the entire contents

    void clean_out()
    {
	for(int i = 0; i < 6; i++)
	{
	  kill_p_gon(pg[i]);
	  kill_p_gon(pgn[i]);
	  pg[i] = 0;
	  pgn[i] = 0;
        }
    }

// Turn all the polygons into an attribute
    
    sv_attribute attribute();

// Return the sign of all the box corners
    
    mem_test definite() { return(diff_sign); }

// Return the set

   sv_set set() { return(s); }

// Clip the tets to a primitive

    void clip(const sv_primitive&, sv_real);

// Clip to create a polyline

    void line(const sv_primitive&, sv_real);

// Notch a tet pattern to the union of two primitives

    void notch(const sv_primitive&, const sv_primitive&, sv_real);

};

// Set the gradient factor

extern void set_user_grad_fac(sv_real);
extern sv_real get_user_grad_fac();

// Box aspect ratio calculation

inline int too_thin(sv_real x, sv_real y, sv_real z)
{
	sv_real s = min(x, min(y, z));
	sv_real t = max(x, max(y, z));
	return((s/t) < LONG_THIN);
}

// Write a faceted model to a VRML file

extern void sv_to_vrml(ostream&, const sv_model&);

inline void sv_to_vrml(char* file_name, const sv_model& m)
{
    ofstream opf(file_name);
    if(!opf)
    {
        svlis_error("sv_to_vrml","can't open output file", SV_WARNING);
        return;
    }
    
    sv_to_vrml(opf, m);
    opf.close();
}

// Walk a model returning all its facets as a single attribute
// Use with care

extern sv_attribute get_all_facets(const sv_model&);

#endif



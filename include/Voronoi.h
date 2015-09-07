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
 * SvLis - Voronoi diagram code
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 20 June 1998
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_VORONOI
#define SVLIS_VORONOI

// Set and return floating-point accuracy

extern void set_accuracy(sv_real);
extern sv_real get_accuracy();

// Predeclarations

class sv_vertex;
class sv_voronoi;

//******************************************************************************

// The Delaunay class
// A single data point

class sv_delaunay
{
private:

    friend class sv_vertex;
    friend class sv_voronoi;

    sv_point p;      // The point in the Voronoi diagram
    sv_set own;      // The set where it came from
    sv_model m;	     // The model where it came from
    sv_integer flg;  // Setting bits for housekeeping
    sv_delaunay* nx; // Next point - used for chains
    sv_vertex* t;    // A vertex on this point's territory
    
    sv_delaunay(const sv_point& pp) 
    {
	p = pp;
	nx = 0;
	flg = 0;
	t = 0;
    }
        
public:

// Set and reset flags.

    void set_flag(sv_integer f) { flg = flg | f; }
    void reset_flag(sv_integer f) { flg = flg & (~f); }
          
    sv_point point() const {return(p);}
    void set(const sv_set& s) {own = s;}
    void model(const sv_model& mm) {m = mm;}
    sv_set set() const {return(own);}
    sv_model model() const {return(m);}
    sv_delaunay* next() const {return(nx);}
            
// Return the flags that have been set

    sv_integer flag() const {return(flg);}
        
// This point on the convex hull?

    int c_hull() const { return(flg & SV_CH); }

// Unique tag

    sv_integer tag() const;

};

//*******************************************************************************

// The sv_vertex class - data structure for a single
// Voronoi diagram territory-boundary vertex.

class sv_vertex
{
private:
    friend class sv_voronoi;
    
    sv_delaunay* d[SV_VD1]; // The Delaunay tetrahedron
    sv_vertex* v[SV_VD1];   // Neighbouring vertices
    sv_vertex* nx;          // Temporary vertex linked list
    sv_point pos;           // The vertex's position
    sv_real r2;             // The vertex's squared radius
    sv_integer flg;         // Setting bits for housekeeping

// Build a null vertex - only used for vertices at infinity

    sv_vertex();

// Initialize a vertex with a single Delaunay point.

    sv_vertex(sv_delaunay*);

// Set point number i for the vertex.
   
    void set_delaunay(sv_delaunay* dd,  sv_integer i)
    {
	d[i] = dd;
	dd->t = this;
    }

// Compute the centre and squared radius of the vertex's circumsphere

    int set_centre();

// Build part of the vertex structure between a dead and a preserved vertex

    int ring(sv_vertex*, sv_vertex*);
    
public:

// Set and reset flags.

    void set_flag(sv_integer f) { flg = flg | f; }
    void reset_flag(sv_integer f) { flg = flg & (~f); }  

// Return Delaunay point i.

    sv_delaunay* delaunay(sv_integer i) const {return(d[i]);}

// Return vertex neighbour i (NB may be at infinity)

    sv_vertex* neighbour(sv_integer i) const {return(v[i]);}

// Return the co-ordinates of the circumcentre

    sv_point position() const {return(pos);}

// Return the squared radius

    sv_real r_squared() const {return(r2);}

// Return the centroid

    sv_point centroid() const;

// Next entry in the chain
    
    sv_vertex* next() const {return(nx);}

// Return the flags that have been set

    sv_integer flag() const {return(flg);}

// Return TRUE if this vertex is at infinity.

    int infinity() const {return(d[0] == 0);}

// Unique tag

    sv_integer tag() const;
};

// Find the tetrahedron that contains a point

extern sv_vertex* find_enclosing_tet(sv_vertex*, const sv_point&);

// Walk vertices resetting flags

extern void reset_flags(sv_vertex*, sv_integer);

// Compute the signed volume determinant of a tetrahedron
// [For the actual volume take abs(tet_vol)/6]

inline sv_real tet_vol(const sv_point& k, const sv_point& l, const sv_point& m, const sv_point& n)
{
        sv_point lk = l - k;
        sv_point mk = m - k;
        sv_point nk = n - k;
        sv_point d = mk^nk;
        return(lk*d);
}

//******************************************************************************

// The highest-level Voronoi class: sv_voronoi::

class sv_voronoi
{
private:
    sv_vertex* at_inf[SV_VD1]; // The vertices at infinity
    sv_vertex* walk_s;	       // Walk start
    sv_integer d_count;        // Count of points
    sv_vertex* link_v;         // Temporary linked list of vertices
    sv_delaunay* link_d;       // Temporary linked list of points 
    sv_box b;                  // enclosing box

// Private member functions to build a new territory

    void build_new(sv_vertex*, sv_delaunay*);
    void link_new();
    void flag_dead(sv_vertex*, sv_delaunay*);
    void delete_dead_vertices(sv_vertex*);
    
// And for building lists

    void dneighbours_r(sv_delaunay*, sv_vertex*);
    void vneighbours_r(sv_delaunay*, sv_vertex*);
    void vneighbours2_r(sv_delaunay*, sv_delaunay*, sv_vertex*, sv_integer);

// Build the initial tetrahedron directly
   
    sv_voronoi(sv_delaunay* d[]);
    
// Add an existing point to a Voronoi diagram

    sv_delaunay* add_point(sv_delaunay*);    
     
public:

// Initialize a null voronoi diagram

    sv_voronoi() {walk_s = 0;}

// Initialize an empty Voronoi diagram in an enclosing box

    sv_voronoi(const sv_box& bb);

// Add a point to a Voronoi diagram

    sv_delaunay* add_point(const sv_point&);

// Return the start vertex for recursive walks.  This will always be a vertex
// of the last inserted point.  Use it but don't save it - it may be deleted
// by subsequent point insertions.
    
    sv_vertex* walk_start() const { return(walk_s);}

// Return the total number of points in the structure.

    sv_integer point_count() const { return(d_count); }

// Return the initial box

    sv_box box() const {return(b);}

// Return the nearest neighbour of a point (that is
// the Delaunay point in whos territory the point lies).

    sv_delaunay* nearest(const sv_point&);
    
// Return a linked list which contains all the Voronoi
// neighbours of a Delaunay point

    sv_delaunay* neighbours(sv_delaunay*);
    
// Return a linked list which contains all the vertices
// of a point's territory

    sv_vertex* territory(sv_delaunay*);
    
// Return a linked list which contains all the vertices
// common to two neighbouring points.

    sv_vertex* contiguity(sv_delaunay*, sv_delaunay*);
        
// Anything in there?

    int exists() const {return(!(walk_s==0));}

// Unique tag

    sv_integer tag() const;

// Print it

    void write(ostream&);
};

sv_voronoi tet_mesh(const sv_model&, sv_real);

// Graphics procedures

extern void plot_voronoi(const sv_voronoi&, char*, 
  sv_integer, sv_integer, sv_integer,
	sv_real);

inline void plot_voronoi(const sv_voronoi& v)
{
     plot_voronoi(v, "SvLis Voronoi", 1, 0, 1, (sv_real)0.8);
}

//**********************************************************************************



#endif

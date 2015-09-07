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
 * This version: 7 July 2000
 *
 */



#include "svlis.h"
#if macintosh
 #pragma export on
#endif

//*****************************************************************************************

// The Delaunay class
// A single data point

static sv_real accuracy = 1.0e-6;

void set_accuracy(sv_real a) { accuracy = a; }
sv_real get_accuracy() { return(accuracy); }

// Unique tag

sv_integer sv_delaunay::tag() const
{
        return(SVT_F*SVT_DELAUNAY);
}

//******************************************************************************************

// The vertex class - data structure for a single
// Voronoi diagram territory-boundary vertex.

// Null vertex constructor
// A vertex where d[0] stays 0 is at infinity

sv_vertex::sv_vertex() 
{ 
	flg = 0;
	nx = 0;
	for(int i = 0; i <= SV_VD; i++) 
	{
		v[i] = 0;
		d[i] = 0;
	}
}

// Initial constructor for the first point
    
sv_vertex::sv_vertex(sv_delaunay* dd) 
{
	flg = 0;
	nx = 0;
	for(int i = 0; i <= SV_VD; i++) 
	{
		v[i] = 0;
		d[i] = 0;
	}	
	d[0] = dd;
	dd->t = this;
}


// This vertex lies on the line between ded (which will be deleted by the insertion
// of a new point) and live_n - a neighbour of ded which remains alive.  d[0] will
// already have been set to the new Delaunay point.  Note that live_n may be at infinity.

int sv_vertex::ring(sv_vertex* ded, sv_vertex* live_n)
{
    int di = 0;
    int i;
    sv_delaunay* dn;

// live_n must be opposite d[0] (which is already set).

    v[0] = live_n;


// All the points which circumscribed ded must circumscribe the
// new vertex except for the one opposite live_n 

    for(i = 0; i <= SV_VD; i++)
    {
	dn = ded->delaunay(i);

// If the corresponding neighbour is not live_n then dn
// must be another forming point for this vertex.
  
	if (ded->neighbour(i) != live_n)
	{

// If this forming point used to have a just-dead vertex as its t, make
// its t this.

		if (dn->t->flag() & SV_DED) dn->t = this;
		di++;
		d[di] = dn;
	} 
    }

// Check for trouble

    if(di != SV_VD)
    {
	svlis_error("sv_vertex::ring(...)", "ring incomplete", SV_CORRUPT);
        return(0);
    }

// Make live_n point to the new vertex instead of ded

    di = 1;
    for(i = 0; i <= SV_VD; i++)
	if (live_n->neighbour(i) == ded)
	{
		live_n->v[i] = this;
		di = 0;
	}

    if(di)
    {
	svlis_error("sv_vertex::ring(...)", "back-vertex not found", SV_CORRUPT);
        return(0);
    }

// Set the centre and squared radius of the circumsphere

    return(set_centre());
} 

sv_point sv_vertex::centroid() const
{
	sv_point p = SV_OO;
	for(int i = 0; i <= SV_VD; i++)
		p = p + delaunay(i)->point();

	return(p/(sv_real)SV_VD1);
}

// Circumcentre and squared radius of a vertex tetrahedron
// See `A programmer's Geometry', page 120

int sv_vertex::set_centre()
{
	sv_point k = d[0]->point();

	sv_point lk = d[1]->point() - k;
	sv_point mk = d[2]->point() - k;
	sv_point nk = d[3]->point() - k;

	sv_point dd = mk^nk;

	sv_real det = lk*dd;

	if(fabs(det) < get_accuracy()) return(0);

	sv_real detinv = 0.5/det;

	sv_point rsq = sv_point(lk*lk, mk*mk, nk*nk);

	sv_point dr = rsq.y*nk - rsq.z*mk;

	pos = sv_point(rsq.x*dd.x - lk.y*dr.z + lk.z*dr.y,
		lk.x*dr.z + rsq.x*dd.y - lk.z*dr.x,
		-lk.x*dr.y + lk.y*dr.x + rsq.x*dd.z);

	pos = pos*detinv;
	r2 = pos*pos;
	pos = pos + k;

	return(1);
}

// Unique tag

sv_integer sv_vertex::tag() const
{
        return(SVT_F*SVT_VERTEX);
}

// Slight hack - search out from w across
// neighbours to see if any of them have a vertex the radius of
// which is greater than their vertex's distance to p

sv_vertex* within_d(sv_vertex* w, const sv_point& p)
{
  sv_point q =  p - w->position();
  if(q*q <= w->r_squared()) return(w);
  return(0);
}

sv_vertex* search_the_lot_r(sv_vertex* w, const sv_point& p)
{
	sv_vertex* n;
	sv_vertex* result;

	if(!w->infinity())
	{
	        w->set_flag(SV_VISITED_1);
		if(result = within_d(w, p)) return(result);
		for(int i = 0; i < SV_VD1; i++)
		{
			n = w->neighbour(i);
			if(!(n->flag() & SV_VISITED_1))
			{
			  if(result = search_the_lot_r(n, p)) return(result);
			}
		}
	}
	return(0);
}

sv_vertex* search_the_lot(sv_vertex* w, const sv_point& p)
{
  sv_vertex* result = search_the_lot_r(w, p);
  reset_flags(w, SV_VISITED_1);
  return(result);
}


// Find the tetrahedron that contains a point

sv_vertex* find_enclosing_tet(sv_vertex* ww, const sv_point& p)
{
	sv_vertex* w = ww;
	sv_vertex* next = w;
	sv_vertex* n;
	sv_real tv;
	sv_point p0, p1, p2, p3;

	do
	{
		w = next;
		w->set_flag(SV_WALK);
		next = 0;
		n = 0;
		p0 = w->delaunay(0)->point();
		p1 = w->delaunay(1)->point();
		p2 = w->delaunay(2)->point();
		p3 = w->delaunay(3)->point();
		tv = tet_vol(p0, p1, p2, p3);
		if(tv*tet_vol(p, p1, p2, p3) < 0) 
		{
			n = w->neighbour(0);
			if(!(n->flag() & SV_WALK) && !n->infinity()) next = n;
		}
		if(!next) if(tv*tet_vol(p0, p, p2, p3) < 0)
		{
			n = w->neighbour(1);
			if(!(n->flag() & SV_WALK) && !n->infinity()) next = n;
		}
		if(!next) if(tv*tet_vol(p0, p1, p, p3) < 0)
		{
			n = w->neighbour(2);
			if(!(n->flag() & SV_WALK) && !n->infinity()) next = n;
		}
		if(!next) if(tv*tet_vol(p0, p1, p2, p) < 0)
		{
			n = w->neighbour(3);
			if(!(n->flag() & SV_WALK) && !n->infinity()) next = n;
		}
		if(n && !next)
		{
			next = search_the_lot(w,p);
			if(!next) svlis_error("find_enclosing_tet","no tet found",SV_WARNING);
			reset_flags(ww, SV_WALK);
			return(next);
		}
	} while(next);

	reset_flags(ww, SV_WALK);

	return(w);
}

// Recursively walk the vertex structure resetting all vertices flagged as f.

void reset_flags(sv_vertex* start, sv_integer f)
{
	sv_vertex* next;
	sv_integer i;

	start->reset_flag(f);

	for(i = 0; i <= SV_VD; i++)
		start->delaunay(i)->reset_flag(f);

	for(i = 0; i <= SV_VD; i++)
	{
		next = start->neighbour(i);
		if(next->flag() & f)
			reset_flags(next, f);
	}
}


//*****************************************************************************************

// The highest level Voronoi class: sv_voronoi::

// Constructor - initialize a Voronoi diagram in an enclosing
// tetrahedron of points in d[].

sv_voronoi::sv_voronoi(sv_delaunay* d[])
{
	link_v = 0;
	link_d = 0;

	for(int i = 0; i <= SV_VD; i++)
	{
		d[i]->set_flag(SV_CH);
		if(!i)
			walk_s = new sv_vertex(d[i]);
		else
			walk_s->set_delaunay(d[i], i);
		at_inf[i] = new sv_vertex();
		walk_s->v[i] = at_inf[i];
		at_inf[i]->v[0] = walk_s;
	}
	walk_s->set_centre();

	d_count = SV_VD1;			
}


// Constructor - initialize a Voronoi diagram in an enclosing box

sv_voronoi::sv_voronoi(const sv_box& bb)
{
	sv_delaunay* d[SV_VD1];

	sv_real sc = 5*sqrt(bb.diag_sq());

// Build a tetrahedron that encloses the box

	sv_point cen = b.centroid();
	d[0] = new sv_delaunay(cen + sv_point(0, 0, sc));
	d[1] = new sv_delaunay(cen + sv_point(0, 0.25*sqrt(15)*sc, -sc/3));
	d[2] = new sv_delaunay(cen + sv_point(-0.125*sqrt(45)*sc, 
		-0.125*sqrt(15)*sc, -sc/3));
	d[3] = new sv_delaunay(cen + sv_point(0.125*sqrt(45)*sc, 
		-0.125*sqrt(15)*sc, -sc/3));
	*this = sv_voronoi(d);
        b = bb;
}


// Add a point to the structure

sv_delaunay* sv_voronoi::add_point(sv_delaunay* d)
{

// Initialize the linked list of new vertices

	link_v = 0;

// General case - find a vertex that will be killed

	sv_vertex* ded = find_enclosing_tet(walk_s, d->point());

	if(!ded)
	{
		svlis_error("sv_voronoi::add_point",
			"invalid vertex to delete",SV_FATAL);
		return(0);
	}

// Outside the convex hull?

	if(ded->infinity())
	{
		svlis_error("sv_voronoi::add_point",
			"point outside convex hull",SV_FATAL);
		return(0);
	}

// Recursively walk the vertex structure flagging all vertices that 
// will be killed by the new point.

	flag_dead(ded, d);

// Recursively re-visit those deleted vertices starting the construction of all the new ones.

	build_new(ded, d);

// Complete the construction of the new vertices, which are now held in the linked list
// starting at link.

	link_new();

// Remove the deleted vertices.

	delete_dead_vertices(ded);
	
	d_count++;
	return(d);
}

// Add a point to the structure

sv_delaunay* sv_voronoi::add_point(const sv_point& p)
{
	sv_delaunay* d = new sv_delaunay(p);
	return(add_point(d));
}


// After the points have been set up for a new territory by build_new along with
// the pointers out from the new vertices to existing old ones, this links up the new vertices
// with each other.

void sv_voronoi::link_new()
{
	sv_vertex* n1;
	sv_vertex* n2;
	sv_integer com_c, i, j;
	sv_integer n1_not_n2[SV_VD1], n2_not_n1[SV_VD1];
	sv_delaunay* dc;

// Outer loop through all new vertices

	n1 = link_v;
	while(n1)
	{

// Inner loop through all new vertices

		n2 = link_v;
		while(n2)
		{
			if(n2 != n1)
			{

// Initialize the flag arrays for delaunay points that are in n1's list and not n2's, and
// in n2's list but not n1's.

				for(i = 0; i <= SV_VD; i++)
				{
					n1_not_n2[i] = 1;
					n2_not_n1[i] = 1;
				}

// Zero the counter for common points between n1 and n2

				com_c = 0;

// Outer loop through n1's points

				for(i = 0; i <= SV_VD; i++)
				{
					dc = n1->delaunay(i);

// Inner loop through n2's points

				 	for(j = 0; j <= SV_VD; j++)
					{

// If the two points are the same, increment the common counter, and flag the fact
// that i and j cannot be uncommon points in n1's list and n2's list repectively

						if(n2->delaunay(j) == dc)
						{
							com_c++;
							n2_not_n1[j] = 0;
							n1_not_n2[i] = 0;
						}
					}
				}

// If the common counter is the same as the number of dimensions of the tessellation, then n1 and n2
// must be linked.  The vertex n2 is opposite the point for which n1_not_n2 remains 1 in n1's list, 
// and similarly for n1.

				if(com_c == SV_VD)
				{
					com_c = 0;
					for(i = 0; i <= SV_VD; i++)
					{
						if(n1_not_n2[i]) 
						{
							n1->v[i] = n2;
							com_c++;
						}
						if(n2_not_n1[i])
						{
							n2->v[i] = n1;
							com_c++;
						}
					}
					if(com_c != 2)
						svlis_error("sv_voronoi::link","symmetry not found",SV_CORRUPT);
				}
			}
			n2 = n2->next();
		}
		n1 = n1->next();
	}
}




// Recursively build a new territory for Delaunay point d. The vertex ded is one that will
// be deleted by the new point

void sv_voronoi::build_new(sv_vertex* ded, sv_delaunay* d)
{
	sv_vertex* next;
	sv_vertex* n = 0;

	ded->set_flag(SV_VISITED_1);

	for(int i = 0; i <= SV_VD; i++)
	{
		next = ded->neighbour(i);

// If the neighbour is also dead, visit it recursively; SV_VISITED_1 ensures that each
// gets visited only once.

		if (next->flag() & SV_DED)
		{
 			if(!(next->flag() & SV_VISITED_1))
			    build_new(next, d);
		} else
		{

// If the neighbour is not dead, then there must be a new vertex somewhere along the link from ded
// to next.

		    n = new sv_vertex(d);

// Add n to the linked list of new vertices.

		    n->nx = link_v;
		    link_v = n;

// Set the new vertex as the walk start

		    walk_s = n;

// Set the points and one initial vertex neighbour for the new vertex.

		    n->ring(ded, next);
		}
	}
}



// Recursively walk the vertex structure flagging all vertices that will be deleted by Delaunay
// point d.  The vertex ded is one that will be killed and is used as a start vertex.

void sv_voronoi::flag_dead(sv_vertex* ded, sv_delaunay* d)
{
	sv_vertex* next;
	sv_point gap;

	if(ded->infinity()) return;

	ded->set_flag(SV_DED);

// Look at all ded's neighbouring vertices.

	for(int i = 0; i <= SV_VD; i++)
	{
		next = ded->neighbour(i);

// If they're not at infinity, and they're not already flagged, check if they've gotta go.

		if(!next->infinity() && (!(next->flag() & SV_DED)))
		{

// Any vertex that is closer to d than its own circumsphere radius must be deleted.

		        if(within_d(next, d->point())) flag_dead(next, d);
			//gap = next->position() - d->point();
			//if (gap*gap < next->r_squared()) flag_dead(next, d);
		}
	}
}



// Release storage for deleted vertices

void sv_voronoi::delete_dead_vertices(sv_vertex* ded)
{
	sv_vertex* next;

	ded->set_flag(SV_VISITED_3);
	for(int i = 0; i <= SV_VD; i++)
	{
		next = ded->neighbour(i);
		if(next->flag() & SV_DED)
		{

// Visit the neighbours...

		    if( !(next->flag() & SV_VISITED_3) ) delete_dead_vertices(next);
		}
	}

// ... _then_ delete this one

	delete ded;
}

// Unique tag

sv_integer sv_voronoi::tag() const
{
        return(SVT_F*SVT_VORONOI);
}

// Return the nearest neighbour of a point (that is
// the Delaunay point in whos territory the point lies).

sv_delaunay* sv_voronoi::nearest(const sv_point& p)
{

// Start by finding the tet in which p lies

	sv_vertex* v0 = find_enclosing_tet(walk_s, p);
	if(!v0)
	{
		svlis_error("sv_voronoi::nearest", "no enclosing tet found",
			SV_WARNING);
		return(0);
	} else
		walk_s = v0;  // Assume we'll be searching near here in the future

// Find the nearest Delaunay point on that tet to p

	sv_delaunay* dmin = v0->delaunay(0);
	sv_delaunay* dm;
	sv_point pd = p - dmin->point();
	sv_real rmin2 = pd*pd;
	sv_real r2;
	for(int i = 1; i <= SV_VD ; i++)
	{
		dm = v0->delaunay(i);
		pd = p - dm->point();
		if((r2 = pd*pd) < rmin2)
		{
			rmin2 = r2;
			dmin = dm;
		}
	}

// Do a Delaunay neighbour walk from dmin to find the nearest
// neighbour

	sv_integer searching = 1;

	while(searching)
	{
		searching = 0;
		dm = neighbours(dmin);
		while(dm)
		{
			pd = p - dm->point();
			if((r2 = pd*pd) < rmin2)
			{
				rmin2 = r2;
				dmin = dm;
				searching = 1;
			}
			dm = dm->next();
		}
	}
	return(dmin);	
}
    
// Return a linked list which contains all the Voronoi
// neighbours of a Delaunay point - internal recursion

void sv_voronoi::dneighbours_r(sv_delaunay* dpt, sv_vertex* start)
{
	sv_delaunay* nb;
	sv_integer i, j;

// Flag start as visited

	start->set_flag(SV_VISITED_1);

// Add start's Delaunay points to the list if they're not already on it

	for(i = 0; i <= SV_VD; i++)
	{
		nb = start->delaunay(i);
		if(nb != dpt)
		{
			if(!(nb->flag() & SV_VISITED_1))
			{
				nb->set_flag(SV_VISITED_1);
				nb->nx = link_d;
				link_d = nb;
			}	
		}
	}

// Recurse through start's neighbouring vertices

	sv_vertex* vb;
	sv_integer visit;

	for(i = 0; i <= SV_VD; i++)
	{
		vb = start->neighbour(i);
		if(!(vb->flag() & SV_VISITED_1) && !vb->infinity())
		{
			visit = 0;
			j = 0;
			while((j <= SV_VD) && !visit)
			{
				nb = vb->delaunay(j);
				if(nb == dpt) visit = 1;
				j++;
			}
			if(visit) dneighbours_r(dpt, vb);
		}
	}
}

// Return a linked list which contains all the Voronoi
// neighbours of a Delaunay point - user-called function

sv_delaunay* sv_voronoi::neighbours(sv_delaunay* dpt)
{
	sv_vertex* start = dpt->t;
	if(!start)
	{
		svlis_error("sv_voronoi::neighbours", "zero Delaunay vertex",
			SV_CORRUPT);
		return(0);
	}

	link_d = 0;

	dneighbours_r(dpt, start);

	reset_flags(start, SV_VISITED_1);

	return(link_d);
}

// Return a linked list which contains all the Voronoi
// vertices of a Delaunay point - internal recursion

void sv_voronoi::vneighbours_r(sv_delaunay* dpt, sv_vertex* start)
{
	sv_integer i;

// Flag start as visited

	start->set_flag(SV_VISITED_1);

// Recurse through start's neighbouring vertices

	sv_vertex* vb;

	for(i = 0; i <= SV_VD; i++)
	{
		vb = start->neighbour(i);
		if(!(vb->flag() & SV_VISITED_1) && !vb->infinity() && (vb->d[i] != dpt))
		{
			vb->nx = link_v;
			link_v = vb;	
			vneighbours_r(dpt, vb);
		}
	}
}
   
// Return a linked list which contains all the vertices
// of a point's territory

sv_vertex* sv_voronoi::territory(sv_delaunay* d)
{
	sv_vertex* start = d->t;
	if(!start)
	{
		svlis_error("sv_voronoi::territory", "zero Delaunay vertex",
			SV_CORRUPT);
		return(0);
	}

	link_v = 0;

	vneighbours_r(d, start);

	reset_flags(start, SV_VISITED_1);

	return(link_v);
}

// Return a linked list which contains all the Voronoi
// vertices of a two Delaunay points - internal recursion

void sv_voronoi::vneighbours2_r(sv_delaunay* d0, sv_delaunay* d1, sv_vertex* start, sv_integer looping)
{
	sv_integer i, j;

// Flag start as visited

	start->set_flag(SV_VISITED_1);

// Recurse through start's neighbouring vertices

	sv_vertex* vb;
	sv_integer common;

	for(i = 0; i <= SV_VD; i++)
	{
		vb = start->neighbour(i);
		if(!(vb->flag() & SV_VISITED_1) && !vb->infinity() && (vb->d[i] != d0))
		{
			j = 0;
			common = 0;
			while( (j <= SV_VD) && !common )
			{
				common = (vb->d[j] == d1);
				j++;
			}
			if(common)
			{
				looping = 1;
				vb->nx = link_v;
				link_v = vb;
				vneighbours2_r(d0, d1, vb, looping);
			}
			if(!looping) vneighbours2_r(d0, d1, vb, looping);
		}
	}
}
    
// Return a linked list which contains all the vertices
// common to two neighbouring points.

sv_vertex* sv_voronoi::contiguity(sv_delaunay* d0, sv_delaunay* d1)
{
	sv_vertex* start = d0->t;
	if(!start)
	{
		svlis_error("sv_voronoi::contiguity", "zero Delaunay vertex",
			SV_CORRUPT);
		return(0);
	}

	link_v = 0;

	vneighbours2_r(d0, d1, start, 0);

	reset_flags(start, SV_VISITED_1);

	return(link_v);
}

// Mesh a model
// NB sv_voronoi should be a hidden class like the rest of svLis

static sv_voronoi tet_mod;

// Categorize all tets as being solid or empty

void cat_vtxs(sv_vertex* v, const sv_model& m)
{
	sv_vertex* vn;
	sv_primitive dum;

	v->set_flag(SV_WALK);

	sv_point p = v->centroid();

	if (m.member(p, &dum) != SV_SOLID) v->set_flag(SV_EMPTY);

	for(int i = 0; i <= SV_VD; i++)
	{
		vn = v->neighbour(i);
		if(!vn->infinity() && !(vn->flag() & SV_WALK))
			cat_vtxs(vn, m);
	}
}

// Generate points along an edge

void tet_edge(const sv_model& m, const sv_set& s, sv_real dist, 
	const sv_set& mem, sv_primitive ks[])
{
    sv_box b = m.box();
    sv_point p0 = edge(s, b.centroid(), 0.001);
    sv_point q, p;
    sv_primitive p1 = s.child_1().primitive();
    sv_primitive p2 = s.child_2().primitive();
    sv_point dir = p1.grad(p0)^p2.grad(p0);
    if(dir.mod() < get_accuracy()) return;
    sv_set win;
    sv_real sv;
    sv_integer add, loop, dirs;
    sv_delaunay* d;
    
    loop = 1;
    dirs = 1;
    p = p0;
    while(loop)
    {
    	if(loop = (b.member(p) == SV_SOLID)) // I mean = not ==
    	{
	    sv = s.value(p, &win);
	    if(fabs(sv) < 0.05)
	    {
		    q = tet_mod.nearest(p)->point() - p;
		    if(q*q > dist*dist*0.25)
		    {
			    add = 1;
			    if(mem.exists())
				add = (mem.member(p, ks) == SV_SURFACE);
			    if(add)
			    {
				d = tet_mod.add_point(p);
				d->set_flag(SV_ON_EDGE);
				d->set(s);
				d->model(m);
			    }
		    }
	    }
    	} else
	{
		dirs++;
		if(dirs == 2)
		{
			loop = 1;
			p = p0;
			dir = -dir;
		}
	}
	p = edge_step(s, p, dir, dist*(0.75+0.5*ran_real()), 0.001);
    }
}

// Generate points in a face

void tet_face(const sv_model& m, const sv_set& s, sv_real dist, 
	const sv_set& mem, sv_primitive ks[])
{
    sv_box b = m.box();
    sv_primitive pr = s.primitive();
    sv_point p0 = b.centroid();
    sv_point nrm = pr.grad(p0);
    sv_point x, y, z, p, q;
    sv_axes(nrm, x, y, z);
    sv_real size = 2.0*sqrt(b.diag_sq()) + ran_real();
    sv_real xc, yc, sv;
    sv_set win;
    sv_integer add;
    sv_integer total = (sv_integer)(2*size/dist);
    if(total < 1) total = 1;
    sv_integer loop = 20*total;
    sv_delaunay* d;

    while(loop && total)
    {
	xc = 2*size*ran_real() - size;
	yc = 2*size*ran_real() - size;
	p = p0 + x*xc + y*yc;
	p = newton(pr, p, 0.001);
	loop--;
	if(b.member(p) == SV_SOLID)
	{
	    sv = s.value(p, &win);
	    if(fabs(sv) < 0.05)
	    {
		q = tet_mod.nearest(p)->point() - p;
		if(q*q > dist*dist*0.1)
		{
			total--;
			add = 1;
			if(mem.exists())
			    add = (mem.member(p, ks) == SV_SURFACE);
			if(add)
			{
				d = tet_mod.add_point(p);
				d->set_flag(SV_ON_FACE);
				d->set(s);
				d->model(m);
			}
		}
	    }
	} 
    }
}

// Generate points in a volume

void tet_volume(const sv_model& m, sv_real dist, const sv_set& mem)
{
	sv_box b = m.box();
	sv_point p, q;
	sv_integer total = (sv_integer)(b.vol()/(dist*dist*dist));
	if (total < 1) total = 1;
	sv_integer add;
	sv_primitive ks;
	sv_integer loop = 30*total;
	sv_delaunay* d;

    	while(total && loop)
	{
	    loop--;
	    p = ran_point(b);
	    q = tet_mod.nearest(p)->point() - p;
	    if(q*q > dist*dist*0.1)
	    {
		    add = 1;
		    total--;
		    if(mem.exists())
			add = (mem.member(p, &ks) == SV_SOLID);
		    if(add) 
		    {
			d = tet_mod.add_point(p);
			d->set_flag(SV_IN_SOLID);
			d->model(m);
		    }
	    }   
	} 

}


// Add the model's corners

void tet_mesh_r3(const sv_model& m, sv_real dist)
{
	sv_set s, win, se0, se1, se2;
	sv_point p, q;
	sv_primitive ks[3], dum;
	sv_real sv;
	sv_delaunay* d;

	if(m.kind() == LEAF_M)
	{
		s = m.set_list().set();
		if(s.contents() != 3) return;

		p = corner(s, m.box().centroid(), 0.001);

		if(m.box().member(p) == SV_SOLID)
		{
			sv = s.value(p, &win);
			if(fabs(sv) < 0.05)
			{
				q = tet_mod.nearest(p)->point() - p;
				if(q*q > dist*dist*0.02)
				{
					d = tet_mod.add_point(p);
					d->set_flag(SV_ON_CORNER);
					d->set(s);
					d->model(m);
				}
			}
		}

		se0 = s.child_2();
		if(se0.contents() != 2)
			svlis_error("tet_mesh_r3", "dud corner edge", SV_CORRUPT);
		ks[0] = se0.child_1().primitive();
		ks[1] = se0.child_2().primitive();
		ks[2] = dum;
		tet_edge(m, se0, dist, s, ks);
		if(s.op() == SV_UNION)
		{
			se1 = s.child_1() | se0.child_1();
			ks[0] = se1.child_1().primitive();
			ks[1] = se1.child_2().primitive();
			tet_edge(m, se1, dist, s, ks);

			se2 = s.child_1() | se0.child_2();
			ks[0] = se2.child_1().primitive();
			ks[1] = se2.child_2().primitive();
			tet_edge(m, se2, dist, s, ks);
		} else
		{
			se1 = s.child_1() & se0.child_1();
			ks[0] = se1.child_1().primitive();
			ks[1] = se1.child_2().primitive();
			tet_edge(m, se1, dist, s, ks);

			se2 = s.child_1() & se0.child_2();
			ks[0] = se2.child_1().primitive();
			ks[1] = se2.child_2().primitive();
			tet_edge(m, se2, dist, s, ks);
		}

		ks[0] = s.child_1().primitive();
		ks[1] = dum;
		tet_face(m, s.child_1(), dist, s, ks);

		ks[0] = s.child_2().child_1().primitive();
		tet_face(m, s.child_2().child_1(), dist, s, ks);

		ks[0] = s.child_2().child_2().primitive();
		tet_face(m, s.child_2().child_2(), dist, s, ks);

		tet_volume(m, dist, s);

	} else
	{
		tet_mesh_r3(m.child_1(), dist);
		tet_mesh_r3(m.child_2(), dist);
	}
}

// Add points along edges

void tet_mesh_r2(const sv_model& m, sv_real dist)
{
	sv_set s, sw;
	sv_primitive pr;
	sv_primitive ks[2], dum;
	sv_point p, q;

	if(m.kind() == LEAF_M)
	{
		s = m.set_list().set();
		if(s.contents() != 2) return;
		tet_edge(m, s, dist, sw, &pr);
		ks[0] = s.child_1().primitive();
		ks[1] = dum;
		tet_face(m, s.child_1(), dist, s, ks);
		ks[0] = s.child_2().primitive();
		tet_face(m, s.child_2(), dist, s, ks);

		tet_volume(m, dist, s);
	} else
	{
		tet_mesh_r2(m.child_1(), dist);
		tet_mesh_r2(m.child_2(), dist);
	}
}

// Add points in faces

void tet_mesh_r1(const sv_model& m, sv_real dist)
{
	sv_set s, mem;
	sv_primitive ks;

	if(m.kind() == LEAF_M)
	{
		s = m.set_list().set();
		if(s.contents() != 1) return;
		tet_face(m, s, dist, mem, &ks);
		tet_volume(m, dist, s);
	} else
	{
		tet_mesh_r1(m.child_1(), dist);
		tet_mesh_r1(m.child_2(), dist);
	}
}

// Add points in solid

void tet_mesh_r0(const sv_model& m, sv_real dist)
{
	sv_set s, dum;
	sv_point p, q;

	if(m.kind() == LEAF_M)
	{
		s = m.set_list().set();
		if(s.contents() != SV_EVERYTHING) return;
		tet_volume(m, dist, dum);	
	} else
	{
		tet_mesh_r0(m.child_1(), dist);
		tet_mesh_r0(m.child_2(), dist);
	}
}

sv_voronoi tet_mesh(const sv_model& m, sv_real dist)
{
	tet_mod = sv_voronoi(m.box());
	tet_mesh_r3(m, dist);
	tet_mesh_r2(m, dist);
	tet_mesh_r1(m, dist);
	tet_mesh_r0(m, dist);
	cat_vtxs(tet_mod.walk_start(), m);
	reset_flags(tet_mod.walk_start(), SV_WALK);
	return(tet_mod);
}

// Print details for a vertex

void vprint(ostream& s, sv_vertex* v)
{
	int i;
	sv_vertex* n;

	s << "\n   Vertex: " << v << '\n';
	s << "   Flags: " << v->flag() << '\n';
	v->set_flag(SV_WALK);
	s << "   Radius: " << sqrt(v->r_squared()) << '\n';
	s << "   Position: " << v->position() << '\n';
	s << "   Points:\n";
	for(i = 0; i <= SV_VD; i++) 
		s << "      " << v->delaunay(i)->point() << '\n';
	s << "   Neighbours: ";
	for(i = 0; i <= SV_VD; i++)
	{
		n = v->neighbour(i);
		s << n;
		if(n->infinity()) s << '*';
		s << ' ';
	}
	s << '\n';
	for(i = 0; i <= SV_VD; i++)
	{
		n = v->neighbour(i);
		if(!n->infinity() && !(n->flag() & SV_WALK))
			vprint(s, n);		
	}
}

void sv_voronoi::write(ostream& s)
{
	s << "\nVertices at infinity:\n   ";
	for(int i = 0; i <= SV_VD; i++) s << at_inf[i] << ' ';
	s << "\nWalk start:\n   " << walk_start() << '\n';
	s << "Point count:\n   " << point_count() << '\n';	
	s << "Enclosing box:\n   " << box() << "\n\n";
	s << "\nVertex structure:\n";
	vprint(s, walk_start());
	s << '\n';
	reset_flags(walk_start(), SV_WALK);
	s.flush();
}
//****************************************************************************************

#if macintosh
 #pragma export off
#endif





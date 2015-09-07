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
 * SvLis - generaly useful things (some user-supplied) 
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 5 March 1993
 * This version: 23 September 2000  (JG)
 *
 */


#include "svlis.h"
#if macintosh
 #pragma export on
#endif

// Monte Carlo volume function

// The model must have been divided and its set-list must contain only one set.
// For accurate results the swell factor should have been set to 0, and there
// should be a fairly small minimum box size for surface boxes.

// accy is how accurate you want the result; so 0.01 would give a result
// to within 1% of the true value, for example.  If you set a value of 0,
// you are being a bit optimistic...


// Accumulators for volume, centroid and MoI calculations

static sv_real svx, svy, svz, svx2, svy2, svz2, svs, sva, tsv, svxy, svxz, svyz;
static sv_integer n_ran_p;
static sv_real n_to_use = -1;
static sv_integer const_work = 0;

// Flag for whether to use niederreiter or uniform

static int niederreiter = 0;

void sv_niederreiter_ran(int i) { niederreiter = i; }

sv_point ran_point_n(const sv_box& b);
sv_point ran_point_u(const sv_box& b);

sv_point ran_point(const sv_box& b)
{
	n_ran_p++;
	if (niederreiter)
		return(ran_point_n(b));
	else
		return(ran_point_u(b));
}

sv_integer integral_points() { return n_ran_p; }
void integral_points(sv_real np) {n_to_use = np; }

sv_integer constant_work() { return const_work; }
void constant_work(sv_integer i) { const_work = i; }

// Use the accumulated values to compute the values for the model

void compute_averages(sv_real& vol, sv_point& centroid, sv_point&
	mxyz, sv_point& nxyz)
{
	vol = svs;
        sv_point sq_off = sv_point(centroid.y*centroid.y + centroid.z*centroid.z,
			centroid.x*centroid.x + centroid.z*centroid.z,
			centroid.x*centroid.x + centroid.y*centroid.y);
	if(svs > 0.0)
	{
		centroid = sv_point(svx/svs, svy/svs, svz/svs);
		mxyz = sv_point(svx2/svs, svy2/svs, svz2/svs) - sq_off;
		
//JG 24/3/00
               nxyz = sv_point(svyz/svs, svxz/svs, svxy/svs) - sq_off;
	} else
	{
		centroid = SV_OO;
		mxyz = SV_OO;
	}
}

// volume_sa - this just walks the model tree summing boxes that are
// totally solid or totally air.  Assumes that the accumulators
// have been zeroed.

void volume_sa(const sv_model& m)
{
	sv_real x,y,z,v;
	sv_point p;

	switch(m.kind())
	{
	case LEAF_M:
		switch(m.set_list().set().contents())
		{
		case SV_EVERYTHING:
			v = m.box().vol();
			p = m.box().centroid();
			x = p.x;
			y = p.y;
			z = p.z;
			svx = svx + v*x;
			svy = svy + v*y;
			svz = svz + v*z;
// JG 24/3/00
			svyz = svyz + (v*y * v*z);
			svxy = svxy + (v*x * v*z);
			svxz = svxz + (v*x * v*y);						

			svx2 = svx2 + v*(y*y + z*z);
			svy2 = svy2 + v*(x*x + z*z);
			svz2 = svz2 + v*(x*x + y*y);
			x = m.box().xi.hi() - m.box().xi.lo();
			y = m.box().yi.hi() - m.box().yi.lo();
			z = m.box().zi.hi() - m.box().zi.lo();
			svx2 = svx2 + v*(y*y + z*z)/12.0;
			svy2 = svy2 + v*(x*x + z*z)/12.0;
			svz2 = svz2 + v*(x*x + y*y)/12.0;
			svs = svs + v;
			break;

		case SV_NOTHING: 
			sva = sva + m.box().vol(); break;

		default:
			tsv = tsv + m.box().vol(); break;
			break;
		}
		break;
	
	case X_DIV:
	case Y_DIV:
	case Z_DIV:
		volume_sa(m.child_1());
		volume_sa(m.child_2());
		break;
	
	default:
		svlis_error("volume_sa", "dud model kind", SV_CORRUPT);
	}
}



// Monte Carlo volume procedure
// Assumes the accumulators contain what was calculated
// by volume_sa.

void volume_monte(const sv_model& m, const sv_real accy)
{
	sv_integer n, n_needed, n_done, s, i;
	sv_point p;
	sv_real vol;
	sv_primitive ks;
	sv_real sx, sy, sz, sx2, sy2, sz2, v, svxy, svxz, svy;
	sv_set ss = m.set_list().set();

	sx = 0;
	sy = 0;
	sz = 0;
	sx2 = 0;
	sy2 = 0;
	sz2 = 0;
	svxy = 0;	
	svxz = 0;
	svyz = 0;

	switch(m.kind())
	{
	case LEAF_M:
		if(ss.contents() >= 1)
		{

// See - Stephen Parry-Barwick: Multidimensional set-theoretic geometric modelling
// PhD thesis, University of Bath 1995, pp 163-167

			s = 0;
			n_done = 0;
            		v = m.box().vol();
			if (n_to_use > 0)
				n_needed = round(v*n_to_use);
			else
				n_needed = N_MONTE;
			do
			{
				n = n_needed - n_done;
				for(i = 0; i < n; i++)
				{
					p = ran_point(m.box());
					if(ss.member(p,&ks) != SV_AIR) 
					{
						s++;
						sx = sx + p.x;
						sy = sy + p.y;
						sz = sz + p.z;
						sx2 = sx2 + p.y*p.y + p.z*p.z;
						sy2 = sy2 + p.x*p.x + p.z*p.z;
						sz2 = sz2 + p.y*p.y + p.x*p.x;

// JG 24/3/00					svxy = svxy + (p.x*p.y);
					        svxz = svxz + (p.x*p.z);
						svyz = svyz + (p.y*p.z);
					}
				}
				n_done = n_done + n;
				if(n_to_use < 0)
				{
				   vol = ((sv_real)s)/((sv_real)n_done);
				   if (vol > 0.5)
					n_needed = round((1 - vol)/(vol*accy*accy));
				   else
					n_needed = round(vol/((1 - vol)*accy*accy));
				}
			} while (n_needed > n_done);
			v = v/((sv_real)n_done);
			svx2 = svx2 + sx2*v;
			svy2 = svy2 + sy2*v;
			svz2 = svz2 + sz2*v;
// JG 24/3/00
                        svxy = svxy + svxy*v;
			svxz = svxz + svxz*v;
			svyz = svyz + svyz*v;
						
			svx = svx + sx*v;
			svy = svy + sy*v;
			svz = svz + sz*v;
			svs = svs + v*(sv_real)s;
	    }
	    break;
	
	case X_DIV:
	case Y_DIV:
	case Z_DIV:
		volume_monte(m.child_1(), accy);
		volume_monte(m.child_2(), accy);
		break;
	
	default:
		svlis_error("volume_monte", "dud model kind", SV_CORRUPT);
	}
}

// This is the function that the user calls

void integral(const sv_model& m, sv_real accy, sv_real& vol,
	sv_point& centroid, sv_point& mxyz, sv_point& nxyz)
{
	sv_real v_box = m.box().vol();
	sv_real v_unknown;

	svx = 0;
	svy = 0;
	svz = 0;
	
	svxy = 0;	
	svxz = 0;
	svyz = 0;

	svx2 = 0;
	svy2 = 0;
	svz2 = 0;
	svs = 0;
	sva = 0;
        n_ran_p = 0;
        tsv = 0;

	volume_sa(m);

	sv_real remember = n_to_use;

	if(const_work) n_to_use = (float)const_work/tsv;

// Check if division was fine enough to give an answer already:

	v_unknown = v_box - svs - sva;
	if (svs > 0)
	{
		if ( v_unknown/svs <= accy)
		{
			compute_averages(vol, centroid, mxyz, nxyz);
			n_to_use = remember;
			return;
		}
	} else
	{
		if(sva > 0)
		   if ( v_unknown/sva <= accy)
		   {
			compute_averages(vol, centroid, mxyz, nxyz);
			n_to_use = remember;
			return;
		   }
	}

// Now is the point to check if the user's been silly		
	
	if(accy <= 0)
	{
		svlis_error("integral", 
		  "integral properties cannot be computed perfectly accurately", 
			SV_WARNING);
		accy = 0.01;
	}	

// Now we know some of the answer, the remainder can be found to a coarser
// accuracy (assume half v_unknown is solid):
	
	volume_monte(m, accy*(2*svs/v_unknown + 1));
	compute_averages(vol, centroid, mxyz, nxyz);
	n_to_use = remember;	
}

// Recursively add-up the polygon areas

void area_sum(const sv_set& s, sv_real* a_tot, const sv_set& interest)
{
	sv_attribute a;
	sv_user_attribute* u;
	sv_p_gon* pg;

	if (s.contents() > 1)
	{
		area_sum(s.child_1(), a_tot, interest);
		area_sum(s.child_2(), a_tot, interest);
	}

// Note we need to compare primitives to avoid confusion
// with attributes

	if(interest.exists())
	{
		if(s.contents() == 1)
		  if(s.primitive() != interest.primitive()) return;
	}

	a = s.attribute();
	while(a.exists())
	{
		if( a.tag_val() == -pg->tag() )
		{
              		u = a.user_attribute();
                	pg = (sv_p_gon*)u->pointer;
			*a_tot = *a_tot + p_gon_area(pg);
		}
		a = a.next();
	}
}

void area_sum(const sv_model& m, sv_real* a_tot, const sv_set& interest)
{
    sv_set_list pgl, next;
    sv_set s;
    
	if (m.has_polygons())
	{
		pgl = m.set_list();
		while(pgl.exists())
		{
			area_sum(pgl.set(), a_tot, interest);
			pgl = pgl.next();	
		}
	}
	switch(m.kind())
	{
	case LEAF_M:
		return;
	
	case X_DIV:
	case Y_DIV:
	case Z_DIV:
		area_sum(m.child_1(), a_tot, interest);
		area_sum(m.child_2(), a_tot, interest);
		break;
	
	default:
		svlis_error("area_sum", "dud model kind", SV_CORRUPT);
	}
}

// Area function
// The model must have been faceted with a swell of 0

// Note that there is a danger that a plane which exactly
// coincides with a division plane will have its area
// counted twice - make the root model box with non-simple-
// integer corner coordinates.

sv_real area(const sv_model& m)
{
	sv_real a_tot = 0;
	sv_set undefined;
	area_sum(m, &a_tot, undefined);
        return(a_tot);
	// return(a_tot/(1 + 2*get_swell_fac()));  // Accuracy hack
}

// Area function for one set's contribution
// The model must have been faceted with a swell of 0

sv_real area(const sv_model& m, const sv_set& s)
{
	sv_real a_tot = 0;
	area_sum(m, &a_tot, s);
        return(a_tot);
	// return(a_tot/(1 + 2*get_swell_fac()));  // Accuracy hack
}

// Minimum and maximum squared distance between two boxes

sv_interval dist_2(const sv_box& a, const sv_box& b)
{

// Check for overlaps

	sv_integer overlap = 0;
	sv_box tmp = a & b;
	if (!tmp.xi.empty()) overlap = overlap | 1;
	if (!tmp.yi.empty()) overlap = overlap | 2;
	if (!tmp.zi.empty()) overlap = overlap | 4;

// Compute max and min corner-to-corner squared distances

	tmp = pow(a-b,2);
	sv_interval result = tmp.xi + tmp.yi + tmp.zi;

	switch(overlap)
	{
	case 7:
	case 0: // No overlap at all or all 3 overlap
		return(result);

	case 1: // Overlap in x only
		result = sv_interval(tmp.yi.lo() + tmp.zi.lo(), result.hi());
		return(result);

	case 2: // Overlap in y only
		result = sv_interval(tmp.xi.lo() + tmp.zi.lo(), result.hi());
		return(result);

	case 3: // Overlap in x and y
		result = sv_interval(tmp.zi.lo(), result.hi());
		return(result);

	case 4: // Overlap in z only
		result = sv_interval(tmp.xi.lo() + tmp.yi.lo(), result.hi());
		return(result);

	case 5: // Overlap in x and z
		result = sv_interval(tmp.yi.lo(), result.hi());
		return(result);

	case 6: // Overlap in y and z
		result = sv_interval(tmp.xi.lo(), result.hi());
		return(result);

	default:
		svlis_error("dist_2", "the | op seems not to work", SV_CORRUPT);
	}
	return(result); // Should never get here
}


// Tight fit function to find the smallest enclosing box round the
// sets in a model.  This uses the faceting procedures to expand a box
// out to acommodate the furthest vertices - it therefore only returns
// a close approximation to the answer.

static sv_box tfb;

void update_tfb(const sv_set& s)
{
	sv_p_gon* pg;
	sv_p_gon* n;
	sv_attribute a;

	if(s.contents() > 1)
	{
		update_tfb(s.child_1());
		update_tfb(s.child_2());
	} 
		
	a = s.attribute();
	while(a.exists())
	{
		if(a.tag_val() == -pg->tag())
		{
			pg = (sv_p_gon*)(a.user_attribute()->pointer);
			if(pg)
			{
				n = pg;
				do
				{
					if(n->p.x > tfb.xi.hi()) tfb.xi = sv_interval(tfb.xi.lo(), n->p.x);
					if(n->p.x < tfb.xi.lo()) tfb.xi = sv_interval(n->p.x, tfb.xi.hi());
					if(n->p.y > tfb.yi.hi()) tfb.yi = sv_interval(tfb.yi.lo(), n->p.y);
					if(n->p.y < tfb.yi.lo()) tfb.yi = sv_interval(n->p.y, tfb.yi.hi());
					if(n->p.z > tfb.zi.hi()) tfb.zi = sv_interval(tfb.zi.lo(), n->p.z);
					if(n->p.z < tfb.zi.lo()) tfb.zi = sv_interval(n->p.z, tfb.zi.hi());
					n = n->next;
				} while(n != pg);
			}
		}
		a = a.next();
	}
}

void tight_decision(const sv_model& m, sv_integer level, void* vp, mod_kind* k, sv_real* c, 
		sv_model* c_1, sv_model* c_2)
{

// Don't bother if we're inside what we've found already

	sv_box b = m.box();
	if (b.inside(tfb))
	{
		*k = LEAF_M;
		return;
	}

// Use facet decision to generate polygons (maybe) in this model box

	facet_decision(m, level, 0, k, c, c_1, c_2);

// If facet_decision put polygons in here, expand the tight-fit box to
// include their vertices.

	sv_set_list pgl = m.set_list();

	while(pgl.exists())
	{
		update_tfb(pgl.set());
		pgl = pgl.next();
	}
}

sv_box sv_model::tight_fit() const
{

// Make a new model to force division

	sv_box b = box();
	sv_set_list sl = set_list();
	sv_model mtf = sv_model(sl,b,sv_model());

// Set the box empty

	tfb.xi = sv_interval(b.xi.hi(), b.xi.lo());
	tfb.yi = sv_interval(b.yi.hi(), b.yi.lo());
	tfb.zi = sv_interval(b.zi.hi(), b.zi.lo());

// tight_decision updates box tfb

	sv_model junk = mtf.redivide(sl, 0, &tight_decision);

	return(tfb);
}


// Characteristic point function for sets

// This computes the point which has a minimum squared
// distance to all the planes that make the primitives
// in a set.

// Accumulators

static sv_real sa2, sab, sac, sb2, sbc, sc2, sad, sbd, scd;

void prim_2_p(const sv_primitive& p)
{
	sv_real a,b,c,d;

	switch(p.kind())
	{
	case SV_REAL:
		break;
	case SV_PLANE:
		a = p.plane().normal.x;
		b = p.plane().normal.y;
		c = p.plane().normal.z;
		d = p.plane().d;
		sa2 = sa2 + a*a;
		sab = sab + a*b;
		sac = sac + a*c;
		sb2 = sb2 + b*b;
		sbc = sbc + b*c;
		sc2 = sc2 + c*c;
		sad = sad + a*d;
		sbd = sbd + b*d;
		scd = scd + c*d;
		break;
	default:
		if(diadic(p.op())) prim_2_p(p.child_2());
		prim_2_p(p.child_1());
		break;
	}
}

void set_2_p(const sv_set& s)
{
	switch(s.contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		break;
	case 1:
		prim_2_p(s.primitive());
		break;
	default:
		set_2_p(s.child_1());
		set_2_p(s.child_2());
		break;	
	}
}

void zero_c_point()
{

	sa2 = 0;
	sab = 0;
	sac = 0;
	sb2 = 0;
	sbc = 0;
	sc2 = 0;
	sad = 0;
	sbd = 0;
	scd = 0;
}

sv_point compute_cp()
{

// Easiest way to solve the linear system is to treat
// it as an intersecting planes problem

	sv_plane f1 = sv_plane(sa2, sab, sac, sad);
	sv_plane f2 = sv_plane(sab, sb2, sbc, sbd);
	sv_plane f3 = sv_plane(sac, sbc, sc2, scd);
	sv_point result = planes_point(f1,f2,f3);
	return(result);
}

sv_point sv_primitive::point() const
{
	zero_c_point();
	prim_2_p(*this);
	return(compute_cp());
}

sv_point sv_set::point() const
{
	zero_c_point();
	set_2_p(*this);
	return(compute_cp());
}

sv_point sv_set_list::point() const
{
	sv_set_list sl = *this;
	zero_c_point();
	while(sl.exists())
	{
		set_2_p(sl.set());
		sl = sl.next();
	}
	return(compute_cp());
}

sv_point sv_model::point() const
{
	return(this->set_list().point());
}

// Do a Newton-Raphson on a point and a primitive to find a point 
// on the primitive's surface.  accy is the value of potential
// below which it's assumed to be 0.

#define MAXIT 25

sv_point newton(const sv_primitive& a, sv_point p, sv_real accy)
{
	sv_real v = 2.0*accy;
	sv_real nasty = accy*accy;
	sv_real gv;
	sv_point g;
	int count = 0;

	while( (fabs(v) > accy) && (count <= MAXIT) )
	{
		g = a.grad(p);
		v = a.value(p);
		gv = g*g;
		if (gv >= nasty) p = p - g*(v/gv);
		count++;
	}
		
	if(count > MAXIT)
	{
		set_svlis_flag(NO_CONVERGE);
		// svlis_error("newton", "no convergence", SV_WARNING);
	} else
		set_svlis_flag(NO_FLAG);

	return(p);
}

sv_point newton(const sv_set& a, sv_point p, sv_real accy)
{
	sv_real v = 2.0*accy;
	sv_real nasty = accy*accy;
	sv_real gv;
	sv_point g;
	int count = 0;

	while( (fabs(v) > accy) && (count <= MAXIT) )
	{
		g = a.grad(p, v);
		gv = g*g;
		if (gv >= nasty) p = p - g*(v/gv);
		count++;
	}
		
	if(count > MAXIT)
	{
		set_svlis_flag(NO_CONVERGE);
		// svlis_error("newton", "no convergence", SV_WARNING);
	} else
		set_svlis_flag(NO_FLAG);

	return(p);
}

// Use Newton-Raphson to find a corner for a set containing
// two operations on three primitives.  The start point for
// the search is p, which should be reasonably near the
// answer (the centriod of a small leaf model box containing the
// corner is a good start).  Don't be too optimistic with accy.

sv_point corner(const sv_set& s, sv_point p, sv_real accy)
{
	if(s.contents() != 3)
	{
		svlis_error("corner", "set hasn't a contents of 3", 
			SV_WARNING);
		return(p);
	}

// We know that child_1() is the single primitive

	sv_primitive a = s.child_1().primitive();
	sv_real av = a.value(p);
	if(av == 0) av = 1;
	sv_primitive b = s.child_2().child_1().primitive();
	sv_real bv = b.value(p);
	if(bv == 0) bv = 1;
	sv_primitive c = s.child_2().child_2().primitive();
	sv_real cv = c.value(p);
	if(cv == 0) cv = 1;

// Sum the squares of the primitives rescaled by their potential
// values to keep numerical problems down.

	sv_primitive z = (a^2)/sv_primitive(av*av) + 
		(b^2)/sv_primitive(bv*bv) + 
		(c^2)/sv_primitive(cv*cv);

// Point where z = 0 must be the corner

	return(newton(z, p, accy));
}

// Use Newton-Raphson to find a point on an edge for a set containing
// two primitives.

sv_point edge(const sv_set& s, sv_point p, sv_real accy)
{
	if(s.contents() != 2)
	{
		svlis_error("edge", "set hasn't a contents of 2", 
			SV_WARNING);
		return(p);
	}
	sv_primitive a = s.child_1().primitive();
	sv_real av = a.value(p);
	if(av == 0) av = 1;
	sv_primitive b = s.child_2().primitive();
	sv_real bv = b.value(p);
	if(bv == 0) bv = 1;

// Sum the squares of the primitives rescaled by their potential
// values to keep numerical problems down.

	sv_primitive z = (a^2)/sv_primitive(av*av) + 
		(b^2)/sv_primitive(bv*bv);

// Any point where z = 0 must be on the edge

	return(newton(z, p, accy));
}

// Move a distance (roughly) r along an edge in (roughly) direction d

sv_point edge_step(const sv_set& s, sv_point p, const sv_point& d, sv_real r, sv_real accy)
{
	if(s.contents() != 2)
	{
		svlis_error("edge_step", "set hasn't a contents of 2", 
			SV_WARNING);
		return(p);
	}
	sv_primitive a = s.child_1().primitive();
	sv_point ag = a.grad(p);
	sv_primitive b = s.child_2().primitive();
	sv_point bg = b.grad(p);
	sv_point dd = ag^bg;
	if(dd*d < 0) dd = -dd;
	p = p + dd.norm()*r;
	p = edge(s,p,accy);
	return(p);
}

sv_point newton_line(const sv_primitive& a, sv_point p0, const sv_point& p1, sv_real accy)
{
	sv_real v = 2.0*accy;
	sv_real nasty = accy*accy;
	sv_real gv;
	sv_point g;
	int count = 0;
	sv_point dir = (p1 - p0).norm();

	while( (fabs(v) > accy) && (count <= MAXIT) )
	{
		g = a.grad(p0);
		g = dir*(g*dir);
		v = a.value(p0);
		gv = g*g;
		if (gv >= nasty) p0 = p0 - g*(v/gv);
		count++;
	}
		
	if(count > MAXIT)
	{
		set_svlis_flag(NO_CONVERGE);
		// svlis_error("newton", "no convergence", SV_WARNING);
	} else
		set_svlis_flag(NO_FLAG);

	return(p0);
}


// Do a binary chop along a line segment to find a root

sv_point binary_chop(const sv_primitive& p, sv_point p0, sv_real v0,
		sv_point p1,  sv_real v1, sv_real accy)
{
	if(v0*v1 > 0)
		svlis_error("binary_chop","ends have the same sign",SV_WARNING);

// Treat planes as a special case

	if(p.kind() == SV_PLANE)
		return(p0 + (p1 - p0)*fabs(v0)/(fabs(v1)+fabs(v0)));

	sv_point pd = p1 - p0;
	sv_real l2 = pd*pd;
	sv_point m = 0.5*(p0 + p1);
	sv_real vm;

	while(l2 > accy)
	{
		vm = p.value(m);
		if(vm*v0 < 0)
		{
			p1 = m;
			v1 = vm;
		} else
		{
			p0 = m;
			v0 = vm;
		}
		m = 0.5*(p0 + p1);
		l2 = 0.25*l2;  // Squared length so 0.25 not 0.5
	}

	return(p0 + (p1 - p0)*fabs(v0)/(fabs(v1)+fabs(v0)));
}

//*********************************************************************

// By Dan Pidcock

// returns a face of a box as planes

sv_plane box_face(const sv_box& b, sv_integer i) 
{
    // i is 0-5 and is:
    // left, bottom, front, right, top, back

    sv_point org, n;

    if (i < 3) // plane through x_lo, y_lo, z_lo
	org = sv_point(b.xi.lo(), b.yi.lo(), b.zi.lo());
    else // plane through x_hi, y_hi, z_hi
	org = sv_point(b.xi.hi(), b.yi.hi(), b.zi.hi());

    // normal depends on face

    switch(i) 
    {
    case 0:
	n = -SV_X;
	break;
    case 1:
	n = -SV_Y;
	break;
    case 2:
	n = -SV_Z;
	break;
    case 3:
	n = SV_X;
	break;
    case 4:
	n = SV_Y;
	break;
    case 5:
	n = SV_Z;
	break;
    default:
	svlis_error("box_face",
		"Invalid index to box_face", SV_WARNING);
	n = SV_X;
	break;
    }; // end switch
    
    return(sv_plane(n, org));
}

// makes a wireframe box from a svlis box

sv_set wireframe_box(const sv_box& b) 
{
    sv_integer i;
    sv_plane f;
    sv_set sf[6];
    sv_set saf[6];
    sv_set lines_set;

    for (i = 0; i < 6; i++) 
    {
	f = box_face(b,i);
	sf[i] = sv_set(sv_primitive(f));
	saf[i] = sv_set(abs(sv_primitive(f)));
    }

    // make edges by intersecting plane-pairs in fours and clipping with the end planes

    lines_set = ( 
		 ( 
		   (saf[1] & saf[2]) | (saf[2] & saf[4]) | (saf[4] & saf[5]) | (saf[5] & saf[1]) 
		 )
		 &
		 (
		   sf[0] & sf[3]
		 )
		)
		|
		( 
		 ( 
		   (saf[0] & saf[2]) | (saf[2] & saf[3]) | (saf[3] & saf[5]) | (saf[5] & saf[0]) 
		 )
		 &
		 (
		   sf[1] & sf[4]
		 )
		)
		|
		( 
		 ( 
		   (saf[0] & saf[1]) | (saf[1] & saf[3]) | (saf[3] & saf[4]) | (saf[4] & saf[0]) 
		 )
		 &
		 (
		   sf[2] & sf[5]
		 )
		);

    return (lines_set);
}

// Returns the `crinkliness' of a FACETED model, defined as the surface 
// area of the model divided by the surface area of a sphere having
// the same volume as the model.

sv_real crinkliness(const sv_model& mod) 
{
    sv_real s_area = area(mod);
    sv_real vol;
    sv_point pt1, pt2, pt3;
    integral(mod, 0.01, vol, pt1, pt2, pt3);
    // The expression in the return is equivalent to finding:
    //   radius = powf(3.0*vol/(4.0*M_PI), (1.0/3.0));
    //   s_area_sph = 4.0 * M_PI * radius * radius;
    // and returning s_area / s_area_sph.
    return(s_area / (pow((double)(M_PI * 36.0 * vol * vol), (double)(1.0/3.0))));
}


//*********************************************************************

// Utilities by Kevin Wise and David Eisenthal


// Slice a dimension out of a svLis box

sv_box slice(const sv_box& b, const mod_kind d, const sv_real r)
{
    sv_box result;
    sv_interval thin(r, r);
    switch (d)
    {
	case X_DIV:
	    result = sv_box(thin, b.yi, b.zi);
	    break;
	
	case Y_DIV:
	    result = sv_box(b.xi, thin, b.zi);
	    break;

	case Z_DIV:
	    result = sv_box(b.xi, b.yi, thin);
	    break;
	
	default:
	    svlis_error(
		"slice(box,...)", 
		"Duff d value - leaving box as it was.", 
		SV_WARNING);
	    result = b;
	    break;
    }
    
    return (result);
}

sv_primitive slice(const sv_plane& p, const mod_kind d, const sv_real r)
{
    sv_primitive result;
    sv_point res_norm;
    sv_real  res_d;

// The normal has the dim value cut out of it.

    switch (d)
    {
	case X_DIV:
	    res_norm = sv_point(0, p.normal.y, p.normal.z);
	    res_d = p.normal.x*r;
	    break;

	case Y_DIV:
	    res_norm = sv_point(p.normal.x, 0, p.normal.z);
	    res_d = p.normal.y*r;
	    break;

	case Z_DIV:
	    res_norm = sv_point(p.normal.x, p.normal.y, 0);
	    res_d = p.normal.z*r;
	    break;
	
	default:
	    svlis_error(
		"slice(sv_plane,...)", 
		"Duff d value - leaving plane as it was.", 
		SV_CORRUPT);
	    return(sv_primitive(p));    
    }

// The distance from origin is increased by the product of
// the disappearing normal coeffecient and the slice value.

    res_d = p.d + res_d;
	    
// If the plane's normal was purely in the sliced dimension,
// then the plane turns into a real.

    if (res_norm.mod() < sv_same_tol)
	result = sv_primitive(res_d);
    else
	result = sv_primitive(sv_plane(res_norm, res_d));
    
    return(result);
}

// Deal with special cases that (sometimes) generate cylinders.

sv_primitive special_slice(const sv_primitive& p, const mod_kind d, const sv_real r)
{
	sv_primitive result;
        sv_integer k;
        sv_real r0, r1, r2;
        sv_plane f; 
        sv_point cen;
        sv_line axis;
	sv_point sl_d;

	switch (d)
	{
	case X_DIV: sl_d = SV_X; break;
	case Y_DIV: sl_d = SV_Y; break;
	case Z_DIV: sl_d = SV_Z; break;
	default:
		    svlis_error(
		"special_slice", 
		"duff d value", 
		SV_CORRUPT);
	}

	prim_op po = p.parameters(&k, &r0, &r1, &r2, &f, &cen, &axis);
	switch (p.kind())
	{
	case SV_CYLINDER:
		if(same(axis.direction,sl_d)) return(p);
		return(result);

	case SV_CONE:
		if(!same(axis.direction,sl_d)) return(result);
		r2 = sl_d*axis.origin - r;
		r0 = r2*tan(r0*0.5);
		result = p_cylinder(axis,r0);
		break;

	case SV_SPHERE:
		r2 = sl_d*cen - r;
		if (r2 > r0) 
			result = sv_primitive(r2 - r0);
		else
		{
			r0 = sqrt(r0*r0 - r2*r2);
			result = p_cylinder(sv_line(sl_d,cen),r0);
		}
		break;

	default:
	    svlis_error(
		"special_slice(sv_primitive, mod_kind, real)", 
		"wrong kind of primitive", 
		SV_CORRUPT);
	}

	if(!result.exists()) return(result);

        switch(po)
        {
        case SV_PLUS:   break;
        case SV_COMP:   result = -result; break;
        case SV_ABS:    result = abs(result); break;
        case SV_SSQRT:  result = s_sqrt(result); break;
        case SV_SIGN:   result = sign(result); break;
        default:
                svlis_error("special_slice","dud primitive operator",
                        SV_CORRUPT);
        }

	return(result);
}

sv_primitive slice(const sv_primitive& p, const mod_kind d, const sv_real r)
{
    sv_primitive result, res_1, res_2;
    
    switch (p.kind())
    {
        case SV_REAL:
	    result = p;
	    break;
	
	case SV_PLANE:
	    result = slice(p.plane(), d, r);
	    break;
	
	case SV_CYLINDER:
	case SV_SPHERE:
	case SV_CONE:
		result = special_slice(p, d, r);
		if(result.exists()) break;

	case SV_TORUS:
	case SV_CYCLIDE:	   	    	    
	case SV_GENERAL:
	    switch (p.op())
	    {
               case SV_PLUS:
		    res_1 = slice(p.child_1(), d, r);		
		    res_2 = slice(p.child_2(), d, r);
		    result = res_1 + res_2;
		    break;

		case SV_MINUS:
		    res_1 = slice(p.child_1(), d, r);		
		    res_2 = slice(p.child_2(), d, r);
		    result = res_1 - res_2;
		    break;

		case SV_TIMES:		    		
		    res_1 = slice(p.child_1(), d, r);		
		    res_2 = slice(p.child_2(), d, r);
		    result = res_1 * res_2;
		    break;

		case SV_DIVIDE:
		    res_1 = slice(p.child_1(), d, r);		
		    res_2 = slice(p.child_2(), d, r);
		    result = res_1/res_2;
		    break;

		case SV_POW:
		    res_1 = slice(p.child_1(), d, r);		
		    res_2 = slice(p.child_2(), d, r);
		    result = res_1^res_2;
		    break;

		case SV_ABS:
		    res_1 = slice(p.child_1(), d, r);		
		    result = abs(res_1);
		    break;

		case SV_COMP:
		    res_1 = slice(p.child_1(), d, r);		
		    result = -res_1;
		    break;

		case SV_SIN:
		    res_1 = slice(p.child_1(), d, r);		
		    result = sin(res_1);
		    break;

		case SV_COS:
		    res_1 = slice(p.child_1(), d, r);		
		    result = cos(res_1);
		    break;

		case SV_EXP:
		    res_1 = slice(p.child_1(), d, r);		
		    result = exp(res_1);
		    break;

		case SV_SSQRT:
		    res_1 = slice(p.child_1(), d, r);		
		    result = s_sqrt(res_1);
		    break;

		case SV_SIGN:
		    res_1 = slice(p.child_1(), d, r);		
		    result = sign(res_1);
		    break;

		default:
	    svlis_error(
		"slice(sv_primitive, mod_kind, real)", 
		"attempt to slice user primitive", 
		SV_WARNING);
	    }
	    break;
	default:
	    svlis_error(
		"slice(sv_primitive, mod_kind, real)", 
		"dud primitive op!", 
		SV_CORRUPT);
    }
    
    return(result);
}

sv_set slice(const sv_set& s, const mod_kind d, const sv_real r)
{
    sv_set result, res_1, res_2;
    
    switch (s.contents())
    {
	case SV_NOTHING:
	case SV_EVERYTHING:
	    result = s;
	    break;
	
	case 1:
	    result = sv_set(slice(s.primitive(), d, r));
	    break;
	
	default:
	    switch (s.op())
	    {
		case SV_UNION:
		    res_1 = slice(s.child_1(), d, r);
		    res_2 = slice(s.child_2(), d, r);
		    result = res_1 | res_2;
		    break;
		
		case SV_INTERSECTION:
		    res_1 = slice(s.child_1(), d, r);
		    res_2 = slice(s.child_2(), d, r);
		    result = res_1 & res_2;
		    break;
		
		default:
		    svlis_error(
			"slice(sv_set,...)", 
			"Duff operator!", 
			SV_CORRUPT);
	    }
    }
    
    // Make sure attributes (eg colour) are retained.
    if (s.has_attribute())
	result = result.attribute(s.attribute());
	
    return (result);
}


sv_set_list slice(const sv_set_list& sl, const mod_kind d, const sv_real r)
{
    sv_set_list result;
    sv_set_list n = sl;

    while(n.exists())
    {
        result = merge(result,slice(n.set(), d, r));
	n = n.next();
    }

    return(result);
}


// Slice a dimension out of a svLis model

sv_model slice(const sv_model& m, const mod_kind d, const sv_real r)
{
    sv_box b = slice(m.box(), d, r);
    sv_set_list sl = slice(m.set_list(), d, r);
    
    sv_model result = sv_model(sl, b, sv_model());
    return (result);
}


// Procedure to rewrite a svLis set tree
// in the Disjunctive Form.

sv_set sv_set::disjunctive_form() const
{
    sv_set result, res_1, res_2;
    switch (contents())
    {
	case SV_EVERYTHING:
	case SV_NOTHING:
	case 1:
	    result = *this;
	    break;

	default:
	    res_1 = child_1().disjunctive_form();		    
	    res_2 = child_2().disjunctive_form();

	    switch (op())
	    {
		case SV_UNION:
		    result = res_1 | res_2;
		    break;

		case SV_INTERSECTION:
		    if (    res_1.contents() > 1
			&&  res_1.op() == SV_UNION)
		    {
			// Have to rearrange.
			result = (res_1.child_1() & res_2).disjunctive_form() | (res_1.child_2() & res_2).disjunctive_form();
		    }
		    else
		    {
			if (    res_2.contents() > 1
			    &&  res_2.op() == SV_UNION)
			{
			    // Have to rearrange.
			    result = (res_2.child_1() & res_1).disjunctive_form() | (res_2.child_2() & res_1).disjunctive_form();
			}
			else
			{
			    // No unions around so no need to rearrange.
			    result = res_1 & res_2;
			}
		    }
		    break;
		
		default:
		    svlis_error("sv_set::disjunctive_form()", 
			     "Duff operator!", 
			     SV_CORRUPT);
	    }
    }
    
    return (result);
}


sv_set_list listOfProducts_r(const sv_set& dj_form)
{
    sv_set_list result, res_1, res_2;
    
    if ((dj_form.op() == SV_INTERSECTION) || (dj_form.contents() < 2))
	result = sv_set_list(dj_form);
    else
    {
	res_1 = listOfProducts_r(dj_form.child_1());
	res_2 = listOfProducts_r(dj_form.child_2());
	result = merge(res_1, res_2);
    }
    
    return (result);
}

// Put a set into the disjunctive form and pull out
// the products into a list.

sv_set_list sv_set::list_products() const
{
    sv_set dj_form = disjunctive_form();
    
    sv_set_list result = listOfProducts_r(dj_form);
    return (result);
}

// Approximations to the natural quadrics as polyhedra
// These are useful as the faceter renders convex polyhedra
// very quiclky, so they can be used as approximations
// to set things up

static sv_real p_apprx = 0.01;
static sv_integer p_faces = 0;

void polygon_approximation(sv_real a) { p_apprx = a; }
sv_real polygon_approximation() { return(p_apprx); }

void polygon_faces(sv_integer fc) { p_faces = fc; }
sv_integer polygon_faces() { return(p_faces); }

sv_integer poly_count()
{
        if(p_faces) return(p_faces);
	sv_integer i;
	i = (sv_integer)(M_PI/acos(1/(1 + p_apprx)));
	if(i < 2) i = 2;
	return(i);
}

sv_set poly_cylinder(const sv_line& axis, sv_real radius)
{

//  This returns an infinitely long cylinder aligned with the line axis
//  and of radius radius.
    
	sv_point ax = axis.direction;
	sv_point cent = axis.origin;

//   Generate a vector perpendicular to the cylinder's axis.

	sv_point rad = right(ax);

	sv_set result = sv_set(SV_EVERYTHING);

	sv_plane f = sv_plane(rad, cent + rad*radius);

	sv_integer i = poly_count();
	sv_real ainc = 2*M_PI/(sv_real)i;

	while(i)
	{
		result = result & sv_set(f);
		f = f.spin(axis, ainc);
		i--;
	}
	return(result);
}


sv_set poly_cone(const sv_line& axis, sv_real angle)
{

//  This returns a cone aligned with the line axis
//  and of radius radius.
    
	sv_point ax = axis.direction;
	sv_point cent = axis.origin;

//   Generate a vector perpendicular to the cone's axis.

	sv_point rad = right(ax);

	sv_set result = sv_set(SV_EVERYTHING);

	sv_plane f = sv_plane(rad*cos(angle/2) + ax*sin(angle/2), cent);

	sv_integer i = poly_count();
	sv_real ainc = 2*M_PI/(sv_real)i;

	while(i)
	{
		result = result & sv_set(f);
		f = f.spin(axis, ainc);
		i--;
	}
	return(result);
}

sv_set poly_sphere(const sv_point& centre, sv_real radius)
{
	sv_integer i = poly_count()/4;
	if(i < 2) i = 2;
	sv_real ainc = M_PI/(2*(sv_real)i);
	sv_real a = 0;
	sv_line axis;

	sv_set result = sv_set(sv_plane(SV_Z, centre + radius*SV_Z)) &
			sv_set(sv_plane(-SV_Z, centre - radius*SV_Z));

	sv_real r0, r1, b;

	while(i)
	{
		i--;
		a = a + ainc;
		r0 = radius*sin(a);
		r1 = radius*cos(a);
		b = M_PI*0.5 - a;
		axis = sv_line(SV_Z, centre + SV_Z*(r1 + r0/tan(b)));
		result = result & poly_cone(axis, 2*b);
		axis = sv_line(-SV_Z, centre - SV_Z*(r1 + r0/tan(b)));
		if(i)result = result & poly_cone(axis, 2*b);
	}

	return(result);
}

// Generate a random point uniformly inside a box

// Uniform distribution

sv_point ran_point_u(const sv_box& b)
{
	sv_real x = b.xi.lo() + (b.xi.hi() - b.xi.lo())*ran_real();
	sv_real y = b.yi.lo() + (b.yi.hi() - b.yi.lo())*ran_real();
	sv_real z = b.zi.lo() + (b.zi.hi() - b.zi.lo())*ran_real();
	return(sv_point(x,y,z));
}


// Niederreiter low-discrepancy distribution

static int nd_new = 1;
sv_point ran_point_n(const sv_box& b)
{

        int inlo2_(long int *, long int *);
        int golo2_(sv_real *);



        long skip = 4096;  
        long dimen = 3;
        static sv_real quasi[12];
        if(nd_new)
	{
           inlo2_(&dimen,&skip);
           nd_new =0;
	}
        golo2_(quasi); 
	sv_real x = b.xi.lo() + (b.xi.hi() - b.xi.lo())*quasi[0];
	sv_real y = b.yi.lo() + (b.yi.hi() - b.yi.lo())*quasi[1];
	sv_real z = b.zi.lo() + (b.zi.hi() - b.zi.lo())*quasi[2];
	return(sv_point(x,y,z));
}

#if macintosh
 #pragma export off
#endif

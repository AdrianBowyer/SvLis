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
 * SvLis - polygon procedures
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 2 June 1993
 * This version: 23 September 2000
 *
 */ 


#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "interval.h"
#include "sv_b_cls.h"
#include "prim.h"
#include "attrib.h"
#include "sv_set.h"
#include "decision.h"
#include "polygon.h"
#include "model.h"
#include "picture.h"
#include "surface.h"
#include "u_attrib.h"
#include "sv_util.h"
#if macintosh
 #pragma export on
#endif

// Unique tag

sv_integer sv_p_gon::tag() { return(SVT_F*SVT_POLYGON); }

// Polygon I/O

void write(ostream& s, sv_p_gon_kind pk, sv_integer level)
{
	put_white(s, level);
	switch(pk)
	{
	case PT_SET: s << 'P'; break;
	case P_LINE: s << 'L'; break;
	case P_GON: s << 'O'; break;

	default:
		svlis_error("write(.. sv_p_gon_kind)",
			"dud polygon kind",
			SV_CORRUPT); 
	}
}

void write(ostream& s, sv_p_gon* pg, sv_integer level)
{
	sv_p_gon* n;
	sv_p_gon pg_dum;
	sv_integer nxl = level+1;

	put_white(s, level);
	put_token(s, SVT_POLYGON, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB, 0, 0);
	s << SV_EL;

	if(pg)
	{
		writei(s, 1, nxl); s << SV_EL;
		n = pg;
		do
		{
			writei(s, 1, nxl); 
			s << ' ';
			write(s, n->kind, 0);
			s << ' ' << n->p << ' ' << 
			   n->g << ' ' << 
			   (int)n->edge << SV_EL;
			n = n->next;
		} while (n != pg);
		writei(s, 0, nxl);
		s << SV_EL;
	} else
	{
		writei(s, 0, nxl);
		s << SV_EL;
	}
	put_white(s, level);
	put_token(s, SVT_CB, 0, 0);
	s << SV_EL;	
}

ostream& operator<<(ostream& s, sv_p_gon* pg)
{
	write(s, pg, 0);
	return(s);
}

istream& operator<<(istream& s, sv_p_gon* pg)
{
	read(s, &pg);
	return(s);
}

// return the number of vertices

sv_integer p_gon_vertex_count(sv_p_gon* pg)
{
	sv_p_gon* n;
	sv_integer total = 0;
	if(pg)
	{
		n = pg;
		do
		{
			total++;
			n = n->next;
		} while (n != pg);
	}
	return(total);
}

// Set the grad values from a parent primitive
// NB this uses the p_grad function, q.v.

void set_p_gon_grad(sv_p_gon* pg, const sv_primitive& pp)
{
	sv_primitive p = pp;
	if(p_thin(p)) p = p.child_1();  // Loose abs, which just causes trouble
	sv_p_gon* n;
	if(pg)
	{
		n = pg;
		do
		{
			n->g = (p.p_grad(n->p)).norm();
			n = n->next;
		} while (n != pg);
	}
}

// delete flagged points after clipping and remove edge
// flags

sv_p_gon* tidy_p_gon(sv_p_gon* pg)
{
	if(!pg) return(0);

	sv_p_gon* n = pg->next;
	sv_p_gon* last = pg;

	while(n != pg)
	{
		if (n->edge & DEAD)  // I mean & not &&
		{
			last->next = n->next;
			delete n;
		} else
		{
			last = n;
			n->edge = 0; // Remove flags
		}
		n = last->next;
	}

// Deal with the posibility that pg needs to be deleted.

	if (pg->edge & DEAD)
	{
		if (pg->next == pg)
		{
			delete pg;	// Nothing left!
			last = 0;
		} else
		{
			last->next = pg->next;
			delete pg;
		}
		return(last);
	}

	pg->edge = 0; // Remove flags

	return(pg);
}

// Throw away a whole polygon

void kill_p_gon(sv_p_gon* pg)
{
	sv_p_gon* n;
	sv_p_gon* temp;

	if (pg)
	{
		n = pg->next;
		while (n != pg)
		{
			temp = n->next;
			delete n;
			n = temp;
		}
		delete pg;
	}
}

// Copy a whole polygon

sv_p_gon* copy_p_gon(sv_p_gon* pg)
{
	sv_p_gon* n;
	sv_p_gon* temp1 = 0;
	sv_p_gon* temp2 = 0;

	if (pg)
	{
		temp1 = first_point(pg->p, pg->kind);
		// if (pg->edge & INVISIBLE) make_invisible(temp1);
		temp1->g = pg->g;
		n = pg->next;
		while (n != pg)
		{
			temp2 = add_edge(temp1,n->p);
			// if (n->edge & INVISIBLE) make_invisible(temp1);
                        temp2->g = n->g;
			temp1 = temp2;
			n = n->next;
		}
		temp1 = temp1->next;  // Ensure copy starts in the right place
	}
	return(temp1);
}



// Copy a whole polygon changing its kind

sv_p_gon* copy_p_gon(sv_p_gon* pg, sv_p_gon_kind k)
{
	sv_p_gon* n;
	sv_p_gon* temp1 = 0;
	sv_p_gon* temp2 = 0;

	if (pg)
	{
		temp1 = first_point(pg->p, k);
		temp1->edge = pg->edge;
                temp1->g = pg->g;
		n = pg->next;
		while (n != pg)
		{
			temp2 = add_edge(temp1,n->p);
			temp1->edge = n->edge;
                        temp1->g = n->g;
			temp1 = temp2;
			n = n->next;
		}
		temp1 = temp1->next; // ensure copy starts in the right place
	}
	return(temp1);
}


// This chops the corners off the polygon that lie outside primitive cut
// If 0 is returned, there's nothing left.  This assumes that all flags
// are 0.  The polygon pointer returned points to the new point generated
// where the polygon leaves solid and becomes air (if such is created),
// otherwise the original is returned.  If line is true the procedure
// generates the polyline of intersection of the polygon and the
// primitive; if the polygon is already a polyline, then a point set
// is generated.

sv_p_gon* p_gon_chop(sv_p_gon* pg, const sv_primitive& cut, sv_real accy, int line)
{
	if(!pg) return(0);

	sv_p_gon* n = pg;
	sv_p_gon* last;
	sv_p_gon* new_pt;
	sv_line l;
	sv_real v, lv;

	switch(pg->kind)
	{
	case PT_SET:
		if(line)
		{
			kill_p_gon(pg);
			return(0);
		}

		do
		{
			if (member(cut.value(n->p)) == SV_AIR) n->edge = n->edge | DEAD;
			n = n->next;
		} while(n != pg);
		return(tidy_p_gon(pg));

	case P_LINE:
	case P_GON:
		n = pg->next;
		last = pg;
		if (member(lv = cut.value(last->p)) == SV_AIR) 
			last->edge = last->edge | DEAD;
		do
		{
			if (member(v = cut.value(n->p)) == SV_AIR)
			{
				if (!(last->edge & DEAD))
				{
					new_pt = add_edge(last, binary_chop(cut, n->p, v,
							last->p, lv, accy));
					new_pt->edge = new_pt->edge | NEW;
				}
				n->edge = n->edge | DEAD;
			} else
			{
				if (last->edge & DEAD)
				{
					new_pt = add_edge(last, binary_chop(cut, n->p, v,
							last->p, lv, accy));
					new_pt->edge = new_pt->edge | NEW;
				}
			}
			last = n;
			lv = v;
			n = n->next;
		} while(last != pg);
		if(line) 
		{
			n = pg;
			do
			{
				if(n->kind == P_GON)
					n->kind = P_LINE;
				else
					n->kind = PT_SET;
				if(!(n->edge & NEW)) n->edge = n->edge | DEAD;
				n = n->next;
			} while(n != pg);
		}
		break;

	default:
		svlis_error("p_gon_chop", "dud polygon kind", SV_CORRUPT);
	}

	return(tidy_p_gon(pg));
}


// Clip a polygon to a box

sv_p_gon* p_gon_chop(sv_p_gon* pg, const sv_box& b)
{
	sv_real accy = 1; // Not needed as planes treated specially
	sv_primitive f;
	for(int i = 0; i < 6; i++)
	{
		f = sv_primitive(box_face(b,0));
		pg = p_gon_chop(pg, f, accy, 0);
		if(!pg) return(0);
	}
	
	return(pg);
}

// Work out the polygon for a plane intersecting a box

sv_p_gon* box_plane_pg(const sv_plane& f, const sv_box& b)
{
	sv_integer c0, c1;  	// Corner indices
	sv_point p0, p1;    	// Corner points
	sv_point n = f.normal;
	sv_primitive p = sv_primitive(f);
	sv_point pc;        	// Cut point

	sv_integer i;    	// Loop variables
	sv_p_gon* pg = 0;	 	// Polygon being built

	sv_real m_dir;      	// Normal main direction +ve or -ve?
	sv_integer dir;     	// Normal main direction 0=z, 1=y, 2=x

// Set up main direction

	if (fabs(n.z) > 0.5)
	{
		dir = 0;
		m_dir = n.z;
	} else
	{
		if (fabs(n.y) > 0.5)
		{
			dir = 1;
			m_dir = n.y;
		} else
		{
			if (fabs(n.x) > 0.5)
			{
				dir = 2;
				m_dir = n.x;
			} else
				svlis_error("box_plane_pg",
					"dud plane normal",SV_WARNING);
		}
	}

// Loop through the four box edges in the primitive's normal direction,

	for(i = 0; i < 4; i++)
	{
		if(m_dir > 0)
			box_edge(4*dir + i,&c0, &c1);
		else
			box_edge(4*dir - i + 3,&c0, &c1);
		p0 = b.corner(c0);
		p1 = b.corner(c1);
		pc =  plane_line(f, sv_line(p1-p0,p0));
		if(!pg)  // First valid polygon point?
			pg = first_point(pc, P_GON);
		else
			pg = add_edge(pg, pc);
	}

	sv_primitive face;
	switch(dir)
	{
	case 0: // z
		face = sv_primitive(box_face(b,2));
		pg = p_gon_chop(pg, face, 1, 0);
		face = sv_primitive(box_face(b,5));
		pg = p_gon_chop(pg, face, 1, 0);
		break;

	case 1: // y
		face = sv_primitive(box_face(b,1));
		pg = p_gon_chop(pg, face, 1, 0);
		face = sv_primitive(box_face(b,4));
		pg = p_gon_chop(pg, face, 1, 0);
		break;

	case 2: // x
		face = sv_primitive(box_face(b,0));
		pg = p_gon_chop(pg, face, 1, 0);
		face = sv_primitive(box_face(b,3));
		pg = p_gon_chop(pg, face, 1, 0);
		break;

	default:
		svlis_error("box_plane_pg","dud plane direction",SV_CORRUPT);
	}

	return(pg);
}

// Work out the mean triangle normal vector of a polygon

sv_point p_gon_tri_norm(sv_p_gon* pg)
{
	sv_point nml = SV_OO;
	sv_p_gon* n1; 
	sv_p_gon* n2;
	if(pg)
	{
		n1 = pg->next;
		n2 = n1->next;
		do
		{
			nml = nml + ((n1->p - pg->p)^(n2->p - pg->p));
			n1 = n2;
			n2 = n2->next;
		} while ((n2 != pg) && (n1 != pg));
	}
	return(nml);
}

// Generate the apex of a cone
// NB - does a double cone

void cone_facets(const sv_primitive& p, const sv_box& b, sv_p_gon* pg[])
{
	sv_integer k; 
	sv_real r0;
	sv_real r1; 
	sv_real r2;
	sv_plane f;
        sv_point cen1; 
	sv_line axis;

        p.parameters(&k, &r0, &r1, &r2, &f, &cen1, &axis);
	r1 = 2*sqrt(b.diag_sq());
	cen1 = axis.origin + r1*axis.direction;
	sv_point cen2 = axis.origin - r1*axis.direction;
	sv_point rad = right(axis.direction)*tan(r0/2)*r1;
	sv_point pp = cen1 + rad;
	sv_point qq = cen2 + rad;
	sv_point rr;
	sv_real angle = M_PI/3;
	for(int i = 0; i < 6; i++)
	{
		pg[i] = first_point(pp, P_GON);
		rr = axis.origin + rad*0.001;   // Cheat to get grads at the apex
		pg[i] = add_edge(pg[i], rr);
		pg[i] = add_edge(pg[i], qq);
		rad = rad.spin(axis, angle);
		pp = cen1 + rad;
		qq = cen2 + rad;
		rr = axis.origin + rad*0.001;
		pg[i] = add_edge(pg[i], rr);
		pg[i] = add_edge(pg[i], pp);
		pg[i] = p_gon_chop(pg[i], b);
	}
}

// Create a polygon attribute from a box of tetrahedra containing polygons

sv_attribute sv_box_tets::attribute()
{
	sv_attribute result;
	sv_p_gon* p = 0;

	for(int i = 0; i < 6; i++)
	{
	  if(pg[i]) 
	  {
		set_p_gon_grad(pg[i], s.primitive());
		result = merge(result, sv_attribute(-p->tag(), 
                	new sv_user_attribute((void*)pg[i])));
	  }
	  if(pgn[i]) 
	  {
		set_p_gon_grad(pgn[i], s.primitive());
		result = merge(result, sv_attribute(-p->tag(), 
                	new sv_user_attribute((void*)pgn[i])));
	  }
        }
	return(result);
}

// Create a convex polyhedron in a box from its set representation

// Clip a polygon to a convex set

sv_p_gon* set_clip(sv_p_gon* p, const sv_set& s, const sv_set& s_clip)
{
	if(!p) return(0);

	if(s.unique() == s_clip.unique()) return(p);

	if(s_clip.contents() == 1)
		return(p_gon_chop(p, s_clip.primitive(), 1, 0));

	if(s_clip.contents() > 1)
		return(set_clip(set_clip(p, s, s_clip.child_1()), s, s_clip.child_2()));

// Should never get here

	return(p);
}

// Create a polygon and clip it for each leaf set

sv_set c_p_r(const sv_set& s, const sv_set& srt, const sv_box& b)
{
	sv_p_gon* p;
	
	if(s.contents() > 1)
	{
		if(s.op() == SV_UNION)
			svlis_error("c_p_r","convex set has union operator",SV_CORRUPT);
		else
			return(c_p_r(s.child_1(), srt, b) & c_p_r(s.child_2(), srt, b));
	}

	if(s.contents() != 1) return(s);

	sv_set result = s;

	p = box_plane_pg(s.primitive().dump_scales().plane(), b);
	if(p)
	{
		p = set_clip(p, s, srt);
		if(p) 
		{
			set_p_gon_grad(p, s.primitive());
			result = result.attribute(merge(result.attribute(), 
				sv_attribute(-p->tag(), new sv_user_attribute((void*)p))));
		}
	}

	return(result);
}

// First the special cases - all those involving thin sets

// Three planes intersecting in a point

sv_set s_point(const sv_set& s, const sv_box& b, const sv_set& s0, 
	const sv_set& s1, const sv_set& s2)
{
	sv_set result;
	return(result);
}

// Two planes intersecting in a line

sv_set s_line(const sv_set& s, const sv_box& b, const sv_set& s0, 
	const sv_set& s1)
{
	sv_set result;
	return(result);
}

// One plane making a single polygon

sv_set s_plane(const sv_set& sp, const sv_set& s, const sv_box& b, const sv_set& clipper)
{
	sv_set result;
	sv_primitive pr;

	if(s.contents() == 1)
	{
		if(s == sp)
		{
	 		result = sp;
			pr = sp.primitive();
			if(pr.op() != SV_ABS)
				svlis_error("s_plane","non-thin set",SV_CORRUPT);
			else
				pr = pr.child_1();
	  		sv_p_gon* p = box_plane_pg(pr.plane(), b);
	  		if(p)
	  		{
				p = set_clip(p, sp, clipper);
				if(p) 
				{
					set_p_gon_grad(p, pr);
					result = result.attribute(merge(result.attribute(), 
						sv_attribute(-p->tag(), new sv_user_attribute((void*)p))));
				}
	  		}

	  		return(result);
		}
		return(s);
	}

	if(s.contents() <= 1) return(s);

	if(s.op() == SV_UNION)
		svlis_error("s_plane","convex set has union operator",SV_CORRUPT);
	else
		return(s_plane(sp, s.child_1(), b, clipper) & s_plane(sp, s.child_2(), b, clipper));

	return(result);
}

// Count and record the thin primitives

void gather_thin(const sv_set& s, sv_set thin[], sv_integer& i)
{
	if(i > 3) return;

	if(s.contents() > 1)
	{
		gather_thin(s.child_1(), thin, i);
		gather_thin(s.child_2(), thin, i);
		return;
	}

	if(s.contents() != 1) return;

	if(p_thin(s.primitive())) 
	{
		thin[i] = s;
		i++;
	}
}

// Deal with the three possible special cases

sv_set conv_polyhedron_special(const sv_set& s, const sv_box& b)
{
	sv_set result;
	sv_set thin[4];
	sv_integer i = 0;
	sv_set clipper;

	gather_thin(s, thin, i);

	switch(i)
	{
	case 1: // Single plane
		clipper = s;
		return(s_plane(thin[0], s, b, clipper));
	case 2: // Single line
		return(s_line(s, b, thin[0], thin[1]));
	case 3:// Single point
		return(s_point(s, b, thin[0], thin[1], thin[2]));
	case 0:
	default:
		;
	}
	
	return(result);
}



// Non-recursive procedure to generate a convex polyhedron

sv_set conv_polyhedron(const sv_set& s, const sv_box& b)
{
	sv_set rs = conv_polyhedron_special(s, b);
	if(rs.exists()) return(rs);
	return(c_p_r(s, s, b));
}


// Create the tetrahedra and polygons (if any) for a leaf set in a box

sv_box_tets::sv_box_tets(const sv_set& ss, const sv_box& bb, sv_real accy, sv_integer special)
{
	sv_integer i,j,k;   // Loop variables
	mem_test ds;        // Temprary store for membership test values
	sv_point corner[8]; // The box corners
	sv_real value[8];   // Potentials of the leaf primitive at the corners
	sv_point sect[19];  // Points where the primitive cuts the tet edges

	if(ss.contents() != 1)
	    svlis_error("sv_box_tets", "set contents not 1", 
		SV_CORRUPT);

	s = ss;
	diff_sign = SV_AIR;

	for(i = 0; i < 6; i++)
	{
		pg[i] = 0;
		pgn[i] = 0;
	}

// Find the un-absolute-valued primitive

	sv_primitive p = ss.primitive();
	if(p_thin(p)) p = p.child_1();

// Special case - everything in this box is a plane; just
// make a single polygon.

	if(special == SV_F_PLANES)
	{
		if(p.kind() != SV_PLANE)
			svlis_error("sv_box_tets", "primitive not a plane", 
				SV_CORRUPT);
		sv_plane pf = p.plane();
		pg[0] = box_plane_pg(pf,bb);
		diff_sign = member(pf.value(bb.corner(0)));
		if(pg[0]) diff_sign = SV_SURFACE; 
		return;
	}

// Special case - deal with a cone with its nose in the box

	if(special == SV_F_CONE)
	{
		cone_facets(p, bb, pg);
		if(pg[0]) diff_sign = SV_SURFACE;
		return;
	}

// General case - compute the corner potentials

	for(i = 0; i < 8; i++)
	{
	    corner[i] = bb.corner(i);
	    value[i] = p.value(corner[i]);
	}

// Check they're not all the same sign

	ds = member(value[0]);
	diff_sign = ds;
	i = 1;
	while( (i < 8) && (diff_sign != SV_SURFACE) )
	{
	    if(value[i]*value[i-1] < 0) diff_sign = SV_SURFACE;
	    i++;
	}
	if(diff_sign != SV_SURFACE) return;

// Find the intersection points along the 19 tet edges

	for(i = 0; i < 19; i++)
	{
		box_edge(i,&j,&k);
		if(value[j]*value[k] < 0)
			sect[i] = binary_chop(p, corner[j], value[j], corner[k], value[k], accy);
	}

// Compute the six or fewer polygons - here the lookup tables come
// into their own

	sv_integer pol_pat;
	for(i = 0; i < 6; i++)
	{
		pol_pat = 0;
		for(j = 0; j < 4; j++)
		   if(value[sv_tet_vert(i,j)] < 0) pol_pat = pol_pat | (8 >> j);
		if(pol_pat && (pol_pat != 15))
		{
		   pg[i] = first_point(sect[sv_tet_edge(i, sv_tet_pol(pol_pat, 0))], P_GON);
		   for(j = 1; j < sv_v_count(pol_pat); j++)
			pg[i] = add_edge(pg[i],sect[sv_tet_edge(i, sv_tet_pol(pol_pat,j))]);
		} else
			pg[i] = 0;
	}
}

// Clip a tet pattern to a primitive

void sv_box_tets::clip(const sv_primitive& p, sv_real accy)
{
	for(int i = 0; i < 6; i++)
		pg[i] = p_gon_chop(pg[i], p, accy, 0);
}


// Clip the tets down to a single line

void sv_box_tets::line(const sv_primitive& p, sv_real accy)
{
	for(int i = 0; i < 6; i++)
		pg[i] = p_gon_chop(pg[i], p, accy, 1);
}

// Notch a tet pattern to the union of two primitives
// NB notching must be done after clipping and/or lining

void sv_box_tets::notch(const sv_primitive& p1, const sv_primitive& p2, sv_real accy)
{
	for(int i = 0; i < 6; i++)
	{
		pgn[i] = copy_p_gon(pg[i]);
		pg[i] = p_gon_chop(pg[i], p1, accy, 0);
		pg[i] = p_gon_chop(pg[i], -p2, accy, 0);  // Added to stop overlap
		pgn[i] = p_gon_chop(pgn[i], p2, accy, 0);
	}
}

// Deal with two polygons that (may) cut in an edge

sv_set clip_two(const sv_set& s, sv_box_tets& t1, sv_box_tets& t2, sv_real accy)
{
	sv_set result;
	sv_attribute a1, a2;
	sv_integer choice = 0;
	sv_set s1 = t1.set();
	sv_set s2 = t2.set();
	sv_primitive p1 = s1.primitive();
	sv_primitive p2 = s2.primitive();

	if (p_thin(p1)) choice = choice | 1;
	if (p_thin(p2)) choice = choice | 2;
	if (s.op() == SV_INTERSECTION) choice = choice | 4;

	switch(choice)
	{
	case 0:	// solid | solid
		t1.clip(-p2, accy);
		t2.clip(-p1, accy);
		break;

	case 1: // surface | solid
		t1.clip(-p2, accy);
		break;
			
	case 2: // solid | surface
		t2.clip(-p1, accy);
		break;

	case 3: // surface | surface
		break;

	case 4: // solid & solid
		t1.clip(p2, accy);
		t2.clip(p1, accy);
		break;

	case 5: // surface & solid
		t1.clip(p2, accy);
		t2.clean_out();
		break;

	case 6: // solid & surface
		t1.clean_out();
		t2.clip(p1, accy);
		break;

	case 7: // surface & surface
		t2.line(p1.child_1(), accy);
		t1.clean_out();
		result = s.attribute(merge(s.attribute(), t2.attribute()));
		return(result);

	default:
		svlis_error("clip_two","| operator doesn't work on integers",
			SV_CORRUPT);
	}

	a1 = merge(t1.attribute(), s.child_1().attribute());
	a2 = merge(t2.attribute(), s.child_2().attribute());

	if(s.op() == SV_UNION)
		result = s1.attribute(a1) | s2.attribute(a2);
	else
		result = s1.attribute(a1) & s2.attribute(a2);

	result = result.attribute(s.attribute());

	return(result);
}

// Deal with three polygons that (may) cut in a corner

sv_set clip_three(const sv_set& s, sv_box_tets& t1, sv_box_tets& t2, sv_box_tets& t3, sv_real accy)
{
	sv_set result, temp;
	sv_attribute a1, a2, a3;
	sv_attribute atemp = s.child_2().attribute();
	sv_integer choice = 0;
	sv_set s1 = t1.set();
	sv_set s2 = t2.set();
	sv_set s3 = t3.set(); 
	sv_primitive p1 = s1.primitive();
	sv_primitive p2 = s2.primitive();
	sv_primitive p3 = s3.primitive();

	if (p_thin(p1)) choice = choice | 1;
	if (p_thin(p2)) choice = choice | 2;
	if (p_thin(p3)) choice = choice | 4;
	if (s.op() == SV_INTERSECTION) choice = choice | 8;
	if (s.child_2().op() == SV_INTERSECTION) choice = choice | 16;



	switch(choice)
	{
	case 0: // solid | (solid | solid) ###################
		t1.clip(-p2, accy);
		t1.clip(-p3, accy);
		t2.clip(-p1, accy);
		t2.clip(-p3, accy);
		t3.clip(-p2, accy);
		t3.clip(-p1, accy);	
		break;

	case 1: // surface | (solid | solid)
		t1.clip(-p2, accy);
		t1.clip(-p3, accy);
		t2.clip(-p3, accy);
		t3.clip(-p2, accy);		
		break;

	case 2: // solid | (surface | solid)
		t1.clip(-p3, accy);
		t2.clip(-p1, accy);
		t2.clip(-p3, accy);
		t3.clip(-p1, accy);	
		break;

	case 3: // surface | (surface | solid)
		t1.clip(-p3, accy);
		t2.clip(-p3, accy);
		break;
			

	case 4: // solid | (solid | surface)
		t1.clip(-p2, accy);
		t2.clip(-p1, accy);
		t3.clip(-p1, accy);
		t3.clip(-p2, accy);	
		break;

	case 5: // surface | (solid | surface)
		t1.clip(-p2, accy);
		t3.clip(-p2, accy);
		break;

	case 6: // solid | (surface | surface)
		t2.clip(-p1, accy);
		t3.clip(-p1, accy);
		break;

	case 7: // surface | (surface | surface)
		break;

	case 8: // solid & (solid | solid) ##################
		t2.clip(p1, accy);
		t2.clip(-p3, accy);
		t3.clip(p1, accy);
		t3.clip(-p2, accy);
		t1.notch(p2, p3, accy);
		break;

	case 9: // surface & (solid | solid)
		t1.notch(p2, p3, accy);
		t2.clean_out();
		t3.clean_out();
		break;

	case 10: // solid & (surface | solid)
		t2.clip(p1, accy);
		t2.clip(-p3, accy);
		t3.clip(p1, accy);
		t1.clip(p3, accy);
		break;

	case 11: // surface & (surface | solid)
		t1.clip(p3, accy);
		t2.line(p1.child_1(), accy);
		t2.clip(-p3, accy);
		t3.clean_out();
		temp = s2 | s3;
		temp = temp.attribute(merge(t2.attribute(), atemp));
		break;

	case 12: // solid & (solid | surface)
		t2.clip(p1, accy);
		t1.clip(p2, accy);
		t3.clip(p1, accy);
		t3.clip(-p2, accy);
		break;

	case 13: // surface & (solid | surface)
		t1.clip(p2, accy);
		t3.line(p1.child_1(), accy);
		t3.clip(-p2, accy);
		t2.clean_out();
		temp = s2 | s3;
		temp = temp.attribute(merge(t3.attribute(), atemp));
		break;

	case 14: // solid & (surface | surface)
		t1.clean_out();
		t2.clip(p1, accy);
		t3.clip(p1, accy);
		break;

	case 15: // surface & (surface | surface)
		t2.line(p1.child_1(), accy);
		t3.line(p1.child_1(), accy);
		t1.clean_out();
		break;

	case 16: // solid | (solid & solid) ###############
		t2.clip(p3, accy);
		t2.clip(-p1, accy);
		t3.clip(p2, accy);
		t3.clip(-p1, accy);
		t1.notch(-p2, -p3, accy);
		break;

	case 17: // surface | (solid & solid)
		t2.clip(p3, accy);
		t3.clip(p2, accy);
		t1.notch(-p2, -p3, accy);
		break;

	case 18: // solid | (surface & solid)
		t3.clean_out();
		t2.clip(-p1, accy);
		t2.clip(p3, accy);
		break;

	case 19: // surface | (surface & solid)
		t3.clean_out();
		t2.clip(p3, accy);
		break;

	case 20: // solid | (solid & surface)
		t2.clean_out();
		t3.clip(-p1, accy);
		t3.clip(p2, accy);
		break;

	case 21: // surface | (solid & surface)
		t2.clean_out();
		t3.clip(p2, accy);
		break;

	case 22: // solid | (surface & surface)
		t2.line(p3.child_1(), accy);
		t2.clip(-p1, accy);
		t3.clean_out();
		temp = s2 & s3;
		temp = temp.attribute(merge(t2.attribute(), atemp));
		break;

	case 23: // surface | (surface & surface)
		t2.line(p3.child_1(), accy);
		t3.clean_out();
		temp = s2 & s3;
		temp = temp.attribute(merge(t2.attribute(), atemp));
		break;

	case 24: // solid & (solid & solid) #############
		t1.clip(p2, accy);
		t1.clip(p3, accy);
		t2.clip(p1, accy);
		t2.clip(p3, accy);
		t3.clip(p1, accy);
		t3.clip(p2, accy);
		break;

	case 25: // surface & (solid & solid)
		t2.clean_out();
		t3.clean_out();
		t1.clip(p2, accy);
		t1.clip(p3, accy);
		break;

	case 26: // solid & (surface & solid)
		t1.clean_out();
		t3.clean_out();
		t2.clip(p1, accy);
		t2.clip(p3, accy);
		break;

	case 27: // surface & (surface & solid)
		t1.clean_out();
		t3.clean_out();
		t2.line(p1.child_1(), accy);
		t2.clip(p3, accy);
		break;

	case 28: // solid & (solid & surface)
		t1.clean_out();
		t2.clean_out();
		t3.line(p2, accy);
		t3.clip(p1, accy);
		break;

	case 29: // surface & (solid & surface)
		t1.clean_out();
		t2.clean_out();
		t3.line(p1.child_1(), accy);
		t3.clip(p2, accy);
		break;

	case 30: // solid & (surface & surface)
		t1.clean_out();
		t3.clean_out();
		t2.line(p3.child_1(), accy);
		t2.clip(p1, accy);
		break;

	case 31: // surface & (surface & surface)
		t2.clean_out();
		t3.clean_out();
		t1.line(p2.child_1(), accy);
		t1.line(p3.child_1(), accy);
		result = s.attribute(merge(s.attribute(), t1.attribute()));
		return(result);

	default:
		svlis_error("clip_three","| doesn't work on integers",SV_CORRUPT);
	}

	a1 = merge(t1.attribute(), s1.attribute());

	if(!temp.exists())
	{
	    a2 = merge(t2.attribute(), s2.attribute());
	    a3 = merge(t3.attribute(), s3.attribute());
	    if(s.child_2().op() == SV_UNION)
		temp = s2.attribute(a2) | s3.attribute(a3);
	    else
		temp = s2.attribute(a2) & s3.attribute(a3);
	    temp = temp.attribute(atemp);
	}

	if(s.op() == SV_UNION)
		result = s1.attribute(a1) | temp;
	else
		result = s1.attribute(a1) & temp;

	result = result.attribute(s.attribute());

	return(result);
}

// Compute the maximum coordinate of a vector and return the direction

mod_kind max_comp(const sv_point& q)
{
	mod_kind result = X_DIV;
	sv_point p = sv_point(fabs(q.x), fabs(q.y), fabs(q.z));

	if (p.x > p.y)
	{
		if(p.x < p.z) return(Z_DIV);
	} else
	{
		if(p.y > p.z) return(Y_DIV);
			else return(Z_DIV);
	}
	return(result);
}

// Decide, for a primitive, if it is flat enough to be considered
// planar in a box.  If the primitive is too curved, a
// division direction is returned  (X_DIV etc) otherwise, LEAF_M is returned.
// If ZDIV+1 is returned no decision could be made on division direction.  
// If ZDIV+2 is returned an error has occured.

static sv_real grad_fac = 1;	// How flat is flat

void set_user_grad_fac(sv_real gf)
{
	grad_fac = gf;
}
sv_real get_user_grad_fac()
{
	return(grad_fac);
}

// Empirical values for grad vector ranges

static const sv_real grad_lims[7] = {1,1,0.8,0.42,0.125,3.5,4};

int prim_flat(const sv_set& s, const sv_box& mb, sv_integer& special)
{
	sv_primitive p = s.primitive();
	sv_integer pc;
	sv_integer k; 
	sv_real r0;
	sv_real r1; 
	sv_real r2;
	sv_plane f;
        sv_point cen; 
	sv_line axis;

// Forget about complement and absolute values
// (NB p.op() should == SV_ZERO for leaves)

	while( (p.op() == SV_COMP) || (p.op() == SV_ABS) )
		p = p.child_1();

	pc = p.kind();

	switch(pc)
	{
	case SV_REAL:
		svlis_error("prim_flat","called for constant primitive",SV_WARNING);
		return(Z_DIV+2);

	case SV_PLANE:
		special = SV_F_PLANES;
		return(LEAF_M);

// Special case: cone with vertex in the box

	case SV_CONE:
        	p.parameters(&k, &r0, &r1, &r2, &f, &cen, &axis);
		if((mb.member(axis.origin) != SV_AIR) && (mb.vol() < 10*user_little_box()))
		{
			special = SV_F_CONE;
			return(LEAF_M);
		}
		break;

	default:
		break;
	}


	sv_point av_grad, ma;
	sv_real major, minor;
	sv_real gl;

	switch(box_spread(p.grad(mb), &av_grad, &ma, &major, &minor))
	{
	case BSP_C: return(Z_DIV+1);	// Singularity

	case BSP_S: return(Z_DIV+1);	// Surface points everywhere

	case BSP_OK:			// Surface points somewhere . . .

// The constants here are an empirical hack

		if(pc < 7) // Reals & Planes (0 & 1) already dealt with
			gl = grad_lims[pc];
		else
			gl = (p.degree()*1.54 - 2.66);

		if (major < gl*grad_fac) 
			return(LEAF_M);

		if (minor/major < THIN_EL)
			return(max_comp(ma));
		else
			return(Z_DIV+1);

// Dunno what's going on...

	case BSP_DUD:
	default:
		return(Z_DIV+2);
	}
	return(Z_DIV+2);
}


// Split a set tree with a contents of 2 or 3 into its leaves

void tree_split(const sv_set& s, sv_set& s1, sv_set& s2, sv_set& s3)
{
	switch(s.contents())
	{
	case 2:
		s1 = s.child_1();
		s2 = s.child_2();
		break;

	case 3:
		s1 = s.child_1();
		if(s1.contents() != 1)
			svlis_error("tree_split","set with 3 primitives has dud child",SV_CORRUPT);
		s2 = s.child_2().child_1();
		s3 = s.child_2().child_2();
		break;

	default:
		svlis_error("tree_split","wrong set contents",SV_CORRUPT);
	}
}

// Find any primitive in a set that has surface in the box

sv_primitive def_prim(const sv_set& s, const sv_box& b, sv_integer* def)
{
	sv_primitive p, q;
	mem_test res;
	sv_integer corn;

	switch(s.contents())
	{
	case SV_NOTHING:
	case SV_EVERYTHING:
			return p;

	case 1:
		p = s.primitive();
		definite(p, b, &res, &corn);
		if(res == SV_SURFACE)
		{
			*def = 1; 
			return(p);
		}
		else 
			return(q);

	default:
		p = def_prim(s.child_1(), b, def);
		if (*def) return(p);
		p = def_prim(s.child_2(), b, def);		
	}
	return(p);
}


// When a model is so complicated that it gets divided down to a very small
// box, and still hasn't terminated, this forces a facet in that box

void force_facet(const sv_model& min, const sv_set& s)
{
	sv_model m = min;
	sv_box b = m.box();
	sv_set ss;
	sv_primitive p;
	sv_integer def = 0;
	sv_plane f;
	sv_point cen, n;

	svlis_error("force_facet","called",SV_DEBUG);

	p = def_prim(s, b, &def);
	if (!def) 
	{
		svlis_error("force_facet", "can't find one", SV_DEBUG);
		return;
	}
	sv_real accy = b.diag_sq()*SMALL_LEN*SMALL_LEN;
	ss = s;
	while(ss.contents() > 1) ss = ss.child_1();
	sv_box_tets t = sv_box_tets(ss,b,accy, 0);
	ss = ss.attribute(merge(ss.attribute(), t.attribute()));
	m.replace_set(s,ss,SV_POLYGON_FLAG);
}

// Special (but common) case.  Check s for the pattern
//    [(a & b) | (c & d)]
// with all of them solid.  This happens a lot with blends
// and tangencies.

sv_integer contents_4(const sv_model& mm, const sv_set& s, int force)
{
	sv_model m = mm;

// Squared length for accuracy of facet corners

	sv_real accy = m.box().diag_sq()*SMALL_LEN*SMALL_LEN;

// The box tetrahedral decompositions

	sv_box_tets t1, t2, t3, t4;

// The leaf sets for the surfaces, and the rebuilt set

	sv_set s1, s2, s3, s4, ss;

// Primitives for testing for planes

	sv_primitive p1, p2, p3, p4;

// All flat flag

	sv_integer special1 = 0, special2 = 0, special3 = 0, special4 = 0;

// Divide direction

	sv_integer div_dir;

// Attributes for polygone

	sv_attribute a1, a2, a3, a4;

// Check for
//    [(s1 & s2) | (s3 & s4)]

	if(s.op() != SV_UNION) return(0);

	s1 = s.child_1();
	s3 = s.child_2();

	if(s1.contents() != 2) return(0);
	if(s3.contents() != 2) return(0);  // Not really needed

	if(s1.op() != SV_INTERSECTION) return(0);
	if(s3.op() != SV_INTERSECTION) return(0);

	s2 = s1.child_2();
	s1 = s1.child_1();
	s4 = s3.child_2();
	s3 = s3.child_1();

	if(s1.contents() != 1) return(0); // Not really needed
	if(s2.contents() != 1) return(0); // Not really needed
	if(s3.contents() != 1) return(0); // Not really needed
	if(s4.contents() != 1) return(0); // Not really needed

// All fat?

	p1 = s1.primitive();
	if(p_thin(p1)) return(0);
	p2 = s2.primitive();
	if(p_thin(p2)) return(0);
	p3 = s3.primitive();
	if(p_thin(p3)) return(0);
	p4 = s4.primitive();
	if(p_thin(p4)) return(0);

	if ( ((div_dir = prim_flat(s1, m.box(), special1)) == LEAF_M) || force)
	{
		 if ( ((div_dir = prim_flat(s2, m.box(), special2)) == LEAF_M) || force)
		 {
		    if ( ((div_dir = prim_flat(s3, m.box(), special3)) == LEAF_M) || force)
		    {

// Check s4 (even though s2 was flat enough) to set special4

		        if ( ((div_dir = prim_flat(s4, m.box(), special4)) == LEAF_M) || force)
		        {	
			   t1 = sv_box_tets(s1, m.box(), accy, special1);
			   t2 = sv_box_tets(s2, m.box(), accy, special2);
			   t3 = sv_box_tets(s3, m.box(), accy, special3);
			   t4 = sv_box_tets(s4, m.box(), accy, special4);
			   if( (t1.definite() == SV_SURFACE) || 
				(t2.definite() == SV_SURFACE) ||
				(t3.definite() == SV_SURFACE) ||
				(t4.definite() == SV_SURFACE)
			   )
			   {
				t1.clip(p2, accy);
				t2.clip(p1, accy);
				t3.clip(p4, accy);
				t4.clip(p3, accy);
				t1.notch(-p3, -p4, accy);
				t2.notch(-p3, -p4, accy);
				t3.notch(-p1, -p2, accy);
				t4.notch(-p1, -p2, accy);
				a1 = merge(t1.attribute(), s1.attribute());
				a2 = merge(t2.attribute(), s2.attribute());
				a3 = merge(t3.attribute(), s3.attribute());
				a4 = merge(t4.attribute(), s4.attribute());
				ss = (s1.attribute(a1) & s2.attribute(a2)) | (s3.attribute(a3) & s4.attribute(a4));
				m.replace_set(s,ss,SV_POLYGON_FLAG);
				return(1);
			   }
                        }
		     }
		 }
	}

	return(0);
}

// Decide if a set is simple enough to facet.  For this it must have 1, 2
// or 3 primitives (or 0, in which case it is thrown away).  If all the
// primitives are flat enough in the box, then
// the box is decomposed into tetrahedra and facets generated in each
// one.  If force is true, facet regardless of how non-flat the
// primitive is.

sv_integer did_facet(const sv_model& mm, const sv_set& s, int force)
{
	sv_model m = mm;

// Squared length for accuracy of facet corners

	sv_real accy = m.box().diag_sq()*SMALL_LEN*SMALL_LEN;

// The box tetrahedral decompositions

	sv_box_tets t1, t2, t3;

// The leaf sets for the surfaces, and the rebuilt set

	sv_set s1, s2, s3, ss;

// Primitives for testing for planes

	sv_primitive p1, p2, p3;

// All flat flag

	sv_integer special1 = 0, special2 = 0, special3 = 0;

// Divide direction

	sv_integer div_dir;


// Decide what to do.

// Does this box just contain one convex polyhedron?

	if(s.flags() & SV_CV_POL)
	{
		ss = conv_polyhedron(s, m.box());
		ss = ss.attribute(merge(s.attribute(), ss.attribute()));
		m.replace_set(s,ss,SV_POLYGON_FLAG);
		return(LEAF_M);
	}

// Does the box contain 1, 2, 3, or many primitives?

	switch (s.contents())
	{

// No surface there - throw this set away by saying it's been faceted.

	case SV_EVERYTHING:
	case SV_NOTHING:
		return(LEAF_M);

// A single primitive

	case 1:

// Is it reasonably flat?

		if ( ((div_dir = prim_flat(s, m.box(), special1)) == LEAF_M) || force )
		{
			p1 = s.primitive();
			t1 = sv_box_tets(s, m.box(), accy, special1);
			if(t1.definite() == SV_SURFACE)
			{
				ss = s.attribute(merge(s.attribute(), t1.attribute()));
				m.replace_set(s,ss,SV_POLYGON_FLAG);
			}
		}
		if(force) return(LEAF_M);
		return(div_dir);

// An edge?

	case 2:
		tree_split(s,s1,s2,s3);
		if ( ((div_dir = prim_flat(s1, m.box(), special1)) == LEAF_M) || force)
		{
		   if ( ((div_dir = prim_flat(s2, m.box(), special2)) == LEAF_M) || force)
		   {
			p1 = s1.primitive();
			p2 = s2.primitive();
			t1 = sv_box_tets(s1, m.box(), accy, special1);
			t2 = sv_box_tets(s2, m.box(), accy, special2);
			if ( (t1.definite() == SV_SURFACE) || (t2.definite() == SV_SURFACE) )
			{
				ss = clip_two(s, t1, t2, accy);
				m.replace_set(s,ss,SV_POLYGON_FLAG);
			}
		   }
		   if(force) return(LEAF_M);
		   return(div_dir);
		}
		return(div_dir);
// A corner?

	case 3:
		tree_split(s,s1,s2,s3);
		if ( ((div_dir = prim_flat(s1, m.box(), special1)) == LEAF_M) || force)
		{
		  if ( ((div_dir = prim_flat(s2, m.box(), special2)) == LEAF_M) || force)
		  {
		    if ( ((div_dir = prim_flat(s3, m.box(), special3)) == LEAF_M) || force)
		    {
			p1 = s1.primitive();
			p2 = s2.primitive();
			p3 = s3.primitive();
			t1 = sv_box_tets(s1, m.box(), accy, special1);
			t2 = sv_box_tets(s2, m.box(), accy, special2);
			t3 = sv_box_tets(s3, m.box(), accy, special3);
			if( (t1.definite() == SV_SURFACE) || 
				(t2.definite() == SV_SURFACE) ||
				(t3.definite() == SV_SURFACE) )
			{
				ss = clip_three(s, t1, t2, t3, accy);
				m.replace_set(s,ss,SV_POLYGON_FLAG);
			}
		     }
		     if(force) return(LEAF_M);
		     return(div_dir);
		  }
		  return(div_dir);
		}
		return(div_dir);

// Special (but common) case

	case 4:
		if(contents_4(mm, s, force)) return(LEAF_M);
		break;

	default:
		break;
	}

	if(force)
        {
		force_facet(m, s);
		return(LEAF_M);
	}

// Too complicated to facet

	return(Z_DIV + 1);
}


// This is the decision procedure used by the faceter to add facets to a
// model by dividing it to find boxes in which the individual sets in
// the set list are simple enough to facet.

void facet_decision(const sv_model& m, sv_integer level, void* vp, mod_kind* k, sv_real* c, 
		sv_model* c_1, sv_model* c_2)
{
	sv_box mb = m.box();
	sv_integer div_dir = Z_DIV+1;
	sv_integer dd_temp;
	int force = 0;

	if (mb.vol() < user_little_box())
	{
		force = 1;
	}

// Check each set in the list to see if it has enough contents to make 
// further division needed.

	sv_set_list sl = m.set_list();
	sv_set_list finished_with;		// list of sets faceted this time
	sv_set s;
	sv_integer* fw = new sv_integer[sl.count()]; // Flag for faceted sets
	sv_integer i = 0;

// For conflicting division directions this just takes the last one.

	while(sl.exists())
	{
		s = sl.set();
		if((dd_temp = did_facet(m, s, force)) == LEAF_M) 
		{
			fw[i] = 1;
		} else
		{
			div_dir = dd_temp;
			fw[i] = 0;
		}
		sl = sl.next();
		i++;
	}

// Remove any faceted sets from further consideration

	sl = m.set_list();
	i = 0;
	while(sl.exists())
	{
		s = sl.set();
		if(fw[i]) finished_with = merge(finished_with, s);
		sl = sl.next();
		i++;
	}
	sv_set_list child_sl = remove(m.set_list(), finished_with);
	delete [] fw;

// Anything left?

	if (!child_sl.exists())
	{
		*c_1 = m;
		*k = LEAF_M;
		return;
	}

// Divide the box

	sv_interval xi = mb.xi;
	sv_interval yi = mb.yi;
	sv_interval zi = mb.zi;
	sv_real x = xi.hi() - xi.lo();
	sv_real y = yi.hi() - yi.lo();
	sv_real z = zi.hi() - zi.lo();

	if (too_thin(x, y, z)) div_dir = Z_DIV + 1;

	switch(div_dir)
	{
	case X_DIV: *k = X_DIV; break;
	case Y_DIV: *k = Y_DIV; break;
	case Z_DIV: *k = Z_DIV; break;
	case Z_DIV+1:
		if (x > y)
		{
			if (z > x)
				*k = Z_DIV;
			else
				*k = X_DIV;
		} else
		{
			if (z > y)
				*k = Z_DIV;
			else
				*k = Y_DIV;
		}
		break;
	default:
		svlis_error("facet_decision","dud division direction",SV_CORRUPT);
	}

	sv_interval i_part;
	sv_box b_lo, b_hi;

	switch(*k)
	{
	case X_DIV:
			*c = xi.lo() + x/2;
			i_part = sv_interval(xi.lo(), *c + (*c - xi.lo())*get_swell_fac());
          		b_lo = sv_box(i_part, yi, zi);
			i_part = sv_interval(*c - (xi.hi() - *c)*get_swell_fac(), xi.hi());
			b_hi = sv_box(i_part, yi, zi);
			break;

	case Y_DIV:
			*c = yi.lo() + y/2;
			i_part = sv_interval(yi.lo(), *c + (*c - yi.lo())*get_swell_fac());
          		b_lo = sv_box(xi, i_part, zi);
			i_part = sv_interval(*c - (yi.hi() - *c)*get_swell_fac(), yi.hi());
			b_hi = sv_box(xi, i_part, zi);
			break;

	case Z_DIV:
			*c = zi.lo() + z/2;
			i_part = sv_interval(zi.lo(), *c + (*c - zi.lo())*get_swell_fac());
          		b_lo = sv_box(xi, yi, i_part);
			i_part = sv_interval(*c - (zi.hi() - *c)*get_swell_fac(), zi.hi());
			b_hi = sv_box(xi, yi, i_part);
			break;

	case LEAF_M:
	default:
		svlis_error("facet_decision", "dud *k value", SV_CORRUPT);
		return;
	}

	*c_1 = sv_model(child_sl, b_lo, m);
	*c_2 = sv_model(child_sl, b_hi, m);

	return;
}

//************************************************************************************

// Output a faceted model in VRML
// Dan Pidcock and Adrian Bowyer

// Write out a polygon

void polygon_to_vrml(ostream& os, sv_p_gon* pg, const sv_surface& surf, const sv_set& s)
{
	sv_p_gon* n = pg;

	os << "Shape" << SV_EL;
	os << "{" << SV_EL;
        os << " geometry IndexedFaceSet" << SV_EL;
	os << " {" << SV_EL;

	sv_integer c = p_gon_vertex_count(pg);
	os << "  coordIndex  [";
	for(int i = 0; i < c; i++) os << i << ", ";
	os << " -1]" << SV_EL;

        os << "  coord Coordinate { point [";
       	do
       	{
		os << n->p.x << ' ' << n->p.y << ' ' << n->p.z;
               	n = n->next;
		if (n != pg) os << ", ";
       	}while (n != pg);
	os << "] }" << SV_EL;

	n = pg;
        os << "  normal Normal { vector [";
       	do
       	{
		os << n->g.x << ' ' << n->g.y << ' ' << n->g.z;
               	n = n->next;
		if (n != pg) os << ", ";
       	}while (n != pg);
	os << "] }" << SV_EL;
	os << " }" << SV_EL;

	sv_point col = s.colour();
	sv_real t = surf.transmission();
        os << " appearance Appearance { material Material { diffuseColor ";
	os << col.x << ' ' << col.y << ' ' << col.z;
	if (t > 0.001) // Hack
	    os << " transparency " << t;
	os << " } }" << SV_EL;

	os << "}" << SV_EL;
}

// Write out a polyline

void polyline_to_vrml(ostream& os, sv_p_gon* pg, const sv_surface& surf, const sv_set& s)
{
	sv_p_gon* n = pg;

	os << "Shape" << SV_EL;
	os << "{" << SV_EL;
        os << " geometry IndexedLineSet" << SV_EL;
	os << " {" << SV_EL;

	sv_integer c = p_gon_vertex_count(pg);
	os << "  coordIndex  [";
	for(int i = 0; i < c; i++) os << i << ", ";
	os << " -1]" << SV_EL;

        os << "  coord Coordinate { point [";
       	do
       	{
		os << n->p.x << ' ' << n->p.y << ' ' << n->p.z;
               	n = n->next;
		if (n != pg) os << ", ";
       	}while (n != pg);
	os << "] }" << SV_EL;

	os << " }" << SV_EL;

	sv_point col = s.colour();
        os << " appearance Appearance { material Material { emissiveColor ";
	os << col.x << ' ' << col.y << ' ' << col.z;
	os << " } }" << SV_EL;

	os << "}" << SV_EL;
}

// Write out a point set

void points_to_vrml(ostream& os, sv_p_gon* pg, const sv_surface& surf, const sv_set& s)
{
	sv_p_gon* n = pg;

	os << "Shape" << SV_EL;
	os << "{" << SV_EL;
        os << " geometry PointSet" << SV_EL;
	os << " {" << SV_EL;

        os << "  coord Coordinate { point [";
       	do
       	{
		os << n->p.x << ' ' << n->p.y << ' ' << n->p.z;
               	n = n->next;
		if (n != pg) os << ", ";
       	}while (n != pg);
	os << "] }" << SV_EL;
	os << " }" << SV_EL;
	
	sv_point col = s.colour();
        os << " appearance Appearance { material Material { emissiveColor ";
	os << col.x << ' ' << col.y << ' ' << col.z;
	os << " } }" << SV_EL;



	os << "}" << SV_EL;
}

// Write out the polygons of a set

void set_to_vrml(ostream& os, const sv_set& s)
{
        sv_p_gon* pg;
	sv_surface surf;
	sv_attribute a;
	sv_user_attribute* u;

// Children (if any) may also have polygon attributes

	if (s.contents() > 1)
	{
		set_to_vrml(os, s.child_1());
		set_to_vrml(os, s.child_2());
	}

	surf = s.surface();

// Loop through the attributes plotting all polygon ones

	a = s.attribute();
	while(a.exists())
	{
		if( a.tag_val() == -pg->tag() )
		{
              		u = a.user_attribute();
                	pg = (sv_p_gon*)u->pointer;
			switch(pg->kind)
			{
			case PT_SET:
				points_to_vrml(os, pg, surf, s);
				break;
			case P_LINE:
				polyline_to_vrml(os, pg, surf, s);
				break;
			case P_GON:
				polygon_to_vrml(os, pg, surf, s);
				break;
			default:
				svlis_error("set_to_vrml","dud polygon kind",SV_CORRUPT);
			}
		}
		a = a.next();
	}
}


// Recursively walk a model plotting its polygons

void model_to_vrml(ostream& os, const sv_model& m)
{
	sv_set_list pgl;
	
	if (m.has_polygons())
	{
	    pgl = m.set_list();
	    while (pgl.exists())
	    {
		set_to_vrml(os, pgl.set());
		pgl = pgl.next();
	    }
	}

	if (m.kind() == LEAF_M) return;

	model_to_vrml(os, m.child_1());
	model_to_vrml(os, m.child_2());
}


// Write a faceted model to a VRML file

void sv_to_vrml(ostream& os, const sv_model& m)
{
        time_t l_time;
        time(&l_time);
	sv_box b = m.box();
	sv_real d = sqrt(b.diag_sq());
        sv_point c = b.centroid() + SV_Z*d;

	os << "#VRML V2.0 utf8" << SV_EL << SV_EL;
	os << "WorldInfo" << SV_EL;
	os << "{" << SV_EL;
	os << " info" << SV_EL;
	os << " [" << SV_EL;
	os << "  \"Created by svLis version " << get_svlis_version() << " - ";
        os << "see  http://www.bath.ac.uk/~ensab/G_mod/Svlis/ \"" << SV_EL;
	os << " ]" << SV_EL;
	os << " title \"svLis\"" << SV_EL;
	os << "}" << SV_EL;
	os << "Transform" << SV_EL;
        os << "{" << SV_EL;
	os << "children" << SV_EL;
        os << "[" << SV_EL;
	os << "NavigationInfo { headlight TRUE type \"EXAMINE\"}" << SV_EL;
	os << "Viewpoint { orientation 0 0 0  0  position 0 0 10  description \"Front\" }" << SV_EL;
	os << "Background" << SV_EL;
	os << "{" << SV_EL;
	os << "groundColor  [ 0.3 0.2 0.1 ]" << SV_EL;
	os << "skyColor  [ 0.6 0.7 1.0 ]" << SV_EL;
	os << "}" << SV_EL;
	os << "Transform" << SV_EL; 
        os << "{" << SV_EL;
	os << "translation " << -c.x << ' ' << -c.y << ' ' << -c.z << SV_EL;
	os << "children" << SV_EL;
        os << "[" << SV_EL;
	os << "Transform" << SV_EL;
	os << "{" << SV_EL;
	os << " translation " << (-d/10) << ' ' << (-(b.yi.hi() - b.yi.lo())/2 - d*0.1) << ' ' << 0 << SV_EL;
	os << "children" << SV_EL;
        os << "[" << SV_EL;
	os << "Shape" << SV_EL;
	os << "{" << SV_EL;
	os << " geometry Text" << SV_EL;
	os << " {" << SV_EL;
 	os << "  string [\"svLis\"]" << SV_EL;
	os << "  length [ " << d/5 << " ]" << SV_EL;
	os << " }" << SV_EL;
	//os << " script" << SV_EL;
	//os << " {" << SV_EL;
	//os << "  eventIn SFBool start" << SV_EL;
	//os << "  url" << SV_EL;
	//os << "  [" << SV_EL;
	//os << "   \"http://www.bath.ac.uk/~ensab/G_mod/Svlis/\"" << SV_EL;
	//os << "  ]" << SV_EL;
	//os << " }" << SV_EL;
	os << "}" << SV_EL;
	os << "]" << SV_EL;
	os << "}" << SV_EL;

	model_to_vrml(os, m);

	os << "]" << SV_EL;
	os << "}" << SV_EL;
	os << "]" << SV_EL;
	os << "}" << SV_EL;
}

// Walk a model returning all its facets as a single attribute
// Use with care

static sv_attribute all_f;

void get_set_p_gons(const sv_set& s)
{
        sv_p_gon* pg;
        sv_p_gon pt;
	sv_attribute a;
	sv_user_attribute* u;
	sv_integer t;

// Children (if any) may also have polygon attributes

	if (s.contents() > 1)
	{
		get_set_p_gons(s.child_1());
		get_set_p_gons(s.child_2());
	}

// Loop through the attributes appending all polygon ones

	a = s.attribute();
	while(a.exists())
	{
		if( (t = a.tag_val()) == -pt.tag() )
		{
              		u = a.user_attribute();
			all_f = merge(all_f, sv_attribute(t, u));
			copy_user(u, t);
		}
		a = a.next();
	}
}

// Get polygons attached to all sets in a list

void get_set_list_p_gons(sv_set_list pgl)
{
	sv_set s;

        while ( pgl.exists() )
        {
                s = pgl.set();
		get_set_p_gons(s);
		pgl = pgl.next();
	}
}

void gafr(const sv_model& m)
{
        if (m.has_polygons())
                   get_set_list_p_gons(m.set_list());

	if ( m.kind() != LEAF_M )
	{
		gafr(m.child_1());
		gafr(m.child_2());
	}
}

sv_attribute get_all_facets(const sv_model& m)
{
	sv_attribute null;
	all_f = null;
	gafr(m);
	return(all_f);
}
#if macintosh
 #pragma export off
#endif

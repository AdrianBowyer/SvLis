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
 * SvLis set procedures
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 4 March 1993
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
#if macintosh
 #pragma export on
#endif


// Flags a flat primitive as convex

sv_integer sv_c_flag(const sv_primitive& p)
{
  // Plane is convex

	if (p.kind() == SV_PLANE) return(SV_CV_POL);

  // So is a scaled plane

	if (p.kind() == SV_GENERAL)
	{
	  if(p.op() == SV_TIMES)
	  {
	    if((p.child_1().kind() == SV_REAL) && (p.child_2().kind() == SV_PLANE)) return(SV_CV_POL);
	    if((p.child_2().kind() == SV_REAL) && (p.child_1().kind() == SV_PLANE)) return(SV_CV_POL);
	  }
	}
	return(0);
}

// Unique tag

sv_integer sv_set::tag() const { return(SVT_F*SVT_SET); }

// Normal user i/o functions

ostream& operator<<(ostream& s, sv_set& a)
{
	unwrite(a);
	write_svlis_header(s);
	write(s, a, 0);
	s.flush();
	return(s);
}

istream& operator>>(istream& s, sv_set& a)
{
	sv_clear_input_tables();
	check_svlis_header(s);
	read(s, a);
	return(s);
}

// Set some flag bits - needs to be here as FLAG_MASK is defined in private.h

void sv_set::set_flags(sv_integer f) { set_info->set_flags(f & FLAG_MASK); }
void sv_set::reset_flags(sv_integer f) { set_info->reset_flags(f & FLAG_MASK); }

// Set constructors for when the set is a line or a point

sv_set::sv_set(const sv_line& l)
{
//  This returns the intersection of two mutually perpendicular
//  planes that forms the line.
    
	sv_point ax = l.direction;
	sv_point cent = l.origin;

//   Generate a vector perpendicular to the line.

	sv_point srad0 = right(ax);

//   And another perpendicular to both

	sv_point srad1 = srad0^ax;

//  Generate two perpendicular planes intersecting in the axis

	sv_set hs0 = sv_set(abs(sv_primitive(sv_plane(srad0, cent))));
	sv_set hs1 = sv_set(abs(sv_primitive(sv_plane(srad1, cent))));
	*this = hs0 & hs1;
}

sv_set::sv_set(const sv_point& p)
{
	sv_set hs0 = sv_set(abs(sv_primitive(sv_plane(SV_X, p))));
	sv_set hs1 = sv_set(abs(sv_primitive(sv_plane(SV_Y, p))));
	sv_set hs2 = sv_set(abs(sv_primitive(sv_plane(SV_Z, p))));
	*this = hs0 & hs1 & hs2;
}

// Attach an attribute to a set; this overwrites any existing attributes

sv_set sv_set::attribute(const sv_attribute& aa) const
{
	sv_set p = *this;
	p.a = aa;
	return(p);
}

// I/O

// This ought to be a self-organizing list

look_up<sv_set> s_write_list;

void clean_set_lookup()
{
	s_write_list.clean();
}

// Flag a set as not written.

void unwrite(sv_set& a)
{
	sv_set s_temp;
	sv_primitive p_temp;
	sv_attribute a_temp;

	a.reset_flags_priv(WRIT_BIT);
	if(a.primitive().exists())
	{
		p_temp = a.primitive();
		unwrite(p_temp);
	}
	if(a.child_1().exists())
	{	
		s_temp = a.child_1();
		unwrite(s_temp);
		s_temp = a.child_2();
		unwrite(s_temp);
	}
	if(a.complement().exists())
	{	
		s_temp = a.complement();
		if(s_temp.flags() & WRIT_BIT)
			unwrite(s_temp);
	}
	if(a.has_attribute())
	{
		a_temp = a.attribute();
		unwrite(a_temp);
	}
}

// Write a set operator

void write(ostream& s, const set_op& o, sv_integer level)
{
	put_white(s, level);

	switch(o)
	{
	case SV_UNION: s << '|'; break;
	case SV_INTERSECTION: s << '&';  break;

	default:
		svlis_error("write(.. set_op","dud value",SV_CORRUPT);
		break;
	}
}

// Write a set to a stream.  

void write(ostream& s, const sv_set& aa, sv_integer level)
{
	sv_set a = aa;
	long s_ptr = a.unique();
	sv_primitive p_temp;
	sv_set s_temp;
	sv_attribute a_temp;
	sv_integer nxl = level+1;

	put_white(s, level);
	put_token(s, SVT_SET, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB_S, 0, 0);
	s << SV_EL;
	writei(s, s_ptr, nxl);
	s << ' ';

	if (!(a.flags() & WRIT_BIT))
	{
		writei(s, a.contents(), 0); s << ' ';
		writei(s, a.flags(), 0); s << ' ';
		a.set_flags_priv(WRIT_BIT);
		switch(a.contents())
		{
		case SV_EVERYTHING:
		case SV_NOTHING: 
			s << SV_EL;
			break;

		case 1:	p_temp = a.primitive();
			s << SV_EL;
			write(s, p_temp, nxl);
			break;

		default:
			write(s, a.op(), 0);
			s << SV_EL;
			s_temp = a.child_1();
			write(s, s_temp, nxl);
			s_temp = a.child_2();
			write(s, s_temp, nxl);
		}
		if (a.has_attribute())
		{
			writei(s, 1, nxl); s << SV_EL;
			a_temp = a.attribute();
			write(s, a_temp, nxl);
		} else
		{
			writei(s, 0, nxl);
			s << SV_EL;
		}
		if (a.complement().exists())
		{
			s_temp = a.complement();
			if(!(s_temp.flags() & WRIT_BIT))
			{
				writei(s, 1, nxl); s << SV_EL;
				write(s, s_temp, nxl);
			} else
			{
				writei(s, 0, nxl);
				s << SV_EL;
			}
		} else
		{
			writei(s, 0, nxl);
			s << SV_EL;
		}
	}
	put_white(s, level);
	put_token(s, SVT_CB_S, 0, 0);
	s << SV_EL;
}

// Return the degree of the highest-degree primitive in 
// all the sets

sv_integer sv_set::max_degree() const
{
	switch(contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		return(0);
	case 1:
		return(primitive().degree());
	default:
		return(max(child_1().max_degree(), 
			child_2().max_degree()));
	}
	return(0);
}

// Are two sets the same? (ignores attributes)

// Possible answers:

// SV_ZERO - No
// SV_PLUS - Yes
// SV_COMP - One is the complement of the other
// SV_ABS  - One is the absolute value of the other

prim_op same(const sv_set& a, const sv_set& b)
{
	prim_op r1, r2;
	int flip = 0;
	
	if (a == b) return(SV_PLUS);  // Simple things first...

	if(a.complement().exists())
		if(a.complement() == b) return(SV_COMP); 
	
	switch(a.contents())
	{
	case SV_EVERYTHING:
		if (b.contents() == SV_EVERYTHING) return(SV_PLUS);
	    if (b.contents() == SV_NOTHING) return(SV_COMP);
	    return(SV_ZERO);

	case SV_NOTHING:
		if (b.contents() == SV_NOTHING) return(SV_PLUS);
	    if (b.contents() == SV_EVERYTHING) return(SV_COMP);
	    return(SV_ZERO);
	    	    
	case 1:
		if (b.contents() != 1) return(SV_ZERO);
		return(same(a.primitive(), b.primitive()));
	
	default:
		if (b.contents() != a.contents()) return(SV_ZERO);
		r1 = same(a.child_1(), b.child_1());
		if (!r1)
		{
			r1 = same(a.child_1(), b.child_2());
			flip = 1;
		}
		if (!r1) return(SV_ZERO);
		
		if(flip)
			r2 = same(a.child_2(), b.child_1());
		else
			r2 = same(a.child_2(), b.child_2());			
		if (!r2) return(SV_ZERO);
		
		if(a.op() == SV_UNION)
		{
			if(b.op() == SV_UNION)
			{
				if ((r1 == SV_PLUS) && (r2 == SV_PLUS)) return(SV_PLUS);
				return(SV_ZERO);
			} else
			{
				if ((r1 == SV_COMP) && (r2 == SV_COMP)) return(SV_COMP);
				return(SV_ZERO);
			}						
		} else
		{
			if(b.op() == SV_UNION)
			{
				if ((r1 == SV_COMP) && (r2 == SV_COMP)) return(SV_COMP);
				return(SV_ZERO);
			} else
			{
				if ((r1 == SV_PLUS) && (r2 == SV_PLUS)) return(SV_PLUS);
				return(SV_ZERO);
			}			
		}		
	}

// Should never get here

	return(SV_ZERO);
}

// Deep copy

sv_set sv_set::deep() const
{
	sv_set b;

	switch (contents())
	{
	case SV_EVERYTHING:
		b = sv_set(SV_EVERYTHING);
		break;

	case SV_NOTHING:
		b = sv_set(SV_NOTHING);
		break;

	case 1:
		b = sv_set(primitive().deep());
		break;

	default:
		if (op() == SV_UNION)	
			b = child_1().deep() | child_2().deep();
		else
			b = child_1().deep() & child_2().deep();
		break;
	}

	return(b.attribute(a.deep()));
}

// Translate a set

sv_set operator+(const sv_set& a, const sv_point& p)
{
	sv_set b;

	switch (a.contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		b = a;
		break;

	case 1:
		b = sv_set(a.primitive() + p);
		break;

	default:
		if (a.op() == SV_UNION)	
			b = (a.child_1() + p) | (a.child_2() + p);
		else
			b = (a.child_1() + p) & (a.child_2() + p);
		break;
	}

	return(att_slide(b, a, p));
}

// Rotate a set about a line

sv_set sv_set::spin(const sv_line& l, sv_real angle) const
{
	sv_set b;

	switch (contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		b = *this;
		break;

	case 1:
		b = sv_set(primitive().spin(l, angle));
		break;

	default:
		if (op() == SV_UNION)
			b = child_1().spin(l,angle) | child_2().spin(l,angle);
		else
			b = child_1().spin(l,angle) & child_2().spin(l,angle);
		break;
	}

	return(att_spin(b, *this, l, angle));
}

// Scale a set about a point

sv_set sv_set::scale(const sv_point& p, sv_real scl) const
{
	sv_set b;

	switch (contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		b = *this;
		break;

	case 1:
		b = sv_set(primitive().scale(p, scl));
		break;

	default:
		if (op() == SV_UNION)
			b = child_1().scale(p,scl) | child_2().scale(p,scl);
		else
			b = child_1().scale(p,scl) & child_2().scale(p,scl);
		break;
	}

	return(att_scale(b, *this, p, scl));
}

// Scale a set along a line

sv_set sv_set::scale(const sv_line& s_ax, sv_real scl) const
{
	sv_set b;

	switch (contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		b = *this;
		break;

	case 1:
		b = sv_set(primitive().scale(s_ax, scl));
		break;

	default:
		if (op() == SV_UNION)
			b = child_1().scale(s_ax,scl) | child_2().scale(s_ax,scl);
		else
			b = child_1().scale(s_ax,scl) & child_2().scale(s_ax,scl);
		break;
	}

	return(att_scale(b, *this, s_ax, scl));
}

// Mirror a set in a plane

sv_set sv_set::mirror(const sv_plane& m) const
{
	sv_set b;

	switch (contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
		b = *this;
		break;

	case 1:
		b = sv_set(primitive().mirror(m));
		break;

	default:
		if (op() == SV_UNION)
			b = child_1().mirror(m) | child_2().mirror(m);
		else
			b = child_1().mirror(m) & child_2().mirror(m);
		break;
	}

	return(att_mirror(b, *this, m));
}

// Complement a set.  If this has been done already, the result is
// in a.complement(); if not it needs to be computed.

sv_set operator-(const sv_set& aa)
{
	sv_set a = aa;

	if (a.complement().exists()) return (a.complement());    // NB: does not call
								 // att_complement

	sv_set b;

	switch (a.contents())
	{
	case SV_EVERYTHING:
		b = sv_set(SV_NOTHING);
		break;

	case SV_NOTHING:	
		b = sv_set(SV_EVERYTHING);
		break;

	case 1:
		b = sv_set(-a.primitive());
		break;

	default:                            // De Morgan's law
		if (a.op() == SV_UNION)
			b = (-a.child_1()) & (-a.child_2());
		else
			b = (-a.child_1()) | (-a.child_2());
		break;
	}

// Remember the results so it doesn't have to be done again

	b = att_complement(b, a);
	a.complement(b);
	b.complement(a);

	return(b);
}

// Set union
// Keep the simplest set at the first child whenever possible

int quickor(const sv_set& a, const sv_set& b, sv_set* c)
{

// Quick test for simple cases

	int result = 0;
	if (a == b)
	{
		*c = a;
		result = 1;
	} else
	{
		if (a.complement().exists())
			if (a.complement() == b) 
			{
				*c = sv_set(SV_EVERYTHING);
				result = 1;
			}
	}
	return(result);
}

sv_set operator|(const sv_set& a, const sv_set& b)
{
	sv_set c;
	
	if ( !quickor(a, b, &c) )
	{
		switch (a.contents())
		{
		case SV_EVERYTHING:
			c = sv_set(SV_EVERYTHING);
			break;

		case SV_NOTHING:	
			c = b; 
			break;

		case 1:
			switch (b.contents())
			{
			case SV_EVERYTHING:
				c = sv_set(SV_EVERYTHING); 
				break;
			case SV_NOTHING:
				c = a;
				break;	 
			case 1:
				if (a.primitive().degree() > b.primitive().degree())
					c = sv_set(b,a,SV_UNION);
				else
					c = sv_set(a,b,SV_UNION);
				break;
			default:
				c = sv_set(a,b,SV_UNION);
				break;
			}
			break;

		default:	 
			switch (b.contents())
			{
			case SV_EVERYTHING:
				c = sv_set(SV_EVERYTHING); 
				break;
			case SV_NOTHING:
				c = a;
				break;	 
			case 1:
				c = sv_set(b,a,SV_UNION);
				break;
			default:
				if (a.contents() > b.contents())
					c = sv_set(b,a,SV_UNION);
				else
					c = sv_set(a,b,SV_UNION);
				break;
			}
			break;
		}
	}
	return(att_union(c, a, b));
}

// Set intersection
// Again, keep the simplest set as child 1 whenever possible

int quickand(const sv_set& a, const sv_set& b, sv_set* c)
{

// Quick test for simple cases

	int result = 0;
	if (a == b)
	{
		*c = a;
		result = 1;
	} else
	{
		if (a.complement().exists())
			if (a.complement() == b) 
			{
				*c = sv_set(SV_NOTHING);
				result = 1;
			}
	}
	return(result);
}

sv_set operator&(const sv_set& a, const sv_set& b)
{
	sv_set c;

	
	if ( !quickand(a,b,&c) )
    {
		switch (a.contents())
		{
		case SV_EVERYTHING:
			c = b;
			break;
		case SV_NOTHING:	
			c = sv_set(SV_NOTHING); 
			break;
		case 1:
			switch (b.contents())
			{
			case SV_EVERYTHING:
				c = a; 
				break;
			case SV_NOTHING:
				c = sv_set(SV_NOTHING);
				break;
			case 1:
				if (a.primitive().degree() > b.primitive().degree())
					c = sv_set(b,a,SV_INTERSECTION);
				else
					c = sv_set(a,b,SV_INTERSECTION);
				break;
			default:
				c = sv_set(a,b,SV_INTERSECTION);
				break;
			}
			break;

		default:	 
			switch (b.contents())
			{
			case SV_EVERYTHING:
				c = a; 
				break;
			case SV_NOTHING:
				c = sv_set(SV_NOTHING);
				break;	 
			case 1:
				c = sv_set(b,a,SV_INTERSECTION);
				break;
			default:
				if (a.contents() > b.contents())
					c = sv_set(b,a,SV_INTERSECTION);
				else
					c = sv_set(a,b,SV_INTERSECTION);
				break;
			}
			break;
		}
	}

	if( (a.flags() & SV_CV_POL) && (b.flags() & SV_CV_POL) ) c.set_flags_priv(SV_CV_POL);

	return(att_intersection(c, a, b));
}

// Code to sort out three sets (a x (b y c)) being regularized where x and
// y are & or |.

sv_set three_reg(const sv_set& a, const sv_set& b, const sv_set& c,
	prim_op sab, sv_integer ops)
{
	sv_set result;

	switch(ops)
	{
	case 0:
		switch(sab)
		{
		case SV_PLUS: result = a & c; break;
		case SV_COMP: result = sv_set(SV_NOTHING); break;
		case SV_ABS: 
			if(b.primitive().op() == SV_ABS)
				result = b & c;
			else
				result = a & c;
			break;
		default:
			svlis_error("three_reg", "dud sab0",
				SV_CORRUPT);
		}
		break;
	case 1:
		switch(sab)
		{
		case SV_PLUS: result = a; break;
		case SV_COMP: result = a & c; break;
		case SV_ABS: 
			if(b.primitive().op() == SV_ABS)
				result = b;
			else
				result = a;
			break;
		default:
			svlis_error("three_reg", "dud sab1",
				SV_CORRUPT);
		}
		break;
	case 2:
		switch(sab)
		{
		case SV_PLUS: result = a; break;
		case SV_COMP: result = a | c; break;
		case SV_ABS: 
			if(b.primitive().op() == SV_ABS)
				result = a;
			else
				result = a | (b & c);
			break;
		default:
			svlis_error("three_reg", "dud sab2",
				SV_CORRUPT);
		}
		break;
	case 3:
		switch(sab)
		{
		case SV_PLUS: result = a | c; break;
		case SV_COMP: result = sv_set(SV_EVERYTHING); break;
		case SV_ABS: 
			if(b.primitive().op() == SV_ABS)
				result = a | c;
			else
				result = b | c;
			break;
		default:
			svlis_error("three_reg", "dud sab3",
				SV_CORRUPT);
		}
		break;
	default:
		svlis_error("three_reg", "dud | op",SV_CORRUPT);
	}
	return(result);
}

// This member function regularizes sets with a contents
// of 4 or less using the same(...) function

sv_set sv_set::regularize() const
{
	sv_set result = *this;
	sv_set a, b, bc, c;
	sv_integer ops = 0;
	prim_op sab, sac;

	switch (contents())
	{
	case SV_EVERYTHING:
	case SV_NOTHING:
	case 1: return(*this);

	case 2:
		a = child_1();
		b = child_2();
		switch(same(a,b))
		{
		case SV_PLUS:
			result = a;
			break;
		case SV_COMP:
			if(op() == SV_UNION)
				result = sv_set(SV_EVERYTHING);
			else
				result = sv_set(SV_NOTHING);
			break;
		case SV_ABS:
			// Make ab the thin one, and b the thick
			if(a.primitive().op() == SV_ABS)
				bc = a;
			else
			{
				bc = b;
				b = a;
			}
			if(op() == SV_UNION)
				result = b;
			else
				result = bc;
			break;
		case SV_ZERO:
			return(*this);
		default:
			svlis_error("sv_set::regularize", "dud prim_op",
				SV_CORRUPT);
		}
		result = result.attribute(this->attribute());
		return(result);

	case 3:
		a = child_1();
		bc = child_2().regularize();
		if(bc.contents() < 2)
		{
			if(op() == SV_UNION)
				result = (a | bc).regularize();
			else
				result = (a & bc).regularize();
			result = result.attribute(this->attribute());
			return(result);
		}

// From here on we know b and c are different

		b = bc.child_1();
		c = bc.child_2();
		if(op() == SV_UNION) ops = ops | 2;
		if(bc.op() == SV_UNION) ops = ops | 1;

		if(sab = same(a,b)) // I mean = not ==
		{
		   result = three_reg(a,b,c,sab,ops);
		   result = result.attribute(this->attribute());
		   return(result);
		}

		if(sac = same(a,c)) // I mean = not ==
		{
		   result = three_reg(a,c,b,sac,ops);
		   result = result.attribute(this->attribute());
		   return(result);
		}
		break;

// The following is not optimal, but it's safe and simple
// Could do this for contents=N checking for result.contents() < N?

	case 4:
	case 5:
		if(op() == SV_UNION)
			result = child_1().regularize() |
				child_2().regularize();
		else
			result = child_1().regularize() &
				child_2().regularize();
		if(result.contents() < contents())
			result = result.regularize();
		else
			return(*this);
		result = result.attribute(this->attribute());
		return(result);

	default:
		break;
	}

	return(*this);	
}

// This membership-tests a point against a set
// When points are on surfaces, the result is regularized.
// If the point is known to lie on the surfaces of some primitives,
// they should be listed in known_surface[]; the list is terminated
// with an undefined primitive.  If no such primitives exist, just
// send a pointer to an undefined primitive for the third argument.

mem_test sv_set::member(const sv_point& p, sv_primitive known_surface[]) const
{
	mem_test result_1 = SV_AIR;
	mem_test result_2 = SV_AIR;
	sv_integer i = 0;
	sv_real work;

	switch (contents())
	{
	case SV_EVERYTHING:
		return(SV_SOLID);

	case SV_NOTHING:
		return(SV_AIR);

	case 1:
		while(known_surface[i].exists())
			if (primitive() == known_surface[i++]) return(SV_SURFACE);
//		result_1 = member(primitive().value(p));
// That won't go with Microsoft Visual C++, so time for a workround...
		work = primitive().value(p);
		if (work > 0)
			result_1 = SV_AIR;
		else
		{
			if (work < 0) 
				result_1 = SV_SOLID;
			else
				result_1 = SV_SURFACE;
		}
		return(result_1);
	
	default:

// child_1 should have a lower complexity than child_2

		result_1 = child_1().member(p,known_surface);

		if (op() == SV_UNION)
		{
			if (result_1 == SV_SOLID) return(SV_SOLID);
			result_2 = child_2().member(p,known_surface);
			if (result_2 == SV_SOLID) return(SV_SOLID);
			if ((result_1 == SV_SURFACE)||(result_2 == SV_SURFACE))
				return(SV_SURFACE);
			else
				return(SV_AIR);
		} else
		{
			if (result_1 == SV_AIR) return(SV_AIR);
			result_2 = child_2().member(p,known_surface);
			if (result_2 == SV_AIR) return(SV_AIR);
			if ((result_1 == SV_SURFACE)||(result_2 == SV_SURFACE))
				return(SV_SURFACE);
			else
				return(SV_SOLID);
		}
		break;
	}

// Should never get to here.

	svlis_error("sv_set::member(sv_point...)","returning AIR",SV_CORRUPT);

	return(result_1);   
}

// Value for a point (and winning leaf)

sv_real sv_set::value(const sv_point& p, sv_set* winner) const
{
	sv_real result;
	sv_real result_2;
	sv_set w_2;

	switch (contents())
	{
	case SV_EVERYTHING:
		*winner = *this;
		return(-1.0);

	case SV_NOTHING:
		*winner = *this;
		return(1.0);

	case 1:
		*winner = *this;
		return(primitive().value(p));
	
	default:

		result = child_1().value(p, winner);
		result_2 = child_2().value(p, &w_2);

		if (op() == SV_UNION)	// == minimum
		{
			if (result > result_2)
			{
				*winner = w_2;
				result = result_2;
			}
		} else
		{	// INTERSECTION == maximum
			if (result < result_2)
			{
				*winner = w_2;
				result = result_2;
			}
		}
		if ( has_attribute() && ( !(winner->has_attribute()) ) )
			*winner = winner->attribute(attribute());
		return(result);
	}
}

// Range for a box (and winning leaves)

sv_interval sv_set::range(const sv_box& b, sv_set* w_lo, sv_set* w_hi) const
{
	sv_interval result_2;
	sv_interval result;
	sv_set w_2_lo, w_2_hi;

	switch (contents())
	{
	case SV_EVERYTHING:
		*w_lo = *this;
		*w_hi = *this;
		return(sv_interval(-2.0,-1.0)); // By convention (what does this do to the min/max?)

	case SV_NOTHING:
		*w_lo = *this;
		*w_hi = *this;
		return(sv_interval(1.0,2.0));

	case 1:
		*w_lo = *this;
		*w_hi = *this;
		return(primitive().range(b));
	
	default:

		result = child_1().range(b, w_lo, w_hi);
		result_2 = child_2().range(b ,&w_2_lo, &w_2_hi);

		if (op() == SV_UNION)	// U == minimum
		{
			if (result_2.lo() < result.lo())
			{
				*w_lo = w_2_lo;
				result = sv_interval(result_2.lo(), result.hi());
			}
			if (result_2.hi() < result.hi())
			{
				*w_hi = w_2_hi;
				result = sv_interval(result.lo(), result_2.hi());
			}
		} else
		{	// INTERSECTION == maximum
			if (result_2.lo() > result.lo())
			{
				*w_lo = w_2_lo;
				result = sv_interval(result_2.lo(), result.hi());
			}
			if (result_2.hi() > result.hi())
			{
				*w_hi = w_2_hi;
				result = sv_interval(result.lo(), result_2.hi());
			}
		}
		if (has_attribute() && ( !w_lo->has_attribute() ))
			*w_lo = w_lo->attribute(attribute());
		if (has_attribute() && ( !w_hi->has_attribute() ))
			*w_hi = w_hi->attribute(attribute());
		return(result);
	}
}

// Flag for whether the results of pruning are
// regularized

static sv_integer reg_prune = 0;

void regular_prune(sv_integer p) { reg_prune = p; }

// This prunes a set to a box

sv_set sv_set::prune(const sv_box& b) const
{
	sv_set pruned, temp;
	mem_test m = SV_AIR;
	int c_1_same;

	switch (contents())
	{
        case SV_EVERYTHING:
	case SV_NOTHING:
		pruned = *this;
		break;

        case 1:
		m = primitive().range(b).member();
		switch (m)
		{
		case SV_AIR:
			pruned = sv_set(SV_NOTHING);
			break;
		case SV_SURFACE:
			pruned = *this;
			break;
		case SV_SOLID:
			pruned = sv_set(SV_EVERYTHING);
			break;
		default:
			svlis_error("sv_set::prune(sv_box)", "dud mem test", SV_CORRUPT);
		}
		break;
					
	default:
		pruned = child_1().prune(b);
		c_1_same = ( pruned == child_1() );
		if (op() == SV_UNION)
		{
			switch (pruned.contents())
			{
			case SV_EVERYTHING:
				break;
			case SV_NOTHING:
				pruned = child_2().prune(b);
				break;
			case 1:
			default:
				temp = child_2().prune(b);
				if (c_1_same && (temp == child_2()))
					pruned = *this;
				else
					pruned = pruned | temp;
				break;
			}
		} else
		{
			switch (pruned.contents())
			{
			case SV_EVERYTHING:
				pruned = child_2().prune(b);
				break;
			case SV_NOTHING:
				break;
			case 1:
			default:
				temp = child_2().prune(b);
				if (c_1_same && (temp == child_2()))
					pruned = *this;
				else
					pruned = pruned & temp;
				break;
			}
		}
		break;
	}

	if(reg_prune) pruned = pruned.regularize();
	return(att_prune(pruned, *this, b));
}

// Polygons as attributes

sv_integer sv_set::polygon_count() const
{

	sv_integer c1 = 0, c2;
	sv_attribute a;
	sv_p_gon* pg = 0;

	if (contents() > 1)
	{
		c1 = child_1().polygon_count();
		c2 = child_2().polygon_count();
		c1 = c1 + c2;
	}

	a = attribute();
	while(a.exists())
	{
		if( a.tag_val() == -pg->tag() ) c1++;
		a = a.next();
	}

	return(c1);
}


// A tetrahedron with vertices at four points v[0...3]

sv_set tetrahedron(sv_point v[])
{
	sv_set result = sv_set(SV_EVERYTHING);
	sv_point p[3], q[3];
	int count;
	sv_real vol;
	sv_plane flat;
	for (int i = 0; i < 4; i++)
	{
		count = 0;
		for(int j = 0; j < 4; j++)
		{
			if(j != i)
			{
				p[count] = v[j] - v[i];
				q[count] = v[j];
				count++;
			}
		}
   		vol = p[0].x*(p[1].y*p[2].z - p[2].y*p[1].z) -
			p[1].x*(p[0].y*p[2].z - p[2].y*p[0].z) +
			p[2].x*(p[0].y*p[1].z - p[1].y*p[0].z);
		if(vol < 0)
		    flat = sv_plane(q[0], q[2], q[1]);
		else
		    flat = sv_plane(q[0], q[1], q[2]);
		result = result & sv_set(sv_primitive(flat));
	}
	return(result);
}

// sv_set
// ***************************************************************************

// Set lists

// Unique tag

sv_integer sv_set_list::tag() const { return(SVT_F*SVT_SET_LIST); }

// Normal user i/o functions - set lists

ostream& operator<<(ostream& s, sv_set_list& sl)
{
	unwrite(sl);
	write_svlis_header(s);
	write(s, sl, 0);
	s.flush();
	return(s);
}

istream& operator>>(istream& s, sv_set_list& sl)
{
	sv_clear_input_tables();
	check_svlis_header(s);
	read(s, sl);
	return(s);
}

// Set flags

void sv_set_list::set_flags(sv_integer f) { set_list_info->set_flags(f & FLAG_MASK); }
void sv_set_list::reset_flags(sv_integer f) { set_list_info->reset_flags(f & FLAG_MASK); }


// Set list I/O

look_up<sv_set_list> sl_write_list;

void clean_sl_lookup()
{
	sl_write_list.clean();
}

// Set a set list unwritten

void unwrite(sv_set_list& s)
{
	sv_set_list x;
	sv_set ss;
	s.reset_flags_priv(WRIT_BIT);
	ss = s.set();;
	unwrite(ss);
	if( (x = s.next()).exists() ) unwrite(x);
}

// Write a set list list to a stream

void write(ostream& s, const sv_set_list& sl, sv_integer level)
{
	sv_set_list n = sl;
	long sl_ptr;
	short carry_on = 1;
	sv_integer nxl = level+1;

	while(n.exists() && carry_on)
	{
		put_white(s, level);
		put_token(s, SVT_SET_LIST, 0, 0);
		s << SV_EL;
		put_white(s, level);
		put_token(s, SVT_OB_L, 0, 0);
		s << SV_EL;

		carry_on = !(n.flags() & WRIT_BIT);
		sl_ptr = n.unique();
		writei(s, sl_ptr, nxl);
		if(carry_on)
		{
			s << ' ';
			writei(s, n.flags(), 0); s << SV_EL;
			write(s, n.set(), nxl);
			n.set_flags_priv(WRIT_BIT);
		} else
			s << SV_EL;
		put_white(s, level);
		put_token(s, SVT_CB_L, 0, 0);
		s << SV_EL;
		n = n.next();
	}
	put_white(s, level);
	put_token(s, SVT_SET_LIST, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB_L, 0, 0);
	s << SV_EL;
	writei(s, 0, nxl);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_CB_L, 0, 0);
	s << SV_EL;
}

// Return the degree of the highest-degree primitive in 
// all the sets

sv_integer sv_set_list::max_degree() const
{
	sv_set_list local = *this;
	sv_integer result = 0;
	sv_integer r;

	while(local.exists())
	{
		if((r = set().max_degree()) > result)
			result = r;
		local = local.next();
	}
	return(result);
}



// return the number of sets

sv_integer sv_set_list::count() const
{
	sv_set_list local = *this;
	sv_integer total = 0;

	while(local.exists())
	{
		total++;
		local = local.next();
	}
	return(total);
}

// The total contents of a set list

sv_integer sv_set_list::contents() const
{
	sv_set_list n = *this;
	sv_integer total = 0;

	while(n.exists())
	{
		total = total + max(0, n.set().contents());
		n = n.next();
	}
	return(total);
}

// Two set lists are equal if their sets are the same in 
// any order (or if they point to the same thing). 

int operator==(const sv_set_list& a, const sv_set_list& b)
{
	int result = (a.unique() == b.unique());
	if (result) return(result);

	if (a.count() != b.count()) return(0);

	sv_set_list na = a;
	sv_set_list nb;
	result = 1;

	while(result && na.exists())
	{
		nb = b;
		result = 0;
		while(nb.exists() && !result)
		{
			result = (na.set() == nb.set());
			nb = nb.next();
		}
		na = na.next();
	}

	return(result);
}

// Deep copy

sv_set_list sv_set_list::deep() const
{
	sv_set_list temp1 = *this;
	sv_set_list temp2;

	while(temp1.exists())
	{
		temp2 = sv_set_list((temp1.set()).deep(), temp2);
		temp1 = temp1.next();
	}
	return(temp2);
}

// Operators on collections of sets... 

sv_set_list merge(const sv_set_list& a, const sv_set_list& b)
{
	sv_set_list temp1;
	sv_set_list temp2;
	sv_set_list temp3;
	int add;

// Build a deep copy of a (but not a deep copy of a's sets)
	
	temp1 = a;
	while(temp1.exists())
	{
		temp2 = sv_set_list(temp1.set(), temp2);
		temp1 = temp1.next();
	}

// Add anything in b that's not already in a

	temp1 = b;
	while(temp1.exists())
	{
		add = 1;
		temp3 = a;
		while(temp3.exists() && add)
		{
			if(temp1.set() == temp3.set()) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_set_list(temp1.set(), temp2);

		temp1 = temp1.next();
	}

	return(temp2);
}

sv_set_list overlap(const sv_set_list& a, const sv_set_list& b)
{
	sv_set_list temp1;
	sv_set_list temp2;
	sv_set_list temp3;
	int add;

// Add anything in b that's also in a

	temp1 = b;
	while(temp1.exists())
	{
		add = 0;
		temp3 = a;
		while(temp3.exists() && (!add))
		{
			if( temp1.set() == temp3.set() ) add = 1;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_set_list(temp1.set(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

sv_set_list remove(const sv_set_list& a, const sv_set_list& b)
{
	sv_set_list temp1;
	sv_set_list temp2;
	sv_set_list temp3;
	int add;

// Remove anything in a that's also in b

	temp1 = a;
	while(temp1.exists())
	{
		add = 1;
		temp3 = b;
		while(temp3.exists() && add)
		{
			if( temp1.set() == temp3.set() ) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_set_list(temp1.set(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

sv_set_list ex_or(const sv_set_list& a, const sv_set_list& b)
{
	sv_set_list temp1;
	sv_set_list temp2;
	sv_set_list temp3;
	sv_integer add;

// Remove anything in a that's also in b

	temp1 = a;
	while(temp1.exists())
	{
		add = 1;
		temp3 = b;
		while(temp3.exists() && add)
		{
			if( temp1.set() == temp3.set() ) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_set_list(temp1.set(), temp2);
		temp1 = temp1.next();
	}

// Remove anything in b that's also in a

	temp1 = b;
	while(temp1.exists())
	{
		add = 1;
		temp3 = a;
		while(temp3.exists() && add)
		{
			if( temp1.set() == temp3.set() ) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_set_list(temp1.set(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

// Two set lists the same?

prim_op same(const sv_set_list& a, const sv_set_list& b)
{
	if (a == b) return(SV_PLUS);
	prim_op result = SV_ZERO;

	if (a.count() != b.count()) return(SV_ZERO);

	sv_set_list na = a;
	sv_set_list nb;
	result = SV_PLUS;

	while((result == SV_PLUS) && na.exists())
	{
		nb = b;
		result = SV_ZERO;
		while(nb.exists() && !result)
		{
			result = same(na.set(), nb.set());
			nb = nb.next();
		}
		na = na.next();
	}
	
	return(result);
}


// Create a new set list that is a copy of an old one, with the sets
// each pruned to a box

sv_set_list sv_set_list::prune(const sv_box& b) const
{
	sv_set_list result, n;
	
	if (!exists())
	{
		svlis_error("sv_set_list::prune(sv_box)","attempt to prune undefined set list",
				SV_WARNING);
		return(result);
	}

	n = next();

	if(n.exists())
		result = merge(n.prune(b), set().prune(b));
	else
		result = sv_set_list(set().prune(b));

	return(result);
}

// Return all the elements of a set list as a union or intersection

sv_set sv_set_list::unite() const
{
    sv_set_list slt = next();
    sv_set st = set();
    
    while(slt.exists()) 
    {
    	    st = st | slt.set();
	    slt = slt.next();
    }

    return(st);
}

sv_set sv_set_list::intersect() const
{
    sv_set_list slt = next();
    sv_set st = set();
    
    while(slt.exists()) 
    {
    	    st = st & slt.set();
	    slt = slt.next();
    }

    return(st);
}
#if macintosh
 #pragma export off
#endif

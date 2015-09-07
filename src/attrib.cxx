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
 * Attribute functions
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
 * This version: 8 March 2000
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

// Deep copy

sv_attribute sv_attribute::deep() const
{	
	sv_attribute temp1 = *this;
	sv_attribute temp2;
	sv_user_attribute* ua;
	sv_integer t;

	while(temp1.exists())
	{
		ua = temp1.user_attribute();
		t = temp1.tag_val();
		temp2 = sv_attribute(t, ua, temp2);
		copy_user(ua, t);
		temp1 = temp1.next();
	}
	return(temp2);
}

sv_attribute merge(const sv_attribute& a, const sv_attribute& b)
{
	sv_attribute temp1;
	sv_attribute temp2;
	sv_attribute temp3;
	int add;

// Build a deepish copy of a
	
	temp1 = a;
	while(temp1.exists())
	{
		temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
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
			if(a_eq(temp1, temp3)) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);

		temp1 = temp1.next();
	}

	return(temp2);
}

sv_attribute overlap(const sv_attribute& a, const sv_attribute& b)
{
	sv_attribute temp1;
	sv_attribute temp2;
	sv_attribute temp3;
	int add;

// Add anything in b that's also in a

	temp1 = b;
	while(temp1.exists())
	{
		add = 0;
		temp3 = a;
		while(temp3.exists() && (!add))
		{
			if( a_eq(temp1, temp3) ) add = 1;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

sv_attribute remove(const sv_attribute& a, const sv_attribute& b)
{
	sv_attribute temp1;
	sv_attribute temp2;
	sv_attribute temp3;
	int add;

// Remove anything in a that's also in b

	temp1 = a;
	while(temp1.exists())
	{
		add = 1;
		temp3 = b;
		while(temp3.exists() && add)
		{
			if( a_eq(temp1, temp3) ) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

sv_attribute ex_or(const sv_attribute& a, const sv_attribute& b)
{
	sv_attribute temp1;
	sv_attribute temp2;
	sv_attribute temp3;
	sv_integer add;

// Remove anything in a that's also in b

	temp1 = a;
	while(temp1.exists())
	{
		add = 1;
		temp3 = b;
		while(temp3.exists() && add)
		{
			if( a_eq(temp1, temp3) ) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
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
			if( a_eq(temp1, temp3) ) add = 0;
			temp3 = temp3.next();
		}
		if(add) temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

// Select a given tag value

sv_attribute overlap(const sv_attribute& a, sv_integer ts)
{
	sv_attribute temp1 = a;
	sv_attribute temp2;

// Add anything in a that has tag value ts

	while(temp1.exists())
	{
		if(temp1.tag_val() == ts)
			temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

// Remove a given tag value

sv_attribute remove(const sv_attribute& a, sv_integer ts)
{
	sv_attribute temp1 = a;
	sv_attribute temp2;

// Remove anything in a that's got tag value ts

	while(temp1.exists())
	{
		if(temp1.tag_val() != ts)
			temp2 = sv_attribute(temp1.tag_val(), temp1.user_attribute(), temp2);
		temp1 = temp1.next();
	}

	return(temp2);
}

// Attribute equality

int equals2(const sv_attribute& a, const sv_attribute& b)
{
	int result = 1;
	sv_attribute ta = a;
	sv_attribute tb;

// Counts must be the same
	
	if (a.count() != b.count()) return(0);

// Search for each element in a in b

	while(result && ta.exists())
	{
		tb = b;
		result = 0;
		while(tb.exists() && !result)
		{
		   result = a_eq(ta, tb);
		   tb = tb.next();
		}
		ta = ta.next();
	}

	return(result);
}

// Number of attributes in a collection

sv_integer sv_attribute::count() const
{
	sv_integer result = 0;
	sv_attribute t = *this;
	while(t.exists())
	{
		result++;
		t = t.next();
	}
	return(result);
}


// Unique tag

sv_integer sv_attribute::tag() const { return(SVT_F*SVT_ATTRIBUTE); }

// Flag bit manipulators

void sv_attribute::set_flags(sv_integer f) { attribute_info->set_flags(f & FLAG_MASK); }
void sv_attribute::reset_flags(sv_integer f) { attribute_info->reset_flags(f & FLAG_MASK); }


// I/O

look_up<sv_attribute> a_write_list;

void clean_attribute_lookup()
{
	a_write_list.clean();
}

void unwrite(sv_attribute& at)
{
	sv_attribute x;
	at.reset_flags_priv(WRIT_BIT);
	if( (x = at.next()).exists() ) unwrite(x);
}

// Write an attribute linked list to a stream

void write(ostream& s, const sv_attribute& at, sv_integer level)
{
	sv_attribute n = at;
	long a_ptr;
	short carry_on = 1;
	sv_integer nxl = level+1;

	while(n.exists() && carry_on)
	{
		put_white(s, level);
		put_token(s, SVT_ATTRIBUTE, 0, 0);
		s << SV_EL;
		put_white(s, level);
		put_token(s, SVT_OB_P, 0, 0);
		s << SV_EL;

		carry_on = !(n.flags() & WRIT_BIT);
		a_ptr = n.unique();
		writei(s, a_ptr, nxl);
		if(carry_on)
		{
			s << ' ';
			writei(s, n.tag_val(), 0); s << SV_EL;
			write(s, n.user_attribute(), n.tag_val(), nxl);
			writei(s, n.flags(), nxl); s << SV_EL;
			n.set_flags_priv(WRIT_BIT);
		} else
			s << SV_EL;
		put_white(s, level);
		put_token(s, SVT_CB_P, 0, 0);
		s << SV_EL;
		n = n.next();
	}
	put_white(s, level);
	put_token(s, SVT_ATTRIBUTE, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB_P, 0, 0);
	s << SV_EL;
	writei(s, 0, nxl);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_CB_P, 0, 0);
	s << SV_EL;
}

// Normal user i/o functions

ostream& operator<<(ostream& s, sv_attribute& at)
{
	unwrite(at);
	write_svlis_header(s);
	write(s, at, 0);
	return(s);
}

istream& operator>>(istream& s, sv_attribute& at)
{
	sv_clear_input_tables();
	check_svlis_header(s);
	read(s, at);
	return(s);
}

// Remove all attributes from a set

sv_set sv_set::strip_attributes() const
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
		b = sv_set(primitive());
		break;

	default:
		if (op() == SV_UNION)	
			b = child_1().strip_attributes() | child_2().strip_attributes();
		else
			b = child_1().strip_attributes() & child_2().strip_attributes();
		break;
	}

	return(b);
}

// Search a set tree for the attribute of a given node

sv_attribute ar_search(const sv_attribute& current, const sv_set& tree, const sv_set& node)
{
	sv_attribute a_here, result;

	if (tree.has_attribute()) 
		a_here = tree.attribute();
	else
		a_here = current;

	if (tree == node) return(a_here);

	if (tree.contents() > 1)
	{
		if( (result = ar_search(a_here, tree.child_1(), node)).exists() ) return(result);
		if( (result = ar_search(a_here, tree.child_2(), node)).exists() ) return(result);
	}
	return(result);
}

sv_attribute sv_set::inherit(const sv_set& node) const
{
	if (node.has_attribute()) return(node.attribute());

	sv_attribute current;

	return(ar_search(current, *this, node));
}

// percolate the attributes down a set tree

sv_set percolate_r(const sv_set& s, const sv_attribute& a)
{
	sv_attribute current;
	sv_set result = s;

	if (s.has_attribute())
	{
		if(s.contents() <= 1) return(result);
		current = s.attribute();
	} else
		current = a;

	if (s.contents() > 1)
	{
		if (s.op() == SV_UNION)
			result = percolate_r(s.child_1(), current) |
				percolate_r(s.child_2(), current);
		else
			result = percolate_r(s.child_1(), current) &
				percolate_r(s.child_2(), current);
	}
	result = result.attribute(current);

	return(result);
}

sv_set sv_set::percolate() const
{
	sv_attribute current;

	if (has_attribute())
		current = attribute();
	else
		svlis_error("sv_set::percolate", "set has no root attribute", 
			SV_COMMENT);

	return(percolate_r(*this, current));
}

sv_set_list sv_set_list::percolate() const
{
	sv_set_list result;
	sv_set_list sl = *this;
	sv_set s;

	while(sl.exists())
	{
		s = sl.set().percolate();
		result = merge(result, s);
		sl = sl.next();
	}

	return(result);
}
#if macintosh
 #pragma export off
#endif

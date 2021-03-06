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
 * SvLis - This defines the model classes that svLis uses.
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
#if macintosh
 #pragma export on
#endif

// Unique tag

sv_integer sv_model::tag() const { return(SVT_F*SVT_MODEL); }

// I/O

// Normal user i/o functions

ostream& operator<<(ostream& s, sv_model& m)
{
	unwrite(m);
	write_svlis_header(s);
	write(s, m, 0);
	s.flush();
	return(s);
}

istream& operator>>(istream& s, sv_model& m)
{
	sv_clear_input_tables();
	check_svlis_header(s);
	read(s, m);
	return(s);
}

// Set some flag bits

// Set some flag bits - needs to be here as FLAG_MASK is defined in private.h

void sv_model::set_flags(sv_integer f) { model_info->set_flags(f & FLAG_MASK); }
void sv_model::reset_flags(sv_integer f) { model_info->reset_flags(f & FLAG_MASK); }


// I/O

void write(ostream& s, const mod_kind& k, sv_integer level)
{
	put_white(s, level);
	switch(k)
	{
	case LEAF_M: s << 'L'; break;
	case X_DIV: s << 'X'; break;
	case Y_DIV: s << 'Y'; break;
	case Z_DIV: s << 'Z'; break;

	default:
		svlis_error("write(.. mod_kind)","dud value",SV_CORRUPT);
		break;
	}
}

look_up<sv_model> m_write_list;

void clean_model_lookup()
{
	m_write_list.clean();
}

// Flag a model as not written.

void unwrite(sv_model& a)
{
	sv_model m_temp;
	sv_set_list sl;

	a.reset_flags_priv(WRIT_BIT);
	if(a.set_list().exists())
	{
		sl = a.set_list();
		unwrite(sl);
	}
	if(a.child_1().exists())
	{	
		m_temp = a.child_1();
		unwrite(m_temp);
		m_temp = a.child_2();
		unwrite(m_temp);
	}
	if(a.parent().exists())
	{	
		m_temp = a.parent();
		unwrite(m_temp);
	}
}

// Write a model to a stream.

void write(ostream& s, sv_model& a, sv_integer level)
{
	long m_ptr = a.unique();
	sv_set_list sl;
	sv_model m_temp;
	sv_integer nxl = level+1;

	put_white(s, level);
	put_token(s, SVT_MODEL, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB_M, 0, 0);
	s << SV_EL;
	writei(s, m_ptr, nxl);

	if (!(a.flags() & WRIT_BIT))
	{
		s << ' ';
		write(s, a.kind(), 0); s << ' ';
		writei(s, a.flags(), 0); s << SV_EL;
		a.set_flags_priv(WRIT_BIT);
		write(s, a.box(), nxl); s << SV_EL;
		sl = a.set_list();
		write(s, sl, nxl);
		if(a.parent().exists())
		{
			writei(s, 1, nxl); s << SV_EL;
			m_temp = a.parent();
			write(s, m_temp, nxl);
		} else
		{
			writei(s, 0, nxl); s << SV_EL;
		}
		switch(a.kind())
		{
		case LEAF_M: break;

		case X_DIV:
		case Y_DIV:
		case Z_DIV:
			writer(s, a.coord(), nxl); s << SV_EL;
			m_temp = a.child_1();
			write(s, m_temp, nxl);
			m_temp = a.child_2();
			write(s, m_temp, nxl);
			break;

		default:
			svlis_error("write(model)", "dud kind", SV_CORRUPT);
		}
	} else
		s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_CB_M, 0, 0);
	s << SV_EL;
}

// Are two models the same?  (ignores parent and children)

prim_op same(const sv_model& a, const sv_model& b)
{
	prim_op sls = same(a.set_list(), b.set_list());
	if(!sls) return(SV_ZERO);
	prim_op bs = same(a.box(), b.box());
	if((bs == SV_PLUS)) return(sls);
	return(SV_ZERO);
}

// Divide a model with a user-supplied decision procedure.
// This is the actual recursive procedure, which is called by redivide below.
// The child boxes are grown slightly in the division direction by swell_fac
// as a fudge to ensure things don't fall down the gaps.  If the model
// has already been divided, sub-models that would be unaffected by the
// new division are detected and left as they were.

static sv_real swell_fac = DEF_SWELL_FAC;

void set_swell_fac(sv_real sf) {swell_fac = sf;}
sv_real get_swell_fac() {return(swell_fac);}

void redivide_r(void* vsdd)
{
	sv_div_data *sdd = (sv_div_data*) vsdd;
	sv_set_list s = sdd->set_list();
	sv_model m = sv_model(sdd->model().parent(), s, sdd->model().box(), sdd->model().child_1(),
		sdd->model().child_2(), sdd->model().kind(), sdd->model().coord(), sdd->model().flags()); 
	sv_integer level = sdd->level();
	void* vp = sdd->pointer();
	sv_model result;
	sv_model c_1;			// The two children that may be created by decision
	sv_model c_2;
	mod_kind k;			// The result from decision
	sv_real cut;			// The result from decision
	sv_box b = m.box();
	sv_interval x = b.xi;		// The parent box intervals
	sv_interval y = b.yi;
	sv_interval z = b.zi;
	sv_interval i_part;		// The lower and upper halves of the divided interval
	sv_box b_part;			// The sub-boxes
	sv_model nul;			// Get rid of unwanted sub-trees by assigning this

	sv_decision decis = sdd->decision();
	(*decis) (m, level, vp, &k, &cut, &c_1, &c_2);

	switch (k)
	{ 
	case LEAF_M:
		if (c_1.exists())  // User done the work?
		{
			sdd->result(c_1);
			return;
		}
		result = sv_model(m.set_list(), m.box(), m.parent());
		sdd->result(result);
		return;

	case X_DIV:
		if ( !c_1.exists() )
		{
			i_part = sv_interval(x.lo(), cut + (cut - x.lo())*swell_fac);
          		b_part = sv_box(i_part, y, z);
			c_1 = sv_model(s, b_part, m);
		}
		if ( !c_2.exists() )
		{
			i_part = sv_interval(cut - (x.hi() - cut)*swell_fac, x.hi());
			b_part = sv_box(i_part, y, z);
			c_2 = sv_model(s, b_part, m);
		}
		break;

	  case Y_DIV:
		if ( !c_1.exists() )
		{
			i_part = sv_interval(y.lo(), cut + (cut - y.lo())*swell_fac);
          		b_part = sv_box(x, i_part, z);
			c_1 = sv_model(s, b_part, m);
		}
		if ( !c_2.exists() )
		{
			i_part = sv_interval(cut - (y.hi() - cut)*swell_fac, y.hi());
			b_part = sv_box(x, i_part, z);
			c_2 = sv_model(s, b_part, m);
		}
		break;

	  case Z_DIV:
		if ( !c_1.exists() )
		{
			i_part = sv_interval(z.lo(), cut + (cut - z.lo())*swell_fac);
          		b_part = sv_box(x, y, i_part);
			c_1 = sv_model(s, b_part, m);
		}
		if ( !c_2.exists() )
		{
			i_part = sv_interval(cut - (z.hi() - cut)*swell_fac, z.hi());
			b_part = sv_box(x, y, i_part);
			c_2 = sv_model(s, b_part, m);
		}
		break;

	  default:
	  	svlis_error("redivide_r", "dud model kind", SV_CORRUPT);
	}


	level++;

// Create two threads on a parallel machine to divide the two halves of the model

#ifdef SV_PARALLEL

	sv_model d_1 = c_1.deep();
	sv_model d_2 = c_2.deep();
	sv_div_data sd1 = sv_div_data(d_1, d_1.set_list(), level, vp, decis);
	sv_div_data sd2 = sv_div_data(d_2, d_2.set_list(), level, vp, decis);

	pthread_t th_1;
	pthread_t th_2;
	pthread_attr_t attr;
	sv_integer t1 = 1;	// Go to 0 if thread 1/2 runs
	sv_integer t2 = 1;	
	sv_integer get_c1 = 0;  // Go to 1 if we need a new c_1/2
	sv_integer get_c2 = 0;
	void* rtval;

//	if(t1 = pthread_attr_setstacksize(&attr, (size_t) 1000000))
//		svlis_error("redivide_r","can't change stack size",SV_WARNING);

	if (t1 = pthread_attr_init(&attr))
              svlis_error("redivide_r","can't initialise thread attribute",
		SV_WARNING);

	if (m.kind() == LEAF_M)
	{
		get_c1 = 1;
		get_c2 = 1;
		if(t1 = pthread_create(&th_1, &attr, (void *(*)(void *))redivide_r, (void *)&sd1))
			redivide_r((void*) &sd1);

		if(t2 = pthread_create(&th_2, &attr, (void *(*)(void *))redivide_r, (void *)&sd2))
			redivide_r((void*) &sd2);
	} else
	{

// If the model already has children, check if they're the same as those
// found and, if so, don't bother to replace them.
  
		if (m.child_1() != c_1)
		{
			get_c1 = 1;
			if(t1 = pthread_create(&th_1, &attr, (void *(*)(void *))redivide_r, (void *)&sd1))
				redivide_r((void*) &sd1);			
		}

		if (m.child_2() != c_2)
		{
			get_c2 = 1;
			if(t2 = pthread_create(&th_2, &attr, (void *(*)(void *))redivide_r, (void *)&sd2))
				redivide_r((void*) &sd2);	
		}

	}

// Wait for t1 and/or t2 to come back

	if(!t1)
	  if(pthread_join(th_1, &rtval))
		svlis_error("redivide_r","can't join thread 1",SV_WARNING);
	if(!t2)
	  if(pthread_join(th_2, &rtval))
		svlis_error("redivide_r","can't join thread 2",SV_WARNING);

// Get the computed sub-models if they are needed

	if(get_c1) c_1 = sd1.result();
	if(get_c2) c_2 = sd2.result();

#else

// Ordinary von Neuman architecture

	sv_div_data sd1 = sv_div_data(c_1, c_1.set_list(), level, vp, decis);
	sv_div_data sd2 = sv_div_data(c_2, c_2.set_list(), level, vp, decis);

	if (m.kind() == LEAF_M)
	{
		redivide_r((void*)&sd1);
		redivide_r((void*)&sd2);
		c_1 = sd1.result();
		c_2 = sd2.result();
	} else
	{
		if (m.child_1() != c_1)
		{
			redivide_r((void*)&sd1);
			c_1 = sd1.result();
		}

		if (m.child_2() != c_2)
		{
			redivide_r((void*)&sd2);
			c_2 = sd2.result();
		}
	}

#endif

	sv_model mcc = sv_model(m, c_1, c_2, k, cut);
	mcc.set_flags_priv(m.flags());
	sdd->result( mcc );
}

// Storage for the root model

static sv_model r_m;

sv_model root_model() { return(r_m); }

// Initialize recursive division 

sv_model sv_model::redivide(const sv_set_list& s, void* vp, sv_decision decision ) const
{
	r_m = *this;
	sv_model nul;
	sv_div_data sdd = sv_div_data(*this, s, 0, vp, decision);
	sv_model result;
	redivide_r((void*)&sdd);
	result = sdd.result();
	r_m = nul;	// Easy way to force system to junk storage for *this.
				// If it goes away later, r_m won't be left pointing
				// to it.
	return(result);
}

int sv_model::has_polygons() const 
{
	return((flags() & SV_POLYGON_FLAG) != 0);
}


// Make a deep copy of a _leaf_ model
// NB the parent is not deep copied.

sv_model sv_model::deep() const
{
	sv_model result;
	
	switch(kind())
	{
	case X_DIV:
	case Y_DIV:
	case Z_DIV:
		svlis_error("sv_model::deep()","deep copy of non-leaf model",SV_WARNING);

// For model membership tests, the lists are unioned.
	
	case LEAF_M:
		return(sv_model(set_list().deep(), box(), LEAF_M, parent()));
		break;			
	
	default:
		svlis_error("sv_model::deep()","dud model kind",SV_CORRUPT);
		break;
	}
	return(result);
}

// Membership test a point agains a model

mem_test sv_model::member(const sv_point& p, sv_primitive* ks) const
{

	sv_set_list sl;
	mem_test result = SV_AIR;
	mem_test temp;
	
	if (box().member(p) == SV_AIR) return(SV_AIR);
	
	switch(kind())
	{
	case X_DIV:
		if (p.x < coord())
			return(child_1().member(p, ks));
		else
			return(child_2().member(p, ks));
	case Y_DIV:
		if (p.y < coord())
			return(child_1().member(p, ks));
		else
			return(child_2().member(p, ks));
	case Z_DIV:
		if (p.z < coord())
			return(child_1().member(p, ks));
		else
			return(child_2().member(p, ks));

// For model membership tests, the lists are unioned.
	
	case LEAF_M:
		sl = set_list();
		while(sl.exists())
		{
			temp = sl.set().member(p, ks);
			if(temp == SV_SURFACE) result = SV_SURFACE;
			if(temp == SV_SOLID) return(SV_SOLID);
			sl = sl.next();
		}
		break;			
	
	default:
		svlis_error("sv_model::member(...)","dud model kind",SV_CORRUPT);
		break;
	}
	return(result);
}

// Return the leaf containing a point

sv_model sv_model::leaf(const sv_point& p) const
{
	sv_model nothing;

	switch(kind())
	{
	case X_DIV:
		if(p.x < coord())
			return(child_1().leaf(p));
		else
			return(child_2().leaf(p));

	case Y_DIV:
		if(p.y < coord())
			return(child_1().leaf(p));
		else
			return(child_2().leaf(p));
	
	case Z_DIV:
		if(p.z < coord())
			return(child_1().leaf(p));
		else
			return(child_2().leaf(p));


	case LEAF_M:
		if(box().member(p) != SV_AIR)
			return(*this);
		else
			return(nothing);

	default:
		svlis_error("sv_model::leaf", "dud model kind", SV_CORRUPT);
	}

	return(nothing);
}

// Gather model statistics

void m_stats::model_stats(const sv_model& m, m_stats* ms)
{
	sv_set_list sl;
	sv_set s;
	sv_integer i, j, sol;
	sv_box b = m.box();
	sv_real x = b.xi.hi() - b.xi.lo();
	sv_real y = b.yi.hi() - b.yi.lo();
	sv_real z = b.zi.hi() - b.zi.lo();

	if (b.diag_sq() < ms->littlest.diag_sq()) ms->littlest = b;
	ms->total_boxes++;


	i = m.polygon_count();
	if(i)
	{
		if(!m.has_polygons())
		{
			svlis_error("m_stats::model_stats(...)","model polygon flag un-set",SV_WARNING);
			// m.set_flags_priv(SV_POLYGON_FLAG);
		}
		ms->pg_count = ms->pg_count + (sv_real)i;
		if (i > ms->max_pg_count) ms->max_pg_count = i;
		ms->pgl_x = ms->pgl_x + x;
		ms->pgl_y = ms->pgl_y + y;
		ms->pgl_z = ms->pgl_z + z;
		ms->pgl_boxes++;
		ms->pgl_volume = ms->pgl_volume + b.vol();
	}

	switch(m.kind())
	{
	case LEAF_M:	sl = m.set_list();

			// Anything there other than NOTHING or EVERYTHING?

			j = 0;
                        sol = 0;
			while(sl.exists())
			{
				s = sl.set();
				if ((i = s.contents()) > 0) j = j + i;
				if (i == SV_EVERYTHING) sol = 1;       // Set lists unioned
				sl = sl.next();
			}

			if(!j || sol)
			{
			      if(sol)
			      {
				ms->s_volume = ms->s_volume + b.vol();
				ms->s_boxes++;
				ms->s_x = ms->s_x + x;
				ms->s_y = ms->s_y + y;
				ms->s_z = ms->s_z + z;
			      } else
			      {
				ms->a_volume = ms->a_volume + b.vol();
				ms->a_boxes++;
				ms->a_x = ms->a_x + x;
				ms->a_y = ms->a_y + y;
				ms->a_z = ms->a_z + z;
			      }
			} else
			{
				ms->surface_volume = ms->surface_volume + b.vol();
				ms->surface_boxes++;
				ms->surface_x = ms->surface_x + x;
				ms->surface_y = ms->surface_y + y;
				ms->surface_z = ms->surface_z + z;
				ms->surface_contents = ms->surface_contents + 
					(sv_real)j;
				if (j > ms->max_surface_contents) 
					ms->max_surface_contents = j;
			}

			break;

	case X_DIV:
	case Y_DIV:
	case Z_DIV:
			m_stats::model_stats(m.child_1(), ms);
			m_stats::model_stats(m.child_2(), ms);
			break;

	default:
			svlis_error("model_stats", "dud model kind", SV_CORRUPT);
			break;
	}
}

// Compute averages after stats have been gathered

void m_stats::model_stats_av(m_stats* ms)
{
	sv_real ct;

	if (ms->a_boxes)
	{
		ct = 1/(sv_real)ms->a_boxes;
		ms->a_x = ms->a_x*ct;
		ms->a_y = ms->a_y*ct;
		ms->a_z = ms->a_z*ct;
	}
	if(ms->s_boxes)
	{
		ct = 1/(sv_real)ms->s_boxes;
		ms->s_x = ms->s_x*ct;
		ms->s_y = ms->s_y*ct;
		ms->s_z = ms->s_z*ct;
	}

	if (ms->surface_boxes)
	{
		ct = 1/(sv_real)ms->surface_boxes;
		ms->surface_x = ms->surface_x*ct;
		ms->surface_y = ms->surface_y*ct;
		ms->surface_z = ms->surface_z*ct;
		ms->surface_contents = ms->surface_contents*ct;
	}

	if (ms->pgl_boxes)
	{
		ct = 1/(sv_real)ms->pgl_boxes;
		ms->pgl_x = ms->pgl_x*ct;
		ms->pgl_y = ms->pgl_y*ct;
		ms->pgl_z = ms->pgl_z*ct;
		// NB polygon total is not averaged here
	}

}


// Print a stats report to a stream

void sv_model::div_stat_report(ostream& f) const
{
	m_stats* ms = new m_stats(*this);

	sv_box b = box();
	sv_real x = b.xi.hi() - b.xi.lo();
	sv_real y = b.yi.hi() - b.yi.lo();
	sv_real z = b.zi.hi() - b.zi.lo();
	sv_real vol = b.vol();
	sv_integer cts = set_list().contents();

	f << SV_EL << "SvLis division statistics" << SV_EL << SV_EL;
	f << "  Note: percentages will total more than 100 if box swell is > 0." << SV_EL << SV_EL;
	f << "  The model consists of " << cts << " primitives." << SV_EL;
	f << "  Its x, y, and z lengths are: [" << x << ", " << y << ", " << z << "] " <<
		"and its total volume is " << vol << "." << SV_EL;
	f << "  The model contains " << ms->total_boxes << " boxes in all." << SV_EL << SV_EL;

	f << "  There are " << ms->a_boxes << " air leaf boxes." << SV_EL;
	if (ms->a_boxes)
	{
		f << "    The volume of air leaf boxes is " << ms->a_volume <<
			" (which is " << 100.0*ms->a_volume/vol << "% of the total)." << SV_EL;
		f << "    Their average x, y, and z lengths are: [" <<
 			ms->a_x << ", " << ms->a_y << ", " << ms->a_z << "]." << SV_EL << SV_EL;
	}
 	f << "  There are " << ms->s_boxes << " solid leaf boxes." << SV_EL;
        if(ms->s_boxes)
	{
		f << "    The volume of solid leaf boxes is " << ms->s_volume <<
			" (which is " << 100.0*ms->s_volume/vol << "% of the total)." << SV_EL;
		f << "    Their average x, y, and z lengths are: [" <<
 			ms->s_x << ", " << ms->s_y << ", " << ms->s_z << "]." << SV_EL << SV_EL;
	} 
        if(!ms->s_boxes && ! ms->a_boxes) f << SV_EL;
	
	f << "  The smallest box is: [" << ms->littlest.xi.lo() << ',' << ms->littlest.xi.hi() << "],";
	f << " [" << ms->littlest.yi.lo() << ',' << ms->littlest.yi.hi() << "],";
    f << " [" << ms->littlest.zi.lo() << ',' << ms->littlest.zi.hi() << "]" << SV_EL << SV_EL;
    
	f << "  There are " << ms->surface_boxes << " leaf boxes containing surfaces." << SV_EL;
	if (ms->surface_boxes)
	{
		f << "    The volume of surface-containing boxes is " << 
				ms->surface_volume <<
				" (which is " << 100.0*ms->surface_volume/vol << 
				"% of the total)." << SV_EL;
		f << "    Their average x, y, and z lengths are: [" <<
 			ms->surface_x << ", " << ms->surface_y << ", " << 
				ms->surface_z << "]." << SV_EL;
		f << "    The average number of primitives in each surface-containing box is " <<
			ms->surface_contents << SV_EL;
		f << "    The maximum number of primitives in a box is " <<
			ms->max_surface_contents << SV_EL << SV_EL;
	} else
		f << SV_EL;

	f << "  There are " << ms->pgl_boxes << " boxes in the tree containing polygons." << SV_EL;
	if (ms->pgl_boxes)
	{
		f << "    The volume of polygon-containing boxes is " << 
				ms->pgl_volume <<
				" (which is " << 100.0*ms->pgl_volume/vol << 
				"% of the total)." << SV_EL;
		f << "    Their average x, y, and z lengths are: [" <<
 			ms->pgl_x << ", " << ms->pgl_y << ", " << ms->pgl_z << "]." << SV_EL;
		f << "    The total number of polygons is " <<
			ms->pg_count << ", which averages " <<
			(sv_real)ms->pg_count/(sv_real)ms->pgl_boxes << " per polygon-containing box." << SV_EL;
		f << "    The maximum number of polygons in a box is " <<
			ms->max_pg_count << SV_EL;
	}

	f << SV_EL << SV_EL;

	f.flush();

	delete ms;
}
#if macintosh
 #pragma export off
#endif










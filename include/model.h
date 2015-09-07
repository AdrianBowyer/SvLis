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
 * First version: 8 March 1993
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_MODEL
#define SVLIS_MODEL

// The form of svLis decision procedures

typedef void (*sv_decision) (const sv_model&, sv_integer, void*,  mod_kind*, 
		    sv_real*, sv_model*, sv_model*);

// The argument of redivide_r (q.v.)
		    
class sv_div_data;

// As usual models are handles pointing to a hidden class, with reference
// counting storage de-allocation

extern istream& operator>>(istream&, mod_kind&);

// the facet decision procedure

extern void facet_decision(const sv_model&, sv_integer, void*, mod_kind*, 
    sv_real*, sv_model*, sv_model*);

// The svLis model class

class sv_model
{
private:

   struct model_data : public sv_refct 
   {
        friend class sv_smart_ptr<model_data>;

        sv_model* child_1;	// Sub-models underneath
	sv_model* child_2;
	
	sv_model* p;		// The parent (if any)

	sv_box b;		// Box round this model
	sv_set_list sl;		// Its contents

	mod_kind kind;		// Leaf, or divided in X, Y, or Z
	sv_real coord;		// The division coordinate
	
        ~model_data() { delete child_1; delete child_2; delete p; }

// Constructor to build a leaf model

	model_data(const sv_set_list& sls, const sv_box& bx, const sv_model& pt)
	{
	        b = bx;
		sl = sls;
		kind = LEAF_M;
		coord = 0;
	        child_1 = new sv_model();
	        child_2 = new sv_model();
	        p = new sv_model(pt);
	}

// Constructor to copy everything

	model_data(const sv_model& pt, const sv_set_list& sls, const sv_box& bx, const sv_model& c1, const sv_model& c2, 
		    mod_kind k, sv_real c, sv_integer ff)
	{
	        b = bx;
		sl = sls;
		kind = k;
		coord = c;
	        child_1 = new sv_model(c1);
	        child_2 = new sv_model(c2);
	        p = new sv_model(pt);
		set_flags(ff);
	}

// Constructor to build a non-leaf model

	model_data (const sv_model& pt, const sv_model& c1, const sv_model& c2, mod_kind k, sv_real c)
	{
	        b = pt.box();
		sl = pt.set_list();
		kind = k;
		coord = c;
	        child_1 = new sv_model(c1);
	        child_2 = new sv_model(c2);
	        p = new sv_model(pt);
	}

   }; // model_data

// This is the pointer that gets ref counted

   sv_smart_ptr<model_data> model_info;

// Priveleged (Re)Set flag bit(s)

        void set_flags_priv(sv_integer a) { model_info->set_flags(a); }
	void reset_flags_priv(sv_integer a) { model_info->reset_flags(a); }

public:

// Constructor for null model

	sv_model() { }

// Safe constructor to make a model from a set and a box; the set is 
// pruned to the box.

	sv_model(const sv_set_list& sls, const sv_box& bx, const sv_model& pt) { model_info = new model_data(sls.prune(bx), bx, pt); }

// Equivalent for no parent

	sv_model(const sv_set_list& sls, const sv_box& bx) { model_info = new model_data(sls.prune(bx), bx, sv_model()); }

// Constructor for when we know what set list and box we really want - the
// mod_kind is just a check to make the distinction between this and the safe
// constructor above.  It should be LEAF_M.

	sv_model(const sv_set_list& sls, const sv_box& bx, mod_kind k, const sv_model& pt) { model_info = new model_data(sls, bx, pt); }

// Equivalent for no parent

	sv_model(const sv_set_list& sls, const sv_box& bx, mod_kind k) { model_info = new model_data(sls, bx, sv_model()); }

// Constructor to build a divided model

	sv_model(const sv_model& parent, const sv_model& c_1, const sv_model& c_2, 
		mod_kind k, sv_real cut) { model_info = new model_data(parent, c_1, c_2, k, cut); }

// Constructor to copy everything

	sv_model(const sv_model& pt, const sv_set_list& sls, const sv_box& bx, const sv_model& c1, const sv_model& c2, 
		    mod_kind k, sv_real c, sv_integer f) { model_info = new model_data(pt, sls, bx, c1, c2, k, c, f); }

// Initialization of a model

	sv_model(const sv_model& m) { *this = m; }

// Functions to return the contents and children

	sv_box box() const { return(model_info->b); }
	sv_set_list set_list() const { return(model_info->sl); }
	mod_kind kind() const { return(model_info->kind); }
	sv_real coord() const { return(model_info->coord); }

// Return the leaf that contains a point

	sv_model leaf(const sv_point&) const;

// Note that the children may be null if the model is a leaf - check with
// m_kind first.

	sv_model child_1() const { return(*(model_info->child_1)); }
	sv_model child_2() const { return(*(model_info->child_2)); }
	sv_model parent() const { return(*(model_info->p)); }

// Make a deep copy

	sv_model deep() const;

// Return the flags
	
	sv_integer flags() const { return(model_info->flags()); }

// Unique value (effectively the pointer to this set)
// This is explicitly long not sv_integer

        long unique() const { return(model_info.unique()); }

// Any polygons stored for this model?

	int has_polygons() const;
	sv_integer polygon_count() const;

// Models are equal if they point to the same thing, or contain the same set and
// box.

	friend int operator==(const sv_model&, const sv_model&);
	friend int operator!=(const sv_model&, const sv_model&);

// Has the model been defined?

	int exists() const { return(model_info.exists()); }
	
// The recursive model divider

	friend void redivide_r(void*);

// The redivider

	sv_model redivide(const sv_set_list&, void*, sv_decision) const;
			
// The model divider.

	sv_model divide(void* vp, sv_decision) const;


// The procedures that (re)facet a model

	sv_model refacet(const sv_set_list& new_set_list) const
	{
		sv_set_list sl = new_set_list.percolate();
		return(redivide(sl, 0, facet_decision));
	}
 
	sv_model facet() const;

// Membership test against a model

	mem_test member(const sv_point&, sv_primitive*) const;

	mem_test member(const sv_point& p)
	{
		sv_primitive x;
		return(member(p, &x));
	}

// Ray-trace into a model

	sv_set fire_ray(const sv_line&, const sv_interval&, sv_real*) const;
	sv_set fire_ray(const sv_line&, sv_real*) const;

// Find an approximation to the minimum enclosing box round the objects in
// a model using the faceter

	sv_box tight_fit() const;

// Characteristic point

	sv_point point() const;

// Name is the name of the set list, which is the name of the first set in it

	char* name() const { return(set_list().name()); }

// Set flag bit(s)

	void set_flags(sv_integer);
	void reset_flags(sv_integer);

// I/O

// report model statistics to a stream

	void div_stat_report(ostream&) const;

// Write and read are public so that the other classes can get at them
// without everything having to be a friend of everything else.  The
// user shouldn't normally call these.

	friend void unwrite(sv_model&);
	friend void write(ostream&, sv_model&,  sv_integer);
	friend void read(istream&, sv_model&);
	friend void read1(istream&, sv_model&);
	
// Unique tag

	sv_integer tag() const;

// Replace a set in the set list with another - this allows sets
// with polygon attributes to take over from their originals
// The user shouldn't normally call this; but it's public
// for convenience of Jakob Berchtold's Bernstein-basis work

	void replace_set(const sv_set&, const sv_set&,  sv_integer);
};


// ***************************************************************************

// Single structure for all the arguments to the model division procedure
// redivide_r to allow the use of threads and parallelism

class sv_div_data
{
private:

    sv_model md;
    sv_model r;
    sv_integer l;
    sv_set_list new_sl;
    void* user_pointer;
    sv_decision d;
    
public:

    sv_div_data(const sv_model& m, const sv_set_list& sl, sv_integer i,
	        void* up, sv_decision svd)
    {
	md = m;
	l = i;
	new_sl = sl;
	user_pointer = up;
	d = svd;
    }
    
    sv_model model() const { return(md); }
    sv_model result() const { return(r); }
    void result(const sv_model& m) { r = m; }    
    sv_integer level() const { return(l); }
    sv_set_list set_list() const { return(new_sl); }
    void* pointer() const { return(user_pointer); }
    sv_decision decision() const { return(d); }
};

// *********************************************************************************

// Model externs

// The function that returns the root model to a decision procedure

extern sv_model root_model();

extern void set_swell_fac(sv_real);
extern sv_real get_swell_fac();

// Two models the same?

extern prim_op same(const sv_model&, const sv_model&);

// Normal user i/o functions

extern ostream& operator<<(ostream&, sv_model&);
extern istream& operator>>(istream&, sv_model&);

// Lookup table for input

extern look_up<sv_model> m_write_list;

// The did_ and force_facet procedures; these are in polygon.cxx

extern sv_integer did_facet(const sv_model&, const sv_set&);
extern void force_facet(const sv_model& m);

// *******************************************

// Model inlines

// Raytrace into the model returning the hit set and ray parameter

inline sv_set sv_model::fire_ray(const sv_line& l, sv_real* r) const
{
    sv_interval i = line_box(l, box());
    sv_set result;
    if(!i.empty())result = fire_ray(l, i, r);
    return(result);
}

// Models are equal if they point to the same thing

inline int operator==(const sv_model& a, const sv_model& b)
{
	return(a.unique() == b.unique());
}
inline int operator!=(const sv_model& a, const sv_model& b)
{
	return(!(a == b));
}

inline sv_model sv_model::facet() const
{
	return(refacet(set_list()));
}

// The divide function needs to go here so that the set_list function 
// can be defined first.

inline sv_model sv_model::divide(void* vp, sv_decision svd) const
{
	return(redivide(set_list(), vp, svd));
}

// Next is used by the faceter

inline void sv_model::replace_set(const sv_set& sold, const sv_set& snew, 
    sv_integer add_flag)
{
	set_list().replace_set(sold, snew);
	set_flags_priv(add_flag);
}

inline sv_integer sv_model::polygon_count() const 
{
    return(set_list().polygon_count());
}

// model
// **************************************************************************

// Structure to accumulate model statistics in

struct m_stats
{
	sv_model root_m;
	sv_integer total_boxes;

	sv_real a_volume;
	sv_integer a_boxes;
	sv_real s_volume;
	sv_integer s_boxes;
	sv_real a_x; 
	sv_real a_y;
	sv_real a_z;
	sv_real s_x; 
	sv_real s_y;
	sv_real s_z;

	sv_real surface_volume;
	sv_integer surface_boxes;
	sv_real surface_contents;
	sv_integer max_surface_contents;
	sv_real surface_x;
	sv_real surface_y;
	sv_real surface_z;

	sv_integer pgl_boxes;
	sv_real pgl_volume;
	sv_real pgl_x; 
	sv_real pgl_y;
	sv_real pgl_z;
	sv_real pg_count;
	sv_integer max_pg_count;
	
	sv_box littlest;

// Accumulate the statistics

	void model_stats(const sv_model&, m_stats*);
	void model_stats_av(m_stats*);


// Zero all the data, then call model_stats

	m_stats(const sv_model& m)
	{
		root_m = m;
		total_boxes = 0;

		a_volume = 0.0;
		a_boxes = 0;
		a_x = 0.0; 
		a_y = 0.0;
		a_z = 0.0;
		s_volume = 0.0;
		s_boxes = 0;
		s_x = 0.0; 
		s_y = 0.0;
		s_z = 0.0;

		surface_volume = 0.0;
		surface_boxes = 0;
		surface_contents = 0.0;
		max_surface_contents = 0;
		surface_x = 0.0;
		surface_y = 0.0;
		surface_z = 0.0;

		pgl_boxes = 0;
		pgl_volume = 0.0;
		pgl_x = 0.0;
		pgl_y = 0.0;
		pgl_z = 0.0;
		pg_count = 0.0;
		max_pg_count = 0;
		
		littlest = m.box();

		model_stats(m, this);
		model_stats_av(this);
	}
};

#endif




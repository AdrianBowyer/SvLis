/*
 * SvLis - This defines all the set classes that Svlis uses.
 *
 * Adrian Bowyer
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * First version: 4 March 1993
 * This version: 26 October 1998
 * 
 */

#ifndef SVLIS_SET
#define SVLIS_SET

// sv_set: a solid created by doing set-theory on primitive half-spaces



// Set operator output

extern void write(ostream&, set_op);

// Convexity flag detector - only used by set_data::set_data(const sv_primitive&)

sv_integer sv_c_flag(const sv_primitive&);


class sv_set
{
private:

// struct containing the actual data derived
// from the reference-counting base class

   struct set_data : public sv_refct 
   {
        friend class sv_smart_ptr<set_data>;

        set_op op;              // Operator to apply to children.
        sv_integer contents;	// EVERYTHING, NOTHING, or primitive count
        sv_primitive prim;	// If the set is a single half-space
        sv_set *child_1;	// Children if the set is compound
        sv_set *child_2;
        sv_set *complement;	// The set's complement (see -set)

        ~set_data() { delete child_1; delete child_2; delete complement; }

// Special reference count decrement to handle *complement <-> *this

        void remove_reference()
        {
           lock.shut();
           --ref_count;

           if ((ref_count == 1) && complement->exists()) // From the complement?
           {
	      complement->set_info->lock.shut();
	      if(complement->set_info->ref_count == 1)
	      {
		ref_count = 10; // Hack
		complement->set_info->lock.open();
		lock.open();
		delete complement;
		complement = 0;
		delete this;
	      } else
	      {
		complement->set_info->lock.open();
		lock.open();
	      }

	      return;
           } 

           if (ref_count <= 0) 
           {
              lock.open();
              delete this;
           } else
              lock.open();
        }

// Constructor for when it's all or nothing.

	set_data(sv_integer a)
	{
		if ((a != SV_NOTHING) && (a != SV_EVERYTHING))
		    svlis_error("set_data(sv_integer)",
			"sv_set neither null nor universal set",SV_WARNING);
		contents = a;
		child_1 = new sv_set();
		child_2 = new sv_set();
		complement = new sv_set();
	}

// Constructor for set that will be a simple primitive 

	set_data(const sv_primitive& p)
        {

// If the primitive's just a constant . . .

	   if (p.kind() == SV_REAL)
	   {
		if (p.real() <= 0.0) contents = SV_EVERYTHING;
			else contents = SV_NOTHING;
	   } else
	        contents = 1;

	   prim = p;

// A single plane is a convex polygon - sv_c_flag detects this

	   set_flags(sv_c_flag(p));
	   child_1 = new sv_set();
	   child_2 = new sv_set();
	   complement = new sv_set();
        }

// Constructor to build a compound set

	set_data(const sv_set& a, const sv_set& b, set_op optr)
	{
		op = optr;
		contents = a.contents() + b.contents();
		child_1 = new sv_set(a);
		child_2 = new sv_set(b);
	        complement = new sv_set();
	}

// Set the complenment

        void set_complement(const sv_set& c) 
        {
	  *complement = c;
        }

   }; // set_data

// Attributes are stored with the pointers, so that the same geometry
// can have different attributes

	sv_attribute a;

// Deal with complementation

	sv_set complement() const { return(*(set_info->complement)); }
	void complement(const sv_set& c) { set_info->set_complement(c); }

// Constructor for set that is compound.

	sv_set(const sv_set& a, const sv_set& b, set_op optr) { set_info = new set_data(a, b, optr); }

// Priveleged (Re)Set flag bit(s)

        void set_flags_priv(sv_integer a) { set_info->set_flags(a); }
	void reset_flags_priv(sv_integer a) { set_info->reset_flags(a); }

	friend struct set_data;

// This is the pointer that gets ref counted

   sv_smart_ptr<set_data> set_info;


public:

// Constructor for the null (ie undefined, _NOT_ empty) set

	sv_set() { }

// Copy constructor (uses the ref count across = )

	sv_set(const sv_set& s) { *this = s; }

// Constructor for when it's all or nothing.

	sv_set(sv_integer c) { set_info = new set_data(c); }

// Constructor for set that will be a simple primitive.

	sv_set(const sv_primitive& p) { set_info = new set_data(p); }

// Constructors for when the set is a line or a point

	sv_set(const sv_line&);
	sv_set(const sv_point&);

// Functions to return the data

	sv_integer contents() const { return(set_info->contents); }
	set_op op() const { return(set_info->op); }
	sv_primitive primitive() const { return(set_info->prim); }
	sv_set child_1() const { return(*(set_info->child_1)); }
	sv_set child_2() const { return(*(set_info->child_2)); }
	sv_integer flags() const { return(set_info->flags()); }
	sv_set disjunctive_form() const;
	sv_set_list list_products() const;

// Deep copy

	sv_set deep() const;

// Sets are equal when they point to the same hidden set; note
// that two sets with different attributes sharing
// the same geometry are not equal.

	friend int operator==(const sv_set& a, const sv_set& b) { return( (a.unique() == b.unique()) && (a.a == b.a) ); }
	friend int operator!=(const sv_set& a, const sv_set& b) { return(!(a == b)); }

// Test for an undefined set

	int exists() const { return(set_info.exists()); }

// Unique value (effectively the pointer to this set)
// This is explicitly long not sv_integer

	long unique() const { return(set_info.unique()); }

// Unique tag

	sv_integer tag() const;

// Return the degree of the highest-degree primitive

	sv_integer max_degree() const;

// Test for a set with some attributes

	int has_attribute() const { return(a.exists()); }

// Set union

	friend sv_set operator|(const sv_set&, const sv_set&);
	friend int quickor(const sv_set&, const sv_set&, sv_set*);
    
// Set intersection

	friend sv_set operator&(const sv_set&, const sv_set&);
	friend int quickand(const sv_set&, const sv_set&, sv_set*);

// Set complement

	friend sv_set operator-(const sv_set&);
	
// Membership test a point

	mem_test member(const sv_point&) const;

// Membership test a point (with known surface)

	mem_test member(const sv_point&, sv_primitive []) const;

// Value for a point (and winning leaf)

	sv_real value(const sv_point&, sv_set*) const;

// Range for a box (and winning leaves)

	sv_interval range(const sv_box&, sv_set*, sv_set*) const;

// Prune a set to a box

	sv_set prune(const sv_box&) const;

// Return the characteristic point of a set

	sv_point point() const;

// Rotate a set about a line

	sv_set spin(const sv_line&, sv_real) const;

// Scale a set about a point

	sv_set scale(const sv_point&, sv_real) const;

// Mirror a set in a plane

	sv_set mirror(const sv_plane&) const;

// Are two sets the same?  (NB this completely ignores attributes
// and is only concerned with geometry.)  This needs access to
// the complement, and so is a friend

	friend prim_op same(const sv_set&, const sv_set&);
	
// Return a reglarized version of simple sets

	sv_set regularize() const;

// Attribute stuff.
// Colour, string, and polygons as attributes - special member functions cos 
// they're so common, also surface

	sv_set attribute(const sv_attribute&) const;
	sv_attribute attribute() const { return(a); }
	sv_set colour(const sv_point&) const;
	sv_point colour() const;
	sv_set name(char*) const;
	char* name() const;
	sv_set polygon(sv_p_gon*) const;
	sv_p_gon* polygon() const;
	sv_integer polygon_count() const;
	sv_set surface(const sv_surface&) const;
	sv_surface surface() const;

// Remove all attributes from a set

	sv_set strip_attributes() const;

// Resolve attribute inheritance and percolate attributes to leaves

	sv_attribute inherit(const sv_set&) const;
	sv_set percolate() const;

// Set flag bit(s)

	void set_flags(sv_integer);
	void reset_flags(sv_integer);

// I/O

// Write and read are public so that the other classes can get at them
// without everything having to be a friend of everything else.  The
// user shouldn't normally call these.

	friend void unwrite(sv_set&);
	friend void write(ostream&, const sv_set&, sv_integer);
	friend void read(istream&, sv_set&);
	friend void read1(istream&, sv_set&);
}; // sv_set


// ************* External functions

extern istream& operator>>(istream&, set_op&);

// Add and subtract a point

extern sv_set operator+(const sv_set&, const sv_point&);

// See if a leaf set in a box can be approximated by a plane.

extern sv_integer leaf_s_to_plane(const sv_set&, const sv_box&, sv_plane*);

// Normal user i/o functions

extern ostream& operator<<(ostream&, sv_set&);
extern istream& operator>>(istream&, sv_set&);

// Look up list for input

extern look_up<sv_set> s_write_list;

// Set flag for whether the results of pruning are
// regularized (true forces regularization)

extern void regular_prune(sv_integer);

// ************** Inlines

// Simplest way to subtract a point is to negate it and add

inline sv_set operator-(const sv_set& a, const sv_point& p) { return(a + (-p));}

// Make translation commute (is this sensible?)

inline sv_set operator+(const sv_point& p, const sv_set& a) { return(a + p);}

// Difference is intersection with complement

inline sv_set operator-(const sv_set& a, const sv_set& b)
{
	return(a & (-b));
}

// Symmetric difference is union of a-b and b-a

inline sv_set operator^(const sv_set& a, const sv_set& b)
{
	return( (a & (-b)) | (b & (-a)) );
}

// Compatibility with the old version

inline sv_set attach_attribute(const sv_set& s, const sv_attribute& a)
{
	return(s.attribute(a));
}

// Simple membership (null surface primitive list)

inline mem_test sv_set::member(const sv_point& p) const
{
    sv_primitive a;
    return(member(p, &a));
}

// sv_set
// ***************************************************************************

// List of sets


class sv_set_list
{
private:

// struct containing the actual data derived
// from the reference-counting base class

   struct set_list_data : public sv_refct 
   {
        friend class sv_smart_ptr<set_list_data>;

	sv_set s;			// The set
	sv_set_list* next;		// The next one along

        ~set_list_data() { delete next; }

// Constructor for when it's a single set.

	set_list_data(const sv_set& a)
	{
	        s = a;
		next = new sv_set_list();
	}

// Constructor to put a new set at the head.

	set_list_data(const sv_set& a, const sv_set_list& sl)
	{
	        s = a;
		next = new sv_set_list(sl);
	}


   }; // set_list_data

// This is the pointer that gets ref counted

   sv_smart_ptr<set_list_data> set_list_info;


// Priveleged (Re)Set flag bit(s)

        void set_flags_priv(sv_integer a) { set_list_info->set_flags(a); }
	void reset_flags_priv(sv_integer a) { set_list_info->reset_flags(a); }

	
// Directly replace a set in the list

	void replace_set(const sv_set&,  const sv_set&);
	friend class sv_model;

public:

// Null list

	sv_set_list() { }

// Construct from a single set

	sv_set_list(const sv_set& a) { set_list_info = new set_list_data(a); }

// Constructor to put a new set at the head.

	sv_set_list(const sv_set& a, const sv_set_list& sl) { set_list_info = new set_list_data(a, sl); }

// Copy constructor (uses the ref count across = )

	sv_set_list(const sv_set_list& sl) { *this = sl; }

// Two set lists are equal if their sets are the same in any order

	friend int operator==(const sv_set_list&, const sv_set_list&);

// Functions to return contents

	sv_integer flags() const { return(set_list_info->flags()); }
	sv_set_list next() const { return(*(set_list_info->next)); }
	sv_set set() const { return(set_list_info->s); }
	sv_set unite() const;
	sv_set intersect() const;

// Deep copy

	sv_set_list deep() const;

// Unique value (effectively the pointer to this set)
// This is explicitly long not sv_integer

	long unique() const { return(set_list_info.unique()); }
	
// return the number of sets

	sv_integer count() const;

// Does the list exist?

	int exists() const { return(set_list_info.exists()); }

// The total contents of a set list

	sv_integer contents() const;

// Prune a set list to a box

	sv_set_list prune(const sv_box&) const;

// Operators on collections of sets... 

        friend sv_set_list merge(const sv_set_list&, const sv_set_list&);    // Union
        friend sv_set_list overlap(const sv_set_list&, const sv_set_list&);  // Intersection
        friend sv_set_list remove(const sv_set_list&, const sv_set_list&);   // Subtraction
        friend sv_set_list ex_or(const sv_set_list&, const sv_set_list&);    // Symmetric difference

// Append for backwards compatibility

        sv_set_list append(const sv_set_list&) const;

// Remove some sets from a list, returning a new list of the ones remaining
// If there are none to remove, return the original list (NOT a copy).
// Retained for backwards compatibility

	sv_set_list remove_sets(const sv_set_list& a) const { return(remove(*this, a)); }

// Return the degree of the highest-degree primitive in 
// all the sets

	sv_integer max_degree() const;
		
// Percolate attributes for all sets in the list

        sv_set_list percolate() const;

// Polygons

	sv_integer polygon_count() const;
	
// Characteristic point

	sv_point point() const;

// Unique tag

        sv_integer tag() const;

// Set flag bit(s)

	void set_flags(sv_integer);
	void reset_flags(sv_integer);

// I/O

// Write and read are public so that the other classes can get at them
// without everything having to be a friend of everything else.  The
// user shouldn't normally call these.

	friend void unwrite(sv_set_list&);
	friend void write(ostream&, const sv_set_list&, sv_integer);
	friend void read(istream&, sv_set_list&);
	friend void read1(istream&, sv_set_list&);
	friend sv_set_list read_sl_r(istream&);
	friend sv_set_list read_sl_r1(istream&);
};


// *************** externs

// Normal user i/o functions

extern ostream& operator<<(ostream&, sv_set_list&);
extern istream& operator>>(istream&, sv_set_list&);

// Look up list for input

extern look_up<sv_set_list> sl_write_list;

// Set lists the same?

extern prim_op same(const sv_set_list&, const sv_set_list&);

// Set operators on collections of sets... 

extern sv_set_list merge(const sv_set_list&, const sv_set_list&);
extern sv_set_list overlap(const sv_set_list&, const sv_set_list&);
extern sv_set_list remove(const sv_set_list&, const sv_set_list&);
extern sv_set_list ex_or(const sv_set_list&, const sv_set_list&);

// **************** inlines

inline int operator!=(const sv_set_list& a, const sv_set_list& b) {return(!(a == b));}

// Ops on collections and single sets

inline sv_set_list merge(const sv_set_list& a, const sv_set& b) { return( merge(a, sv_set_list(b))); }
inline sv_set_list overlap(const sv_set_list& a, const sv_set& b) { return( overlap(a, sv_set_list(b))); }
inline sv_set_list remove(const sv_set_list& a, const sv_set& b) { return( remove(a, sv_set_list(b))); }
inline sv_set_list ex_or(const sv_set_list& a, const sv_set& b) { return( ex_or(a, sv_set_list(b))); }
inline sv_set_list merge(const sv_set& a, const sv_set_list& b) { return( merge(sv_set_list(a), b)); }
inline sv_set_list overlap(const sv_set& a, const sv_set_list& b) { return( overlap(sv_set_list(a), b)); }
inline sv_set_list remove(const sv_set& a, const sv_set_list& b) { return( remove(sv_set_list(a), b)); }
inline sv_set_list ex_or(const sv_set& a, const sv_set_list& b) { return( ex_or(sv_set_list(a), b)); }

// Append for old times sake

inline sv_set_list sv_set_list::append(const sv_set_list& s) const { return(merge(*this, s)); }

// Polygon procedures

inline sv_integer sv_set_list::polygon_count() const
{
	sv_set_list sl = *this;
	sv_integer c = 0;

	while(sl.exists())
	{
		c = c + set().polygon_count();
		sl = sl.next();
	}
	return(c);
}

// Directly replace a set in the list
// NB if the set is in the list more than
// once, all instances are replaced

inline void sv_set_list::replace_set(const sv_set& sold,  const sv_set& snew) 
{
	sv_set_list sls = *this;
	int hit = 0;
	
	while(sls.exists())
	{
		if (sls.set() == sold) 
		{
		    sls.set_list_info->s = snew;
		    hit = 1;
		}
		sls = sls.next();
	}
	if(!hit) svlis_error("sv_set_list::replace_set",
	    "old set not found",  SV_WARNING);
}

// sv_set_list
// *************************************************************************

#endif


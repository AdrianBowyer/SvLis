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
 * SvLis - user-defined attributes
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


// Currently attributes can be points, text strings, polygons, or intervals
// Surfaces added 28/12/95
// Copy the form to make any other type, class or struct you like an attribute

#include "svlis.h"
#if macintosh
 #pragma export on
#endif

// Global just to get the tag values

static sv_p_gon pt;


// Read in an sv_user_attribute
// Current version format

void read(istream& s, sv_user_attribute** uap)
{
    if(get_read_version() != get_svlis_version())
    {
	if(get_read_version() != (get_svlis_version()-1))
		svlis_error("read(.. sv_user_attribute*)",
		"file version is too old",
		SV_WARNING);
	read1(s, uap);
	return;
    }

    sv_point p;
    sv_point* pp;
    sv_interval i;
    sv_interval* ip;
    sv_p_gon* pg;
    char* c;
    char dc;
    sv_surface surf;
    sv_real rd;
    sv_integer c_count;

    *uap = new sv_user_attribute();
    
    long dummy;
    sv_integer user_tag_v = 0;
	
    if(check_token(s, SVT_USER_ATTRIBUTE))
    {
	check_token(s, SVT_OB);
	get_token(s, dummy, rd, 1); // Read the pointer the write function wrote
	
	if (!dummy)
	{
		delete *uap;
		*uap = 0;	// Return a null attribute
		return;
	}
	
	delete *uap; // Result is going to be non-null
	*uap = 0;
	
	get_token(s, user_tag_v, rd, 1); // Read the tag
	
// Read an item of the type decided by the tag (the item goes out of scope on return);
// Then create an sv_user_attribute out of a pointer to a copy of it

	if (user_tag_v == text_tag())
	{
		check_token(s, SVT_TEXT);
		get_token(s, c_count, rd, 1); // Read the number of bytes
		c = new char[c_count+2];
		s.get(dc);  // Throw away the newline
		s.get(c,c_count+2,(char)0); // Read the string
		*uap = new sv_user_attribute((void*)c);
		s.get(dc);  // Throw away the 0 string terminator
	} else if (user_tag_v == p.tag()) 
	{
		junk_junk(s);
		s >> p;
		pp = new sv_point(p);
		*uap = new sv_user_attribute((void*)pp);
	} else if (user_tag_v == i.tag())
	{
		junk_junk(s);
		s >> i;
		ip = new sv_interval(i);
		*uap = new sv_user_attribute((void*)ip);
	} else if (user_tag_v == pt.tag())
	{
		read(s, &pg);
		*uap = new sv_user_attribute((void*)pg);
	} else if (user_tag_v == surf.tag())
	{
		read(s, surf);
		*uap = new sv_user_attribute((void*) new sv_surface(surf));
	} else
		svlis_error("read(...sv_user_attribute*...)","unknown attribute tag",
			SV_WARNING);
	check_token(s, SVT_CB);
    }
}

// Read in an sv_user_attribute
// Version 2 format
// If you intend to edit this procedure, you're probably 
// making a mistake...

void read1(istream& s, sv_user_attribute** uap)
{
    sv_point p;
    sv_point* pp;
    sv_interval i;
    sv_interval* ip;
    sv_p_gon* pg;
    char* c;
    char dc;
    sv_surface surf;
    sv_real rd;
    sv_integer c_count;

    *uap = new sv_user_attribute();
    
    long dummy;
    sv_integer user_tag_v = 0;
	
    if(check_token(s, SVT_USER_ATTRIBUTE))
    {
	check_token(s, SVT_OB);
	get_token(s, dummy, rd, 1); // Read the pointer the write function wrote
	
	if (!dummy)
	{
		delete *uap;
		*uap = 0;	// Return a null attribute
		return;
	}
	
	delete *uap; // Result is going to be non-null
	*uap = 0;
	
	get_token(s, user_tag_v, rd, 1); // Read the tag
	
// Read an item of the type decided by the tag (the item goes out of scope on return);
// Then create an sv_user_attribute out of a pointer to a copy of it

	if (user_tag_v == text_tag())
	{
		check_token(s, SVT_TEXT);
		get_token(s, c_count, rd, 1); // Read the number of bytes
		c = new char[c_count+2];
		s.get(dc);  // Throw away the newline
		s.get(c,c_count+2,(char)0); // Read the string
		*uap = new sv_user_attribute((void*)c);
		s.get(dc);  // Throw away the 0 string terminator
	} else if (user_tag_v == p.tag()) 
	{
		junk_junk(s);
		s >> p;
		pp = new sv_point(p);
		*uap = new sv_user_attribute((void*)pp);
	} else if (user_tag_v == i.tag())
	{
		junk_junk(s);
		s >> i;
		ip = new sv_interval(i);
		*uap = new sv_user_attribute((void*)ip);
	} else if (user_tag_v == pt.tag())
	{
		read1(s, &pg);
		*uap = new sv_user_attribute((void*)pg);
	} else if (user_tag_v == surf.tag())
	{
		read1(s, surf);
		*uap = new sv_user_attribute((void*) new sv_surface(surf));
	} else
		svlis_error("read1(...sv_user_attribute*...)","unknown attribute tag",
			SV_WARNING);
	check_token(s, SVT_CB);
    }
}

// Write an sv_user_attribute to a stream; svLis uses the negative of the
// tag internally so that all positive tags are available for the user

void write(ostream& s, const sv_user_attribute* ua, 
	sv_integer tag, sv_integer level)
{

// The following four are needed so their tag functions can be called

	sv_point p;
	sv_interval i;
	sv_p_gon* pg = 0;
	sv_surface surf;

	sv_integer nxl = level+1;

	put_white(s, level);
	put_token(s, SVT_USER_ATTRIBUTE, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB, 0, 0);
	s << SV_EL;

// Got anything to do?

	if(ua)
	{
		if(ua->pointer)
		{
			// Write the pointer as a long, then the appropriate tag, then the 
			// Thing the attribute is

			writei(s, (long)ua->pointer, nxl);
			s << ' ';
			if (tag == -text_tag())
			{
				writei(s, text_tag(), 0);
				s << SV_EL;
				put_white(s, nxl);
				put_token(s, SVT_TEXT, 0, 0);
				s << ' ';
				writei(s, sv_strlen((char*)ua->pointer), 0);
				s << SV_EL;
				s << (char*)ua->pointer;
				s << '\0';
			}
			else if (tag == -p.tag())
			{
				writei(s, p.tag(), 0); s << SV_EL;
				put_white(s, nxl);
				s << *((sv_point*)ua->pointer);
				s << SV_EL;
			}
			else if (tag == -i.tag())
			{
				writei(s, i.tag(), 0); s << SV_EL;
				put_white(s, nxl);
				s << *((sv_interval*)ua->pointer);
				s << SV_EL;
			}
			else if (tag == -pt.tag())
			{
				writei(s, pt.tag(), 0); s << SV_EL;
				write(s, (sv_p_gon*)ua->pointer, nxl);
			}
			else if (tag == -surf.tag())
			{
				writei(s, surf.tag(), 0); s << SV_EL;
				write(s, *((sv_surface*)ua->pointer), nxl);
			}
			else
			{
				svlis_error("write(...sv_user_attribute*...)","unknown attribute tag",
					SV_WARNING);
				writei(s, 0, nxl);
			}
		} else
			writei(s, 0, nxl);
	} else
		writei(s, 0, nxl);
	s << SV_EL;		
	put_white(s, level);
	put_token(s, SVT_CB, 0, 0);
	s << SV_EL;
}


// Intervals as attributes

sv_interval set_interval(const sv_set& s)
{
	sv_user_attribute* u;
	sv_interval result;

        sv_attribute a = s.attribute(); // Pick up the attribute
        a = a.tag_find(-result.tag());  // Find the attribute in the linked list
        if (a.exists())
        {
                u = a.user_attribute();
                result = *( (sv_interval*)u->pointer );
        }
	return(result);
}

// These allow the user to do sensible memory management

// This gets called whenever something that points to ua gets deleted

extern void free_user(sv_user_attribute* ua, sv_integer tag)
{

// These are needed for their tags

	sv_point p;
	sv_interval i;
	sv_p_gon* pg;
	sv_surface surf;
	
	if(ua)
	{
		ua->lock.shut();	   // No other process must alter the ref count
		if(--(ua->ref_count) <= 0) // If the reference count goes <= 0 nothing points to ua
		{
			if (tag == -text_tag())
				delete [] (char*)ua->pointer;
			else if (tag == -p.tag())
			    delete (sv_point*)ua->pointer;
			else if (tag == -i.tag())
				delete (sv_interval*)ua->pointer;
			else if (tag == -pt.tag())
				kill_p_gon((sv_p_gon*)ua->pointer);
			else if (tag == -surf.tag())
				delete (sv_surface*)ua->pointer;
			else
				svlis_error("free_user","unknown attribute tag",SV_WARNING);
			ua->lock.open();   // Can't delete a shut lock
			delete ua;
		} else
			ua->lock.open();   // Now others can change the ref count
	}
} 

// This gets called whenever a pointer to ua gets copied inside svLis
		
void copy_user(sv_user_attribute* ua, sv_integer tag)
{
	if(ua)
	{
		ua->lock.shut();  // Two processes trying to change the ref count is a bad idea
		ua->ref_count++;
		ua->lock.open();
	}
}

// Functions that get called to track attributes when operations are
// done on sets

sv_set att_union(const sv_set& result, const sv_set& arg1, const sv_set& arg2)
{
	return(result);
}

sv_set att_intersection(const sv_set& result, const sv_set& arg1, const sv_set& arg2)
{
	return(result);
}

// The transformations (including scale and complement) copy the original attribute

sv_set att_slide(const sv_set& result, const sv_set& arg1, const sv_point& arg2)
{
	if (arg1.has_attribute())
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}
	
sv_set att_spin(const sv_set& result, const sv_set& arg1, const sv_line& arg2, sv_real arg3)
{
	if (arg1.has_attribute())
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}

sv_set att_mirror(const sv_set& result, const sv_set& arg1, const sv_plane& arg2)
{
	if (arg1.has_attribute())
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}

sv_set att_scale(const sv_set& result, const sv_set& arg1, const sv_point& arg2, sv_real arg3)
{
	if (arg1.has_attribute())
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}

sv_set att_scale(const sv_set& result, const sv_set& arg1, const sv_line& arg2, sv_real arg3)
{
	if (arg1.has_attribute())
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}

sv_set att_complement(const sv_set& result, const sv_set& arg1)
{
	if (arg1.has_attribute())
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}

// If pruning something with an attribute creates something without, copy
// the attribute down.

sv_set att_prune(const sv_set& result, const sv_set& arg1, const sv_box& arg2)
{
	if (arg1.has_attribute() && (result != arg1) && (!result.has_attribute()))  
		return( result.attribute(arg1.attribute()) );
	else
		return(result);
}


// Surfaces as attributes

sv_set sv_set::surface(const sv_surface& surf) const
{
	sv_surface* sp = new sv_surface(surf);
	sv_attribute a = sv_attribute(-surf.tag(), new sv_user_attribute((void*)sp));
	sv_attribute b;
	if (has_attribute())
	{
		b = attribute();
		b = b.replace(a);
	} else
		b = a;

	return(this->attribute(b));
}

// Find the surface of a set.

sv_surface sv_set::surface() const
{
	sv_user_attribute* u;
	
	sv_surface surf;

	sv_attribute a = attribute();
	a = a.tag_find(-surf.tag());
	if (a.exists())
	{
		u = a.user_attribute();
		surf = *((sv_surface*)u->pointer);
	}
	return(surf);
}

// Colours as attributes
// Recoded to use surfaces 28/12/95

// Colour a set.  The colour red green and blue are in p.x, p.y, and p.z
// 0.0 is black, 1.0 is full saturation.

sv_set sv_set::colour(const sv_point& p) const
{
	sv_surface surf;
	surf.set_diffuse_colour(p);
	return(this->surface(surf));
}

// Find the colour of a set.

sv_point sv_set::colour() const
{
	sv_surface surf = this->surface();
	return(surf.diffuse_colour()*surf.diffuse_coeff());
}



// Strings as attributes

// Name a set

sv_set sv_set::name(char* str) const
{
	char* s = new char[sv_strlen(str) + 2];
	sv_strcpy(s,str);
	sv_attribute a = sv_attribute(-text_tag(), 
		new sv_user_attribute((void*)s));
	sv_attribute b;
	if (has_attribute())
	{
		b = attribute();
		b = merge(b, a);
	} else
		b = a;

	return(this->attribute(b));
}

// Find the name of a set.

char* sv_set::name() const
{
        sv_user_attribute* u;

        sv_attribute a = attribute();
        a = a.tag_find(-text_tag());
        if (a.exists())
        {
                u = a.user_attribute();
                return((char*)u->pointer);
        }
        else
                return(0);
}


// Polygons as attributes

sv_set sv_set::polygon(sv_p_gon* pg) const
{
	sv_attribute a = sv_attribute(-pt.tag(), 
		new sv_user_attribute((void*)pg));
	sv_attribute b;
	if (has_attribute())
	{
		b = attribute();
		b = merge(b, a);
	} else
		b = a;

	return(this->attribute(b));
}

// Find the polygon attribute of a set.
// NB this returns the first; there may be more than
// one.  

sv_p_gon* sv_set::polygon() const
{
        sv_user_attribute* u;
        sv_p_gon* pg;
	sv_p_gon dum;

        sv_attribute a = attribute();
        a = a.tag_find(-dum.tag());
        if (a.exists())
        {
                u = a.user_attribute();
                return((sv_p_gon*)u->pointer);
        }
        else
                return(0);
}

#if macintosh
 #pragma export off
#endif

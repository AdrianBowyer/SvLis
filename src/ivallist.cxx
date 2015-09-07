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
 * SvLis RAYTRACER - sorted-list-of-intervals function file
 *
 * Andy Wallis
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: May 1994
 * This version: 23 September 2000
 *
 */


#include "svlis.h"
#include "ivallist.h"
#if macintosh
 #pragma export on
#endif

// Add an entry to the tail of a sorted-list-of-intervals

void
sorted_interval_list::add_to_tail(const sv_interval& i,
				  const sv_set& l, const sv_set& h)
{
   interval_list_entry *ptr = list;
   interval_list_entry *prev = 0;
   interval_list_entry *new_entry;

   while(ptr) {
      prev = ptr;
      ptr = ptr->next;
   }

   new_entry = new interval_list_entry;
   new_entry->intrval = i;
   new_entry->slo = l;
   new_entry->shi = h;
   new_entry->next = ptr;

   if(prev)
      prev->next = new_entry;
   else
      list = new_entry;
}


// Insert an entry into a sorted-list-of-intervals

void
sorted_interval_list::insert(const sv_interval& intrval,
			     const sv_set& l, const sv_set& h)
{
    interval_list_entry *ptr = list;
    interval_list_entry *prev = 0;
    interval_list_entry *new_entry;

   while(ptr && (ptr->intrval.lo() < intrval.lo())) { // This depends on lazy eval - AB
      prev = ptr;
      ptr = ptr->next;
   }

   new_entry = new interval_list_entry;
   new_entry->intrval = intrval;
   new_entry->slo = l;
   new_entry->shi = h;
   new_entry->next = ptr;

   if(prev)
      prev->next = new_entry;
   else
      list = new_entry;
}

// Remove an entry from the head of an intersection list

void
sorted_interval_list::remove_from_head(sv_interval* i, sv_set* l,
	sv_set* h)
{
   sv_set dummy;
   
   if(list) {
      interval_list_entry *new_list = list->next;
      *i = list->intrval;
      *l = list->slo;
      *h = list->shi;
      delete list;
      list = new_list;
   } else
   {
	*i = sv_interval();
	*l = dummy;
	*h = dummy;
   }
}

// Assignment operator: perform deep copy

sorted_interval_list
sorted_interval_list::operator=(const sorted_interval_list& src)
{
    interval_list_entry *ptr = list;
    interval_list_entry *next;
    interval_list_entry **prev_next_ptr = &list;
    interval_list_entry *new_entry;

   if( this != &src) {	// Check for "dest=src"

      // Free any storage previously used by target of assignment
      while(ptr) {
	 next = ptr->next;
	 delete ptr;
	 ptr = next;
      }

      // Clear new list
      list = 0;

      // Copy src to target
      ptr = src.list;
      while(ptr) {
	 new_entry = new interval_list_entry;
	 new_entry->intrval = ptr->intrval;
	 new_entry->slo = ptr->slo;
	 new_entry->shi = ptr->shi;
	 new_entry->next = 0;
	 *prev_next_ptr = new_entry;
	 prev_next_ptr = &(new_entry->next);
	 ptr = ptr->next;
      }
   }
   return *this;
}

// Union operator: perform union of terms in both lists

sorted_interval_list
operator|(const sorted_interval_list& a, const sorted_interval_list& b)
{
   interval_list_entry *a_ptr = a.list;		// Pointer to current interval in a
   interval_list_entry *b_ptr = b.list;		// Pointer to current interval in b
   float next_change_in_a;			// Value of next change in a
   float next_change_in_b;			// Value of next change in b
   sv_set next_set_in_a;			// Same for sets
   sv_set next_set_in_b;

   sv_integer in_a;					// Flag non-zero if currently in solid for a
   sv_integer in_b;					// Flag non-zero if currently in solid for b

   float new_lo;				// Value of start of new (unioned) interval
   sv_set new_set;					// Set for new (unioned) interval

   sorted_interval_list result;

   // if either list is empty, return the other list
   if(a_ptr == 0)
      return b;

   if(b_ptr == 0)
      return a;

   // Initialise next change in each list

   if(a_ptr->intrval.lo() < b_ptr->intrval.lo()) {
      next_change_in_a = a_ptr->intrval.hi();
      next_set_in_a = a_ptr->shi;
      next_change_in_b = b_ptr->intrval.lo();
      next_set_in_b = b_ptr->slo;
      new_lo = a_ptr->intrval.lo();
      new_set = a_ptr->slo;
      in_a = 1;
      in_b = 0;
   } else {
      next_change_in_a = a_ptr->intrval.lo();
      next_set_in_a = a_ptr->slo;
      next_change_in_b = b_ptr->intrval.hi();
      next_set_in_b = b_ptr->shi;
      new_lo = b_ptr->intrval.lo();
      new_set = b_ptr->slo;
      in_a = 0;
      in_b = 1;
   }

   do {
      if(next_change_in_a < next_change_in_b) {
	 if(in_a) {
	    if(!in_b)
	       result.add_to_tail(sv_interval(new_lo, next_change_in_a), 
		new_set, next_set_in_a);
	    in_a = 0;
	    a_ptr = a_ptr->next;
	    if(a_ptr)
	    {
	       next_change_in_a = a_ptr->intrval.lo();
	       next_set_in_a = a_ptr->slo;
	    }
	 } else {
	    in_a = 1;
	    if(!in_b) {
	       new_set = a_ptr->slo;
	       new_lo = next_change_in_a;
	    }
	    next_change_in_a = a_ptr->intrval.hi();
	    next_set_in_a = a_ptr->shi;
	 }
      } else {
	 if(in_b) {
	    if(!in_a)
	       result.add_to_tail(sv_interval(new_lo, next_change_in_b), 
		new_set, next_set_in_b);
	    in_b = 0;
	    b_ptr = b_ptr->next;
	    if(b_ptr)
	    {
		next_change_in_b = b_ptr->intrval.lo();
		next_set_in_b = b_ptr->slo;
	    }
	 } else {
	    in_b = 1;
	    if(!in_a) {
	       new_set = b_ptr->slo;
	       new_lo = next_change_in_b;
	    }
	    next_change_in_b = b_ptr->intrval.hi();
	    next_set_in_b = b_ptr->shi;
	 }
      }
   }
   while(a_ptr && b_ptr);

   // At least one list finished, add rest of other list to result

   if(!a_ptr) {
      if(in_b) {
	 result.add_to_tail(sv_interval(new_lo, next_change_in_b), 
		new_set, next_set_in_b);	 
	 b_ptr = b_ptr->next;
      }
      
      while(b_ptr) {
	 result.add_to_tail(b_ptr->intrval,b_ptr->slo,b_ptr->shi);
	 b_ptr = b_ptr->next;
      }
   } else {
      if(in_a) {
	 result.add_to_tail(sv_interval(new_lo, next_change_in_a), 
		new_set, next_set_in_a);	 
	 a_ptr = a_ptr->next;
      }
      
      while(a_ptr) {
	 result.add_to_tail(a_ptr->intrval,a_ptr->slo,a_ptr->shi);
	 a_ptr = a_ptr->next;
      }
   }

   return result;
}


// Intersection operator: perform intersection of terms in both lists

sorted_interval_list
operator&(const sorted_interval_list& a, const sorted_interval_list& b)
{

   interval_list_entry *a_ptr = a.list;		// Pointer to current interval in a
   interval_list_entry *b_ptr = b.list;		// Pointer to current interval in b
   float next_change_in_a;			// Value of next change in a
   float next_change_in_b;			// Value of next change in b
   sv_set next_set_in_a;			// Same for sets
   sv_set next_set_in_b;

   sv_integer in_a;					// Flag non-zero if currently in solid for a
   sv_integer in_b;					// Flag non-zero if currently in solid for b

   float new_lo;				// Value of start of new (intersected) interval
   sv_set new_set;					// Set for new (intersected) interval

   sorted_interval_list result;

   // if either list is empty, return an empty list
   if((a_ptr == 0) || (b_ptr == 0))
      return result;

   // Initialise next change in each list

   if(a_ptr->intrval.lo() < b_ptr->intrval.lo()) {
      next_change_in_a = a_ptr->intrval.hi();
      next_set_in_a = a_ptr->shi;
      next_change_in_b = b_ptr->intrval.lo();
      next_set_in_b = b_ptr->slo;
      new_lo = a_ptr->intrval.lo();
      new_set = a_ptr->slo;
      in_a = 1;
      in_b = 0;
   } else {
      next_change_in_a = a_ptr->intrval.lo();
      next_set_in_a = a_ptr->slo;
      next_change_in_b = b_ptr->intrval.hi();
      next_set_in_b = b_ptr->shi;
      new_lo = b_ptr->intrval.lo();
      new_set = b_ptr->slo;
      in_a = 0;
      in_b = 1;
   }

   do {
      if(next_change_in_a < next_change_in_b) {
	 if(in_a) {
	    if(in_b)
	       result.add_to_tail(sv_interval(new_lo, next_change_in_a), 
		new_set, next_set_in_a);
	    in_a = 0;
	    a_ptr = a_ptr->next;
	    if(a_ptr)
	    {
	       next_change_in_a = a_ptr->intrval.lo();
	       next_set_in_a = a_ptr->slo;
	    }
	 } else {
	    in_a = 1;
	    if(in_b) {
	       new_set = a_ptr->slo;
	       new_lo = next_change_in_a;
	    }
	    next_change_in_a = a_ptr->intrval.hi();
	    next_set_in_a = a_ptr->shi;
	 }
      } else {
	 if(in_b) {
	    if(in_a)
	       result.add_to_tail(sv_interval(new_lo, next_change_in_b), 
		new_set, next_set_in_b);
	    in_b = 0;
	    b_ptr = b_ptr->next;
	    if(b_ptr)
	    {
	       next_change_in_b = b_ptr->intrval.lo();
	       next_set_in_b = b_ptr->slo;
	    }
	 } else {
	    in_b = 1;
	    if(in_a) {
	       new_set = b_ptr->slo;
	       new_lo = next_change_in_b;
	    }
	    next_change_in_b = b_ptr->intrval.hi();
	    next_set_in_b = b_ptr->shi;
	 }
      }
   }
   while(a_ptr && b_ptr);

   // At least one list finished: all done
	 
   return result;
}



// Debug print for ray_int_list

void
debug_print_sil(const sorted_interval_list& i,
		char* msg)
{
   interval_list_entry *ptr;
   void *set_addr;

   cout << "----- sorted-interval-list ";
   if(msg) cout << msg;
//   cout << " (at " << &i << ") is:\n";
   cout << " (at " << (sv_integer)&i << ") is:\n";		// GMB 27-7-94
   ptr = i.list;
   while(ptr) {
      cout << "interval " << ptr->intrval << ": sets ";
      if(ptr->slo.exists()) {
	 set_addr = (void*)&(ptr->slo);
	 cout << set_addr << "\n";
      } else {
	 cout << "NULL-set\n";
      }
      if(ptr->shi.exists()) {
	 set_addr = (void*)&(ptr->shi);
	 cout << set_addr << "\n";
      } else {
	 cout << "NULL-set\n";
      }
      ptr = ptr->next;
   }
   cout << "-----\n\n";
}

#if macintosh
 #pragma export off
#endif










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
 * SvLis - sorted-list-of-intervals header file
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
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_IVLIST
#define SVLIS_IVLIST

// Ammended by AB to store sets at both ends of each interval

struct interval_list_entry {
   sv_interval intrval;
   sv_set slo,shi;
   struct interval_list_entry *next;
};

// The sorted-list-of-intervals class stores intersections between an unspecified
// directed line (ray) and primitives.  Each interval represents a region of `solid
// The list consists of a sorted list of disjoint intervals.
// The list is kept sorted in increasing value of the low limit of each interval.

// For the future?
// This list COULD be forced to be kept in consecutive memory locations
// to allow a binary chop to be used in the insertion function.  this
// would need a copy-constructor etc.


class sorted_interval_list
{
   struct interval_list_entry *list;	// Pointer to list
   
 public:

// Functions to insert an entry into a list and to remove an entry from the head of a list

   void insert(const sv_interval&, const sv_set&, const sv_set&);
   void remove_from_head(sv_interval*,sv_set*,sv_set*);
   void add_to_tail(const sv_interval&, const sv_set&, const sv_set&);

// Constructors and Destructor for a sorted_interval_list

   sorted_interval_list() { list = 0; }

   sorted_interval_list(const sv_interval& i, const sv_set& l,
	 const sv_set& h)
   {
      list = new interval_list_entry;
      list->intrval = i;
      list->slo = l;
      list->shi = h;
      list->next = 0;
   }


// Copy constructor

   sorted_interval_list(const sorted_interval_list& src)
   {
      struct interval_list_entry *ptr;
      struct interval_list_entry **prev_next_ptr = &list;
      struct interval_list_entry *new_entry;

      list = 0;		// Is this needed?
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

   ~sorted_interval_list()
   {
      struct interval_list_entry *next;
      struct interval_list_entry *ptr = list;
      while(ptr) {
	 next = ptr->next;
	 delete ptr;
	 ptr = next;
      }
   }

// Added by AB

   interval_list_entry* entry() { return(list);}
   
// Assignment operator

   sorted_interval_list operator=(const sorted_interval_list &src);

// Set functions for sorted_interval_lists

   friend sorted_interval_list operator|(const sorted_interval_list& a, const sorted_interval_list& b);
   friend sorted_interval_list operator&(const sorted_interval_list& a, const sorted_interval_list& b);

// Debug print for sorted_interval_list

   friend void debug_print_sil(const sorted_interval_list& a, char* msg);
};

#endif

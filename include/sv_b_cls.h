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
 * SvLis - reference counting, input lists, and parallel locks
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 8 October 1998
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_BASE
#define SVLIS_BASE

#ifdef SV_MSOFT

#include <stdmutex.h>
class sv_lock : public RWSTDMutex
{
public:
  sv_lock() {};
  ~sv_lock() {};
  void shut() { acquire(); };
  void open() { release(); };
};

#else

#ifdef __FreeBSD__
#include <pthread.h>
#endif

// This allows reference counts (and other things)
// to be locked

// Unix lock definition

class sv_lock
{
private:
  pthread_mutex_t mutex;

public:

  sv_lock()
  {
    pthread_mutexattr_t mat;
    if(pthread_mutexattr_init(&mat))
      svlis_error("sv_lock()","can't ititialize mutex attribute", SV_WARNING);
    if(pthread_mutex_init(&mutex, &mat))
      svlis_error("sv_lock()","can't create mutex", SV_WARNING);
  }

  ~sv_lock()
  {
    if(pthread_mutex_destroy(&mutex))
      svlis_error("~sv_lock()","can't destroy mutex", SV_WARNING);
  }

  void shut()
  {
    if(pthread_mutex_lock(&mutex))
      svlis_error("sv_lock.shut()","can't obtain lock", SV_WARNING);
  }

  void open()
  {
    if(pthread_mutex_unlock(&mutex))
      svlis_error("sv_lock.open()","can't release lock", SV_WARNING);
  }


};

#endif

// The svLis reference counting template and base class
// This owes a lot to:

//  More Effective C++ by Scott Meyers, Addison-Wesley, 1996
//  ISBN 0-201-63371-X

// template class for smart pointers-to-T objects; T
// must inherit from sv_refct (see below)

template<class T>
class sv_smart_ptr
{
  private:

   T *Pointee;

   void init() { if (Pointee) Pointee->add_reference(); }

  public:

   sv_smart_ptr(T* realPtr = 0) 
   {
	Pointee = realPtr;
	init();
   }

   sv_smart_ptr(const sv_smart_ptr& rhs)
   {
	Pointee = rhs.Pointee;
	init();
   }

   ~sv_smart_ptr() { if (Pointee) Pointee->remove_reference(); }

   sv_smart_ptr& operator=(const sv_smart_ptr& rhs)
   {
    if (Pointee != rhs.Pointee)
    {
      if (Pointee) Pointee->remove_reference();

      Pointee = rhs.Pointee;
      init();
    }

    return *this;
   }

   int exists() const { return(Pointee != 0); }

   long unique() const { return((long) Pointee); }

   T* operator->() const { return(Pointee); }

   T& operator*() const { return(*Pointee); }
};



// Base class for reference-counted objects
// This also includes the flag word, which is common
// to all such objects, and record locking for
// running on parallel machines.

class sv_refct
{
 protected:

   sv_integer ref_count;  // The reference count
   sv_lock lock;          // Record locking
   sv_integer f;          // Flag bits

   sv_refct()
   {
	ref_count = 0;
	f = 0;
   }

   sv_refct(const sv_refct& rhs)
   {
	ref_count = 0;
	f = 0;
   }

   sv_refct& operator=(const sv_refct& rhs) { return *this; }

   virtual ~sv_refct() {}

  public:

   void add_reference() 
   {
     lock.shut(); 
     ++ref_count;
     lock.open();
   }

   virtual void remove_reference() 
   {
     lock.shut();
     if (--ref_count <= 0) 
     {
         lock.open();
         delete this;
     } else
         lock.open();
   }

   sv_integer flags() { return(f); }

   void set_flags(sv_integer a) 
   {
     lock.shut(); 
     f = f | a; 
     lock.open();
   }

   void reset_flags(sv_integer a)
   {
     lock.shut(); 
     f = f & (~a); 
     lock.open();
   }
};



// Input lookup list template class

#define TP_LEN 10000
#define CACHE 25

template<class T>
class look_up
{
        long* id;                 // Array of unique indices
	T* sv_class;              // Array of svLis items
	sv_integer length;        // Array lengths
	sv_integer sorted;        // Length of sorted section (remainder is unsorted cache)
	sv_integer free;          // Next free location in the arrays
	sv_integer sort_interval; // How frequently do we sort

public:

    look_up() 
    {
      id = new long[TP_LEN + 1];
      sv_class = new T[TP_LEN + 1];
      length = TP_LEN;
      sorted = -1;
      free = 0;
      sort_interval = CACHE;
    }

// Shellsort the arrays on the index up to free; this
// effectively merges in the cache.

    void lu_sort()
    {
       sv_integer mesh,i,j,inow,inext;
       T tsv;
       long tc;

       mesh = (free + 1)/3;

       while(mesh > 0)
       {
          for(i = mesh; i < free; i++)
          {
               inow = i;
               tc = id[i];
               tsv = sv_class[i];
               for(j = mesh; j <= i; j = j + mesh)
               {
                    inext = inow - mesh;
                    if(tc >= id[inext]) goto a;
                    id[inow] = id[inext];
                    sv_class[inow] = sv_class[inext];
                    inow = inext;
               }
a:             id[inow] = tc;
               sv_class[inow] = tsv;
          }
          mesh = (mesh + 1)/3;
       }

       sorted = free - 1;
       sort_interval = sort_interval + CACHE; // Longer till next time
    }

// Do a binary chop to find p_p; if it's not in the sorted
// section, do a linear search of the cache.

    T find(long p_p)
    {
        sv_integer b = 0;
	sv_integer t = sorted;
	sv_integer mid;
	T result;

// Binary chop in sorted section

	if(t > -1) // Not true for unsorted list
	{
	    while( (t - b) > 1)
	    {
		mid = (b + t)/2;
		if(p_p > id[mid])
		  b = mid;
		else
		  t = mid;
	    }
	    if(id[b] == p_p) return(sv_class[b]);
	    if(id[t] == p_p) return(sv_class[t]);
	}

// Linear search in cache

	b = sorted + 1;
	while(b < free)
	{
	    if(id[b] == p_p) return(sv_class[b]);
	    b++;
	}
	return(result);
    }

// Add a new entry on the ends of the lists

    void add(const T& p, long p_p)
    {
         long* idnew;
         T* svnew;
	 T sv_dum;

         id[free] = p_p;
	 sv_class[free] = p;
	 free++;

// Out of room?

	 if(free >= length)
	 {
	     idnew = new long[length + TP_LEN + 1];
	     svnew = new T[length + TP_LEN + 1];
	     for(int i = 0; i < free; i++)
	     {
	       idnew[i] = id[i];
	       svnew[i] = sv_class[i];
	       sv_class[i] = sv_dum;     // Ensure reference counting is OK
	     }
	     delete [] id;
	     delete [] sv_class;
	     id = idnew;
	     sv_class = svnew;
	     length = length + TP_LEN;
	 }

// Time to sort?

	 if( (free - sorted) > sort_interval) lu_sort();
    }

// Set the lists empty
// Note that this does not shorten long lists back to TP_LEN

    void clean()
    {
      T sv_dum;

// Release references

      for(int i = 0; i < free; i++) sv_class[i] = sv_dum;

      sort_interval = CACHE;
      sorted = -1;
      free = 0;
    }

};



#endif

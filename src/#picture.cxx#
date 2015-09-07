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
 * SvLis RAYTRACER - picture function file
 *
 *	Andy Wallis
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


#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "picture.h"
#if macintosh
 #pragma export on
#endif

// Constructors

sv_picture::sv_picture()
{
   x_res = 0;
   y_res = 0;
   mask = 0;
   image_bitmap = 0;
}


// Copy constructor

sv_picture::sv_picture(sv_picture &src)
{
   x_res = src.x_res;
   y_res = src.y_res;
   mask = src.mask;
   image_bitmap = src.image_bitmap;
}

// Destructor

sv_picture::~sv_picture()
{
   if(this->mask) free(this->mask);
   this->mask = 0;

   if(image_bitmap) free(image_bitmap);
   image_bitmap = 0;
}


// Assignment operator	***** Not really needed!

sv_picture sv_picture::operator=(sv_picture &src)
{
// Fix for a = a needed? AB
   x_res = src.x_res;
   y_res = src.y_res;
   mask = src.mask;
   image_bitmap = src.image_bitmap;

   return *this;
}


// Member-setting functions

sv_integer sv_picture::resolution(sv_integer x_r, sv_integer y_r)
{
   x_res = x_r;
   y_res = y_r;
   if(image_bitmap) free(image_bitmap);

   image_bitmap = (sv_pixel*)malloc((x_res+5)*(y_res+5)*sizeof(sv_pixel));
   if(image_bitmap == 0)
      return -1;
   else
      return 0;
}

// Member accessing functions

void sv_picture::pixel(sv_integer ix, sv_integer iy, sv_pixel pixel_colour)
{
   if(!image_bitmap)
      svlis_error("sv_picture::set_pixel",
	"Cannot set pixel - null image bitmap.", SV_WARNING);

   if((ix < 0) || (iy < 0) || (ix >= x_res) || (iy >= y_res))
   svlis_error("sv_picture::set_pixel",
	"Cannot set pixel - coordinates out of range.", SV_WARNING);

   image_bitmap[ix + iy*x_res] = pixel_colour;
}
   

sv_pixel sv_picture::pixel(sv_integer ix, sv_integer iy) const
{
   if(!image_bitmap)
      svlis_error("sv_picture::pixel",
	"Cannot get pixel - null image bitmap.", SV_WARNING);


   if((ix < 0) || (iy < 0) || (ix >= x_res) || (iy >= y_res))
   svlis_error("sv_picture::pixel",
	"Cannot get pixel - coordinates out of range.", SV_WARNING);

   return image_bitmap[ix + iy*x_res];
}

// Set entire picture to a given colour

sv_integer sv_picture::picture_to_colour(sv_pixel colour)
{
   sv_integer ix, iy;

   for(ix=0; ix < x_res; ix++)
      for(iy=0; iy<y_res; iy++)
	 pixel(ix,iy,colour);
   return 0;
}

// Image I/O

#ifdef SV_MSOFT
 static sv_image_type sv_it = SV_BMP;  // Default under Windows
#else
 static sv_image_type sv_it = SV_PPM;  // Default under everything else
#endif

void image_type(sv_image_type t) { sv_it = t; }

sv_image_type image_type() { return(sv_it); }

// Write picture as a ppm file

void write_ppm(ostream& s, const sv_picture* p, const char* comment)
{
   sv_pixel* ptr = p->image_bitmap;
   if(!ptr)
   {
	svlis_error("write_ppm","no image stored",SV_WARNING);
	return;
   }

   sv_integer count = (p->x_res)*(p->y_res);


// Write new header
// Explicitly use newline on all systems

   s << "P6" << '\x0a' << "#" <<
	   comment << '\x0a' <<
	   p->x_res << ' ' <<
	   p->y_res << '\x0a'  <<
	   255 << '\x0a';


// Write image data to file

   unsigned char bf;
   while(count--)
   {
      bf = (unsigned char) (ptr->r);
      s.put(bf);
      bf = (unsigned char) (ptr->g);
      s.put(bf);
      bf = (unsigned char) (ptr->b);
      s.put(bf);
      if(s.rdstate() & ios::badbit)
      {
	svlis_error("write_ppm","file output error",SV_WARNING);
	return;
      }
      ptr++;
   }
   s.flush();
}

// Write a BMP file

void put_short(ostream& ofs, short s)
{
   for(int i = 0; i < 2; i++)
   {
	ofs.put((unsigned char)(s & 0xff));
	s = s >> 8;
   }
}

void put_long(ostream& ofs, long l)
{
   for(int i = 0; i < 4; i++)
   {
	ofs.put((unsigned char)(l & 0xff));
	l = l >> 8;
   }
}

void write_bmp(ostream& s, const sv_picture* p)
{
	long w = p->x_resolution();
	long h = p->y_resolution();
        long rowbytes = ((24*w + 31)/32)*4;
	s.put('B');
	s.put('M');
	put_long(s, 54 + rowbytes*h*3);
	put_short(s, 0);
	put_short(s, 0);
	put_long(s, 54);
	put_long(s, 40);
	put_long(s, w);
	put_long(s, h);
	put_short(s, 1);
	put_short(s, 24);
	put_long(s, 0);
	put_long(s, rowbytes*h*3);
	put_long(s, 0xb6d);
	put_long(s, 0xb6d);
	put_long(s, 0);
	put_long(s, 0);
	sv_pixel px;
        long rowleft = rowbytes - w*3;
	for(long i = 0; i < h; i++)
        {
	  for(long j = 0; j < w; j++)
	  {
		px = p->pixel(j, h-i-1);
		s.put((unsigned char) px.b);
		s.put((unsigned char) px.g);
		s.put((unsigned char) px.r);
	  }
	  if(rowleft > 0) 
            for(int k = 0; k < rowleft; k++) 
              s.put((unsigned char)0);
        }
	s.flush();
}
#if macintosh
 #pragma export off
#endif




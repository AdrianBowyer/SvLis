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
 * SvLis - image/texture map header file
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
 * First version: August 1994
 * This version: 8 March 2000
 *
 */


// The surface class stores surface image/texture map parameters

// This has been much-modified by Adrian Bowyer from the original tex_map.h

#ifndef SVLIS_SFACE
#define SVLIS_SFACE

class sv_surface
{
   sv_real diff_cf;		// Diffuse coefficient 
   sv_point diff_c;		// Diffuse surface colour (RGB)
   sv_real spec_cf;		// Specular reflection coefficient [0...1]
   sv_point spec_c;		// Specular surface colour (RGB)
   sv_real spec_p;		// Power to raise ..
   sv_integer m;       		// Mirror flag
   sv_integer sh;		// Cast shadows on this surface
   sv_integer mst;		// Aerial perspective on this surface
   sv_real attn;		// Reflection attenuation
   sv_real xmiss_cf;		// Transmission coefficient
   sv_picture* tex;		// Texture map
   sv_point mo;			// Map origin
   sv_point mud;		// Map u direction
   sv_point muvs;		// Map u-v size (z not used)
   sv_integer mt;		// Map tiled flag
   sv_integer zxmit;		// (0,0,0) map colour means transparent

public:

// Default constructor

   sv_surface();

// Member setting functions

   void diffuse_coeff(sv_real);
   void diffuse_colour(const sv_point&);
   void specular_coeff(sv_real);
   void specular_power(sv_real);
   void specular_colour(const sv_point&);
   void mirror(int);
   void shadow(int);
   void mist(int);
   void attenuation(sv_real);
   void transmission(sv_real);
   void texture(sv_picture*);
   void map_origin(const sv_point&);
   void map_u(const sv_point&);
   void map_size(const sv_point&);
   void map_type(sv_integer);
   void map_0_xmit(sv_integer);

// Member accessing functions

   sv_real diffuse_coeff(void) const;
   sv_point diffuse_colour(void) const;
   sv_real specular_coeff(void) const;
   sv_real specular_angle_power(void) const;
   sv_point specular_colour(void) const;
   sv_integer mirror(void) const;
   sv_integer shadow(void) const;
   sv_integer mist(void) const;
   sv_real attenuation(void) const;
   sv_real transmission(void) const;
   sv_picture* texture(void) const;
   sv_point map_origin(void) const;
   sv_point map_u_dir(void) const;
   sv_point map_uv_size(void) const;
   sv_integer map_type(void) const;
   sv_integer map_0_xmit(void) const;

// I/O

   friend ostream& operator<<(ostream&, const sv_surface&);
   friend istream& operator>>(istream&, sv_surface&);
   friend void write(ostream&, const sv_surface&, sv_integer);
   friend void read(istream&, sv_surface&);
   friend void read1(istream&, sv_surface&);

// Tag value

   sv_integer tag() const;

// Debug print for surface

   friend void debug_print_surface(const sv_surface&, char*);


// The following are retained for backwards compatibility

// Member-setting functions

   void set_diffuse_coeff(sv_real);
   void set_diffuse_colour(const sv_point&);
   void set_specular_coeff(sv_real);
   void set_specular_power(sv_real);
   void set_specular_colour(const sv_point&);
   void set_mirror(int);
   void set_shadow(int);
   void set_mist(int);
   void set_attenuation(sv_real);
   void set_transmission(sv_real);
   void set_texture(sv_picture*);
   void set_map_origin(const sv_point&);
   void set_map_u(const sv_point&);
   void set_map_size(const sv_point&);
   void set_map_tiled(sv_integer);
   void set_map_0_xmit(sv_integer);

};


// The following procedures are user-extendible in sv_user/surface.cxx

void get_tex_map(const sv_set&, const sv_surface&, 
	const sv_point&, const sv_point&,
	sv_integer&, sv_integer&);


void get_tex_map(const sv_set&, const sv_surface&, 
	const sv_point&, const sv_point&,
	sv_real&, sv_real&);

void get_solid_tex(const sv_set&, const sv_surface&, 
	const sv_point&, const sv_point&,
	sv_point*);
#endif




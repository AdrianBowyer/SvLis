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
 * SvLis - Surfaces of sets
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


// This has been much-modified by Adrian from tex_map.cxx

#include "svlis.h"
#if macintosh
 #pragma export on
#endif

sv_surface::sv_surface()
{
	diff_cf = 1.0;
	diff_c = DEF_COL;
	spec_cf = 0;
	spec_c = diff_c;
	spec_p = 1;
	m = 0;
	sh = 0;
	mst = 0;
	attn = 0.9;
	xmiss_cf = 0;
	tex = 0;
	mud = sv_point(1,0,0);
	muvs = sv_point(1,1,0);
	mt = 0;
	zxmit = 0;
}


// Member-setting functions

void sv_surface::diffuse_coeff(sv_real drc){this->diff_cf = drc;}
void sv_surface::diffuse_colour(const sv_point& dc){this->diff_c = dc;}
void sv_surface::specular_coeff(sv_real src){this->spec_cf = src;}
void sv_surface::specular_power(sv_real sap){this->spec_p = sap;}
void sv_surface::specular_colour(const sv_point& sc){this->spec_c = sc;}
void sv_surface::mirror(int mv){this->m = mv;}
void sv_surface::shadow(int sv){this->sh = sv;}
void sv_surface::mist(int mv) {this->mst = mv;}
void sv_surface::attenuation(sv_real a) {this->attn = a;}
void sv_surface::transmission(sv_real t) {this->xmiss_cf = t;}
void sv_surface::texture(sv_picture* t) {this->tex = t;}
void sv_surface::map_origin(const sv_point& o) {this->mo = o;}
void sv_surface::map_u(const sv_point& u) {this->mud = u;}
void sv_surface::map_size(const sv_point& s) {this->muvs = s;}
void sv_surface::map_type(sv_integer t) {this->mt = t;}
void sv_surface::map_0_xmit(sv_integer x) {this->zxmit = x;}

// Member accessing functions

sv_real sv_surface::diffuse_coeff(void) const {return(this->diff_cf);}
sv_point sv_surface::diffuse_colour(void) const {return(this->diff_c);}
sv_real sv_surface::specular_coeff(void) const {return(this->spec_cf);}
sv_real sv_surface::specular_angle_power(void) const {return(this->spec_p);}
sv_point sv_surface::specular_colour(void) const {return(this->spec_c);}
sv_integer sv_surface::mirror(void) const {return(this->m);}
sv_integer sv_surface::shadow(void) const {return(this->sh);}
sv_integer sv_surface::mist(void) const {return(this->mst);}
sv_real sv_surface::attenuation(void) const {return(this->attn);}
sv_real sv_surface::transmission(void) const {return(this->xmiss_cf);}
sv_picture* sv_surface::texture(void) const {return(this->tex);}
sv_point sv_surface::map_origin(void) const {return(this->mo);}
sv_point sv_surface::map_u_dir(void) const {return(this->mud);}
sv_point sv_surface::map_uv_size(void) const {return(this->muvs);}
sv_integer sv_surface::map_type(void) const {return(this->mt);}
sv_integer sv_surface::map_0_xmit(void) const {return(this->zxmit);}


// Unique tag

sv_integer sv_surface::tag() const
{
        return(SVT_F*SVT_SURFACE);
}

// ------------- The following are retained for backwards compatibility --------------------

void sv_surface::set_diffuse_coeff(sv_real drc){this->diff_cf = drc;}
void sv_surface::set_diffuse_colour(const sv_point& dc){this->diff_c = dc;}
void sv_surface::set_specular_coeff(sv_real src){this->spec_cf = src;}
void sv_surface::set_specular_power(sv_real sap){this->spec_p = sap;}
void sv_surface::set_specular_colour(const sv_point& sc){this->spec_c = sc;}
void sv_surface::set_mirror(int mv){this->m = mv;}
void sv_surface::set_shadow(int sv){this->sh = sv;}
void sv_surface::set_mist(int mv) {this->mst = mv;}
void sv_surface::set_attenuation(sv_real a) {this->attn = a;}
void sv_surface::set_transmission(sv_real t) {this->xmiss_cf = t;}
void sv_surface::set_texture(sv_picture* t) {this->tex = t;}
void sv_surface::set_map_origin(const sv_point& o) {this->mo = o;}
void sv_surface::set_map_u(const sv_point& u) {this->mud = u;}
void sv_surface::set_map_size(const sv_point& s) {this->muvs = s;}
void sv_surface::set_map_tiled(sv_integer t) {this->mt = t;}
void sv_surface::set_map_0_xmit(sv_integer x) {this->zxmit = x;}

// ------------------------------------------------------------------------------------------

// I/O

void write(ostream& s, const sv_surface& surf, sv_integer level)
{
        put_white(s, level);
	put_token(s, SVT_SURFACE, 0, 0);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_OB, 0, 0);
	s << SV_EL;
	sv_integer nxl = level+1;

	writer(s, surf.diff_cf, nxl); s << SV_EL;
	write(s, surf.diff_c, nxl); s << SV_EL;
	writer(s, surf.spec_cf, nxl); s << SV_EL;
	write(s, surf.spec_c, nxl); s << SV_EL;
	writer(s, surf.spec_p, nxl); s << SV_EL;
	writei(s, surf.m, nxl); s << SV_EL;
	writei(s, surf.sh, nxl); s << SV_EL;
        writei(s, surf.mst, nxl); s << SV_EL;
	writer(s, surf.attn, nxl); s << SV_EL;
	writer(s, surf.xmiss_cf, nxl); s << SV_EL;
	write(s, surf.mo, nxl); s << SV_EL;
	write(s, surf.mud, nxl); s << SV_EL;
	write(s, surf.muvs, nxl); s << SV_EL;
	writei(s, surf.mt, nxl); s << SV_EL;
	writei(s, surf.zxmit, nxl); s << SV_EL;

// Always record images in svLis files as ppm

	if (surf.tex)
	{
		writei(s, 1, nxl); s << SV_EL;
		write_ppm(s, surf.tex, "svLis surface texture");
	} else
		writei(s, 0, nxl);
	s << SV_EL;
	put_white(s, level);
	put_token(s, SVT_CB, 0, 0);
	s << SV_EL;
}

ostream& operator<<(ostream& s, const sv_surface& surf)
{
	write(s, surf, 0);
	return(s);
}

void read(istream& s, sv_surface& surf)
{

	if(get_read_version() != get_svlis_version())
	{
		if(get_read_version() != (get_svlis_version()-1))
		  svlis_error("read(.. sv_surface)",
		  "file version is too old",
		  SV_WARNING);
		read1(s, surf);
		return;
	}
	sv_integer id;
	sv_real rd;

        if (check_token(s, SVT_SURFACE))
        {
		check_token(s, SVT_OB);
		get_token(s, id, surf.diff_cf, 0);
		read(s, surf.diff_c);
		get_token(s, id, surf.spec_cf, 0);
		read(s, surf.spec_c);
		get_token(s, id, surf.spec_p, 0);
		get_token(s, surf.m, rd, 1);
		get_token(s, surf.sh, rd, 1);
		get_token(s, surf.mst, rd, 1);
		get_token(s, id, surf.attn, 0);
		get_token(s, id, surf.xmiss_cf, 0);
		read(s, surf.mo);
		read(s, surf.mud);
		read(s, surf.muvs);
		get_token(s, surf.mt, rd, 1);
		get_token(s, surf.zxmit, rd, 1);
		get_token(s, id, rd, 1);
		if (id)
			read(s, &(surf.tex));
		else
			surf.tex = 0;
                check_token(s, SVT_CB);
        }
}

// Svlis Version 2 surface read

void read1(istream& s, sv_surface& surf)
{
	sv_integer id;
	sv_real rd;

        if (check_token(s, SVT_SURFACE))
        {
		check_token(s, SVT_OB);
		get_token(s, id, surf.diff_cf, 0);
		read1(s, surf.diff_c);
		get_token(s, id, surf.spec_cf, 0);
		read1(s, surf.spec_c);
		get_token(s, id, surf.spec_p, 0);
		get_token(s, surf.m, rd, 1);
		get_token(s, surf.sh, rd, 1);
		get_token(s, surf.mst, rd, 1);
		get_token(s, id, surf.attn, 0);
		get_token(s, id, surf.xmiss_cf, 0);
		read1(s, surf.mo);
		read1(s, surf.mud);
		read1(s, surf.muvs);
		get_token(s, surf.mt, rd, 1);
		get_token(s, surf.zxmit, rd, 1);
		get_token(s, id, rd, 1);
		if (id)
			read1(s, &(surf.tex));
		else
			surf.tex = 0;
                check_token(s, SVT_CB);
        }
}

istream& operator>>(istream& s, sv_surface& surf)
{
	read(s, surf);
	return(s);
}

// Debug print for surface

void debug_print_surface(const sv_surface& surf, char* msg)
{
   cout << "----- surface ";
   if(msg) cout << msg;
   cout << " (at " << (long)&surf << ") is:\n";
   cout << "  diffuse_coeff: " << surf.diff_cf << "\n";
   cout << "  diffuse_colour: " << surf.diff_c.x << ", " << surf.diff_c.y 
		<< ", " << surf.diff_c.z << "\n";
   cout << "  specular_coeff: " << surf.spec_cf << "\n";
   cout << "  specular_power: " << surf.spec_p << "\n";
   cout << "  specular_colour: " << surf.spec_c.x << ", " << surf.spec_c.y 
		<< ", " << surf.spec_c.z << "\n";
   cout << "  mirror: " << surf.m << "\n";
   cout << "  shadow: " << surf.sh << "\n";
   cout << "  mist: " << surf.mst << "\n";
   cout << "  attenuation: " << surf.attn << "\n";
   cout << "-----\n\n";
}

// Texture map a surface for the ray tracer

void get_tex_map(const sv_set& hit_surface, const sv_surface& surf, 
	const sv_point& hit_point, const sv_point& surface_normal,
	sv_real& u, sv_real& v)
{

// Calculate coordinates of the hit point in the image map

	sv_real ur, vr;
	sv_primitive prim = hit_surface.primitive();
	sv_point map_v_dir, hit_p;
	prim_op op;
	sv_integer k;
	sv_real r0, r1, r2;
	sv_plane f;
	sv_point cen;
	sv_line axis;
	sv_point ax_0;
	sv_point ax_d;
	sv_point a0,a1;
	sv_real t0,t1;
	switch(prim.kind())
	{
	case SV_PLANE:	
		hit_p = hit_point - surf.map_origin();
		map_v_dir = surf.map_u_dir()^surface_normal;
		u = (hit_p*surf.map_u_dir())/
			surf.map_uv_size().x;
		v = hit_p*map_v_dir/surf.map_uv_size().y;
		break;

       case SV_CYLINDER:

// u direction is ALWAYS around circumference of cylinder
// v direction is ALWAYS along axis of cylinder
// closest point of cylinder surface to map_origin defines (u=0, v=0) location

		op = prim.parameters(&k, &r0, &r1, &r2, &f, &cen, &axis);
		ax_0 = axis.origin;
		ax_d = axis.direction;
		t0 = (surf.map_origin() - ax_0)*ax_d;
		t1 = (hit_point - ax_0)*ax_d;
		vr = t1 - t0;
		a0 = surf.map_origin() - line_point(axis,t0);
		a1 = hit_point - line_point(axis,t1);
		a0 = a0.norm();
		a1 = a1.norm();
		t0 = ((a0^a1) + ax_d).mod();
		ur = acos(a0*a1);
		if (fabs(t0) < 0.1) ur = 2*M_PI - ur;
		ur = ur*r0;
		u = ur / surf.map_uv_size().x;
		v = vr / surf.map_uv_size().y;
		break;

// Add your surface parameterisations here...

	case SV_SPHERE:
	case SV_CONE:
	case SV_TORUS:
	case SV_CYCLIDE:
	case SV_GENERAL:
	case SV_REAL:
	default:   
	 	svlis_error("get_tex_map()",
			"cannot map surface texture/image onto this type of surface", 
	 		SV_WARNING);
		break;
	}
}

void get_tex_map(const sv_set& hit_surface, const sv_surface& surf, 
	const sv_point& hit_point, const sv_point& surface_normal,
	sv_integer& u, sv_integer& v)
{
	sv_real ur, vr;
        sv_integer map_u_res = (surf.texture())->x_resolution();
        sv_integer map_v_res = (surf.texture())->y_resolution();

	get_tex_map(hit_surface, surf, hit_point, surface_normal, ur, vr);
	u = sv_integer(sv_real(map_u_res)*ur);
	v = map_v_res - 1 - sv_integer(sv_real(map_v_res)*vr);
}


// All right - I know it's silly to have a _solid_ texture function
// in a _surface_ class...

void get_solid_tex(const sv_set& hit_surface, const sv_surface& surf, 
	const sv_point& hit_point, const sv_point& surface_normal,
	sv_point* diffuse_colour)
{

}

#if macintosh
 #pragma export off
#endif


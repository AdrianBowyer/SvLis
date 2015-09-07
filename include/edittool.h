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
 * SvLis - device-independent model editor
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 30 April 1995
 * This version: 8 March 2000
 *
 */

// edittool is a set of definitions and procedures that keep track
// of a svLis model as it is interactively constructed.  They can be
// called by a GUI or by a text-menu program (or by anything else, for
// that matter).  They maintain a consistent structure so that models
// can be built, edited, undone, and saved to and loaded from disc.

// The source code is in programs/sv_edit/edittool.cxx

#ifndef SVLIS_EDIT_TOOL
#define SVLIS_EDIT_TOOL

// Editing procedures      
       
extern void sv_edit_init();
extern void sv_edit_close();
extern void redraw();
extern void set_redivide_draw();
extern void box_redivide_draw();
extern void home_e_view();
extern void e_view_change(const sv_view&);
extern void spin();
extern void e_view_left();
extern void e_view_right();
extern void e_view_up();
extern void e_view_down();
extern void e_view_forward();
extern void e_view_back();
extern void add_set(const sv_set&, sv_ed_op);
extern void model_change(const sv_set&);
extern void checksave();
extern void printstats(char*, ostream&);
extern sv_surface current_surface();
extern void new_surface(const sv_surface&);
extern sv_box current_box();
extern void new_box(const sv_box&,  sv_integer);
extern void new_cuboid(const sv_box&);
extern sv_box current_cuboid();
extern void new_cyl(const sv_line&, sv_real);
extern void current_cyl(sv_line*, sv_real*);
extern void new_sphere(const sv_point&, sv_real);
extern void current_sphere(sv_point*, sv_real*);
extern void new_plane(const sv_plane&);
extern sv_plane current_plane();
extern char* current_name();
extern sv_model current_model();
extern sv_model current_divided_model();
extern void new_name(char*);
extern void undo();
extern void redo();
extern void cleansheet();
extern sv_model model2save();
extern void m_vol(sv_real, sv_real&, sv_point&, sv_point&, sv_point&);
extern sv_real m_area();
extern sv_integer get_pic_x();
extern sv_integer get_pic_y();
extern sv_light_list* get_lamp_list();
extern void set_lamp_list(sv_light_list*);
extern char* get_pic_filename();
extern void set_pic_filename(char*);    
extern sv_integer get_quickview();
extern void set_quickview(sv_integer);
extern sv_real get_report_step();
extern void set_report_step(sv_real);
extern mem_test membership(const sv_point&);
extern sv_set raytrace(const sv_line&, sv_real*);
extern sv_integer sv_edit_message(char*, int); // from the driver program
extern void save_model(const sv_model& m);
extern void plot_box(sv_integer);
extern sv_line get_ray();
extern void update_lamplist(char*,const sv_lightsource&);
extern void update_lamplist(char*, light_type, sv_point, sv_point, sv_real, 
    sv_point, sv_real);
extern sv_integer get_faceting();
extern void set_faceting(sv_integer);
extern sv_integer get_raytracing();
extern void set_raytracing(sv_integer);
extern sv_view get_view();
extern sv_integer get_low();
extern void set_low(sv_integer);
extern sv_real get_little();
extern void set_little(sv_real);
extern sv_integer get_pic_x(void);
extern sv_integer get_pic_y(void);
extern void set_pic_resolution(sv_integer, sv_integer);

// from the driver program

extern void model2load(const sv_model&);
extern void prompt(char*);
extern void cprompt(char*);
extern void eprompt(char*);
extern void r_to_str(char*,  sv_real);
extern void p_to_str(char*,  const sv_point&);

// Structure for one instant in the model-building process.

struct instant
{
	sv_model m;  
	instant* previous;
	instant* next;
	
	instant(const sv_model& mm, instant* p, instant* n)
	{
		m = mm;
		previous = p;
		next = n;
	}
	
	sv_box box() { return(m.box()); }
	sv_set set() { return(m.set_list().set()); }
};

// Current state of the world

struct state
{
	instant* cur_inst;	    // Where we are now in the list
	sv_model divided_model_r;   // Current ray divided model
	sv_model divided_model_f;   // Current faceted divided model
	sv_surface surf;	    // Current surface
	sv_box cur_cuboid;	    // Current cuboid
	sv_line cyl_axis;	    // Current cylinder
	sv_real radius;
	sv_point sph_cen;	    // Current centre
	sv_plane cur_plane;	    // Current plane
	sv_integer plt_b;	    // Plotting boxes?
	sv_integer modified;	    // Set whenever the model is modified
	char* filename;		    // Current file name for the model
	sv_integer ray_render;	    // Pictures by raytracing?
	sv_integer facet_render;    // and/or by faceting?
    sv_picture pic;		    // Ray-traced image
	char* temp_pic_filename;    // File for temp picture
	char* pic_filename;	    // File for final picture	
	sv_light_list *lamp_list;// List of lights
	sv_integer start_time;	    // Start of raytracing
	sv_integer quickview;	    // coarse -> fine raytrace
	sv_real report_step;	    // Frequency of updates
	sv_view vw;		    // Eye pos etc.
	sv_integer low_contents;    // For the dumb divider
	sv_real little_box;
#ifdef SV_UNIX
	pid_t xv_pid;               // xv process i.d.
#endif	
				
	state()   // Constructor
	{
	    cur_inst = new instant( sv_model(sv_set_list(
		DEF_SET.colour(DEF_COL)), DEF_BOX, LEAF_M,  sv_model()), 0, 0 );
	    cur_cuboid = DEF_CUBOID;
	    modified = 0;
	    surf = sv_surface();
	    filename = new char[sv_strlen(DEF_NAME) + sv_strlen(DEF_EXTN) + 3];
	    sv_strcpy(filename, DEF_NAME);
	    sv_strcat(filename,  DEF_EXTN);
	    cyl_axis = DEF_AXIS;
	    radius = DEF_RAD;
	    sph_cen = DEF_CEN;
	    cur_plane = DEF_PLANE;
	    plt_b = 0;
	    ray_render = 0;
	    facet_render = 1;
	    lamp_list = 0;
	    quickview = 1;
	    report_step = 5;
	    sv_view v;
	    vw = v;
	    pic_filename = new char[sv_strlen(DEF_NAME) + sv_strlen(PIC_EXTN) +3];
	    sv_strcpy(pic_filename, DEF_NAME);
	    sv_strcat(pic_filename,  PIC_EXTN);
	    pic.resolution(DEF_PIC_X, DEF_PIC_Y);
	    low_contents = 3;
	    little_box = 1;
		divided_model_r = sv_model(sv_set(SV_EVERYTHING),cur_cuboid);
		divided_model_f = divided_model_r;
	}
};

#endif





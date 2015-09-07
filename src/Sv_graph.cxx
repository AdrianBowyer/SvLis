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
 * SvLis graphics interface for OpenGL
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 20 May 1993
 * This version: 9 November 2000
 *
 */

 
#include <svlis.h> 
#include "sv_cols.h" 
#if macintosh 
 #pragma export on 
 #define MENU_SEP "-" 
#else 
 #define MENU_SEP "" 
#endif 


#define RAY_PIK 1
#define TRANS_PIK 2
 
//************************************************************************************ 
 
// Globals used by all graphics software 


static int thread_running = 0;			// Is there an active GL window?
static int to_user = 0;                         // Flag for release_display_list
static int force_recompile = 0;			// Force a recompilation 
static sv_model null_mod;			// Null model used to release storage
static sv_box enclosure;			// Encloses everything 
static sv_real box_d = 1.0;			// The size of the enclosure box diagonal 
static sv_real user_cross = 0.02;       	// Fraction of box_d that is the size of a cross 
static sv_integer plot_empties = 1;		// Plot boxes with a contents of 0? 
static sv_integer wireframe=0;			// Wireframe or solid -uses GL_POLYGON_MODE 
static sv_integer plot_b=0;			// Plot the faceter's boxes 
static sv_integer texturing=0;			// Switches texturing on/off 
static sv_integer smoothing=0;			// Switches smoothing on/off 
static sv_integer face_culling=1;		// Toggles face culling 
static sv_integer fog=0;			// Toggles aerial perspective 
static sv_integer ortho = 0;			// Orthographic/perspective projection 
static sv_real fog_d = 1;			// Fog density 
static char* win_name = 0;			// Window title 
static int lb=0, mb=0;				// State of the mouse buttons 
static sv_integer mouse_count=0;                // Mouse interrupt throwaway
static sv_integer mouse_top=5;                  // How many to throw away
static int mx_0=0, my_0=0;			// Last mouse coordinates (some button down) 
static int raytrace = 0;			// Trace a ray at the next mouse click 
static sv_real ww, wh;				// Window width and height 
static sv_point bgc = BG_COL;			// Background colour 
static int bgc_num = 4;     	    	    	// Number of current background colour 
const int bgc_cnt = 4;	    	    	    	// Total number of colours 
static mouse_mode_t mouse_mode = none;		// The current mode 
static GLdouble aspect_r = 1;			// Aspect ratio of the window 
static GLdouble near_clip = 1;			// Clipping planes 
static GLdouble far_clip = 500; 
static GLdouble lens_y = 50;			// Screen-y subtended angle in degrees
static int cursor =  GLUT_CURSOR_LEFT_ARROW;    // The current cursor
 
static char* view_file = 0;				// The view paste file 
 
static int win_id;					// The curreent window ID 
static int newModel=1;
static int spinning = 0;				// Is it spinning? 
static float curquat[4];				// Current quaternion 
static float lastquat[4]; 
 
static int sspinning[5];				// stored versions of the above 
static float scurquat[5][4]; 
static float slastquat[5][4];

// Display list lock

static sv_lock dl_lock;

// Need these four prototypes

void animate(void);
void Re_Draw(void);
void callbacks(void);
void set_idle_func(int);

// The file name root for recording animated sequences

static char* anim_seq = 0;
static sv_integer frame_count = 1;

void sv_animate_root(char* ar, sv_integer rst) 
{
  delete [] anim_seq;
  if(ar)
  {
    anim_seq = new char[sv_strlen(ar)+1];
    sv_strcpy(anim_seq, ar);
  } else
    anim_seq = 0;
  if(rst)frame_count = 1;
}

char* sv_animate_root() {return anim_seq;}

// Function to change the size of a cross 
 
void set_user_cross(sv_real uc) { user_cross = uc; } 
 
// Set the background colour
 
void set_background(const sv_point& b) { bgc = b; } 
 
// OpenGL transform 
 
static GLfloat view_xt=0, view_yt=0, view_zt=0; 
static GLfloat cen_x=0, cen_y=0, cen_z=0; 
 
static GLfloat sview_xt[5], sview_yt[5], sview_zt[5]; 
 
 
// Index of the polygons put into OpenGL

static GLuint svv_as_gl = 0,    // Voronoi diagram
              decs_as_gl = 0;   // Decorations (axes etc)
 
// The faceted model being plotted, and the model with its boxes to be plotted 
// and the transparent model, and the saved model 
 
static sv_model box_mod, t_mod, save_mod;

static  sv_model_list* svm = 0;

// Return the current display list

sv_model_list* all_the_models() { return svm; }

static  sv_model_list* last_picked = 0;
static  sv_model_list* svml_trans = 0;

// Voronoi stuff - to be tidied... 
 
static sv_voronoi svv; 
 
static sv_integer plot_v_x = 0; 
static sv_integer plot_v_v = 0; 
static sv_integer plot_v_d = 0; 
static sv_integer supress_internal = 0; 
static sv_real tet_fac = 0.8; 
 
// Flag for plotting the axes 
 
static sv_integer plot_axes = 0; 

//********************************************************************************************************

// Thread handling
 
// Mac threads (RRM) 
//==================
 
#if macintosh 
 
static ThreadID thethreadID; // Added by RRM for Mac 
 
pascal void *aThread( void *threadParameter ) 
{ 
	sv_run_glut(nil); 
	return nil;
} 
 
void branch_proc() 
{
	thread_running = 1; 

        if( NewThread( kCooperativeThread, aThread, nil, 500000L, 
                kCreateIfNeeded, nil, &thethreadID) != noErr )
	{
		thread_running = 0; 
                svlis_error("branch_proc()","Got an error creating a Mac thread",
                        SV_WARNING);
	}
	YieldToThread(thethreadID);  
} 
 
void kill_proc()  
{ 
 	thread_running = 0;
 	sv_delete_picture_list();
 	if(glIsList(svv_as_gl)) glDeleteLists(svv_as_gl, 1);
	box_mod = null_mod;
	glutDestroyWindow(win_id);
	DisposeThread(thethreadID,nil,false); 
} 

void get_display_list() {  } 
void release_display_list() { YieldToAnyThread(); }
 
#endif  
 
// Microsoft Visual C++ threads
//============================= 
 
#ifdef SV_MSOFT 
 
 void branch_proc() 
 {
	thread_running = 1;
	if(_beginthread(sv_run_glut,0,0) == -1)
	{
		thread_running = 0;
		svlis_error("branch_proc","Microsoft thread failed", SV_FATAL);
	}
 } 
 
 void kill_proc() 
 {
	thread_running = 0; 
	sv_delete_picture_list(); 
	if(glIsList(svv_as_gl)) glDeleteLists(svv_as_gl, 1);
	box_mod = null_mod;
	glutDestroyWindow(win_id); 
	_endthread(); 
 }
 
 void get_display_list() { dl_lock.shut(); } 
 void release_display_list() { to_user = 0; dl_lock.open(); } 

#endif

#ifdef SV_UNIX 
 
// Unix threads
//=============

 static pthread_attr_t attr;
 static pthread_t th_1;
 static int thrd;

 void branch_proc() 
 { 
	if (thrd = pthread_attr_init(&attr))
              svlis_error("branch_proc","can't initialise Unix thread attribute",
		SV_WARNING);
	thread_running = 1;
	if( thrd = pthread_create(&th_1, &attr, (void *(*)(void *))sv_run_glut, 0) )
	{
	      thread_running = 0;
              svlis_error("branch_proc","can't initialise Unix thread",SV_WARNING);
	}
 }

 void kill_proc() 
 {
	thread_running = 0;
	sv_delete_picture_list();	// Release storage 
	box_mod = null_mod;	
	glutDestroyWindow(win_id);
	pthread_exit(0);
 }

 void get_display_list() { dl_lock.shut(); } 
 void release_display_list() { to_user = 0; dl_lock.open(); } 
 
#endif 

//**************************************************************************************************

// View and other transforms
 
 
// Invert the view transform for a point in the window 
 
sv_line inverse_transform(int x, int y) 
{ 
	sv_line result; 
	sv_point e = sv_point(0,0,box_d); 
	sv_line ra; 
	sv_real phi; 
	sv_point o; 
 
	quaternionToAxis(curquat, ra, phi); 
 
	o.x = ((sv_real)x - 0.5*(sv_real)ww)/(sv_real)wh;  // NB _not_ ww 
	o.y = (0.5*(sv_real)wh - (sv_real)y)/(sv_real)wh; 
	o.z = box_d - 0.5/tan(lens_y*M_PI/360); 
 
	o = o - sv_point(view_xt, view_yt, view_zt); 
	o = o.spin(ra, phi); 
	o = o - sv_point(cen_x, cen_y, cen_z); 
 
	e = e - sv_point(view_xt, view_yt, view_zt); 
	e = e.spin(ra, phi); 
	e = e - sv_point(cen_x, cen_y, cen_z); 
 
	result = sv_line(o - e, e); 
 
	return(result); 
}
 
// Set and store the view position 
 
void set_view(int view) 
{ 
	scurquat[view][0] = curquat[0]; 
	scurquat[view][1] = curquat[1]; 
	scurquat[view][2] = curquat[2]; 
	scurquat[view][3] = curquat[3]; 
	slastquat[view][0] = lastquat[0]; 
	slastquat[view][1] = lastquat[1]; 
	slastquat[view][2] = lastquat[2]; 
	slastquat[view][3] = lastquat[3]; 
	sspinning[view] = spinning; 
 
	sview_xt[view] = view_xt; 
	sview_yt[view] = view_yt; 
	sview_zt[view] = view_zt; 
} 
 
// Restore view position 
 
void get_view(int view) 
{ 
	curquat[0] = scurquat[view][0]; 
	curquat[1] = scurquat[view][1]; 
	curquat[2] = scurquat[view][2]; 
	curquat[3] = scurquat[view][3]; 
	lastquat[0] = slastquat[view][0]; 
	lastquat[1] = slastquat[view][1]; 
	lastquat[2] = slastquat[view][2]; 
	lastquat[3] = slastquat[view][3]; 
	spinning = sspinning[view]; 
 
	view_xt = sview_xt[view]; 
	view_yt = sview_yt[view]; 
	view_zt = sview_zt[view]; 
	sv_draw(1); 
}

// Default model transform callback

void xform_model(sv_model_list* ml, const sv_point& p, const sv_line& ax, sv_real ang, sv_xform_action a)
{
        if(a != SV_TRANSFORM)
            svlis_error("xform_model","attempt to do other than transform", SV_WARNING);
	ml->new_transform(p, ax, ang);
	sv_draw_all();
}

// Some new transforms have been specified by the user in the graphics window

void new_transforms(float q[4], const sv_point& p)
{
        void (*trns_cb)(sv_model_list*, const sv_point&, const sv_line&, sv_real, sv_xform_action);
	
// Transforming a single model?...

	if(svml_trans)
	{
		trns_cb = svml_trans->call_back();
		if(trns_cb) 
		{
			float neg_quat[4];
			sv_line ax;
			sv_real ang;
			quaternionToAxis(q, ax, ang);
			axisToQuaternion(ax, -ang*5, neg_quat); // 5 is a scaling hack
			ax = svml_trans->axis();
			ang = svml_trans->angle();
			float new_quat[4];
			axisToQuaternion(ax, ang, new_quat);
			add_quats(neg_quat, new_quat, new_quat);
			quaternionToAxis(new_quat, ax, ang);
			ax = ax + svml_trans->axis().origin;
			sv_point q = svml_trans->offset() + p;
			trns_cb(svml_trans, q, ax, ang, SV_TRANSFORM);
		}
	} else
	{

// ... or the whole scene
	
		add_quats(q, curquat, curquat);

		view_xt += p.x;
		view_yt += p.y;
		view_zt += p.z;

		sv_draw(1);
	}
} 
 
// Set the location for the view file 
 
void set_view_file() 
{ 
#ifdef SV_UNIX 
	char* henv = getenv("HOME"); 
 
	mode_t mode = 0700; 
 
	if(!henv) 
	{ 
		svlis_error("set_view_file()", "can't find home directory name", SV_WARNING); 
		return; 
	} 
 
	view_file = new char[sv_strlen(henv) + sv_strlen("/.svlis/svlis_vw") + 2]; 
 
	sv_strcpy(view_file, henv); 
	sv_strcat(view_file, "/.svlis"); 
	if(access(view_file, W_OK)) 
		if(mkdir(view_file, mode)) 
		{ 
			svlis_error("set_view_file()", "can't create/access $HOME/.svlis", SV_WARNING); 
			return; 
		} else 
			svlis_error("set_view_file()", "creating $HOME/.svlis", SV_WARNING); 
 
	sv_strcat(view_file, "/svlis_vw"); 
#endif 
 
#if macintosh 
 
	view_file = new char[sv_strlen("::results:svlis_vw") + 2]; 
	sv_strcpy(view_file, "::results:svlis_vw"); 
 
#endif 
 
#ifdef SV_MSOFT 
 
	view_file = new char[sv_strlen("c:/svlis_vw") + 2]; 
	sv_strcpy(view_file, "c:/svlis_vw"); 
 
#endif 
} 
 
// Write view positions

void write_views() 
{ 
	if(!view_file) set_view_file(); 
 
	ofstream op(view_file); 
	if(!op) 
	{ 
		svlis_error("write_views","can't open view file",SV_WARNING); 
		return; 
	}
        int i, view;
	for(view = 0; view < 5; view++)
	{
 
	  for(i = 0; i < 4; i++) op << scurquat[view][i] << ' ' << 
				       slastquat[view][i] << SV_EL; 
	  op << sspinning[view] << SV_EL; 
 
	  op << sview_xt[view] << ' '; 
	  op << sview_yt[view] << ' '; 
	  op << sview_zt[view] << SV_EL;
	}
	op.close(); 
} 
 
// Read view positions 
 
void read_views()
{ 
	if(!view_file) set_view_file(); 
 
	ifstream ip(view_file); 
	if(!ip) 
	{ 
		svlis_error("read_views","can't open view file",SV_WARNING); 
		return; 
	}
        int i, view;
	for(view = 0; view < 5; view++)
	{ 
	  for(i = 0; i < 4; i++)  
	  { 
		ip >> scurquat[view][i];  
		ip >> slastquat[view][i]; 
	  } 
	  ip >> sspinning[view]; 
 
	  ip >> sview_xt[view]; 
	  ip >> sview_yt[view]; 
	  ip >> sview_zt[view];
	}
	ip.close();
} 
 
 
// Home position of the transforms 
 
void home() 
{ 
	sv_point c = enclosure.centroid(); 
	cen_x = -c.x; 
	cen_y = -c.y; 
	cen_z = -c.z; 
	view_xt = 0; 
	view_yt = 0; 
	view_zt = 0; 
 
	trackball(curquat, 0.0, 0.0, 0.0, 0.0); 
	trackball(lastquat, 0.0, 0.0, 0.0, 0.0); 
	spinning = 0; 
} 

// Initialize and return the enclosing box

void sv_enclosure(const sv_box& b)
{
	enclosure = b;
	box_d = sqrt(b.diag_sq());
}

sv_box sv_enclosure() { return(enclosure); }
 
//****************************************************************************************************
 
// Ray-tracing 
 
static sv_picture ray_pic; 
static char* ray_pic_file = 0; 
static sv_view view; 
static sv_light_list* lights = 0; 
 
#ifdef SV_UNIX 
 static pid_t xv_pid = 0; 
 static char* xv_command = "/usr/local/bin/xv"; 
#endif 
 
static char* comment = "Raytraced by svLis; see http://www.bath.ac.uk/~ensab/G_mod/Svlis"; 
 
// Set up the view parameters for the ratracer 
 
void set_view_and_lights() 
{ 
 
      sv_line dead_cen = inverse_transform(round(ww/2), round(wh/2)); 
      sv_line up = inverse_transform(round(ww/2), 0); 
      sv_line left = inverse_transform(0, round(wh/2)); 
 
      view.eye_point(dead_cen.origin); 
      sv_point c = dead_cen.origin + dead_cen.direction*box_d*1.5; 
      view.centre(c); 
      sv_point x = up.direction^dead_cen.direction; 
      sv_point y = dead_cen.direction^x; 
      view.vertical_dir(y);       
      //view.set_vertical_dir(up.origin + up.direction*box_d); 
      //view.set_lens_angle(lens_y*M_PI/180); 
      sv_real au = 2.0*acos(dead_cen.direction*up.direction); 
      sv_real al = 2.0*acos(dead_cen.direction*left.direction); 
      view.lens_angle(max(au,al)); 
 
      delete lights; 
 
      lights = new sv_light_list;	  
      lights->source = new sv_lightsource; 
      y = dead_cen.point(-box_d*100) + y.norm()*box_d*100 + x.norm()*box_d*50; 
      lights->source->location(y); 
      lights->source->direction(c - y); 
      lights->name = new char[sv_strlen("LP0")+1]; 
      sv_strcpy(lights->name, "LP0"); 
      lights->next = 0; 
} 
 
// Update the raytraced screen image 
 
void ray_progress(sv_real percent) 
{ 
#ifdef SV_UNIX 
// Update temp file and tell xv to redisplay it 
 
   write_image(ray_pic_file, &ray_pic, comment); 
   kill(xv_pid, SIGQUIT); 
#endif 
} 
 
// Generate a ray-traced version of the OpenGL display 
 
static sv_real little_b = 0.001; 
static sv_real little_b_s; 
static sv_real swell = 0.08; 
static sv_real swell_s; 
static sv_integer low_c = 2; 
static sv_integer low_c_s; 
 
void ray_t_pic() 
{ 
   if(!last_picked)  
   { 
        svlis_error("ray_t_pic()","multiple models - only one can be selected for raytracing",SV_FATAL); 
	return; 
   }
 
   sv_set st = last_picked->model().set_list().set(); 
 
   if(init_raytrace_cache(st))  
   { 
        svlis_error("ray_t_pic()","Cannot initialize raytracing cache",SV_FATAL); 
	return; 
   } 
 
   // Set up image and set it to the background colour 
 
   ray_pic.resolution(ww, wh); 
   ray_pic.picture_to_colour(sv_pixel(bgc)); 
  // set_background_colour(bgc.x, bgc.y, bgc.z, 1); 
 
   set_horizon_colour(.9, .9, 1, .9); 
   set_ground_colour(0.2, 0.3, 0.1, 1); 
   set_overhead_colour(.1, .1, 1, .6); 
   set_ambient_light_level(sv_point(1,1,1),1); 
 
 
// Create temp file and start an xv 
 
	if(image_type() == SV_BMP) 
      		ray_pic_file = "sv_ray.bmp"; 
	else 
      		ray_pic_file = "sv_ray.ppm"; 
 
	write_image(ray_pic_file, &ray_pic, comment); 
 
 
#ifdef SV_UNIX 
	xv_pid = 0; 
 
	if((xv_pid = fork()) < 0)  
        { 
	        // Fork fails 
	        svlis_error("ray_t_pic()", 
	       	  "cannot fork to start xv", SV_WARNING); 
		return; 
	} else  
	{ 
	        if(xv_pid == 0)  
                { 
		   // This is the child - exec an xv 
		   execl(xv_command,"xv","-name","SvLis raytracer",ray_pic_file,0); 
	           svlis_error("ray_t_pic()", 
	       	    "cannot exec xv", SV_WARNING); 
	        } 
	} 
#endif 
  
	set_view_and_lights(); 
	low_c_s = user_low_contents(); 
	little_b_s = get_small_volume(); 
	swell_s = get_swell_fac(); 
	set_small_volume(little_b); 
	set_swell_fac(swell); 
	set_low_contents(low_c); 
 
#ifdef SV_UNIX 
	generate_quickview_picture(last_picked->model().divide(0, &dumb_decision), view, *lights, 
		ray_pic, 5, ray_progress); 
#else 
	generate_picture(last_picked->model().divide(0, &dumb_decision), view, *lights, ray_pic); 
#endif 
 
	destroy_raytrace_cache(); 
 
// Save final image 
 
	write_image(ray_pic_file, &ray_pic, comment); 
	set_small_volume(little_b_s); 
	set_swell_fac(swell_s); 
	set_low_contents(low_c_s); 
}                                            
 
//***************************************************************************************************

// Compile geometry into OpenGL

// Plot the coordinate axes 
 
void plot_ax(const sv_box& b) 
{ 
	sv_point o = SV_OO; 
	sv_real b_d = sqrt(b.diag_sq()); 
	sv_point px = sv_point(b_d, 0.0, 0.0); 
	sv_point py = sv_point(0.0, b_d, 0.0); 
	sv_point pz = sv_point(0.0, 0.0, b_d); 
	sv_point col = SV_X; 
 
	glDisable(GL_LIGHTING); // No shading 
 
	glBegin(GL_LINES); 
	 glColor3fv(&(col.x)); 
         glVertex3fv(&(o.x)); 
         glVertex3fv(&(px.x)); 
 
	 col.x = 0; 
	 col.y = 1; 
	 glColor3fv(&(col.x)); 
         glVertex3fv(&(o.x)); 
         glVertex3fv(&(py.x)); 
 
	 col.y = 0; 
	 col.z = 1; 
	 glColor3fv(&(col.x)); 
         glVertex3fv(&(o.x)); 
         glVertex3fv(&(pz.x)); 
	glEnd(); 
 
	glEnable(GL_LIGHTING); 
 
} 
 
 
// Draw a wire-cross to identify an sv_point (better than a dot) 
 
void wireCross(const sv_point& p, const sv_point& col) 
{ 
	sv_real crossbar = box_d*user_cross; 
 
	sv_point px0 = p - sv_point(crossbar, 0.0, 0.0); 
	sv_point px1 = p + sv_point(crossbar, 0.0, 0.0); 
	sv_point py0 = p - sv_point(0.0, crossbar, 0.0); 
	sv_point py1 = p + sv_point(0.0, crossbar, 0.0); 
	sv_point pz0 = p - sv_point(0.0, 0.0, crossbar); 
	sv_point pz1 = p + sv_point(0.0, 0.0, crossbar); 
 
	glDisable(GL_LIGHTING); 
 
	glBegin(GL_LINES); 
	 glColor3fv(&(col.x)); 
         glVertex3fv(&(px0.x)); 
         glVertex3fv(&(px1.x)); 
         glVertex3fv(&(py0.x)); 
         glVertex3fv(&(py1.x)); 
         glVertex3fv(&(pz0.x)); 
         glVertex3fv(&(pz1.x)); 
	glEnd(); 
 
	glEnable(GL_LIGHTING); 
} 
 
// Draw a box in wireframe 
 
void plot_box(const sv_box& b, const sv_point& col) 
{ 
  sv_point p, q; 
  sv_integer i, j, k; 
 
  glDisable(GL_LIGHTING);   // No shading 
 
  for(i = 0; i < 12; i++) 
  { 
     box_edge(i, &j, &k); 
     glBegin(GL_LINES); 
      glColor3fv(&(col.x)); 
      p = b.corner(j); 
      glVertex3fv(&p.x); 
      q = b.corner(k); 
      glVertex3fv(&q.x); 
     glEnd(); 
  } 
 
  glEnable(GL_LIGHTING);	// Shading back on. 
} 
 
 
// Draw an individual polygon in wireframe 
 
void wireframePolygon(sv_p_gon* pg, const sv_surface& surf) 
{ 
        if (!pg) return;  // Nothing there? 
 
        sv_p_gon* n; 
 
	sv_point col = surf.diffuse_colour(); 
 
	glDisable(GL_LIGHTING);	// Shading off here. 
 
	switch(pg->kind) 
	{ 
	case PT_SET: 
        	n = pg; 
        	do 
        	{ 
			wireCross(n->p, col); 
			n = n->next; 
       		}while (n != pg); 
		break; 
 
	case P_LINE: 
	case P_GON: 
		if (pg->kind == P_LINE) 
			glBegin(GL_LINE_STRIP); 
		else 
			glBegin(GL_LINE_LOOP); 
		 n = pg; 
		 do 
		 { 
			glColor3fv(&(col.x)); 
               		glVertex3fv(&(n->p.x)); 
			n = n->next; 
		 } while (n != pg); 
		glEnd(); 
 
		break; 
 
	default: 
		svlis_error("wireframePolygon","dud polygon kind",SV_WARNING); 
		break; 
	} 
 
	glEnable(GL_LIGHTING);	// Shading back on. 
} 
 
 
// Render an individual polygon 
 
void renderPolygon(sv_p_gon* pg, const sv_surface& surf, const sv_set& s) 
{ 
        if (!pg) return;   // Anything there? 
 
	if (pg->kind != P_GON)  // If it's not filled, draw using the wireframe procedure 
	{ 
		wireframePolygon(pg, surf); 
		return; 
	} 
 
// Pick up colour and any texture from the set that this polygon 
// represents' attributes. 
 
	sv_point col = surf.diffuse_colour(); 
	sv_picture* pic = surf.texture(); 
        sv_p_gon *n; 
 
// If the set is thin, then both sides of the polygon will be visible 
 
	if(s.contents() == 1) 
	{ 
		if(p_thin(s.primitive())) 
			glDisable(GL_CULL_FACE); 
	} 
 
// Set up texture (if any) 
 
	if(pic && texturing) 
	{ 
		glTexImage2D( GL_TEXTURE_2D, 0, 3, pic->x_resolution(), 
			pic->y_resolution(), 0, GL_RGB, GL_UNSIGNED_BYTE, 
			pic->image() ); 
		if(surf.map_type() == SV_TILED) 
		{ 
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT); 
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT); 
		} else 
		{ 
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP); 
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP); 
		} 
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST); 
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
		glEnable(GL_TEXTURE_2D); 
		glShadeModel(GL_FLAT); 
	} else 
		glEnable(GL_COLOR_MATERIAL); 
 
 
// Deal with transparency 
 
	int st1 = 0; 
	 
	if(surf.transmission() > 0.001) // Hack 
	{ 
     		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
                glEnable(GL_BLEND); 
                glDisable(GL_POLYGON_SMOOTH); 
		glDepthMask(GL_FALSE); 
		st1 = 1; 
        } else 
        { 
                glBlendFunc(GL_ONE, GL_ZERO); 
                glDisable(GL_BLEND); 
                glEnable(GL_POLYGON_SMOOTH); 
	} 
 
// Deal with aerial perspective 
 
	if(fog && surf.mist()) glEnable(GL_FOG); 
 
// Now plot the polygon 
 
	sv_point st; 
	sv_real u, v; 
 
	glBegin(GL_POLYGON); 
       	 n = pg; 
       	 do 
       	 { 
		if(pic && texturing) 
		{ 
		   get_tex_map(s, surf, n->p, n->g, u, v); 
		   st.x = u; 
		   st.y = v; 
		   glTexCoord2fv(&st.x); 
		} else 
		   glColor4f(col.x, col.y, col.z, 1 - surf.transmission()); 
 
		glNormal3fv(&(n->g.x));		// Normal vector 
               	glVertex3fv(&(n->p.x));		// Point 
 
               	n = n->next; 
       	 }while (n != pg); 
	glEnd(); 
 
// Tidy up 
 
	if(pic && texturing) 
	{ 
		glDisable(GL_TEXTURE_2D); 
		glShadeModel(GL_SMOOTH); 
	} 
	else 
		glDisable(GL_COLOR_MATERIAL); 
 
	if(face_culling) glEnable(GL_CULL_FACE); 
	if(fog && surf.mist()) glDisable(GL_FOG); 
	 
	if (st1)  
		glDepthMask(GL_TRUE); 
} 
 
 
// Render a set 
 
void renderSet(const sv_set& s) 
{ 
        sv_p_gon* pg; 
	sv_surface surf; 
	sv_attribute a; 
	sv_user_attribute* u; 
 
// Children (if any) may also have polygon attributes 
 
	if (s.contents() > 1) 
	{ 
		renderSet(s.child_1()); 
		renderSet(s.child_2()); 
	} 
 
	surf = s.surface(); 
 
// Loop through the attributes plotting all polygon ones 
 
	a = s.attribute(); 
	while(a.exists()) 
	{ 
		if( a.tag_val() == -pg->tag() ) 
		{ 
              		u = a.user_attribute(); 
                	pg = (sv_p_gon*)u->pointer; 
			renderPolygon(pg, surf, s); 
		} 
		a = a.next(); 
	} 
} 
 
// Render polygons attached to all sets in a list 
 
void renderSetList(sv_set_list pgl) 
{ 
	sv_set s; 
 
        while ( pgl.exists() ) 
        { 
                s = pgl.set(); 
		renderSet(s); 
		pgl = pgl.next(); 
	} 
} 
 
 
// render a model's polygons 
 
void sv_render_a_model(const sv_model &m) 
{ 
        if (m.has_polygons()) 
                   renderSetList(m.set_list()); 
 
	if ( m.kind() != LEAF_M ) 
	{ 
		sv_render_a_model(m.child_1()); 
		sv_render_a_model(m.child_2()); 
	} 
} 
 
// Render a model's boxes (for diagnostics) 
 
void renderBoxes(const sv_model &m) 
{ 
	sv_point col; 
	sv_integer s_conts; 
 
	if (m.kind() == LEAF_M) 
	{ 
 
// change colour according to contents 
// blue = 0, cyan = 1, green = 2, yellow = 3, red > 3 
 
	    s_conts = m.set_list().contents(); 
	    if(plot_empties || (s_conts > 0 )) 
	    { 
	       if (s_conts < 2) 
		col.z = 0.8; 
	       else 
		col.z = 0.0; 
 
	       if (s_conts > 0 && s_conts < 4) 
		col.y = 0.8; 
	       else 
		col.y = 0.0; 
 
	       if (s_conts > 2) 
		col.x = 0.8; 
	       else 
		col.x = 0.0; 
	       plot_box(m.box(),col); 
	    } 
	} else 
	{ 
	    renderBoxes(m.child_1()); 
	    renderBoxes(m.child_2()); 
	} 
} 

//*****************************************************************************************

// Voronoi diagram rendering
 
// Plot Delaunay tetrahedra 
 
void plot_tets(sv_vertex* v) 
{ 
    sv_point c; 
    sv_point col = SV_GREY40; 
    sv_delaunay* d; 
    sv_point p; 
    sv_point e[4]; 
    sv_integer i, j; 
    sv_integer plot_tri; 
 
    c = v->centroid(); 
 
    for(i = 0; i <= SV_VD; i++) 
    { 
	d = v->delaunay(i); 
	if(d->c_hull()) return; 
	p = d->point(); 
	e[i] = c + (p - c)*tet_fac; 
    } 
 
    glDisable(GL_CULL_FACE); 
    glEnable(GL_COLOR_MATERIAL); 
    for(i = 0; i <= SV_VD; i++) 
    { 
	plot_tri = 1; 
	 for(j = 0; j <= SV_VD; j++) 
           if (j != i) 
		if(v->delaunay(j)->flag() & SV_IN_SOLID) 
			plot_tri = 0; 
	if((!supress_internal) || plot_tri) 
	{ 
	    glBegin(GL_POLYGON); 
	     glColor3fv(&(col.x)); 
	     for(j = 0; j <= SV_VD; j++) 
	       if (j != i) glVertex3fv(&(e[j].x)); 
	    glEnd(); 
	} 
    } 
 
    glDisable(GL_COLOR_MATERIAL); 
    if(face_culling) glEnable(GL_CULL_FACE); 
} 
 
// Plot the Voronoi edges of this vertex 
 
void plot_ve(sv_vertex* v) 
{ 
  sv_point p = v->position(); 
  sv_point pp, q, r, s; 
  sv_vertex* n; 
  sv_point col = SV_DARKGREEN; 
  sv_integer i; 
  sv_line ln; 
  sv_interval ib; 
 
  for(i = 0; i <= SV_VD; i++) 
  { 
	n = v->neighbour(i); 
	if(!n->infinity() && !(n->flag() & SV_WALK)) 
	{ 
		q = n->position(); 
		ln = sv_line(q - p, p); 
		ib = line_box(ln, svv.box()); 
		if(!ib.empty()) 
		{ 
			if(ib.lo < 0) ib.lo = 0; 
			if(!ib.empty()) 
			{ 
				pp = line_point(ln, ib.hi); 
				r = pp - p; 
				s = q - p; 
				if(r*r < s*s) q = pp; 
				pp = line_point(ln, ib.lo); 
				r = pp - p; 
				if(r*r < s*s) 
				{ 
					glDisable(GL_LIGHTING); 
					glBegin(GL_LINES); 
					 glColor3fv(&(col.x)); 
					 glVertex3fv(&pp.x); 
					 glVertex3fv(&q.x); 
					glEnd(); 
					glEnable(GL_LIGHTING); 
				} 
			} 
		} 
	} 
  } 
} 
 
// Plot the Delaunay points 
 
void plot_vx(sv_vertex* v) 
{ 
  sv_delaunay* d; 
  for(int i = 0; i <= SV_VD; i++) 
  { 
     	d = v->delaunay(i); 
     	if(!(d->flag() & SV_WALK) && !d->c_hull()) 
     	{ 
     		wireCross(d->point(),SV_DARKBLUE); 
		d->set_flag(SV_WALK); 
     	} 
  } 
} 
 
// Render the Voronoi diagram 
 
void renderVoronoi(sv_vertex* v) 
{ 
  sv_vertex* n; 
 
  v->set_flag(SV_WALK); 
 
  if(plot_v_v) plot_ve(v); 
  if(plot_v_x) plot_vx(v); 
  if(plot_v_d) 
  { 
	if(!(v->flag() & SV_EMPTY)) 
	{ 
		plot_tets(v); 
	} 
  } 
 
  for(int i = 0; i <= SV_VD; i++) 
  { 
     n = v->neighbour(i); 
     if(!n->infinity() && !(n->flag() & SV_WALK)) 
	renderVoronoi(n); 
  } 
} 


//*****************************************************************************************

// Ray enquiry


// Fire a ray into the entire scene of models in the list

sv_model_list *sv_ray_into_scene(
	const sv_line& ray,       // The ray
	sv_set& hit_set,          // The hit set (undefined if a miss)
	sv_point& hit_point,      // The point hit in the _set_'s coordinate system
	sv_real& hit_param)       // The ray parameter in the view space coordinate system
{
	hit_param = 20*enclosure.diag_sq(); // slight hack
	sv_model_list* this_model = svm;
	sv_model_list* result = 0;
	sv_set r_set;
	sv_point r_point;
	sv_real r_param, r_param2;
	sv_line x_ray;
	sv_point view_p;

	while(this_model)
	{

// Put the ray into the model's coordinates

		x_ray = this_model->model_line(ray);
		r_set = this_model->faceted_model().fire_ray(x_ray, &r_param);
		if(r_set.exists())
		{

// Get the hit point back into scene coordinates

			r_point = x_ray.point(r_param);
			r_point = this_model->scene_point(r_point);

// Distance from point to ray origin is the ray parameter

			r_point = r_point - ray.origin;
			r_param2 = r_point*r_point;       // NB hit point should never be behind the eye
			if(r_param2 < hit_param)
			{
				hit_param = r_param2;
				hit_set = r_set;
				hit_point = x_ray.point(r_param);
				result = this_model;
			}
		}
		this_model = this_model->next();
	}

	hit_param = sqrt(hit_param);
	last_picked = result;
	return(result);
} 	

// Put a point in a string 
 
void PtoStr(char* str, const sv_point& p) 
{ 
        ostrstream ost(str, 100); 
        ost << '(' << p.x << ", "; 
        ost << p.y << ", "; 
        ost << p.z << ')' << '\0'; 
} 
 
// Put a real in a string 
 
void RtoStr(char* str, sv_real r) 
{ 
        ostrstream ost(str, 100); 
        ost << r << '\0'; 
} 
 
 
// Trace a ray at the pixel pointed to, and report on what's hit 
 
 
void ray_enquire(int x, int y, int button) 
{ 
	sv_line ray = inverse_transform(x,y); 
	sv_real t; 
	sv_set s;
	sv_point hit;
	char* nm = 0;
	sv_model_list* hit_sml = 0; 
	char ip[100]; 
 
        sv_interval t_int = line_box(ray,enclosure);
        if (!t_int.empty()) 
        	hit_sml = sv_ray_into_scene(ray, s, hit, t);
 
	cout << SV_EL; 
 
	if(!hit_sml) 
	{ 
		cout << "    The ray missed." << SV_EL; 
		cout.flush(); 
		return; 
	} 
  
	sv_point col; 
	sv_primitive p = s.primitive(); 
	sv_integer k; 
	sv_real r0, r1, r2; 
	sv_plane f; 
        sv_point cen; 
	sv_line axis;

	cout << SV_EL;
	nm = hit_sml->name();
	if(nm) cout << "    The ray hit a model named " << nm << "." << SV_EL; 
 
	cout << "    The ray hit at the point ";  
	PtoStr(ip, hit); 
	cout << ip; 
	cout << " [scene coordinates: ";
	PtoStr(ip, ray.point(t));
	cout << ip << "]";
	cout << "." << SV_EL; 
	cout << "    The colour at that point is "; 
	col = s.colour(); 
	PtoStr(ip, col); 
	cout << ip; 
	cout << "." << SV_EL; 
	cout << "    The primitive is "; 
	switch(p.parameters(&k, &r0, &r1, &r2, &f, &cen, &axis)) 
	{ 
	case SV_PLUS:	cout << "ordinary." << SV_EL; break; 
	case SV_COMP:	cout << "complemented." << SV_EL; break; 
	case SV_TIMES:	cout << " a scaled plane (the factor is "; 
			RtoStr(ip, r0); 
			cout << ip; 
			cout << ")." << SV_EL; 
			break; 
	case SV_ABS:	cout << "thin." << SV_EL; break; 
	case SV_SSQRT:	cout << "signed-square-rooted." << SV_EL; break; 
	case SV_SIGN:   cout << "signed." << SV_EL; break; 
	default: 
		svlis_error("ray_enquire","dud primitive operator", 
			SV_CORRUPT); 
	} 
	cout << "    It is a "; 
	switch(k) 
	{ 
        case SV_REAL: 
		cout << "real, value "; 
		RtoStr(ip, r0); 
		cout << ip; 
		cout << " (this is rather a surprise...)."  << SV_EL; 
		break; 
 
        case SV_PLANE: 
		cout << "plane." << SV_EL; 
		cout << "    Its normal is "; 
		PtoStr(ip, f.normal); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its D term is "; 
		RtoStr(ip, f.d); 
		cout << ip; 
		cout << "." << SV_EL; 
		break; 
 
        case SV_CYLINDER: 
		cout << "cylinder." << SV_EL; 
		cout << "    Its axis direction is "; 
		PtoStr(ip, axis.direction); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its axis origin is "; 
		PtoStr(ip, axis.origin); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its radius is "; 
		RtoStr(ip, r0); 
		cout << ip; 
		cout << "." << SV_EL; 
		break; 
 
        case SV_SPHERE: 
		cout << "sphere." << SV_EL; 
		cout << "    Its centre is "; 
		PtoStr(ip, cen); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its radius is "; 
		RtoStr(ip, r0); 
		cout << ip; 
		cout << "." << SV_EL; 
		break; 
 
        case SV_CONE: 
		cout << "cone." << SV_EL; 
		cout << "    Its axis direction is "; 
		PtoStr(ip, axis.direction); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its axis origin is "; 
		PtoStr(ip, axis.origin); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its included angle is "; 
		r0 = r0*180/M_PI; 
		RtoStr(ip, r0); 
		cout << ip; 
		cout << " degrees." << SV_EL; 
		break; 
 
        case SV_TORUS: 
		cout << "torus." << SV_EL; 
		cout << "    Its axis direction is "; 
		PtoStr(ip, axis.direction); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its centre is "; 
		PtoStr(ip, axis.origin); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its major radius is "; 
		RtoStr(ip, r0); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its minor radius is "; 
		RtoStr(ip, r1); 
		cout << ip; 
		cout << "." << SV_EL; 
		break; 
 
        case SV_CYCLIDE: 
		cout << "cyclide." << SV_EL; 
		cout << "    Its axis direction is "; 
		PtoStr(ip, axis.direction); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its centre is "; 
		PtoStr(ip, axis.origin); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its major radius is "; 
		RtoStr(ip, r0); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its minor radius is "; 
		RtoStr(ip, r1); 
		cout << ip; 
		cout << "." << SV_EL; 
		cout << "    Its radial range (+/-) is "; 
		RtoStr(ip, r2); 
		cout << ip; 
		cout << "." << SV_EL; 
		break; 
 
 
        case SV_GENERAL: 
		cout << "general primitive." << SV_EL; 
		break; 
 
	default: 
		cout << "user primitive." << SV_EL; 
	} 
 
// Work out the thickness of the object 
 
        sv_point in_norm = -(p.grad(hit).norm()); 
	ray = sv_line(in_norm, hit + in_norm*sqrt(hit_sml->model().box().diag_sq())*0.00001);
	sv_interval tint = line_box(ray, hit_sml->model().box());
	tint.lo = 0;
        s = hit_sml->faceted_model().fire_ray(ray, tint, &t);
	sv_point dif; 
	if(s.exists()) 
        { 
		dif = line_point(ray,t) - hit; 
        	cout << "    The thickness of the object perpendicular to that surface is " << sqrt(dif*dif) << "." << SV_EL << SV_EL; 
	} 
 
// If the user pressed the middle button, blow up the leaf box 
// containing the hit, or revert to the full model 
 
	if(button == GLUT_MIDDLE_BUTTON) 
	{ 
		if(save_mod.exists()) 
		{ 
			hit_sml->faceted_model(save_mod);     // Restore full model
			save_mod = null_mod; 
		} else 
		{ 
			save_mod = hit_sml->faceted_model();
			hit_sml->faceted_model(hit_sml->faceted_model().leaf(hit));  // Blow up a leaf
		} 
		sv_compile_model(); 
	} 
 
	cout.flush(); 
 
}

//*********************************************************************************************

inline sv_pixel get_from_bytes(sv_integer i, sv_integer j, sv_integer wx, sv_integer hy, 
			       unsigned char *t_store)
{
  sv_pixel result;
  sv_integer tp;
#if 1
  if(!(i%2))
    tp = (i/2 + (hy - j - 1)*wx)*3;
  else
    tp = (i/2 + wx/2 + 1 + (hy - j)*wx)*3;
#else
    tp = (i + (hy - j - 1)*wx)*3;
#endif   
  result.r = (GLubyte)t_store[tp];
  tp++;
  result.g = (GLubyte)t_store[tp];
  tp++;
  result.b = (GLubyte)t_store[tp];
  return(result);
}

// Constructor for sv_picture from an existing image from GL

sv_picture* get_pic_from_screen(int w_id, int owned)
{
   sv_picture* result = new sv_picture();

// Remember current window

   int c_id = glutGetWindow();
   glutSetWindow(w_id);

   if(!owned) get_display_list();

   sv_integer wx = glutGet((GLenum)GLUT_WINDOW_WIDTH);
   sv_integer hy = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
   if(result->resolution(wx, hy))
	svlis_error("get_pic_from_screen(int win_id)","can't malloc image",SV_FATAL);
   sv_integer sz = wx*hy*3; 
   unsigned char *t_store = new unsigned char[sz + 4000];
   glReadPixels(0, 0, wx, hy, GL_RGB, GL_UNSIGNED_BYTE, t_store);

   if(!owned) release_display_list();

// Restore current window

   glutSetWindow(c_id);

// Invert image

   sv_integer a1;
   for(sv_integer j = 0; j < hy; j++)
    for(sv_integer i = 0; i < wx; i++)
    {
        a1 = i + j*wx;
        result->image_bitmap[a1] = get_from_bytes(i, j, wx, hy, t_store);
    }

   delete [] t_store;

    return(result);
}

// User-called picture member function

sv_picture::sv_picture(int w_id)
{
	to_user = 1;
	*this = *get_pic_from_screen(w_id, 0);
}

// OpenGL rendering

// Set the idle function

void set_idle_func(int i)
{
   if(i)
   {
   	if(spinning) 
     		glutIdleFunc(animate); 
   	else
	        glutIdleFunc(Re_Draw);
   } else
         glutIdleFunc(NULL);
}

// Write this frame into the animated sequence

void write_frame()
{
         sv_integer length = sv_strlen(anim_seq) + SV_FC_MAX + 10;
         char* frame_file = new char[length];
	 sv_strcpy(frame_file, anim_seq);
	 sv_integer digit = 1;
	 for(int i = 0; i < SV_FC_MAX-1; i++) digit = digit*10;
	 while(digit > frame_count)
	 {
	   sv_strcat(frame_file, "0"); 
	   digit = digit/10;
	 }
	 sv_integer new_l = sv_strlen(frame_file);
         ostrstream ost(&(frame_file[new_l]), length - new_l);
         ost << frame_count << '\0';               
	 if(image_type() == SV_BMP) 
	        sv_strcat(frame_file, ".bmp"); 
	 else 
		sv_strcat(frame_file, ".ppm"); 

         sv_picture *pic; 
	 pic = get_pic_from_screen(win_id, 1); 
	 write_image(frame_file, pic, "SvLis animation");
	 delete [] frame_file;
	 delete pic;
	 frame_count++;
}
 
// Draw the model after the polygons have been put in sv_as_gl 
 
void sv_draw() 
{ 
  GLfloat mr[3][3], m[4][4];

   get_display_list(); 
 
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
 
   glPushMatrix();
   // if(newModel)
   //{  
    	glTranslatef(view_xt, view_yt, view_zt); 
    	quaternionToMatrix(curquat, mr);
	homogenizeMatrix(mr, m); 
    	glMultMatrixf(&m[0][0]); 
    	glTranslatef(cen_x, cen_y, cen_z);
    	newModel = 0;
	// } 
    if(svm)gl_call_picture_list(svm);
    if(svv.exists())glCallList(svv_as_gl);
    glCallList(decs_as_gl);
   glPopMatrix(); 
 
   glutSwapBuffers();

   if(anim_seq) write_frame();

   callbacks();

   release_display_list();

   set_idle_func(1);
}

void sv_draw(int nm)
{ 
	newModel = nm;
	sv_draw();
}
  
// Compile axes etc 

void compile_decs_m()
{
	if(!glIsList(decs_as_gl)) decs_as_gl = glGenLists(1); 
	glNewList(decs_as_gl, GL_COMPILE); 
	 if(plot_axes) plot_ax(enclosure);
         if(box_mod.exists()) renderBoxes(box_mod);
         if(plot_b && last_picked) renderBoxes(last_picked->faceted_model());
         if(plot_b) plot_box(enclosure,SV_BLACK);
	glEndList();
}

void compile_decs_v()
{
	if(!glIsList(decs_as_gl)) decs_as_gl = glGenLists(1); 
	glNewList(decs_as_gl, GL_COMPILE); 
	 if(plot_axes) plot_ax(enclosure);
         if(plot_b) plot_box(enclosure,SV_DARK_BLUE);
	glEndList();
}

// Put the model svm or the Voronoi tessellation svv into OpenGL polygons
  
void sv_compile_model() 
{
   get_display_list();

   glutSetCursor(GLUT_CURSOR_WAIT); 
 
   if(svm) 
   { 
	recompile_list(svm);
	compile_decs_m();
   } 
 
   if(svv.exists()) 
   { 
   	if(!glIsList(svv_as_gl)) svv_as_gl = glGenLists(1); 
   	glNewList(svv_as_gl, GL_COMPILE); 
   	 renderVoronoi(svv.walk_start());
         reset_flags(svv.walk_start(), SV_WALK); 
   	glEndList(); 
        compile_decs_v();  
   } 
	
   glutSetCursor(cursor);

   force_recompile = 0; 

   release_display_list();

   sv_draw(1);
}

// Spin the picture by changing the quaternion 
 
void animate(void) 
{ 
  new_transforms(lastquat, SV_OO);  
  sv_draw(1); 
} 
 
// Redraw only if need be 
 
void Re_Draw(void) 
{
   if (force_recompile) sv_compile_model(); 
   if (newModel) sv_draw(1); 
}

void sv_draw_all()
{
	to_user = 1;
	force_recompile = 1;
        newModel = 1;
	Re_Draw();
}

//***************************************************************************

// sv_model_list procedures

// Procedure that gets called when no thread is running to initiate it

void first_model(char* title)
{
	if(thread_running) return;
        newModel = 1;
	force_recompile = 1;
   	home(); 
   	for(int i = 0; i < 5; i++) set_view(i); 
   	win_name = new char[sv_strlen(title) + 1]; 
   	sv_strcpy(win_name,title); 
   	branch_proc(); // Thread this process off
}

// Add a model to the list of them being displayed


sv_model_list* add_model_to_picture(const sv_model& mm, const sv_model& mfacet, const sv_point& p, 
	   const sv_line& a, sv_real th, 
	   void (*cb)(sv_model_list*, const sv_point&, const sv_line&, sv_real, sv_xform_action) )
{
  first_model("svLis");

  get_display_list();

  sv_model_list* result = new sv_model_list();

  result->org = mm;
  result->m = mfacet;
  result->m_changed = 1;
  result->n = svm;
  svm = result;
  last_picked = svm;

  result->trans = p;
  result->ax = a;
  result->spn = th;
  result->call_back(cb);
  result->x_changed = 1;

  release_display_list();

  return(result);
}

sv_model_list* add_model_to_picture(const sv_model& mm, const sv_point& p, 
	   const sv_line& a, sv_real th, 
	   void (*cb)(sv_model_list*, const sv_point&, const sv_line&, sv_real, sv_xform_action) )
{
  sv_model mfacet = mm.facet();
  sv_model_list* result = add_model_to_picture(mm, mfacet, p, a, th, cb);
  return(result);
}

// Compile every model and transform in the list

void recompile_list(sv_model_list* spl)
{
	sv_model_list* l = spl;
	while(l)
	{
		l->compile_transform();
		l->compile_model();
		l = l->next();
	}
}

// Delete everything in the list

void sv_delete_picture_list()
{
	sv_model_list* l = svm;
	sv_model_list* n;
	while(l)
	{
		n = l->next();
		delete l;
		l = n;
	}
	svm = 0;
	last_picked = 0;
}

// Call all the compiled transforms and display lists

void gl_call_picture_list(sv_model_list* spl)
{
	sv_model_list* l = spl;

	while(l)
	{
		if(l->visible())
		{
   		  glPushMatrix();
		   glCallList(l->xlist);
		   glCallList(l->mlist);
   		  glPopMatrix();
		}
		l = l->next();
	}
}

// Find the model list entry with a given name

sv_model_list* sml_named(char* nm)
{
	sv_model_list* result = svm;
	char* mnm;
	while(result)
	{
		mnm = result->name();
		if(mnm)
		  if(sv_strcmp(mnm, nm) == 0) return(result);
		result = result->next();
	}
	return(0);
}

//***************************************************************************

// Handle mouse, spaceball/mouse, and window changes

static float r_scale = 0.001;  // Rotation scale
static float t_scale = 0.001;  // Translation scale
static int r_free = 1;	       // Free to rotate?
static int t_free = 1;	       // Free to translate?
static int dump = 1;

// Space ball/mouse rotation

void space_rot(int x, int y, int z)
{
        sv_line axis;
	float new_quat[4];
	float temp_quat[4];

	if(r_free)
	{
		axis = sv_line(SV_X, SV_OO);
		axisToQuaternion(axis, ((float)x)*r_scale*M_PI/1800.0, new_quat);
 		axis = sv_line(SV_Y, SV_OO);
		axisToQuaternion(axis, ((float)y)*r_scale*M_PI/1800.0, temp_quat);
		add_quats(temp_quat, new_quat, new_quat); 
		axis = sv_line(SV_Z, SV_OO);
		axisToQuaternion(axis, ((float)z)*r_scale*M_PI/1800.0, temp_quat);
		add_quats(temp_quat, new_quat, new_quat);
		new_transforms(new_quat, SV_OO);
	}
}

// Space ball/mouse translation

void space_trans(int x, int y, int z)
{
	sv_point new_trans;
	sv_real quat[4];

	if(t_free)
	{
	    new_trans.x = (sv_real)(x)*t_scale; 
	    new_trans.y = (sv_real)(y)*t_scale; 
	    new_trans.z = (sv_real)(z)*t_scale;
	    trackball(quat, 0, 0, 0, 0);
	    new_transforms(quat, new_trans); 
	}
}

// Space ball/mouse buttons

void space_button(int button, int state)
{
	if (state == GLUT_UP) return;

	switch ( button )
	{
	case 1: t_free = !t_free; break;
	case 2: r_free = !r_free; break;
	case 3: dump++; break;
	case 4: dump--; if(dump < 1) dump = 1; break;
 	case 5: r_scale = r_scale/2; break;
	case 6: r_scale = r_scale*2; break;
	case 7: t_scale = t_scale/2; break;
	case 8: t_scale = t_scale*2; break;
	default: break;
	};
}

// Spacemouse callback procedure

// Switch to C calling convention

#ifdef SV_SPACEMOUSE

#ifdef __cplusplus
extern "C" {
#endif

int sv_event_handler(Display* display, Window window, XEvent* event)
{
	MagellanFloatEvent MagellanEvent;
	static int magellan_started = 0;
	int x, y, z;
	int mte;


	if(event->type == ClientMessage)
	{

		if(!magellan_started)
		{
			if(!MagellanInit(display, window))
				svlis_error("sv_event_handler","spacemouse driver not running",SV_WARNING);
			else
				magellan_started = 1;
		}

		mte = MagellanTranslateEvent( display, event, &MagellanEvent, 1.0, 1.0 );

		switch( mte )
		{
		case MagellanInputMotionEvent:

// Throw away dump-1 out of dump motion events to slow the damned thing down.

			// count++;
			// if(count%dump) return(1);
			// count = 0;

			x = (int)(-MagellanEvent.MagellanData[ MagellanA ]*10);
			y = (int)(-MagellanEvent.MagellanData[ MagellanB ]*10);
			z = (int)(MagellanEvent.MagellanData[ MagellanC ]*10);
			space_rot(x, y, z);


			x = (int)MagellanEvent.MagellanData[ MagellanX ]; 
	    		y = (int)MagellanEvent.MagellanData[ MagellanY ]; 
	    		z = (int)-MagellanEvent.MagellanData[ MagellanZ ]; 
			space_trans(x, y, z);

	 	  	return(1);
	 
		case MagellanInputButtonPressEvent:
			space_button(MagellanEvent.MagellanButton, GLUT_DOWN);
			return(1);
	  
		case MagellanInputButtonReleaseEvent:
			space_button(MagellanEvent.MagellanButton, GLUT_UP);
			return(1);

		default : 
			break;
		}
	}

	return 0;
}


// Switch back to C++ calling convention

#ifdef __cplusplus
 }
#endif

#endif

// Set and return the mouse speed

void sv_mouse_speed(sv_integer ms)
{
    if(ms < 0)
    {
      svlis_error("sv_mouse_speed", "can't set a speed less than 0", SV_WARNING);
      ms = 1;
    }
    mouse_top = ms;
}

sv_integer sv_mouse_speed() { return(mouse_top); }

  
// The user has pressed a mouse button in the window, so set mouse_mode 
 
void mouse(int button, int state, int x, int y) 
{ 
 
// LMB rotates 
// MMB or Ctrl+LMB translates 
// LMB+MMB or Ctrl+MMB zooms 
 
// If raytrace is set, just trace the ray, report, and return 
// If the middle button was pressed magnify the box hit.
  
	switch(raytrace) 
	{
	case TRANS_PIK:
		{
			sv_line ray = inverse_transform(x,y); 
			sv_real t; 
			sv_set s;
			sv_point hit;
			char* nm;
			svml_trans = 0;

        		sv_interval t_int = line_box(ray,enclosure);
        		if (!t_int.empty())
			{
        			svml_trans = sv_ray_into_scene(ray, s, hit, t);
				if(s.exists())
				{
				   nm = s.name();
				   if(nm) cout << SV_EL << "    Model named " << nm <<
					  " picked for transformations." << endl;
				   //if(svml_trans)
				   //if(trns_cb = svml_trans->call_back())
				   //trns_cb(svml_trans, SV_OO, SV_ZL, 0, SV_PICK);
				}
			}
			raytrace = 0;
			cursor = GLUT_CURSOR_LEFT_ARROW; 
			glutSetCursor(cursor);		
			return;
		}

	case RAY_PIK: 
		ray_enquire(x,y,button);
		raytrace = 0;
		cursor = GLUT_CURSOR_LEFT_ARROW; 
		glutSetCursor(cursor); 
		return;

	default:
		break; 
	} 
 
	int mods = glutGetModifiers();

	lb = 0;
        mb = 0;
 
	switch(button) 
	{ 
	case GLUT_LEFT_BUTTON: 
		if(state == GLUT_DOWN) 
			lb = 1; 
		break; 
 
	case GLUT_MIDDLE_BUTTON: 
		if(state == GLUT_DOWN) 
			mb = 1; 
		break; 
 
	default: 
		svlis_error("mouse","non-existent button pressed",SV_COMMENT); 
	} 
 
	if(state == GLUT_DOWN) 
	{ 
		mx_0 = x; 
		my_0 = y; 
	}
 
	if ((mb && lb) || ((mods & GLUT_ACTIVE_CTRL)&& mb)) 
	{ // zooming
	    cursor = GLUT_CURSOR_UP_DOWN;  
	    mouse_mode = zoom; 
	} 
	else if (mb || ((mods & GLUT_ACTIVE_CTRL)&& lb)) 
	{ // translating 
	    cursor = GLUT_CURSOR_INFO;
	    mouse_mode = translate; 
	} 
	else if (lb) 
	{  // rotating 
	    spinning = 0;
	    set_idle_func(0);
	    cursor = GLUT_CURSOR_CYCLE;
	    mouse_mode = svrotate; 
	} else 
	{ 
	    cursor = GLUT_CURSOR_LEFT_ARROW; 
	    mouse_mode = none; 
	}

	glutSetCursor(cursor);
 
} 
 
// The mouse has been moved in the window 
 
void mouse_move(int x, int y) 
{
        mouse_count++;
        if(mouse_count < mouse_top) return;
        mouse_count = 0;

	sv_integer xd, yd, d2;
	int change = 1;
	sv_point new_trans = SV_OO;
	trackball(lastquat, 0, 0, 0, 0);

	xd = x - mx_0;
	yd = y - my_0;
	d2 = xd*xd + yd*yd;

	switch (mouse_mode) 
	{ 
	case zoom:
	    new_trans.z = ((sv_real)yd/2.0);
	    break; 
 
	case svrotate: //Edited RRM
		// the mouse positions have to be scaled so they are -1 to +1 
		trackball(lastquat,
		    (2.0 * (sv_real)mx_0 - ww)/ww, (wh - 2.0 * (sv_real)my_0)/wh,
		    (2.0 *  (sv_real)x   - ww)/ww, (wh - 2.0 *  (sv_real)y  )/wh);
		if(d2 > 5)
			spinning = 1; 
		else
			spinning = 0;
	    break; 
 
	case translate:
	    new_trans.x = ((sv_real)xd/16.0);
	    new_trans.y = -((sv_real)yd/16.0);
	    break; 
 
	default:
		change = 0;
		break; 
	} 

	if(change && (d2 > 0)) new_transforms(lastquat, new_trans);
        set_idle_func(1);
	mx_0 = x; 
	my_0 = y; 
} 
 
// New window size or exposure 
 
static void reshape( int width, int height ) 
{
   to_user = 0;
   get_display_list();
 
   ww = (sv_real) width; 
   wh = (sv_real) height; 
   aspect_r = ww/wh; 
   sv_real top = near_clip*tan(lens_y*M_PI/360); 
   sv_real left = top*aspect_r; 
 
   glViewport(0, 0, (GLint)width, (GLint)height); 
   glMatrixMode(GL_PROJECTION); 
   glLoadIdentity(); 
 
   if(ortho) 
       glOrtho(-box_d*0.5*aspect_r, box_d*0.5*aspect_r, -box_d*0.5, box_d*0.5, near_clip, far_clip); 
   else 
       glFrustum(-left, left, -top, top, near_clip, far_clip);
 
   glMatrixMode(GL_MODELVIEW); 
   glLoadIdentity(); 
   glTranslatef(0.0, 0.0, -box_d); 
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
   newModel=1; 

   release_display_list();
} 
 
// Window's visibility has changed 
 
void vis(int visible) 
{ 
  if (visible == GLUT_VISIBLE) 
  { 
    newModel = 1; 
    set_idle_func(1); 
  } else 
  { 
    if (spinning) 
      set_idle_func(0); 
  } 
}

//*****************************************************************************************

// Menu-handling functions 
 
void menuBGColour(int value) 
{ 
	sv_real colour; 
 
	if(value == 4) 
	{ 
		set_background(BG_COL); 
	} else 
	{ 
		colour = (value-1.0)/2.0; 
		set_background(sv_point(colour,colour,colour)); 
	} 
	glClearColor(bgc.x, bgc.y, bgc.z, 1); 
	//sv_compile_model();
	sv_draw(1); 
} 
 
void menuFacet(int value) 
{ 
    char tmp_str[255]; 
 
    double mult; 
    sv_real fac; 
 
    glutSetCursor(GLUT_CURSOR_WAIT); 
 
    if (value < 200) 
    { 
      if (value < 100) // refine 
	mult = 1.0/(pow(2.0,(double)value)); 
      else             // coarsen 
	mult = pow(2.0, (double)(value-100)); 
 
      fac = get_user_grad_fac(); 
      set_user_grad_fac(fac*mult); 
      fac = get_small_volume(); 
      set_small_volume(fac*mult); 
 
      sprintf(tmp_str, "Facet refinement: %.2f", get_user_grad_fac()); 
      glutSetMenu(6); 
      glutChangeToMenuEntry(1, tmp_str, -1); 
    } else 
    { 
	switch(value) 
	{ 
	case 201: 
		set_swell_fac(0.05); 
		break; 
	case 202: 
		set_swell_fac(0); 
		break; 
	case 203: 
		set_swell_fac(2*get_swell_fac()); 
		break; 
	case 204: 
		set_swell_fac(0.5*get_swell_fac()); 
		break; 
	default: 
		; 
	} 
 
        sprintf(tmp_str, "Box swell factor: %.2f", get_swell_fac()); 
        glutSetMenu(6); 
        glutChangeToMenuEntry(13, tmp_str, -1); 
    }

    sv_model_list* svml = svm;
    while(svml)
    {
      svml->faceted_model(svml->model().facet());
      svml = svml->next();
    }
  
    sv_compile_model(); 
    glutSetCursor(cursor);
} 
 
 
 
// Give help on view change hot keys and mouse buttons 
 
void help() 
{ 
#if macintosh 
	cout << SV_EL << SV_EL << "SvLis graphics" << SV_EL << SV_EL; 
	cout << " Mouse Button rotates (drag and release to spin)" << SV_EL; 
    cout << " Option + Mouse Button translates" << SV_EL; 
    cout << " Command + Option + Mouse Button zooms" << SV_EL; 
	cout << " Control + Mouse Button gives options menu" << SV_EL; 
#else	 
	cout << SV_EL << SV_EL << "SvLis graphics" << SV_EL << SV_EL; 
	cout << " Left Mouse Button rotates (drag and release to spin)" << SV_EL; 
    cout << " Middle Mouse Button or Ctrl+Left Mouse Button translates" << SV_EL; 
    cout << " Left Mouse Button+Middle Mouse Button or Ctrl+Middle Mouse Button zooms" << SV_EL; 
	cout << " Right Mouse Button gives options menu" << SV_EL; 
#endif 
	 
	 
	 
	cout << SV_EL << " Hot keys:" << SV_EL; 
	cout << "  + - ray enquire" << SV_EL; 
/* 
	cout << "  u - rotate +x" << SV_EL; 
	cout << "  d - rotate -x" << SV_EL; 
	cout << "  l - rotate +y" << SV_EL; 
	cout << "  r - rotate -y" << SV_EL; 
	cout << "  a - rotate +z" << SV_EL; 
	cout << "  c - rotate -z" << SV_EL; 
*/ 
	cout << "  x - translate +x" << SV_EL; 
	cout << "  X - translate -x" << SV_EL; 
	cout << "  y - translate +y" << SV_EL; 
	cout << "  Y - translate -y" << SV_EL; 
	cout << "  z - translate +z" << SV_EL; 
	cout << "  Z - translate -z" << SV_EL; 
	cout << "  w - wireframe on/off" << SV_EL; 
	cout << "  e - back face culling on/off" << SV_EL; 
	cout << "  b - faceter's boxes on/off" << SV_EL; 
	cout << "  0 (zero) - axes on/off" << SV_EL; 
	cout << "  c - toggle background colour" << SV_EL; 
	cout << "  h - home position" << SV_EL; 
	cout << "  f - refine facets" << SV_EL; 
	cout << "  F - coarsen facets" << SV_EL; 
	cout << "  t - texturing on/off" << SV_EL; 
	cout << "  s - smoothing on/off" << SV_EL; 
	cout << "  p - print division statistics" << SV_EL; 
	cout << "  P - print the first set in the set list (without its attributes)" << SV_EL; 
        cout << "  v - write as VRML to svlis.wrl" << SV_EL; 
	if(image_type() == SV_PPM) 
        	cout << "  g - write image to svlis.ppm" << SV_EL; 
	else 
        	cout << "  g - write image to svlis.bmp" << SV_EL; 
	cout << "  Shift 1..5 - store view (NB Shifted chars: ! \" # $ %)" << SV_EL; 
	cout << "  1..5 - restore view" << SV_EL; 
	cout << "  <CTRL> C - copy points of view" << SV_EL; 
	cout << "  <CTRL> V - paste points of view" << SV_EL; 
	cout << "  ESC - quit" << SV_EL; 
	cout << "  ? - print this list" << SV_EL << SV_EL; 
	cout.flush(); 
} 
 
// change view angle etc, exit upon ESC 
// Must go after menus as some keys use them 
 
static void key(unsigned char k, int x, int y) 
{ 
   sv_real step = box_d*0.02;
   float new_quat[4];
   sv_point new_trans = SV_OO; 
   sv_set s; 
   char* pfn;
   int new_x = 0; 
 
   trackball(new_quat, 0, 0, 0, 0);

   switch (k) 
   { 
      case 'x': 
         new_trans.x = step; 
	 new_x = 1; 
	 break; 
 
      case 'X': 
         new_trans.x = -step; 
	 new_x = 1; 
	 break; 
 
      case 'y': 
         new_trans.y = step; 
	 new_x = 1; 
	 break; 
 
      case 'Y': 
         new_trans.y = -step; 
	 new_x = 1; 
	 break; 
 
      case 'z': 
	 new_trans.z = step; 
	 new_x = 1; 
	 break; 
 
      case 'Z': 
	 new_trans.z = -step; 
	 new_x = 1; 
	 break; 
 
      case 'h': 
	 home(); 
         sv_draw(1); 
	 break; 
 
      case '0': 
	 plot_axes = !plot_axes; 
	 sv_compile_model(); 
	 break; 
 
      case 'c': 
      	 bgc_num++; 
      	 if (bgc_num > bgc_cnt) 
    	    bgc_num = 1; 
	 menuBGColour(bgc_num); 
	 break; 
 
      case 0x1b:       // ESC 
	 kill_proc(); 
 
      case 0x3:        // CTRL C 
	 write_views(); 
	 break; 
 
      case 22:         // CTRL V 
	 read_views(); 
	 break; 
 
      case 'w': 
	 wireframe = !wireframe; 
	 if (wireframe) 
         { 
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
	    glDisable(GL_CULL_FACE); 
	 } else 
         { 
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
	    if(face_culling) glEnable(GL_CULL_FACE); 
	 }
	 sv_compile_model(); 
	 break; 
 
     case 'e': // toggle face culling 
         face_culling = !face_culling; 
         if (face_culling) 
              glEnable(GL_CULL_FACE); 
         else 
              glDisable(GL_CULL_FACE);
	 sv_compile_model(); 
         break; 
 
     case 'b': 
	 plot_b = !plot_b; 
	 sv_compile_model(); 
	 break; 
 
     case 'f': 
	 menuFacet(1); 
         break; 
 
     case 'F': 
	 menuFacet(101); 
         break; 
 
      case 't': 
	 texturing = !texturing;
 	 svm->force_recompile();  // Is this right?
	 sv_compile_model(); 
	 break; 
 
      case 's': 
	  smoothing = !smoothing; 
          if (smoothing) 
               glEnable(GL_POLYGON_SMOOTH); 
          else 
               glDisable(GL_POLYGON_SMOOTH);
 	  svm->force_recompile();    // And this??
	  sv_compile_model(); 
	  break; 
 
      case '?': 
	 help(); 
         break; 
 
      case '+': 
	 raytrace = RAY_PIK;
         cursor = GLUT_CURSOR_CROSSHAIR;
	 glutSetCursor(cursor); 
	 break;

      case '-': 
	 raytrace = TRANS_PIK;
         cursor = GLUT_CURSOR_CROSSHAIR;
	 glutSetCursor(cursor); 
	 break; 
 
      case 'p': 
	 if(last_picked)
		last_picked->faceted_model().div_stat_report(cout);
	 else
		svlis_error("key", "multiple models - one must be picked for the report", SV_WARNING);
	 break; 
 
      case 'P':
	if(last_picked)
	{ 
		pretty_print(1); 
		s = last_picked->model().set_list().set().strip_attributes();
		cout << SV_EL << SV_EL << s << SV_EL << SV_EL;
	} else
		svlis_error("key", "multiple models - one must be picked for output", SV_WARNING);
	 break; 
 
      case 'v': 
        {
	  if(!last_picked)
		svlis_error("key", "multiple models - one must be picked for VRML output", SV_WARNING);
	  else
	  {
	    ofstream a("svlis.wrl"); 
	    if(!a) 
	    { 
		svlis_error("key","can't open svlis.wrl",SV_WARNING); 
		break; 
	    } 
	    sv_to_vrml(a, last_picked->faceted_model());  // Should really write them all with transforms
	    cout << SV_EL << "Written VRML to svlis.wrl" << SV_EL; 
            cout.flush(); 
	    a.close();
	  }
        }
	break;
 
      case 'g': 
        {
         sv_picture *pic; 
	 pic = get_pic_from_screen(win_id, 0); 
	 if(image_type() == SV_BMP) 
		pfn = "svlis.bmp"; 
	 else 
		pfn = "svlis.ppm"; 
	 write_image(pfn, pic, "Image from svLis display"); 
	 cout << SV_EL << "Picture written to " << pfn << SV_EL; 
         cout.flush(); 
	 break; 
        }

      case '1': get_view(0); break;
      case '2': get_view(1); break;
      case '3': get_view(2); break;
      case '4': get_view(3); break;
      case '5': get_view(4); break;

      case '!': set_view(0); break;
      case '\"': set_view(1); break;
      case '#': set_view(2); break;
      case '$': set_view(3); break;
      case '%': set_view(4); break;
 
      default: 
		break; 
   } 
   if(new_x) new_transforms(new_quat, new_trans);
} 
 
void menuDrawStyle(int value) 
{ 
    switch (value) 
    { 
    case 1: 
		texturing = 1; 
	        svm->force_recompile();
		sv_compile_model(); 
		break; 
 
    case 2: 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
		face_culling = 1; 
		glEnable(GL_CULL_FACE); 
		texturing = 0;
 	        svm->force_recompile();
		sv_compile_model(); 
		break; 
 
    case 3: 
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
		face_culling = 0; 
		glDisable(GL_CULL_FACE); 
		texturing = 0; 
	        svm->force_recompile();
		sv_compile_model(); 
		break; 
 
    case 4: 
		key('b',0,0); 
		break; 
 
    case 5: 
		key('0',0,0); 
		break; 
 
// case 6: used to be transparency 
 
    case 7: 
		key('s',0,0); 
		break; 
 
    case 8:	// toggle face culling 
		key('e',0,0); 
		break; 
 
    case 9:	// toggle wireframe 
		key('w',0,0); 
		break; 
 
    case 10:	// Draw polygons as points 
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); 
		texturing = 0;
	        svm->force_recompile();
		sv_compile_model(); 
		break; 
 
    case 11:	// toggle plotting empty model boxes 
		plot_empties = !plot_empties; 
		sv_compile_model(); 
		break; 
 
    case 12:	// toggle plotting Voronoi crosses 
		plot_v_x = !plot_v_x; 
		sv_compile_model(); 
		break; 
 
    case 13:	// toggle plotting Voronoi tets 
		plot_v_d = !plot_v_d; 
		sv_compile_model(); 
		break; 
 
    case 14:	// toggle plotting Voronoi territories 
		plot_v_v = !plot_v_v; 
		sv_compile_model(); 
		break; 
 
    case 15:	// toggle aerial perspective 
		fog = !fog;
	        svm->force_recompile(); 
		sv_compile_model(); 
		break; 
 
   default: 
		; 
 
    } 
} 
 
 
// Special key hit 
 
static void spec_key(int k, int x, int y) 
{ 
    int mods = glutGetModifiers(); 
 
    switch (k) 
    { 
	case GLUT_KEY_HOME: 
	    key('h',0,0); 
	    break; 
 
	case GLUT_KEY_F1: 
	    if (mods & GLUT_ACTIVE_SHIFT) 
		set_view(0); 
	    else 
		get_view(0); 
	    break; 
 
	case GLUT_KEY_F2: 
	    if (mods & GLUT_ACTIVE_SHIFT) 
		set_view(1); 
	    else 
		get_view(1); 
	    break; 
 
	case GLUT_KEY_F3: 
	    if (mods & GLUT_ACTIVE_SHIFT) 
		set_view(2); 
	    else 
		get_view(2); 
	    break; 
 
	case GLUT_KEY_F4: 
	    if (mods & GLUT_ACTIVE_SHIFT) 
		set_view(3); 
	    else 
		get_view(3); 
	    break; 
 
	case GLUT_KEY_F5: 
	    if (mods & GLUT_ACTIVE_SHIFT) 
		set_view(4); 
	    else 
		get_view(4); 
	    break; 
    } 
} 
 
// Master menu function only needs to deal with Quit 
 
void menuRMB(int value) 
{ 
    if (value == 100) kill_proc(); 
} 
 
// Handle the various menu choices 
 
void menuFunctions(int value) 
{ 
    switch (value) 
    { 
    case 1: help(); break; 
    case 2: key('h',0,0); break; 
 
    case 3:    // Recover view 1..5 
    case 4: 
    case 5: 
    case 6: 
    case 7: 
		get_view(value-3); 
		break; 
 
    case 8:    // Save view 1..5 
    case 9: 
    case 10: 
    case 11: 
    case 12: 
		set_view(value-8); 
		break; 
 
    case 14: 
		key('p',0,0); 
		break; 
 
    case 15:  
		key('v',0,0); 
		break; 
 
    case 16: 
		key(03,0,0); 
		break; 
 
    case 17: 
		key(22,0,0); 
		break; 
 
    case 19:	// Write image 
		key('g',0,0); 
		break;

    default: 
		; 
    } 
} 
 
 
 
void menuPrefs(int value) 
{ 
   char tmp_str[255]; 
 
   switch(value) { 
   case 1: 
	near_clip = near_clip-1; 
	break; 
   case 2: 
	near_clip = near_clip+1; 
	break; 
   case 3: 
	near_clip = 0.1; 
	break; 
   case 4: 
	near_clip = 1; 
	break; 
   case 5: 
	near_clip = 2; 
	break; 
   case 6: 
	near_clip = 5; 
	break; 
   case 7: 
	near_clip = 10; 
	break; 
   case 101: 
	far_clip = far_clip-10; 
	break; 
   case 102: 
	far_clip = far_clip+10; 
	break; 
   case 103: 
	far_clip = 10; 
	break; 
   case 104: 
	far_clip = 20; 
	break; 
   case 105: 
	far_clip = 50; 
	break; 
   case 106: 
	far_clip = 100; 
	break; 
   case 107: 
	far_clip = 1000; 
	break; 
   case 150: 
	fog_d = 0.5*fog_d; 
	glFogf(GL_FOG_DENSITY, fog_d); 
	break; 
   case 151: 
	fog_d = 2*fog_d; 
	glFogf(GL_FOG_DENSITY, fog_d); 
	break; 
 
   case 200: 
	ortho = !ortho; 
	break; 
 
   default: 
	     ; 
   } // end switch 
 
   if (near_clip <= 0.01) 
	near_clip = 0.01; 
   if (near_clip >= far_clip) 
	near_clip = far_clip-0.01; 
 
 
   sv_real top = near_clip*tan(lens_y*M_PI/360); 
   sv_real left = top*ww/wh; 
 
   glMatrixMode(GL_PROJECTION); 
   glLoadIdentity(); 
   // gluPerspective(lens_y, aspect_r, near_clip, far_clip); 
   if(ortho) 
      glOrtho(-box_d*0.5*aspect_r, box_d*0.5*aspect_r, -box_d*0.5, box_d*0.5, near_clip, far_clip); 
   else 
      glFrustum(-left, left, -top, top, near_clip, far_clip); 
 
   glMatrixMode(GL_MODELVIEW); 
   glLoadIdentity(); 
   glTranslatef(0.0, 0.0, -box_d); 
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   svm->force_recompile();
   sv_compile_model(); 
 
   glutSetMenu(5); 
   sprintf(tmp_str, "Near clip plane is %.2f", near_clip); 
   glutChangeToMenuEntry(1, tmp_str, -1); 
   sprintf(tmp_str, "Far clip plane is %.2f", far_clip); 
   glutChangeToMenuEntry(10, tmp_str, -1); 
 
   return; 
} 
 
// Handle raytracing 
 
void menuRaytrace(int value) 
{ 
   char tmp_str[255]; 
 
    switch (value) 
    { 
    case 1: key('+', 0, 0); break;
 
 
    case 2: 
	glutSetCursor(GLUT_CURSOR_WAIT); 
	ray_t_pic();
        glutSetCursor(cursor);
	break; 
 
    case 3: 
	if(low_c > 0) low_c--; 
   	glutSetMenu(3); 
    	sprintf(tmp_str, "Low contents is %d", low_c); 
   	glutChangeToMenuEntry(4, tmp_str, -1); 
	break; 
 
    case 4: 
	low_c++; 
   	glutSetMenu(3); 
    	sprintf(tmp_str, "Low contents is %d", low_c); 
   	glutChangeToMenuEntry(4, tmp_str, -1); 
	break; 
 
     case 5: 
	swell = swell*0.5; 
   	glutSetMenu(3); 
   	sprintf(tmp_str, "Box swell is %.4f", swell); 
   	glutChangeToMenuEntry(8, tmp_str, -1); 
	break;  
 
    case 6: 
	swell = swell*2; 
   	glutSetMenu(3); 
   	sprintf(tmp_str, "Box swell is %.4f", swell); 
   	glutChangeToMenuEntry(8, tmp_str, -1); 
	break;  
 
    case 7: 
	little_b = little_b*0.5; 
   	glutSetMenu(3); 
   	sprintf(tmp_str, "Small box is %.4f", little_b); 
   	glutChangeToMenuEntry(12, tmp_str, -1); 
	break; 
 
    case 8: 
	little_b = little_b*2; 
   	glutSetMenu(3); 
   	sprintf(tmp_str, "Small box is %.4f", little_b); 
   	glutChangeToMenuEntry(12, tmp_str, -1); 
	break; 
 
    case 9: key('-', 0, 0); break; 
       
 
    default: 
		; 
    } 
} 

//*************************************************************************************

// Register the callbacks

void callbacks()
{
 
    glutReshapeFunc( reshape ); 
    glutKeyboardFunc( key ); 
    glutSpecialFunc( spec_key ); // special keys 
    //glutIdleFunc( NULL ); 
    glutMouseFunc( mouse ); 
    glutMotionFunc( mouse_move ); 
    glutDisplayFunc( sv_draw ); 
    glutVisibilityFunc(vis);
    glutSpaceballMotionFunc(space_trans);
    glutSpaceballRotateFunc(space_rot);
    glutSpaceballButtonFunc(space_button);

#ifdef SV_SPACEMOUSE

    glutUserEventFunc(sv_event_handler);

#endif
}



 
// Run OpenGL on the model 
 
void sv_run_glut(void *dummy) 
{ 
   char tmp_str[255]; 
 
   //#if macintosh 
   //glutInitWindowSize( 300, 300 ); 
     //#else 
   //glutInitWindowSize( 600, 600 ); 
     //#endif

    glutInitWindowSize( 600, 600 );  

    glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE ); 
 
    char* wn = win_name; 
    if(!wn) wn = "svLis"; 
 
    if ((win_id = glutCreateWindow(wn)) == GL_FALSE) 
    { 
      svlis_error("sv_run_glut","can't create graphics window",SV_FATAL); 
      kill_proc(); 
    } 
 
    // Make an RMB menu 
    int submenuF = glutCreateMenu(menuFunctions); 
    glutAddMenuEntry("Help [?]", 1); 
    glutAddMenuEntry("Home [h]", 2); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Print stats [p]", 14); 
    glutAddMenuEntry("Write VRML to svlis.wrl [v]", 15); 
    if(image_type() == SV_BMP) 
    { 
      glutAddMenuEntry("Store this as svlis.bmp [g]", 19); 
    } else 
    { 
      glutAddMenuEntry("Store this as svlis.ppm [g]", 19); 
    } 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Paste views [<CTRL> V]", 17); 
    glutAddMenuEntry("View 1 [F1]", 3); 
    glutAddMenuEntry("View 2 [F2]", 4); 
    glutAddMenuEntry("View 3 [F3]", 5); 
    glutAddMenuEntry("View 4 [F4]", 6); 
    glutAddMenuEntry("View 5 [F5]", 7); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Copy views [<CTRL> C]", 16); 
    glutAddMenuEntry("Set view 1 [s-F1]", 8); 
    glutAddMenuEntry("Set view 2 [s-F2]", 9); 
    glutAddMenuEntry("Set view 3 [s-F3]", 10); 
    glutAddMenuEntry("Set view 4 [s-F4]", 11); 
    glutAddMenuEntry("Set view 5 [s-F5]", 12); 
 
 
    int submenuDS = glutCreateMenu(menuDrawStyle); 
    glutAddMenuEntry("Textured", 1); 
    glutAddMenuEntry("Shaded", 2); 
    glutAddMenuEntry("Wireframe", 3); 
    glutAddMenuEntry("Points", 10); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Toggle aerial perspective", 15); 
    glutAddMenuEntry("Toggle faceter's boxes [b]", 4); 
    glutAddMenuEntry("Toggle empty boxes", 11); 
    glutAddMenuEntry("Toggle axes [0]", 5); 
    glutAddMenuEntry("Toggle smoothing [s]", 7); 
    glutAddMenuEntry("Toggle face culling [e]", 8); 
    glutAddMenuEntry("Toggle line strip", 9); 
    glutAddMenuEntry("Toggle Voronoi points", 12); 
    glutAddMenuEntry("Toggle Voronoi tetrahedra", 13); 
    glutAddMenuEntry("Toggle Voronoi territories", 14); 
    // next is 12 
 
    int submenuR = glutCreateMenu(menuRaytrace); 
    glutAddMenuEntry("Ray enquire [+]", 1);
    glutAddMenuEntry("Pick transformed object [-]", 9); 
    if(image_type() == SV_BMP) 
    { 
      glutAddMenuEntry("Raytrace this to sv_ray.bmp", 2); 
    } else 
    { 
      glutAddMenuEntry("Raytrace this to sv_ray.ppm", 2); 
    } 
    glutAddMenuEntry(MENU_SEP, -1); 
    sprintf(tmp_str, "Low contents is %d", low_c); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry("Decrease low contents by 1", 3);     
    glutAddMenuEntry("Increase low contents by 1", 4); 
    glutAddMenuEntry(MENU_SEP, -1); 
    sprintf(tmp_str, "Box swell is %.4f", swell); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry("Box swell /2", 5);     
    glutAddMenuEntry("Box swell x2", 6);  
    glutAddMenuEntry(MENU_SEP, -1); 
    sprintf(tmp_str, "Small box is %.4f", little_b); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry("Small box /2", 7);     
    glutAddMenuEntry("Small box x2", 8);  
 
 
    int submenuBC = glutCreateMenu(menuBGColour); 
    glutAddMenuEntry("Black", 1); 
    glutAddMenuEntry("Grey", 2); 
    glutAddMenuEntry("White", 3); 
    glutAddMenuEntry("Wheat", 4); 
 
    int submenuPrefs = glutCreateMenu(menuPrefs); 
    sprintf(tmp_str, "Near clip plane is %.2f", near_clip); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry("Decrease near clip plane by 1", 1); 
    glutAddMenuEntry("Increase near clip plane by 1", 2); 
    glutAddMenuEntry("Set near clip plane to 0.1", 3); 
    glutAddMenuEntry("Set near clip plane to 1", 4); 
    glutAddMenuEntry("Set near clip plane to 2", 5); 
    glutAddMenuEntry("Set near clip plane to 5", 6); 
    glutAddMenuEntry("Set near clip plane to 10", 7); 
    glutAddMenuEntry(MENU_SEP, -1); 
    sprintf(tmp_str, "Far clip plane is %.0f", far_clip); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry("Decrease far clip plane by 10", 101); 
    glutAddMenuEntry("Increase far clip plane by 10", 102); 
    glutAddMenuEntry("Set far clip plane to 10", 103); 
    glutAddMenuEntry("Set far clip plane to 20", 104); 
    glutAddMenuEntry("Set far clip plane to 50", 105); 
    glutAddMenuEntry("Set far clip plane to 100", 106); 
    glutAddMenuEntry("Set far clip plane to 1000", 107); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Halve fog density", 150); 
    glutAddMenuEntry("Double fog density", 151); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Toggle ortho/perspective", 200); 
 
    int submenuFacet = glutCreateMenu(menuFacet); 
    sprintf(tmp_str, "Facet refinement: %.3f", get_user_grad_fac()); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Refine facets /2", 1); 
    glutAddMenuEntry("Refine facets /4", 2); 
    glutAddMenuEntry("Refine facets /8", 3); 
    glutAddMenuEntry("Refine facets /16", 4); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Coarsen facets x2", 101); 
    glutAddMenuEntry("Coarsen facets x4", 102); 
    glutAddMenuEntry("Coarsen facets x8", 103); 
    glutAddMenuEntry("Coarsen facets x16", 104); 
    glutAddMenuEntry(MENU_SEP, -1); 
    sprintf(tmp_str, "Box swell factor: %.4f", get_swell_fac()); 
    glutAddMenuEntry(tmp_str, -1); 
    glutAddMenuEntry(MENU_SEP, -1); 
    glutAddMenuEntry("Set box swell to 0.05", 201); 
    glutAddMenuEntry("Set box swell to 0", 202); 
    glutAddMenuEntry("Double box swell", 203); 
    glutAddMenuEntry("Halve box swell", 204); 
 
    glutCreateMenu(menuRMB); 
    glutAddSubMenu("Functions", submenuF); 
    glutAddSubMenu("Draw style", submenuDS); 
    glutAddSubMenu("Raytracing", submenuR); 
    glutAddSubMenu("Background colour", submenuBC); 
    glutAddSubMenu("Projection", submenuPrefs); 
    glutAddSubMenu("Facet quality", submenuFacet); 
    glutAddMenuEntry("Exit [Esc]", 100); 
 
    glutAttachMenu(GLUT_RIGHT_BUTTON); 
 
// Initialise OpenGL; compile the model's polygons 
  
    static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0 }; 
 
    glClearColor(bgc.x, bgc.y, bgc.z, 1); 
    glLightfv( GL_LIGHT0, GL_POSITION, pos ); 
    glEnable( GL_CULL_FACE ); 
    glEnable( GL_LIGHTING ); 
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); 
    glEnable( GL_LIGHT0 ); 
    glEnable( GL_DEPTH_TEST ); 
    glPointSize(2); 
    fog_d = get_haze()/box_d; 
    glFogf(GL_FOG_DENSITY, fog_d); 
    sv_point fc = get_mist(); 
    GLfloat fp[4] = {fc.x, fc.y, fc.z, 0}; 
    glFogfv(GL_FOG_COLOR, fp); 
 
    sv_compile_model(); 
 
    glEnable( GL_NORMALIZE );
 
    trackball(curquat, 0.0, 0.0, 0.0, 0.0); 
    newModel = 1; 

// Register the callback procedures

    callbacks(); 

    sv_draw(1); 
 
// That bourn (shell?) from which no traveller returns 
 
    glutMainLoop(); 
} 
 
//********************************************************************************

// User-called functions
 
void plot_m_p_gons( const sv_model& m, char* title ) 
{
   sv_enclosure(m.box()); 
   if(thread_running)
   {
	svm->model(m);
	sv_draw_all();
   } else
      add_model_to_picture(m, SV_OO, SV_ZL, 0, 0);
} 

// Plot the model _and_ its boxes 
 
void plot_m_boxes( const sv_model& m, sv_integer plot_mod, char* title ) 
{ 
	box_mod = m; 
	sv_model mf = sv_model(m.set_list(), m.box(), sv_model()); 
	switch(plot_mod) 
	{ 
	case SV_NM_NE: 
		plot_empties = 0; 
		break; 
	case SV_NM_E: 
		plot_empties = 1; 
		break; 
	case SV_M_NE: 
		plot_empties = 0; 
		mf = mf.facet(); 
		break; 
	case SV_M_E: 
		plot_empties = 1; 
		mf = mf.facet(); 
		break; 
	default: 
		svlis_error("plot_m_boxes","dud plot_mod value", 
			SV_WARNING); 
	} 
	plot_m_p_gons(mf, title); 
} 

// Plot a Voronoi diagram

void plot_voronoi(const sv_voronoi& v, char* title, 
	sv_integer pv, sv_integer pd, sv_integer px, 
	sv_real tf) 
{ 
   plot_v_v = pv; 
   plot_v_d = pd; 
   plot_v_x = px; 
   tet_fac = tf; 
   svv = v;
   sv_enclosure(v.box()); 
   if(thread_running)
   {
	force_recompile = 1;
	Re_Draw();
   } else
	first_model(title);
} 

//*******************************************************************************
 
#if macintosh 
 #pragma export off 
#endif 
 
 
 
 
 
 
 
 
 
 
 

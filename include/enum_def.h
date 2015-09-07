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
 * SvLis - all the enums and #defines
 *   plus pre-declaration of all svLis structs and classes
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 2 April 1996
 * This version: 23 September 2000
 *
 */

#ifndef SVLIS_EN_DEF
#define SVLIS_EN_DEF

// Useful to allow switch to doubles/shorts etc easily:

#define sv_real float
#define sv_integer long

// End of line on different systems

#ifdef SV_UNIX
 #define SV_EL "\n"
#endif

#ifdef SV_MSOFT
 #define SV_EL "\r\n"
#endif

#if macintosh
 #define SV_EL "\n"
#endif

// The coordinate directions and origin; useful in all sorts
// of places; also the positive diagonal

#define SV_X sv_point(1, 0, 0)
#define SV_Y sv_point(0, 1, 0)
#define SV_Z sv_point(0, 0, 1)
#define SV_OO sv_point(0, 0, 0)
#define SV_00 SV_OO                 // Saves remembering...
#define SV_DIAG sv_point(1, 1, 1)

// Coordinate planes

#define SV_XP sv_plane(SV_X,  SV_OO)
#define SV_YP sv_plane(SV_Y,  SV_OO)
#define SV_ZP sv_plane(SV_Z,  SV_OO)

// Coordinate axes

#define SV_XL sv_line(SV_X,  SV_OO)
#define SV_YL sv_line(SV_Y,  SV_OO)
#define SV_ZL sv_line(SV_Z,  SV_OO)

// Declaration list of Svlis structs and classes

struct sv_point;
struct sv_line;
struct sv_plane;
struct sv_i;
struct sv_interval;
struct sv_box;
struct sv_p_gon;
struct sv_light_list;
struct sv_pixel;
class sv_primitive;
class sv_set;
class sv_set_list;
class sv_user_attribute;
class sv_attribute;
class sv_model;
class sv_surface;
class sv_delaunay;
class sv_vertex;
class sv_voronoi;
class sv_view;
class sv_lightsource;
class sv_picture;

// Useful to have reals, integers, and text in the tag scheme

extern sv_integer real_tag();
extern sv_integer integer_tag();
extern sv_integer text_tag();

/********************************************************************/

// This section used to be in private.h

// Random number data

#define BIG_INT 0x7fffffff   /* 2^31 - 1  Used to mask out the sign bit */
#define R1 715136305
#define R2 2147001325

// General purpose small factor - used for getting an idea of how much
// smaller than X a thing has to be to be ignored w.r.t. X

#define QUITE_SMALL 1.0e-5

// A small part of a length value used for computing rough approximations.

#define SMALL_LEN 0.01

// How much smaller a divided box has to be from the parent box for the faceter
// not to bother any more

#define dont_bother_fac 80.0

// The flag bit that indicates that an object has been written

#define WRIT_BIT 0x10000000

// Flag bit for polygons

#define SV_POLYGON_FLAG 0x20000000

// Flag bit for convex polyhedron

#define SV_CV_POL 0x40000000

// Version number write/check

#define SV_VER 4

// Flag privalage mask - the user can't set the 16 msb of a flag

#define FLAG_MASK 0x0000ffff

//*******************************************************************************

// Errors and flags

// Here are the error levels

enum sv_err
{
	SV_DEBUG,
	SV_COMMENT,
	SV_WARNING,
	SV_FATAL,
	SV_CORRUPT
};

// Flag mechanism

// There are many occasions when an operation may `fail', though it is
// perfectly legitimate to attempt it (such as trying to find the line
// of intersection between two parallel planes).  Svlis uses a 
// flagging mechanism to record this.  This file, flags.cxx, and sve.cxx
// are the error and flag mechanisms

enum flag_val
{
	NO_FLAG,	// Zero - nothing amiss
	LONG_CROSS0,	// Near parallel - cross a long way away
	LONG_CROSS1,
	NO_CROSS, 	// Planes/lines etc do not cross at all
	NO_CONVERGE	// Iteration has not converged
};

/********************************************************************/

// Range of gradient values represented by a box

// BSP_C on return means that nothing useful is returned as the centroid of
// the box is the origin.
// BSP_S means that the box contains the origin, but everything will be set.
// BSP_OK means everythin will be set
// BSP_DUD means an error has occured.

enum bsp_type
{
	BSP_C,
	BSP_S,
	BSP_OK,
	BSP_DUD
};

/********************************************************************/

// Primitives section

#define S_U_PRIM 1024	// Minimum value for a user-defined primitive
#define USER_DEGREE 100	// `degree' of a user-defined primitive

// The different types of primitive

enum prim_kind
{
	SV_REAL,		// Just a number
	SV_PLANE,		// a plane
	SV_CYLINDER,		// Special shapes
	SV_SPHERE,
	SV_CONE,
	SV_TORUS,
	SV_CYCLIDE,
	SV_GENERAL		// Any old primitive
};

// Arithmetic on primitives; operators must be all the diadic, followed
// by all the monadic.  

enum prim_op
{
	SV_ZERO,	// Don't want 0 as an operator
	SV_PLUS,
	SV_MINUS,
	SV_TIMES,
	SV_DIVIDE,
	SV_POW,
	SV_COMP,
	SV_ABS,
	SV_SIN,
	SV_COS,
	SV_EXP,
	SV_SSQRT,
        SV_SIGN
};

#define MONADIC SV_COMP	// First monadic operator
#define diadic(a) ((a) < MONADIC)  // Test for type of operator

/********************************************************************/

// Polygons

enum sv_p_gon_kind
{
	PT_SET,		// Point set
	P_LINE,		// Polyline
	P_GON		// Polygon
};

// Flag bit, invisible bit, and too-far-away bits

enum
{
	DEAD=1,
	INVISIBLE=2,
	FAR0=4,
	FAR1=8, 
	NEW=16
};

// Special cases for the faceter

enum 
{
	SV_F_PLANES = 1,
	SV_F_CONE = 2
};

// Box aspect-ratio calculation

#define LONG_THIN 0.2
#define THIN_EL 0.3

/********************************************************************/

// Sets

// Results of a membership test

enum mem_test
{
	SV_AIR,		// point/box is all in air
	SV_SURFACE,	// point is on surface/box (may) contain surface
	SV_SOLID	// point/box is all in solid
};

#define SV_EVERYTHING -1	// Universal and null sets
#define SV_NOTHING 0

// Set operators

enum set_op
{
        SV_UNION,
        SV_INTERSECTION
};

/********************************************************************/

// Models

enum mod_kind
{
	LEAF_M,		// Undivided leaf model
	X_DIV,		// Divided in x direction
	Y_DIV,		// Divided in y direction
	Z_DIV		// Divided in z direction
};

// Set the box swell factor

#define DEF_SWELL_FAC 0.03

/********************************************************************/

// The svLis editor

// Set operators

enum sv_ed_op
{
	UNI,
	INTS,
	DIF,
	S_DIF
};

// Default file name and extension

#define DEF_EXTN ".mod"
#define PIC_EXTN ".ppm"                             
#define DEF_NAME "noname"

// Default box size

#define DEF_BOX sv_box(sv_point(-10, -10, -10), sv_point(10, 10, 10))

// Up direction

#define UPWARDS SV_Z

// Default view point

#define DEFVP sv_point (1.5, 2.5, 1.7)

// Default cuboid

#define DEF_CUBOID sv_box (sv_point(-5,-5,-5), sv_point(5,5,5))

// Default set
              
#define DEF_SET sv_set (SV_NOTHING)

// Default axis and radius

#define DEF_AXIS sv_line (SV_X, SV_OO)
#define DEF_RAD 1

// Default centre

#define DEF_CEN SV_OO

// Default plane

#define DEF_PLANE sv_plane (SV_Z, SV_OO)

// Default lamp name

#define DEF_LAMP "L_0"
              
/********************************************************************/

// Miscellaneous...

// Default little box factor and low contents

#define SV_SMALL_BOX 0.0001
#define SV_LOW_CONTENTS 3

// Default colour attribute - grey

#define DEF_COL sv_point (0.5, 0.5, 0.5)

// Initial number of Monte Carlo points

#define N_MONTE 50

// Visual C++ math.h doesn't have PI in . . .

#ifndef M_PI
  #define M_PI 3.1415926
#endif

// For the plot_m_boxes procedure

#define SV_NM_NE 0
#define SV_NM_E 1
#define SV_M_NE 2
#define SV_M_E 3

// Background minimum light

#define DARK 0.3

// Depth buffer default far distance

#define LONG_WAY 1.0e30    // Bit of a hack

// Size of a graphics window

#define DEF_PIC_X 400
#define DEF_PIC_Y 300

//*****************************************************************

// I/O stuff

// The struct and class tags - also tags for i/o formatting

enum sv_tag
{
	SVT_XXXX,		// Don't want 0
	SVT_POINT,
	SVT_LINE,
	SVT_PLANE,
	SVT_INTERVAL,
	SVT_BOX,
	SVT_PRIM,
	SVT_SET,
	SVT_SET_LIST,
	SVT_USER_ATTRIBUTE,
	SVT_ATTRIBUTE,
	SVT_POLYGON,
	SVT_MODEL,
	SVT_TEXT,
	SVT_REAL,
	SVT_INTEGER,
	SVT_SURFACE,
	SVT_OB_M,		// { models
	SVT_OB_L,		// [ set lists
	SVT_OB_S,		// ( sets
	SVT_OB_P,		// < primitives & attributes
	SVT_OB,			// \ lower than primitives
	SVT_CB_M,		// }
	SVT_CB_L,		// ]
	SVT_CB_S,		// )
	SVT_CB_P,		// >
	SVT_CB,			// /	
	SVT_SVLIS,		// Master tag ("SvLis")
	SVT_DELAUNAY,		// Voronoi stuff
	SVT_VERTEX, 
	SVT_VORONOI,
        SVT_VIEW,
	SVT_LSOURCE,
	SVT_LLIST,
	SVT_PIC,
        SVT_PIXEL,
	SVT_I,

// Add more from here...
	
	SVT_TOP_TT
};

#define SVT_TOP_T SVT_TOP_TT	    // Compiler winges if this is absent
#define SVT_F 2 		    // Gap between tags
#define SVT_TOP (SVT_F*SVT_TOP_T)   // Bigger than the biggest tag
#define SV_COM_CHAR '#'    // The comment character in svLis files
#define SV_TAG_S_LEN 30    // Maximum length of a tag string
#define SV_TAB_LEN 8	   // Number of spaces/TAB

//***********************************************************

// The light class stores lighting parameters that are used to 
// generate an image

enum light_type
{
   PARALLEL,
   POINT_SOURCE
};

//************************************************************

// Voronoi diagram 

#define SV_VD 3     // Number of dimensions
#define SV_VD1 4    // SV_VD + 1

// Flags for recursive walks.

enum v_flag
{
     SV_VF0 = 0, 
     SV_DED = 1, 
     SV_VISITED_1 = 2, 
     SV_VISITED_2 = 4,
     SV_VISITED_3 = 8,
     SV_WALK = 16, 
     SV_CH = 32, 
     SV_EMPTY = 64, 
     SV_IN_SOLID = 128, 
     SV_ON_FACE = 256, 
     SV_ON_EDGE = 512, 
     SV_ON_CORNER = 1024
};

//*************************************************************

// Texture maps

enum tex_type
{
	SV_IMAGE = 0,
	SV_TILED = 1,
	SV_SOLID_TEX = 2
};

//**************************************************************

// Image files

#define SV_BMP_FHSZ 14
#define SV_BMP_OLDHSZ 12
#define SV_BMP_NEWHSZ 40
#define SV_BMP_OS2HSZ 64

enum sv_image_type
{
  SV_PPM,
  SV_BMP
};

//***************************************************************

// Graphics

#define BG_COL sv_point(0.96, 0.871, 0.702)

enum mouse_mode_t 				// Mouse button & ctrl key state
{
	none, 
	svrotate, 
	translate, 
	zoom
}; 

// What action is being done to a thing in a model list?

enum sv_xform_action
{
	SV_XF_0, 
	SV_TRANSFORM, 
	SV_PICK
};

// How many digits in an animated frame number

#define SV_FC_MAX 6

//****************************************************************
#endif














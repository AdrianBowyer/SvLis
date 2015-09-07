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
 * SvLis - quickview raytraced picture
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

 
#ifndef SVLIS_QV
#define SVLIS_QV

sv_integer generate_quickview_picture(const sv_model& m, const sv_view& view_params,
			       const sv_light_list& light_list, sv_picture& picture_params,
			       sv_real progress_report_step,
			       void report_procedure(sv_real percent));


struct qv_rectangle {
   sv_integer x_pos;		// X-coordinate of min corner
   sv_integer y_pos;		// Y-coordinate of min corner
   sv_integer width;		// Width of region
   sv_integer height;		// Height of region
   sv_integer step;		// Pixel step (starting from (x_pos,y_pos)
   sv_integer prev_step;	// Previous value of `step'
   sv_integer nice;		// Niceness value (lower == better)
   };


void init_qv_rectlist(void);
void add_qv_rect(qv_rectangle rect);
sv_integer get_qv_rect(qv_rectangle& rect);
void print_qv_rectlist(void);

#endif

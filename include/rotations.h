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
 * SvLis - this handles rotational transforms.
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 2 July 1999
 * This version: 8 March 2000
 *
 */

/* 
 * This is a modified and extended version of the SGI code
 * trackball.c and trackball.h.  See below for SGI's copyright
 * notice.
 */

// This handles 5 rotational representations:

// 1. Svlis line and angle
// 2. Quaternions
// 3. 3x3 matrices
// 4. Euler angles
// 5. Homogeneous 4x4 matrices

// Type 1 is the most general, as it allows a rotation anywhere in space.
// The remaining 4 all rotate about the origin.

#ifndef SVLIS_ROTATIONS
#define SVLIS_ROTATIONS


/*
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */
/*
 * trackball.h
 * A virtual trackball implementation
 * Written by Gavin Bell for Silicon Graphics, November 1988.
 */

/*
 * Pass the x and y coordinates of the last and current positions of
 * the mouse, scaled so they are from (-1.0 ... 1.0).
 *
 * The resulting rotation is returned as a quaternion rotation in the
 * first paramater.
 */

void trackball(sv_real q[4], const sv_real p1x, const sv_real p1y, 
	       const sv_real p2x, const sv_real p2y);

/*
 * Given two quaternions, add them together to get a third quaternion.
 * Adding quaternions to get a compound rotation is analagous to adding
 * translations to get a compound translation.  When incrementally
 * adding rotations, the first argument here should be the new
 * rotation, the second and third the total rotation (which will be
 * over-written with the resulting new total rotation).
 */

void add_quats(const sv_real *q1, sv_real *q2, sv_real *dest);

/*
 * A useful function, builds a rotation matrix in Matrix based on
 * given quaternion.
 */
// (This was build_rotmatrix in SGI's original, the arguments were reversed,
// and the output was a homogeneous matrix.)

void quaternionToMatrix(const sv_real q[4], sv_real m[][3]);

// Go the other way

void matrixToQuaternion(const sv_real m[][3], sv_real q[4]);

// Get an angle between 0 and 2 PI

inline void normalize_angle(sv_real& phi)
{
	while(phi < 0) phi = phi + 2*M_PI;
	while(phi > 2*M_PI) phi = phi - 2*M_PI;
}

// Go between Matrices and Euler angles

inline void matrixToEuler(const sv_real mr[][3], sv_real& phi, sv_real& theta, sv_real& psi)
{
// From Latombe p72

	phi = atan2(mr[1][2], mr[0][2]);
	theta = atan2(sqrt(mr[2][0]*mr[2][0] + mr[2][1]*mr[2][1]), mr[2][2]);
	psi = atan2(mr[2][1], -mr[2][0]);
        normalize_angle(phi); 
        normalize_angle(theta);
        normalize_angle(psi);  
} 
  
void eulerToMatrix(const sv_real phi, const sv_real theta, const sv_real psi, sv_real mr[][3]);

/*
 * This function computes a quaternion based on an axis (defined by
 * the svlis line) and an angle about which to rotate.  The angle is
 * expressed in radians.  The result is put into the third argument.
 */
// (This was axis_to_quat in SGI's original, and the first argument
// was float a[3].)

void axisToQuaternion(const sv_line& axis, const sv_real a, sv_real q[4]);

// Go the other way

void quaternionToAxis(const sv_real q[4], sv_line& axis, sv_real& a);

// Invert a quaternion rotation

void invertQuaternion(const sv_real qin[], sv_real qout[]);

// Go between Quaternions and Euler angles (these should be direct)

inline void quaternionToEuler(const sv_real q[4], sv_real& phi, sv_real& theta, sv_real& psi)
{
        sv_real mr[3][3];
	quaternionToMatrix(q, mr);
	matrixToEuler(mr, phi, theta, psi);
}

inline void eulerToQuaternion(const sv_real phi, const sv_real theta, const sv_real psi, sv_real q[4])
{
	sv_real m[3][3];
	eulerToMatrix(phi, theta, psi, m);
	matrixToQuaternion(m, q);
}

// Turn a rotation matrix into a homogeneous one and back

void homogenizeMatrix(const sv_real mr[][3], sv_real mh[][4]);
void rotateMatrix(const sv_real mh[][4], sv_real mr[][3]);

// Inlines to do the others.  Quaternions are the lingua franca.


inline void axisToEuler(const sv_line& axis, const sv_real a, sv_real& phi, sv_real& theta, sv_real& psi)
{
	sv_real q[4], mr[3][3];
	axisToQuaternion(axis, a, q);
	quaternionToEuler(q, phi, theta, psi);
} 

inline void eulerToAxis(const sv_real phi, const sv_real theta, const sv_real psi, sv_line& axis, sv_real& a)
{
	sv_real q[4];
	eulerToQuaternion(phi, theta, psi, q);
	quaternionToAxis(q, axis, a);
}

inline void axisToMatrix(const sv_line& axis, const sv_real a, sv_real mr[][3])
{
	sv_real q[4];
	axisToQuaternion(axis, a, q);
	quaternionToMatrix(q, mr);
}

inline void matrixToAxis(const sv_real mr[][3], sv_line& axis, sv_real& a)
{
	sv_real q[4];
	matrixToQuaternion(mr, q);
	quaternionToAxis(q, axis, a);
}

// Work out the rotation that matches one point to another

inline void pointsToAxis(const sv_point& p0, const sv_point& p1, sv_point& ax, sv_real& angle)
{
        sv_point pa = p0.norm();
        sv_point pb = p1.norm();
        ax = pa^pb;
	if(ax.mod() < 0.000001)
	  ax = right((pa + pb)/2);

// Find orthogonal vector to ax and pb

	ax = ax.norm();

       sv_point w = ax^pb;

// Now we have a local coordinate system ax, pb and w

       sv_real p = pa*pb;
       sv_real q = pa*w.norm();
       angle = atan2(q, p);
       normalize_angle(angle);
}

// Work out the quaternion that matches one point to another rotating about a given axis

inline void pointsAxisToQuaternion(const sv_point& p0, const sv_point& p1, const sv_point& ax, sv_real q[4])
{
        sv_point u, v, w;
	sv_axes(ax, u, v, w);
	sv_real u0 = p0*u;
	sv_real v0 = p0*v;
	sv_real u1 = p1*u;
	sv_real v1 = p1*v;
	sv_real a1 = atan2(v0, u0);
	sv_real angle = atan2(v1, u1);
	angle = angle - a1;
	normalize_angle(angle);
	axisToQuaternion(sv_line(ax, SV_OO), angle, q);  
}

// Turn a svLis-style rotation into a rotation about the origin and a subsequent translation

inline sv_line axisToAxisTranslate(const sv_line& axis, sv_real angle, sv_point& trans)
{
  sv_line ax = axis - axis.origin;
  trans = axis.origin - axis.origin.spin(ax, angle);
  return(ax);
}

#endif


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
 * Trackball code:
 *
 * Implementation of a virtual trackball.
 * Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 *   the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 *
 * Vector manip code:
 *
 * Original code from:
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
 *
 * Much mucking with by:
 * Gavin Bell
 */

#if defined(SV_MSOFT)
#pragma warning (disable:4244)          /* disable bogus conversion warnings */
#endif

#include "sv_std.h"
#include "enum_def.h"
#include "flag.h"
#include "sums.h"
#include "geometry.h"
#include "rotations.h"

#if macintosh
 #pragma export on
#endif

/*
 * This size should really be based on the distance from the center of
 * rotation to the point on the object underneath the mouse.  That
 * point would then track the mouse as closely as possible.  This is a
 * simple example, though, so that is left as an Exercise for the
 * Programmer.
 */
#define TRACKBALLSIZE  (0.8)

/*
 * Local function prototypes (not defined in trackball.h)
 */
static sv_real tb_project_to_sphere(sv_real, sv_real, sv_real);
static void normalize_quat(sv_real [4]);

void
vzero(sv_real *v)
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

void
vset(sv_real *v, sv_real x, sv_real y, sv_real z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void
vsub(const sv_real *src1, const sv_real *src2, sv_real *dst)
{
    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];
}

void
vcopy(const sv_real *v1, sv_real *v2)
{
    register int i;
    for (i = 0 ; i < 3 ; i++)
        v2[i] = v1[i];
}

void
vcross(const sv_real *v1, const sv_real *v2, sv_real *cross)
{
    sv_real temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}

sv_real
vlength(const sv_real *v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void
vscale(sv_real *v, sv_real div)
{
    v[0] *= div;
    v[1] *= div;
    v[2] *= div;
}

void
vnormal(sv_real *v)
{
    vscale(v,1.0/vlength(v));
}

sv_real
vdot(const sv_real *v1, const sv_real *v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void
vadd(const sv_real *src1, const sv_real *src2, sv_real *dst)
{
    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * trackball, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed trackball-- is a trackball in the center,
 * but is deformed into a hyperbolic sheet of rotation away from the
 * center.  This particular function was chosen after trying out
 * several variations.
 *
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */
void
trackball(sv_real q[4], const sv_real p1x, const sv_real p1y, const sv_real p2x, const sv_real p2y)
{
    sv_real a[3]; /* Axis of rotation */
    sv_real phi;  /* how much to rotate about axis */
    sv_real p1[3], p2[3], d[3];
    sv_real t;

    if (p1x == p2x && p1y == p2y) {
        /* Zero rotation */
        vzero(q);
        q[3] = 1.0;
        return;
    }

    /*
     * First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere
     */
    vset(p1,p1x,p1y,tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));
    vset(p2,p2x,p2y,tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));

    /*
     *  Now, we want the cross product of P1 and P2
     */
    vcross(p2,p1,a);

    /*
     *  Figure out how much to rotate around that axis.
     */
    vsub(p1,p2,d);
    t = vlength(d) / (2.0*TRACKBALLSIZE);

    /*
     * Avoid problems with out-of-control values...
     */
    if (t > 1.0) t = 1.0;
    if (t < -1.0) t = -1.0;
    phi = 2.0 * asin(t);

    sv_line axis = sv_line(sv_point(a[0], a[1],a[2]), SV_OO);
    axisToQuaternion(axis,phi,q);
}

// Turn a quaternion into a svLis rotation axis and angle 
 
void quaternionToAxis(const sv_real q[4], sv_line& axis, sv_real& a) 
{ 
    axis.direction.x = q[0]; 
    axis.direction.y = q[1]; 
    axis.direction.z = q[2]; 
    axis.origin = SV_OO; 
    a = acos(q[3]); 
    if(a != 0) 
    	axis.direction = axis.direction/sin(a); 
    else 
        axis.direction = SV_Z; 
    a = 2*a;

#if 0
// Go the short way

    if(a > M_PI)
    {
	a = 2*M_PI - a;
	axis = -axis;
    }
#endif
} 

/*
 *  Given an axis and angle, compute quaternion.
 */
void
axisToQuaternion(const sv_line& axis, sv_real a, sv_real q[4])
{
    if(axis.origin.mod() > QUITE_SMALL)
      svlis_error("axisToQuaternion","Axis does not run through the origin", SV_WARNING);
    vcopy(&axis.direction.x,q);
    vscale(q,sin(a/2.0));
    q[3] = cos(a/2.0);
}

/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
static sv_real
tb_project_to_sphere(sv_real r, sv_real x, sv_real y)
{
    sv_real d, t, z;

    d = sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440) {    /* Inside sphere */
        z = sqrt(r*r - d*d);
    } else {           /* On hyperbola */
        t = r / 1.41421356237309504880;
        z = t*t / d;
    }
    return z;
}

/*
 * Given two rotations, e1 and e2, expressed as quaternion rotations,
 * figure out the equivalent single rotation and stuff it into dest.
 *
 * This routine also normalizes the result every RENORMCOUNT times it is
 * called, to keep error from creeping in.
 *
 * NOTE: This routine is written so that q1 or q2 may be the same
 * as dest (or each other).
 */

#define RENORMCOUNT 97

void
add_quats(const sv_real q1[4], sv_real q2[4], sv_real dest[4])
{
    static int count=0;
    sv_real t1[4], t2[4], t3[4];
    sv_real tf[4];

    vcopy(q1,t1);
    vscale(t1,q2[3]);

    vcopy(q2,t2);
    vscale(t2,q1[3]);

    vcross(q2,q1,t3);
    vadd(t1,t2,tf);
    vadd(t3,tf,tf);
    tf[3] = q1[3] * q2[3] - vdot(q1,q2);

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++count > RENORMCOUNT) 
    {
        count = 0;
        normalize_quat(dest);
    }
}

/*
 * Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 * If they don't add up to 1.0, dividing by their magnitued will
 * renormalize them.
 *
 * Note: See the following for more information on quaternions:
 *
 * - Shoemake, K., Animating rotation with quaternion curves, Computer
 *   Graphics 19, No 3 (Proc. SIGGRAPH'85), 245-254, 1985.
 * - Pletinckx, D., Quaternion calculus as a basic tool in computer
 *   graphics, The Visual Computer 5, 2-13, 1989.
 */
static void
normalize_quat(sv_real q[4])
{
    int i;
    sv_real mag;

    mag = (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    for (i = 0; i < 4; i++) q[i] /= mag;
}

void invertQuaternion(const sv_real qin[], sv_real qout[])
{
	sv_real mag = qin[0]*qin[0] + qin[1]*qin[1] + qin[2]*qin[2] + qin[3]*qin[3];
	qout[0] = -qin[0]/mag;
	qout[1] = -qin[1]/mag;
	qout[2] = -qin[2]/mag;
	qout[3] = qin[3]/mag;
}

/*
 * Build a rotation matrix, given a quaternion rotation.
 *
 */
void
quaternionToMatrix(const sv_real q[4], sv_real m[][3])
{
    m[0][0] = 1 - 2*(q[1]*q[1] + q[2]*q[2]);
    m[0][1] = 2*(q[0]*q[1] - q[2]*q[3]);
    m[0][2] = 2*(q[2]*q[0] + q[1]*q[3]);

    m[1][0] = 2*(q[0]*q[1] + q[2]*q[3]);
    m[1][1] = 1 - 2*(q[2]*q[2] + q[0]*q[0]);
    m[1][2] = 2*(q[1]*q[2] - q[0]*q[3]);

    m[2][0] = 2*(q[2]*q[0] - q[1]*q[3]);
    m[2][1] = 2*(q[1]*q[2] + q[0]*q[3]);
    m[2][2] = 1 - 2*(q[1]*q[1] + q[0]*q[0]);
}

void matrixToQuaternion(const sv_real m[][3], sv_real q[])
{
	sv_real t;
	q[3] = 0.25*(1 + m[0][0] + m[1][1] + m[2][2]);
	if(q[3] > QUITE_SMALL)
	{
		q[3] = sqrt(q[3]);
		t = 0.25/q[3];
		q[0] = t*(m[1][2] - m[2][1]);
		q[1] = t*(m[2][0] - m[0][2]);
		q[2] = t*(m[0][1] - m[1][0]);
	} else
	{
		q[3] = 0;
		q[0] = -0.5*(m[1][1] + m[2][2]);
		if(q[0] > QUITE_SMALL)
		{
			q[0] = sqrt(q[0]);
			q[1] = 0.5*m[0][1]/q[0];
			q[2] = 0.5*m[0][2]/q[0];
		} else
		{
			q[0] = 0;
			q[1] = 0.5*(1 - m[2][2]);
			if(q[1] > QUITE_SMALL)
			{
				q[1] = sqrt(q[1]);
				q[2] = 0.5*m[1][2]/q[1];
			} else
			{
				q[1] = 0;
				q[2] = 1;
			}
		}
	}
	invertQuaternion(q,q);  // Why's this needed then?
}

// Turn a rotation matrix into a homogeneous one

void homogenizeMatrix(const sv_real mr[][3], sv_real mh[][4])
{
	for(int i = 0; i< 3; i++)
	 for(int j = 0; j< 3; j++)
	  mh[i][j] = mr[i][j];

	mh[0][3] = 0.0;
	mh[1][3] = 0.0;
	mh[2][3] = 0.0;
	mh[3][0] = 0.0;
	mh[3][1] = 0.0;
	mh[3][2] = 0.0;
	mh[3][3] = 1.0;
}

// Go the other way

void rotateMatrix(const sv_real mh[][4], sv_real mr[][3])
{
	for(int i = 0; i< 3; i++)
	 for(int j = 0; j< 3; j++)
	  mr[i][j] = mh[i][j];
}


void eulerToMatrix(const sv_real phi, const sv_real theta, const sv_real psi, sv_real m[][3])
{
       sv_real c_phi = cos(phi);
       sv_real c_theta = cos(theta);
       sv_real c_psi = cos(psi);
            
       sv_real s_phi = sin(phi);
       sv_real s_theta = sin(theta);
       sv_real s_psi = sin(psi);

// Nnagy_F_1987_B, p78.
       m[0][0] = c_phi*c_theta*c_psi - s_phi*s_psi; m[0][1] = -c_phi*c_theta*s_psi - s_phi*c_psi; m[0][2] = c_phi*s_theta;
       m[1][0] = s_phi*c_theta*c_psi + c_phi*s_psi; m[1][1] = -s_phi*c_theta*s_psi + c_phi*c_psi; m[1][2] = s_phi*s_theta;
       m[2][0] = -s_theta*c_psi                   ; m[2][1] = s_theta*s_psi                     ; m[2][2] = c_theta;
}

#if macintosh
 #pragma export off
#endif

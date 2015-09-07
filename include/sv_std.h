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
 * SvLis - this pulls in all the system .h files 
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 29 December 1997 
 * This version: 8 March 2000
 *
 */

 
#ifndef SVLIS_STANDARD 
#define SVLIS_STANDARD 
 
// Universally applicable includes 
 
#include <math.h> 
#include <iostream> 
#include <fstream> 
#include <ctype.h> 
#include <time.h> 
#include <stdlib.h> 
#include <new> 
#include <signal.h> 
#include <stdio.h>

// Spacemouse only works under X at the moment

#ifdef SV_SPACEMOUSE
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #include <X11/Xos.h>
 #include <X11/Xatom.h>
 #include <X11/keysym.h>
 #ifdef __cplusplus
  extern "C" {
 #endif
 #include "xdrvlib.h"
 #ifdef __cplusplus
  }
 #endif
#endif

#include "GL/glut.h" 
 
// Architecture-dependent includes 
 
// Microsoft Windows 
 
#ifdef SV_MSOFT 
 #include "strstrea.h"  //Yuk! Blame Bill Gates 
 #include "process.h"
 #include "sys/stat.h"
#endif 
 
// Apple Mac 
 
#if macintosh 
 #include "strstream.h" 
 #include "unistd.h"
 #include <Memory.h>
 #include "Threads.h" 
 #include <SIOUX.h> 
 #include <stat.h> 
#endif
 
// Unix 

#ifdef SV_UNIX
 #include "sys/stat.h" 
 #include "sys/types.h" 
 #include <strstream> 
 #include "fcntl.h" 
 #include "unistd.h"
 #include "pthread.h"  
#endif 
 
#endif 
 

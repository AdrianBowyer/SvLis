#  The SvLis Geometric Modelling Kernel
#  ------------------------------------
#
#  Copyright (C) 1993, 1997, 1998, 2000
#  University of Bath & Information Geometers Ltd
#
#  http://www.bath.ac.uk/
#  http://www.inge.com/
#
#  Principal author:
#
#     Adrian Bowyer
#     Department of Mechanical Engineering
#     Faculty of Engineering and Design
#     University of Bath
#     Bath BA2 7AY
#     U.K.
#
#     e-mail: A.Bowyer@bath.ac.uk
#        web: http://www.bath.ac.uk/~ensab/
#
#   SvLis is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   Licence as published by the Free Software Foundation; either
#   version 2 of the Licence, or (at your option) any later version.
#
#   SvLis is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public Licence for more details.
#
#   You should have received a copy of the GNU Library General Public
#   Licence along with svLis; if not, write to the Free
#   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA,
#   or see
#
#      http://www.gnu.org/
#
# =====================================================================
#
# This Makefile is intended for GNU make (usually gmake on most systems)
# If you haven't got it, see http://www.gnu.org/
#
#
# See the svLis web site for the manual and other details:
#
#    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
#
# or see the file
#
#    docs/svlis.html
#
# First version: 15 March 1993
# This version: 12 July 2000
#
# ===========================================================================
#
# Uncomment (remove the # from the beginning of) the line for the type of 
# machine you're on:
#

MACHINE = LINUX
#MACHINE = FreeBSD
#MACHINE = SGI
#MACHINE = SUN

# then type gmake.
#
# ============================================================================
#

# The root directory containing svLis

SVLIS = .

#-----------------------------------------------------------
#
# FreeBSD
# ---
#

ifeq ("$(MACHINE)","FreeBSD")

# Name of the C++ compiler
CC = c++

#Flags to send the compiler
#FL = -DSV_AFFINE -DSV_UNIX -pthread -I/opt/Mesa-3.0/include -I/usr/X11R6/include
FL = -DSV_UNIX -pthread -I/opt/Mesa-3.0/include -I/usr/X11R6/include

#Graphics libraries

GB = -L/opt/Mesa-3.0/lib -lglut -lGLU -lGL -L/usr/X11R6/lib -lXmu -lXi -lXext -lX11 -lm 

#Debug or optimize
DEBUG = -O2

RANLIB = ranlib $(SVLIS)/lib/libsvlis.a
TAROPT = -cvf
TARFILE = svlis.tar

# Spacemouse inactive

SPACEMOUSE =

#-----------------------------------------------------------
#
# SGI
# ---
#

else

ifeq ("$(MACHINE)","SGI")

# Name of the C++ compiler
CC = CC

#Flags to send the compiler
#FL = -DSV_AFFINE -DSV_UNIX
FL = -DSV_UNIX

#Graphics libraries

GB = -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm 

#Debug or optimize
DEBUG = -OPT:Olimit=5000

# There is no ranlib on SGI systems

RANLIB = 
TAROPT = -cvf
TARFILE = svlis.tar

# Spacemouse inactive

SPACEMOUSE =

#-----------------------------------------------------------
#
#  Sun
#  ---
#

else

ifeq ("$(MACHINE)","SUN")

# Name of the C++ compiler
CC = g++

# Flags to send the compiler

# If the OpenGL include files are not in the standard search path, add
# -I[OpenGL include directory] onto the end of the next line
# where [OpenGL include directory] is wherever they are.

#FL = -DSV_AFFINE -DSV_UNIX -pedantic -fPIC -funroll-loops -DSHM
FL = -DSV_UNIX -pedantic -fPIC -funroll-loops -DSHM

# Graphics libraries

# If the OpenGL libraries are not in the standard search path, put
# -L[OpenGL library directory] before the -lglut on the next line
# where [OpenGL library directory] is wherever they are.

GB = -L/usr/local/lib -lglut -lGLU -lGL -lXt -L/usr/openwin/lib -lXext \
       -lX11 -lXi -lXext -lXmu -lXi -lX11 -lm -Wl,-R/usr/openwin/lib \
	-lthread -lpthread

# Debug or optimize (-g or -O)
DEBUG = -O

# Define the ranlib command

RANLIB = ranlib $(SVLIS)/lib/libsvlis.a
TAROPT = -cvf
TARFILE = svlis.tar

# Spacemouse inactive

#SPACEMOUSE =

#-----------------------------------------------------------
#
# Linux
# -----
#
# (This is the default if you forget to uncomment at the top.)
#

else

ifeq ("$(MACHINE)","LINUX")

# Name of the C++ compiler
CC = g++


# Flags to send the compiler

# If the OpenGL include files are not in the standard search path, add
# -I[OpenGL include directory] onto the end of the next line
# where [OpenGL include directory] is wherever they are.

#FL = -DSV_AFFINE -DSV_UNIX -pedantic -fPIC -funroll-loops -DSHM
FL = -DSV_UNIX -pedantic -fPIC -funroll-loops -DSHM

# Graphics libraries

# If the OpenGL libraries are not in the standard search path, add
# -L[OpenGL library directory] before the -lglut on the next line
# where [OpenGL library directory] is wherever they are.

GB = -lglut -lsvlis -lGLU -lGL -L/usr/X11R6/lib -lXmu -lXi -lXext -lX11 -lpthread -lm

# Debug or optimize (-pg, -g or -O)
DEBUG = -pg

# Define the ranlib command

RANLIB = ranlib $(SVLIS)/lib/libsvlis.a
TAROPT = -cvzf
TARFILE = svlis.tar.gz

# Spacemouse active if uncommented

#SPACEMOUSE = -DSV_SPACEMOUSE

else

no_target:
	echo "Uncomment the line defining your machine type at the head of the Makefile."; exit

endif
endif
endif
endif

# -------------------------------------------------------------------------

# Object, Runnable and Include directories

ODIR = $(SVLIS)/obj
RDIR = $(SVLIS)/bin
IDIR = $(SVLIS)/include

# The svLis library, programs and results directories

LDIR = $(SVLIS)/lib
PDIR = $(SVLIS)/programs
SDIR = $(SVLIS)/src
RESULTS = $(SVLIS)/results

# Libraries for the linker (with graphics)

GLIBS =	-L$(LDIR) -lsvlis $(GB)

# Arguments to send to the compiler

FLAGS = $(DEBUG) $(FL) $(SPACEMOUSE) -I$(IDIR)

# Files to put in the distribution tar set (NB pathnames RELATIVE)

DISTRIBUTION = README* LICENCE Makefile data include programs projects src docs

#Files to move from/to the PC version

PC_BU = README* LICENCE data include programs projects src docs 
PC_DIR = /dosc/local

# List of all the include files

INCLUDE = $(IDIR)/arf.h \
		$(IDIR)/arpors.h \
		$(IDIR)/attrib.h \
		$(IDIR)/decision.h \
		$(IDIR)/enum_def.h \
		$(IDIR)/environs.h \
		$(IDIR)/flag.h \
		$(IDIR)/geometry.h \
		$(IDIR)/interval.h \
		$(IDIR)/ivallist.h \
		$(IDIR)/light.h \
		$(IDIR)/model.h \
		$(IDIR)/picture.h \
		$(IDIR)/polygon.h \
		$(IDIR)/polynml.h \
		$(IDIR)/prim.h \
		$(IDIR)/qv.h \
		$(IDIR)/raytrace.h \
		$(IDIR)/sv_render.h \
		$(IDIR)/sv_set.h \
		$(IDIR)/shade.h \
		$(IDIR)/solids.h \
		$(IDIR)/sums.h \
		$(IDIR)/surface.h \
		$(IDIR)/sv_graph.h \
		$(IDIR)/rotations.h \
		$(IDIR)/sv_std.h \
		$(IDIR)/sv_util.h \
		$(IDIR)/svlis.h \
		$(IDIR)/u_attrib.h \
		$(IDIR)/view.h \
		$(IDIR)/voronoi.h \
		$(IDIR)/sv_b_cls.h

# The object file

OBJECTS = \
		$(ODIR)/attrib.o \
		$(ODIR)/flag.o\
		$(ODIR)/geometry.o \
		$(ODIR)/interval.o \
		$(ODIR)/model.o \
		$(ODIR)/polygon.o \
		$(ODIR)/prim.o \
		$(ODIR)/set.o \
		$(ODIR)/sums.o \
		$(ODIR)/svlis.o \
		$(ODIR)/arf.o \
		$(ODIR)/arpors.o \
		$(ODIR)/ivallist.o \
		$(ODIR)/polynml.o \
		$(ODIR)/raytrace.o \
		$(ODIR)/picture.o \
		$(ODIR)/environs.o \
		$(ODIR)/light.o \
		$(ODIR)/qv.o \
		$(ODIR)/render.o \
		$(ODIR)/shade.o \
		$(ODIR)/view.o \
		$(ODIR)/read1.o \
		$(ODIR)/read.o \
		$(ODIR)/rotations.o \
		$(ODIR)/sv_graph.o \
		$(ODIR)/voronoi.o \
		$(ODIR)/sve.o \
		$(ODIR)/u_attrib.o \
		$(ODIR)/u_prim.o \
		$(ODIR)/decision.o \
		$(ODIR)/sv_util.o \
		$(ODIR)/surface.o \
		$(ODIR)/niederreiter.o \
		$(ODIR)/xdrvlib.o

# Targets....
#============

install:	$(ODIR) $(LDIR) $(RESULTS) $(RDIR) all

all:		library test svlis refinery

library:        $(ODIR) $(LDIR) $(OBJECTS)
		rm -rf $(LDIR)/libsvlis.a
		ar -rus $(LDIR)/libsvlis.a $(OBJECTS)
		$(RANLIB)

# The next one is intended for general short experiments
# Edit programs/tst_progs/expt.cxx to do whatever you like

expt:		$(ODIR)/expt.o
		$(CC) -pthread -o $(RDIR)/expt $(ODIR)/expt.o $(GLIBS)

sv_display:	$(ODIR)/sv_display.o $(INCLUDE)
		$(CC) -pthread -o $(RDIR)/sv_display $(ODIR)/sv_display.o $(GLIBS)

test:		sv_tst_1 sv_tst_2 sv_tst_g engine sv_display sv_convert voronoi_tst

clean:
		rm -rf $(LDIR); rm -rf $(RESULTS); \
		rm -rf $(RDIR); rm -rf $(ODIR)

distribution:
		tar $(TAROPT) $(TARFILE) $(DISTRIBUTION)

to_pc:
		tar -cvf pc.tar $(PC_BU) \
		mv pc.tar $(PC_DIR); \
		cd $(PC_DIR)/svlis; tar -xvf ../pc.tar $(PC_BU); rm -rf ../pc.tar

$(ODIR):
		mkdir $(ODIR)

$(LDIR):
		mkdir $(LDIR)

$(RESULTS):
		mkdir $(RESULTS)

$(RDIR):
		mkdir $(RDIR)

# Test and other short programs

sv_tst_1:	$(ODIR)/sv_tst_1.o
		$(CC) -pthread -o $(RDIR)/sv_tst_1 $(ODIR)/sv_tst_1.o $(GLIBS)

sv_tst_2:	$(ODIR)/sv_tst_2.o
		$(CC) -pthread -o $(RDIR)/sv_tst_2 $(ODIR)/sv_tst_2.o $(GLIBS)

sv_tst_g:	$(ODIR)/sv_tst_g.o
		$(CC) -pthread -o $(RDIR)/sv_tst_g $(ODIR)/sv_tst_g.o $(GLIBS)

engine:		$(ODIR)/engine.o
		$(CC) -pthread -o $(RDIR)/engine $(ODIR)/engine.o $(GLIBS)

sv_convert:	$(ODIR)/sv_convert.o
		$(CC) -pthread -o $(RDIR)/sv_convert $(ODIR)/sv_convert.o $(GLIBS)

voronoi_tst:	$(ODIR)/voronoi_tst.o
		$(CC) -pthread -o $(RDIR)/voronoi_tst $(ODIR)/voronoi_tst.o $(GLIBS)

# Program objects

TDIR = $(PDIR)/tst_prgs

$(ODIR)/sv_tst_1.o:  $(TDIR)/sv_tst_1.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sv_tst_1.o $(TDIR)/sv_tst_1.cxx

$(ODIR)/sv_tst_2.o:  $(TDIR)/sv_tst_2.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sv_tst_2.o $(TDIR)/sv_tst_2.cxx

$(ODIR)/sv_tst_g.o:  $(TDIR)/sv_tst_g.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sv_tst_g.o $(TDIR)/sv_tst_g.cxx

$(ODIR)/engine.o:  $(TDIR)/engine.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/engine.o $(TDIR)/engine.cxx

$(ODIR)/expt.o:  $(TDIR)/expt.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/expt.o $(TDIR)/expt.cxx

$(ODIR)/sv_display.o:  $(TDIR)/sv_display.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sv_display.o $(TDIR)/sv_display.cxx

$(ODIR)/sv_convert.o:	$(TDIR)/sv_convert.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sv_convert.o $(TDIR)/sv_convert.cxx

$(ODIR)/voronoi_tst.o:	$(TDIR)/voronoi_tst.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/voronoi_tst.o $(TDIR)/voronoi_tst.cxx

#
# sv_edit - the interactive svlis model editor
#

EDITS = $(PDIR)/sv_edit/src

svlis:	$(ODIR)/sv_edit.o $(ODIR)/edittool.o $(IDIR)/edittool.h $(IDIR)/sv_edit.h
	$(CC) $(FLAGS) $(ODIR)/sv_edit.o $(ODIR)/edittool.o -o $(RDIR)/svlis \
	$(GLIBS)

$(ODIR)/sv_edit.o:	$(EDITS)/sv_edit.cxx $(INCLUDE) $(IDIR)/edittool.h $(IDIR)/sv_edit.h
	$(CC) -c $(FLAGS) $(EDITS)/sv_edit.cxx -o $(ODIR)/sv_edit.o

$(ODIR)/edittool.o:	$(EDITS)/edittool.cxx $(INCLUDE) $(IDIR)/edittool.h $(IDIR)/sv_edit.h
	$(CC) -c $(FLAGS) $(EDITS)/edittool.cxx -o $(ODIR)/edittool.o

#
#  Refinery model
#

REFD = $(PDIR)/refinery/src

refinery:	$(RDIR)/refinery

$(RDIR)/refinery:	$(ODIR)/refinery.o
	$(CC) $(FLAGS) $(ODIR)/refinery.o -o $(RDIR)/refinery \
	$(GLIBS)

$(ODIR)/refinery.o:	$(REFD)/refinery.cxx $(INCLUDE) 
	$(CC) -c $(FLAGS) $(REFD)/refinery.cxx -o $(ODIR)/refinery.o


# Library object files

$(ODIR)/svlis.o:	$(SDIR)/svlis.cxx  $(INCLUDE) 
		$(CC) -c $(FLAGS) -o $(ODIR)/svlis.o $(SDIR)/svlis.cxx

$(ODIR)/attrib.o:	 $(SDIR)/attrib.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/attrib.o $(SDIR)/attrib.cxx

$(ODIR)/flag.o:		$(SDIR)/flag.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/flag.o $(SDIR)/flag.cxx

$(ODIR)/geometry.o:	 $(SDIR)/geometry.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/geometry.o $(SDIR)/geometry.cxx

$(ODIR)/sv_graph.o:	 $(SDIR)/sv_graph.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) $(SPACEMOUSE) -o $(ODIR)/sv_graph.o $(SDIR)/sv_graph.cxx

$(ODIR)/interval.o:	 $(SDIR)/interval.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/interval.o $(SDIR)/interval.cxx

$(ODIR)/model.o:	 $(SDIR)/model.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/model.o $(SDIR)/model.cxx

$(ODIR)/polygon.o:	 $(SDIR)/polygon.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/polygon.o $(SDIR)/polygon.cxx

$(ODIR)/prim.o:	 $(SDIR)/prim.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/prim.o $(SDIR)/prim.cxx

$(ODIR)/set.o:	 $(SDIR)/set.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/set.o $(SDIR)/set.cxx

$(ODIR)/sums.o:	 $(SDIR)/sums.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/sums.o $(SDIR)/sums.cxx

$(ODIR)/arf.o:  $(SDIR)/arf.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/arf.o $(SDIR)/arf.cxx

$(ODIR)/arpors.o:  $(SDIR)/arpors.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/arpors.o $(SDIR)/arpors.cxx

$(ODIR)/ivallist.o:  $(SDIR)/ivallist.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/ivallist.o $(SDIR)/ivallist.cxx

$(ODIR)/polynml.o:  $(SDIR)/polynml.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/polynml.o $(SDIR)/polynml.cxx

$(ODIR)/raytrace.o:  $(SDIR)/raytrace.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/raytrace.o $(SDIR)/raytrace.cxx

$(ODIR)/picture.o:	$(SDIR)/picture.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/picture.o $(SDIR)/picture.cxx

$(ODIR)/sv_rend.o:	$(SDIR)/sv_rend.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sv_rend.o $(SDIR)/sv_rend.cxx

$(ODIR)/render.o:	$(SDIR)/render.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/render.o $(SDIR)/render.cxx

$(ODIR)/qv.o:	$(SDIR)/qv.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/qv.o $(SDIR)/qv.cxx

$(ODIR)/view.o:	$(SDIR)/view.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/view.o $(SDIR)/view.cxx

$(ODIR)/light.o:	$(SDIR)/light.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/light.o $(SDIR)/light.cxx

$(ODIR)/shade.o:	$(SDIR)/shade.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/shade.o $(SDIR)/shade.cxx

$(ODIR)/environs.o:	$(SDIR)/environs.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/environs.o $(SDIR)/environs.cxx

$(ODIR)/read1.o:	$(SDIR)/read1.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/read1.o $(SDIR)/read1.cxx

$(ODIR)/read.o:	$(SDIR)/read.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/read.o $(SDIR)/read.cxx

$(ODIR)/rotations.o:	$(SDIR)/rotations.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/rotations.o $(SDIR)/rotations.cxx

$(ODIR)/voronoi.o:	$(SDIR)/voronoi.cxx $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/voronoi.o $(SDIR)/voronoi.cxx

$(ODIR)/sve.o:	$(SDIR)/sve.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/sve.o $(SDIR)/sve.cxx

$(ODIR)/u_attrib.o:	$(SDIR)/u_attrib.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/u_attrib.o $(SDIR)/u_attrib.cxx

$(ODIR)/u_prim.o:	$(SDIR)/u_prim.cxx  $(INCLUDE)
		$(CC) -c $(FLAGS) -o $(ODIR)/u_prim.o $(SDIR)/u_prim.cxx

$(ODIR)/sv_util.o:	 $(SDIR)/sv_util.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/sv_util.o $(SDIR)/sv_util.cxx

$(ODIR)/decision.o:	 $(SDIR)/decision.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/decision.o $(SDIR)/decision.cxx

$(ODIR)/surface.o:	 $(SDIR)/surface.cxx  $(INCLUDE)
		 $(CC) -c $(FLAGS) -o $(ODIR)/surface.o $(SDIR)/surface.cxx

$(ODIR)/niederreiter.o:	 $(SDIR)/niederreiter.cxx
		 $(CC) -c $(FLAGS) -o $(ODIR)/niederreiter.o $(SDIR)/niederreiter.cxx

$(ODIR)/xdrvlib.o:	 $(SDIR)/xdrvlib.c $(IDIR)/xdrvlib.h
		 cc -c $(FLAGS) -I$(IDIR) -o $(ODIR)/xdrvlib.o $(SDIR)/xdrvlib.c

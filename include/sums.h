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
 * SvLis - miscelaneous mathematical functions and macros
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: 12 April 1993
 * This version: 8 March 2000
 *
 */


#ifndef SVLIS_SUMS
#define SVLIS_SUMS

#ifdef SV_MSOFT
 #define max maxx
 #define min minn
#endif

// Round a float:

inline sv_integer round(sv_real a) 
{
	return( (a > 0.0) ? (sv_integer)(a + 0.5) : (sv_integer)(a - 0.5) );
}

// Sign function (NB 0 => -1)

inline sv_real sign(sv_real a)
{
	return( (a > 0) ? 1.0 : -1.0 );
}

// Maximum of 2 numbers

inline sv_real max(sv_real a, sv_real b)
{
	return( (a > b) ? a : b );
}
inline sv_integer max(sv_integer a, sv_integer b)
{
	return( (a > b) ? a : b );
}

// Minimum of 2 numbers

inline sv_real min(sv_real a, sv_real b)
{
	return( (a > b) ? b : a );
}
inline sv_integer min(sv_integer a, sv_integer b)
{
	return( (a > b) ? b : a );
}

// Signed square-root

inline sv_real s_sqrt(sv_real x)
{
	sv_real result;
	if (x < 0)
		result = (sv_real)-sqrt(-x);
	else
		result = (sv_real)sqrt(x);
	return(result);
}

// Raise a real to a power

extern sv_real pow(sv_real, sv_integer);

// Convenient to have a canonical form for the mem test of a real:

inline mem_test member(sv_real a)
{
	mem_test result;

	if (a > 0)
		result = SV_AIR;
	else
	{
		if (a < 0) 
			result = SV_SOLID;
		else
			result = SV_SURFACE;
	}

	return(result);
}


// Random number functions

extern void set_seed(sv_integer);
extern sv_integer ran_int();
extern sv_real ran_real();

// Are two reals pretty close?

extern sv_real sv_same_tol;

extern void set_same_tol(sv_real);

// 0 means different
// SV_PLUS means the same
// SV_MINUS means a = -b

inline prim_op same(sv_real a, sv_real b)
{
    if (fabs(a - b) <= sv_same_tol) return(SV_PLUS);
    if (fabs(a + b) <= sv_same_tol) return(SV_COMP);
    return(SV_ZERO);
}

// I/O procedures and structures

// Write spaces white to a stream using tabs

// Switch indented output on (1) or off (0)

extern void pretty_print(sv_integer);

// Write white space to a stream using tabs

extern void put_white(ostream&, sv_integer);

// Write a token to the output

extern void put_token(ostream&, sv_tag, sv_integer, sv_real);

// Separator used in i/o

inline int issep(char c) 
{ 
	return((c == ',') || (c == ':') || (c == ';'));
}

// Get rid of white space and comments from an input stream

extern void junk_junk(istream&);

// Read an input stream up to the end of line; allow for
// CR, LF and both as terminations.

sv_integer r_to_eoln(istream&, char*, sv_integer);

// Is a character a line terminator?

inline int newline(char c) { return( (c == '\n') || (c == '\r')); }

// Get a tag and (optionally) a real or an integer
// If an integer is expected, the last argument should be TRUE

extern sv_tag get_token(istream&, sv_integer&, sv_real&, int);

// Return the last token read

extern sv_tag last_token();

// Check that an expected token is the next one

extern int check_token(istream&, sv_tag);

// Create and check the svLis file header

extern void write_svlis_header(ostream&);
extern void check_svlis_header(istream&);

// Set and check the version number of the file being read

extern void set_read_version(sv_integer);
extern sv_integer get_read_version();
extern sv_integer get_svlis_version();

// Useful to have real and integer write in the canonical form

inline void writer(ostream& s, sv_real r, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_REAL, 0, r);
}

inline void writei(ostream& s, sv_integer i, sv_integer level)
{
	put_white(s, level);
	put_token(s, SVT_INTEGER, i, 0);
}

// Clear all the input tables

extern void clean_attribute_lookup();
extern void clean_model_lookup();
extern void clean_sl_lookup();
extern void clean_primitive_lookup();
extern void clean_set_lookup();

inline void sv_clear_input_tables()
{
	clean_attribute_lookup();
	clean_model_lookup();
	clean_sl_lookup();
	clean_primitive_lookup();
	clean_set_lookup();
}

// Unfortunately POSIX-compatibility means we need our own strcmp...

extern int sv_strcmp(const char *, const char *);
extern size_t sv_strlen(const char *);
extern char *sv_strcpy(char *, const char *);
extern char *sv_strcat(char *, const char *);

#endif


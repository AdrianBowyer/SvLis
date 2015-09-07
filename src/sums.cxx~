
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
 *  SVLIS miscelaneous mathematical functions
 *  stuff for i/o and other odds and sods
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


#include "sv_std.h"
#include "enum_def.h"
#include "sums.h"
#include "flag.h"
#if macintosh
 #pragma export on
#endif

static sv_integer r_seed;  // Random number seed

// Useful to have reals, integers, and text in the tag scheme

sv_integer real_tag() { return(SVT_F*SVT_REAL); }
sv_integer integer_tag() { return(SVT_F*SVT_INTEGER); }
sv_integer text_tag() { return(SVT_F*SVT_TEXT); }

// This procedure sets the seed

void set_seed(sv_integer i)
{
        r_seed = i;
}

// This procedure returns a random non-negative integer in [0,BIG_INT] 

sv_integer ran_int()
{
        r_seed = R1 + r_seed*R2;
        return(r_seed & BIG_INT);
}


// This returns a floating point number in [0,1] 

sv_real ran_real()
{
        return((sv_real)ran_int()/(sv_real)BIG_INT);
}



// a^b

sv_real pow(sv_real a, sv_integer j)
{
	sv_integer i = abs(j);

	sv_real c = 1.0;

	while (i)
	{
		c = a*c;
		i--;
	}
	if(j < 0) c = 1/c;
	return(c);
}


// Tolerance for `same' fudge comparisons

sv_real sv_same_tol = QUITE_SMALL;

void set_same_tol(sv_real a)
{
	sv_same_tol = a;
}


// Utilities for formating output and reading input

// Version number tracking

static sv_integer rv = SV_VER;

void set_read_version(sv_integer v) { rv = v;}
sv_integer get_read_version() { return(rv);}
sv_integer get_svlis_version() { return(SV_VER);}

// Tokens for tags

static char* tag_names[SVT_TOP_T + 1] = 
{
	"zero",
	"point",
	"line",
	"plane",
	"interval",
	"box",
	"primitive",
	"set",
	"set_list",
	"user_attribute",
	"attribute",
	"polygon",
	"model",
	"text",
	"real",
	"integer",
	"surface",
	"{",
	"[",
	"(",
	"<",
	"\\",
	"}",
	"]",
	")",
	">",
	"/",
	"SvLis",
	"delaunay",
	"v_vertex",
	"voronoi",
	"view",
	"light",
	"l_list",
	"picture",
	"pixel",
	"end"
};

char* sv_tag_lu(sv_tag t) {return(tag_names[t]);}

sv_tag sv_name_lu(char* str)
{
	int t = 0;
	do
	{
		if (!sv_strcmp(str, tag_names[t])) 
			return((sv_tag)t);
		t++;
	} while(t < SVT_TOP_T);
	char msg[80];
	ostrstream ost(msg, 80);
	ost << "dud tag name: " << str << '\0';
	svlis_error("sv_name_lu", msg, SV_WARNING);
	return(SVT_TOP_T); 
}

// Pretty_print flag

static sv_integer pprt = 0;

void pretty_print(sv_integer i) { pprt = i; }

// Write spaces white to a stream using tabs

void put_white(ostream& s, sv_integer spaces)
{
	if(!pprt) return;

	sv_integer ct = spaces/SV_TAB_LEN;
	while(ct) { s.put('\t'); ct--; }
	ct = spaces%SV_TAB_LEN;
	while(ct) { s.put(' '); ct--; }
}

// Write a token to the output

void put_token(ostream& s, sv_tag t, sv_integer j, sv_real r)
{
	switch(t)
	{
	case SVT_REAL: s << r; break;
	case SVT_INTEGER: s << j; break;
	default:	
		s << sv_tag_lu(t);
	}
}

// Get rid of white space and comments from an input stream

void junk_junk(istream& s)
{
	char c = ' ';
	while(isspace(c) || issep(c))
	{
		s.get(c);
		if (c == SV_COM_CHAR)
		{
			do
			{
				s.get(c);
			} while(!newline(c));
		} 
	}
	s.putback(c);	
}

// Read an input stream up to the end of line; allow for
// CR, LF and both as terminations.

sv_integer r_to_eoln(istream& s, char* dest, sv_integer max)
{
	char c;
	sv_integer count = -1;

	do
	{
		count++;
		s.get(c);
		dest[count] = c;
	} while( !newline(c) && (count < max) && !s.eof() );

	dest[count] = 0;

	if(!s.eof())
	{
		s.get(c);
		if(!newline(c)) s.putback(c);
	}

	return(count);	
}

// Remember the last token

static sv_tag lt = SVT_TOP_T;

sv_tag last_token() { return(lt); }

// Get a tag and (optionally) a real or an integer
// If an integer is expected, expect_i should be TRUE

sv_tag get_token(istream& s, sv_integer& j, sv_real& r, int expect_i) 
{
	char str[SV_TAG_S_LEN];
	sv_integer i = 0;
	char c;

	junk_junk(s);

	s.get(c);

	if(!isdigit(c) && (c != '.') && (c != '-') && (c != '+'))
	{
		if(isalpha(c) || (c == '_'))
		{
			while(isalpha(c) || (c == '_'))
			{
				str[i++] = c;
				s.get(c);
			}
			s.putback(c);
			str[i] = 0;
		} else
		{
			str[i++] = c;
			str[i] = 0;
		}
		lt = sv_name_lu(str);
		return(lt);
	}
	
	s.putback(c);

	if(expect_i)
	{
		s >> j;
		lt = SVT_INTEGER;
		return(lt);
	}

	s >> r;
	lt = SVT_REAL;
	return(lt);
}

int check_token(istream& s, sv_tag t)
{
	char msg[80];
	ostrstream ost(msg, 80);
	
	sv_integer di = 0;
	sv_real dr = 0;

	sv_tag tt = get_token(s, di, dr, 0);
	if (tt != t)
	{
		ost << "dud token: " << sv_tag_lu(tt) << " (expecting: " <<
			sv_tag_lu(t) << ")\0";
		svlis_error("check_token", msg, SV_WARNING);
		return(0);
	} else
		return(1);
}

void write_svlis_header(ostream& s)
{
	put_token(s, SVT_SVLIS, 0, 0);
	put_token(s, SVT_OB_S, 0, 0);
	put_token(s, SVT_INTEGER, get_svlis_version(), 0);
	put_token(s, SVT_CB_S, 0, 0);
        time_t l_time;
        time(&l_time);
	s << SV_EL << SV_EL << "# SvLis created this file on " << ctime(&l_time);
	s << "# See  http://www.bath.ac.uk/~ensab/G_mod/Svlis/" << SV_EL << SV_EL;
}


void check_svlis_header(istream& s)
{
	sv_integer ver = -1;
	sv_real r;
	char msg[80];
	ostrstream ost(msg,80);

	if(!check_token(s, SVT_SVLIS))
	{
		svlis_error("check_svlis_header",
			"no header, assuming file is version 1",
			SV_WARNING);
		set_read_version(1);
		s.seekg(ios::beg);
		return;
	}
	check_token(s, SVT_OB_S);
	get_token(s, ver, r, 1);
	if(ver != SV_VER)
	{
	  ost << "different file version: " << ver << " (expecting: " << SV_VER << ")" << '\0'; 
	  svlis_error("check_svlis_header", msg, SV_WARNING);
	}
	set_read_version(ver);
	check_token(s, SVT_CB_S);
	junk_junk(s);
}

// Unfortunately IRIX POSIX-compatibility means we need our own strcmp etc...

int sv_strcmp(const char *s1, const char *s2)
{
	while(*s1 && *s2)
	{
		if(*s1 < *s2) return(-1);
		if(*s1 > *s2) return(1);
		s1++;
		s2++;
	}
	if(*s1) return(1);
	if(*s2) return(-1);
	return(0);
}

size_t sv_strlen(const char *s)
{
	size_t result = 0;

	while(*s)
	{
		s++;
		result++;
	}
	return(result);
}

char *sv_strcpy(char *dest, const char *src)
{
	char *cto;
	cto = dest;
	while(*src)
	{
		*cto = *src;
		cto++;
		src++;
	}
	*cto = '\0';
	return(dest);
}

char *sv_strcat(char *dest, const char *src)
{
	char* cto;
	cto = dest;
	while(*cto)cto++;
	while(*src)
	{
		*cto = *src;
		cto++;
		src++;
	}
	*cto = '\0';
	return(dest);
}
#if macintosh
 #pragma export off
#endif


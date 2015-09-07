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
 * SvLis low-discrepancy random numbers
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * or see the file
 *
 *    docs/svlis.html
 *
 * First version: from ACM library
 * This version: 8 March 2000
 *
 */

/* niederreiter.f -- translated by f2c (version 19970805).

   Tidied up by Adrian Bowyer
*/

#include "svlis.h"

// Common Block Declarations

struct 
{
    long int cj[372], dimen, count, nextq[12];
} comm2_;

#define comm2_1 comm2_

struct {
    long int p, q, add[2500], mul[2500], sub[2500];
} field_;

#define field_1 field_

// Table of constant values

static long int c__9 = 9;
static long int c__1 = 1;
static long int c__2 = 2;
static long int c__36 = 36;
static long int c__3 = 3;


// Initialize the sequence.  dim is the number of
// dimensions, skip is the number of RNs to throw
// away at the start

int inlo2_(long int *dim, long int *skip)
{
// System generated local

    long int i__1;

    int s_stop(char *, long int);

// Local variables

    static long int gray, i__, r__;
    extern int calcc2_();


/* ************************************************************

  This version :  12 February 1992

   See the general comments on implementing Niederreiter's 
   low-discrepancy sequences. 

   This subroutine calculates the values of Niederreiter's 
   C(I,J,R) and performs other initialisation necessary 
   before calling GOLO2. 

 INPUT : 
   DIMEN - The dimension of the sequence to be generated. 
        {DIMEN is called DIM in the argument of INLO2, 
        because DIMEN is subsequently passed via COMMON 
        and is called DIMEN there.} 

   SKIP  - The number of values to throw away at the beginning 
           of the sequence. 

 OUTPUT : 
   To GOLO2, labelled common /COMM2/. 

 USES : 
   Calls CALCC2 to calculate the values of CJ. 
   ***** A non-standard function is used to compute *****
   ***** bitwise exclusive-ors.                     *****


  ------------------------------------------------------------


   This file defines the common block /COMM2/ and some 
   associated parameters.  These are for use in the base 2 
   version of the generator. 


   The parameter MAXDIM is the maximum dimension that will be used. 
   NBITS is the number of bits (not counting the sign) in a 
   fixed-point integer. 


   The common block /COMM2/ : 
     CJ    - Contains the packed values of Niederreiter's C(I,J,R) 
     DIMEN   - The dimension of the sequence to be generated 
     COUNT - The index of the current item in the sequence, 
             expressed as an array of bits.  COUNT(R) is the same 
             as Niederreiter's AR(N) (page 54) except that N is 
             implicit. 
     NEXTQ - The numerators of the next item in the series.  These 
             are like Niederreiter's XI(N) (page 54) except that 
             N is implicit, and the NEXTQ are integers.  To obtain 
             the values of XI(N), multiply by RECIP (see GOLO2). 

   Array CJ of the common block is set up by subroutine CALCC2. 
   The other items in the common block are set up by INLO2. 

************************************************************* */




    comm2_1.dimen = *dim;

//       This assignment just relabels the variable for
//       subsequent use.

    if (comm2_1.dimen <= 0 || comm2_1.dimen > 12) 
    {
	svlis_error("INLO2","bad dimension", SV_WARNING);
	return 0;
    }

    calcc2_();

//   Translate SKIP into Gray code 

    gray = *skip ^ (*skip / 2);

//   Now set up NEXTQ appropriately for this value of the Gray code 

    i__1 = comm2_1.dimen;
    for (i__ = 1; i__ <= i__1; ++i__) 
    {
	comm2_1.nextq[i__ - 1] = 0;
    }

    r__ = 0;
L10:
    if (gray != 0) 
    {
	if (gray % 2 != 0) 
        {
	    i__1 = comm2_1.dimen;
	    for (i__ = 1; i__ <= i__1; ++i__) 
            {
// This is the non-standard exclusive-or again
// Vax version :

		comm2_1.nextq[i__ - 1] ^= comm2_1.cj[i__ + r__ * 12 - 1];

// ***** Unix version :
// NEXTQ(I) = XOR(NEXTQ(I), CJ(I,R))
// L20:
	    }
	}
	gray /= 2;
	++r__;
	goto L10;
    }

    comm2_1.count = *skip;
    return 0;
} /* inlo2_ */


// This generates a random point in a unit [0,1] box in the *dim dimensional space
// in the array quasi.

int golo2_(sv_real *quasi)
{

// System generated locals

    long int i__1;

// Local variables

    static long int i__, r__;


/* *******************************************************************

  This version :  21 February 1992

        This routine is for base 2 only.  The driver, GENIN2, 
        calls it after proper set-up. 

 See the general comments on implementing Niederreiter's 
 low-discrepancy sequences. 

 This subroutine generates a new quasi-random vector 
 on each call. 

 INPUT 
   From INLO2, labelled common /COMM2/, properly initialized. 

 OUTPUT 
   To the caller, the next vector in the sequence in the 
   array QUASI. 

 USES 
   ***** A non-standard function is used to compute ***** 
   ***** bitwise exclusive-ors.                     ***** 


   ------------------------------------------------------------ 


   This file defines the common block /COMM2/ and some 
   associated parameters.  These are for use in the base 2 
   version of the generator. 


   The parameter MAXDIM is the maximum dimension that will be used. 
   NBITS is the number of bits (not counting the sign) in a 
   fixed-point integer. 


   The common block /COMM2/ : 
     CJ    - Contains the packed values of Niederreiter's C(I,J,R) 
     DIMEN   - The dimension of the sequence to be generated 
     COUNT - The index of the current item in the sequence, 
             expressed as an array of bits.  COUNT(R) is the same 
             as Niederreiter's AR(N) (page 54) except that N is 
             implicit. 
     NEXTQ - The numerators of the next item in the series.  These 
             are like Niederreiter's XI(N) (page 54) except that 
             N is implicit, and the NEXTQ are integers.  To obtain 
             the values of XI(N), multiply by RECIP (see GOLO2). 

   Array CJ of the common block is set up by subroutine CALCC2. 
   The other items in the common block are set up by INLO2. 

******************************************************************** */


//   The parameter RECIP is the multiplier which changes the 
//   integers in NEXTQ into the required real values in QUASI. 


// Multiply the numerators in NEXTQ by RECIP to get the next
// quasi-random vector

// Parameter adjustments

    --quasi;

// Function Body

    i__1 = comm2_1.dimen;
    for (i__ = 1; i__ <= i__1; ++i__) 
    {
	quasi[i__] = comm2_1.nextq[i__ - 1] * (float)4.6566128730773926e-10;
// L5:
    }

// Find the position of the right-hand zero in COUNT.  This
// is the bit that changes in the Gray-code representation as
// we go from COUNT to COUNT+1.

    r__ = 0;
    i__ = comm2_1.count;

L10:
    if (i__ % 2 != 0) 
    {
	++r__;
	i__ /= 2;
	goto L10;
    }

// Check that we have not passed 2**NBITS calls on GOLO2

    if (r__ >= 31) 
    {
	svlis_error("GOLO2","Too many calls", SV_WARNING);
	return 0;
    }

// Compute the new numerators in vector NEXTQ

    i__1 = comm2_1.dimen;
    for (i__ = 1; i__ <= i__1; ++i__) 
    {
//       ***** Bitwise exclusive-or is not standard in Fortran

	comm2_1.nextq[i__ - 1] ^= comm2_1.cj[i__ + r__ * 12 - 1];
//       ***** This is the Unix version
//      NEXTQ(I) = XOR(NEXTQ(I), CJ(I,R))
// L20:
    }

    ++comm2_1.count;
    return 0;
} /* golo2_ */


// Compute constants

int calcc2_()
{
    
// Initialized data

    static struct {
	long int e_1[36];
	long int fill_2[2];
	long int e_3[10];
	long int fill_4[3];
	long int e_5[9];
	long int fill_6[5];
	long int e_7[7];
	long int fill_8[8];
	long int e_9[4];
	} equiv_25 = { 1, 1, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 0, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, {0}, 1, 
		0, 1, 0, 0, 1, 1, 0, 1, 1, {0}, 1, 1, 0, 1, 1, 0, 1, 1, 0, {0}
		, 1, 1, 1, 0, 0, 0, 1, {0}, 1, 1, 1, 1 };

#define irred ((long int *)&equiv_25)


// System generated locals

    long int i__1, i__2;

// Local variables

    static long int term, b[52], e, i__, j, r__, u, v[37];
    extern int calcv_(long int *, long int *, long int *, long int *);
    static long int ci[961], px[52];
    extern int setfld_(long int *);


/* *******************************************************************

  This version :  12 February 1992

      *****  For base-2 only.

   See the general comments on implementing Niederreiter's 
   low-discrepancy sequences. 

   This program calculates the values of the constants C(I,J,R). 
   As far as possible, we use Niederreiter's notation. 
   For each value of I, we first calculate all the corresponding 
   values of C :  these are held in the array CI.  All these 
   values are either 0 or 1.  Next we pack the values into the 
   array CJ, in such a way that CJ(I,R) holds the values of C 
   for the indicated values of I and R and for every value of 
   J from 1 to NBITS.  The most significant bit of CJ(I,R) 
   (not counting the sign bit) is C(I,1,R) and the least 
   significant bit is C(I,NBITS,R). 
     When all the values of CJ have been calculated, we return 
   this array to the calling program. 

  -------------------------------------------------------------- 

   We define the common block /COMM2/ and some 
   associated parameters.  These are for use in the base 2 
   version of the generator. 


   The parameter MAXDIM is the maximum dimension that will be used. 
   NBITS is the number of bits (not counting the sign) in a 
   fixed-point integer. 


   The common block /COMM2/ : 
     CJ    - Contains the packed values of Niederreiter's C(I,J,R) 
     DIMEN   - The dimension of the sequence to be generated 
     COUNT - The index of the current item in the sequence, 
             expressed as an array of bits.  COUNT(R) is the same 
             as Niederreiter's AR(N) (page 54) except that N is 
             implicit. 
     NEXTQ - The numerators of the next item in the series.  These 
             are like Niederreiter's XI(N) (page 54) except that 
             N is implicit, and the NEXTQ are integers.  To obtain 
             the values of XI(N), multiply by RECIP (see GOLO2). 

   Array CJ of the common block is set up by subroutine CALCC2. 
   The other items in the common block are set up by INLO2. 

   -------------------------------------------------------------- 

   The following COMMON block, used by many subroutines, 
   gives the order Q of a field, its characteristic P, and its 
   addition, multiplication and subtraction tables. 
   The parameter MAXQ gives the order of the largest field to 
   be handled. 


   The following definitions concern the representation of 
   polynomials. 


   The parameter MAXDEG gives the highest degree of polynomial 
   to be handled.  Polynomials stored as arrays have the 
   coefficient of degree n in POLY(N), and the degree of the 
   polynomial in POLY(-1).  The parameter DEG is just to remind 
   us of this last fact.  A polynomial which is identically 0 
   is given degree -1. 

   A polynomial can also be stored in an integer I, with 
        I = AN*Q**N + ... + A0. 
   Routines ITOP and PTOI convert between these two formats. 

   --------------------------------------------------------------- 



   MAXE   - We need MAXDIM irreducible polynomials over Z2. 
            MAXE is the highest degree among these. 
   MAXV   - The maximum possible index used in V. 


 INPUT : 
   The array CJ to be initialised, and DIMEN the number of 
   dimensions we are using, are transmitted through /COMM2/. 

 OUTPUT : 
   The array CJ is returned to the calling program. 

 USES : 
   Subroutine SETFLD is used to set up field arithmetic tables. 
   (Although this is a little heavy-handed for the field of 
   order 2, it makes for uniformity with the general program.) 
   Subroutine CALCV is used to for the auxiliary calculation 
   of the values V needed to get the Cs. 



   This DATA statement supplies the coefficients and the 
   degrees of the first 12 irreducible polynomials over Z2. 
   They are taken from Lidl and Niederreiter, FINITE FIELDS, 
   Cambridge University Press (1984), page 553. 
   The order of these polynomials is the same as the order in 
   file 'irrtabs.dat' used by the general program. 

   In this block PX(I, -1) is the degree of the Ith polynomial, 
   and PX(I, N) is the coefficient of x**n in the Ith polynomial. 

********************************************************************** */

//   Prepare to work in Z2 

    setfld_(&c__2);

    i__1 = comm2_1.dimen;
    for (i__ = 1; i__ <= i__1; ++i__) 
    {

//   For each dimension, we need to calculate powers of an 
//   appropriate irreducible polynomial :  see Niederreiter 
//   page 65, just below equation (19). 
//     Copy the appropriate irreducible polynomial into PX, 
//   and its degree into E.  Set polynomial B = PX ** 0 = 1. 
//   M is the degree of B.  Subsequently B will hold higher 
//   powers of PX. 

	e = irred[i__ - 1];
	i__2 = e;
	for (j = -1; j <= i__2; ++j) 
        {
	    px[j + 1] = irred[i__ + j * 12 + 11];
// L10:
	}
	b[0] = 0;
	b[1] = 1;

//   Niederreiter (page 56, after equation (7), defines two
//   variables Q and U.  We do not need Q explicitly, but we 
//   do need U.

	u = 0;

	for (j = 1; j <= 31; ++j) 
        {

//   If U = 0, we need to set B to the next power of PX
//   and recalculate V.  This is done by subroutine CALCV.

	    if (u == 0) 
            {
		calcv_(px, b, v, &c__36);
	    }

// Now C is obtained from V.  Niederreiter
// obtains A from V (page 65, near the bottom), and then gets
// C from A (page 56, equation (7)).  However this can be done
// in one step.  Here CI(J,R) corresponds to
// Niederreiter's C(I,J,R).

	    for (r__ = 0; r__ <= 30; ++r__) 
            {
		ci[j + r__ * 31 - 1] = v[r__ + u];
// L50:
	    }

// Increment U.  If U = E, then U = 0 and in Niederreiter's
// paper Q = Q + 1.  Here, however, Q is not used explicitly.

	    ++u;
	    if (u == e) 
            {
		u = 0;
	    }
// L90:
	}

//  The array CI now holds the values of C(I,J,R) for this value
//  of I.  We pack them into array CJ so that CJ(I,R) holds all
//  the values of C(I,J,R) for J from 1 to NBITS.

	for (r__ = 0; r__ <= 30; ++r__) 
        {
	    term = 0;
	    for (j = 1; j <= 31; ++j) 
            {
		term = (term << 1) + ci[j + r__ * 31 - 1];
// L110:
	    }
	    comm2_1.cj[i__ + r__ * 12 - 1] = term;
// L120:
	}

// L1000:
    }
    return 0;
} /* calcc2_ */

#undef irred



int calcv_(long int *px, long int *b, long int *v, long int * maxv)
{
    
// System generated locals
 
    long int i__1, i__2;

// Local variables

    static long int bigm, term, e, h__[52], i__, j, m, r__, kj, nonzer;
    extern int plymul_(long int *, long int *, long int *);


/*   *************************************************************************

   This version :  12 February 1991 

   See the general comments on implementing Niederreiter's 
   low-discrepancy sequences. 

   This program calculates the values of the constants V(J,R) as 
   described in BFN section 3.3.  It is called from either CALCC or 
   CALCC2.  The values transmitted through common /FIELD/ determine 
   which field we are working in. 

 INPUT : 
   PX is the appropriate irreducible polynomial for the dimension 
     currently being considered.  The degree of PX will be called E. 
   B is the polynomial defined in section 2.3 of BFN.  On entry to 
     the subroutine, the degree of B implicitly defines the parameter 
     J of section 3.3, by degree(B) = E*(J-1). 
   MAXV gives the dimension of the array V. 
   On entry, we suppose that the common block /FIELD/ has been set 
     up correctly (using SETFLD). 

 OUTPUT : 
   On output B has been multiplied by PX, so its degree is now E*J. 
   V contains the values required. 

 USES : 
   The subroutine PLYMUL is used to multiply polynomials. 


   ------------------------------------------------------------ 

   The following COMMON block, used by many subroutines, 
   gives the order Q of a field, its characteristic P, and its 
   addition, multiplication, and subtraction tables. 
   The parameter MAXQ gives the order of the largest field to 
   be handled. 


   The following definitions concern the representation of 
   polynomials. 


   The parameter MAXDEG gives the highest degree of polynomial 
   to be handled.  Polynomials stored as arrays have the 
   coefficient of degree n in POLY(N), and the degree of the 
   polynomial in POLY(-1).  The parameter DEG is just to remind 
   us of this last fact.  A polynomial which is identically 0 
   is given degree -1. 

   A polynomial can also be stored in an integer I, with 
        I = AN*Q**N + ... + A0. 
   Routines ITOP and PTOI convert between these two formats. 

   ----------------------------------------------------------- 


   We use ARBIT() to indicate where the user can place 
   an arbitrary element of the field of order Q, while NONZER 
   shows where he must put an arbitrary non-zero element 
   of the same field.  For the code, 
   this means 0 <= ARBIT < Q and 0 < NONZER < Q.  Within these 
   limits, the user can do what he likes.  ARBIT is declared as 
   a function as a reminder that a different arbitrary value may 
   be returned each time ARBIT is referenced. 

    BIGM is the M used in section 3.3. 
    It differs from the [little] m used in section 2.3, 
    denoted here by M. 

******************************************************************* */

// Parameter adjustments

    ++px;
    ++b;

// Function Body

    nonzer = 1;

    e = px[-1];

//   The poly H is PX**(J-1), which is the value of B on arrival. 
//   In section 3.3, the values of Hi are defined with a minus sign : 
//   don't forget this if you use them later !

    i__1 = b[-1];
    for (i__ = -1; i__ <= i__1; ++i__) 
    {
// L10:
	h__[i__ + 1] = b[i__];
    }
    bigm = h__[0];

//   Now multiply B by PX so B becomes PX**J.
//   In section 2.3, the values of Bi are defined with a minus sign :
//   don't forget this if you use them later !

    plymul_(&px[-1], &b[-1], &b[-1]);
    m = b[-1];

//   We don't use J explicitly anywhere, but here it is just in case.

    j = m/e;

//   Now choose a value of Kj as defined in section 3.3.
//   We must have 0 <= Kj < E*J = M.
//   The limit condition on Kj does not seem very relevant 
//   in this program.

    kj = bigm;

//   Now choose values of V in accordance with the conditions in 
//   section 3.3 

    i__1 = kj - 1;
    for (r__ = 0; r__ <= i__1; ++r__) 
    {
// L20:
	v[r__] = 0;
    }
    v[kj] = 1;

    if (kj < bigm) 
    {

	term = field_1.sub[h__[kj + 1] * 50];

	i__1 = bigm - 1;
	for (r__ = kj + 1; r__ <= i__1; ++r__) 
        {
	    v[r__] = 1;

//   Check the condition of section 3.3,
//   remembering that the H's have the opposite sign.

	    term = field_1.sub[term + field_1.mul[h__[r__ + 1] + v[r__] * 50] 
		    * 50];
// L30:
	}

// Now V(BIGM) is anything but TERM

	v[bigm] = field_1.add[nonzer + term * 50];

	i__1 = m - 1;
	for (r__ = bigm + 1; r__ <= i__1; ++r__) 
        {
// L40:
	    v[r__] = 1;
	}

    } else 
    {
//  This is the case KJ .GE. BIGM

	i__1 = m - 1;
	for (r__ = kj + 1; r__ <= i__1; ++r__) 
        {
// L50:
	    v[r__] = 1;
	}

    }

//   Calculate the remaining V's using the recursion of section 2.3,
//   remembering that the B's have the opposite sign.

    i__1 = *maxv - m;
    for (r__ = 0; r__ <= i__1; ++r__) 
    {
	term = 0;
	i__2 = m - 1;
	for (i__ = 0; i__ <= i__2; ++i__) 
        {
	    term = field_1.sub[term + field_1.mul[b[i__] + v[r__ + i__] * 50] * 50];
// L60:
	}
	v[r__ + m] = term;
// L70:
    }

    return 0;
} /* calcv_ */


long int charac_(long int *qin)
{
    
// Initialized data 

    static long int ch[50] = { 0,2,3,2,5,0,7,2,3,0,11,0,13,0,0,2,17,0,19,0,0,0,
	    23,0,5,0,3,0,29,0,31,2,0,0,0,0,37,0,0,0,41,0,43,0,0,0,47,0,7,0 };

// System generated locals

    long int ret_val;


/*   ******************************************************************

    This version :  12 December 1991 

   This function gives the characteristic for a field of 
   order QIN.  If no such field exists, or if QIN is out of
   the range we can handle, returns 0. 


   ------------------------------------------------------------ 

   The following COMMON block, used by many subroutines, 
   gives the order Q of a field, its characteristic P, and its 
   addition, multiplication and subtraction tables. 
   The parameter MAXQ gives the order of the largest field to 
   be handled. 


   The following definitions concern the representation of 
   polynomials. 


   The parameter MAXDEG gives the highest degree of polynomial 
   to be handled.  Polynomials stored as arrays have the 
   coefficient of degree n in POLY(N), and the degree of the 
   polynomial in POLY(-1).  The parameter DEG is just to remind 
   us of this last fact.  A polynomial which is identically 0 
   is given degree -1. 

   A polynomial can also be stored in an long int I, with 
        I = AN*Q**N + ... + A0. 
   Routines ITOP and PTOI convert between these two formats. 

******************************************************************** */


    if (*qin <= 1 || *qin > 50) 
    {
	ret_val = 0;
    } else 
    {
	ret_val = ch[*qin - 1];
    }

    return ret_val;
} /* charac_ */


int setfld_(long int *qin)
{
    
// Format strings

    static char fmt_900[] = "(20i3)";

// System generated locals

    long int i__1, i__2;

// Local variables

    static long int i__, j, n;
    extern long int charac_(long int *);



/*   ********************************************************************

    This version : 12 December 1991 

   This subroutine sets up addition, multiplication, and 
   subtraction tables for the finite field of order QIN. 
   If necessary, it reads precalculated tables from the file 
   'gftabs.dat' using unit 1.  These precalculated tables 
   are supposed to have been put there by GFARIT. 

      *****  For the base-2 programs, these precalculated
      *****  tables are not needed and, therefore, neither
      *****  is GFARIT.


   Unit 1 is closed both before and after the call of SETFLD. 

 USES 
   Integer function CHARAC gets the characteristic of a field. 


   ------------------------------------------------------------ 

   The following COMMON block, used by many subroutines, 
   gives the order Q of a field, its characteristic P, and its 
   addition, multiplication and subtraction tables. 
   The parameter MAXQ gives the order of the largest field to 
   be handled. 


   The following definitions concern the representation of 
   polynomials. 


   The parameter MAXDEG gives the highest degree of polynomial 
   to be handled.  Polynomials stored as arrays have the 
   coefficient of degree n in POLY(N), and the degree of the 
   polynomial in POLY(-1).  The parameter DEG is just to remind 
   us of this last fact.  A polynomial which is identically 0 
   is given degree -1. 

   A polynomial can also be stored in an integer I, with 
        I = AN*Q**N + ... + A0. 
   Routines ITOP and PTOI convert between these two formats. 

***************************************************************** */


    if (*qin <= 1 || *qin > 50) 
    {
	svlis_error("SETFLD","bad value of Q", SV_WARNING);
	return 0;
    }

    field_1.q = *qin;
    field_1.p = charac_(&field_1.q);

    if (field_1.p == 0) 
    {
	svlis_error("SETFLD","there is no field of the given order", SV_WARNING);
	return 0;
    }

// Set up to handle a field of prime order :  calculate ADD and MUL.

    if (field_1.p == field_1.q) 
    {
	i__1 = field_1.q - 1;
	for (i__ = 0; i__ <= i__1; ++i__) 
        {
	    i__2 = field_1.q - 1;
	    for (j = 0; j <= i__2; ++j) 
            {
		field_1.add[i__ + j * 50] = (i__ + j) % field_1.p;
		field_1.mul[i__ + j * 50] = i__ * j % field_1.p;
// L10:
	    }
	}

// Set up to handle a field of prime-power order :  tables for
// ADD and MUL are in the file 'gftabs.dat'.

    } else 
    {
	svlis_error("setfld_","field_1.p != field_1.q",SV_WARNING);
/*	o__1.oerr = 0;
	o__1.ounit = 1;
	o__1.ofnmlen = 10;
	o__1.ofnm = "gftabs.dat";
	o__1.orl = 0;
	o__1.osta = "old";
	o__1.oacc = 0;
	o__1.ofm = 0;
	o__1.oblnk = 0;
	f_open(&o__1);

//    *****  OPEN statements are system dependent.

L20:
	i__2 = s_rsfe(&io___41);
	if (i__2 != 0) 
        {
	    goto L500;
	}
	i__2 = do_fio(&c__1, (char *)&n, (long int)sizeof(long int));
	if (i__2 != 0) 
        {
	    goto L500;
	}
	i__2 = e_rsfe();
	if (i__2 != 0) 
        {
	    goto L500;
	}
	i__2 = n - 1;
	for (i__ = 0; i__ <= i__2; ++i__) 
        {
	    s_rsfe(&io___43);
	    i__1 = n - 1;
	    for (j = 0; j <= i__1; ++j) 
            {
		do_fio(&c__1, (char *)&field_1.add[i__ + j * 50], (long int)
			sizeof(long int));
	    }
	    e_rsfe();
// L30:
	}
	i__2 = n - 1;
	for (i__ = 0; i__ <= i__2; ++i__) 
        {
	    s_rsfe(&io___44);
	    i__1 = n - 1;
	    for (j = 0; j <= i__1; ++j) 
            {
		do_fio(&c__1, (char *)&field_1.mul[i__ + j * 50], (long int)sizeof(long int));
	    }
	    e_rsfe();
// L40:
	}
	if (n != field_1.q) 
        {
	    goto L20;
	}
	cl__1.cerr = 0;
	cl__1.cunit = 1;
	cl__1.csta = 0;
	f_clos(&cl__1);
*/
    }

// Now use the addition table to set the subtraction table.

    i__2 = field_1.q - 1;
    for (i__ = 0; i__ <= i__2; ++i__) 
    {
	i__1 = field_1.q - 1;
	for (j = 0; j <= i__1; ++j) 
        {
	    field_1.sub[field_1.add[i__ + j * 50] + i__ * 50] = j;
// L50:
	}
// L60:
    }
    return 0;
/*
L500:
    svlis_error("SETFLD","tables for q value not found", SV_WARNING);

    return 0;
*/
} /* setfld_ */


int plymul_(long int *pa, long int *pb, long int *pc)
{
    
// System generated locals

    long int i__1, i__2, i__3, i__4;

// Local variables

    static long int dega, degb, degc, term, i__, j, pt[52];


/*   ********************************************************************

    This version :  12 December 1991


   ------------------------------------------------------------ 

   The following COMMON block, used by many subroutines, 
   gives the order Q of a field, its characteristic P, and its 
   addition, multiplication and subtraction tables. 
   The parameter MAXQ gives the order of the largest field to 
   be handled. 


   The following definitions concern the representation of 
   polynomials. 


   The parameter MAXDEG gives the highest degree of polynomial 
   to be handled.  Polynomials stored as arrays have the 
   coefficient of degree n in POLY(N), and the degree of the 
   polynomial in POLY(-1).  The parameter DEG is just to remind 
   us of this last fact.  A polynomial which is identically 0 
   is given degree -1. 

   A polynomial can also be stored in an integer I, with 
        I = AN*Q**N + ... + A0. 
   Routines ITOP and PTOI convert between these two formats. 

********************************************************************* */

//   Multiplies polynomial PA by PB putting the result in PC. 
//   Coefficients are elements of the field of order Q. 

// Parameter adjustments

    ++pc;
    ++pb;
    ++pa;

// Function Body

    dega = pa[-1];
    degb = pb[-1];
    if (dega == -1 || degb == -1) 
    {
	degc = -1;
    } else 
    {
	degc = dega + degb;
    }
    if (degc > 50) 
    {
	svlis_error("PLYMUL","degree of product exceeds MAXDEG", SV_WARNING);
	return 0;
    }

    i__1 = degc;
    for (i__ = 0; i__ <= i__1; ++i__) 
    {
	term = 0;
// Computing MAX
	i__2 = 0, i__3 = i__ - dega;
	i__4 = min(degb,i__);
	for (j = max(i__2,i__3); j <= i__4; ++j) 
        {
// L10:
	    term = field_1.add[term + field_1.mul[pa[i__ - j] + pb[j] * 50] * 50];
	}
// L20:
	pt[i__ + 1] = term;
    }

    pc[-1] = degc;
    i__1 = degc;
    for (i__ = 0; i__ <= i__1; ++i__) 
    {
// L30:
	pc[i__] = pt[i__ + 1];
    }
    for (i__ = degc + 1; i__ <= 50; ++i__) 
    {
// L40:
	pc[i__] = 0;
    }
    return 0;
} /* plymul_ */


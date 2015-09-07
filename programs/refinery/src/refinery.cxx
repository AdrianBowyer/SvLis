/*
 * SvLis - Make the oil refinery model
 *
 * Adrian Bowyer
 *
 * See the svLis web site for the manual and other details:
 *
 *    http://www.bath.ac.uk/~ensab/G_mod/Svlis/
 *
 * First version: 12 December 1995
 * This version: 18 May 1998
 *
 */

#include "svlis.h"
#if macintosh
 #pragma export on
#endif

// Where to put the output

#if macintosh
 #define RES_FILE "::results:refinery.mod"
 #define SAND_TEX "::data:sand.ppm"
 #define SVLIS_TEX "::data:svlis.ppm"
#else
 #define RES_FILE "results/refinery.mod"
 #define SAND_TEX "data/sand.ppm"
 #define SVLIS_TEX "data/svlis.ppm"
#endif

// Colours

#define steel_c sv_point(0.4, 0.4, 0.5)
#define black_c sv_point(0.1, 0.1, 0.1)
#define aluminium_c sv_point(0.6, 0.6, 0.7)
#define brick_c sv_point(0.8, 0.4, 0.2)
#define sand_c sv_point(0.95, 0.93, 0.8)
#define road_c sv_point(0.3, 0.3, 0.3)

// Fudge factor

#define eps 0.000001

// Materials used

sv_surface steel;
sv_surface aluminium;
sv_surface brick;
sv_surface sand;
sv_surface road;
sv_surface black;
sv_surface logo;

// Initialize everything

void ref_init()
{
	svlis_init();

// Set the diffuse reflected colours for all materials

	steel.diffuse_colour(steel_c);
	aluminium.diffuse_colour(aluminium_c);
	brick.diffuse_colour(brick_c);
	sand.diffuse_colour(sand_c);
	road.diffuse_colour(road_c);
	black.diffuse_colour(black_c);
	logo.diffuse_colour(brick_c);

// Set the texture maps for sand and the svLis logo

	sand.texture(sv_read_image(SAND_TEX));
	sand.map_origin(SV_OO);
	sand.map_u(-SV_X);
	sand.map_size(sv_point(30, 30, 0));
	sand.map_type(SV_TILED);

	logo.texture(sv_read_image(SVLIS_TEX));
	logo.map_origin(sv_point(95, 33, 1));
	logo.map_u(-SV_X);
	logo.map_size(sv_point(10, 5, 0));
	logo.map_type(SV_IMAGE);

// Every surface takes cast shadows - switch these off for lots of
// mist.

	road.shadow(1);
	steel.shadow(1);
	aluminium.shadow(1);
	brick.shadow(1);
	sand.shadow(1);
	black.shadow(1);
	logo.shadow(1);

// Every surface is attenuated by mist.

	road.mist(1);
	steel.mist(1);
	aluminium.mist(1);
	brick.mist(1);
	sand.mist(1);
	black.mist(1);
	logo.mist(1);

// Aluminium is the only shiny one

	aluminium.specular_coeff(0.4);

}

/***************************************************************/

// Square-section beam between points p0 and p1,
// width & depth thick

sv_set beam(sv_point p0, sv_point p1, sv_real thick)
{
	sv_point del = p1 - p0;
	sv_real t2 = thick * 0.5;
	sv_real len = del.mod();
	del = del.norm();
	sv_set result;
	sv_real ang;
	sv_line ax;

// Special case - plumb vertical

	if(del.z > 1-eps)
	{
		result = cuboid(sv_point(-t2, -t2, 0),
			sv_point(t2, t2, len));
	}else

// General case

	{
		result = cuboid(sv_point(0, -t2, -t2),
			sv_point(len, t2, t2));
		ang = atan2(del.y, del.x);
		ax = sv_line(SV_Z, SV_OO);
		result = result.spin(ax, ang);
		ax = sv_line(del^SV_Z, SV_OO);
		ang = atan2(del.z, sqrt(del.x * del.x + del.y * del.y));
		result = result.spin(ax, ang);
	}
	return(result+p0);
}

/***************************************************************/

// Flat-ended cylinder between two points p0 & p1 of radius r

sv_set cyl_f(sv_point p0, sv_point p1, sv_real r)
{
	sv_point dir = p1-p0;
	sv_line axis = sv_line(dir, p0);
	sv_set result = cylinder(axis, r);
	result = result & sv_set(sv_plane(-dir, p0)) &
			sv_set(sv_plane(dir, p1));
	return(result);
}

// Round-ended cylinder between two points p0 & p1 of radius r

sv_set cyl_r(sv_point p0, sv_point p1, sv_real r)
{
	return(cyl_f(p0, p1, r)|sphere(p0, r)|sphere(p1, r));
}

/*************************************************************/

// Valve along an axis (centre: axis.origin) for a pipe
// of radius r

sv_set valve(sv_line axis, sv_real r)
{
	sv_set result = cyl_f(axis.origin-axis.direction * 2*r,
			axis.origin+axis.direction * 2*r, 2 * r);
	sv_point d;

// Get handle vertical

	if (fabs(axis.direction.x) < 0.05)
		d = (axis.direction ^ SV_X).norm();
	else
		d = (axis.direction ^ SV_Y).norm();

// Union in handle

	result = result | cyl_f(axis.origin, axis.origin+d * (2 * r + 0.2), 0.1);
	result = result | cyl_f(axis.origin+d * (2 * r + 0.15),
		axis.origin+d * (2 * r + 0.25), 0.4);
	return(result);
}

/**************************************************************/

// Pipe along a poly-line p[], radius r with s straight sections

sv_set poly_pipe(sv_point p[], sv_real r, sv_integer s)
{
	sv_set result = sv_set(SV_NOTHING);

	sv_integer i;
	sv_point a, b;
	sv_real tlen;
	sv_plane af, bf, cf;
	sv_set as, bs, cs;
	sv_line axis;
	flag_val flag;

// Fancy one with tori

	cs = sv_set(sv_plane(p[0]-p[1], p[0]));
	for(i = 1; i < s; i++)
	{
		a = (p[i-1] - p[i]).norm();
		b = (p[i+1] - p[i]).norm();
		tlen = 3 * r/(tan(0.5 * acos(a * b)));
		af = sv_plane(a, p[i]+a * tlen);
		bf = sv_plane(b, p[i]+b * tlen);
		axis = planes_line(af, bf, flag);
		if (flag)
			svlis_error("poly_pipe", "af and bf are parallel",
				SV_WARNING);
		cf = sv_plane(a^b, p[i]);
		axis.origin = axis.origin + plane_line_t(cf, axis,
flag) * axis.direction;
                if (flag)
                        svlis_error("poly_pipe", "a and b are parallel",
				SV_WARNING);
		as = sv_set(af);
		bs = sv_set(bf);
		result = result |
			(torus(axis, 3 * r, r) & (as & bs)) |
			(cylinder(sv_line(-a, p[i-1]), r) & (cs & -as));
		cs = -bs;
	}
	result = result | (cylinder(sv_line(cs.primitive().plane().normal,
		p[s - 1]), r) & (cs & sv_set(sv_plane(p[s]-
		p[s-1], p[s]))));

#if 0

// Simple one with spheres

	for(i = 1; i <= s; i++)
	{
		result = result | cyl_f(p[i-1], p[i], r);
		if(i != s) result = result | sphere(p[i], r);
	}

#endif

	return(result);
}

/**************************************************************/

// Gasometer-style storage tank at point base, radius r

sv_set storage(sv_point base, sv_real r)
{
	sv_set result = sphere(base-3 * r*SV_Z, 4 * r).surface(aluminium);
	result = result & cylinder(sv_line(SV_Z, base), r);
	result = result & sv_set(sv_plane(-SV_Z, base));
	return(result);
}

/**************************************************************/

// High-pressure storage cylinder between points p0 and p1,
// radius r, with stands

sv_set hp_storage(sv_point p0, sv_point p1, sv_real r)
{

// Cylinder is the easy bit

	sv_set result = cyl_r(p0, p1, r);

	sv_point dir = p1-p0;
	sv_point n;

// Get `down' direction

	if (fabs(dir.x) < 0.1)
		n = (dir^SV_X).norm();
	else
		n = (dir^SV_Y).norm();
	if (n.z > 0) n = -n;

// Now build the supports

	sv_set f = sv_set(sv_plane(n, p0+n * r*1.3));
	sv_line axis = sv_line(dir, p0);
	sv_point m = (dir^n).norm();
	sv_set g = sv_set(sv_plane(m, p0+r * m*0.6).spin(axis, 0.3));
	sv_set h = g.mirror(sv_plane(m, p0));
	sv_set base = f & g & h & sv_set(sv_plane(-n, p0));
	dir = dir.norm() * r;
	base = base & sv_set(sv_plane(-dir, p0+dir * 0.5)) &
		sv_set(sv_plane(dir, p1-dir * 0.5));
	base = base - (sv_set(sv_plane(-dir, p0+dir * 0.6)) &
		sv_set(sv_plane(dir, p1-dir * 0.6)));
	result = result | base;
	return(result);
}

/***************************************************************/

// Make an infinite vertical polygonal prism, centred at c,
// `radius' r0, with sides faces

sv_set poly_pris(sv_point c, sv_real r0, sv_integer sides)
{
	sv_line axis = sv_line(SV_Z, c);
	sv_set result = sv_set(SV_EVERYTHING);
	sv_set p0 = sv_set(sv_plane(SV_X, r0 * SV_X+c));
	for(sv_real a = 0; a < 2 * M_PI-eps; a = a + 2 * M_PI/sides)
		result = result & p0.spin(axis, a);
	return(result);
}

// Make a finite polygonal prism starting at c ending distance t
// above c.

sv_set poly_sheet(sv_point c, sv_real r0, sv_real t,
		sv_integer sides)
{
	sv_set result = poly_pris(c, r0, sides);
	result = result & sv_set(sv_plane(SV_Z, c + t * 0.5 * SV_Z)) &
			sv_set(sv_plane(-SV_Z, c - t * 0.5 * SV_Z));
	return(result);
}

// Make a polygonal annulus starting at c, of thickness t, with
// inner `radius' r0 and outer r1, with sides faces

sv_set poly_an(sv_point c, sv_real r0, sv_real r1, sv_real t, sv_integer sides)
{
	sv_set result = poly_pris(c, r1, sides) - poly_pris(c, r0, sides);
	result = result & sv_set(sv_plane(SV_Z, c + t * 0.5 * SV_Z)) &
			sv_set(sv_plane(-SV_Z, c - t * 0.5 * SV_Z));
	return(result);
}

/***************************************************************/

// Make a conical open diagonal-strut framework centred at
// point c, bottom `radius' r0, top r1 of height height and
// with sides faces.

sv_set con_frame(sv_point c,
		sv_real r0,
		sv_real r1,
		sv_real height,
		sv_integer sides)
{
	sv_point sx = c + r0 * SV_X;
	sv_line axis = sv_line(SV_Y, sx);
	sv_set result = sv_set(SV_NOTHING);
	sv_real ang = atan((r0-r1)/height);
	sv_real ainc = 2 * M_PI/sides;

	sv_point tp = sx+sv_point(r1-r0, 0, height);
	sv_set p0 = beam(sx, tp, 0.2);

// Each level is 5m high

	sv_integer levels = (sv_integer)(height * 0.2 + 1);
	axis = sv_line(SV_Z, c);
	sv_real frac;
	sv_point last = sx;
	sv_point here;
	sv_point herer;
	for(sv_integer i = 1; i <= levels; i++)
	{
		frac = (sv_real)i/(sv_real)levels;
		here = sx + frac * (tp-sx);
		herer = here.spin(axis, ainc);
		p0 = p0 | beam(last, herer, 0.2) |
			beam(here, herer, 0.2);
		last = here;
	}


	for(sv_real a = 0; a < 2 * M_PI-eps; a = a + ainc)
		result = result | p0.spin(axis, a);
	result = result & sv_set(sv_plane(SV_Z, SV_Z * height));
	return(result);
}

/***************************************************************/

// Make an open rectangular stand between points sw and ne
// with levels levels.

sv_set rec_stand(sv_point sw, sv_point ne, sv_integer levels)
{
	sv_set result = sv_set(SV_NOTHING);
	sv_real dx = ne.x - sw.x;
	sv_real dy = ne.y - sw.y;
	sv_real dz = ne.z - sw.z;
	sv_integer xincs = (sv_integer)(dx * 0.2 + 1);
	sv_integer yincs = (sv_integer)(dx * 0.2 + 1);
	dx = (dx - 0.2)/xincs;
	dy = (dy - 0.2)/yincs;
	dz = dz/levels;
	for (sv_integer i = 0; i <= xincs; i++)
	{
		for(sv_integer j = 0; j <= yincs; j++)
		{
			result = result | poly_pris(sv_point(
				sw.x + i * dx + 0.1,
				sw.y + j * dy + 0.1, 0), 0.1, 4);
		}
	}
	for(sv_integer j = 1; j <= levels; j++)
	{
		result = result | cuboid(sw + sv_point(0, 0, dz * j),
			ne + sv_point(0, 0, -dz * (levels-j) + 0.1));
	}

	result = result | cuboid(sw + sv_point(0, 0, -5),
			ne + sv_point(0, 0, -dz * levels + 0.1));

	result = result & sv_set(sv_plane(-SV_Z, sw)) &
			sv_set(sv_plane(SV_Z, ne + SV_Z));

	return(result);
}

/**************************************************************/

// Fractionating column of radius r and height h

sv_set column(sv_point base, sv_real r, sv_real h)
{
	sv_set sheets = sv_set(SV_NOTHING);
	sv_integer i;
	for(i = 1; i * 10 < h; i++)
	{
		sheets = sheets |
		   (sv_set(sv_plane(-SV_Z, i * 10 * SV_Z)) &
		    sv_set(sv_plane(SV_Z, (i * 10+0.2) * SV_Z)));
	}
	sv_set pols = poly_pris(base, r+1, 5);
	sv_set result = pols & sheets;
	pols = pols - poly_pris(base, r+0.8, 5);
	sheets = sv_set(SV_NOTHING);
 	for(i = 1; i * 10 < h; i++)
	{
		sheets = sheets |
		   (sv_set(sv_plane(-SV_Z, (i * 10+0.5) * SV_Z)) &
		    sv_set(sv_plane(SV_Z, (i * 10+0.7) * SV_Z)));
	}
	result = result | (sheets & pols);
	result = result.surface(black);
	result = result | cyl_r(base, base + h * SV_Z, r).surface(steel);
	return(result);
}

/***************************************************************/

// build the whole refinery

sv_set refinery()
{
	sv_real x, y, z;
	sv_integer i, j;
	sv_point p, q;
	sv_point pipe[30];
	sv_set temp = sv_set(SV_NOTHING);
	sv_set result = sv_set(sv_plane(SV_Z, SV_OO)).surface(sand);

// Big storage tanks on a hexagonal grid

	for (i = 0; i < 6; i++)
		for(j = 0; j < 4; j++)
	{
		x = 120 + i * 20 + (j%2) * 10;
		y = 40 + j * (10 * sqrt(3));
		if(!((i > 2) && (j == 0)))
		temp = temp | storage(sv_point(x, y, 0), 8);
	}

	temp = temp.surface(aluminium);

	result = result | temp;

// High-pressure storage with wall surround

	temp = sv_set(SV_NOTHING);

	x = 170;
	for (j = 0; j < 5; j++)
	{
		y = 120 + j * 10;
		p = sv_point(x, y, 3 * 1.28);
		q = p + sv_point(23, 0, 0);
		temp = temp | hp_storage(p, q, 3);
	}

	temp = temp.surface(aluminium);

	result = result | temp;

	p = sv_point(160, 110, -10);
	q = sv_point(200, 170, 3.5);
	temp = cuboid(p, q);
	p = sv_point(160.5, 110.5, -20);
	q = sv_point(199.5, 169.5, 10);
	temp = temp - cuboid(p, q);
	temp = temp.surface(brick);

	result = result | temp;

// Tower sphere storage

	p = sv_point(110, 110, 0);
	result = result | con_frame(p, 5, 3.5, 20, 5).surface(black);
	p = sv_point(110, 110, 22.5);
	result = result | sphere(p, 5).surface(aluminium);

// Low-pressure tanks + framework

	p = sv_point(100, 140, 0);
	q = sv_point(140, 165, 8);
	result = result | rec_stand(p, q, 1).surface(black);

	temp = sv_set(SV_NOTHING);
	for(i=0;i<2;i++) for(j=0;j<2;j++)
	{
		p = sv_point(102+i * 20, 146.25+j * 12.5, 5);
		q = p + sv_point(16, 0, 0);
		temp = temp | cyl_f(p, q, 4);
	}

	result = result | temp.surface(aluminium);

// Fractionating columns

	p = sv_point(60, 100, 0);
	q = sv_point(0, 15, 0);
	for(j=0;j<5;j++)
	{
		result = result | column(p, 1, 25);
		p = p + q;
	}

// Roadways - tricky to figure from the | & and - ....

	p = sv_point(88, 180, 0);
	q = sv_point(210, 180, 0);
	sv_set rc1 = sv_set(sv_plane(-SV_X, p));
	sv_set rc2 = sv_set(sv_plane(SV_X, q));
	sv_set rc3 = sv_set(sv_plane(SV_Y, p));
	sv_set rc4 = (rc1 & rc2) | rc3;
	temp = (sv_set(sv_plane(-SV_X, SV_X * 72)) &
		sv_set(sv_plane(SV_Y, SV_Y * 196)) &
		sv_set(sv_plane(SV_X, SV_X * 226)) &
		rc4) |
		cylinder(sv_line(SV_Z, p), 16) |
		cylinder(sv_line(SV_Z, q), 16);
	rc4 = -rc4;
	sv_set cl1 = sv_set(sv_primitive(-1) * p_cylinder(sv_line(SV_Z, p), 8));
	sv_set cl2 = sv_set(sv_primitive(-1) * p_cylinder(sv_line(SV_Z, q), 8));
	sv_plane pl1 = sv_plane(SV_X, SV_X * 80);
	sv_plane pl2 = sv_plane(-SV_Y, SV_Y * 188);
	sv_plane pl3 = sv_plane(-SV_X, SV_X * 218);
	sv_plane pl4 = sv_plane(-SV_Y, SV_Y * 115);
	sv_plane pl5 = sv_plane(SV_Y, SV_Y * 135);
	sv_plane pl6 = sv_plane(SV_X, SV_X * 140);
	sv_plane pl7 = sv_plane(-SV_Y, SV_Y * 127);
	temp = temp &
	 ((sv_set(pl1) | sv_set(pl2) | (sv_set(pl3) & sv_set(pl4)) |
	   (rc4 & (cl1 & cl2))) |
	  (sv_set(pl5) & sv_set(pl6) & sv_set(pl7)));
	p = sv_point(140, 119, 0);
	q = sv_point(226, 115, 0);
	temp = temp | cylinder(sv_line(SV_Z, p), 16) |
		cylinder(sv_line(SV_Z, q), 16);
	temp = temp & sv_set(sv_plane(-SV_Z, -SV_Z * 0.25));
	temp = temp.surface(road);
	result = result - temp;

// Notice board

	result = result |
	(cuboid(sv_point(84, 30, -5), sv_point(96, 35, 7)).surface(brick) &
		sv_set(sv_plane(-SV_Y, sv_point(85, 33, 2))).surface(logo)
	);
// Pipework

// From frac columns

	pipe[0] = sv_point(60, 100, 25);
	pipe[1] = sv_point(60, 100, 27);
	pipe[2] = sv_point(61.5, 100, 27);
	pipe[3] = sv_point(61.5, 100, 1);
	pipe[4] = sv_point(67, 100, 1);
	pipe[5] = sv_point(67, 150, 1);
	pipe[6] = sv_point(70, 150, 1);
	pipe[7] = sv_point(70, 150, 8);
	pipe[8] = sv_point(82, 150, 8);
	pipe[9] = sv_point(82, 150, 1);
	pipe[10] = sv_point(110, 150, 1);

	result = result | poly_pipe(pipe, 0.3,
		10).surface(steel);
	for(i = 0; i < 5; i++)
		pipe[i] = pipe[i] + sv_point(0, 15, 0);
	for(i = 4; i < 6; i++)
		pipe[i] = pipe[i] + sv_point(-1, 0, 0);
	for(i = 5; i < 11; i++)
		pipe[i] = pipe[i] + sv_point(0, 1, 0);
	result = result | poly_pipe(pipe, 0.3,
		10).surface(steel);
	for(i = 0; i < 5; i++)
		pipe[i] = pipe[i] + sv_point(0, 15, 0);
	for(i = 4; i < 6; i++)
		pipe[i] = pipe[i] + sv_point(-1, 0, 0);
	for(i = 5; i < 11; i++)
		pipe[i] = pipe[i] + sv_point(0, 1, 0);
	result = result | poly_pipe(pipe, 0.3,
		10).surface(steel);
	for(i = 0; i < 5; i++)
		pipe[i] = pipe[i] + sv_point(0, 15, 0);
	for(i = 4; i < 6; i++)
		pipe[i] = pipe[i] + sv_point(-1, 0, 0);
	for(i = 5; i < 11; i++)
		pipe[i] = pipe[i] + sv_point(0, 1, 0);
	result = result | poly_pipe(pipe, 0.3,
		10).surface(steel);
	for(i = 0; i < 5; i++)
		pipe[i] = pipe[i] + sv_point(0, 15, 0);
	for(i = 4; i < 6; i++)
		pipe[i] = pipe[i] + sv_point(-1, 0, 0);
	for(i = 5; i < 11; i++)
		pipe[i] = pipe[i] + sv_point(0, 1, 0);
	result = result | poly_pipe(pipe, 0.3,
		10).surface(steel);

	return(result);
}

int main()
{
	ref_init();
	sv_set s = refinery();
	sv_model m = sv_model(s, sv_box(
		sv_point(0, 0, -30),
		sv_point(250, 200, 50)), sv_model());

	ofstream of(RES_FILE);
	if (!of)
		cerr << "Can't open file refinery.mod in directory results" << SV_EL;
	else
		of << m;
	of.close();
	return(0);
}
#if macintosh
 #pragma export off
#endif



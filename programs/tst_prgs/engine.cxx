
// Demonstration of svLis's animation capabilities

#include <svlis.h>
#include "sv_cols.h"

#if macintosh
 #pragma export on
#endif

#define CRL 8
#define CONL 15
#define PL 6
#define PD 6
#define SD 1

sv_model crank()
{
	sv_point bl = sv_point(-SD, -SD, -SD*2);
	sv_point tr = bl + sv_point(CRL + 2*SD, 2*SD, SD);
	sv_set cr = cuboid(bl, tr).colour(SV_RED);
	sv_set cyl = poly_cylinder(SV_ZL, SD*0.5);
	cyl = cyl & sv_set(sv_plane(-SV_Z, -SV_Z*SD*1.5));
	cyl = cyl & sv_set(sv_plane(SV_Z, SV_Z*SD));
	cyl = cyl.colour(SV_RED);
	sv_set_list r = sv_set_list(cr);
	r = merge(r, cyl - SV_Z*SD*3);
	r = merge(r, cyl + SV_X*CRL);

	bl = bl - SV_DIAG;
	tr = tr + SV_DIAG + SV_Z*3*SD;
	return(sv_model(r, sv_box(bl,tr)));
}

sv_model con()
{
	sv_point bl = sv_point(-SD, -SD, -SD*0.5);
	sv_point tr = bl + sv_point(CONL + 2*SD, 2*SD, SD);
	sv_set cr = cuboid(bl, tr).colour(SV_GREEN);
	bl = bl - SV_DIAG;
	tr = tr + SV_DIAG;
	return(sv_model(cr, sv_box(bl,tr)));
}

sv_model piston()
{
	sv_set cyl = poly_cylinder(SV_XL, PD*0.5);
	cyl = cyl & sv_set(sv_plane(SV_X, SV_X*(PD - SD)));
	cyl = cyl & sv_set(sv_plane(-SV_X, -SV_X*SD));
	cyl = cyl & (poly_cylinder(SV_ZL, SD*0.5)  | 
		(-(poly_cylinder(SV_XL, (PD - SD)*0.5) & 
		sv_set(sv_plane(SV_X, SV_X*(PD - 2*SD))))));
	cyl = cyl.colour(SV_BLUE);
	sv_point bl = sv_point(-SD*2, -(PD+SD), -(PD+SD));
	sv_point tr = bl + sv_point(PD + 2*SD, 2*(PD+SD), 2*(PD+SD));
	return(sv_model(cyl, sv_box(bl,tr)));
}

int main()
{
	sv_model cb;
	char dummy;

	set_small_volume(0.2);

	sv_enclosure(sv_box(sv_point(-10,-10,-10), sv_point(30,10,10)));

	sv_model_list* pcr = add_model_to_picture(crank(), SV_OO, SV_ZL, 0, 0);
	sv_model_list* pcn = add_model_to_picture(con(), SV_X*CRL, SV_ZL, 0, 0);
	sv_model_list* ppi = add_model_to_picture(piston(), SV_X*(CRL+CONL), SV_ZL, 0, 0);

	cout << "Type any character to continue: ";
	cin >> dummy;

	sv_point join;

	sv_real th = 0, phi = 0, y, x0, x1;
	sv_integer count = 0;
	cout << "Number of frames of animation: ";
	cin >> count;
	while(count)
	{
		pcr->new_transform(SV_OO, SV_ZL, th);
		x0 = CRL*cos(th);
		y = CRL*sin(th);
		join = sv_point(x0, y, 0);
		x1 = sqrt(CONL*CONL-y*y);
		phi = atan2(y,x1);
		pcn->new_transform(join, SV_ZL, -phi);
		ppi->new_transform(SV_X*(x0+x1), SV_ZL, 0);
		sv_draw_all();
		th = th + 0.03;
		if(th > 2*M_PI) th = th - 2*M_PI;
		count--;
	}

	return(svlis_end(0));
}
#if macintosh
 #pragma export off
#endif

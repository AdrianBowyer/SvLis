/*
 * SvLis Voronoi test program
 *
 *   Adrian Bowyer  10 February 1999
 * 
 *   This interactively creates a Voronoi diagram.
 */  

#include <svlis.h>
#if macintosh
 #pragma export on
#endif

int main()
{
	char r = ' ';
	sv_point p, q;
	sv_integer i, c;

	svlis_init();

	cout << SV_EL << "SvLis Voronoi diagram test" << SV_EL << SV_EL;

	cout << "Lowest corner of enclosing box (x, y, z): ";
	cin >> p.x >> p.y >> p.z;
	cout << "Highest corner of enclosing box (x, y, z): ";
	cin >> q.x >> q.y >> q.z;
	sv_voronoi v = sv_voronoi(sv_box(p,q));

	while(r != 'q')
	{
		cout << SV_EL << "SvLis Voronoi (? for help): ";
		cin >> r;
		switch(r)
		{
		case 'q': break;
		case 'a':
			cout << " Point coordinates (x, y, z): ";
			cin >> p.x >> p.y >> p.z;
			v.add_point(p);
			break;
		case 'r':
			cout << " Number of random points to add: ";
			cin >> c;
			for(i = 0; i < c; i++)
				v.add_point(ran_point(v.box()));
			break;
		case 'p':
			plot_voronoi(v);
			break;

		case '?':
		default:
			cout << " ? - print this list." << SV_EL;
			cout << " a - add a point to the structure." << SV_EL;
			cout << " r - add some random points." << SV_EL;
			cout << " p - plot the diagram." << SV_EL;
			cout << " q - quit." << SV_EL;
		}
	}

	return(0);
}
#if macintosh
 #pragma export off
#endif

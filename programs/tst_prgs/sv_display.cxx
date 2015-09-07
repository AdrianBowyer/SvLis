/*
 * Simple program to read in a svLis model and display it
 *
 *  Adrian Bowyer
 *
 *   First version 1 November 1998
 *   This version 1 December 1998
 */

#include <svlis.h>
#if macintosh
 #pragma export on
#endif

int main(int argc, char* argv[])
{
	sv_model m;

	svlis_init();
	if(argc != 2)
	{
		cerr << "Usage: sv_display svlis_model.mod" << SV_EL;
		return(svlis_end(1));
	}
	
	ifstream ifs(argv[1]);
	if(!ifs)
	{
		cerr << "Can't open file: " << argv[1] << SV_EL;
		return(svlis_end(1));
	}

	ifs >> m;
	m = m.facet();
	char* title = new char[sv_strlen(argv[1]) + sv_strlen("svLis - ") + 3];
	sv_strcpy(title, "svLis - ");
	sv_strcat(title, argv[1]);
	plot_m_p_gons(m, title);

	return(svlis_end(0));
}
#if macintosh
 #pragma export off
#endif

/* 
 * SvLis graphics test program 
 * 
 *   Adrian Bowyer  15 May 1994 
 * 
 *   This reads a model from a file, facets it, then 
 *   plots it. 
 */ 
 
#include <svlis.h> 
#if macintosh 
 #pragma export on 
#endif 
 
// Where to get the model 
 
#if macintosh 
 #define RES_FILE "::results:sv_tst.mod" 
#else 
 #define RES_FILE "results/sv_tst.mod" 
#endif 
 
int main() 
{ 
	char dummy; 
 
// Initialize svLis and declare a model 
 
	svlis_init(); 
	sv_model m; 
 
// Read the model 
 
	ifstream ipf(RES_FILE); 
	if(!ipf) 
	{ 
		svlis_error("sv_tst_graph","can't open input file sv_tst.mod in directory results", SV_WARNING); 
		return(svlis_end(1)); 
	} else 
		ipf >> m; 
 
// Facet it and plot the results 
 
	m = m.facet(); 
	plot_m_p_gons(m); 
	cout << SV_EL << SV_EL << "SvLis test program sv_tst_g has finished successfully." << SV_EL << SV_EL; 
 
	return(svlis_end(0)); 
} 
#if macintosh 
 #pragma export off 
#endif 

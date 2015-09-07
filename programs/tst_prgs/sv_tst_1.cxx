/* 
 * SvLis test program number 1 
 * 
 *   Adrian Bowyer  15 May 1994 
 * 
 *   This creates a simple model then writes it to a file. 
 */ 
 
#include "svlis.h" 
 
#include "sv_cols.h" 
#if macintosh 
 #pragma export on 
#endif 
 
#if macintosh 
 #define RES_FILE "::results:sv_tst.mod" 
 #define TEX_FILE "::data:svlis.ppm" 
#else 
 #define RES_FILE "results/sv_tst.mod" 
 #define TEX_FILE "data/svlis.ppm" 
#endif 
 
int main() 
{ 
    char dummy; 
 
// Initialise svLis 
 
    svlis_init(); 
 
// Define the corners of a box, then the box 
 
    sv_point p = SV_OO; 
    sv_point q = sv_point(10,10,10); 
    sv_box mod_box = sv_box(p,q); 
 
// Now a slightly smaller cuboid set 
 
    p = p + q*0.2; 
    q = q - q*0.2; 
    sv_set c = cuboid(p,q); 
 
// Put a texture on the cuboid 
 
    sv_surface tex = sv_surface(); 
    tex.texture(sv_read_image(TEX_FILE)); 
    tex.map_origin(SV_OO); 
    tex.map_u(sv_point(1,1,1)); 
    tex.map_size(sv_point(2,1,0)); 
    tex.map_type(SV_TILED); 
    c = c.surface(tex); 
 
// A sphere intersected with the cuboid; colour it greenish 
 
    sv_set result = sphere(sv_point(5,5,5),4) & c; 
    result = result.colour(SV_MEDIUM_SEA_GREEN); 
 
// Make sure anything that needs to can find attributes 
// (i.e. colours) where it expects. 
 
    result = result.percolate(); 
 
// Build a model with the result and the box 
 
    sv_model m = sv_model(result,mod_box,sv_model()); 
 
// Switch on pretty_print output 
 
    pretty_print(1); 
 
// Write the file 
 
    ofstream opf(RES_FILE); 
	if(!opf) 
	{ 
		svlis_error("sv_tst_1","can't open output file sv_tst.mod in directory results", SV_WARNING); 
		return(svlis_end(1)); 
	} else 
		opf << m; 
 
	cout << SV_EL << SV_EL << "SvLis test program sv_tst_1 has finished successfully." << SV_EL << SV_EL; 

	return(svlis_end(0)); 
} 
 
#if macintosh 
 #pragma export off 
#endif 

/* 
 * Program to convert files from the last version to this. 
 * 
 *  Adrian Bowyer 
 * 
 *   First version 1 January 1999 
 * 
 */ 
 
#include "svlis.h" 
#if macintosh 
 #pragma export on 
#endif 
 
// Tatty old Version 1 stuff 
 
#define BASE_C ' ' 
#define start_k(a) ((char)(a.tag() + BASE_C)) 
 
 
int main(int argc, char* argv[]) 
{ 
	sv_model m; 
	sv_set_list sl; 
	sv_attribute a; 
	sv_set s; 
	sv_primitive p; 
	char c; 
 
	svlis_init(); 
 
	if(argc != 2) 
	{ 
		cerr << "Usage: sv_convert svlis_file" << SV_EL; 
		return(1); 
	} 
 
	ifstream ifs(argv[1]); 
	if(!ifs) 
	{ 
		cerr << "Can't open " << argv[1] << SV_EL; 
		return(1); 
	} 
 
        sv_integer ver = -1; 
        sv_real r; 
 
	cerr << SV_EL << "Ignore the next warning message." << SV_EL; 
        check_svlis_header(ifs); 
 
	if(get_read_version() != get_svlis_version() - 1) 
	{ 
		cerr << "This program can only update svlis files from version "; 
		cerr << get_svlis_version() - 1 << " to version " << 
			get_svlis_version() << "." << SV_EL; 
		return(svlis_end(1)); 
	} 
 
	sv_tag thing = SVT_POINT; 
	if(get_read_version() == 1) 
	{ 
		ifs >> c; 
		if(c == start_k(m)) 
			thing = SVT_MODEL; 
		else if(c == start_k(sl)) 
			thing = SVT_SET_LIST; 
		else if(c == start_k(s)) 
			thing = SVT_SET; 
		else if(c == start_k(a)) 
			thing = SVT_ATTRIBUTE; 
		else if(c == start_k(p)) 
			thing = SVT_PRIM; 
		ifs.putback(c); 
	} else 
		thing = get_token(ifs, ver, r, 0); 
 
	ifs.close(); 
	if((thing != SVT_MODEL) && (thing != SVT_SET_LIST) &&  (thing != SVT_SET) && 
		(thing != SVT_ATTRIBUTE) && (thing != SVT_PRIM)) 
	{ 
		cerr << "sv_convert can only convert models, set lists, sets, "; 
		cerr << "attributes and primitives." << SV_EL; 
		return(svlis_end(1)); 
	} 
 
	sv_integer slen = sv_strlen(argv[1]); 
	sv_integer dot = slen; 
	for(int i = 0; i < slen; i++) if(argv[1][i] == '.') dot = i; 
	char* oldf = new char[dot + 6]; 
	sv_strcpy(oldf, argv[1]); 
	oldf[dot] = 0; 
	sv_strcat(oldf,".old"); 
	char resp = 'a'; 
	struct stat buf; 
	int fs = stat(oldf, &buf); 
	while(!fs && (resp != 'y') && (resp != 'n')) 
	{ 
		cout << "Overwrite " << oldf << "? "; 
		cin >> resp; 
		if(resp == 'n' || resp == 'N') 
		{ 
			cerr << "SvLis file " << argv[1] << "not updated." << SV_EL; 
			return(svlis_end(1)); 
		} 
		if(resp == 'Y') resp = 'y'; 
		if(resp != 'y') cerr << "Please answer y or n." << SV_EL; 
	} 
 
	rename(argv[1], oldf); 
 
	ifstream ifs2(oldf); 
	if(!ifs2) 
	{ 
		cerr << "Can't open " << oldf << " for reading."; 
		return(svlis_end(1)); 
	} 
 
	ofstream ofs(argv[1]); 
	if(!ofs) 
	{ 
		cerr << "Can't open " << argv[1] << " for writing."; 
		return(svlis_end(1)); 
	} 
 
	cerr << SV_EL << "Ignore the next warning message:" << SV_EL; 
 
	switch(thing) 
	{ 
	case SVT_MODEL: 
		ifs2 >> m; 
		ofs << m; 
		break; 
 
	case SVT_SET_LIST: 
		ifs2 >> sl; 
		ofs << sl; 
		break; 
 
	case SVT_ATTRIBUTE: 
		ifs2 >> a; 
		ofs << a; 
		break; 
 
	case SVT_SET: 
		ifs2 >> s; 
		ofs << s; 
		break; 
 
	case SVT_PRIM: 
		ifs2 >> p; 
		ofs << p; 
		break; 
 
	default: 
		svlis_error("sv_convert","dud object type", SV_CORRUPT); 
		return(1); 
	} 
 
	cout << SV_EL << "Svlis file " << argv[1] << " updated.  Original is " << oldf << "." << SV_EL << SV_EL; 
 
	return(svlis_end(0)); 
} 
#if macintosh 
 #pragma export off 
#endif 

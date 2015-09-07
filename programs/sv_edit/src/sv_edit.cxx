/*
 *   SVLIS's text-based model editor
 *
 *   Adrian Bowyer - 19 October 1996
 */


#include "svlis.h"

#include "edittool.h"
#include "sv_edit.h"
#if macintosh
 #pragma export on
#endif

#define BS 8		        // Backspace
#define I_STK 20		// Include file stack

istream* cip[I_STK];		// So we can swap input to a batch file
sv_integer isp = 0;
ofstream* logfile;	        // Log file
ofstream* cf;			// Commetary file
sv_integer prp = 1;	        // Print prompts (not when in batch)
sv_integer logging = 0;         // Log input flag
sv_integer pretty = 0;          // Flag for prettyprinting
sv_integer djf = 0;             // Flag for disjunctive form
sv_integer c_prune = 0;         // Flag for regularized pruning
sv_integer com = 0;		// Flag for commentary

sv_integer mouse = 0;	        // Flag for getting rays from the mouse

char ip[STLEN];		// Command line read

// Commentary

void co(char* s)
{
	if(!com) return;
	*cf << s;
	cf->flush();
}

void con() { co(SV_EL); }

// Shut down the logfile

void stoplog()
{
	*logfile << "n" << SV_EL;
	logfile->close();
	delete logfile;
	co("Logging stopped and logfile closed."); co(SV_EL);
}

// Finish up

void sv_edit_windup_1()
{
    if(logging) stoplog();
    if(com) cf->close();
	if(isp > 0) eprompt("Warning - quitting from within included file."); eprompt(SV_EL);
	return;
}

void sv_edit_windup()
{
	sv_edit_windup_1();
    sv_edit_close();
	exit(0);
}

// Put a string in lower case

void lowcase(char s[])
{
	sv_integer i = 0;
	while(s[i])
	{
		if(isupper(s[i])) s[i] = s[i] + 'a' - 'A';
		i++;
	}
}

// Replace ( ) , and ; with space

void sp_str(char s[])
{
	sv_integer i = 0;
	while(s[i])
	{
		if(s[i] == ',') s[i] = ' ';
		if(s[i] == ';') s[i] = ' ';
		if(s[i] == '(') s[i] = ' ';
		if(s[i] == ')') s[i] = ' ';
		i++;
	}
}

// Set the input string empty

inline void ip_0() { ip[0] = 0; }

// Check for exhausted input stream and get next input character

void get_next(char& c)
{
	if(isp >= 0)
	{
		if(!cip[isp]->eof())
        {
			cip[isp]->get(c);
			return;
		}
		co("End of included file."); co(SV_EL);
		isp--;
		if(isp == 0) prp = 1;
		if(isp < 0)
		{
			sv_edit_windup_1();
			exit(0);
		}else
			cip[isp]->get(c);
	} else
	{
		eprompt("Error: premature end of input."); eprompt(SV_EL);
		sv_edit_windup_1();
		exit(0);
	}
}

// Read a command line

int rd()
{
	sv_integer i = 0;
	char c;
        char instr[STLEN];

	get_next(c);

// Ignore comments

	if(c == SV_COM_CHAR)
        {
               do
               {
                     get_next(c);
               } while( !newline(c) );
	       get_next(c);
        } 

// Pull in line and deal with backspaces

	while( !newline(c) ) 
	{
		if((c == BS) && (i > 0))
			i--;
		else
			if (c != BS) instr[i++] = c;
		get_next(c);
	}

	instr[i] = 0;

// If the user's typed something copy it to ip

	if(i) sv_strcpy(ip,instr);

// Write either the default or the user's i/p to the logfile if needed

	if(logging) *logfile << ip << SV_EL;

	sp_str(ip);

// Return 0 for a blank line

	return(i);
}

// Get ip and put in lower case

inline int rdl()
{
	int i = rd();
	lowcase(ip);
	return(i);
}

// Error prompt - make sure it appears even from batch

void eprompt(char* s)
{
	cerr << s;
	cerr.flush();
	co(s);        // Put it in the commetary file too
}

// Erroneous command

void cerror(char a)
{
	char as[2];
	eprompt("  The command `");
	as[0] = a;
	as[1] = 0;
	eprompt(as);
	eprompt("' is not valid."); eprompt(SV_EL);
}
	

// Ordinary prompt - supressed in batch mode

void prompt(char* s)
{
	if(!prp) return;
	cout << s;
	cout.flush();
}

// Prompt to both the commentary and the screen

void cprompt(char* s)
{
	co(s);
	prompt(s);
}

// Bit of prompt for a default

void defp(char* s)
{
	prompt("[ENTER for ");
	prompt(s);
	prompt("]: ");
}

// Put a point in a string

void p_to_str(char* str, const sv_point& p)
{
	ostrstream ost(str, STLEN);
	ost << '(' << p.x << ", ";
	ost << p.y << ", ";
	ost << p.z << ')' << '\0';
}

// Put a real in a string

void r_to_str(char* str, sv_real r)
{
	ostrstream ost(str, STLEN);
	ost << r << '\0';
}

// Get a single default point

int def_point(sv_point& p)
{
	p_to_str(ip,p);
	defp(ip);
	int i = rd();
	co(ip);
	istrstream ist(ip, STLEN);
	ist >> p.x;
	ist >> p.y;
	ist >> p.z;
	return(i);	
}

// Get a single default real

int def_real(sv_real& r)
{
	r_to_str(ip,r);
	defp(ip);
	int i = rd();
	co(ip);
	istrstream ist(ip, STLEN);
	ist >> r;
	return(i);
}

// Put up a message and wait for a yes/no answer.

sv_integer sv_edit_message(char* msg, int def_n)
{
	prompt(msg);
        if(def_n)
		sv_strcpy(ip,"n");
	else
		sv_strcpy(ip,"y");
	defp(ip);
	rdl();

	while((ip[0] != 'y') && (ip[0] != 'n'))
	{
		eprompt("    You must answer y(es) or n(o)."); eprompt(SV_EL);
		prompt(msg);
		rdl();
	}              
	return(ip[0] == 'y');
}

// Get a surface and its texture

void get_surface(sv_surface &sf)
{
   sv_point p;
   sv_real r;
   sv_integer i;

   co("Setting new surface characteristics."); co(SV_EL);

   cprompt("    surface's diffuse colour ");
   p = sf.diffuse_colour();
   def_point(p);
   con();
   sf.set_diffuse_colour(p);

   cprompt("    surface's diffuse coefficient ");
   r = sf.diffuse_coeff();
   def_real(r);
   con();
   sf.set_diffuse_coeff(r);

   cprompt("    surface's specular colour ");
   p = sf.specular_colour();
   def_point(p);
   con();
   sf.set_specular_colour(p);

   cprompt("    surface's specular coefficient ");
   r = sf.specular_coeff();
   def_real(r);
   con();
   sf.set_specular_coeff(r);

   cprompt("    surface's specular angle power ");
   r = sf.specular_angle_power();
   def_real(r);
   con();
   sf.set_specular_power(r);

   i = sf.mirror();
   prompt("    The surface is ");
   if(!i) prompt("not ");
   prompt("a mirror."); prompt(SV_EL);
   if(!sv_edit_message("    Keep it that way? ", 0)) i = !i;
   sf.set_mirror(i);
   co("    surface ");
   if(i)
	co("is ");
   else
	co("is not ");
   co("a mirror."); co(SV_EL);

   cprompt("    surface's mirror attenuation ");
   r = sf.attenuation();
   def_real(r);
   con();
   sf.set_attenuation(r);

   cprompt("    surface's transmission coefficient ");
   r = sf.transmission();
   def_real(r);
   con();
   sf.set_transmission(r);

   i = sf.shadow();
   prompt("    The surface will ");
   if(!i) prompt("not ");
   prompt("exhibit shadows."); prompt(SV_EL);
   if(!sv_edit_message("    Keep it that way? ", 0)) i = !i;
   sf.set_shadow(i);
   co("    surface ");
   if(i)
	co("will ");
   else
	co("will not ");
   co("exhibit shadows."); co(SV_EL);

   i = sf.mist();
   prompt("    The surface colour will ");
   if(!i) prompt("not ");
   prompt("be attenuated by mist."); prompt(SV_EL);
   if(!sv_edit_message("    Keep it that way? ", 0)) i = !i;
   sf.set_mist(i);
   co("    surface colour");
   if(i)
	co("will ");
   else
	co("will not ");
   co("be attenuated by mist."); co(SV_EL);

   ip_0();
   prompt("    File name for the surface's texture (ENTER for none): ");
   if(rd())
   {
   	sf.set_texture(sv_read_image(ip));
        co("    surface texture read from file ");
        co(ip);
        con();

	cprompt("    texture map origin ");
	p = sf.map_origin();
	def_point(p);
        con();
	sf.set_map_origin(p);

	cprompt("    texture map's `x' dirction ");
	p = sf.map_u_dir();
	def_point(p);
        con();
	sf.set_map_u(p);

	p = sf.map_uv_size();
	cprompt("    texture map's `x' length ");
	def_real(p.x);
        con();
	cprompt("    texture map's `y' length ");
	def_real(p.y);
        con();
	sf.set_map_size(p);

	i = sf.map_type();
	prompt("    The map is ");
        switch(i)
        {
	case SV_IMAGE: prompt("a single image."); prompt(SV_EL); break;
	case SV_TILED: prompt("tiled."); prompt(SV_EL); break;
	case SV_SOLID_TEX: prompt("a solid texture."); prompt(SV_EL); break;
	default:
		svlis_error("get_surface","dud texture map flag",SV_WARNING);
	}
	r = i;
	cprompt("    New texture type (0 for image, 1 for tiled, 2 for solid) ");
	def_real(r);
	con();
	i = round(r);
	sf.map_type(i);

	i = sf.map_0_xmit();
	prompt("    Colour 0 in the map is ");
	if(!i) prompt("not ");
	prompt("transparent."); prompt(SV_EL);
	if(!sv_edit_message("    Keep it that way? ", 0)) i = !i;
	sf.set_map_0_xmit(i);
	co("    colour (0,0,0) in the map ");
   	if(i)
		co("will ");
   	else
		co("will not ");
   	co("be transparent."); co(SV_EL);
   } else
        co("    No surface texture set."); co(SV_EL);
}


// Add a set to the model

void add_new_set(const sv_set& s, sv_integer ask)
{
	sv_ed_op op;
	int ok = 0;

	sv_strcpy(ip,"|");

	if(ask)
        {
	  while(!ok)
	  {
	     sv_strcpy(ip,"|");
	     prompt("    set operator {& - ^ or |} ");
	     defp(ip);
	     rd();
	     if((ip[0] != '|') 
		&& (ip[0] != '&')
		&& (ip[0] != '-')
		&& (ip[0] != '^')
	       ) {eprompt("    You must answer & - ^ or | ."); eprompt(SV_EL);}
	     else
	        ok = 1;
	  }
	}
	switch(ip[0])
	{
	case '|': op = UNI; break; 
	case '&': op = INTS; break;
	case '-': op = DIF; break;
	case '^': op = S_DIF; break;
	default:
		svlis_error("add_new_set","dud operator", 
			SV_CORRUPT);
		return;
	}

	sv_surface sf = current_surface();
	sv_point col = sf.diffuse_colour();
	if(ask)
	{
	  prompt("    colour (-ve to change whole surface) ");
	  co("[colour ");
	  def_point(col);
	  co("] ");
	  if( (col.x < 0) || (col.y < 0) || (col.z < 0) ) 
	  {
		get_surface(sf);
		col = sf.diffuse_colour();
	  }
	}
	sf.set_diffuse_colour(col);
	new_surface(sf);
	sv_set sc = s.surface(sf);
	add_set(sc, op);
	switch(op)
	{
	case UNI: co("unioned with "); break; 
	case INTS: co("intersected with ");break;
	case DIF: co("subtracted from ");break;
	case S_DIF: co("symmetric differenced with ");break;
	default:
		svlis_error("add_new_set","dud operator", 
			SV_CORRUPT);
	}
	co("the model."); co(SV_EL);
}

// Add a point to the model

void add_point()
{
	sv_point p;
	sv_real r;
	current_sphere(&p, &r);
	co("Point at ");
	prompt("    point's position ");
	def_point(p);
	new_sphere(p,r);
	co(" ");
	add_new_set(sv_set(p),1);
}

// Get a line for either raytrace or inclusion in the model

sv_line get_line(char* thing)
{
	sv_line l;
	sv_real r;
	current_cyl(&l, &r);
	prompt("    ");
	prompt(thing);
	prompt("'s direction ");
	def_point(l.direction);
	co(" [direction ");
	co(", origin ");
	prompt("    ");
	prompt(thing);
	prompt("'s origin ");
	def_point(l.origin);
	co("] ");
	new_cyl(l,r);
	return(l);
}

// Add a line to the model

void add_line()
{
	co("Line ");
	add_new_set(get_line("line"),1);
}

// Add a plane to the model

void add_plane(sv_integer thin)
{
	sv_plane f = current_plane();
	sv_point n = f.normal;
        sv_point o = -f.d*f.normal;
	co("plane [normal: ");
	prompt("    plane's normal ");
	def_point(n);
	co(", origin: ");
	prompt("    plane's origin ");
	def_point(o);
	co("] ");
	f = sv_plane(n,o);
	new_plane(f);

	if(thin)
		add_new_set(sv_set(abs(sv_primitive(f))),1);
	else
		add_new_set(sv_set(sv_primitive(f)),1);
}

// Add a tetrahedon

void add_tetrahedron()
{
	sv_set s;
	sv_point v[4];

	co("Tetrahedron [");
	for(int i = 0; i < 4; i++)
        {
		prompt("    tetrahedron's vertex ");
		v[i] = SV_OO;
		def_point(v[i]);
		co(", ");
	}
	co("] ");

	s = tetrahedron(v);
	add_new_set(s,1);
}

// Add a cuboid

void add_cuboid()
{
	sv_set s;
	sv_box b = current_cuboid();
	sv_point l, h;

	prompt("    cuboid's bottom corner ");
	co("Cuboid [");
	l = b.corner(0);
	def_point(l);
	co(", ");
	prompt("    cuboid's top corner ");
	h = b.corner(7);
	def_point(h);
	co("] ");
	b = sv_box(l,h);
	new_cuboid(b);
	s = cuboid(b);
	add_new_set(s,1);
}

// Add a cylinder

void add_cylinder(sv_integer thin)
{
	sv_line l;
	sv_real r;

	current_cyl(&l, &r);
	co("cylinder [axis direction: ");
	prompt("    cylinder's axis direction ");
	def_point(l.direction);
	co(", origin: ");
	prompt("    cylinder's axis origin ");
	def_point(l.origin);
	co(", radius: ");
	prompt("    cylinder's radius ");
	def_real(r);
	co("] ");
	new_cyl(l,r);

	if(thin)
		add_new_set(thin_cylinder(l,r),1);
	else
		add_new_set(cylinder(l,r),1);
}

// Add a new cone

void add_cone(sv_integer thin)
{
	sv_line l;
	sv_real r;

	current_cyl(&l, &r);
        r = r*180/M_PI;
	co("cone [axis direction: ");
	prompt("    cone's axis direction ");
	def_point(l.direction);
	co(", origin: ");
	prompt("    cone's axis origin ");
	def_point(l.origin);
	co(", angle: ");
	prompt("    cones's included angle (degrees) ");
	def_real(r);
	co("] ");
	r = r*M_PI/180;
	new_cyl(l,r);

	if(thin)
		add_new_set(thin_cone(l,r),1);
	else
		add_new_set(cone(l,r),1);
}


// Add a sphere

void add_sphere(sv_integer thin)
{
	sv_point c;
	sv_real r;

	current_sphere(&c, &r);
	co("sphere [centre: ");
	prompt("    sphere's centre ");
	def_point(c);
	co(", radius: ");
	prompt("    sphere's radius ");
	def_real(r);
	co("] ");
	new_sphere(c,r);

	if(thin)
		add_new_set(thin_sphere(c,r),1);
	else
		add_new_set(sphere(c,r),1);
}

// Add a torus

void add_torus(sv_integer thin)
{
	sv_line l;
	sv_real r0,r1;
	sv_point sym;

	current_cyl(&l, &r0);
	co("torus [axis direction: ");
	prompt("    torus's axis direction ");
	def_point(l.direction);
	co(", origin: ");
	prompt("    torus's axis origin ");
	def_point(l.origin);
	co(", major radius: ");
	prompt("    torus's major radius ");
	def_real(r0);
	co(", minor radius: ");
	prompt("    torus's minor radius ");
        r1 = r0*0.25;
	def_real(r1);
	new_cyl(l,r0);

	if(thin)
		add_new_set(thin_torus(l,r0,r1),1);
	else
		add_new_set(torus(l,r0,r1),1);
}


// Add a cyclide

void add_cyclide(sv_integer thin)
{
	sv_line l;
	sv_real r0,r1,r2;
	sv_point sym = SV_Y;

	current_cyl(&l, &r0);
	co("cyclide [axis direction: ");
	prompt("    cyclide's axis direction ");
	def_point(l.direction);
	co(", origin: ");
	prompt("    cyclide's axis origin ");
	def_point(l.origin);
	co(", major radius: ");
	prompt("    cyclide's major radius ");
	def_real(r0);
	co(", minor radius: ");
	prompt("    cyclide's minor radius ");
        r1 = r0*0.25;
        r2 = r1*0.2;
	def_real(r1);
	co(", radius variation: ");
	prompt("    cyclide's radius variation");
	def_real(r2);
	co(", symmetry point: ");
	prompt("    cyclide's symmetry point ");
	def_point(sym);
	co("] ");
	new_cyl(l,r0);

	if(thin)
		add_new_set(thin_cyclide(l,sym,r0,r1,r2),1);
	else
		add_new_set(cyclide(l,sym,r0,r1,r2),1);
}

// Add a model from a file

void add_model(char* fn, sv_integer ask)
{
	sv_model m;

	co("Model from file ");
	new_name(fn);
	co(fn);
	co(" ");
	ifstream mf(current_name());
	if(!mf)
	{
		eprompt("    Unable to open the file ");
		eprompt(current_name());
		 eprompt(SV_EL);
		return;
	} else
	{
		mf >> m;
		new_box(current_box() | m.box(), 0);
		new_surface(m.set_list().set().surface());
		add_new_set(m.set_list().set(),ask);
	}
}


void add_model()
{
	sv_model m;

	prompt("    model's file name ");
	sv_strcpy(ip, current_name());
	defp(ip);
	rd();
	add_model(ip,1);
}


// Add a set from a file

void add_set()
{
	sv_set s;

	prompt("    set's file name: ");
	rd();
	co("Set from file ");
	co(ip);
	co(" ");
	ifstream sf(ip);
	if(!sf)
	{
		eprompt("    Unable to open the file ");
		eprompt(ip);
		 eprompt(SV_EL);
		return;
	} else
	{
		sf >> s;
		add_new_set(s,1);
	}
}

// Thick or thin?

sv_integer gt(char* s)
{
	sv_strcpy(ip, "    Do you want a thin ");
	sv_strcat(ip, s);
	sv_strcat(ip, "? ");
	sv_integer r = sv_edit_message(ip, 1);
	if(r)
		co("Thin ");
	else
		co("Regular ");
	return(r);
}

// Add menu help

void helpa()
{
	prompt("    ? - print this list"); prompt(SV_EL);
	prompt("    p - add a point"); prompt(SV_EL);
	prompt("    l - add a line"); prompt(SV_EL);
	prompt("    f - add a flat plane"); prompt(SV_EL);
	prompt("    b - add a box cuboid"); prompt(SV_EL);
	prompt("    c - add a cylinder"); prompt(SV_EL);
	prompt("    s - add a sphere"); prompt(SV_EL);
	prompt("    t - add a tetrahedron"); prompt(SV_EL);
	prompt("    > - add a cone"); prompt(SV_EL);
	prompt("    o - add a torus"); prompt(SV_EL);
	prompt("    % - add a cyclide"); prompt(SV_EL);
	prompt("    m - add a model from a file"); prompt(SV_EL);
	prompt("    z - add a set from a file"); prompt(SV_EL);
	prompt("    r - return to the main menu"); prompt(SV_EL);
}

// Add something to the model

void add()
{
	char a;
	do
	{
		prompt("  Add (ENTER for list): ");
		ip_0();
		rdl();
		switch(a = ip[0])
		{
		case '?': helpa(); break;
		case 'p': add_point(); break;
		case 'l': add_line(); break;
		case 'f': add_plane(gt("plane")); break;
		case 'b': add_cuboid(); break;
		case 'c': add_cylinder(gt("cylinder")); break;
		case 's': add_sphere(gt("sphere")); break;
		case 't': add_tetrahedron(); break;
		case '>': add_cone(gt("cone")); break;
		case 'o': add_torus(gt("torus")); break;
		case '%': add_cyclide(gt("cyclide")); break;
		case 'm': add_model(); break;
		case 'z': add_set(); break;
		case 'r': break;
		default:
			prompt("  Valid commands are:"); prompt(SV_EL);
			helpa(); 
		}
	}while(a != 'r');
}


// Write the model to a file

void save_model(const sv_model& mm)
{
	sv_model m = mm;
	sv_set_list sl, sl1;
	if(djf)
	{
		sl = m.set_list().set().list_products();
		m = sv_model(sl, m.box(), sv_model());
	}
	prompt("    name for model ");
	ip[0] = 0;
	defp(ip);
	rd();
	if(ip[0])
	{
	  sl = m.set_list();
	  while(sl.exists())
	  {
		sl1 = merge(sl.set().name(ip).percolate(), sl1);
		sl = sl.next();
	  }
	  m = sv_model(sl1, m.box(), sv_model());
	}
	prompt("    model's file name ");
	sv_strcpy(ip, current_name());
	defp(ip);
	rd();
	new_name(ip);
	co("Model saved to file ");
	ofstream mf(current_name());
	if(!mf)
	{
		eprompt("    Unable to open the file ");
		eprompt(current_name());
		 eprompt(SV_EL);
		return;
	} else
		mf << m;
	co(current_name());
	con();
}


// Switch clever pruning on/off

void clever_prune()
{
	if(c_prune)
	{
		c_prune = 0;
		cprompt("    Regularized pruning off."); cprompt(SV_EL);
	} else
	{
		c_prune = 1;
		cprompt("    Regularized pruning on."); cprompt(SV_EL);
	}
	regular_prune(c_prune);
	box_redivide_draw();
}


// Switch prettyprinting on/off

void pp_output()
{
	if(pretty)
	{
		pretty = 0;
		cprompt("    File output will be compact."); cprompt(SV_EL);
	} else
	{
		pretty = 1;
		cprompt("    File output will be pretty-printed."); cprompt(SV_EL);
	}
	pretty_print(pretty);
}

// Switch disjunctive form on/off

void djf_output()
{
	if(djf)
	{
		djf = 0;
		cprompt("    File output will not be in disjunctive form."); cprompt(SV_EL);
	} else
	{
		djf = 1;
		cprompt("    File output will be in disjunctive form."); cprompt(SV_EL);
	}
}

#if 0
// Switch between using the mouse and typing for rays

void change_mouse()
{
	if(mouse)
	{
		mouse = 0;
		cprompt("    Ray data will be typed."); cprompt(SV_EL);
	} else
	{
		mouse = 1;
		cprompt("    Rays will be derived from the mouse."); cprompt(SV_EL);
	}

	cprompt("    Sorry.  Not available under OpenGL."); cprompt(SV_EL);
}
#endif

// Change the model's box

void change_box()
{
	sv_box b = current_box();
	sv_point l = b.corner(0);
	sv_point h = b.corner(7);
	co("Model box set to: ");
	prompt("    new bottom corner ");
	int i = def_point(l);
	co(", ");
	prompt("    new top corner ");
	int j = def_point(h);
	if(i || j)
	{
	   b = sv_box(l,h);
	   new_box(b,1);
	   con();
	} else
	   co(" (no change)."); co(SV_EL);
}


// Change the low contents value (nasty real/integer conversion...)

void change_low()
{
	sv_real lw = (sv_real)get_low();
	prompt("    new low contents ");
	co("Low box contents set to ");
	if(def_real(lw))
	{
	   set_low(round(lw));
	   box_redivide_draw();
	   co("."); co(SV_EL);
	} else
	   co(" (no change)."); co(SV_EL);
}

// Change the littlest box size

void change_little()
{
	sv_real f = get_little();
	prompt("    new little box size ");
	co("Small box size set to ");
	if(def_real(f))
	{
	   set_little(f);
	   box_redivide_draw();
	   con();
	} else
	   co(" (no change)."); co(SV_EL);
}

// Shift the model by a point

void shift()
{
	sv_point p;
	sv_real r;
	current_sphere(&p, &r);
	co("Translating model by ");
	prompt("    offset ");
	def_point(p);
	co(""); co(SV_EL);
	model_change(current_model().set_list().set() + p);
}

// Rotate the model by a point

void spin()
{
	sv_point p = SV_OO;
	co("Rotating model by ");
	prompt("    axis spins (degrees) ");
	def_point(p);
	co(""); co(SV_EL);
	p.x = p.x*M_PI/180;
	p.y = p.y*M_PI/180;
	p.z = p.z*M_PI/180;
	sv_set s = current_model().set_list().set();
	s = s.spin(SV_XL, p.x);
	s = s.spin(SV_YL, p.y);
	s = s.spin(SV_ZL, p.z);
	model_change(s);
}

// Change-menu help

void helpc()
{
	prompt("    ? - print this list"); prompt(SV_EL);
	prompt("    t - translate the model"); prompt(SV_EL);
	prompt("    s - rotate (spin) the model"); prompt(SV_EL);
	prompt("    p - prettyprint or don't-prettyprint output files"); prompt(SV_EL);
	prompt("    d - output files in/not-in disjunctive form"); prompt(SV_EL);
	// prompt("    c - clever pruning on or off"); prompt(SV_EL);
	// prompt("    m - mouse/type ray details"); prompt(SV_EL);
	prompt("    b - change the model's box"); prompt(SV_EL);
	prompt("    x - change the maximum divided box contents"); prompt(SV_EL);
	prompt("    l - change the littlest divided box"); prompt(SV_EL);
	prompt("    r - return to the main menu"); prompt(SV_EL);
}

// Change something

void change()
{
	char a;

	do
	{
		prompt("  Change (ENTER for list): ");
		ip_0();
		rdl();
		switch(a = ip[0])
		{
		case '?': helpc(); break;
		case 't': shift(); break;
		case 's': spin(); break;
		case 'p': pp_output(); break;
		case 'd': djf_output(); break;
		// case 'c': clever_prune(); break;
		case 'b': change_box(); break;
		// case 'm': change_mouse(); break;
		case 'x': change_low(); break;
		case 'l': change_little(); break;
		case 'r': break;
		default:
			prompt("  Valid commands are:"); prompt(SV_EL);
			helpc(); 
		}
	}while(a != 'r');
}

// Trace a ray into the model

void trace()
{
	sv_line ray;
	if(mouse)
	{
		co("Ray from the mouse ");
		ray = get_ray();
	} else
	{
		co("Ray ");
		ray = get_line("ray");
	}
	co(" cast into the model."); co(SV_EL);
	sv_real t;
	sv_set s = raytrace(ray, &t);

// Print an answer even in batch mode

	sv_integer ptemp = prp;
	prp = 1;
	if(!s.exists())
	{
		cprompt("    The ray missed."); cprompt(SV_EL);
		prp = ptemp;
		return;
	}

	sv_point hit;
	sv_point col;
	sv_primitive p;
	sv_integer k;
	sv_real r0, r1, r2;
	sv_plane f; 
        sv_point cen;
	sv_line axis;

	cprompt(SV_EL); cprompt("    The ray hit at the point ");
	hit = line_point(ray, t);
	p_to_str(ip, hit);
	cprompt(ip);
	cprompt("."); cprompt(SV_EL);
	current_sphere(&cen, &r0);
	new_sphere(hit,r0);
	new_cyl(ray,r0);
	cprompt("    The colour at that point is ");
	col = s.colour();
	p_to_str(ip, col);
	cprompt(ip);
	cprompt("."); cprompt(SV_EL);
//	new_colour(col);
	p = s.primitive();
	cprompt("    The primitive is ");
	switch(p.parameters(&k, &r0, &r1, &r2, &f, &cen, &axis))
	{
	case SV_PLUS:	cprompt("ordinary."); cprompt(SV_EL); break;
	case SV_COMP:	cprompt("complemented."); cprompt(SV_EL); break;
	case SV_TIMES:	cprompt(" a scaled plane (the factor is ");
			r_to_str(ip, r0);
			cprompt(ip);
			cprompt(")."); cprompt(SV_EL);
			break;
	case SV_ABS:	cprompt("thin."); cprompt(SV_EL); break;
	case SV_SSQRT:	cprompt("signed-square-rooted."); cprompt(SV_EL); break;
	case SV_SIGN:   cprompt("signed."); cprompt(SV_EL); break;
	default:
		svlis_error("trace()","dud primitive operator",
			SV_CORRUPT);
	}
	cprompt("    It is a ");
	switch(k)
	{
        case SV_REAL:
		cprompt("real, value ");
		r_to_str(ip, r0);
		cprompt(ip);
		cprompt(" (this is rather a surprise...)."); cprompt(SV_EL);
		break;

        case SV_PLANE:
		cprompt("plane."); cprompt(SV_EL);
		cprompt("    Its normal is ");
		p_to_str(ip, f.normal);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its D term is ");
		r_to_str(ip, f.d);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		break;

        case SV_CYLINDER:
		cprompt("cylinder."); cprompt(SV_EL);
		cprompt("    Its axis direction is ");
		p_to_str(ip, axis.direction);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its axis origin is ");
		p_to_str(ip, axis.origin);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its radius is ");
		r_to_str(ip, r0);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		new_cyl(ray, r0);
		break;

        case SV_SPHERE:
		cprompt("sphere."); cprompt(SV_EL);
		cprompt("    Its centre is ");
		p_to_str(ip, cen);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its radius is ");
		r_to_str(ip, r0);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		new_sphere(hit, r0);  // NB hit not cen
		break;

        case SV_CONE:
		cprompt("cone."); cprompt(SV_EL);
		cprompt("    Its axis direction is ");
		p_to_str(ip, axis.direction);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its axis origin is ");
		p_to_str(ip, axis.origin);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its included angle is ");
		r0 = r0*180/M_PI;
		r_to_str(ip, r0);
		cprompt(ip);
		cprompt(" degrees."); cprompt(SV_EL);
		// new_cone(axis, r0);
		break;

        case SV_TORUS:
		cprompt("torus."); cprompt(SV_EL);
		cprompt("    Its axis direction is ");
		p_to_str(ip, axis.direction);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its centre is ");
		p_to_str(ip, axis.origin);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its major radius is ");
		r_to_str(ip, r0);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its minor radius is ");
		r_to_str(ip, r1);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		// new_torus(axis, r0, r1);
		break;

        case SV_CYCLIDE:
		cprompt("cyclide."); cprompt(SV_EL);
		cprompt("    Its axis direction is ");
		p_to_str(ip, axis.direction);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its centre is ");
		p_to_str(ip, axis.origin);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its major radius is ");
		r_to_str(ip, r0);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its minor radius is ");
		r_to_str(ip, r1);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		cprompt("    Its radial range (+/-) is ");
		r_to_str(ip, r2);
		cprompt(ip);
		cprompt("."); cprompt(SV_EL);
		// new_torus(axis, r0, r1);
		break;


        case SV_GENERAL:
		cprompt("general primitive."); cprompt(SV_EL);
		break;

	default:
		cprompt("user primitive."); cprompt(SV_EL);
	}
	 cprompt(SV_EL);
	prp = ptemp;
}

// Membership test a point

void member()
{
// Print an answer even in batch mode

	sv_integer ptemp = prp;
	prp = 1;
	sv_point p;
	sv_real r;
	current_sphere(&p, &r);
	co("Membership testing point ");
	prompt("    point coordinates ");
	def_point(p);
	co(" against the model."); co(SV_EL);
	sv_model m = current_divided_model();
	sv_primitive ks;
	cprompt("    The point is ");
	switch(m.member(p, &ks))
	{
	case SV_SOLID: cprompt("inside."); cprompt(SV_EL); break;
	case SV_SURFACE: cprompt("on the surface."); cprompt(SV_EL); break;
	case SV_AIR: cprompt("outside."); cprompt(SV_EL); break;
	default:
		svlis_error("member","dud value returned",
			SV_CORRUPT);
	}
	prp = ptemp;
}

// Mass properties

void mass_p()
{
	sv_real vol;
	sv_point centroid, mxyz, nxyz;
	m_vol(0.01, vol, centroid, mxyz, nxyz);
	sv_integer ptemp = prp;
	prp = 1;
	co("Finding the model's mass properties."); co(SV_EL);
	cprompt("    The model's volume is ");
	r_to_str(ip, vol);
	cprompt(ip);
	cprompt(" cubic units."); cprompt(SV_EL);
	cprompt("    The centroid is at the point ");
	p_to_str(ip, centroid);
	cprompt(ip);
	cprompt("."); cprompt(SV_EL);
	cprompt("    The second moments about X, Y, and Z through the centriod are ");
	p_to_str(ip, mxyz);
	cprompt(ip);
	cprompt("."); cprompt(SV_EL);
	cprompt("    The product moments through the centriod are ");
	p_to_str(ip, nxyz);
	cprompt(ip);
	cprompt("."); cprompt(SV_EL);
	prp = ptemp;
}


// Surface area

void area()
{
	sv_real a = m_area();
	sv_integer ptemp = prp;
	prp = 1;
	co("Finding the model's surface area."); co(SV_EL);
	cprompt("    The model's surface area is ");
	r_to_str(ip, a);
	cprompt(ip);
	cprompt(" square units."); cprompt(SV_EL);
	prp = ptemp;
}

// Smallest enclosing box

void sm_box()
{
	sv_model m = current_model();
	sv_box b = m.tight_fit();
	sv_integer ptemp = prp;
	prp = 1;
	co("Finding the model's enclosing box."); co(SV_EL);
	cprompt("    The model's enclosing box is ");
	sv_point p = b.corner(0);
	p_to_str(ip, p);
	cprompt(ip);
	cprompt(", ");
	p = b.corner(7);
	p_to_str(ip, p);
	cprompt(ip);
	 cprompt(SV_EL);	
	prp = ptemp;
}


// Enquire of the model

void helpq()
{
	prompt("    ? - print this list"); prompt(SV_EL);
	prompt("    t - trace a ray into the model and print what it hits"); prompt(SV_EL);
	prompt("    x - membership test a point."); prompt(SV_EL);
	prompt("    p - print division statistics"); prompt(SV_EL);
	prompt("    m - the model's mass properties"); prompt(SV_EL);
	prompt("    a - the model's surface area"); prompt(SV_EL);
	prompt("    b - the set's smallest enclosing box"); prompt(SV_EL);
	prompt("    r - return to the main menu"); prompt(SV_EL);
}

void query()
{
	char a;

	do
	{
		prompt("  Enquire (ENTER for list): ");
		ip_0();
		rdl();
		switch(a = ip[0])
		{
		case '?': helpq(); break;
		case 't': trace(); break;
		case 'x': member(); break;
		case 'p': printstats(current_name(), cout); break;
		case 'm': mass_p(); break;
		case 'a': area(); break;
		case 'b': sm_box(); break;
		case 'r': break;
		default:
			prompt("  Valid commands are:"); prompt(SV_EL);
			helpq(); 
		}
	}while(a != 'r');
}


void picture_size()
{
   sv_integer w;
   sv_integer h;
   char temp[STLEN];

   prompt("    picture size ");
   r_to_str(ip, (sv_real)get_pic_x());
   sv_strcat(ip,", ");
   r_to_str(temp, (sv_real)get_pic_y());
   sv_strcat(ip,temp);
   defp(ip);
   rd();
   istrstream ist(ip, STLEN);
   ist >> w;
   ist >> h;
   set_pic_resolution(w,h);
}



void ambient_light()
{
	sv_point col = sv_point(1,1,1);
	sv_real intensity = 0.4;

	cprompt("    ambient light colour ");
	def_point(col);
        con();
	cprompt("    ambient light intensity ");
	def_real(intensity);
	con();
	set_ambient_light_level(col, intensity);
}


void light_source()
{
	sv_point loc, col, dir;
	sv_real intensity;
	light_type type;
	char name[STLEN];
	sv_real p;
	sv_light_list *l = get_lamp_list();
	sv_light_list *ll = l;
	

	prompt("    The lights defined are:"); prompt(SV_EL); prompt("      ");
	while(ll)
	{
		l = ll;
		prompt(l->name);
		prompt(" ");
		ll = ll->next;
	}
	 prompt(SV_EL);
	sv_strcpy(ip, l->name);

	prompt("    name of the light to add or change ");
	defp(ip);
	rd();
	sv_strcpy(name,ip);
        co("Changing light ");
        co(name);

	l = get_lamp_list();
        if(l) while( sv_strcmp(l->name,name) && (l->next) ) l = l->next;

	if(l)
	{
		type = l->source->type();
		loc = l->source->location();
		col = l->source->colour().norm();
                intensity = l->source->colour().mod();
		dir = l->source->direction();
   		p = l->source->angle_power();
                con();
	} else
	{
	     type = POINT_SOURCE;
	     loc = sv_point(0,0,current_box().zi.hi() + 100*(current_box().zi.hi() - current_box().zi.lo()));
	     col = sv_point(1,1,1); 
	     intensity = 1;
	     dir = -SV_Z;
	     p = 1;
             co(" (new light)."); co(SV_EL);
	}

	cprompt("    light's location ");
	def_point(loc);
	con();

	cprompt("    light's colour ");
	def_point(col);
	con();
	cprompt("    light's intensity ");
	def_real(intensity);
	con();

	cprompt("    light's direction ");
	def_point(dir);
	con();

	cprompt("    light's angle power ");
	def_real(p);
	con();

	cprompt("    light's type ");
	switch(type)
	{
	case PARALLEL:
		sv_strcpy(ip,"parallel or `po' for point source");
		break;

	case POINT_SOURCE:
		sv_strcpy(ip,"point source or `pa' for parallel");
		break;

	default:
		svlis_error("do_light_source", "dud light type", SV_CORRUPT);
	}
	defp(ip);
	rdl();
	if(ip[1] == 'o')
	{
		type = POINT_SOURCE;
		co(" point source."); co(SV_EL);
	} else
	{
		type = PARALLEL;
		co(" parallel."); co(SV_EL);
	}
	update_lamplist(name, type, loc, col, intensity, dir, p);
}



void delete_light()
{
	char name[STLEN];
	sv_light_list *l = get_lamp_list();
	sv_light_list *ll = l;
	sv_integer count = 0;

	prompt("    The lights defined are:"); prompt(SV_EL); prompt("      ");
	while(ll)
	{
		count++;
		l = ll;
		prompt(l->name);
		prompt(" ");
		ll = ll->next;
	}
	 prompt(SV_EL);

	if(count <= 1)
	{
		prompt("    You can't delete the last light."); prompt(SV_EL);
		return;
	}

	sv_strcpy(ip, l->name);

	prompt("    name of the light to delete ");
	defp(ip);
	rd();
	co("Deleting light ");
	sv_strcpy(name,ip);
	co(name);
	con();

      l = get_lamp_list();
      ll = 0;
      int no_match = sv_strcmp(l->name,name);
      while(no_match && l->next)
      {
	 ll = l;
	 l = l->next;
	 no_match = sv_strcmp(l->name,name);
      }

      if(!no_match) 
      {
	 // l is entry to delete, ll the one before

	 if(ll == 0)
	    set_lamp_list(l->next);
	 else
	    ll->next = l->next;

	 delete l->source;
	 free(l);
      } else 
      {
	 // No entry with supplied name
	 eprompt("    There is not a light source named `");
	 eprompt(name);
	 eprompt("'"); eprompt(SV_EL);
      }
}


void background_colour()
{
	sv_point bgc = get_background_colour();
	cprompt("    background colour ");
	def_point(bgc);
	con();
   	set_background_colour(bgc.x, bgc.y, bgc.z, 1);
}


void mist()
{
	sv_point c;
	sv_real b;
	sv_real h;

	c = get_mist();
	b = get_back();
	h = get_haze();

	co("Setting mist values."); co(SV_EL);
	cprompt("    mist's colour ");
	def_point(c);
	con();
	cprompt("    mist's distance ");
	def_real(b);
	con();
	cprompt("    haze value ");
	def_real(h);
	con();

	set_mist(c);
	set_back(b);
	set_haze(h);
}


void horizon_colour()
{
	sv_point hc = get_horizon_colour();
	cprompt("    horizon's colour ");
	def_point(hc);
	con();
   	set_horizon_colour(hc.x, hc.y, hc.z, 1);
}


void ground_colour()
{
	sv_point gc = get_ground_colour();
	cprompt("    ground's colour ");
	def_point(gc);
	con();
   	set_ground_colour(gc.x, gc.y, gc.z, 1);
}


void overhead_colour()
{
	sv_point oc = get_overhead_colour();
	cprompt("    overhead colour ");
	def_point(oc);
	con();
   	set_overhead_colour(oc.x, oc.y, oc.z, 1);
}

void picture_name()
{
   cprompt("    new picture file name ");
   sv_strcpy(ip, get_pic_filename());
   defp(ip);
   rd();
   co(ip);
   con();
   set_pic_filename(ip);
}


void quickview()
{
   sv_integer qv = get_quickview();
   if(qv)
   {
	qv = 0;
        cprompt("    The raytraced image will go to the file ");
	cprompt(get_pic_filename());
	cprompt(" only."); cprompt(SV_EL); 
   } else
   {
	qv = 1;
        cprompt("    The raytraced image will go to the file ");
 	cprompt(get_pic_filename());
	cprompt(" and the screen."); cprompt(SV_EL); 
   }
   set_quickview(qv);
}


void report_progress()
{
   sv_real rs = get_report_step();
   prompt("    Report raytracing progress at percentage (-ve to supress) ");
   co("Raytracing progress reports will be at ");
   def_real(rs);
   co("% intervals."); co(SV_EL);
   set_report_step(rs);
}

// Change the view

void set_view()
{

	cprompt("    Changes of view only affect raytracing."); cprompt(SV_EL);

	sv_view v = get_view();
	sv_point p = v.eye_point();
        co("Setting new view."); co(SV_EL);
	cprompt("    new eye position ");
	def_point(p);
	con();
	v.eye_point(p);

	p = v.centre();
	cprompt("    new centre of interest ");
	def_point(p);
	con();
	v.centre(p);

	p = v.up_vector();
	cprompt("    new up direction ");
	def_point(p);
	con();
	v.vertical_dir(p);

	sv_real la;
	la = v.lens_angle()*180.0/M_PI;
	cprompt("    new lens angle ");
	def_real(la);
	con();
	v.lens_angle(la*M_PI/180.0);

	e_view_change(v);
}

void facet_change()
{
	sv_integer f = get_faceting();
	if(f)
	{
		cprompt("    Faceted pictures will not be generated"); cprompt(SV_EL);
		set_faceting(0);
	} else
	{
		cprompt("    Faceted pictures will be generated"); cprompt(SV_EL);
		set_faceting(1);
		box_redivide_draw();
	}
}

void ray_change()
{
	sv_integer f = get_raytracing();
	if(f)
	{
		cprompt("    Raytraced pictures will not be generated"); cprompt(SV_EL);
		set_raytracing(0);
	} else
	{
        	cprompt("    Raytraced pictures will go to the file ");
		cprompt(get_pic_filename());
		 cprompt(SV_EL); 
		set_raytracing(1);
		box_redivide_draw();
	}
}


void helpr()
{
	prompt("      ? - print this list"); prompt(SV_EL);
	prompt("      f - faceted pictures on/off"); prompt(SV_EL);
	prompt("      t - raytraced pictures on/off"); prompt(SV_EL);
	prompt("      v - set the view position variables"); prompt(SV_EL);
	prompt("      a - set ambient light"); prompt(SV_EL);
	prompt("      l - edit or add a light source"); prompt(SV_EL);
	prompt("      d - delete a light source"); prompt(SV_EL);
	prompt("      b - set background colour"); prompt(SV_EL);
	prompt("      m - set mist/fog"); prompt(SV_EL);
	prompt("      h - set horizon colour"); prompt(SV_EL);
	prompt("      g - set ground colour"); prompt(SV_EL);
	prompt("      o - set overhead colour"); prompt(SV_EL);
	prompt("      p - set picture file name"); prompt(SV_EL);
	prompt("      i - set raytraced screen image on/off"); prompt(SV_EL);
	prompt("      s - set progress report step"); prompt(SV_EL);
	prompt("      n - set a new default surface"); prompt(SV_EL);
	prompt("      w - set a new ray-trace plot window size"); prompt(SV_EL);
	prompt("      r - return to the main menu"); prompt(SV_EL);
}

void rayren()
{
	char a;
	sv_surface sf;
	sf = current_surface();

	do
	{
		prompt("    Raytrace picture control (ENTER for list): ");
		ip_0();
		rdl();
		switch(a = ip[0])
		{
		case '?': helpr(); break;
		case 'f': facet_change(); break;
		case 't': ray_change(); break;
		case 'v': set_view(); break;
		case 'a': ambient_light(); break;
		case 'l': light_source(); break;
		case 'd': delete_light(); break;
		case 'b': background_colour(); break;
		case 'm': mist(); break;
		case 'h': horizon_colour(); break;
		case 'g': ground_colour(); break;
		case 'o': overhead_colour(); break;
		case 'p': picture_name(); break;
		case 'i': quickview(); break;
		case 's': report_progress(); break;
		case 'n': get_surface(sf); break;
		case 'w': picture_size(); break;
		case 'r': break;
		default:
			prompt("    Valid commands are:"); prompt(SV_EL);
			helpr(); 
		}
	}while(a != 'r');
	redraw();
}

// Switch logging on or off

void logfile_change()
{
	if(logging)
	{
		logging = !sv_edit_message(
			"  Stop logging and close the log file? ", 1);
		if(!logging)
		{
#if macintosh
			logfile->seekp(-4,(ios::seekdir)1);
#else
			logfile->seekp(-4,(ios::seek_dir)1);
#endif
			*logfile << "q" << SV_EL; 
			stoplog();
		}
	}else
	{
		prompt("  log file name: ");
		ip_0();
		rd();
		logfile = new ofstream(ip);
		co("Logging started to file ");
		co(ip);
		con();
		if(!logfile)
		{
			eprompt("  Unable to open the log file ");
			eprompt(ip);
			 eprompt(SV_EL);
		} else
			logging = 1;
	}
}

// Open a file for batch input

int readfrom(char* s)
{
	isp++;
	if(isp > I_STK)
	{
		svlis_error("readfrom()","input stream stack exhausted",SV_FATAL);
		return(0);
	}
	cip[isp] = new ifstream(s);
	if(!cip[isp])
	{
		sv_strcpy(ip, "Unable to open the input file ");
		sv_strcat(ip, s);
		svlis_error("sv_edit", ip, SV_FATAL);
		return(0);
	} else
		prp = 0;
	co("Taking input from file ");
	co(s);
	con();
	return(1);
}

// include file

void include()
{

	prompt("  include file name: ");
	ip_0();
	rd();
	readfrom(ip);
}

// Switch comentary on

void comon()
{

	prompt("  commentary file name: ");
	ip_0();
	rd();
	cf = new ofstream(ip);
	if(!cf)
	{
		eprompt("  Unable to open the commentary file ");
		eprompt(ip);
		 eprompt(SV_EL);
	} else
		com = 1;
}



// Root menu help

void help0()
{
	prompt("  ? - print this list"); prompt(SV_EL);
	prompt("  a - add a new shape"); prompt(SV_EL);
	prompt("  t - ray-trace control"); prompt(SV_EL);
	prompt("  s - save the model to a file"); prompt(SV_EL);
	prompt("  i - read commands from a file"); prompt(SV_EL);
	prompt("  c - change variables"); prompt(SV_EL);
	prompt("  < - go back; undo the last edit"); prompt(SV_EL);
	prompt("  > - go forward; redo an undone edit"); prompt(SV_EL);
	prompt("  e - enquire about the model"); prompt(SV_EL);
	prompt("  n - start a new model"); prompt(SV_EL);
	prompt("  l - turn the log file on or off"); prompt(SV_EL);
	prompt("  m - turn commentary file on"); prompt(SV_EL);
	prompt("  q - quit svLis edit"); prompt(SV_EL);
}

// Main loop

void edit_loop()
{
	char a;

	do
	{
		prompt("SvLis editor (ENTER for command list): ");
		ip_0();
		rdl();
		switch(a = ip[0])
		{
		case '?': help0(); break;
		case 'a': add(); break;
		case 't': rayren(); break;
		case 'i': include(); break;
		case 's': save_model(model2save()); break;
		case 'c': change(); break;
		case '<': undo(); break;
		case '>': redo(); break;
		case 'e': query(); break;
		case 'n': cleansheet(); break;
		case 'l': logfile_change(); break;
		case 'm': comon(); break;
		case 'q': if( !sv_edit_message("Are you sure? ", 1) )
				a = ' ';
			  break;

		default:
			prompt("Valid commands are:"); prompt(SV_EL);
			help0(); 
		}
	}while(a != 'q');
}


int main(int argc, char* argv[])
{
	sv_box b;
	sv_integer slen;

	svlis_init();
	sv_edit_init();
	set_swell_fac(0.05);
	switch(argc)
	{
	case 1: 
		prp = 1;
		isp = 0;
		cip[isp] = &cin;
		break;
	case 2:
		slen = sv_strlen(argv[1]);

		if((argv[1][slen-3] == 'm') &&
			(argv[1][slen-2] == 'o') &&
			(argv[1][slen-1] == 'd'))
		{
			new_box(b,0);
			add_model(argv[1],0);
			prp = 1;
			isp = 0;
			cip[isp] = &cin;
		} else
		{
			isp = -1;
			if(readfrom(argv[1]))
				prp = 0;
			else
				return(1);
		}
		break;
	default:
		cerr << "Format is: svlis [log_file.log/model_file.mod]" << SV_EL;
		return(2);
	}
	prompt(SV_EL); prompt("SvLis Interactive Model Editor"); prompt(SV_EL); prompt(SV_EL);
	edit_loop();
	sv_edit_windup();
	return(0);
}
#if macintosh
 #pragma export off
#endif





















#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_core.h"
#include "typemap.h"

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int len, int arg)
{
	int return_code;

	errno = 0;
	if (strEQ(name, "CMISS_SCENE_VIEWER_PERSPECTIVE"))
	{
		return_code = CMISS_SCENE_VIEWER_PERSPECTIVE;
	}
	else if (strEQ(name, "CMISS_SCENE_VIEWER_PARALLEL"))
	{
		return_code = CMISS_SCENE_VIEWER_PARALLEL;
	}
	else if (strEQ(name, "CMISS_SCENE_VIEWER_FAST_TRANSPARENCY"))
	{
		return_code = CMISS_SCENE_VIEWER_FAST_TRANSPARENCY;
	}
	else if (strEQ(name, "CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY"))
	{
		return_code = CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY;
	}
	else if (strEQ(name, "CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY"))
	{
		return_code = CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY;
	}	
	else
	{
    	errno = ENOENT;
		return_code = 0;
	}
   return (return_code);
}

MODULE = Cmiss::scene_viewer		PACKAGE = Cmiss::scene_viewer		PREFIX = Cmiss_scene_viewer_

PROTOTYPES: DISABLE

double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL

# The return code is not passed back here, it could be used to generate a die.
# Instead each of the returned parameters is appended to the returned list
NO_OUTPUT int
Cmiss_scene_viewer_get_lookat_parameters(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST double eyex, OUTLIST double eyey, OUTLIST double eyez, \
	OUTLIST double lookatx, OUTLIST double lookaty, OUTLIST double lookatz, \
	OUTLIST double upx, OUTLIST double upy, OUTLIST double upz)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_lookat_parameters_non_skew( \
	Cmiss::scene_viewer scene_viewer, double eyex, double eyey, double eyez, \
	double lookatx, double lookaty, double lookatz, double upx, double upy, \
	double upz)

NO_OUTPUT int
Cmiss_scene_viewer_get_projection_mode(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST enum Cmiss_scene_viewer_projection_mode projection_mode)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_projection_mode(Cmiss::scene_viewer scene_viewer, \
	enum Cmiss_scene_viewer_projection_mode projection_mode)

NO_OUTPUT int
Cmiss_scene_viewer_get_transparency_mode(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST enum Cmiss_scene_viewer_transparency_mode transparency_mode)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_transparency_mode(Cmiss::scene_viewer scene_viewer, \
	enum Cmiss_scene_viewer_transparency_mode transparency_mode)

NO_OUTPUT int
Cmiss_scene_viewer_get_transparency_layers( \
	IN Cmiss::scene_viewer scene_viewer, OUTLIST int transparency_layers)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_transparency_layers(Cmiss::scene_viewer scene_viewer, \
	int transparency_layers)

NO_OUTPUT int
Cmiss_scene_viewer_get_view_angle(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST double view_angle)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_view_angle(Cmiss::scene_viewer scene_viewer, \
	double view_angle)

NO_OUTPUT int
Cmiss_scene_viewer_get_near_and_far_plane(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST double near, OUTLIST double far)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_near_and_far_plane(Cmiss::scene_viewer scene_viewer, \
	double near, double far)

NO_OUTPUT int
Cmiss_scene_viewer_get_antialias_mode(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST int antialias_mode)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_antialias_mode(Cmiss::scene_viewer scene_viewer, \
	int antialias_mode)

NO_OUTPUT int
Cmiss_scene_viewer_get_perturb_lines(IN Cmiss::scene_viewer scene_viewer, \
	OUTLIST int perturb_lines)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_perturb_lines(Cmiss::scene_viewer scene_viewer, \
	int perturb_lines)

NO_OUTPUT int
Cmiss_scene_viewer_get_background_colour_rgb( \
	IN Cmiss::scene_viewer scene_viewer, OUTLIST double red, \
	OUTLIST double green, OUTLIST double blue)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_scene_viewer_set_background_colour_rgb(Cmiss::scene_viewer scene_viewer, \
	double red, double green, double blue)

char *
Cmiss_scene_viewer_get_interactive_tool_name(Cmiss::scene_viewer scene_viewer)
	PPCODE:
		Cmiss_scene_viewer_get_interactive_tool_name(scene_viewer, &RETVAL);
		EXTEND(SP, 1);
		PUSHs(sv_2mortal(newSVpv(RETVAL, 0)));
		Cmiss_deallocate(RETVAL);

int
Cmiss_scene_viewer_set_interactive_tool_by_name( \
	Cmiss::scene_viewer scene_viewer, char *tool_name)

char *
Cmiss_scene_viewer_get_scene_name(Cmiss::scene_viewer scene_viewer)
	PPCODE:
		Cmiss_scene_viewer_get_scene_name(scene_viewer, &RETVAL);
		EXTEND(SP, 1);
		PUSHs(sv_2mortal(newSVpv(RETVAL, 0)));
		Cmiss_deallocate(RETVAL);

int
Cmiss_scene_viewer_set_scene_by_name(Cmiss::scene_viewer scene_viewer, \
	char *scene_name)

int
Cmiss_scene_viewer_view_all(Cmiss::scene_viewer scene_viewer)

int
Cmiss_scene_viewer_redraw_now(Cmiss::scene_viewer scene_viewer)

int
Cmiss_scene_viewer_write_image_to_file(Cmiss::scene_viewer scene_viewer, \
	char *file_name, int force_onscreen = 0, int preferred_width = 0, \
	int preferred_height = 0, int preferred_antialias = 0, \
	int preferred_transparency_layers = 0)

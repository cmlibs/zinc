#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <api/cmiss_graphics_window.h>
#include <perl/Cmiss/Scene_viewer/typemap.h>

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

MODULE = Cmiss::Graphics_window		PACKAGE = Cmiss::Graphics_window		PREFIX = Cmiss_graphics_window_


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

NO_OUTPUT int
Cmiss_graphics_window_get_scene_viewer_by_name(IN char *graphics_window_name, \
	IN int pane_number, OUTLIST Cmiss::Scene_viewer scene_viewer)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

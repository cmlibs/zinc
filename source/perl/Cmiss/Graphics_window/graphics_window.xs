#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <api/cmiss_graphics_window.h>

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

MODULE = Cmiss::graphics_window		PACKAGE = Cmiss::graphics_window		PREFIX = Cmiss_graphics_window_


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
	IN int pane_number, OUTLIST Cmiss_scene_viewer_id scene_viewer_id)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

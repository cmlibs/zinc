#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_core.h"
#include "gtk/gtk_cmiss_scene_viewer.h"

#include "PerlGtkCmissSceneViewerInt.h"

#include "perl/Cmiss/scene_viewer/typemap.h"
#include "GtkDefs.h"
#include "GtkCmissSceneViewerDefs.h"

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

	errno = ENOENT;
	return_code = 0;
   return (return_code);
}

MODULE = Cmiss::Gtk::SceneViewer		PACKAGE = Cmiss::Gtk::SceneViewer		PREFIX = gtk_cmiss_scene_viewer_

PROTOTYPES: DISABLE

void
init(Class)
	SV *	Class
	CODE:
	{
		static int did_it = 0;
		if (did_it)
			return;
		did_it = 1;
		GtkCmissSceneViewer_InstallTypedefs();
		GtkCmissSceneViewer_InstallObjects();
	}

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

#ifdef GTK_DRAWING_AREA

Cmiss::Gtk::SceneViewer_Sink
new(Class)
	SV *	Class
	CODE:
	RETVAL = (GtkCmissSceneViewer *)(gtk_cmiss_scene_viewer_new());
	OUTPUT:
	RETVAL

Cmiss::scene_viewer
gtk_cmiss_scene_viewer_get_cmiss_scene_viewer(gtk_cmiss_scene_viewer)
	Cmiss::Gtk::SceneViewer gtk_cmiss_scene_viewer

#endif

/*******************************************************************************
FILE : mapping_work_area.h

LAST MODIFIED : 16 September 1992

DESCRIPTION :
==============================================================================*/
#if !defined (MAPPING_WORK_AREA_H)
#define MAPPING_WORK_AREA_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "rig.h"
#include "mapping_window.h"

/*
Global types
============
*/
struct Mapping_work_area
/*******************************************************************************
LAST MODIFIED : 16 September 1992

DESCRIPTION :
==============================================================================*/
{
	Widget *activation,outer_form,*parent,window_shell;
	enum Mapping_associate associate;
	struct Mapping_window *mapping_window;
	char open;
};
#endif

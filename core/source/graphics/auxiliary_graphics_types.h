/*******************************************************************************
FILE : auxiliary_graphics_types.h

LAST MODIFIED : 12 March 2002

DESCRIPTION :
Structures and enumerated types needed to produce graphics primitives but not
specific to any of them. Examples are:
- struct Element_discretization: stores the number of segments used to
represent curves in three xi-directions;
- Triple;
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (AUXILIARY_GRAPHICS_TYPES_H)
#define AUXILIARY_GRAPHICS_TYPES_H

#include "zinc/zincconfigure.h"
#include "zinc/types/graphicsid.h"

#include "general/enumerator.h"
//#if defined (WIN32_SYSTEM)
//#define NOMINMAX
//#include <windows.h>
//#endif /* defined (WIN32_SYSTEM) || defined (CYGWIN) */
#if defined (USE_GLEW)
#include <GL/glew.h>
#endif
/*
Global types
------------
*/

struct Element_discretization
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Stores the number of segment used in drawing each side of an element.
Controls quality of curve scene.
==============================================================================*/
{
	int number_in_xi1,number_in_xi2,number_in_xi3;
}; /* struct Element_discretization */

typedef GLfloat Triple[3];

enum Graphics_compile_status
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Graphical objects use this enumeration to flag their compile status, which
can indicate either no compilation is required, child graphics objects need
compiling, or the object and its children need compilation.
Currently applied only to OpenGL display lists, but relevant to other
intermediate storage systems.
==============================================================================*/
{
	GRAPHICS_COMPILED,
	CHILD_GRAPHICS_NOT_COMPILED,
	GRAPHICS_NOT_COMPILED
}; /* enum Graphics_compile_status */

enum Streamline_data_type
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	STREAM_DATA_INVALID = 0,
	STREAM_NO_DATA = 1,          /* The code relies on NODATA being zero as the
												  field creation types test if(data_type) */
	STREAM_FIELD_SCALAR = 2,     /* Generalised scalar as in other graphics objects */
	STREAM_MAGNITUDE_SCALAR = 3, /* The vector is necessarily calculated already
												  so can save computation by no using the scalar */
	STREAM_TRAVEL_SCALAR = 4     /* Integrate time along the curve so that you can
												  see how long each bit is */
};

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_graphics_select_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_graphics_render_polygon_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_graphicslineattributes_shape_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_graphics_streamlines_track_direction);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Streamline_data_type);

int check_Circle_discretization(int *circle_discretization);

int check_Element_discretization(struct Element_discretization
	*element_discretization);

#endif /* AUXILIARY_GRAPHICS_TYPES_H */

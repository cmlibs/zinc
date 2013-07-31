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
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (AUXILIARY_GRAPHICS_TYPES_H)
#define AUXILIARY_GRAPHICS_TYPES_H

#include "zinc/zincconfigure.h"
#include "zinc/types/graphicid.h"
#include "zinc/types/graphicsrendertype.h"

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

enum Graphics_select_mode
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Enumerator for specifying which parts of a graphic are named for selecting, if
any, and how they should be rendered depending on their selection status.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	GRAPHICS_SELECT_MODE_INVALID = 0,
	GRAPHICS_SELECT_ON = 1,
	GRAPHICS_NO_SELECT = 2,
	GRAPHICS_DRAW_SELECTED = 3,
	GRAPHICS_DRAW_UNSELECTED = 4
}; /* enum Graphics_select_mode */

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

PROTOTYPE_ENUMERATOR_FUNCTIONS(Graphics_select_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphics_render_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphic_line_attributes_shape);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphic_streamlines_track_direction);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Streamline_data_type);

int check_Circle_discretization(int *circle_discretization);

int check_Element_discretization(struct Element_discretization
	*element_discretization);

#endif /* AUXILIARY_GRAPHICS_TYPES_H */

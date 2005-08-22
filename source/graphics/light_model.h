/*******************************************************************************
FILE : light_model.h

LAST MODIFIED : 21 September 1998

DESCRIPTION :
The data structures used for representing light models.
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
#if !defined (LIGHT_MODEL_H)
#define LIGHT_MODEL_H

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/graphics_library.h"

/*
Global types
------------
*/
enum Light_model_viewer_mode
/*******************************************************************************
Local viewer lighting calculations take into account the angle from the
vertex to the viewer and are more accurate with some speed penalty.
Using an infinite viewer is the faster - and should be the default - option.
==============================================================================*/
{
	LIGHT_MODEL_LOCAL_VIEWER,
	LIGHT_MODEL_INFINITE_VIEWER
};

enum Light_model_side_mode
/*******************************************************************************
Two sided lighting lights the backs of the surfaces the same as the front;
One sided lighting does not.
==============================================================================*/
{
	LIGHT_MODEL_ONE_SIDED,
	LIGHT_MODEL_TWO_SIDED
};

struct Light_model;
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Light_model);

DECLARE_MANAGER_TYPES(Light_model);

struct Modify_light_model_data
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
==============================================================================*/
{
	struct Light_model *default_light_model;
	struct MANAGER(Light_model) *light_model_manager;
}; /* struct Modify_light_model_data */

/*
Global functions
----------------
*/
struct Light_model *CREATE(Light_model)(char *name);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Allocates memory and assigns fields for a light model.  Adds the light model to
the list of all light models.
==============================================================================*/

int DESTROY(Light_model)(struct Light_model **light_model_address);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the light model and sets <*light_model_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Light_model);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Light_model);

PROTOTYPE_LIST_FUNCTIONS(Light_model);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Light_model,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Light_model,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Light_model);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Light_model,name,char *);

int Light_model_get_ambient(struct Light_model *light_model,
	struct Colour *ambient);
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the ambient colour of the light_model.
==============================================================================*/

int Light_model_set_ambient(struct Light_model *light_model,
	struct Colour *ambient);
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the ambient colour of the light_model.
==============================================================================*/

int Light_model_get_lighting_status(struct Light_model *light_model);
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Returns true if lighting is on for <light_model>.
==============================================================================*/

int Light_model_set_lighting_status(struct Light_model *light_model,
	int lighting_on);
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Sets whether lighting is on in <light_model>. On if <lighting_on> non-zero.
==============================================================================*/

int Light_model_get_side_mode(struct Light_model *light_model,
	enum Light_model_side_mode *side_mode);
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the side_mode of the light_model (local/infinite).
==============================================================================*/

int Light_model_set_side_mode(struct Light_model *light_model,
	enum Light_model_side_mode side_mode);
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the side_mode of the light_model (local/infinite).
==============================================================================*/

int Light_model_get_viewer_mode(struct Light_model *light_model,
	enum Light_model_viewer_mode *viewer_mode);
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the viewer_mode of the light_model (local/infinite).
==============================================================================*/

int Light_model_set_viewer_mode(struct Light_model *light_model,
	enum Light_model_viewer_mode viewer_mode);
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the viewer_mode of the light_model (local/infinite).
==============================================================================*/

int modify_Light_model(struct Parse_state *parse_state,void *light_model_void,
	void *modify_light_model_data_void);
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
Modifies the properties of a light model.
==============================================================================*/

int list_Light_model(struct Light_model *light_model,void *dummy);
/*****************************************************************************
LAST MODIFIED : 27 July 1995

DESCRIPTION :
Write the properties of the <light_model> to the command window.
============================================================================*/

int set_Light_model(struct Parse_state *state,
	void *light_model_address_void,void *light_model_manager_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Modifier function to set the light model from a command.
==============================================================================*/

int compile_Light_model(struct Light_model *light_model);
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Rebuilds the display_list for <light_model> if it is not current. If
<light_model>does not have a display list, first attempts to give it one. The
display list created here may be called using execute_Light_model, below.
???RC Light_models must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of light_models is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Light_model should just call direct_render_Light_model.
==============================================================================*/

int execute_Light_model(struct Light_model *light_model);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Activates <light_model> by calling its display list. If the display list is not
current, an error is reported.
???RC The behaviour of light_models is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Light_model.
==============================================================================*/
#endif

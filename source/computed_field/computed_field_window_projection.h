/*******************************************************************************
FILE : computed_field_window_projection.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_WINDOW_PROJECTION_H)
#define COMPUTED_FIELD_WINDOW_PROJECTION_H

#include "graphics/scene_viewer.h"

enum Computed_field_window_projection_type
{
	NDC_PROJECTION,
	TEXTURE_PROJECTION,
	VIEWPORT_PROJECTION,
	INVERSE_NDC_PROJECTION,
	INVERSE_TEXTURE_PROJECTION,
	INVERSE_VIEWPORT_PROJECTION
};

struct MANAGER(Graphics_window);

int Computed_field_set_type_window_projection(struct Computed_field *field,
	struct Computed_field *source_field, struct Scene_viewer *scene_viewer,
	char *graphics_window_name, int pane_number,
	enum Computed_field_window_projection_type projection_type);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_WINDOW_PROJECTION, returning the 
<source_field> with each component multiplied by the perspective transformation
of the <scene_viewer>.  The <graphics_window_name> and <pane_number> are stored
so that the command to reproduce this field can be written out.
The manager for <field> is notified if the <scene_viewer> closes.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_register_type_window_projection(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Graphics_window) *graphics_window_manager);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_WINDOW_PROJECTION_H) */

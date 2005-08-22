/*******************************************************************************
FILE : scene_editor.h

LAST MODIFIED : 3 March 2003

DESCRIPTION :
Widgets for editing scene, esp. changing visibility of members.
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
#if !defined (SCENE_EDITOR_H)
#define SCENE_EDITOR_H

#include "graphics/scene.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Scene_editor;

/*
Global functions
----------------
*/

struct Scene_editor *CREATE(Scene_editor)(
	struct Scene_editor **scene_editor_address, Widget parent,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Note on successful return the dialog is put at <*scene_editor_address>.
==============================================================================*/

int DESTROY(Scene_editor)(struct Scene_editor **scene_editor_address);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
==============================================================================*/

int Scene_editor_bring_to_front(struct Scene_editor *scene_editor);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
De-iconifies and brings the scene editor to the front.
==============================================================================*/

struct Scene *Scene_editor_get_scene(struct Scene_editor *scene_editor);
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Returns the root scene of the <scene_editor>.
==============================================================================*/

int Scene_editor_set_scene(struct Scene_editor *scene_editor,
	struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Sets the root scene of the <scene_editor>. Updates widgets.
==============================================================================*/

#endif /* !defined (SCENE_EDITOR_H) */

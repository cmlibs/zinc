/*******************************************************************************
FILE : scene_editor.h

LAST MODIFIED : 26 Febuary 2007

DESCRIPTION :
Widgets for editing scene, esp. changing visibility of members.
==============================================================================*/

#if !defined (SCENE_EDITOR_WX_H)
#define SCENE_EDITOR_WX_H

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
	struct Scene_editor **scene_editor_address,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 November 2005

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


#endif /* !defined (SCENE_EDITOR_WX_H) */

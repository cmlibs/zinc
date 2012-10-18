/*******************************************************************************
FILE : region_tree_viewer_wx.h

LAST MODIFIED : 26 Febuary 2007

DESCRIPTION :
Widgets for editing scene, esp. changing visibility of members.
==============================================================================*/

#if !defined (REGION_TREE_VIEWER_WX_H)
#define REGION_TREE_VIEWER_WX_H_WX_H

#include "user_interface/user_interface.h"

struct Scene;
struct MANAGER(Scene);

/*
Global types
------------
*/

struct Region_tree_viewer;

/*
Global functions
----------------
*/

struct Region_tree_viewer *CREATE(Region_tree_viewer)(
	struct Region_tree_viewer **region_tree_viewer_address,
	struct Cmiss_graphics_module *graphics_module,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Cmiss_graphics_font *default_font,
	struct MANAGER(GT_object) *glyph_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 November 2005

DESCRIPTION :
Note on successful return the dialog is put at <*region_tree_viewer_address>.
==============================================================================*/

int DESTROY(Region_tree_viewer)(struct Region_tree_viewer **region_tree_viewer_address);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
==============================================================================*/

int Region_tree_viewer_bring_to_front(struct Region_tree_viewer *region_tree_viewer);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
De-iconifies and brings the scene editor to the front.
==============================================================================*/

struct Scene *Region_tree_viewer_get_scene(struct Region_tree_viewer *region_tree_viewer);
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Returns the root scene of the <region_tree_viewer>.
==============================================================================*/

int Region_tree_viewer_set_scene(struct Region_tree_viewer *region_tree_viewer,
	struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Sets the root scene of the <region_tree_viewer>. Updates widgets.
==============================================================================*/


#endif /* !defined (REGION_TREE_VIEWER_WX_H) */

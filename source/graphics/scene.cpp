/*******************************************************************************
FILE : scene.cpp

DESCRIPTION :
Structure for storing the collections of objects that make up a 3-D graphical
model - lights, materials, primitives, etc.
Also contains interface routines for having these converted to display lists,
and for these to be assembled into a single display list of the whole scene.
The display list for the whole scene lives and dies with the scene, but it is
up to others - ie. the Scene_viewer - to display.

???RC In hierarchical model a Scene should be a type derived from a
Scene_object/base graphics object - that way they can be stored in the same
containers - and in a scene themselves. Furthermore, I think Graphical
Finite Elements/Regions should be a class derived from a Scene, not just
extra functionality for it as is currently the case.

HISTORY :
November 1997. Created from Scene description part of Drawing.
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
 *   Shane Blackett <shane@blackett.co.nz>
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
#include <stdio.h>
#include <string>
#include <set>
#include <vector>
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

extern "C" {
#include "api/cmiss_scene.h"
#include "api/cmiss_scene_filter.h"
#include "api/cmiss_rendition.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/callback_private.h"
#include "general/compare.h"
#include "general/debug.h"
}
#include "general/enumerator_private_cpp.hpp"
extern "C" {
#include "general/indexed_list_private.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/rendition.h"
#include "graphics/graphics_library.h"
#include "graphics/font.h"
#include "graphics/glyph.h"
#include "graphics/graphics_object.h"
#include "graphics/light.h"
#include "graphics/scene.h"
#include "graphics/texture.h"
#include "time/time.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}
#include "graphics/scene.hpp"
#include "graphics/scene_filters.hpp"
#include "graphics/rendergl.hpp"

#if defined(USE_OPENCASCADE)
#	include "cad/computed_field_cad_geometry.h"
#	include "cad/computed_field_cad_topology.h"
#	include "cad/cad_geometry_to_graphics_object.h"
#	include "cad/cad_element.h"
//#	include "cad/cad_geometry_to_graphics_object.h"
#endif /* defined(USE_OPENCASCADE) */

/*
Module constants
----------------
*/

/*
Module variables
----------------
*/

/* select buffer size grows in increments of SELECT_BUFFER_SIZE_INCREMENT when
	 select buffer overflows. Hence, always large enough to fit all picked objects
	 in the scene */
#define SELECT_BUFFER_SIZE_INCREMENT 10000
static int select_buffer_size=10000;

/*
Module types
------------
*/
/* new_code */

/***************************************************************************//**
 * Functor for comparing/sorting regions in region tree.
 */
class Rendition_list_compare
{
private:

public:
	Rendition_list_compare()
	{
	}

	bool isRegionPlacedBefore(Cmiss_region *region1, Cmiss_region *region2) const
	{
		bool return_code = false;
		if (region1 == region2)
		{
			return false;
		}
		if (!region1 || !region2)
		{
			return false;
		}
		if (Cmiss_region_contains_subregion(region1, region2))
		{
			return true;
		}
		else if (Cmiss_region_contains_subregion(region2, region1))
		{
			return false;
		}
		Cmiss_region *parent_region1 = Cmiss_region_get_parent(region1);
		Cmiss_region *parent_region2 = Cmiss_region_get_parent(region2);
		Cmiss_region *child_region1 = NULL;
		Cmiss_region *child_region2 = NULL;
		if (parent_region1 == parent_region2)
		{
			child_region1 = Cmiss_region_access(region1);
			child_region2 = Cmiss_region_access(region2);
		}
		else
		{
			while (parent_region2 && (parent_region1 != parent_region2))
			{
				child_region2 = parent_region2;
				parent_region2 =
					Cmiss_region_get_parent(parent_region2);
				if (parent_region1 != parent_region2)
					Cmiss_region_destroy(&child_region2);
			}
			if (parent_region1 != parent_region2)
			{
				parent_region2 = Cmiss_region_get_parent(region2);
				child_region2 = Cmiss_region_access(region2);
				while (parent_region1 &&
					(parent_region2 != parent_region1))
				{
					child_region1 = parent_region1;
					parent_region1 =
						Cmiss_region_get_parent(parent_region1);
					if (parent_region1 != parent_region2)
						Cmiss_region_destroy(&child_region1);
				}
			}
		}
		if (child_region1 && child_region2 && parent_region1 == parent_region2)
		{
			Cmiss_region *child_region = Cmiss_region_get_first_child(
				parent_region1);
			while (child_region)
			{
				if (child_region == child_region1)
				{
					return_code = true;
					break;
				}
				else if (child_region == child_region2)
				{
					return_code = false;
					break;
				}
				Cmiss_region_reaccess_next_sibling(&child_region);
			}
			if (child_region)
				Cmiss_region_destroy(&child_region);
		}
		if (parent_region1)
			Cmiss_region_destroy(&parent_region1);
		if (parent_region2)
			Cmiss_region_destroy(&parent_region2);
		if (child_region1)
			Cmiss_region_destroy(&child_region1);
		if (child_region2)
			Cmiss_region_destroy(&child_region2);
		return return_code;
	}

	bool operator() (const Cmiss_rendition *rendition1, const Cmiss_rendition *rendition2) const
	{
		Cmiss_region *region1 = Cmiss_rendition_get_region(const_cast<Cmiss_rendition *>(rendition1));
		Cmiss_region *region2 = Cmiss_rendition_get_region(const_cast<Cmiss_rendition *>(rendition2));

		return isRegionPlacedBefore(region1, region2);
	}
};

typedef std::set<Cmiss_rendition*,Rendition_list_compare> Rendition_set;
typedef std::vector<Cmiss_scene_filter*> Scene_filter_vector;

struct Scene
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Stores the collections of objects that make up a 3-D graphical model.
==============================================================================*/
{
	/* the name of the scene */
	const char *name;
	/* keep pointer to this scene's manager since can pass on manager change */
	/* messages if member manager changes occur (eg. materials) */
	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Scene) *scene_manager;
	int manager_change_status;

	/* list of objects in the scene (plus visibility flag) */
	enum Scene_change_status change_status;

	struct Cmiss_region *region;
	struct MANAGER(Light) *light_manager;
	void *light_manager_callback_id;
	struct LIST(Light) *list_of_lights;
#if defined (OLD_CODE)
	/* need following info for autocreation of graphical finite elements, */
	/* material/light changes, etc. */
	enum Scene_graphical_element_mode graphical_element_mode;

	/*???RC temporary; have root_region until Scenes are
		incorporated into the regions themselves */
	struct Cmiss_region *root_region;

	/* global stores of selected objects */
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;

	/* attribute managers and defaults: */
	struct MANAGER(GT_object) *glyph_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_material;
	struct Graphics_font *default_font;
	void *graphical_material_manager_callback_id;
	struct MANAGER(Spectrum) *spectrum_manager;
	void *spectrum_manager_callback_id;
	struct Spectrum *default_spectrum;
	struct MANAGER(Texture) *texture_manager;
	struct Time_keeper *default_time_keeper;
	struct User_interface *user_interface;
#endif /* defined (OLD_CODE) */
	/* routine to call and data to pass to a module that handles scene input */
	/* such as picking and mouse drags */
	struct Scene_input_callback input_callback;
	Rendition_set *list_of_rendition;
	/* level of cache in effect */
	int cache;
	/* flag indicating that graphics objects in scene objects need building */
	int build;
	Scene_filter_vector *filters;
#if defined (OPENGL_API)
	/* display list identifier for the scene */
	GLuint display_list,fast_changing_display_list;
	/* objects compiled within the context of this scene will execute these
		display lists if the want to switch to ndc coordinate system and when
		they want to switch back. These are overridden when picking to ensure that
		the correct objects are selected. */
	GLuint ndc_display_list;
	GLuint end_ndc_display_list;
#endif /* defined (OPENGL_API) */
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/* the number of objects accessing this scene. The scene cannot be removed
		from manager unless it is 1 (ie. only the manager is accessing it) */
	int access_count;
}; /* struct Scene */



FULL_DECLARE_LIST_TYPE(Scene);
FULL_DECLARE_MANAGER_TYPE(Scene);

struct Scene_picked_object
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Describes a single picked item in a format compatible with objects in our
display hierarchy.
==============================================================================*/
{
	/* the number of this picking event */
	int hit_no;
	/* path of scene objects to picked graphic in display hierarchy */
 	int number_of_renditions;
 	struct Cmiss_rendition **renditions;
	/* integer names identifying parts of picked graphic, eg. node numbers */
	int number_of_subobjects;
	int *subobjects; /*???RC unsigned int instead? */
	/* z-range of picked part of graphics_object/sub_objects: */
	double nearest,farthest;
	/* so LISTs etc. can be used: */
	int access_count;
}; /* struct Scene_picked_object */

FULL_DECLARE_LIST_TYPE(Scene_picked_object);

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Scene)

#if defined (USE_SCENE_OBJECT)
/* prototype */
static int Scene_object_set_time_dependent_transformation(struct Time_object *time_object,
	 double current_time, void *scene_object_void);
#endif /* defined (USE_SCENE_OBJECT) */

int Scene_compile_members(struct Scene *scene, Render_graphics *renderer)
{
	int return_code;

	ENTER(Scene_compile_members);
	if (scene)
	{
		return_code = 1;
		if (GRAPHICS_COMPILED != scene->compile_status)
		{
#if defined (NEW_CODE)
			if (scene->ndc_display_list || (scene->ndc_display_list = glGenLists(1)))
			{
				glNewList(scene->ndc_display_list, GL_COMPILE);
				/* Push the current model matrix and reset the model matrix to identity */
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				/* near = 1.0 and far = 3.0 gives -1 to be the near clipping plane
					and +1 to be the far clipping plane */
				glOrtho(-1.0,1.0,-1.0,1.0,1.0,3.0);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				gluLookAt(/*eye*/0.0,0.0,2.0, /*lookat*/0.0,0.0,0.0,
					/*up*/0.0,1.0,0.0);
				glEndList();
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Scene.  Could not generate display list");
				return_code = 0;
			}
			if (scene->end_ndc_display_list || (scene->end_ndc_display_list = glGenLists(1)))
			{
				glNewList(scene->end_ndc_display_list, GL_COMPILE);
				/* Pop the model matrix stack */
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
 				glEndList();
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Scene.  Could not generate display list");
				return_code = 0;
			}
#endif // defined (NEW_CODE)
			/* compile objects in the scene */

			if (scene->list_of_rendition && 
				!scene->list_of_rendition->empty())
			{
				Rendition_set::iterator pos =
					scene->list_of_rendition->begin();
				while (pos != scene->list_of_rendition->end())
				{
					renderer->Cmiss_rendition_compile(*pos);
					++pos;
				}
			}

			/* compile lights in the scene */
			FOR_EACH_OBJECT_IN_LIST(Light)(compile_Light,(void *)NULL,
				scene->list_of_lights);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_compile_members.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_compile_members */

int Scene_compile_opengl_display_list(struct Scene *scene,
	Callback_base< Scene * > *execute_function,
	Render_graphics_opengl *renderer)
{
	int return_code;
	Rendition_set::iterator pos;

	ENTER(compile_Scene);
	USE_PARAMETER(execute_function);
	if (scene)
	{
		return_code = 1;
		if (GRAPHICS_COMPILED != scene->compile_status)
		{
			if (scene->display_list || (scene->display_list = glGenLists(1)))
			{
				/* compile non-fast changing part of scene */
				glNewList(scene->display_list, GL_COMPILE);
				/* Normally we would call execute_Scene here with the 
				 * display_list renderer but we are splitting it into two display
				 * lists.
				 * execute_function(scene);
				 */
				/* push a dummy name to be overloaded with scene_object identifiers */
				glPushName(0);
				if (scene->list_of_rendition && 
					!scene->list_of_rendition->empty())
				{
					pos = scene->list_of_rendition->begin();
					while (pos != scene->list_of_rendition->end())
					{
						Cmiss_rendition_call_renderer(*pos, (void *)renderer);
						++pos;
					}
				}
				renderer->Overlay_graphics_execute();
				glPopName();
				glEndList();
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Scene.  Could not generate display list");
				return_code = 0;
			}
			if (Scene_has_fast_changing_objects(scene))
			{
				if (scene->fast_changing_display_list ||
					(scene->fast_changing_display_list = glGenLists(1)))
				{
					renderer->fast_changing = 1;
					glNewList(scene->fast_changing_display_list, GL_COMPILE);
					/* push dummy name to be overloaded with scene_object identifiers */
					glPushName(0);
					if (scene->list_of_rendition && 
						!scene->list_of_rendition->empty())
					{
						pos = scene->list_of_rendition->begin();
						while (pos != scene->list_of_rendition->end())
						{
							Cmiss_rendition_call_renderer(*pos, (void *)renderer);
							++pos;
						}
					}
					glPopName();
					glEndList();
					renderer->fast_changing = 0;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"compile_Scene.  Could not generate fast changing display list");
					return_code = 0;
				}
			}
		}
		/* Assume the child graphics have just been compiled if necessary, could change
		 * to toggle GRAPHICS_COMPILED and CHILD_GRAPHICS_COMPILED flags separately.
		 */
		if (return_code)
		{
			scene->compile_status = GRAPHICS_COMPILED;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "compile_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* compile_Scene */

/***************************************************************************//**
 * Execute the display list for this object.
 */
int Scene_execute_opengl_display_list(struct Scene *scene, Render_graphics_opengl *renderer)
{
	int return_code;

	ENTER(execute_Scene);
	USE_PARAMETER(renderer);
	if (scene)
	{
		if (GRAPHICS_COMPILED == scene->compile_status)
		{
			/* Should handle cases where we only want fast or non-fast */
			glCallList(scene->display_list);
			if (Scene_has_fast_changing_objects(scene))
			{
				glCallList(scene->fast_changing_display_list);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_child_Scene.  display list not current");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_child_Scene.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_child_Scene */

static int Scene_refresh(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Tells the scene it has changed, forcing it to send the manager message
MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_refresh);
	if (scene)
	{
		return_code = 1;	
		/* send no messages if caching is enabled or no changes */
		if ((0 == scene->cache) && (SCENE_NO_CHANGE != scene->change_status))
		{
			return_code = MANAGED_OBJECT_CHANGE(Scene)(scene,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Scene));
			/* clear the change_status after messages sent */
			scene->change_status = SCENE_NO_CHANGE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_refresh.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_refresh */

static int Scene_notify_object_changed(struct Scene *scene,int fast_changing)
/*******************************************************************************
LAST MODIFIED : 13 March 2002

DESCRIPTION :
Scene functions such as add/remove graphics_object and set_visibility change
the scene->compile_status to GRAPHICS_NOT_COMPILED. Changes to objects in the
scene only require a rebuild of those objects themselves, not the scene. This
latter case a compile_status of CHILD_GRAPHICS_NOT_COMPILED. This function sets
the compile status to CHILD_GRAPHICS_NOT_COMPILED if it is not already
GRAPHICS_NOT_COMPILED, then calls Scene_refresh to inform clients of the change.
They must call compile_Scene to make sure its display list is made up to date.
If the <fast_changing> flag is set, and the scene is not already set for general
change, set it to SCENE_FAST_CHANGE, otherwise SCENE_CHANGE.
Private to the Scene and Scene_objects
==============================================================================*/
{
	int return_code;

	ENTER(Scene_notify_object_changed);
	if (scene)
	{
		/* mark scene as needing a build */
		scene->build = 1;
		if (fast_changing && (SCENE_CHANGE != scene->change_status))
		{
			scene->change_status = SCENE_FAST_CHANGE;
		}
		else
		{
			scene->change_status = SCENE_CHANGE;
		}
		if (GRAPHICS_NOT_COMPILED != scene->compile_status)
		{
			/* objects in scene need recompiling */
			scene->compile_status = CHILD_GRAPHICS_NOT_COMPILED;
		}
#if defined (DEBUG)
		/*???debug*/
		printf("Scene %s changed %i\n",scene->name,scene->compile_status);
#endif /* defined (DEBUG) */
		if (scene->scene_manager)
		{
			return_code = Scene_refresh( scene );
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_notify_object_changed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_notify_object_changed */

static int Scene_changed_private(struct Scene *scene,int fast_changing)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Tells the scene it has changed, forcing it to send the manager message
MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER.  Recompiles the scene display list as well
as the objects in the scene unlike the public Scene_changed which only compiles
the component objects. If the <fast_changing> flag is set, and the scene is not
already set for general change, set it to SCENE_FAST_CHANGE, otherwise
SCENE_CHANGE. If the <fast_changing> flag is set, and the
scene is not already set for general change, set it to SCENE_FAST_CHANGE,
otherwise SCENE_CHANGE.
Private to the Scene and Scene_objects.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_changed_private);
	if (scene)
	{
		/* mark scene as needing a build */
		scene->build = 1;
		if (fast_changing && (SCENE_CHANGE != scene->change_status))
		{
			scene->change_status = SCENE_FAST_CHANGE;
		}
		else
		{
			scene->change_status = SCENE_CHANGE;
		}
		scene->compile_status = GRAPHICS_NOT_COMPILED;
#if defined (DEBUG)
		/*???debug*/
		printf("Scene %s changed %i\n",scene->name,scene->compile_status);
#endif /* defined (DEBUG) */
		if (scene->scene_manager)
		{
			return_code = Scene_refresh( scene );
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_changed_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_changed_private */

#if defined (OLD_CODE)
int Scene_update_graphical_element_groups(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
Ensures there is a GT_element_group in <scene> for every Cmiss_region.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_update_graphical_element_groups);
	if (scene)
	{
		Scene_begin_cache(scene);
		struct Cmiss_region *child_region =
			Cmiss_region_get_first_child(scene->root_region);
		while (NULL != child_region)
		{
#if defined (TO_BE_EDITED)
			if (!Scene_has_Cmiss_region(scene, child_region))
			{
				char *child_region_name = Cmiss_region_get_name(child_region);
				Scene_add_graphical_element_group(scene, child_region,
					/*position*/0, child_region_name);
				DEALLOCATE(child_region_name);
			}
			Cmiss_region_reaccess_next_sibling(&child_region);
#endif
		}
		Scene_end_cache(scene);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_update_graphical_element_groups.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_update_graphical_element_groups */
#endif /* defined (OLD_CODE) */

#if defined (TO_BE_EDITED)
static void Scene_Cmiss_region_change(struct Cmiss_region *root_region,
	struct Cmiss_region_changes *region_changes, void *scene_void)
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
Callback from <root_region> informing of <changes>.
<scene> adds or removes graphical element groups to match.
==============================================================================*/
{
	int return_code;
	struct Scene *scene;
	struct Scene_object *scene_object;

	ENTER(Scene_Cmiss_region_change);
	if (root_region && region_changes && (scene = (struct Scene *)scene_void))
	{
		if (region_changes->children_changed)
		{
			Scene_begin_cache(scene);
			if (region_changes->child_removed)
			{
				return_code = 1;
				while (return_code &&
					(scene_object = FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
						Scene_object_has_Cmiss_region, (void *)region_changes->child_removed,
						scene->scene_object_list)))
				{
					return_code =
						Scene_remove_Scene_object_private(scene, scene_object);
				}
			}
			else if (region_changes->child_added)
			{
				if (GRAPHICAL_ELEMENT_MANUAL != scene->graphical_element_mode)
				{
					Scene_begin_cache(scene);
					/* ensure we have a graphical element for this child region if
					 we now have a node and data pair */
					if (root_region == scene->root_region)
					{
						if (!Scene_has_Cmiss_region(scene, region_changes->child_added))
						{
							char *child_region_name =
								Cmiss_region_get_name(region_changes->child_added);
							return_code = Scene_add_graphical_element_group(scene,
								region_changes->child_added, /*position*/0, child_region_name);
							DEALLOCATE(child_region_name);
						}
					}
					else
					{
						return_code = 0;
					}
					Scene_end_cache(scene);
				}
			}
			else
			{
				/* All change case, check everything */
				/* remove any graphical elements for regions that no longer exist */
				return_code = 1;
				while (return_code &&
					(scene_object = FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
						Scene_object_has_removed_Cmiss_region, scene_void,
						scene->scene_object_list)))
				{
					return_code =
						Scene_remove_Scene_object_private(scene, scene_object);
				}
				if (GRAPHICAL_ELEMENT_MANUAL != scene->graphical_element_mode)
				{
					/* ensure we have a graphical element for each child region */
					Scene_update_graphical_element_groups(scene);
				}
			}
			Scene_end_cache(scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_Cmiss_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_Cmiss_region_change */
#endif /* defined (TO_BE_EDITED) */

#if defined (OLD_CODE)
/***************************************************************************//**
 * Something has changed globally in the material manager.
 * Tell the scene it has changed and it will rebuild affected materials too.
 */
static void Scene_Graphical_material_change(
	struct MANAGER_MESSAGE(Graphical_material) *message, void *scene_void)
{
	struct Scene *scene;

	ENTER(Scene_Graphical_material_change);
	if (message && (scene = (struct Scene *)scene_void))
	{
		struct LIST(Graphical_material) *changed_material_list =
			MANAGER_MESSAGE_GET_CHANGE_LIST(Graphical_material)(message,
				MANAGER_CHANGE_RESULT(Graphical_material));
		if (changed_material_list)
		{
			Scene_begin_cache(scene);// 				Cmiss_rendition_Graphical_material_change(Cmiss_region_get_rendition(scene->region),
// 					message->changed_object_list);
// 				for_each_rendition_in_Cmiss_rendition(
// 					Cmiss_region_get_rendition(scene->region),
// 					&Cmiss_rendition_Graphical_material_change,(void *)message->changed_object_list);
				Scene_end_cache(scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_Graphical_material_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_Graphical_material_change */

/***************************************************************************//**
 * Something has changed globally in the spectrum manager. Mark status of all
 * graphics_obects using the affected spectrums as g_NOT_CREATED.
 */
static void Scene_Spectrum_change(
	struct MANAGER_MESSAGE(Spectrum) *message,void *scene_void)
{
	struct Scene *scene;

	ENTER(Scene_Spectrum_change);
	if (message && (scene = (struct Scene *)scene_void))
	{
		struct LIST(Spectrum) *changed_spectrum_list =
			MANAGER_MESSAGE_GET_CHANGE_LIST(Spectrum)(message,
				MANAGER_CHANGE_RESULT(Spectrum));
		if (changed_spectrum_list)
		{
			Scene_begin_cache(scene);
#if defined (USE_SCENE_OBJECT)
			FOR_EACH_OBJECT_IN_LIST(Scene_object)(Scene_object_Spectrum_change,
				(void *)(message->changed_object_list), scene->scene_object_list);
#endif /* defined (USE_SCENE_OBJECT) */
#if defined (TO_BE_EDITED)
			GT_element_group_Spectrum_change(scene->gt_element_group,
				message->changed_spectrum_list);
#endif
			DESTROY_LIST(Spectrum)(&changed_spectrum_list);
			Scene_end_cache(scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_Spectrum_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_Spectrum_change */
#endif /* defined (OLD_CODE) */

struct Scene_picked_object_get_nearest_any_object_data
{
	/* "nearest" value from Scene_picked_object for picked_any_object */
	double nearest;
	struct Any_object *nearest_any_object;
	/* information about the nearest any_object */
	struct Scene_picked_object *scene_picked_object;
};

static int Scene_picked_object_get_nearest_any_object(
	struct Scene_picked_object *scene_picked_object,
	void *nearest_any_object_data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
If the <scene_picked_object> refers to an any_object, the "nearest" value is
compared with that for the current nearest any_object in the
<nearest_any_object_data>. If there was no current nearest any_object or the new
any_object is nearer, it becomes the nearest any_object and its "nearest" value
is stored in the nearest_any_object_data.
==============================================================================*/
{
	int return_code;
	//struct Any_object *any_object;
#if defined (USE_SCENE_OBJECT)
	struct Scene_object *scene_object;
#endif /* defined (USE_SCENE_OBJECT) */
	struct Scene_picked_object_get_nearest_any_object_data
		*nearest_any_object_data;

	ENTER(Scene_picked_object_get_nearest_any_object);
	if (scene_picked_object&&(nearest_any_object_data=
		(struct Scene_picked_object_get_nearest_any_object_data	*)
		nearest_any_object_data_void))
	{
		return_code=1;
		/* proceed only if there is no picked_any_object or object is nearer */
		if (((struct Any_object *)NULL ==
			nearest_any_object_data->nearest_any_object) ||
			(Scene_picked_object_get_nearest(scene_picked_object) <
				nearest_any_object_data->nearest))
		{
			/* if the last scene_object represents an any_object, add it to list */
#if defined (USE_SCENE_OBJECT)
			if ((scene_object=Scene_picked_object_get_Scene_object(
				scene_picked_object,Scene_picked_object_get_number_of_scene_objects(
					scene_picked_object)-1))&&
				(any_object=Scene_object_get_represented_object(scene_object)))
			{
				nearest_any_object_data->nearest_any_object=any_object;
				nearest_any_object_data->scene_picked_object=scene_picked_object;
				nearest_any_object_data->nearest=
					Scene_picked_object_get_nearest(scene_picked_object);
			}
#endif /* defined (USE_SCENE_OBJECT) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_any_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_nearest_any_object */

static int Scene_picked_object_get_picked_any_objects(
	struct Scene_picked_object *scene_picked_object,
	void *any_object_list_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
If the <scene_picked_object> refers to an any_object, it is converted into
an Any_object and added to the <picked_any_objects_list>.
==============================================================================*/
{
	int return_code;
	//struct Any_object *any_object;
	struct LIST(Any_object) *any_object_list;
#if defined (USE_SCENE_OBJECT)
	struct Scene_object *scene_object;
#endif /* defined (USE_SCENE_OBJECT) */

	ENTER(Scene_picked_object_get_picked_any_objects);
	if (scene_picked_object&&
		(any_object_list=(struct LIST(Any_object) *)any_object_list_void))
	{
		return_code=1;
		/* if the last scene_object represents an any_object, add it to list */
#if defined (USE_SCENE_OBJECT)
		if ((scene_object=Scene_picked_object_get_Scene_object(scene_picked_object,
			Scene_picked_object_get_number_of_scene_objects(scene_picked_object)-1))&&
			(any_object=Scene_object_get_represented_object(scene_object)))
		{
			ADD_OBJECT_TO_LIST(Any_object)(any_object,any_object_list);
		}
#endif /* defined (USE_SCENE_OBJECT) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_picked_any_objects.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_picked_any_objects */

struct Scene_picked_object_get_nearest_element_data
{
	int select_elements_enabled,select_faces_enabled,select_lines_enabled;
	/* "nearest" value from Scene_picked_object for picked_element */
	double nearest;
	struct FE_element *nearest_element;
	/* group that the element must be in, or any group if NULL */
	struct Cmiss_region *cmiss_region;
	/* information about the nearest element */
	struct Scene_picked_object *scene_picked_object;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
};

#if defined (USE_OPENCASCADE)
struct Scene_picked_object_get_cad_primitive_data
{
	int select_surfaces_enabled,select_lines_enabled;
	/* value from Scene_picked_object for picked_element */
	Cmiss_cad_identifier_id nearest_cad_element;
	double nearest;
	Cad_primitive_type element_type;
	/* group that the element must be in, or any group if NULL */
	struct Cmiss_region *cmiss_region;
	/* information about the nearest element */
	struct Scene_picked_object *scene_picked_object;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
};
#endif /* defined (USE_OPENCASCADE) */

static int Scene_picked_object_get_nearest_element(
	struct Scene_picked_object *scene_picked_object,
	void *nearest_element_data_void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to an element, the "nearest" value is
compared with that for the current nearest element in the
<nearest_element_data>. If there was no current nearest element or the new
element is nearer, it becomes the nearest element and its "nearest" value is
stored in the nearest_element_data.
==============================================================================*/
{
	int dimension,return_code;
	struct CM_element_information cm;
	struct FE_element *element;
	struct FE_region *fe_region;
	struct Cmiss_region *cmiss_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_get_nearest_element_data	*nearest_element_data;

	ENTER(Scene_picked_object_get_nearest_element);
	if (scene_picked_object&&(nearest_element_data=
		(struct Scene_picked_object_get_nearest_element_data	*)
		nearest_element_data_void))
	{
		return_code=1;
		/* proceed only if there is no picked_element or object is nearer */
		if (((struct FE_element *)NULL==nearest_element_data->nearest_element)||
			(Scene_picked_object_get_nearest(scene_picked_object) <
				nearest_element_data->nearest))
		{
			/* is the last scene_object a Graphical_element wrapper, and does the
				 settings for the graphic refer to elements? */
			if ((NULL != (rendition=Scene_picked_object_get_rendition(
				scene_picked_object,
				Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
				&&(NULL != (cmiss_region = Cmiss_rendition_get_region(rendition)))&&
				(2<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
				(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
						Scene_picked_object_get_subobject(scene_picked_object,0))))&&
				(Cmiss_graphic_selects_elements(graphic)))
			{
				if (CM_element_information_from_graphics_name(&cm,
					Scene_picked_object_get_subobject(scene_picked_object,1)) &&
					(fe_region = Cmiss_region_get_FE_region(cmiss_region)) &&
					(element = FE_region_get_FE_element_from_identifier(fe_region, &cm)))
				{
					dimension = get_FE_element_dimension(element);
					if (((nearest_element_data->select_elements_enabled &&
						((CM_ELEMENT == cm.type) || (3 == dimension))) ||
						(nearest_element_data->select_faces_enabled &&
							((CM_FACE == cm.type) || (2 == dimension))) ||
						(nearest_element_data->select_lines_enabled &&
							((CM_LINE == cm.type) || (1 == dimension))))&&
						((!nearest_element_data->cmiss_region) ||
							((fe_region = Cmiss_region_get_FE_region(
								nearest_element_data->cmiss_region)) &&
								FE_region_contains_FE_element(fe_region, element))))
					{
						nearest_element_data->nearest_element=element;
						nearest_element_data->scene_picked_object=scene_picked_object;
						nearest_element_data->rendition = rendition;
						nearest_element_data->graphic=graphic;
						nearest_element_data->nearest=
							Scene_picked_object_get_nearest(scene_picked_object);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_picked_object_get_nearest_element.  "
						"Invalid element %s %d",CM_element_type_string(cm.type),cm.number);
					return_code=0;
				}
			}
			if (graphic)
				Cmiss_graphic_destroy(&graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_nearest_element */

struct Scene_picked_object_get_picked_elements_data
{
	int select_elements_enabled,select_faces_enabled,select_lines_enabled;
	struct LIST(FE_element) *picked_element_list;
};

typedef std::multimap<Cmiss_region *, Cmiss_element_id> Region_element_map;

struct Scene_picked_object_region_element_map_data
{
  Region_element_map *element_list;
	/* flag set when searching for nearest data point rather than node */
	int select_elements_enabled,select_faces_enabled,select_lines_enabled;
};

struct Scene_picked_object_get_nearest_element_point_data
{
	/* "nearest" value from Scene_picked_object for picked_element_point */
	double nearest;
	struct Element_point_ranges *nearest_element_point;
	/* region that the element_point must be in, or root_region if NULL */
	struct Cmiss_region *cmiss_region;
	/* information about the nearest element_point */
	struct Scene_picked_object *scene_picked_object;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
};

static int Scene_picked_object_get_nearest_element_point(
	struct Scene_picked_object *scene_picked_object,
	void *nearest_element_point_data_void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to an element_point, the "nearest" value is
compared with that for the current nearest element_point in the
<nearest_element_point_data>. If there was no current nearest element_point or
the new element_point is nearer, it becomes the nearest element_point and its
"nearest" value is stored in the nearest_element_point_data.
Note that the <nearest_element_point> is an Element_point_ranges structure
created to store the nearest point; it is up to the calling function to manage
and destroy it once returned.
==============================================================================*/
{
	int element_point_number,face_number,i,return_code,
		top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct CM_element_information cm;
	struct Element_discretization element_discretization;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct FE_element *element,*top_level_element;
	struct FE_field *native_discretization_field;
	struct FE_region *fe_region;
	struct Cmiss_region *cmiss_region;
	struct Cmiss_rendition *rendition = NULL;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_get_nearest_element_point_data
		*nearest_element_point_data;
	Triple xi;

	ENTER(Scene_picked_object_get_nearest_element_point);
	if (scene_picked_object&&(nearest_element_point_data=
		(struct Scene_picked_object_get_nearest_element_point_data	*)
		nearest_element_point_data_void))
	{
		return_code=1;
		/* proceed only if there is no picked_element_point or object is nearer */
		if (((struct Element_point_ranges *)NULL==
			nearest_element_point_data->nearest_element_point)||
			(Scene_picked_object_get_nearest(scene_picked_object) <
				nearest_element_point_data->nearest))
		{
			/* is the last scene_object a Graphical_element wrapper, and does the
				 settings for the graphic refer to element_points? */
			if ((NULL != (rendition=Scene_picked_object_get_rendition(
				scene_picked_object,
				Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
				&&(NULL != (cmiss_region = Cmiss_rendition_get_region(rendition)))&&
				(3<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
				(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
						Scene_picked_object_get_subobject(scene_picked_object,0))))&&
				(CMISS_GRAPHIC_ELEMENT_POINTS==
					Cmiss_graphic_get_graphic_type(graphic)))
			{
				if (CM_element_information_from_graphics_name(&cm,
					Scene_picked_object_get_subobject(scene_picked_object,1))&&
					(fe_region = Cmiss_region_get_FE_region(cmiss_region)) &&
					(element = FE_region_get_FE_element_from_identifier(fe_region, &cm)))
				{
					if ((!nearest_element_point_data->cmiss_region) ||
						FE_region_contains_FE_element(Cmiss_region_get_FE_region(
							nearest_element_point_data->cmiss_region), element))
					{
						/* determine discretization of element for graphic */
						top_level_element=(struct FE_element *)NULL;
						Cmiss_graphic_get_discretization(graphic,
							&element_discretization);
						top_level_number_in_xi[0]=element_discretization.number_in_xi1;
						top_level_number_in_xi[1]=element_discretization.number_in_xi2;
						top_level_number_in_xi[2]=element_discretization.number_in_xi3;
						Cmiss_graphic_get_face(graphic,&face_number);
						native_discretization_field=
							Cmiss_graphic_get_native_discretization_field(graphic);
						if (FE_region_get_FE_element_discretization(fe_region, element,
							face_number, native_discretization_field, top_level_number_in_xi,
							&top_level_element, element_point_ranges_identifier.number_in_xi))
						{
							element_point_ranges_identifier.element=element;
							element_point_ranges_identifier.top_level_element=
								top_level_element;
							Cmiss_graphic_get_xi_discretization(graphic,
								&(element_point_ranges_identifier.xi_discretization_mode),
								/*xi_point_density_field*/(struct Computed_field **)NULL);
							if (XI_DISCRETIZATION_EXACT_XI==
								element_point_ranges_identifier.xi_discretization_mode)
							{
								for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
								{
									element_point_ranges_identifier.number_in_xi[i]=1;
								}
							}
							Cmiss_graphic_get_seed_xi(graphic,xi);
							/*???RC temporary, hopefully */
							for (i=0;i<3;i++)
							{
								element_point_ranges_identifier.exact_xi[i]=xi[i];
							}
							if (element_point_ranges=CREATE(Element_point_ranges)(
								&element_point_ranges_identifier))
							{
								element_point_number=
									Scene_picked_object_get_subobject(scene_picked_object,2);
								if (Element_point_ranges_add_range(element_point_ranges,
									element_point_number,element_point_number))
								{
									if (nearest_element_point_data->nearest_element_point)
									{
										DESTROY(Element_point_ranges)(
											&(nearest_element_point_data->nearest_element_point));
									}
									nearest_element_point_data->nearest_element_point=
										element_point_ranges;
									nearest_element_point_data->scene_picked_object=
										scene_picked_object;
									nearest_element_point_data->rendition = rendition;
									nearest_element_point_data->graphic=graphic;
									nearest_element_point_data->nearest=
										Scene_picked_object_get_nearest(scene_picked_object);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Scene_picked_object_get_nearest_element_point.  "
										"Could not add element point range");
									DESTROY(Element_point_ranges)(&element_point_ranges);
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Scene_picked_object_get_nearest_element_point.  "
									"Could not create Element_point_ranges");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Scene_picked_object_get_nearest_element_point.  "
								"Could not get discretization");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_picked_object_get_nearest_element_point.  "
						"Invalid element %s %d",CM_element_type_string(cm.type),cm.number);
					return_code=0;
				}
			}
		}
		if (graphic)
		{
			Cmiss_graphic_destroy(&graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_element_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_nearest_element_point */

static int Scene_picked_object_get_picked_element_points(
	struct Scene_picked_object *scene_picked_object,
	void *picked_element_points_list_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to an element_point, it is converted into
an Element_point_ranges and added to the <picked_element_points_list>.
==============================================================================*/
{
	int return_code;

// 	int element_point_number,face_number,i,return_code,
// 		top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
// 	struct CM_element_information cm;
// 	struct Cmiss_region *cmiss_region;
// 	struct Element_discretization element_discretization;
// 	struct Element_point_ranges *element_point_ranges;
// 	struct Element_point_ranges_identifier element_point_ranges_identifier;
// 	struct FE_element *element,*top_level_element;
// 	struct FE_field *native_discretization_field;
// 	struct FE_region *fe_region;
//	struct Cmiss_rendition *rendition;
//	struct Cmiss_graphic *graphic = NULL;
//	Triple xi;

	ENTER(Scene_picked_object_get_picked_element_points);
	if (scene_picked_object&&picked_element_points_list_void)
	{
		return_code=1;
//		/* is the last scene_object a Graphical_element wrapper, and does the
//			 settings for the graphic refer to element_points? */
//		if ((NULL != (rendition=Scene_picked_object_get_rendition(
//			scene_picked_object,
//			Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
//			&&(NULL != (cmiss_region = Cmiss_rendition_get_region(rendition)))&&
//			(3<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
//			(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
//					Scene_picked_object_get_subobject(scene_picked_object,0))))&&
//			(CMISS_GRAPHIC_ELEMENT_POINTS ==
//				Cmiss_graphic_get_graphic_type(graphic)))
//		{
//			if (CM_element_information_from_graphics_name(&cm,
//				Scene_picked_object_get_subobject(scene_picked_object,1))&&
//				(fe_region = Cmiss_region_get_FE_region(cmiss_region)) &&
//				(element = FE_region_get_FE_element_from_identifier(fe_region, &cm)))
//			{
//				/* determine discretization of element for graphic */
//				top_level_element=(struct FE_element *)NULL;
//				Cmiss_graphic_get_discretization(graphic, &element_discretization);
//				top_level_number_in_xi[0]=element_discretization.number_in_xi1;
//				top_level_number_in_xi[1]=element_discretization.number_in_xi2;
//				top_level_number_in_xi[2]=element_discretization.number_in_xi3;
//				Cmiss_graphic_get_face(graphic,&face_number);
//				native_discretization_field=
//					GT_element_settings_get_native_discretization_field(settings);
//				if (FE_region_get_FE_element_discretization(fe_region, element,
//					face_number, native_discretization_field, top_level_number_in_xi,
//					&top_level_element, element_point_ranges_identifier.number_in_xi))
//				{
//					element_point_ranges_identifier.element=element;
//					element_point_ranges_identifier.top_level_element=top_level_element;
//					GT_element_settings_get_xi_discretization(settings,
//						&(element_point_ranges_identifier.xi_discretization_mode),
//						/*xi_point_density_field*/(struct Computed_field **)NULL);
//					if (XI_DISCRETIZATION_EXACT_XI==
//						element_point_ranges_identifier.xi_discretization_mode)
//					{
//						for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
//						{
//							element_point_ranges_identifier.number_in_xi[i]=1;
//						}
//					}
//					GT_element_settings_get_seed_xi(settings,xi);
//					/*???RC temporary, hopefully */
//					for (i=0;i<3;i++)
//					{
//						element_point_ranges_identifier.exact_xi[i]=xi[i];
//					}
//					if (element_point_ranges=CREATE(Element_point_ranges)(
//						&element_point_ranges_identifier))
//					{
//						element_point_number=
//							Scene_picked_object_get_subobject(scene_picked_object,2);
//						if (!(Element_point_ranges_add_range(element_point_ranges,
//							element_point_number,element_point_number)&&
//							Element_point_ranges_add_to_list(element_point_ranges,
//								picked_element_points_list_void)))
//						{
//							display_message(ERROR_MESSAGE,
//								"Scene_picked_object_get_picked_element_points.  "
//								"Could not add element point to picked list");
//							return_code=0;
//						}
//						DESTROY(Element_point_ranges)(&element_point_ranges);
//					}
//					else
//					{
//						display_message(ERROR_MESSAGE,
//							"Scene_picked_object_get_picked_element_points.  "
//							"Could not create Element_point_ranges");
//						return_code=0;
//					}
//				}
//				else
//				{
//					display_message(ERROR_MESSAGE,
//						"Scene_picked_object_get_picked_element_points.  "
//						"Could not get discretization");
//					return_code=0;
//				}
//			}
//			else
//			{
//				display_message(ERROR_MESSAGE,
//					"Scene_picked_object_get_picked_element_points.  "
//					"Invalid element %s %d",CM_element_type_string(cm.type),cm.number);
//				return_code=0;
//			}
//		}
//		if (graphic)
//		{
//			Cmiss_graphic_destroy(&graphic);
//		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_picked_element_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_picked_element_points */

struct Scene_picked_object_get_nearest_node_data
{
	/* "nearest" value from Scene_picked_object for picked_node */
	double nearest;
	struct FE_node *nearest_node;
	/* flag set when searching for nearest data point rather than node */
	int use_data;
	/* region that the node must be in, or any region if NULL */
	struct Cmiss_region *cmiss_region;
	/* information about the nearest node */
	struct Scene_picked_object *scene_picked_object;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
};

static int Scene_picked_object_get_nearest_node(
	struct Scene_picked_object *scene_picked_object,void *nearest_node_data_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to a node, the "nearest" value is compared
with that for the current nearest node in the <nearest_node_data>. If there was
no current nearest node or the new node is nearer, it becomes the picked node
and its "nearest" value is stored in the nearest_node_data.
==============================================================================*/
{
	enum Cmiss_graphic_type graphic_type;
	int node_number,return_code;
	struct FE_node *node;
	struct FE_region *fe_region;
	struct Scene_picked_object_get_nearest_node_data *nearest_node_data;
	struct Cmiss_region *cmiss_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;

	ENTER(Scene_picked_object_get_nearest_node);
	if (scene_picked_object&&(nearest_node_data=
		(struct Scene_picked_object_get_nearest_node_data	*)nearest_node_data_void))
	{
		return_code=1;
		/* proceed only if there is no picked_node or object is nearer */
		if (((struct FE_node *)NULL==nearest_node_data->nearest_node)||
			(Scene_picked_object_get_nearest(scene_picked_object) <
				nearest_node_data->nearest))
		{
			/* is the last scene_object a Graphical_element wrapper, and does the
				 settings for the graphic refer to node_points or data_points? */
			if ((NULL != (rendition=Scene_picked_object_get_rendition(
				scene_picked_object,
				Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
				&&(3<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
				(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(
					rendition,Scene_picked_object_get_subobject(scene_picked_object,0))))&&
				(((CMISS_GRAPHIC_NODE_POINTS==
					(graphic_type=Cmiss_graphic_get_graphic_type(graphic)))&&
					(!nearest_node_data->use_data)) ||
					((CMISS_GRAPHIC_DATA_POINTS == graphic_type) &&
						nearest_node_data->use_data)) &&
				(cmiss_region = Cmiss_rendition_get_region(rendition)))
			{
				node_number=Scene_picked_object_get_subobject(scene_picked_object,2);
				fe_region = Cmiss_region_get_FE_region(cmiss_region);
				if (nearest_node_data->use_data)
				{
					fe_region = FE_region_get_data_FE_region(fe_region);
				}
				if (fe_region && (node =
					FE_region_get_FE_node_from_identifier(fe_region, node_number)))
				{
					/* is the node in the nearest_node_data->cmiss_region, if supplied */
					if (nearest_node_data->cmiss_region)
					{
						fe_region = Cmiss_region_get_FE_region(nearest_node_data->cmiss_region);
						if (nearest_node_data->use_data)
						{
							fe_region = FE_region_get_data_FE_region(fe_region);
						}
					}
					if ((!nearest_node_data->cmiss_region) ||
						FE_region_contains_FE_node(fe_region, node))
					{
						nearest_node_data->nearest_node=node;
						nearest_node_data->scene_picked_object=scene_picked_object;
						nearest_node_data->rendition=rendition;
						nearest_node_data->graphic=graphic;
						nearest_node_data->nearest=
							Scene_picked_object_get_nearest(scene_picked_object);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_picked_object_get_nearest_node.  Invalid node %d",
						node_number);
					return_code=0;
				}
			}
			if (graphic)
				Cmiss_graphic_destroy(&graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_nearest_node */

struct Scene_picked_object_get_picked_nodes_data
{
	struct LIST(FE_node) *node_list;
	/* flag set when searching for nearest data point rather than node */
	int use_data;
};

typedef std::multimap<Cmiss_region *, Cmiss_node_id> Region_node_map;

struct Scene_picked_object_region_node_map_data
{
  Region_node_map *node_list;
	/* flag set when searching for nearest data point rather than node */
	int use_data;
};

static int Scene_picked_object_get_picked_region_sorted_nodes(
	struct Scene_picked_object *scene_picked_object,void *picked_nodes_data_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to a node and the node is in the given
manager, ensures it is in the list.
==============================================================================*/
{
	USE_PARAMETER(scene_picked_object);
	USE_PARAMETER(picked_nodes_data_void);

	enum Cmiss_graphic_type graphic_type;
	int node_number,return_code;
	struct Cmiss_region *cmiss_region;
	struct FE_node *node;
	struct FE_region *fe_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_region_node_map_data *picked_nodes_data;


	ENTER(Scene_picked_object_get_picked_nodes);
	if (scene_picked_object&&(picked_nodes_data=
		(struct Scene_picked_object_region_node_map_data	*)picked_nodes_data_void))
	{
		return_code=1;
		/* is the last scene_object a Graphical_element wrapper, and does the
			 settings for the graphic refer to node_points? */
		if ((NULL != (rendition=Scene_picked_object_get_rendition(scene_picked_object,
						Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
			&&(3<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
			(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
				Scene_picked_object_get_subobject(scene_picked_object,0))))&&
			(((CMISS_GRAPHIC_NODE_POINTS ==
					(graphic_type = Cmiss_graphic_get_graphic_type(graphic))) &&
				(!picked_nodes_data->use_data)) ||
			((CMISS_GRAPHIC_DATA_POINTS == graphic_type) &&
				picked_nodes_data->use_data)) &&
			(cmiss_region = Cmiss_rendition_get_region(rendition)))
		{
			node_number=Scene_picked_object_get_subobject(scene_picked_object,2);
			fe_region = Cmiss_region_get_FE_region(cmiss_region);
			if (picked_nodes_data->use_data)
			{
				fe_region = FE_region_get_data_FE_region(fe_region);
			}
			node = FE_region_get_FE_node_from_identifier(fe_region, node_number);
			if (node)
			{
				picked_nodes_data->node_list->insert(std::make_pair(cmiss_region, node));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_picked_object_get_picked_nodes.  Invalid node %d",node_number);
				return_code=0;
			}
		}
		if (graphic)
			Cmiss_graphic_destroy(&graphic);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_picked_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);

	return 1;
} /* Scene_picked_object_get_picked_nodes */

void *Scene_picked_object_list_get_picked_region_sorted_nodes(
	struct LIST(Scene_picked_object) *scene_picked_object_list,int use_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Returns the list of all nodes in the <scene_picked_object_list>. 
The <use_data> flag indicates that we are searching for data points instead of
nodes, needed since different settings type used for each.
==============================================================================*/
{
	struct Scene_picked_object_region_node_map_data picked_nodes_data;

	ENTER(Scene_picked_object_list_get_picked_nodes);
	if (scene_picked_object_list)
	{	
		picked_nodes_data.use_data=use_data;
		picked_nodes_data.node_list=new Region_node_map();
		if (picked_nodes_data.node_list)
		{
			FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
				Scene_picked_object_get_picked_region_sorted_nodes,(void *)&picked_nodes_data,
				scene_picked_object_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_picked_object_list_get_picked_nodes.  "
				"Could not create node list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_picked_nodes.  Invalid argument(s)");
		picked_nodes_data.node_list=NULL;
	}
	LEAVE;

	return ((void *)picked_nodes_data.node_list);
} /* Scene_picked_object_list_get_picked_nodes */

/*
Global functions
----------------
*/

#if defined (OLD_CODE)
PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Scene_graphical_element_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Scene_graphical_element_mode));
	switch (enumerator_value)
	{
		case GRAPHICAL_ELEMENT_EMPTY:
		{
			enumerator_string = "empty_g_element";
		} break;
		case GRAPHICAL_ELEMENT_INVISIBLE:
		{
			enumerator_string = "invisible_g_element";
		} break;
		case GRAPHICAL_ELEMENT_LINES:
		{
			enumerator_string = "g_element_lines";
		} break;
		case GRAPHICAL_ELEMENT_MANUAL:
		{
			enumerator_string = "manual_g_element";
		} break;
		case GRAPHICAL_ELEMENT_NONE:
		{
			enumerator_string = "no_g_element";
		} break;
		default:
		{
			/* up to calling function to handle invalid enumerators */
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Scene_graphical_element_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Scene_graphical_element_mode)
#endif /* defined (OLD_CODE) */

#if defined (USE_SCENE_OBJECT)
DECLARE_OBJECT_FUNCTIONS(Scene_object)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Scene_object)
DECLARE_INDEXED_LIST_FUNCTIONS(Scene_object)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Scene_object, \
	position,int,compare_int)
#endif /* defined (USE_SCENE_OBJECT) */

#if defined (USE_SCENE_OBJECT)
int Scene_object_add_transformation_callback(struct Scene_object *scene_object,
	CMISS_CALLBACK_FUNCTION(Scene_object_transformation) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Adds a callback to <scene_object> for when its transformation is changed.
<function> should have 3 arguments: struct Scene_object *, a gtMatrix * and the
given void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_add_transformation_callback);
	if (scene_object && function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_object_transformation)(
			scene_object->transformation_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_object_add_transformation_callback.  Could not add callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_add_transformation_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_add_transformation_callback */

int Scene_object_remove_transformation_callback(
	struct Scene_object *scene_object,
	CMISS_CALLBACK_FUNCTION(Scene_object_transformation) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Removes the transformation callback calling <function> with <user_data> from
<scene_object>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_remove_transformation_callback);
	if (scene_object && function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_object_transformation)(
			scene_object->transformation_callback_list, function,user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_object_remove_transformation_callback.  "
				"Could not remove callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_remove_transformation_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_remove_transformation_callback */

int Scene_object_has_transformation(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Returns 1 if the <scene_object> has a nonidentity transformation matrix
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_has_transformation);
	if (scene_object)
	{
		if(scene_object->transformation)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_transformation */

int Scene_object_get_transformation(struct Scene_object *scene_object,
	gtMatrix *transformation)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Returns the transformation of <scene_object>.
==============================================================================*/
{
	int i, j, return_code;

	ENTER(Scene_object_get_transformation);
	if (scene_object)
	{
		if(scene_object->transformation)
		{
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					(*transformation)[i][j] = (*scene_object->transformation)[i][j];
				}
			}
		}
		else
		{
			/* Set the identity */
			(*transformation)[0][0] = 1.0;
			(*transformation)[0][1] = 0.0;
			(*transformation)[0][2] = 0.0;
			(*transformation)[0][3] = 0.0;
			(*transformation)[1][0] = 0.0;
			(*transformation)[1][1] = 1.0;
			(*transformation)[1][2] = 0.0;
			(*transformation)[1][3] = 0.0;
			(*transformation)[2][0] = 0.0;
			(*transformation)[2][1] = 0.0;
			(*transformation)[2][2] = 1.0;
			(*transformation)[2][3] = 0.0;
			(*transformation)[3][0] = 0.0;
			(*transformation)[3][1] = 0.0;
			(*transformation)[3][2] = 0.0;
			(*transformation)[3][3] = 1.0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_get_transformation */

int Scene_object_set_transformation(struct Scene_object *scene_object,
	gtMatrix *transformation)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Sets the transformation of <scene_object>.
==============================================================================*/
{
	int i, j, return_code;

	ENTER(Scene_object_set_transformation);
	if (scene_object)
	{
		return_code = 1;
		if ((!transformation) || gtMatrix_is_identity(transformation))
		{
			if (scene_object->transformation)
			{
				DEALLOCATE(scene_object->transformation);
			}
		}
		else
		{
			if (scene_object->transformation)
			{
				if (!gtMatrix_match(transformation, scene_object->transformation))
				{
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							(*scene_object->transformation)[i][j] = (*transformation)[i][j];
						}
					}
				}
			}
			else
			{
				if (ALLOCATE(scene_object->transformation, gtMatrix, 1))
				{
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							(*scene_object->transformation)[i][j] = (*transformation)[i][j];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Scene_object_set_transformation.  "
						"Unable to allocate transformation");
					return_code = 0;				
				}
			}
		}
		CMISS_CALLBACK_LIST_CALL(Scene_object_transformation)(
			scene_object->transformation_callback_list, scene_object,
			scene_object->transformation);
		Scene_object_changed_external(scene_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_set_transformation.  Missing scene_object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_set_transformation */

void Scene_object_remove_time_dependent_transformation(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Remove the time dependent transformation of <scene_object> if it currently exists.
==============================================================================*/
{
	ENTER(Scene_object_remove_time_dependent_transformation);

	if (scene_object->transformation_time_callback_flag)
	{
		 Time_object_remove_callback(scene_object->time_object,
				Scene_object_set_time_dependent_transformation, scene_object);
		 DEACCESS(Computed_field)(&(scene_object->transformation_field));
		 scene_object->transformation_field = NULL;
			 scene_object->transformation_time_callback_flag = 0;
	}

	LEAVE;
}

static int Scene_object_set_time_dependent_transformation(struct Time_object *time_object,
	double current_time, void *scene_object_void)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Transform a scene object with the values of a computed field 
at the provided time.
==============================================================================*/
{
	 int return_code, i, j, k;
	 struct Scene_object *scene_object;
	 FE_value *values;
	 gtMatrix transformation_matrix;

	 ENTER(Scene_object_set_time_dependent_transformation);
	 USE_PARAMETER(time_object);

	 if (scene_object = (struct Scene_object *)scene_object_void)
	 {
			if (scene_object->transformation_field)
			{
				 if (ALLOCATE(values, FE_value, 16))
				 {
						Computed_field_evaluate_without_node(scene_object->transformation_field, 
							 current_time, values);
						k = 0;
						for (i = 0;i < 4; i++)
						{
							 for (j = 0; j < 4; j++)
							 {
									transformation_matrix[i][j] = values[k];
									k++;
							 }
						}
						return_code = Scene_object_set_transformation(scene_object,
							 &transformation_matrix);
						DEALLOCATE(values);
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "Scene_object_set_time_dependent_transformation.  "
							 "Unable to allocate values.");
						return_code=0;
				 }
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"Scene_object_set_time_dependent_transformation.  "
						"Missing transformation field.");
				 return_code=0;
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Scene_object_set_time_dependent_transformation.  "
				 "invalid argument.");
			return_code=0;
	 }
	 LEAVE;

	 return (return_code);
}

int Scene_object_set_transformation_with_time_callback(struct Scene_object *scene_object,
	 struct Computed_field *transformation_field)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Setup the callback for time dependent transformation.
==============================================================================*/
{
	 int return_code;

	 ENTER(Scene_object_set_transformation_with_time_callback);

	 return_code = 0;
	 if (scene_object && transformation_field)
	 {
			if (scene_object->time_object)
			{
				 Scene_object_remove_time_dependent_transformation(scene_object);
				 scene_object->transformation_field=
						ACCESS(Computed_field)(transformation_field);
				 Scene_object_set_time_dependent_transformation(scene_object->time_object,
						Time_object_get_current_time(scene_object->time_object),
						(void *)scene_object);
				 Time_object_add_callback(scene_object->time_object,
						Scene_object_set_time_dependent_transformation, scene_object);
				 scene_object->transformation_time_callback_flag = 1;
				 return_code = 1;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"Scene_object_set_transformation_with_time_callback.  "
						"Missing time object.");
				 return_code=0;
			}
			return_code = scene_object->transformation_time_callback_flag;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Scene_object_set_transformation_with_time_callback.  "
				 "Invalid argument(s).");
			return_code=0;
	 }

	 LEAVE;	

	 return (return_code);
}

int Scene_object_iterator_set_visibility(struct Scene_object *scene_object,
	void *visibility_void)
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
List iterator function for setting the visibility of <scene_object>.
Note: second parameter should be passed as (void *)visibility, where
visibility is of type enum GT_visibility_type.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_iterator_set_visibility);
	if (scene_object)
	{
		scene_object->visibility=static_cast<GT_visibility_type>(
			VOIDPTR2INT(visibility_void));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_iterator_set_visibility.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_iterator_set_visibility */

int Scene_object_has_name(struct Scene_object *scene_object,
	void *name_void)
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> has the 
specified <name>.
==============================================================================*/
{
	char *name;
	int return_code;

	ENTER(Scene_object_has_name);
	if (scene_object&&(name=(char *)name_void))
	{
		return_code=!strcmp(name,scene_object->name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_name */

struct GT_object *Scene_object_get_gt_object(
	struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Returns the GT_object referenced by <scene_object>.
==============================================================================*/
{
	struct GT_object *return_gt_object;

	ENTER(Scene_object_get_gt_object);
	if (scene_object && (SCENE_OBJECT_GRAPHICS_OBJECT==scene_object->type))
	{
		return_gt_object=scene_object->gt_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_gt_object.  Invalid scene_object");
		return_gt_object=(struct GT_object *)NULL;
	}
	LEAVE;

	return (return_gt_object);
} /* Scene_object_get_gt_object */

int Scene_object_set_gt_object(struct Scene_object *scene_object,
	struct GT_object *gt_object)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Changes the GT_object referenced by <scene_object>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_set_gt_object);
	if (scene_object&&gt_object&&
		(SCENE_OBJECT_GRAPHICS_OBJECT==scene_object->type))
	{
		REACCESS(GT_object)(&(scene_object->gt_object), gt_object);
		Scene_object_changed_internal(scene_object);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_gt_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_set_gt_object */

int Scene_object_has_gt_object(struct Scene_object *scene_object,
	void *gt_object_void)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> is of type
SCENE_OBJECT_GRAPHICS_OBJECT and references <gt_object>.  If <gt_object_void>
is NULL then returns true if <scene_object> contains any graphics_object.
==============================================================================*/
{
	int return_code;
	struct GT_object *gt_object;

	ENTER(Scene_object_has_gt_object);
	if (scene_object)
	{
		if (gt_object=(struct GT_object *)gt_object_void)
		{
			return_code = (SCENE_OBJECT_GRAPHICS_OBJECT==scene_object->type)&&
				((scene_object->gt_object)==gt_object);
		}
		else
		{
			return_code = (SCENE_OBJECT_GRAPHICS_OBJECT==scene_object->type)&&
				(scene_object->gt_object);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_gt_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_gt_object */

struct Scene *Scene_object_get_child_scene(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct Scene *return_scene;

	ENTER(Scene_object_get_child_scene);
	if (scene_object && (SCENE_OBJECT_SCENE==scene_object->type))
	{
		return_scene = scene_object->child_scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_child_scene.  Invalid scene_object");
		return_scene = (struct Scene *)NULL;
	}
	LEAVE;

	return (return_scene);
} /* Scene_object_get_child_scene */

int Scene_object_has_child_scene(struct Scene_object *scene_object,
	void *child_scene_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> is of type
SCENE_OBJECT_SCENE and references <child_scene> (or any child_scene if NULL).
==============================================================================*/
{
	int return_code;
	struct Scene *child_scene;

	ENTER(Scene_object_has_child_scene);
	if (scene_object)
	{
		child_scene=(struct Scene *)child_scene_void;
		return_code=((SCENE_OBJECT_SCENE == scene_object->type)&&
			((!child_scene)||(scene_object->child_scene == child_scene)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_child_scene.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_child_scene */

struct Any_object *Scene_object_get_represented_object(
	struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Returns the global object that <scene_object> represents, if any.
==============================================================================*/
{
	struct Any_object *represented_object;

	ENTER(Scene_object_get_represented_object);
	if (scene_object)
	{
		represented_object=scene_object->represented_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_represented_object.  Missing scene_object");
		represented_object=(struct Any_object *)NULL;
	}
	LEAVE;

	return (represented_object);
} /* Scene_object_get_represented_object */

int Scene_object_set_represented_object(struct Scene_object *scene_object,
	struct Any_object *represented_object)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Sets the global object that <scene_object> will represent, if any.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_set_represented_object);
	if (scene_object)
	{
		REACCESS(Any_object)(&(scene_object->represented_object),
			represented_object);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_set_represented_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_set_represented_object */

double Scene_object_get_time(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the actual time referenced by <scene_object>.
==============================================================================*/
{
	double return_time;

	ENTER(Scene_object_get_time);
	if (scene_object)
	{
		if(scene_object->time_object)
		{
			return_time = Time_object_get_current_time(scene_object->time_object);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_object_get_time.  Missing time_object");
			return_time=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_time.  Missing scene_object");
		return_time=0.0;
	}
	LEAVE;

	return (return_time);
} /* Scene_object_get_time */

int Scene_object_has_time(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns 1 if the Scene object has a time dependence, 0 if not
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_has_time);
	if (scene_object)
	{
		if(scene_object->time_object)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_time.  Missing scene_object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_time */

struct Time_object *Scene_object_get_time_object(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the Time_object referenced by <scene_object>.
==============================================================================*/
{
	struct Time_object *return_time;

	ENTER(Scene_object_get_time_object);
	if (scene_object)
	{
		return_time=scene_object->time_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_time_object.  Missing scene_object");
		return_time=(struct Time_object *)NULL;
	}
	LEAVE;

	return (return_time);
} /* Scene_object_get_time_object */

int Scene_object_set_time_object(struct Scene_object *scene_object,
	struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
Changes the Time_object referenced by <scene_object>, if <time> is NULL it frees
the references to the time_object.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_object_set_time_object);
	if (scene_object)
	{
		if ((scene_object->type == SCENE_OBJECT_GRAPHICS_OBJECT)
			|| (scene_object->type == SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP))
		{
			if (scene_object->time_object != time)
			{
				if (scene_object->time_object)
				{
					Time_object_remove_callback(scene_object->time_object,
						Scene_object_time_update_callback, scene_object);
					if (scene_object->scene)
					{
						Time_object_remove_callback(scene_object->time_object,
							Scene_time_update_callback, scene_object->scene);
					}
				}
				REACCESS(Time_object)(&(scene_object->time_object),time);
				if (time)
				{
					Time_object_add_callback(scene_object->time_object,
						Scene_object_time_update_callback, scene_object);
					if (scene_object->scene)
					{
						Time_object_add_callback(scene_object->time_object,
							Scene_time_update_callback, scene_object->scene);
					}
				}
			}
			return_code=1;
				
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_object_set_time_object.  "
				"Cannot associate this scene_object with a time object");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_set_time_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_set_time_object */

enum Scene_object_type Scene_object_get_type(struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the Scene_object_type of <scene_object>.
==============================================================================*/
{
	enum Scene_object_type scene_object_type;

	ENTER(Scene_object_get_type);
	if (scene_object)
	{
		scene_object_type=scene_object->type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_type.  Missing scene_object");
		scene_object_type=SCENE_OBJECT_TYPE_INVALID;
	}
	LEAVE;

	return (scene_object_type);
} /* Scene_object_get_type */

int Scene_object_has_Cmiss_region(struct Scene_object *scene_object,
	void *cmiss_region_void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> contains a
graphical element group for the given <cmiss_region>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *cmiss_region;

	ENTER(Scene_object_has_Cmiss_region);
	if (scene_object && (cmiss_region = (struct Cmiss_region *)cmiss_region_void))
	{
		if (SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP == scene_object->type)
		{
			return_code = (cmiss_region == 
				GT_element_group_get_Cmiss_region(scene_object->gt_element_group));
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_Cmiss_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_Cmiss_region */

int Scene_object_has_graphical_element_group(struct Scene_object *scene_object,
	void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> contains a
gt_ELEMENT_GROUP which matches <gt_element_group_void>. If
<gt_element_group_void> is NULL the function returns true if the scene_object
contains any gt_element_group.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;

	ENTER(Scene_object_has_graphical_element_group);
	if (scene_object)
	{
		if (SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP==scene_object->type)
		{
			if (gt_element_group=(struct GT_element_group *)gt_element_group_void)
			{
				return_code=(scene_object->gt_element_group==gt_element_group);
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_has_graphical_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_has_graphical_element_group */

struct GT_element_group *Scene_object_get_graphical_element_group(
	struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Returns the GT_element_group referenced by <scene_object>.
==============================================================================*/
{
	struct GT_element_group *return_gt_element_group;

	ENTER(Scene_object_get_gt_object);
	if (scene_object && (SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP==scene_object->type))
	{
		return_gt_element_group=scene_object->gt_element_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_gt_element_group.  Missing scene_object");
		return_gt_element_group=(struct GT_element_group *)NULL;
	}
	LEAVE;

	return (return_gt_element_group);
} /* Scene_object_get_gt_element_group */

int Scene_object_get_range(struct Scene_object *scene_object,
	void *graphics_object_range_void)
/*******************************************************************************
LAST MODIFIED : 8 June 2001

DESCRIPTION :
Scene_object list iterator function. If <scene_object> is visible, expands
the <graphics_object_range> to include the range of the linked list of
graphics objects in scene_object.
==============================================================================*/
{
	float coordinates[4],transformed_coordinates[4];
	gtMatrix *transformation;
	int i,j,k,return_code;
	struct Graphics_object_range_struct *graphics_object_range,
		temp_graphics_object_range;
	struct GT_object *graphics_object;
	void *use_range_void;

	ENTER(Scene_object_get_range);
	if (scene_object && (graphics_object_range =
		(struct Graphics_object_range_struct *)graphics_object_range_void))
	{
		/* must first build graphics objects */
		Render_graphics_build_objects renderer;
		renderer.Scene_object_compile(scene_object);
		if (transformation=scene_object->transformation)
		{
			temp_graphics_object_range.first=1;
			use_range_void=(void *)&temp_graphics_object_range;
		}
		else
		{
			use_range_void=graphics_object_range_void;
		}
		if (g_VISIBLE==scene_object->visibility)
		{
			switch(scene_object->type)
			{
				case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
				{
					return_code=for_each_settings_in_GT_element_group(
						scene_object->gt_element_group,
						GT_element_settings_get_visible_graphics_object_range,
						use_range_void);
				} break;
				case SCENE_OBJECT_GRAPHICS_OBJECT:
				{
					return_code=1;
					for (graphics_object=scene_object->gt_object;
						  return_code&&(graphics_object != NULL);
						  graphics_object=GT_object_get_next_object(graphics_object))
					{
						return_code=
							get_graphics_object_range(graphics_object,use_range_void);
					}
				} break;
				case SCENE_OBJECT_SCENE:
				{
					return_code=for_each_Scene_object_in_Scene(scene_object->child_scene,
						Scene_object_get_range,use_range_void);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Scene_object_get_range.  Unknown scene object type");
					return_code=0;
				} break;
			}
		}
		else
		{
			return_code = 1;
		}
		if (return_code&&transformation&&(!temp_graphics_object_range.first))
		{
			coordinates[3]=1.0;
			/* transform and compare ranges of each of 8 corners of the cube */
			for (i=0;i<8;i++)
			{
				if (i & 1)
				{
					coordinates[0]=temp_graphics_object_range.maximum[0];
				}
				else
				{
					coordinates[0]=temp_graphics_object_range.minimum[0];
				}
				if (i & 2)
				{
					coordinates[1]=temp_graphics_object_range.maximum[1];
				}
				else
				{
					coordinates[1]=temp_graphics_object_range.minimum[1];
				}
				if (i & 4)
				{
					coordinates[2]=temp_graphics_object_range.maximum[2];
				}
				else
				{
					coordinates[2]=temp_graphics_object_range.minimum[2];
				}
				for (j=0;j<4;j++)
				{
					transformed_coordinates[j]=0.0;
					for (k=0;k<4;k++)
					{
						transformed_coordinates[j] +=
							(*transformation)[k][j]*coordinates[k];
					}
				}
				if (0.0<transformed_coordinates[3])
				{
					transformed_coordinates[0] /= transformed_coordinates[3];
					transformed_coordinates[1] /= transformed_coordinates[3];
					transformed_coordinates[2] /= transformed_coordinates[3];
					for (j=0;j<3;j++)
					{
						if (graphics_object_range->first)
						{
							graphics_object_range->minimum[j]=
								graphics_object_range->maximum[j]=transformed_coordinates[j];
						}
						else
						{
							if (transformed_coordinates[j] >
								graphics_object_range->maximum[j])
							{
								graphics_object_range->maximum[j]=transformed_coordinates[j];
							}
							else if (transformed_coordinates[j] <
								graphics_object_range->minimum[j])
							{
								graphics_object_range->minimum[j]=transformed_coordinates[j];
							}
						}
					}
					graphics_object_range->first=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_get_range */

int Scene_object_get_time_range(struct Scene_object *scene_object,
	void *graphics_object_time_range_void)
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Scene_object list iterator function. Enlarges the minimum and maximum time
ranges by those of the graphics_objects contained in the <scene_object>.
==============================================================================*/
{
	int return_code;
	struct GT_object *graphics_object;

	ENTER(Scene_object_get_time_range);
	if (scene_object&&scene_object->gt_object)
	{
		return_code=1;
		for (graphics_object=scene_object->gt_object;(graphics_object != NULL)&&
			return_code;graphics_object=GT_object_get_next_object(graphics_object))
		{
			return_code=get_graphics_object_time_range(graphics_object,
				graphics_object_time_range_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_time_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_get_time_range */

int list_Scene_object(struct Scene_object *scene_object,void *dummy)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Iterator function called by list_Scene. Writes the scene object position,
name, visibility and information about the object in it to the command window.
???RC list transformation? Have separate gfx list transformation commands.
==============================================================================*/
{
	char *object_name, *time_object_name;
	int return_code;

	ENTER(list_Scene_object);
	USE_PARAMETER(dummy);
	if (scene_object)
	{
		return_code=1;
		/* four spaces before position to align with list_Scene */
		display_message(INFORMATION_MESSAGE,"    %d. %s",scene_object->position,
			scene_object->name);
		if (g_VISIBLE != scene_object->visibility)
		{
			display_message(INFORMATION_MESSAGE," [INVISIBLE]");
		}
		switch (scene_object->type)
		{
			case SCENE_OBJECT_GRAPHICS_OBJECT:
			{
				if (scene_object->gt_object && 
					GET_NAME(GT_object)(scene_object->gt_object, &object_name))
				{
					display_message(INFORMATION_MESSAGE," = graphics object %s",
						object_name);
					DEALLOCATE(object_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"list_Scene_object.  Invalid gt_object");
					return_code=0;
				}
			} break;
			case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
			{
				display_message(INFORMATION_MESSAGE,
					" = graphical finite element group");
			} break;
			case SCENE_OBJECT_SCENE:
			{
				if (scene_object->child_scene&&scene_object->child_scene->name)
				{
					display_message(INFORMATION_MESSAGE," = child scene %s",
						scene_object->child_scene->name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"list_Scene_object.  Invalid child scene");
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"list_Scene_object.  Invalid Scene_object_type");
				return_code=0;
			} break;
		}
		if (scene_object->time_object)
		{
			if (GET_NAME(Time_object)(scene_object->time_object,&time_object_name))
			{
				display_message(INFORMATION_MESSAGE,"; time object %s\n",
					time_object_name);
				DEALLOCATE(time_object_name);
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"; no time object\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Scene_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Scene_object */

int list_Scene_object_transformation(struct Scene_object *scene_object,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Iterator function for writing the transformation in effect for <scene_object>
in an easy-to-interpret matrix multiplication form.
==============================================================================*/
{
	const char *coordinate_symbol="xyzh";
	int i,return_code;
	gtMatrix transformation_matrix;

	ENTER(list_Scene_object_transformation);
	USE_PARAMETER(dummy);
	if (scene_object)
	{
		if (return_code=Scene_object_get_transformation(scene_object,
			&transformation_matrix))
		{
			display_message(INFORMATION_MESSAGE,"%d. %s transformation:\n",
				scene_object->position,scene_object->name);
			for (i=0;i<4;i++)
			{
				display_message(INFORMATION_MESSAGE,
					"  |%c.out| = | %13.6e %13.6e %13.6e %13.6e | . |%c.in|\n",
					coordinate_symbol[i],
					transformation_matrix[0][i],transformation_matrix[1][i],
					transformation_matrix[2][i],transformation_matrix[3][i],
					coordinate_symbol[i]);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Scene_object_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Scene_object_transformation */

int list_Scene_object_transformation_commands(struct Scene_object *scene_object,
	void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Iterator function for writing the transformation in effect for <scene_object>
as a command, using the given <command_prefix>.
==============================================================================*/
{
	char *command_prefix, *scene_object_name;
	int i,j,return_code;
	gtMatrix transformation_matrix;
 
	ENTER(list_Scene_object_transformation_commands);
	if (scene_object&&(command_prefix=(char *)command_prefix_void))
	{
		if (return_code=Scene_object_get_transformation(scene_object,
			&transformation_matrix))
		{
			scene_object_name = duplicate_string(scene_object->name);
			/* put quotes around name if it contains special characters */
			make_valid_token(&scene_object_name);
			display_message(INFORMATION_MESSAGE, "%s %s", command_prefix,
				scene_object_name);
			DEALLOCATE(scene_object_name);
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					display_message(INFORMATION_MESSAGE," %g",
						(transformation_matrix)[i][j]);
				}
			}
			display_message(INFORMATION_MESSAGE,";\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Scene_object_transformation_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Scene_object_transformation_commands */
#endif /* defined (USE_SCENE_OBJECT) */

struct Scene *CREATE(Scene)(void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Scene now has pointer to its scene_manager, and it uses manager modify
messages to inform its clients of changes. The pointer to the scene_manager
is set and unset by the add/remove object from manager routines, overwritten
from the default versions of these functions.
==============================================================================*/
{
	struct Scene *scene;

	ENTER(CREATE(Scene));
	ALLOCATE(scene,struct Scene,1);
	if (scene)
	{
		/* assign values to the fields */
		scene->name = NULL;
		scene->change_status=SCENE_NO_CHANGE;
		scene->access_count = 1;
		scene->scene_manager=(struct MANAGER(Scene) *)NULL;
		/* fields, elements, nodes and data */
		scene->region = NULL;
		scene->light_manager=(struct MANAGER(Light) *)NULL;
		scene->light_manager_callback_id=(void *)NULL;
		scene->list_of_lights=CREATE(LIST(Light))();
		scene->list_of_rendition = NULL;
#if defined (OLD_CODE)
		/*???RC temporary; have root_region and data_root_region until Scenes are
			incorporated into the regions themselves */
		scene->root_region = (struct Cmiss_region *)NULL;
		/* defaults to not adding GFEs - besides, need managers anyway */
		scene->graphical_element_mode=GRAPHICAL_ELEMENT_NONE;
		/* global stores of selected objects */
		scene->element_point_ranges_selection=
			(struct Element_point_ranges_selection *)NULL;
		scene->element_selection=(struct FE_element_selection *)NULL;
		scene->node_selection=(struct FE_node_selection *)NULL;
		scene->data_selection=(struct FE_node_selection *)NULL;
		/* attributes: */
		scene->glyph_manager=(struct MANAGER(GT_object) *)NULL;
		scene->graphical_material_manager=
			(struct MANAGER(Graphical_material) *)NULL;
		scene->graphical_material_manager_callback_id=(void *)NULL;
		scene->default_material=(struct Graphical_material *)NULL;
		scene->default_font = (struct Graphics_font *)NULL;
		scene->spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
		scene->spectrum_manager_callback_id=(void *)NULL;
		scene->default_spectrum=(struct Spectrum *)NULL;
		scene->texture_manager=(struct MANAGER(Texture) *)NULL;
		scene->default_time_keeper=(struct Time_keeper *)NULL;
		scene->user_interface=(struct User_interface *)NULL;
#endif /* defined (OLD_CODE) */
		scene->filters = new Scene_filter_vector();
		scene->cache = 0;
		scene->build = 1;
		/* display list index and current flag: */
		scene->display_list = 0;
		scene->fast_changing_display_list = 0;
		scene->ndc_display_list = 0;
		scene->end_ndc_display_list = 0;
		scene->compile_status = GRAPHICS_NOT_COMPILED;
		/* input callback handling information: */
		scene->input_callback.procedure=(Scene_input_callback_procedure *)NULL;
		scene->input_callback.data=(void *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Scene).  Not enough memory");
	}
	LEAVE;

	return (scene);
} /* CREATE(Scene) */

int DESTROY(Scene)(struct Scene **scene_address)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Closes the scene and disposes of the scene data structure.
==============================================================================*/
{
	int return_code;
	struct Scene *scene;

	ENTER(DESTROY(Scene));
	if (scene_address && (scene = *scene_address))
	{
		if (0==scene->access_count)
		{
			/* cache should be back to zero by now */
			if (0 != scene->cache)
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(Scene).  scene->cache = %d != 0", scene->cache);
			}
			/* mark the cache as on so no messages go out again */
			(scene->cache)++;
#if defined (OLD_CODE)
			Scene_disable_time_behaviour(scene);
			Scene_set_graphical_element_mode(scene,
				GRAPHICAL_ELEMENT_NONE,
				(struct Cmiss_region *)NULL,
				(struct Element_point_ranges_selection *)NULL,
				(struct FE_element_selection *)NULL,
				(struct FE_node_selection *)NULL,
				(struct FE_node_selection *)NULL,
				(struct User_interface *)NULL);
			Scene_disable_graphics(scene);
#endif /* defined (OLD_CODE) */
			DEALLOCATE(scene->name);
			/* must destroy the display list */
			if (scene->display_list)
			{
				glDeleteLists(scene->display_list,1);
			}
			if (scene->fast_changing_display_list)
			{
				glDeleteLists(scene->fast_changing_display_list,1);
			}
			if (scene->ndc_display_list)
			{
				glDeleteLists(scene->ndc_display_list,1);
			}
			if (scene->end_ndc_display_list)
			{
				glDeleteLists(scene->end_ndc_display_list,1);
			}
			if (scene->list_of_rendition)
			{
				if (!scene->list_of_rendition->empty())
				{
					Rendition_set::iterator pos =
						scene->list_of_rendition->begin();
					while (pos != scene->list_of_rendition->end())
					{
						Cmiss_rendition_remove_scene(*pos, scene);
						/* the following function is to remove any field
						 * being used by rendition and its graphics as cross
						 * referecing from field to region may cause meomoryleak.
						 */
						Cmiss_rendition_detach_fields(*pos);
						++pos;
					}
				}
				delete scene->list_of_rendition;
				scene->list_of_rendition = NULL;
			}
			Cmiss_scene_clear_filters(scene);
			delete scene->filters;
			if (scene->region)
			{
			  DEACCESS(Cmiss_region)(&scene->region);		  
			}
#if defined (USE_SCENE_OBJECT)
			DESTROY(LIST(Scene_object))(&(scene->scene_object_list));
#endif /* defined (USE_SCENE_OBJECT) */
			DESTROY(LIST(Light))(&(scene->list_of_lights));
			DEALLOCATE(*scene_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Scene).  Non-zero access count!");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Scene).  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Scene) */

int Scene_begin_cache(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Call before making several changes to the scene so only a single change message
is sent. Call Scene_end_cache at the end of the changes.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_begin_cache);
	if (scene)
	{
		/* increment cache to allow nesting */
		(scene->cache)++;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_begin_cache.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_begin_cache */

int Scene_end_cache(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 6 November 2001

DESCRIPTION :
Call after making changes preceded by a call to Scene_begin_cache to enable a
final message to be sent to clients.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_end_cache);
	if (scene)
	{
		if (0 < scene->cache)
		{
			/* decrement cache to allow nesting */
			(scene->cache)--;
			/* once cache has run out, inform clients of all changes to date */
			if (0 == scene->cache)
			{
				if (scene->scene_manager)
				{
					Scene_refresh(scene);
				}
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_end_cache.  Caching is already disabled");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_end_cache.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_end_cache */

#if defined (OLD_CODE)
int Scene_enable_graphics(struct Scene *scene,
	struct MANAGER(GT_object) *glyph_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 24 November 2005

DESCRIPTION :
The scene is initially incapable of generating any graphics, since it does not
have access to the material, light and spectrum managers. This routine must be
called soon after creating the scene to give the scene these managers.
Do not have to call this routine if MANAGER_COPY_WITHOUT_IDENTIFIER is used to
create a scene from an existing scene with graphics enabled.
NOTE: The light_manager is not currently used by the scene.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_enable_graphics);
	if (scene&&glyph_manager&&graphical_material_manager&&default_material&&
		default_font&&light_manager&&spectrum_manager&&default_spectrum&&texture_manager)
	{
		if (scene->graphical_material_manager)
		{
			display_message(WARNING_MESSAGE,
				"Scene_enable_graphics.  Graphics already enabled");
		}
		else
		{
			scene->glyph_manager=glyph_manager;
			scene->graphical_material_manager=graphical_material_manager;
			scene->default_material=ACCESS(Graphical_material)(default_material);
			scene->default_font=ACCESS(Graphics_font)(default_font);
			scene->light_manager=light_manager;
			scene->spectrum_manager=spectrum_manager;
			scene->default_spectrum=ACCESS(Spectrum)(default_spectrum);
			scene->texture_manager=texture_manager;
			/* register for any graphical_material changes */
			scene->graphical_material_manager_callback_id=
				MANAGER_REGISTER(Graphical_material)(Scene_Graphical_material_change,
				(void *)scene,scene->graphical_material_manager);
			/* register for any spectrum changes */
			scene->spectrum_manager_callback_id=
				MANAGER_REGISTER(Spectrum)(Scene_Spectrum_change,
				(void *)scene,scene->spectrum_manager);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_enable_graphics.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_enable_graphics */

int Scene_disable_graphics(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 5 June 2001

DESCRIPTION :
Removes links to all objects required to display graphics.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_disable_graphics);
	if (scene)
	{
		/* remove all Scene_objects */
#if defined (USE_SCENE_OBJECT)
		REMOVE_ALL_OBJECTS_FROM_LIST(Scene_object)(scene->scene_object_list);
#endif /* defined (USE_SCENE_OBJECT) */
		/* turn off manager messages */
		if (scene->graphical_material_manager_callback_id)
		{
			MANAGER_DEREGISTER(Graphical_material)(
				scene->graphical_material_manager_callback_id,
				scene->graphical_material_manager);
		}
		if (scene->spectrum_manager_callback_id)
		{
			MANAGER_DEREGISTER(Spectrum)(
				scene->spectrum_manager_callback_id,scene->spectrum_manager);
		}
		if (scene->default_material)
		{
			DEACCESS(Graphical_material)(&(scene->default_material));
		}
		if (scene->default_font)
		{
			DEACCESS(Graphics_font)(&(scene->default_font));
		}
		if (scene->default_spectrum)
		{
			DEACCESS(Spectrum)(&(scene->default_spectrum));
		}
		scene->graphical_material_manager=
			(struct MANAGER(Graphical_material) *)NULL;
		scene->graphical_material_manager_callback_id=(void *)NULL;
		scene->default_material=(struct Graphical_material *)NULL;
		scene->light_manager=(struct MANAGER(Light) *)NULL;
		scene->light_manager_callback_id=(void *)NULL;
		scene->spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
		scene->spectrum_manager_callback_id=(void *)NULL;
		scene->default_spectrum=(struct Spectrum *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_disable_graphics.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_disable_graphics */

int Scene_enable_time_behaviour(struct Scene *scene,
	struct Time_keeper *default_time_keeper)
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
The scene is initially incapable of varying objects with time as it has no
time_keeper to put time_objects into.
Do not have to call this routine if MANAGER_COPY_WITHOUT_IDENTIFIER is used to
create a scene from an existing scene with graphics enabled.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_enable_graphics);
	if (scene&&default_time_keeper)
	{
		if (scene->default_time_keeper)
		{
			display_message(WARNING_MESSAGE,
				"Scene_enable_time_behaviour.  Time already enabled");
		}
		else
		{
			scene->default_time_keeper = ACCESS(Time_keeper)(default_time_keeper);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_enable_time_behaviour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_enable_time_behaviour */

struct Time_keeper *Scene_get_default_time_keeper(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
==============================================================================*/
{
	struct Time_keeper *return_time_keeper;

	ENTER(Scene_get_default_time_keeper);
	if (scene)
	{
		return_time_keeper = scene->default_time_keeper;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_default_time_keeper.  Invalid argument(s)");
		return_time_keeper = (struct Time_keeper *)NULL;
	}
	LEAVE;

	return (return_time_keeper);
} /* Scene_get_default_time_keeper */

int Scene_disable_time_behaviour(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Removes links to all objects required to vary graphics objects with time.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_disable_time_behaviour);
	if (scene)
	{
		if (scene->default_time_keeper)
		{
			DEACCESS(Time_keeper)(&(scene->default_time_keeper));
		}
		scene->default_time_keeper=(struct Time_keeper *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_disable_time_behaviour.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_disable_time_behaviour */

enum Scene_graphical_element_mode Scene_get_graphical_element_mode(
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 16 March 2001

DESCRIPTION :
Returns the mode controlling how graphical element groups are displayed in the
scene.
==============================================================================*/
{
	enum Scene_graphical_element_mode graphical_element_mode;

	ENTER(Scene_get_graphical_element_mode);
	if (scene)
	{
		graphical_element_mode = scene->graphical_element_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_graphical_element_mode.  Invalid argument");
		graphical_element_mode = GRAPHICAL_ELEMENT_NONE;
	}
	LEAVE;

	return (graphical_element_mode);
} /* enum Scene_graphical_element_mode graphical_element_mode */

int Scene_set_graphical_element_mode(struct Scene *scene,
	enum Scene_graphical_element_mode graphical_element_mode,
	struct Cmiss_region *root_region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Sets the mode controlling how graphical element groups are displayed in the
scene. Passes the managers and other data required to create and update the
graphical elements.
Must be called after Scene_enable_graphics since GFEs require the default
material and spectrum.
==============================================================================*/
{
	int return_code;

	USE_PARAMETER(data_selection);
	USE_PARAMETER(user_interface);
#if defined (USE_SCENE_OBJECT)
	struct Scene_object *scene_object;
#endif /* (USE_SCENE_OBJECT) */

	ENTER(Scene_set_graphical_element_mode);
	if (scene && ((GRAPHICAL_ELEMENT_NONE == graphical_element_mode) || (
		root_region && element_point_ranges_selection && 
		element_selection && node_selection)))
	{
		return_code = 1;
		Scene_begin_cache(scene);
		if (GRAPHICAL_ELEMENT_NONE == graphical_element_mode)
		{
			/* remove all graphical elements from scene */
#if defined (USE_SCENE_OBJECT)
			while (return_code && (scene_object =
				FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
					Scene_object_has_graphical_element_group,(void *)NULL,
					scene->scene_object_list)))
			{
				return_code = Scene_remove_Scene_object_private(scene, scene_object);
			}
#endif /* (USE_SCENE_OBJECT) */
			if (return_code)
			{
				if (GRAPHICAL_ELEMENT_NONE != scene->graphical_element_mode)
				{
					#if defined (TO_BE_EDITED)
					Cmiss_region_remove_callback(scene->root_region,
						Scene_Cmiss_region_change, (void *)scene);
					#endif
				}
				scene->graphical_element_mode = graphical_element_mode;
				scene->root_region = (struct Cmiss_region *)NULL;
				scene->element_point_ranges_selection=
					(struct Element_point_ranges_selection *)NULL;
				scene->element_selection=(struct FE_element_selection *)NULL;
				scene->node_selection=(struct FE_node_selection *)NULL;
				scene->data_selection=(struct FE_node_selection *)NULL;
				scene->user_interface=(struct User_interface *)NULL;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Scene_set_graphical_element_mode.  "
					"Could not remove graphical elements");
			}
		}
		else
		{
			/* check managers consistent current mode - unless this is
				 GRAPHICAL_ELEMENT_NONE so setting for the first time */
			if ((GRAPHICAL_ELEMENT_NONE == scene->graphical_element_mode) || (
				(root_region == scene->root_region)))
			{
				if (GRAPHICAL_ELEMENT_NONE == scene->graphical_element_mode)
				{
					if (GRAPHICAL_ELEMENT_MANUAL != graphical_element_mode)
					{
						/* ensure we have a graphical element for each child region */
						Scene_update_graphical_element_groups(scene);
					}
					/* add region callbacks */
#if defined (TO_BE_EDITED)
					Cmiss_region_add_callback(root_region,
						Scene_Cmiss_region_change, (void *)scene);
#endif
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_set_graphical_element_mode.  Inconsistent managers");
				return_code=0;
			}
		}
		Scene_end_cache(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_graphical_element_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_graphical_element_mode */
#endif /* defined (OLD_CODE) */

DECLARE_OBJECT_FUNCTIONS(Scene)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Scene)
DECLARE_LIST_FUNCTIONS(Scene)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene,name,const char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Scene,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Scene,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Scene,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Scene,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Scene,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Scene,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Scene,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Scene,name)
{
	int return_code;
// 	struct LIST(Light) *temp_list_of_lights;
#if defined (USE_SCENE_OBJECT)
	struct LIST(Scene_object) *temp_scene_object_list;
#endif /* defined (USE_SCENE_OBJECT) */

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name));
	if (source && destination)
	{
#if defined (OLD_CODE)
		Scene_disable_graphics(destination);
		if (source->graphical_material_manager)
		{
			Scene_enable_graphics(destination,source->glyph_manager,
				source->graphical_material_manager,source->default_material,source->default_font,
				source->light_manager,source->spectrum_manager,source->default_spectrum,
				source->texture_manager);
		}
		if (source->default_time_keeper)
		{
			Scene_enable_time_behaviour(destination,
				source->default_time_keeper);
		}
		Scene_set_graphical_element_mode(destination,source->graphical_element_mode,
			source->root_region, source->element_point_ranges_selection,
			source->element_selection,source->node_selection,source->data_selection,
			source->user_interface);
#endif /* defined (OLD_CODE) */
		/* copy list of lights to destination */
		/* duplicate each scene_object in source and put in destination list */
#if defined (USE_SCENE_OBJECT)
		if ((temp_list_of_lights=CREATE(LIST(Light))())&&
			(temp_scene_object_list=CREATE(LIST(Scene_object))())&&
			(FOR_EACH_OBJECT_IN_LIST(Light)(Light_to_list,
				(void *)temp_list_of_lights,source->list_of_lights))&&
			(FOR_EACH_OBJECT_IN_LIST(Scene_object)(Scene_object_copy_to_list,
				(void *)temp_scene_object_list,source->scene_object_list)))
		{
			DESTROY(LIST(Light))(&(destination->list_of_lights));
			destination->list_of_lights=temp_list_of_lights;
			DESTROY(LIST(Scene_object))(&(destination->scene_object_list));
			destination->scene_object_list=temp_scene_object_list;
			/* NOTE: MUST NOT COPY SCENE_MANAGER! */
			destination->compile_status = GRAPHICS_NOT_COMPILED;
			return_code=1;
		}
		else
		{
			if (temp_list_of_lights)
			{
				DESTROY(LIST(Light))(&temp_list_of_lights);
				if (temp_scene_object_list)
				{
					DESTROY(LIST(Scene_object))(&temp_scene_object_list);
				}
			}
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name).  Could not copy lists");
			return_code=0;
		}
#endif /* defined (USE_SCENE_OBJECT) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Scene,name,const char *)
{
	const char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Scene,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy((char *)destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Scene,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Scene,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Scene,name) */

DECLARE_MANAGER_FUNCTIONS(Scene,scene_manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Scene,scene_manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS( \
	Scene,name,const char *,scene_manager)

#if defined (USE_SCENE_OBJECT)
int for_each_Scene_object_in_Scene(struct Scene *scene,
	LIST_ITERATOR_FUNCTION(Scene_object) *iterator_function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 23 December 1997

DESCRIPTION :
Allows clients of the <scene> to perform functions with the scene_objects in
it. For example, rendervrml.c needs to output all the window objects in a scene.
==============================================================================*/
{
	int return_code;

	ENTER(for_each_Scene_object_in_Scene);
	if (scene&&iterator_function)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(Scene_object)(iterator_function,
			user_data,scene->scene_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_Scene_object_in_Scene.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_each_Scene_object_in_Scene */
#endif /* defined (USE_SCENE_OBJECT) */

int Scene_for_each_material(struct Scene *scene,
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 3 May 2005

DESCRIPTION :
Iterates through every material used by the scene.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Scene_for_each_material);
#if defined (OLD_CODE)
	if (scene && iterator_function && scene->graphical_material_manager)
	{
		/* Could be smarter if there was a reduced number used by the 
			scene, however for now just do every material in the manager */
		return_code = FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
			iterator_function, user_data,scene->graphical_material_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_for_each_material.  Invalid arguments.");
		return_code=0;
	}
#else
	if (scene && iterator_function && scene->region)
	{
		/* Could be smarter if there was a reduced number used by the 
			scene, however for now just do every material in the manager */
		struct Cmiss_rendition *rendition =
			Cmiss_region_get_rendition_internal(scene->region);
		if (rendition)
		{
			return_code = Cmiss_rendition_for_each_material(rendition, iterator_function,
				user_data);
			DEACCESS(Cmiss_rendition)(&rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_for_each_material.  Invalid arguments.");
		return_code=0;
	}
#endif
	LEAVE;

	return (return_code);
} /* Scene_for_each_material */

struct Scene_picked_object *CREATE(Scene_picked_object)(int hit_no)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Creates a Scene_picked_object for storing picking information in a format
compatible with objects in our display hierarchy. Creates a blank object that
must be filled with appropriate data.
==============================================================================*/
{
	struct Scene_picked_object *scene_picked_object;

	ENTER(CREATE(Scene_picked_object));
	if (ALLOCATE(scene_picked_object,struct Scene_picked_object,1))
	{
		scene_picked_object->hit_no=hit_no;
		scene_picked_object->number_of_renditions=0;
		scene_picked_object->renditions=(struct Cmiss_rendition **)NULL;
		scene_picked_object->number_of_subobjects=0;
		scene_picked_object->subobjects=(int *)NULL;
		scene_picked_object->nearest=0;
		scene_picked_object->farthest=0;
		scene_picked_object->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Scene_picked_object).  Not enough memory");
	}
	LEAVE;

	return (scene_picked_object);
} /* CREATE(Scene_picked_object) */

int DESTROY(Scene_picked_object)(
	struct Scene_picked_object **scene_picked_object_address)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Destroys the Scene_picked_object.
==============================================================================*/
{
	int i,return_code;
	struct Scene_picked_object *scene_picked_object;

	ENTER(DESTROY(Scene_picked_object));
	if (scene_picked_object_address&&
		(scene_picked_object= *scene_picked_object_address))
	{
		if (0==scene_picked_object->access_count)
		{
			for (i=0;i<scene_picked_object->number_of_renditions;i++)
			{
				Cmiss_rendition_destroy(&(scene_picked_object->renditions[i]));
			}
			DEALLOCATE(scene_picked_object->renditions);
			DEALLOCATE(scene_picked_object->subobjects);
			DEALLOCATE(*scene_picked_object_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Scene_picked_object).  Non-zero access count!");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Scene_picked_object).  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Scene_picked_object) */

int Scene_picked_object_add_rendition(
	struct Scene_picked_object *scene_picked_object,
	struct Cmiss_rendition *rendition)
{
	int return_code;
	struct Cmiss_rendition **temp_renditions;

	ENTER(Scene_picked_object_add_rendition);
	if (scene_picked_object&&rendition)
	{
		if (REALLOCATE(temp_renditions,scene_picked_object->renditions,
			struct Cmiss_rendition *,scene_picked_object->number_of_renditions+1))
		{
			scene_picked_object->renditions = temp_renditions;
			scene_picked_object->
				renditions[scene_picked_object->number_of_renditions]=
				ACCESS(Cmiss_rendition)(rendition);
			scene_picked_object->number_of_renditions++;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_picked_object_add_rendition.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_add_rendition.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_add_rendition */



#if defined (USE_SCENE_OBJECT)
int Scene_picked_object_add_Scene_object(
	struct Scene_picked_object *scene_picked_object,
	struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Adds the <scene_object> to the end of the list specifying the path to the
picked graphic represented by the <scene_picked_object>.
==============================================================================*/
{
	int return_code;
	struct Scene_object **temp_scene_objects;

	ENTER(Scene_picked_object_add_Scene_object);
	if (scene_picked_object&&scene_object)
	{
		if (REALLOCATE(temp_scene_objects,scene_picked_object->scene_objects,
			struct Scene_object *,scene_picked_object->number_of_scene_objects+1))
		{
			scene_picked_object->scene_objects = temp_scene_objects;
			scene_picked_object->
				scene_objects[scene_picked_object->number_of_scene_objects]=
				ACCESS(Scene_object)(scene_object);
			scene_picked_object->number_of_scene_objects++;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_picked_object_add_Scene_object.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_add_Scene_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_add_Scene_object */

int Scene_picked_object_get_number_of_scene_objects(
	struct Scene_picked_object *scene_picked_object)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the number of scene objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/
{
	int number_of_scene_objects;

	ENTER(Scene_picked_object_get_number_of_scene_objects);
	if (scene_picked_object)
	{
		number_of_scene_objects = scene_picked_object->number_of_scene_objects;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_number_of_scene_objects.  Invalid argument(s)");
		number_of_scene_objects=0;
	}
	LEAVE;

	return (number_of_scene_objects);
} /* Scene_picked_object_get_number_of_scene_objects */

struct Scene_object *Scene_picked_object_get_Scene_object(
	struct Scene_picked_object *scene_picked_object,int scene_object_no)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the scene_object at position <scene_object_no> - where 0 is the first -
in the list of scene_objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/
{
	struct Scene_object *scene_object;

	ENTER(Scene_picked_object_get_Scene_object);
	if (scene_picked_object&&(0<=scene_object_no)&&
		(scene_object_no<scene_picked_object->number_of_scene_objects))
	{
		scene_object = scene_picked_object->scene_objects[scene_object_no];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_Scene_object.  Invalid argument(s)");
		scene_object=(struct Scene_object *)NULL;
	}
	LEAVE;

	return (scene_object);
} /* Scene_picked_object_get_Scene_object */
#endif /* defined (USE_SCENE_OBJECT) */


int Scene_picked_object_get_number_of_renditions(
	struct Scene_picked_object *scene_picked_object)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the number of scene objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/
{
	int number_of_renditions;

	ENTER(Scene_picked_object_get_number_of_scene_objects);
	if (scene_picked_object)
	{
		number_of_renditions = scene_picked_object->number_of_renditions;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_number_of_scene_objects.  Invalid argument(s)");
		number_of_renditions=0;
	}
	LEAVE;

	return (number_of_renditions);
} /* Scene_picked_object_get_number_of_scene_objects */

struct Cmiss_rendition *Scene_picked_object_get_rendition(
	struct Scene_picked_object *scene_picked_object,int rendition_no)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the scene_object at position <scene_object_no> - where 0 is the first -
in the list of scene_objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/
{
	struct Cmiss_rendition *rendition;

	ENTER(Scene_picked_object_get_rendition);
	if (scene_picked_object&&(0<=rendition_no)&&
		(rendition_no<scene_picked_object->number_of_renditions))
	{
		rendition = scene_picked_object->renditions[rendition_no];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_rendition.  Invalid argument(s)");
		rendition=(struct Cmiss_rendition *)NULL;
	}
	LEAVE;

	return (rendition);
} /* Scene_picked_object_get_rendition */

int Scene_picked_object_add_subobject(
	struct Scene_picked_object *scene_picked_object,int subobject)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Adds the <subobject> name to the end of the list of names identifying the
particular picked graphic represented by the <scene_picked_object>.
==============================================================================*/
{
	int return_code,*temp_subobjects;

	ENTER(Scene_picked_object_add_subobject);
	if (scene_picked_object)
	{
		if (REALLOCATE(temp_subobjects,scene_picked_object->subobjects,
			int,scene_picked_object->number_of_subobjects+1))
		{
			scene_picked_object->subobjects = temp_subobjects;
			scene_picked_object->subobjects[scene_picked_object->number_of_subobjects]
				= subobject;
			scene_picked_object->number_of_subobjects++;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_picked_object_add_subobject.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_add_subobject.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_add_subobject */

int Scene_picked_object_get_number_of_subobjects(
	struct Scene_picked_object *scene_picked_object)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the number of integer subobject names identifying the
<scene_picked_object>.
==============================================================================*/
{
	int number_of_subobjects;

	ENTER(Scene_picked_object_get_number_of_subobjects);
	if (scene_picked_object)
	{
		number_of_subobjects = scene_picked_object->number_of_subobjects;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_number_of_subobjects.  Invalid argument(s)");
		number_of_subobjects=0;
	}
	LEAVE;

	return (number_of_subobjects);
} /* Scene_picked_object_get_number_of_subobjects */

int Scene_picked_object_get_subobject(
	struct Scene_picked_object *scene_picked_object,int subobject_no)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the subobject at position <subobject_no> - where 0 is the first - in
the list of integer subobject names identifying the <scene_picked_object>.
==============================================================================*/
{
	int subobject;

	ENTER(Scene_picked_object_get_subobject);
	if (scene_picked_object&&(0<=subobject_no)&&
		(subobject_no<scene_picked_object->number_of_subobjects))
	{
		subobject = scene_picked_object->subobjects[subobject_no];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_subobject.  Invalid argument(s)");
		subobject=0;
	}
	LEAVE;

	return (subobject);
} /* Scene_picked_object_get_subobject */

double Scene_picked_object_get_farthest(
	struct Scene_picked_object *scene_picked_object)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the <farthest> position at which the <scene_picked_object> was picked.
==============================================================================*/
{
	double farthest;

	ENTER(Scene_picked_object_get_farthest);
	if (scene_picked_object)
	{
		farthest = scene_picked_object->farthest;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_farthest.  Invalid argument(s)");
		farthest=0.0;
	}
	LEAVE;

	return (farthest);
} /* Scene_picked_object_get_farthest */

int Scene_picked_object_set_farthest(
	struct Scene_picked_object *scene_picked_object,double farthest)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Sets the <farthest> position at which the <scene_picked_object> was picked.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_picked_object_set_farthest);
	if (scene_picked_object)
	{
		scene_picked_object->farthest = farthest;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_set_farthest.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_set_farthest */

double Scene_picked_object_get_nearest(
	struct Scene_picked_object *scene_picked_object)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the <nearest> position at which the <scene_picked_object> was picked.
==============================================================================*/
{
	double nearest;

	ENTER(Scene_picked_object_get_nearest);
	if (scene_picked_object)
	{
		nearest = scene_picked_object->nearest;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest.  Invalid argument(s)");
		nearest=0.0;
	}
	LEAVE;

	return (nearest);
} /* Scene_picked_object_get_nearest */

int Scene_picked_object_set_nearest(
	struct Scene_picked_object *scene_picked_object,double nearest)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Sets the <nearest> position at which the <scene_picked_object> was picked.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_picked_object_set_nearest);
	if (scene_picked_object)
	{
		scene_picked_object->nearest = nearest;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_set_nearest.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_set_nearest */

int Scene_picked_object_write(struct Scene_picked_object *scene_picked_object)
/*******************************************************************************
LAST MODIFIED : 19 July 1999

DESCRIPTION :
Writes the contents of the <scene_picked_object> as:
hit_no: scene_object_name[.scene_object_name...] subobject_number...
==============================================================================*/
{
	int i,return_code;

	ENTER(Scene_picked_object_write);
	if (scene_picked_object)
	{
		display_message(INFORMATION_MESSAGE,"%d: ",scene_picked_object->hit_no);
#if defined (USE_SCENE_OBJECT)
		for (i=0;i<scene_picked_object->number_of_scene_objects;i++)
		{
			if (0<i)
			{
				display_message(INFORMATION_MESSAGE,".");
			}
			display_message(INFORMATION_MESSAGE,"%s",
				scene_picked_object->scene_objects[i]->name);
		}
#endif /* defined (USE_SCENE_OBJECT) */
		for (i=0;i<scene_picked_object->number_of_subobjects;i++)
		{
			display_message(INFORMATION_MESSAGE," %d",
				scene_picked_object->subobjects[i]);
		}
		display_message(INFORMATION_MESSAGE,", near=%d far=%d\n",
			scene_picked_object->nearest,scene_picked_object->farthest);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_write.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_write */

int Scene_picked_objects_have_same_transformation(
	struct Scene_picked_object *scene_picked_object1,
	struct Scene_picked_object *scene_picked_object2)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Returns true if <scene_picked_object1> and <scene_picked_object2> have the
same total transformation.
==============================================================================*/
{
	double transformation_matrix1[16],transformation_matrix2[16];
	int i,return_code,transformation_required1,transformation_required2;

	ENTER(Scene_picked_objects_have_same_transformation);
	return_code=0;
	if (scene_picked_object1&&scene_picked_object2)
	{
		if (Scene_picked_object_get_total_transformation_matrix(
			scene_picked_object1,&transformation_required1,transformation_matrix1)&&
			Scene_picked_object_get_total_transformation_matrix(
				scene_picked_object2,&transformation_required2,transformation_matrix2)&&
			(transformation_required1==transformation_required2))
		{
			return_code=1;
			for (i=0;(i>16)&&return_code;i++)
			{
				return_code=(transformation_matrix1[i] == transformation_matrix2[i]);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_objects_have_same_transformation.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_objects_have_same_transformation */

int Scene_picked_object_get_total_transformation_matrix(
	struct Scene_picked_object *scene_picked_object,int *transformation_required,
	double *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Multiplies the transformation matrices for all the scene_objects in the
<scene_picked_object>, returning the overall <matrix>. The matrix has 16 values
in the order of along rows first, which operate on the untransformed homogeneous
coordinates [x y z h(=1)] to give [x' y' z' h'], with xm = x'/h', etc. as in:
|x'| |M11 M12 M13 M14| |x|
|y'|=|M21 M22 M23 M24|.|y|
|z'| |M31 M32 M33 M34| |z|
|h'| |M41 M42 M43 M44| |h|
However, if none of the scene objects have transformations, the flag
<transformation_required> will be set to 0 and the <transformation_matrix> will
be set to the identity.
==============================================================================*/
{
// 	double mat1[16],mat2[16];
//  int i,j,k,number_of_transformations,return_code;
	int i,j,number_of_transformations,return_code;
// 	gtMatrix gt_transformation;

	ENTER(Scene_picked_object_get_total_transformation_matrix);
	if (scene_picked_object&&transformation_required&&transformation_matrix)
	{
		number_of_transformations=0;
#if defined (USE_SCENE_OBJECT)
		for (k=0;k<scene_picked_object->number_of_scene_objects;k++)
		{
			if (Scene_object_has_transformation(
				scene_picked_object->scene_objects[k]))
			{
				number_of_transformations++;
				Scene_object_get_transformation(scene_picked_object->scene_objects[k],
					&gt_transformation);
				if (1==number_of_transformations)
				{
					/* transpose gtMatrix */
					for (i=0;i<4;i++)
					{
						for (j=0;j<4;j++)
						{
							transformation_matrix[i*4+j] = gt_transformation[j][i];
						}
					}
				}
				else
				{
					/* transpose gtMatrix */
					for (i=0;i<4;i++)
					{
						for (j=0;j<4;j++)
						{
							mat1[i*4+j] = gt_transformation[j][i];
						}
					}
					multiply_matrix(4,4,4,transformation_matrix,mat1,mat2);
					copy_matrix(4,4,mat2,transformation_matrix);
				}
			}
		}
#endif /* defined (USE_SCENE_OBJECT) */
		if (!((*transformation_required)=(0<number_of_transformations)))
		{
			/* return the identity matrix - just in case */
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					if (i==j)
					{
						transformation_matrix[i*4+j] = 1.0;
					}
					else
					{
						transformation_matrix[i*4+j] = 0.0;
					}
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_total_transformation_matrix.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_total_transformation_matrix */

DECLARE_OBJECT_FUNCTIONS(Scene_picked_object)

DECLARE_LIST_FUNCTIONS(Scene_picked_object)

struct Any_object *Scene_picked_object_list_get_nearest_any_object(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Scene_picked_object **scene_picked_object_address)
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Returns the nearest picked any_object in <scene_picked_object_list>.
If <scene_picked_object_address> is supplied, the pointer to the
Scene_picked_object referring to the nearest any_object is put there.
==============================================================================*/
{
	struct Scene_picked_object_get_nearest_any_object_data
		nearest_any_object_data;

	ENTER(Scene_picked_object_list_get_nearest_any_object);
	nearest_any_object_data.nearest=0.0;
	nearest_any_object_data.nearest_any_object=(struct Any_object *)NULL;
	nearest_any_object_data.scene_picked_object=
		(struct Scene_picked_object *)NULL;
	if (scene_picked_object_list)
	{
		FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
			Scene_picked_object_get_nearest_any_object,
			(void *)&nearest_any_object_data,scene_picked_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_nearest_any_object.  Invalid argument(s)");
	}
	if (scene_picked_object_address)
	{
		*scene_picked_object_address=nearest_any_object_data.scene_picked_object;
	}
	LEAVE;

	return (nearest_any_object_data.nearest_any_object);
} /* Scene_picked_object_list_get_nearest_any_object */

struct LIST(Any_object) *Scene_picked_object_list_get_picked_any_objects(
	struct LIST(Scene_picked_object) *scene_picked_object_list)
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Returns the list of all any_objects in the <scene_picked_object_list>. 
==============================================================================*/
{
	struct LIST(Any_object) *any_object_list;

	ENTER(Scene_picked_object_list_get_picked_any_objects);
	if (scene_picked_object_list)
	{	
		if (any_object_list=CREATE(LIST(Any_object))())
		{
			FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
				Scene_picked_object_get_picked_any_objects,(void *)any_object_list,
				scene_picked_object_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_picked_object_list_get_picked_any_objects.  "
				"Could not create any_object list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_picked_any_objects.  Invalid argument(s)");
		any_object_list=(struct LIST(Any_object) *)NULL;
	}
	LEAVE;

	return (any_object_list);
} /* Scene_picked_object_list_get_picked_any_objects */

struct FE_element *Scene_picked_object_list_get_nearest_element(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	int select_elements_enabled,int select_faces_enabled,int select_lines_enabled,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the nearest picked element in <scene_picked_object_list> that is in
<cmiss_region> (or in root_region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest element.
<select_elements_enabled> allows top-level/3-D elements to be selected.
<select_faces_enabled> allows face and 2-D elements to be selected.
<select_lines_enabled> allows line and 1-D elements to be selected.
==============================================================================*/
{
	struct Scene_picked_object_get_nearest_element_data nearest_element_data;

	ENTER(Scene_picked_object_list_get_nearest_element);
	nearest_element_data.nearest=0.0;
	nearest_element_data.nearest_element=(struct FE_element *)NULL;
	nearest_element_data.cmiss_region = cmiss_region;
	nearest_element_data.select_elements_enabled=select_elements_enabled;
	nearest_element_data.select_faces_enabled=select_faces_enabled;
	nearest_element_data.select_lines_enabled=select_lines_enabled;
	nearest_element_data.scene_picked_object=(struct Scene_picked_object *)NULL;
	nearest_element_data.rendition=(struct Cmiss_rendition *)NULL;
	nearest_element_data.graphic=(struct Cmiss_graphic *)NULL;
	if (scene_picked_object_list)
	{
		FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
			Scene_picked_object_get_nearest_element,(void *)&nearest_element_data,
			scene_picked_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_nearest_element.  Invalid argument(s)");
	}
	if (scene_picked_object_address)
	{
		*scene_picked_object_address=nearest_element_data.scene_picked_object;
	}
	if (rendition_address)
	{
		*rendition_address=nearest_element_data.rendition;
	}
	if (graphic_address)
	{
		*graphic_address=nearest_element_data.graphic;
	}
	LEAVE;

	return (nearest_element_data.nearest_element);
} /* Scene_picked_object_list_get_nearest_element */

#if defined (USE_OPENCASCADE)
static int Scene_picked_object_get_nearest_cad_primitive(
	struct Scene_picked_object *scene_picked_object,
	void *cad_primitive_data_void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to an element, the "nearest" value is
compared with that for the current nearest element in the
<nearest_element_data>. If there was no current nearest element or the new
element is nearer, it becomes the nearest element and its "nearest" value is
stored in the nearest_element_data.
==============================================================================*/
{
	int return_code;
	struct Cad_primitive_identifier cad;
	struct Cmiss_region *cmiss_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_get_cad_primitive_data *cad_primitive_data;

	cad_primitive_data = static_cast<struct Scene_picked_object_get_cad_primitive_data *>(cad_primitive_data_void);
	if (scene_picked_object&&cad_primitive_data)
	{
		return_code=1;
		/* proceed only if there is no picked_element or object is nearer */
		if (((Cmiss_cad_identifier_id)NULL==cad_primitive_data->nearest_cad_element)||
			(Scene_picked_object_get_nearest(scene_picked_object) <
				cad_primitive_data->nearest))
		{
			/* is the last scene_object a Graphical_element wrapper, and does the
				 settings for the graphic refer to elements? */
			if ((NULL != (rendition=Scene_picked_object_get_rendition(
				scene_picked_object,
				Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
				&&(NULL != (cmiss_region = Cmiss_rendition_get_region(rendition)))&&
				(2<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
				(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
						Scene_picked_object_get_subobject(scene_picked_object,0))))&&
				(Cmiss_graphic_selects_elements(graphic)))
			{
				cad.number = -1;
				cad.type = Cad_primitive_INVALID;
				if (Cmiss_field_is_cad_geometry(Cmiss_graphic_get_coordinate_field(graphic), NULL))
				{
					cad.number = Scene_picked_object_get_subobject(scene_picked_object,1);
					if (Cmiss_graphic_get_graphic_type(graphic) == CMISS_GRAPHIC_SURFACES)
					{
						cad.type = Cad_primitive_SURFACE;
						struct Computed_field *coordinate_field = Cmiss_graphic_get_coordinate_field(graphic);
						//display_message(INFORMATION_MESSAGE, "CAD element is type %d element %d\n", cad.type, cad.number);
						struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
						return_code = Computed_field_get_domain( coordinate_field, domain_field_list );
						if	( return_code )
						{
							struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
								( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
							if ( cad_topology_field )
							{
								Cmiss_cad_identifier_id cad_shape = new Cmiss_cad_identifier();
								//cad_shape.cad_topology = cad_topology_field;
								//cad_shape.number = cad.number;
								//Cmiss_field_access(cad_topology_field);
								cad_shape->cad_topology = Cmiss_field_cast_cad_topology(cad_topology_field);
								cad_shape->identifier = cad;

								cad_primitive_data->nearest_cad_element = cad_shape;
								cad_primitive_data->graphic = graphic;
								cad_primitive_data->scene_picked_object=scene_picked_object;
								cad_primitive_data->rendition = rendition;
								cad_primitive_data->nearest = Scene_picked_object_get_nearest(scene_picked_object);
								//Cad_topology_information( cad_topology_field, cad );
							}
						}
						if ( domain_field_list )
							DESTROY_LIST(Computed_field)(&domain_field_list);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_picked_object_get_nearest_cad_primitive.  "
						"Invalid cad element %s %d",Cad_primitive_type_string(cad.type),cad.number);
					return_code=0;
				}
			}
			if (graphic)
				Cmiss_graphic_destroy(&graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_cad_primitive.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Scene_picked_object_get_nearest_cad_primitive */

Cmiss_cad_identifier_id Scene_picked_object_list_get_cad_primitive(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	int select_surfaces_enabled,int select_lines_enabled,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the nearest picked element in <scene_picked_object_list> that is in
<cmiss_region> (or in root_region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest element.
<select_elements_enabled> allows top-level/3-D elements to be selected.
<select_faces_enabled> allows face and 2-D elements to be selected.
<select_lines_enabled> allows line and 1-D elements to be selected.
==============================================================================*/
{
	struct Scene_picked_object_get_cad_primitive_data cad_primitive_data;

	ENTER(Scene_picked_object_list_get_cad_primitive);
	cad_primitive_data.nearest=0.0;
	cad_primitive_data.nearest_cad_element=(Cmiss_cad_identifier_id)NULL;
	cad_primitive_data.cmiss_region = cmiss_region;
	cad_primitive_data.select_surfaces_enabled=select_surfaces_enabled;
	cad_primitive_data.select_lines_enabled=select_lines_enabled;
	cad_primitive_data.scene_picked_object=(struct Scene_picked_object *)NULL;
	cad_primitive_data.rendition=(struct Cmiss_rendition *)NULL;
	cad_primitive_data.graphic=(struct Cmiss_graphic *)NULL;
	if (scene_picked_object_list)
	{
		FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
			Scene_picked_object_get_nearest_cad_primitive,(void *)&cad_primitive_data,
			scene_picked_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_nearest_cad_primitive.  Invalid argument(s)");
	}
	if (scene_picked_object_address)
	{
		*scene_picked_object_address=cad_primitive_data.scene_picked_object;
	}
	if (rendition_address)
	{
		*rendition_address=cad_primitive_data.rendition;
	}
	if (graphic_address)
	{
		*graphic_address=cad_primitive_data.graphic;
	}
	LEAVE;

	return (cad_primitive_data.nearest_cad_element);
} /* Scene_picked_object_list_get_nearest_cad_primitive */
#endif /* defined (USE_OPENCASCADE) */

static int Scene_picked_object_get_picked_region_sorted_elements(
	struct Scene_picked_object *scene_picked_object,void *picked_elements_data_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
If the <scene_picked_object> refers to a element and the element is in the given
manager, ensures it is in the list.
==============================================================================*/
{
	int dimension, return_code;
	struct Cmiss_region *cmiss_region;
	struct FE_element *element;
	struct FE_region *fe_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_region_element_map_data *picked_elements_data;
	struct CM_element_information cm;

	ENTER(Scene_picked_object_get_picked_elements);
	if (scene_picked_object&&(picked_elements_data=
		(struct Scene_picked_object_region_element_map_data	*)picked_elements_data_void))
	{
		return_code=1;
		/* is the last scene_object a Graphical_element wrapper, and does the
			 settings for the graphic refer to element_points? */
		if ((NULL != (rendition=Scene_picked_object_get_rendition(scene_picked_object,
			Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
			&&((cmiss_region = Cmiss_rendition_get_region(rendition)))
			&&(2<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
			(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
			Scene_picked_object_get_subobject(scene_picked_object,0))))&&
			(Cmiss_graphic_selects_elements(graphic)))
		{
			if (CM_element_information_from_graphics_name(&cm,
				Scene_picked_object_get_subobject(scene_picked_object,1))&&
				(fe_region = Cmiss_region_get_FE_region(cmiss_region)) &&
				(element = FE_region_get_FE_element_from_identifier(fe_region, &cm)))
			{
				dimension = get_FE_element_dimension(element);
				if ((picked_elements_data->select_elements_enabled &&
					((CM_ELEMENT == cm.type) || (3 == dimension))) ||
					(picked_elements_data->select_faces_enabled &&
						((CM_FACE == cm.type) || (2 == dimension))) ||
					(picked_elements_data->select_lines_enabled &&
						((CM_LINE == cm.type) || (1 == dimension))))
				{
					picked_elements_data->element_list->insert(std::make_pair(cmiss_region, element));

				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_picked_object_get_picked_elements.  "
					"Invalid element %s %d",CM_element_type_string(cm.type),cm.number);
				return_code=0;
			}
		}
		if (graphic)
			Cmiss_graphic_destroy(&graphic);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_picked_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);

	return 1;
} /* Scene_picked_object_get_picked_nodes */

void *Scene_picked_object_list_get_picked_region_sorted_elements(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int select_elements_enabled,int select_faces_enabled,
	int select_lines_enabled)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the list of all elements identified in the <scene_picked_object_list>.
<select_elements_enabled> allows top-level/3-D elements to be selected.
<select_faces_enabled> allows face and 2-D elements to be selected.
<select_lines_enabled> allows line and 1-D elements to be selected.
==============================================================================*/
{
	struct Scene_picked_object_region_element_map_data picked_elements_data;

	ENTER(Scene_picked_object_list_get_picked_elements);
	if (scene_picked_object_list)
	{
		picked_elements_data.select_elements_enabled=select_elements_enabled;
		picked_elements_data.select_faces_enabled=select_faces_enabled;
		picked_elements_data.select_lines_enabled=select_lines_enabled;
		picked_elements_data.element_list=new Region_element_map();
		if (picked_elements_data.element_list)
		{
			FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
				Scene_picked_object_get_picked_region_sorted_elements,(void *)&picked_elements_data,
				scene_picked_object_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_picked_elements.  Invalid argument(s)");
		picked_elements_data.element_list = NULL;
	}
	LEAVE;

	return ((void *)picked_elements_data.element_list);
} /* Scene_picked_object_list_get_picked_elements */

struct Element_point_ranges *Scene_picked_object_list_get_nearest_element_point(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the nearest picked element point in <scene_picked_object_list> that is
in <cmiss_region> (or in root_region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest element point.
The returned Element_point_ranges structure should be used or destroyed by the
calling function.
==============================================================================*/
{
	struct Scene_picked_object_get_nearest_element_point_data
		nearest_element_point_data;

	ENTER(Scene_picked_object_list_get_nearest_element_point);
	nearest_element_point_data.nearest=0.0;
	nearest_element_point_data.nearest_element_point=
		(struct Element_point_ranges *)NULL;
	nearest_element_point_data.cmiss_region = cmiss_region;
	nearest_element_point_data.scene_picked_object=
		(struct Scene_picked_object *)NULL;
	nearest_element_point_data.rendition=(struct Cmiss_rendition *)NULL;
	nearest_element_point_data.graphic=
		(struct Cmiss_graphic *)NULL;
	if (scene_picked_object_list)
	{
		FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
			Scene_picked_object_get_nearest_element_point,
			(void *)&nearest_element_point_data,scene_picked_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_nearest_element_point.  "
			"Invalid argument(s)");
	}
	if (scene_picked_object_address)
	{
		*scene_picked_object_address=nearest_element_point_data.scene_picked_object;
	}
	if (rendition_address)
	{
		*rendition_address=nearest_element_point_data.rendition;
	}
	if (graphic_address)
	{
		*graphic_address=nearest_element_point_data.graphic;
	}
	LEAVE;

	return (nearest_element_point_data.nearest_element_point);
} /* Scene_picked_object_list_get_nearest_element_point */

struct LIST(Element_point_ranges) *Scene_picked_object_list_get_picked_element_points(
	struct LIST(Scene_picked_object) *scene_picked_object_list)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Returns the list of all element_points in the <scene_picked_object_list>.
==============================================================================*/
{
	struct LIST(Element_point_ranges) *picked_element_points_list;

	ENTER(Scene_picked_object_list_get_picked_element_points);
	if (scene_picked_object_list)
	{	
		if (picked_element_points_list=CREATE(LIST(Element_point_ranges))())
		{
			FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
				Scene_picked_object_get_picked_element_points,
				(void *)picked_element_points_list,scene_picked_object_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_picked_object_list_get_picked_element_points.  "
				"Could not create element point ranges list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_picked_element_points.  "
			"Invalid argument(s)");
		picked_element_points_list=(struct LIST(Element_point_ranges) *)NULL;
	}
	LEAVE;

	return (picked_element_points_list);
} /* Scene_picked_object_list_get_picked_element_points */

struct FE_node *Scene_picked_object_list_get_nearest_node(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int use_data, struct Cmiss_region *cmiss_region,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the nearest picked node in <scene_picked_object_list> that is in
<cmiss_region> (or any region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest node.
The <use_data> flag indicates that we are searching for a data point instead of
a node, needed since different settings type used for each.
==============================================================================*/
{
	struct Scene_picked_object_get_nearest_node_data nearest_node_data;

	ENTER(Scene_picked_object_list_get_nearest_node);
	nearest_node_data.nearest=0.0;
	nearest_node_data.nearest_node=(struct FE_node *)NULL;
	nearest_node_data.use_data=use_data;
	nearest_node_data.cmiss_region = cmiss_region;
	nearest_node_data.scene_picked_object=(struct Scene_picked_object *)NULL;
	nearest_node_data.rendition=(struct Cmiss_rendition *)NULL;
	nearest_node_data.graphic=(struct Cmiss_graphic *)NULL;
	if (scene_picked_object_list)
	{
		FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
			Scene_picked_object_get_nearest_node,(void *)&nearest_node_data,
			scene_picked_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_nearest_node.  Invalid argument(s)");
	}
	if (scene_picked_object_address)
	{
		*scene_picked_object_address=nearest_node_data.scene_picked_object;
	}
	if (rendition_address)
	{
		*rendition_address=nearest_node_data.rendition;
	}
	if (graphic_address)
	{
		*graphic_address=nearest_node_data.graphic;
	}
	LEAVE;

	return (nearest_node_data.nearest_node);
} /* Scene_picked_object_list_get_nearest_node */

int Scene_get_input_callback(struct Scene *scene,
	struct Scene_input_callback *scene_input_callback)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Fills <scene_input_callback> with the current scene input_callback information.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_get_input_callback);
	if (scene&&scene_input_callback)
	{
		scene_input_callback->procedure=scene->input_callback.procedure;
		scene_input_callback->data=scene->input_callback.data;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_get_input_callback */

int Scene_set_input_callback(struct Scene *scene,
	struct Scene_input_callback *scene_input_callback)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Sets the function that will be called to pass on input information, and the data
that that function wants to receive.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_set_input_callback);
	if (scene&&scene_input_callback)
	{
		scene->input_callback.procedure=scene_input_callback->procedure;
		scene->input_callback.data=scene_input_callback->data;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_input_callback */

int Scene_input(struct Scene *scene,enum Scene_input_type input_type,
	int button_number,int input_modifier,double viewx,double viewy,double viewz,
	double nearx,double neary,double nearz,double farx,double fary,double farz,
	int num_hits,GLuint *select_buffer)
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Routine called by Scene_viewers - and in future possibly other Scene clients -
to tell about input. At present only mouse input is handled; the <input_type>
says whether it is a mouse button press, motion or button release event, and the
button_number and whether modifier keys were depressed at the time of the
event are given in subsequent parameters. The coordinates of two points in
space, near and far, are given to identify the ray under the mouse pointer at
the time of the event. The view direction is also needed for clients of pick and
drag information. Finally, if the event involved selection/picking, the number
of hits and the select buffer will be set accordingly. The main function of
this routine is to convert picking information into a list of easy-to-interpret
Scene_picked_objects to pass to clients of the scene, eg. node editor.
==============================================================================*/
{
	GLuint *select_buffer_ptr;
// 	GLuint *select_buffer_ptr,scene_object_no;
	int hit_no,number_of_names,return_code;
	struct Scene_input_callback_data scene_input_data;
	struct Scene_picked_object *scene_picked_object;
#if defined (USE_SCENE_OBJECT)
 	struct Scene_object *scene_object;
#endif /* defined (USE_SCENE_OBJECT) */

	ENTER(Scene_input);
	if (scene)
	{
		/* processing successful unless otherwise specified */
		return_code=1;
		switch (input_type)
		{
			case SCENE_BUTTON_PRESS:
			case SCENE_MOTION_NOTIFY:
			case SCENE_BUTTON_RELEASE:
			{
				if (scene_input_data.picked_object_list=
					CREATE(LIST(Scene_picked_object))())
				{
					select_buffer_ptr=select_buffer;
					for (hit_no=0;(hit_no<num_hits)&&return_code;hit_no++)
					{
						if (scene_picked_object=CREATE(Scene_picked_object)(hit_no))
						{
							number_of_names=(int)(*select_buffer_ptr);
							select_buffer_ptr++;
							Scene_picked_object_set_nearest(scene_picked_object,
								(double)(*select_buffer_ptr));
							select_buffer_ptr++;
							Scene_picked_object_set_farthest(scene_picked_object,
								(double)(*select_buffer_ptr));
							select_buffer_ptr++;

							/* first part of names identifies list of scene_objects in path
								 to picked graphic. Must be at least one; only more that one
								 if contains child_scene */
#if defined (USE_SCENE_OBJECT)
							do
							{
								scene_object_no= *select_buffer_ptr;
								select_buffer_ptr++;
								number_of_names--;
								if (scene_object=FIND_BY_IDENTIFIER_IN_LIST(Scene_object,
									position)(scene_object_no,scene->scene_object_list))
								{
									return_code=Scene_picked_object_add_Scene_object(
										scene_picked_object,scene_object);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Scene_input.  No scene object at position %d",
										scene_object_no);
									return_code=0;
								}
							}
							while (return_code&&(SCENE_OBJECT_SCENE==scene_object->type));
#endif /* defined (USE_SCENE_OBJECT) */
							if (return_code)
							{
								for (;0<number_of_names&&return_code;number_of_names--)
								{
									return_code=Scene_picked_object_add_subobject(
										scene_picked_object,(int)(*select_buffer_ptr));
									select_buffer_ptr++;
								}
							}
							if (return_code)
							{
								if (!ADD_OBJECT_TO_LIST(Scene_picked_object)(
									scene_picked_object,
									scene_input_data.picked_object_list))
								{
									return_code=0;
								}
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,
									"Scene_input.  Failed to build Scene_picked_object");
								DESTROY(Scene_picked_object)(&scene_picked_object);
							}
						}
					}
					if (return_code)
					{
						/* now send the callback */
						if ((scene->input_callback).procedure)
						{
							/* complete members of callback data structure */
							scene_input_data.viewx=viewx;
							scene_input_data.viewy=viewy;
							scene_input_data.viewz=viewz;
							scene_input_data.nearx=nearx;
							scene_input_data.neary=neary;
							scene_input_data.nearz=nearz;
							scene_input_data.farx=farx;
							scene_input_data.fary=fary;
							scene_input_data.farz=farz;
							scene_input_data.input_type=input_type;
							scene_input_data.button_number=button_number;
							scene_input_data.input_modifier=input_modifier;
							if (scene->input_callback.procedure)
							{
								(scene->input_callback.procedure)(scene,
									scene->input_callback.data,&scene_input_data);
							}
						}
					}
					DESTROY(LIST(Scene_picked_object))(
						&(scene_input_data.picked_object_list));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_input.  Could not create picked object list");
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"Scene_input.  Invalid input_type");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_input.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_input */


Cmiss_rendition *Scene_get_rendition_of_position(struct Scene *scene, int position)
{
	Cmiss_rendition *rendition = NULL;

	if (scene)
	{
		if (scene->list_of_rendition && 
			!scene->list_of_rendition->empty())
		{
			Rendition_set::iterator pos =
				scene->list_of_rendition->begin();
			while (pos != scene->list_of_rendition->end() && !rendition)
			{
				if (position == Cmiss_rendition_get_position(*pos))
				{
					rendition = *pos;
				}
				++pos;
				if (rendition)
					break;
			}
		}
	}
	else if (position == 0)
	{
		return NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_get_rendition_of_position.  Invalid argument(s)");
	}

	return rendition;
}

struct LIST(Scene_picked_object) *Scene_pick_objects(struct Scene *scene,
	struct Interaction_volume *interaction_volume,
	struct Graphics_buffer *graphics_buffer)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Returns a list of all the graphical entities in the <interaction_volume> of
<scene>. The nearest member of each scene_picked_object will be adjusted as
understood for the type of <interaction_volume> passed.
==============================================================================*/
{
#if defined (OLD_CODE)
	double depth,normalised_z;
#endif /* defined (OLD_CODE) */
	double modelview_matrix[16],projection_matrix[16];
	GLdouble opengl_modelview_matrix[16],opengl_projection_matrix[16];
	GLuint *select_buffer,*select_buffer_ptr;
	int hit_no,i,j,num_hits,number_of_names,return_code, rendition_no;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene *parent_scene;
	struct Scene_picked_object *scene_picked_object;
	struct Cmiss_rendition *rendition;
	ENTER(Scene_pick_objects);
	scene_picked_object_list=(struct LIST(Scene_picked_object) *)NULL;
	if (scene&&interaction_volume)
	{
		if (scene_picked_object_list=CREATE(LIST(Scene_picked_object))())
		{
			Render_graphics_opengl *renderer = 
				Render_graphics_opengl_create_glbeginend_renderer(graphics_buffer);
			renderer->picking = 1;
			if (renderer->Scene_compile(scene))
			{
				select_buffer=(GLuint *)NULL;
				num_hits=-1;
				while (0>num_hits)
				{
					if (ALLOCATE(select_buffer,GLuint,select_buffer_size))
					{
						select_buffer_ptr=select_buffer;
						Interaction_volume_get_modelview_matrix(interaction_volume,
							modelview_matrix);
						Interaction_volume_get_projection_matrix(interaction_volume,
							projection_matrix);
						/* transpose projection matrix for OpenGL */
						for (i=0;i<4;i++)
						{
							for (j=0;j<4;j++)
							{
								opengl_modelview_matrix[j*4+i] = modelview_matrix[i*4+j];
								opengl_projection_matrix[j*4+i] = projection_matrix[i*4+j];
							}
						}

						glSelectBuffer(select_buffer_size,select_buffer);
						glRenderMode(GL_SELECT);
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						glMultMatrixd(opengl_projection_matrix);
						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						glMultMatrixd(opengl_modelview_matrix);
						/* set an arbitrary viewport - not really needed
						   SAB 22 July 2004 This is causing the view frustrums
						   to not match when picking, so instead I am not changing the
						   viewport, so presumably the last rendered viewport is OK. */
						/* glViewport(0,0,1024,1024); */
						glDepthRange((GLclampd)0,(GLclampd)1);
						{
							renderer->Scene_execute(scene);
						}
						glFlush();
						num_hits=glRenderMode(GL_RENDER);
						if (0<=num_hits)
						{
							return_code=1;
							for (hit_no=0;(hit_no<num_hits)&&return_code;hit_no++)
							{
								if (scene_picked_object=CREATE(Scene_picked_object)(hit_no))
								{
									number_of_names=(int)(*select_buffer_ptr);
									select_buffer_ptr++;
									/* get range of depth of picked object */
									/*???RC OpenGL Programming Guide p361 says depth values are
										made into integers from 0 to 2^32-1. Just convert to
										doubles for now */
									Scene_picked_object_set_nearest(scene_picked_object,
										(double)(*select_buffer_ptr));
									select_buffer_ptr++;
									Scene_picked_object_set_farthest(scene_picked_object,
										(double)(*select_buffer_ptr));
									select_buffer_ptr++;
									/* first part of names identifies list of scene_objects in
										 path to picked graphic. Must be at least one; only more
										 than one if contains child_scene, which then becomes the
										 parent of the next level of scene objects */
									parent_scene = scene;
									
									rendition_no=(int)(*select_buffer_ptr);
									select_buffer_ptr++;
									number_of_names--;
									if (NULL != (rendition = 
											Scene_get_rendition_of_position(scene, rendition_no)))
									{
											return_code = Scene_picked_object_add_rendition(
												scene_picked_object, rendition);
									}
									if (return_code)
									{
										for (;0<number_of_names&&return_code;number_of_names--)
										{
											return_code=Scene_picked_object_add_subobject(
												scene_picked_object,(int)(*select_buffer_ptr));
											select_buffer_ptr++;
										}
									}
									if (return_code)
									{
										if (!ADD_OBJECT_TO_LIST(Scene_picked_object)(
											scene_picked_object,scene_picked_object_list))
										{
											return_code=0;
										}
									}
									if (!return_code)
									{
										display_message(ERROR_MESSAGE,"Scene_pick_objects.  "
											"Failed to build Scene_picked_object");
										DESTROY(Scene_picked_object)(&scene_picked_object);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"Scene_pick_objects.  "
										"Could not create Scene_picked_object");
									return_code=0;
								}
							}
						}
						else
						{
							/* select buffer overflow; enlarge and repeat */
							select_buffer_size += SELECT_BUFFER_SIZE_INCREMENT;
						}
						DEALLOCATE(select_buffer);

					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_pick_objects.  Unable to compile scene.");
			}
			delete renderer;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_pick_objects.  Could not create picked object list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_pick_objects.  Invalid argument(s)");
	}
	LEAVE;

	return (scene_picked_object_list);
} /* Scene_pick_objects */

int Scene_add_light(struct Scene *scene,struct Light *light)
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Adds a light to the Scene list_of_lights.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_add_light);
	if (scene&&light)
	{
		if (!IS_OBJECT_IN_LIST(Light)(light,scene->list_of_lights))
		{
			return_code=ADD_OBJECT_TO_LIST(Light)(light,scene->list_of_lights);
			Scene_changed_private(scene,/*fast_changing*/0);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_add_light.  Light already in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_add_light */

int Scene_has_light_in_list(struct Scene *scene,
	struct LIST(Light) *light_list)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Returns true if the list_of_lights in <Scene> intersects <light_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_has_light_in_list);
	if (scene && light_list)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Light)(Light_is_in_list,
			(void *)light_list, scene->list_of_lights))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_has_light_in_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_has_light_in_list */

int Scene_remove_light(struct Scene *scene,struct Light *light)
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Removes a light from the Scene list_of_lights.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_remove_light);
	if (scene&&light)
	{
		if (IS_OBJECT_IN_LIST(Light)(light,scene->list_of_lights))
		{
			return_code=REMOVE_OBJECT_FROM_LIST(Light)(light,
				scene->list_of_lights);
			Scene_changed_private(scene,/*fast_changing*/0);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_remove_light.  Light not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_light */

int for_each_Light_in_Scene(struct Scene *scene,
	LIST_ITERATOR_FUNCTION(Light) *iterator_function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 18 December 1997

DESCRIPTION :
Allows clients of the <scene> to perform functions with the lights in it. The
most common task will be to call execute_Light.
==============================================================================*/
{
	int return_code;

	ENTER(for_each_Light_in_Scene);
	if (scene&&iterator_function)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(Light)(iterator_function,user_data,
			scene->list_of_lights);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_Light_in_Scene.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_each_Light_in_Scene */

int Scene_has_fast_changing_objects(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Returns true if the scene may require special rendering because it has
fast_changing objects in it, involving separately calling
execute_Scene_non_fast_changing and execute_Scene_fast_changing, instead of
execute_Scene.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_has_fast_changing_objects);
	if (scene)
	{
		return_code=0;
#if defined (USE_SCENE_OBJECT)
		if (FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_is_fast_changing,(void *)NULL,scene->scene_object_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
#endif /* defined (USE_SCENE_OBJECT) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_has_fast_changing_objects.  Missing scene");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_has_fast_changing_objects */

enum Scene_change_status Scene_get_change_status(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Returns the change state of the scene; SCENE_NO_CHANGE, SCENE_FAST_CHANGE or
SCENE_CHANGE. Clients may respond to SCENE_FAST_CHANGE more efficiently.
==============================================================================*/
{
	enum Scene_change_status change_status;

	ENTER(Scene_get_change_status);
	if (scene)
	{
		change_status=scene->change_status;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_change_status.  Missing scene");
		change_status=SCENE_NO_CHANGE;
	}
	LEAVE;

	return (change_status);
} /* Scene_get_change_status */

#if defined (TO_BE_EDITED)
static int Scene_get_data_range_for_autoranging_spectrum_iterator(
	struct Scene_object *scene_object, void *data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2002

DESCRIPTION :
Expands the range to include the data values of only those element_group settings
in the <scene> which are autoranging and which point to this spectrum.
==============================================================================*/
{
	int return_code;
	
	ENTER(Scene_get_data_range_for_spectrum_iterator);
	if (scene_object && data_void)
	{
		for_each_graphic_in_Cmiss_rendition
		if (g_VISIBLE == Scene_object_get_visibility(scene_object))
		{
			switch(scene_object->type)
			{
				case SCENE_OBJECT_GRAPHICS_OBJECT:
				{
					/* Do nothing */
				} break;
				case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
				{
					for_each_settings_in_GT_element_group(scene_object->gt_element_group,
						GT_element_settings_get_data_range_for_autoranging_spectrum,
						data_void);
				} break;
				case SCENE_OBJECT_SCENE:
				{
					for_each_Scene_object_in_Scene(scene_object->child_scene,
						Scene_get_data_range_for_autoranging_spectrum_iterator, data_void);
				} break;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_data_range_for_spectrum_iterator.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Scene_get_data_range_for_spectrum_iterator */
#endif

#if defined (OLD_CODE)
static int Scene_expand_data_range_for_autoranging_spectrum(
	struct Spectrum *spectrum, void *scene_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2001

DESCRIPTION :
Expands the range to include the data values of only those element_group settings
in the <scene> which are autoranging and which point to this spectrum.
==============================================================================*/
{
	int return_code;
	struct Scene *scene;
	struct Spectrum *spectrum_to_be_modified_copy;

	ENTER(Scene_expand_data_range_for_autoranging_spectrum);

	if (spectrum && (scene = (struct Scene *)scene_void))
	{
		float minimum = 0, maximum = 0;
		int range_set;
		Scene_get_data_range_for_spectrum(scene,
			spectrum, &minimum, &maximum, &range_set);
		return_code = Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);		
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_expand_data_range_for_autoranging_spectrum.  "
			"Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Scene_expand_data_range_for_autoranging_spectrum */
#endif /* defined (OLD_CODE) */

int build_Scene(struct Scene *scene)
{
	int return_code;

	ENTER(build_Scene);
	if (scene)
	{
		/* check whether scene contents need building */
		if (scene->build)
		{
			Render_graphics_build_objects renderer;
			
			scene->build = 0;
			return_code = renderer.Scene_compile(scene);
#if defined (OLD_CODE)
			// GRC not sure why this is done here
			if (return_code && scene->spectrum_manager)
			{
				return_code = FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
					Scene_expand_data_range_for_autoranging_spectrum,
					(void *)scene, scene->spectrum_manager);
			}
			if (return_code && scene->build)
			{
				/* The autoranging spectrum could possibly have had some side effect
					like changing contours and therefore some settings may have changed.
				   I haven't put it in a while loop as I don't want to make a possible
				   infinite loop */
				return_code = renderer.Scene_compile(scene);
			}
#endif /* defined (OLD_CODE) */
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "build_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* build_Scene */

int Scene_render_opengl(Scene *scene, Render_graphics_opengl *renderer)
{
	int return_code = 0;
	Rendition_set::iterator pos;
	ENTER(Scene_render_opengl);
	if (scene && renderer)
	{
		renderer->fast_changing = 0;
		glPushName(0);
#if defined (USE_SCENE_OBJECT)
		return_code = FOR_EACH_OBJECT_IN_LIST(Scene_object)(Scene_object_call_renderer,
			(void *)renderer, scene->scene_object_list);
#else
		if (scene->list_of_rendition && 
			!scene->list_of_rendition->empty())
		{
			pos = scene->list_of_rendition->begin();
			while (pos != scene->list_of_rendition->end())
			{
				Cmiss_rendition_call_renderer(*pos, (void *)renderer);
				++pos;
			}
		}
#endif
		glPopName();
		if (Scene_has_fast_changing_objects(scene))
		{
			renderer->fast_changing = 1;
			glPushName(0);
#if defined (USE_SCENE_OBJECT)
		return_code = FOR_EACH_OBJECT_IN_LIST(Scene_object)(Scene_object_call_renderer,
			(void *)renderer, scene->scene_object_list);
#else
		if (scene->list_of_rendition && 
			!scene->list_of_rendition->empty())
		{
			pos = scene->list_of_rendition->begin();
			while (pos != scene->list_of_rendition->end())
			{
				Cmiss_rendition_call_renderer(*pos, (void *)renderer);
				++pos;
			}
		}
#endif
			renderer->fast_changing = 0;
			glPopName();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_render_opengl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* build_Scene */

#if defined (USE_SCENE_OBJECT)

int Scene_remove_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Removes all scene objects containing <graphics object> from <scene>.
Does not complain if <graphics_object> is not used in <scene>.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_remove_graphics_object);
	if (scene && graphics_object)
	{
		return_code = 1;
		while (return_code && (scene_object =
			FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_gt_object,
				(void *)graphics_object, scene->scene_object_list)))
		{
			if (!Scene_remove_Scene_object(scene, scene_object))
			{
				display_message(ERROR_MESSAGE, "Scene_remove_graphics_object.  Failed");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_graphics_object */

int Scene_add_child_scene(struct Scene *scene, struct Scene *child_scene,
	int position, const char *scene_object_name, struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Adds <child_scene> to the list of objects on <scene> at <position>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
The optional <scene_object_name> allows the scene_object to be given a different
name from that of the <child_scene>, and must be unique for the scene.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;
	
	ENTER(Scene_add_child_scene);
	if (scene && child_scene)
	{
		if (!scene_object_name)
		{
			scene_object_name = child_scene->name;
		}
		if (!FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_name,
			(void *)scene_object_name, scene->scene_object_list))
		{
			if (scene_object = create_Scene_object_with_Scene(
				scene_object_name, child_scene, scene_manager))
			{
				return_code = Scene_add_Scene_object(scene, scene_object, position);
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Scene_add_child_scene.  "
				"Object with name '%s' already in scene", scene_object_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_child_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_add_child_scene */

int Scene_remove_child_scene(struct Scene *scene, struct Scene *child_scene)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Removes all scene objects containing <child_scene> from <scene>.
Does not complain if <child_scene> is not used in <scene>.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_remove_child_scene);
	if (scene && child_scene)
	{
		return_code = 1;
		while (return_code && (scene_object =
			FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_child_scene,
				(void *)child_scene, scene->scene_object_list)))
		{
			if (!Scene_remove_Scene_object(scene, scene_object))
			{
				display_message(ERROR_MESSAGE, "Scene_remove_child_scene.  Failed");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_child_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_child_scene */
#endif /* defined (USE_SCENE_OBJECT) */

#if defined (TO_BE_EDITED)
int Scene_add_graphical_element_group(struct Scene *scene,
	struct Cmiss_region *cmiss_region, int position, char *scene_object_name)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Adds a graphical element group for <cmiss_region> to the list of objects in
<scene> at <position>.
The group will be given a default rendition depending on the scene's current
graphical_element_mode.
The new Scene_object will take the <scene_object_name>; an error is
reported if this name is already in use in <scene>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
Note if the scene is in GRAPHICAL_ELEMENT_MANUAL mode, a group may be added
more than once with a different name, however, it will share the underlying
GT_element_group and therefore have the same rendition.
==============================================================================*/
{
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum GT_visibility_type visibility;
	int default_coordinate_field_defined, default_value, return_code;
	struct Computed_field *default_coordinate_field, *element_xi_coordinate_field,
		*orientation_scale_field, *variable_scale_field;
	struct Element_discretization element_discretization;
	struct FE_region *fe_region;
	struct GT_object *glyph;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;
	struct Scene_object *scene_object;
	Triple glyph_centre, glyph_size, glyph_scale_factors;

	ENTER(Scene_add_graphical_element_group);
	if (scene && cmiss_region && scene_object_name)
	{
		if (!FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_name,
			(void *)scene_object_name, scene->scene_object_list))
		{
			/* see if group is currently in scene under any name */
			if (scene_object = FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
				Scene_object_has_Cmiss_region, (void *)cmiss_region,
				scene->scene_object_list))
			{
				gt_element_group =
					Scene_object_get_graphical_element_group(scene_object);
			}
			else
			{
				gt_element_group = (struct GT_element_group *)NULL;
			}
			return_code = 1;
			switch (scene->graphical_element_mode)
			{
				case GRAPHICAL_ELEMENT_INVISIBLE:
				case GRAPHICAL_ELEMENT_EMPTY:
				case GRAPHICAL_ELEMENT_LINES:
				{
					/* not allowed to add group more than once, and must have same name
						 as element_group */
					if (scene_object)
					{
						display_message(ERROR_MESSAGE,
							"Scene_add_graphical_element_group.  "
							"Can not add graphical groups more than once in %s mode",
							ENUMERATOR_STRING(Scene_graphical_element_mode)(
								scene->graphical_element_mode));
						return_code = 0;
					}
				} break;
				case GRAPHICAL_ELEMENT_MANUAL:
				{
					/* ok - already checked no object of same name in scene */
				} break;
				case GRAPHICAL_ELEMENT_NONE:
				{
					display_message(ERROR_MESSAGE, "Scene_add_graphical_element_group.  "
						"Graphical element groups are not enabled for scene %s",
						scene->name);
					return_code = 0;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE, "Scene_add_graphical_element_group.  "
						"Unknown scene graphical element mode");
					return_code = 0;
				} break;
			}
			if (return_code)
			{
				if (!gt_element_group)
				{
					/* Make a new GT_element_group */
					if (gt_element_group = CREATE(GT_element_group)(
						cmiss_region,
						scene->element_point_ranges_selection,
						scene->element_selection,
						scene->node_selection,
						scene->data_selection))
					{
						default_coordinate_field = (struct Computed_field *)NULL;
						//GT_element_group_get_default_coordinate_field(gt_element_group);
						/* set default circle and element discretization in group */
						read_circle_discretization_defaults(&default_value,
							scene->user_interface);
						GT_element_group_set_circle_discretization(gt_element_group,
							default_value);
						read_element_discretization_defaults(&default_value,
							scene->user_interface);
						element_discretization.number_in_xi1 = default_value;
						element_discretization.number_in_xi2 = default_value;
						element_discretization.number_in_xi3 = default_value;
						GT_element_group_set_element_discretization(gt_element_group,
							&element_discretization);

						if (GRAPHICAL_ELEMENT_LINES == scene->graphical_element_mode)
						{
							/* add default settings - wireframe (line) rendition */
							if (settings =
								CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_LINES))
							{
								GT_element_settings_set_material(settings,
									scene->default_material);	
								GT_element_settings_set_label_field(settings,
									(struct Computed_field *)NULL, scene->default_font);	
								GT_element_settings_set_selected_material(settings,
									FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
										"default_selected", scene->graphical_material_manager));
								if (!GT_element_group_add_settings(gt_element_group,
									settings, 0))
								{
									display_message(ERROR_MESSAGE,
										"Scene_add_graphical_element_group.  "
										"Could not add default line settings");
									DESTROY(GT_element_settings)(&settings);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Scene_add_graphical_element_group.  "
									"Could not create default line settings");
							}
							/* if the group has data, and the default_coordinate_field
								 element_xi_coordinate field defined over them, then
								 add data_points to the rendition */
							element_xi_coordinate_field =
								FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_type_embedded, NULL,
									Cmiss_region_get_Computed_field_manager(cmiss_region));
							if ((fe_region = FE_region_get_data_FE_region(
									Cmiss_region_get_FE_region(cmiss_region))) &&
								FE_region_get_first_FE_node_that(fe_region,
									(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL))
							{
								default_coordinate_field_defined = default_coordinate_field && 
									FE_region_get_first_FE_node_that(fe_region,
										FE_node_has_Computed_field_defined,
										(void *)default_coordinate_field);
								if (default_coordinate_field_defined ||
									(element_xi_coordinate_field &&
										FE_region_get_first_FE_node_that(fe_region,
											FE_node_has_Computed_field_defined,
											(void *)element_xi_coordinate_field)))
								{
									if (settings = CREATE(GT_element_settings)(
										GT_ELEMENT_SETTINGS_DATA_POINTS))
									{
										if (!default_coordinate_field_defined)
										{
											GT_element_settings_set_coordinate_field(
												settings, element_xi_coordinate_field);
										}
										GT_element_settings_set_material(settings,
											scene->default_material);
										GT_element_settings_set_label_field(settings,
											(struct Computed_field *)NULL, scene->default_font);	
										GT_element_settings_set_selected_material(settings,
											FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
												"default_selected", scene->graphical_material_manager));
										/* set the glyph to "point" */
										GT_element_settings_get_glyph_parameters(settings,
											&glyph, &glyph_scaling_mode, glyph_centre,
											glyph_size, &orientation_scale_field,
											glyph_scale_factors, &variable_scale_field);
										glyph = FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)(
											"point", scene->glyph_manager);
										GT_element_settings_set_glyph_parameters(settings,
											glyph, glyph_scaling_mode, glyph_centre,
											glyph_size, orientation_scale_field,
											glyph_scale_factors, variable_scale_field);
										if (!GT_element_group_add_settings(gt_element_group,
											settings, 0))
										{
											display_message(ERROR_MESSAGE,
												"Scene_add_graphical_element_group.  "
												"Could not add default data_point settings");
											DESTROY(GT_element_settings)(&settings);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Scene_add_graphical_element_group.  "
											"Could not create default data_point settings");
									}
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_add_graphical_element_group.  "
							"Could not create graphical element group");
						return_code = 0;
					}
				}
				if (gt_element_group)
				{
					ACCESS(GT_element_group)(gt_element_group);
					switch (scene->graphical_element_mode)
					{
						case GRAPHICAL_ELEMENT_INVISIBLE:
						{
							visibility = g_INVISIBLE;
						} break;
						case GRAPHICAL_ELEMENT_EMPTY:
						case GRAPHICAL_ELEMENT_MANUAL:
						{
							visibility = g_VISIBLE;
						} break;
						case GRAPHICAL_ELEMENT_LINES:
						{
							visibility = g_VISIBLE;
							scene->compile_status = GRAPHICS_NOT_COMPILED;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Scene_add_graphical_element_group.  "
								"Unknown graphical element mode");
							visibility = g_INVISIBLE;
						} break;
					}
					if (scene_object = create_Scene_object_with_Graphical_element_group(
						scene_object_name, gt_element_group))
					{
						Scene_object_set_visibility(scene_object, visibility);
						if (!Scene_add_Scene_object(scene, scene_object, position))
						{
							display_message(ERROR_MESSAGE,
								"Scene_add_graphical_element_group.  "
								"Could not add scene object to scene");
							DESTROY(Scene_object)(&scene_object);
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_add_graphical_element_group.  "
							"Could not create scene object");
						return_code = 0;
					}
					DEACCESS(GT_element_group)(&gt_element_group);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Scene_add_graphical_element_group.  "
				"Object with name '%s' already in scene", scene_object_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_graphical_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_add_graphical_element_group */

int Scene_remove_graphical_element_group(struct Scene *scene,
	struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes all scene objects containing a graphical rendition of <cmiss_region>
from <scene>. Does not complain if <cmiss_region> is not used in <scene>.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_remove_graphical_element_group);
	if (scene && cmiss_region)
	{
		return_code = 1;
#if (USE_SCENE_OBJECT)
		while (return_code && (scene_object =
			FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_Cmiss_region,
				(void *)cmiss_region, scene->scene_object_list)))
		{
			if (!Scene_remove_Scene_object(scene, scene_object))
			{
				display_message(ERROR_MESSAGE,
					"Scene_remove_graphical_element_group.  Failed");
				return_code = 0;
			}
		}
#endif /* (USE_SCENE_OBJECT) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_graphical_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_graphical_element_group */
#endif /* defined (TO_BE_EDITED) */

#if defined (USE_SCENE_OBJECT)
int Scene_update_time_behaviour(struct Scene *scene,
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
If the graphics_object has more than one time, this function ensures that the
Scene_object has a Time_object.
==============================================================================*/
{
	char *graphics_object_name, *time_object_name;
	int return_code = 0;
	struct Scene_object *scene_object;
	struct Time_object *time;
	
	ENTER(Scene_update_time_behaviour);
	/* check arguments */
	if (scene&&graphics_object)
	{
		/* Ensure the Scene object has a time object if the graphics
			object has more than one time */
		if(1 < GT_object_get_number_of_times(graphics_object))
		{
			if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
				Scene_object_has_gt_object,
				(void *)graphics_object,scene->scene_object_list))
			{
				time = Scene_object_get_time_object(scene_object);
				if(!time)
				{
					time = Time_object_create_regular(/*update_frequency*/10.0,
						/*time_offset*/0.0);				
					if(GET_NAME(GT_object)(graphics_object,&graphics_object_name)
						&& ALLOCATE(time_object_name, char, strlen(graphics_object_name)
							+ strlen(scene->name) + 5))
					{
						sprintf(time_object_name, "%s_in_%s", graphics_object_name,
							scene->name);
						Time_object_set_name(time, time_object_name);
						DEALLOCATE(time_object_name);
						DEALLOCATE(graphics_object_name);
					}
					Scene_object_set_time_object(scene_object, time);
					Time_keeper_add_time_object(scene->default_time_keeper, time);
				}
				else
				{
					ACCESS(Time_object)(time);
				}
				return_code=FOR_EACH_OBJECT_IN_LIST(Scene_object)(
					Scene_object_update_time_behaviour, (void *)time,
					scene->scene_object_list);
				DEACCESS(Time_object)(&time);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_update_time_behaviour.  Unable to find Scene_object for graphics_object");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_update_time_behaviour.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Scene_update_time_behaviour */
#endif /* (USE_SCENE_OBJECT) */

#if defined (USE_SCENE_OBJECT)
int Scene_update_time_behaviour_with_gt_element_group(struct Scene *scene,
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
If the <gt_element_group> has more than one time, this function ensures that the
Scene_object has a Time_object.
==============================================================================*/
{
	char *time_object_name;
	int return_code;
	struct Scene_object *scene_object;
	struct Time_object *time;
	
	ENTER(Scene_update_time_behaviour_with_gt_element_group);
	if (scene && gt_element_group)
	{
		return_code = 1;
		/* Ensure the Scene object has a time object if and only if the 
			graphics object has more than one time */
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_graphical_element_group,
			(void *)gt_element_group,scene->scene_object_list))
		{
			if(GT_element_group_has_multiple_times(gt_element_group))
			{
				time = Scene_object_get_time_object(scene_object);
				if(!time)
				{
					time = Time_object_create_regular(/*update_frequency*/10.0,
						/*time_offset*/0.0);
					if (ALLOCATE(time_object_name, char, strlen(scene_object->name)
						+ strlen(scene->name) + 5))
					{
						sprintf(time_object_name, "%s_in_%s", scene_object->name,
							scene->name);
						Time_object_set_name(time, time_object_name);
						DEALLOCATE(time_object_name);
					}
					Scene_object_set_time_object(scene_object, time);
					Time_keeper_add_time_object(scene->default_time_keeper, time);
					/* Time_object_set_next_time_function(time,
						 GT_element_group_next_time_function, gt_element_group); */
				}
				else
				{
					ACCESS(Time_object)(time);
				}
				return_code=FOR_EACH_OBJECT_IN_LIST(Scene_object)(
					Scene_object_update_time_behaviour, (void *)time,
					scene->scene_object_list);
				DEACCESS(Time_object)(&time);
			}
			else
			{
				if(Scene_object_has_time(scene_object))
				{
					/* Remove the time object then */
					Scene_object_set_time_object(scene_object,
						(struct Time_object *)NULL);
				}		
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_update_time_behaviour_with_gt_element_group.  "
				"Unable to find Scene_object for element_group");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_update_time_behaviour_with_gt_element_group.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_update_time_behaviour_with_gt_element_group */
#endif /* (USE_SCENE_OBJECT) */

#if defined (USE_SCENE_OBJECT)
int Scene_set_time_behaviour(struct Scene *scene, char *scene_object_name,
	char *time_object_name, struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Creates a Time_object with name <time_object_name> and sets that as the time
object for the scene_object named <scene_object_name>.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;
	struct Time_object *time;
	
	ENTER(Scene_update_time_behaviour);
	/* check arguments */
	if (scene&&scene_object_name&&time_object_name)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_name,
			(void *)scene_object_name,scene->scene_object_list))
		{
			if(time = Time_object_create_regular(
						/*update_frequency*/10.0, /*time_offset*/0.0))
			{
				Time_object_set_name(time, time_object_name);
				Scene_object_set_time_object(scene_object, time);
				Time_keeper_add_time_object(time_keeper, time);
				DEACCESS(Time_object)(&time);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_update_time_behaviour.  Unable to create time_object");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_update_time_behaviour.  Unable to find object %s", scene_object_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_update_time_behaviour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_update_time_behaviour */
#endif /* (USE_SCENE_OBJECT) */

int Scene_get_graphics_range(struct Scene *scene,
	double *centre_x, double *centre_y, double *centre_z,
	double *size_x, double *size_y, double *size_z)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the range of all visible graphics objects in scene.
Returns 0 without error if scene is empty.
==============================================================================*/
{
	double max_x, max_y, max_z, min_x, min_y, min_z;
	int return_code = 1;
	struct Graphics_object_range_struct graphics_object_range;

	ENTER(Scene_get_graphics_range);
	if (scene && centre_x && centre_y && centre_z && size_x && size_y && size_z)
	{
		/* must first build graphics objects */
		build_Scene(scene);
		/* get range of visible graphics_objects in scene */
		graphics_object_range.first = 1;
		if (scene->list_of_rendition && 
			!scene->list_of_rendition->empty())
		{
			Rendition_set::iterator pos =
				scene->list_of_rendition->begin();
			while (pos != scene->list_of_rendition->end())
			{
				return_code &= Cmiss_rendition_get_range(*pos, scene, &graphics_object_range);
				++pos;
			}
		}
		if (graphics_object_range.first)
		{
			/* nothing in the scene; return zeros */
			*centre_x = *centre_y = *centre_z = 0.0;
			*size_x = *size_y = *size_z =0.0;
		}
		else
		{
			/* get centre and size of smallest cube enclosing visible scene */
			max_x = (double)graphics_object_range.maximum[0];
			max_y = (double)graphics_object_range.maximum[1];
			max_z = (double)graphics_object_range.maximum[2];
			min_x = (double)graphics_object_range.minimum[0];
			min_y = (double)graphics_object_range.minimum[1];
			min_z = (double)graphics_object_range.minimum[2];
			*centre_x = 0.5*(max_x + min_x);
			*centre_y = 0.5*(max_y + min_y);
			*centre_z = 0.5*(max_z + min_z);
			*size_x = max_x - min_x;
			*size_y = max_y - min_y;
			*size_z = max_z - min_z;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_graphics_range.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_get_graphics_range */

#if defined (TO_BE_EDITED)
int Scene_get_element_group_position(struct Scene *scene,
	struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function returns the position of <cmiss_region> in <scene>, starting
from 1 at the top. A return value of 0 indicates an error - probably saying
that the GFE for element_group is not in the scene.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_get_element_group_position);
	if (scene && cmiss_region)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_Cmiss_region, (void *)cmiss_region,
			scene->scene_object_list))
		{
			return_code = Scene_object_get_position(scene_object);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_get_element_group_position.  "
				"Element_group not in scene");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_element_group_position.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_get_element_group_position */

int Scene_set_element_group_position(struct Scene *scene,
	struct Cmiss_region *cmiss_region, int position)
/*******************************************************************************
LAST MODIFIED : 2 December 200

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <cmiss_region> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <cmiss_region> at the end.
Scene_object for the group keeps the same name.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_set_element_group_position);
	if (scene && cmiss_region)
	{
		if (scene_object = FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_Cmiss_region, (void *)cmiss_region,
			scene->scene_object_list))
		{
			return_code =
				Scene_set_scene_object_position(scene, scene_object, position);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_set_element_group_position.  "
				"Graphics object not in scene");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_element_group_position.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_element_group_position */
#endif /* defined (TO_BE_EDITED) */

#if defined (USE_GRAPHICS_OBJECT)
int Scene_get_graphics_object_position(struct Scene *scene,
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function returns the position of <graphics_object> in <scene>, starting
from 1 at the top. A return value of 0 indicates an error - probably saying
that the graphics object is not in the scene.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_get_graphics_object_position);
	if (scene&&graphics_object)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_gt_object,(void *)graphics_object,
			scene->scene_object_list))
		{
			return_code=Scene_object_get_position(scene_object);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_get_graphics_object_position.  "
				"Graphics object not in scene");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_graphics_object_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_get_graphics_object_position */

int Scene_set_graphics_object_position(struct Scene *scene,
	struct GT_object *graphics_object,int position)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <graphics_object> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <graphics_object> at the end.
==============================================================================*/
{
	int return_code;
	struct Scene_object *scene_object;

	ENTER(Scene_set_graphics_object_position);
	if (scene&&graphics_object)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_gt_object,(void *)graphics_object,
			scene->scene_object_list))
		{
			return_code=Scene_set_scene_object_position(scene,scene_object,position);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_set_graphics_object_position.  "
				"Graphics object not in scene");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_graphics_object_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_graphics_object_position */
#endif /* defined (USE_GRAPHICS_OBJECT) */

#if defined (USE_SCENE_OBJECT)
int Scene_get_scene_object_position(struct Scene *scene,
	struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 12 November 2001

DESCRIPTION :
Returns the position of <scene_object> in the <scene>->scene_object_list.
==============================================================================*/
{
	int position;

	ENTER(Scene_get_scene_object_position);
	if (scene && scene_object &&
		IS_OBJECT_IN_LIST(Scene_object)(scene_object, scene->scene_object_list))
	{
		position = Scene_object_get_position(scene_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_scene_object_position.  Invalid argument(s)");
		position = 0;
	}
	LEAVE;

	return (position);
} /* Scene_get_scene_object_position */

struct Scene_object *Scene_get_scene_object_at_position(struct Scene *scene,
	int position)
/*******************************************************************************
LAST MODIFIED : 7 May 2007

DESCRIPTION :
Returns the <scene_object> at <position> in the <scene>->scene_object_list.
==============================================================================*/
{
	struct Scene_object *scene_object;

	ENTER(Scene_get_scene_object_at_position);
	if (scene)
	{
		scene_object = FIND_BY_IDENTIFIER_IN_LIST(Scene_object,
			 position)(position,scene->scene_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_scene_object_at_position.  Invalid argument(s)");
		scene_object = (	struct Scene_object *)NULL;
	}
	LEAVE;

	return (scene_object);
} /* Scene_get_scene_object_at_position */

int Scene_set_scene_object_position(struct Scene *scene,
	struct Scene_object *scene_object,int position)
/*******************************************************************************
LAST MODIFIED : 6 November 2001

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <scene_object> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <scene_object> at the end.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_set_scene_object_position);
	if (scene&&scene_object)
	{
		/* take it out of the list and add it at the new position */
		Scene_begin_cache(scene);
		ACCESS(Scene_object)(scene_object);
		return_code = (Scene_remove_Scene_object_private(scene, scene_object)&&
			Scene_add_Scene_object(scene, scene_object, position));
		DEACCESS(Scene_object)(&scene_object);
		Scene_end_cache(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_scene_object_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_scene_object_position */ 
#endif /* defined (USE_SCENE_OBJECT) */


#if defined (TO_BE_EDITED)
enum GT_visibility_type Scene_get_element_group_visibility(
	struct Scene *scene, struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the visibility of the GFE for <cmiss_region> in <scene>.
==============================================================================*/
{
	enum GT_visibility_type visibility;
	struct Scene_object *scene_object;

	ENTER(Scene_get_element_group_visibility);
	if (scene&&cmiss_region)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_Cmiss_region,(void *)cmiss_region,
			scene->scene_object_list))
		{
			visibility=Scene_object_get_visibility(scene_object);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_get_element_group_visibility.  "
				"Group not in scene");
			visibility=g_INVISIBLE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_element_group_visibility.  Invalid argument(s)");
		visibility=g_INVISIBLE;
	}
	LEAVE;

	return (visibility);
} /* Scene_get_element_group_visibility */
#endif /* defined (TO_BE_EDITED) */

struct Element_group_visibility_data
{
	struct Cmiss_region *cmiss_region;
	enum GT_visibility_type visibility;
};

#if defined (USE_SCENE_OBJECT)
static int Scene_object_set_element_group_visibility_iterator(
	struct Scene_object *scene_object, void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
If the <scene_object> is a graphical element group using <cmiss_region>, set
its <visibility>.
==============================================================================*/
{
	int return_code;
	struct Element_group_visibility_data *user_data;

	ENTER(Scene_object_set_element_group_visibility_iterator);
	if (scene_object &&
		(user_data = (struct Element_group_visibility_data *)user_data_void))
	{
		if (Scene_object_has_Cmiss_region(scene_object,
			(void *)user_data->cmiss_region))
		{
			return_code =
				Scene_object_set_visibility(scene_object, user_data->visibility);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_set_element_group_visibility_iterator.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_set_element_group_visibility_iterator */
#endif /* defined (USE_SCENE_OBJECT) */

#if defined (TO_BE_EDITED)
int Scene_set_element_group_visibility(struct Scene *scene,
	struct Cmiss_region *cmiss_region,enum GT_visibility_type visibility)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Sets the visibility of all scene objects that are graphical element groups for
<cmiss_region> in <scene>.
==============================================================================*/
{
	int return_code;
	struct Element_group_visibility_data user_data;

	ENTER(Scene_set_element_group_visibility);
	if (scene && cmiss_region)
	{
		user_data.cmiss_region = cmiss_region;
		user_data.visibility = visibility;
		return_code = FOR_EACH_OBJECT_IN_LIST(Scene_object)(
			Scene_object_set_element_group_visibility_iterator,
			(void *)&user_data, scene->scene_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_element_group_visibility.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_element_group_visibility */
#endif /* defined (TO_BE_EDITED) */

#if defined (USE_GRAPHICS_OBJECT)
enum GT_visibility_type Scene_get_graphics_object_visibility(
	struct Scene *scene,struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Returns the visibility of <graphics_object> in <scene>.
==============================================================================*/
{
	enum GT_visibility_type visibility;
	struct Scene_object *scene_object;

	ENTER(Scene_get_graphics_object_visibility);
	if (scene&&graphics_object)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_gt_object,(void *)graphics_object,
			scene->scene_object_list))
		{
			visibility=Scene_object_get_visibility(scene_object);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_get_graphics_object_visibility.  "
				"Graphics object not in scene");
			visibility=g_INVISIBLE;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_graphics_object_visibility.  Invalid argument(s)");
		visibility=g_INVISIBLE;
	}
	LEAVE;

	return (visibility);
} /* Scene_get_graphics_object_visibility */

struct Graphics_object_visibility_data
{
	struct GT_object *graphics_object;
	enum GT_visibility_type visibility;
};

static int Scene_object_set_graphics_object_visibility_iterator(
	struct Scene_object *scene_object, void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
If the <scene_object> contains <graphics_object>, sets its <visibility>.
==============================================================================*/
{
	int return_code;
	struct Graphics_object_visibility_data *user_data;

	ENTER(Scene_object_set_graphics_object_visibility_iterator);
	if (scene_object &&
		(user_data = (struct Graphics_object_visibility_data *)user_data_void))
	{
		if (Scene_object_has_gt_object(scene_object,
			(void *)user_data->graphics_object))
		{
			return_code =
				Scene_object_set_visibility(scene_object, user_data->visibility);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_set_graphics_object_visibility_iterator.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_set_graphics_object_visibility_iterator */

int Scene_set_graphics_object_visibility(struct Scene *scene,
	struct GT_object *graphics_object,enum GT_visibility_type visibility)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Sets the visibility of all instances of <graphics_object> in <scene>.
==============================================================================*/
{
	int return_code;
	struct Graphics_object_visibility_data user_data;

	ENTER(Scene_set_graphics_object_visibility);
	if (scene && graphics_object)
	{
		user_data.graphics_object = graphics_object;
		user_data.visibility = visibility;
		return_code = FOR_EACH_OBJECT_IN_LIST(Scene_object)(
			Scene_object_set_graphics_object_visibility_iterator,
			(void *)&user_data, scene->scene_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_set_graphics_object_visibility.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_set_graphics_object_visibility */

int Scene_has_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object)
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Returns true if <graphics object> is in the list of objects on <scene>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_has_graphics_object);
	/* check arguments */
	if (scene&&graphics_object)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_gt_object,
			(void *)graphics_object,scene->scene_object_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_has_graphics_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_has_graphics_object */
#endif /* defined (USE_GRAPHICS_OBJECT) */

#if defined (USE_CHILD_SCENE)
int Scene_has_child_scene(struct Scene *scene,struct Scene *child_scene)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Returns true if <child_scene> is in the list of scenes in <scene>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_has_child_scene);
	if (scene&&child_scene)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_child_scene,
			(void *)child_scene,scene->scene_object_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_has_child_scene.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_has_child_scene */
#endif /* defined (USE_CHILD_SCENE) */

#if defined (USE_SCENE_OBJECT)
struct Scene_object *Scene_get_Scene_object_by_name(struct Scene *scene,
	char *name)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Returns the Scene_object called <name> in <scene>, or NULL if not found.
==============================================================================*/
{
	struct Scene_object *scene_object;

	ENTER(Scene_get_Scene_object_by_name);
	if (scene && name)
	{
		scene_object = FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_name, (void *)name, scene->scene_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_Scene_object_by_name.  Invalid argument(s)");
		scene_object = (struct Scene_object *)NULL;
	}
	LEAVE;

	return (scene_object);
} /* Scene_get_Scene_object_by_name */
#endif /* defined (USE_SCENE_OBJECT) */

#if defined (TO_BE_EDITED)
int Scene_has_Cmiss_region(struct Scene *scene,
	struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns true if <scene> contains a graphical element for <cmiss_region>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_has_Cmiss_region);
	if (scene && cmiss_region)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Scene_object)(Scene_object_has_Cmiss_region,
			(void *)cmiss_region, scene->scene_object_list))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_has_Cmiss_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_has_Cmiss_region */
#endif

#if defined (TO_BE_EDITED)
struct GT_element_group *Scene_get_graphical_element_group(
	struct Scene *scene, struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the graphical element_group for <cmiss_region> in <scene>.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;
	struct Scene_object *scene_object;

	ENTER(Scene_get_graphical_element_group);
	if (scene && cmiss_region)
	{
		if (scene_object=FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_Cmiss_region, (void *)cmiss_region,
			scene->scene_object_list))
		{
			gt_element_group=Scene_object_get_graphical_element_group(scene_object);
		}
		else
		{
			gt_element_group=(struct GT_element_group *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_graphical_element_group.  Invalid argument(s)");
		gt_element_group = (struct GT_element_group *)NULL;
	}
	LEAVE;

	return (gt_element_group);
} /* Scene_get_graphical_element_group */
#endif /* defined (TO_BE_EDITED) */

#if defined (USE_SCENE_OBJECT)
struct Scene_object *Scene_get_scene_object_with_Cmiss_region(
	struct Scene *scene, struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the scene_object for <element_group> in <scene>.
==============================================================================*/
{
	struct Scene_object *scene_object;

	ENTER(Scene_get_scene_object_with_Cmiss_region);
	if (scene && cmiss_region)
	{
		scene_object = FIRST_OBJECT_IN_LIST_THAT(Scene_object)(
			Scene_object_has_Cmiss_region, (void *)cmiss_region,
			scene->scene_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_scene_object_with_Cmiss_region.  Invalid argument(s)");
		scene_object = (struct Scene_object *)NULL;
	}
	LEAVE;

	return (scene_object);
} /* Scene_get_scene_object_with_Cmiss_region */
#endif /* defined (USE_SCENE_OBJECT) */

int set_Scene(struct Parse_state *state,
	void *scene_address_void,void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the scene from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Scene *scene,**scene_address;
	struct MANAGER(Scene) *scene_manager;

	ENTER(set_Scene);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((scene_address=(struct Scene **)scene_address_void)&&
					(scene_manager=(struct MANAGER(Scene) *)scene_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*scene_address)
						{
							DEACCESS(Scene)(scene_address);
							*scene_address=(struct Scene *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(current_token,
							scene_manager))
						{
							if (*scene_address != scene)
							{
								ACCESS(Scene)(scene);
								if (*scene_address)
								{
									DEACCESS(Scene)(scene_address);
								}
								*scene_address=scene;
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown scene : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Scene.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SCENE_NAME|none");
				if (scene_address=(struct Scene **)scene_address_void)
				{
					if (scene= *scene_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",scene->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing scene name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Scene */

int set_Scene_including_sub_objects(struct Parse_state *state,
	void *scene_address_void,void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Modifier function to set the scene from a command.  This function understands
the use of a period '.' to delimit sub objects and will automatically create
a Scene that wraps a graphics_object from either the scene or a 
GT_element_settings or a whole GT_element_settings so that export commands can
work on these sub_elements.  These created scenes are not added to the manager.
==============================================================================*/
{
#if defined (TO_BE_EDITED)	
	const char *current_token;
	char *index, *next_index, *string_copy;
	FE_value time;
	int return_code;
	gtMatrix transformation;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct Scene *scene,**scene_address;
	struct Scene_object *new_scene_object, *scene_object;
	struct Time_object *time_object;
	struct MANAGER(Scene) *scene_manager;

	ENTER(set_Scene);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((scene_address=(struct Scene **)scene_address_void)&&
					(scene_manager=(struct MANAGER(Scene) *)scene_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*scene_address)
						{
							DEACCESS(Scene)(scene_address);
							*scene_address=(struct Scene *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (strchr(current_token, '.'))
						{
							if (ALLOCATE(string_copy, char, strlen(current_token) + 1))
							{
								strcpy (string_copy, current_token);
								/* Find top level scene */
								index = strchr(string_copy, '.');
								*index = 0;
								if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(string_copy,
									scene_manager))
								{
									index++;
									/* Find sub object (could be recursive at some point but not
										currently useful) */
									if (next_index = strchr(index, '.'))
									{
										*next_index = 0;
									}
									if (scene_object = Scene_get_Scene_object_by_name(scene, index))
									{
										switch (scene_object->type)
										{
											case SCENE_OBJECT_GRAPHICS_OBJECT:
											{
												/* Use the same name so that output using the name is correct */
												if (scene = CREATE(Scene)(scene->name))
												{
													new_scene_object = create_Scene_object_with_Graphics_object(
														index, Scene_object_get_gt_object(scene_object));
													if (Scene_object_has_transformation(scene_object))
													{
														Scene_object_get_transformation(scene_object,
															&transformation);
														Scene_object_set_transformation(new_scene_object,
															&transformation);
													}
													if (Scene_object_has_time(scene_object))
													{
														time_object = Scene_object_get_time_object(scene_object);
														Scene_object_set_time_object(new_scene_object, time_object);
													}
													return_code = Scene_add_Scene_object(scene, new_scene_object, 0);
												}
												else
												{
													display_message(ERROR_MESSAGE,"set_Scene_including_sub_objects.  "
														"Unable to create temporary scene");
													return_code=0;
												}
											} break;
											case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
											{
												Render_graphics_build_objects renderer;
												renderer.Scene_object_compile(scene_object);

												gt_element_group = Scene_object_get_graphical_element_group(scene_object);
												
												/* Use the same name so that output using the name is correct */
												if (scene = CREATE(Scene)(scene->name))
												{
													return_code = 1;
													if (next_index)
													{
														index = next_index + 1;
														/* Look for a particular settings */
														if (gt_element_settings = first_settings_in_GT_element_group_that(
															gt_element_group, GT_element_settings_has_name, index))
														{
															/* Create a scene object for the graphics object in settings */
															new_scene_object = create_Scene_object_with_Graphics_object(index,
																GT_element_settings_get_graphics_object(gt_element_settings));
														}
														else
														{
															display_message(ERROR_MESSAGE,"Unknown settings %s", index);
															return_code=0;
														}
													}
													else
													{
														/* Create a scene object for the whole graphical element */
														new_scene_object = create_Scene_object_with_Graphical_element_group(
															index, gt_element_group);
													}
													if (return_code)
													{
														if (Scene_object_has_transformation(scene_object))
														{
															Scene_object_get_transformation(scene_object,
																&transformation);
															Scene_object_set_transformation(new_scene_object,
																&transformation);
														}
														if (Scene_object_has_time(scene_object))
														{
															time_object = Scene_object_get_time_object(scene_object);
															Scene_object_set_time_object(new_scene_object, time_object);
														}
														return_code = Scene_add_Scene_object(scene, new_scene_object, 0);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,"set_Scene_including_sub_objects.  "
														"Unable to create temporary scene");
													return_code=0;
												}
											} break;
											case SCENE_OBJECT_SCENE:
											{
												scene = Scene_object_get_child_scene(scene_object);
											} break;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,"Unknown sub_object %s of %s",
											index, string_copy);
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"Unknown parent scene : %s",
										string_copy);
									return_code=0;
								}
								if (return_code)
								{
									REACCESS(Scene)(scene_address, scene);
								}
								DEALLOCATE(string_copy);
							}
							else
							{
								display_message(ERROR_MESSAGE,"set_Scene_including_sub_objects.  "
									"Unable to allocate memory");
								return_code=0;
							}
						}
						else
						{
							if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(current_token,
								scene_manager))
							{
								if (*scene_address != scene)
								{
									REACCESS(Scene)(scene_address, scene);
								}
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown scene : %s",
									current_token);
								return_code=0;
							}
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Scene_including_sub_objects.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SCENE_NAME|none");
				if (scene_address=(struct Scene **)scene_address_void)
				{
					if (scene= *scene_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",scene->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing scene name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Scene_including_sub_objects.  Missing state");
		return_code=0;
	}
	LEAVE;
#else
	USE_PARAMETER(state);
	USE_PARAMETER(scene_address_void);
	USE_PARAMETER(scene_manager_void);
	int return_code = 0;
#endif

	return (return_code);
} /* set_Scene */

struct Define_scene_filter_data
{
public:
	char match_all;
	char *match_graphic_name;
	char match_visibility_flags;
	int show;

	Define_scene_filter_data() :
		match_all(0),
		match_graphic_name(NULL),
		match_visibility_flags(0),
		show(1)
	{	
	}

	~Define_scene_filter_data()
	{
		DEALLOCATE(match_graphic_name);
	}
};

int define_Scene_filter(struct Parse_state *state, void *define_scene_filter_data_void,
	void *define_scene_data_void)
{
	int return_code;

	ENTER(define_Scene_filter);
	USE_PARAMETER(define_scene_data_void);
	Define_scene_filter_data *filter_data =
		(struct Define_scene_filter_data *)define_scene_filter_data_void;
	if (state && filter_data)
	{
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_char_flag_entry(option_table, "match_all",
			&(filter_data->match_all));
		Option_table_add_string_entry(option_table, "match_graphic_name",
			&(filter_data->match_graphic_name), " MATCH_NAME");
		Option_table_add_char_flag_entry(option_table, "match_visibility_flags",
			&(filter_data->match_visibility_flags));
		Option_table_add_switch(option_table,
			"show", "hide", &(filter_data->show));
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			int number_of_match_criteria = 
				filter_data->match_all +
				filter_data->match_visibility_flags +
				(NULL != filter_data->match_graphic_name);
			if (1 < number_of_match_criteria)
			{
				display_message(ERROR_MESSAGE,
					"Only one match criterion can be specified per filter.");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Scene_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int define_Scene_contents(struct Parse_state *state, void *scene_void,
	void *define_scene_data_void)
{
	int return_code;

	ENTER(define_Scene_contents);
	struct Define_scene_data *define_scene_data =
		(struct Define_scene_data *)define_scene_data_void;
	if (state && define_scene_data)
	{
		Cmiss_scene *scene = (Cmiss_scene *)scene_void; // can be NULL;
		struct Light *light_to_add = NULL;
		struct Light *light_to_remove = NULL;
		Cmiss_region *region = NULL;
		if (scene && scene->region)
		{
			region = Cmiss_region_access(scene->region);
		}
		else
		{
			region = Cmiss_region_access(define_scene_data->root_region);
		}
		char clear_filters_flag = 0;
		Define_scene_filter_data filter_data;

		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, "add_light", &light_to_add,
			define_scene_data->light_manager, set_Light);
		Option_table_add_char_flag_entry(option_table, "clear_filters",
			&clear_filters_flag);
		Option_table_add_set_Cmiss_region(option_table, "region",
			define_scene_data->root_region, &region);
		Option_table_add_entry(option_table, "remove_light", &light_to_remove,
			define_scene_data->light_manager, set_Light);
		Option_table_add_entry(option_table, "filter",
		  (void *)&filter_data, define_scene_data_void, define_Scene_filter);

		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (scene)
			{
				Scene_begin_cache(scene);
				if (light_to_add)
				{
					Scene_add_light(scene,light_to_add);
				}
				if (light_to_remove)
				{
					Scene_remove_light(scene,light_to_remove);
				}
				Cmiss_scene_set_region(scene, region);
				if (clear_filters_flag)
				{
					Cmiss_scene_clear_filters(scene);
				}
				Cmiss_scene_filter *filter = NULL; 
				if (filter_data.match_all)
				{
					filter = Cmiss_scene_create_filter_all(scene);
				}
				else if (filter_data.match_visibility_flags)
				{
					filter = Cmiss_scene_create_filter_visibility_flags(scene);
				}
				else if (filter_data.match_graphic_name)
				{
					filter = Cmiss_scene_create_filter_graphic_name(scene,
						filter_data.match_graphic_name);
				}
				if (filter)
				{
					Cmiss_scene_filter_set_action(filter,
						filter_data.show ? CMISS_SCENE_FILTER_SHOW : CMISS_SCENE_FILTER_HIDE);
					Cmiss_scene_filter_destroy(&filter);
				}
				Scene_end_cache(scene);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (light_to_add)
		{
			DEACCESS(Light)(&light_to_add);
		}
		if (light_to_remove)
		{
			DEACCESS(Light)(&light_to_remove);
		}
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Scene_contents.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int define_Scene(struct Parse_state *state, void *dummy_to_be_modified,
	void *define_scene_data_void)
{
	int return_code;
	const char *current_token;
	struct Option_table *option_table;
	struct Define_scene_data *define_scene_data;

	ENTER(define_Scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (define_scene_data =
		(struct Define_scene_data *)define_scene_data_void))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				return_code = 1;
				Cmiss_scene *scene = FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
					current_token, define_scene_data->scene_manager);
				if (scene)
				{
					shift_Parse_state(state,1);
					return_code = define_Scene_contents(state, (void *)scene, define_scene_data_void);
				}
				else
				{
					scene = CREATE(Cmiss_scene)();
					if ((!Cmiss_scene_set_region(scene, define_scene_data->root_region)) ||
						(!Cmiss_scene_set_name(scene, current_token)))
					{
						return_code = 0;
					}
					shift_Parse_state(state,1);
					if (state->current_token)
					{
						if (!define_Scene_contents(state, (void *)scene, define_scene_data_void))
						{
							return_code = 0;
						}
					}
					if (return_code)
					{
						ADD_OBJECT_TO_MANAGER(Scene)(scene, define_scene_data->scene_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx define scene:  scene not created due to errors");
					}
					DEACCESS(Scene)(&scene);
				}
			}
			else
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "SCENE_NAME",
					/*scene*/(void *)NULL, define_scene_data_void,
					define_Scene_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing scene name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "define_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int list_Scene(struct Scene *scene,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Writes the properties of the <scene> to the command window.
==============================================================================*/
{
	int return_code;

	ENTER(list_Scene);
	USE_PARAMETER(dummy_void);
	if (scene)
	{
		display_message(INFORMATION_MESSAGE,"Scene : %s\n",scene->name);
		char *region_path = NULL;
		if (scene->region)
		{
			region_path = Cmiss_region_get_path(scene->region);
		}
		else
		{
			region_path = duplicate_string("none");
		}
		display_message(INFORMATION_MESSAGE,"  top region: %s\n", region_path);
		DEALLOCATE(region_path);
		int number_of_filters = scene->filters->size();
		if (0 == number_of_filters)
		{
			display_message(INFORMATION_MESSAGE,"  no filters\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  filters:\n");
			for (int i = 0; i < number_of_filters; i++)
			{
				Cmiss_scene_filter *filter = scene->filters->at(i);
				display_message(INFORMATION_MESSAGE,"    %d. ", i + 1);
				filter->list();
			}
		}
		display_message(INFORMATION_MESSAGE,"  access count = %d\n",
			scene->access_count);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Scene */

#if defined (OLD_CODE)
int gfx_modify_g_element_general(struct Parse_state *state,
	void *cmiss_region_void, void *scene_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT GENERAL command.
Allows general element_group settings to be changed (eg. discretization) and
updates graphics of settings affected by the changes (probably all).
==============================================================================*/
{
	int circle_discretization, clear_flag, return_code;
	struct Cmiss_region *cmiss_region;
	struct Computed_field *default_coordinate_field;
	struct FE_field *native_discretization_field;
	struct Set_FE_field_conditional_FE_region_data
		native_discretization_field_conditional_data;
	struct Element_discretization element_discretization;
// 	struct GT_element_group *gt_element_group;
// 	struct GT_element_settings *settings;
	struct Option_table *option_table;
	struct Scene *scene;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(gfx_modify_g_element_general);
	if (state && (cmiss_region = (struct Cmiss_region *)cmiss_region_void))
	{
		/* get default scene */
		if (scene = (struct Scene *)scene_void)
		{
			default_coordinate_field = NULL;
			native_discretization_field = NULL;
#if defined (TO_BE_EDITED)
			/* if possible, get defaults from element_group on default scene */
			if (gt_element_group=Scene_get_graphical_element_group(scene,
						cmiss_region))
		  {
				if (default_coordinate_field=
					GT_element_group_get_default_coordinate_field(gt_element_group))
				{
					ACCESS(Computed_field)(default_coordinate_field);
				}
				circle_discretization=GT_element_group_get_circle_discretization(
					gt_element_group);
				GT_element_group_get_element_discretization(gt_element_group,
					&element_discretization);
				if (native_discretization_field=
					GT_element_group_get_native_discretization_field(gt_element_group))
				{
					ACCESS(FE_field)(native_discretization_field);
				}
			}
			else
			{
				circle_discretization=-1;
				element_discretization.number_in_xi1=-1;
				element_discretization.number_in_xi2=-1;
				element_discretization.number_in_xi3=-1;
				default_coordinate_field=(struct Computed_field *)NULL;
				native_discretization_field=(struct FE_field *)NULL;
			}
#endif
			/* ACCESS scene for use by set_Scene */
			clear_flag=0;
			ACCESS(Scene)(scene);

			option_table = CREATE(Option_table)();
			/* circle_discretization */
			Option_table_add_entry(option_table, "circle_discretization",
				(void *)&circle_discretization, (void *)(scene->user_interface),
				set_Circle_discretization);
			/* clear */
			Option_table_add_entry(option_table, "clear",
				(void *)&clear_flag, NULL, set_char_flag);
			/* default_coordinate */
			set_coordinate_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(cmiss_region);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table, "default_coordinate",
				(void *)&default_coordinate_field, (void *)&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* element_discretization */
			Option_table_add_entry(option_table, "element_discretization",
				(void *)&element_discretization, (void *)(scene->user_interface),
				set_Element_discretization);
			/* native_discretization */
			native_discretization_field_conditional_data.conditional_function = 
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
			native_discretization_field_conditional_data.user_data = (void *)NULL;
			native_discretization_field_conditional_data.fe_region =
				Cmiss_region_get_FE_region(scene->root_region);
			Option_table_add_entry(option_table, "native_discretization",
				(void *)&native_discretization_field,
				(void *)&native_discretization_field_conditional_data,
				set_FE_field_conditional_FE_region);
			/* scene */
			Option_table_add_entry(option_table, "scene",
				(void *)&scene, (void *)(scene->scene_manager), set_Scene);
			if (return_code = Option_table_multi_parse(option_table, state))
			{
				/* scene may have changed so get gt_element_group again */
#if defined (TO_BE_EDITED)
				if (gt_element_group=Scene_get_graphical_element_group(scene,
					cmiss_region))
				{
					GT_element_group_begin_cache(gt_element_group);
					if (clear_flag)
					{
						/* remove all settings from group */
						while (settings=
							first_settings_in_GT_element_group_that(gt_element_group,
								(LIST_CONDITIONAL_FUNCTION(GT_element_settings) *)NULL,
								(void *)NULL))
						{
							GT_element_group_remove_settings(gt_element_group, settings);
						}
					}
					GT_element_group_set_circle_discretization(gt_element_group,
						circle_discretization);
					GT_element_group_set_element_discretization(gt_element_group,
						&element_discretization);
					if (default_coordinate_field)
					{
						GT_element_group_set_default_coordinate_field(gt_element_group,
							default_coordinate_field);
					}
					GT_element_group_set_native_discretization_field(gt_element_group,
						native_discretization_field);
					/* regenerate graphics for changed settings */
					GT_element_group_end_cache(gt_element_group);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_g_element_general.  Missing gt_element_group");
					return_code=0;
				}
#else
				return_code = 0;
#endif
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (default_coordinate_field)
			{
				DEACCESS(Computed_field)(&default_coordinate_field);
			}
			if (native_discretization_field)
			{
				DEACCESS(FE_field)(&native_discretization_field);
			}
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_g_element_general.  Missing scene");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_g_element_general.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_general */
#endif /* defined (OLD_CODE) */

struct Scene_graphics_object_iterator_data
{
	const char *graphic_name;
	graphics_object_tree_iterator_function iterator_function;
	void *user_data;
	Cmiss_scene *scene;
};

#if defined (TO_BE_EDITED)
static int Scene_graphics_objects_in_GT_element_settings_iterator(
	struct GT_element_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct GT_object *graphics_object;
	struct Scene_graphics_object_iterator_data *data;
 
	ENTER(Scene_graphics_objects_in_GT_element_settings_iterator);
	if (settings && (data = (struct Scene_graphics_object_iterator_data *)data_void))
	{
		if (GT_element_settings_get_visibility(settings) &&
			(graphics_object = GT_element_settings_get_graphics_object(
			settings)))
		{
			(data->iterator_function)(graphics_object, 0.0, data->user_data);			
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_graphics_objects_in_GT_element_settings_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_graphics_objects_in_GT_element_settings_iterator */
#endif

#if defined (USE_SCENE_OBJECT)
static int Scene_graphics_objects_in_Scene_object_iterator(
	struct Scene_object *scene_object, void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
==============================================================================*/
{
	double time;
	int return_code;
	struct GT_object *graphics_object;
	struct Scene_graphics_object_iterator_data *data;
 
	ENTER(Scene_graphics_objects_in_Scene_object_iterator);
	if (scene_object && (data = (struct Scene_graphics_object_iterator_data *)data_void))
	{
		if (g_VISIBLE == Scene_object_get_visibility(scene_object))
		{
			switch(scene_object->type)
			{
				case SCENE_OBJECT_GRAPHICS_OBJECT:
				{
					if(Scene_object_has_time(scene_object))
					{
						time = Scene_object_get_time(scene_object);
					}
					else
					{
						time = 0;
					}
					graphics_object = scene_object->gt_object;
					while ( graphics_object )
					{
						(data->iterator_function)(graphics_object, time, data->user_data);
						graphics_object = GT_object_get_next_object(graphics_object);
					}
				} break;
				case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
				{
					for_each_settings_in_GT_element_group(scene_object->gt_element_group,
						Scene_graphics_objects_in_GT_element_settings_iterator, data_void);
				} break;
				case SCENE_OBJECT_SCENE:
				{
					for_each_Scene_object_in_Scene(scene_object->child_scene,
						Scene_graphics_objects_in_Scene_object_iterator, data_void);
				} break;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_graphics_objects_in_Scene_object_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* Scene_graphics_objects_in_Scene_object_iterator */
#endif /* defined (USE_SCENE_OBJECT) */

static int Scene_graphics_objects_in_Cmiss_graphic_iterator(
	struct Cmiss_graphic *graphic, void *data_void)
{
	int return_code;
	struct GT_object *graphics_object;
	struct Scene_graphics_object_iterator_data *data;
 
	ENTER(Scene_graphics_objects_in_Cmiss_graphic_iterator);
	if (graphic && (data = (struct Scene_graphics_object_iterator_data *)data_void))
	{
		if (!data->graphic_name || 
			Cmiss_graphic_has_name(graphic, (void *)data->graphic_name))
		{
			if (Cmiss_scene_shows_graphic(data->scene, graphic) &&
				(graphics_object = Cmiss_graphic_get_graphics_object(
					 graphic)))
			{
				(data->iterator_function)(graphics_object, 0.0, data->user_data);			
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_graphics_objects_in_Cmiss_graphic_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int for_each_graphics_object_in_scene(struct Scene *scene,
	graphics_object_tree_iterator_function iterator_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
This function iterates through every graphics object in the scene
including those in each settings of the graphical finite
elements and those chained together with other graphics objects
==============================================================================*/
{
	int return_code = 0;
	struct Scene_graphics_object_iterator_data data;

	ENTER(for_each_graphics_object_in_scene);

	if (scene && iterator_function && user_data)
	{
		data.iterator_function = iterator_function;
		data.user_data = user_data;
		data.graphic_name = NULL;
		data.scene = scene;
		Rendition_set::iterator pos =
			scene->list_of_rendition->begin();
		while (pos != scene->list_of_rendition->end())
		{
			return_code = for_each_graphic_in_Cmiss_rendition(*pos,
				Scene_graphics_objects_in_Cmiss_graphic_iterator, (void *)&data);
			++pos;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_graphics_object_in_scene.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_each_graphics_object_in_scene */

static int Scene_get_data_range_for_spectrum_iterator(
	struct GT_object *graphics_object, double time, void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range to include the data values of any of the graphics object
in the <scene> which point to this spectrum.
==============================================================================*/
{
	int return_code;
	struct Scene_get_data_range_for_spectrum_data *data;
	
	ENTER(Scene_get_data_range_for_spectrum_iterator);
	USE_PARAMETER(time);
	if (graphics_object &&
		(data = (struct Scene_get_data_range_for_spectrum_data *)data_void))
	{
		if (get_GT_object_spectrum(graphics_object) == data->spectrum )
		{
			get_graphics_object_data_range(graphics_object,
				(void *)&(data->range));
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_data_range_for_spectrum_iterator.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Scene_get_data_range_for_spectrum_iterator */

int Scene_get_data_range_for_spectrum(struct Scene *scene,
	struct Spectrum *spectrum, float *minimum, float *maximum,
	int *range_set)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Expands the range to include the data values of any of the graphics object
in the <scene> which point to this spectrum.
==============================================================================*/
{
	int return_code;
	struct Scene_get_data_range_for_spectrum_data data;

	ENTER(Scene_get_data_range_for_spectrum);

	if ( scene && spectrum )
	{
		/* must ensure the scene is built first! */
		build_Scene(scene);

		data.spectrum = spectrum;
		data.range.first = 1;
		data.range.minimum = 0;
		data.range.maximum = 0;
		for_each_graphics_object_in_scene(scene,
			Scene_get_data_range_for_spectrum_iterator, (void *)&data);
		if ( data.range.first )
		{
			*range_set = 0;
		}
		else
		{
			*range_set = 1;
			*minimum = data.range.minimum;
			*maximum = data.range.maximum;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_get_data_range_for_spectrum.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Scene_get_data_range_for_spectrum */

struct Temp_data
{
	 const char *write_into_comfile;
	 const char *name;
};

#if defined (TO_BE_EDITED)
int list_scene_object_in_scene_get_command_list(struct Scene_object *scene_object,
	 void *temp_data_void)
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct Temp_data *temp_data;
	ENTER(for_each_graphics_object_in_scene_get_command_list);
	return_code = 0;
	if (scene_object)
	{
		 return_code = 1;
		 switch (scene_object->type)
		 {
				case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
				{
					 if (temp_data = (struct Temp_data *)temp_data_void)
					 {
							int error = 0;
							char *command_prefix, *command_suffix;
							char *region_path;
							command_prefix = duplicate_string("gfx modify g_element ");
							region_path = (char *)scene_object->name;
							make_valid_token(&region_path);
							append_string(&command_prefix, region_path, &error);
							append_string(&command_prefix, " ", &error);
							command_suffix = (char *)NULL;
							if (temp_data->name)
							{
								 if (strcmp(temp_data->name, "default") != 0)
								 {
										append_string(&command_suffix, " scene ", &error);
										append_string(&command_suffix,temp_data->name, &error);
								 }
							}
							append_string(&command_suffix, ";", &error);
							return_code = 0;
							gt_element_group = Scene_object_get_graphical_element_group(
								 scene_object);
							if (strcmp(temp_data->write_into_comfile, "false") == 0)
							{
								 return_code = GT_element_group_list_commands(gt_element_group,
										command_prefix, command_suffix);
							}
							else
							{
								 return_code = GT_element_group_write_commands_to_comfile(
										gt_element_group, command_prefix, command_suffix);
							}
							DEALLOCATE(command_suffix);
							DEALLOCATE(command_prefix);
					 }
				} break;
				default:
				{
				} break;
		 }
	}
	LEAVE;

	return (return_code);
}

#endif /* defined (TO_BE_EDITED) */

char *Cmiss_scene_get_name(struct Scene *scene)
{
	if (scene && scene->name)
		return duplicate_string(scene->name);
	return NULL;
}

int Cmiss_scene_set_name(struct Scene *scene, const char *name)
{
	int return_code;
	
	ENTER(Cmiss_scene_set_name);
	if (scene && name)
	{
		if (scene->scene_manager)
		{
			return_code = MANAGER_MODIFY_IDENTIFIER(Scene, name)
				(scene, name, scene->scene_manager);
		}
		else
		{
			if (scene->name)
			{
				DEALLOCATE(scene->name);
			}
			if (scene->name=duplicate_string(name))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					" Cmiss_scene_set_name. Cannot duplicate name to scene");
			}
		}
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_set_name. Invalid argument(s)");
	}
	return (return_code);
}

struct Cmiss_region *Cmiss_scene_get_region(Scene *scene)
{
	struct Cmiss_region *region;

	ENTER(Cmiss_scene_get_region);
	if (scene)
	{
		region = scene->region;
		if (region)
		{
			Cmiss_region_access(region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_get_region.  Invalid argument(s)");	
		region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return region;
}

int Cmiss_scene_destroy(Cmiss_scene **scene_address)
{
	return DEACCESS(Scene)(scene_address);
}

static int Scene_rendition_update_callback(struct Cmiss_rendition *rendition,
	void *scene_void)
{
	int return_code;
	struct Scene *scene;

	ENTER(Scene_rendition_update_callback);
	if (rendition &&(scene = (struct Scene *)scene_void))
	{
		Scene_notify_object_changed(scene,0);
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Unset rendition on scene.
 * @param scene The scene to be set
 * @return If successfully disable rendition returns 1 else 0.
 */
int Cmiss_scene_disable_renditions(Scene *scene)
{
	int return_code;

	ENTER(Cmiss_scene_disable_renditions);
	if (scene)
	{
		if (scene->region)
		{
			struct Cmiss_rendition *rendition =
				Cmiss_region_get_rendition_internal(scene->region);
			if (rendition)
			{
				Cmiss_rendition_remove_callback(rendition,
					Scene_rendition_update_callback,
					(void *)scene);
				Cmiss_rendition_unset_graphics_managers_callback(rendition);
				DEACCESS(Cmiss_rendition)(&rendition);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_disable_renditions. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Enable rendition on scene. This function will add rendition to top region
 * and its child regions in scene if they do not already have an rendition.
 * This function will also set appropriate callback to the top region's
 * rendition to avoid multiple calls of the same callback functions in the same
 * region.
 *
 * @param scene The scene to be set
 * @return If successfully enable rendition returns 1 else 0.
 */
int Cmiss_scene_enable_renditions(Scene *scene)
{
	int return_code;

	ENTER(Cmiss_scene_enable_renditions);
	if (scene)
	{
		if (scene->region)
		{
			struct Cmiss_rendition *rendition;
			if (rendition = Cmiss_region_get_rendition_internal(scene->region))
			{
				Cmiss_rendition_add_scene(rendition,scene,/*hieracrical*/1);
				Cmiss_rendition_add_callback(rendition,
					Scene_rendition_update_callback, (void *)scene);
				Cmiss_rendition_set_graphics_managers_callback(rendition);
				DEACCESS(Cmiss_rendition)(&rendition);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_enable_renditions. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_scene_set_region(Scene *scene, Cmiss_region *region)
{
  int return_code = 0;

	ENTER(Cmiss_scene_set_region);
	if (scene && region)
	{
		if (scene->region != region)
		{
			Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
			if (rendition)
			{
				if (scene->list_of_rendition)
				{
					if (!scene->list_of_rendition->empty())
					{
						Rendition_set::iterator pos =
							scene->list_of_rendition->begin();
						while (pos != scene->list_of_rendition->end())
						{
							Cmiss_rendition_remove_scene(*pos, scene);
							++pos;
						}
						scene->list_of_rendition->clear();
					}
				}
				else
				{
					scene->list_of_rendition = new Rendition_set;
				}
				Cmiss_scene_disable_renditions(scene);
				REACCESS(Cmiss_region)(&(scene->region), region);
				Cmiss_rendition_add_scene(rendition, scene, 1);
				DEACCESS(Cmiss_rendition)(&rendition);
				Scene_changed_private(scene, 0);
				Cmiss_scene_enable_renditions(scene);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					" Cmiss_scene_set_region. Region does not have a rendition");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_set_region. Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

struct Scene *Cmiss_scene_create(void)
{
	Scene *scene;

	ENTER(Cmiss_scene_create);
	if (!(scene = CREATE(Scene)()))
	{
		scene = NULL;
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_create. Cannot create scene");
	}
	LEAVE;

	return (scene);
}

int Scene_rendition_changed(
	struct Scene *scene,struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Scene_rendition_changed);
	USE_PARAMETER(rendition);
	if (scene)
	{
		return_code = 0;
		/* mark scene as needing a build */
		if (scene->list_of_rendition &&	
			(scene->list_of_rendition->find(rendition)!=scene->list_of_rendition->end()))
		{
			scene->build = 1;
			scene->compile_status = GRAPHICS_NOT_COMPILED;
			scene->change_status = SCENE_CHANGE;
			if (scene->scene_manager)
			{
				return_code = Scene_refresh( scene );
			}
			else
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_changed_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_changed_private */

int Cmiss_scene_add_rendition(Scene *scene, struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_scene_add_rendition);

	if (scene && rendition)
	{
		if (!scene->list_of_rendition)
		{
			scene->list_of_rendition = new Rendition_set;
		}
		scene->list_of_rendition->insert(rendition);
		Scene_changed_private(scene, /*fast_changing*/ 0);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_add_rendition. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_scene_remove_rendition(Scene *scene, struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_scene_remove_rendition);

	if (scene && scene->list_of_rendition && rendition )
	{
		/* cannot use erase(renditon) here as our sorting algorithm uses the region
		 * tree and at this time in the program, the pointer to theregion to be deleted
		 * may have already been removed from its parent.
		 * The following code may not be great performance wise but its safe.
		 */
		Rendition_set::iterator pos =
			scene->list_of_rendition->begin();
		while (pos != scene->list_of_rendition->end())
		{
			if (*pos == rendition)
			{
				scene->list_of_rendition->erase(pos);
				break;
			}
			++pos;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" Cmiss_scene_remove_rendition. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Scene_export_region_graphics_object(Scene *scene, Cmiss_region *region,const char *graphic_name,
	graphics_object_tree_iterator_function iterator_function, void *user_data)
{
	int return_code = 0;
	struct Scene_graphics_object_iterator_data data;
	
	ENTER(Scene_export_region_graphics_object);
	
	if (scene && region && iterator_function && user_data)
	{
		data.iterator_function = iterator_function;
		data.user_data = user_data;
		data.graphic_name = graphic_name;
		data.scene = scene;
		struct Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
		if (rendition)
		{
			Rendition_set::iterator pos =
				scene->list_of_rendition->find(rendition);
			if (pos != scene->list_of_rendition->end())
			{
				return_code = for_each_graphic_in_Cmiss_rendition(rendition,
					Scene_graphics_objects_in_Cmiss_graphic_iterator, (void *)&data);
			}
			DEACCESS(Cmiss_rendition)(&rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_export_region_graphics_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Scene_add_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object, const char *cmiss_graphic_name)
{
	char *graphics_object_name = NULL;
	int return_code = 0;
	struct Cmiss_rendition *rendition = NULL;

	ENTER(Scene_add_graphics_object);
	if (scene && graphics_object)
	{
		if (scene->region && 
			(NULL != (rendition = Cmiss_region_get_rendition_internal(scene->region))))
		{
			if (!cmiss_graphic_name)
			{
				GET_NAME(GT_object)(graphics_object, &graphics_object_name);
				cmiss_graphic_name = graphics_object_name;
			}
			return_code = Cmiss_rendition_add_glyph(rendition, graphics_object, 
				cmiss_graphic_name);
			DEACCESS(Cmiss_rendition)(&rendition);
			if (graphics_object_name)
			{
					DEALLOCATE(graphics_object_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Scene_add_graphics_object */

int Cmiss_scene_clear_filters(Cmiss_scene *scene)
{
	int return_code = 1;
	if (scene)
	{
		int number_of_filters = scene->filters->size();
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_scene_filter *filter = scene->filters->at(i);
			Cmiss_scene_filter_destroy(&filter);
		}
		scene->filters->clear();
		if (scene->list_of_rendition)
		{
			scene->build = 1;
			scene->compile_status = GRAPHICS_NOT_COMPILED;
			scene->change_status = SCENE_CHANGE;
			if (scene->scene_manager)
			{
				Scene_refresh( scene );
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_clear_filters.  Invalid argument");
		return_code = 0;
	}
	return return_code;
}

int Scene_add_filter_private(Cmiss_scene *scene, Cmiss_scene_filter *filter)
{
	int return_code = 1;
	if (scene && filter)
	{
		scene->filters->push_back(Cmiss_scene_filter_access(filter));
		if (return_code && scene->list_of_rendition)
		{
			scene->build = 1;
			scene->compile_status = GRAPHICS_NOT_COMPILED;
			scene->change_status = SCENE_CHANGE;
			if (scene->scene_manager)
			{
				Scene_refresh(scene);
			}
		}
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

int Cmiss_scene_remove_filter(Cmiss_scene *scene, Cmiss_scene_filter *filter)
{
	int return_code = 0;
	if (scene && filter && (filter->getScene() == scene))
	{
		int number_of_filters = scene->filters->size();
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_scene_filter *temp_filter = scene->filters->at(i);
			if (temp_filter == filter)
			{
				Cmiss_scene_filter_destroy(&temp_filter);
				scene->filters->erase(scene->filters->begin() + i);
				return_code = 1;
				break;
			}
		}
		if (return_code && scene->list_of_rendition)
		{
			scene->build = 1;
			scene->compile_status = GRAPHICS_NOT_COMPILED;
			scene->change_status = SCENE_CHANGE;
			if (scene->scene_manager)
			{
				Scene_refresh(scene);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_remove_filter.  Invalid argument(s)");
	}
	return return_code;
}

int Cmiss_scene_get_number_of_filters(Cmiss_scene *scene)
{
	if (scene)
		return scene->filters->size();
	return 0;
}

int Cmiss_scene_get_filter_priority(Cmiss_scene *scene,
	Cmiss_scene_filter *filter)
{
	int priority = 0;
	if (scene && filter && (filter->getScene() == scene))
	{
		int number_of_filters = scene->filters->size();
		for (int i = 0; i < number_of_filters; i++)
		{
			if (scene->filters->at(i) == filter)
			{
				priority = i + 1;
				break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_get_filter_priority.  Invalid argument(s)");
	}
	return priority;
}

int Cmiss_scene_set_filter_priority(Cmiss_scene *scene,
	Cmiss_scene_filter_id filter, int priority)
{
	int return_code = 0;
	if (scene && filter && (filter->getScene() == scene) &&
		(0 <= priority) && (priority <= (int)scene->filters->size()))
	{
		int number_of_filters = scene->filters->size();
		int new_index = (priority ? priority : number_of_filters) - 1;
		for (int i = 0; i < number_of_filters; i++)
		{
			if (scene->filters->at(i) == filter)
			{
				scene->filters->erase(scene->filters->begin() + i);
				scene->filters->insert(scene->filters->begin() + new_index, filter);
				return_code = 1;
				break;
			}
		}
		if (return_code && scene->list_of_rendition)
		{
			scene->build = 1;
			scene->compile_status = GRAPHICS_NOT_COMPILED;
			scene->change_status = SCENE_CHANGE;
			if (scene->scene_manager)
			{
				Scene_refresh(scene);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_set_filter_priority.  Invalid argument(s)");
	}
	return return_code;
}

struct Cmiss_scene_filter *Cmiss_scene_get_filter_at_priority(
	Cmiss_scene *scene, int priority)
{
	Cmiss_scene_filter *filter = NULL;
	if (scene && (priority > 0) && (priority <= (int)scene->filters->size()))
	{
		filter = Cmiss_scene_filter_access(scene->filters->at(priority));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_get_filter_at_priority.  Invalid argument(s)");
	}
	return filter;
}

int Cmiss_scene_shows_graphic(struct Cmiss_scene *scene, struct Cmiss_graphic *graphic)
{
	int return_code = 0;
	if (scene && graphic)
	{
		/* default for no filters = show nothing */
		return_code = 0;
		int number_of_filters = scene->filters->size();
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_scene_filter *filter = scene->filters->at(i);
			if (filter->match(graphic))
			{
				return_code = (filter->getAction() == CMISS_SCENE_FILTER_SHOW);
				break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_shows_graphic.  Invalid argument(s)");
	}
	return return_code;
}

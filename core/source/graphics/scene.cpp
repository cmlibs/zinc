/***************************************************************************//**
 * FILE : scene.cpp
 *
 * Implementation of Cmiss_scene which describes a collection of graphics able
 * to be output to a Cmiss_scene_viewer or other outputs/devices.
 * It broadly comprises a reference to a region sub-tree and filters controlling
 * which graphics are displayed from its renditions.
 */
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
#include <cstdio>
#include <string>
#include <map>
#include <vector>

#include "zinc/zincconfigure.h"


#include "zinc/scene.h"
#include "zinc/scenepicker.h"
#include "zinc/status.h"
#include "zinc/graphicsfilter.h"
#include "zinc/rendition.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/callback_private.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/enumerator_conversion.hpp"
#include "general/enumerator_private.hpp"
#include "general/indexed_list_private.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/scene.h"
#include "graphics/rendition.h"
#include "graphics/graphics_library.h"
#include "graphics/font.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/light.h"
#include "graphics/texture.h"
#include "graphics/scene_picker.hpp"
#include "time/time.h"
#include "time/time_keeper.hpp"
#include "general/message.h"
#include "graphics/scene.hpp"
#include "graphics/graphics_filter.hpp"
#include "graphics/render_gl.h"

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

FULL_DECLARE_LIST_TYPE(Scene);
FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Scene, Cmiss_graphics_module, void *);

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
DECLARE_MANAGER_OWNER_FUNCTIONS(Scene, struct Cmiss_graphics_module)

int Scene_manager_set_owner(struct MANAGER(Scene) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Scene)(manager, graphics_module);
}

int Scene_compile_members(struct Scene *scene, Render_graphics *renderer)
{
	int return_code;

	ENTER(Scene_compile_members);
	if (scene)
	{
		return_code = 1;
		/* compile objects in the scene */
		if (scene->list_of_rendition)
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
	else
	{
		display_message(ERROR_MESSAGE, "Scene_compile_members.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_compile_members */

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

static int Scene_changed_private(struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Tells the scene it has changed, forcing it to send the manager message
MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER.  Recompiles the scene display list as well
as the objects in the scene unlike the public Scene_changed which only compiles
the component objects.
Private to the Scene and Scene_objects.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_changed_private);
	if (scene)
	{
		/* mark scene as needing a build */
		scene->build = 1;
		scene->change_status = SCENE_CHANGE;
		if (scene->manager)
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

typedef std::multimap<Cmiss_region *, Cmiss_cad_identifier_id> Region_cad_primitive_map;

struct Scene_picked_object_region_cad_primitive_map_data
{
	Region_cad_primitive_map *cad_primitive_map;
	int select_surfaces_enabled, select_lines_enabled;
};

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
	int dimension,return_code, element_type = 0;
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
				fe_region = Cmiss_region_get_FE_region(cmiss_region);
				if (fe_region)
				{
					element_type = Cmiss_graphic_get_domain_dimension(graphic);
					cm.number = Scene_picked_object_get_subobject(scene_picked_object, 1);
					if (element_type == 1)
					{
						cm.type = CM_LINE;
					}
					else if (element_type == 2)
					{
						cm.type = CM_FACE;
					}
					else
					{
						cm.type = CM_ELEMENT;
					}
					element = FE_region_get_FE_element_from_identifier_deprecated(fe_region, &cm);
					if (element)
					{
						dimension = get_FE_element_dimension(element);
						if (((nearest_element_data->select_elements_enabled && ((CM_ELEMENT
							== cm.type) || (3 == dimension)))
							|| (nearest_element_data->select_faces_enabled && ((CM_FACE
								== cm.type) || (2 == dimension)))
							|| (nearest_element_data->select_lines_enabled && ((CM_LINE
								== cm.type) || (1 == dimension))))
							&& ((!nearest_element_data->cmiss_region)
								|| ((fe_region = Cmiss_region_get_FE_region(
									nearest_element_data->cmiss_region))
									&& FE_region_contains_FE_element(fe_region, element))))
						{
							nearest_element_data->nearest_element = element;
							nearest_element_data->scene_picked_object = scene_picked_object;
							nearest_element_data->rendition = rendition;
							nearest_element_data->graphic = graphic;
							nearest_element_data->nearest = Scene_picked_object_get_nearest(
								scene_picked_object);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_picked_object_get_nearest_element.  "
								"Invalid element %s %d", CM_element_type_string(cm.type),
							cm.number);
						return_code = 0;
					}
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
	int element_point_number, i, return_code, element_type = 0;
	struct CM_element_information cm;
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
				(CMISS_GRAPHIC_POINTS == Cmiss_graphic_get_graphic_type(graphic)) &&
				Cmiss_graphic_selects_elements(graphic))
			{
				fe_region = Cmiss_region_get_FE_region(cmiss_region);
				if (fe_region)
				{
					element_type = Cmiss_graphic_get_domain_dimension(graphic);
					cm.number = Scene_picked_object_get_subobject(scene_picked_object, 1);
					if (element_type == 1)
					{
						cm.type = CM_LINE;
					}
					else if (element_type == 2)
					{
						cm.type = CM_FACE;
					}
					else
					{
						cm.type = CM_ELEMENT;
					}
					element = FE_region_get_FE_element_from_identifier_deprecated(fe_region, &cm);
					if (element)
					{
						if ((!nearest_element_point_data->cmiss_region)
							|| FE_region_contains_FE_element(Cmiss_region_get_FE_region(
								nearest_element_point_data->cmiss_region), element))
						{
							int top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
							Cmiss_graphic_get_top_level_number_in_xi(graphic,
								MAXIMUM_ELEMENT_XI_DIMENSIONS, top_level_number_in_xi);
							Cmiss_graphic_face_type face = Cmiss_graphic_get_face(graphic);
							native_discretization_field
								= Cmiss_graphic_get_native_discretization_field(graphic);
							top_level_element = (struct FE_element *) NULL;
							if (get_FE_element_discretization(element,
								(LIST_CONDITIONAL_FUNCTION(FE_element) *)0, (void *)0,
								face, native_discretization_field,
								top_level_number_in_xi, &top_level_element,
								element_point_ranges_identifier.number_in_xi))
							{
								element_point_ranges_identifier.element = element;
								element_point_ranges_identifier.top_level_element
									= top_level_element;
								Cmiss_graphic_get_xi_discretization(graphic,
									&(element_point_ranges_identifier.xi_discretization_mode),
									/*xi_point_density_field*/(struct Computed_field **) NULL);
								if (XI_DISCRETIZATION_EXACT_XI
									== element_point_ranges_identifier.xi_discretization_mode)
								{
									for (i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
									{
										element_point_ranges_identifier.number_in_xi[i] = 1;
									}
								}
								Cmiss_graphic_get_seed_xi(graphic, xi);
								/*???RC temporary, hopefully */
								for (i = 0; i < 3; i++)
								{
									element_point_ranges_identifier.exact_xi[i] = xi[i];
								}
								element_point_ranges = CREATE(Element_point_ranges)(
									&element_point_ranges_identifier);
								if (element_point_ranges != 0)
								{
									element_point_number = Scene_picked_object_get_subobject(
										scene_picked_object, 2);
									if (Element_point_ranges_add_range(element_point_ranges,
										element_point_number, element_point_number))
									{
										if (nearest_element_point_data->nearest_element_point)
										{
											DESTROY(Element_point_ranges)(
												&(nearest_element_point_data->nearest_element_point));
										}
										nearest_element_point_data->nearest_element_point
											= element_point_ranges;
										nearest_element_point_data->scene_picked_object
											= scene_picked_object;
										nearest_element_point_data->rendition = rendition;
										nearest_element_point_data->graphic = graphic;
										nearest_element_point_data->nearest
											= Scene_picked_object_get_nearest(scene_picked_object);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Scene_picked_object_get_nearest_element_point.  "
												"Could not add element point range");
										DESTROY(Element_point_ranges)(&element_point_ranges);
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Scene_picked_object_get_nearest_element_point.  "
											"Could not create Element_point_ranges");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Scene_picked_object_get_nearest_element_point.  "
										"Could not get discretization");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_picked_object_get_nearest_element_point.  "
								"Invalid element %s %d", CM_element_type_string(cm.type),
							cm.number);
						return_code = 0;
					}
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
				(((CMISS_FIELD_DOMAIN_NODES == Cmiss_graphic_get_domain_type(graphic))) &&
					(!nearest_node_data->use_data)) ||
				(((CMISS_FIELD_DOMAIN_DATA == Cmiss_graphic_get_domain_type(graphic)) &&
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
	Cmiss_field_domain_type domain_type; // nodes or data
};

typedef std::multimap<Cmiss_region *, Cmiss_node_id> Region_node_map;

struct Scene_picked_object_region_node_map_data
{
  Region_node_map *node_list;
	Cmiss_field_domain_type domain_type; // nodes or data
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
			(picked_nodes_data->domain_type == Cmiss_graphic_get_domain_type(graphic)) &&
			(cmiss_region = Cmiss_rendition_get_region(rendition)))
		{
			node_number=Scene_picked_object_get_subobject(scene_picked_object,2);
			fe_region = Cmiss_region_get_FE_region(cmiss_region);
			if (picked_nodes_data->domain_type == CMISS_FIELD_DOMAIN_DATA)
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
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	enum Cmiss_field_domain_type domain_type)
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
		picked_nodes_data.domain_type = domain_type;
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

static int Scene_rendition_update_callback(struct Cmiss_rendition *rendition,
	void *scene_void)
{
	int return_code;
	struct Scene *scene;

	ENTER(Scene_rendition_update_callback);
	if (rendition &&(scene = (struct Scene *)scene_void))
	{
		Scene_changed_private(scene);
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
 * Detaches scene from rendition tree starting at the current region.
 * Removes hierarchical change callback from current region.
 *
 * @param scene  The scene to detach. Must have region pointer set.
 * @return  1 on success, otherwise 0.
 */
static int Cmiss_scene_detach_from_renditions(Scene *scene)
{
	int return_code;

	ENTER(Cmiss_scene_detach_from_renditions);
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
				DEACCESS(Cmiss_rendition)(&rendition);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_detach_from_renditions.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Attaches scene to rendition tree starting at the current region for tracking
 * changes. Sets up hierarchical change callback from current region.
 *
 * @param scene  The scene to attach. Must have region pointer set.
 * @return  1 on success, otherwise 0.
 */
static int Cmiss_scene_attach_to_renditions(Scene *scene)
{
	int return_code;

	ENTER(Cmiss_scene_attach_to_renditions);
	if (scene && scene->region)
	{
		struct Cmiss_rendition *rendition;
		rendition = Cmiss_region_get_rendition_internal(scene->region);
		if (rendition != 0)
		{
			Cmiss_rendition_add_scene(rendition, scene, /*hierarchical*/1);
			Cmiss_rendition_add_callback(rendition,
				Scene_rendition_update_callback, (void *)scene);
			DEACCESS(Cmiss_rendition)(&rendition);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_attach_to_renditions.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct Scene *CREATE(Scene)(void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Scene now has pointer to its manager, and it uses manager modify
messages to inform its clients of changes. The pointer to the manager
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
		scene->manager=(struct MANAGER(Scene) *)NULL;
		/* fields, elements, nodes and data */
		scene->region = NULL;
		scene->light_manager=(struct MANAGER(Light) *)NULL;
		scene->light_manager_callback_id=(void *)NULL;
		scene->list_of_lights=CREATE(LIST(Light))();
		scene->list_of_rendition = NULL;
		scene->manager_change_status = MANAGER_CHANGE_NONE(Scene);
		scene->is_managed_flag = false;
		scene->filter = NULL;;
		scene->cache = 0;
		scene->build = 1;
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
			DEALLOCATE(scene->name);

			Cmiss_scene_detach_from_renditions(scene);
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
			if (scene->filter)
			{
				DEACCESS(Cmiss_graphics_filter)(&scene->filter);
			}
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
				if (scene->manager)
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

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Scene)
DECLARE_LIST_FUNCTIONS(Scene)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene,name,const char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Scene,name)

DECLARE_ACCESS_OBJECT_FUNCTION(Scene)

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(Scene)
{
	int return_code;
	struct Scene *object;

	ENTER(DEACCESS(Scene));
	if (object_address && (object = *object_address))
	{
		(object->access_count)--;
		if (object->access_count <= 0)
		{
			return_code = DESTROY(Scene)(object_address);
		}
		else if ((!object->is_managed_flag) && (object->manager) &&
			((1 == object->access_count) || ((2 == object->access_count) &&
				(MANAGER_CHANGE_NONE(Scene) != object->manager_change_status))))
		{
			return_code =
				REMOVE_OBJECT_FROM_MANAGER(Scene)(object, object->manager);
		}
		else
		{
			return_code = 1;
		}
		*object_address = (struct Scene *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Scene) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(Scene)
{
	int return_code;

	ENTER(REACCESS(Scene));
	if (object_address)
	{
		return_code = 1;
		if (new_object)
		{
			/* access the new object */
			(new_object->access_count)++;
		}
		if (*object_address)
		{
			/* deaccess the current object */
			DEACCESS(Scene)(object_address);
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(Scene).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(Scene) */

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
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name)(
				destination, source);
			if (return_code)
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
	int return_code = 0;
// 	struct LIST(Light) *temp_list_of_lights;
#if defined (USE_SCENE_OBJECT)
	struct LIST(Scene_object) *temp_scene_object_list;
#endif /* defined (USE_SCENE_OBJECT) */

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Scene,name));
	if (source && destination)
	{
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
			/* NOTE: MUST NOT COPY MANAGER! */
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
	const char *destination_name = NULL;
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

DECLARE_MANAGER_FUNCTIONS(Scene,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Scene,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS( \
	Scene,name,const char *,manager)

#if defined (USE_SCENE_OBJECT)
int for_each_Scene_object_in_Scene(struct Scene *scene,
	LIST_ITERATOR_FUNCTION(Scene_object) *iterator_function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 23 December 1997

DESCRIPTION :
Allows clients of the <scene> to perform functions with the scene_objects in
it. For example, render_vrml.c needs to output all the window objects in a scene.
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
		any_object_list=CREATE(LIST(Any_object))();
		if (any_object_list != 0)
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
	if (scene_picked_object && cad_primitive_data)
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
				(Cmiss_graphic_selects_cad_primitives(graphic)))
			{
				cad.number = -1;
				cad.type = Cad_primitive_INVALID;
				Cmiss_field_id coordinate_field = Cmiss_graphic_get_coordinate_field(graphic);
				if (Cmiss_field_is_cad_geometry(coordinate_field, NULL))
				{
					cad.number = Scene_picked_object_get_subobject(scene_picked_object,1);
					if (Cmiss_graphic_get_graphic_type(graphic) == CMISS_GRAPHIC_SURFACES)
					{
						cad.type = Cad_primitive_SURFACE;
					}
					else if (Cmiss_graphic_get_graphic_type(graphic) == CMISS_GRAPHIC_LINES)
					{
						cad.type = Cad_primitive_CURVE;
						//DEBUG_PRINT("cad number: %d\n", cad.number);
					}
					if (cad.type != Cad_primitive_INVALID)
					{
						struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
						return_code = Computed_field_get_domain( coordinate_field, domain_field_list );
						if ( return_code )
						{
							struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
								( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
							if ( cad_topology_field )
							{
								if (cad_primitive_data->nearest_cad_element)
									delete cad_primitive_data->nearest_cad_element;

								//DEBUG_PRINT("nearest cad element %d\n", Cmiss_field_get_access_count(cad_topology_field));
								Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology(cad_topology_field);
								Cmiss_cad_identifier_id cad_shape = new Cmiss_cad_identifier(cad_topology, cad);
								Cmiss_field_cad_topology_destroy(&cad_topology);

								cad_primitive_data->nearest_cad_element = cad_shape;
								cad_primitive_data->graphic = graphic;
								cad_primitive_data->scene_picked_object = scene_picked_object;
								cad_primitive_data->rendition = rendition;
								cad_primitive_data->nearest = Scene_picked_object_get_nearest(scene_picked_object);
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
				Cmiss_field_destroy(&coordinate_field);
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

static int Scene_picked_object_get_picked_region_cad_primitives(
	struct Scene_picked_object *scene_picked_object,void *picked_cad_primitive_data_void)
{
	int return_code = 0;
	struct Cad_primitive_identifier cad;
	struct Cmiss_region *cmiss_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_region_cad_primitive_map_data *picked_cad_primitive_data;

	if (scene_picked_object&&(picked_cad_primitive_data=
		(struct Scene_picked_object_region_cad_primitive_map_data *)picked_cad_primitive_data_void))
	{
		return_code = 1;
		/* is the last scene_object a Graphical_element wrapper, and does the
			 settings for the graphic refer to cad primitives? */
		if ((NULL != (rendition=Scene_picked_object_get_rendition(scene_picked_object,
			Scene_picked_object_get_number_of_renditions(scene_picked_object)-1)))
			&&((cmiss_region = Cmiss_rendition_get_region(rendition)))
			&&(2<=Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
			(NULL != (graphic=Cmiss_rendition_get_graphic_at_position(rendition,
			Scene_picked_object_get_subobject(scene_picked_object,0))))&&
			(Cmiss_graphic_selects_cad_primitives(graphic)))
		{
			cad.number = -1;
			cad.type = Cad_primitive_INVALID;
			Cmiss_field_id coordinate_field = Cmiss_graphic_get_coordinate_field(graphic);
			if (Cmiss_field_is_cad_geometry(coordinate_field, NULL))
			{
				cad.number = Scene_picked_object_get_subobject(scene_picked_object,1);
				if (Cmiss_graphic_get_graphic_type(graphic) == CMISS_GRAPHIC_SURFACES)
				{
					cad.type = Cad_primitive_SURFACE;
				}
				else if (Cmiss_graphic_get_graphic_type(graphic) == CMISS_GRAPHIC_LINES)
				{
					cad.type = Cad_primitive_CURVE;
				}
				if (cad.type != Cad_primitive_INVALID)
				{
					struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
					return_code = Computed_field_get_domain( coordinate_field, domain_field_list );
					if ( return_code )
					{
						struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
							( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
						if ( cad_topology_field )
						{
							Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology(cad_topology_field);
							Cmiss_cad_identifier_id cad_shape_identifier = new Cmiss_cad_identifier(cad_topology, cad);
							Cmiss_field_cad_topology_destroy(&cad_topology);

							picked_cad_primitive_data->cad_primitive_map->insert(std::make_pair(cmiss_region, cad_shape_identifier));
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
			Cmiss_field_destroy(&coordinate_field);
		}
		if (graphic)
			Cmiss_graphic_destroy(&graphic);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_picked_region_cad_primitives.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Scene_picked_object_get_picked_region_cad_primitives */

void *Scene_picked_object_list_get_picked_region_cad_primitives(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int select_surfaces_enabled, int select_lines_enabled)
{
	struct Scene_picked_object_region_cad_primitive_map_data picked_cad_primitive_data;

	if (scene_picked_object_list)
	{
		picked_cad_primitive_data.select_surfaces_enabled=select_surfaces_enabled;
		picked_cad_primitive_data.select_lines_enabled=select_lines_enabled;
		picked_cad_primitive_data.cad_primitive_map = new Region_cad_primitive_map();
		if (picked_cad_primitive_data.cad_primitive_map)
		{
			FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
				Scene_picked_object_get_picked_region_cad_primitives,(void *)&picked_cad_primitive_data,
				scene_picked_object_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_list_get_picked_elements.  Invalid argument(s)");
		picked_cad_primitive_data.cad_primitive_map = NULL;
	}

	return ((void *)picked_cad_primitive_data.cad_primitive_map);
} /* Scene_picked_object_list_get_picked_region_cad_primitives */

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
	int dimension, return_code, element_type = 0;
	struct Cmiss_region *cmiss_region;
	struct FE_element *element;
	struct FE_region *fe_region;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic = NULL;
	struct Scene_picked_object_region_element_map_data *picked_elements_data;
	struct CM_element_information cm;
	cm.type = CM_ELEMENT_TYPE_INVALID;
	cm.number = -1;

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
			fe_region = Cmiss_region_get_FE_region(cmiss_region);
			if (fe_region)
			{
				element_type = Cmiss_graphic_get_domain_dimension(graphic);
				cm.number = Scene_picked_object_get_subobject(scene_picked_object, 1);
				if (element_type == 1)
				{
					cm.type = CM_LINE;
				}
				else if (element_type == 2)
				{
					cm.type = CM_FACE;
				}
				else
				{
					cm.type = CM_ELEMENT;
				}
				element = FE_region_get_FE_element_from_identifier_deprecated(fe_region, &cm);
				if (element)
				{
					dimension = get_FE_element_dimension(element);
					if ((picked_elements_data->select_elements_enabled && ((CM_ELEMENT
						== cm.type) || (3 == dimension)))
						|| (picked_elements_data->select_faces_enabled && ((CM_FACE
							== cm.type) || (2 == dimension)))
						|| (picked_elements_data->select_lines_enabled && ((CM_LINE
							== cm.type) || (1 == dimension))))
					{
						picked_elements_data->element_list->insert(std::make_pair(
							cmiss_region, element));

					}
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
	int select_elements_enabled, int select_faces_enabled,
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
		picked_element_points_list=CREATE(LIST(Element_point_ranges))();
		if (picked_element_points_list != 0)
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
				scene_input_data.picked_object_list=
					CREATE(LIST(Scene_picked_object))();
				if (scene_input_data.picked_object_list != 0)
				{
					select_buffer_ptr=select_buffer;
					for (hit_no=0;(hit_no<num_hits)&&return_code;hit_no++)
					{
						scene_picked_object=CREATE(Scene_picked_object)(hit_no);
						if (scene_picked_object != 0)
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
	struct Interaction_volume *interaction_volume)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Returns a list of all the graphical entities in the <interaction_volume> of
<scene>. The nearest member of each scene_picked_object will be adjusted as
understood for the type of <interaction_volume> passed.
==============================================================================*/
{
	double modelview_matrix[16],projection_matrix[16];
	GLdouble opengl_modelview_matrix[16],opengl_projection_matrix[16];
	GLuint *select_buffer,*select_buffer_ptr;
	int hit_no,i,j,num_hits,number_of_names,return_code, rendition_no;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene_picked_object *scene_picked_object;
	struct Cmiss_rendition *rendition;
	ENTER(Scene_pick_objects);
	scene_picked_object_list=(struct LIST(Scene_picked_object) *)NULL;
	if (scene&&interaction_volume)
	{
		scene_picked_object_list=CREATE(LIST(Scene_picked_object))();
		if (scene_picked_object_list != 0)
		{
			Render_graphics_opengl *renderer =
				Render_graphics_opengl_create_glbeginend_renderer();
			renderer->picking = 1;
			Cmiss_graphics_filter_id filter = Cmiss_scene_get_filter(scene);
			if (renderer->Scene_compile(scene, filter))
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
						renderer->set_world_view_matrix(opengl_modelview_matrix);

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
							do
							{
								return_code = renderer->Scene_execute(scene);
							}
							while (return_code && renderer->next_layer());
						}
						glFlush();
						num_hits=glRenderMode(GL_RENDER);
						if (0<=num_hits)
						{
							return_code=1;
							for (hit_no=0;(hit_no<num_hits)&&return_code;hit_no++)
							{
								scene_picked_object=CREATE(Scene_picked_object)(hit_no);
								if (scene_picked_object != 0)
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
			if (filter)
				Cmiss_graphics_filter_destroy(&filter);
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
			Scene_changed_private(scene);
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
			Scene_changed_private(scene);
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
			Cmiss_graphics_filter_id filter = Cmiss_scene_get_filter(scene);
			return_code = renderer.Scene_compile(scene, filter);
			if (filter)
				Cmiss_graphics_filter_destroy(&filter);
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
	int return_code = 1;
	static int count = 0;
	count += 1;
	Rendition_set::iterator pos;
	ENTER(Scene_render_opengl);
	if (scene && renderer)
	{
		glPushName(0);
		if (scene->region)
		{
			struct Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(
				scene->region);
			if (rendition)
			{
				renderer->Cmiss_rendition_execute(rendition);
				Cmiss_rendition_destroy(&rendition);
			}
		}
		glPopName();
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_render_opengl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* build_Scene */

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
		graphics_object_range.minimum[0] = 0.0;
		graphics_object_range.minimum[1] = 0.0;
		graphics_object_range.minimum[2] = 0.0;
		graphics_object_range.maximum[0] = 0.0;
		graphics_object_range.maximum[1] = 0.0;
		graphics_object_range.maximum[2] = 0.0;
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
		if (!scene->filter)
		{
			display_message(INFORMATION_MESSAGE,"  no filter\n");
		}
		else
		{
			char *filter_name = scene->filter->getName();
			display_message(INFORMATION_MESSAGE,"  filter: %s\n", filter_name);
			DEALLOCATE(filter_name);
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

struct Scene_graphics_object_iterator_data
{
	const char *graphic_name;
	graphics_object_tree_iterator_function iterator_function;
	void *user_data;
	Cmiss_scene *scene;
};

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
			if (Cmiss_graphics_filter_evaluate_graphic(data->scene->filter, graphic) &&
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

/***************************************************************************//**
 * Calls Scene_graphics_objects_in_Cmiss_graphic_iterator for the rendition
 * of region, then calls this function for each of its child regions.
 */
static int Cmiss_region_recursive_for_each_graphics_object(Cmiss_region *region,
	Scene_graphics_object_iterator_data *data)
{
	int return_code = 0;
	if (region && data)
	{
		// a bit naughty using this internal API, but Scene doesn't yet have
		// pointer to graphics_module...
		Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
		if (rendition)
		{
			return_code = for_each_graphic_in_Cmiss_rendition(rendition,
				Scene_graphics_objects_in_Cmiss_graphic_iterator, (void *)data);
			if (return_code)
			{
				Cmiss_region *child_region = Cmiss_region_get_first_child(region);
				while (child_region)
				{
					if (!Cmiss_region_recursive_for_each_graphics_object(
						child_region, data))
					{
						return_code = 0;
						break;
					}
					Cmiss_region_reaccess_next_sibling(&child_region);
				}
				if (child_region)
				{
					Cmiss_region_destroy(&child_region);
				}
			}
			Cmiss_rendition_destroy(&rendition);
		}
	}
	return (return_code);
}

int for_each_graphics_object_in_scene(struct Scene *scene,
	graphics_object_tree_iterator_function iterator_function,
	void *user_data)
{
	int return_code = 0;

	ENTER(for_each_graphics_object_in_scene);
	if (scene && iterator_function)
	{
		Scene_graphics_object_iterator_data data;
		data.iterator_function = iterator_function;
		data.user_data = user_data;
		data.graphic_name = NULL;
		data.scene = scene;
		return_code =
			Cmiss_region_recursive_for_each_graphics_object(scene->region, &data);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"for_each_graphics_object_in_scene.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_graphics_object_in_scene.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

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
	struct Spectrum *spectrum, GLfloat *minimum, GLfloat *maximum,
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
		if (scene->manager)
		{
			return_code = MANAGER_MODIFY_IDENTIFIER(Scene, name)
				(scene, name, scene->manager);
		}
		else
		{
			if (scene->name)
			{
				DEALLOCATE(scene->name);
			}
			scene->name=duplicate_string(name);
			if (scene->name != 0)
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

Cmiss_scene_id Cmiss_scene_access(Cmiss_scene_id scene)
{
	return ACCESS(Scene)(scene);
}


int Cmiss_scene_destroy(Cmiss_scene **scene_address)
{
	return DEACCESS(Scene)(scene_address);
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
				if (scene->region)
				{
					Cmiss_scene_detach_from_renditions(scene);
				}
				REACCESS(Cmiss_region)(&(scene->region), region);
				DEACCESS(Cmiss_rendition)(&rendition);
				Cmiss_scene_attach_to_renditions(scene);
				Scene_changed_private(scene);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_set_region.  Region does not have a rendition");
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
			scene->change_status = SCENE_CHANGE;
			if (scene->manager)
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
		Scene_changed_private(scene);
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
		Cmiss_rendition_triggers_scene_region_change_callback(rendition, scene);
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

Cmiss_graphics_filter_id Cmiss_scene_get_filter(Cmiss_scene_id scene)
{
	Cmiss_graphics_filter_id filter = NULL;
	if (scene && scene->filter)
	{
		filter = Cmiss_graphics_filter_access(scene->filter);
	}
	return filter;
}

int Cmiss_scene_set_filter(Cmiss_scene_id scene, Cmiss_graphics_filter_id filter)
{
	int return_code = 1;
	if (scene)
	{
		return_code = REACCESS(Cmiss_graphics_filter)(&scene->filter, filter);
		if (return_code && scene->list_of_rendition)
		{
			scene->build = 1;
			scene->change_status = SCENE_CHANGE;
			if (scene->manager)
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

int Cmiss_scene_graphics_filter_change(struct Scene *scene,	void *message_void)
{
	int return_code = 1;
	struct MANAGER_MESSAGE(Cmiss_graphics_filter) *message =
		(struct MANAGER_MESSAGE(Cmiss_graphics_filter) *)message_void;
	if (scene && message)
	{
		int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Cmiss_graphics_filter)(
			message, scene->filter);
		if (change_flags & MANAGER_CHANGE_RESULT(Cmiss_graphics_filter))
		{
			scene->build = 1;
			scene->change_status = SCENE_CHANGE;
			if (scene->manager)
			{
				Scene_refresh(scene);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_graphics_filter_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_scene_cleanup_top_rendition_scene_projection_callback(
	struct Scene *scene, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (scene)
	{
		if (scene->region)
		{
			Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(scene->region);
			if (rendition)
			{
				Cmiss_rendition_triggers_scene_region_change_callback(rendition, scene);
				Cmiss_rendition_destroy(&rendition);
			}
		}
		return 1;
	}
	return 0;
}

int Cmiss_scene_is_managed(Cmiss_scene_id scene)
{
	if (scene)
	{
		return (int)scene->is_managed_flag;
	}
	return 0;
}

int Cmiss_scene_set_managed(Cmiss_scene_id scene, int value)
{
	if (scene)
	{
		int old_value = (int)scene->is_managed_flag;
		scene->is_managed_flag = (value != 0);
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_scene)(scene,
				MANAGER_CHANGE_NOT_RESULT(Cmiss_scene));
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

struct Cmiss_graphics_module *Cmiss_scene_get_graphics_module(Cmiss_scene_id scene)
{
	Cmiss_graphics_module_id graphics_module = 0;
	if (scene && scene->manager)
	{
		graphics_module = MANAGER_GET_OWNER(Scene)(scene->manager);
		if (graphics_module)
			Cmiss_graphics_module_access(graphics_module);
	}
	return graphics_module;
}

Cmiss_scene_picker_id Cmiss_scene_create_picker(Cmiss_scene_id scene)
{
	if (scene)
	{
		Cmiss_graphics_module_id graphics_module = Cmiss_scene_get_graphics_module(scene);
		Cmiss_scene_picker_id scene_picker = Cmiss_scene_picker_create(graphics_module);
		if (graphics_module)
			Cmiss_graphics_module_destroy(&graphics_module);
		Cmiss_scene_picker_set_scene(scene_picker, scene);
		return scene_picker;
	}
	return 0;
}

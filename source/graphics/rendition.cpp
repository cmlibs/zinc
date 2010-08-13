/*******************************************************************************
FILE : rendition.cpp

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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#include <list>
extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "api/cmiss_graphics_module.h"
#include "api/cmiss_rendition.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_wrappers.h"
#include "api/cmiss_field_sub_group_template.h"
#include "region/cmiss_region.h"
#include "finite_element/finite_element_region.h"
#include "graphics/graphic.h"
#include "graphics/graphics_module.h"
#include "graphics/rendition.h"
#include "general/any_object_private.h"
#include "general/any_object_definition.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region_private.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "time/time.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"
}
#include "computed_field/computed_field_private.hpp"
#include "graphics/rendition.hpp"
#include "graphics/rendergl.hpp"
#include "user_interface/process_list_or_write_command.hpp"

FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_rendition_transformation, \
	struct Cmiss_rendition *, gtMatrix *);

struct Cmiss_rendition_callback_data
{
	Cmiss_rendition_callback callback;
	void *callback_user_data;
	Cmiss_rendition_callback_data *next;
}; /* struct Cmiss_rendition_callback_data */

static void Cmiss_rendition_region_change(struct Cmiss_region *region,
	struct Cmiss_region_changes *region_changes, void *rendition_void);

struct Cmiss_rendition
/*******************************************************************************
LAST MODIFIED : 16 October 2008

DESCRIPTION :
Structure for maintaining a graphical rendition of region.
==============================================================================*/
{
	/* the [FE] region being drawn */
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct FE_region *data_fe_region;
	int fe_region_callback_set, data_fe_region_callback_set;
	/* settings shared by whole rendition */
	/* default coordinate field for graphics drawn with settings below */
	struct Computed_field *default_coordinate_field;
	/* list of objects interested in changes to the Cmiss_rendition */
	struct Cmiss_rendition_callback_data *update_callback_list;
	/* managers for updating graphics in response to global changes */
	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(Cmiss_graphic) *list_of_graphics;
	void *computed_field_manager_callback_id;
	/* global stores of selected objects for automatic highlighting */
	void *graphical_material_manager_callback_id;
	void *spectrum_manager_callback_id;
	void *texture_manager_callback_id;
	std::list<struct Scene *> *list_of_scene;
	/* level of cache in effect */
	int cache;
	int fast_changing;
	/* for accessing objects */
	int access_count;
	gtMatrix *transformation;
	bool visibility_flag;
	/* transformaiton field for time dependent transformation */
	struct Computed_field *transformation_field;
	int transformation_time_callback_flag;
	/* curve approximation with line segments over elements */
	struct Element_discretization element_discretization;
	/* number of segments used around cylinders */
	int circle_discretization;
	/* optional native_discretization for graphics drawn with settings below */
	struct FE_field *native_discretization_field;
	struct Cmiss_graphics_module *graphics_module;
	struct Time_object *time_object;
	const char *name_prefix;
	/* callback list for transformation changes */
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_rendition_transformation)) *transformation_callback_list;
	unsigned int position;
	struct Computed_field *selection_group;
}; /* struct Cmiss_rendition */

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Cmiss_rendition_transformation, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Cmiss_rendition_transformation, \
	struct Cmiss_rendition *, gtMatrix *)

int Cmiss_rendition_begin_change(Cmiss_rendition_id rendition)
{
	if (rendition)
	{
		Cmiss_rendition_begin_cache(rendition);
		return 1;
	}
	else
	{
		return 0;
	}
}


/***************************************************************************//**
 * If changes have been made to <rendition>, as indicated by its build flag
 * being set, sends a callback to all registered clients.
 */
static int Cmiss_rendition_inform_clients(
	struct Cmiss_rendition *rendition)
{
	int return_code;
	struct Cmiss_rendition_callback_data *callback_data;

	ENTER(Cmiss_rendition_inform_clients);
	if (rendition)
	{
		callback_data = rendition->update_callback_list;
		while(callback_data)
		{
			(callback_data->callback)(rendition,
					callback_data->callback_user_data);
				callback_data = callback_data->next;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_inform_clients.  Invalid Cmiss_rendition");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_inform_clients */

static int Cmiss_rendition_changed_external(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_changed_external);
	if (rendition)
	{
		if (rendition->list_of_scene &&
			!rendition->list_of_scene->empty())
		{
			std::list<struct Scene *>::iterator pos = 
				rendition->list_of_scene->begin();
			while (pos != rendition->list_of_scene->end())
			{
				Scene_rendition_changed(*pos, rendition);
				++pos;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_changed_external.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_changed_external */

/***************************************************************************//**
 * External modules that change a Cmiss_rendition should call this routine so that
 * objects interested in this Cmiss_rendition will be notified that is has
 * changed.
 */
static int Cmiss_rendition_changed(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_changed);
	if (rendition)
	{
		/* inform clients only if caching is off */
		if (0 == rendition->cache)
		{
			Cmiss_rendition_inform_clients(rendition);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_changed. Invalid Cmiss_rendition");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_changed */

int Cmiss_rendition_notify_parent_rendition_callback(struct Cmiss_rendition *child_rendition,
	void *region_void)
{
	int return_code;
	struct Cmiss_rendition *parent_rendition;
	
	ENTER(Cmiss_rendition_notify_parent_rendition_callback);
	if (child_rendition &&
		(parent_rendition = Cmiss_region_get_rendition_internal((struct Cmiss_region *)region_void)))
	{
		Cmiss_rendition_changed(parent_rendition);
		DEACCESS(Cmiss_rendition)(&parent_rendition);
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int Cmiss_rendition_void_detach_from_Cmiss_region(void *cmiss_rendition_void)
{
	int return_code;
	struct Cmiss_rendition *rendition;

	ENTER(Cmiss_rendition_void_detach_from_Cmiss_region);
	if ((rendition = (struct Cmiss_rendition *)cmiss_rendition_void))
	{
		rendition->region = (struct Cmiss_region *)NULL;
		return_code = DEACCESS(Cmiss_rendition)(&rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendtion_void_detach_from_Cmiss_region.  Missing void Cmiss_rendition");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * If the <rendition> doesn't have a default coordinate yet then it 
 * tries to find one, first in the element_group, then in the node_group and
 * finally in the data group.
 */
static int Cmiss_rendition_update_default_coordinate(
	struct Cmiss_rendition *rendition)
{
	int return_code;
	struct Computed_field *computed_field;
	struct FE_field *fe_field;

	ENTER(Cmiss_rendition_changed);
	if (rendition)
	{
		return_code = 1;
		/* if we don't have a computed_field_manager, we are working on an
			 "editor copy" which does not update graphics; the
			 default_coordinate_field will have been supplied by the global object */
		if (rendition->computed_field_manager)
		{
			if (rendition->default_coordinate_field)
			{
				/* Don't second guess, just keep what we have */
			}
			else
			{
				/* Try to find one */
				if (FE_region_get_default_coordinate_FE_field(
					rendition->fe_region, &fe_field) ||
					(rendition->data_fe_region && 
					FE_region_get_default_coordinate_FE_field(
						rendition->data_fe_region, &fe_field)))
				{
					/* Find the computed_field wrapper */
					if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_read_only_with_fe_field,
						(void *)fe_field, rendition->computed_field_manager)))
					{
						rendition->default_coordinate_field = 
							ACCESS(Computed_field)(computed_field);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_changed. Invalid Cmiss_rendition");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_changed */

static void Cmiss_rendition_element_points_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *rendition_void)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Callback for change in the global element selection.
==============================================================================*/
{
	struct Cmiss_rendition *rendition;

	ENTER(Cmiss_rendition_element_point_ranges_selection_change);
	if (element_point_ranges_selection && changes &&
		(rendition = (struct Cmiss_rendition *)rendition_void))
	{
		/* find out if any of the changes affect elements in this group */
		if (FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
			Element_point_ranges_element_is_in_FE_region,
			(void *)rendition->fe_region,
			changes->newly_selected_element_point_ranges_list) ||
			FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
				Element_point_ranges_element_is_in_FE_region,
				(void *)rendition->fe_region,
				changes->newly_unselected_element_point_ranges_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
					Cmiss_graphic_selected_element_points_change, (void *)NULL,
				rendition->list_of_graphics);
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_element_point_ranges_selection_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_element_point_ranges_selection_change */

/***************************************************************************//**
 * Allocates memory and assigns fields for a cmiss rendition for the given 
 * cmiss_region. Access count is set to 1.
 */
struct Cmiss_rendition *CREATE(Cmiss_rendition)(struct Cmiss_region *cmiss_region,
	struct Cmiss_graphics_module *graphics_module)
{
	struct FE_region *fe_region, *data_fe_region;
	struct Cmiss_rendition *cmiss_rendition;

	ENTER(CREATE(Cmiss_rendition));
	data_fe_region = NULL;
	if (cmiss_region && (fe_region = Cmiss_region_get_FE_region(cmiss_region)))
	{
		data_fe_region = FE_region_get_data_FE_region(fe_region);
		if (ALLOCATE(cmiss_rendition, struct Cmiss_rendition, 1))
		{
			cmiss_rendition->list_of_graphics = NULL;
			if (NULL != (cmiss_rendition->list_of_graphics =
					CREATE(LIST(Cmiss_graphic))()))
			{
				cmiss_rendition->region = cmiss_region;
 				cmiss_rendition->fe_region = ACCESS(FE_region)(fe_region);
				cmiss_rendition->data_fe_region = ACCESS(FE_region)(data_fe_region);
				cmiss_rendition->fe_region_callback_set = 0;
				cmiss_rendition->data_fe_region_callback_set = 0;
				/* set settings shared by whole rendition */
				cmiss_rendition->element_discretization.number_in_xi1=4;
				cmiss_rendition->element_discretization.number_in_xi2=4;
				cmiss_rendition->element_discretization.number_in_xi3=4;
				cmiss_rendition->circle_discretization=6;
				cmiss_rendition->native_discretization_field=(struct FE_field *)NULL;
				cmiss_rendition->visibility_flag = true;
				cmiss_rendition->default_coordinate_field=
					(struct Computed_field *)NULL;
				cmiss_rendition->update_callback_list=
					(struct Cmiss_rendition_callback_data *)NULL;
 				/* managers and callback ids */
				cmiss_rendition->computed_field_manager=Cmiss_region_get_Computed_field_manager(
					 cmiss_region);
				cmiss_rendition->computed_field_manager_callback_id=(void *)NULL;
				cmiss_rendition->graphical_material_manager_callback_id=(void *)NULL;
				cmiss_rendition->spectrum_manager_callback_id=(void *)NULL;
				cmiss_rendition->texture_manager_callback_id=(void *)NULL;
				cmiss_rendition->transformation = (gtMatrix *)NULL;
				cmiss_rendition->graphics_module =	graphics_module;
				cmiss_rendition->time_object = NULL;
				cmiss_rendition->list_of_scene = NULL;
				cmiss_rendition->fast_changing = 0;
				cmiss_rendition->cache = 0;
				cmiss_rendition->position = 0;
				cmiss_rendition->transformation_callback_list =
					CREATE(LIST(CMISS_CALLBACK_ITEM(Cmiss_rendition_transformation)))();
				cmiss_rendition->transformation_field = NULL;
				cmiss_rendition->transformation_time_callback_flag = 0;
				cmiss_rendition->name_prefix = Cmiss_region_get_path(cmiss_region);
				cmiss_rendition->selection_group = NULL;
 				Cmiss_rendition_update_default_coordinate(cmiss_rendition);
 				if (graphics_module)
 				{
 					Element_point_ranges_selection_add_callback(
 						Cmiss_graphics_module_get_element_point_ranges_selection(graphics_module),
 						Cmiss_rendition_element_points_ranges_selection_change,
 						(void *)cmiss_rendition);
 				}
			}
			else
			{
				DESTROY(LIST(Cmiss_graphic))(
					&(cmiss_rendition->list_of_graphics));
				DEALLOCATE(cmiss_rendition);
				cmiss_rendition = (struct Cmiss_rendition *)NULL;
			}
			cmiss_rendition->access_count = 1;
		}
		else
		{
			DEALLOCATE(cmiss_rendition);
			cmiss_rendition = (struct Cmiss_rendition *)NULL;
		}
		if (!cmiss_rendition)
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cmiss_rendition).  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_rendition).  Invalid argument(s)");
		cmiss_rendition=(struct Cmiss_rendition *)NULL;
	}
	LEAVE;

	return (cmiss_rendition);
} /* CREATE(Cmiss_rendition) */

int Cmiss_rendition_get_circle_discretization(
	struct Cmiss_rendition *rendition)
{
	int circle_discretization;

	ENTER(Cmiss_rendition_get_circle_discretization);
	if (rendition)
	{
		circle_discretization=rendition->circle_discretization;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_circle_discretization.  Invalid argument(s)");
		circle_discretization=0;
	}
	LEAVE;

	return (circle_discretization);
} /* Cmiss_rendition_get_circle_discretization */

int Cmiss_rendition_set_circle_discretization_private(
	struct Cmiss_rendition *rendition, int circle_discretization)
{
	int return_code;

	ENTER(Cmiss_rendition_set_circle_discretization);
	if (rendition)
	{
		return_code = 1;
		if (circle_discretization != rendition->circle_discretization)
		{
			rendition->circle_discretization = circle_discretization;
			/* clear graphics for all graphic using circle discretization */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_circle_discretization_change,(void *)NULL,
				rendition->list_of_graphics);
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_circle_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_circle_discretization */

int Cmiss_rendition_set_circle_discretization(
	struct Cmiss_rendition *rendition,int circle_discretization)
{
	int return_code, redraw;

	ENTER(Cmiss_rendition_set_circle_discretization);

	if (rendition)
	{
		return_code = 1;
		if (FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_set_general_circle_discretization,
			(void *)&circle_discretization,
			rendition->list_of_graphics))
		{
			redraw = 1;
		}
		else
		{
			redraw = 0;
		}
		if (circle_discretization != rendition->circle_discretization)
		{
			rendition->circle_discretization = circle_discretization;
			/* clear graphics for all settings using circle discretization */
			redraw = 1;
		}
		if (redraw)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_circle_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_default_coordinate_field */

struct Computed_field *Cmiss_rendition_get_default_coordinate_field(
	struct Cmiss_rendition *rendition)
{
	struct Computed_field *default_coordinate_field;

	ENTER(Cmiss_rendition_get_default_coordinate_field);
	if (rendition)
	{
		if (!rendition->default_coordinate_field)
		{
			/* Try and get one now before returning nothing */
			Cmiss_rendition_update_default_coordinate(rendition);
		}
		default_coordinate_field=rendition->default_coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_default_coordinate_field.  Invalid argument(s)");
		default_coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (default_coordinate_field);
} /* Cmiss_rendition_get_default_coordinate_field */

int Cmiss_rendition_set_default_coordinate_field_private(
	struct Cmiss_rendition *rendition,
	struct Computed_field *default_coordinate_field)
{
	int return_code;

	ENTER(Cmiss_rendition_set_default_coordinate_field);
	if (rendition && default_coordinate_field &&
		(3 >= Computed_field_get_number_of_components(default_coordinate_field)))
	{
		return_code = 1;
		if (default_coordinate_field != rendition->default_coordinate_field)
		{
			REACCESS(Computed_field)(&(rendition->default_coordinate_field),
				default_coordinate_field);
			/* clear graphics for all graphic using default coordinate field */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_default_coordinate_field_change,(void *)NULL,
				rendition->list_of_graphics);
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_default_coordinate_field_private.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_default_coordinate_field */


int Cmiss_rendition_set_default_coordinate_field(
	struct Cmiss_rendition *rendition,
	struct Computed_field *default_coordinate_field)
{
  int return_code, redraw;

	ENTER(Cmiss_rendition_set_default_coordinate_field);
	if (rendition && default_coordinate_field &&
		(3 >= Computed_field_get_number_of_components(default_coordinate_field)))
	{
		return_code = 1;
		if (FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_set_general_coordinate_field,
				(void *)default_coordinate_field,
				rendition->list_of_graphics))
		{
			redraw = 1;
		}
		else
		{
			redraw = 0;
		}
		if (default_coordinate_field != rendition->default_coordinate_field)
		{
			REACCESS(Computed_field)(&(rendition->default_coordinate_field),
				default_coordinate_field);
			redraw = 1;
		}
		/* clear graphics for all graphic using default coordinate field */
		if (redraw)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_default_coordinate_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_default_coordinate_field */

int Cmiss_rendition_get_element_discretization(
	struct Cmiss_rendition *rendition,
	struct Element_discretization *element_discretization)
{
	int return_code;

	ENTER(Cmiss_rendition_get_element_discretization);
	if (rendition&&element_discretization)
	{
		element_discretization->number_in_xi1=
			rendition->element_discretization.number_in_xi1;
		element_discretization->number_in_xi2=
			rendition->element_discretization.number_in_xi2;
		element_discretization->number_in_xi3=
			rendition->element_discretization.number_in_xi3;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_element_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_get_element_discretization */

int Cmiss_rendition_set_element_discretization_private(
	struct Cmiss_rendition *rendition,
	struct Element_discretization *element_discretization)
{
  int return_code;

	ENTER(Cmiss_rendition_set_element_discretization);
	if (rendition && element_discretization)
	{
		return_code = 1;
		if ((rendition->element_discretization.number_in_xi1 !=
			element_discretization->number_in_xi1) ||
			(rendition->element_discretization.number_in_xi2 !=
				element_discretization->number_in_xi2) ||
			(rendition->element_discretization.number_in_xi3 !=
				element_discretization->number_in_xi3))
		{
			rendition->element_discretization.number_in_xi1=
				element_discretization->number_in_xi1;
			rendition->element_discretization.number_in_xi2=
				element_discretization->number_in_xi2;
			rendition->element_discretization.number_in_xi3=
				element_discretization->number_in_xi3;
		}
		/* clear graphics for all settings using element discretization */
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_element_discretization_change,(void *)NULL,
			rendition->list_of_graphics);
		Cmiss_rendition_changed(rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_element_discretization_private.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_element_discretization */

int Cmiss_rendition_set_element_discretization(
	struct Cmiss_rendition *rendition,
	struct Element_discretization *element_discretization)
{
  int redraw, return_code;

	ENTER(Cmiss_rendition_set_element_discretization);
	if (rendition && element_discretization)
	{
		return_code = 1;
		if (FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
		    Cmiss_graphic_set_general_element_discretization,
		    (void *)element_discretization,
		    rendition->list_of_graphics))
		{
			redraw = 1;
		}
		else
		{
			redraw = 0;
		}
		if ((rendition->element_discretization.number_in_xi1 !=
			element_discretization->number_in_xi1) ||
			(rendition->element_discretization.number_in_xi2 !=
				element_discretization->number_in_xi2) ||
			(rendition->element_discretization.number_in_xi3 !=
				element_discretization->number_in_xi3))
		{
			rendition->element_discretization.number_in_xi1=
				element_discretization->number_in_xi1;
			rendition->element_discretization.number_in_xi2=
				element_discretization->number_in_xi2;
			rendition->element_discretization.number_in_xi3=
				element_discretization->number_in_xi3;
			redraw = 1;
		}
		/* clear graphics for all settings using element discretization */
		if (redraw)
		{
		  Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_element_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_element_discretization */

struct FE_field *Cmiss_rendition_get_native_discretization_field(
	struct Cmiss_rendition *rendition)
{
	struct FE_field *native_discretization_field;

	ENTER(Cmiss_rendition_get_native_discretization_field);
	if (rendition)
	{
		native_discretization_field=rendition->native_discretization_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_native_discretization_field.  Invalid argument(s)");
		native_discretization_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (native_discretization_field);
} /* Cmiss_rendition_get_native_discretization_field */

int Cmiss_rendition_set_native_discretization_field_private(
	struct Cmiss_rendition *rendition,
	struct FE_field *native_discretization_field)
{
	int return_code;

	ENTER(Cmiss_rendition_set_native_discretization_field);
	if (rendition)
	{
		return_code = 1;
		if (native_discretization_field !=
			rendition->native_discretization_field)
		{
			REACCESS(FE_field)(&(rendition->native_discretization_field),
				native_discretization_field);
			/* clear graphics for all graphic using element discretization */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_element_discretization_change, (void *)NULL,
				rendition->list_of_graphics);
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_native_discretization_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_native_discretization_field */

int Cmiss_rendition_set_native_discretization_field(
	struct Cmiss_rendition *rendition,
	struct FE_field *native_discretization_field)
{
  int return_code, redraw;

	ENTER(Cmiss_rendition_set_native_discretization_field);
	if (rendition)
	{
		return_code = 1;
		if (FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_set_general_native_discretization_field, 
				(void *)native_discretization_field,
				rendition->list_of_graphics))
		{
			redraw = 1;
		}
		else
		{
			redraw = 0;
		}
		if (native_discretization_field !=
			rendition->native_discretization_field)
		{
			REACCESS(FE_field)(&(rendition->native_discretization_field),
				native_discretization_field);
			redraw = 1;
		}
			/* clear graphics for all graphic using element discretization */
		if (redraw)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_native_discretization_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_native_discretization_field */

static void Cmiss_rendition_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *rendition_void)
{
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic_FE_region_change_data data;

	ENTER(Cmiss_rendition_FE_region_change);
	if (fe_region && changes &&
		(rendition = (struct Cmiss_rendition *)rendition_void))
	{
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&data.fe_field_change_summary);
		data.fe_field_changes = changes->fe_field_changes;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(changes->fe_node_changes,
			&data.fe_node_change_summary);
		CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_node)(changes->fe_node_changes,
			&data.number_of_fe_node_changes);
		data.fe_node_changes = changes->fe_node_changes;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(changes->fe_element_changes,
			&data.fe_element_change_summary);
		CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_element)(changes->fe_element_changes,
			&data.number_of_fe_element_changes);
		data.fe_element_changes = changes->fe_element_changes;
		/*???RC Is there a better way of getting time to here? */
		data.time = 0;
		data.graphics_changed = 0;
		data.fe_region = fe_region;
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_FE_region_change, (void *)&data,
			rendition->list_of_graphics);
		if (data.graphics_changed)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_FE_region_change */

static void Cmiss_rendition_data_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *rendition_void)
{
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic_FE_region_change_data data;

	ENTER(Cmiss_rendition_data_FE_region_change);
	if (fe_region && changes &&
		(rendition = (struct Cmiss_rendition *)rendition_void))
	{
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&data.fe_field_change_summary);
		data.fe_field_changes = changes->fe_field_changes;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(changes->fe_node_changes,
			&data.fe_node_change_summary);
		CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_node)(changes->fe_node_changes,
			&data.number_of_fe_node_changes);
		data.fe_node_changes = changes->fe_node_changes;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(changes->fe_element_changes,
			&data.fe_element_change_summary);
		CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_element)(changes->fe_element_changes,
			&data.number_of_fe_element_changes);
		data.fe_element_changes = changes->fe_element_changes;
		/*???RC Is there a better way of getting time to here? */
		if (rendition->time_object)
		{
			data.time = Time_object_get_current_time(rendition->time_object);
		}
		else
		{
			data.time = 0;
		}
		
		data.graphics_changed = 0;
		data.fe_region = fe_region;
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_data_FE_region_change, (void *)&data,
			rendition->list_of_graphics);
		if (data.graphics_changed)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_data_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_data_FE_region_change */

static void Cmiss_rendition_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *rendition_void)
{
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic_Computed_field_change_data change_data;

	ENTER(Cmiss_rendition_Computed_field_change);
	if (message &&
		(rendition = (struct Cmiss_rendition *)rendition_void))
	{
		change_data.changed_field_list =
			MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(message,
				MANAGER_CHANGE_RESULT(Computed_field));
		if (change_data.changed_field_list)
		{
			/*???RC Is there a better way of getting time to here? */
			change_data.time = 0;
			change_data.graphics_changed = 0;
			change_data.default_coordinate_field =
				rendition->default_coordinate_field;
			if (Cmiss_rendition_has_selection_group(rendition))
			{
				change_data.group_field = rendition->selection_group;
			}
			else
			{
				change_data.group_field = NULL;
			}
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_Computed_field_change, (void *)&change_data,
				rendition->list_of_graphics);
			if (change_data.graphics_changed)
			{
				Cmiss_rendition_changed(rendition);
			}
			DESTROY(LIST(Computed_field))(&change_data.changed_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_Computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_Computed_field_change */

int Cmiss_region_attach_rendition(struct Cmiss_region *region, 
	struct Cmiss_rendition *rendition)
{
	int return_code;
	struct Any_object *any_object;

	ENTER(Cmiss_region_attach_rendition);	

	if (NULL != (any_object = CREATE(ANY_OBJECT(Cmiss_rendition))(rendition)) &&
		Cmiss_region_private_attach_any_object(region, any_object))
	{
		Cmiss_region_add_callback(rendition->region,
			Cmiss_rendition_region_change, (void *)rendition);
		rendition->fe_region_callback_set =
			FE_region_add_callback(rendition->fe_region,
				Cmiss_rendition_FE_region_change, (void *)rendition);
		if (rendition->data_fe_region)
		{
			rendition->data_fe_region_callback_set =
				FE_region_add_callback(rendition->data_fe_region,
					Cmiss_rendition_data_FE_region_change, (void *)rendition);
		}
		/* request callbacks from any managers supplied */
		if (rendition->computed_field_manager)
		{
			rendition->computed_field_manager_callback_id=
				MANAGER_REGISTER(Computed_field)(
					Cmiss_rendition_Computed_field_change,(void *)rendition,
					rendition->computed_field_manager);
		}
		
// 		ACCESS(Cmiss_rendition)(rendition);
		Any_object_set_cleanup_function(any_object,
			Cmiss_rendition_void_detach_from_Cmiss_region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_attach_rendition. Could not attach object.");	
		DESTROY(Any_object)(&any_object);
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct Cmiss_rendition *Cmiss_rendition_create_internal(struct Cmiss_region *cmiss_region,
	struct Cmiss_graphics_module *graphics_module)
{
	struct Cmiss_rendition *rendition;

	ENTER(Cmiss_rendition_create);
	if (cmiss_region && graphics_module)
	{
		rendition = CREATE(Cmiss_rendition)(cmiss_region, graphics_module);
		{
			if (!(rendition && Cmiss_region_attach_rendition(cmiss_region, 
						rendition)))
			{
				DEACCESS(Cmiss_rendition)(&rendition);
			}
			else
			{
				/* add the default graphic here */
				struct Cmiss_graphic *graphic;
				if (rendition->default_coordinate_field && 
					(graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_LINES)))
				{
					struct Material_package *material_package =
						Cmiss_graphics_module_get_material_package(graphics_module);
					Cmiss_graphic_set_material(graphic, 
						Material_package_get_default_material(material_package));

					struct Graphics_font *default_font = Cmiss_graphics_module_get_default_font(
						graphics_module);
					Cmiss_graphic_set_label_field(graphic,(struct Computed_field *)NULL, default_font);
					DEACCESS(Graphics_font)(&default_font);
					Cmiss_graphic_set_selected_material(graphic,
						FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
							"default_selected", Material_package_get_material_manager(material_package)));
					Cmiss_graphic_set_general_coordinate_field(
						graphic,(void *)rendition->default_coordinate_field);
					if (!Cmiss_rendition_add_graphic(rendition, graphic, 0))
					{
						display_message(ERROR_MESSAGE, "Cmiss_rendition_create. "
							"Could not add default line graphic");
						DESTROY(Cmiss_graphic)(&graphic);
					}
					DEACCESS(Material_package)(&material_package);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_rendition).  Invalid argument(s)");
		rendition = NULL;
	}
	LEAVE;

	return (rendition);
}

void Cmiss_rendition_add_child_region(struct Cmiss_rendition *rendition, 
	struct Cmiss_region *child_region)
{
	struct Cmiss_rendition *child_rendition;
	if (rendition && child_region &&
		(NULL != (child_rendition = Cmiss_rendition_create_internal(
			child_region, rendition->graphics_module))))
	{
		int child_region_number = 1;

		unsigned int parent_position = rendition->position;
		struct Cmiss_region *temp_region = Cmiss_region_get_first_child(rendition->region);
		while (temp_region)
		{
			Cmiss_region_reaccess_next_sibling(&temp_region);
			child_region_number = child_region_number +1;
		}

		char position_string[100];
 		sprintf(position_string,"%u0%u", parent_position, child_region_number);
		Cmiss_rendition_set_position(child_rendition,
 			atoi((const char *)position_string));
		Cmiss_rendition_add_callback(child_rendition, Cmiss_rendition_update_callback,
			(void *)NULL);
		if (rendition->list_of_scene &&
			!rendition->list_of_scene->empty())
		{
			std::list<struct Scene *>::iterator pos = 
				rendition->list_of_scene->begin();
			while (pos != rendition->list_of_scene->end())
			{
				Cmiss_rendition_add_scene(child_rendition, *pos, 0);
				++pos;
			}
		}
		Cmiss_rendition_add_callback(child_rendition,
			Cmiss_rendition_notify_parent_rendition_callback,
			(void *)rendition->region);
		temp_region = Cmiss_region_get_first_child(
			child_region);
		while (temp_region)
		{
			if (!Cmiss_region_has_rendition(temp_region))
			{
				Cmiss_rendition_add_child_region(child_rendition,
					temp_region);
			}
			Cmiss_region_reaccess_next_sibling(&temp_region);
		}
	}
}

int Cmiss_rendition_update_child_rendition(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_update_child_rendition);
	if (rendition)
	{
		Cmiss_rendition_begin_cache(rendition);
		/* ensure we have a graphical element for each child region */
		struct Cmiss_region *child_region = Cmiss_region_get_first_child(rendition->region);
		while (child_region)
		{
			if (!Cmiss_region_has_rendition(child_region))
			{
				Cmiss_rendition_add_child_region(rendition, 
					child_region);
			}
			Cmiss_region_reaccess_next_sibling(&child_region);
		}
		Cmiss_rendition_end_cache(rendition);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_update_child_rendition.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static void Cmiss_rendition_region_change(struct Cmiss_region *region,
	struct Cmiss_region_changes *region_changes, void *rendition_void)
{
	struct Cmiss_region *child_region;
	struct Cmiss_rendition *rendition;

	ENTER(Scene_Cmiss_region_change);

	if (region && region_changes && (rendition = (struct Cmiss_rendition *)rendition_void))
	{
		if (region_changes->children_changed)
		{
 			Cmiss_rendition_begin_cache(rendition);

			if (region_changes->child_added)
			{
				Cmiss_rendition_begin_cache(rendition);
				child_region = region_changes->child_added;
				Cmiss_rendition_add_child_region(rendition, 
					child_region);
				Cmiss_rendition_end_cache(rendition);
			}
			else if (region_changes->child_removed)
			{
			}
			else
			{
					Cmiss_rendition_update_child_rendition(rendition);	
			}
			Cmiss_rendition_end_cache(rendition);
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_Cmiss_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_region_change */

DECLARE_OBJECT_FUNCTIONS(Cmiss_rendition);
DEFINE_ANY_OBJECT(Cmiss_rendition);

int Cmiss_rendition_begin_cache(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_begin_cache);
	if (rendition)
	{
		/* increment cache to allow nesting */
		(rendition->cache)++;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_begin_cache.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_begin_cache */

int Cmiss_rendition_end_cache(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_end_cache);
	if (rendition)
	{
		/* decrement cache to allow nesting */
		(rendition->cache)--;
		/* once cache has run out, inform clients of any changes */
		if (0 == rendition->cache)
		{
			Cmiss_rendition_inform_clients(rendition);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_end_cache.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_end_cache */


int Cmiss_rendition_add_graphic(struct Cmiss_rendition *rendition,
	struct Cmiss_graphic *graphic,int position)
{
	int return_code;

	ENTER(Cmiss_rendition_add_graphic);
	if (rendition && graphic && (NULL == Cmiss_graphic_get_rendition_private(graphic)))
	{
		return_code=Cmiss_graphic_add_to_list(graphic,position,
			rendition->list_of_graphics);
		Cmiss_graphic_set_rendition_private(graphic, rendition);
		Cmiss_rendition_changed(rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_add_graphic.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_add_graphics */

int Cmiss_rendition_remove_graphic(struct Cmiss_rendition *rendition,
	struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_rendition_remove_graphic);
	if (rendition && graphic && (rendition == Cmiss_graphic_get_rendition_private(graphic)))
	{
		Cmiss_graphic_set_rendition_private(graphic, NULL);
		return_code=Cmiss_graphic_remove_from_list(graphic,
			rendition->list_of_graphics);
		Cmiss_rendition_changed(rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_remove_graphic.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_remove_graphic */

/***************************************************************************//**
 * Changes the contents of <graphic> to match <new_graphic>, with no change in
 * position in <rendition>.
 */
int Cmiss_rendition_modify_graphic(struct Cmiss_rendition *rendition,
	struct Cmiss_graphic *graphic,struct Cmiss_graphic *new_graphic)
{
	int return_code;

	ENTER(Cmiss_rendition_modify_graphic);
	if (rendition&&graphic&&new_graphic)
	{
		return_code=Cmiss_graphic_modify_in_list(graphic,new_graphic,
			rendition->list_of_graphics);
		Cmiss_rendition_changed(rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_modify_graphic.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_modify_graphic */

static int Cmiss_rendition_build_graphics_objects(
	struct Cmiss_rendition *rendition, struct Cmiss_scene *scene,
	FE_value time, const char *name_prefix)
{
	int return_code;
	struct Element_discretization element_discretization;
	struct Cmiss_graphic_to_graphics_object_data graphic_to_object_data;

	ENTER(Cmiss_rendition_build_graphics_object);
	if (rendition)
	{
		// use begin/end cache to avoid field manager messages being sent when
		// field wrappers are created and destroyed
		MANAGER_BEGIN_CACHE(Computed_field)(rendition->computed_field_manager);
		/* update default coordinate field */
		Cmiss_rendition_update_default_coordinate(rendition);
		
		graphic_to_object_data.default_rc_coordinate_field = 
			(struct Computed_field *)NULL;
		if ((!rendition->default_coordinate_field) || 
			(graphic_to_object_data.default_rc_coordinate_field=
			Computed_field_begin_wrap_coordinate_field(
			rendition->default_coordinate_field)))
		{
			graphic_to_object_data.name_prefix = name_prefix;
			graphic_to_object_data.rc_coordinate_field =
				(struct Computed_field *)NULL;
			graphic_to_object_data.wrapper_orientation_scale_field =
				(struct Computed_field *)NULL;
			graphic_to_object_data.native_discretization_field =
				rendition->native_discretization_field;
			graphic_to_object_data.region = rendition->region;
			graphic_to_object_data.fe_region = rendition->fe_region;
			graphic_to_object_data.data_fe_region = rendition->data_fe_region;
			graphic_to_object_data.scene = scene;
			element_discretization.number_in_xi1 =
				rendition->element_discretization.number_in_xi1;
			element_discretization.number_in_xi2 =
				rendition->element_discretization.number_in_xi2;
			element_discretization.number_in_xi3 =
				rendition->element_discretization.number_in_xi3;
			graphic_to_object_data.element_discretization = &element_discretization;
			graphic_to_object_data.circle_discretization =
				rendition->circle_discretization;
			graphic_to_object_data.time = time;
			graphic_to_object_data.selected_element_point_ranges_list =
				Element_point_ranges_selection_get_element_point_ranges_list(
					Cmiss_graphics_module_get_element_point_ranges_selection(rendition->graphics_module));
			graphic_to_object_data.selected_element_list = FE_element_selection_get_element_list(
				Cmiss_graphics_module_get_element_selection(rendition->graphics_module));
			if (Cmiss_rendition_has_selection_group(rendition))
			{
				graphic_to_object_data.group_field = rendition->selection_group;
			}
			else
			{
				graphic_to_object_data.group_field = NULL;
			}
			graphic_to_object_data.iso_surface_specification = NULL;
			return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_to_graphics_object,(void *)&graphic_to_object_data,
				rendition->list_of_graphics);

			if (graphic_to_object_data.default_rc_coordinate_field)
			{
				Computed_field_end_wrap(
					&graphic_to_object_data.default_rc_coordinate_field);
			}
			MANAGER_END_CACHE(Computed_field)(rendition->computed_field_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_rendition_build_graphics_objects.  "
				"Could not get default_rc_coordinate_field wrapper");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmiss_rendition_build_graphics_objects.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_rendition_get_position(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_set_position);
	if (rendition)
	{
		return_code = rendition->position;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_position */



int Cmiss_rendition_set_position(struct Cmiss_rendition *rendition, unsigned int position)
{
	int return_code;

	ENTER(Cmiss_rendition_set_position);
	if (rendition&&(0<position))
	{
		rendition->position=position;
		return_code=1;

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_position */

int Cmiss_rendition_get_range(struct Cmiss_rendition *rendition,
	struct Cmiss_scene *scene, struct Graphics_object_range_struct *graphics_object_range)
{
	float coordinates[4],transformed_coordinates[4];
	gtMatrix *transformation;
	int i,j,k,return_code;
	struct Graphics_object_range_struct temp_graphics_object_range;
	void *use_range_void;

	ENTER(Cmiss_rendition_get_range);
	if (rendition && graphics_object_range)
	{
		/* must first build graphics objects */
		Render_graphics_build_objects renderer;
		Render_graphics_push_scene push_scene(renderer, scene);
		renderer.Cmiss_rendition_compile(rendition);
		if (NULL != (transformation = rendition->transformation))
		{
			temp_graphics_object_range.first = 1;
			temp_graphics_object_range.scene = scene;
			use_range_void = (void *)&temp_graphics_object_range;
		}
		else
		{
			graphics_object_range->scene = scene;
			use_range_void = (void *)graphics_object_range;
		}
		return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_get_visible_graphics_object_range, use_range_void,
			rendition->list_of_graphics);
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
} /* Cmiss_rendition_get_range */

struct Cmiss_rendition *Cmiss_region_get_rendition_internal(struct Cmiss_region *cmiss_region)
{
	struct Cmiss_rendition *rendition = NULL;

	ENTER(Cmiss_region_get_rendition_internal);
	if (cmiss_region)
	{
		if (!(rendition = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(Cmiss_rendition))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(Cmiss_rendition) *)NULL, (void *)NULL,
			Cmiss_region_private_get_any_object_list(cmiss_region))))
		{
#if defined (DEBUG)
			display_message(ERROR_MESSAGE,
				"Cmiss_region_get_rendition. Region does not have rendition.");
			/* It probably should create a rendition here then return it*/
#endif
		}
		else
		{
			ACCESS(Cmiss_rendition)(rendition);
		}
	}
	LEAVE;

	return (rendition);
}

int Cmiss_region_has_rendition(struct Cmiss_region *cmiss_region)
{
	int return_code = 0;

	ENTER(Cmiss_region_get_rendition_internal);
	if (cmiss_region)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(Cmiss_rendition))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(Cmiss_rendition) *)NULL, (void *)NULL,
			Cmiss_region_private_get_any_object_list(cmiss_region)))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	LEAVE;

	return (return_code);
}

int Cmiss_region_deaccess_rendition(struct Cmiss_region *region)
{
	int return_code;
	struct Cmiss_rendition *rendition;

	if (region && (rendition = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(Cmiss_rendition))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(Cmiss_rendition) *)NULL, (void *)NULL,
			Cmiss_region_private_get_any_object_list(region))))
	{
		DEACCESS(Cmiss_rendition)(&(rendition));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_deaccess_rendition. Rendition does not exist");	
		return_code = 0;
	}
	return (return_code);
}

int Cmiss_rendition_destroy(struct Cmiss_rendition **rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_destroy);
	if (rendition && *rendition)
	{
		return_code = DEACCESS(Cmiss_rendition)(rendition);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return return_code;
}

int Cmiss_rendition_Spectrum_change(struct Cmiss_rendition *rendition,
	void *changed_spectrum_list_void)
{
	int return_code = 0;
	struct Cmiss_graphic_Spectrum_change_data spectrum_change_data;
	struct LIST(Spectrum) *changed_spectrum_list = (struct LIST(Spectrum) *)changed_spectrum_list_void;

	ENTER(Cmiss_rendition_Spectrum_change);
	if (rendition && changed_spectrum_list)
	{
		spectrum_change_data.changed_spectrum_list = changed_spectrum_list;
		/* flag to indicate if GT_element_group_changed needs to be called */
		spectrum_change_data.changed = 0;
		return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_Spectrum_change, (void *)&spectrum_change_data,
			rendition->list_of_graphics);
		if (spectrum_change_data.changed)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_Spectrum_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_Spectrum_change */


/***************************************************************************//**
 * Something has changed globally in the material manager.
 * Tell the rendition it has changed and it will rebuild affected materials too.
 */
static void Cmiss_rendition_Spectrum_callback(
	struct MANAGER_MESSAGE(Spectrum) *message, void *rendition_void)
{
	struct Cmiss_rendition *rendition;

	ENTER(Cmiss_rendition_Spectrum_callback);
	if (message && (rendition = (struct Cmiss_rendition *)rendition_void))
	{

		struct LIST(Spectrum) *changed_spectrum_list =
			MANAGER_MESSAGE_GET_CHANGE_LIST(Spectrum)(message,
				MANAGER_CHANGE_RESULT(Spectrum));
		if (changed_spectrum_list)
		{
			/* cache the following in case more than one changed spectrum in use */
			Cmiss_rendition_begin_cache(rendition);
			for_each_rendition_in_Cmiss_rendition(
				rendition,
				&Cmiss_rendition_Spectrum_change,
				(void *)changed_spectrum_list);
			DESTROY_LIST(Spectrum)(&changed_spectrum_list);
			Cmiss_rendition_end_cache(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_Spectrum_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_Spectrum_callback */

int Cmiss_rendition_Graphical_material_change(struct Cmiss_rendition *rendition,
	void *changed_material_list_void)
{
	int return_code;
	struct LIST(Graphical_material) *changed_material_list;
	struct Cmiss_graphic_Graphical_material_change_data
		material_change_data;

	ENTER(Cmiss_rendition_Graphical_material_change);
	if (rendition && 
		(changed_material_list = (struct LIST(Graphical_material) *)changed_material_list_void))
	{
		material_change_data.changed_material_list = changed_material_list;
		/* flag to indicate if rendition_changed needs to be called */
		material_change_data.changed = 0;
		return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_Graphical_material_change,
			(void *)&material_change_data, rendition->list_of_graphics);
		if (material_change_data.changed)
		{
			Cmiss_rendition_changed(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_Graphical_material_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} 

/***************************************************************************//**
 * Something has changed globally in the material manager.
 * Tell the rendition it has changed and it will rebuild affected materials too.
 */
static void Cmiss_rendition_Graphical_material_callback(
	struct MANAGER_MESSAGE(Graphical_material) *message, void *rendition_void)
{
	struct Cmiss_rendition *rendition;

	ENTER(Cmiss_rendition_Graphical_material_callback);
	if (message && (rendition = (struct Cmiss_rendition *)rendition_void))
	{
		struct LIST(Graphical_material) *changed_material_list =
			MANAGER_MESSAGE_GET_CHANGE_LIST(Graphical_material)(message,
				MANAGER_CHANGE_RESULT(Graphical_material));
		if (changed_material_list)
		{
			/* cache the following in case more than one changed material in use */
			Cmiss_rendition_begin_cache(rendition);
			/* let Scene_object deal with the changes */
			for_each_rendition_in_Cmiss_rendition(
				rendition,
				&Cmiss_rendition_Graphical_material_change,
				(void *)changed_material_list);
			DESTROY_LIST(Graphical_material)(&changed_material_list);
			Cmiss_rendition_end_cache(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_Graphical_material_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_rendition_Graphical_material_callback */

int Cmiss_rendition_unset_graphics_managers_callback(struct Cmiss_rendition *rendition)
{
	int return_code = 0;

	ENTER(Cmiss_rendition_set_graphics_managers_callback);
	if (rendition && rendition->graphics_module && rendition->list_of_scene->empty())
	{
		struct MANAGER(Graphical_material) *material_manager =
				Cmiss_graphics_module_get_material_manager(rendition->graphics_module);
		if (rendition->graphical_material_manager_callback_id &&
				material_manager)
		{
				MANAGER_DEREGISTER(Graphical_material)(rendition->graphical_material_manager_callback_id,
					material_manager);
				rendition->graphical_material_manager_callback_id = NULL;
		}
		struct MANAGER(Spectrum) *spectrum_manager =
				Cmiss_graphics_module_get_spectrum_manager(rendition->graphics_module);
		if (rendition->spectrum_manager_callback_id &&	spectrum_manager)
		{
			MANAGER_DEREGISTER(Spectrum)(rendition->spectrum_manager_callback_id,
				spectrum_manager);
			rendition->spectrum_manager_callback_id = NULL;
		}
		return_code = 1;
	}
	LEAVE;

	return return_code;
}

int Cmiss_rendition_set_graphics_managers_callback(struct Cmiss_rendition *rendition)
{
	int return_code = 0;
	
	ENTER(Cmiss_rendition_set_graphics_managers_callback);
	if (rendition && rendition->graphics_module)
	{
		struct MANAGER(Graphical_material) *material_manager =
				Cmiss_graphics_module_get_material_manager(rendition->graphics_module);
		if (!rendition->graphical_material_manager_callback_id &&
				material_manager)
		{
			rendition->graphical_material_manager_callback_id=
				MANAGER_REGISTER(Graphical_material)(Cmiss_rendition_Graphical_material_callback,
					(void *)rendition,material_manager);
		}
		struct MANAGER(Spectrum) *spectrum_manager =
				Cmiss_graphics_module_get_spectrum_manager(rendition->graphics_module);
		if (!rendition->spectrum_manager_callback_id &&	spectrum_manager)
		{
			rendition->spectrum_manager_callback_id=
				MANAGER_REGISTER(Spectrum)(Cmiss_rendition_Spectrum_callback,
					(void *)rendition, spectrum_manager);
		}
		return_code = 1;
	}
	LEAVE;

	return return_code;
}

int Cmiss_rendition_call_renderer(struct Cmiss_rendition *rendition, 
	void *renderer_void)
{
		Render_graphics *renderer = static_cast<Render_graphics *>(renderer_void);

		return renderer->Cmiss_rendition_execute(rendition);
}

int Cmiss_rendition_compile_members_rendition(Cmiss_rendition *rendition,
	Render_graphics_compile_members *renderer)
{
	int return_code;

	ENTER(Cmiss_rendition_compile_members);
	if (rendition)
	{
		/* check whether rendition contents need building */
		return_code = Cmiss_rendition_build_graphics_objects(rendition,
			renderer->get_scene(),renderer->time, renderer->name_prefix);
    /* Call the renderer to compile each of the graphics */
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_compile_visible_graphic, (void *)renderer,
			rendition->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_compile_members.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_compile_members  */

int Cmiss_rendition_compile_rendition(Cmiss_rendition *cmiss_rendition,
	Render_graphics_compile_members *renderer)
{
	int return_code = 1;

	ENTER(Cmiss_rendition_compile_members);
	if (cmiss_rendition)
	{
		if (cmiss_rendition->time_object)
		{
			renderer->time = Time_object_get_current_time(cmiss_rendition->time_object);
		}
		else
		{
			renderer->time = 0;
		}
		renderer->name_prefix = cmiss_rendition->name_prefix;
		return_code = renderer->Cmiss_rendition_compile_members(cmiss_rendition);
		renderer->name_prefix = NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_compile.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_rendition_render_opengl(struct Cmiss_rendition *cmiss_rendition,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_rendition_render_opengl);
	if (cmiss_rendition)
	{
		glPushName(0);
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_execute_visible_graphic,
			renderer, cmiss_rendition->list_of_graphics);
		glPopName();
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_render_opengl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_render_opengl */

int execute_Cmiss_rendition(struct Cmiss_rendition *rendition,
	Render_graphics_opengl *renderer)
{
	int return_code;

	ENTER(execute_Cmiss_rendition);
	if (rendition)
	{
		return_code = 1;
		if (rendition->fast_changing == (renderer->fast_changing))
		{
			/* put out the name (position) of the scene_object: */
			//printf("%i \n", rendition->position);
			glLoadName((GLuint)rendition->position);
			/* save a matrix multiply when identity transformation */
			if(rendition->transformation)
			{
				/* Save starting modelview matrix */
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glPushAttrib(GL_TRANSFORM_BIT);
				glEnable(GL_NORMALIZE);
				/* perform individual object transformation */
				wrapperMultiplyCurrentMatrix(rendition->transformation);
			}
			if (rendition->time_object)
			{
				renderer->time = Time_object_get_current_time(rendition->time_object);
			}
			else
			{
				renderer->time = 0;
			}
			return_code = renderer->Cmiss_rendition_execute_members(rendition);
			if (rendition->transformation)	
			{
				/* Restore starting modelview matrix */
				glPopAttrib();
				glPopMatrix();
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_Cmiss_rendition.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct Cmiss_graphic *first_graphic_in_Cmiss_rendition_that(
	struct Cmiss_rendition *cmiss_rendition,
	LIST_CONDITIONAL_FUNCTION(Cmiss_graphic) *conditional_function,
	void *data)
{
	struct Cmiss_graphic *graphic;

	ENTER(first_graphic_in_Cmiss_rendition_that);
	if (cmiss_rendition)
	{
		graphic=FIRST_OBJECT_IN_LIST_THAT(Cmiss_graphic)(
			conditional_function,data,cmiss_rendition->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"first graphic_in_Cmiss_rendition_that.  Invalid arguments");
		graphic=(struct Cmiss_graphic *)NULL;
	}
	LEAVE;

	return (graphic);
} /* first_graphic_in_Cmiss_rendition_that */

int Cmiss_region_modify_rendition(struct Cmiss_region *region,
	struct Cmiss_graphic *graphic, int delete_flag, int position)
{
	int return_code;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *same_graphic;

	ENTER(iterator_modify_rendition);
	if (region && graphic)
	{
		if (NULL != (rendition = Cmiss_region_get_rendition_internal(region)))
		{
			/* get graphic describing same geometry in list */
			same_graphic = first_graphic_in_Cmiss_rendition_that(
				rendition, Cmiss_graphic_same_name_or_geometry,
				(void *)graphic);
			if (delete_flag)
			{
				/* delete */
				if (same_graphic)
				{
					return_code =
						Cmiss_rendition_remove_graphic(rendition, same_graphic);
				}
				else
				{
					return_code = 1;
				}
			}
			else
			{
				/* add/modify */
				if (same_graphic)
				{
					ACCESS(Cmiss_graphic)(same_graphic);
					if (-1 != position)
					{
						/* move same_graphic to new position */
						Cmiss_rendition_remove_graphic(rendition, same_graphic);
						Cmiss_rendition_add_graphic(rendition, same_graphic,
							position);
					}
					/* modify same_graphic to match new ones */
					return_code = Cmiss_rendition_modify_graphic(rendition,
						same_graphic, graphic);
					if (!Cmiss_graphic_get_rendition_private(same_graphic))
						Cmiss_graphic_set_rendition_private(same_graphic, rendition);
					DEACCESS(Cmiss_graphic)(&same_graphic);
				}
				else
				{
					return_code = 0;
					if (NULL != (same_graphic = CREATE(Cmiss_graphic)(
								 Cmiss_graphic_get_graphic_type(graphic))))
					{
						ACCESS(Cmiss_graphic)(same_graphic);
						if (Cmiss_graphic_copy_without_graphics_object(
							same_graphic, graphic))
						{
							return_code = Cmiss_rendition_add_graphic(rendition,
								same_graphic, position);
						}
						DEACCESS(Cmiss_graphic)(&same_graphic);
					}
				}
			}
			DEACCESS(Cmiss_rendition)(&rendition);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_modify_rendition.  Region rendition cannot be found");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_modify_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* "Cmiss_modify_g_element */

int Cmiss_rendition_add_callback(struct Cmiss_rendition *rendition, 
	Cmiss_rendition_callback callback, void *user_data)
{
	int return_code;
	struct Cmiss_rendition_callback_data *callback_data, *previous;

	ENTER(Cmiss_rendition_add_callback);

	if (rendition && callback)
	{
		if(ALLOCATE(callback_data, struct Cmiss_rendition_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct Cmiss_rendition_callback_data *)NULL;
			if(rendition->update_callback_list)
			{
				previous = rendition->update_callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				rendition->update_callback_list = callback_data;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_rendition_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_add_callback.  Missing rendition object or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_rendition_remove_callback(struct Cmiss_rendition *rendition,
	Cmiss_rendition_callback callback, void *user_data)
{
	int return_code;
	struct Cmiss_rendition_callback_data *callback_data, *previous;

	ENTER(Cmiss_rendition_remove_callback);

	if (rendition && callback && rendition->update_callback_list)
	{
		callback_data = rendition->update_callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			rendition->update_callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;		
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
"Cmiss_rendition_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"Cmiss_rendition_remove_callback.  Missing Cmiss_rendition, callback or callback list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_remove_callback */

int gfx_modify_rendition_general(struct Parse_state *state,
	void *cmiss_region_void, void *dummy_void)
{
	int circle_discretization, clear_flag, return_code = 0;
	struct Cmiss_region *cmiss_region;
	struct Computed_field *default_coordinate_field = NULL;
	struct FE_field *native_discretization_field = NULL;
	struct Set_FE_field_conditional_FE_region_data
		native_discretization_field_conditional_data;
	struct Element_discretization element_discretization;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(gfx_modify_rendition_general);
	USE_PARAMETER(dummy_void);
	if (state && (cmiss_region = (struct Cmiss_region *)cmiss_region_void))
	{
		/* if possible, get defaults from element_group on default scene */
		rendition = Cmiss_region_get_rendition_internal(cmiss_region);
		if (rendition)
		{
			if (NULL != (default_coordinate_field=
					Cmiss_rendition_get_default_coordinate_field(rendition)))
			{
				ACCESS(Computed_field)(default_coordinate_field);
			}
			circle_discretization=Cmiss_rendition_get_circle_discretization(
				rendition);
			Cmiss_rendition_get_element_discretization(rendition,
				&element_discretization);
			if (NULL != (native_discretization_field=
					Cmiss_rendition_get_native_discretization_field(rendition)))
			{
				ACCESS(FE_field)(native_discretization_field);
			}
			clear_flag=0;
			
			option_table = CREATE(Option_table)();
			/* circle_discretization */
			Option_table_add_entry(option_table, "circle_discretization",
				(void *)&circle_discretization, (void *)NULL,
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
				(void *)&element_discretization, (void *)NULL,
				set_Element_discretization);
			/* native_discretization */
			native_discretization_field_conditional_data.conditional_function = 
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
			native_discretization_field_conditional_data.user_data = (void *)NULL;
			native_discretization_field_conditional_data.fe_region =
				Cmiss_region_get_FE_region(cmiss_region);
			Option_table_add_entry(option_table, "native_discretization",
				(void *)&native_discretization_field,
				(void *)&native_discretization_field_conditional_data,
				set_FE_field_conditional_FE_region);
			if ((return_code = Option_table_multi_parse(option_table, state)))
			{
				Cmiss_rendition_begin_cache(rendition);
				if (clear_flag)
 				{
					/* remove all graphic from group */
					while (return_code && (NULL != (graphic=
						first_graphic_in_Cmiss_rendition_that(rendition,
							(LIST_CONDITIONAL_FUNCTION(Cmiss_graphic) *)NULL,
							(void *)NULL))))
					{
						return_code = Cmiss_rendition_remove_graphic(rendition, graphic);
					}
				}
				Cmiss_rendition_set_circle_discretization(rendition,
					circle_discretization);
				Cmiss_rendition_set_element_discretization(rendition,
					&element_discretization);
				if (default_coordinate_field)
				{
					Cmiss_rendition_set_default_coordinate_field(rendition,
						default_coordinate_field);
				}
				Cmiss_rendition_set_native_discretization_field(rendition,
					native_discretization_field);
				/* regenerate graphics for changed graphic */
				Cmiss_rendition_end_cache(rendition);
				return_code=1;
				
				DESTROY(Option_table)(&option_table);
				DEACCESS(Cmiss_rendition)(&rendition);
			}
			if (default_coordinate_field)
			{
				DEACCESS(Computed_field)(&default_coordinate_field);
			}
			if (native_discretization_field)
			{
				DEACCESS(FE_field)(&native_discretization_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_general.  "
				"Missing state");
			return_code=0;
		}
		LEAVE;
	}
	return (return_code);
} /* gfx_modify_rendition_general */
	
int Cmiss_rendition_for_each_material(struct Cmiss_rendition *rendition,	
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data)
{
	int return_code;

	ENTER(Cmiss_rendition_for_each_material);
	if (rendition && iterator_function && user_data)
	{
		/* Could be smarter if there was a reduced number used by the 
			scene, however for now just do every material in the manager */
		MANAGER(Graphical_material) *graphical_material_manager =
				Cmiss_graphics_module_get_material_manager(rendition->graphics_module);
		return_code = FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
			iterator_function, user_data, graphical_material_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_for_each_material.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return return_code;
}

struct Cmiss_rendition_process_temp_data
{
	 class Process_list_or_write_command_class *process_message;
	 struct Cmiss_graphic_list_data *list_data;
};

/***************************************************************************//**
 * Writes out the <graphic> as a text string in the command window with the
 * <graphic_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int Cmiss_rendition_process_Cmiss_graphic_list_contents(
	 struct Cmiss_graphic *graphic,	void *process_temp_data_void)
{
	int return_code;
	char *graphic_string; /*,line[40];*/
	struct Cmiss_graphic_list_data *list_data;
	class Process_list_or_write_command_class *process_message;
	struct Cmiss_rendition_process_temp_data *process_temp_data;

	ENTER(Cmiss_rendition_process_Cmiss_graphic_list_contents);
	if (graphic&&
		 (process_temp_data=(struct Cmiss_rendition_process_temp_data *)process_temp_data_void))
	{
		 if (NULL != (process_message = process_temp_data->process_message) && 
			 NULL != (list_data = process_temp_data->list_data))
		 {
			 if (NULL != (graphic_string=Cmiss_graphic_string(graphic,
						 list_data->graphic_string_detail)))
				{
					 if (list_data->line_prefix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_prefix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,graphic_string);
					 if (list_data->line_suffix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_suffix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,"\n");
					 DEALLOCATE(graphic_string);
					 return_code=1;
				}
			 return_code= 1;
		 }
		 else
		 {
				return_code=0;
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Cmiss_rendition_process_Cmiss_graphic_list_contents.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;
	
	return (return_code);
}

/***************************************************************************//**
 * Will write commands to comfile or list commands to the command windows
 * depending on the class.
 */
int Cmiss_rendition_process_list_or_write_window_commands(struct Cmiss_rendition *rendition,
	 char *command_prefix,char *command_suffix, class Process_list_or_write_command_class *process_message)
{
	int return_code;
	struct Cmiss_graphic_list_data list_data;

	ENTER(Cmiss_rendition_process_list_or_write_window_command);
	if (rendition && command_prefix)
	{
		process_message->process_command(INFORMATION_MESSAGE,command_prefix);
		process_message->process_command(INFORMATION_MESSAGE,"general clear");
		if (command_suffix)
		{
			process_message->process_command(INFORMATION_MESSAGE,command_suffix);
		}
		process_message->process_command(INFORMATION_MESSAGE,"\n");
		list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE;
		list_data.line_prefix=command_prefix;
		list_data.line_suffix=command_suffix;
		struct Cmiss_rendition_process_temp_data *process_temp_data;
		if (ALLOCATE(process_temp_data,struct Cmiss_rendition_process_temp_data,1))
		{
			 process_temp_data->process_message = process_message;
			 process_temp_data->list_data = &list_data;
			 return_code=FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
					Cmiss_rendition_process_Cmiss_graphic_list_contents,(void *)process_temp_data,
					rendition->list_of_graphics);
			 DEALLOCATE(process_temp_data);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_list_commands.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_process_list_or_write_window_command*/

int Cmiss_rendition_list_commands(struct Cmiss_rendition *rendition,
	 char *command_prefix,char *command_suffix)
{
	 int return_code;

	 ENTER(Cmiss_rendition_list_commands);
	 if (Process_list_command_class *list_message =
			new Process_list_command_class())
	 {
			return_code = Cmiss_rendition_process_list_or_write_window_commands(
				rendition, command_prefix, command_suffix, list_message);
			delete list_message;
	 }
	 LEAVE;

	 return (return_code);
}

int Cmiss_rendition_list_contents(struct Cmiss_rendition *rendition)
{
	char *name = 0;
	int return_code;
	struct Cmiss_graphic_list_data list_data;

	ENTER(Cmiss_rendition_list_contents);
	if (rendition)
	{
		display_message(INFORMATION_MESSAGE,"  circle discretization: %d\n",
			rendition->circle_discretization);
		if (rendition->default_coordinate_field)
		{
			if (GET_NAME(Computed_field)(rendition->default_coordinate_field,
				&name))
			{
				display_message(INFORMATION_MESSAGE,
					"  default coordinate field: %s\n",name);
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE,
				"  default coordinate field: NONE\n");
		}
		display_message(INFORMATION_MESSAGE,"  element discretization: %d*%d*%d\n",
			rendition->element_discretization.number_in_xi1,
			rendition->element_discretization.number_in_xi2,
			rendition->element_discretization.number_in_xi3);
		display_message(INFORMATION_MESSAGE,"  native discretization field: ");
		if (rendition->native_discretization_field)
		{
			if (GET_NAME(FE_field)(rendition->native_discretization_field,
				&name))
			{
				display_message(INFORMATION_MESSAGE,"%s\n",name);
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"none\n");
		}
		if (0 < NUMBER_IN_LIST(Cmiss_graphic)(
			rendition->list_of_graphics))
		{
			display_message(INFORMATION_MESSAGE,"  graphics objects defined:\n");
			list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE_PLUS;
			list_data.line_prefix="  ";
			list_data.line_suffix="";
			return_code=FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_list_contents,(void *)&list_data,
				rendition->list_of_graphics);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no graphics graphic defined\n");
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_list_contents.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_list_contents */

int for_each_rendition_in_Cmiss_rendition(
	struct Cmiss_rendition *rendition,
	int (*cmiss_rendition_tree_iterator_function)(struct Cmiss_rendition *rendition,
		void *user_data),	void *user_data)
{
	int return_code;
	struct Cmiss_region *region, *child_region = NULL;
	struct Cmiss_rendition *child_rendition;
	
	ENTER(for_each_rendition_in_Cmiss_rendition);
	if (rendition)
	{
		region = ACCESS(Cmiss_region)(rendition->region);
		return_code = (*cmiss_rendition_tree_iterator_function)(rendition, user_data);
		if (return_code)
		{
			child_region = Cmiss_region_get_first_child(region);
			while (child_region)
			{
				child_rendition = Cmiss_region_get_rendition_internal(child_region);
				if (child_rendition)
				{
					return_code = for_each_rendition_in_Cmiss_rendition(
						child_rendition, cmiss_rendition_tree_iterator_function,user_data);
					DEACCESS(Cmiss_rendition)(&child_rendition);
				}
				Cmiss_region_reaccess_next_sibling(&child_region);
			}
		}
		DEACCESS(Cmiss_region)(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"for_each_rendition_in_Cmiss_rendition.  "
			"Invalid argument(s)");
		return_code=0;	
	}
	LEAVE;

	return (return_code);	
}

int Cmiss_rendition_get_graphic_position(
	struct Cmiss_rendition *rendition, struct Cmiss_graphic *graphic)
{
	int position;

	ENTER(Cmiss_rendition_get_graphic_position);
	if (rendition&&graphic)
	{
		position=Cmiss_graphic_get_position_in_list(graphic,
			rendition->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_graphic_position. Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* Cmiss_rendition_get_graphic_position */

int Cmiss_renditions_match(struct Cmiss_rendition *rendition1,
	struct Cmiss_rendition *rendition2)
{
	int i, number_of_graphic, return_code;
	struct Cmiss_graphic *graphic1, *graphic2;

	ENTER(Cmiss_renditions_match);
	if (rendition1 && rendition2)
	{
		if ((rendition1->fe_region ==
			rendition2->fe_region) &&
			(rendition1->data_fe_region ==
				rendition2->data_fe_region) &&
			(rendition1->element_discretization.number_in_xi1 ==
				rendition2->element_discretization.number_in_xi1) &&
			(rendition1->element_discretization.number_in_xi2 ==
				rendition2->element_discretization.number_in_xi2) &&
			(rendition1->element_discretization.number_in_xi3 ==
				rendition2->element_discretization.number_in_xi3) &&
			(rendition1->circle_discretization ==
				rendition2->circle_discretization) &&
			(rendition1->default_coordinate_field ==
				rendition2->default_coordinate_field) &&
			(rendition1->native_discretization_field ==
				rendition2->native_discretization_field) &&
			((number_of_graphic = NUMBER_IN_LIST(Cmiss_graphic)(
				rendition1->list_of_graphics)) ==
				NUMBER_IN_LIST(Cmiss_graphic)(
					rendition2->list_of_graphics)))
		{
			return_code = 1;
			for (i = 1; return_code && (i <= number_of_graphic); i++)
			{
				graphic1 = FIND_BY_IDENTIFIER_IN_LIST(Cmiss_graphic, position)(
					i, rendition1->list_of_graphics);
				graphic2 = FIND_BY_IDENTIFIER_IN_LIST(Cmiss_graphic, position)(
					i, rendition2->list_of_graphics);
				return_code = Cmiss_graphic_match(graphic1, graphic2);
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_renditions_match.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_renditions_match */

int Cmiss_rendition_copy(struct Cmiss_rendition *destination,
	struct Cmiss_rendition *source)
/***************************************************************************//**
 * Copies the Cmiss_rendition contents from source to destination.
 * Pointers to graphics_objects are cleared in the destination list of graphic.
 * NOTES:
 * - not a full copy; does not copy groups, selection etc. Use copy_create for
 * this task so that callbacks can be set up for these.
 * - does not copy graphics objects to graphic in destination.
 */
{
	int return_code;

	ENTER(Cmiss_rendition_copy);
	if (destination&&source)
	{
		/* copy general settings */
		destination->element_discretization.number_in_xi1=
			source->element_discretization.number_in_xi1;
		destination->element_discretization.number_in_xi2=
			source->element_discretization.number_in_xi2;
		destination->element_discretization.number_in_xi3=
			source->element_discretization.number_in_xi3;
		destination->circle_discretization=source->circle_discretization;
		if (source->default_coordinate_field)
		{
			ACCESS(Computed_field)(source->default_coordinate_field);
		}
		if (destination->default_coordinate_field)
		{
			DEACCESS(Computed_field)(&(destination->default_coordinate_field));
		}
		destination->default_coordinate_field=source->default_coordinate_field;
		if (source->native_discretization_field)
		{
			ACCESS(FE_field)(source->native_discretization_field);
		}
		if (destination->native_discretization_field)
		{
			DEACCESS(FE_field)(&(destination->native_discretization_field));
		}
		destination->native_discretization_field=
			source->native_discretization_field;
		/* empty original list_of_settings */
		REMOVE_ALL_OBJECTS_FROM_LIST(Cmiss_graphic)(
			destination->list_of_graphics);
		/* put copy of each settings in source list in destination list */
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_copy_and_put_in_list,
			(void *)destination->list_of_graphics,source->list_of_graphics);
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_set_rendition_for_list_private,
			destination, destination->list_of_graphics);
		destination->visibility_flag = source->visibility_flag;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_copy.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_copy */

struct Cmiss_rendition *create_editor_copy_Cmiss_rendition(
	struct Cmiss_rendition *existing_rendition)
{
	struct Cmiss_rendition *rendition;

	ENTER(create_editor_copy_Cmiss_rendition);
	if (existing_rendition)
	{
		/* make an empty Cmiss_rendition for the same groups */
		if (NULL != (rendition = CREATE(Cmiss_rendition)(
					 existing_rendition->region, NULL)))
		{
			/* copy settings WITHOUT graphics objects; do not cause whole function
				 to fail if copy fails */
			Cmiss_rendition_copy(rendition,existing_rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_editor_copy_Cmiss_rendition.  Invalid argument(s)");
		rendition=(struct Cmiss_rendition *)NULL;
	}
	LEAVE;

	return (rendition);
} /* create_editor_copy_Cmiss_rendition */

int for_each_graphic_in_Cmiss_rendition(
	struct Cmiss_rendition *rendition,
	int (*cmiss_rendition_graphic_iterator_function)(struct Cmiss_graphic *graphic,
		void *user_data),	void *user_data)
{
	int return_code = 0;

	ENTER( for_each_graphic_in_Cmiss_rendition);
	if (rendition&&cmiss_rendition_graphic_iterator_function)
	{
		return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			*cmiss_rendition_graphic_iterator_function,user_data,
			rendition->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_graphic_in_Cmiss_rendition.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_rendition_get_number_of_graphic(struct Cmiss_rendition *rendition)
{
	int number_of_graphic;

	ENTER(Cmiss_group_get_number_of_graphic);
	if (rendition)
	{
		number_of_graphic =
			NUMBER_IN_LIST(Cmiss_graphic)(rendition->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_group_get_number_of_graphic.  Invalid argument(s)");
		number_of_graphic = 0;
	}
	LEAVE;

	return (number_of_graphic);
} /* Cmiss_group_get_number_of_graphic */

struct Cmiss_graphic *Cmiss_rendition_get_graphic_at_position(
	struct Cmiss_rendition *rendition,int position)
{
	struct Cmiss_graphic *graphic;

	ENTER(get_graphic_at_position_in_Cmiss_rendition);
	if (rendition)
	{
		graphic=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_graphic,
			position)(position,rendition->list_of_graphics);
		if (graphic)
		{
			ACCESS(Cmiss_graphic)(graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_graphic_at_position_in_Cmiss_rendition.  Invalid arguments");
		graphic=(struct Cmiss_graphic *)NULL;
	}
	LEAVE;

	return (graphic);
} /* get_graphic_at_position_in_Cmiss_rendition */

struct Cmiss_region *Cmiss_rendition_get_region(
	struct Cmiss_rendition *rendition)
{
	struct Cmiss_region *region;

	ENTER(Cmiss_rendition_get_region);
	if (rendition)
	{
		region=rendition->region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_region.  Invalid arguments");
		region=(struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (region);
}

int Cmiss_rendition_modify(struct Cmiss_rendition *destination,
	struct Cmiss_rendition *source)
{
	int return_code;
	struct LIST(Cmiss_graphic) *temp_list_of_graphics;

	ENTER(Cmiss_rendition_modify);
	if (destination && source)
	{
		if (NULL != (temp_list_of_graphics = CREATE(LIST(Cmiss_graphic))()))
		{
			if (source->default_coordinate_field)
			{
				Cmiss_rendition_set_default_coordinate_field_private(destination,
					source->default_coordinate_field);
			}
			Cmiss_rendition_set_circle_discretization_private(destination,
				source->circle_discretization);
			Cmiss_rendition_set_element_discretization_private(destination,
				&(source->element_discretization));
			Cmiss_rendition_set_native_discretization_field_private(destination,
				source->native_discretization_field);
			/* make copy of source graphic without graphics objects */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_copy_and_put_in_list,
				(void *)temp_list_of_graphics, source->list_of_graphics);
			/* extract graphics objects that can be reused from destination list */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_extract_graphics_object_from_list,
				(void *)destination->list_of_graphics, temp_list_of_graphics);
			FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_set_rendition_for_list_private,
				destination, temp_list_of_graphics);
			/* replace the destination list of graphic with temp_list_of_graphics */
			DESTROY(LIST(Cmiss_graphic))(&(destination->list_of_graphics));
			destination->list_of_graphics = temp_list_of_graphics;
			/* inform the client of the change */
			Cmiss_rendition_changed(destination);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_rendition_modify.  Could not create temporary list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_modify.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_modify */

int Cmiss_rendition_set_visibility_flag(struct Cmiss_rendition *rendition, 
	int visibility_flag)
{
	if (rendition)
	{
		bool bool_visibility_flag = visibility_flag;
		if (rendition->visibility_flag != bool_visibility_flag)
		{
			rendition->visibility_flag = bool_visibility_flag;
			Cmiss_rendition_changed(rendition);
		}
		return 1;
	}
	return 0;
}

int Cmiss_rendition_get_visibility_flag(
	struct Cmiss_rendition *rendition)
{
	if (rendition)
		return rendition->visibility_flag;
	else
		return 0;
}

int Cmiss_rendition_add_transformation_callback(struct Cmiss_rendition *rendition,
	CMISS_CALLBACK_FUNCTION(Cmiss_rendition_transformation) *function, void *user_data)
{
	int return_code;

	ENTER(Cmiss_rendition_add_transformation_callback);
	if (rendition && function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Cmiss_rendition_transformation)(
			rendition->transformation_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_rendition_add_transformation_callback.  Could not add callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_add_transformation_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_add_transformation_callback */

int Cmiss_rendition_remove_transformation_callback(
	struct Cmiss_rendition *rendition,
	CMISS_CALLBACK_FUNCTION(Cmiss_rendition_transformation) *function, void *user_data)
{
	int return_code;

	ENTER(Cmiss_rendition_remove_transformation_callback);
	if (rendition && function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Cmiss_rendition_transformation)(
			rendition->transformation_callback_list, function,user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_rendition_remove_transformation_callback.  "
				"Could not remove callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_remove_transformation_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_remove_transformation_callback */


int Cmiss_rendition_has_transformation(struct Cmiss_rendition *rendition)
{
	int return_code;

	ENTER(Cmiss_rendition_has_transformation);
	if (rendition)
	{
		if(rendition->transformation)
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
			"Cmiss_rendition_has_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_has_transformation */

int Cmiss_rendition_get_transformation(struct Cmiss_rendition *rendition,
	gtMatrix *transformation)
{
	int i, j, return_code;

	ENTER(Cmiss_rendition_get_transformation);
	if (rendition)
	{
		if(rendition->transformation)
		{
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					(*transformation)[i][j] = (*rendition->transformation)[i][j];
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
			"Cmiss_rendition_get_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_get_transformation */

int Cmiss_rendition_set_transformation(struct Cmiss_rendition *rendition,
	gtMatrix *transformation)
{
	int i, j, return_code;

	ENTER(Cmiss_rendition_set_transformation);
	if (rendition)
	{
		return_code = 1;
		if ((!transformation) || gtMatrix_is_identity(transformation))
		{
			if (rendition->transformation)
			{
				DEALLOCATE(rendition->transformation);
			}
		}
		else
		{
			if (rendition->transformation)
			{
				if (!gtMatrix_match(transformation, rendition->transformation))
				{
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							(*rendition->transformation)[i][j] = (*transformation)[i][j];
						}
					}
				}
			}
			else
			{
				if (ALLOCATE(rendition->transformation, gtMatrix, 1))
				{
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							(*rendition->transformation)[i][j] = (*transformation)[i][j];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Cmiss_rendition_set_transformation.  "
						"Unable to allocate transformation");
					return_code = 0;				
				}
			}
		}
		CMISS_CALLBACK_LIST_CALL(Cmiss_rendition_transformation)(
			rendition->transformation_callback_list, rendition,
			rendition->transformation);
		Cmiss_rendition_changed_external(rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_transformation.  Missing rendition");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_transformation */

static int Cmiss_rendition_set_time_dependent_transformation(struct Time_object *time_object,
	double current_time, void *rendition_void)
{
	 int return_code, i, j, k;
	 struct Cmiss_rendition *rendition;
	 FE_value *values;
	 gtMatrix transformation_matrix;

	 ENTER(Cmiss_rendition_set_time_dependent_transformation);
	 USE_PARAMETER(time_object);

	 if (NULL != (rendition = (struct Cmiss_rendition *)rendition_void))
	 {
			if (rendition->transformation_field)
			{
				 if (ALLOCATE(values, FE_value, 16))
				 {
						Computed_field_evaluate_without_node(rendition->transformation_field, 
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
						return_code = Cmiss_rendition_set_transformation(rendition,
							 &transformation_matrix);
						DEALLOCATE(values);
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "Cmiss_rendition_set_time_dependent_transformation.  "
							 "Unable to allocate values.");
						return_code=0;
				 }
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"Cmiss_rendition_set_time_dependent_transformation.  "
						"Missing transformation field.");
				 return_code=0;
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Cmiss_rendition_set_time_dependent_transformation.  "
				 "invalid argument.");
			return_code=0;
	 }
	 LEAVE;

	 return (return_code);
}

void Cmiss_rendition_remove_time_dependent_transformation(struct Cmiss_rendition *rendition)
{
	ENTER(Cmiss_rendition_remove_time_dependent_transformation);

	if (rendition->transformation_time_callback_flag)
	{
		 Time_object_remove_callback(rendition->time_object,
				Cmiss_rendition_set_time_dependent_transformation, rendition);
		 DEACCESS(Computed_field)(&(rendition->transformation_field));
		 rendition->transformation_time_callback_flag = 0;
	}

	LEAVE;
}

int Cmiss_rendition_set_transformation_with_time_callback(struct Cmiss_rendition *rendition,
	 struct Computed_field *transformation_field)
{
	 int return_code;

	 ENTER(Cmiss_rendition_set_transformation_with_time_callback);

	 return_code = 0;
	 if (rendition && transformation_field)
	 {
			if (rendition->time_object)
			{
				 Cmiss_rendition_remove_time_dependent_transformation(rendition);
				 rendition->transformation_field=
						ACCESS(Computed_field)(transformation_field);
				 Cmiss_rendition_set_time_dependent_transformation(rendition->time_object,
						Time_object_get_current_time(rendition->time_object),
						(void *)rendition);
				 Time_object_add_callback(rendition->time_object,
						Cmiss_rendition_set_time_dependent_transformation, rendition);
				 rendition->transformation_time_callback_flag = 1;
				 return_code = 1;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"Cmiss_rendition_set_transformation_with_time_callback.  "
						"Missing time object.");
				 return_code=0;
			}
			return_code = rendition->transformation_time_callback_flag;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Cmiss_rendition_set_transformation_with_time_callback.  "
				 "Invalid argument(s).");
			return_code=0;
	 }

	 LEAVE;	

	 return (return_code);
}

static int Cmiss_rendition_time_update_callback(struct Time_object *time_object,
	double current_time, void *rendition_void)
{
	int return_code;
	struct Cmiss_rendition *rendition;

	ENTER(Cmiss_rendition_time_update_callback);
	USE_PARAMETER(current_time);
	if (time_object && (rendition=(struct Cmiss_rendition *)rendition_void))
	{
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_time_change,NULL,
			rendition->list_of_graphics);
		Cmiss_rendition_changed(rendition);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_time_update_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_time_update_callback */

int Cmiss_rendition_has_multiple_times(
	struct Cmiss_rendition *rendition)
{
	int return_code;
	struct Cmiss_graphic_update_time_behaviour_data data;

	ENTER(Cmiss_rendition_has_multiple_times);
	if (rendition)
	{
		data.default_coordinate_depends_on_time = 0;
		data.time_dependent = 0;
		FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
			Cmiss_graphic_update_time_behaviour, (void *)&data,
			rendition->list_of_graphics);
		return_code = data.time_dependent;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_has_multiple_times.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_has_multiple_times */

struct Time_object *Cmiss_rendition_get_time_object(struct Cmiss_rendition *rendition)
{
	struct Time_object *return_time;

	ENTER(Cmiss_rendition_get_time_object);
	if (rendition)
	{
		return_time=rendition->time_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_get_time_object.  Missing rendition");
		return_time=(struct Time_object *)NULL;
	}
	LEAVE;

	return (return_time);
} /* Cmiss_rendition_get_time_object */

int Cmiss_rendition_set_time_object(struct Cmiss_rendition *rendition,
	struct Time_object *time)
{
	int return_code;
	ENTER(Cmiss_rendition_set_time_object);
	if (rendition)
	{
		if (rendition->time_object != time)
		{
			if (rendition->time_object)
			{
				Time_object_remove_callback(rendition->time_object,
					Cmiss_rendition_time_update_callback, rendition);
			}
			REACCESS(Time_object)(&(rendition->time_object),time);
			if (time)
			{
				Time_object_add_callback(rendition->time_object,
					Cmiss_rendition_time_update_callback, rendition);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_set_time_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_set_time_object */

static int Cmiss_rendition_update_time_behaviour(struct Cmiss_rendition *rendition)
{
// 	char *time_object_name;
	int return_code;
	struct Time_object *time;
	
	ENTER(Cmiss_rendition_update_time_behaviour);
	if (rendition)
	{
		return_code = 1;
		/* Ensure the Scene object has a time object if and only if the 
			graphics object has more than one time */
		if(Cmiss_rendition_has_multiple_times(rendition))
		{
			time = Cmiss_rendition_get_time_object(rendition);
			if(!time)
			{
				time = Time_object_create_regular(/*update_frequency*/10.0,
					/*time_offset*/0.0);
				Cmiss_rendition_set_time_object(rendition, time);
				struct Time_keeper *time_keeper = Cmiss_graphics_module_get_time_keeper_internal(
					rendition->graphics_module);
				if(time_keeper)
				{
					Time_keeper_add_time_object(time_keeper, time);
					DEACCESS(Time_keeper)(&time_keeper);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_rendition_update_time_behaviour.  "
						"Missing time keeper ");
					return_code =0;
				}
				DEACCESS(Time_object)(&time);
			}
		}
		else
		{
			time = Cmiss_rendition_get_time_object(rendition);
			if(time == NULL)
			{
				Cmiss_rendition_set_time_object(rendition,
					(struct Time_object *)NULL);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_update_time_behaviour.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_rendition_update_callback(struct Cmiss_rendition *rendition, void *dummy_void)
{
	int return_code;
	USE_PARAMETER(dummy_void);

	ENTER(Cmiss_rendition_update_callback);
	if (rendition)
	{
		if (rendition->visibility_flag)
		{
			//Cmiss_rendition_changed(rendition);
			Cmiss_rendition_update_time_behaviour(rendition);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_update_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_update_callback */

int Cmiss_rendition_add_scene(struct Cmiss_rendition *rendition,
	struct Scene *scene, int hierarchical)
{
	if (rendition && scene)
	{
		Scene_begin_cache(scene);
		if (!rendition->list_of_scene)
		{
			rendition->list_of_scene = new std::list<struct Scene *>;
		}
		rendition->list_of_scene->push_back(scene);
		Cmiss_scene_add_rendition(scene, rendition);
		if (hierarchical)
		{
			struct Cmiss_region *region = ACCESS(Cmiss_region)(rendition->region);
			struct Cmiss_region *child_region;
			struct Cmiss_rendition *child_rendition;
			
			child_region = Cmiss_region_get_first_child(region);
			while (child_region)
			{
				if (NULL != (child_rendition = Cmiss_region_get_rendition_internal(child_region)))
				{
					Cmiss_rendition_add_scene(child_rendition,scene, 1);
					DEACCESS(Cmiss_rendition)(&child_rendition);
				}
				Cmiss_region_reaccess_next_sibling(&child_region);
			}
			DEACCESS(Cmiss_region)(&region);
		}
		Scene_end_cache(scene);
		return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_add_scene.  Invalid argument(s)");
		return 0;
	}
}

int Cmiss_rendition_remove_scene(struct Cmiss_rendition *rendition, struct Scene *scene)
{
	if (rendition && scene)
	{
		if (rendition->list_of_scene &&
			!rendition->list_of_scene->empty())
		{
			rendition->list_of_scene->remove(scene);
		}
		return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_remove_scene.  Invalid argument(s)");
		return 0;
	}
}

int DESTROY(Cmiss_rendition)(
	struct Cmiss_rendition **cmiss_rendition_address)
{
	int return_code;
	struct Cmiss_rendition *cmiss_rendition;
	struct Cmiss_rendition_callback_data *callback_data, *next;

	ENTER(DESTROY(Cmiss_rendition));

	if (cmiss_rendition_address && (cmiss_rendition = *cmiss_rendition_address))
	{
		Cmiss_rendition_remove_time_dependent_transformation(cmiss_rendition);
		if (cmiss_rendition->graphics_module)
		{
			Element_point_ranges_selection_add_callback(
				Cmiss_graphics_module_get_element_point_ranges_selection(cmiss_rendition->graphics_module),
				Cmiss_rendition_element_points_ranges_selection_change,
				(void *)cmiss_rendition);
		}
		if (cmiss_rendition->name_prefix)
		{
			DEALLOCATE(cmiss_rendition->name_prefix);
		}
		if (cmiss_rendition->transformation_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Cmiss_rendition_transformation)))(
				&(cmiss_rendition->transformation_callback_list));
		}
		if (cmiss_rendition->transformation)
		{
			DEALLOCATE(cmiss_rendition->transformation);
		}
		if (cmiss_rendition->selection_group)
		{
			DEACCESS(Computed_field)(&(cmiss_rendition->selection_group));
		}
		if (cmiss_rendition->list_of_scene)
		{
			if (!cmiss_rendition->list_of_scene->empty())
			{
				std::list<struct Scene *>::iterator pos =
					cmiss_rendition->list_of_scene->begin();
				while (pos != cmiss_rendition->list_of_scene->end())
				{
					Cmiss_scene_remove_rendition(*pos,cmiss_rendition);
					++pos;
				}
			}
			delete cmiss_rendition->list_of_scene;
		}
		if (cmiss_rendition->fe_region_callback_set)
		{
			FE_region_remove_callback(cmiss_rendition->fe_region,
				Cmiss_rendition_FE_region_change, (void *)cmiss_rendition);
		}
		if (cmiss_rendition->data_fe_region && cmiss_rendition->data_fe_region_callback_set)
		{
			FE_region_remove_callback(cmiss_rendition->data_fe_region,
				Cmiss_rendition_data_FE_region_change, (void *)cmiss_rendition);
		}
		if (cmiss_rendition->time_object)
		{
			DEACCESS(Time_object)(&cmiss_rendition->time_object);
		}
		if (cmiss_rendition->graphical_material_manager_callback_id)
		{
			MANAGER_DEREGISTER(Graphical_material)(
				cmiss_rendition->graphical_material_manager_callback_id,
				Cmiss_graphics_module_get_material_manager(cmiss_rendition->graphics_module));
		}
		if (cmiss_rendition->spectrum_manager_callback_id)
		{
			MANAGER_DEREGISTER(Spectrum)(
				cmiss_rendition->spectrum_manager_callback_id,
				Cmiss_graphics_module_get_spectrum_manager(cmiss_rendition->graphics_module));
		}
		if (cmiss_rendition->default_coordinate_field)
		{
			DEACCESS(Computed_field)(&(cmiss_rendition->default_coordinate_field));
		}
		if (cmiss_rendition->native_discretization_field)
		{
			DEACCESS(FE_field)(&(cmiss_rendition->native_discretization_field));
		}
		if (cmiss_rendition->list_of_graphics)
		{
			DESTROY(LIST(Cmiss_graphic))(
				&(cmiss_rendition->list_of_graphics));
		}
		if (cmiss_rendition->data_fe_region)
		{
			DEACCESS(FE_region)(&(cmiss_rendition->data_fe_region));
		}
		if (cmiss_rendition->fe_region)
		{
			DEACCESS(FE_region)(&(cmiss_rendition->fe_region));
		}
		callback_data = cmiss_rendition->update_callback_list;
		while(callback_data)
		{
			next = callback_data->next;
			DEALLOCATE(callback_data);
			callback_data = next;
		}
		if (cmiss_rendition->transformation_field)
		{
			DEACCESS(Computed_field)(&(cmiss_rendition->transformation_field));
		}
		DEALLOCATE(*cmiss_rendition_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_rendition).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_rendition_end_change(Cmiss_rendition_id rendition)
{
	if (rendition)
	{
		Cmiss_rendition_changed_external(rendition);
		Cmiss_rendition_end_cache(rendition);
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_rendition_add_glyph(struct Cmiss_rendition *rendition, 
	struct GT_object *glyph, const char *cmiss_graphic_name)
{
	int return_code = 0;

	ENTER(Cmiss_rendition_add_glyph);
	if (rendition && glyph)
	{
		if (!FIRST_OBJECT_IN_LIST_THAT(Cmiss_graphic)(Cmiss_graphic_has_name,
				(void *)cmiss_graphic_name, rendition->list_of_graphics))
		{
			Cmiss_rendition_begin_change(rendition);
			Cmiss_graphic *graphic = Cmiss_rendition_create_static(rendition);
			struct Graphical_material *material = get_GT_object_default_material(glyph);
			if (!material)
			{
				struct Material_package *material_package =
					Cmiss_graphics_module_get_material_package(rendition->graphics_module);
				if (material_package)
				{
					material = Material_package_get_default_material(material_package);
					DEACCESS(Material_package)(&material_package);
				}
			}
			//set_GT_object_default_material(glyph, NULL);
			if (graphic && Cmiss_graphic_set_name(graphic, cmiss_graphic_name))
			{
				struct Computed_field *orientation_scale_field, *variable_scale_field;
				struct GT_object *old_glyph;
				Triple glyph_centre,glyph_scale_factors,glyph_size;
				enum Graphic_glyph_scaling_mode glyph_scaling_mode;
				if (Cmiss_graphic_set_material(graphic, material) &&
					Cmiss_graphic_set_selected_material(graphic, material) &&
					Cmiss_graphic_get_glyph_parameters(
						graphic, &old_glyph, &glyph_scaling_mode,
						glyph_centre, glyph_size, &orientation_scale_field, glyph_scale_factors,
						&variable_scale_field)&&
					Cmiss_graphic_set_glyph_parameters(
						graphic, glyph, glyph_scaling_mode,
						glyph_centre, glyph_size,	orientation_scale_field, glyph_scale_factors,
						variable_scale_field))
				{
					return_code = 1;
				}
				Cmiss_graphic_destroy(&graphic);
			}
			Cmiss_rendition_end_change(rendition);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_rendition_add_glyph.  Graphic with the same name alreadu exists");
		}
	}
	LEAVE;

	return return_code;
}

int Cmiss_rendition_has_selection_group(Cmiss_rendition_id rendition)
{
	int return_code = 0;

	if (rendition)
	{
		if (rendition->selection_group)
		{
			return_code = 1;
		}
		else
		{
			/* this handles the case when selection is shared between "group" */
			Cmiss_field_module_id field_module =
				Cmiss_region_get_field_module(rendition->region);
			if (field_module)
			{
				Computed_field *selection_field = Cmiss_field_module_find_field_by_name(
					field_module, "cmiss_selection");
				if (selection_field)
				{
					rendition->selection_group = selection_field;
					Cmiss_region *parent_region = Cmiss_region_get_parent(rendition->region);
					if (parent_region)
					{
						Cmiss_rendition *parent_rendition =
							Cmiss_region_get_rendition_internal(parent_region);
						Cmiss_field_id parent_group_field = NULL;
						if (parent_rendition)
						{
							parent_group_field =
								Cmiss_rendition_get_selection_group(parent_rendition);
							DEACCESS(Cmiss_rendition)(&parent_rendition);
						}
						if (parent_group_field)
						{
							if (parent_group_field == selection_field)
							{
								Cmiss_field_group_id parent_group =
									Cmiss_field_cast_group(parent_group_field);
								Cmiss_field_destroy(&parent_group_field);
								Cmiss_field_group_add_region(parent_group, rendition->region);
								parent_group_field = reinterpret_cast<Computed_field*>(parent_group);
							}
							Cmiss_field_destroy(&parent_group_field);
						}
						DEACCESS(Cmiss_region)(&parent_region);
					}
					return_code = 1;
				}
				Cmiss_field_module_destroy(&field_module);
			}
		}
	}

	return return_code;
}

Computed_field *Cmiss_rendition_get_selection_group(Cmiss_rendition_id rendition)
{
	Computed_field *sub_group = NULL;

	if (rendition)
	{
		if (rendition->selection_group)
		{
			sub_group = Cmiss_field_access(rendition->selection_group);
		}
		else
		{
			Computed_field *parent_group_field = NULL;
 			Cmiss_region *region = ACCESS(Cmiss_region)(rendition->region);
			Computed_field *selection_field = NULL;
			Cmiss_region *parent_region = Cmiss_region_get_parent(region);
			/* Making sure that parent rendition already has a group field,
			 * this is not important for true region but is important for
			 * group as they multiple renditions may share the same group
			 * field, this would make sure the group field to have the right
			 * hierarchy.
			 */
			if (parent_region)
			{
				Cmiss_rendition *parent_rendition =
					Cmiss_region_get_rendition_internal(parent_region);
				if (parent_rendition)
				{
					parent_group_field =
						Cmiss_rendition_get_selection_group(parent_rendition);
					DEACCESS(Cmiss_rendition)(&parent_rendition);
				}
				DEACCESS(Cmiss_region)(&parent_region);
			}
			Cmiss_field_module_id field_module =
				Cmiss_region_get_field_module(region);
			if (field_module)
			{
				selection_field = Cmiss_field_module_find_field_by_name(
					field_module, "cmiss_selection");
				if (!selection_field)
				{
					selection_field = Cmiss_field_module_create_group(field_module, region);
					Cmiss_field_set_name(selection_field, "cmiss_selection");
					Cmiss_field_set_persistent(selection_field, 0);
				}
				Cmiss_field_module_destroy(&field_module);
			}
			if (selection_field)
			{
				rendition->selection_group = selection_field;
				sub_group = Cmiss_field_access(rendition->selection_group);
				if (parent_group_field)
				{
					Cmiss_field_group_id parent_group =
						Cmiss_field_cast_group(parent_group_field);
					Cmiss_field_destroy(&parent_group_field);
					Cmiss_field_group_add_region(parent_group, region);
					parent_group_field = reinterpret_cast<Computed_field*>(parent_group);
				}
			}
			DEACCESS(Cmiss_region)(&region);
			if (parent_group_field)
			{
				Cmiss_field_destroy(&parent_group_field);
			}
		}
	}
	return (sub_group);
}

int Cmiss_rendition_remove_selection_group(Cmiss_rendition_id rendition)
{
	int return_code = 1;

	if (rendition)
	{
		if (rendition->selection_group)
		{
			Cmiss_field_destroy(&rendition->selection_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_remove_selection_group.  Invalid argument(s)");
		return_code = 0;
	}

	return return_code;
}

int Cmiss_field_node_group_add_node_iterator(Cmiss_node_id node,
	void *node_group_void)
{
	Cmiss_field_node_group_template_id node_group = 
		(Cmiss_field_node_group_template_id)node_group_void;
	if (NULL != node_group)
	{
		Cmiss_field_node_group_template_add_node(node_group, node);
	}
	return 1;
}

int Cmiss_rendition_create_node_list_selection(Cmiss_rendition_id rendition,
	struct LIST(FE_node) *node_list)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
FE_node iterator version of FE_node_selection_select_node.
==============================================================================*/
{
	int return_code = 1;

	ENTER(FE_node_select_in_FE_node_selection);
	if (rendition)
	{
		Cmiss_field_id sub_group_field = 
			Cmiss_rendition_get_selection_group(rendition);
		if (sub_group_field)
		{
			Cmiss_field_group_id sub_group = 
				Cmiss_field_cast_group(sub_group_field);
			if (sub_group)
			{
				Cmiss_field_id node_group_field = 
					Cmiss_field_group_create_node_group(sub_group);
				if (node_group_field)
				{
					Cmiss_field_node_group_template_id node_group = 
						Cmiss_field_cast_node_group_template(node_group_field);
					Cmiss_field_destroy(&node_group_field);
					if (node_group)
					{
						Cmiss_field_node_group_template_clear(node_group);
						FOR_EACH_OBJECT_IN_LIST(FE_node)(
							Cmiss_field_node_group_add_node_iterator,
							(void *)node_group, node_list);
						Computed_field_changed(sub_group_field);
						Cmiss_field_destroy(&sub_group_field);
						Cmiss_field_id temporary_handle = 
							reinterpret_cast<Computed_field *>(node_group);
						Cmiss_field_destroy(&temporary_handle);
					}
				}
				if (NULL != (sub_group_field = 
						reinterpret_cast<Computed_field *>(sub_group)))
				{
					Cmiss_field_destroy(&sub_group_field);
				}
			}
		}
	}
	LEAVE;

	return (return_code);
} /* FE_node_select_in_FE_node_selection */

int Cmiss_rendition_detach_fields(struct Cmiss_rendition *rendition)
{
	int return_code = 1;
	if (rendition)
	{
		if (!rendition->list_of_scene || rendition->list_of_scene->empty())
		{
			Cmiss_rendition_remove_time_dependent_transformation(rendition);
			if (rendition->default_coordinate_field)
			{
				DEACCESS(Computed_field)(&(rendition->default_coordinate_field));
			}
			if (rendition->native_discretization_field)
			{
				DEACCESS(FE_field)(&(rendition->native_discretization_field));
			}
			if (rendition->list_of_graphics)
			{
				FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
					Cmiss_graphic_detach_fields, (void *)NULL,
					rendition->list_of_graphics);
			}
			if (rendition->transformation_field)
			{
				DEACCESS(Computed_field)(&(rendition->transformation_field));
			}
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_detach_fields.  Invalid argument(s)");
		return_code = 0;
	}

	return return_code;
}

Cmiss_rendition *Cmiss_rendition_access(Cmiss_rendition_id rendition)
{
	return (ACCESS(Cmiss_rendition)(rendition));
}

int Cmiss_rendition_execute_command(Cmiss_rendition_id rendition, const char *command_string)
{
	int return_code = 0;
	if (rendition && command_string)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		if (state)
		{
			struct Cmiss_graphics_module *graphics_module =	rendition->graphics_module;
			if(graphics_module)
			{
				struct Option_table *option_table;
				struct Modify_rendition_data modify_rendition_data;
				struct Rendition_command_data rendition_command_data;
				modify_rendition_data.delete_flag = 0;
				modify_rendition_data.position = -1;
				modify_rendition_data.graphic = (struct Cmiss_graphic *)NULL;
				modify_rendition_data.default_coordinate_field = NULL;
				modify_rendition_data.circle_discretization = 4;
				modify_rendition_data.native_discretization_field = NULL;
				modify_rendition_data.element_discretization.number_in_xi1 = 4;
				modify_rendition_data.element_discretization.number_in_xi2 = 4;
				modify_rendition_data.element_discretization.number_in_xi3 = 4;
				int previous_state_index;
				if (state && (previous_state_index = state->current_index))
				{
					option_table = CREATE(Option_table)();
					/* as */
					char *graphic_name = (char *)NULL;
					Option_table_add_name_entry(option_table, "as", &graphic_name);
					/* default to absorb everything else */
					char *dummy_string = (char *)NULL;
					Option_table_add_name_entry(option_table, (char *)NULL, &dummy_string);
					return_code = Option_table_multi_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
					if (return_code)
					{
						if (graphic_name && (modify_rendition_data.graphic = first_graphic_in_Cmiss_rendition_that(
									rendition, Cmiss_graphic_has_name, (void *)graphic_name)))
						{
							ACCESS(Cmiss_graphic)(modify_rendition_data.graphic);
						}
						modify_rendition_data.default_coordinate_field =
							Cmiss_rendition_get_default_coordinate_field(rendition);
						modify_rendition_data.circle_discretization =
							Cmiss_rendition_get_circle_discretization(rendition);
						modify_rendition_data.native_discretization_field =
								Cmiss_rendition_get_native_discretization_field(rendition);
						struct Element_discretization element_discretization;
						Cmiss_rendition_get_element_discretization(rendition,
							&element_discretization);
						modify_rendition_data.element_discretization =
							element_discretization;
					}
					if (dummy_string)
					{
						DEALLOCATE(dummy_string);
					}
					if (graphic_name)
					{
						DEALLOCATE(graphic_name);
					}
					/* Return back to where we were */
					shift_Parse_state(state, previous_state_index - state->current_index);
				}
				struct Material_package *material_package =
					Cmiss_graphics_module_get_material_package(graphics_module);
				rendition_command_data.default_material =
					Material_package_get_default_material(material_package);
				rendition_command_data.graphics_font_package =
					Cmiss_graphics_module_get_font_package(graphics_module);
				rendition_command_data.default_font =
					Cmiss_graphics_module_get_default_font(graphics_module);
				rendition_command_data.glyph_manager =
					Cmiss_graphics_module_get_default_glyph_manager(graphics_module);
				rendition_command_data.computed_field_manager =
					 Cmiss_region_get_Computed_field_manager(rendition->region);
				rendition_command_data.region = rendition->region;
				rendition_command_data.root_region = rendition->region;
				rendition_command_data.graphical_material_manager =
					Cmiss_graphics_module_get_material_manager(graphics_module);
				rendition_command_data.spectrum_manager =
					Cmiss_graphics_module_get_spectrum_manager(graphics_module);
				rendition_command_data.default_spectrum =
						Cmiss_graphics_module_get_default_spectrum(graphics_module);
				rendition_command_data.texture_manager =
					Cmiss_graphics_module_get_texture_manager(graphics_module);

				option_table = CREATE(Option_table)();
				/* cylinders */
				Option_table_add_entry(option_table, "cylinders",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_cylinders);
				/* data_points */
				Option_table_add_entry(option_table, "data_points",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_data_points);
				/* element_points */
				Option_table_add_entry(option_table, "element_points",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_element_points);
				/* general */
				Option_table_add_entry(option_table, "general",
					(void *)rendition->region, (void *)NULL,	gfx_modify_rendition_general);
				/* iso_surfaces */
				Option_table_add_entry(option_table, "iso_surfaces",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_iso_surfaces);
				/* lines */
				Option_table_add_entry(option_table, "lines",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_lines);
				/* node_points */
				Option_table_add_entry(option_table, "node_points",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_node_points);
				/* static_graphic */
				Option_table_add_entry(option_table, "static_graphic",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_static_graphic);
				/* streamlines */
				Option_table_add_entry(option_table, "streamlines",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_streamlines);
				/* surfaces */
				Option_table_add_entry(option_table, "surfaces",
					(void *)&modify_rendition_data, (void *)&rendition_command_data,
					gfx_modify_rendition_surfaces);

				return_code = Option_table_parse(option_table, state);
				if (return_code && (modify_rendition_data.graphic))
				{
					return_code = Cmiss_region_modify_rendition(rendition->region,
						modify_rendition_data.graphic,
						modify_rendition_data.delete_flag,
						modify_rendition_data.position);
				} /* parse error,help */
				DESTROY(Option_table)(&option_table);
				if (modify_rendition_data.graphic)
				{
					DEACCESS(Cmiss_graphic)(&(modify_rendition_data.graphic));
				}
				if (material_package)
				{
					DEACCESS(Material_package)(&material_package);
				}
				if (rendition_command_data.default_font)
				{
					DEACCESS(Graphics_font)(&rendition_command_data.default_font);
				}
				if (rendition_command_data.default_spectrum)
				{
					DEACCESS(Spectrum)(&rendition_command_data.default_spectrum);
				}
			}

			destroy_Parse_state(&state);
		}
	}
	return return_code;
}


/*******************************************************************************
FILE : graphical_element.c

LAST MODIFIED : 25 March 2003

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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/element_group_settings.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct GT_element_group_callback_data
{
	GT_element_group_callback callback;
	void *callback_user_data;
	struct GT_element_group_callback_data *next;
}; /* struct GT_element_group_callback_data */

struct GT_element_group
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Structure for maintaining a graphical rendition of an element group.
==============================================================================*/
{
	/* the [FE] region being drawn */
	struct Cmiss_region *cmiss_region;
	struct FE_region *fe_region;
	/*???RC temporary until data_root_region is removed:
		the data Cmiss_region partner for cmiss_region */
	struct Cmiss_region *data_cmiss_region;
	struct FE_region *data_fe_region;
	/* settings shared by whole rendition */
	/* curve approximation with line segments over elements */
	struct Element_discretization element_discretization;
	/* number of segments used around cylinders */
	int circle_discretization;
	/* default coordinate field for graphics drawn with settings below */
	struct Computed_field *default_coordinate_field;
	/* optional native_discretization for graphics drawn with settings below */
	struct FE_field *native_discretization_field;
	/* list of settings making up the graphical finite element group rendition */
	struct LIST(GT_element_settings) *list_of_settings;
	/* list of objects interested in changes to the GT_element_group */
	struct GT_element_group_callback_data *update_callback_list;
	/* managers for updating graphics in response to global changes */
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;

	/* global stores of selected objects for automatic highlighting */
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;

	/* level of cache in effect */
	int cache;
	/* flag indicating that graphics objects in graphical element need building */
	int build;
	/* display list identifier for the graphical_element */
	GLuint display_list;
	/* flag indicating the status of the display_list */
	int display_list_current;

	/* for accessing objects */
	int access_count;
}; /* struct GT_element_group */

/*
Module functions
----------------
*/

static int GT_element_group_inform_clients(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
If changes have been made to <gt_element_group>, as indicated by its build flag
being set, sends a callback to all registered clients.
==============================================================================*/
{
	int return_code;
	struct GT_element_group_callback_data *callback_data;

	ENTER(GT_element_group_inform_clients);
	if (gt_element_group)
	{
		if (gt_element_group->build)
		{
			callback_data = gt_element_group->update_callback_list;
			while(callback_data)
			{
				(callback_data->callback)(gt_element_group,
					callback_data->callback_user_data);
				callback_data = callback_data->next;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_inform_clients.  Invalid GT_element_group");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_inform_clients */

static int GT_element_group_changed(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
External modules that change a GT_element_group should call this routine so that
objects interested in this GT_element_group will be notified that is has
changed.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_changed);
	if (gt_element_group)
	{
		/* mark graphical_element as needing a build */
		gt_element_group->build = 1;
		/* mark display list as not current */
		gt_element_group->display_list_current = 0;
		/* inform clients only if caching is off */
		if (0 == gt_element_group->cache)
		{
			GT_element_group_inform_clients(gt_element_group);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_changed. Invalid GT_element_group");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_changed */

static int GT_element_group_update_default_coordinate(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If the <gt_element_group> doesn't have a default coordinate yet then it 
tries to find one, first in the element_group, then in the node_group and
finally in the data group.
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field;
	struct FE_field *fe_field;

	ENTER(GT_element_group_changed);
	if (gt_element_group)
	{
		return_code = 1;
		/* if we don't have a computed_field_manager, we are working on an
			 "editor copy" which does not update graphics; the
			 default_coordinate_field will have been supplied by the global object */
		if (gt_element_group->computed_field_manager)
		{
			if (gt_element_group->default_coordinate_field)
			{
				/* Don't second guess, just keep what we have */
			}
			else
			{
				/* Try to find one */
				if (FE_region_get_default_coordinate_FE_field(
					gt_element_group->fe_region, &fe_field) ||
					(gt_element_group->data_fe_region && 
					FE_region_get_default_coordinate_FE_field(
						gt_element_group->data_fe_region, &fe_field)))
				{
					/* Find the computed_field wrapper */
					if (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_read_only_with_fe_field,
						(void *)fe_field, gt_element_group->computed_field_manager))
					{
						gt_element_group->default_coordinate_field = 
							ACCESS(Computed_field)(computed_field);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_changed. Invalid GT_element_group");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_changed */

static int GT_element_group_build_graphics_objects(
	struct GT_element_group *gt_element_group, FE_value time, char *name_prefix)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Builds graphics objects for all settings in <gt_element_group> that either have
none or are flagged "rebuild_graphics".
If either or both of <changed_element_list> and <changed_node_list> are
supplied, settings with graphics objects are updated for just those changed
objects.
Does NOT call GT_element_group_changed as rebuilding is done just-in-time for
rendering.
==============================================================================*/
{
	int return_code;
	struct Element_discretization element_discretization;
	struct GT_element_settings_to_graphics_object_data settings_to_object_data;

	ENTER(GT_element_group_build_graphics_objects);
	if (gt_element_group && name_prefix)
	{
		/* update default coordinate field */
		GT_element_group_update_default_coordinate(gt_element_group);
		
		settings_to_object_data.default_rc_coordinate_field = 
			(struct Computed_field *)NULL;
		if ((!gt_element_group->default_coordinate_field) || 
			(settings_to_object_data.default_rc_coordinate_field=
			Computed_field_begin_wrap_coordinate_field(
			gt_element_group->default_coordinate_field)))
		{
			settings_to_object_data.name_prefix = name_prefix;
			settings_to_object_data.rc_coordinate_field =
				(struct Computed_field *)NULL;
			settings_to_object_data.wrapper_orientation_scale_field =
				(struct Computed_field *)NULL;
			settings_to_object_data.native_discretization_field =
				gt_element_group->native_discretization_field;
			settings_to_object_data.region = gt_element_group->cmiss_region;
			settings_to_object_data.fe_region = gt_element_group->fe_region;
			settings_to_object_data.data_fe_region = gt_element_group->data_fe_region;
			element_discretization.number_in_xi1 =
				gt_element_group->element_discretization.number_in_xi1;
			element_discretization.number_in_xi2 =
				gt_element_group->element_discretization.number_in_xi2;
			element_discretization.number_in_xi3 =
				gt_element_group->element_discretization.number_in_xi3;
			settings_to_object_data.element_discretization = &element_discretization;
			settings_to_object_data.circle_discretization =
				gt_element_group->circle_discretization;
			settings_to_object_data.time = time;

			settings_to_object_data.selected_element_point_ranges_list =
				Element_point_ranges_selection_get_element_point_ranges_list(
					gt_element_group->element_point_ranges_selection);
			settings_to_object_data.selected_element_list =
				FE_element_selection_get_element_list(
					gt_element_group->element_selection);
			if (gt_element_group->data_selection)
			{
				settings_to_object_data.selected_data_list =
					FE_node_selection_get_node_list(gt_element_group->data_selection);
			}
			else
			{
				settings_to_object_data.selected_data_list = (struct LIST(FE_node) *)NULL;
			}
			settings_to_object_data.selected_node_list =
				FE_node_selection_get_node_list(gt_element_group->node_selection);

			return_code = FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_to_graphics_object,(void *)&settings_to_object_data,
				gt_element_group->list_of_settings);

			if (settings_to_object_data.default_rc_coordinate_field)
			{
				Computed_field_end_wrap(
					&settings_to_object_data.default_rc_coordinate_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"GT_element_group_build_graphics_objects.  "
				"Could not get default_rc_coordinate_field wrapper");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_element_group_build_graphics_objects.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_build_graphics_objects */

static void GT_element_group_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Called when <changes> to fields in <fe_region> have occurred. Also lists nodes
and elements in which those fields have changed for a more efficient response.
Sets up to graphics to be appropriately rebuilt before next rendering.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;
	struct GT_element_settings_FE_region_change_data data;

	ENTER(GT_element_group_FE_region_change);
	if (fe_region && changes &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
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
		data.default_coordinate_field =
			gt_element_group->default_coordinate_field;
		data.fe_region = fe_region;
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_FE_region_change, (void *)&data,
			gt_element_group->list_of_settings);
		if (data.graphics_changed)
		{
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_FE_region_change */

static void GT_element_group_data_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Called when <changes> to fields in <fe_region> have occurred. Also lists nodes
and elements in which those fields have changed for a more efficient response.
Sets up to graphics to be appropriately rebuilt before next rendering.
???RC Temporary until data_root_region/data_hack removed.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;
	struct GT_element_settings_FE_region_change_data data;

	ENTER(GT_element_group_data_FE_region_change);
	if (fe_region && changes &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
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
		data.default_coordinate_field =
			gt_element_group->default_coordinate_field;
		data.fe_region = fe_region;
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_data_FE_region_change, (void *)&data,
			gt_element_group->list_of_settings);
		if (data.graphics_changed)
		{
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_data_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_data_FE_region_change */

static void GT_element_group_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
One or more of the computed_fields have changed in the manager.
Updates any graphics affected by the change.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;
	struct GT_element_settings_Computed_field_change_data change_data;

	ENTER(GT_element_group_Computed_field_change);
	if (message &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				/*???RC Is there a better way of getting time to here? */
				change_data.time = 0;
				change_data.graphics_changed = 0;
				change_data.default_coordinate_field =
					gt_element_group->default_coordinate_field;
				change_data.changed_field_list = CREATE(LIST(Computed_field))();
				/* copy the fields that have directly changed */
				COPY_LIST(Computed_field)(change_data.changed_field_list,
					message->changed_object_list);
				/* add any other fields indirectly changed */
				FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
					Computed_field_add_to_list_if_depends_on_list,
					(void *)change_data.changed_field_list,
					gt_element_group->computed_field_manager);

				FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
					GT_element_settings_Computed_field_change, (void *)&change_data,
					gt_element_group->list_of_settings);
				if (change_data.graphics_changed)
				{
					GT_element_group_changed(gt_element_group);
				}
				DESTROY(LIST(Computed_field))(&change_data.changed_field_list);
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_REMOVE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_Computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_Computed_field_change */

static void GT_element_group_element_point_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Callback for change in the global element selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_element_point_ranges_selection_change);
	if (element_point_ranges_selection && changes &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect elements in this group */
		if (FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
			Element_point_ranges_element_is_in_FE_region,
			(void *)gt_element_group->fe_region,
			changes->newly_selected_element_point_ranges_list) ||
			FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
				Element_point_ranges_element_is_in_FE_region,
				(void *)gt_element_group->fe_region,
				changes->newly_unselected_element_point_ranges_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_element_points_change, (void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_element_point_ranges_selection_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_element_point_ranges_selection_change */

static void GT_element_group_element_selection_change(
	struct FE_element_selection *element_selection,
	struct FE_element_selection_changes *changes,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Callback for change in the global element selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_element_selection_change);
	if (element_selection && changes &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect elements in this fe_region */
		if (FIRST_OBJECT_IN_LIST_THAT(FE_element)(
			FE_region_contains_FE_element_conditional,
			(void *)gt_element_group->fe_region,
			changes->newly_selected_element_list) ||
			FIRST_OBJECT_IN_LIST_THAT(FE_element)(
				FE_region_contains_FE_element_conditional,
				(void *)gt_element_group->fe_region,
				changes->newly_unselected_element_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_elements_change, (void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_element_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_element_selection_change */

static void GT_element_group_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_node_selection_change);
	if (node_selection && changes &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect nodes in this fe_region */
		if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(
			FE_region_contains_FE_node_conditional,
			(void *)gt_element_group->fe_region,
			changes->newly_selected_node_list) ||
			FIRST_OBJECT_IN_LIST_THAT(FE_node)(
				FE_region_contains_FE_node_conditional,
				(void *)gt_element_group->fe_region,
				changes->newly_unselected_node_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_nodes_change, (void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_node_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_node_selection_change */

static void GT_element_group_data_selection_change(
	struct FE_node_selection *data_selection,
	struct FE_node_selection_changes *changes,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Callback for change in the global data selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_data_selection_change);
	if (data_selection && changes &&
		(gt_element_group = (struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect nodes in this data_fe_region */
		if (gt_element_group->data_fe_region)
		{
			if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(
				FE_region_contains_FE_node_conditional,
				(void *)gt_element_group->data_fe_region,
				changes->newly_selected_node_list) ||
				FIRST_OBJECT_IN_LIST_THAT(FE_node)(
				FE_region_contains_FE_node_conditional,
				(void *)gt_element_group->data_fe_region,
				changes->newly_unselected_node_list))
			{
				/* update the graphics to match */
				FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
					GT_element_settings_selected_data_change, (void *)NULL,
					gt_element_group->list_of_settings);
				GT_element_group_changed(gt_element_group);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_data_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_data_selection_change */

/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(GT_element_group)

struct GT_element_group *CREATE(GT_element_group)(
	struct Cmiss_region *cmiss_region,
	struct Cmiss_region *data_cmiss_region,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Allocates memory and assigns fields for a graphical finite element group for
the given <cmiss_region> and <data_cmiss_region>. Neither region is accessed;
instead the Scene is required to destroy GT_element_group if either region is
destroyed.
If supplied, callbacks are requested from the <element_selection> and
<node_selection> to enable automatic highlighting of selected graphics.
==============================================================================*/
{
	struct FE_region *data_fe_region, *fe_region;
	struct GT_element_group *gt_element_group;

	ENTER(CREATE(GT_element_group));
	data_fe_region = (struct FE_region *)NULL;
	if (cmiss_region &&
		(fe_region = Cmiss_region_get_FE_region(cmiss_region)) && 
		(!data_cmiss_region || 
		(data_fe_region = Cmiss_region_get_FE_region(data_cmiss_region))))
	{
		if (ALLOCATE(gt_element_group, struct GT_element_group, 1))
		{
			if (gt_element_group->list_of_settings =
				CREATE(LIST(GT_element_settings))())
			{
				gt_element_group->cmiss_region = ACCESS(Cmiss_region)(cmiss_region);
				gt_element_group->fe_region = ACCESS(FE_region)(fe_region);
				if (data_cmiss_region)
				{
					gt_element_group->data_cmiss_region =
						ACCESS(Cmiss_region)(data_cmiss_region);
					gt_element_group->data_fe_region = ACCESS(FE_region)(data_fe_region);
				}
				else
				{
					gt_element_group->data_cmiss_region = (struct Cmiss_region *)NULL;
					gt_element_group->data_fe_region = (struct FE_region *)NULL;
				}
				/* set settings shared by whole rendition */
				gt_element_group->element_discretization.number_in_xi1=2;
				gt_element_group->element_discretization.number_in_xi2=2;
				gt_element_group->element_discretization.number_in_xi3=2;
				gt_element_group->circle_discretization=3;
				gt_element_group->default_coordinate_field=
					(struct Computed_field *)NULL;
				gt_element_group->native_discretization_field=(struct FE_field *)NULL;
				gt_element_group->update_callback_list=
					(struct GT_element_group_callback_data *)NULL;
				/* managers and callback ids */
				gt_element_group->computed_field_manager=computed_field_manager;
				gt_element_group->computed_field_manager_callback_id=(void *)NULL;
				/* request callbacks from FE_regions */
				FE_region_add_callback(fe_region,
					GT_element_group_FE_region_change, (void *)gt_element_group);
				if (data_fe_region)
				{
					FE_region_add_callback(data_fe_region,
						GT_element_group_data_FE_region_change, (void *)gt_element_group);
				}
				/* request callbacks from any managers supplied */
				if (computed_field_manager)
				{
					gt_element_group->computed_field_manager_callback_id=
						MANAGER_REGISTER(Computed_field)(
							GT_element_group_Computed_field_change,(void *)gt_element_group,
							gt_element_group->computed_field_manager);
				}

				/* global selections */
				gt_element_group->element_point_ranges_selection=
					element_point_ranges_selection;
				gt_element_group->element_selection=element_selection;
				gt_element_group->node_selection=node_selection;
				gt_element_group->data_selection=data_selection;
				gt_element_group->cache = 0;
				gt_element_group->build = 1;
				/* display list index and current flag: */
				gt_element_group->display_list = 0;
				gt_element_group->display_list_current = 0;

				gt_element_group->access_count = 0;
				/* request callbacks from the global selections */
				if (element_point_ranges_selection)
				{
					Element_point_ranges_selection_add_callback(
						element_point_ranges_selection,
						GT_element_group_element_point_ranges_selection_change,
						(void *)gt_element_group);
				}
				if (element_selection)
				{
					FE_element_selection_add_callback(element_selection,
						GT_element_group_element_selection_change,(void *)gt_element_group);
				}
				if (node_selection)
				{
					FE_node_selection_add_callback(node_selection,
						GT_element_group_node_selection_change,(void *)gt_element_group);
				}
				if (data_selection)
				{
					FE_node_selection_add_callback(data_selection,
						GT_element_group_data_selection_change,(void *)gt_element_group);
				}
				GT_element_group_update_default_coordinate(gt_element_group);
			}
			else
			{
				DESTROY(LIST(GT_element_settings))(
					&(gt_element_group->list_of_settings));
				DEALLOCATE(gt_element_group);
				gt_element_group = (struct GT_element_group *)NULL;
			}
		}
		else
		{
			DEALLOCATE(gt_element_group);
			gt_element_group = (struct GT_element_group *)NULL;
		}
		if (!gt_element_group)
		{
			display_message(ERROR_MESSAGE,
				"CREATE(GT_element_group).  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(GT_element_group).  Invalid argument(s)");
		gt_element_group=(struct GT_element_group *)NULL;
	}
	LEAVE;

	return (gt_element_group);
} /* CREATE(GT_element_group) */

struct GT_element_group *create_editor_copy_GT_element_group(
	struct GT_element_group *existing_gt_element_group)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Creates a GT_element_group that is a copy of <existing_gt_element_group> -
WITHOUT copying graphics objects, and WITHOUT manager and selection callbacks.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(create_editor_copy_GT_element_group);
	if (existing_gt_element_group)
	{
		/* make an empty GT_element_group for the same groups */
		if (gt_element_group = CREATE(GT_element_group)(
			existing_gt_element_group->cmiss_region,
			existing_gt_element_group->data_cmiss_region,
			(struct MANAGER(Computed_field) *)NULL,
			(struct Element_point_ranges_selection *)NULL,
			(struct FE_element_selection *)NULL,
			(struct FE_node_selection *)NULL,
			(struct FE_node_selection *)NULL))
		{
			/* copy settings WITHOUT graphics objects; do not cause whole function
				 to fail if copy fails */
			GT_element_group_copy(gt_element_group,existing_gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_editor_copy_GT_element_group.  Invalid argument(s)");
		gt_element_group=(struct GT_element_group *)NULL;
	}
	LEAVE;

	return (gt_element_group);
} /* create_editor_copy_GT_element_group */

int DESTROY(GT_element_group)(
	struct GT_element_group **gt_element_group_address)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Frees the memory for <**gt_element_group> and sets <*gt_element_group> to NULL.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_group_callback_data *callback_data, *next;

	ENTER(DESTROY(GT_element_group));
	if (gt_element_group_address)
	{
		return_code=1;
		if (gt_element_group = *gt_element_group_address)
		{
			if (0 != gt_element_group->access_count)
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(GT_element_group).  Access count = %d",
					gt_element_group->access_count);
				return_code=0;
			}
			else
			{
				/* cache should be back to zero by now */
				if (0 != gt_element_group->cache)
				{
					display_message(ERROR_MESSAGE,
						"DESTROY(Gt_element_group).  gt_element_group->cache = %d != 0",
						gt_element_group->cache);
				}
				/* remove callbacks from FE_regions */
				FE_region_remove_callback(gt_element_group->fe_region,
					GT_element_group_FE_region_change, (void *)gt_element_group);
				if (gt_element_group->data_fe_region)
				{
					FE_region_remove_callback(gt_element_group->data_fe_region,
						GT_element_group_data_FE_region_change, (void *)gt_element_group);
				}
				/* turn off manager messages */
				if (gt_element_group->computed_field_manager_callback_id)
				{
					MANAGER_DEREGISTER(Computed_field)(
						gt_element_group->computed_field_manager_callback_id,
						gt_element_group->computed_field_manager);
				}
				/* remove callbacks from the global selections */
				if (gt_element_group->element_point_ranges_selection)
				{
					Element_point_ranges_selection_remove_callback(
						gt_element_group->element_point_ranges_selection,
						GT_element_group_element_point_ranges_selection_change,
						(void *)gt_element_group);
				}
				if (gt_element_group->element_selection)
				{
					FE_element_selection_remove_callback(
						gt_element_group->element_selection,
						GT_element_group_element_selection_change,(void *)gt_element_group);
				}
				if (gt_element_group->data_selection)
				{
					FE_node_selection_remove_callback(gt_element_group->data_selection,
						GT_element_group_data_selection_change,(void *)gt_element_group);
				}
				if (gt_element_group->node_selection)
				{
					FE_node_selection_remove_callback(gt_element_group->node_selection,
						GT_element_group_node_selection_change,(void *)gt_element_group);
				}
				DESTROY(LIST(GT_element_settings))(
					&(gt_element_group->list_of_settings));
				if (gt_element_group->default_coordinate_field)
				{
					DEACCESS(Computed_field)(
						&(gt_element_group->default_coordinate_field));
				}
				if (gt_element_group->native_discretization_field)
				{
					DEACCESS(FE_field)(
						&(gt_element_group->native_discretization_field));
				}
				callback_data = gt_element_group->update_callback_list;
				while(callback_data)
				{
					next = callback_data->next;
					DEALLOCATE(callback_data);
					callback_data = next;
				}
				/* must destroy the display list */
				if (gt_element_group->display_list)
				{
					glDeleteLists(gt_element_group->display_list,1);
				}
				DEACCESS(Cmiss_region)(&(gt_element_group->cmiss_region));
				DEACCESS(FE_region)(&(gt_element_group->fe_region));
				if (gt_element_group->data_cmiss_region)
				{
					DEACCESS(Cmiss_region)(&(gt_element_group->data_cmiss_region));
				}
				if (gt_element_group->data_fe_region)
				{
					DEACCESS(FE_region)(&(gt_element_group->data_fe_region));
				}

				DEALLOCATE(*gt_element_group_address);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(GT_element_group).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_element_group) */

int GT_element_groups_match(struct GT_element_group *gt_element_group1,
	struct GT_element_group *gt_element_group2)
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Returns true if <gt_element_group1> and <gt_element_group2> match in
main attributes of element group, settings etc. such that they would produce
the same graphics.
==============================================================================*/
{
	int i, number_of_settings, return_code;
	struct GT_element_settings *settings1, *settings2;

	ENTER(GT_element_groups_match);
	if (gt_element_group1 && gt_element_group2)
	{
		if ((gt_element_group1->fe_region ==
			gt_element_group2->fe_region) &&
			(gt_element_group1->data_fe_region ==
				gt_element_group2->data_fe_region) &&
			(gt_element_group1->element_discretization.number_in_xi1 ==
				gt_element_group2->element_discretization.number_in_xi1) &&
			(gt_element_group1->element_discretization.number_in_xi2 ==
				gt_element_group2->element_discretization.number_in_xi2) &&
			(gt_element_group1->element_discretization.number_in_xi3 ==
				gt_element_group2->element_discretization.number_in_xi3) &&
			(gt_element_group1->circle_discretization ==
				gt_element_group2->circle_discretization) &&
			(gt_element_group1->default_coordinate_field ==
				gt_element_group2->default_coordinate_field) &&
			(gt_element_group1->native_discretization_field ==
				gt_element_group2->native_discretization_field) &&
			((number_of_settings = NUMBER_IN_LIST(GT_element_settings)(
				gt_element_group1->list_of_settings)) ==
				NUMBER_IN_LIST(GT_element_settings)(
					gt_element_group2->list_of_settings)))
		{
			return_code = 1;
			for (i = 1; return_code && (i <= number_of_settings); i++)
			{
				settings1 = FIND_BY_IDENTIFIER_IN_LIST(GT_element_settings, position)(
					i, gt_element_group1->list_of_settings);
				settings2 = FIND_BY_IDENTIFIER_IN_LIST(GT_element_settings, position)(
					i, gt_element_group2->list_of_settings);
				return_code = GT_element_settings_match(settings1, settings2);
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
			"GT_element_groups_match.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_groups_match */

int GT_element_group_begin_cache(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Call before making several changes to the gt_element_group so only a single
change message is sent. Call GT_element_group_end_cache at the end of the
changes.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_begin_cache);
	if (gt_element_group)
	{
		/* increment cache to allow nesting */
		(gt_element_group->cache)++;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_begin_cache.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_begin_cache */

int GT_element_group_end_cache(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Call after making changes preceded by a call to GT_element_group_begin_cache to
enable a final message to be sent to clients.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_end_cache);
	if (gt_element_group)
	{
		/* decrement cache to allow nesting */
		(gt_element_group->cache)--;
		/* once cache has run out, inform clients of any changes */
		if (0 == gt_element_group->cache)
		{
			GT_element_group_inform_clients(gt_element_group);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_end_cache.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_end_cache */

int GT_element_group_time_changed(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Invalidate any components of a GT_element group that depend on time
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_time_changed);

	if (gt_element_group)
	{
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_time_change,NULL,
			gt_element_group->list_of_settings);		
		gt_element_group->build = 1;
		gt_element_group->display_list_current = 0;
		/* GT_element_group_changed(gt_element_group); */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_time_changed. Invalid GT_element_group");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_time_changed */

int GT_element_group_add_callback(struct GT_element_group *gt_element_group, 
	GT_element_group_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Adds a callback routine which is called whenever a GT_element_group is aware of
changes.
==============================================================================*/
{
	int return_code;
	struct GT_element_group_callback_data *callback_data, *previous;

	ENTER(GT_element_group_add_callback);

	if (gt_element_group && callback)
	{
		if(ALLOCATE(callback_data, struct GT_element_group_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct GT_element_group_callback_data *)NULL;
			if(gt_element_group->update_callback_list)
			{
				previous = gt_element_group->update_callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				gt_element_group->update_callback_list = callback_data;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_element_group_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_add_callback.  Missing gt_element_group object or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_add_callback */

int GT_element_group_remove_callback(struct GT_element_group *gt_element_group,
	GT_element_group_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/
{
	int return_code;
	struct GT_element_group_callback_data *callback_data, *previous;

	ENTER(GT_element_group_remove_callback);

	if (gt_element_group && callback && gt_element_group->update_callback_list)
	{
		callback_data = gt_element_group->update_callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			gt_element_group->update_callback_list = callback_data->next;
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
"GT_element_group_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"GT_element_group_remove_callback.  Missing GT_element_group, callback or callback list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_remove_callback */

int GT_element_group_add_settings(struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings,int position)
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Adds the <settings> to <gt_element_group> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_add_settings);
	if (gt_element_group&&settings)
	{
		return_code=GT_element_settings_add_to_list(settings,position,
			gt_element_group->list_of_settings);
		GT_element_group_changed(gt_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_add_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_add_settings */

int GT_element_group_remove_settings(struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Removes the <settings> from <gt_element_group> and decrements the position
of all subsequent settings.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_remove_settings);
	if (gt_element_group&&settings)
	{
		return_code=GT_element_settings_remove_from_list(settings,
			gt_element_group->list_of_settings);
		GT_element_group_changed(gt_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_remove_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_remove_settings */

int GT_element_group_modify_settings(struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings,struct GT_element_settings *new_settings)
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Changes the contents of <settings> to match <new_settings>, with no change in
position in <gt_element_group>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_modify_settings);
	if (gt_element_group&&settings&&new_settings)
	{
		return_code=GT_element_settings_modify_in_list(settings,new_settings,
			gt_element_group->list_of_settings);
		GT_element_group_changed(gt_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_modify_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_modify_settings */

int GT_element_group_get_settings_position(
	struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 13 November 2001

DESCRIPTION :
Returns the position of <settings> in <gt_element_group>.
==============================================================================*/
{
	int position;

	ENTER(GT_element_group_get_settings_position);
	if (gt_element_group&&settings)
	{
		position=GT_element_settings_get_position_in_list(settings,
			gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_settings_position.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* GT_element_group_get_settings_position */

int GT_element_group_get_number_of_settings(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 13 November 2001

DESCRIPTION :
Returns the number of settings in <gt_element_group>.
==============================================================================*/
{
	int number_of_settings;

	ENTER(GT_element_group_get_number_of_settings);
	if (gt_element_group)
	{
		number_of_settings =
			NUMBER_IN_LIST(GT_element_settings)(gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_number_of_settings.  Invalid argument(s)");
		number_of_settings = 0;
	}
	LEAVE;

	return (number_of_settings);
} /* GT_element_group_get_number_of_settings */

int GT_element_group_get_circle_discretization(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the circle discretization of the gt_element_group.
==============================================================================*/
{
	int circle_discretization;

	ENTER(GT_element_group_get_circle_discretization);
	if (gt_element_group)
	{
		circle_discretization=gt_element_group->circle_discretization;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_circle_discretization.  Invalid argument(s)");
		circle_discretization=0;
	}
	LEAVE;

	return (circle_discretization);
} /* GT_element_group_get_circle_discretization */

int GT_element_group_set_circle_discretization(
	struct GT_element_group *gt_element_group,int circle_discretization)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Sets the circle discretization of the gt_element_group.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_circle_discretization);
	if (gt_element_group)
	{
		return_code = 1;
		if (circle_discretization != gt_element_group->circle_discretization)
		{
			gt_element_group->circle_discretization = circle_discretization;
			/* clear graphics for all settings using circle discretization */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_circle_discretization_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_circle_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_set_circle_discretization */

struct Computed_field *GT_element_group_get_default_coordinate_field(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 16 February 1999

DESCRIPTION :
Returns the default coordinate field of the <gt_element_group>.
==============================================================================*/
{
	struct Computed_field *default_coordinate_field;

	ENTER(GT_element_group_get_default_coordinate_field);
	if (gt_element_group)
	{
		if (!gt_element_group->default_coordinate_field)
		{
			/* Try and get one now before returning nothing */
			GT_element_group_update_default_coordinate(gt_element_group);
		}
		default_coordinate_field=gt_element_group->default_coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_default_coordinate_field.  Invalid argument(s)");
		default_coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (default_coordinate_field);
} /* GT_element_group_get_default_coordinate_field */

int GT_element_group_set_default_coordinate_field(
	struct GT_element_group *gt_element_group,
	struct Computed_field *default_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Sets the <default_coordinate_field> used by <gt_element_group>. Settings without
a specific coordinate field will use this one.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_default_coordinate_field);
	if (gt_element_group && default_coordinate_field &&
		(3 >= Computed_field_get_number_of_components(default_coordinate_field)))
	{
		return_code = 1;
		if (default_coordinate_field != gt_element_group->default_coordinate_field)
		{
			REACCESS(Computed_field)(&(gt_element_group->default_coordinate_field),
				default_coordinate_field);
			/* clear graphics for all settings using default coordinate field */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_default_coordinate_field_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_default_coordinate_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_set_default_coordinate_field */

int GT_element_group_get_element_discretization(
	struct GT_element_group *gt_element_group,
	struct Element_discretization *element_discretization)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the element discretization of the gt_element_group.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_get_element_discretization);
	if (gt_element_group&&element_discretization)
	{
		element_discretization->number_in_xi1=
			gt_element_group->element_discretization.number_in_xi1;
		element_discretization->number_in_xi2=
			gt_element_group->element_discretization.number_in_xi2;
		element_discretization->number_in_xi3=
			gt_element_group->element_discretization.number_in_xi3;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_element_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_get_element_discretization */

int GT_element_group_set_element_discretization(
	struct GT_element_group *gt_element_group,
	struct Element_discretization *element_discretization)
/*******************************************************************************
LAST MODIFIED : 28 September 2005

DESCRIPTION :
Sets the element discretization of the gt_element_group.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_element_discretization);
	if (gt_element_group && element_discretization)
	{
		return_code = 1;
		if ((gt_element_group->element_discretization.number_in_xi1 !=
			element_discretization->number_in_xi1) ||
			(gt_element_group->element_discretization.number_in_xi2 !=
				element_discretization->number_in_xi2) ||
			(gt_element_group->element_discretization.number_in_xi3 !=
				element_discretization->number_in_xi3))
		{
			gt_element_group->element_discretization.number_in_xi1=
				element_discretization->number_in_xi1;
			gt_element_group->element_discretization.number_in_xi2=
				element_discretization->number_in_xi2;
			gt_element_group->element_discretization.number_in_xi3=
				element_discretization->number_in_xi3;
			/* clear graphics for all settings using element discretization */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_element_discretization_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_element_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_set_element_discretization */

struct FE_field *GT_element_group_get_native_discretization_field(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the default coordinate field of the <gt_element_group>.
==============================================================================*/
{
	struct FE_field *native_discretization_field;

	ENTER(GT_element_group_get_native_discretization_field);
	if (gt_element_group)
	{
		native_discretization_field=gt_element_group->native_discretization_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_native_discretization_field.  Invalid argument(s)");
		native_discretization_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (native_discretization_field);
} /* GT_element_group_get_native_discretization_field */

int GT_element_group_set_native_discretization_field(
	struct GT_element_group *gt_element_group,
	struct FE_field *native_discretization_field)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Sets the <native_discretization_field> used by <gt_element_group>. If the field
is not NULL and is element-based in a given element, its native discretization
is used in preference to the global element_discretization.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_native_discretization_field);
	if (gt_element_group)
	{
		return_code = 1;
		if (native_discretization_field !=
			gt_element_group->native_discretization_field)
		{
			REACCESS(FE_field)(&(gt_element_group->native_discretization_field),
				native_discretization_field);
			/* clear graphics for all settings using element discretization */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_element_discretization_change, (void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_native_discretization_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_set_native_discretization_field */

int GT_element_group_copy(struct GT_element_group *destination,
	struct GT_element_group *source)
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Copies the GT_element_group contents from source to destination.
Pointers to graphics_objects are cleared in the destination list of settings.
NOTES:
- not a full copy; does not copy groups, selection etc. Use copy_create for
this task so that callbacks can be set up for these.
- does not copy graphics objects to settings in destination.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_copy);
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
		REMOVE_ALL_OBJECTS_FROM_LIST(GT_element_settings)(
			destination->list_of_settings);
		/* put copy of each settings in source list in destination list */
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_copy_and_put_in_list,
			(void *)destination->list_of_settings,source->list_of_settings);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_copy.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_copy */

int GT_element_group_modify(struct GT_element_group *destination,
	struct GT_element_group *source)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Copies the GT_element_group contents from source to destination, keeping any
graphics objects from the destination that will not change with the new settings
from source. Used to apply the changed GT_element_group from the editor to the
actual GT_element_group.
==============================================================================*/
{
	int return_code;
	struct LIST(GT_element_settings) *temp_list_of_settings;

	ENTER(GT_element_group_modify);
	if (destination && source)
	{
		if (temp_list_of_settings = CREATE(LIST(GT_element_settings))())
		{
			GT_element_group_set_default_coordinate_field(destination,
				source->default_coordinate_field);
			GT_element_group_set_circle_discretization(destination,
				source->circle_discretization);
			GT_element_group_set_element_discretization(destination,
				&(source->element_discretization));
			GT_element_group_set_native_discretization_field(destination,
				source->native_discretization_field);
			/* make copy of source settings without graphics objects */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_copy_and_put_in_list,
				(void *)temp_list_of_settings, source->list_of_settings);
			/* extract graphics objects that can be reused from destination list */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_extract_graphics_object_from_list,
				(void *)destination->list_of_settings, temp_list_of_settings);
			/* replace the destination list of settings with temp_list_of_settings */
			DESTROY(LIST(GT_element_settings))(&(destination->list_of_settings));
			destination->list_of_settings = temp_list_of_settings;
			/* inform the client of the change */
			GT_element_group_changed(destination);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_element_group_modify.  Could not create temporary list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_modify.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_modify */

struct Cmiss_region *GT_element_group_get_Cmiss_region(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the Cmiss_region used by <gt_element_group>.
==============================================================================*/
{
	struct Cmiss_region *cmiss_region;

	ENTER(GT_element_group_get_Cmiss_region);
	if (gt_element_group)
	{
		cmiss_region = gt_element_group->cmiss_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_Cmiss_region.  Invalid argument(s)");
		cmiss_region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (cmiss_region);
} /* GT_element_group_get_Cmiss_region */

struct Cmiss_region *GT_element_group_get_data_Cmiss_region(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the data_Cmiss_region used by <gt_element_group>.
==============================================================================*/
{
	struct Cmiss_region *data_cmiss_region;

	ENTER(GT_element_group_get_data_Cmiss_region);
	if (gt_element_group)
	{
		data_cmiss_region = gt_element_group->data_cmiss_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_data_Cmiss_region.  Invalid argument(s)");
		data_cmiss_region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (data_cmiss_region);
} /* GT_element_group_get_data_Cmiss_region */

int GT_element_group_list_commands(struct GT_element_group *gt_element_group,
	char *command_prefix,char *command_suffix)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Lists the general settings and graphics defined for <gt_element_group> - as a
set of commands that can be used to reproduce the groups appearance. The
<command_prefix> should generally contain "gfx modify g_element" while the
optional <command_suffix> may describe the scene (eg. "scene default").
Note the command prefix is expected to contain the name of the region.
==============================================================================*/
{
	char *name;
	int return_code;
	struct GT_element_settings_list_data list_data;

	ENTER(GT_element_group_list_commands);
	if (gt_element_group && command_prefix)
	{
		display_message(INFORMATION_MESSAGE,command_prefix);
		display_message(INFORMATION_MESSAGE,"general clear");
		display_message(INFORMATION_MESSAGE," circle_discretization %d",
			gt_element_group->circle_discretization);
		if (gt_element_group->default_coordinate_field)
		{
			if (GET_NAME(Computed_field)(gt_element_group->default_coordinate_field,
				&name))
			{
				make_valid_token(&name);
				display_message(INFORMATION_MESSAGE," default_coordinate %s",name);
				DEALLOCATE(name);
			}
		}
		display_message(INFORMATION_MESSAGE,
			" element_discretization \"%d*%d*%d\"",
			gt_element_group->element_discretization.number_in_xi1,
			gt_element_group->element_discretization.number_in_xi2,
			gt_element_group->element_discretization.number_in_xi3);
		display_message(INFORMATION_MESSAGE," native_discretization ");
		if (gt_element_group->native_discretization_field)
		{
			if (GET_NAME(FE_field)(gt_element_group->native_discretization_field,
				&name))
			{
				make_valid_token(&name);
				display_message(INFORMATION_MESSAGE,name);
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"none");
		}
		if (command_suffix)
		{
			display_message(INFORMATION_MESSAGE,command_suffix);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		list_data.settings_string_detail=SETTINGS_STRING_COMPLETE;
		list_data.line_prefix=command_prefix;
		list_data.line_suffix=command_suffix;
		return_code=FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_list_contents,(void *)&list_data,
			gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_list_commands.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_list_commands */

int GT_element_group_list_contents(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Lists the general settings and graphics defined for <gt_element_group>.
==============================================================================*/
{
	char *name;
	int return_code;
	struct GT_element_settings_list_data list_data;

	ENTER(GT_element_group_list_contents);
	if (gt_element_group)
	{
		display_message(INFORMATION_MESSAGE,"  circle discretization: %d\n",
			gt_element_group->circle_discretization);
		if (gt_element_group->default_coordinate_field)
		{
			if (GET_NAME(Computed_field)(gt_element_group->default_coordinate_field,
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
				"  default coordinate field: NONE\n",name);
		}
		display_message(INFORMATION_MESSAGE,"  element discretization: %d*%d*%d\n",
			gt_element_group->element_discretization.number_in_xi1,
			gt_element_group->element_discretization.number_in_xi2,
			gt_element_group->element_discretization.number_in_xi3);
		display_message(INFORMATION_MESSAGE,"  native discretization field: ");
		if (gt_element_group->native_discretization_field)
		{
			if (GET_NAME(FE_field)(gt_element_group->native_discretization_field,
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
		if (0 < NUMBER_IN_LIST(GT_element_settings)(
			gt_element_group->list_of_settings))
		{
			display_message(INFORMATION_MESSAGE,"  graphics objects defined:\n");
			list_data.settings_string_detail=SETTINGS_STRING_COMPLETE_PLUS;
			list_data.line_prefix="  ";
			list_data.line_suffix="";
			return_code=FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_list_contents,(void *)&list_data,
				gt_element_group->list_of_settings);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no graphics settings defined\n");
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_list_contents.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_list_contents */

struct GT_element_settings *get_settings_at_position_in_GT_element_group(
	struct GT_element_group *gt_element_group,int position)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Wrapper for accessing the list of settings in <gt_element_group>.
==============================================================================*/
{
	struct GT_element_settings *settings;

	ENTER(get_settings_at_position_in_GT_element_group);
	if (gt_element_group)
	{
		settings=FIND_BY_IDENTIFIER_IN_LIST(GT_element_settings,
			position)(position,gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_settings_at_position_in_GT_element_group.  Invalid arguments");
		settings=(struct GT_element_settings *)NULL;
	}
	LEAVE;

	return (settings);
} /* get_settings_at_position_in_GT_element_group */

struct GT_element_settings *first_settings_in_GT_element_group_that(
	struct GT_element_group *gt_element_group,
	LIST_CONDITIONAL_FUNCTION(GT_element_settings) *conditional_function,
	void *data)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Wrapper for accessing the list of settings in <gt_element_group>.
==============================================================================*/
{
	struct GT_element_settings *settings;

	ENTER(first_settings_in_GT_element_group_that);
	if (gt_element_group)
	{
		settings=FIRST_OBJECT_IN_LIST_THAT(GT_element_settings)(
			conditional_function,data,gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"first_settings_in_GT_element_group_that.  Invalid arguments");
		settings=(struct GT_element_settings *)NULL;
	}
	LEAVE;

	return (settings);
} /* first_settings_in_GT_element_group_that */

int for_each_settings_in_GT_element_group(
	struct GT_element_group *gt_element_group,
	LIST_ITERATOR_FUNCTION(GT_element_settings) *iterator_function,void *data)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Wrapper for accessing the list of settings in <gt_element_group>.
==============================================================================*/
{
	int return_code;

	ENTER(for_each_settings_in_GT_element_group);
	if (gt_element_group&&iterator_function)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			iterator_function,data,gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_settings_in_GT_element_group.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_each_settings_in_GT_element_group */

int GT_element_group_has_multiple_times(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 6 December 2001

DESCRIPTION :
Returns true if <gt_element_group> contains settings which depend on time.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_update_time_behaviour_data data;

	ENTER(GT_element_group_has_multiple_times);
	if (gt_element_group)
	{
		data.default_coordinate_depends_on_time = 0;
		data.time_dependent = 0;
		if (!gt_element_group->default_coordinate_field)
		{
			GT_element_group_update_default_coordinate(gt_element_group);
		}
		if (gt_element_group->default_coordinate_field)
		{
			if (Computed_field_has_multiple_times(
				gt_element_group->default_coordinate_field))
			{
				data.default_coordinate_depends_on_time = 1;
			}
		}
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_update_time_behaviour, (void *)&data,
			gt_element_group->list_of_settings);
		return_code = data.time_dependent;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_has_multiple_times.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_has_multiple_times */

int build_GT_element_group(struct GT_element_group *gt_element_group,
	FE_value time, char *name_prefix)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Builds any graphics objects for settings without them in <gt_element_group>.
The <name_prefix> is used for the start of all names of graphics generated for
the group.
==============================================================================*/
{
	int return_code;

	ENTER(build_GT_element_group);
	if (gt_element_group && name_prefix)
	{
		/* check whether graphical_element contents need building */
		if (gt_element_group->build)
		{
			return_code = GT_element_group_build_graphics_objects(gt_element_group,
				time, name_prefix);
			gt_element_group->build = 0;
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"build_GT_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* build_GT_element_group  */

int compile_GT_element_group(struct GT_element_group *gt_element_group,
	struct GT_object_compile_context *context)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Rebuilds the display list for graphical_element_group. 
The object is compiled at the <time>.  The <graphics_buffer> is used to 
provide a operating system dependent rendering contexts.
==============================================================================*/
{
	int return_code;

	ENTER(compile_GT_element_group);
	if (gt_element_group)
	{
		return_code = 1;
		if (!gt_element_group->display_list_current)
		{
			/* compile visible graphics objects in the graphical element */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_compile_visible_settings, (void *)context,
				gt_element_group->list_of_settings);
			if (gt_element_group->display_list ||
				(gt_element_group->display_list = glGenLists(1)))
			{
				glNewList(gt_element_group->display_list, GL_COMPILE);
				/* push initial name to make space for subobject names */
				glPushName(0);
				FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
					GT_element_settings_execute_visible_settings,
					NULL, gt_element_group->list_of_settings);
				glPopName();
				glEndList();
				gt_element_group->display_list_current = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE, "compile_GT_element_group.  Failed");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "compile_GT_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* compile_GT_element_group */

int execute_GT_element_group(struct GT_element_group *gt_element_group,
	float time)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(execute_GT_element_group);
	USE_PARAMETER(time);
	if (gt_element_group)
	{
		if (gt_element_group->display_list_current)
		{
			glCallList(gt_element_group->display_list);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_GT_element_group.  display list not current");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_GT_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_GT_element_group */

int GT_element_group_Graphical_material_change(
	struct GT_element_group *gt_element_group,
	struct LIST(Graphical_material) *changed_material_list)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
NOTE: TEMPORARY UNTIL GRAPHICAL ELEMENTS RECEIVE THEIR OWN MATERIAL MESSAGES.
If any of the settings in <gt_element_group> use materials with spectrums in the
<changed_material_list>, clear their graphics objects and call
GT_element_group_changed.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_Graphical_material_change_data
		material_change_data;

	ENTER(GT_element_group_Graphical_material_change);
	if (gt_element_group && changed_material_list)
	{
		material_change_data.changed_material_list = changed_material_list;
		/* flag to indicate if GT_element_group_changed needs to be called */
		material_change_data.changed = 0;
		return_code = FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_Graphical_material_change,
			(void *)&material_change_data, gt_element_group->list_of_settings);
		if (material_change_data.changed)
		{
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_Graphical_material_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_Graphical_material_change */

int GT_element_group_Spectrum_change(struct GT_element_group *gt_element_group,
	struct LIST(Spectrum) *changed_spectrum_list)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
NOTE: TEMPORARY UNTIL GRAPHICAL ELEMENTS RECEIVE THEIR OWN SPECTRUM MESSAGES.
If any of the settings in <gt_element_group> use spectrums in the
<changed_spectrum_list>, clear their graphics objects and call
GT_element_group_changed.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_Spectrum_change_data spectrum_change_data;

	ENTER(GT_element_group_Spectrum_change);
	if (gt_element_group && changed_spectrum_list)
	{
		spectrum_change_data.changed_spectrum_list = changed_spectrum_list;
		/* flag to indicate if GT_element_group_changed needs to be called */
		spectrum_change_data.changed = 0;
		return_code = FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_Spectrum_change, (void *)&spectrum_change_data,
			gt_element_group->list_of_settings);
		if (spectrum_change_data.changed)
		{
			GT_element_group_changed(gt_element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_Spectrum_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_Spectrum_change */

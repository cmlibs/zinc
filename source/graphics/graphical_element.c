/*******************************************************************************
FILE : graphical_element.c

LAST MODIFIED : 6 June 2000

DESCRIPTION :
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/element_group_settings.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/makegtobj.h"
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
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Structure for maintaining a graphical rendition of an element group.
==============================================================================*/
{
	/* the element group being drawn */
	struct GROUP(FE_element) *element_group;
	/* the node group - of the same name as the element group - assumed to contain
		 at least the nodes referenced by the elements in the element group */
	struct GROUP(FE_node) *node_group;
	/* the data group - of the same name as the element group */
	struct GROUP(FE_node) *data_group;
	/* settings shared by whole rendition */
	/* curve approximation with line segments over elements */
	struct Element_discretization element_discretization;
	/* number of segments used around cylinders and later spheres */
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
	struct MANAGER(FE_element) *element_manager;
	void *element_manager_callback_id;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	void *element_group_manager_callback_id;
	struct MANAGER(FE_node) *node_manager;
	void *node_manager_callback_id;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	void *node_group_manager_callback_id;
	struct MANAGER(FE_node) *data_manager;
	void *data_manager_callback_id;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	void *data_group_manager_callback_id;
	struct Computed_field_package *computed_field_package;
	void *computed_field_manager_callback_id;

	/* global stores of selected objects for automatic highlighting */
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;
	/* flag storing whether the GT_element_group has changed since last cleared */
	int changed;
	/* for accessing objects */
	int access_count;
}; /* struct GT_element_group */

/*
Module functions
----------------
*/

static void GT_element_group_element_change(
	struct MANAGER_MESSAGE(FE_element) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or several of the nodes have changed in the manager. If any changes affect
this <gt_element_group>, affected graphics are updated.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_element_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_element):
			{
				/* all graphics using elements need rebuilding */
				for_each_settings_in_GT_element_group(gt_element_group,
					GT_element_settings_element_change,(void *)NULL);
				GT_element_group_build_graphics_objects(gt_element_group,
					(struct FE_element *)NULL,(struct FE_node *)NULL);
			} break;
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_element):
			case MANAGER_CHANGE_OBJECT(FE_element):
			{
				if (IS_OBJECT_IN_GROUP(FE_element)(message->object_changed,
					gt_element_group->element_group))
				{
					/* regenerate those graphics affected by changed element */
					GT_element_group_build_graphics_objects(gt_element_group,
						message->object_changed,(struct FE_node *)NULL);
				}
				else
				{
					/* Check to see if any embedded fields are affected by this
						 element change */
					if (GT_element_group_has_embedded_field(gt_element_group,
						message->object_changed, (struct FE_node *)NULL))
					{
						/* rebuild those graphics objects with embedded fields */
						for_each_settings_in_GT_element_group(gt_element_group,
							GT_element_settings_remove_graphics_object_if_embedded_field,
							(void *)NULL);
						GT_element_group_build_graphics_objects(gt_element_group,
							(struct FE_element *)NULL,(struct FE_node *)NULL);
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_element):
			case MANAGER_CHANGE_DELETE(FE_element):
			case MANAGER_CHANGE_IDENTIFIER(FE_element):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_element_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_element_change */

static void GT_element_group_element_group_change(
	struct MANAGER_MESSAGE(GROUP(FE_element)) *message,
	void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or several of the element groups have changed in the manager. If the
element_group for <gt_element_group> has changed, affected graphics are updated.
Note that add and delete messages are handled by the parent to create and
destroy GT_element_groups - including this one. Hence, ignore them here.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_element_group_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(GROUP(FE_element)):
			case MANAGER_CHANGE_OBJECT(GROUP(FE_element)):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_element)):
			{
				if ((!message->object_changed) ||
					(gt_element_group->element_group==message->object_changed))
				{
					for_each_settings_in_GT_element_group(gt_element_group,
						GT_element_settings_element_change,(void *)NULL);
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
				}
			} break;
			case MANAGER_CHANGE_ADD(GROUP(FE_element)):
			case MANAGER_CHANGE_DELETE(GROUP(FE_element)):
			case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_element)):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_element_group_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_element_group_change */

static void GT_element_group_node_change(
	struct MANAGER_MESSAGE(FE_node) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or several of the nodes have changed in the manager. If any changes affect
this <gt_element_group>, affected graphics are updated.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_node_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_node):
			{
				/* rebuild all graphics for gt_element_group */
				for_each_settings_in_GT_element_group(gt_element_group,
					GT_element_settings_remove_graphics_object,(void *)NULL);
				GT_element_group_build_graphics_objects(gt_element_group,
					(struct FE_element *)NULL,(struct FE_node *)NULL);
			} break;
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			case MANAGER_CHANGE_OBJECT(FE_node):
			{
				if (IS_OBJECT_IN_GROUP(FE_node)(message->object_changed,
					gt_element_group->node_group))
				{
					/* rebuild those graphics affected by changed node */
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,message->object_changed);
				}
				else
				{
					/* Check to see if any embedded fields are affected by this
						 node change */
					if (GT_element_group_has_embedded_field(gt_element_group,
						(struct FE_element *)NULL, message->object_changed))
					{
						/* rebuild those graphics objects with embedded fields */
						for_each_settings_in_GT_element_group(gt_element_group,
							GT_element_settings_remove_graphics_object_if_embedded_field,
							(void *)NULL);
						GT_element_group_build_graphics_objects(gt_element_group,
							(struct FE_element *)NULL,(struct FE_node *)NULL);
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_node):
			case MANAGER_CHANGE_DELETE(FE_node):
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_node_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_node_change */

static void GT_element_group_node_group_change(
	struct MANAGER_MESSAGE(GROUP(FE_node)) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or several of the node groups have changed in the manager. If the node_group
for <gt_element_group> has changed, affected graphics are updated.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_node_group_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(GROUP(FE_node)):
			case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
			{
				if ((!message->object_changed) ||
					(gt_element_group->node_group==message->object_changed))
				{
					for_each_settings_in_GT_element_group(gt_element_group,
						GT_element_settings_node_change,(void *)NULL);
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
				}
			} break;
			case MANAGER_CHANGE_DELETE(GROUP(FE_node)):
			case MANAGER_CHANGE_ADD(GROUP(FE_node)):
			case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_node)):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_node_group_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_node_group_change */

static void GT_element_group_computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or more of the computed_fields have changed in the manager.
Updates any graphics affected by the change.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;
	struct GT_element_settings_computed_field_change_data change_data;

	ENTER(GT_element_group_computed_field_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Computed_field):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				change_data.rebuild_graphics=0;
				change_data.changed_field=message->object_changed;
				change_data.default_coordinate_field=
					gt_element_group->default_coordinate_field;
				FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
					GT_element_settings_computed_field_change,(void *)&change_data,
					gt_element_group->list_of_settings);
				if (change_data.rebuild_graphics)
				{
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
				}
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_DELETE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_computed_field_change */

static void GT_element_group_data_change(
	struct MANAGER_MESSAGE(FE_node) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or several of the nodes have changed in the manager. If any changes affect
this <gt_element_group>, affected graphics are updated.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_data_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_node):
			{
				/* rebuild all graphics for gt_element_group */
				/*???RC rebuild all for a data change? */
				for_each_settings_in_GT_element_group(gt_element_group,
					GT_element_settings_remove_graphics_object,(void *)NULL);
				GT_element_group_build_graphics_objects(gt_element_group,
					(struct FE_element *)NULL,(struct FE_node *)NULL);
			} break;
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			case MANAGER_CHANGE_OBJECT(FE_node):
			{
				if (IS_OBJECT_IN_GROUP(FE_node)(message->object_changed,
					gt_element_group->data_group))
				{
					/* rebuild those graphics affected by changed data */
					for_each_settings_in_GT_element_group(gt_element_group,
						GT_element_settings_data_change,(void *)NULL);
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_node):
			case MANAGER_CHANGE_DELETE(FE_node):
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_data_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_data_change */

static void GT_element_group_data_group_change(
	struct MANAGER_MESSAGE(GROUP(FE_node)) *message,void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
One or several of the node groups have changed in the manager. If the data_group
for <gt_element_group> has changed, affected graphics are updated.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_data_group_change);
	if (message&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(GROUP(FE_node)):
			case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
			{
				if ((!message->object_changed) ||
					(gt_element_group->data_group==message->object_changed))
				{
					for_each_settings_in_GT_element_group(gt_element_group,
						GT_element_settings_data_change,(void *)NULL);
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
				}
			} break;
			case MANAGER_CHANGE_DELETE(GROUP(FE_node)):
			case MANAGER_CHANGE_ADD(GROUP(FE_node)):
			case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_node)):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_data_group_change.  Invalid argument(s)");
	}
	LEAVE;
} /* GT_element_group_data_group_change */

static void GT_element_group_element_point_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Callback for change in the global element selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_element_point_ranges_selection_change);
	if (element_point_ranges_selection&&changes&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect elements in this group */
		if (FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
			Element_point_ranges_element_is_in_group,
			(void *)gt_element_group->element_group,
			changes->newly_selected_element_point_ranges_list)||
			FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
				Element_point_ranges_element_is_in_group,
				(void *)gt_element_group->element_group,
				changes->newly_unselected_element_point_ranges_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_element_points_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_build_graphics_objects(gt_element_group,
				(struct FE_element *)NULL,(struct FE_node *)NULL);
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
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Callback for change in the global element selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_element_selection_change);
	if (element_selection&&changes&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect elements in this group */
		if (FIRST_OBJECT_IN_LIST_THAT(FE_element)(FE_element_is_in_group,
			(void *)gt_element_group->element_group,
			changes->newly_selected_element_list)||
			FIRST_OBJECT_IN_LIST_THAT(FE_element)(FE_element_is_in_group,
				(void *)gt_element_group->element_group,
				changes->newly_unselected_element_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_elements_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_build_graphics_objects(gt_element_group,
				(struct FE_element *)NULL,(struct FE_node *)NULL);
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
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_node_selection_change);
	if (node_selection&&changes&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect nodes in this group */
		if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_is_in_group,
			(void *)gt_element_group->node_group,changes->newly_selected_node_list)||
			FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_is_in_group,
				(void *)gt_element_group->node_group,
				changes->newly_unselected_node_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_nodes_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_build_graphics_objects(gt_element_group,
				(struct FE_element *)NULL,(struct FE_node *)NULL);
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
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Callback for change in the global data selection.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(GT_element_group_data_selection_change);
	if (data_selection&&changes&&
		(gt_element_group=(struct GT_element_group *)gt_element_group_void))
	{
		/* find out if any of the changes affect data in this group */
		if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_is_in_group,
			(void *)gt_element_group->data_group,changes->newly_selected_node_list)||
			FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_is_in_group,
				(void *)gt_element_group->data_group,
				changes->newly_unselected_node_list))
		{
			/* update the graphics to match */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_selected_data_change,(void *)NULL,
				gt_element_group->list_of_settings);
			GT_element_group_build_graphics_objects(gt_element_group,
				(struct FE_element *)NULL,(struct FE_node *)NULL);
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
	struct GROUP(FE_element) *element_group,struct GROUP(FE_node) *node_group,
	struct GROUP(FE_node) *data_group,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct Computed_field_package *computed_field_package,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Allocates memory and assigns fields for a graphical finite element group for
the given <element_group>. Its partner <node_group> must also be supplied, and
is expected to be of the same name. The rest of the application must ensure
it contains at least the nodes referenced by the elements in the element group.
Likewise, the <data_group> is expected to be supplied and of the same name.
The GT_element_group does not access the element group, but it does access the
node and data groups. It must therefore be destroyed in response to element
group manager delete messages - currently handled by a Scene - which must
precede removing the node and data groups from their respective managers.
If supplied, callbacks are requested from the <element_selection> and
<node_selection> to enable automatic highlighting of selected graphics.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(CREATE(GT_element_group));
	if (element_group&&node_group&&data_group)
	{
		if (ALLOCATE(gt_element_group,struct GT_element_group,1))
		{
			gt_element_group->list_of_settings=
				(struct LIST(GT_element_settings) *)NULL;
			if (gt_element_group->list_of_settings=
				CREATE(LIST(GT_element_settings))())
			{
				gt_element_group->element_group=element_group;
#if defined (OLD_CODE)
				/*???DB.  element_group can recover if group is destroyed because will
					be told.  So does not need to ACCESS */
				gt_element_group->element_group=
					ACCESS(GROUP(FE_element))(element_group);
#endif /* defined (OLD_CODE) */
				gt_element_group->node_group=ACCESS(GROUP(FE_node))(node_group);
				gt_element_group->data_group=ACCESS(GROUP(FE_node))(data_group);
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
				gt_element_group->element_manager=element_manager;
				gt_element_group->element_manager_callback_id=(void *)NULL;
				gt_element_group->element_group_manager=element_group_manager;
				gt_element_group->element_group_manager_callback_id=(void *)NULL;
				gt_element_group->node_manager=node_manager;
				gt_element_group->node_manager_callback_id=(void *)NULL;
				gt_element_group->node_group_manager=node_group_manager;
				gt_element_group->node_group_manager_callback_id=(void *)NULL;
				gt_element_group->data_manager=data_manager;
				gt_element_group->data_manager_callback_id=(void *)NULL;
				gt_element_group->data_group_manager=data_group_manager;
				gt_element_group->data_group_manager_callback_id=(void *)NULL;
				gt_element_group->computed_field_package=computed_field_package;
				gt_element_group->computed_field_manager_callback_id=(void *)NULL;
				/* request callbacks from any managers supplied */
				if (element_manager)
				{
					gt_element_group->element_manager_callback_id=
						MANAGER_REGISTER(FE_element)(GT_element_group_element_change,
							(void *)gt_element_group,gt_element_group->element_manager);
				}
				if (element_group_manager)
				{
					gt_element_group->element_group_manager_callback_id=
						MANAGER_REGISTER(GROUP(FE_element))(
							GT_element_group_element_group_change,(void *)gt_element_group,
							gt_element_group->element_group_manager);
				}
				if (node_manager)
				{
					gt_element_group->node_manager_callback_id=
						MANAGER_REGISTER(FE_node)(GT_element_group_node_change,
							(void *)gt_element_group,gt_element_group->node_manager);
				}
				if (node_group_manager)
				{
					gt_element_group->node_group_manager_callback_id=
						MANAGER_REGISTER(GROUP(FE_node))(GT_element_group_node_group_change,
							(void *)gt_element_group,gt_element_group->node_group_manager);
				}
				if (data_manager)
				{
					gt_element_group->data_manager_callback_id=MANAGER_REGISTER(FE_node)(
						GT_element_group_data_change,(void *)gt_element_group,
						gt_element_group->data_manager);
				}
				if (data_group_manager)
				{
					gt_element_group->data_group_manager_callback_id=
						MANAGER_REGISTER(GROUP(FE_node))(GT_element_group_data_group_change,
							(void *)gt_element_group,gt_element_group->data_group_manager);
				}
				if (computed_field_package)
				{
					gt_element_group->computed_field_manager_callback_id=
						MANAGER_REGISTER(Computed_field)(
							GT_element_group_computed_field_change,(void *)gt_element_group,
							Computed_field_package_get_computed_field_manager(
								gt_element_group->computed_field_package));
				}
				/* global selections */
				gt_element_group->element_point_ranges_selection=
					element_point_ranges_selection;
				gt_element_group->element_selection=element_selection;
				gt_element_group->node_selection=node_selection;
				gt_element_group->data_selection=data_selection;
				gt_element_group->changed=1;
				gt_element_group->access_count=0;
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
			}
			else
			{
				DESTROY(LIST(GT_element_settings))(
					&(gt_element_group->list_of_settings));
				DEALLOCATE(gt_element_group);
			}
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
LAST MODIFIED : 28 April 2000

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
		if (gt_element_group=CREATE(GT_element_group)(
			existing_gt_element_group->element_group,
			existing_gt_element_group->node_group,
			existing_gt_element_group->data_group,
			(struct MANAGER(FE_element) *)NULL,
			(struct MANAGER(GROUP(FE_element)) *)NULL,
			(struct MANAGER(FE_node) *)NULL,
			(struct MANAGER(GROUP(FE_node)) *)NULL,
			(struct MANAGER(FE_node) *)NULL,
			(struct MANAGER(GROUP(FE_node)) *)NULL,
			(struct Computed_field_package *)NULL,
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
LAST MODIFIED : 6 April 2000

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
				/* turn off manager messages */
				if (gt_element_group->element_manager_callback_id)
				{
					MANAGER_DEREGISTER(FE_element)(
						gt_element_group->element_manager_callback_id,
						gt_element_group->element_manager);
				}
				if (gt_element_group->element_group_manager_callback_id)
				{
					MANAGER_DEREGISTER(GROUP(FE_element))(
						gt_element_group->element_group_manager_callback_id,
						gt_element_group->element_group_manager);
				}
				if (gt_element_group->node_manager_callback_id)
				{
					MANAGER_DEREGISTER(FE_node)(
						gt_element_group->node_manager_callback_id,
						gt_element_group->node_manager);
				}
				if (gt_element_group->node_group_manager_callback_id)
				{
					MANAGER_DEREGISTER(GROUP(FE_node))(
						gt_element_group->node_group_manager_callback_id,
						gt_element_group->node_group_manager);
				}
				if (gt_element_group->data_manager_callback_id)
				{
					MANAGER_DEREGISTER(FE_node)(
						gt_element_group->data_manager_callback_id,
						gt_element_group->data_manager);
				}
				if (gt_element_group->data_group_manager_callback_id)
				{
					MANAGER_DEREGISTER(GROUP(FE_node))(
						gt_element_group->data_group_manager_callback_id,
						gt_element_group->data_group_manager);
				}
				if (gt_element_group->computed_field_manager_callback_id)
				{
					MANAGER_DEREGISTER(Computed_field)(
						gt_element_group->computed_field_manager_callback_id,
						Computed_field_package_get_computed_field_manager(
							gt_element_group->computed_field_package));
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
#if defined (OLD_CODE)
				/*???DB.  element_group can recover if group is destroyed because will
					be told.  So does not need to ACCESS */
				DEACCESS(GROUP(FE_element))(&(gt_element_group->element_group));
#endif /* defined (OLD_CODE) */
				DEACCESS(GROUP(FE_node))(&(gt_element_group->node_group));
				DEACCESS(GROUP(FE_node))(&(gt_element_group->data_group));
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
				DEALLOCATE(*gt_element_group_address);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_element_group).  "
			"Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_element_group) */

int GET_NAME(GT_element_group)(struct GT_element_group *gt_element_group,
	char **name_ptr)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Returns the name of the <gt_element_group>.
==============================================================================*/
{
	int return_code;

	ENTER(GET_NAME(GT_element_group));
	if (gt_element_group)
	{
		return_code = GET_NAME(GROUP(FE_element))(gt_element_group->element_group,
			name_ptr);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GET_NAME(GT_element_group).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GET_NAME(GT_element_group) */

int GT_element_group_changed(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
External modules that change a GT_element_group should call this routine so that
objects interested in this GT_element_group will be notified that is has changed.
==============================================================================*/
{
	int return_code;
	struct GT_element_group_callback_data *callback_data;

	ENTER(GT_element_group_changed);

	if (gt_element_group)
	{
		gt_element_group->changed = 1;
		callback_data = gt_element_group->update_callback_list;
		while(callback_data)
		{
			(callback_data->callback)(gt_element_group,
				callback_data->callback_user_data);
			callback_data = callback_data->next;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_changed. Invalid GT_element_group");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_changed */

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

int GT_element_group_get_settings_position(struct GT_element_group
	*gt_element_group,struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

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
	struct GT_element_group *gt_element_group,int circle_discretization,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Sets the circle discretization of the gt_element_group.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_circle_discretization);
	if (gt_element_group&&
		check_Circle_discretization(&circle_discretization,user_interface))
	{
		return_code=1;
		if (gt_element_group->circle_discretization != circle_discretization)
		{
			gt_element_group->circle_discretization=circle_discretization;
			/* clear graphics for all settings using circle discretization */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_circle_discretization_change,(void *)NULL,
				gt_element_group->list_of_settings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_circle_discretization.  Invalid argument(s)");
		return_code=0;
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
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Sets the <default_coordinate_field> used by <gt_element_group>. Settings without
a specific coordinate field will use this one.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_default_coordinate_field);
	if (gt_element_group&&default_coordinate_field&&
		(3>=Computed_field_get_number_of_components(default_coordinate_field)))
	{
		return_code=1;
		if (default_coordinate_field != gt_element_group->default_coordinate_field)
		{
			if (default_coordinate_field)
			{
				ACCESS(Computed_field)(default_coordinate_field);
			}
			if (gt_element_group->default_coordinate_field)
			{
				DEACCESS(Computed_field)(&(gt_element_group->default_coordinate_field));
			}
			gt_element_group->default_coordinate_field=default_coordinate_field;
			/* clear graphics for all settings using default coordinate field */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_default_coordinate_field_change,(void *)NULL,
				gt_element_group->list_of_settings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_default_coordinate_field.  Invalid argument(s)");
		return_code=0;
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
	struct Element_discretization *element_discretization,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Sets the element discretization of the gt_element_group.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_set_element_discretization);
	if (gt_element_group&&element_discretization&&
		check_Element_discretization(element_discretization,user_interface))
	{
		return_code=1;
		if ((gt_element_group->element_discretization.number_in_xi1 !=
			element_discretization->number_in_xi1)||
			(gt_element_group->element_discretization.number_in_xi2 !=
				element_discretization->number_in_xi2)||
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
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_element_discretization.  Invalid argument(s)");
		return_code=0;
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
LAST MODIFIED : 3 March 1999

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
		return_code=1;
		if (native_discretization_field !=
			gt_element_group->native_discretization_field)
		{
			if (native_discretization_field)
			{
				ACCESS(FE_field)(native_discretization_field);
			}
			if (gt_element_group->native_discretization_field)
			{
				DEACCESS(FE_field)(&(gt_element_group->native_discretization_field));
			}
			gt_element_group->native_discretization_field=native_discretization_field;
			/* clear graphics for all settings using element discretization */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_element_discretization_change,(void *)NULL,
				gt_element_group->list_of_settings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_set_native_discretization_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_set_native_discretization_field */

int GT_element_group_build_graphics_objects(
	struct GT_element_group *gt_element_group,struct FE_element *changed_element,
	struct FE_node *changed_node)
/*******************************************************************************
LAST MODIFIED : 6 June 2000

DESCRIPTION :
Adds or edits a graphics object for each settings in <gt_element_group> without
a graphics object or affected by <changed_element> or <changed_node>.
==============================================================================*/
{
	int return_code;
	struct Element_discretization element_discretization;
	struct GT_element_settings_to_graphics_object_data settings_to_object_data;

	ENTER(GT_element_group_build_graphics_objects);
	if (gt_element_group)
	{
		if (settings_to_object_data.default_rc_coordinate_field=
			Computed_field_begin_wrap_coordinate_field(
				gt_element_group->default_coordinate_field))
		{
			settings_to_object_data.rc_coordinate_field=(struct Computed_field *)NULL;
			settings_to_object_data.wrapper_orientation_scale_field=
				(struct Computed_field *)NULL;
			settings_to_object_data.native_discretization_field=
				gt_element_group->native_discretization_field;
			settings_to_object_data.element_group=gt_element_group->element_group;
			settings_to_object_data.node_group=gt_element_group->node_group;
			settings_to_object_data.data_group=gt_element_group->data_group;
			element_discretization.number_in_xi1=
				gt_element_group->element_discretization.number_in_xi1;
			element_discretization.number_in_xi2=
				gt_element_group->element_discretization.number_in_xi2;
			element_discretization.number_in_xi3=
				gt_element_group->element_discretization.number_in_xi3;
			settings_to_object_data.element_discretization=&element_discretization;
			settings_to_object_data.circle_discretization=
				gt_element_group->circle_discretization;
			settings_to_object_data.changed_element=changed_element;
			settings_to_object_data.changed_node=changed_node;
			settings_to_object_data.selected_element_point_ranges_list=
				Element_point_ranges_selection_get_element_point_ranges_list(
					gt_element_group->element_point_ranges_selection);
			settings_to_object_data.selected_element_list=
				FE_element_selection_get_element_list(
					gt_element_group->element_selection);
			settings_to_object_data.selected_data_list=
				FE_node_selection_get_node_list(gt_element_group->data_selection);
			settings_to_object_data.selected_node_list=
				FE_node_selection_get_node_list(gt_element_group->node_selection);
			/* create list for storing removed_primative names in edit mode */
			settings_to_object_data.removed_primitives=
				CREATE(LIST(Selected_graphic))();
			return_code=FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_to_graphics_object,(void *)&settings_to_object_data,
				gt_element_group->list_of_settings);
			/* destroy list created above */
			DESTROY(LIST(Selected_graphic))(
				&(settings_to_object_data.removed_primitives));
			GT_element_group_changed(gt_element_group);
			Computed_field_end_wrap(
				&settings_to_object_data.default_rc_coordinate_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,"GT_element_group_build_graphics_objects.  "
				"Could not get default_rc_coordinate_field wrapper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_element_group_build_graphics_objects.  "
			"Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_build_graphics_objects */

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
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_remove_graphics_object,(void *)NULL,
			destination->list_of_settings);
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
LAST MODIFIED : 3 March 1999

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
	if (destination&&source)
	{
		if (temp_list_of_settings=CREATE(LIST(GT_element_settings))())
		{
			GT_element_group_set_default_coordinate_field(destination,
				source->default_coordinate_field);
			GT_element_group_set_circle_discretization(destination,
				source->circle_discretization,(struct User_interface *)NULL);
			GT_element_group_set_element_discretization(destination,
				&(source->element_discretization),(struct User_interface *)NULL);
			GT_element_group_set_native_discretization_field(destination,
				source->native_discretization_field);
			/* move copy of current destination settings to temporary list */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_copy_and_put_in_list,
				(void *)temp_list_of_settings,destination->list_of_settings);
			REMOVE_ALL_OBJECTS_FROM_LIST(GT_element_settings)(
				destination->list_of_settings);
			/* put copy of each settings in source list in destination list */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_copy_and_put_in_list,
				(void *)destination->list_of_settings,source->list_of_settings);
			/* extract graphics_object that can be reused from temporary list */
			FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_extract_graphics_object_from_list,
				(void *)temp_list_of_settings,destination->list_of_settings);
			/* rebuild the graphics_objects */
			GT_element_group_build_graphics_objects(destination,
				(struct FE_element *)NULL,(struct FE_node *)NULL);
			DESTROY(LIST(GT_element_settings))(&temp_list_of_settings);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_element_group_modify.  Could not create temporary list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_modify.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_modify */

struct GROUP(FE_element) *GT_element_group_get_element_group(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the element_group used by <gt_element_group>.
==============================================================================*/
{
	struct GROUP(FE_element) *element_group;

	ENTER(GT_element_group_get_element_group);
	if (gt_element_group)
	{
		element_group=gt_element_group->element_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_element_group.  Invalid argument(s)");
		element_group=(struct GROUP(FE_element) *)NULL;
	}
	LEAVE;

	return (element_group);
} /* GT_element_group_get_element_group */

struct GROUP(FE_node) *GT_element_group_get_node_group(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the node_group used by <gt_element_group>.
==============================================================================*/
{
	struct GROUP(FE_node) *node_group;

	ENTER(GT_element_group_get_node_group);
	if (gt_element_group)
	{
		node_group=gt_element_group->node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_node_group.  Invalid argument(s)");
		node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;

	return (node_group);
} /* GT_element_group_get_node_group */

struct GROUP(FE_node) *GT_element_group_get_data_group(
	struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Returns the data_group used by <gt_element_group>.
==============================================================================*/
{
	struct GROUP(FE_node) *data_group;

	ENTER(GT_element_group_get_data_group);
	if (gt_element_group)
	{
		data_group=gt_element_group->data_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_get_data_group.  Invalid argument(s)");
		data_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;

	return (data_group);
} /* GT_element_group_get_data_group */

int GT_element_group_list_commands(struct GT_element_group *gt_element_group,
	char *command_prefix,char *command_suffix)
/*******************************************************************************
LAST MODIFIED : 14 December 1999

DESCRIPTION :
Lists the general settings and graphics defined for <gt_element_group> - as a
set of commands that can be used to reproduce the groups appearance. The
<command_prefix> should generally contain "gfx modify g_element" while the
optional <command_suffix> may describe the scene (eg. "scene default").
==============================================================================*/
{
	char *name,*line_prefix;
	int return_code;
	struct GT_element_settings_list_data list_data;

	ENTER(GT_element_group_list_commands);
	if (gt_element_group&&command_prefix)
	{
		/* add element group name to command_prefix */
		if (GET_NAME(GROUP(FE_element))(gt_element_group->element_group,&name))
		{
			/* put quotes around group_name if it contains special characters */
			make_valid_token(&name);
			if (ALLOCATE(line_prefix,char,strlen(name)+strlen(command_prefix)+3))
			{
				sprintf(line_prefix,"%s %s ",command_prefix,name);
				DEALLOCATE(name);
				display_message(INFORMATION_MESSAGE,line_prefix);
				display_message(INFORMATION_MESSAGE,"general clear");
				display_message(INFORMATION_MESSAGE," circle_discretization %d",
					gt_element_group->circle_discretization);
				if (GET_NAME(Computed_field)(gt_element_group->default_coordinate_field,
					&name))
				{
					make_valid_token(&name);
					display_message(INFORMATION_MESSAGE," default_coordinate %s",name);
					DEALLOCATE(name);
				}
				display_message(INFORMATION_MESSAGE," element_discretization %d*%d*%d",
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
				list_data.line_prefix=line_prefix;
				list_data.line_suffix=command_suffix;
				return_code=FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
					GT_element_settings_list_contents,(void *)&list_data,
					gt_element_group->list_of_settings);
				DEALLOCATE(line_prefix);
			}
			else
			{
				return_code=0;
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
		if (GET_NAME(Computed_field)(gt_element_group->default_coordinate_field,
			&name))
		{
			display_message(INFORMATION_MESSAGE,
				"  default coordinate field: %s\n",name);
			DEALLOCATE(name);
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

int GT_element_group_clear_changed(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Clears the changed flag in <gt_element_group>. The flag can subsequently be
checked with GT_element_group_has_changed to see whether the display list for
the graphics object enclosing <gt_element_group> needs updating. Once it is
updated, this routine should be called.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_clear_changed);
	if (gt_element_group)
	{
		gt_element_group->changed=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_clear_changed.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_clear_changed */

int GT_element_group_has_changed(struct GT_element_group *gt_element_group)
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Returns true if <gt_element_group> is flagged as having been changed.
See also GT_element_group_clear_changed.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_group_has_changed);
	if (gt_element_group)
	{
		return_code=gt_element_group->changed;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_has_changed.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_has_changed */

int GT_element_group_has_embedded_field(struct GT_element_group *gt_element_group,
	struct FE_element *changed_element, struct FE_node *changed_node)
/*******************************************************************************
LAST MODIFIED : 28 April 1999

DESCRIPTION :
Returns true if <gt_element_group> contains settings which use embedded fields which
are affected by the <changed_node> or <changed_element>.
If <changed_node> and <changed_element> are both NULL then this returns true 
if the group contains any settings using embedded fields.
==============================================================================*/
{
	struct FE_node_field_has_embedded_element_data data;
	int return_code;

	ENTER(GT_element_group_has_embedded_field);
	if (gt_element_group)
	{
		if (Computed_field_depends_on_embedded_field(
			gt_element_group->default_coordinate_field)
			||(FIRST_OBJECT_IN_LIST_THAT(GT_element_settings)(
			GT_element_settings_has_embedded_field,NULL,
			gt_element_group->list_of_settings)))
		{
			if ((!changed_element)&&(!changed_node))
			{
				return_code = 1;
			}
			else
			{
				data.changed_node = changed_node;
				data.changed_element = changed_element;
				if((FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
					FE_node_has_embedded_element_or_node,
					(void *)&data, gt_element_group->node_group))||
					(FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
					FE_node_has_embedded_element_or_node,
					(void *)&data, gt_element_group->data_group)))
				{
					return_code = 1;
				}
				else
				{
					return_code = 0;
				}
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
			"GT_element_group_has_embedded_fields.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_has_embedded_fields */

int compile_GT_element_group(struct GT_element_group *gt_element_group,
	float time)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Rebuilds the display list for graphical_element_group. 
The object is compiled at the time pointed to by <time_void>.
==============================================================================*/
{
	int return_code;

	ENTER(compile_GT_element_group);
	if (gt_element_group)
	{
		return_code=1;

		if (GT_element_group_has_changed(gt_element_group))
		{
			GT_element_group_clear_changed(gt_element_group);
		}
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_compile_visible_settings,(void *)&time,
			gt_element_group->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,"compile_GT_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_GT_object */

int execute_GT_element_group(struct GT_element_group *gt_element_group,
	float time)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(execute_GT_element_group);
	USE_PARAMETER(time);
	if (gt_element_group)
	{
		return_code=1;
		/* Execute display lists of graphics objects in settings making up
			the GT_element_group objects, adding the position as names for
			OpenGL picking */
		/* push initial name to make space for subobject names */
		glPushName(-1);
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_execute_visible_settings,
			(void *)&time,gt_element_group->list_of_settings);
		glPopName();
		/* close individual object */
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_GT_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_GT_element_group */


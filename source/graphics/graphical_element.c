/*******************************************************************************
FILE : graphical_element.c

LAST MODIFIED : 21 September 1999

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
LAST MODIFIED : 20 July 1999

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
	/* flag storing whether the GT_element_group has changed since last cleared */
	int changed;
	/* for accessing objects */
	int access_count;
}; /* struct GT_element_group */

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(GT_element_group)

struct GT_element_group *CREATE(GT_element_group)(
	struct GROUP(FE_element) *element_group,struct GROUP(FE_node) *node_group,
	struct GROUP(FE_node) *data_group)
/*******************************************************************************
LAST MODIFIED : 20 July 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphical finite element group for
the given <element_group>. Its partner <node_group> must also be supplied, and
is expected to be of the same name. The rest of the application must ensure
it contains at least the nodes referenced by the elements in the element group.
Likewise, the <data_group> is expected to be supplied and of the same name.
The GT_element_group does not access the element group, but it does access the
node and data groups. It must therefore be destroyed in response to element
group manager delete messages (currently handled by a Scene), which must
precede removing the node and data groups from their respective managers.
==============================================================================*/
{
	struct GT_element_group *gt_element_group;

	ENTER(CREATE(GT_element_group));
	if (element_group&&node_group&&data_group)
	{
		if (ALLOCATE(gt_element_group,struct GT_element_group,1)&&
			(gt_element_group->list_of_settings=CREATE(LIST(GT_element_settings))()))
		{
			gt_element_group->element_group=element_group;
#if defined (OLD_CODE)
			/*???DB.  element_group can recover if group is destroyed because will be
			told.  So does not need to ACCESS */
			gt_element_group->element_group=ACCESS(GROUP(FE_element))(element_group);
#endif /* defined (OLD_CODE) */
			gt_element_group->node_group=ACCESS(GROUP(FE_node))(node_group);
			gt_element_group->data_group=ACCESS(GROUP(FE_node))(data_group);
			/* set settings shared by whole rendition */
			gt_element_group->element_discretization.number_in_xi1=2;
			gt_element_group->element_discretization.number_in_xi2=2;
			gt_element_group->element_discretization.number_in_xi3=2;
			gt_element_group->circle_discretization=3;
			gt_element_group->default_coordinate_field=(struct Computed_field *)NULL;
			gt_element_group->native_discretization_field=(struct FE_field *)NULL;
			gt_element_group->update_callback_list=
				(struct GT_element_group_callback_data *)NULL;
			gt_element_group->changed=1;
			gt_element_group->access_count=0;
		}
		else
		{
			DEALLOCATE(gt_element_group);
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

int DESTROY(GT_element_group)(struct GT_element_group **gt_element_group)
/*******************************************************************************
LAST MODIFIED : 20 July 1999

DESCRIPTION :
Frees the memory for <**gt_element_group> and sets <*gt_element_group> to NULL.
==============================================================================*/
{
	int return_code;
	struct GT_element_group_callback_data *callback_data, *next;

	ENTER(DESTROY(GT_element_group));
	if (gt_element_group)
	{
		return_code=1;
		if (*gt_element_group)
		{
			if (0 != (*gt_element_group)->access_count)
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(GT_element_group).  Access count = %d",
					(*gt_element_group)->access_count);
				return_code=0;
			}
			else
			{
				DESTROY(LIST(GT_element_settings))(
					&((*gt_element_group)->list_of_settings));
#if defined (OLD_CODE)
				/*???DB.  element_group can recover if group is destroyed because will
					be told.  So does not need to ACCESS */
				DEACCESS(GROUP(FE_element))(&((*gt_element_group)->element_group));
#endif /* defined (OLD_CODE) */
				DEACCESS(GROUP(FE_node))(&((*gt_element_group)->node_group));
				DEACCESS(GROUP(FE_node))(&((*gt_element_group)->data_group));
				if ((*gt_element_group)->default_coordinate_field)
				{
					DEACCESS(Computed_field)(
						&((*gt_element_group)->default_coordinate_field));
				}
				if ((*gt_element_group)->native_discretization_field)
				{
					DEACCESS(FE_field)(
						&((*gt_element_group)->native_discretization_field));
				}
				callback_data = (*gt_element_group)->update_callback_list;
				while(callback_data)
				{
					next = callback_data->next;
					DEALLOCATE(callback_data);
					callback_data = next;
				}
				DEALLOCATE(*gt_element_group);
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
LAST MODIFIED : 12 March 1999

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
			/* (de)access default_rc_coordinate_field to clean up if created above */
			ACCESS(Computed_field)(
				settings_to_object_data.default_rc_coordinate_field);
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
			return_code=FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
				GT_element_settings_to_graphics_object,(void *)&settings_to_object_data,
				gt_element_group->list_of_settings);
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
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Copies the GT_element_group contents from source to destination.
Pointers to graphics_objects are cleared in the destination list of settings.
???RC Not a full COPY object function: copies only general and specific
settings while element_group, node_group and other members are not copied.
Otherwise need to work out how to handle node_groups that are maintained by the
GT_element_group. Current routine is sufficient for the graphical element
attribute editor.
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
LAST MODIFIED : 4 March 1999

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
				display_message(INFORMATION_MESSAGE,"general circle_discretization %d",
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

int GT_element_group_computed_field_change(
	struct GT_element_group *gt_element_group,
	struct Computed_field *changed_field)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
Tells the <gt_element_group> about the <changed_field>, where NULL means all
fields have changed. If any of the graphics objects in the settings need
rebuilding, this function takes care of that, and returns true so that the
scene knows it must invoke a scene redraw.
???RC This should be done with callbacks... get GT_element_group out of
graphics_object.c!
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_computed_field_change_data change_data;

	ENTER(GT_element_group_computed_field_change);
	if (gt_element_group)
	{
		change_data.rebuild_graphics=0;
		change_data.changed_field=changed_field;
		change_data.default_coordinate_field=
			gt_element_group->default_coordinate_field;
		FOR_EACH_OBJECT_IN_LIST(GT_element_settings)(
			GT_element_settings_computed_field_change,(void *)&change_data,
			gt_element_group->list_of_settings);
		if (change_data.rebuild_graphics)
		{
			GT_element_group_build_graphics_objects(gt_element_group,
				(struct FE_element *)NULL,(struct FE_node *)NULL);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_group_computed_field_change.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_group_computed_field_change */

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


/*******************************************************************************
FILE : element_tool.c

LAST MODIFIED : 20 June 2002

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
#include "command/command.h"
#include "element/element_operations.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "user_interface/message.h"

int destroy_listed_elements(struct LIST(FE_element) *element_list,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Destroys all the elements in <element_list> that are not accessed outside
<element_manager>, the groups in <element_group_manager>,
<element_selection> and <element_point_ranges_selection>.
<element_group_manager>, <element_selection> and
<element_point_ranges_selection> are optional. Upon return <element_list>
contains all the elements that could not be destroyed.
???RC Should really be in its own module.
Note: currently requires all elements in the <element_list> to be of the same
CM_element_type, otherwise likely to fail. ???RC Fix this by filtering out
elements with all parents also in the list?
==============================================================================*/
{
	int number_of_elements_destroyed, number_of_elements_not_destroyed,
		return_code;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element *element;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_element) *not_destroyed_element_list, *selected_element_list;
	struct LIST(Element_point_ranges) *selected_element_point_ranges_list;

	ENTER(destroy_listed_elements);
	if (element_list && element_manager)
	{
		return_code = 1;
		/* build list of elements that could not be destroyed */
		not_destroyed_element_list = CREATE(LIST(FE_element))();
		if (element_group_manager)
		{
			/* remove the elements - and their faces recursively - from all
				 groups they are in */
			while (return_code && (element_group =
				FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_element))(
					FE_element_group_intersects_list, (void *)element_list,
					element_group_manager)))
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
				while (return_code && (element = FIRST_OBJECT_IN_GROUP_THAT(FE_element)(
					FE_element_is_in_list, (void *)element_list, element_group)))
				{
					return_code = remove_FE_element_and_faces_from_group(element,
						element_group, RECURSIVE_REMOVE_ELEMENT_AND_PARENTLESS_FACES);
				}
				MANAGED_GROUP_END_CACHE(FE_element)(element_group);
			}
		}
		if (element_selection)
		{
			/* remove elements - and their faces and lines - from the
				 global element_selection */
			FE_element_selection_begin_cache(element_selection);
			selected_element_list =
				FE_element_selection_get_element_list(element_selection);
			while (return_code && (element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(
				FE_element_is_wholly_within_element_list_tree, (void *)element_list,
				selected_element_list)))
			{
				return_code =
					FE_element_selection_unselect_element(element_selection, element);
			}
			FE_element_selection_end_cache(element_selection);
		}
		if (element_point_ranges_selection)
		{
			/* remove all references to elements being removed from the global
				 element_point_ranges_selection */
			Element_point_ranges_selection_begin_cache(
				element_point_ranges_selection);
			selected_element_point_ranges_list=
				Element_point_ranges_selection_get_element_point_ranges_list(
					element_point_ranges_selection);
			while (return_code&&(element_point_ranges=
				FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
					Element_point_ranges_is_wholly_within_element_list_tree,
					(void *)element_list, selected_element_point_ranges_list)))
			{
				return_code =
					Element_point_ranges_selection_unselect_element_point_ranges(
						element_point_ranges_selection, element_point_ranges);
			}
			Element_point_ranges_selection_end_cache(element_point_ranges_selection);
		}
		/* now remove the elements from the manager */
		MANAGER_BEGIN_CACHE(FE_element)(element_manager);
		number_of_elements_destroyed = 0;
		while (return_code && (element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(
			(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL, (void *)NULL,
			element_list)))
		{
			/* element cannot be destroyed while it is in a list */
			if (REMOVE_OBJECT_FROM_LIST(FE_element)(element, element_list))
			{
				if (MANAGED_OBJECT_NOT_IN_USE(FE_element)(element, element_manager))
				{
					if (return_code =
						remove_FE_element_and_faces_from_manager(element, element_manager))
					{
						number_of_elements_destroyed++;
					}
				}
				else
				{
					/* add it to not_destroyed_element_list for reporting */
					ADD_OBJECT_TO_LIST(FE_element)(element, not_destroyed_element_list);
				}
			}
			else
			{
				return_code = 0;
			}
		}
		MANAGER_END_CACHE(FE_element)(element_manager);
		if (0 < (number_of_elements_not_destroyed =
			NUMBER_IN_LIST(FE_element)(not_destroyed_element_list)))
		{
			display_message(WARNING_MESSAGE, "%d element(s) destroyed; "
				"%d element(s) could not be destroyed because in use",
				number_of_elements_destroyed,number_of_elements_not_destroyed);
			return_code = 0;
		}
		FOR_EACH_OBJECT_IN_LIST(FE_element)(ensure_FE_element_is_in_list,
			(void *)element_list, not_destroyed_element_list);
		DESTROY(LIST(FE_element))(&not_destroyed_element_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_listed_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* destroy_listed_elements */

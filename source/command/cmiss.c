/*******************************************************************************
FILE : cmiss.c

LAST MODIFIED : 9 October 2001

DESCRIPTION :
Functions for executing cmiss commands.
==============================================================================*/
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#if defined (MOTIF)
#include <Xm/List.h>
#endif /* defined (MOTIF) */
#if !defined (WINDOWS_DEV_FLAG)
#include "application/application.h"
#if defined (CELL)
#include "cell/cell_interface.h"
#include "cell/cell_window.h"
#endif /* defined (CELL) */
#include "comfile/comfile_window.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "command/cmiss.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "command/command_window.h"
#include "command/example_path.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "command/parser.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_matrix_operations.h"
#include "computed_field/computed_field_wrappers.h"
#include "data/data_grabber_dialog.h"
#include "data/node_transform.h"
#include "data/sync_2d_3d.h"
#include "element/element_creator.h"
#include "element/element_point_viewer.h"
#include "element/element_tool.h"
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "finite_element/grid_field_calculator.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/snake.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "general/debug.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "general/image_utilities.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/environment_map.h"
#include "graphics/graphical_element.h"
#include "graphics/graphical_element_editor_dialog.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/import_graphics_object.h"
#if defined (OLD_CODE)
#include "graphics/interactive_streamlines_dialog.h"
#endif /* defined (OLD_CODE) */
#include "graphics/iso_field_calculation.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/movie_graphics.h"
#if defined (NEW_ALIAS)
#include "graphics/renderalias.h"
#endif /* defined (NEW_ALIAS) */
#include "graphics/renderbinarywavefront.h"
#include "graphics/rendervrml.h"
#include "graphics/renderwavefront.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_editor.h"
#include "graphics/spectrum_editor_dialog.h"
#include "graphics/spectrum_settings.h"
#include "graphics/texture.h"
#include "graphics/texturemap.h"
#include "graphics/transform_tool.h"
#include "graphics/userdef_objects.h"
#include "graphics/volume_texture.h"
#include "graphics/volume_texture_editor.h"
#include "graphics/write_to_video.h"
#include "help/help_interface.h"
#if defined (SELECT_DESCRIPTORS)
#include "io_devices/io_device.h"
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (HAPTIC)
#include "io_devices/haptic_input_module.h"
#endif /* defined (HAPTIC) */
#include "io_devices/input_module_dialog.h"
#if defined (LINK_CMISS)
#include "link/cmiss.h"
#endif /* defined (LINK_CMISS) */
#include "material/material_editor_dialog.h"
#include "menu/menu_window.h"
#if defined (MIRAGE)
/*#include "mirage/movie.h"*/
#include "mirage/tracking_editor_dialog.h"
#endif /* defined (MIRAGE) */
#include "node/interactive_node_editor_dialog.h"
#include "node/node_tool.h"
#include "node/node_viewer.h"
#include "projection/projection_window.h"
#include "slider/emoter_dialog.h"
#include "slider/node_group_slider_dialog.h"
#if defined (OLD_CODE)
#include "socket/socket.h"
#endif /* defined (OLD_CODE) */
#include "three_d_drawing/movie_extensions.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "time/time_editor_dialog.h"
#include "time/time_keeper.h"
#include "transformation/transformation_editor_dialog.h"
#if defined (UNEMAP)
#include "unemap/system_window.h"
#include "unemap/unemap_package.h"
#endif /* defined (UNEMAP) */
#include "user_interface/filedir.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "user_interface/confirmation.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "curve/control_curve.h"
#include "curve/control_curve_editor_dialog.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "view/coord_trans.h"
#include "xvg/include/xvg_interface.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if defined (F90_INTERPRETER)
#include "command/f90_interpreter.h"
#else /* defined (F90_INTERPRETER) */
#if defined (PERL_INTERPRETER)
#include "perl_interpreter.h"
#endif /* defined (PERL_INTERPRETER) */
#endif /* defined (F90_INTERPRETER) */

/*
Module variables
----------------
*/
#if defined (LINK_CMISS)
/*???GMH.  This is a hack - when we register it will disappear (used in
	user_interface.c) */
struct CMISS_connection *CMISS = (struct CMISS_connection *)NULL;
#endif /* LINK_CMISS */

/*
Module functions
----------------
*/

struct Add_FE_element_to_list_if_in_range_data
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Data for iterator function add_FE_element_to_list_if_in_range.
==============================================================================*/
{
	struct Multi_range *element_ranges;
	struct LIST(FE_element) *element_list;
}; /* struct Add_FE_element_to_list_if_in_range_data */

static int add_FE_element_to_list_if_in_range(struct FE_element *element,
	void *element_in_range_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Adds the <element> to the <element_list> if it is a top-level element and
either <multi_range> is NULL or contains the element cm.number.
Does not report any errors if the element is already in the list.
==============================================================================*/
{
	int return_code;
	struct Add_FE_element_to_list_if_in_range_data *element_in_range_data;
	
	ENTER(add_FE_element_to_list_if_in_range);
	if (element&&(element_in_range_data=
		(struct Add_FE_element_to_list_if_in_range_data *)
		element_in_range_data_void)&&element_in_range_data->element_list)
	{
		if ((CM_ELEMENT==element->cm.type)&&
			((!element_in_range_data->element_ranges)||Multi_range_is_value_in_range(
				element_in_range_data->element_ranges,element->cm.number))&&
			(!FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(
				element->identifier,element_in_range_data->element_list)))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_element)(element,
				element_in_range_data->element_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_element_to_list_if_in_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_element_to_list_if_in_range */

struct Add_FE_node_to_list_if_in_range_data
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Data for iterator function add_FE_node_to_list_if_in_range.
==============================================================================*/
{
	struct Multi_range *node_ranges;
	struct LIST(FE_node) *node_list;
}; /* struct Add_FE_node_to_list_if_in_range_data */

static int add_FE_node_to_list_if_in_range(struct FE_node *node,
	void *node_in_range_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Adds the <node> to the <node_list> if either the <multi_range> is NULL or
contains the node cm_node_number.
Does not report any errors if the node is already in the list.
==============================================================================*/
{
	int cm_node_identifier,return_code;
	struct Add_FE_node_to_list_if_in_range_data *node_in_range_data;

	ENTER(add_FE_node_to_list_if_in_range);
	if (node&&(node_in_range_data=
		(struct Add_FE_node_to_list_if_in_range_data *)node_in_range_data_void)&&
		node_in_range_data->node_list)
	{
		return_code=1;
		cm_node_identifier=get_FE_node_cm_node_identifier(node);
		if (((!node_in_range_data->node_ranges)||Multi_range_is_value_in_range(
			node_in_range_data->node_ranges,cm_node_identifier))&&
			(!FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
				cm_node_identifier,node_in_range_data->node_list)))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_node)(node,
				node_in_range_data->node_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_node_to_list_if_in_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_node_to_list_if_in_range */

struct FE_element_values_number
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Data for changing element identifiers.
==============================================================================*/
{
	struct FE_element *element;
	int number_of_values;
	FE_value *values;
	int new_number;
};

static int compare_FE_element_values_number_values(
	const void *element_values1_void, const void *element_values2_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Compares the values in <element_values1> and <element_values2> from the last to
then first, returning -1 as soon as a value in <element_values1> is less than
its counterpart in <element_values2>, or 1 if greater. 0 is returned if all
values are identival. Used as a compare function for qsort.
==============================================================================*/
{
	int i, number_of_values, return_code;
	struct FE_element_values_number *element_values1, *element_values2;

	ENTER(compare_FE_element_values_number_values);
	return_code = 0;
	if ((element_values1 =
		(struct FE_element_values_number *)element_values1_void) &&
		(element_values2 =
			(struct FE_element_values_number *)element_values2_void) &&
		(0 < (number_of_values = element_values1->number_of_values)) &&
		(number_of_values == element_values2->number_of_values))
	{
		for (i = number_of_values - 1; (!return_code) && (0 <= i); i--)
		{
			if (element_values1->values[i] < element_values2->values[i])
			{
				return_code = -1;
			}
			else if (element_values1->values[i] > element_values2->values[i])
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_FE_element_values_number_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* compare_FE_element_values_number_values */

struct FE_element_count_if_type_data
{
	enum CM_element_type cm_type;
	int number_of_elements;
};

static int FE_element_count_if_type(struct FE_element *element,
	void *count_data_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
If <element> is of the given CM_type, increment number_of_elements.
==============================================================================*/
{
	int return_code;
	struct FE_element_count_if_type_data *count_data;

	ENTER(FE_element_count_if_type);
	if (element && (count_data =
		(struct FE_element_count_if_type_data *)count_data_void))
	{
		return_code = 1;
		if (element->cm.type == count_data->cm_type)
		{
			(count_data->number_of_elements)++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_count_if_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_count_if_type */

struct FE_element_and_values_to_array_data
{
	enum CM_element_type cm_type;
	struct FE_element_values_number *element_values;
	struct Computed_field *sort_by_field;
}; /* FE_element_and_values_to_array_data */

static int FE_element_and_values_to_array(struct FE_element *element,
	void *array_data_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value xi[3] = {0.5,0.5,0.5};
	int return_code;
	struct FE_element_and_values_to_array_data *array_data;

	ENTER(FE_element_and_values_to_array);
	if (element && (array_data =
		(struct FE_element_and_values_to_array_data *)array_data_void) &&
		array_data->element_values)
	{
		return_code = 1;
		if (element->cm.type == array_data->cm_type)
		{
			array_data->element_values->element = element;
			if (array_data->sort_by_field)
			{
				if (!(array_data->element_values->values &&
					Computed_field_evaluate_in_element(array_data->sort_by_field, element,
						xi, /*top_level_element*/(struct FE_element *)NULL,
						array_data->element_values->values,
						/*derivatives*/(FE_value *)NULL)))
				{
					display_message(ERROR_MESSAGE, "FE_element_and_values_to_array.  "
						"sort_by field could not be evaluated in element");
					return_code = 0;
				}
			}
			(array_data->element_values)++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_and_values_to_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_and_values_to_array */

static int FE_element_manager_change_element_group_identifiers(
	struct MANAGER(FE_element) *element_manager,
	struct GROUP(FE_element) *element_group, enum CM_element_type cm_type,
	int element_offset, struct Computed_field *sort_by_field)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Changes the identifiers of all elements of <cm_type> in <element_group>.
If <sort_by_field> is NULL, adds <element_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated at the centre of all elements
in the group and the elements are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the elements in the group an
identifier already used by a element not in the group.
Caching the manager should be done outside this function so that more than one
group of elements can be renumbered at once.
Note function avoids iterating through the group as this is not allowed during
identifier changes.
==============================================================================*/
{
	int i, number_of_elements, number_of_values, return_code;
	struct CM_element_information cm, next_spare_element_identifier;
	struct FE_element *element_with_identifier;
	struct FE_element_and_values_to_array_data array_data;
	struct FE_element_count_if_type_data count_data;
	struct FE_element_values_number *element_values;

	ENTER(FE_element_manager_change_element_group_identifiers);
	if (element_manager && element_group)
	{
		return_code = 1;
		count_data.cm_type = cm_type;
		count_data.number_of_elements = 0;
		if (!FOR_EACH_OBJECT_IN_GROUP(FE_element)(FE_element_count_if_type,
			(void *)&count_data, element_group))
		{
			display_message(ERROR_MESSAGE,
				"FE_element_manager_change_element_group_identifiers.  "
				"Could not count elements of given type");
			return_code = 0;
		}
		number_of_elements = count_data.number_of_elements;
		if ((0 < number_of_elements) && return_code)
		{
			cm.type = cm_type;
			if (sort_by_field)
			{
				number_of_values =
					Computed_field_get_number_of_components(sort_by_field);
			}
			else
			{
				number_of_values = 0;
			}
			if (ALLOCATE(element_values, struct FE_element_values_number,
				number_of_elements))
			{
				for (i = 0; i < number_of_elements; i++)
				{
					element_values[i].number_of_values = number_of_values;
					element_values[i].values = (FE_value *)NULL;
				}
				if (sort_by_field)
				{
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						if (!ALLOCATE(element_values[i].values, FE_value, number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"FE_element_manager_change_element_group_identifiers.  "
								"Not enough memory");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* make a linear array of elements in the group in current order */
					array_data.element_values = element_values;
					array_data.sort_by_field = sort_by_field;
					array_data.cm_type = cm_type;
					if (!FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						FE_element_and_values_to_array, (void *)&array_data, element_group))
					{
						display_message(ERROR_MESSAGE,
							"FE_element_manager_change_element_group_identifiers.  "
							"Could not build element/field values array");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if (sort_by_field)
					{
						/* sort by field values with higher components more significant */
						qsort(element_values, number_of_elements,
							sizeof(struct FE_element_values_number),
							compare_FE_element_values_number_values);
						/* give the elements sequential values starting at element_offset */
						for (i = 0; i < number_of_elements; i++)
						{
							element_values[i].new_number = element_offset + i;
						}
					}
					else
					{
						/* offset element numbers by element_offset */
						for (i = 0; i < number_of_elements; i++)
						{
							element_values[i].new_number =
								element_values[i].element->cm.number + element_offset;
						}
					}
					/* check element numbers are positive and ascending */
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						if (0 >= element_values[i].new_number)
						{
							display_message(ERROR_MESSAGE,
								"FE_element_manager_change_element_group_identifiers.  "
								"element_offset would give negative element numbers");
							return_code = 0;
						}
						else if ((0 < i) && (element_values[i].new_number <=
							element_values[i - 1].new_number))
						{
							display_message(ERROR_MESSAGE,
								"FE_element_manager_change_element_group_identifiers.  "
								"Element numbers are not strictly increasing");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* check no new numbers are in use by elements not in element_group */
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						cm.number = element_values[i].new_number;
						if ((element_with_identifier =
							FIND_BY_IDENTIFIER_IN_MANAGER(FE_element, identifier)(
								&cm, element_manager)) &&
							(!IS_OBJECT_IN_GROUP(FE_element)(element_with_identifier,
								element_group)))
						{
							display_message(ERROR_MESSAGE,
								"FE_element_manager_change_element_group_identifiers.  "
								"Element using new number exists outside of group");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* change identifiers */
					/* maintain next_spare_element_number to renumber elements in same
						 group which already have the same number as the new_number */
					next_spare_element_identifier.type = cm_type;
					next_spare_element_identifier.number =
						element_values[number_of_elements - 1].new_number + 1;
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						cm.number = element_values[i].new_number;
						element_with_identifier =
							FIND_BY_IDENTIFIER_IN_GROUP(FE_element, identifier)(
								&cm, element_group);
						/* only modify if element doesn't already have correct identifier */
						if (element_with_identifier != element_values[i].element)
						{
							if (element_with_identifier)
							{
								while (((struct FE_element *)NULL !=
									FIND_BY_IDENTIFIER_IN_MANAGER(FE_element, identifier)(
										&next_spare_element_identifier, element_manager)))
								{
									next_spare_element_identifier.number++;
								}
								if (!MANAGER_MODIFY_IDENTIFIER(FE_element, identifier)(
									element_with_identifier, &next_spare_element_identifier,
									element_manager))
								{
									return_code = 0;
								}
							}
							if (!MANAGER_MODIFY_IDENTIFIER(FE_element, identifier)(
								element_values[i].element, &cm, element_manager))
							{
								display_message(ERROR_MESSAGE,
									"FE_element_manager_change_element_group_identifiers.  "
									"Could not change element identifier");
								return_code = 0;
							}
						}
					}
				}
				for (i = 0; i < number_of_elements; i++)
				{
					if (element_values[i].values)
					{
						DEALLOCATE(element_values[i].values);
					}
				}
				DEALLOCATE(element_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_manager_change_element_group_identifiers.  "
					"Not enough memory");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_manager_change_element_group_identifiers.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_manager_change_element_group_identifiers */

struct FE_node_values_number
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Data for changing node identifiers.
==============================================================================*/
{
	struct FE_node *node;
	int number_of_values;
	FE_value *values;
	int new_number;
};

static int compare_FE_node_values_number_values(
	const void *node_values1_void, const void *node_values2_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Compares the values in <node_values1> and <node_values2> from the last to the
first, returning -1 as soon as a value in <node_values1> is less than its
counterpart in <node_values2>, or 1 if greater. 0 is returned if all values
are identival. Used as a compare function for qsort.
==============================================================================*/
{
	int i, number_of_values, return_code;
	struct FE_node_values_number *node_values1, *node_values2;

	ENTER(compare_FE_node_values_number_values);
	return_code = 0;
	if ((node_values1 = (struct FE_node_values_number *)node_values1_void) &&
		(node_values2 = (struct FE_node_values_number *)node_values2_void) &&
		(0 < (number_of_values = node_values1->number_of_values)) &&
		(number_of_values == node_values2->number_of_values))
	{
		for (i = number_of_values - 1; (!return_code) && (0 <= i); i--)
		{
			if (node_values1->values[i] < node_values2->values[i])
			{
				return_code = -1;
			}
			else if (node_values1->values[i] > node_values2->values[i])
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_FE_node_values_number_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* compare_FE_node_values_number_values */

struct FE_node_and_values_to_array_data
{
	struct FE_node_values_number *node_values;
	struct Computed_field *sort_by_field;
}; /* FE_node_and_values_to_array_data */

static int FE_node_and_values_to_array(struct FE_node *node,
	void *array_data_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct FE_node_and_values_to_array_data *array_data;

	ENTER(FE_node_and_values_to_array);
	if (node && (array_data =
		(struct FE_node_and_values_to_array_data *)array_data_void) &&
		array_data->node_values)
	{
		return_code = 1;
		array_data->node_values->node = node;
		if (array_data->sort_by_field)
		{
			if (!(array_data->node_values->values && Computed_field_evaluate_at_node(
				array_data->sort_by_field, node, array_data->node_values->values)))
			{
				display_message(ERROR_MESSAGE, "FE_node_and_values_to_array.  "
					"sort_by field could not be evaluated at node");
				return_code = 0;
			}
		}
		(array_data->node_values)++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_and_values_to_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_and_values_to_array */

static int FE_node_manager_change_node_group_identifiers(
	struct MANAGER(FE_node) *node_manager, struct GROUP(FE_node) *node_group,
	int node_offset, struct Computed_field *sort_by_field)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Changes the identifiers of all nodes in <node_group>.
If <sort_by_field> is NULL, adds <node_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated for all nodes
in the group and the nodes are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the nodes in the group an
identifier already used by a node not in the group.
Caching the manager should be done outside this function so that more than one
group of nodes can be renumbered at once.
Note function avoids iterating through the group as this is not allowed during
identifier changes.
==============================================================================*/
{
	int i, next_spare_node_number, number_of_nodes, number_of_values, return_code;
	struct FE_node *node_with_identifier;
	struct FE_node_and_values_to_array_data array_data;
	struct FE_node_values_number *node_values;

	ENTER(FE_node_manager_change_node_group_identifiers);
	if (node_manager && node_group)
	{
		return_code = 1;
		number_of_nodes = NUMBER_IN_GROUP(FE_node)(node_group);
		if (0 < number_of_nodes)
		{
			if (sort_by_field)
			{
				number_of_values =
					Computed_field_get_number_of_components(sort_by_field);
			}
			else
			{
				number_of_values = 0;
			}
			if (ALLOCATE(node_values, struct FE_node_values_number,
				number_of_nodes))
			{
				for (i = 0; i < number_of_nodes; i++)
				{
					node_values[i].number_of_values = number_of_values;
					node_values[i].values = (FE_value *)NULL;
				}
				if (sort_by_field)
				{
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (!ALLOCATE(node_values[i].values, FE_value, number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"FE_node_manager_change_node_group_identifiers.  "
								"Not enough memory");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* make a linear array of the nodes in the group in current order */
					array_data.node_values = node_values;
					array_data.sort_by_field = sort_by_field;
					if (!FOR_EACH_OBJECT_IN_GROUP(FE_node)(FE_node_and_values_to_array,
						(void *)&array_data, node_group))
					{
						display_message(ERROR_MESSAGE,
							"FE_node_manager_change_node_group_identifiers.  "
							"Could not build node/field values array");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if (sort_by_field)
					{
						/* sort by field values with higher components more significant */
						qsort(node_values, number_of_nodes,
							sizeof(struct FE_node_values_number),
							compare_FE_node_values_number_values);
						/* give the nodes sequential values starting at node_offset */
						for (i = 0; i < number_of_nodes; i++)
						{
							node_values[i].new_number = node_offset + i;
						}
					}
					else
					{
						/* offset node numbers by node_offset */
						for (i = 0; i < number_of_nodes; i++)
						{
							node_values[i].new_number =
								get_FE_node_cm_node_identifier(node_values[i].node) +
								node_offset;
						}
					}
					/* check node numbers are positive and ascending */
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (0 >= node_values[i].new_number)
						{
							display_message(ERROR_MESSAGE,
								"FE_node_manager_change_node_group_identifiers.  "
								"node_offset would give negative node numbers");
							return_code = 0;
						}
						else if ((0 < i) &&
							(node_values[i].new_number <= node_values[i - 1].new_number))
						{
							display_message(ERROR_MESSAGE,
								"FE_node_manager_change_node_group_identifiers.  "
								"Node numbers are not strictly increasing");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* check no new numbers are in use by nodes not in node_group */
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if ((node_with_identifier =
							FIND_BY_IDENTIFIER_IN_MANAGER(FE_node, cm_node_identifier)(
								node_values[i].new_number, node_manager)) &&
							(!IS_OBJECT_IN_GROUP(FE_node)(node_with_identifier, node_group)))
						{
							display_message(ERROR_MESSAGE,
								"FE_node_manager_change_node_group_identifiers.  "
								"Node using new number exists outside of group");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* change identifiers */
					/* maintain next_spare_node_number to renumber nodes in same group
						 which already have the same number as the new_number */
					next_spare_node_number =
						node_values[number_of_nodes - 1].new_number + 1;
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						node_with_identifier =
							FIND_BY_IDENTIFIER_IN_GROUP(FE_node, cm_node_identifier)(
								node_values[i].new_number, node_group);
						/* only modify if node doesn't already have correct identifier */
						if (node_with_identifier != node_values[i].node)
						{
							if (node_with_identifier)
							{
								next_spare_node_number =
									get_next_FE_node_number(node_manager, next_spare_node_number);
								if (!MANAGER_MODIFY_IDENTIFIER(FE_node, cm_node_identifier)(
									node_with_identifier, next_spare_node_number, node_manager))
								{
									return_code = 0;
								}
							}
							if (!MANAGER_MODIFY_IDENTIFIER(FE_node, cm_node_identifier)(
								node_values[i].node, node_values[i].new_number, node_manager))
							{
								display_message(ERROR_MESSAGE,
									"FE_node_manager_change_node_group_identifiers.  "
									"Could not change node identifier");
								return_code = 0;
							}
						}
					}
				}
				for (i = 0; i < number_of_nodes; i++)
				{
					if (node_values[i].values)
					{
						DEALLOCATE(node_values[i].values);
					}
				}
				DEALLOCATE(node_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_manager_change_node_group_identifiers.  "
					"Not enough memory");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_manager_change_node_group_identifiers.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_manager_change_node_group_identifiers */

static int gfx_change_identifier(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
==============================================================================*/
{
	char data_flag, element_flag, face_flag, *group_name, line_flag, node_flag;
	int data_offset, element_offset, face_offset, line_offset, node_offset;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *sort_by_field;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group, *node_group;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_sort_by_field_data;

	ENTER(gfx_change_identifier);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		element_group = (struct GROUP(FE_element) *) NULL;
		data_flag = 0;
		data_offset = 0;
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		sort_by_field = (struct Computed_field *)NULL;

		option_table = CREATE(Option_table)();
		/* data_offset */
		Option_table_add_entry(option_table, "data_offset", &data_offset,
			&data_flag, set_int_and_char_flag);
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &element_group,
			command_data->element_group_manager, set_FE_element_group);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* sort_by */
		set_sort_by_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_sort_by_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_sort_by_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table, "sort_by", &sort_by_field,
			&set_sort_by_field_data, set_Computed_field_conditional);

		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (element_group)
			{
				GET_NAME(GROUP(FE_element))(element_group,&group_name);
				if (data_flag)
				{
					data_group = FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node), name)(
						group_name, command_data->data_group_manager);
					MANAGER_BEGIN_CACHE(FE_node)(command_data->data_manager);
					if (!FE_node_manager_change_node_group_identifiers(
						command_data->data_manager, data_group, data_offset, sort_by_field))
					{
						return_code = 0;
					}
					MANAGER_END_CACHE(FE_node)(command_data->data_manager);
				}
				if (element_flag || face_flag || line_flag)
				{
					MANAGER_BEGIN_CACHE(FE_element)(command_data->element_manager);
					if (element_flag)
					{
						if (!FE_element_manager_change_element_group_identifiers(
							command_data->element_manager, element_group, CM_ELEMENT,
							element_offset, sort_by_field))
						{
							return_code = 0;
						}
					}
					if (face_flag)
					{
						if (!FE_element_manager_change_element_group_identifiers(
							command_data->element_manager, element_group, CM_FACE,
							face_offset, sort_by_field))
						{
							return_code = 0;
						}
					}
					if (line_flag)
					{
						if (!FE_element_manager_change_element_group_identifiers(
							command_data->element_manager, element_group, CM_LINE,
							line_offset, sort_by_field))
						{
							return_code = 0;
						}
					}
					MANAGER_END_CACHE(FE_element)(command_data->element_manager);
				}
				if (node_flag)
				{
					node_group = FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node), name)(
						group_name, command_data->node_group_manager);
					MANAGER_BEGIN_CACHE(FE_node)(command_data->node_manager);
					if (!FE_node_manager_change_node_group_identifiers(
						command_data->node_manager, node_group, node_offset, sort_by_field))
					{
						return_code = 0;
					}
					MANAGER_END_CACHE(FE_node)(command_data->node_manager);
				}
				DEALLOCATE(group_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_change_identifier.  Must specify a group to change");
				return_code = 1;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		if (sort_by_field)
		{
			DEACCESS(Computed_field)(&sort_by_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_change_identifier.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_change_identifier */

static int gfx_create_annotation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2000

DESCRIPTION :
Executes a GFX CREATE ANNOTATION command. Creates a graphics object containing
a single point in 3-D space with a text string drawn beside it.
==============================================================================*/
{
	char *annotation_text,*graphics_object_name,**text;
	float time;
	int number_of_components,return_code;
	static char default_annotation_text[]="\"annotation text\"";
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct GT_pointset *point_set;
	struct Option_table *option_table;
	Triple *pointlist,position;

	ENTER(gfx_create_annotation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("annotation");
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			position[0]=0.0;
			position[1]=0.0;
			position[2]=0.0;
			if (ALLOCATE(annotation_text,char,strlen(default_annotation_text)+1))
			{
				strcpy(annotation_text,default_annotation_text);
			}
			else
			{
				annotation_text=(char *)NULL;
			}
			time=0.0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* position */
			number_of_components=3;
			Option_table_add_entry(option_table,"position",position,
				&number_of_components,set_float_vector);
			/* text */
			Option_table_add_entry(option_table,"text",&annotation_text,
				(void *)1,set_name);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			return_code=Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (annotation_text)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if (g_POINTSET==graphics_object->object_type)
						{
							if (GT_object_has_time(graphics_object,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
								return_code=GT_object_delete_time(graphics_object,time);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
							g_POINTSET,material))&&
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list)))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_points.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						/* create the pointset used to display the annotation */
						pointlist=(Triple *)NULL;
						text=(char **)NULL;
						point_set=(struct GT_pointset *)NULL;
						if (ALLOCATE(pointlist,Triple,1)&&ALLOCATE(text,char *,1))
						{
							*text = annotation_text;
							(*pointlist)[0]=position[0];
							(*pointlist)[1]=position[1];
							(*pointlist)[2]=position[2];
							if ((point_set=CREATE(GT_pointset)(1,pointlist,text,g_NO_MARKER,
								0.0,g_NO_DATA,(GTDATA *)NULL,(int *)NULL))&&
								GT_OBJECT_ADD(GT_pointset)(graphics_object,time,point_set))
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
							return_code=0;
						}
						if (return_code)
						{
							/* annotation string now owned by point set */
							annotation_text=(char *)NULL;
						}
						else
						{
							display_message(WARNING_MESSAGE,"Could not create annotation");
							if (point_set)
							{
								DESTROY(GT_pointset)(&point_set);
							}
							else
							{
								DEALLOCATE(pointlist);
								DEALLOCATE(text);
							}
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing annotation text");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (annotation_text)
			{
				DEALLOCATE(annotation_text);
			}
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_annotation.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_annotation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_annotation */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_colour_bar(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE COLOUR_BAR command. Creates a colour bar graphics object
with tick marks and labels for showing the scale of a spectrum.
==============================================================================*/
{
	char *graphics_object_name,*number_format;
	float bar_length,bar_radius,extend_length,tick_length;
	int number_of_components,return_code,tick_divisions;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *label_material,*material;
	struct GT_object *graphics_object;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	Triple bar_axis,bar_centre,side_axis;

	ENTER(gfx_create_colour_bar);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("colour_bar");
			number_format = duplicate_string("%+.4e");
			/* must access it now, because we deaccess it later */
			label_material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			number_of_components=3;
			bar_centre[0]=-0.9;
			bar_centre[1]=0.0;
			bar_centre[2]=0.5;
			bar_axis[0]=0.0;
			bar_axis[1]=1.0;
			bar_axis[2]=0.0;
			side_axis[0]=1.0;
			side_axis[1]=0.0;
			side_axis[2]=0.0;
			bar_length=1.6;
			extend_length=0.06;
			bar_radius=0.06;
			tick_length=0.04;
			tick_divisions=10;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* axis */
			Option_table_add_entry(option_table,"axis",bar_axis,
				&number_of_components,set_float_vector);
			/* centre */
			Option_table_add_entry(option_table,"centre",bar_centre,
				&number_of_components,set_float_vector);
			/* divisions */
			Option_table_add_entry(option_table,"divisions",&tick_divisions,
				NULL,set_int_non_negative);
			/* extend_length */
			Option_table_add_entry(option_table,"extend_length",&extend_length,
				NULL,set_float_non_negative);
			/* label_material */
			Option_table_add_entry(option_table,"label_material",&label_material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* length */
			Option_table_add_entry(option_table,"length",&bar_length,
				NULL,set_float_positive);
			/* number_format */
			Option_table_add_entry(option_table,"number_format",&number_format,
				(void *)1,set_name);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* radius */
			Option_table_add_entry(option_table,"radius",&bar_radius,
				NULL,set_float_positive);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* tick_direction */
			Option_table_add_entry(option_table,"tick_direction",side_axis,
				&number_of_components,set_float_vector);
			/* tick_length */
			Option_table_add_entry(option_table,"tick_length",&tick_length,
				NULL,set_float_non_negative);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (100 < tick_divisions)
				{
					display_message(WARNING_MESSAGE,"Limited to 100 tick_divisions");
					tick_divisions=100;
				}
				/* try to find existing colour_bar for updating */
				graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list);
				if (create_Spectrum_colour_bar(&graphics_object,
					graphics_object_name,spectrum,bar_centre,bar_axis,side_axis,
					bar_length,bar_radius,extend_length,tick_divisions,tick_length,
					number_format,material,label_material))
				{
					ACCESS(GT_object)(graphics_object);
					if (IS_OBJECT_IN_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list) ||
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_colour_bar.  Could not add graphics object to list");
						return_code=0;
					}
					DEACCESS(GT_object)(&graphics_object);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_colour_bar.  Could not create colour bar");
					return_code=0;
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			DEACCESS(Graphical_material)(&label_material);
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
			DEALLOCATE(number_format);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_colour_bar.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_colour_bar.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_colour_bar */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_cylinders(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE CYLINDERS command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name;
	float constant_radius,scale_factor,time;
	gtObject *graphics_object;
	int face_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Element_discretization discretization;
	struct Element_to_cylinder_data element_to_cylinder_data;
	struct Computed_field *coordinate_field,*data_field,*radius_field;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_radius_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_cylinders);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("cylinders");
			element_group=(struct GROUP(FE_element) *)NULL;
			constant_radius=0.0;
			scale_factor=1.0;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			radius_field=(struct Computed_field *)NULL;
			time=0;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=6;
			discretization.number_in_xi3=0;
			exterior_flag=0;
			face_number=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* constant_radius */
			Option_table_add_entry(option_table,"constant_radius",&constant_radius,
				NULL,set_float);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* radius_scalar */
			set_radius_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_radius_field_data.conditional_function=Computed_field_is_scalar;
			set_radius_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"radius_scalar",&radius_field,
				&set_radius_field_data,set_Computed_field_conditional);
			/* scale_factor */
			Option_table_add_entry(option_table,"scale_factor",&scale_factor,
				NULL,set_float);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_SURFACE==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_SURFACE,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_cylinders.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					element_to_cylinder_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_cylinder_data.element_group=element_group;
					element_to_cylinder_data.exterior=exterior_flag;
					element_to_cylinder_data.face_number=face_number;
					/* radius = constant_radius + scale_factor*radius_scalar */
					element_to_cylinder_data.constant_radius=constant_radius;
					element_to_cylinder_data.radius_field=radius_field;
					element_to_cylinder_data.scale_factor=scale_factor;
					element_to_cylinder_data.graphics_object=graphics_object;
					element_to_cylinder_data.time=time;
					element_to_cylinder_data.material=material;
					element_to_cylinder_data.data_field=data_field;
					element_to_cylinder_data.number_of_segments_along=
						discretization.number_in_xi1;
					element_to_cylinder_data.number_of_segments_around=
						discretization.number_in_xi2;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_cylinder,(void *)&element_to_cylinder_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_cylinder,(void *)&element_to_cylinder_data,
							command_data->element_manager);
					}
					if (return_code)
					{
						if (!GT_object_has_time(graphics_object,time))
						{
							/* add a NULL surface to make an empty time */
							GT_OBJECT_ADD(GT_surface)(graphics_object,time,
								(struct GT_surface *)NULL);
						}
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No cylinders created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(&(element_to_cylinder_data.coordinate_field));
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (radius_field)
			{
				DEACCESS(Computed_field)(&radius_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_cylinders.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_cylinders.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_cylinders */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_CREATOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->element_creator)
				{
					return_code=Element_creator_bring_window_to_front(
						command_data->element_creator);
				}
				else
				{
					if (CREATE(Element_creator)(&(command_data->element_creator),
						command_data->basis_manager,
						command_data->element_manager,command_data->element_group_manager,
						command_data->fe_field_manager,
						command_data->node_manager,command_data->node_group_manager,
						command_data->element_selection,command_data->node_selection,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_creator.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_element_creator.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_creator */

static int gfx_create_element_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2001

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINTS command.
==============================================================================*/
{
	char exterior_flag, *graphics_object_name, *use_element_type_string,
		**valid_strings, *xi_discretization_mode_string;
	enum Use_element_type use_element_type;
	enum Xi_discretization_mode xi_discretization_mode;
	float time;
	int face_number,number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field, *data_field, *label_field,
		*orientation_scale_field, *variable_scale_field, *xi_point_density_field;
	struct Element_discretization discretization;
	struct Element_to_glyph_set_data element_to_glyph_set_data;
	struct FE_field *native_discretization_field;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct GT_object *glyph,*graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data, set_xi_point_density_field_data;
	Triple exact_xi,glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_element_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state &&
		(command_data=(struct Cmiss_command_data *)command_data_void)&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package)))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("element_points");
		number_of_components=3;
		exact_xi[0]=0.5;
		exact_xi[1]=0.5;
		exact_xi[2]=0.5;
		glyph_centre[0]=0.0;
		glyph_centre[1]=0.0;
		glyph_centre[2]=0.0;
		coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			"default_coordinate",computed_field_manager);
		ACCESS(Computed_field)(coordinate_field);
		data_field = (struct Computed_field *)NULL;
		xi_point_density_field = (struct Computed_field *)NULL;
		exterior_flag=0;
		face_number=0;
		element_group=(struct GROUP(FE_element) *)NULL;
		if (glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
			command_data->glyph_list))
		{
			ACCESS(GT_object)(glyph);
		}
		label_field=(struct Computed_field *)NULL;
		material=
			ACCESS(Graphical_material)(command_data->default_graphical_material);
		native_discretization_field=(struct FE_field *)NULL;
		orientation_scale_field = (struct Computed_field *)NULL;
		variable_scale_field = (struct Computed_field *)NULL;
		glyph_scale_factors[0]=1.0;
		glyph_scale_factors[1]=1.0;
		glyph_scale_factors[2]=1.0;
		glyph_size[0]=1.0;
		glyph_size[1]=1.0;
		glyph_size[2]=1.0;
		spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
		time=0.0;
		discretization.number_in_xi1=1;
		discretization.number_in_xi2=1;
		discretization.number_in_xi3=1;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* cell_centres/cell_corners/cell_random */ 
		xi_discretization_mode = XI_DISCRETIZATION_CELL_CENTRES;
		xi_discretization_mode_string =
			ENUMERATOR_STRING(Xi_discretization_mode)(xi_discretization_mode);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Xi_discretization_mode)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Xi_discretization_mode) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&xi_discretization_mode_string);
		DEALLOCATE(valid_strings);
		/* centre [of glyph] */
		Option_table_add_entry(option_table,"centre",glyph_centre,
			&(number_of_components),set_float_vector);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* density */
		set_xi_point_density_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_xi_point_density_field_data.conditional_function =
			Computed_field_is_scalar;
		set_xi_point_density_field_data.conditional_function_user_data =
			(void *)NULL;
		Option_table_add_entry(option_table, "density",
			&xi_point_density_field, &set_xi_point_density_field_data,
			set_Computed_field_conditional);
		/* exterior */
		Option_table_add_entry(option_table,"exterior",&exterior_flag,
			NULL,set_char_flag);
		/* face */
		Option_table_add_entry(option_table,"face",&face_number,
			NULL,set_exterior);
		/* from [element_group] */
		Option_table_add_entry(option_table,"from",&element_group,
			command_data->element_group_manager,set_FE_element_group);
		/* glyph */
		Option_table_add_entry(option_table,"glyph",&glyph,
			command_data->glyph_list,set_Graphics_object);
		/* label */
		set_label_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_label_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_label_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"label",&label_field,
			&set_label_field_data,set_Computed_field_conditional);
		/* material */
		Option_table_add_entry(option_table,"material",&material,
			command_data->graphical_material_manager,set_Graphical_material);
		/* native_discretization */
		Option_table_add_entry(option_table,"native_discretization",
			&native_discretization_field,command_data->fe_field_manager,
			set_FE_field);
		/* orientation */
		set_orientation_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_orientation_scale_field_data.conditional_function=
			Computed_field_is_orientation_scale_capable;
		set_orientation_scale_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"orientation",
			&orientation_scale_field,&set_orientation_scale_field_data,
			set_Computed_field_conditional);
		/* scale_factors */
		Option_table_add_entry(option_table,"scale_factors",
			glyph_scale_factors,"*",set_special_float3);
		/* size [of glyph] */
		Option_table_add_entry(option_table,"size",
			glyph_size,"*",set_special_float3);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",
			&spectrum,command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* use_elements/use_faces/use_lines */
		use_element_type = USE_ELEMENTS;
		use_element_type_string =
			ENUMERATOR_STRING(Use_element_type)(use_element_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&use_element_type_string);
		DEALLOCATE(valid_strings);
		/* variable_scale */
		set_variable_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_variable_scale_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_variable_scale_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"variable_scale", &variable_scale_field,
			&set_variable_scale_field_data, set_Computed_field_conditional);
		/* with */
		Option_table_add_entry(option_table,"with",&discretization,
			command_data->user_interface,set_Element_discretization);
		/* xi */
		Option_table_add_entry(option_table,"xi",
			exact_xi,&number_of_components,set_float_vector);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			face_number -= 2;
			STRING_TO_ENUMERATOR(Xi_discretization_mode)(
				xi_discretization_mode_string, &xi_discretization_mode);
			if (((XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode)) &&
				((struct Computed_field *)NULL == xi_point_density_field))
			{
				display_message(ERROR_MESSAGE,
					"No density field specified for cell_density|cell_poisson");
				return_code = 0;
			}
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_GLYPH_SET == graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_GLYPH_SET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_points.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				element_to_glyph_set_data.time=time;
				element_to_glyph_set_data.xi_discretization_mode =
					xi_discretization_mode;
				element_to_glyph_set_data.xi_point_density_field =
					xi_point_density_field;
				element_to_glyph_set_data.coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				if (orientation_scale_field)
				{
					element_to_glyph_set_data.orientation_scale_field=
						Computed_field_begin_wrap_orientation_scale_field(
							orientation_scale_field,
							element_to_glyph_set_data.coordinate_field);
				}
				else
				{
					element_to_glyph_set_data.orientation_scale_field=
						(struct Computed_field *)NULL;
				}
				STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
					&use_element_type);
				element_to_glyph_set_data.use_element_type = use_element_type;
				element_to_glyph_set_data.element_group=element_group;
				element_to_glyph_set_data.exterior=exterior_flag;
				element_to_glyph_set_data.face_number=face_number;
				element_to_glyph_set_data.number_in_xi[0]=
					discretization.number_in_xi1;
				element_to_glyph_set_data.number_in_xi[1]=
					discretization.number_in_xi2;
				element_to_glyph_set_data.number_in_xi[2]=
					discretization.number_in_xi3;
				element_to_glyph_set_data.variable_scale_field = variable_scale_field;
				element_to_glyph_set_data.data_field = data_field;
				element_to_glyph_set_data.label_field = label_field;
				element_to_glyph_set_data.native_discretization_field =
					native_discretization_field;
				element_to_glyph_set_data.glyph = glyph;
				element_to_glyph_set_data.graphics_object=graphics_object;
				element_to_glyph_set_data.exact_xi[0] = exact_xi[0];
				element_to_glyph_set_data.exact_xi[1] = exact_xi[1];
				element_to_glyph_set_data.exact_xi[2] = exact_xi[2];
				element_to_glyph_set_data.base_size[0] = (FE_value)(glyph_size[0]);
				element_to_glyph_set_data.base_size[1] = (FE_value)(glyph_size[1]);
				element_to_glyph_set_data.base_size[2] = (FE_value)(glyph_size[2]);
				element_to_glyph_set_data.centre[0] = (FE_value)(glyph_centre[0]);
				element_to_glyph_set_data.centre[1] = (FE_value)(glyph_centre[1]);
				element_to_glyph_set_data.centre[2] = (FE_value)(glyph_centre[2]);
				element_to_glyph_set_data.scale_factors[0] =
					(FE_value)(glyph_scale_factors[0]);
				element_to_glyph_set_data.scale_factors[1] =
					(FE_value)(glyph_scale_factors[1]);
				element_to_glyph_set_data.scale_factors[2] =
					(FE_value)(glyph_scale_factors[2]);
				element_to_glyph_set_data.select_mode = GRAPHICS_NO_SELECT;
				if (element_group)
				{
					return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						element_to_glyph_set,(void *)&element_to_glyph_set_data,
						element_group);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						element_to_glyph_set,(void *)&element_to_glyph_set_data,
						command_data->element_manager);
				}
				if (return_code)
				{
					if (!GT_object_has_time(graphics_object,time))
					{
						/* add a NULL glyph_set to make an empty time */
						GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,
							(struct GT_glyph_set *)NULL);
					}
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No element_points created");
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
				if (element_to_glyph_set_data.orientation_scale_field)
				{
					Computed_field_end_wrap(
						&(element_to_glyph_set_data.orientation_scale_field));
				}
				Computed_field_end_wrap(
					&(element_to_glyph_set_data.coordinate_field));
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
		if (glyph)
		{
			DEACCESS(GT_object)(&glyph);
		}
		if (orientation_scale_field)
		{
			DEACCESS(Computed_field)(&orientation_scale_field);
		}
		if (variable_scale_field)
		{
			DEACCESS(Computed_field)(&variable_scale_field);
		}
		if (native_discretization_field)
		{
			DEACCESS(FE_field)(&native_discretization_field);
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		if (xi_point_density_field)
		{
			DEACCESS(Computed_field)(&xi_point_density_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_points */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_element_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX CREATE EGROUP command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Add_FE_element_to_list_if_in_range_data element_in_range_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group,*from_element_group;
	struct GROUP(FE_node) *data_group,*node_group;
	struct LIST(FE_node) *node_list;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_element_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		name=(char *)NULL;
		if (set_name(state,(void *)&name,(void *)1))
		{
			/* initialise defaults */
			from_element_group=(struct GROUP(FE_element) *)NULL;
			element_in_range_data.element_list=CREATE(LIST(FE_element))();
			element_in_range_data.element_ranges=CREATE(Multi_range)();
			(option_table[0]).to_be_modified=element_in_range_data.element_ranges;
			(option_table[1]).to_be_modified=&from_element_group;
			(option_table[1]).user_data=command_data->element_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name&&!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
					command_data->data_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
						command_data->node_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(name,
						command_data->element_group_manager))
				{
					/* create node and element groups of same name simultaneously */
					data_group=CREATE_GROUP(FE_node)(name);
					node_group=CREATE_GROUP(FE_node)(name);
					element_group=CREATE_GROUP(FE_element)(name);
					if (data_group&&node_group&&element_group)
					{
						if (0<Multi_range_get_number_of_ranges(
							element_in_range_data.element_ranges))
						{
							/* make list of elements to add to group, then add to group */
							if (from_element_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									add_FE_element_to_list_if_in_range,
									(void *)&element_in_range_data,from_element_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									add_FE_element_to_list_if_in_range,
									(void *)&element_in_range_data,command_data->element_manager);
							}
							if (return_code)
							{
								/* also fill node_group with nodes used by these elements */
								node_list=CREATE(LIST(FE_node))();
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
									ensure_FE_element_and_faces_are_in_group,
									(void *)element_group,element_in_range_data.element_list)&&
									FOR_EACH_OBJECT_IN_LIST(FE_element)(
										ensure_top_level_FE_element_nodes_are_in_list,
										(void *)node_list,element_in_range_data.element_list)&&
									FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_group,
										(void *)node_group,node_list);
								DESTROY(LIST(FE_node))(&node_list);
							}
						}
						if (return_code)
						{
							/* must add node group before element group so the node group
								 exists when a GT_element_group is made for the element group */
							if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(node_group,
									command_data->node_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(element_group,
									command_data->element_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
									"Could not add data/node/element group(s) to manager");
								DESTROY_GROUP(FE_element)(&element_group);
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->node_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
										command_data->node_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&node_group);
								}
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->data_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
										command_data->data_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&data_group);
								}
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
								"Could not add fill node/element group(s)");
							DESTROY(GROUP(FE_element))(&element_group);
							DESTROY(GROUP(FE_node))(&node_group);
							DESTROY(GROUP(FE_node))(&data_group);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
							"Could not create data/node/element group(s)");
						DESTROY(GROUP(FE_element))(&element_group);
						DESTROY(GROUP(FE_node))(&node_group);
						DESTROY(GROUP(FE_node))(&data_group);
						return_code=0;
					}
				}
				else
				{
					if (name)
					{
						display_message(ERROR_MESSAGE,"gfx_create_element_group.  "
							"Data/node/element group '%s' already exists",name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_group.  Must specify a name for the group");
					}
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&element_in_range_data.element_ranges);
			DESTROY(LIST(FE_element))(&element_in_range_data.element_list);
			if (from_element_group)
			{
				DEACCESS(GROUP(FE_element))(&from_element_group);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 October 1996

DESCRIPTION :
Executes a GFX CREATE ENVIRONMENT_MAP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;
	struct Modify_environment_map_data modify_environment_map_data;

	ENTER(gfx_create_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,name)(
						current_token,command_data->environment_map_manager))
					{
						if (environment_map=CREATE(Environment_map)(current_token))
						{
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_environment_map_data.graphical_material_manager=
									command_data->graphical_material_manager;
								modify_environment_map_data.environment_map_manager=
									command_data->environment_map_manager;
								return_code=modify_Environment_map(state,
									(void *)environment_map,
									(void *)(&modify_environment_map_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Environment_map)(environment_map,
								command_data->environment_map_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_environment_map.  Error creating environment_map");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Environment map already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_environment_map_data.graphical_material_manager=
						command_data->graphical_material_manager;
					modify_environment_map_data.environment_map_manager=
						command_data->environment_map_manager;
					return_code=modify_Environment_map(state,
						(void *)NULL,(void *)(&modify_environment_map_data));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_environment_map.  Missing command_data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment_map_name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_environment_map */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int set_intersect(struct Parse_state *state,
	void *intersect_mu_theta_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 27 June 1996

DESCRIPTION :
A modifier function for setting mu and theta (prolate spheroidal coordinates)
for intersect - look at how fibre angle varies through the heart wall.
???DB.  Specialist
==============================================================================*/
{
	char *current_token;
	float *intersect_mu_theta;
	int return_code;

	ENTER(set_intersect);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (intersect_mu_theta=(float *)intersect_mu_theta_void)
				{
					/* turn on intersect */
					*intersect_mu_theta=1;
					if (1==sscanf(current_token,"%f",intersect_mu_theta+1))
					{
						shift_Parse_state(state,1);
						if (current_token=state->current_token)
						{
							if (1==sscanf(current_token,"%f",intersect_mu_theta+2))
							{
								shift_Parse_state(state,1);
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid theta: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing theta");
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid mu: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_intersect.  Missing intersect_mu_theta");
					return_code=0;
				}
			}
			else
			{
				if (intersect_mu_theta=(float *)intersect_mu_theta_void)
				{
					display_message(INFORMATION_MESSAGE," MU[%g] THETA[%g]",
						intersect_mu_theta[1],intersect_mu_theta[2]);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MU THETA");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing mu");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_intersect.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_intersect */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE FLOW_PARTICLES command.
==============================================================================*/
{
	char *graphics_object_name;
	float time,xi[3];
	gtObject *graphics_object;
	struct GT_pointset *pointset;
	int element_number,i,number_of_particles,return_code,
		vector_components;
	struct Cmiss_command_data *command_data;
	struct Element_to_particle_data element_to_particle_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	Triple *particle_positions,*final_particle_positions;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"element",NULL,NULL,set_int_non_negative},
		{"from",NULL,NULL,set_FE_element_group},
		{"initial_xi",NULL,NULL,set_FE_value_array},
		{"material",NULL,NULL,set_Graphical_material},
		{"spectrum",NULL,NULL,set_Spectrum},
		{"time",NULL,NULL,set_float},
		{"vector",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_create_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("particles");
			element_group=(struct GROUP(FE_element) *)NULL;
			element_number=0;  /* Zero gives all elements in group */
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			vector_components=3;
			xi[0]=0.5;
			xi[1]=0.5;
			xi[2]=0.5;
			time=0;
			/* must access it now,because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);
			i=0;
			/* as */
			(option_table[i]).to_be_modified= &graphics_object_name;
			i++;
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &coordinate_field;
			(option_table[i]).user_data= &set_coordinate_field_data;
			i++;
			/* element */
			(option_table[i]).to_be_modified= &element_number;
			i++;
			/* from */
			(option_table[i]).to_be_modified= &element_group;
			(option_table[i]).user_data=command_data->element_group_manager;
			i++;
			/* initial_xi */
			(option_table[i]).to_be_modified=xi;
			(option_table[i]).user_data= &(vector_components);
			i++;
			/* material */
			(option_table[i]).to_be_modified= &material;
			(option_table[i]).user_data=command_data->graphical_material_manager;
			i++;
			/* spectrum */
			(option_table[i]).to_be_modified= &spectrum;
			(option_table[i]).user_data=command_data->spectrum_manager;
			i++;
			/* time */
			(option_table[i]).to_be_modified= &time;
			i++;
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &stream_vector_field;
			(option_table[i]).user_data= &set_stream_vector_field_data;
			i++;
			return_code=process_multiple_options(state,option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles.  Object already exists");
					return_code=0;
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_POINTSET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					if (element_group)
					{
						number_of_particles=NUMBER_IN_GROUP(FE_element)(element_group);
					}
					else
					{
						number_of_particles=
							NUMBER_IN_MANAGER(FE_element)(command_data->element_manager);
					}
					if (ALLOCATE(particle_positions,Triple,number_of_particles))
					{
						if (pointset=CREATE(GT_pointset)(number_of_particles,
							particle_positions,(char **)NULL,	g_POINT_MARKER,
							1,g_NO_DATA,(GTDATA *) NULL,(int *)NULL))
						{
							element_to_particle_data.coordinate_field=coordinate_field;
							element_to_particle_data.element_number=element_number;
							element_to_particle_data.stream_vector_field=stream_vector_field;
							element_to_particle_data.pointlist= &(pointset->pointlist);
							element_to_particle_data.index=0;
							element_to_particle_data.graphics_object=graphics_object;
							element_to_particle_data.xi[0]=xi[0];
							element_to_particle_data.xi[1]=xi[1];
							element_to_particle_data.xi[2]=xi[2];
							element_to_particle_data.number_of_particles=0;
							element_to_particle_data.list= &(command_data->streampoint_list);
							if (element_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									element_to_particle,(void *)&element_to_particle_data,
									element_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									element_to_particle,(void *)&element_to_particle_data,
									command_data->element_manager);
							}
							number_of_particles=element_to_particle_data.number_of_particles;
							if (return_code && number_of_particles &&
								REALLOCATE(final_particle_positions,particle_positions,
								Triple,number_of_particles))
							{
								pointset->pointlist=final_particle_positions;
								pointset->n_pts=number_of_particles;
								if (GT_OBJECT_ADD(GT_pointset)(graphics_object,time,pointset))
								{
									return_code=set_GT_object_Spectrum(graphics_object,spectrum);
								}
								else
								{
									DESTROY(GT_pointset)(&pointset);
									display_message(ERROR_MESSAGE,
			"gfx_create_flow_particles.  Could not add pointset to graphics object");
									return_code=0;
								}
							}
							else
							{
								DESTROY(GT_pointset)(&pointset);
								if (return_code)
								{
									display_message(WARNING_MESSAGE,"No particles created");
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx_create_flow_particles.  Error creating particles");
								}
							}
						}
						else
						{
							DEALLOCATE (particle_positions);
							display_message(ERROR_MESSAGE,
								"gfx_create_flow_particles.  Unable to create pointset");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"gfx_create_flow_particles.  Unable to allocate memory for particle positions");
						return_code=0;
					}
				}
			} /* parse error,help */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_flow_particles.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_create_flow_particles */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_more_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE MORE_FLOW_PARTICLES command.
==============================================================================*/
{
	char *graphics_object_name;
	float time,xi[3];
	gtObject *graphics_object;
	int current_number_of_particles,element_number,i,number_of_particles,
		return_code,vector_components;
	struct Cmiss_command_data *command_data;
	struct Element_to_particle_data element_to_particle_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	Triple *new_particle_positions,*particle_positions,
		*final_particle_positions;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"element",NULL,NULL,set_int_non_negative},
		{"from",NULL,NULL,set_FE_element_group},
		{"initial_xi",NULL,NULL,set_float_vector},
		{"material",NULL,NULL,set_Graphical_material},
		{"spectrum",NULL,NULL,set_Spectrum},
		{"time",NULL,NULL,set_float},
		{"vector",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_create_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("particles");
			element_group=(struct GROUP(FE_element) *)NULL;
			element_number=0;  /* Zero gives all elements in group */
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			vector_components=3;
			xi[0]=0.5;
			xi[1]=0.5;
			xi[2]=0.5;
			time=0;
			/* must access it now,because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);
			i=0;
			/* as */
			(option_table[i]).to_be_modified= &graphics_object_name;
			i++;
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &coordinate_field;
			(option_table[i]).user_data= &set_coordinate_field_data;
			i++;
			/* element */
			(option_table[i]).to_be_modified= &element_number;
			i++;
			/* from */
			(option_table[i]).to_be_modified= &element_group;
			(option_table[i]).user_data=command_data->element_group_manager;
			i++;
			/* initial_xi */
			(option_table[i]).to_be_modified=xi;
			(option_table[i]).user_data= &(vector_components);
			i++;
			/* material */
			(option_table[i]).to_be_modified= &material;
			(option_table[i]).user_data=command_data->graphical_material_manager;
			i++;
			/* spectrum */
			(option_table[i]).to_be_modified= &spectrum;
			(option_table[i]).user_data=command_data->spectrum_manager;
			i++;
			/* time */
			(option_table[i]).to_be_modified= &time;
			i++;
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[i]).to_be_modified= &stream_vector_field;
			(option_table[i]).user_data= &set_stream_vector_field_data;
			i++;
			return_code=process_multiple_options(state,option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					particle_positions=(*((graphics_object->gu).gt_pointset))->pointlist;
					current_number_of_particles=
						(*((graphics_object->gu).gt_pointset))->n_pts;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles.  Graphics object does not exist");
					return_code=0;
				}
				if (return_code)
				{
					if (element_group)
					{
						number_of_particles=current_number_of_particles +
							NUMBER_IN_GROUP(FE_element)(element_group);
					}
					else
					{
						number_of_particles=current_number_of_particles +
							NUMBER_IN_MANAGER(FE_element)(command_data->element_manager);
					}
					if (REALLOCATE(new_particle_positions,particle_positions,Triple,
						number_of_particles))
					{
						(*((graphics_object->gu).gt_pointset))->pointlist=
							new_particle_positions;
						element_to_particle_data.coordinate_field=coordinate_field;
						element_to_particle_data.element_number=element_number;
						element_to_particle_data.stream_vector_field=stream_vector_field;
						element_to_particle_data.index=current_number_of_particles;
						element_to_particle_data.pointlist=
							&((*((graphics_object->gu).gt_pointset))->pointlist);
						element_to_particle_data.graphics_object=graphics_object;
						element_to_particle_data.xi[0]=xi[0];
						element_to_particle_data.xi[1]=xi[1];
						element_to_particle_data.xi[2]=xi[2];
						element_to_particle_data.number_of_particles=0;
						element_to_particle_data.list= &(command_data->streampoint_list);
						if (element_group)
						{
							return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								element_to_particle,(void *)&element_to_particle_data,
								element_group);
						}
						else
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								element_to_particle,(void *)&element_to_particle_data,
								command_data->element_manager);
						}
						number_of_particles=current_number_of_particles +
							element_to_particle_data.number_of_particles;
						if (return_code && number_of_particles &&
							REALLOCATE(final_particle_positions,new_particle_positions,
								Triple,number_of_particles))
						{
							(*((graphics_object->gu).gt_pointset))->pointlist=
								final_particle_positions;
							(*((graphics_object->gu).gt_pointset))->n_pts=number_of_particles;
							GT_object_changed(graphics_object);
						}
						else
						{
							DEALLOCATE(particle_positions);
							if (return_code)
							{
								display_message(WARNING_MESSAGE,"No particles created");
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_more_flow_particles.  Error creating particles");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"gfx_create_more_flow_particles.  Unable to allocate memory for particle positions");
					}
				}
			} /* parse error,help */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_more_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_more_flow_particles.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_create_more_flow_particles */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX MODIFY FLOW_PARTICLES command.
==============================================================================*/
{
	int return_code;
	float stepsize,time;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_modify_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			stepsize=1;
			/* If time of 0 is sent the previous points are updated at the previous
				time value */
			time=0;

			option_table=CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* stepsize */
			Option_table_add_entry(option_table,"stepsize",&stepsize,
				NULL,set_float);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code=update_flow_particle_list(
					command_data->streampoint_list,coordinate_field,stream_vector_field,
					stepsize,time);
			}
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_flow_particles.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_flow_particles */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_g_element_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Executes a GFX CREATE G_ELEMENT_EDITOR command.
Invokes the graphical element group editor.
==============================================================================*/
{
	int return_code;
	struct GROUP(FE_element) *element_group;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;

	ENTER(gfx_create_g_element_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			element_group = (struct GROUP(FE_element) *)NULL;
			scene = (struct Scene *)NULL;
			if (command_data->graphical_element_editor_dialog)
			{
				Graphical_element_editor_dialog_get_element_group_and_scene(
					command_data->graphical_element_editor_dialog,&element_group,&scene);
			}
			if (!element_group)
			{
				element_group = FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_element))(
					(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_element)) *)NULL, (void *)NULL,
					command_data->element_group_manager);
			}
			if (!scene)
			{
				scene = command_data->default_scene;
			}
			if (element_group)
			{
				ACCESS(GROUP(FE_element))(element_group);
			}
			if (scene)
			{
				ACCESS(Scene)(scene);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->scene_manager,set_Scene);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (command_data->graphical_element_editor_dialog)
				{
					Graphical_element_editor_dialog_set_element_group_and_scene(
						command_data->graphical_element_editor_dialog,element_group,scene);
					Graphical_element_editor_dialog_bring_to_front(
						command_data->graphical_element_editor_dialog);
				}
				else
				{
					if ((!command_data->user_interface) ||
						(!CREATE(Graphical_element_editor_dialog)(
						&(command_data->graphical_element_editor_dialog),
						command_data->user_interface->application_shell,
						command_data->computed_field_package,
						command_data->element_manager,
						command_data->element_group_manager,element_group,
						command_data->fe_field_manager,
						command_data->graphical_material_manager,
						command_data->default_graphical_material,command_data->glyph_list,
						command_data->scene_manager,scene,command_data->spectrum_manager,
						command_data->default_spectrum,command_data->volume_texture_manager,
						command_data->user_interface)))
					{
						display_message(ERROR_MESSAGE,"gfx_create_g_element_editor.  "
							"Could not create graphical element editor");
						return_code=0;
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_g_element_editor.  "
				"Missing command_data.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_g_element_editor.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_g_element_editor */

static int gfx_create_graphical_material_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE GRAPHICAL_MATERIAL_EDITOR command.
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_material_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_material_editor_dialog(
					&(command_data->material_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->graphical_material_manager,
					command_data->texture_manager,(struct Graphical_material *)NULL,
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_material_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_material_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_material_editor */

static int gfx_create_grid_field_calculator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 December 1999

DESCRIPTION :
Executes a GFX CREATE GRID_FIELD_CALCULATOR command.
Invokes the grid field calculator dialog.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_grid_field_calculator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				return_code=bring_up_grid_field_calculator(
					&(command_data->grid_field_calculator_dialog),
					command_data->user_interface->application_shell,
					command_data->computed_field_package,
					&(command_data->control_curve_editor_dialog),
					command_data->control_curve_manager,
					command_data->element_manager,
					command_data->user_interface);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_grid_field_calculator.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_grid_field_calculator.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_grid_field_calculator */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_input_module_control(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE IM_CONTROL command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_input_module_control);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
#if defined (EXT_INPUT)
				return_code=bring_up_input_module_dialog(
					&(command_data->input_module_dialog),
					command_data->user_interface->application_shell,
					command_data->default_graphical_material,
					command_data->graphical_material_manager,command_data->default_scene,
					command_data->scene_manager);
					/*???DB.  commmand_data should not be used outside of command.c */
#else /* defined (EXT_INPUT) */
				display_message(ERROR_MESSAGE,"External input module was not linked");
#endif /* defined (EXT_INPUT) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_input_module_control.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_input_module_control.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_input_module_control */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_interactive_data_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE INTERACTIVE_DATA_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_interactive_data_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_interactive_node_editor_dialog(
					&(command_data->interactive_data_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->data_manager,command_data->execute_command,
					(struct FE_node *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_interactive_data_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_interactive_data_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_interactive_data_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_interactive_node_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE INTERACTIVE_NODE_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_interactive_node_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_interactive_node_editor_dialog(
					&(command_data->interactive_node_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->node_manager,command_data->execute_command,
					(struct FE_node *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_interactive_node_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_interactive_node_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_interactive_node_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
static int set_iso_field_with_floats(struct Parse_state *state,
	void *iso_field_data_ptr_void,void *type_user_data)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
A modifier function to set the iso_field_calculation_type and the associated
float parameters.
==============================================================================*/
{
	char *current_token;
	float coeffs[3], *trace_coeffs;
	int return_code;
	struct Iso_field_calculation_data *data;
	enum Iso_field_calculation_type type;
	int number_of_points, i;

	ENTER(set_iso_field_with_floats);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (data=
					*((struct Iso_field_calculation_data **)iso_field_data_ptr_void))
				{
					if (type= *((enum Iso_field_calculation_type *)type_user_data))
					{
						switch (type)
						{
							case COORDINATE_PLANE:
							case COORDINATE_SPHERE:
							{
								if ((1==sscanf(current_token," %f ",&(coeffs[0])))&&
									shift_Parse_state(state,1)&&(state->current_token)&&
									(1==sscanf(state->current_token," %f ",&(coeffs[1])))&&
									shift_Parse_state(state,1)&&(state->current_token)&&
									(1==sscanf(state->current_token," %f ",&(coeffs[2]))))
								{
									set_Iso_field_calculation_with_floats(data,type,3,coeffs);
									return_code=shift_Parse_state(state,1);
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Missing/invalid coeff value(s)");
									display_parse_state_location(state);
									return_code=0;
								}
							} break;
							case VERTICAL_POINT_TRACE:
							{
								if ((1==sscanf(current_token," %d ",&(number_of_points)))&&
									(number_of_points>1))
								{
									return_code=1;
									if (ALLOCATE(trace_coeffs,FE_value,number_of_points*2))
									{
										i=0;
										while (return_code&&(i<number_of_points*2))
										{
											if (!(shift_Parse_state(state,1)&&(state->current_token)&&
												(1==sscanf(state->current_token," %f ",
												&(trace_coeffs[i])))))
											{
												display_message(WARNING_MESSAGE,
													"Missing/invalid trace point value(s)");
												display_parse_state_location(state);
												return_code=0;
											}
											i++;
										}
										if (return_code)
										{
											set_Iso_field_calculation_with_floats(data,type,
												number_of_points*2,trace_coeffs);
											return_code=shift_Parse_state(state,1);
										}
										DEALLOCATE(trace_coeffs);
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Missing/invalid number of points.");
									display_parse_state_location(state);
									return_code=0;
								}
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_iso_field_with_floats.  Invalid iso_field_calculation_type");
						return_code=0;
					}

				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_iso_field_with_floats.  Missing iso_field_data structure");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," Value 1# Value 2# Value 3#");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing value(s) for plane");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_iso_field_with_floats.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_iso_field_with_floats */
#endif /* defined (OLD_CODE) */

static int gfx_create_iso_surfaces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE ISO_SURFACES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name,*render_type_string,
		*use_element_type_string, **valid_strings;
	enum GT_object_type graphics_object_type;
	enum Render_type render_type;
	enum Use_element_type use_element_type;
	float time;
	int face_number,number_of_valid_strings,return_code;
	struct Clipping *clipping;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field, *data_field, *scalar_field,
		*surface_data_coordinate_field, *surface_data_density_field;
	struct Element_discretization discretization;
	struct Element_to_iso_scalar_data element_to_iso_scalar_data;
	struct FE_element *first_element;
	struct FE_field *native_discretization_field;
	struct FE_node *data_to_destroy;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *surface_data_group;
	struct GT_object *graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_scalar_field_data,
		set_surface_data_coordinate_field_data, set_surface_data_density_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_iso_surfaces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("iso_surfaces");
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate", computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			surface_data_group = (struct GROUP(FE_node) *)NULL;
			surface_data_coordinate_field = (struct Computed_field *)NULL;
			surface_data_density_field = (struct Computed_field *)NULL;
			time=0;
			material=ACCESS(Graphical_material)(
				command_data->default_graphical_material);
			element_group=(struct GROUP(FE_element) *)NULL;
			clipping=(struct Clipping *)NULL;
			if (scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_scalar,(void *)NULL,computed_field_manager))
			{
				ACCESS(Computed_field)(scalar_field);
			}
			element_to_iso_scalar_data.iso_value=0;
			native_discretization_field=(struct FE_field *)NULL;
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=4;
			discretization.number_in_xi3=4;
			exterior_flag=0;
			face_number=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* clipping */
			Option_table_add_entry(option_table,"clipping",&clipping,
				NULL,set_Clipping);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* iso_scalar */
			set_scalar_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_scalar_field_data.conditional_function=Computed_field_is_scalar;
			set_scalar_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"iso_scalar",&scalar_field,
				&set_scalar_field_data,set_Computed_field_conditional);
			/* iso_value */
			Option_table_add_entry(option_table,"iso_value",
				&(element_to_iso_scalar_data.iso_value),NULL,set_double);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* native_discretization */
			Option_table_add_entry(option_table,"native_discretization",
				&native_discretization_field,command_data->fe_field_manager,
				set_FE_field);
			/* render_type */
			render_type = RENDER_TYPE_SHADED;
			render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* surface_data_coordinate */
			set_surface_data_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_surface_data_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_surface_data_coordinate_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table,"surface_data_coordinate",
				&surface_data_coordinate_field,
				&set_surface_data_coordinate_field_data,set_Computed_field_conditional);
			/* surface_data_density */
			set_surface_data_density_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_surface_data_density_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_surface_data_density_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"surface_data_density",
				&surface_data_density_field,&set_surface_data_density_field_data,
				set_Computed_field_conditional);
			/* surface_data_group */
			Option_table_add_entry(option_table,"surface_data_group",
				&surface_data_group,command_data->data_group_manager,set_FE_node_group);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* use_elements/use_faces/use_lines */
			use_element_type = USE_ELEMENTS;
			use_element_type_string =
				ENUMERATOR_STRING(Use_element_type)(use_element_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings, &use_element_type_string);
			DEALLOCATE(valid_strings);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				face_number -= 2;
				if (surface_data_group&&(!surface_data_density_field))
				{
					display_message(ERROR_MESSAGE,"gfx_create_volumes.  Must supply "
						"a surface_data_density_field with a surface_data_group");
					return_code=0;
				}
				if ((!surface_data_group)&&surface_data_density_field)
				{
					display_message(ERROR_MESSAGE,"gfx_create_volumes.  Must supply "
						"a surface_data_group with a surface_data_density_field");
					return_code=0;
				}
				if (!scalar_field)
				{
					display_message(WARNING_MESSAGE,"Missing iso_scalar field");
					return_code=0;
				}
				STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
					&use_element_type);
				element_to_iso_scalar_data.use_element_type = use_element_type;
				switch (element_to_iso_scalar_data.use_element_type)
				{
					case USE_ELEMENTS:
					{
						graphics_object_type=g_VOLTEX;
						if (element_group)
						{
							first_element=FIRST_OBJECT_IN_GROUP_THAT(FE_element)(
								FE_element_is_top_level,(void *)NULL,element_group);
						}
						else
						{
							first_element=FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
								FE_element_is_top_level,(void *)NULL,
								command_data->element_manager);
						}
						if (first_element&&(2==get_FE_element_dimension(first_element)))
						{
							graphics_object_type=g_POLYLINE;
						}
					} break;
					case USE_FACES:
					{
						graphics_object_type=g_POLYLINE;
					} break;
					case USE_LINES:
					{
						display_message(ERROR_MESSAGE,
							".  "
							"USE_LINES not supported for iso_scalar");
						return_code=0;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							".  "
							"Unknown use_element_type");
						return_code=0;
					} break;
				}
				if (!graphics_object_name)
				{
					display_message(WARNING_MESSAGE,"Missing name");
					return_code=0;
				}
				if (return_code)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if (graphics_object_type==graphics_object->object_type)
						{
							if (GT_object_has_time(graphics_object,time))
							{
								return_code=GT_object_delete_time(graphics_object,time);
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
							graphics_object_type,material))&&
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list)))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_iso_surfaces.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
				}
				if (return_code && surface_data_group)
				{
					return_code = 1;
					MANAGED_GROUP_BEGIN_CACHE(FE_node)(surface_data_group);

					/* Remove all the current data from the group */
					while(return_code&&(data_to_destroy=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
						((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, surface_data_group)))
					{
						return_code = REMOVE_OBJECT_FROM_GROUP(FE_node)(
							data_to_destroy, surface_data_group);
						if (FE_node_can_be_destroyed(data_to_destroy))
						{
							return_code = REMOVE_OBJECT_FROM_MANAGER(FE_node)(data_to_destroy,
								command_data->data_manager);
						}
					}
					if(!return_code)
					{
						display_message(ERROR_MESSAGE,"gfx_create_iso_surfaces.  "
							"Unable to remove all data from data_group");
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
				if (return_code)
				{
					element_to_iso_scalar_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_iso_scalar_data.data_field=data_field;
					element_to_iso_scalar_data.scalar_field=scalar_field;
					element_to_iso_scalar_data.graphics_object=graphics_object;
					element_to_iso_scalar_data.clipping=clipping;
					element_to_iso_scalar_data.surface_data_coordinate_field = 
						surface_data_coordinate_field;
					element_to_iso_scalar_data.computed_field_manager =
						Computed_field_package_get_computed_field_manager(
							command_data->computed_field_package);
					element_to_iso_scalar_data.surface_data_density_field = 
						surface_data_density_field;
					element_to_iso_scalar_data.surface_data_group = 
						surface_data_group;
					element_to_iso_scalar_data.data_manager = 
						command_data->data_manager;
					element_to_iso_scalar_data.fe_field_manager =
						command_data->fe_field_manager;
					element_to_iso_scalar_data.time=time;
					element_to_iso_scalar_data.number_in_xi[0]=
						discretization.number_in_xi1;
					element_to_iso_scalar_data.number_in_xi[1]=
						discretization.number_in_xi2;
					element_to_iso_scalar_data.number_in_xi[2]=
						discretization.number_in_xi3;
					STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
					element_to_iso_scalar_data.render_type = render_type;
					element_to_iso_scalar_data.element_group=element_group;
					element_to_iso_scalar_data.exterior=exterior_flag;
					element_to_iso_scalar_data.face_number=face_number;
					element_to_iso_scalar_data.native_discretization_field=
						native_discretization_field;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_iso_scalar,(void *)&element_to_iso_scalar_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_iso_scalar,(void *)&element_to_iso_scalar_data,
							command_data->element_manager);
					}
					if (return_code)
					{
						if (!GT_object_has_time(graphics_object,time))
						{
							/* add a NULL primitive of the correct type to make an empty time
								 so there are no gaps in any time animation */
							switch (graphics_object_type)
							{
								case g_POLYLINE:
								{
									GT_OBJECT_ADD(GT_polyline)(graphics_object,time,
										(struct GT_polyline *)NULL);
								} break;
								case g_VOLTEX:
								{
									GT_OBJECT_ADD(GT_voltex)(graphics_object,time,
										(struct GT_voltex *)NULL);
								} break;
							}
						}
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(
								element_to_iso_scalar_data.graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No iso_surface created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(
						&(element_to_iso_scalar_data.coordinate_field));
					if (surface_data_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (scalar_field)
			{
				DEACCESS(Computed_field)(&scalar_field);
			}
			if (surface_data_coordinate_field)
			{
				DEACCESS(Computed_field)(&surface_data_coordinate_field);
			}
			if (surface_data_density_field)
			{
				DEACCESS(Computed_field)(&surface_data_density_field);
			}
			if (surface_data_group)
			{
				DEACCESS(GROUP(FE_node))(&surface_data_group);
			}
			if (clipping)
			{
				DESTROY(Clipping)(&clipping);
			}
			if (native_discretization_field)
			{
				DEACCESS(FE_field)(&native_discretization_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_iso_surfaces.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_iso_surfaces.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_iso_surfaces */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LIGHT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light *light;
	struct Modify_light_data modify_light_data;

	ENTER(gfx_create_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						command_data->light_manager))
					{
						if (light=CREATE(Light)(current_token))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(light,
								command_data->default_light);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_data.default_light=command_data->default_light;
								modify_light_data.light_manager=command_data->light_manager;
								return_code=modify_Light(state,(void *)light,
									(void *)(&modify_light_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light)(light,command_data->light_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light.  Could not create light");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_data.default_light=command_data->default_light;
					modify_light_data.light_manager=command_data->light_manager;
					return_code=modify_Light(state,(void *)NULL,
						(void *)(&modify_light_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LMODEL command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light_model *light_model;
	struct Modify_light_model_data modify_light_model_data;

	ENTER(gfx_create_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(current_token,
						command_data->light_model_manager))
					{
						if (light_model=CREATE(Light_model)(current_token))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(light_model,
								command_data->default_light_model);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_model_data.default_light_model=
									command_data->default_light_model;
								modify_light_model_data.light_model_manager=
									command_data->light_model_manager;
								return_code=modify_Light_model(state,(void *)light_model,
									(void *)(&modify_light_model_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light_model)(light_model,
								command_data->light_model_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light_model.  Could not create light model");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light_model already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_model_data.default_light_model=
						command_data->default_light_model;
					modify_light_model_data.light_model_manager=
						command_data->light_model_manager;
					return_code=modify_Light_model(state,(void *)NULL,
						(void *)(&modify_light_model_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light_model.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light model name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light_model */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_lines(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE LINES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name;
	float time;
	struct GT_object *graphics_object;
	int face_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Element_discretization discretization;
	struct Element_to_polyline_data element_to_polyline_data;
	struct Computed_field *coordinate_field,*data_field;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_lines);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("lines");
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			time=0;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=0;
			discretization.number_in_xi3=0;
			exterior_flag=0;
			face_number=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_POLYLINE==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_POLYLINE,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_lines.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					element_to_polyline_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_polyline_data.element_group=element_group;
					element_to_polyline_data.exterior=exterior_flag;
					element_to_polyline_data.face_number=face_number;
					element_to_polyline_data.data_field=data_field;
					element_to_polyline_data.graphics_object=graphics_object;
					element_to_polyline_data.time=time;
					element_to_polyline_data.number_of_segments_in_xi1=
						discretization.number_in_xi1;
					if (element_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							element_to_polyline,(void *)&element_to_polyline_data,
							element_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							element_to_polyline,(void *)&element_to_polyline_data,
							command_data->element_manager);
					}
					if (return_code)
					{
						if (!GT_object_has_time(graphics_object,time))
						{
							/* add a NULL polyline to make an empty time */
							GT_OBJECT_ADD(GT_polyline)(graphics_object,time,
								(struct GT_polyline *)NULL);
						}
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No lines created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(&(element_to_polyline_data.coordinate_field));
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_lines.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_lines.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_lines */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE MATERIAL command.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/
{
	char *current_token;
	int material_is_new,return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct Modify_graphical_material_data modify_graphical_material_data;

	ENTER(gfx_create_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					/* if there is an existing material of that name, just modify it */
					if (!(material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						current_token,command_data->graphical_material_manager)))
					{
						if (material=CREATE(Graphical_material)(current_token))
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(material,
								command_data->default_graphical_material);
						}
						material_is_new=1;
					}
					else
					{
						material_is_new=0;
					}
					if (material)
					{
						shift_Parse_state(state,1);
						if (state->current_token)
						{
							/* modify the material with the rest of the parse state */
							modify_graphical_material_data.default_graphical_material=
								command_data->default_graphical_material;
							modify_graphical_material_data.graphical_material_manager=
								command_data->graphical_material_manager;
							modify_graphical_material_data.texture_manager=
								command_data->texture_manager;
							return_code=modify_Graphical_material(state,(void *)material,
								(void *)(&modify_graphical_material_data));
						}
						else
						{
							return_code=1;
						}
						if (material_is_new)
						{
							ADD_OBJECT_TO_MANAGER(Graphical_material)(material,
								command_data->graphical_material_manager);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_material.  Error creating material");
						return_code=0;
					}
				}
				else
				{
					modify_graphical_material_data.default_graphical_material=
						command_data->default_graphical_material;
					modify_graphical_material_data.graphical_material_manager=
						command_data->graphical_material_manager;
					modify_graphical_material_data.texture_manager=
						command_data->texture_manager;
					return_code=modify_Graphical_material(state,(void *)NULL,
						(void *)(&modify_graphical_material_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_material.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_material */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_morph(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE MORPH command.  This command interpolates between two
graphics objects, and produces a new object
==============================================================================*/
{
	char *graphics_object_name;
	float proportion;
	gtObject *graphics_object;
	gtObject *initial,*final;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"final",NULL,NULL,set_Graphics_object},
		{"initial",NULL,NULL,set_Graphics_object},
		{"proportion",NULL,NULL,set_float_0_to_1_inclusive},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_morph);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("morph");
			proportion = 0.0;
			initial = (gtObject *)NULL;
			final = (gtObject *)NULL;
			(option_table[0]).to_be_modified= &graphics_object_name;
			(option_table[1]).to_be_modified= &final;
			(option_table[1]).user_data= (void *)command_data->graphics_object_list;
			(option_table[2]).to_be_modified= &initial;
			(option_table[2]).user_data= (void *)command_data->graphics_object_list;
			(option_table[3]).to_be_modified= &proportion;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* check for valid arguments */
				if (initial&&final)
				{
					if (graphics_object=morph_gtObject(graphics_object_name,
						proportion,initial,final))
					{
						if (!ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_morph.  Could not add graphics object to list");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_morph.  Could not create morph from surface");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must specify initial and final graphics objects for morph");
					return_code=0;
				}
			} /* parse error, help */
			DEALLOCATE(graphics_object_name);
			if (initial)
			{
				DEACCESS(GT_object)(&initial);
			}
			if (final)
			{
				DEACCESS(GT_object)(&final);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_morph.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_morph.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_morph */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_node_group_slider(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SLIDER command.  If there is a node group slider dialog
in existence, then bring it to the front, otherwise create new one.  If the
fixed node and node group don't have a slider in the slider dialog then add a
new slider.
???DB.  Temporary command ?
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_node_group_slider);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=create_node_group_slider_dialog(state,
				&(command_data->node_group_slider_dialog),
				command_data->user_interface->application_shell,
				command_data->node_manager,command_data->node_group_manager,
				command_data->execute_command,command_data->user_interface);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_node_group_slider.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_group_slider.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_group_slider */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

static int gfx_create_node_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE NODE_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_node_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->node_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->node_viewer);
				}
				else
				{
					if (command_data->node_viewer = CREATE(Node_viewer)(
						&(command_data->node_viewer),
						"Node Viewer",
						(struct FE_node *)NULL,
						command_data->node_manager,
						command_data->node_manager,
						command_data->element_manager,
						command_data->node_selection,
						command_data->computed_field_package,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_node_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_viewer */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE DATA_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_data_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->data_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->data_viewer);
				}
				else
				{
					if (command_data->data_viewer = CREATE(Node_viewer)(
						&(command_data->data_viewer),
						"Data Viewer",
						(struct FE_node *)NULL,
						command_data->data_manager,
						command_data->node_manager,
						command_data->element_manager,
						command_data->data_selection,
						command_data->computed_field_package,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_viewer */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_element_point_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINT_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_element_point_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->element_point_viewer)
				{
					return_code=Element_point_viewer_bring_window_to_front(
						command_data->element_point_viewer);
				}
				else
				{
					if (command_data->element_point_viewer=CREATE(Element_point_viewer)(
						&(command_data->element_point_viewer),
						command_data->element_manager,
						command_data->node_manager,
						command_data->element_point_ranges_selection,
						command_data->computed_field_package,
						command_data->fe_field_manager,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_point_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_point_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_point_viewer */

static int gfx_create_node_points(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Executes a GFX CREATE NODE_POINTS or GFX CREATE DATA_POINTS command.
If <use_data> is set, creating data points, otherwise creating node points.
==============================================================================*/
{
	char *graphics_object_name;
	FE_value base_size[3], centre[3], scale_factors[3];
	float time;
	int number_of_components, return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field, *data_field, *label_field,
		*orientation_scale_field, *rc_coordinate_field, *variable_scale_field,
		*wrapper_orientation_scale_field;
	struct GROUP(FE_node) *node_group;
	struct Graphical_material *material;
	struct GT_glyph_set *glyph_set;
	struct GT_object *glyph,*graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_node_points);
	if (state &&
		(command_data=(struct Cmiss_command_data *)command_data_void) &&
		(computed_field_manager = Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package)))
	{
		if (use_data)
		{
			node_manager = command_data->data_manager;
			node_group_manager = command_data->data_group_manager;
			graphics_object_name = duplicate_string("data_points");
		}
		else
		{
			node_manager = command_data->node_manager;
			node_group_manager = command_data->node_group_manager;
			graphics_object_name = duplicate_string("node_points");
		}
		/* initialise defaults */
		/* default to point glyph for fastest possible display */
		if (glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
			command_data->glyph_list))
		{
			ACCESS(GT_object)(glyph);
		}
		node_group = (struct GROUP(FE_node) *)NULL;
		coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			"default_coordinate", computed_field_manager);
		/* must access it now, because we deaccess it later */
		ACCESS(Computed_field)(coordinate_field);
		data_field = (struct Computed_field *)NULL;
		/* must access it now, because we deaccess it later */
		material =
			ACCESS(Graphical_material)(command_data->default_graphical_material);
		label_field = (struct Computed_field *)NULL;
		orientation_scale_field = (struct Computed_field *)NULL;
		variable_scale_field = (struct Computed_field *)NULL;
		spectrum = ACCESS(Spectrum)(command_data->default_spectrum);
		time = 0.0;
		/* final_size = size + scale_factors*magnitude */
		glyph_scale_factors[0] = 1.0;
		glyph_scale_factors[1] = 1.0;
		glyph_scale_factors[2] = 1.0;
		glyph_size[0] = 1.0;
		glyph_size[1] = 1.0;
		glyph_size[2] = 1.0;
		number_of_components = 3;
		glyph_centre[0] = 0.0;
		glyph_centre[1] = 0.0;
		glyph_centre[2] = 0.0;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* centre [of glyph] */
		Option_table_add_entry(option_table,"centre",glyph_centre,
			&(number_of_components),set_float_vector);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* from [node_group] */
		Option_table_add_entry(option_table,"from",&node_group,
			node_group_manager,set_FE_node_group);
		/* glyph */
		Option_table_add_entry(option_table,"glyph",&glyph,
			command_data->glyph_list,set_Graphics_object);
		/* label */
		set_label_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_label_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_label_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"label",&label_field,
			&set_label_field_data,set_Computed_field_conditional);
		/* material */
		Option_table_add_entry(option_table,"material",&material,
			command_data->graphical_material_manager,set_Graphical_material);
		/* orientation */
		set_orientation_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_orientation_scale_field_data.conditional_function=
			Computed_field_is_orientation_scale_capable;
		set_orientation_scale_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"orientation",
			&orientation_scale_field,&set_orientation_scale_field_data,
			set_Computed_field_conditional);
		/* scale_factors */
		Option_table_add_entry(option_table,"scale_factors",
			glyph_scale_factors,"*",set_special_float3);
		/* size [of glyph] */
		Option_table_add_entry(option_table,"size",
			glyph_size,"*",set_special_float3);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",
			&spectrum,command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* variable_scale */
		set_variable_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_variable_scale_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_variable_scale_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"variable_scale",&variable_scale_field,
			&set_variable_scale_field_data,set_Computed_field_conditional);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (g_GLYPH_SET==graphics_object->object_type)
				{
					if (GT_object_has_time(graphics_object,time))
					{
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'",time,
							graphics_object_name);
						return_code=GT_object_delete_time(graphics_object,time);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
			}
			else
			{
				if ((graphics_object=CREATE(GT_object)(graphics_object_name,
					g_GLYPH_SET,material))&&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_node_points.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code=0;
				}
			}
			if (return_code)
			{
				rc_coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				if (orientation_scale_field)
				{
					wrapper_orientation_scale_field=
						Computed_field_begin_wrap_orientation_scale_field(
							orientation_scale_field,rc_coordinate_field);
				}
				else
				{
					wrapper_orientation_scale_field=(struct Computed_field *)NULL;
				}
				base_size[0] = (FE_value)glyph_size[0];
				base_size[1] = (FE_value)glyph_size[1];
				base_size[2] = (FE_value)glyph_size[2];
				centre[0] = (FE_value)glyph_centre[0];
				centre[1] = (FE_value)glyph_centre[1];
				centre[2] = (FE_value)glyph_centre[2];
				scale_factors[0] = (FE_value)glyph_scale_factors[0];
				scale_factors[1] = (FE_value)glyph_scale_factors[1];
				scale_factors[2] = (FE_value)glyph_scale_factors[2];
				if (glyph_set = create_GT_glyph_set_from_FE_node_group(
					node_group, node_manager, rc_coordinate_field,
					glyph, base_size, centre, scale_factors,
					wrapper_orientation_scale_field, variable_scale_field,
					data_field, label_field, GRAPHICS_NO_SELECT,
					(struct LIST(FE_node) *)NULL))
				{
					if (!GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,glyph_set))
					{
						DESTROY(GT_glyph_set)(&glyph_set);
						return_code=0;
					}
				}
				if (wrapper_orientation_scale_field)
				{
					Computed_field_end_wrap(&wrapper_orientation_scale_field);
				}
				Computed_field_end_wrap(&rc_coordinate_field);
				if (return_code)
				{
					if (!GT_object_has_time(graphics_object,time))
					{
						/* add a NULL glyph_set to make an empty time */
						GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,
							(struct GT_glyph_set *)NULL);
					}
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					if (use_data)
					{
						display_message(WARNING_MESSAGE,"No data_glyphs created");
					}
					else
					{
						display_message(WARNING_MESSAGE,"No node_glyphs created");
					}
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
			} /* not duplicate name */
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		if (orientation_scale_field)
		{
			DEACCESS(Computed_field)(&orientation_scale_field);
		}
		if (variable_scale_field)
		{
			DEACCESS(Computed_field)(&variable_scale_field);
		}
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
		if (glyph)
		{
			DEACCESS(GT_object)(&glyph);
		}
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_points */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE DGROUP command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Add_FE_node_to_list_if_in_range_data node_in_range_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*from_data_group,*node_group;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_node_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_data_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		name=(char *)NULL;
		if (set_name(state,(void *)&name,(void *)1))
		{
			/* initialise defaults */
			from_data_group=(struct GROUP(FE_node) *)NULL;
			node_in_range_data.node_list=CREATE(LIST(FE_node))();
			node_in_range_data.node_ranges=CREATE(Multi_range)();
			(option_table[0]).to_be_modified=node_in_range_data.node_ranges;
			(option_table[1]).to_be_modified= &from_data_group;
			(option_table[1]).user_data=command_data->data_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name&&!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
					command_data->data_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
						command_data->node_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(name,
						command_data->element_group_manager))
				{
					/* create node and element groups of same name simultaneously */
					data_group=CREATE_GROUP(FE_node)(name);
					node_group=CREATE_GROUP(FE_node)(name);
					element_group=CREATE_GROUP(FE_element)(name);
					if (data_group&&node_group&&element_group)
					{
						if (0<Multi_range_get_number_of_ranges(
							node_in_range_data.node_ranges))
						{
							/* make list of data to add to group, then add to group */
							if (from_data_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									from_data_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									command_data->data_manager);
							}
							if (return_code)
							{
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_node)(
									ensure_FE_node_is_in_group,(void *)data_group,
									node_in_range_data.node_list);
							}
						}
						if (return_code)
						{
							/* must add node group before element group so the node group
								 exists when a GT_element_group is made for the element group */
							if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(node_group,
									command_data->node_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(element_group,
									command_data->element_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_create_data_group.  "
									"Could not add data/node/element group(s) to manager");
								DESTROY_GROUP(FE_element)(&element_group);
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->node_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
										command_data->node_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&node_group);
								}
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->data_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
										command_data->data_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&data_group);
								}
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_data_group.  Could not add data ranges to group");
							DESTROY(GROUP(FE_element))(&element_group);
							DESTROY_GROUP(FE_node)(&node_group);
							DESTROY(GROUP(FE_node))(&data_group);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx_create_data_group.  "
							"Could not create data/node/element group(s)");
						DESTROY(GROUP(FE_element))(&element_group);
						DESTROY(GROUP(FE_node))(&node_group);
						DESTROY(GROUP(FE_node))(&data_group);
						return_code=0;
					}
				}
				else
				{
					if (name)
					{
						display_message(ERROR_MESSAGE,"gfx_create_data_group.  "
							"Data/node/element group '%s' already exists",name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_data_group.  Must specify a name for the group");
					}
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&node_in_range_data.node_ranges);
			DESTROY(LIST(FE_node))(&node_in_range_data.node_list);
			if (from_data_group)
			{
				DEACCESS(GROUP(FE_node))(&from_data_group);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_data_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_node_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE NGROUP command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Add_FE_node_to_list_if_in_range_data node_in_range_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_node) *data_group,*from_node_group,*node_group;
	struct GROUP(FE_element) *element_group;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_node_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_node_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		name=(char *)NULL;
		if (set_name(state,(void *)&name,(void *)1))
		{
			/* initialise defaults */
			from_node_group=(struct GROUP(FE_node) *)NULL;
			node_in_range_data.node_list=CREATE(LIST(FE_node))();
			node_in_range_data.node_ranges=CREATE(Multi_range)();
			(option_table[0]).to_be_modified=node_in_range_data.node_ranges;
			(option_table[1]).to_be_modified= &from_node_group;
			(option_table[1]).user_data=command_data->node_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name&&!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
					command_data->data_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(name,
						command_data->node_group_manager)&&
					!FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(name,
						command_data->element_group_manager))
				{
					/* create node and element groups of same name simultaneously */
					data_group=CREATE_GROUP(FE_node)(name);
					node_group=CREATE_GROUP(FE_node)(name);
					element_group=CREATE_GROUP(FE_element)(name);
					if (data_group&&node_group&&element_group)
					{
						if (0<Multi_range_get_number_of_ranges(
							node_in_range_data.node_ranges))
						{
							/* make list of nodes to add to group, then add to group */
							if (from_node_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									from_node_group);
							}
							else
							{
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
									add_FE_node_to_list_if_in_range,(void *)&node_in_range_data,
									command_data->node_manager);
							}
							if (return_code)
							{
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_node)(
									ensure_FE_node_is_in_group,(void *)node_group,
									node_in_range_data.node_list);
							}
						}
						if (return_code)
						{
							/* must add node group before element group so the node group
								 exists when a GT_element_group is made for the element group */
							if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(node_group,
									command_data->node_group_manager)&&
								ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(element_group,
									command_data->element_group_manager))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_create_node_group.  "
									"Could not add data/node/element group(s) to manager");
								DESTROY_GROUP(FE_element)(&element_group);
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->node_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
										command_data->node_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&node_group);
								}
								if (FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
									name,command_data->data_group_manager))
								{
									REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
										command_data->data_group_manager);
								}
								else
								{
									DESTROY_GROUP(FE_node)(&data_group);
								}
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_node_group.  Could not add node ranges to group");
							DESTROY(GROUP(FE_element))(&element_group);
							DESTROY_GROUP(FE_node)(&node_group);
							DESTROY(GROUP(FE_node))(&data_group);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx_create_node_group.  "
							"Could not create data/node/element group(s)");
						DESTROY(GROUP(FE_element))(&element_group);
						DESTROY(GROUP(FE_node))(&node_group);
						DESTROY(GROUP(FE_node))(&data_group);
						return_code=0;
					}
				}
				else
				{
					if (name)
					{
						display_message(ERROR_MESSAGE,"gfx_create_node_group.  "
							"Data/node/element group '%s' already exists",name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_node_group.  Must specify a name for the group");
					}
					return_code=0;
				}
			}
			DESTROY(Multi_range)(&node_in_range_data.node_ranges);
			DESTROY(LIST(FE_node))(&node_in_range_data.node_list);
			if (from_node_group)
			{
				DEACCESS(GROUP(FE_node))(&from_node_group);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Executes a GFX CREATE SCENE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_scene_data modify_scene_data;
	struct Scene *scene;

	ENTER(gfx_create_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				/* set up data for modify_Scene since called in two places */
				modify_scene_data.light_manager=command_data->light_manager;
				modify_scene_data.scene_manager=command_data->scene_manager;
				modify_scene_data.default_scene=command_data->default_scene;
				/* following used for enabling GFEs */
				modify_scene_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
				modify_scene_data.element_manager=command_data->element_manager;
				modify_scene_data.element_group_manager=
					command_data->element_group_manager;
				modify_scene_data.fe_field_manager=
					command_data->fe_field_manager;
				modify_scene_data.node_manager=command_data->node_manager;
				modify_scene_data.node_group_manager=command_data->node_group_manager;
				modify_scene_data.data_manager=command_data->data_manager;
				modify_scene_data.data_group_manager=command_data->data_group_manager;
				modify_scene_data.element_point_ranges_selection=
					command_data->element_point_ranges_selection;
				modify_scene_data.element_selection=command_data->element_selection;
				modify_scene_data.node_selection=command_data->node_selection;
				modify_scene_data.data_selection=command_data->data_selection;
				modify_scene_data.user_interface=command_data->user_interface;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
						current_token,command_data->scene_manager))
					{
						if (scene=CREATE(Scene)(current_token))
						{
							Scene_enable_graphics(scene,command_data->glyph_list,
								command_data->graphical_material_manager,
								command_data->default_graphical_material,
								command_data->light_manager,
								command_data->spectrum_manager,
								command_data->default_spectrum,
								command_data->texture_manager);
							Scene_enable_time_behaviour(scene,
								command_data->default_time_keeper);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=modify_Scene(state,(void *)scene,
									(void *)(&modify_scene_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Scene)(scene,
								command_data->scene_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_scene.  Error creating scene");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Scene already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=
						modify_Scene(state,(void *)NULL,(void *)(&modify_scene_data));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_scene.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing scene name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_scene */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_snake(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Executes a GFX CREATE SNAKE command.
==============================================================================*/
{
	float density_factor, stiffness;
	int number_of_elements, return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *coordinate_field;
	struct GROUP(FE_element) *element_group;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_coordinate_field_data;

	ENTER(gfx_create_snake);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		coordinate_field = (struct FE_field *)NULL;
		density_factor = 0.0;
		element_group = (struct GROUP(FE_element) *)NULL;
		number_of_elements = 1;
		stiffness = 0.0;

		option_table = CREATE(Option_table)();
		/* coordinate */
		set_coordinate_field_data.fe_field_manager = command_data->fe_field_manager;
		set_coordinate_field_data.conditional_function =
			FE_field_is_coordinate_field;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table, "coordinate", &coordinate_field,
			&set_coordinate_field_data, set_FE_field_conditional);
		/* density_factor */
		Option_table_add_entry(option_table, "density_factor",
			&density_factor, NULL, set_float_0_to_1_inclusive);
		/* destination_group */
		Option_table_add_entry(option_table, "destination_group", &element_group,
			command_data->element_group_manager, set_FE_element_group);
		/* number_of_elements */
		Option_table_add_entry(option_table, "number_of_elements",
			&number_of_elements, NULL, set_int_positive);
		/* stiffness */
		Option_table_add_entry(option_table, "stiffness",
			&stiffness, NULL, set_float_non_negative);
		return_code = Option_table_multi_parse(option_table, state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE, "gfx create snake.  "
					"Must specify a coordinate_field to define on elements in snake");
				return_code = 0;
			}
			if (!element_group)
			{
				display_message(ERROR_MESSAGE, "gfx create snake.  "
					"Must specify a destination_group to put the snake elements in");
				return_code = 0;
			}
			if (return_code)
			{
				return_code = create_FE_element_snake_from_data_points(
					command_data->element_manager,
					command_data->node_manager,
					element_group,
					command_data->node_group_manager,
					command_data->basis_manager,
					coordinate_field,
					FE_node_selection_get_node_list(command_data->data_selection),
					number_of_elements,
					density_factor,
					stiffness);
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(FE_field)(&coordinate_field);
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_snake.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_snake */

static int gfx_modify_Spectrum(struct Parse_state *state,void *spectrum_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Modifier function that parses all the command line options for creating or
modifying a spectrum.
I would put this with the other gfx modify routines but then it can't be
static and referred to by gfx_create_Spectrum
==============================================================================*/
{
	char autorange, blue_to_red, blue_white_red, clear, *current_token, lg_blue_to_red,
		lg_red_to_blue, overlay_colour, overwrite_colour, red_to_blue;
	int process, range_set, return_code;
	float maximum, minimum;
	struct Cmiss_command_data *command_data;
	struct Modify_spectrum_data modify_spectrum_data;
	struct Option_table *option_table;
	struct Scene *autorange_scene;
	struct Spectrum *spectrum_to_be_modified,*spectrum_to_be_modified_copy;
	struct Spectrum_command_data spectrum_command_data;

	ENTER(gfx_modify_Spectrum);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (spectrum_to_be_modified=(struct Spectrum *)spectrum_void)
				{
					if (IS_MANAGED(Spectrum)(spectrum_to_be_modified,
						command_data->spectrum_manager))
					{
						if (spectrum_to_be_modified_copy=CREATE(Spectrum)(
							"spectrum_modify_temp"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(
								spectrum_to_be_modified_copy,spectrum_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create spectrum copy.");
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified_copy=spectrum_to_be_modified;
						spectrum_to_be_modified=(struct Spectrum *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (spectrum_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Spectrum,name)(current_token,
							command_data->spectrum_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (spectrum_to_be_modified_copy=CREATE(Spectrum)(
									"spectrum_modify_temp"))
								{
									MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name)(
										spectrum_to_be_modified_copy,spectrum_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Could not create spectrum copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (spectrum_to_be_modified=CREATE(Spectrum)("dummy"))
						{
							option_table=CREATE(Option_table)();
							Option_table_add_entry(option_table,"SPECTRUM_NAME",
								(void *)spectrum_to_be_modified,command_data_void,
								gfx_modify_Spectrum);
							return_code=Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
							DESTROY(Spectrum)(&spectrum_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create dummy spectrum");
							return_code=0;
						}
					}
				}
				if (process)
				{
					autorange = 0;
					autorange_scene = ACCESS(Scene)(command_data->default_scene);
					blue_to_red = 0;
					clear = 0;
					lg_blue_to_red = 0;
					lg_red_to_blue = 0;
					overlay_colour = 0;
					overwrite_colour = 0;
					red_to_blue = 0;
					blue_white_red = 0;
					modify_spectrum_data.position = 0;
					modify_spectrum_data.settings = (struct Spectrum_settings *)NULL;
					modify_spectrum_data.spectrum_minimum = get_Spectrum_minimum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.spectrum_maximum = get_Spectrum_maximum(
						spectrum_to_be_modified_copy);
					spectrum_command_data.spectrum_manager 
						= command_data->spectrum_manager;
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"autorange",&autorange,NULL,
						set_char_flag);					
					Option_table_add_entry(option_table,"blue_to_red",&blue_to_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_white_red",&blue_white_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"clear",&clear,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"linear",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_linear);
					Option_table_add_entry(option_table,"log",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_log);
					Option_table_add_entry(option_table,"lg_blue_to_red",&lg_blue_to_red,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"lg_red_to_blue",&lg_red_to_blue,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"maximum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_maximum_command);
					Option_table_add_entry(option_table,"minimum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_minimum_command);
					Option_table_add_entry(option_table,"overlay_colour",&overlay_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"overwrite_colour",&overwrite_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"scene_for_autorange",&autorange_scene,
						command_data->scene_manager,set_Scene);
					Option_table_add_entry(option_table,"red_to_blue",&red_to_blue,
						NULL,set_char_flag);									
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (return_code)
						{
							if ( clear )
							{
								Spectrum_remove_all_settings(spectrum_to_be_modified_copy);
							}
							if (blue_to_red + blue_white_red +red_to_blue + lg_red_to_blue + 
								lg_blue_to_red > 1 )
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one simple spectrum type\n "
									"   (blue_to_red, blue_white_red, red_to_blue, lg_red_to_blue, lg_blue_to_red)");
								return_code=0;
							}
							else if (red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									RED_TO_BLUE_SPECTRUM);
							}
							else if (blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_TO_RED_SPECTRUM);
							}
							else if (blue_white_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_WHITE_RED_SPECTRUM);
							}
							else if (lg_red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_RED_TO_BLUE_SPECTRUM);
							}
							else if (lg_blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_BLUE_TO_RED_SPECTRUM);
							}
							if ( modify_spectrum_data.settings )
							{
								/* add new settings */
								return_code=Spectrum_add_settings(spectrum_to_be_modified_copy,
									modify_spectrum_data.settings,
									modify_spectrum_data.position);
							}
							if (overlay_colour && overwrite_colour)
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one colour mode, overwrite_colour or overlay_colour");
								return_code=0;
							}
							else if (overlay_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									0);
							}
							else if (overwrite_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									1);
							}
							if (autorange)
							{
								/* Could also do all scenes */
								range_set = 0;
								Scene_get_data_range_for_spectrum(autorange_scene,
									spectrum_to_be_modified
									/* Not spectrum_to_be_modified_copy as this ptr 
										identifies the valid graphics objects */,
									&minimum, &maximum, &range_set);
								if ( range_set )
								{
									Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
										minimum, maximum );
								}
							}
							if (spectrum_to_be_modified)
							{

								MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(
									spectrum_to_be_modified,spectrum_to_be_modified_copy,
									command_data->spectrum_manager);
								DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
							}
							else
							{
								spectrum_to_be_modified=spectrum_to_be_modified_copy;
							}
						}
						else
						{
							DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
						}
					}
					if(option_table)
					{
						DESTROY(Option_table)(&option_table);
					}
					if ( modify_spectrum_data.settings )
					{
						DEACCESS(Spectrum_settings)(&(modify_spectrum_data.settings));
					}
					DEACCESS(Scene)(&autorange_scene);
				}
			}
			else
			{
				if (spectrum_void)
				{
					display_message(ERROR_MESSAGE,"Missing spectrum modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing spectrum name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Spectrum */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SPECTRUM command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)(
						current_token,command_data->spectrum_manager))
					{
						if (spectrum=CREATE(Spectrum)(current_token))
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(spectrum,
								command_data->default_spectrum);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=gfx_modify_Spectrum(state,(void *)spectrum,
									command_data_void);
							}
							else
							{
								return_code=1;
							}
							if (return_code)
							{
								ADD_OBJECT_TO_MANAGER(Spectrum)(spectrum,
									command_data->spectrum_manager);
							}
							else
								DESTROY(Spectrum)(&spectrum);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_spectrum.  Error creating spectrum");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Spectrum already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Spectrum(state,(void *)NULL,command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_spectrum.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_spectrum */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_streamlines(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE STREAMLINES command.
==============================================================================*/
{
	char *graphics_object_name,reverse_track,*streamline_data_type_string,
		*streamline_type_string,**valid_strings;
	enum GT_object_type graphics_object_type;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	FE_value seed_xi[3];
	float length,time,width;
	int number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*stream_vector_field;
	struct Element_to_streamline_data element_to_streamline_data;
	struct FE_element *seed_element;
	struct FE_field *seed_data_field;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *seed_data_group;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Node_to_streamline_data node_to_streamline_data;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_stream_vector_field_data;
	struct Set_FE_field_conditional_data set_seed_data_field_data;

	ENTER(gfx_create_streamlines);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("streamlines");
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			data_field=(struct Computed_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			seed_data_group = (struct GROUP(FE_node) *)NULL;
			seed_data_field = (struct FE_field *)NULL;
			time=0;
			length=1;
			width = 1;
			reverse_track = 0;
			number_of_components = 3;
			seed_xi[0] = 0.5;
			seed_xi[1] = 0.5;
			seed_xi[2] = 0.5;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* ellipse/line/rectangle/ribbon */
			streamline_type = STREAM_LINE;
			streamline_type_string =
				ENUMERATOR_STRING(Streamline_type)(streamline_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_type_string);
			DEALLOCATE(valid_strings);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* length */
			Option_table_add_entry(option_table,"length",&length,NULL,set_float);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* no_data/field_scalar/magnitude_scalar/travel_scalar */
			streamline_data_type = STREAM_NO_DATA;
			streamline_data_type_string =
				ENUMERATOR_STRING(Streamline_data_type)(streamline_data_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_data_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &streamline_data_type_string);
			DEALLOCATE(valid_strings);
			/* reverse */
			/*???RC use negative length to denote reverse track instead? */
			Option_table_add_entry(option_table,"reverse",
				&reverse_track,NULL,set_char_flag);
			/* seed_data_field */
			set_seed_data_field_data.fe_field_manager=command_data->fe_field_manager;
			set_seed_data_field_data.conditional_function=FE_field_has_value_type;
			set_seed_data_field_data.conditional_function_user_data=
				(void *)ELEMENT_XI_VALUE;
			Option_table_add_entry(option_table,"seed_data_field",&seed_data_field,
				&set_seed_data_field_data,set_FE_field_conditional);
			/* seed_data_group */
			Option_table_add_entry(option_table,"seed_data_group",&seed_data_group,
				command_data->data_group_manager,set_FE_node_group);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,command_data->element_manager,
				set_FE_element_dimension_3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			/* width */
			Option_table_add_entry(option_table,"width",&width,NULL,set_float);
			/* xi */
			Option_table_add_entry(option_table,"xi",
				seed_xi,&number_of_components,set_float_vector);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (element_group&&seed_element&&(!IS_OBJECT_IN_GROUP(FE_element)(
					seed_element,element_group)))
				{
					display_message(ERROR_MESSAGE,
						"seed_element is not in element_group");
					return_code=0;
				}
				if (!stream_vector_field)
				{
					display_message(ERROR_MESSAGE,"Must specify a vector");
					return_code=0;
				}
				STRING_TO_ENUMERATOR(Streamline_type)(streamline_type_string,
					&streamline_type);
				STRING_TO_ENUMERATOR(Streamline_data_type)(streamline_data_type_string,
					&streamline_data_type);
				if (data_field)
				{
					if (STREAM_FIELD_SCALAR != streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must use field_scalar option with data; ensuring this");
						streamline_data_type=STREAM_FIELD_SCALAR;
					}
				}
				else
				{
					if (STREAM_FIELD_SCALAR == streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must specify data field with field_scalar option");
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if (seed_data_field&&(!seed_data_group))
				{
					display_message(ERROR_MESSAGE,
						"If you specify a seed_data_field then you must also specity a seed_data_group");
					return_code=0;					
				}
				if ((!seed_data_field)&&seed_data_group)
				{
					display_message(ERROR_MESSAGE,
						"If you specify a seed_data_group then you must also specity a seed_data_field");
					return_code=0;
				}
				if (return_code)
				{
					if (STREAM_LINE==streamline_type)
					{
						graphics_object_type=g_POLYLINE;
					}
					else
					{
						graphics_object_type=g_SURFACE;
					}
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if (graphics_object_type == graphics_object->object_type)
						{
							if (GT_object_has_time(graphics_object,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
								return_code=GT_object_delete_time(graphics_object,time);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (STREAM_LINE==streamline_type)
						{
							graphics_object=CREATE(GT_object)(graphics_object_name,
								g_POLYLINE,material);
						}
						else
						{
							graphics_object=CREATE(GT_object)(graphics_object_name,
								g_SURFACE,material);
						}
						if (graphics_object&&ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
						{
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_streamlines.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						if (seed_data_group)
						{
							node_to_streamline_data.coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							node_to_streamline_data.stream_vector_field=
								Computed_field_begin_wrap_orientation_scale_field(
									stream_vector_field,
									node_to_streamline_data.coordinate_field);
							node_to_streamline_data.type = streamline_type;
							node_to_streamline_data.data_type = streamline_data_type;
							node_to_streamline_data.data_field = data_field;
							node_to_streamline_data.element_group = element_group;
							node_to_streamline_data.graphics_object = graphics_object;
							node_to_streamline_data.length = length;
							node_to_streamline_data.width = width;
							node_to_streamline_data.time = time;
							/* reverse_track = track -vector and store -travel_scalar */
							node_to_streamline_data.reverse_track=(int)reverse_track;
							node_to_streamline_data.seed_data_field=seed_data_field;
							node_to_streamline_data.seed_element = seed_element;
							return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
								node_to_streamline,
								(void *)&node_to_streamline_data,seed_data_group);
							Computed_field_end_wrap(
								&(node_to_streamline_data.stream_vector_field));
							Computed_field_end_wrap(
								&(node_to_streamline_data.coordinate_field));
						}
						else
						{
							element_to_streamline_data.coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							element_to_streamline_data.stream_vector_field=
								Computed_field_begin_wrap_orientation_scale_field(
									stream_vector_field,
									element_to_streamline_data.coordinate_field);
							element_to_streamline_data.type = streamline_type;
							element_to_streamline_data.data_type = streamline_data_type;
							element_to_streamline_data.data_field = data_field;
							element_to_streamline_data.graphics_object = graphics_object;
							element_to_streamline_data.length = length;
							element_to_streamline_data.width = width;
							element_to_streamline_data.time = time;
							/* reverse_track = track -vector and store -travel_scalar */
							element_to_streamline_data.reverse_track=(int)reverse_track;
							element_to_streamline_data.seed_xi[0]=seed_xi[0];
							element_to_streamline_data.seed_xi[1]=seed_xi[1];
							element_to_streamline_data.seed_xi[2]=seed_xi[2];
							if (seed_element)
							{
								element_to_streamline(seed_element,
									(void *)&element_to_streamline_data);
							}
							else
							{
								if (element_group)
								{
									return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
										element_to_streamline,
										(void *)&element_to_streamline_data,element_group);
								}
								else
								{
									return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
										element_to_streamline,
										(void *)&element_to_streamline_data,
										command_data->element_manager);
								}
							}
							Computed_field_end_wrap(
								&(element_to_streamline_data.stream_vector_field));
							Computed_field_end_wrap(
								&(element_to_streamline_data.coordinate_field));
						}
						if (return_code)
						{
							if (!GT_object_has_time(graphics_object,time))
							{
								/* add a NULL primitive of the correct type to make an empty
									 time so there are no gaps in any time animation */
								switch (graphics_object_type)
								{
									case g_POLYLINE:
									{
										GT_OBJECT_ADD(GT_polyline)(graphics_object,time,
											(struct GT_polyline *)NULL);
									} break;
									case g_SURFACE:
									{
										GT_OBJECT_ADD(GT_surface)(graphics_object,time,
											(struct GT_surface *)NULL);
									} break;
								}
							}
							if (STREAM_NO_DATA != streamline_data_type)
							{
								return_code=set_GT_object_Spectrum(graphics_object,spectrum);
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"No streamlines created");
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (seed_data_field)
			{
				DEACCESS(FE_field)(&seed_data_field);
			}
			if (seed_data_group)
			{
				DEACCESS(GROUP(FE_node))(&seed_data_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_streamlines.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_streamlines.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_streamlines */

#if defined (OLD_CODE)
static int gfx_create_interactive_streamline(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE INTERACTIVE_STREAMLINE command.
==============================================================================*/
{
	char *graphics_object_name,*line_graphics_object_name,reverse_track,
		*streamline_data_type_string,*streamline_type_string,**valid_strings;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	FE_value seed_xi[3];
	float length,width;
	int number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*rc_coordinate_field,
		*stream_vector_field,*wrapper_stream_vector_field;
	struct FE_element *seed_element;
	struct GROUP(FE_element) *element_group;
	struct Graphical_material *material;
	struct GT_object *graphics_object,*line_graphics_object;
	struct GT_pointset *point_set;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct Interactive_streamline *streamline;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_stream_vector_field_data;

	ENTER(gfx_create_interactive_streamline);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("interactive_streamline");
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			if (stream_vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_stream_vector_capable,(void *)NULL,
				computed_field_manager))
			{
				ACCESS(Computed_field)(stream_vector_field);
			}
			data_field=(struct Computed_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			length=1;
			width = 1;
			reverse_track = 0;
			number_of_components = 3;
			seed_xi[0] = 0.5;
			seed_xi[1] = 0.5;
			seed_xi[2] = 0.5;
			graphics_object=(struct GT_object *)NULL;
			line_graphics_object=(struct GT_object *)NULL;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=
				ACCESS(Spectrum)(command_data->default_spectrum);

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* ellipse/line/rectangle/ribbon */
			streamline_type = STREAM_LINE;
			streamline_type_string =
				ENUMERATOR_STRING(Streamline_type)(streamline_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_type_string);
			DEALLOCATE(valid_strings);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* initial_xi */
			Option_table_add_entry(option_table,"initial_xi",
				seed_xi,&number_of_components,set_float_vector);
			/* length */
			Option_table_add_entry(option_table,"length",&length,NULL,set_float);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* no_data/field_scalar/magnitude_scalar/travel_scalar */
			streamline_data_type = STREAM_NO_DATA;
			streamline_data_type_string =
				ENUMERATOR_STRING(Streamline_data_type)(streamline_data_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_data_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &streamline_data_type_string);
			DEALLOCATE(valid_strings);
			/* reverse */
			/*???RC use negative length to denote reverse track instead? */
			Option_table_add_entry(option_table,"reverse",
				&reverse_track,NULL,set_char_flag);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,command_data->element_manager,
				set_FE_element_dimension_3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			/* width */
			Option_table_add_entry(option_table,"width",&width,NULL,set_float);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (seed_element)
				{
					if (element_group&&(!IS_OBJECT_IN_GROUP(FE_element)(
						seed_element,element_group)))
					{
						display_message(ERROR_MESSAGE,
							"seed_element is not in element_group");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must specify a seed_element");
					return_code=0;
				}
				if (!stream_vector_field)
				{
					display_message(ERROR_MESSAGE,"Must specify a vector");
					return_code=0;
				}
				STRING_TO_ENUMERATOR(Streamline_type)(streamline_type_string,
					&streamline_type);
				STRING_TO_ENUMERATOR(Streamline_data_type)(streamline_data_type_string,
					&streamline_data_type);
				if (data_field)
				{
					if (STREAM_FIELD_SCALAR != streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must use field_scalar option with data; ensuring this");
						streamline_data_type=STREAM_FIELD_SCALAR;
					}
				}
				else
				{
					if (STREAM_FIELD_SCALAR == streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must specify data field with field_scalar option");
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if (return_code)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
					else
					{
						if ((graphics_object=CREATE(GT_object)(graphics_object_name,
							g_POINTSET,material)))
						{
							if (ALLOCATE(line_graphics_object_name,char,
								strlen(graphics_object_name)+15))
							{
								strcpy(line_graphics_object_name,graphics_object_name);
								strcat(line_graphics_object_name,"_streamline");
								if (STREAM_LINE==streamline_type)
								{
									line_graphics_object=CREATE(GT_object)(
										line_graphics_object_name,g_POLYLINE,material);
								}
								else
								{
									line_graphics_object=CREATE(GT_object)(
										line_graphics_object_name,g_SURFACE,material);
								}
								if (line_graphics_object)
								{
									graphics_object->nextobject=
										ACCESS(GT_object)(line_graphics_object);
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,"gfx_create_streamlines.  "
										"Could not create line_graphics_object");
									DESTROY(GT_object)(&line_graphics_object);
									DESTROY(GT_object)(&graphics_object);
									return_code=0;
								}
								DEALLOCATE(line_graphics_object_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_interactive_streamline.  "
									"Could not allocate memory for line_graphics_object_name");
								DESTROY(GT_object)(&graphics_object);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_interactive_streamline.  "
								"Could not create main graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						if (point_set=create_interactive_streampoint(seed_element,
							coordinate_field,length,seed_xi))
						{
							GT_OBJECT_ADD(GT_pointset)(graphics_object,/*time*/0.0,point_set);
							rc_coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							wrapper_stream_vector_field=
								Computed_field_begin_wrap_orientation_scale_field(
									stream_vector_field,rc_coordinate_field);
							/* build the streamline */
							if (STREAM_LINE==streamline_type)
							{
								if (polyline=create_GT_polyline_streamline_FE_element(
									seed_element,seed_xi,rc_coordinate_field,
									wrapper_stream_vector_field,reverse_track,
									length,streamline_data_type,data_field))
								{
									if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
										line_graphics_object,/*time*/0.0,polyline)))
									{
										DESTROY(GT_polyline)(&polyline);
									}
								}
								else
								{
									return_code=0;
								}
							}
							else if ((streamline_type == STREAM_RIBBON)||
								(streamline_type == STREAM_EXTRUDED_RECTANGLE)||
								(streamline_type == STREAM_EXTRUDED_ELLIPSE))
							{
								if (surface=create_GT_surface_streamribbon_FE_element(
									seed_element,seed_xi,rc_coordinate_field,
									wrapper_stream_vector_field,reverse_track,
									length,width,streamline_type,streamline_data_type,data_field))
								{
									if (!(return_code=GT_OBJECT_ADD(GT_surface)(
										line_graphics_object,/*time*/0.0,surface)))
									{
										DESTROY(GT_surface)(&surface);
									}
								}
								else
								{
									return_code=0;
								}
							}
							if (return_code)
							{
								if (STREAM_NO_DATA != streamline_data_type)
								{
									return_code=set_GT_object_Spectrum(graphics_object,spectrum)&&
										set_GT_object_Spectrum(line_graphics_object,spectrum);
								}
								streamline=CREATE(Interactive_streamline)(
									graphics_object_name,streamline_type,seed_element,seed_xi,
									rc_coordinate_field,wrapper_stream_vector_field,reverse_track,
									length,width,streamline_data_type,data_field,graphics_object,
									line_graphics_object);
								ADD_OBJECT_TO_MANAGER(Interactive_streamline)(streamline,
									command_data->interactive_streamline_manager);
								/* bring up interactive_streamline dialog */
								bring_up_interactive_streamline_dialog(
									&(command_data->interactive_streamlines_dialog),
									command_data->user_interface->application_shell,
									command_data->interactive_streamline_manager,
									streamline,command_data->user_interface,
									command_data->scene_manager);
							}
							Computed_field_end_wrap(&wrapper_stream_vector_field);
							Computed_field_end_wrap(&rc_coordinate_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_interactive_streamline.  "
								"Unable to create streampoint");
							return_code=0;
						}
						if (!return_code)
						{
							display_message(WARNING_MESSAGE,"No streamline created");
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			DEACCESS(Spectrum)(&spectrum);
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_interactive_streamline.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_interactive_streamline.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_interactive_streamline */
#endif /* defined (OLD_CODE) */

static int gfx_create_surfaces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE SURFACES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name,nurb,*render_type_string,
		reverse_normals,**valid_strings;
	enum GT_object_type object_type;
	enum Render_type render_type;
	float time;
	gtObject *graphics_object;
	int face_number,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*data_field,*texture_coordinate_field;
	struct Element_discretization discretization;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Element_to_surface_data element_to_surface_data;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_texture_coordinate_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_surfaces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("surfaces");
			element_group=(struct GROUP(FE_element) *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			texture_coordinate_field=(struct Computed_field *)NULL;
			time=0;
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			discretization.number_in_xi1=4;
			discretization.number_in_xi2=4;
			discretization.number_in_xi3=0;
			exterior_flag=0;
			face_number=0;
			nurb=0;
			reverse_normals=0;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&exterior_flag,
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&face_number,
				NULL,set_exterior);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* nurb */
			Option_table_add_entry(option_table,"nurb",&nurb,NULL,set_char_flag);
			/* reverse_normals */
			Option_table_add_entry(option_table,"reverse_normals",
				&reverse_normals,NULL,set_char_flag);
			/* render_type */
			render_type = RENDER_TYPE_SHADED;
			render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* texture_coordinates */
			set_texture_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_texture_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_texture_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"texture_coordinates",
				&texture_coordinate_field,&set_texture_coordinate_field_data,
				set_Computed_field_conditional);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* with */
			Option_table_add_entry(option_table,"with",&discretization,
				command_data->user_interface,set_Element_discretization);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (nurb)
				{
					object_type = g_NURBS;
				}
				else
				{
					object_type = g_SURFACE;
				}
				face_number -= 2;
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (object_type==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						object_type,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_surfaces.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				element_to_surface_data.coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				element_to_surface_data.element_group=element_group;
				element_to_surface_data.exterior=exterior_flag;
				element_to_surface_data.face_number=face_number;
				element_to_surface_data.object_type = object_type;
				element_to_surface_data.data_field=data_field;
				element_to_surface_data.reverse_normals=reverse_normals;
				element_to_surface_data.graphics_object=graphics_object;
					STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
				element_to_surface_data.render_type = render_type;
				element_to_surface_data.texture_coordinate_field=
					texture_coordinate_field;
				element_to_surface_data.time=time;
				element_to_surface_data.number_of_segments_in_xi1=
					discretization.number_in_xi1;
				element_to_surface_data.number_of_segments_in_xi2=
					discretization.number_in_xi2;
				if (element_group)
				{
					return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						element_to_surface,(void *)&element_to_surface_data,
						element_group);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						element_to_surface,(void *)&element_to_surface_data,
						command_data->element_manager);
				}
				if (return_code)
				{
					if (!GT_object_has_time(graphics_object,time))
					{
						/* add a NULL surface to make an empty time */
						GT_OBJECT_ADD(GT_surface)(graphics_object,time,
							(struct GT_surface *)NULL);
					}
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					if (return_code)
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_surfaces.  No surfaces created");
						return_code=0;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_surfaces.  Error creating surfaces");
					}
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
				Computed_field_end_wrap(&(element_to_surface_data.coordinate_field));
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_surfaces.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_surfaces.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_surfaces */

static int set_Texture_image_from_field(struct Texture *texture, struct Computed_field *field,
	struct Computed_field *texture_coordinate_field, struct Spectrum *spectrum,
	struct GROUP(FE_element) *element_group, enum Texture_storage_type storage,
	int image_width,int image_height,
	char *image_file_name,int crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 September 2000

DESCRIPTION :
Creates the image in the format given by sampling the <field> according to the
reverse mapping of the <texture_coordinate_field>.  The values returned by
field are converted to "colours" by applying the <spectrum>
==============================================================================*/
{
	FE_value *data_values, values[3], xi[3];
	float hint_minimums[3] = {0.0, 0.0, 0.0};
	float hint_maximums[3] = {1.0, 1.0, 1.0};
	float hint_resolution[3];
	float	red, green, blue, alpha, texture_height, texture_width;
	int j,k,number_of_components,number_of_data_components,return_code,
		tex_number_of_components;
	long unsigned *texture_image;
	struct Colour result;
	struct Computed_field_find_element_xi_special_cache *cache;
	struct FE_element *element;
	struct Graphical_material *material;
	unsigned char *ptr;

	ENTER(set_Texture_image_from_field);
	if (texture&&field&&texture_coordinate_field&&spectrum&&
		(4>=(number_of_components = Texture_get_number_of_components_from_storage_type(storage)))
		&&(3>=(tex_number_of_components = Computed_field_get_number_of_components(texture_coordinate_field)))
		&&Computed_field_is_find_element_xi_capable(texture_coordinate_field, /*dummy*/NULL))
	{
		cache = (struct Computed_field_find_element_xi_special_cache *)NULL;
		number_of_data_components = Computed_field_get_number_of_components(field);
		Texture_get_physical_size(texture, &texture_width, &texture_height);
		if (ALLOCATE(texture_image,unsigned long,
			(image_width*image_height*number_of_components+3)/4) &&
			 ALLOCATE(data_values, FE_value, Computed_field_get_number_of_components(field))
			&& (material = CREATE(Graphical_material)("texture_tmp")))
		{
			hint_resolution[0] = image_width;
			hint_resolution[1] = image_height;
			hint_resolution[2] = 0;
			ptr = (unsigned char *)texture_image;
			values[2] = 0.0;
			for (j=0;j<image_height;j++)
			{
				values[1] = texture_height * ((float)j + 0.5) / (float)image_height;
				for (k = 0 ; k < image_width ; k++)
				{
					values[0] = texture_width * ((float)k + 0.5) / (float)image_width;
					if(Computed_field_find_element_xi_special(texture_coordinate_field,
						&cache, values, tex_number_of_components, &element, xi, element_group,
						user_interface, hint_minimums, hint_maximums, hint_resolution)
						|| Computed_field_find_element_xi(texture_coordinate_field,
						values, tex_number_of_components, &element, xi, element_group))
					{
						/* Computed_field_find_element_xi_special returns true if it has
							performed a valid calculation even if the element isn't found
							to stop the slow Computed_field_find_element_xi being called */
						if (element)
						{
							if(Computed_field_evaluate_in_element(field,
								element, xi,(struct FE_element *)NULL, data_values, (FE_value *)NULL))
							{
								if (spectrum_render_value_on_material(spectrum,
									material, number_of_data_components, data_values))
								{
									Graphical_material_get_diffuse(material, &result);
									red = result.red;
									green = result.green;
									blue = result.blue;
									Graphical_material_get_alpha(material, &alpha);
								}
								else
								{
									red = 0.5;
									green = 0.5;
									blue = 0.5;
									alpha = 1.0;
									display_message(ERROR_MESSAGE,
										"set_Texture_image_from_field.  Unable to evaluate spectrum");
								}
							}
							else
							{
								red = 0.5;
								green = 0.5;
								blue = 0.5;
								alpha = 1.0;
								display_message(ERROR_MESSAGE,
									"set_Texture_image_from_field.  Unable to evaluate field in element");
							}
						}
						else
						{
							red = 0.5;
							green = 0.5;
							blue = 0.5;
							alpha = 1.0;
						}
					}
					else
					{
						red = 0.5;
						green = 0.5;
						blue = 0.5;
						alpha = 1.0;
						display_message(ERROR_MESSAGE,
							"set_Texture_image_from_field.  Unable to find element_xi for values");
					}
					switch(storage)
					{
						case TEXTURE_LUMINANCE:
						{
							*ptr = (unsigned char)((red + green + blue) * 255.0 / 3.0);
							ptr++;
						} break;
						case TEXTURE_LUMINANCE_ALPHA:
						{
							*ptr = (unsigned char)((red + green + blue) * 255.0 / 3.0);
							ptr++;
							*ptr = (unsigned char)(alpha * 255.0);
							ptr++;
						} break;
						case TEXTURE_RGB:
						{
							*ptr = (unsigned char)(red * 255.0);
							ptr++;
							*ptr = (unsigned char)(green * 255.0);
							ptr++;
							*ptr = (unsigned char)(blue * 255.0);
							ptr++;
						} break;
						case TEXTURE_RGBA:
						{
							*ptr = (unsigned char)(red * 255.0);
							ptr++;
							*ptr = (unsigned char)(green * 255.0);
							ptr++;
							*ptr = (unsigned char)(blue * 255.0);
							ptr++;
							*ptr = (unsigned char)(alpha * 255.0);
							ptr++;
						} break;
						case TEXTURE_ABGR:
						{
							*ptr = (unsigned char)(alpha * 255.0);
							ptr++;
							*ptr = (unsigned char)(blue * 255.0);
							ptr++;
							*ptr = (unsigned char)(green * 255.0);
							ptr++;
							*ptr = (unsigned char)(red * 255.0);
							ptr++;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"set_Texture_image_from_field.  Unsupported storage type.");
							return_code=0;
						} break;
					}
				}
			}
			Texture_set_image(texture, texture_image,
				storage, /* number_of_bytes_per_component */ 1,
				image_width, image_height, TEXTURE_BOTTOM_TO_TOP,
				image_file_name, crop_left_margin, crop_bottom_margin,
				crop_width, crop_height,/*perform_crop*/0);
			Computed_field_clear_cache(field);
			Computed_field_clear_cache(texture_coordinate_field);
			DEALLOCATE(data_values);
			DEALLOCATE(texture_image);
			DESTROY(Graphical_material)(&material);
			if (cache)
			{
				DESTROY(Computed_field_find_element_xi_special_cache)(&cache);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Could not allocate texture image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Texture_image_from_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Texture_image_from_field */

struct Texture_evaluate_image_data
{
	enum Texture_storage_type storage;
	int height_texels;
	int width_texels;
	struct Computed_field *field, *texture_coordinates_field;
	struct GROUP(FE_element) *element_group;
	struct Spectrum *spectrum;	
};

static int gfx_modify_Texture_evaluate_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_field_data,
		set_texture_coordinates_field_data;
	struct Texture_evaluate_image_data *data;

	ENTER(gfx_modify_Texture_evaluate_image);
	if (state)
	{
		if (state->current_token)
		{
			if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
				(data = (struct Texture_evaluate_image_data *)data_void))
			{

				option_table = CREATE(Option_table)();
				/* element_group */
				Option_table_add_entry(option_table, "element_group", &data->element_group,
					command_data->element_group_manager, set_FE_element_group);
				/* field */
				set_field_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
				set_field_data.conditional_function=
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				set_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table, "field", &data->field,
					&set_field_data, set_Computed_field_conditional);
				/* format */
				Option_table_add_entry(option_table, "format", &data->storage,
					NULL, set_Texture_storage);
				/* height_texels */
				Option_table_add_entry(option_table, "height_texels", &data->height_texels,
					NULL, set_int_positive);
				/* spectrum */
				Option_table_add_entry(option_table, "spectrum", &data->spectrum, 
					command_data->spectrum_manager, set_Spectrum);
				/* width_texels */
				Option_table_add_entry(option_table, "width_texels", &data->width_texels, 
					NULL, set_int_positive);
				/* texture_coordinates */
				set_texture_coordinates_field_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
				set_texture_coordinates_field_data.conditional_function=
					Computed_field_is_find_element_xi_capable;
				set_texture_coordinates_field_data.conditional_function_user_data=
					(void *)NULL;
				Option_table_add_entry(option_table, "texture_coordinates",
					&data->texture_coordinates_field,
					&set_texture_coordinates_field_data, set_Computed_field_conditional);

				return_code=Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture_evaluate_image.  Missing command_data_void");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_evaluate_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_evaluate_image */

struct Texture_image_data
{
	char *image_file_name;
	int crop_bottom_margin,crop_left_margin,crop_height,crop_width;
};

static int gfx_modify_Texture_image(struct Parse_state *state,void *data_void,
	void *set_file_name_option_table_void)
/*******************************************************************************
LAST MODIFIED : 12 October 2000

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Modifier_entry *entry;
	struct Texture_image_data *data;

	ENTER(gfx_modify_Texture_image);
	if (state && (data = (struct Texture_image_data *)data_void) &&
		(entry=(struct Modifier_entry *)set_file_name_option_table_void))
	{
		return_code=1;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare("crop",current_token))
				{
					if (!(shift_Parse_state(state,1)&&
						(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_left_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_bottom_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_width)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_height)))&&
						shift_Parse_state(state,1)))
					{
						display_message(WARNING_MESSAGE,"Missing/invalid crop value(s)");
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" <crop LEFT_MARGIN#[0] BOTTOM_MARGIN#[0] WIDTH#[0] HEIGHT#[0]>");
			}
		}
		if (return_code)
		{
			if (current_token=state->current_token)
			{
				while (entry->option)
				{
					entry->to_be_modified= &(data->image_file_name);
					entry++;
				}
				entry->to_be_modified= &(data->image_file_name);
				return_code=process_option(state,
					(struct Modifier_entry *)set_file_name_option_table_void);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx modify texture image:  Missing image file name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_image */

int gfx_modify_Texture(struct Parse_state *state,void *texture_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 October 2000

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *combine_mode_string, *current_token, *field_name, *filter_mode_string,
		*raw_image_storage_string, **valid_strings, *wrap_mode_string;
	double texture_distortion[3];
	float alpha,distortion_centre_x,distortion_centre_y,distortion_factor_k1,
		height,width;
	int number_of_valid_strings, process, return_code, specify_height,
		specify_width;
	struct Cmiss_command_data *command_data;
	struct Colour colour;
	struct Option_table *option_table;
	struct Texture *texture_to_be_modified,*texture_to_be_modified_copy;
	struct Texture_evaluate_image_data evaluate_data;
	struct Texture_image_data image_data;
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_distortion_data=
		{3," DISTORTION_CENTRE_X DISTORTION_CENTRE_Y DISTORTION_FACTOR_K1",0};
#if defined (SGI_MOVIE_FILE)
	struct Movie_graphics *movie,*old_movie;
	struct X3d_movie *x3d_movie;
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(gfx_modify_Texture);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				process=0;
				if (texture_to_be_modified=(struct Texture *)texture_void)
				{
					if (IS_MANAGED(Texture)(texture_to_be_modified,
						command_data->texture_manager))
					{
						if (texture_to_be_modified_copy=CREATE(Texture)((char *)NULL))
						{
							MANAGER_COPY_WITH_IDENTIFIER(Texture,name)(
								texture_to_be_modified_copy,texture_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create texture copy");
							return_code=0;
						}
					}
					else
					{
						texture_to_be_modified_copy=texture_to_be_modified;
						texture_to_be_modified=(struct Texture *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (command_data->texture_manager)
						{
							if (texture_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,
								name)(current_token,command_data->texture_manager))
							{
								if (return_code=shift_Parse_state(state,1))
								{
									if (texture_to_be_modified_copy=CREATE(Texture)((char *)NULL))
									{
										MANAGER_COPY_WITH_IDENTIFIER(Texture,name)(
											texture_to_be_modified_copy,texture_to_be_modified);
										process=1;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"gfx_modify_Texture.  Could not create texture copy");
										return_code=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown texture : %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing texture manager");
							return_code=0;
						}
					}
					else
					{
						if (texture_to_be_modified=CREATE(Texture)((char *)NULL))
						{
							option_table = CREATE(Option_table)();
							Option_table_add_entry(option_table, "TEXTURE_NAME",
								(void *)texture_to_be_modified,command_data_void,gfx_modify_Texture);
							return_code = Option_table_parse(option_table, state);
								/*???DB.  return_code will be 0 ? */
							DESTROY(Option_table)(&option_table);
							DESTROY(Texture)(&texture_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create dummy texture");
							return_code=0;
						}
					}
				}
				if (process)
				{
#if defined (SGI_MOVIE_FILE)
					if (x3d_movie=Texture_get_movie(texture_to_be_modified_copy))
					{
						if (movie=FIRST_OBJECT_IN_MANAGER_THAT(Movie_graphics)(
							Movie_graphics_has_X3d_movie,(void *)x3d_movie,
							command_data->movie_graphics_manager))
						{
							ACCESS(Movie_graphics)(movie);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing Movie_graphics for X3d_movie");
						}
					}
					else
					{
						movie = (struct Movie_graphics *)NULL;
					}
					old_movie=movie;
#endif /* defined (SGI_MOVIE_FILE) */
					Texture_get_combine_alpha(texture_to_be_modified_copy, &alpha);
					Texture_get_combine_colour(texture_to_be_modified_copy, &colour);
					Texture_get_physical_size(texture_to_be_modified_copy,&width,&height);
					Texture_get_distortion_info(texture_to_be_modified_copy,
						&distortion_centre_x,&distortion_centre_y,&distortion_factor_k1);
					texture_distortion[0]=(double)distortion_centre_x;
					texture_distortion[1]=(double)distortion_centre_y;
					texture_distortion[2]=(double)distortion_factor_k1;

					specify_width=0;
					specify_height=0;

					image_data.image_file_name=(char *)NULL;
					image_data.crop_left_margin=0;
					image_data.crop_bottom_margin=0;
					image_data.crop_width=0;
					image_data.crop_height=0;

					evaluate_data.field = (struct Computed_field *)NULL;
					evaluate_data.spectrum = (struct Spectrum *)NULL;
					evaluate_data.element_group = (struct GROUP(FE_element) *)NULL;
					evaluate_data.height_texels = 100;
					evaluate_data.width_texels = 100;
					evaluate_data.storage = TEXTURE_RGB;
					evaluate_data.texture_coordinates_field =
						(struct Computed_field *)NULL;
					
					option_table = CREATE(Option_table)();
					/* alpha */
					Option_table_add_entry(option_table, "alpha", &alpha,
					  NULL,set_float_0_to_1_inclusive);
					/* blend/decal/modulate */
					combine_mode_string = Texture_combine_mode_string(
						Texture_get_combine_mode(texture_to_be_modified_copy));
					valid_strings =
						Texture_combine_mode_get_valid_strings(&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&combine_mode_string);
					DEALLOCATE(valid_strings);
					/* clamp_wrap/repeat_wrap */
					wrap_mode_string = Texture_wrap_mode_string(
						Texture_get_wrap_mode(texture_to_be_modified_copy));
					valid_strings =
						Texture_wrap_mode_get_valid_strings(&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&wrap_mode_string);
					DEALLOCATE(valid_strings);
					/* colour */
					Option_table_add_entry(option_table, "colour", &colour,
					  NULL,set_Colour);
					/* distortion */
					Option_table_add_entry(option_table, "distortion",
						&texture_distortion,
					  &texture_distortion_data,set_double_vector_with_help);
					/* height */
					Option_table_add_entry(option_table, "height", &height,
					  NULL,set_float_positive);
					/* image */
					Option_table_add_entry(option_table, "image",
						&image_data, command_data->set_file_name_option_table,
						gfx_modify_Texture_image);
					/* linear_filter/nearest_filter */
					filter_mode_string = Texture_filter_mode_string(
						Texture_get_filter_mode(texture_to_be_modified_copy));
					valid_strings =
						Texture_filter_mode_get_valid_strings(&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&filter_mode_string);
					DEALLOCATE(valid_strings);
#if defined (SGI_MOVIE_FILE)
					/* movie */
					Option_table_add_entry(option_table, "movie", &movie,
					  command_data->movie_graphics_manager, set_Movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
					/* raw image storage mode */
					raw_image_storage_string=Raw_image_storage_string(RAW_PLANAR_RGB);
					valid_strings =
						Raw_image_storage_get_valid_strings(&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&raw_image_storage_string);
					DEALLOCATE(valid_strings);
					/* specify_height */
					Option_table_add_entry(option_table, "specify_height",&specify_height,
					  NULL,set_int_non_negative);
					/* specify_width */
					Option_table_add_entry(option_table, "specify_width",&specify_width,
					  NULL,set_int_non_negative);
					/* width */
					Option_table_add_entry(option_table, "width", &width,
					  NULL,set_float_positive);
					/* evaluate_image */
					Option_table_add_entry(option_table, "evaluate_image",
					  &evaluate_data, command_data, gfx_modify_Texture_evaluate_image);
					return_code=Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						if (evaluate_data.field || evaluate_data.element_group ||
							evaluate_data.spectrum || evaluate_data.texture_coordinates_field)
						{
							if((!evaluate_data.field) || (!evaluate_data.element_group) ||
								(!evaluate_data.spectrum) ||
								(!evaluate_data.texture_coordinates_field))
							{
								display_message(ERROR_MESSAGE,
									"To evaluate the texture image from a field you must specify\n"
									"a field, element_group, spectrum and texture_coordinates");
								return_code = 0;
							}
						}
					}
					if (return_code)
					{
						if (image_data.image_file_name)
						{
							return_code=Texture_set_image_file(texture_to_be_modified_copy,
								image_data.image_file_name,specify_width,specify_height,
								Raw_image_storage_from_string(raw_image_storage_string),
								image_data.crop_left_margin,image_data.crop_bottom_margin,
								image_data.crop_width,image_data.crop_height,
								0.0,0.0,0.0);
						}

						Texture_set_combine_alpha(texture_to_be_modified_copy, alpha);
						Texture_set_combine_colour(texture_to_be_modified_copy, &colour);
						Texture_set_physical_size(texture_to_be_modified_copy,width,height);

						Texture_set_combine_mode(texture_to_be_modified_copy,
							Texture_combine_mode_from_string(combine_mode_string));

						Texture_set_filter_mode(texture_to_be_modified_copy,
							Texture_filter_mode_from_string(filter_mode_string));

						Texture_set_wrap_mode(texture_to_be_modified_copy,
							Texture_wrap_mode_from_string(wrap_mode_string));

						if (texture_distortion_data.set)
						{
							distortion_centre_x=(float)texture_distortion[0];
							distortion_centre_y=(float)texture_distortion[1];
							distortion_factor_k1=(float)texture_distortion[2];
							Texture_set_distortion_info(texture_to_be_modified_copy,
								distortion_centre_x,distortion_centre_y,distortion_factor_k1);
						}
						if (texture_to_be_modified)
						{
							MANAGER_MODIFY_NOT_IDENTIFIER(Texture,name)(
								texture_to_be_modified,texture_to_be_modified_copy,
								command_data->texture_manager);
							DESTROY(Texture)(&texture_to_be_modified_copy);
						}
						else
						{
							texture_to_be_modified=texture_to_be_modified_copy;
						}
#if defined (SGI_MOVIE_FILE)
						if ( movie != old_movie )
						{
							/* Movie is outside manager copy so that is updates
								the correct texture based on movie events */
							Texture_set_movie(texture_to_be_modified,
								Movie_graphics_get_X3d_movie(movie),
								command_data->user_interface, "movie");
						}
#endif /* defined (SGI_MOVIE_FILE) */
						if (evaluate_data.field && evaluate_data.spectrum && 
							evaluate_data.texture_coordinates_field)
						{
							GET_NAME(Computed_field)(evaluate_data.field, &field_name);
							set_Texture_image_from_field(texture_to_be_modified, 
								evaluate_data.field,
								evaluate_data.texture_coordinates_field, 
								evaluate_data.spectrum, evaluate_data.element_group,
								evaluate_data.storage, evaluate_data.width_texels, 
								evaluate_data.height_texels, field_name,
								0, 0, 0, 0, command_data->user_interface);
							DEALLOCATE(field_name);
						}
#if defined (GL_API)
						texture_to_be_modified->index= -(texture_to_be_modified->index);
#endif
#if defined (MS_22AUG96)
#if defined (OPENGL_API)
						if (texture_to_be_modified->list_index)
						{
							glDeleteLists(texture_to_be_modified->list_index,1);
							texture_to_be_modified->list_index=0;
						}
#endif
#endif /* defined (MS_22AUG96) */
					}
					if (image_data.image_file_name)
					{
						DEALLOCATE(image_data.image_file_name);
					}
					DESTROY(Option_table)(&option_table);
#if defined (SGI_MOVIE_FILE)
					if (movie)
					{
						DEACCESS(Movie_graphics)(&movie);
					}
#endif /* defined (SGI_MOVIE_FILE) */
					if (evaluate_data.element_group)
					{
						DEACCESS(GROUP(FE_element))(&evaluate_data.element_group);
					}
					if (evaluate_data.spectrum)
					{
						DEACCESS(Spectrum)(&evaluate_data.spectrum);
					}
					if (evaluate_data.field)
					{
						DEACCESS(Computed_field)(&evaluate_data.field);
					}
					if (evaluate_data.texture_coordinates_field)
					{
						DEACCESS(Computed_field)(&evaluate_data.texture_coordinates_field);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			if (texture_void)
			{
				display_message(WARNING_MESSAGE,"Missing texture modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing texture name");
			}
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_Texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE TEXTURE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_create_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (command_data->texture_manager)
					{
						if (!FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(current_token,
							command_data->texture_manager))
						{
							if (texture=CREATE(Texture)(current_token))
							{
								shift_Parse_state(state,1);
								if (state->current_token)
								{
									return_code=gfx_modify_Texture(state,(void *)texture,
										command_data_void);
								}
								else
								{
									return_code=1;
								}
								if (return_code)
								{
									ADD_OBJECT_TO_MANAGER(Texture)(texture,
										command_data->texture_manager);
								}
								else
								{
									DESTROY(Texture)(&texture);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture.  Error creating texture");
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"Texture '%s' already exists",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_texture.  Missing texture_manager");
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Texture(state,(void *)NULL,
						command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_texture */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_texture_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Executes a GFX CREATE TEXMAP command.
==============================================================================*/
{
	char *in_file_name,*out_file_name;
	double xi_max[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"extent",NULL,NULL,set_Element_discretization},
		{"from",NULL,(void *)1,set_name},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field;
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;
	struct Graphics_window *window;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(gfx_create_texture_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(out_file_name,char,11))
			{
				strcpy(out_file_name,"texmap.rgb");
			}
			in_file_name=(char *)NULL;
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package));
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);;
			(option_table[0]).to_be_modified= &out_file_name;
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &coordinate_field;
			(option_table[1]).user_data=&set_coordinate_field_data;
			(option_table[2]).to_be_modified= &extent;
			(option_table[2]).user_data=(void *)(command_data->user_interface);
			(option_table[3]).to_be_modified= &in_file_name;
			(option_table[4]).to_be_modified= &seed_element;
			(option_table[4]).user_data=command_data->element_manager;
			(option_table[5]).to_be_modified= &window;
			(option_table[5]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					if (seed_element)
					{
						xi_max[0]=(double)(extent.number_in_xi1);
						xi_max[1]=(double)(extent.number_in_xi2);
						xi_max[2]=(double)(extent.number_in_xi3);
						if (in_file_name&&out_file_name&&coordinate_field)
						{
							generate_textureimage_from_FE_element(window,in_file_name,
								out_file_name,seed_element,xi_max,coordinate_field);
						}
						else
						{
							display_message(WARNING_MESSAGE,"Missing option(s)");
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing seed element");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
			if (in_file_name)
			{
				DEALLOCATE(in_file_name);
			}
			if (out_file_name)
			{
				DEALLOCATE(out_file_name);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_texture_map.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_texture_map */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_time_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE TIME_EDITOR command.
If there is a time editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one time
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_time_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_time_editor_dialog(
					&(command_data->time_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->default_time_keeper, 
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_time_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_time_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_time_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_control_curve_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Executes a GFX CREATE CONTROL_CURVE_EDITOR command.
If there is a variable editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one variable
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_control_curve_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_control_curve_editor_dialog(
					&(command_data->control_curve_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->control_curve_manager,
					(struct Control_curve *)NULL,
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_control_curve_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_control_curve_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_control_curve_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_tracking_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Executes a GFX CREATE TRACKING_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_tracking_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
#if defined (MIRAGE)
				return_code=open_tracking_editor_dialog(
					&(command_data->tracking_editor_dialog),
					tracking_editor_close_cb,
					&(command_data->background_colour),
					command_data->basis_manager,
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package),
					command_data->element_manager,
					command_data->element_group_manager,
					command_data->fe_field_manager,
					command_data->glyph_list,
					command_data->graphical_material_manager,
					command_data->default_graphical_material,
					command_data->graphics_window_manager,
					command_data->light_manager,
					command_data->default_light,
					command_data->light_model_manager,
					command_data->default_light_model,
					command_data->node_manager,
					command_data->node_group_manager,
					command_data->data_manager,
					command_data->data_group_manager,
					command_data->element_point_ranges_selection,
					command_data->element_selection,
					command_data->node_selection,
					command_data->data_selection,
					command_data->scene_manager,
					command_data->default_scene,
					command_data->spectrum_manager,
					command_data->default_spectrum,
					command_data->texture_manager,
					command_data->interactive_tool_manager,
					command_data->user_interface);
#else /* defined (MIRAGE) */
				display_message(ERROR_MESSAGE,"Tracking editor is not available");
				return_code=0;
#endif /* defined (MIRAGE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_tracking_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_tracking_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_tracking_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_robot_7dof(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE OBJECT HEART_ROBOT command.
==============================================================================*/
{
	char *graphics_object_name;
	gtObject *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct GT_userdef *robot_7dof;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"material",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_robot_7dof);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("robot_7dof");
			/* must access it now, because we deaccess it later */
			material=ACCESS(Graphical_material)(command_data->
				default_graphical_material);
			(option_table[0]).to_be_modified= &graphics_object_name;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data=command_data->graphical_material_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(graphics_object_name,
					command_data->graphics_object_list))
				{
					display_message(ERROR_MESSAGE,"Object '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,g_USERDEF,
						material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						if (!((robot_7dof=create_robot_7dof())&&
							GT_OBJECT_ADD(GT_userdef)(graphics_object,0.0,robot_7dof)))
						{
							if (robot_7dof)
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_robot_7dof.  Could not add userdef primitive");
								display_message(ERROR_MESSAGE,"gfx_create_robot_7dof.  "
									"No destroy function for userdef objects - memory lost");
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_robot_7dof.  Could not create 7dof robot");
							}
							/* the object will be automatically destroyed when removed */
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_robot_7dof.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			} /* parse error, help */
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_robot_7dof.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_robot_7dof.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_robot_7dof */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_userdef(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE OBJECT command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"robot_7dof",NULL,NULL,gfx_create_robot_7dof},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_userdef);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (state->current_token)
		{
			(option_table[0]).user_data=command_data_void;
			return_code=process_option(state,option_table);
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				set_command_prompt("gfx create object",command_data->command_window);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_userdef.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_robot_7dof.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_userdef */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_volume_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE VOLUME_EDITOR command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_volume_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				create_texture_edit_window(command_data->default_graphical_material,
					command_data->graphical_material_manager,
					command_data->environment_map_manager,command_data->texture_manager,
					&(command_data->material_editor_dialog),
					&(command_data->transformation_editor_dialog),
					command_data->user_interface);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_volume_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_volume_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_volume_editor */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_create_volumes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED :  29 March 2001

DESCRIPTION :
Executes a GFX CREATE VOLUMES command.
==============================================================================*/
{
	char *graphics_object_name,*render_type_string,**valid_strings;
	enum Render_type render_type;
	float time;
	int displacement_map_xi_direction,number_of_valid_strings, return_code;
	struct Clipping *clipping;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field, *data_field,
		*displacement_map_field, *surface_data_coordinate_field,
		*surface_data_density_field, *blur_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Element_to_volume_data element_to_volume_data;
	struct FE_element *seed_element;
	struct FE_node *data_to_destroy;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *surface_data_group;
	struct GT_object *graphics_object;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_displacement_map_field_data, 
		set_surface_data_coordinate_field_data, set_surface_data_density_field_data,
		set_blur_field_data;
	struct Spectrum *spectrum;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_create_volumes);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("volumes");
			element_group=(struct GROUP(FE_element) *)NULL;
			clipping=(struct Clipping *)NULL;
			coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_manager);
			/* must access it now, because we deaccess it later */
			ACCESS(Computed_field)(coordinate_field);
			data_field=(struct Computed_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			material=
				ACCESS(Graphical_material)(command_data->default_graphical_material);
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			surface_data_group = (struct GROUP(FE_node) *)NULL;
			surface_data_coordinate_field = (struct Computed_field *)NULL;
			surface_data_density_field=(struct Computed_field *)NULL;
			time=0;
			volume_texture=(struct VT_volume_texture *)NULL;
			displacement_map_field = (struct Computed_field *)NULL;
			displacement_map_xi_direction=3;
			blur_field = (struct Computed_field *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* clipping */
			Option_table_add_entry(option_table,"clipping",&clipping,
				NULL,set_Clipping);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&data_field,
				&set_data_field_data,set_Computed_field_conditional);
			/* displacement_map_field */
			set_displacement_map_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_displacement_map_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_displacement_map_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"displacement_map_field",
				&displacement_map_field,&set_displacement_map_field_data,
				set_Computed_field_conditional);
			/* displacement_map_xi_direction */
			Option_table_add_entry(option_table,"displacement_map_xi_direction",
				&displacement_map_xi_direction,NULL,set_int_positive);
			/* from */
			Option_table_add_entry(option_table,"from",&element_group,
				command_data->element_group_manager,set_FE_element_group);
			/* material */
			Option_table_add_entry(option_table,"material",&material,
				command_data->graphical_material_manager,set_Graphical_material);
			/* render_type */
			render_type = RENDER_TYPE_SHADED;
			render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &render_type_string);
			DEALLOCATE(valid_strings);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,command_data->element_manager,set_FE_element_dimension_3);
			/* smooth_field */
			set_blur_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_blur_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_blur_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"smooth_field",
				&blur_field,&set_blur_field_data,set_Computed_field_conditional);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* surface_data_coordinate */
			set_surface_data_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_surface_data_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_surface_data_coordinate_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table,"surface_data_coordinate",
				&surface_data_coordinate_field,
				&set_surface_data_coordinate_field_data,set_Computed_field_conditional);
			/* surface_data_density */
			set_surface_data_density_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_surface_data_density_field_data.conditional_function=
				Computed_field_has_up_to_4_numerical_components;
			set_surface_data_density_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"surface_data_density",
				&surface_data_density_field,&set_surface_data_density_field_data,
				set_Computed_field_conditional);
			/* surface_data_group */
			Option_table_add_entry(option_table,"surface_data_group",
				&surface_data_group,command_data->data_group_manager,set_FE_node_group);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vtexture */
			Option_table_add_entry(option_table,"vtexture",
				&volume_texture,command_data->volume_texture_manager,
				set_VT_volume_texture);
			return_code=Option_table_multi_parse(option_table,state);
			if(surface_data_group&&(!surface_data_density_field))
			{
				display_message(ERROR_MESSAGE,"gfx_create_volumes.  "
					"Must supply a surface_data_density_field with a surface_data_group");
				return_code=0;
			}
			if((!surface_data_group)&&surface_data_density_field)
			{
				display_message(ERROR_MESSAGE,"gfx_create_volumes.  "
					"Must supply a surface_data_group with a surface_data_density_field");
				return_code=0;
			}

			/* no errors, not asking for help */
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_VOLTEX==graphics_object->object_type)
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code=GT_object_delete_time(graphics_object,time);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_VOLTEX,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_volumes.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code && surface_data_group)
				{
					return_code = 1;
					MANAGED_GROUP_BEGIN_CACHE(FE_node)(surface_data_group);

					/* Remove all the current data from the group */
					while(return_code&&(data_to_destroy=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
						((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL, surface_data_group)))
					{
						return_code = REMOVE_OBJECT_FROM_GROUP(FE_node)(
							data_to_destroy, surface_data_group);
						if (FE_node_can_be_destroyed(data_to_destroy))
						{
							return_code = REMOVE_OBJECT_FROM_MANAGER(FE_node)(data_to_destroy,
								command_data->data_manager);
						}
					}
					if(!return_code)
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_volumes.  Unable to remove all data from data_group");
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
				if (return_code)
				{
					element_to_volume_data.coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					element_to_volume_data.data_field=data_field;
					element_to_volume_data.graphics_object=graphics_object;
					element_to_volume_data.time=time;
					STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
					element_to_volume_data.render_type = render_type;
					element_to_volume_data.volume_texture=volume_texture;
					element_to_volume_data.displacement_map_field =
						displacement_map_field;
					element_to_volume_data.displacement_map_xi_direction =
						displacement_map_xi_direction;
					element_to_volume_data.surface_data_coordinate_field = 
						surface_data_coordinate_field;
					element_to_volume_data.computed_field_manager =
						Computed_field_package_get_computed_field_manager(
							command_data->computed_field_package);
					element_to_volume_data.surface_data_density_field = 
						surface_data_density_field;
					element_to_volume_data.surface_data_group = 
						surface_data_group;
					element_to_volume_data.data_manager = 
						command_data->data_manager;
					element_to_volume_data.fe_field_manager =
						command_data->fe_field_manager;
					element_to_volume_data.blur_field= blur_field;
					element_to_volume_data.clipping=clipping;
					if (seed_element)
					{
						return_code=element_to_volume(seed_element,
							(void *)&element_to_volume_data);
					}
					else
					{
						/* no seed element specified, use all elements in the group as
							seeds*/
						if (element_group)
						{
							return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								element_to_volume,(void *)&element_to_volume_data,
								element_group);
						}
						else
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								element_to_volume,(void *)&element_to_volume_data,
								command_data->element_manager);
						}
					}
					if (return_code)
					{
						if (!GT_object_has_time(graphics_object,time))
						{
							/* add a NULL voltex to make an empty time */
							GT_OBJECT_ADD(GT_voltex)(graphics_object,time,
								(struct GT_voltex *)NULL);
						}
						if (data_field)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No volumes created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
					Computed_field_end_wrap(
						&(element_to_volume_data.coordinate_field));
					if (surface_data_group)
					{
						MANAGED_GROUP_END_CACHE(FE_node)(surface_data_group);
					}
				}
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			/* DEACCESS blur and displacement map stuff ? */
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (data_field)
			{
				DEACCESS(Computed_field)(&data_field);
			}
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			if (clipping)
			{
				DESTROY(Clipping)(&clipping);
			}
			DEALLOCATE(graphics_object_name);
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			if (surface_data_coordinate_field)
			{
				DEACCESS(Computed_field)(&surface_data_coordinate_field);
			}
			if (surface_data_density_field)
			{
				DEACCESS(Computed_field)(&surface_data_density_field);
			}
			if (surface_data_group)
			{
				DEACCESS(GROUP(FE_node))(&surface_data_group);
			}
			if (volume_texture)
			{
				DEACCESS(VT_volume_texture)(&volume_texture);
			}
			if (displacement_map_field)
			{
				DEACCESS(Computed_field)(&displacement_map_field);
			}
			if (blur_field)
			{
				DEACCESS(Computed_field)(&blur_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_volumes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_volumes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_volumes */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_volume_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE VTEXTURE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_VT_volume_texture_data modify_VT_volume_texture_data;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_create_volume_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,name)(
						current_token,command_data->volume_texture_manager))
					{
						if (volume_texture=CREATE(VT_volume_texture)(current_token))
						{
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_VT_volume_texture_data.graphical_material_manager=
									command_data->graphical_material_manager;
								modify_VT_volume_texture_data.environment_map_manager=
									command_data->environment_map_manager;
								modify_VT_volume_texture_data.volume_texture_manager=
									command_data->volume_texture_manager;
								modify_VT_volume_texture_data.set_file_name_option_table=
									command_data->set_file_name_option_table;
								return_code=modify_VT_volume_texture(state,
									(void *)volume_texture,
									(void *)(&modify_VT_volume_texture_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(VT_volume_texture)(volume_texture,
								command_data->volume_texture_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_volume_texture.  Error creating volume texture");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_volume_texture.  Missing command_data_void");
					return_code=0;
				}
			}
			else
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					modify_VT_volume_texture_data.graphical_material_manager=
						command_data->graphical_material_manager;
					modify_VT_volume_texture_data.environment_map_manager=
						command_data->environment_map_manager;
					modify_VT_volume_texture_data.volume_texture_manager=
						command_data->volume_texture_manager;
					modify_VT_volume_texture_data.set_file_name_option_table=
						command_data->set_file_name_option_table;
					return_code=modify_VT_volume_texture(state,(void *)NULL,
						(void *)(&modify_VT_volume_texture_data));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_volume_texture.  Missing command_data_void");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing volume texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_volume_texture */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX CREATE WINDOW command.
==============================================================================*/
{
	char double_buffer_flag,*name,single_buffer_flag;
	enum Scene_viewer_buffer_mode buffer_mode;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry
		buffer_option_table[]=
		{
			{"single_buffer",NULL,NULL,set_char_flag},
			{"double_buffer",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"name",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,set_name}
		};

	ENTER(gfx_create_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* set_defaults */
			name=Graphics_window_manager_get_new_name(
				command_data->graphics_window_manager);
			buffer_mode=SCENE_VIEWER_DOUBLE_BUFFER;
			if (state->current_token)
			{
				/* change defaults */
				single_buffer_flag=0;
				double_buffer_flag=0;
				(option_table[0]).to_be_modified= &name;
				(buffer_option_table[0]).to_be_modified= &single_buffer_flag;
				(buffer_option_table[1]).to_be_modified= &double_buffer_flag;
				(option_table[1]).user_data=buffer_option_table;
				(option_table[2]).to_be_modified= &name;
				if (return_code=process_multiple_options(state,option_table))
				{
					if (single_buffer_flag)
					{
						buffer_mode=SCENE_VIEWER_SINGLE_BUFFER;
						if (double_buffer_flag)
						{
							display_message(ERROR_MESSAGE,
								"Only one of single_buffer/double_buffer");
							return_code=0;
						}
					}
					else
					{
						buffer_mode=SCENE_VIEWER_DOUBLE_BUFFER;
					}
				}
			}
			if (!name)
			{
				display_message(ERROR_MESSAGE,"gfx_create_window.  Missing name");
				return_code=0;
			}
			if (return_code)
			{
				if (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(name,
					command_data->graphics_window_manager))
				{
					display_message(WARNING_MESSAGE,
						"Graphics window '%s' already exists",name);
					return_code=0;
				}
				else
				{
					if (window=CREATE(Graphics_window)(name,
						buffer_mode,&(command_data->background_colour),
						command_data->light_manager,command_data->default_light,
						command_data->light_model_manager,command_data->default_light_model,
						command_data->scene_manager,command_data->default_scene,
						command_data->texture_manager,
						command_data->interactive_tool_manager,
						command_data->user_interface))
					{
						if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							command_data->graphics_window_manager))
						{
							DESTROY(Graphics_window)(&window);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_window.  Could not create graphics window");
						return_code=0;
					}
				}
			}
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_window.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_window */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (HAPTIC)
static int gfx_create_haptic(struct Parse_state *state,
	void *dummy__to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 May 1998

DESCRIPTION :
Executes a GFX CREATE HAPTIC command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	float dynamic_friction, static_friction, damping,
		spring_k;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry
		option_table[]=
		{
			{"scene",NULL,NULL,set_Scene},
			{"dynamic_surface_friction",NULL,NULL,set_float_0_to_1_inclusive},
			{"static_surface_friction",NULL,NULL,set_float_0_to_1_inclusive},
			{"damping_surface",NULL,NULL,set_float_0_to_1_inclusive},
			{"spring_k_surface",NULL,NULL,set_float /* SAB set_float_0_to_1_inclusive */},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(gfx_create_haptic);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* set_defaults */
			scene = command_data->default_scene;
			dynamic_friction = 0.2;
			static_friction = 0.35;
			damping = 0.0;
			spring_k = 1.0;
			if (current_token=state->current_token)
			{
				(option_table[0]).to_be_modified= &scene;
				(option_table[0]).user_data=command_data->scene_manager;
				(option_table[1]).to_be_modified= &dynamic_friction;
				(option_table[2]).to_be_modified= &static_friction;
				(option_table[3]).to_be_modified= &damping;
				(option_table[4]).to_be_modified= &spring_k;
				if (return_code=process_multiple_options(state,option_table))
				{
					/* SAB Damping is really in Kg/1000.0*sec and ranges from
						0 to 0.005, so the parameter is scaled here */
					damping *= 0.005;
					haptic_set_surface_defaults( dynamic_friction, static_friction,
						damping, spring_k );
				}
			}
			if ( return_code )
			{
				haptic_create_scene( scene );
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_haptic.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_haptic.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_haptic */
#endif /* defined (HAPTIC) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_3d_digitizer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX CREATE 3D_DIGITIZER command.
If there is a data_grabber dialog in existence, then bring it to the front,
else create a new one.  Assumes we will only ever want one data_grabber at
a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_3d_digitizer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_data_grabber_dialog(
					&(command_data->data_grabber_dialog),
					command_data->user_interface->application_shell,
					command_data->execute_command,command_data->user_interface,
					command_data->fe_field_manager,
					command_data->node_manager, command_data->data_manager,
					command_data->node_group_manager, command_data->data_group_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_3d_digitizer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_3d_digitizer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_3d_digitizer */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_data_sync(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE DATA_SYNC command.
If there is a data_sync dialog in existence, then bring it to the front,
else create a new one.  Assumes we will only ever want one data_sync at
a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_data_sync);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_sync_2d_3d_dialog(
					&(command_data->sync_2d_3d_dialog),
					command_data->user_interface->application_shell,
					command_data->execute_command,command_data->fe_field_manager,
					command_data->data_manager,command_data->data_group_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_sync.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_sync.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_sync */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_create_cmiss(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 9 October 2001

DESCRIPTION :
Executes a GFX CREATE CMISS_CONNECTION command.
==============================================================================*/
{
	char asynchronous_commands,*examples_directory,*host_name,mycm_flag,
		*parameters_file_name;
	enum Machine_type host_type;
	int connection_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
/*???DB.  Not sure if this is quite the right place */
	double wormhole_timeout;
#if defined (MOTIF)
#define XmNwormholeTimeoutSeconds "wormholeTimeoutSeconds"
#define XmCWormholeTimeoutSeconds "WormholeTimeoutSeconds"
	static XtResource resources[]=
	{
		{
			XmNwormholeTimeoutSeconds,
			XmCWormholeTimeoutSeconds,
			XmRInt,
			sizeof(int),
			0,
			XmRString,
			"300"
		}
	};
	int wormhole_timeout_seconds;
#endif /* defined (MOTIF) */

	ENTER(gfx_create_cmiss);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (command_data->cm_examples_directory)
			{
				if (ALLOCATE(examples_directory,char,
					strlen(command_data->cm_examples_directory)+1))
				{
					strcpy(examples_directory,command_data->cm_examples_directory);
				}
				else
				{
					examples_directory=(char *)NULL;
				}
			}
			else
			{
				examples_directory=(char *)NULL;
			}
			if (command_data->cm_parameters_file_name)
			{
				if (ALLOCATE(parameters_file_name,char,
					strlen(command_data->cm_parameters_file_name)+1))
				{
					strcpy(parameters_file_name,command_data->cm_parameters_file_name);
				}
				else
				{
					parameters_file_name=(char *)NULL;
				}
			}
			else
			{
				parameters_file_name=(char *)NULL;
			}
			if ((command_data->user_interface)&&
				(command_data->user_interface->local_machine_info)&&
				(command_data->user_interface->local_machine_info->name))
			{
				if (ALLOCATE(host_name,char,
					strlen(command_data->user_interface->local_machine_info->name)+1))
				{
					strcpy(host_name,
						command_data->user_interface->local_machine_info->name);
				}
				else
				{
					host_name=(char *)NULL;
				}
			}
			else
			{
				host_name=(char *)NULL;
			}
			connection_number=0;
			host_type=MACHINE_UNKNOWN;
			mycm_flag=0;
			asynchronous_commands=0;
			wormhole_timeout=300;
#if defined (MOTIF)
			if (command_data->user_interface)
			{
				XtVaGetApplicationResources(command_data->user_interface->
					application_shell,&wormhole_timeout_seconds,resources,
					XtNumber(resources),NULL);
				wormhole_timeout=(double)wormhole_timeout_seconds;
			}
#endif /* defined (MOTIF) */
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"asynchronous_commands",
				&asynchronous_commands,(void *)NULL,set_char_flag);
			Option_table_add_entry(option_table,"connection_number",
				&connection_number,(void *)NULL,set_int_non_negative);
			Option_table_add_entry(option_table,"examples_directory",
				&examples_directory,(void *)1,set_name);
			Option_table_add_entry(option_table,"host",&host_name,(void *)1,set_name);
			Option_table_add_entry(option_table,"mycm",&mycm_flag,(void *)NULL,
				set_char_flag);
			Option_table_add_entry(option_table,"parameters",&parameters_file_name,
				(void *)1,set_name);
			Option_table_add_entry(option_table,"type",&host_type,(void *)NULL,
				set_machine_type);
			Option_table_add_entry(option_table,(char *)NULL,&host_name,(void *)NULL,
				set_name);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
#if defined (LINK_CMISS)
				if (!CMISS)
				{
					if (MACHINE_UNKNOWN==host_type)
					{
						/* have a stab at identifying it */
						if (fuzzy_string_compare(host_name,"esu"))
						{
							host_type=MACHINE_UNIX;
						}
						else
						{
							if (fuzzy_string_compare(host_name,"esv"))
							{
								host_type=MACHINE_VMS;
							}
						}
					}
					if (CMISS=CREATE(CMISS_connection)(host_name,host_type,
						connection_number,wormhole_timeout,mycm_flag,asynchronous_commands,
						command_data->element_manager,command_data->element_group_manager,
						command_data->fe_field_manager,command_data->node_manager,
						command_data->data_manager,command_data->node_group_manager,
						command_data->data_group_manager,&(command_data->prompt_window),
						parameters_file_name,examples_directory,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_cmiss.  Could not create CMISS connection");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_cmiss.  CMISS connection already exists");
					return_code=0;
				}
#else /* defined (LINK_CMISS) */
			display_message(ERROR_MESSAGE,"gfx_create_cmiss.  Define LINK_CMISS");
			return_code=0;
#endif /* defined (LINK_CMISS) */
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(examples_directory);
			DEALLOCATE(host_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_cmiss.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_cmiss.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_cmiss */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (SELECT_DESCRIPTORS)
static int execute_command_attach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes an ATTACH command.
==============================================================================*/
{
	char *current_token, end_detection, *perl_action, start_detection;
	int return_code;
	struct Io_device *device;
	static struct Option_table *option_table;
	struct Cmiss_command_data *command_data;
	
	ENTER(execute_command_attach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list))
					{
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						if (device = CREATE(Io_device)(current_token))
						{
							if (ADD_OBJECT_TO_LIST(Io_device)(device, command_data->device_list))
							{
								return_code=shift_Parse_state(state,1);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_attach.  Unable to create device struture.");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_attach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_attach.  Missing device name");
			return_code=0;
		}
		if (return_code)
		{
			end_detection = 0;
			perl_action = (char *)NULL;
			start_detection = 0;
			
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,"end_detection", &end_detection, 
				NULL, set_char_flag);
			Option_table_add_entry(option_table,"perl_action", &perl_action, (void *)1,
				set_name);
			Option_table_add_entry(option_table,"start_detection", &start_detection,
				NULL, set_char_flag);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (start_detection && end_detection)
				{
					display_message(ERROR_MESSAGE,"execute_command_attach.  "
						"Specify only one of start_detection and end_detection.");
					return_code=0;
				}
			}
			if (return_code)
			{
				if (start_detection)
				{
					Io_device_start_detection(device, command_data->user_interface);
				}
				if (end_detection)
				{
					Io_device_end_detection(device);
				}
				if (perl_action)
				{
					Io_device_set_perl_action(device, perl_action);
				}
			}
			if (perl_action)
			{
				DEALLOCATE(perl_action);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_attach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_attach */

static int execute_command_detach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes a DETACH command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Io_device *device;
	struct Cmiss_command_data *command_data;
	
	ENTER(execute_command_detach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list))
					{
						if (REMOVE_OBJECT_FROM_LIST(Io_device)(device,
							command_data->device_list))
						{
							if (DESTROY(Io_device)(&device))
							{
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"execute_command_detach.  Unable to destroy device %s.",
									current_token);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_detach.  Unable to remove device %s.",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_detach.  Io_device %s not found.", current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_detach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_detach.  Missing device name");
			return_code=0;
		}
		if (return_code)
		{
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_detach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_detach */
#endif /* defined (SELECT_DESCRIPTORS) */

static int execute_command_gfx_create(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2001

DESCRIPTION :
Executes a GFX CREATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Create_emoter_slider_data create_emoter_slider_data;
	struct Create_node_group_slider_data create_node_group_slider_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_create);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				create_node_group_slider_data.execute_command=
					command_data->execute_command;
				create_node_group_slider_data.fe_field_manager=
					command_data->fe_field_manager;
				create_node_group_slider_data.node_manager=
					command_data->node_manager;
				create_node_group_slider_data.node_group_manager=
					command_data->node_group_manager;
				create_node_group_slider_data.node_group_slider_dialog_address=
					&(command_data->node_group_slider_dialog);
				create_node_group_slider_data.control_curve_manager=
					command_data->control_curve_manager;
				create_node_group_slider_data.user_interface=
					command_data->user_interface;
				if (command_data->user_interface)
				{
					create_node_group_slider_data.parent=
						command_data->user_interface->application_shell;
				}
				else
				{
					create_node_group_slider_data.parent=(Widget)NULL;
				}
				create_emoter_slider_data.execute_command=command_data->execute_command;
				create_emoter_slider_data.fe_field_manager=
					command_data->fe_field_manager;
				create_emoter_slider_data.node_manager=command_data->node_manager;
				create_emoter_slider_data.element_manager=command_data->element_manager;
				create_emoter_slider_data.graphics_window_manager=
					command_data->graphics_window_manager;
				create_emoter_slider_data.data_group_manager=
					command_data->data_group_manager;
				create_emoter_slider_data.element_group_manager=
					command_data->element_group_manager;
				create_emoter_slider_data.node_group_manager=
					command_data->node_group_manager;
				create_emoter_slider_data.control_curve_manager=
					command_data->control_curve_manager;
				create_emoter_slider_data.viewer_scene=command_data->default_scene;
				create_emoter_slider_data.viewer_light=command_data->default_light;
				create_emoter_slider_data.viewer_light_model=
					command_data->default_light_model;
				create_emoter_slider_data.viewer_background_colour=
					command_data->background_colour;
				create_emoter_slider_data.emoter_slider_dialog_address=
					&(command_data->emoter_slider_dialog);
				if (command_data->user_interface)
				{
					create_emoter_slider_data.parent=
						command_data->user_interface->application_shell;
				}
				else
				{
					create_emoter_slider_data.parent=(Widget)NULL;
				}
				create_emoter_slider_data.control_curve_editor_dialog_address=
					&(command_data->control_curve_editor_dialog);
				create_emoter_slider_data.user_interface=
					command_data->user_interface;
#if defined (SCENE_IN_EMOTER)
				create_emoter_slider_data.command_data=command_data;
					/*???DB.  command_data shouldn't leave this module */
#endif /* defined (SCENE_IN_EMOTER) */

				option_table=CREATE(Option_table)();
				Option_table_add_entry(option_table,"annotation",NULL,
					command_data_void,gfx_create_annotation);
				Option_table_add_entry(option_table,"cmiss_connection",NULL,
					command_data_void,gfx_create_cmiss);
				Option_table_add_entry(option_table,"colour_bar",NULL,
					command_data_void,gfx_create_colour_bar);
				Option_table_add_entry(option_table,"curve_editor",NULL,
					command_data_void,gfx_create_control_curve_editor);
				Option_table_add_entry(option_table,"cylinders",NULL,
					command_data_void,gfx_create_cylinders);
				Option_table_add_entry(option_table,"data_viewer",NULL,
					command_data_void,gfx_create_data_viewer);
				Option_table_add_entry(option_table,"data_points",/*use_data*/(void *)1,
					command_data_void,gfx_create_node_points);
				Option_table_add_entry(option_table,"data_sync",NULL,
					command_data_void,gfx_create_data_sync);
				Option_table_add_entry(option_table,"dgroup",NULL,
					command_data_void,gfx_create_data_group);
				Option_table_add_entry(option_table,"egroup",NULL,
					command_data_void,gfx_create_element_group);
				Option_table_add_entry(option_table,"element_creator",NULL,
					command_data_void,gfx_create_element_creator);
				Option_table_add_entry(option_table,"element_point_viewer",NULL,
					command_data_void,gfx_create_element_point_viewer);
				Option_table_add_entry(option_table,"element_points",NULL,
					command_data_void,gfx_create_element_points);
#if defined (MIRAGE)
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)&create_emoter_slider_data,gfx_create_emoter);
				Option_table_add_entry(option_table,"em_sliders",NULL,
					(void *)&create_node_group_slider_data,create_em_sliders);
#endif /* defined (MIRAGE) */
				Option_table_add_entry(option_table,"environment_map",NULL,
					command_data_void,gfx_create_environment_map);
				Option_table_add_entry(option_table,"flow_particles",NULL,
					command_data_void,gfx_create_flow_particles);
				Option_table_add_entry(option_table,"g_element_editor",NULL,
					command_data_void,gfx_create_g_element_editor);
				Option_table_add_entry(option_table,"graphical_material_editor",NULL,
					command_data_void,gfx_create_graphical_material_editor);
				Option_table_add_entry(option_table,"grid_field_calculator",NULL,
					command_data_void,gfx_create_grid_field_calculator);
#if defined (HAPTIC)
				Option_table_add_entry(option_table,"haptic",NULL,
					command_data_void,gfx_create_haptic);
#endif /* defined (HAPTIC) */
				Option_table_add_entry(option_table,"im_control",NULL,
					command_data_void,gfx_create_input_module_control);
				Option_table_add_entry(option_table,"interactive_data_editor",NULL,
					command_data_void,gfx_create_interactive_data_editor);
				Option_table_add_entry(option_table,"interactive_node_editor",NULL,
					command_data_void,gfx_create_interactive_node_editor);
				Option_table_add_entry(option_table,"iso_surfaces",NULL,
					command_data_void,gfx_create_iso_surfaces);
				Option_table_add_entry(option_table,"light",NULL,
					command_data_void,gfx_create_light);
				Option_table_add_entry(option_table,"lmodel",NULL,
					command_data_void,gfx_create_light_model);
				Option_table_add_entry(option_table,"lines",NULL,
					command_data_void,gfx_create_lines);
				Option_table_add_entry(option_table,"material",NULL,
					command_data_void,gfx_create_material);
				Option_table_add_entry(option_table,"more_flow_particles",NULL,
					command_data_void,gfx_create_more_flow_particles);
				Option_table_add_entry(option_table,"morph",NULL,
					command_data_void,gfx_create_morph);
				Option_table_add_entry(option_table,"muscle_slider",NULL,
					(void *)&create_node_group_slider_data,create_muscle_slider);
				Option_table_add_entry(option_table,"ngroup",NULL,
					command_data_void,gfx_create_node_group);
				Option_table_add_entry(option_table,"node_points",/*use_data*/(void *)0,
					command_data_void,gfx_create_node_points);
				Option_table_add_entry(option_table,"node_viewer",NULL,
					command_data_void,gfx_create_node_viewer);
				Option_table_add_entry(option_table,"object",NULL,
					command_data_void,gfx_create_userdef);
				Option_table_add_entry(option_table,"pivot_slider",NULL,
					(void *)&create_node_group_slider_data,create_pivot_slider);
				Option_table_add_entry(option_table,"scene",NULL,
					command_data_void,gfx_create_scene);
				Option_table_add_entry(option_table, "snake", NULL,
					command_data_void, gfx_create_snake);
				Option_table_add_entry(option_table,"spectrum",NULL,
					command_data_void,gfx_create_spectrum);
				Option_table_add_entry(option_table,"streamlines",NULL,
					command_data_void,gfx_create_streamlines);
#if defined (OLD_CODE)
				Option_table_add_entry(option_table,"strline_interactive",NULL,
					command_data_void,gfx_create_interactive_streamline);
#endif /* defined (OLD_CODE) */
				Option_table_add_entry(option_table,"surfaces",NULL,
					command_data_void,gfx_create_surfaces);
				Option_table_add_entry(option_table,"texmap",NULL,
					command_data_void,gfx_create_texture_map);
				Option_table_add_entry(option_table,"texture",NULL,
					command_data_void,gfx_create_texture);
				Option_table_add_entry(option_table,"time_editor",NULL,
					command_data_void,gfx_create_time_editor);
				Option_table_add_entry(option_table,"tracking_editor",NULL,
					command_data_void,gfx_create_tracking_editor);
				Option_table_add_entry(option_table,"volumes",NULL,
					command_data_void,gfx_create_volumes);
				Option_table_add_entry(option_table,"vt_editor",NULL,
					command_data_void,gfx_create_volume_editor);
				Option_table_add_entry(option_table,"vtexture",NULL,
					command_data_void,gfx_create_volume_texture);
				Option_table_add_entry(option_table,"window",NULL,
					command_data_void,gfx_create_window);
				Option_table_add_entry(option_table,"3d_digitizer",NULL,
					command_data_void,gfx_create_3d_digitizer);
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx create",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_create.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_create */

static int gfx_define_faces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Executes a GFX DEFINE FACES command.
==============================================================================*/
{
	int return_code;
	struct Add_FE_element_and_faces_to_manager_data *add_element_data;
	struct Cmiss_command_data *command_data;
	struct FE_element_list_CM_element_type_data element_list_type_data;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_element) *element_list;
	static struct Modifier_entry option_table[]=
	{
		{"egroup",NULL,NULL,set_FE_element_group},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_define_faces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		element_group=(struct GROUP(FE_element) *)NULL;
		(option_table[0]).to_be_modified=&element_group;
		(option_table[0]).user_data=command_data->element_group_manager;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (element_list=CREATE(LIST(FE_element))())
			{
				element_list_type_data.cm_element_type = CM_ELEMENT;
				element_list_type_data.element_list = element_list;
				if (element_group)
				{
					return_code = FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						add_FE_element_of_CM_element_type_to_list,
						(void *)&element_list_type_data, element_group);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						add_FE_element_of_CM_element_type_to_list,
						(void *)&element_list_type_data, command_data->element_manager);
				}
				if (return_code)
				{
					/* create user_data for add_FE_element_and_faces_to_manager, which
						 helps efficiently find existing faces to fit new element. Don't
						 forget to destroy it afterwards as it can be huge! */
					if (add_element_data=CREATE(Add_FE_element_and_faces_to_manager_data)(
						command_data->element_manager))
					{
						MANAGER_BEGIN_CACHE(FE_element)(command_data->element_manager);
						if (element_group)
						{
							MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
						}
						return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
							add_FE_element_and_faces_to_manager,
							(void *)add_element_data,element_list);
						/* Destroy add_element_data without fail - it can be huge! */
						DESTROY(Add_FE_element_and_faces_to_manager_data)(
							&add_element_data);
						if (element_group)
						{
							/* make sure new faces are in element_group */
							FOR_EACH_OBJECT_IN_LIST(FE_element)(
								ensure_FE_element_and_faces_are_in_group,
								(void *)element_group,element_list);
							MANAGED_GROUP_END_CACHE(FE_element)(element_group);
						}
						MANAGER_END_CACHE(FE_element)(command_data->element_manager);
					}
				}
				DESTROY(LIST(FE_element))(&element_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_define_faces.  Could not create element list");
				return_code=0;
			}
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_faces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_define_faces */

static int execute_command_gfx_define(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DEFINE command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	auto struct Modifier_entry option_table[]=
	{
		{"curve",NULL,NULL,gfx_define_Control_curve},
		{"faces",NULL,NULL,gfx_define_faces},
		{"field",NULL,NULL,define_Computed_field},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_define);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* curve */
				(option_table[i]).user_data=command_data->control_curve_manager;
				i++;
				/* faces */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* field */
				(option_table[i]).user_data=command_data->computed_field_package;
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx define",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_define.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_define.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_define */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_cmiss(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX DESTROY CMISS_CONNECTION command.
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(gfx_destroy_cmiss);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
#if defined (LINK_CMISS)
		if (CMISS)
		{
			return_code=DESTROY(CMISS_connection)(&CMISS);
		}
		else
		{
			if (!(current_token=state->current_token)||
				(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_cmiss.  No CMISS connection");
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
#else /* defined (LINK_CMISS) */
			display_message(ERROR_MESSAGE,"gfx_destroy_cmiss.  Define LINK_CMISS");
#endif /* defined (LINK_CMISS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_cmiss.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_cmiss */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_data_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX DESTROY DGROUP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*node_group;

	ENTER(gfx_destroy_data_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->node_group_manager);
					element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(
						current_token,command_data->element_group_manager);
					if (data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->data_group_manager))
					{
						/* must remove element_group before node group because the
							 GT_element_groups for it access the node and data groups */
						return_code=((!element_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(element_group,
								command_data->element_group_manager))&&((!node_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
								command_data->node_group_manager))&&
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown data group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing data group name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_data_group.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_data_group.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_data_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_element_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX DESTROY EGROUP command.
???RC.  Essentially the same code as gfx_destroy_node_group since node and
element groups are destroyed together.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*node_group;

	ENTER(gfx_destroy_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->data_group_manager);
					node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->node_group_manager);
					if (element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),
						name)(current_token,command_data->element_group_manager))
					{
						/* must remove element_group before node group because the
							 GT_element_groups for it access the node and data groups */
						return_code=
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(element_group,
								command_data->element_group_manager)&&((!node_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
								command_data->node_group_manager))&&((!data_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager));
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown element group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing element group name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_element_group.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_element_group.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_element_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static struct LIST(FE_element) *
	FE_element_list_from_all_selected_group_ranges(
		enum CM_element_type cm_element_type,
		struct MANAGER(FE_element) *element_manager, int all_flag,
		struct FE_element_selection *element_selection, int selected_flag,
		struct GROUP(FE_element) *element_group,
		struct Multi_range *element_ranges)
/*******************************************************************************
LAST MODIFIED : 15 June 2001

DESCRIPTION :
Creates and returns a element group that is the intersection of:
- all elements in the <element_manager> if <all_flag> is set;
- all elements in the <element_selection> if <selected_flag> is set;
- all elements in the <element_group>, if supplied;
- all elements in the given <element_ranges>, if any.
Up to the calling function to destroy the returned element list.
==============================================================================*/
{
	int ranges_flag, return_code;
	struct CM_element_type_Multi_range_data element_type_ranges_data;
	struct FE_element_list_conditional_data element_list_conditional_data;
	struct LIST(FE_element) *element_list;

	ENTER(FE_element_list_from_all_selected_group_ranges);
	element_list = (struct LIST(FE_element) *)NULL;
	if (element_manager && ((!selected_flag) || element_selection))
	{
		if (element_list = CREATE(LIST(FE_element))())
		{
			return_code = 1;
			ranges_flag = element_ranges &&
				(0 < Multi_range_get_number_of_ranges(element_ranges));
			if (selected_flag)
			{
				/* add the selected elements of given type to element_list, and if
					 element_ranges given, intersect with them */
				element_list_conditional_data.element_list = element_list;
				element_list_conditional_data.function = FE_element_has_CM_element_type;
				element_list_conditional_data.user_data = (void *)cm_element_type;
				if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
					ensure_FE_element_is_in_list_conditional,
					(void *)&element_list_conditional_data,
					FE_element_selection_get_element_list(element_selection)))
				{
					if (return_code && element_group)
					{
						return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
							FE_element_is_not_in_group, (void *)element_group, element_list);
					}
					if (ranges_flag)
					{
						element_type_ranges_data.cm_element_type = cm_element_type;
						element_type_ranges_data.multi_range = element_ranges;
						return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
							FE_element_of_CM_element_type_is_not_in_Multi_range,
							(void *)&element_type_ranges_data, element_list);
					}
				}
			}
			else if (ranges_flag)
			{
				/* add elements of given type in element_ranges to element_list */
				element_type_ranges_data.cm_element_type = cm_element_type;
				element_type_ranges_data.multi_range = element_ranges;
				element_list_conditional_data.element_list = element_list;
				element_list_conditional_data.function =
					FE_element_of_CM_element_type_is_in_Multi_range;
				element_list_conditional_data.user_data =
					(void *)&element_type_ranges_data;
				if (element_group)
				{
					return_code = FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						ensure_FE_element_is_in_list_conditional,
						(void *)&element_list_conditional_data, element_group);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						ensure_FE_element_is_in_list_conditional,
						(void *)&element_list_conditional_data, element_manager);
				}
			}
			else if (element_group)
			{
				element_list_conditional_data.element_list = element_list;
				element_list_conditional_data.function =
					FE_element_has_CM_element_type;
				element_list_conditional_data.user_data = (void *)cm_element_type;
				/* add all elements of given type in element_group to element_list */
				return_code = FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					ensure_FE_element_is_in_list_conditional,
					(void *)&element_list_conditional_data, element_group);
			}
			else if (all_flag)
			{
				/* add all elements to element_list */
				element_list_conditional_data.element_list = element_list;
				element_list_conditional_data.function =
					FE_element_has_CM_element_type;
				element_list_conditional_data.user_data = (void *)cm_element_type;
				/* add all elements of given type to element_list */
				return_code = FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
					ensure_FE_element_is_in_list_conditional,
					(void *)&element_list_conditional_data, element_manager);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_element_list_from_all_selected_group_ranges.  "
					"Could not fill list");
				DESTROY(LIST(FE_element))(&element_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_list_from_all_selected_group_ranges.  "
				"Could not create list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_list_from_all_selected_group_ranges.  Invalid argument(s)");
	}
	LEAVE;

	return (element_list);
} /* FE_element_list_from_all_selected_group_ranges */

static struct LIST(FE_node) *
	FE_node_list_from_all_selected_group_ranges(
		struct MANAGER(FE_node) *node_manager, int all_flag,
		struct FE_node_selection *node_selection, int selected_flag,
		struct GROUP(FE_node) *node_group,
		struct Multi_range *node_ranges)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Creates and returns a node group that is the intersection of:
- all nodes in the <node_manager> if <all_flag> is set;
- all nodes in the <node_selection> if <selected_flag> is set;
- all nodes in the <node_group>, if supplied;
- all nodes in the given <node_ranges>, if any.
Up to the calling function to destroy the returned node list.
==============================================================================*/
{
	int ranges_flag, return_code;
	struct FE_node_list_conditional_data list_conditional_data;
	struct LIST(FE_node) *node_list;

	ENTER(FE_node_list_from_all_selected_group_ranges);
	node_list = (struct LIST(FE_node) *)NULL;
	if (node_manager && ((!selected_flag) || node_selection))
	{
		if (node_list = CREATE(LIST(FE_node))())
		{
			return_code = 1;
			ranges_flag = node_ranges &&
				(0 < Multi_range_get_number_of_ranges(node_ranges));
			if (selected_flag)
			{
				/* add the selected nodes to node_list, and if node_ranges
					 given, intersect with them */
				if (return_code = COPY_LIST(FE_node)(node_list,
					FE_node_selection_get_node_list(node_selection)))
				{
					if (return_code && node_group)
					{
						return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
							FE_node_is_not_in_group, (void *)node_group, node_list);
					}
					if (return_code && ranges_flag)
					{
						return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
							FE_node_is_not_in_Multi_range, (void *)node_ranges, node_list);
					}
				}
			}
			else if (ranges_flag)
			{
				/* add nodes with numbers in node_ranges to node_list */
				list_conditional_data.node_list = node_list;
				list_conditional_data.function = FE_node_is_in_Multi_range;
				list_conditional_data.user_data = node_ranges;
				if (node_group)
				{
					return_code = FOR_EACH_OBJECT_IN_GROUP(FE_node)(
						ensure_FE_node_is_in_list_conditional,
						(void *)&list_conditional_data, node_group);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
						ensure_FE_node_is_in_list_conditional,
						(void *)&list_conditional_data, node_manager);
				}
			}
			else if (node_group)
			{
				return_code = FOR_EACH_OBJECT_IN_GROUP(FE_node)(
					ensure_FE_node_is_in_list, (void *)node_list, node_group);
			}
			else if (all_flag)
			{
				/* add all nodes to node_list */
				return_code = FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
					ensure_FE_node_is_in_list, (void *)node_list, node_manager);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_node_list_from_all_selected_group_ranges.  Could not fill list");
				DESTROY(LIST(FE_node))(&node_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_list_from_all_selected_group_ranges.  Could not create list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_list_from_all_selected_group_ranges.  Invalid argument(s)");
	}
	LEAVE;

	return (node_list);
} /* FE_node_list_from_all_selected_group_ranges */

static int gfx_destroy_elements(struct Parse_state *state,
	void *cm_element_type_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 2001

DESCRIPTION :
Executes a GFX DESTROY ELEMENTS command.
==============================================================================*/
{
	char all_flag, selected_flag;
	enum CM_element_type cm_element_type;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_element) *destroy_element_list;
	struct Multi_range *element_ranges;
	struct Option_table *option_table;

	ENTER(gfx_destroy_elements);
	cm_element_type = (enum CM_element_type)cm_element_type_void;
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		all_flag = 0;
		selected_flag = 0;
		element_group = (struct GROUP(FE_element) *)NULL;
		element_ranges = CREATE(Multi_range)();

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &element_group,
			command_data->element_group_manager, set_FE_element_group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag, NULL,
			set_char_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (destroy_element_list = FE_element_list_from_all_selected_group_ranges(
				cm_element_type, command_data->element_manager, all_flag,
				command_data->element_selection, selected_flag,
				element_group, element_ranges))
			{
				if (0 < NUMBER_IN_LIST(FE_element)(destroy_element_list))
				{
					return_code = destroy_listed_elements(destroy_element_list,
						command_data->element_manager,
						command_data->element_group_manager,
						command_data->element_selection,
						command_data->element_point_ranges_selection);
				}
				else
				{
					switch (cm_element_type)
					{
						case CM_ELEMENT:
						{
							display_message(INFORMATION_MESSAGE,
								"gfx destroy elements:  No elements specified\n");
						} break;
						case CM_FACE:
						{
							display_message(INFORMATION_MESSAGE,
								"gfx destroy faces:  No faces specified\n");
						} break;
						case CM_LINE:
						{
							display_message(INFORMATION_MESSAGE,
								"gfx destroy lines:  No lines specified\n");
						} break;
					}
					return_code = 0;
				}
				DESTROY(LIST(FE_element))(&destroy_element_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_elements.  Could not make destroy_element_list");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		DESTROY(Multi_range)(&element_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_elements */

static int gfx_destroy_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Executes a GFX DESTROY FIELD command.
==============================================================================*/
{
	char *current_token;
	struct Computed_field *computed_field;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *fe_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(gfx_destroy_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void)&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package)))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (computed_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					current_token,computed_field_manager))
				{
					if (Computed_field_can_be_destroyed(computed_field))
					{
						/* also want to destroy wrapped FE_field */
						fe_field=(struct FE_field *)NULL;
						if (Computed_field_is_type_finite_element(computed_field))
						{
							Computed_field_get_type_finite_element(computed_field,
								&fe_field);
						}
						if (return_code=REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
							computed_field,computed_field_manager))
						{
							if (fe_field)
							{
								return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
									fe_field,command_data->fe_field_manager);
							}
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"gfx_destroy_Computed_field.  Could not destroy field");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cannot destroy field in use : %s",current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Field does not exist: %s",
						current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing field name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Computed_field */

static int Scene_remove_graphics_object_iterator(struct Scene *scene,
	void *graphics_object_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Removes all instances of the <graphics_object> from <scene>.
???RC Move to scene.c?
==============================================================================*/
{
	int return_code;
	struct GT_object *graphics_object;

	ENTER(Scene_remove_graphics_object_iterator);
	if (scene&&(graphics_object=(struct GT_object *)graphics_object_void))
	{
		while (Scene_has_graphics_object(scene,graphics_object))
		{
			Scene_remove_graphics_object(scene,graphics_object);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_graphics_object_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_graphics_object_iterator */

static int gfx_destroy_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Executes a GFX DESTROY GRAPHICS_OBJECT command.
==============================================================================*/
{
	char *current_token;
	gtObject *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_destroy_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						current_token,command_data->graphics_object_list))
					{
						/* remove all instances of the graphics object from all scenes */
						FOR_EACH_OBJECT_IN_MANAGER(Scene)(
							Scene_remove_graphics_object_iterator,(void *)graphics_object,
							command_data->scene_manager);
						/* remove graphics object from the global list. Object is destroyed
							 when deaccessed by list */
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics object does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing graphics object name");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_graphics_object.  Missing graphics_object_list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_graphics_object */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_destroy_node_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Executes a GFX DESTROY NGROUP command.
???RC.  Essentially the same code as gfx_destroy_element_group since node and
element groups are destroyed together.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group,*node_group;

	ENTER(gfx_destroy_node_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->data_group_manager);
					element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(
						current_token,command_data->element_group_manager);
					if (node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						current_token,command_data->node_group_manager))
					{
						/* must remove element_group before node group because the
							 GT_element_groups for it access the node and data groups */
						return_code=((!element_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(element_group,
								command_data->element_group_manager))&&
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(node_group,
								command_data->node_group_manager)&&((!data_group)||
							REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
								command_data->data_group_manager));
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown node group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing node group name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_node_group.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_node_group.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_node_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_destroy_nodes(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a GFX DESTROY NODES/DATA command.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag, selected_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_node_selection *node_selection;
	struct GROUP(FE_node) *node_group;
	struct LIST(FE_node) *destroy_node_list;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;

	ENTER(gfx_destroy_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (use_data)
		{
			element_manager = (struct MANAGER(FE_element) *)NULL;
			node_manager = command_data->data_manager;
			node_group_manager = command_data->data_group_manager;
			node_selection = command_data->data_selection;
		}
		else
		{
			element_manager = command_data->element_manager;
			node_manager = command_data->node_manager;
			node_group_manager = command_data->node_group_manager;
			node_selection = command_data->node_selection;
		}
		/* initialise defaults */
		all_flag = 0;
		selected_flag = 0;
		node_group = (struct GROUP(FE_node) *)NULL;
		node_ranges = CREATE(Multi_range)();

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &node_group,
			node_group_manager, set_FE_node_group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (destroy_node_list = FE_node_list_from_all_selected_group_ranges(
				node_manager, all_flag, node_selection, selected_flag,
				node_group, node_ranges))
			{
				if (0 < NUMBER_IN_LIST(FE_node)(destroy_node_list))
				{
					return_code = destroy_listed_nodes(destroy_node_list,
						node_manager, node_group_manager, element_manager, node_selection);
				}
				else
				{
					if (use_data)
					{
						display_message(WARNING_MESSAGE,
							"gfx destroy data:  No data specified");
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"gfx destroy nodes:  No nodes specified");
					}
				}
				DESTROY(LIST(FE_node))(&destroy_node_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_nodes.  Could not make destroy_node_list");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
		DESTROY(Multi_range)(&node_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_nodes */

static int gfx_destroy_vtextures(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
Executes a GFX DESTROY VTEXTURES command.
???DB.  Could merge with destroy_graphics_objects if graphics_objects used the
	new list structures.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_destroy_vtextures);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (volume_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
						name)(current_token,volume_texture_manager))
					{
						/* remove object from list (destroys automatically) */
						return_code=REMOVE_OBJECT_FROM_MANAGER(VT_volume_texture)(
							volume_texture,volume_texture_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				/* destroy all objects in list */
				return_code=REMOVE_ALL_OBJECTS_FROM_MANAGER(VT_volume_texture)(
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_vtextures.  Missing volume texture manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_vtextures.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_vtextures */

static int gfx_destroy_Graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 18 September 2001

DESCRIPTION :
Executes a GFX DESTROY WINDOW command.
==============================================================================*/
{
	char *current_token;
	struct Graphics_window *graphics_window;
	int return_code;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_destroy_Graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (graphics_window_manager =
		(struct MANAGER(Graphics_window) *)graphics_window_manager_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (graphics_window =
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window, name)(
						current_token, graphics_window_manager))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Graphics_window)(graphics_window,
						graphics_window_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove graphics window %s from manager",
							current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown graphics window: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " GRAPHICS_WINDOW_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing graphics window name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Graphics_window.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Graphics_window */

static int execute_command_gfx_destroy(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 September 2001

DESCRIPTION :
Executes a GFX DESTROY command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_destroy);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* cmiss_connection */
				Option_table_add_entry(option_table, "cmiss_connection", NULL,
					command_data_void, gfx_destroy_cmiss);
				/* curve */
				Option_table_add_entry(option_table, "curve", NULL,
					command_data->control_curve_manager, gfx_destroy_Control_curve);
				/* data */
				Option_table_add_entry(option_table, "data", (void *)1,
					command_data_void, gfx_destroy_nodes);
				/* dgroup */
				Option_table_add_entry(option_table, "dgroup", NULL,
					command_data_void, gfx_destroy_data_group);
				/* egroup */
				Option_table_add_entry(option_table, "egroup", NULL,
					command_data_void, gfx_destroy_element_group);
				/* elements */
				Option_table_add_entry(option_table, "elements", (void *)CM_ELEMENT,
					command_data_void, gfx_destroy_elements);
				/* faces */
				Option_table_add_entry(option_table, "faces", (void *)CM_FACE,
					command_data_void, gfx_destroy_elements);
				/* field */
				Option_table_add_entry(option_table, "field", NULL,
					command_data_void, gfx_destroy_Computed_field);
				/* graphics_object */
				Option_table_add_entry(option_table, "graphics_object", NULL,
					command_data_void, gfx_destroy_graphics_object);
				/* lines */
				Option_table_add_entry(option_table, "lines", (void *)CM_LINE,
					command_data_void, gfx_destroy_elements);
				/* ngroup */
				Option_table_add_entry(option_table, "ngroup", NULL,
					command_data_void, gfx_destroy_node_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", (void *)0,
					command_data_void, gfx_destroy_nodes);
				/* spectrum */
				Option_table_add_entry(option_table, "spectrum", NULL,
					command_data->spectrum_manager, gfx_destroy_spectrum);
				/* vtextures */
				Option_table_add_entry(option_table, "vtextures", NULL,
					command_data->volume_texture_manager, gfx_destroy_vtextures);
				/* window */
				Option_table_add_entry(option_table, "window", NULL,
					command_data->graphics_window_manager, gfx_destroy_Graphics_window);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx destroy",command_data->command_window);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_destroy.  Invalid argument(s)");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_destroy.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_destroy */

struct Scene_add_graphics_object_iterator_data
{
	int position;
	struct Scene *scene;
};

static int Scene_add_graphics_object_iterator(struct GT_object *graphics_object,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Scene *scene;
	struct Scene_add_graphics_object_iterator_data *data;

	ENTER(Scene_add_graphics_object_iterator);
	return_code = 0;
	if (graphics_object &&
		(data = (struct Scene_add_graphics_object_iterator_data *)data_void )&&
		(scene = data->scene))
	{
		if (Scene_has_graphics_object(scene, graphics_object))
		{
			return_code = 1;
		}
		else
		{
			return_code = Scene_add_graphics_object(scene,graphics_object,
				data->position, graphics_object->name,/*fast_changing*/0);
			if (0 < data->position)
			{
				data->position++;
			}
		}
		if (1 < GT_object_get_number_of_times(graphics_object))
		{
			Scene_update_time_behaviour(data->scene, graphics_object);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_graphics_object_iterator.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Scene_add_graphics_object_iterator */

static int execute_command_gfx_draw(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Executes a GFX DRAW command.
==============================================================================*/
{
	char *scene_object_name, *time_object_name;
	struct GT_object *graphics_object;
	int return_code, position;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct Scene *child_scene,*scene;
	struct Scene_add_graphics_object_iterator_data data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_draw);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		graphics_object = (struct GT_object *)NULL;
		element_group = (struct GROUP(FE_element) *)NULL;
		scene_object_name = (char *)NULL;
		time_object_name = (char *)NULL;
		position = 0;
		scene = ACCESS(Scene)(command_data->default_scene);
		child_scene = (struct Scene *)NULL;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&scene_object_name,
			(void *)1,set_name);
		/* child_scene */
		Option_table_add_entry(option_table,"child_scene",&child_scene,
			command_data->scene_manager,set_Scene);
		/* graphics_object */
		Option_table_add_entry(option_table,"graphics_object",&graphics_object,
			command_data->graphics_object_list,set_Graphics_object);
		/* group */
		Option_table_add_entry(option_table,"group",&element_group,
			command_data->element_group_manager,set_FE_element_group);
		/* position */
		Option_table_add_entry(option_table,"position",&position,
			(void *)1,set_int);
		/* scene */
		Option_table_add_entry(option_table,"scene",&scene,
			command_data->scene_manager,set_Scene);
		/* time_object */
		Option_table_add_entry(option_table,"time_object",&time_object_name,
			(void *)1,set_name);
		/* default when token omitted (graphics_object) */
		Option_table_add_entry(option_table,(char *)NULL,&graphics_object,
			command_data->graphics_object_list,set_Graphics_object);
		return_code = Option_table_multi_parse(option_table,state);
		if ((child_scene && graphics_object) ||
			(graphics_object && element_group) ||
			(element_group && child_scene))
		{
			display_message(ERROR_MESSAGE, "execute_command_gfx_draw.  "
				"Specify only one of child_scene|graphics_object|group");
			return_code = 0;
		}
		if (child_scene && time_object_name)
		{
			display_message(ERROR_MESSAGE, "execute_command_gfx_draw.  "
				"Time objects may not be associated with a child_scene");
			return_code = 0;
		}
		/* no errors, not asking for help */
		if (return_code)
		{
			if (graphics_object)
			{
				if (scene)
				{
					return_code = Scene_add_graphics_object(scene, graphics_object,
						position, scene_object_name, /*fast_changing*/0);
					if (time_object_name)
					{
						/* SAB A new time_object is created and associated with the named
							 scene_object, the time_keeper is supplied so that the
							 default could be overridden */
						Scene_set_time_behaviour(scene,scene_object_name, time_object_name,
							command_data->default_time_keeper);
					}
					if (1<GT_object_get_number_of_times(graphics_object))
					{
						/* any scene_objects referring to this graphics_object which do
							 not already have a time_object all are associated with a
							 single common time_object */
						Scene_update_time_behaviour(scene,graphics_object);
					}
				}
			}
			else if (child_scene)
			{
				return_code = Scene_add_child_scene(scene, child_scene, position,
					scene_object_name, command_data->scene_manager);
			}
			else if (element_group)
			{
				return_code = Scene_add_graphical_element_group(scene, element_group,
					position, scene_object_name);
			}
			else
			{
				data.scene = scene;
				data.position = position;
				return_code = FOR_EACH_OBJECT_IN_LIST(GT_object)(
					Scene_add_graphics_object_iterator, (void *)&data,
					command_data->graphics_object_list);
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
		if (child_scene)
		{
			DEACCESS(Scene)(&child_scene);
		}
		if (graphics_object)
		{
			DEACCESS(GT_object)(&graphics_object);
		}
		if (scene_object_name)
		{
			DEALLOCATE(scene_object_name);
		}
		if (time_object_name)
		{
			DEALLOCATE(time_object_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_draw.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_draw */

struct Edit_graphics_object_data
{
	char *graphics_object_name;
	struct Scene *scene;
	struct Scene_object *scene_object;
}; /* struct Edit_graphics_object_data */

#if !defined (WINDOWS_DEV_FLAG)
static void edit_graphics_object(Widget widget,void *user_data,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Callback from transformation editor dialog when transformation changes.
==============================================================================*/
{
	gtMatrix transformation_matrix;
	struct Cmgui_coordinate *new_transformation;
	struct Edit_graphics_object_data *edit_graphics_object_data;
	struct Scene_object *scene_object;

	ENTER(edit_graphics_object);
	USE_PARAMETER(widget);
	if ((edit_graphics_object_data=
		(struct Edit_graphics_object_data *)user_data)&&
		(scene_object=edit_graphics_object_data->scene_object)&&
		(new_transformation=(struct Cmgui_coordinate *)call_data))
	{
#if defined (DEBUG)
		/*???debug */
		printf("%s\n",new_transformation->name);
		printf("position : %g %g %g\n",
			((new_transformation->origin).position.data)[0],
			((new_transformation->origin).position.data)[1],
			((new_transformation->origin).position.data)[2]);
		printf("direction : %g %g %g\n",
			((new_transformation->origin).direction.data)[0],
			((new_transformation->origin).direction.data)[1],
			((new_transformation->origin).direction.data)[2]);
#endif /* defined (DEBUG) */
#if defined (GL_API)
		loadmatrix(identity_transform_matrix);
		translate(((new_transformation->origin).position.data)[0],
			((new_transformation->origin).position.data)[1],
			((new_transformation->origin).position.data)[2]);
#if defined (OLD_CODE)
/*???GMH.  Testing out transformations */
		rot(-((new_transformation->origin).direction.data)[2],'z');
		rot(((new_transformation->origin).direction.data)[1],'x');
		rot(-((new_transformation->origin).direction.data)[0],'y');
#endif /* defined (OLD_CODE) */
		rot(((new_transformation->origin).direction.data)[0],'z');
		rot(((new_transformation->origin).direction.data)[1],'y');
		rot(((new_transformation->origin).direction.data)[2],'x');
		getmatrix(transformation_matrix);
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(((new_transformation->origin).position.data)[0],
			((new_transformation->origin).position.data)[1],
			((new_transformation->origin).position.data)[2]);
#if defined (OLD_CODE)
/*???GMH.  Testing out transformations */
		glRotatef(-((new_transformation->origin).direction.data)[2],0.,0.,1.);
		glRotatef(((new_transformation->origin).direction.data)[1],1.,0.,0.);
		glRotatef(-((new_transformation->origin).direction.data)[0],0.,1.,0.);
#endif /* defined (OLD_CODE) */
		glRotatef(((new_transformation->origin).direction.data)[0],0.,0.,1.);
		glRotatef(((new_transformation->origin).direction.data)[1],0.,1.,0.);
		glRotatef(((new_transformation->origin).direction.data)[2],1.,0.,0.);
		wrapperReadMatrix(GL_MODELVIEW_MATRIX,&(transformation_matrix));
#endif /* defined (OPENGL_API) */
		Scene_object_set_transformation(scene_object, &transformation_matrix);
	}
	else
	{
		display_message(ERROR_MESSAGE,"edit_graphics_object.  Invalid argument(s)");
#if defined (DEBUG)
		/*???debug */
		printf("user_data=%p call_data=%p\n",user_data,call_data);
		edit_graphics_object_data=
			(struct Edit_graphics_object_data *)user_data;
		printf("gra=%p\n",edit_graphics_object_data->scene_object);
#endif /* defined (DEBUG) */
	}
	LEAVE;
} /* edit_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static void end_edit_graphics_object(Widget widget,void *user_data,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Callback for when transformation editor for gfx edit graphics_object is closed.
Cleans up <user_data> == edit_graphics_object_data.
==============================================================================*/
{
	struct Edit_graphics_object_data *edit_graphics_object_data;

	ENTER(end_edit_graphics_object);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((edit_graphics_object_data=
		(struct Edit_graphics_object_data *)user_data))
	{
		if (edit_graphics_object_data->graphics_object_name)
		{
			DEALLOCATE(edit_graphics_object_data->graphics_object_name);
		}
		/* deaccess graphics object and scene - see gfx_edit_graphics_object */
		DEACCESS(Scene_object)(&edit_graphics_object_data->scene_object);
		DEACCESS(Scene)(&edit_graphics_object_data->scene);
		DEALLOCATE(edit_graphics_object_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"end_edit_graphics_object.  Invalid argument(s)");
	}
	LEAVE;
} /* end_edit_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

struct Apply_transformation_data
{
	gtMatrix *transformation;
	struct MANAGER(FE_node) *node_manager;
}; /* struct Apply_transformation_data */

static int apply_transformation_to_node(struct FE_node *node,
	void *apply_transformation_data)
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
Iterator that modifies the position of each node according to the 
transformation in the transformation data.
==============================================================================*/
{
	FE_value x, x2, y, y2, z, z2, h2;
	gtMatrix *transformation;
	int return_code;
	struct Apply_transformation_data *data;
	struct FE_node *copy_node;
	struct FE_field *coordinate_field;

	ENTER(apply_transformation_to_node);
	if(node&&(data=(struct Apply_transformation_data *)apply_transformation_data)
		&&(transformation=data->transformation))
	{
		if (coordinate_field=FE_node_get_position_cartesian(node,
			(struct FE_field *)NULL,&x,&y,&z,(FE_value *)NULL))
		{
			/* Get the new position */
			h2 = (*transformation)[0][3] * x
				+ (*transformation)[1][3] * y
				+ (*transformation)[2][3] * z
				+ (*transformation)[3][3];
			x2 = ((*transformation)[0][0] * x
				+ (*transformation)[1][0] * y
				+ (*transformation)[2][0] * z
				+ (*transformation)[3][0]) / h2;
			y2 = ((*transformation)[0][1] * x
				+ (*transformation)[1][1] * y
				+ (*transformation)[2][1] * z
				+ (*transformation)[3][1]) / h2;
			z2 = ((*transformation)[0][2] * x
				+ (*transformation)[1][2] * y
				+ (*transformation)[2][2] * z
				+ (*transformation)[3][2]) / h2;

			/* create a copy of the node: */
			if ((copy_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				COPY(FE_node)(copy_node,node))
			{
				if (FE_node_set_position_cartesian(copy_node,coordinate_field,x2,y2,z2))
				{
					return_code = MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
						node,copy_node,data->node_manager);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_node_translate.  Could not make move node");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_translate.  Could not make copy of node");
				return_code = 0;
			}
			DESTROY(FE_node)(&copy_node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_translate.  Could not calculate coordinate field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"end_edit_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_edit_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EDIT GRAPHICS_OBJECT command.
==============================================================================*/
{
	char apply_flag,*graphics_object_name;
	int return_code;
	struct Callback_data callback,*callback_data;
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate initial_transformation;
	struct Edit_graphics_object_data *edit_graphics_object_data;
	struct Scene_object *scene_object;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"apply_transformation",NULL,NULL,set_char_flag},
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_edit_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			apply_flag=0;
			graphics_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified = &apply_flag;
			(option_table[1]).to_be_modified = &graphics_object_name;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &graphics_object_name;
			return_code=process_multiple_options(state,option_table);
			if (return_code)
			{
				if (scene&&graphics_object_name&&(scene_object=
					Scene_get_Scene_object_by_name(
					scene,graphics_object_name)))
				{
					if (apply_flag)
					{							
						/* SAB Temporary place for this command cause I really
							need to use it, not very general, doesn't work in prolate or
							rotate derivatives */
						struct Apply_transformation_data data;
						struct GT_element_group *gt_element_group;
						struct GROUP(FE_node) *data_group, *node_group;
						gtMatrix identity={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
							transformation;

						if (Scene_object_has_transformation(scene_object)&&
							Scene_object_get_transformation(scene_object,
								&transformation))
						{
							if(Scene_object_has_graphical_element_group(scene_object,NULL) &&
								(gt_element_group = 
									Scene_object_get_graphical_element_group(scene_object)))
							{
								data.transformation = &transformation;
								if (node_group=GT_element_group_get_node_group(
									gt_element_group))
								{
									MANAGER_BEGIN_CACHE(FE_node)(command_data->node_manager);
									data.node_manager = command_data->node_manager;
									FOR_EACH_OBJECT_IN_GROUP(FE_node)
										(apply_transformation_to_node, 
										(void *)&data, node_group);
									MANAGER_END_CACHE(FE_node)(command_data->node_manager);
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Could not get GT_element_group");
									return_code=0;
								}
								if (data_group=GT_element_group_get_data_group(
									gt_element_group))
								{
									MANAGER_BEGIN_CACHE(FE_node)(command_data->data_manager);
									data.node_manager = command_data->data_manager;
									FOR_EACH_OBJECT_IN_GROUP(FE_node)
										(apply_transformation_to_node, 
										(void *)&data, data_group);
									MANAGER_END_CACHE(FE_node)(command_data->data_manager);
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Could not get GT_element_group");
									return_code=0;
								}
								Scene_object_set_transformation(scene_object,
									&identity);
							}
						}
						else
						{
							return_code = 1;
						}
					}
					else
					{
						initial_transformation.name=graphics_object_name;
						initial_transformation.access_count=1;
						/*???DB.  Temp.  Need to be able to convert between gtMatrix and
						  position/direction */
						(initial_transformation.origin.position.data)[0]=0;
						(initial_transformation.origin.position.data)[1]=0;
						(initial_transformation.origin.position.data)[2]=0;
						(initial_transformation.origin.direction.data)[0]=0;
						(initial_transformation.origin.direction.data)[1]=0;
						(initial_transformation.origin.direction.data)[2]=0;
						bring_up_transformation_editor_dialog(
							&(command_data->transformation_editor_dialog),
							command_data->user_interface->application_shell,
							&initial_transformation,command_data->user_interface);
						edit_graphics_object_data=(struct Edit_graphics_object_data *)NULL;
						if ((callback_data=(struct Callback_data *)
							transformation_editor_dialog_get_data(
								command_data->transformation_editor_dialog,
								TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB))&&
							(edit_graphics_object_data=
								(struct Edit_graphics_object_data *)callback_data->data))
						{
							/* deaccess graphics object and scene for existing editor */
							DEACCESS(Scene_object)(&edit_graphics_object_data->scene_object);
							DEACCESS(Scene)(&edit_graphics_object_data->scene);
						}
						/* create data to be passed to transformation editor callbacks */
						if (edit_graphics_object_data||ALLOCATE(edit_graphics_object_data,
							struct Edit_graphics_object_data,1))
						{
							/* access graphics object and scene in case either destroyed while
								editing tranformation */
							edit_graphics_object_data->scene_object=
								ACCESS(Scene_object)(scene_object);
							edit_graphics_object_data->scene=ACCESS(Scene)(scene);
							edit_graphics_object_data->graphics_object_name =
								graphics_object_name;
							/* set callback for when transformation changes */
							callback.data=(void *)edit_graphics_object_data;
							callback.procedure=edit_graphics_object;
							transformation_editor_dialog_set_data(
								command_data->transformation_editor_dialog,
								TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB,&callback);
							/* set callback for when transformation editor is closed */
							callback.procedure=end_edit_graphics_object;
							transformation_editor_dialog_set_data(
								command_data->transformation_editor_dialog,
								TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB,&callback);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No callbacks from transformation editor");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Must specify name of graphics object in scene");
					return_code=0;
				}
			}
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_edit_graphics_object.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_edit_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EDIT SPECTRUM command.
Invokes the graphical spectrum group editor.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_Spectrum},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_edit_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			spectrum=(struct Spectrum *)NULL;
			(option_table[0]).to_be_modified= &spectrum;
			(option_table[0]).user_data=command_data->spectrum_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				return_code=bring_up_spectrum_editor_dialog(
					&(command_data->spectrum_editor_dialog),
					command_data->user_interface->application_shell,
					command_data->spectrum_manager,spectrum,command_data->user_interface,
					command_data->glyph_list,
					command_data->graphical_material_manager,command_data->light_manager,
					command_data->texture_manager,command_data->scene_manager);
			} /* parse error, help */
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_edit_spectrum.  "
				"Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_spectrum.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_spectrum */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_edit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 August 1998

DESCRIPTION :
Executes a GFX EDIT command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"graphics_object",NULL,NULL,gfx_edit_graphics_object},
		{"spectrum",NULL,NULL,gfx_edit_spectrum},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_edit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				(option_table[0]).user_data=command_data_void;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx edit",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_edit.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_edit.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_edit */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Executes a GFX ELEMENT_CREATOR command.
==============================================================================*/
{
	int create_enabled,element_dimension,return_code;
	struct Cmiss_command_data *command_data;
	struct Element_creator *element_creator;
	struct FE_field *coordinate_field;
	struct GROUP(FE_element) *element_group;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_coordinate_field_data;

	ENTER(execute_command_gfx_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (element_creator=command_data->element_creator)
		{
			create_enabled=Element_creator_get_create_enabled(element_creator);
			element_group=Element_creator_get_element_group(element_creator);
			element_dimension=Element_creator_get_element_dimension(element_creator);
			coordinate_field=Element_creator_get_coordinate_field(element_creator);
		}
		else
		{
			create_enabled=0;
			element_group=(struct GROUP(FE_element) *)NULL;
			element_dimension=2;
			coordinate_field=(struct FE_field *)NULL;
		}
		if (coordinate_field)
		{
			ACCESS(FE_field)(coordinate_field);
		}
		if (element_group)
		{
			ACCESS(GROUP(FE_element))(element_group);
		}

		option_table=CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.fe_field_manager=command_data->fe_field_manager;
		set_coordinate_field_data.conditional_function=FE_field_is_coordinate_field;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate_field",
			&coordinate_field,&set_coordinate_field_data,set_FE_field_conditional);
		/* create/no_create */
		Option_table_add_switch(option_table,"create","no_create",&create_enabled);
		/* dimension */
		Option_table_add_entry(option_table,"dimension",
			&element_dimension,NULL,set_int_non_negative);
		/* group */
		Option_table_add_entry(option_table,"group",&element_group,
			command_data->element_group_manager,set_FE_element_group);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (element_creator)
			{
				Element_creator_set_create_enabled(element_creator,create_enabled);
				Element_creator_set_coordinate_field(element_creator,coordinate_field);
				Element_creator_set_element_dimension(element_creator,
					element_dimension);
				if (element_group)
				{
					Element_creator_set_element_group(element_creator,element_group);
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Please specify an element group for the element_creator");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Must create element_creator before modifying it");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		if (coordinate_field)
		{
			DEACCESS(FE_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_creator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_creator */

static int execute_command_gfx_element_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Executes a GFX ELEMENT_TOOL command.
==============================================================================*/
{
	static char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	char *dialog_string;
	int select_elements_enabled,select_faces_enabled,select_lines_enabled,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Element_tool *element_tool;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_element_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (element_tool=command_data->element_tool)
		{
			select_elements_enabled=
				Element_tool_get_select_elements_enabled(element_tool);
			select_faces_enabled=Element_tool_get_select_faces_enabled(element_tool);
			select_lines_enabled=Element_tool_get_select_lines_enabled(element_tool);
		}
		else
		{
			select_elements_enabled=1;
			select_faces_enabled=1;
			select_lines_enabled=1;
		}
		option_table=CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* select_elements/no_select_elements */
		Option_table_add_switch(option_table,"select_elements","no_select_elements",
			&select_elements_enabled);
		/* select_faces/no_select_faces */
		Option_table_add_switch(option_table,"select_faces","no_select_faces",
			&select_faces_enabled);
		/* select_lines/no_select_lines */
		Option_table_add_switch(option_table,"select_lines","no_select_lines",
			&select_lines_enabled);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (element_tool)
			{
				if (dialog_string == dialog_strings[1])
				{
					Element_tool_pop_down_dialog(element_tool);
				}
				Element_tool_set_select_elements_enabled(element_tool,
					select_elements_enabled);
				Element_tool_set_select_faces_enabled(element_tool,
					select_faces_enabled);
				Element_tool_set_select_lines_enabled(element_tool,
					select_lines_enabled);
				if (dialog_string == dialog_strings[0])
				{
					Element_tool_pop_up_dialog(element_tool);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_tool.  Missing element_tool");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_tool */

static int execute_command_gfx_erase(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Executes a GFX ERASE command.
==============================================================================*/
{
	char *scene_name, *scene_object_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;
	struct Scene_object *scene_object;

	ENTER(execute_command_gfx_erase);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		scene_object_name = (char *)NULL;
		scene = ACCESS(Scene)(command_data->default_scene);
		option_table = CREATE(Option_table)();
		/* scene */
		Option_table_add_entry(option_table, "scene", &scene,
			command_data->scene_manager, set_Scene);
		/* default option: scene object name */
		Option_table_add_entry(option_table, (char *)NULL, &scene_object_name,
			NULL, set_name);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (scene && scene_object_name)
			{
				if (scene_object =
					Scene_get_Scene_object_by_name(scene, scene_object_name))
				{
					if (Scene_remove_Scene_object(scene, scene_object))
					{
						return_code = 1;
					}
					else
					{
						GET_NAME(Scene)(scene, &scene_name);
						display_message(ERROR_MESSAGE, "execute_command_gfx_erase.  "
							"Could not erase '%s' from scene '%s'",
							scene_object_name, scene_name);
						DEALLOCATE(scene_name);
						return_code = 0;
					}
				}
				else
				{
					GET_NAME(Scene)(scene, &scene_name);
					display_message(ERROR_MESSAGE,
						"gfx erase:  No object named '%s' in scene '%s'",
						scene_object_name, scene_name);
					DEALLOCATE(scene_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx erase:  Must specify an object and a scene to erase it from");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
		if (scene_object_name)
		{
			DEALLOCATE(scene_object_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_erase.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_erase */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_export_alias(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT ALIAS command.
==============================================================================*/
{
#if defined (NEW_ALIAS)
	char destroy_when_saved,*default_filename="cmgui_wire",*file_name,
		*retrieve_filename,save_now,write_sdl;
	float frame_in,frame_out,view_frame;
#endif /* defined (NEW_ALIAS) */
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
#if defined (NEW_ALIAS)
		{"dont_save_now",NULL,NULL,unset_char_flag},
		{"frame_in",NULL,NULL,set_float},
		{"frame_out",NULL,NULL,set_float},
		{"keep",NULL,NULL,unset_char_flag},
		{"retrieve",NULL,NULL,set_file_name},
#endif /* defined (NEW_ALIAS) */
		{"scene",NULL,NULL,set_Scene},
#if defined (NEW_ALIAS)
		{"sdl",NULL,NULL,set_char_flag},
		{"viewframe",NULL,NULL,set_float},
		{NULL,NULL,NULL,set_file_name}
#else /* defined (NEW_ALIAS) */
		{NULL,NULL,NULL,NULL}
#endif /* defined (NEW_ALIAS) */
	};

	ENTER(gfx_export_alias);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
#if defined (NEW_ALIAS)
			file_name=(char *)NULL;
			frame_in=0;
			frame_out=0;
			retrieve_filename=(char *)NULL;
			save_now=1;
			view_frame=0;
			write_sdl=0;
			destroy_when_saved=1;
#endif /* defined (NEW_ALIAS) */
			scene=command_data->default_scene;
#if defined (NEW_ALIAS)
			(option_table[0]).to_be_modified= &save_now;
			(option_table[1]).to_be_modified= &frame_in;
			(option_table[2]).to_be_modified= &frame_out;
			(option_table[3]).to_be_modified= &destroy_when_saved;
			(option_table[4]).to_be_modified= &retrieve_filename;
			(option_table[5]).to_be_modified= &scene;
			(option_table[5]).user_data=command_data->scene_manager;
			(option_table[6]).to_be_modified= &write_sdl;
			(option_table[7]).to_be_modified= &view_frame;
			(option_table[8]).to_be_modified= &file_name;
#else /* defined (NEW_ALIAS) */
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
#endif /* defined (NEW_ALIAS) */
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene)
				{
#if defined (NEW_ALIAS)
					if (!file_name)
					{
						file_name=default_filename;
					}
					if (write_sdl)
					{
						export_to_alias_sdl(scene,file_name,retrieve_filename,view_frame);
					}
					else
					{
						export_to_alias_frames(scene,file_name,frame_in,frame_out,save_now,
							destroy_when_saved);
					}
#else /* defined (NEW_ALIAS) */
					display_message(ERROR_MESSAGE,"gfx_export_alias.  The old gfx export alias is superseeded by gfx export wavefront");
					return_code=0;
#endif /* defined (NEW_ALIAS) */
				}
			} /* parse error,help */
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_alias */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_export_node(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 November 1998

DESCRIPTION :
Executes a GFX EXPORT NODE command.  This command exports nodes to cmiss as
data.
==============================================================================*/
{
	int base_number,return_code;
	struct Cmiss_command_data *command_data;
	struct LIST(GROUP(FE_node)) *groups;
	static struct Modifier_entry option_table[]=
	{
		{"base_number",NULL,NULL,set_int},
		{"groups",NULL,NULL,set_FE_node_group_list},
		{NULL,NULL,NULL}
	};

	ENTER(gfx_export_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			groups=CREATE(LIST(GROUP(FE_node)))();
			base_number=1;
			/* initialise defaults */
			(option_table[0]).to_be_modified= &base_number;
			(option_table[1]).to_be_modified=groups;
			(option_table[1]).user_data=command_data->node_group_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				export_nodes(groups,base_number,command_data->execute_command);
			} /* parse error, help */
			DESTROY(LIST(GROUP(FE_node)))(&groups);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_node.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_node */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_export_vrml(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT VRML command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"file",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene_including_sub_objects},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Scene *scene;

	ENTER(gfx_export_vrml);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					file_name=confirmation_get_write_filename(".wrl",
						command_data->user_interface);
				}
				if (file_name)
				{
					if (return_code=check_suffix(&file_name,".wrl"))
					{
						return_code=export_to_vrml(file_name,scene);
					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_vrml.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_vrml.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_vrml */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_export_wavefront(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT WAVEFRONT command.
==============================================================================*/
{
	char binary,*file_name,full_comments,*scene_object_name,*temp_filename;
	int frame_number, number_of_frames, return_code, version;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
 		{"binary",NULL,NULL,set_char_flag},
		{"file",NULL,(void *)1,set_name},
		{"frame_number",NULL,NULL,set_int_non_negative},
		{"full_comments",NULL,NULL,set_char_flag},
		{"graphics_object",NULL,(void *)1,set_name},
 		{"number_of_frames",NULL,NULL,set_int_positive},
		{"scene",NULL,NULL,set_Scene_including_sub_objects},
 		{"version",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_export_wavefront);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
 			binary=0;
			file_name=(char *)NULL;
			frame_number = 0;
			full_comments=0;
 			number_of_frames=100;
			scene=ACCESS(Scene)(command_data->default_scene);
			scene_object_name=(char *)NULL;
 			version=3;
 			(option_table[0]).to_be_modified= &binary;
			(option_table[1]).to_be_modified= &file_name;
			(option_table[2]).to_be_modified= &frame_number;
			(option_table[3]).to_be_modified= &full_comments;
			(option_table[4]).to_be_modified= &scene_object_name;
 			(option_table[5]).to_be_modified= &number_of_frames;
			(option_table[6]).to_be_modified= &scene;
			(option_table[6]).user_data=command_data->scene_manager;
 			(option_table[7]).to_be_modified= &version;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene_object_name)
				{
					if (!(scene_object=Scene_get_Scene_object_by_name(scene,
						scene_object_name)))
					{
						display_message(ERROR_MESSAGE,
							"gfx_export_wavefront.  Unable to find object '%s' in scene",
							scene_object_name);
						return_code=0;						
					}
				}
				else
				{
					scene_object=(struct Scene_object *)NULL;
				}
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (scene_object_name)
					{
						if (ALLOCATE(file_name,char,strlen(scene_object_name)+5))
						{
							sprintf(file_name,"%s.obj",scene_object_name);
						}
					}
					else
					{
						if (GET_NAME(Scene)(scene,&file_name))
						{
							if (REALLOCATE(temp_filename,file_name,char,strlen(file_name)+5))
							{
								file_name=temp_filename;
								strcat(file_name,".obj");
							}
						}
					}
				}
				if (file_name)
				{
					if (binary)
 					{
						if (!scene_object)
						{
							if (1==Scene_get_number_of_scene_objects(scene))
							{
								scene_object = first_Scene_object_in_Scene_that(scene,
									(LIST_CONDITIONAL_FUNCTION(Scene_object) *)NULL, NULL);
								if (!Scene_object_has_gt_object(scene_object,
									(struct GT_object *)NULL))
								{
									scene_object = (struct Scene_object *)NULL;
									display_message(ERROR_MESSAGE,"gfx_export_wavefront."
										"Can only export one object or settings at a time with binary wavefront");
									return_code=0;									
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_export_wavefront."
									"Can only export one object or settings at a time with binary wavefront");
								return_code=0;								
							}
						}
 						return_code=export_to_binary_wavefront(file_name,scene_object,
							number_of_frames,version,frame_number);
 					}
 					else
 					{
 						return_code=export_to_wavefront(file_name,scene,scene_object,
							full_comments);
 					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_wavefront.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_wavefront.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_wavefront */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_export(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"alias",NULL,NULL,gfx_export_alias},
		{"node",NULL,NULL,gfx_export_node},
		{"vrml",NULL,NULL,gfx_export_vrml},
		{"wavefront",NULL,NULL,gfx_export_wavefront},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data_void)
		{
			(option_table[0]).user_data=command_data_void;
			(option_table[1]).user_data=command_data_void;
			(option_table[2]).user_data=command_data_void;
			(option_table[3]).user_data=command_data_void;
			return_code=process_option(state,option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_export.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_export.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
int gfx_evaluate(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"destination",NULL,NULL,set_Computed_field_conditional},
		{"dgroup",NULL,NULL,set_FE_node_group},
		{"egroup",NULL,NULL,set_FE_element_group},
		{"ngroup",NULL,NULL,set_FE_node_group},
		{"source",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Computed_field *destination_computed_field,
		*source_computed_field;
	struct Set_Computed_field_conditional_data set_destination_field_data,
		set_source_field_data;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *data_group, *node_group;

	ENTER(gfx_evaluate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			data_group = (struct GROUP(FE_node) *) NULL;
			element_group = (struct GROUP(FE_element) *) NULL;
			node_group = (struct GROUP(FE_node) *) NULL;
			i=0;
			/* destination */
			/* Could have better conditional functions */
			destination_computed_field=(struct Computed_field *)NULL;
			set_destination_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_destination_field_data.conditional_function_user_data=(void *)NULL;
			set_destination_field_data.computed_field_manager =
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			(option_table[i]).to_be_modified= &destination_computed_field;
			(option_table[i]).user_data= &set_destination_field_data;
			i++;
			/* dgroup */
			(option_table[i]).to_be_modified= &data_group;
			(option_table[i]).user_data=command_data->data_group_manager;
			i++;
			/* egroup */
			(option_table[i]).to_be_modified= &element_group;
			(option_table[i]).user_data=command_data->element_group_manager;
			i++;
			/* ngroup */
			(option_table[i]).to_be_modified= &node_group;
			(option_table[i]).user_data=command_data->node_group_manager;
			i++;
			/* source */
			/* Could have better conditional functions */
			source_computed_field=(struct Computed_field *)NULL;
			set_source_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			set_source_field_data.computed_field_manager =
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			(option_table[i]).to_be_modified= &source_computed_field;
			(option_table[i]).user_data= &set_source_field_data;
			i++;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (destination_computed_field&&source_computed_field)
				{
					if (data_group&&(!element_group)&&(!node_group))
					{
						Computed_field_update_nodal_values_from_source(
							destination_computed_field,source_computed_field,
							data_group,command_data->data_manager);
					}
					else if (element_group&&(!data_group)&&(!node_group))
					{
						Computed_field_update_element_values_from_source(
							destination_computed_field,source_computed_field,
							element_group,command_data->element_manager);
					}
					else if (node_group&&(!data_group)&&(!element_group))
					{
						Computed_field_update_nodal_values_from_source(
							destination_computed_field,source_computed_field,
							node_group,command_data->node_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_evaluate.  Must specify one of dgroup/egroup/ngroup");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_evaluate.  Must specify destination and source fields");
					return_code=0;
				}
			}
			if (data_group)
			{
				DEACCESS(GROUP(FE_node))(&data_group);
			}				
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}				
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}				
			if (source_computed_field)
			{
				DEACCESS(Computed_field)(&source_computed_field);
			}
			if (destination_computed_field)
			{
				DEACCESS(Computed_field)(&destination_computed_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_evaluate.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_evaluate.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_evaluate */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_filter(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 1997

DESCRIPTION :
Executes a GFX FILTER command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"node",NULL,NULL,gfx_filter_node},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_filter);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			(option_table[0]).user_data=(void *)(command_data->node_group_manager);
			return_code=process_option(state,option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_filter.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_filter.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_filter */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_grab_frame(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX GRAB_FRAME command.
==============================================================================*/
{
	int number_of_frames,return_code;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_int_positive}
	};

	ENTER(execute_command_gfx_grab_frame);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	/* check argument */
	if (state)
	{
		/* initialize defaults */
		number_of_frames=1;
		(option_table[0]).to_be_modified= &number_of_frames;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			grab_frame(number_of_frames);
		} /* parse error, help */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_grab_frame.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_grab_frame */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a GFX LIST ENVIRONMENT_MAP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;

	ENTER(gfx_list_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					if (environment_map=FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,
						name)(current_token,command_data->environment_map_manager))
					{
						return_code=list_Environment_map(environment_map);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown environment map: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_environment_map.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ENVIRONMENT_MAP_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment map name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_environment_map */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Executes a GFX LIST FIELD.
???RC Could be moved to computed_field.c.
==============================================================================*/
{
	static char	*command_prefix="gfx define field ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,set_Computed_field_conditional}
	};
	struct Computed_field *computed_field;
	struct Computed_field_package *computed_field_package;
	struct LIST(Computed_field) *list_of_fields;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(gfx_list_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((computed_field_package=
			(struct Computed_field_package *)computed_field_package_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				computed_field_package)))
		{
			commands_flag=0;
			/* if no computed_field specified, list all computed_fields */
			computed_field=(struct Computed_field *)NULL;
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					computed_field_package);
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &computed_field;
			(option_table[1]).user_data= &set_field_data;
			(option_table[2]).to_be_modified= &computed_field;
			(option_table[2]).user_data= &set_field_data;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (computed_field)
					{
						return_code=list_Computed_field_commands(computed_field,
							(void *)command_prefix);
					}
					else
					{
						if (list_of_fields=CREATE(LIST(Computed_field))())
						{
							while (return_code&&(computed_field=FIRST_OBJECT_IN_MANAGER_THAT(
								Computed_field)(Computed_field_commands_ready_to_list,
								(void *)list_of_fields,computed_field_manager)))
							{
								return_code=list_Computed_field_commands(computed_field,
									(void *)command_prefix)&&
									ADD_OBJECT_TO_LIST(Computed_field)(computed_field,
										list_of_fields);
							}
							DESTROY(LIST(Computed_field))(&list_of_fields);
						}
						else
						{
							return_code=0;
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"gfx_list_Computed_field.  Could not list field commands");
						}
					}
				}
				else
				{
					if (computed_field)
					{
						return_code=list_Computed_field(computed_field,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							list_Computed_field_name,(void *)NULL,computed_field_manager);
					}
				}
			}
			/* must deaccess computed_field since accessed by set_Computed_field */
			if (computed_field)
			{
				DEACCESS(Computed_field)(&computed_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_Computed_field.  "
				"Missing computed_field_package_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Computed_field.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Computed_field */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_list_FE_element(struct Parse_state *state,
	void *cm_element_type_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 2001

DESCRIPTION :
Executes a GFX LIST ELEMENT.
==============================================================================*/
{
	char all_flag, selected_flag, verbose_flag;
	enum CM_element_type cm_element_type;
	int return_code, start, stop;
	struct CM_element_type_Multi_range_data element_type_ranges_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_element) *element_list;
	struct Multi_range *element_ranges;
	struct Option_table *option_table;

	ENTER(gfx_list_FE_element);
	cm_element_type = (enum CM_element_type)cm_element_type_void;
	if (state && ((CM_ELEMENT == cm_element_type) ||
		(CM_FACE == cm_element_type) || (CM_LINE == cm_element_type)) &&
		(command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		all_flag = 0;
		selected_flag = 0;
		verbose_flag = 0;
		element_group = (struct GROUP(FE_element) *)NULL;
		element_ranges = CREATE(Multi_range)();

		option_table=CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &element_group,
			command_data->element_group_manager, set_FE_element_group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (element_list = FE_element_list_from_all_selected_group_ranges(
				cm_element_type, command_data->element_manager, all_flag,
				command_data->element_selection, selected_flag,
				element_group, element_ranges))
			{
				if (return_code)
				{
					if (0 < NUMBER_IN_LIST(FE_element)(element_list))
					{
						/* always write verbose details if single element asked for and
							 neither all_flag nor selected_flag nor element_group set */
						if (verbose_flag ||
							((!all_flag) && (!selected_flag) && (!element_group) &&
								(1 == Multi_range_get_number_of_ranges(element_ranges)) &&
								(Multi_range_get_range(element_ranges, /*range_number*/0, 
									&start, &stop)) &&
								(start == stop)))
						{
							return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(list_FE_element,
								(void *)NULL, element_list);
						}
						else
						{
							/* write comma separated list of ranges - clear and use existing
								 element_ranges structure */
							switch (cm_element_type)
							{
								case CM_ELEMENT:
								{
									display_message(INFORMATION_MESSAGE,"Elements:\n");
								} break;
								case CM_FACE:
								{
									display_message(INFORMATION_MESSAGE,"Faces:\n");
								} break;
								case CM_LINE:
								{
									display_message(INFORMATION_MESSAGE,"Lines:\n");
								} break;
							}
							Multi_range_clear(element_ranges);
							element_type_ranges_data.cm_element_type = cm_element_type;
							element_type_ranges_data.multi_range = element_ranges;
							if (FOR_EACH_OBJECT_IN_LIST(FE_element)(
								FE_element_of_CM_element_type_add_number_to_Multi_range,
								(void *)&element_type_ranges_data, element_list))
							{
								return_code = Multi_range_display_ranges(element_ranges);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_list_FE_element.  Could not get element ranges");
								return_code = 0;
							}
						}
					}
					else
					{
						switch (cm_element_type)
						{
							case CM_ELEMENT:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx list elements:  No elements specified\n");
							} break;
							case CM_FACE:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx list faces:  No faces specified\n");
							} break;
							case CM_LINE:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx list lines:  No lines specified\n");
							} break;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_FE_element.  Could not fill element_list");
				}
				DESTROY(LIST(FE_element))(&element_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_list_FE_element.  Could not make element_list");
				return_code=0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		DESTROY(Multi_range)(&element_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_FE_element */

static int gfx_list_FE_node(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 2001

DESCRIPTION :
Executes a GFX LIST NODES.
If <use_data> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag, selected_flag, verbose_flag;
	int return_code, start, stop;
	struct Cmiss_command_data *command_data;
	struct FE_node_selection *node_selection;
	struct GROUP(FE_node) *node_group;
	struct LIST(FE_node) *node_list;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;

	ENTER(gfx_list_FE_node);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (use_data)
		{
			node_manager = command_data->data_manager;
			node_group_manager = command_data->data_group_manager;
			node_selection = command_data->data_selection;
		}
		else
		{
			node_manager = command_data->node_manager;
			node_group_manager = command_data->node_group_manager;
			node_selection = command_data->node_selection;
		}
		/* initialise defaults */
		all_flag = 0;
		selected_flag = 0;
		verbose_flag = 0;
		node_group = (struct GROUP(FE_node) *)NULL;
		node_ranges = CREATE(Multi_range)();

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &node_group,
			node_group_manager, set_FE_node_group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (node_list = FE_node_list_from_all_selected_group_ranges(
				node_manager, all_flag, node_selection, selected_flag,
				node_group, node_ranges))
			{
				if (0 < NUMBER_IN_LIST(FE_node)(node_list))
				{
					/* always write verbose details if single node asked for and
						 neither all_flag nor selected_flag nor node_group set */
					if (verbose_flag ||
						((!all_flag) && (!selected_flag) && (!node_group) &&
							(1 == Multi_range_get_number_of_ranges(node_ranges)) &&
							(Multi_range_get_range(node_ranges, /*range_number*/0, 
								&start, &stop)) &&
							(start == stop)))
					{
						return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(list_FE_node,
							(void *)1, node_list);
					}
					else
					{
						if (use_data)
						{
							display_message(INFORMATION_MESSAGE,"Data:\n");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,"Nodes:\n");
						}
						/* write comma separated list of ranges - use existing node
							 ranges structure */
						Multi_range_clear(node_ranges);
						if (FOR_EACH_OBJECT_IN_LIST(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)node_ranges,
							node_list))
						{
							return_code=Multi_range_display_ranges(node_ranges);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_list_FE_node.  Could not get node ranges");
							return_code=0;
						}
					}
				}
				else
				{
					if (use_data)
					{
						display_message(INFORMATION_MESSAGE,
							"gfx list data:  No data specified\n");
					}
					else
					{
						display_message(INFORMATION_MESSAGE,
							"gfx list nodes:  No nodes specified\n");
					}
				}
				DESTROY(LIST(FE_node))(&node_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_list_FE_node.  Could not make node_list");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
		DESTROY(Multi_range)(&node_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_FE_node.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_FE_node */

static int gfx_list_graphical_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Executes a GFX LIST MATERIAL.
???RC Could be moved to material.c.
==============================================================================*/
{
	static char	*command_prefix="gfx create material ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,set_Graphical_material}
	};
	struct Graphical_material *material;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_list_graphical_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphical_material_manager=
			(struct MANAGER(Graphical_material) *)graphical_material_manager_void)
		{
			commands_flag=0;
			/* if no material specified, list all materials */
			material=(struct Graphical_material *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data= graphical_material_manager_void;
			(option_table[2]).to_be_modified= &material;
			(option_table[2]).user_data= graphical_material_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (material)
					{
						return_code=list_Graphical_material_commands(material,
							(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material_commands,(void *)command_prefix,
							graphical_material_manager);
					}
				}
				else
				{
					if (material)
					{
						return_code=list_Graphical_material(material,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material,(void *)NULL,
							graphical_material_manager);
					}
				}
			}
			/* must deaccess material since accessed by set_Graphical_material */
			if (material)
			{
				DEACCESS(Graphical_material)(&material);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphical_material.  "
				"Missing graphical_material_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphical_material */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_group_FE_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *element_group_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 October 1996

DESCRIPTION :
Executes a GFX LIST EGROUP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;

	ENTER(gfx_list_group_FE_element);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (element_group_manager=
			(struct MANAGER(GROUP(FE_element)) *)element_group_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),
						name)(current_token,element_group_manager))
					{
						return_code=list_group_FE_element(element_group,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown element group: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <ELEMENT_GROUP_NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				if (0<NUMBER_IN_MANAGER(GROUP(FE_element))(element_group_manager))
				{
					display_message(INFORMATION_MESSAGE,"element groups:\n");
					return_code=FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_element))(
						list_group_FE_element_name,(void *)NULL,element_group_manager);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"There are no element groups defined\n");
					return_code=1;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_group_FE_element.  Missing element_group_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_group_FE_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_group_FE_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_g_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST G_ELEMENT.
==============================================================================*/
{
	static char	*command_prefix="gfx modify g_element";
	char commands_flag,*command_suffix,*group_name,*scene_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"group",NULL,NULL,set_FE_element_group},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_FE_element_group}
	};
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group;
	struct GT_element_group *gt_element_group;
	struct Scene *scene;

	ENTER(gfx_list_g_element);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			commands_flag=0;
			element_group=(struct GROUP(FE_element) *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &element_group;
			(option_table[1]).user_data= command_data->element_group_manager;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &element_group;
			(option_table[3]).user_data= command_data->element_group_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (GET_NAME(Scene)(scene,&scene_name))
				{
					command_suffix=(char *)NULL;
					if ((scene != command_data->default_scene)&&
						ALLOCATE(command_suffix,char,strlen(scene_name)+8))
					{
						sprintf(command_suffix," scene %s",scene_name);
					}
					if (element_group&&(gt_element_group=
						Scene_get_graphical_element_group(scene,element_group)))
					{
						if (GET_NAME(GROUP(FE_element))(element_group,&group_name))
						{
							if (commands_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Commands for reproducing group %s on scene %s:\n",
									group_name,scene_name);
								return_code=GT_element_group_list_commands(gt_element_group,
									command_prefix,command_suffix);
							}
							else
							{
								display_message(INFORMATION_MESSAGE,
									"Contents of group %s on scene %s:\n",group_name,scene_name);
								return_code=GT_element_group_list_contents(gt_element_group);
							}
							DEALLOCATE(group_name);
						}
					}
					else
					{
						display_message(INFORMATION_MESSAGE,
							"Must specify element group on scene %s\n",scene_name);
						return_code=0;
					}
					if (command_suffix)
					{
						DEALLOCATE(command_suffix);
					}
					DEALLOCATE(scene_name);
				}
			}
			DEACCESS(Scene)(&scene);
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_g_element.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_g_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_g_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_list_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *object_list_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST GLYPH/GRAPHICS_OBJECT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GT_object *object;
	struct LIST(GT_object) *list;

	ENTER(gfx_list_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (list=(struct LIST(GT_object) *)object_list_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						current_token,list))
					{
						return_code=GT_object_list_contents(object,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not find object named '%s'",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," OBJECT_NAME[ALL]");
					return_code=1;
				}
			}
			else
			{
				/* list contents of all objects in list */
				return_code=FOR_EACH_OBJECT_IN_LIST(GT_object)(
					GT_object_list_contents,(void *)NULL,list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_graphics_object.  Missing graphics object list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_list_grid_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 September 2000

DESCRIPTION :
Executes a GFX LIST NODES.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag,ranges_flag,selected_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges_grid_to_multi_range_data grid_to_multi_range_data;
	struct FE_element_grid_to_multi_range_data element_grid_to_multi_range_data;
	struct FE_field *grid_field;
	struct Multi_range *grid_point_ranges,*multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_grid_field_data;

	ENTER(gfx_list_grid_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		all_flag=0;
		selected_flag=0;
		grid_point_ranges=CREATE(Multi_range)();
		if ((grid_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
			"grid_point_number",command_data->fe_field_manager))&&
			FE_field_is_1_component_integer(grid_field,(void *)NULL))
		{
			ACCESS(FE_field)(grid_field);
		}
		else
		{
			grid_field=(struct FE_field *)NULL;
		}

		option_table=CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table,"all",&all_flag,NULL,set_char_flag);
		/* grid_field */
		set_grid_field_data.fe_field_manager=command_data->fe_field_manager;
		set_grid_field_data.conditional_function=FE_field_is_1_component_integer;
		set_grid_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"grid_field",
			&grid_field,&set_grid_field_data,set_FE_field_conditional);
		/* selected */
		Option_table_add_entry(option_table,"selected",&selected_flag,
			NULL,set_char_flag);
		/* default option: grid point number ranges */
		Option_table_add_entry(option_table,(char *)NULL,(void *)grid_point_ranges,
			NULL,set_Multi_range);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (grid_field)
			{
				if (multi_range=CREATE(Multi_range)())
				{
					ranges_flag=(0<Multi_range_get_number_of_ranges(grid_point_ranges));
					if (selected_flag)
					{
						/* fill multi_range with selected grid_point_number ranges */
						grid_to_multi_range_data.grid_fe_field=grid_field;
						grid_to_multi_range_data.multi_range=multi_range;
						grid_to_multi_range_data.all_points_native=1;
						return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
							Element_point_ranges_grid_to_multi_range,
							(void *)&grid_to_multi_range_data,
							Element_point_ranges_selection_get_element_point_ranges_list(
								command_data->element_point_ranges_selection));
					}
					else if (ranges_flag||all_flag)
					{
						/* fill multi_range with all grid_point_number ranges */
						element_grid_to_multi_range_data.grid_fe_field=grid_field;
						element_grid_to_multi_range_data.multi_range=multi_range;
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_grid_to_multi_range,
							(void *)&element_grid_to_multi_range_data,
							command_data->element_manager);
					}
					if (return_code)
					{
						if (ranges_flag)
						{
							/* include in multi_range only values also in grid_point_ranges */
							Multi_range_intersect(multi_range,grid_point_ranges);
						}
						if (0<Multi_range_get_number_of_ranges(multi_range))
						{
							display_message(INFORMATION_MESSAGE,"Grid points:\n");
							return_code=Multi_range_display_ranges(multi_range);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx list grid_points:  No grid points specified");
						}
					}
					DESTROY(Multi_range)(&multi_range);
				}
				else
				{
					return_code=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"To list grid_points, "
					"need integer grid_field (eg. grid_point_number)");
				return_code=0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DESTROY(Multi_range)(&grid_point_ranges);
		if (grid_field)
		{
			DEACCESS(FE_field)(&grid_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_grid_points */

static int gfx_list_group_FE_node(struct Parse_state *state,
	void *use_data,void *node_group_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST DGROUP|NGROUP.
If <use_data> is set help is presented for data, otherwise node. Since the
manager is passed in the arguments, it does not affect normal functionality.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;

	ENTER(gfx_list_group_FE_node);
	if (state && (node_group_manager=
		(struct MANAGER(GROUP(FE_node)) *)node_group_manager_void))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
					current_token,node_group_manager))
				{
					/* check there are no further tokens */
					shift_Parse_state(state,1);
					if (state->current_token)
					{
						display_message(ERROR_MESSAGE,"Unexpected token: %s",
							state->current_token);
						display_parse_state_location(state);
						return_code=0;
					}
					else
					{
						return_code=list_group_FE_node(node_group,(void *)1);
					}
				}
				else
				{
					if (use_data)
					{
						display_message(ERROR_MESSAGE, "Unknown data group: %s",
							current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE, "Unknown node group: %s",
							current_token);
					}
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				if (use_data)
				{
					display_message(INFORMATION_MESSAGE, " <DATA_GROUP_NAME{all}>");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NODE_GROUP_NAME{all}>");
				}
				return_code = 1;
			}
		}
		else
		{
			if (0 < NUMBER_IN_MANAGER(GROUP(FE_node))(node_group_manager))
			{
				if (use_data)
				{
					display_message(INFORMATION_MESSAGE, "Data groups:\n");
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "Node groups:\n");
				}
				return_code = FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_node))(
					list_group_FE_node, (void *)NULL, node_group_manager);
			}
			else
			{
				if (use_data)
				{
					display_message(INFORMATION_MESSAGE,
						"There are no data groups defined\n");
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"There are no node groups defined\n");
				}
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_group_FE_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_group_FE_node */

static int gfx_list_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 September 1996

DESCRIPTION :
Executes a GFX LIST LIGHT.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light *light;
	struct MANAGER(Light) *light_manager;

	ENTER(gfx_list_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (light_manager=(struct MANAGER(Light) *)light_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (light=FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						light_manager))
					{
						return_code=list_Light(light,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Light)(list_Light,(void *)NULL,
					light_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light.  Missing light_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light */

static int gfx_list_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 September 1996

DESCRIPTION :
Executes a GFX LIST LMODEL.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light_model *light_model;
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(gfx_list_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (light_model_manager=
			(struct MANAGER(Light_model) *)light_model_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (light_model=FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(
						current_token,light_model_manager))
					{
						return_code=list_Light_model(light_model,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light model: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_MODEL_NAME");
					return_code=1;
				}
			}
			else
			{
				FOR_EACH_OBJECT_IN_MANAGER(Light_model)(list_Light_model,(void *)NULL,
					light_model_manager);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light_model.  Missing light_model_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light_model */

static int gfx_list_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Executes a GFX LIST SCENE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Scene *scene;
	struct MANAGER(Scene) *scene_manager;

	ENTER(gfx_list_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (scene_manager=(struct MANAGER(Scene) *)scene_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(current_token,
						scene_manager))
					{
						return_code=list_Scene(scene,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown scene: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SCENE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Scene)(list_Scene,(void *)NULL,
					scene_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_scene.  Missing scene_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_scene */

static int gfx_list_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST SPECTRUM.
==============================================================================*/
{
	static char	*command_prefix="gfx modify spectrum";
	char *commands_flag;
	int return_code;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_list_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (spectrum_manager =
		(struct MANAGER(Spectrum) *)spectrum_manager_void))
	{
		commands_flag = 0;
		spectrum = (struct Spectrum *)NULL;

		option_table=CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag,
			NULL, set_char_flag);
		/* default option: spectrum name */
		Option_table_add_entry(option_table, (char *)NULL, &spectrum,
			spectrum_manager_void, set_Spectrum);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (commands_flag)
			{
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,
						"Commands for reproducing spectrum:\n");
					return_code = Spectrum_list_commands(spectrum,
						command_prefix, (char *)NULL);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME\n");
					return_code = 1;
				}
			}
			else
			{
				if (spectrum)
				{
					return_code = Spectrum_list_contents(spectrum, (void *)NULL);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
						Spectrum_list_contents, (void *)NULL, spectrum_manager);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(Spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_spectrum */

static int gfx_list_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Executes a GFX LIST TEXTURE.
???RC Could be moved to texture.c.
==============================================================================*/
{
	static char	*command_prefix="gfx create texture ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Texture},
		{NULL,NULL,NULL,set_Texture}
	};
	struct Texture *texture;
	struct MANAGER(Texture) *texture_manager;

	ENTER(gfx_list_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (texture_manager=(struct MANAGER(Texture) *)texture_manager_void)
		{
			commands_flag=0;
			/* if no texture specified, list all textures */
			texture=(struct Texture *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &texture;
			(option_table[1]).user_data= texture_manager_void;
			(option_table[2]).to_be_modified= &texture;
			(option_table[2]).user_data= texture_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (texture)
					{
						return_code=list_Texture_commands(texture,(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Texture)(
							list_Texture_commands,(void *)command_prefix,texture_manager);
					}
				}
				else
				{
					if (texture)
					{
						return_code=list_Texture(texture,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Texture)(
							list_Texture,(void *)NULL,texture_manager);
					}
				}
			}
			/* must deaccess texture since accessed by set_Texture */
			if (texture)
			{
				DEACCESS(Texture)(&texture);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_texture.  "
				"Missing texture_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_texture */

static int gfx_list_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Executes a GFX LIST TRANSFORMATION.
==============================================================================*/
{
	char *command_prefix,commands_flag,*scene_name,*scene_object_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_list_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			commands_flag=0;
			scene_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			/* parse the command line */
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &scene_object_name;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (GET_NAME(Scene)(scene, &scene_name))
				{
					if ((!scene_object_name)||(scene_object=
						Scene_get_Scene_object_by_name(scene,scene_object_name)))
					{
						if (commands_flag)
						{
							/* quote scene name if it contains special characters */
							make_valid_token(&scene_name);
							if (ALLOCATE(command_prefix, char, 40 + strlen(scene_name)))
							{
								sprintf(command_prefix, "gfx set transformation scene %s name",
									scene_name);
								if (scene_object_name)
								{
									return_code = list_Scene_object_transformation_commands(
										scene_object,(void *)command_prefix);
								}
								else
								{
									return_code = for_each_Scene_object_in_Scene(scene,
										list_Scene_object_transformation_commands,
										(void *)command_prefix);
								}
								DEALLOCATE(command_prefix);
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							if (scene_object_name)
							{
								return_code=
									list_Scene_object_transformation(scene_object,(void *)NULL);
							}
							else
							{
								return_code=for_each_Scene_object_in_Scene(scene,
									list_Scene_object_transformation,(void *)NULL);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No object named '%s' in scene %s",scene_object_name,scene_name);
						return_code=0;
					}
					DEALLOCATE(scene_name);
				}
				else
				{
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_transformation */

static int iterator_list_VT_volume_texture(struct VT_volume_texture *texture,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Iterator function version of list_VT_volume_texture
???DB.  Move to volume_texture.c ?
==============================================================================*/
{
	int return_code;

	ENTER(iterator_list_VT_volume_texture);
	USE_PARAMETER(dummy_user_data);
	return_code=list_VT_volume_texture(texture);
	LEAVE;

	return (return_code);
} /* iterator_list_VT_volume_texture */

static int gfx_list_volume_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 August 1998

DESCRIPTION :
Executes a GFX LIST VTEXTURE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct VT_volume_texture *texture;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;

	ENTER(gfx_list_volume_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,name)(
						current_token,volume_texture_manager))
					{
						return_code=list_VT_volume_texture(texture);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume texture: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," VOLUME_TEXTURE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(VT_volume_texture)(
					iterator_list_VT_volume_texture,(void *)NULL,
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_volume_texture.  Missing volume_texture_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_volume_texture */

static int gfx_list_movie_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *movie_graphics_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Executes a GFX LIST MOVIE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Movie_graphics *movie;
	struct MANAGER(Movie_graphics) *movie_graphics_manager;

	ENTER(gfx_list_movie_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (movie_graphics_manager=
			(struct MANAGER(Movie_graphics) *)movie_graphics_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						current_token,movie_graphics_manager))
					{
						return_code=list_Movie_graphics(movie,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume movie: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MOVIE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Movie_graphics)(
					list_Movie_graphics,(void *)NULL,movie_graphics_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_movie_graphics.  Missing movie_graphics_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_movie_graphics.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_movie_graphics */

static int gfx_list_graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Graphics_window}
	};
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_list_graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphics_window_manager=
			(struct MANAGER(Graphics_window) *)graphics_window_manager_void)
		{
			commands_flag=0;
			/* if no window specified, list all windows */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &window;
			(option_table[1]).user_data= graphics_window_manager_void;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data= graphics_window_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (window)
					{
						return_code=list_Graphics_window_commands(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window_commands,(void *)NULL,
							graphics_window_manager);
					}
				}
				else
				{
					if (window)
					{
						return_code=list_Graphics_window(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window,(void *)NULL,
							graphics_window_manager);
					}
				}
			}
			/* must deaccess window since accessed by set_Graphics_window */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphics_window.  "
				"Missing graphics_window_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_window */

static int execute_command_gfx_list(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_list);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* curve */
			Option_table_add_entry(option_table, "curve", NULL,
				command_data->control_curve_manager, gfx_list_Control_curve);
			/* data */
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_list_FE_node);
			/* dgroup */
			Option_table_add_entry(option_table, "dgroup", /*use_data*/(void *)1,
				command_data->data_group_manager, gfx_list_group_FE_node);
			/* egroup */
			Option_table_add_entry(option_table, "egroup", NULL,
				command_data->element_group_manager, gfx_list_group_FE_element);
			/* element */
			Option_table_add_entry(option_table, "elements", (void *)CM_ELEMENT,
				command_data_void, gfx_list_FE_element);
			/* environment_map */
			Option_table_add_entry(option_table, "environment_map", NULL,
				command_data_void, gfx_list_environment_map);
			/* faces */
			Option_table_add_entry(option_table, "faces", (void *)CM_FACE,
				command_data_void, gfx_list_FE_element);
			/* field */
			Option_table_add_entry(option_table, "field", NULL,
				command_data->computed_field_package, gfx_list_Computed_field);
			/* g_element */
			Option_table_add_entry(option_table, "g_element", NULL,
				command_data_void, gfx_list_g_element);
			/* glyph */
			Option_table_add_entry(option_table, "glyph", NULL,
				command_data->glyph_list, gfx_list_graphics_object);
			/* graphics_object */
			Option_table_add_entry(option_table, "graphics_object", NULL,
				command_data->graphics_object_list, gfx_list_graphics_object);
			/* grid_points */
			Option_table_add_entry(option_table, "grid_points", NULL,
				command_data_void, gfx_list_grid_points);
			/* light */
			Option_table_add_entry(option_table, "light", NULL,
				command_data->light_manager, gfx_list_light);
			/* lines */
			Option_table_add_entry(option_table, "lines", (void *)CM_LINE,
				command_data_void, gfx_list_FE_element);
			/* lmodel */
			Option_table_add_entry(option_table, "lmodel", NULL,
				command_data->light_model_manager, gfx_list_light_model);
			/* material */
			Option_table_add_entry(option_table, "material", NULL,
				command_data->graphical_material_manager, gfx_list_graphical_material);
#if defined (SGI_MOVIE_FILE)
			/* movie */
			Option_table_add_entry(option_table, "movie", NULL,
				command_data->movie_graphics_manager, gfx_list_movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
			/* ngroup */
			Option_table_add_entry(option_table, "ngroup", /*use_data*/(void *)0,
				command_data->node_group_manager, gfx_list_group_FE_node);
			/* nodes */
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_list_FE_node);
			/* slider */
			Option_table_add_entry(option_table, "slider", NULL,
				command_data->node_group_slider_dialog, list_node_group_slider);
			/* scene */
			Option_table_add_entry(option_table, "scene", NULL,
				command_data->scene_manager, gfx_list_scene);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data->spectrum_manager, gfx_list_spectrum);
			/* texture */
			Option_table_add_entry(option_table, "texture", NULL,
				command_data->texture_manager, gfx_list_texture);
			/* transformation */
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_list_transformation);
			/* volume texture */
			Option_table_add_entry(option_table, "vtexture", NULL,
				command_data->volume_texture_manager, gfx_list_volume_texture);
			/* graphics window */
			Option_table_add_entry(option_table, "window", NULL,
				command_data->graphics_window_manager, gfx_list_graphics_window);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx list", command_data->command_window);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_list.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_list */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_element_group(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Executes a GFX MODIFY EGROUP command.
==============================================================================*/
{
	char *group_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"add_ranges",NULL,NULL,set_Multi_range},
		{"from",NULL,NULL,set_FE_element_group},
		{"remove_ranges",NULL,NULL,set_Multi_range},
		{NULL,NULL,NULL,NULL}
	};
	struct Add_FE_element_to_list_if_in_range_data add_element_data,
		remove_element_data;
	struct Cmiss_command_data *command_data;
	struct GROUP(FE_element) *element_group,*from_element_group;
	struct GROUP(FE_node) *node_group;
	struct LIST(FE_node) *add_node_list,*remove_node_list;

	ENTER(gfx_modify_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		element_group=(struct GROUP(FE_element) *)NULL;
		if (set_FE_element_group(state,(void *)&element_group,
			(void *)command_data->element_group_manager))
		{
			/* initialise defaults */
			from_element_group=(struct GROUP(FE_element) *)NULL;
			add_element_data.element_ranges=CREATE(Multi_range)();
			add_element_data.element_list=CREATE(LIST(FE_element))();
			remove_element_data.element_ranges=CREATE(Multi_range)();
			remove_element_data.element_list=CREATE(LIST(FE_element))();
			(option_table[0]).to_be_modified=add_element_data.element_ranges;
			(option_table[1]).to_be_modified= &from_element_group;
			(option_table[1]).user_data=command_data->element_group_manager;
			(option_table[2]).to_be_modified=remove_element_data.element_ranges;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (element_group&&
					GET_NAME(GROUP(FE_element))(element_group,&group_name))
				{
					if ((0<Multi_range_get_number_of_ranges(
						add_element_data.element_ranges))||
						(0<Multi_range_get_number_of_ranges(
							remove_element_data.element_ranges)))
					{
						if (!Multi_ranges_overlap(add_element_data.element_ranges,
							remove_element_data.element_ranges))
						{
							/* get node group of the same name to synchronize nodes from
								 elements added and removed */
							if (node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
								group_name,command_data->node_group_manager))
							{
								if (0<Multi_range_get_number_of_ranges(
									add_element_data.element_ranges))
								{
									if (from_element_group)
									{
										return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
											add_FE_element_to_list_if_in_range,
											(void *)&add_element_data,from_element_group);
									}
									else
									{
										return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
											add_FE_element_to_list_if_in_range,
											(void *)&add_element_data,command_data->element_manager);
									}
								}
								if (0<Multi_range_get_number_of_ranges(
									remove_element_data.element_ranges))
								{
									return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
										add_FE_element_to_list_if_in_range,
										(void *)&remove_element_data,element_group);
								}
								if (return_code)
								{
									/* make sure we add any nodes referred to by new elements to
										 the node group of the same name, and that nodes only
										 referred to by elements we are removing are removed */
									add_node_list=CREATE(LIST(FE_node))();
									remove_node_list=CREATE(LIST(FE_node))();
									MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
									MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group);
									return_code=
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_FE_element_and_faces_are_in_group,
											(void *)element_group,add_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_FE_element_and_faces_are_not_in_group,
											(void *)element_group,remove_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_top_level_FE_element_nodes_are_in_list,
											(void *)add_node_list,add_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_element)(
											ensure_top_level_FE_element_nodes_are_in_list,
											(void *)remove_node_list,
											remove_element_data.element_list)&&
										FOR_EACH_OBJECT_IN_GROUP(FE_element)(
											ensure_top_level_FE_element_nodes_are_not_in_list,
											(void *)remove_node_list,element_group)&&
										FOR_EACH_OBJECT_IN_LIST(FE_node)(
											ensure_FE_node_is_in_group,(void *)node_group,
											add_node_list)&&
										FOR_EACH_OBJECT_IN_LIST(FE_node)(
											ensure_FE_node_is_not_in_group,(void *)node_group,
											remove_node_list);
									MANAGED_GROUP_END_CACHE(FE_node)(node_group);
									MANAGED_GROUP_END_CACHE(FE_element)(element_group);
									DESTROY(LIST(FE_node))(&add_node_list);
									DESTROY(LIST(FE_node))(&remove_node_list);
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,"gfx modify ngroup. Failed");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_modify_element_group.  "
									"Could not find node group %s",group_name);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx modify egroup.  Add and remove ranges overlap");
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"gfx modify egroup.  No modifications requested");
					}
					DEALLOCATE(group_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx modify egroup.  Missing group name");
					return_code=0;
				}
			}
			DESTROY(LIST(FE_element))(&add_element_data.element_list);
			DESTROY(Multi_range)(&add_element_data.element_ranges);
			DESTROY(Multi_range)(&remove_element_data.element_ranges);
			DESTROY(LIST(FE_element))(&remove_element_data.element_list);
			if (from_element_group)
			{
				DEACCESS(GROUP(FE_element))(&from_element_group);
			}
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_element_group.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_element_group */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int iterator_modify_g_element(struct GROUP(FE_element) *element_group,
	void *modify_g_element_data_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Modify_g_element_data *modify_g_element_data;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *same_settings;

	ENTER(iterator_modify_g_element);
	if (element_group&&(modify_g_element_data=
		(struct Modify_g_element_data *)modify_g_element_data_void))
	{
		if (gt_element_group=Scene_get_graphical_element_group(
			modify_g_element_data->scene,element_group))
		{
			/* get settings describing same geometry in list */
			same_settings=first_settings_in_GT_element_group_that(
				gt_element_group,GT_element_settings_same_geometry,
				(void *)(modify_g_element_data->settings));
			if (modify_g_element_data->delete_flag)
			{
				/* delete */
				if (same_settings)
				{
					return_code=GT_element_group_remove_settings(
						gt_element_group,same_settings);
				}
				else
				{
					return_code=1;
				}
			}
			else
			{
				/* add/modify */
				if (same_settings)
				{
					ACCESS(GT_element_settings)(same_settings);
					if (-1 != modify_g_element_data->position)
					{
						/* move same_settings to new position */
						GT_element_group_remove_settings(gt_element_group,same_settings);
						GT_element_group_add_settings(gt_element_group,same_settings,
							modify_g_element_data->position);
					}
					/* modify same_settings to match new ones */
					return_code = GT_element_group_modify_settings(gt_element_group,
						same_settings,modify_g_element_data->settings);
					DEACCESS(GT_element_settings)(&same_settings);
				}
				else
				{
					return_code=0;
					if (same_settings=CREATE(GT_element_settings)(
						GT_element_settings_get_settings_type(
							modify_g_element_data->settings)))
					{
						ACCESS(GT_element_settings)(same_settings);
						if (COPY(GT_element_settings)(same_settings,
							modify_g_element_data->settings))
						{
							return_code=GT_element_group_add_settings(gt_element_group,
								same_settings,modify_g_element_data->position);
						}
						DEACCESS(GT_element_settings)(&same_settings);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"iterator_modify_g_element.  g_element not in scene");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"iterator_modify_g_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* iterator_modify_g_element */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_g_element(struct Parse_state *state,
	void *help_mode,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 December 1999

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT command.
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry
		help_option_table[]=
		{
			{"ELEMENT_GROUP_NAME|ALL",NULL,NULL,gfx_modify_g_element},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{NULL,NULL,NULL,set_FE_element_group_or_all}
		},
		valid_group_option_table[]=
		{
			{"cylinders",NULL,NULL,gfx_modify_g_element_cylinders},
			{"data_points",NULL,NULL,gfx_modify_g_element_data_points},
			{"element_points",NULL,NULL,gfx_modify_g_element_element_points},
			{"general",NULL,NULL,gfx_modify_g_element_general},
			{"iso_surfaces",NULL,NULL,gfx_modify_g_element_iso_surfaces},
			{"lines",NULL,NULL,gfx_modify_g_element_lines},
			{"node_points",NULL,NULL,gfx_modify_g_element_node_points},
			{"streamlines",NULL,NULL,gfx_modify_g_element_streamlines},
			{"surfaces",NULL,NULL,gfx_modify_g_element_surfaces},
			{"volumes",NULL,NULL,gfx_modify_g_element_volumes},
			{NULL,NULL,NULL,NULL}
		};
	struct Cmiss_command_data *command_data;
	struct G_element_command_data g_element_command_data;
	struct GROUP(FE_element) *element_group;
	struct Modify_g_element_data modify_g_element_data;

	ENTER(gfx_modify_g_element);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* initialize defaults */
			element_group=(struct GROUP(FE_element) *)NULL;
			if (!help_mode)
			{
				if (!state->current_token||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* read element group */
					(option_table[0]).to_be_modified= (void *)(&element_group);
					(option_table[0]).user_data=
						(void *)(command_data->element_group_manager);
					return_code=process_option(state,option_table);
				}
				else
				{
					/* write help - use help_mode flag to get correct behaviour */
					(help_option_table[0]).to_be_modified= (void *)1;
					(help_option_table[0]).user_data=command_data_void;
					return_code=process_option(state,help_option_table);
				}
			}
			if (return_code)
			{
				/* set defaults */
				modify_g_element_data.delete_flag=0;
				modify_g_element_data.position=-1;
				modify_g_element_data.scene=ACCESS(Scene)(command_data->default_scene);
				modify_g_element_data.settings=(struct GT_element_settings *)NULL;
				g_element_command_data.default_material=
					command_data->default_graphical_material;
				g_element_command_data.glyph_list=command_data->glyph_list;
				g_element_command_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
				g_element_command_data.element_manager=command_data->element_manager;
				g_element_command_data.fe_field_manager=command_data->fe_field_manager;
				g_element_command_data.graphical_material_manager=
					command_data->graphical_material_manager;
				g_element_command_data.scene_manager=command_data->scene_manager;
				g_element_command_data.spectrum_manager=command_data->spectrum_manager;
				g_element_command_data.default_spectrum=command_data->default_spectrum;
				g_element_command_data.user_interface=command_data->user_interface;
				g_element_command_data.texture_manager=command_data->texture_manager;
				g_element_command_data.volume_texture_manager=
					command_data->volume_texture_manager;
				(valid_group_option_table[0]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[0]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[1]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[1]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[2]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[2]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[3]).to_be_modified=(void *)element_group;
				(valid_group_option_table[3]).user_data=
					(void *)command_data->default_scene;
				(valid_group_option_table[4]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[4]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[5]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[5]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[6]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[6]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[7]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[7]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[8]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[8]).user_data=
					(void *)&g_element_command_data;
				(valid_group_option_table[9]).to_be_modified=
					(void *)(&modify_g_element_data);
				(valid_group_option_table[9]).user_data=
					(void *)&g_element_command_data;
				return_code=process_option(state,valid_group_option_table);
				if (return_code&&(modify_g_element_data.settings))
				{
					if (element_group)
					{
						return_code=iterator_modify_g_element(element_group,
							(void *)&modify_g_element_data);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_modify_g_element.  Must specify element group");
						return_code=0;
#if defined (OLD_CODE)
						return_code=FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_element))(
							iterator_modify_g_element,(void *)&modify_g_element_data,
							command_data->element_group_manager);
#endif /* defined (OLD_CODE) */
					}
				} /* parse error,help */
				if (modify_g_element_data.settings)
				{
					DEACCESS(GT_element_settings)(&(modify_g_element_data.settings));
				}
				DEACCESS(Scene)(&modify_g_element_data.scene);
			} /* parse error,help */
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_g_element.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_g_element.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_modify_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	char glyph_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"glyph",NULL,NULL,set_char_flag},
		{"material",NULL,NULL,set_Graphical_material},
		{"spectrum",NULL,NULL,set_Spectrum},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct GT_object *graphics_object;
	struct Graphical_material *material;
	struct Spectrum *spectrum;

	ENTER(gfx_modify_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		graphics_object=(struct GT_object *)NULL;
		if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
		{
			if(graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)
				(state->current_token,command_data->graphics_object_list))
			{
				shift_Parse_state(state,1);				
				/* initialise defaults */
				glyph_flag = 0;
				if (material = get_GT_object_default_material(graphics_object))
				{
					ACCESS(Graphical_material)(material);
				}
				if (spectrum = get_GT_object_spectrum(graphics_object))
				{
					ACCESS(Spectrum)(spectrum);
				}
				(option_table[0]).to_be_modified=&glyph_flag;
				(option_table[1]).to_be_modified=&material;
				(option_table[1]).user_data=command_data->graphical_material_manager;
				(option_table[2]).to_be_modified=&spectrum;
				(option_table[2]).user_data=command_data->spectrum_manager;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					set_GT_object_default_material(graphics_object, material);
					set_GT_object_Spectrum(graphics_object, spectrum);
					if (glyph_flag)
					{
						if (!(IS_OBJECT_IN_LIST(GT_object)(graphics_object, 
							command_data->glyph_list)))
						{
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object, 
							command_data->glyph_list);
						}
					}
				}
				if (material)
				{
					DEACCESS(Graphical_material)(&material);
				}
				if (spectrum)
				{
					DEACCESS(Spectrum)(&spectrum);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not find object named '%s'",
					state->current_token);
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_graphics_object.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_graphics_object */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_modify_node_group(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Modifies the membership of a group.  Only one of <add> or <remove> can
be specified at once.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char add_flag, all_flag, remove_flag, selected_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_node_selection *node_selection;
	struct GROUP(FE_node) *modify_node_group, *from_node_group;
	struct LIST(FE_node) *node_list;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;

	ENTER(gfx_modify_node_group);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		modify_node_group = (struct GROUP(FE_node) *)NULL;
		if (use_data)
		{
			node_manager = command_data->data_manager;
			node_group_manager = command_data->data_group_manager;
			node_selection = command_data->data_selection;
		}
		else
		{
			node_manager = command_data->node_manager;
			node_group_manager = command_data->node_group_manager;
			node_selection = command_data->node_selection;
		}
		if (set_FE_node_group(state, (void *)&modify_node_group,
			(void *)node_group_manager))
		{
			/* initialise defaults */
			add_flag = 0;
			remove_flag = 0;
			all_flag = 0;
			selected_flag = 0;
			node_ranges = CREATE(Multi_range)();
			from_node_group = (struct GROUP(FE_node) *)NULL;

			option_table=CREATE(Option_table)();
			/* add */
			Option_table_add_entry(option_table, "add", &add_flag,
				NULL, set_char_flag);
			/* all */
			Option_table_add_entry(option_table, "all", &all_flag,
				NULL, set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &from_node_group,
				node_group_manager, set_FE_node_group);
			/* remove */
			Option_table_add_entry(option_table, "remove", &remove_flag,
				NULL, set_char_flag);
			/* selected */
			Option_table_add_entry(option_table, "selected", &selected_flag,
				NULL, set_char_flag);
			/* default option: node number ranges */
			Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
				NULL, set_Multi_range);
			if (return_code = Option_table_multi_parse(option_table, state))
			{
				if (add_flag && remove_flag)
				{
					if (use_data)
					{
						display_message(ERROR_MESSAGE,"gfx modify dgroup:  "
							"Only specify one of add or remove at a time.");
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx modify ngroup:  "
							"Only specify one of add or remove at a time.");
					}
					return_code = 0;
				}
				if ((!add_flag) && (!remove_flag))
				{
					if (use_data)
					{
						display_message(ERROR_MESSAGE,"gfx modify dgroup:  "
							"Must specify an operation, either add or remove.");				
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx modify ngroup:  "
							"Must specify an operation, either add or remove.");				
					}
					return_code = 0;
				}
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				/* make list of nodes to add/remove from modify_node_group */
				if (node_list = FE_node_list_from_all_selected_group_ranges(
					node_manager, all_flag, node_selection, selected_flag,
					from_node_group, node_ranges))
				{
					if (0 < NUMBER_IN_LIST(FE_node)(node_list))
					{
						MANAGED_GROUP_BEGIN_CACHE(FE_node)(modify_node_group);
						if (add_flag)
						{
							return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
								ensure_FE_node_is_in_group, (void *)modify_node_group,
								node_list);
						}
						else /* remove_flag */
						{
							return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
								ensure_FE_node_is_not_in_group, (void *)modify_node_group,
								node_list);
						}
						MANAGED_GROUP_END_CACHE(FE_node)(modify_node_group);
					}
					else
					{
						if (use_data)
						{
							display_message(WARNING_MESSAGE,
								"gfx modify dgroup:  No data specified");
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx modify ngroup:  No nodes specified");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_node_group.  Could not make node_list");
					return_code = 0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (from_node_group)
			{
				DEACCESS(GROUP(FE_node))(&from_node_group);
			}
			DESTROY(Multi_range)(&node_ranges);
		}
		if (modify_node_group)
		{
			DEACCESS(GROUP(FE_node))(&modify_node_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_node_group.  Missing command_data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_node_group */

struct FE_node_field_component_derivatives_data
{
	int *components_number_of_derivatives, number_of_components;
	enum FE_nodal_value_type **components_nodal_value_types;
};

int set_FE_node_field_component_derivatives(struct Parse_state *state,
	void *component_derivatives_data_void, void *field_address_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2001

DESCRIPTION :
Modifier function for entering the number of derivatives and their names
(d/d2, d2/ds1ds3 etc.) for each component of field to be defined at nodes.
Note requires field to be specified prior to entering this function, unless in
help mode.
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type *nodal_value_types;
	int i, j, number_of_components, number_of_derivatives, return_code;
	struct FE_field *field, **field_address;
	struct FE_node_field_component_derivatives_data *component_derivatives_data;

	ENTER(set_FE_node_field_component_derivatives);
	component_derivatives_data =
		(struct FE_node_field_component_derivatives_data *)NULL;
	if (state && (component_derivatives_data =
		(struct FE_node_field_component_derivatives_data *)
		component_derivatives_data_void) &&
		(0 == component_derivatives_data->number_of_components) &&
		(field_address = (struct FE_field **)field_address_void))
	{
		return_code = 1;
		if (field = *field_address)
		{
			number_of_components = get_FE_field_number_of_components(field);
		}
		else
		{
			/* following used for help */
			number_of_components = 3;
		}
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (field)
				{
					ALLOCATE(component_derivatives_data->components_number_of_derivatives,
						int, number_of_components);
					if (ALLOCATE(component_derivatives_data->components_nodal_value_types,
						enum FE_nodal_value_type *, number_of_components))
					{
						for (i = 0; i < number_of_components; i++)
						{
							component_derivatives_data->components_nodal_value_types[i] =
								(enum FE_nodal_value_type *)NULL;
						}
					}
					if (component_derivatives_data->components_number_of_derivatives &&
						component_derivatives_data->components_nodal_value_types)
					{
						/* remember number_of_components so arrays can be deallocated by
							 themselves */
						component_derivatives_data->number_of_components =
							number_of_components;
						for (i = 0; return_code && (i < number_of_components); i++)
						{
							if ((current_token = state->current_token) &&
								(1 == sscanf(current_token, " %d ", &number_of_derivatives)) &&
								(0 <= number_of_derivatives) && shift_Parse_state(state, 1))
							{
								component_derivatives_data->components_number_of_derivatives[i]
									= number_of_derivatives;
								if (ALLOCATE(nodal_value_types, enum FE_nodal_value_type,
									1 + number_of_derivatives))
								{
									component_derivatives_data->components_nodal_value_types[i] =
										nodal_value_types;
									/* first FE_NODAL_VALUE, then derivatives */
									nodal_value_types[0] = FE_NODAL_VALUE;
									for (j = 0; return_code && (j < number_of_derivatives); j++)
									{
										if (!((current_token = state->current_token) &&
											STRING_TO_ENUMERATOR(FE_nodal_value_type)(current_token,
												&(nodal_value_types[j+1])) &&
											shift_Parse_state(state, 1)))
										{
											display_message(ERROR_MESSAGE,
												"Missing or invalid nodal value type: %s",
												current_token);
											display_parse_state_location(state);
											return_code = 0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_FE_node_field_component_derivatives.  "
										"Not enough memory for nodal_value_types");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Missing or invalid number of derivatives: %s",
									current_token);
								display_parse_state_location(state);
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_FE_node_field_component_derivatives.  "
							"Not enough memory for derivatives");
						return_code = 0;
					}
					if (!return_code)
					{
						DEALLOCATE(
							component_derivatives_data->components_number_of_derivatives);
						if (component_derivatives_data->components_nodal_value_types)
						{
							for (i = 0; i < number_of_components; i++)
							{
								if (component_derivatives_data->components_nodal_value_types[i])
								{
									DEALLOCATE(component_derivatives_data->
										components_nodal_value_types[i]);
								}
							}
							DEALLOCATE(
								component_derivatives_data->components_nodal_value_types);
						}
						component_derivatives_data->number_of_components = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must have a field before setting component derivatives");
					return_code = 0;
				}
			}
			else
			{
				for (i = 0; i < number_of_components; i++)
				{
					display_message(INFORMATION_MESSAGE," NUMBER_IN_COMPONENT_%d "
						"DERIVATIVE_NAMES(d/ds1 d2/ds1ds2 etc.) ...", i + 1);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing component derivatives");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		if (component_derivatives_data &&
			(0 != component_derivatives_data->number_of_components))
		{
			display_message(ERROR_MESSAGE,
				"Component derivatives have already been set");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_node_field_component_derivatives.  Invalid argument(s)");
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_field_component_derivatives */

struct FE_node_field_component_versions_data
{
	int *components_number_of_versions, number_of_components;
};

int set_FE_node_field_component_versions(struct Parse_state *state,
	void *component_versions_data_void, void *field_address_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2001

DESCRIPTION :
Modifier function for entering the number of versions for each component of
field to be defined at nodes. Note requires field to be specified prior to
entering this function, unless in help mode.
==============================================================================*/
{
	char *current_token;
	int i, number_of_components, number_of_versions, return_code;
	struct FE_field *field, **field_address;
	struct FE_node_field_component_versions_data *component_versions_data;

	ENTER(set_FE_node_field_component_versions);
	component_versions_data =
		(struct FE_node_field_component_versions_data *)NULL;
	if (state && (component_versions_data =
		(struct FE_node_field_component_versions_data *)
		component_versions_data_void) &&
		(0 == component_versions_data->number_of_components) &&
		(field_address = (struct FE_field **)field_address_void))
	{
		return_code = 1;
		if (field = *field_address)
		{
			number_of_components = get_FE_field_number_of_components(field);
		}
		else
		{
			/* following used for help */
			number_of_components = 3;
		}
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (field)
				{
					if (ALLOCATE(component_versions_data->components_number_of_versions,
						int, number_of_components))
					{
						/* remember number_of_components so arrays can be deallocated by
							 themselves */
						component_versions_data->number_of_components =
							number_of_components;
						for (i = 0; return_code && (i < number_of_components); i++)
						{
							if ((current_token = state->current_token) &&
								(1 == sscanf(current_token, " %d ", &number_of_versions)) &&
								(0 < number_of_versions) &&
								shift_Parse_state(state, 1))
							{
								component_versions_data->components_number_of_versions[i] =
									number_of_versions;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Missing or invalid number of versions: %s",current_token);
								display_parse_state_location(state);
								return_code = 0;
							}
						}
						if (!return_code)
						{
							DEALLOCATE(
								component_versions_data->components_number_of_versions);
							component_versions_data->number_of_components = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_FE_node_field_component_versions.  "
							"Not enough memory for versions");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must have a field before setting component versions");
					return_code = 0;
				}
			}
			else
			{
				for (i = 0; i < number_of_components; i++)
				{
					display_message(INFORMATION_MESSAGE,
						" NUMBER_IN_COMPONENT_%d", i + 1);
				}
				if (!field)
				{
					display_message(INFORMATION_MESSAGE, " ...");
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing component versions");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		if (component_versions_data &&
			(0 != component_versions_data->number_of_components))
		{
			display_message(ERROR_MESSAGE,
				"Component versions have already been set");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_node_field_component_versions.  Invalid argument(s)");
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_field_component_versions */

static int gfx_modify_nodes(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2001

DESCRIPTION :
Executes a GFX DESTROY NODES/DATA command.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag, selected_flag;
	enum FE_nodal_value_type *nodal_value_types;
	int i, number_of_components, return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *define_field, *undefine_field;
	struct FE_node *node, *temp_node;
	struct FE_node_field_component_derivatives_data component_derivatives_data;
	struct FE_node_field_component_versions_data component_versions_data;
	struct FE_node_selection *node_selection;
	struct GROUP(FE_node) *node_group;
	struct LIST(FE_node) *define_node_list, *node_list;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;

	ENTER(gfx_modify_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (use_data)
		{
			element_manager = (struct MANAGER(FE_element) *)NULL;
			node_manager = command_data->data_manager;
			node_group_manager = command_data->data_group_manager;
			node_selection = command_data->data_selection;
		}
		else
		{
			element_manager = command_data->element_manager;
			node_manager = command_data->node_manager;
			node_group_manager = command_data->node_group_manager;
			node_selection = command_data->node_selection;
		}
		/* initialise defaults */
		all_flag = 0;
		selected_flag = 0;
		node_group = (struct GROUP(FE_node) *)NULL;
		node_ranges = CREATE(Multi_range)();
		define_field = (struct FE_field *)NULL;
		component_derivatives_data.number_of_components = 0;
		component_derivatives_data.components_number_of_derivatives = (int *)NULL;
		component_derivatives_data.components_nodal_value_types =
			(enum FE_nodal_value_type **)NULL;
		component_versions_data.number_of_components = 0;
		component_versions_data.components_number_of_versions = (int *)NULL;
		undefine_field = (struct FE_field *)NULL;

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* component_derivatives */
		Option_table_add_entry(option_table, "component_derivatives",
			(void *)&component_derivatives_data, (void *)&define_field,
			set_FE_node_field_component_derivatives);
		/* component_versions */
		Option_table_add_entry(option_table, "component_versions",
			(void *)&component_versions_data, (void *)&define_field,
			set_FE_node_field_component_versions);
		/* define */
		Option_table_add_entry(option_table, "define",
			&define_field, command_data->fe_field_manager, set_FE_field);
		/* group */
		Option_table_add_entry(option_table, "group", &node_group,
			node_group_manager, set_FE_node_group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* undefine */
		Option_table_add_entry(option_table, "undefine",
			&undefine_field, command_data->fe_field_manager, set_FE_field);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if ((!define_field) && (!undefine_field))
			{
				display_message(WARNING_MESSAGE,
					"gfx modify data:  Must specify define or undefine field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (node_list = FE_node_list_from_all_selected_group_ranges(
				node_manager, all_flag, node_selection, selected_flag,
				node_group, node_ranges))
			{
				if (0 < NUMBER_IN_LIST(FE_node)(node_list))
				{
					if (define_field)
					{
						number_of_components =
							get_FE_field_number_of_components(define_field);
						if (0 == component_derivatives_data.number_of_components)
						{
							if (ALLOCATE(
								component_derivatives_data.components_number_of_derivatives,
								int, number_of_components))
							{
								for (i = 0; i < number_of_components; i++)
								{
									component_derivatives_data.
										components_number_of_derivatives[i] = 0;
								}
							}
							else
							{
								return_code = 0;
							}
							if (ALLOCATE(
								component_derivatives_data.components_nodal_value_types,
								enum FE_nodal_value_type *, number_of_components))
							{
								for (i = 0; i < number_of_components; i++)
								{
									component_derivatives_data.components_nodal_value_types[i] =
										(enum FE_nodal_value_type *)NULL;
								}
								for (i = 0; return_code && (i < number_of_components); i++)
								{
									if (ALLOCATE(nodal_value_types, enum FE_nodal_value_type, 1))
									{
										nodal_value_types[0] = FE_NODAL_VALUE;
										component_derivatives_data.components_nodal_value_types[i]
											= nodal_value_types;
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
							if (return_code)
							{
								component_derivatives_data.number_of_components =
									number_of_components;
							}
						}
						if (0 == component_versions_data.number_of_components)
						{
							if (ALLOCATE(
								component_versions_data.components_number_of_versions,
								int, number_of_components))
							{
								for (i = 0; i < number_of_components; i++)
								{
									component_versions_data.
										components_number_of_versions[i] = 1;
								}
							}
							else
							{
								return_code = 0;
							}
							if (return_code)
							{
								component_versions_data.number_of_components =
									number_of_components;
							}
						}
						if (return_code)
						{
							if ((number_of_components ==
								component_derivatives_data.number_of_components) &&
								(number_of_components ==
									component_versions_data.number_of_components))
							{
								MANAGER_BEGIN_CACHE(FE_node)(node_manager);
								define_node_list = CREATE(LIST(FE_node))();
								if (COPY_LIST(FE_node)(define_node_list, node_list))
								{
									while (return_code &&
										(node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
											(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
											define_node_list)))
									{
										if (!FE_field_is_defined_at_node(define_field, node))
										{
											if (temp_node = CREATE(FE_node)(0, node))
											{
												return_code = define_FE_field_at_node(temp_node,
													define_field,
													component_derivatives_data.
													components_number_of_derivatives,
													component_versions_data.components_number_of_versions,
													component_derivatives_data.
													components_nodal_value_types) &&
													MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,
														cm_node_identifier)(node,	temp_node, node_manager);
												DESTROY(FE_node)(&temp_node);
											}
											else
											{
												return_code = 0;
											}
										}
										if (!REMOVE_OBJECT_FROM_LIST(FE_node)(node,
											define_node_list))
										{
											return_code = 0;
										}
									}
								}
								else
								{
									return_code = 0;
								}
								DESTROY(LIST(FE_node))(&define_node_list);
								MANAGER_END_CACHE(FE_node)(node_manager);
							}
							else
							{
								display_message(ERROR_MESSAGE, "gfx_modify_nodes.  "
									"Derivative/version information does not match field");
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, "gfx_modify_nodes.  "
								"Could not create derivative/version information");
						}
					}
					if (undefine_field)
					{
						if (!undefine_field_at_listed_nodes(node_list, undefine_field,
							node_manager, element_manager))
						{
							return_code = 0;
						}
					}
				}
				else
				{
					if (use_data)
					{
						display_message(WARNING_MESSAGE,
							"gfx modify data:  No data specified");
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"gfx modify nodes:  No nodes specified");
					}
				}
				DESTROY(LIST(FE_node))(&node_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_nodes.  Could not make node_list");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
		DESTROY(Multi_range)(&node_ranges);
		if (define_field)
		{
			DEACCESS(FE_field)(&define_field);
		}
		if (undefine_field)
		{
			DEACCESS(FE_field)(&undefine_field);
		}
		if (component_derivatives_data.components_number_of_derivatives)
		{
			DEALLOCATE(component_derivatives_data.components_number_of_derivatives);
		}
		if (component_derivatives_data.components_nodal_value_types)
		{
			for (i = 0; i < component_derivatives_data.number_of_components; i++)
			{
				if (component_derivatives_data.components_nodal_value_types[i])
				{
					DEALLOCATE(
						component_derivatives_data.components_nodal_value_types[i]);
				}
			}
			DEALLOCATE(component_derivatives_data.components_nodal_value_types);
		}
		if (component_versions_data.components_number_of_versions)
		{
			DEALLOCATE(component_versions_data.components_number_of_versions);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_nodes */

static int execute_command_gfx_modify(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a GFX MODIFY command.
???DB.  Part of GFX EDIT ?
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_environment_map_data modify_environment_map_data;
	struct Modify_graphical_material_data modify_graphical_material_data;
	struct Modify_graphics_window_data modify_graphics_window_data;
	struct Modify_light_data modify_light_data;
	struct Modify_light_model_data modify_light_model_data;
	struct Modify_scene_data modify_scene_data;
	struct Modify_VT_volume_texture_data modify_VT_volume_texture_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_modify);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1, 
					(void *)command_data, gfx_modify_nodes);
				/* dgroup */
				Option_table_add_entry(option_table,"dgroup",(void *)1/*data*/, 
					(void *)command_data, gfx_modify_node_group);
				/* egroup */
				Option_table_add_entry(option_table,"egroup",NULL, 
					(void *)command_data, gfx_modify_element_group);
#if defined (MIRAGE)
				/* emoter */
				Option_table_add_entry(option_table,"emoter",NULL, 
					(void *)command_data->emoter_slider_dialog,
					gfx_modify_emoter);
#endif /* defined (MIRAGE) */
				/* environment_map */
				modify_environment_map_data.graphical_material_manager=
					command_data->graphical_material_manager;
				modify_environment_map_data.environment_map_manager=
					command_data->environment_map_manager;
				Option_table_add_entry(option_table,"environment_map",NULL, 
					(&modify_environment_map_data),modify_Environment_map);
				/* flow_particles */
				Option_table_add_entry(option_table,"flow_particles",NULL, 
					(void *)command_data, gfx_modify_flow_particles);
				/* g_element */
				Option_table_add_entry(option_table,"g_element",NULL, 
					(void *)command_data, gfx_modify_g_element);
				/* graphics_object */
				Option_table_add_entry(option_table,"graphics_object",NULL, 
					(void *)command_data, gfx_modify_graphics_object);
				/* light */
				modify_light_data.default_light=command_data->default_light;
				modify_light_data.light_manager=command_data->light_manager;
				Option_table_add_entry(option_table,"light",NULL, 
					(void *)(&modify_light_data), modify_Light);
				/* lmodel */
				modify_light_model_data.default_light_model=
					command_data->default_light_model;
				modify_light_model_data.light_model_manager=
					command_data->light_model_manager;
				Option_table_add_entry(option_table,"lmodel",NULL, 
					(void *)(&modify_light_model_data), modify_Light_model);
				/* material */
				modify_graphical_material_data.default_graphical_material=
					command_data->default_graphical_material;
				modify_graphical_material_data.graphical_material_manager=
					command_data->graphical_material_manager;
				modify_graphical_material_data.texture_manager=
					command_data->texture_manager;
				Option_table_add_entry(option_table,"material",NULL, 
					(void *)(&modify_graphical_material_data), modify_Graphical_material);
				/* ngroup */
				Option_table_add_entry(option_table,"ngroup",NULL, 
					(void *)command_data, gfx_modify_node_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0, 
					(void *)command_data, gfx_modify_nodes);
				/* scene */
				modify_scene_data.light_manager=command_data->light_manager;
				modify_scene_data.scene_manager=command_data->scene_manager;
				modify_scene_data.default_scene=command_data->default_scene;
				/* following used for enabling GFEs */
				modify_scene_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
				modify_scene_data.element_manager=command_data->element_manager;
				modify_scene_data.element_group_manager=
					command_data->element_group_manager;
				modify_scene_data.fe_field_manager=command_data->fe_field_manager;
				modify_scene_data.node_manager=command_data->node_manager;
				modify_scene_data.node_group_manager=command_data->node_group_manager;
				modify_scene_data.data_manager=command_data->data_manager;
				modify_scene_data.data_group_manager=command_data->data_group_manager;
				modify_scene_data.element_point_ranges_selection=
					command_data->element_point_ranges_selection;
				modify_scene_data.element_selection=command_data->element_selection;
				modify_scene_data.node_selection=command_data->node_selection;
				modify_scene_data.data_selection=command_data->data_selection;
				modify_scene_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table,"scene",NULL, 
					(void *)(&modify_scene_data), modify_Scene);
				/* spectrum */
				Option_table_add_entry(option_table,"spectrum",NULL, 
					(void *)command_data, gfx_modify_Spectrum);
				/* texture */
				Option_table_add_entry(option_table,"texture",NULL, 
					(void *)command_data, gfx_modify_Texture);
				/* vtexture */
				modify_VT_volume_texture_data.graphical_material_manager=
					command_data->graphical_material_manager;
				modify_VT_volume_texture_data.environment_map_manager=
					command_data->environment_map_manager;
				modify_VT_volume_texture_data.volume_texture_manager=
					command_data->volume_texture_manager;
				modify_VT_volume_texture_data.set_file_name_option_table=
					command_data->set_file_name_option_table;
				Option_table_add_entry(option_table,"vtexture",NULL, 
					(void *)(&modify_VT_volume_texture_data), modify_VT_volume_texture);
				/* window */
				modify_graphics_window_data.graphics_window_manager=
					command_data->graphics_window_manager;
				modify_graphics_window_data.interactive_tool_manager=
					command_data->interactive_tool_manager;
				modify_graphics_window_data.light_manager=command_data->light_manager;
				modify_graphics_window_data.light_model_manager=
					command_data->light_model_manager;
				modify_graphics_window_data.scene_manager=command_data->scene_manager;
				modify_graphics_window_data.texture_manager=
					command_data->texture_manager;
				Option_table_add_entry(option_table,"window",NULL, 
					(void *)(&modify_graphics_window_data), modify_Graphics_window);

				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx modify",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_modify.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_modify.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_modify */

#if !defined (WINDOWS_DEV_FLAG)
int gfx_movie(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 February 2000

DESCRIPTION :
???RC Movie should ACCESS the graphics window so that it cannot be closed while
movie is being created.
==============================================================================*/
{
	static char *default_movie_name="default";
	char add_frame,avi,cinepak_avi,cinepak_quicktime,*create_file_name,end,
		every_frame,force_onscreen,indeo_avi,indeo_quicktime,loop,*movie_name,
		mvc1_sgi_movie3,once,*open_file_name,play,quicktime,
		rle24_sgi_movie3,skip_frames,sgi_movie3,stop;
	double speed;
	int height, return_code, width;
	struct Option_table *option_table;
	struct Cmiss_command_data *command_data;
	struct Movie_graphics *movie;
	struct X3d_movie *x3d_movie;
	struct Graphics_window *graphics_window;

	ENTER(gfx_movie);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
#if defined (SGI_MOVIE_FILE)
			/* initialise defaults */
			if (ALLOCATE(movie_name,char,strlen(default_movie_name)+1))
			{
				strcpy(movie_name,default_movie_name);
			}
			else
			{
				movie_name=(char *)NULL;
			}
			add_frame = 0;
			avi = 0;
			cinepak_avi = 0;
			cinepak_quicktime = 0;
			create_file_name=(char *)NULL;
			end = 0;
			every_frame = 0;
			force_onscreen = 0;
			height = 0;
			indeo_avi = 0;
			indeo_quicktime = 0;
			loop = 0;
			mvc1_sgi_movie3 = 0;
			once = 0;
			open_file_name=(char *)NULL;
			play = 0;
			quicktime = 0;
			rle24_sgi_movie3 = 0;
			sgi_movie3 = 0;
			skip_frames = 0;
			speed = 0;
			stop = 0;
			width = 0;
			graphics_window=(struct Graphics_window *)NULL;

			option_table=CREATE(Option_table)();
			/* add_frame */
			Option_table_add_entry(option_table,"add_frame",&add_frame,
				NULL,set_char_flag);
			/* avi */
			Option_table_add_entry(option_table,"avi",&avi,
				NULL,set_char_flag);
			/* cinepak_avi */
			Option_table_add_entry(option_table,"cinepak_avi",&cinepak_avi,
				NULL,set_char_flag);
			/* cinepak_quicktime */
			Option_table_add_entry(option_table,"cinepak_quicktime",
				&cinepak_quicktime,NULL,set_char_flag);
			/* create */
			Option_table_add_entry(option_table,"create",&create_file_name,
				(void *)1,set_name);
			/* end */
			Option_table_add_entry(option_table,"end",&end,
				NULL,set_char_flag);
			/* every_frame */
			Option_table_add_entry(option_table,"every_frame",&every_frame,
				NULL,set_char_flag);
			/* force_onscreen */
			Option_table_add_entry(option_table,"force_onscreen",&force_onscreen,
				NULL,set_char_flag);
			/* height */
			Option_table_add_entry(option_table,"height",&height,
				NULL,set_int_non_negative);
			/* indeo_avi */
			Option_table_add_entry(option_table,"indeo_avi",&indeo_avi,
				NULL,set_char_flag);
			/* indeo_quicktime */
			Option_table_add_entry(option_table,"indeo_quicktime",&indeo_quicktime,
				NULL,set_char_flag);
			/* loop */
			Option_table_add_entry(option_table,"loop",&loop,
				NULL,set_char_flag);
			/* mvc1_sgi_movie3 */
			Option_table_add_entry(option_table,"mvc1_sgi_movie3",&mvc1_sgi_movie3,
				NULL,set_char_flag);
			/* name */
			Option_table_add_entry(option_table,"name",&movie_name,
				(void *)1,set_name);
			/* once */
			Option_table_add_entry(option_table,"once",&once,
				NULL,set_char_flag);
			/* open */
			Option_table_add_entry(option_table,"open",&open_file_name,
				(void *)1,set_name);
			/* play */
			Option_table_add_entry(option_table,"play",&play,
				NULL,set_char_flag);
			/* quicktime */
			Option_table_add_entry(option_table,"quicktime",&quicktime,
				NULL,set_char_flag);
			/* rle24_sgi_movie3 */
			Option_table_add_entry(option_table,"rle24_sgi_movie3",&rle24_sgi_movie3,
				NULL,set_char_flag);
			/* sgi_movie3 */
			Option_table_add_entry(option_table,"sgi_movie3",&sgi_movie3,
				NULL,set_char_flag);
			/* skip_frames */
			Option_table_add_entry(option_table,"skip_frames",&skip_frames,
				NULL,set_char_flag);
			/* speed */
			Option_table_add_entry(option_table,"speed",&speed,
				NULL,set_double);
			/* stop */
			Option_table_add_entry(option_table,"stop",&stop,
				NULL,set_char_flag);
			/* width */
			Option_table_add_entry(option_table,"width",&width,
				NULL,set_int_non_negative);
			/* window */
			Option_table_add_entry(option_table,"window",&graphics_window,
				command_data->graphics_window_manager,set_Graphics_window);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				movie=(struct Movie_graphics *)NULL;
				if (movie_name)
				{
					movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						movie_name,command_data->movie_graphics_manager);
				}
				if ((avi + cinepak_avi + cinepak_quicktime + indeo_quicktime + indeo_avi + 
					mvc1_sgi_movie3 + quicktime + rle24_sgi_movie3 + sgi_movie3) > 1)
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Can only specify one movie format, avi,  cinepak_quicktime, "
						"indeo_quicktime, quicktime, rle24_sgi_movie3 or sgi_movie3");
					return_code = 0;
				}
				if (open_file_name && (avi + cinepak_avi + cinepak_quicktime + indeo_avi + 
					indeo_quicktime + mvc1_sgi_movie3 + quicktime + rle24_sgi_movie3 + sgi_movie3))
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Cannot specify a format"
						" (avi, cinepak_quicktime, indeo_quicktime, quicktime or sgi_movie3) "
						"when opening an existing movie (open)");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (movie)
				{
					if (open_file_name || create_file_name)
					{
						display_message(ERROR_MESSAGE,
							"gfx_movie.  Movie %s is already open and must be ended before "
							"another opened or create",movie_name);
					}
				}
				else
				{
					if (movie_name)
					{
						if (open_file_name)
						{
							if (create_file_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_movie.  Specify only one of open and create");
							}
							else
							{
								if (!(movie=CREATE(Movie_graphics)(movie_name,open_file_name,
									X3D_MOVIE_OPEN_FILE)))
								{
									display_message(ERROR_MESSAGE,
										"gfx_movie.  Could not create movie.");
								}
							}
						}
						else
						{
							if (create_file_name)
							{
								if(avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_AVI);
								}
								else if(cinepak_avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_CINEPAK_AVI);
								}
								else if(cinepak_quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME);
								}
								else if(indeo_avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_INDEO_AVI);
								}
								else if(indeo_quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME);
								}
								else if(quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME);
								}
								else if(rle24_sgi_movie3)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3);
								}
								else if(mvc1_sgi_movie3)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3);
								}
								else
								{
									/* Default to this if no format is given */
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_SGI_MOVIE3);
								}
								if (!movie)
								{
									display_message(ERROR_MESSAGE,
										"gfx_movie.  Could not create movie.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_movie.  Need to specify 'open' or 'create' FILENAME");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_movie.  No name given for new movie");
					}
					if (movie)
					{
						if (!ADD_OBJECT_TO_MANAGER(Movie_graphics)(movie,
							command_data->movie_graphics_manager))
						{
							display_message(ERROR_MESSAGE,
								"gfx_movie.  Could not add movie to manager");
							DESTROY(Movie_graphics)(&movie);
							movie = (struct Movie_graphics *)NULL;
						}
					}
					if (movie)
					{
						/* attach the time object of the new movie to the default time
							keeper */
						Time_object_set_time_keeper(
							X3d_movie_get_time_object(Movie_graphics_get_X3d_movie(movie)),
							command_data->default_time_keeper);
					}
				}
				if (movie && (x3d_movie = Movie_graphics_get_X3d_movie(movie)))
				{
					if (graphics_window)
					{
						Movie_graphics_set_Graphics_window(movie, graphics_window);
					}
					if ( add_frame )
					{
						Movie_graphics_add_frame_to_movie(movie, width, height, force_onscreen);
					}
					if ( every_frame )
					{
						X3d_movie_set_play_every_frame(x3d_movie, 1);
					}
					if ( loop )
					{
						X3d_movie_set_play_loop(x3d_movie, 1);
					}
					if ( once )
					{
						X3d_movie_set_play_loop(x3d_movie, 0);
					}
					if ( play )
					{
						X3d_movie_play(x3d_movie);
					}
					if ( skip_frames )
					{
						X3d_movie_set_play_every_frame(x3d_movie, 0);
					}
					if ( speed != 0.0 )
					{
						X3d_movie_set_play_speed(x3d_movie, speed);
					}
					if ( stop )
					{
						X3d_movie_stop(x3d_movie);
					}
					if ( end )
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Movie_graphics)(movie,
							command_data->movie_graphics_manager);
						movie=(struct Movie_graphics *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Invalid movie object");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (graphics_window)
			{
				DEACCESS(Graphics_window)(&graphics_window);
			}
			if (create_file_name)
			{
				DEALLOCATE(create_file_name);
			}
			if (open_file_name)
			{
				DEALLOCATE(open_file_name);
			}
			if (movie_name)
			{
				DEALLOCATE(movie_name);
			}
#else /* defined (SGI_MOVIE_FILE) */
			display_message(ERROR_MESSAGE,
				"gfx_movie.  Movie extensions not available in this compilation");
			return_code=0;
#endif /* defined (SGI_MOVIE_FILE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_movie.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_movie.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_movie */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_node_tool(struct Parse_state *state,
	void *data_tool_flag,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	static char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	char *dialog_string;
	int create_enabled,define_enabled,edit_enabled,motion_update_enabled,
		return_code,select_enabled, streaming_create_enabled;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field;
	struct Node_tool *node_tool;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(execute_command_gfx_node_tool);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (data_tool_flag)
		{
			node_tool=command_data->data_tool;
			node_group_manager=command_data->data_group_manager;
		}
		else
		{
			node_tool=command_data->node_tool;
			node_group_manager=command_data->node_group_manager;
		}
		/* initialize defaults */
		node_group=(struct GROUP(FE_node) *)NULL;
		if (node_tool)
		{
			coordinate_field=Node_tool_get_coordinate_field(node_tool);
			create_enabled=Node_tool_get_create_enabled(node_tool);
			define_enabled=Node_tool_get_define_enabled(node_tool);
			edit_enabled=Node_tool_get_edit_enabled(node_tool);
			motion_update_enabled=Node_tool_get_motion_update_enabled(node_tool);
			select_enabled=Node_tool_get_select_enabled(node_tool);
			streaming_create_enabled =
				Node_tool_get_streaming_create_enabled(node_tool);
			node_group=Node_tool_get_node_group(node_tool);
		}
		else
		{
			coordinate_field=(struct Computed_field *)NULL;
			create_enabled=0;
			define_enabled=0;
			edit_enabled=0;
			motion_update_enabled=0;
			select_enabled=1;
			streaming_create_enabled = 0;
			node_group=(struct GROUP(FE_node) *)NULL;
		}
		if (coordinate_field)
		{
			ACCESS(Computed_field)(coordinate_field);
		}
		if (node_group)
		{
			ACCESS(GROUP(FE_node))(node_group);
		}

		option_table=CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate_field",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* create/no_create */
		Option_table_add_switch(option_table,"create","no_create",&create_enabled);
		/* define/no_define */
		Option_table_add_switch(option_table,"define","no_define",&define_enabled);
		/* edit/no_edit */
		Option_table_add_switch(option_table,"edit","no_edit",&edit_enabled);
		/* group */
		Option_table_add_entry(option_table,"group",&node_group,
			node_group_manager,set_FE_node_group);
		/* motion_update/no_motion_update */
		Option_table_add_switch(option_table,"motion_update","no_motion_update",
			&motion_update_enabled);
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* select/no_select */
		Option_table_add_switch(option_table,"select","no_select",&select_enabled);
		/* streaming_create/no_streaming_create */
		Option_table_add_switch(option_table, "streaming_create",
			"no_streaming_create", &streaming_create_enabled);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (node_tool)
			{
				if (dialog_string == dialog_strings[1])
				{
					Node_tool_pop_down_dialog(node_tool);
				}
				Node_tool_set_select_enabled(node_tool,select_enabled);
				Node_tool_set_edit_enabled(node_tool,edit_enabled);
				Node_tool_set_define_enabled(node_tool,define_enabled);
				Node_tool_set_coordinate_field(node_tool,coordinate_field);
				Node_tool_set_create_enabled(node_tool,create_enabled);
				Node_tool_set_node_group(node_tool,node_group);
				Node_tool_set_streaming_create_enabled(node_tool,
					streaming_create_enabled);
				Node_tool_set_motion_update_enabled(node_tool,motion_update_enabled);
				if (dialog_string == dialog_strings[0])
				{
					Node_tool_pop_up_dialog(node_tool);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_node_tool.  Missing node/data tool");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_node_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_print(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a GFX PRINT command.
==============================================================================*/
{
	char *file_name,force_onscreen_flag,postscript_flag,rgb_flag,tiff_flag;
#if ! defined (IMAGEMAGICK)
	enum Image_file_format file_format;
#endif /* ! defined (IMAGEMAGICK) */
	int height, return_code, width;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *format_option_table, *option_table;

	ENTER(execute_command_gfx_print);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			height = 0;
			force_onscreen_flag = 0;
			width = 0;
			/* default is postscript (see later) */
			postscript_flag=0;
			rgb_flag=0;
			tiff_flag=0;
			/* must have at least one graphics_window to print */
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(
				Graphics_window)((MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
					(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}

			option_table = CREATE(Option_table)();
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* force_onscreen */
			Option_table_add_entry(option_table, "force_onscreen",
				&force_onscreen_flag, NULL, set_char_flag);
			/* height */
			Option_table_add_entry(option_table, "height",
				&height, NULL, set_int_non_negative);
			/* postscript/rgb/tiff */
			format_option_table=CREATE(Option_table)();
			Option_table_add_entry(format_option_table,"postscript",
				&postscript_flag,NULL,set_char_flag);
			Option_table_add_entry(format_option_table,"rgb",
				&rgb_flag,NULL,set_char_flag);
			Option_table_add_entry(format_option_table,"tiff",
				&tiff_flag,NULL,set_char_flag);
			Option_table_add_suboption_table(option_table,format_option_table);
			/* width */
			Option_table_add_entry(option_table, "width",
				&width, NULL, set_int_non_negative);
			/* window */
			Option_table_add_entry(option_table, "window",
				&window, command_data->graphics_window_manager, set_Graphics_window);

			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(NULL,
						command_data->user_interface)))
					{
						return_code = 0;
					}					
				}
				if (!window)
				{
					display_message(ERROR_MESSAGE,"No graphics windows to print");
					return_code=0;
				}
				if (rgb_flag||postscript_flag||tiff_flag)
				{
					if (rgb_flag+postscript_flag+tiff_flag == 1)
					{
#if defined (IMAGEMAGICK)
						/* For imagemagick each type of file format does not
							have to be implemented as the format is automatically
							determined from the file suffix or prefix */
						char *extended_filename;
						if ((!strchr(file_name, ':'))&&
							(ALLOCATE(extended_filename, char, strlen(file_name) + 6)))
						{
							if (rgb_flag)
							{
								sprintf(extended_filename, "sgi:%s", file_name);
							}
							else if (postscript_flag)
							{
								sprintf(extended_filename, "ps:%s", file_name);
							}
							else
							{
								sprintf(extended_filename, "tiff:%s", file_name);
							}
							DEALLOCATE(file_name);
							file_name = extended_filename;
						}
#else /* defined (IMAGEMAGICK) */
							if (rgb_flag)
							{
								file_format = RGB_FILE_FORMAT;
							}
							else if (postscript_flag)
							{
								file_format = POSTSCRIPT_FILE_FORMAT;
							}
							else
							{
								file_format = TIFF_FILE_FORMAT;
							}						
#endif /* defined (IMAGEMAGICK) */
					}
					else
					{
						display_message(ERROR_MESSAGE,"Specify only one of postscript|rgb|tiff.");
						return_code=0;
					}
				}
#if ! defined (IMAGEMAGICK)
				else
				{
					display_message(ERROR_MESSAGE,"A file format type (postscript|rgb|tiff) is required.");
					return_code=0;
				}
#endif /* ! defined (IMAGEMAGICK) */
				if (return_code)
				{
#if defined (IMAGEMAGICK)
					write_Graphics_window_to_file(file_name,
						window, force_onscreen_flag, width, height);
#else /* defined (IMAGEMAGICK) */
					write_Graphics_window_to_file(file_name,
						window, file_format, force_onscreen_flag, width, height);
#endif /* defined (IMAGEMAGICK) */
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_print.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_print.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_print */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_read_Control_curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Reads a curve from 3 files: ~.curve.com, ~.curve.exnode, ~.curve.exelem, where
~ is the name of the curve/file specified here.
Works by executing the .curve.com file, which should have a gfx define curve
instruction to read in the mesh.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_Control_curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".curve.com",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					return_code=check_suffix(&file_name,".curve.com")&&
						execute_comfile(file_name,command_data->execute_command);
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_Control_curve.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_Control_curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_Control_curve */

static int gfx_read_data(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
If a data file is not specified a file selection box is presented to the user,
otherwise the data file is read.
???DB.  Almost identical to gfx_read_nodes.  Could set up struct Read_nodes_data
	to combine, but will probably be adding ipdata format
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct File_read_FE_node_group_data data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_data);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.fe_field_manager=command_data->fe_field_manager;
				data.element_group_manager=command_data->element_group_manager;
				/*???RC note swapping node and data manager stuff - extends to
				  also creating a node_group for the new data group! */
				data.node_manager=command_data->data_manager;
				data.element_manager=command_data->element_manager;
				data.node_group_manager=command_data->data_group_manager;
				data.data_group_manager=command_data->node_group_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exdata",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exdata"))
					{
						return_code=file_read_FE_node_group(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_data.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_data.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_data */

static int gfx_read_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the elements file is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct File_read_FE_element_group_data data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.element_manager=command_data->element_manager;
				data.element_group_manager=command_data->element_group_manager;
				data.fe_field_manager=command_data->fe_field_manager;
				data.node_manager=command_data->node_manager;
				data.node_group_manager=command_data->node_group_manager;
				data.data_group_manager=command_data->data_group_manager;
				data.basis_manager=command_data->basis_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exelem",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exelem"))
					{
						return_code=file_read_FE_element_group(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_elements.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_elements.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_elements */

static int gfx_read_nodes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct File_read_FE_node_group_data data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_nodes);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.fe_field_manager=command_data->fe_field_manager;
				data.element_group_manager=command_data->element_group_manager;
				data.node_manager=command_data->node_manager;
				data.element_manager=command_data->element_manager;
				data.node_group_manager=command_data->node_group_manager;
				data.data_group_manager=command_data->data_group_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exnode",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exnode"))
					{
						return_code=file_read_FE_node_group(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_nodes */

static int gfx_read_objects(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the file of graphics objects is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct File_read_graphics_object_data data;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;

	ENTER(gfx_read_objects);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_multiple_options(state,
				command_data->set_file_name_option_table))
			{
				data.object_list=command_data->graphics_object_list;
				data.graphical_material_manager=
					command_data->graphical_material_manager;
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exgobj",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exgobj"))
					{
						return_code=file_read_graphics_objects(file_name,(void *)&data);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_objects.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_objects.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_objects */

static int gfx_read_wavefront_obj(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the wavefront obj file is read.
==============================================================================*/
{
	char *file_name, *graphics_object_name,*render_type_string,**valid_strings;
	enum Render_type render_type;
	float time;
	int number_of_valid_strings, return_code;
	struct File_read_graphics_object_from_obj_data data;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_wavefront_obj);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void))
		{
			graphics_object_name=(char *)NULL;
			time = 0;
			file_name=(char *)NULL;

			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			  &file_name, NULL,set_file_name);
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* render_type */
			render_type = RENDER_TYPE_SHADED;
			render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);

			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				data.object_list=command_data->graphics_object_list;
				data.graphical_material_manager=
					command_data->graphical_material_manager;
				STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
				data.render_type = render_type;
				data.time = time;
				if(graphics_object_name)
				{
					data.graphics_object_name = graphics_object_name;
				}
				else
				{
					data.graphics_object_name = file_name;
				}
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".obj",
						command_data->user_interface)))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".obj"))
					{
						return_code=file_read_voltex_graphics_object_from_obj(file_name,(void *)&data);
					}
				}
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (graphics_object_name)
			{
				DEALLOCATE(graphics_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_wavefront_obj.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_wavefront_obj.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_wavefront_obj */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Executes a GFX READ command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"curve",NULL,NULL,gfx_read_Control_curve},
		{"data",NULL,NULL,gfx_read_data},
#if !defined (WINDOWS_DEV_FLAG)
		{"elements",NULL,NULL,gfx_read_elements},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{"nodes",NULL,NULL,gfx_read_nodes},
#if !defined (WINDOWS_DEV_FLAG)
		{"objects",NULL,NULL,gfx_read_objects},
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if !defined (WINDOWS_DEV_FLAG)
		{"wavefront_obj",NULL,NULL,gfx_read_wavefront_obj},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_read);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			i=0;
			(option_table[i]).user_data=command_data_void;
			i++;
			(option_table[i]).user_data=command_data_void;
			i++;
#if !defined (WINDOWS_DEV_FLAG)
			(option_table[i]).user_data=command_data_void;
			i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
			(option_table[i]).user_data=command_data_void;
			i++;
#if !defined (WINDOWS_DEV_FLAG)
			(option_table[i]).user_data=command_data_void;
			i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if !defined (WINDOWS_DEV_FLAG)
			(option_table[i]).user_data=command_data_void;
			i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
			return_code=process_option(state,option_table);
		}
		else
		{
			set_command_prompt("gfx read",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_read.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_read */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_select(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Executes a GFX SELECT command.
==============================================================================*/
{
	char *ranges_string;
	int i,j,number_of_ranges,number_selected,return_code,start,stop,
		total_number_in_ranges;
	struct CM_element_information cm;
	struct CM_element_type_Multi_range_data element_type_ranges_data;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element *element;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_node *node;
	struct Multi_range *data_ranges, *element_ranges, *face_ranges,
		*grid_point_ranges, *line_ranges, *multi_range, *node_ranges;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_grid_field_data;

	ENTER(execute_command_gfx_select);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_ranges=CREATE(Multi_range)();
			element_ranges=CREATE(Multi_range)();
			face_ranges=CREATE(Multi_range)();
			grid_point_ranges=CREATE(Multi_range)();
			line_ranges=CREATE(Multi_range)();
			node_ranges=CREATE(Multi_range)();
			if ((grid_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
				"grid_point_number",command_data->fe_field_manager))&&
				FE_field_is_1_component_integer(grid_field,(void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field=(struct FE_field *)NULL;
			}
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"data",data_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"elements",element_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"faces",face_ranges,
				(void *)NULL,set_Multi_range);
			set_grid_field_data.fe_field_manager=command_data->fe_field_manager;
			set_grid_field_data.conditional_function=FE_field_is_1_component_integer;
			set_grid_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"grid_field",
				&grid_field,&set_grid_field_data,set_FE_field_conditional);
			Option_table_add_entry(option_table,"grid_points",grid_point_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"lines",line_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"nodes",node_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)command_data->element_manager,set_Element_point_ranges);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				/* data */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(data_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->data_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(data_ranges);
						/* take numbers not in the manager away from data_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->data_manager))
						{
							Multi_range_intersect(data_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->data_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(data_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(data_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->data_manager))
							{
								if (FE_node_selection_select_node(
									command_data->data_selection,node))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(data_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d data points selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->data_selection);
				}
				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_select_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(element_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager) <
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string = Multi_range_get_ranges_string(element_ranges);
						/* take numbers not in the manager away from element_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						element_type_ranges_data.cm_element_type = CM_ELEMENT;
						element_type_ranges_data.multi_range = CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_of_CM_element_type_add_number_to_Multi_range,
							(void *)&element_type_ranges_data, command_data->element_manager))
						{
							Multi_range_intersect(element_ranges,
								element_type_ranges_data.multi_range);
						}
						DESTROY(Multi_range)(&(element_type_ranges_data.multi_range));
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(element_ranges);
					cm.type=CM_ELEMENT;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(element_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								if (FE_element_selection_select_element(
									command_data->element_selection,element))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(element_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d element(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* faces */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(face_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(face_ranges);
						/* take numbers not in the manager away from face_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						element_type_ranges_data.cm_element_type = CM_FACE;
						element_type_ranges_data.multi_range = CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_of_CM_element_type_add_number_to_Multi_range,
							(void *)&element_type_ranges_data, command_data->element_manager))
						{
							Multi_range_intersect(face_ranges,
								element_type_ranges_data.multi_range);
						}
						DESTROY(Multi_range)(&(element_type_ranges_data.multi_range));
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(face_ranges);
					cm.type=CM_FACE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(face_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								if (FE_element_selection_select_element(
									command_data->element_selection,element))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(face_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d face(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* grid_points */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(grid_point_ranges)))
				{
					if (grid_field)
					{
						if (grid_to_list_data.element_point_ranges_list=
							CREATE(LIST(Element_point_ranges))())
						{
							grid_to_list_data.grid_fe_field=grid_field;
							grid_to_list_data.grid_value_ranges=grid_point_ranges;
							/* inefficient: go through every element in manager */
							FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								FE_element_grid_to_Element_point_ranges_list,
								(void *)&grid_to_list_data,command_data->element_manager);
							if (0<NUMBER_IN_LIST(Element_point_ranges)(
								grid_to_list_data.element_point_ranges_list))
							{
								Element_point_ranges_selection_begin_cache(
									command_data->element_point_ranges_selection);
								FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
									Element_point_ranges_select,
									(void *)command_data->element_point_ranges_selection,
									grid_to_list_data.element_point_ranges_list);
								Element_point_ranges_selection_end_cache(
									command_data->element_point_ranges_selection);
							}
							DESTROY(LIST(Element_point_ranges))(
								&(grid_to_list_data.element_point_ranges_list));
						}
						else
						{
							display_message(ERROR_MESSAGE,"execute_command_gfx_select.  "
								"Could not create grid_point list");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"To select grid_points, "
							"need integer grid_field (eg. grid_point_number)");
					}
				}
				/* lines */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(line_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(line_ranges);
						/* take numbers not in the manager away from line_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						element_type_ranges_data.cm_element_type = CM_LINE;
						element_type_ranges_data.multi_range = CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_of_CM_element_type_add_number_to_Multi_range,
							(void *)&element_type_ranges_data, command_data->element_manager))
						{
							Multi_range_intersect(line_ranges,
								element_type_ranges_data.multi_range);
						}
						DESTROY(Multi_range)(&(element_type_ranges_data.multi_range));
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(line_ranges);
					cm.type=CM_LINE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(line_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								if (FE_element_selection_select_element(
									command_data->element_selection,element))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(line_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d line(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* nodes */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(node_ranges)))
				{
					number_selected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->node_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(node_ranges);
						/* take numbers not in the manager away from node_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->node_manager))
						{
							Multi_range_intersect(node_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->node_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(node_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(node_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->node_manager))
							{
								if (FE_node_selection_select_node(
									command_data->node_selection,node))
								{
									number_selected++;
								}
							}
						}
					}
					if (number_selected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(node_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d node(s) selected from %s",number_selected,ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->node_selection);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&node_ranges);
			DESTROY(Multi_range)(&line_ranges);
			DESTROY(Multi_range)(&grid_point_ranges);
			DESTROY(Multi_range)(&face_ranges);
			DESTROY(Multi_range)(&element_ranges);
			DESTROY(Multi_range)(&data_ranges);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx select",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_select.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_select */

static int execute_command_gfx_unselect(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Executes a GFX UNSELECT command.
==============================================================================*/
{
	char *ranges_string;
	int i,j,number_of_ranges,number_unselected,return_code,start,stop,
		total_number_in_ranges;
	struct CM_element_information cm;
	struct CM_element_type_Multi_range_data element_type_ranges_data;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element *element;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_node *node;
	struct Multi_range *data_ranges, *element_ranges, *face_ranges,
		*grid_point_ranges, *line_ranges, *multi_range, *node_ranges;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_data set_grid_field_data;

	ENTER(execute_command_gfx_unselect);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_ranges=CREATE(Multi_range)();
			element_ranges=CREATE(Multi_range)();
			face_ranges=CREATE(Multi_range)();
			grid_point_ranges=CREATE(Multi_range)();
			line_ranges=CREATE(Multi_range)();
			node_ranges=CREATE(Multi_range)();
			if ((grid_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
				"grid_point_number",command_data->fe_field_manager))&&
				FE_field_is_1_component_integer(grid_field,(void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field=(struct FE_field *)NULL;
			}
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"data",data_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"elements",element_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"faces",face_ranges,
				(void *)NULL,set_Multi_range);
			set_grid_field_data.fe_field_manager=command_data->fe_field_manager;
			set_grid_field_data.conditional_function=FE_field_is_1_component_integer;
			set_grid_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"grid_field",
				&grid_field,&set_grid_field_data,set_FE_field_conditional);
			Option_table_add_entry(option_table,"grid_points",grid_point_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"lines",line_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"nodes",node_ranges,
				(void *)NULL,set_Multi_range);
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)command_data->element_manager,set_Element_point_ranges);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				/* data */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(data_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->data_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(data_ranges);
						/* take numbers not in the manager away from data_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->data_manager))
						{
							Multi_range_intersect(data_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->data_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(data_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(data_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->data_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_node_selection_is_node_selected(
									command_data->data_selection,node)&&
									FE_node_selection_unselect_node(
										command_data->data_selection,node))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(data_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d data unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->data_selection);
				}

				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_unselect_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(element_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(element_ranges);
						/* take numbers not in the manager away from element_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						element_type_ranges_data.cm_element_type = CM_ELEMENT;
						element_type_ranges_data.multi_range = CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_of_CM_element_type_add_number_to_Multi_range,
							(void *)&element_type_ranges_data, command_data->element_manager))
						{
							Multi_range_intersect(element_ranges,
								element_type_ranges_data.multi_range);
						}
						DESTROY(Multi_range)(&(element_type_ranges_data.multi_range));
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(element_ranges);
					cm.type=CM_ELEMENT;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(element_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_element_selection_is_element_selected(
									command_data->element_selection,element)&&
									FE_element_selection_unselect_element(
										command_data->element_selection,element))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(element_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d element(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* faces */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(face_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(face_ranges);
						/* take numbers not in the manager away from face_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						element_type_ranges_data.cm_element_type = CM_FACE;
						element_type_ranges_data.multi_range = CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_of_CM_element_type_add_number_to_Multi_range,
							(void *)&element_type_ranges_data, command_data->element_manager))
						{
							Multi_range_intersect(face_ranges,
								element_type_ranges_data.multi_range);
						}
						DESTROY(Multi_range)(&(element_type_ranges_data.multi_range));
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(face_ranges);
					cm.type=CM_FACE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(face_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_element_selection_is_element_selected(
									command_data->element_selection,element)&&
									FE_element_selection_unselect_element(
										command_data->element_selection,element))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(face_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d face(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* grid_points */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(grid_point_ranges)))
				{
					if (grid_field)
					{
						if (grid_to_list_data.element_point_ranges_list=
							CREATE(LIST(Element_point_ranges))())
						{
							grid_to_list_data.grid_fe_field=grid_field;
							grid_to_list_data.grid_value_ranges=grid_point_ranges;
							/* inefficient: go through every element in manager */
							FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
								FE_element_grid_to_Element_point_ranges_list,
								(void *)&grid_to_list_data,command_data->element_manager);
							if (0<NUMBER_IN_LIST(Element_point_ranges)(
								grid_to_list_data.element_point_ranges_list))
							{
								Element_point_ranges_selection_begin_cache(
									command_data->element_point_ranges_selection);
								FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
									Element_point_ranges_unselect,
									(void *)command_data->element_point_ranges_selection,
									grid_to_list_data.element_point_ranges_list);
								Element_point_ranges_selection_end_cache(
									command_data->element_point_ranges_selection);
							}
							DESTROY(LIST(Element_point_ranges))(
								&(grid_to_list_data.element_point_ranges_list));
						}
						else
						{
							display_message(ERROR_MESSAGE,"execute_command_gfx_unselect.  "
								"Could not create grid_point list");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"To unselect grid_points, "
							"need integer grid_field (eg. grid_point_number)");
					}
				}
				/* lines */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(line_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_element)(command_data->element_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(line_ranges);
						/* take numbers not in the manager away from line_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						element_type_ranges_data.cm_element_type = CM_LINE;
						element_type_ranges_data.multi_range = CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
							FE_element_of_CM_element_type_add_number_to_Multi_range,
							(void *)&element_type_ranges_data, command_data->element_manager))
						{
							Multi_range_intersect(line_ranges,
								element_type_ranges_data.multi_range);
						}
						DESTROY(Multi_range)(&(element_type_ranges_data.multi_range));
					}
					FE_element_selection_begin_cache(command_data->element_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(line_ranges);
					cm.type=CM_LINE;
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(line_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							cm.number=j;
							if (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&cm,command_data->element_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_element_selection_is_element_selected(
									command_data->element_selection,element)&&
									FE_element_selection_unselect_element(
										command_data->element_selection,element))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(line_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d line(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_element_selection_end_cache(command_data->element_selection);
				}
				/* nodes */
				if (0<(total_number_in_ranges=
					Multi_range_get_total_number_in_ranges(node_ranges)))
				{
					number_unselected=0;
					ranges_string=(char *)NULL;
					if (NUMBER_IN_MANAGER(FE_node)(command_data->node_manager)<
						total_number_in_ranges)
					{
						/* get ranges_string for later warning since modifying ranges */
						ranges_string=Multi_range_get_ranges_string(node_ranges);
						/* take numbers not in the manager away from node_ranges to avoid
							 excess computation if, say, 1..1000000 entered */
						multi_range=CREATE(Multi_range)();
						if (FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
							add_FE_node_number_to_Multi_range,(void *)multi_range,
							command_data->node_manager))
						{
							Multi_range_intersect(node_ranges,multi_range);
						}
						DESTROY(Multi_range)(&multi_range);
					}
					FE_node_selection_begin_cache(command_data->node_selection);
					number_of_ranges=Multi_range_get_number_of_ranges(node_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						Multi_range_get_range(node_ranges,i,&start,&stop);
						for (j=start;j<=stop;j++)
						{
							if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)(j,command_data->node_manager))
							{
								/* only unselect those that are currently selected */
								if (FE_node_selection_is_node_selected(
									command_data->node_selection,node)&&
									FE_node_selection_unselect_node(
										command_data->node_selection,node))
								{
									number_unselected++;
								}
							}
						}
					}
					if (number_unselected < total_number_in_ranges)
					{
						if (!ranges_string)
						{
							ranges_string=Multi_range_get_ranges_string(node_ranges);
						}
						display_message(WARNING_MESSAGE,
							"%d node(s) unselected from %s",number_unselected,
							ranges_string);
						DEALLOCATE(ranges_string);
					}
					FE_node_selection_end_cache(command_data->node_selection);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&node_ranges);
			DESTROY(Multi_range)(&line_ranges);
			DESTROY(Multi_range)(&grid_point_ranges);
			DESTROY(Multi_range)(&face_ranges);
			DESTROY(Multi_range)(&element_ranges);
			DESTROY(Multi_range)(&data_ranges);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx unselect",command_data->command_window);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_unselect.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_unselect */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_length(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Sets the axis length from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_special_float3}
	};
	Triple axis_lengths;

	ENTER(gfx_set_axis_length);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			scene = command_data->default_scene;
			ACCESS(Scene)(scene);
			Scene_get_axis_lengths(scene, axis_lengths);
			(option_table[0]).to_be_modified = &scene;
			(option_table[0]).user_data = command_data->scene_manager;
			(option_table[1]).to_be_modified = axis_lengths;
			(option_table[1]).user_data = "*";
			if (return_code = process_multiple_options(state, option_table))
			{
				return_code = Scene_set_axis_lengths(scene, axis_lengths);
			}
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_length.  Missing command_data");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_length.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_length */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Sets the axis material from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *axis_material;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_Graphical_material}
	};

	ENTER(gfx_set_axis_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			scene=command_data->default_scene;
			ACCESS(Scene)(scene);
			axis_material=Scene_get_axis_material(scene);
			ACCESS(Graphical_material)(axis_material);
			(option_table[0]).to_be_modified = &scene;
			(option_table[0]).user_data = command_data->scene_manager;
			(option_table[1]).to_be_modified = &axis_material;
			(option_table[1]).user_data = command_data->graphical_material_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				Scene_set_axis_material(scene, axis_material);
			}
			DEACCESS(Graphical_material)(&axis_material);
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_material.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_material */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_axis_origin(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the axis origin of a scene from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	Triple axis_origin;

	ENTER(gfx_set_axis_origin);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
			scene=command_data->default_scene;
			ACCESS(Scene)(scene);
			Scene_get_axis_origin(scene,axis_origin);
			position.data[0]=axis_origin[0];
			position.data[1]=axis_origin[1];
			position.data[2]=axis_origin[2];
			(option_table[0]).to_be_modified= &coordinate;
			(option_table[1]).to_be_modified= &position;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &position;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* we have a rc position, but relative to a coordinate system we
					 must convert to global */
				get_global_position(&position,coordinate,&global_position);
				axis_origin[0]=global_position.data[0];
				axis_origin[1]=global_position.data[1];
				axis_origin[2]=global_position.data[2];
				Scene_set_axis_origin(scene,axis_origin);
			}
			DEACCESS(Cmgui_coordinate)(&coordinate);
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_axis_origin.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_axis_origin.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_axis_origin */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_background(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets the background colour and/or texture from the command line.
???RC Obsolete.
==============================================================================*/
{
	int return_code,update;
	struct Cmiss_command_data *command_data;
	struct Colour background_colour,save_background_colour;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	struct Texture *background_texture,*save_background_texture;
	static struct Modifier_entry option_table[]=
	{
		{"texture",NULL,NULL,set_Texture},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Colour}
	};

	ENTER(gfx_set_background);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/*???Debug */
			display_message(WARNING_MESSAGE,
				"gfx set background is obsolete, please use gfx modify window");
			if ((window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))&&
				(scene_viewer=Graphics_window_get_Scene_viewer(window,0)))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_background_colour(scene_viewer,
					&background_colour);
				if (background_texture=Scene_viewer_get_background_texture(
					scene_viewer))
				{
					ACCESS(Texture)(background_texture);
				}
			}
			else
			{
				background_colour.red=0.0;
				background_colour.green=0.0;
				background_colour.blue=0.0;
				background_texture=(struct Texture *)NULL;
				return_code=1;
			}
			if (return_code)
			{
				save_background_colour.red=background_colour.red;
				save_background_colour.green=background_colour.green;
				save_background_colour.blue=background_colour.blue;
				save_background_texture=background_texture;
				/* initialise defaults */
				(option_table[0]).to_be_modified= &background_texture;
				(option_table[0]).user_data=(void *)(command_data->texture_manager);
				(option_table[1]).to_be_modified= &window;
				(option_table[1]).user_data=command_data->graphics_window_manager;
				(option_table[2]).to_be_modified= &background_colour;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						if (scene_viewer=Graphics_window_get_Scene_viewer(window,0))
						{
							update=0;
							if ((save_background_colour.red!=background_colour.red)||
								(save_background_colour.green!=background_colour.green)||
								(save_background_colour.blue!=background_colour.blue))
							{
								update=1;
								return_code=Scene_viewer_set_background_colour(scene_viewer,
									&background_colour);
							}
							if (return_code&&(save_background_texture!=background_texture))
							{
								update=1;
								return_code=Scene_viewer_set_background_texture(scene_viewer,
									background_texture);
							}
							if (return_code&&update)
							{
								return_code=Graphics_window_update_now(window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_set_background.  Could not get scene viewer 1");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_set_background.  Could not get background information");
				return_code=0;
			}
			if (background_texture)
			{
				DEACCESS(Texture)(&background_texture);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_background.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_background.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_background */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_far_clipping_plane(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the distance to the far clipping plane from the command line.
==============================================================================*/
{
	double bottom,default_far,far,left,near,right,top;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,set_double}
	};

	ENTER(gfx_set_far_clipping_plane);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if ((window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))&&
				(scene_viewer=Graphics_window_get_Scene_viewer(window,0)))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&near,&default_far);
			}
			else
			{
				left=right=bottom=top=near=default_far=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				(option_table[0]).to_be_modified= &window;
				(option_table[0]).user_data=command_data->graphics_window_manager;
				(option_table[1]).to_be_modified= &default_far;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						if ((scene_viewer=Graphics_window_get_Scene_viewer(window,0))&&
							Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,&bottom,
							&top,&near,&far)&&(default_far!=far))
							/*???DB.  Should not be changing directly because managed ? */
						{
							return_code=Scene_viewer_set_viewing_volume(scene_viewer,left,
								right,bottom,top,near,default_far);
							return_code=Graphics_window_update_now(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"No windows are available");
						return_code=0;
					}
				} /* parse error, help */
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_far_clipping_plane`.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_far_clipping_plane.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_far_clipping_plane */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
int set_graphics_window_resolution(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Modifier function to set the resolution of a graphics window from a command.
==============================================================================*/
{
	int height,old_height,old_width,return_code,width;
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;
	static struct Modifier_entry option_table[]=
	{
		{"height",NULL,NULL,set_int_positive},
		{"width",NULL,NULL,set_int_positive},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(set_graphics_window_resolution);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphics_window_manager=(struct MANAGER(Graphics_window) *)
			graphics_window_manager_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				Graphics_window_get_viewing_area_size(window,&width,&height);
				old_width=width;
				old_height=height;
			}
			else
			{
				height=0;
				width=0;
			}
			(option_table[0]).to_be_modified= &height;
			(option_table[1]).to_be_modified= &width;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data=graphics_window_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (window)
				{
					if ((width != old_width)||(height != old_height))
					{
						return_code=
							Graphics_window_set_viewing_area_size(window,width,height);
					}
					else
					{
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_graphics_window_resolution.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_graphics_window_resolution.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_graphics_window_resolution */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_interest_point(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the current interest point from the command line. The view_point is also
adjusted so that the view direction and distance from it to the interest point
remains the same. The up-vector therefore does not change.
==============================================================================*/
{
	double eye[3],lookat[3],up[3],view[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	struct Graphics_window *window;

	ENTER(gfx_set_interest_point);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_lookat_parameters(
					Graphics_window_get_Scene_viewer(window,0),&eye[0],&eye[1],&eye[2],
					&lookat[0],&lookat[1],&lookat[2],&up[0],&up[1],&up[2]);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
				position.data[0]=lookat[0];
				position.data[1]=lookat[1];
				position.data[2]=lookat[2];
				(option_table[0]).to_be_modified= &coordinate;
				(option_table[1]).to_be_modified= &position;
				(option_table[2]).to_be_modified= &window;
				(option_table[2]).user_data=command_data->graphics_window_manager;
				(option_table[3]).to_be_modified= &position;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						Scene_viewer_get_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),
							&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
							&up[0],&up[1],&up[2]);
						/* get the view direction/distance which will not change */
						view[0]=lookat[0]-eye[0];
						view[1]=lookat[1]-eye[1];
						view[2]=lookat[2]-eye[2];
						/* we have a rc position, but relative to a coordinate system we
							 must convert to global */
						get_global_position(&position,coordinate,&global_position);
						lookat[0]=global_position.data[0];
						lookat[1]=global_position.data[1];
						lookat[2]=global_position.data[2];
						/* make sure the view_point is still the same direction and distance
							 from the interest point. */
						eye[0]=lookat[0]-view[0];
						eye[1]=lookat[1]-view[1];
						eye[2]=lookat[2]-view[2];
						if (return_code=Scene_viewer_set_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),eye[0],eye[1],eye[2],
							lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]))
							/*???DB.  Should not be changing directly because managed ? */
						{
							return_code=Graphics_window_update_now(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_interest_point.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_interest_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_interest_point */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_near_clipping_plane(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the distance to the near clipping plane from the command line.
==============================================================================*/
{
	double bottom,default_near,far,left,near,right,top;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,set_double}
	};

	ENTER(gfx_set_near_clipping_plane);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if ((window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))&&
				(scene_viewer=Graphics_window_get_Scene_viewer(window,0)))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&default_near,&far);
			}
			else
			{
				left=right=bottom=top=default_near=far=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				(option_table[0]).to_be_modified= &window;
				(option_table[0]).user_data=command_data->graphics_window_manager;
				(option_table[1]).to_be_modified= &default_near;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						if ((scene_viewer=Graphics_window_get_Scene_viewer(window,0))&&
							Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,&bottom,
							&top,&near,&far)&&(default_near!=near))
							/*???DB.  Should not be changing directly because managed ? */
						{
							return_code=Scene_viewer_set_viewing_volume(scene_viewer,left,
								right,bottom,top,default_near,far);
							return_code=Graphics_window_update_now(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"No windows are available");
						return_code=0;
					}
				} /* parse error, help */
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_near_clipping_plane`.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_near_clipping_plane.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_near_clipping_plane */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_set_FE_nodal_value(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets nodal field values from a command.
???DB.  Should it be here ?
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type fe_nodal_d_ds1,fe_nodal_d_ds2,fe_nodal_d_ds3,
		fe_nodal_d2_ds1ds2,fe_nodal_d2_ds1ds3,fe_nodal_d2_ds2ds3,
		fe_nodal_d3_ds1ds2ds3,fe_nodal_value,value_type;
	FE_value value;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"value",NULL,NULL,set_enum},
		{"d/ds1",NULL,NULL,set_enum},
		{"d/ds2",NULL,NULL,set_enum},
		{"d/ds3",NULL,NULL,set_enum},
		{"d2/ds1ds2",NULL,NULL,set_enum},
		{"d2/ds1ds3",NULL,NULL,set_enum},
		{"d2/ds2ds3",NULL,NULL,set_enum},
		{"d3/ds1ds2ds3",NULL,NULL,set_enum},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct FE_field_component component;
	struct FE_node *node,*node_copy;

	ENTER(gfx_set_FE_nodal_value);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		node=(struct FE_node *)NULL;
		if (return_code=set_FE_node(state,(void *)&node,
			(void *)(command_data->node_manager)))
		{
			component.field=(struct FE_field *)NULL;
			component.number=0;
			if (return_code=set_FE_field_component(state,(void *)&component,
				(void *)(command_data->fe_field_manager)))
			{
				value_type=FE_NODAL_UNKNOWN;
				option_table[0].to_be_modified= &value_type;
				fe_nodal_value=FE_NODAL_VALUE;
				option_table[0].user_data= &fe_nodal_value;
				option_table[1].to_be_modified= &value_type;
				fe_nodal_d_ds1=FE_NODAL_D_DS1;
				option_table[1].user_data= &fe_nodal_d_ds1;
				option_table[2].to_be_modified= &value_type;
				fe_nodal_d_ds2=FE_NODAL_D_DS2;
				option_table[2].user_data= &fe_nodal_d_ds2;
				option_table[3].to_be_modified= &value_type;
				fe_nodal_d_ds3=FE_NODAL_D_DS3;
				option_table[3].user_data= &fe_nodal_d_ds3;
				option_table[4].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds2=FE_NODAL_D2_DS1DS2;
				option_table[4].user_data= &fe_nodal_d2_ds1ds2;
				option_table[5].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds3=FE_NODAL_D2_DS1DS3;
				option_table[5].user_data= &fe_nodal_d2_ds1ds3;
				option_table[6].to_be_modified= &value_type;
				fe_nodal_d2_ds2ds3=FE_NODAL_D2_DS2DS3;
				option_table[6].user_data= &fe_nodal_d2_ds2ds3;
				option_table[7].to_be_modified= &value_type;
				fe_nodal_d3_ds1ds2ds3=FE_NODAL_D3_DS1DS2DS3;
				option_table[7].user_data= &fe_nodal_d3_ds1ds2ds3;
				if (return_code=process_option(state,option_table))
				{
					if (current_token=state->current_token)
					{
						if (1==sscanf(current_token,FE_VALUE_INPUT_STRING,&value))
						{
							if ((node_copy=CREATE(FE_node)(0,(struct FE_node *)NULL))
								&&COPY(FE_node)(node_copy,node))
							{
								if (return_code=set_FE_nodal_FE_value_value(node_copy,
									&component,0,value_type,value))
								{
									return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,
										cm_node_identifier)(node,node_copy,command_data->node_manager);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_set_FE_nodal_value.  Could not duplicate node");
								return_code=0;
							}
							DESTROY(FE_node)(&node_copy);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid nodal value %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing value for node");
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					if ((current_token=state->current_token)&&
						!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						display_message(INFORMATION_MESSAGE," #\n");
						return_code=1;
					}
				}
			}
			DEACCESS(FE_node)(&node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_FE_nodal_value.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_FE_nodal_value */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_scene_order(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the ordering of graphics objects on scene(s) from the command line.
==============================================================================*/
{
	char *name;
	int return_code,position;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"object",NULL,(void *)1,set_name},
		{"position",NULL,NULL,set_int},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_set_scene_order);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			position=0;
			scene=command_data->default_scene;
			(option_table[0]).to_be_modified= &name;
			(option_table[1]).to_be_modified= &position;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name)
				{
					if (scene_object=Scene_get_Scene_object_by_name(scene,name))
					{
						return_code=Scene_set_scene_object_position(scene,scene_object,
							position);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No graphics object named '%s' in scene",name);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
					return_code=0;
				}
			} /* parse error,help */
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_scene_order.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_scene_order.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_scene_order */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_time(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the time from the command line.
==============================================================================*/
{
	char *timekeeper_name;
	float time;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"timekeeper",NULL,NULL,set_name},
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_time);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			ALLOCATE(timekeeper_name,char,10);
			/* there is only a default timekeeper at the moment but I am making the
				commands with a timekeeper manager in mind */
			strcpy(timekeeper_name,"default");
			if (command_data->default_time_keeper)
			{
				time=Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0;
			}
			(option_table[0]).to_be_modified= &timekeeper_name;
			(option_table[1]).to_be_modified= &time;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* the old routine only use to call this if the time wasn't the
					same as the default time, but the timekeeper might not be the
					default */
				Time_keeper_request_new_time(command_data->default_time_keeper,time);
			} /* parse error, help */
			DEALLOCATE(timekeeper_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_time.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_time.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_time */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int set_transformation_matrix(struct Parse_state *state,
	void *transformation_matrix_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets a transformation matrix from the command line.
==============================================================================*/
{
	char *current_token;
	gtMatrix *transformation_matrix;
	int i,j,return_code;

	ENTER(set_transformation_matrix);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			display_message(INFORMATION_MESSAGE,"# # # # # # # # # # # # # # # #");
			if (transformation_matrix=(gtMatrix *)transformation_matrix_void)
			{
				display_message(INFORMATION_MESSAGE,
					" [%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g]",
					(*transformation_matrix)[0][0],(*transformation_matrix)[0][1],
					(*transformation_matrix)[0][2],(*transformation_matrix)[0][3],
					(*transformation_matrix)[1][0],(*transformation_matrix)[1][1],
					(*transformation_matrix)[1][2],(*transformation_matrix)[1][3],
					(*transformation_matrix)[2][0],(*transformation_matrix)[2][1],
					(*transformation_matrix)[2][2],(*transformation_matrix)[2][3],
					(*transformation_matrix)[3][0],(*transformation_matrix)[3][1],
					(*transformation_matrix)[3][2],(*transformation_matrix)[3][3]);
			}
			return_code=1;
		}
		else
		{
			if (transformation_matrix=(gtMatrix *)transformation_matrix_void)
			{
				return_code=1;
				i=0;
				while ((i<4)&&return_code&&current_token)
				{
					j=0;
					while ((j<4)&&return_code&&current_token)
					{
						if (1==sscanf(current_token,"%f",&((*transformation_matrix)[i][j])))
						{
							shift_Parse_state(state,1);
							current_token=state->current_token;
						}
						else
						{
							return_code=0;
						}
						j++;
					}
					i++;
				}
				if (!return_code||(i<4)||(j<4))
				{
					if (current_token)
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix: %s",current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix");
					}
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_transformation_matrix.  Missing transformation_matrix");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_transformation_matrix.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_transformation_matrix */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets the transformation for a graphics object from the command line.
==============================================================================*/
{
	char *scene_object_name;
	gtMatrix transformation_matrix;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_transformation_matrix}
	};

	ENTER(gfx_set_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			scene_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			transformation_matrix[0][0]=1;
			transformation_matrix[0][1]=0;
			transformation_matrix[0][2]=0;
			transformation_matrix[0][3]=0;
			transformation_matrix[1][0]=0;
			transformation_matrix[1][1]=1;
			transformation_matrix[1][2]=0;
			transformation_matrix[1][3]=0;
			transformation_matrix[2][0]=0;
			transformation_matrix[2][1]=0;
			transformation_matrix[2][2]=1;
			transformation_matrix[2][3]=0;
			transformation_matrix[3][0]=0;
			transformation_matrix[3][1]=0;
			transformation_matrix[3][2]=0;
			transformation_matrix[3][3]=1;
			/* parse the command line */
			(option_table[0]).to_be_modified= &scene_object_name;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			(option_table[2]).to_be_modified= &transformation_matrix;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene_object_name)
				{
					if (scene_object=Scene_get_Scene_object_by_name(scene,
						scene_object_name))
					{
						Scene_object_set_transformation(scene_object,
							&transformation_matrix);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No object named '%s' in scene",scene_object_name);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_transformation */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_up_vector(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the current view point from the command line.
==============================================================================*/
{
	double dot_prod,eye[3],lookat[3],norm[3],up[3],view[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	struct Graphics_window *window;

	ENTER(gfx_set_up_vector);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_lookat_parameters(
					Graphics_window_get_Scene_viewer(window,0),
					&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
					&up[0],&up[1],&up[2]);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
				position.data[0]=up[0];
				position.data[1]=up[1];
				position.data[2]=up[2];
				(option_table[0]).to_be_modified= &coordinate;
				(option_table[1]).to_be_modified= &position;
				(option_table[2]).to_be_modified= &window;
				(option_table[2]).user_data=command_data->graphics_window_manager;
				(option_table[3]).to_be_modified= &position;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						Scene_viewer_get_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),
							&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
							&up[0],&up[1],&up[2]);
						/* we have a rc position, but relative to a coordinate system we
							must convert to global */
						get_global_position(&position,coordinate,&global_position);
						up[0]=global_position.data[0];
						up[1]=global_position.data[1];
						up[2]=global_position.data[2];
						/* get unit vector in view direction */
						view[0]=lookat[0]-eye[0];
						view[1]=lookat[1]-eye[1];
						view[2]=lookat[2]-eye[2];
						normalize3(view);
						normalize3(up);
						/* make sure view direction not colinear with up-vector */
						dot_prod=fabs(dot_product3(view,up));
						if (dot_prod<0.99999)
						{
							/* ensure up vector is normal to view direction */
							if (dot_prod>0.00001)
							{
								cross_product3(up,view,norm);
								cross_product3(view,norm,up);
								normalize3(up);
								display_message(WARNING_MESSAGE,
									"gfx set up_vector.  Adjusting up_vector to (%g,%g,%g) so it "
									"remains normal to view direction",up[0],up[1],up[2]);
							}
							if (return_code=Scene_viewer_set_lookat_parameters(
								Graphics_window_get_Scene_viewer(window,0),eye[0],eye[1],eye[2],
								lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]))
								/*???DB.  Should not be changing directly because managed ? */
							{
								return_code=Graphics_window_update_now(window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx set up_vector.  Up vector colinear with view direction. "
								"Set interest_point and view_point first.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_up_vector.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_up_vector.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_up_vector */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static void update_callback(struct MANAGER_MESSAGE(FE_node) *message,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes "open comfile update execute"
???RC Is this used?
==============================================================================*/
{
	struct Cmiss_command_data *command_data;

	ENTER(update_callback);
	USE_PARAMETER(message);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
		Execute_command_execute_string(command_data->execute_command, "open comfile update execute");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_callback.  Missing command_data_void");
	}
} /* update_callback */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_update_callback(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 January 1998

DESCRIPTION :
Sets the update callback on/off.  When the update callback is on, the command
	open comfile update execute
is issued whenever a node is changed.
==============================================================================*/
{
	char *current_token;
	int return_code;
	static void *callback_id=NULL;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_set_update_callback);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			if (callback_id)
			{
				display_message(INFORMATION_MESSAGE,"on");
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"off");
			}
			display_message(INFORMATION_MESSAGE,
" ! when on, \"open comfile update execute\" is issued whenever a node is changed");
		}
		else
		{
			display_message(WARNING_MESSAGE,"This is a temporary command");
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (callback_id)
				{
					if (MANAGER_DEREGISTER(FE_node)(callback_id,
						command_data->node_manager))
					{
						callback_id=NULL;
					}
				}
				else
				{
					callback_id=MANAGER_REGISTER(FE_node)(update_callback,
						command_data_void,command_data->node_manager);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_set_update_callback.  Missing command_data_void");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_update_callback.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_update_callback */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_video(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 August 1998

DESCRIPTION :
Sets the video on/off for a window from the command line.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL} /*???DB.  set_name ? */
	};

	ENTER(gfx_set_video);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			/* must have at least one graphics_window to print */
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,(void *)NULL,
				command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			/* parse the command line */
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"gfx set video command temporarily disabled\n");
					return_code=0;
#if defined (OLD_GFX_WINDOW)
					return_code=set_video_on_off(window);
#endif /* defined (OLD_GFX_WINDOW) */
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_video.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_video.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_video */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_view_angle(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the view angle from the command line.
==============================================================================*/
{
	float view_angle;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_view_angle);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
			}
			view_angle=90;
			/* parse the command line */
			(option_table[0]).to_be_modified= &window;
			(option_table[1]).to_be_modified= &view_angle;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					view_angle *= PI/180.0;
					if (return_code=Scene_viewer_set_view_angle(
						Graphics_window_get_Scene_viewer(window,0),view_angle))
					{
						return_code=Graphics_window_update_now(window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Must have a graphics window");
					return_code=0;
				}
			} /* parse error, help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_view_angle.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_view_angle.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_view_angle */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_view_point(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Sets the current view point from the command line.
==============================================================================*/
{
	double eye[3],lookat[3],norm[3],oldview[3],up[3],view[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Cmgui_coordinate},
		{"position",NULL,NULL,set_Dof3_position},
		{"window",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Dof3_position},
	};
	struct Cmiss_command_data *command_data;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data global_position,position;
	struct Graphics_window *window;

	ENTER(gfx_set_view_point);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,command_data->graphics_window_manager))
			{
				ACCESS(Graphics_window)(window);
				return_code=Scene_viewer_get_lookat_parameters(
					Graphics_window_get_Scene_viewer(window,0),
					&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
					&up[0],&up[1],&up[2]);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				return_code=1;
			}
			if (return_code)
			{
				/* initialise defaults */
				coordinate=ACCESS(Cmgui_coordinate)(global_coordinate_ptr);
				position.data[0]=eye[0];
				position.data[1]=eye[1];
				position.data[2]=eye[2];
				(option_table[0]).to_be_modified= &coordinate;
				(option_table[1]).to_be_modified= &position;
				(option_table[2]).to_be_modified= &window;
				(option_table[2]).user_data=command_data->graphics_window_manager;
				(option_table[3]).to_be_modified= &position;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (window)
					{
						Scene_viewer_get_lookat_parameters(
							Graphics_window_get_Scene_viewer(window,0),
							&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],
							&up[0],&up[1],&up[2]);
						/* save the current view direction to give to the up-vector in case
							 the new view direction is colinear with the up-vector */
						oldview[0]=lookat[0]-eye[0];
						oldview[1]=lookat[1]-eye[1];
						oldview[2]=lookat[2]-eye[2];
						/* we have a rc position, but relative to a coordinate system we
							must convert to global */
						get_global_position(&position,coordinate,&global_position);
						eye[0]=global_position.data[0];
						eye[1]=global_position.data[1];
						eye[2]=global_position.data[2];
						/* check eye and lookat points at different positions */
						if ((eye[0]!=lookat[0])||(eye[1]!=lookat[1])||(eye[2]!=lookat[2]))
						{
							/* get unit vector in view direction */
							view[0]=lookat[0]-eye[0];
							view[1]=lookat[1]-eye[1];
							view[2]=lookat[2]-eye[2];
							normalize3(view);
							normalize3(up);
							/* is view direction colinear with up-vector? */
							if (fabs(dot_product3(view,up))>0.99999)
							{
								/* set the up vector to the old view direction (normalized) */
								up[0]=oldview[0];
								up[1]=oldview[1];
								up[2]=oldview[2];
								normalize3(up);
								display_message(WARNING_MESSAGE,
									"gfx set view_point.  View direction colinear with up_vector."
									" Changing up_vector to (%g,%g,%g)",up[0],up[1],up[2]);
							}
							/* ensure up vector is normal to view direction */
							if (fabs(dot_product3(view,up))>0.00001)
							{
								cross_product3(up,view,norm);
								cross_product3(view,norm,up);
								normalize3(up);
								display_message(WARNING_MESSAGE,
									"gfx set view_point.  View direction not normal to up_vector."
									" Changing up_vector to (%g,%g,%g)",up[0],up[1],up[2]);
							}
							if (return_code=Scene_viewer_set_lookat_parameters(
								Graphics_window_get_Scene_viewer(window,0),eye[0],eye[1],eye[2],
								lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]))
								/*???DB.  Should not be changing directly because managed ? */
							{
								return_code=Graphics_window_update_now(window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx set view_point.  View_point same as interest_point. "
								"Set the interest_point first");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Must have a graphics window");
						return_code=0;
					}
				} /* parse error, help */
				DEACCESS(Cmgui_coordinate)(&coordinate);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_view_point.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_view_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_view_point */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_set_visibility(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Toggles the visibility of graphics objects on scenes from the command line.
==============================================================================*/
{
	char *name,off_flag,on_flag;
	enum GT_visibility_type current_visibility,new_visibility;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry
		off_on_option_table[]=
		{
			{"off",NULL,NULL,set_char_flag},
			{"on",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"name",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{"scene",NULL,NULL,set_Scene},
			{NULL,NULL,NULL,set_name}
		};

	ENTER(gfx_set_visibility);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			off_flag=0;
			on_flag=0;
			(option_table[0]).to_be_modified = &name;
			(off_on_option_table[0]).to_be_modified= &off_flag;
			(off_on_option_table[1]).to_be_modified= &on_flag;
			(option_table[1]).user_data= &off_on_option_table;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (off_flag&&on_flag)
				{
					display_message(ERROR_MESSAGE,"Only one of off/on");
					return_code=0;
				}
				if (return_code)
				{
					if (name)
					{
						if (scene_object=Scene_get_Scene_object_by_name(scene,name))
						{
							current_visibility=Scene_object_get_visibility(scene_object);
							if (on_flag)
							{
								new_visibility=g_VISIBLE;
							}
							else
							{
								if (off_flag)
								{
									new_visibility=g_INVISIBLE;
								}
								else
								{
									if (g_VISIBLE == current_visibility)
									{
										new_visibility=g_INVISIBLE;
									}
									else
									{
										new_visibility=g_VISIBLE;
									}
								}
							}
							if (new_visibility!=current_visibility)
							{
								Scene_object_set_visibility(scene_object,new_visibility);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"No graphics object named '%s' in scene",name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing graphics object name");
						return_code=0;
					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_visibility.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_visibility.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_visibility */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_gfx_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX SET command.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"application_parameters",NULL,NULL,set_application_parameters},
		{"axis_length",NULL,NULL,gfx_set_axis_length},
		{"axis_material",NULL,NULL,gfx_set_axis_material},
		{"axis_origin",NULL,NULL,gfx_set_axis_origin},
		{"background",NULL,NULL,gfx_set_background},
		{"far_clipping_plane",NULL,NULL,gfx_set_far_clipping_plane},
		{"interest_point",NULL,NULL,gfx_set_interest_point},
		{"line_width",NULL,NULL,set_float_positive},
		{"near_clipping_plane",NULL,NULL,gfx_set_near_clipping_plane},
		{"node_value",NULL,NULL,gfx_set_FE_nodal_value},
		{"order",NULL,NULL,gfx_set_scene_order},
		{"point_size",NULL,NULL,set_float_positive},
		{"resolution",NULL,NULL,set_graphics_window_resolution},
		{"slider",NULL,NULL,set_node_group_slider_value},
		{"transformation",NULL,NULL,gfx_set_transformation},
		{"time",NULL,NULL,gfx_set_time},
		{"up_vector",NULL,NULL,gfx_set_up_vector},
		{"update_callback",NULL,NULL,gfx_set_update_callback},
		{"video",NULL,NULL,gfx_set_video},
		{"view_angle",NULL,NULL,gfx_set_view_angle},
		{"view_point",NULL,NULL,gfx_set_view_point},
		{"visibility",NULL,NULL,gfx_set_visibility},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* application_parameters */
				i++;
				/* axis_length */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* axis_material */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* axis_origin */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* background */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* far_clipping_plane */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* interest_point */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* line_width */
				(option_table[i]).to_be_modified= &global_line_width;
				i++;
				/* near_clipping_plane */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* node_value */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* order */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* point_size */
				(option_table[i]).to_be_modified= &global_point_size;
				i++;
				/* resolution */
				(option_table[i]).user_data=command_data->graphics_window_manager;
				i++;
				/* slider */
				(option_table[i]).user_data=command_data->node_group_slider_dialog;
				i++;
				/* transformation */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* time */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* up_vector */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* update_callback */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* video */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* view_angle */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* view_point */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* visibility */
				(option_table[i]).user_data=command_data_void;
				i++;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx set",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_set */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_smooth(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX SMOOTH command.
==============================================================================*/
{
	int i,j,number_of_components,return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"egroup",NULL,NULL,set_FE_element_group},
		{"field",NULL,NULL,set_FE_field},
		{"smoothing",NULL,NULL,set_float_positive},
		{NULL,NULL,NULL,NULL}
	};
	struct FE_field *field;
	struct GROUP(FE_element) *element_group;
	struct LIST(FE_node) **node_lists;
	struct Smooth_field_over_element_data smooth_field_over_element_data;
	struct Smooth_field_over_node_data smooth_field_over_node_data;

	ENTER(execute_command_gfx_smooth);
	USE_PARAMETER(dummy_to_be_modified);
#if defined (DEBUG)
	printf("enter execute_command_gfx_smooth\n");
#endif /* defined (DEBUG) */
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			element_group=(struct GROUP(FE_element) *)NULL;
			smooth_field_over_element_data.field=(struct FE_field *)NULL;
			smooth_field_over_element_data.smoothing=1;
			smooth_field_over_element_data.node_manager=command_data->node_manager;
			smooth_field_over_element_data.element_manager=
				command_data->element_manager;
			(option_table[0]).to_be_modified= &element_group;
			(option_table[0]).user_data=command_data->element_group_manager;
			(option_table[1]).to_be_modified= &(smooth_field_over_element_data.field);
			(option_table[1]).user_data=command_data->fe_field_manager;
			(option_table[2]).to_be_modified=
				&(smooth_field_over_element_data.smoothing);
			return_code=process_multiple_options(state,option_table);
			if (return_code)
			{
				display_message(WARNING_MESSAGE,"gfx smooth is temporary/not general");
				MANAGER_BEGIN_CACHE(FE_node)(command_data->node_manager);
				MANAGER_BEGIN_CACHE(FE_element)(command_data->element_manager);
				smooth_field_over_element_data.node_lists=(struct LIST(FE_node) **)NULL;
				if (element_group)
				{
					return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
						smooth_field_over_element,(void *)&smooth_field_over_element_data,
						element_group);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
						smooth_field_over_element,(void *)&smooth_field_over_element_data,
						command_data->element_manager);
				}
				if (return_code&&(field=smooth_field_over_element_data.field))
				{
					node_lists=smooth_field_over_element_data.node_lists;
					smooth_field_over_node_data.node_manager=command_data->node_manager;
					smooth_field_over_node_data.field_component.field=field;
					smooth_field_over_node_data.number_of_elements=0;
					for (j=0;j<smooth_field_over_element_data.
						maximum_number_of_elements_per_node;j++)
					{
						(smooth_field_over_node_data.number_of_elements)++;
						number_of_components=get_FE_field_number_of_components(field);
						for (i=0;i<number_of_components;i++)
						{
							smooth_field_over_node_data.field_component.number=i;
							FOR_EACH_OBJECT_IN_LIST(FE_node)(smooth_field_over_node,
								(void *)&smooth_field_over_node_data,*node_lists);
							node_lists++;
						}
					}
				}
				if (node_lists=smooth_field_over_element_data.node_lists)
				{
					number_of_components=get_FE_field_number_of_components(
						smooth_field_over_element_data.field);
					for (i=(number_of_components)*(smooth_field_over_element_data.
						maximum_number_of_elements_per_node);i>0;i--)
					{
						DESTROY_LIST(FE_node)(node_lists);
						node_lists++;
					}
					DEALLOCATE(smooth_field_over_element_data.node_lists);
				}
				MANAGER_END_CACHE(FE_element)(command_data->element_manager);
				MANAGER_END_CACHE(FE_node)(command_data->node_manager);
			}
			if (element_group)
			{
				DEACCESS(GROUP(FE_element))(&element_group);
			}
			if (smooth_field_over_element_data.field)
			{
				DEACCESS(FE_field)(&smooth_field_over_element_data.field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_smooth.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_smooth.  Missing state");
		return_code=0;
	}
#if defined (DEBUG)
	printf("leave execute_command_gfx_smooth\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* execute_command_gfx_smooth */
#endif /* !defined (WINDOWS_DEV_FLAG) */

int gfx_timekeeper(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
	char every, loop, maximum_flag, minimum_flag, once, play, set_time_flag,
		skip, speed_flag, stop, swing;
	double maximum, minimum, set_time, speed;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"every_frame",NULL,NULL,set_char_flag},
		{"loop",NULL,NULL,set_char_flag},
		{"maximum",NULL,NULL,set_double_and_char_flag},
		{"minimum",NULL,NULL,set_double_and_char_flag},
		{"once",NULL,NULL,set_char_flag},
		{"play",NULL,NULL,set_char_flag},
		{"set_time",NULL,NULL,set_double_and_char_flag},
		{"skip_frames",NULL,NULL,set_char_flag},
		{"speed",NULL,NULL,set_double_and_char_flag},
		{"stop",NULL,NULL,set_char_flag},
		{"swing",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Time_keeper *time_keeper;

	ENTER(gfx_timekeeper);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					if (!strcmp(state->current_token, "default"))
					{
						/* Continue */
						return_code = shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Only a default timekeeper at the moment");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"\n      TIMEKEEPER_NAME");
					/* By not shifting the parse state the rest of the help should come out */
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_timekeeper.  Missing timekeeper name");
				return_code=0;
			}
			if (return_code)
			{
				/* initialise defaults */
				if (time_keeper = command_data->default_time_keeper)
				{
					maximum = Time_keeper_get_maximum(time_keeper);
					minimum = Time_keeper_get_minimum(time_keeper);
					set_time = Time_keeper_get_time(time_keeper);
					speed = Time_keeper_get_speed(time_keeper);
				}
				else
				{
					maximum = 0.0;
					minimum = 0.0;
					set_time = 0.0;
					speed = 30;
				}
				every = 0;
				loop = 0;
				maximum_flag = 0;
				minimum_flag = 0;
				once = 0;
				play = 0;
				set_time_flag = 0;
				skip = 0;
				speed_flag = 0;
				stop = 0;
				swing = 0;

				(option_table[0]).to_be_modified = &every;
				(option_table[1]).to_be_modified = &loop;
				(option_table[2]).to_be_modified = &maximum;
				(option_table[2]).user_data = &maximum_flag;
				(option_table[3]).to_be_modified = &minimum;				
				(option_table[3]).user_data = &minimum_flag;
				(option_table[4]).to_be_modified = &once;
				(option_table[5]).to_be_modified = &play;
				(option_table[6]).to_be_modified = &set_time;
				(option_table[6]).user_data = &set_time_flag;
				(option_table[7]).to_be_modified = &skip;
				(option_table[8]).to_be_modified = &speed;
				(option_table[8]).user_data = &speed_flag;
				(option_table[9]).to_be_modified = &stop;
				(option_table[10]).to_be_modified = &swing;
				return_code=process_multiple_options(state,option_table);

				if(return_code)
				{
					if((loop + once + swing) > 1)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of loop, swing or once");
						return_code = 0;
					}
					if(every && skip)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of every_frame or skip_frames");
						return_code = 0;
					}
					if(play && stop)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of play or stop");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if ( time_keeper )
					{
						if ( set_time_flag )
						{
							Time_keeper_request_new_time(time_keeper, set_time);
						}
						if ( speed_flag )
						{
							Time_keeper_set_speed(time_keeper, speed);
						}
						if ( maximum_flag )
						{
							Time_keeper_set_maximum(time_keeper, maximum);
						}
						if ( minimum_flag )
						{
							Time_keeper_set_minimum(time_keeper, minimum);
						}
						if ( loop )
						{
							Time_keeper_set_play_loop(time_keeper);
						}
						if ( swing )
						{
							Time_keeper_set_play_swing(time_keeper);
						}
						if ( once )
						{
							Time_keeper_set_play_once(time_keeper);
						}
						if ( every )
						{
							Time_keeper_set_play_every_frame(time_keeper);
						}
						if ( skip )
						{
							Time_keeper_set_play_skip_frames(time_keeper);
						}
						if ( play )
						{
							Time_keeper_play(time_keeper, TIME_KEEPER_PLAY_FORWARD);
						}
						if ( stop )
						{
							Time_keeper_stop(time_keeper);
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_timekeeper.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_timekeeper.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_timekeeper */

static int gfx_transform_tool(struct Parse_state *state,
	void *dummy_user_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Executes a GFX TRANSFORM_TOOL command.
==============================================================================*/
{
	int free_spin_flag, return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Interactive_tool *transform_tool;

	ENTER(execute_command_gfx_transform_tool);
	USE_PARAMETER(dummy_user_data);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void)
		&& (transform_tool=command_data->transform_tool))
	{
		/* initialize defaults */
		free_spin_flag = Interactive_tool_transform_get_free_spin(transform_tool);

		option_table=CREATE(Option_table)();
		/* free_spin/no_free_spin */
		Option_table_add_switch(option_table,"free_spin","no_free_spin",&free_spin_flag);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			Interactive_tool_transform_set_free_spin(transform_tool, free_spin_flag);
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_transform_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_transform_tool */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_update(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
Executes a GFX UPDATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_update);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					return_code=Graphics_window_update_now(window);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
						Graphics_window_update_now_iterator,(void *)NULL,
						command_data->graphics_window_manager);
				}
			} /* parse error,help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_update.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_update.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_update */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_warp_node(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX WARP NODE command. This warps the coordinate nodal values (not
the derivatives).
???DB.  Don't pass all of command data ?
==============================================================================*/
{
	double ximax[3];
	int return_code,xi_order;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate_field",NULL,NULL,set_FE_field},
		{"extent",NULL,NULL,set_Element_discretization},
		{"from",NULL,NULL,set_FE_node_group},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{"to_coordinate_field",NULL,NULL,set_FE_field},
		{"values",NULL,NULL,set_Warp_values},
		{"warp_field",NULL,NULL,set_FE_field},
		{"xi_order",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;
	struct FE_field *coordinate_field,*to_coordinate_field,*warp_field;
	struct GROUP(FE_node) *node_group;
	struct Warp_values warp_values;

	ENTER(gfx_warp_node);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			coordinate_field=(struct FE_field *)NULL;
			to_coordinate_field=(struct FE_field *)NULL;
			warp_field=(struct FE_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			xi_order=123;
			node_group=(struct GROUP(FE_node) *)NULL;
			warp_values.value[0] = 0.0;
			warp_values.value[1] = 0.0;
			warp_values.value[2] = 0.0;
			warp_values.value[3] = 0.0;
			warp_values.value[4] = 0.0;
			warp_values.value[5] = 0.0;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data=command_data->fe_field_manager;
			(option_table[1]).to_be_modified= &extent;
			(option_table[1]).user_data=command_data->user_interface;
			(option_table[2]).to_be_modified= &node_group;
			(option_table[2]).user_data=command_data->node_group_manager;
			(option_table[3]).to_be_modified= &seed_element;
			(option_table[3]).user_data=command_data->element_manager;
			(option_table[4]).to_be_modified= &to_coordinate_field;
			(option_table[4]).user_data=command_data->fe_field_manager;
			(option_table[5]).to_be_modified= &warp_values;
			(option_table[6]).to_be_modified= &warp_field;
			(option_table[6]).user_data=command_data->fe_field_manager;
			(option_table[7]).to_be_modified= &xi_order;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				ximax[0]=extent.number_in_xi1;
				ximax[1]=extent.number_in_xi2;
				ximax[2]=extent.number_in_xi3;
				return_code=warp_FE_node_group_with_FE_element(node_group,
					command_data->node_manager,coordinate_field,to_coordinate_field,
					seed_element,warp_field,ximax,warp_values.value,xi_order);
			}
			if (node_group)
			{
				DEACCESS(GROUP(FE_node))(&node_group);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			if (coordinate_field)
			{
				DEACCESS(FE_field)(&coordinate_field);
			}
			if (to_coordinate_field)
			{
				DEACCESS(FE_field)(&to_coordinate_field);
			}
			if (warp_field)
			{
				DEACCESS(FE_field)(&warp_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_warp_node.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_warp_node.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_warp_node */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int gfx_warp_voltex(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX WARP VOLTEX command. This warps the vertex and normals of a
gtvoltex according to calculated
???DB.  Don't pass all of command data ?
==============================================================================*/
{
	char *graphics_object_name1,*graphics_object_name2;
	double ximax[3];
	gtObject *graphics_object1,*graphics_object2;
	int itime1,itime2,return_code,xi_order;
	static struct Modifier_entry option_table[]=
	{
		{"extent",NULL,NULL,set_Element_discretization},
		{"field",NULL,NULL,set_FE_field},
		{"from",NULL,(void *)1,set_name},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{"to",NULL,(void *)1,set_name},
		{"values",NULL,NULL,set_Warp_values},
		{"xi_order",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;
	struct FE_field *warp_field;
	struct Warp_values warp_values;

	ENTER(gfx_warp_voltex);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name1 = duplicate_string("volume");
			graphics_object_name2 = duplicate_string("volume");
			warp_field=(struct FE_field *)NULL;
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			(warp_values.value)[0] = 0.0;
			(warp_values.value)[1] = 0.0;
			(warp_values.value)[2] = 0.0;
			(warp_values.value)[3] = 0.0;
			(warp_values.value)[4] = 0.0;
			(warp_values.value)[5] = 0.0;
			xi_order=123;
			(option_table[0]).to_be_modified= &extent;
			(option_table[0]).user_data=command_data->user_interface;
			(option_table[1]).to_be_modified= &warp_field;
			(option_table[1]).user_data=command_data->fe_field_manager;
			(option_table[2]).to_be_modified= &graphics_object_name1;
			(option_table[3]).to_be_modified= &seed_element;
			(option_table[3]).user_data=command_data->element_manager;
			(option_table[4]).to_be_modified= &graphics_object_name2;
			(option_table[5]).to_be_modified= &warp_values;
			(option_table[6]).to_be_modified= &xi_order;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if ((graphics_object1=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name1,command_data->graphics_object_list))&&
					(graphics_object2=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name2,command_data->graphics_object_list)))
				{
					if ((g_VOLTEX==graphics_object1->object_type)&&
						(g_VOLTEX==graphics_object1->object_type))
					{
						if (seed_element&&(seed_element->shape)&&
							(3==seed_element->shape->dimension))
						{
							ximax[0]=extent.number_in_xi1;
							ximax[1]=extent.number_in_xi2;
							ximax[2]=extent.number_in_xi3;
							itime1=graphics_object1->number_of_times;
							itime2=graphics_object2->number_of_times;
							return_code=warp_GT_voltex_with_FE_element(
								(graphics_object1->gu.gt_voltex)[itime1-1],
								(graphics_object2->gu.gt_voltex)[itime2-1],seed_element,
								warp_field,ximax,warp_values.value,xi_order);
#if defined (DEBUG)
							/*???debug */
							printf("Warp called: volume1 = %s, volume2 = %s,  element = %d, coordinates = %s, extent = %d %d %d, values = %f %f,  %f %f,  %f %f xi_order = %d\n",
								graphics_object1->name,graphics_object2->name,
								(seed_element->cmiss).element_number,warp_field->name,
								extent.number_in_xi1,extent.number_in_xi2,extent.number_in_xi3,
								(warp_values.value)[0],(warp_values.value)[1],
								(warp_values.value)[2],(warp_values.value)[3],
								(warp_values.value)[4],(warp_values.value)[5],xi_order);
#endif /* defined (DEBUG) */
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing or non 3-D seed element");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Specified graphics object(s) not of voltex type");
						return_code=0;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"Volumes don't exist");
					return_code=0;
				}
			}
			DEALLOCATE(graphics_object_name1);
			DEALLOCATE(graphics_object_name2);
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
			if (warp_field)
			{
				DEACCESS(FE_field)(&warp_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_warp_voltex.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_warp_voltex.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_warp_voltex */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx_warp(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 1998

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"node",NULL,NULL,gfx_warp_node},
		{"voltex",NULL,NULL,gfx_warp_voltex},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_warp);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				(option_table[0]).user_data=command_data_void;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx warp",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_warp.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int gfx_write_Control_curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes an individual curve or all curves to filename(s) stemming from the name
of the curve, eg. "name" -> name.curve.com name.curve.exnode name.curve.exelem
==============================================================================*/
{
	char write_all_curves_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{"all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Control_curve}
	};
	struct Control_curve *curve;

	ENTER(gfx_write_Control_curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		write_all_curves_flag=0;
		curve=(struct Control_curve *)NULL;
		(option_table[0]).to_be_modified= &write_all_curves_flag;
		(option_table[1]).to_be_modified= &curve;
		(option_table[1]).user_data=command_data->control_curve_manager;
		if (return_code=process_multiple_options(state,option_table))
		{
			if (write_all_curves_flag&&!curve)
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Control_curve)(
					write_Control_curve,(void *)NULL,
					command_data->control_curve_manager);
			}
			else if (curve&&!write_all_curves_flag)
			{
				return_code=write_Control_curve(curve,(void *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_write_Control_curve.  Specify either a curve name or 'all'");
				return_code=0;
			}
		}
		if (curve)
		{
			DEACCESS(Control_curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_Control_curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_Control_curve */

static int gfx_write_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the element file is written.
Can also write individual element groups with the <group> option.
==============================================================================*/
{
	char *file_ext = ".exelem";
	char *file_name, **valid_strings, *write_criterion_string;
	enum FE_write_criterion write_criterion;
	int number_of_valid_strings, return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field_order_info *field_order_info;
	struct Fwrite_all_FE_element_groups_data fwrite_all_FE_element_groups_data;
	struct Fwrite_FE_element_group_data fwrite_FE_element_group_data;
	struct GROUP(FE_element) *element_group;
	struct Option_table *option_table;

	ENTER(gfx_write_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		element_group = (struct GROUP(FE_element) *)NULL;
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;
		write_criterion = FE_WRITE_COMPLETE_GROUP;

		option_table = CREATE(Option_table)();
		/* complete_group|with_all_listed_fields|with_any_listed_fields */ 
		write_criterion_string =
			ENUMERATOR_STRING(FE_write_criterion)(write_criterion);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(FE_write_criterion)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(FE_write_criterion) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &write_criterion_string);
		DEALLOCATE(valid_strings);
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			command_data->fe_field_manager, set_FE_fields);
		/* group */
		Option_table_add_entry(option_table, "group", &element_group,
			command_data->element_group_manager, set_FE_element_group);
		/* default option: file name */
		Option_table_add_entry(option_table, (char *)NULL, &file_name,
			NULL, set_name);

		if (return_code = Option_table_multi_parse(option_table, state))
		{
			STRING_TO_ENUMERATOR(FE_write_criterion)(write_criterion_string,
				&write_criterion);
			if ((!field_order_info ||
				(0 == get_FE_field_order_info_number_of_fields(field_order_info))) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx_write_nodes.  Must specify fields to use %s",
					write_criterion_string);
				return_code = 0;
				
			}
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(file_ext,
					command_data->user_interface)))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* open the file */
				if (return_code = check_suffix(&file_name,".exelem"))
				{
					if (element_group)
					{
						fwrite_FE_element_group_data.write_criterion = write_criterion;
						fwrite_FE_element_group_data.field_order_info = field_order_info;
						fwrite_FE_element_group_data.element_group = element_group;
						return_code = file_write_FE_element_group(file_name,
							(void *)&fwrite_FE_element_group_data);
					}
					else
					{
						fwrite_all_FE_element_groups_data.write_criterion = write_criterion;
						fwrite_all_FE_element_groups_data.field_order_info =
							field_order_info;
						fwrite_all_FE_element_groups_data.element_group_manager=
							command_data->element_group_manager;
						return_code = file_write_all_FE_element_groups(file_name,
							(void *)&fwrite_all_FE_element_groups_data);
					}
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (element_group)
		{
			DEACCESS(GROUP(FE_element))(&element_group);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_elements */

static int gfx_write_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	static char *data_file_ext = ".exdata";
	static char *node_file_ext = ".exnode";
	char *file_ext, *file_name, **valid_strings, *write_criterion_string;
	enum FE_write_criterion write_criterion;
	int number_of_valid_strings, return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field_order_info *field_order_info;
	struct Fwrite_all_FE_node_groups_data fwrite_all_FE_node_groups_data;
	struct Fwrite_FE_node_group_data fwrite_FE_node_group_data;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Option_table *option_table;

	ENTER(gfx_write_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		if (use_data)
		{
			node_group_manager = command_data->data_group_manager;
			file_ext = data_file_ext;
		}
		else
		{
			node_group_manager = command_data->node_group_manager;
			file_ext = node_file_ext;
		}
		node_group = (struct GROUP(FE_node) *)NULL;
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;
		write_criterion = FE_WRITE_COMPLETE_GROUP;

		option_table = CREATE(Option_table)();
		/* complete_group|with_all_listed_fields|with_any_listed_fields */ 
		write_criterion_string =
			ENUMERATOR_STRING(FE_write_criterion)(write_criterion);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(FE_write_criterion)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(FE_write_criterion) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &write_criterion_string);
		DEALLOCATE(valid_strings);
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			command_data->fe_field_manager, set_FE_fields);
		/* group */
		Option_table_add_entry(option_table, "group", &node_group,
			node_group_manager, set_FE_node_group);
		/* default option: file name */
		Option_table_add_entry(option_table, (char *)NULL, &file_name,
			NULL, set_name);

		if (return_code = Option_table_multi_parse(option_table, state))
		{
			STRING_TO_ENUMERATOR(FE_write_criterion)(write_criterion_string,
				&write_criterion);
			if ((!field_order_info ||
				(0 == get_FE_field_order_info_number_of_fields(field_order_info))) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx_write_nodes.  Must specify fields to use %s",
					write_criterion_string);
				return_code = 0;
				
			}
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(file_ext,
					command_data->user_interface)))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* open the file */
				if (return_code = check_suffix(&file_name, file_ext))
				{
					if (node_group)
					{
						fwrite_FE_node_group_data.write_criterion = write_criterion;
						fwrite_FE_node_group_data.field_order_info = field_order_info;
						fwrite_FE_node_group_data.node_group = node_group;
						return_code = file_write_FE_node_group(file_name,
							(void *)&fwrite_FE_node_group_data);
					}
					else
					{
						fwrite_all_FE_node_groups_data.write_criterion = write_criterion;
						fwrite_all_FE_node_groups_data.field_order_info = field_order_info;
						fwrite_all_FE_node_groups_data.node_group_manager =
							node_group_manager;
						return_code = file_write_all_FE_node_groups(file_name,
							(void *)&fwrite_all_FE_node_groups_data);
					}
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_nodes */

static int gfx_write_element_layout(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Executes a GFX WRITE_ELEMENT_LAYOUT command.
==============================================================================*/
{
	double ximax[3];
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"extent",NULL,NULL,set_Element_discretization},
		{"seed_element",NULL,NULL,set_FE_element_dimension_3},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Element_discretization extent;
	struct FE_element *seed_element;

	ENTER(gfx_write_element_layout);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			seed_element=(struct FE_element *)NULL;
			extent.number_in_xi1=1;
			extent.number_in_xi2=1;
			extent.number_in_xi3=1;
			(option_table[0]).to_be_modified= &extent;
			(option_table[0]).user_data=(void *)(command_data->user_interface);
			(option_table[1]).to_be_modified= &seed_element;
			(option_table[1]).user_data=command_data->element_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				ximax[0]=extent.number_in_xi1;
				ximax[1]=extent.number_in_xi2;
				ximax[2]=extent.number_in_xi3;
				return_code=write_FE_element_layout(ximax, seed_element);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_FE_element_layout.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_FE_element_layout.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write_element_layout */

#if defined (IMAGEMAGICK)
static int gfx_write_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 May 1999

DESCRIPTION :
Executes a GFX WRITE TEXTURE command.
==============================================================================*/
{
	char *current_token,*extended_filename,*file_name,postscript_flag,
		rgb_flag,tiff_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *format_option_table, *option_table;
	struct Texture *texture;

	ENTER(gfx_write_texture);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			texture = (struct Texture *)NULL;
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (command_data->texture_manager)
					{
						if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)
							(current_token,command_data->texture_manager))
						{
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_write_texture.  Unknown texture : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_write_Texture.  Missing texture manager");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" TEXTURE_NAME");
					return_code = 1;
					/* By not shifting the parse state the rest of the help should come out */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing texture name");
				return_code=0;
			}

			if (return_code)
			{
				/* initialize defaults */
				file_name=(char *)NULL;
				postscript_flag=0;
				rgb_flag=0;
				tiff_flag=0;

				option_table=CREATE(Option_table)();
				/* file */
				Option_table_add_entry(option_table, "file", &file_name,
					(void *)1, set_name);
				/* postscript/rgb/tiff */
				format_option_table=CREATE(Option_table)();
				Option_table_add_entry(format_option_table,"postscript",
					&postscript_flag,NULL,set_char_flag);
				Option_table_add_entry(format_option_table,"rgb",
					&rgb_flag,NULL,set_char_flag);
				Option_table_add_entry(format_option_table,"tiff",
					&tiff_flag,NULL,set_char_flag);
				Option_table_add_suboption_table(option_table,format_option_table);

				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (postscript_flag + rgb_flag + tiff_flag > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify only one of postscript, rgb or tiff");
						return_code = 0;
					}
					if(!texture)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify texture to write");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (!file_name)
					{
						if (!(file_name = confirmation_get_write_filename(NULL,
							command_data->user_interface)))
						{
							return_code = 0;
						}
					}
					if ((postscript_flag|rgb_flag|tiff_flag) &&
						(!strchr(file_name, ':')) &&
						ALLOCATE(extended_filename, char, strlen(file_name) + 6))
					{
						if (rgb_flag)
						{
							sprintf(extended_filename, "sgi:%s", file_name);
						}
						else if (postscript_flag)
						{
							sprintf(extended_filename, "ps:%s", file_name);
						}
						else
						{
							sprintf(extended_filename, "tiff:%s", file_name);
						}
						DEALLOCATE(file_name);
						file_name = extended_filename;
					}
				}
				if (return_code)
				{
					Texture_write_to_file(texture, file_name);
				} /* parse error, help */
				if (file_name)
				{
					DEALLOCATE(file_name);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_texture.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_write_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_texture */
#else /* defined (IMAGEMAGICK) */
static int gfx_write_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 May 1999

DESCRIPTION :
Executes a GFX WRITE TEXTURE command.
==============================================================================*/
{
	auto struct Modifier_entry
		landscape_portrait_option_table[]=
		{
			{"landscape",NULL,NULL,set_char_flag},
			{"portrait",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"file",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		},
		postscript_rgb_tiff_option_table[]=
		{
			{"postscript",NULL,NULL,set_char_flag},
			{"rgb",NULL,NULL,set_char_flag},
			{"tiff",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token,*file_name,landscape_flag,portrait_flag,postscript_flag,
		rgb_flag,tiff_flag;
	enum Image_file_format file_format;
	enum Image_orientation orientation;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_write_texture);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			texture = (struct Texture *)NULL;
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (command_data->texture_manager)
					{
						if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)
							(current_token,command_data->texture_manager))
						{
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_write_texture.  Unknown texture : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_write_Texture.  Missing texture manager");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" TEXTURE_NAME");
					return_code = 1;
					/* By not shifting the parse state the rest of the help should come out */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing texture name");
				return_code=0;
			}

			if (return_code)
			{
				/* initialize defaults */
				file_name=(char *)NULL;
				landscape_flag=0;
				portrait_flag=0;
				postscript_flag=0;
				rgb_flag=0;
				tiff_flag=0;

				(option_table[0]).to_be_modified= &file_name;
				(landscape_portrait_option_table[0]).to_be_modified= &landscape_flag;
				(landscape_portrait_option_table[1]).to_be_modified= &portrait_flag;
				(option_table[1]).user_data=landscape_portrait_option_table;
				(postscript_rgb_tiff_option_table[0]).to_be_modified= &postscript_flag;
				(postscript_rgb_tiff_option_table[1]).to_be_modified= &rgb_flag;
				(postscript_rgb_tiff_option_table[2]).to_be_modified= &tiff_flag;
				(option_table[2]).user_data=postscript_rgb_tiff_option_table;
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (postscript_flag + rgb_flag + tiff_flag > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify only one of postscript, rgb or tiff");
						return_code = 0;
					}
					if (landscape_flag && portrait_flag)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify only one of landscape or portrait");
						return_code = 0;
					}
					if (postscript_flag + rgb_flag + tiff_flag == 0)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Must specify one of postscript, rgb or tiff");
						return_code = 0;
					}				
					if(!texture)
					{
						display_message(ERROR_MESSAGE,"gfx_write_texture.  Specify texture to write");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (rgb_flag)
					{
						file_format = RGB_FILE_FORMAT;
					}
					else if (tiff_flag)
					{
						file_format = TIFF_FILE_FORMAT;
					}
					else if (postscript_flag)
					{
						file_format = POSTSCRIPT_FILE_FORMAT;
					}
					if (landscape_flag)
					{
						orientation = LANDSCAPE_ORIENTATION;
					}
					else
					{
						/* Default */
						orientation = PORTRAIT_ORIENTATION;
					}
					if (!file_name)
					{
						switch (file_format)
						{
							case RGB_FILE_FORMAT:
							{
								if (!(file_name = confirmation_get_write_filename(".rgb",
									command_data->user_interface)))
								{
									return_code = 0;
								}
							} break;
							case TIFF_FILE_FORMAT:
							{
								if (!(file_name = confirmation_get_write_filename(".tiff",
									command_data->user_interface)))
								{
									return_code = 0;
								}					
							} break;
							case POSTSCRIPT_FILE_FORMAT:
							{
								if (!(file_name = confirmation_get_write_filename(".ps",
									command_data->user_interface)))
								{
									return_code = 0;
								}					
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"gfx_write_texture.  Error setting file extension");
								return_code = 0;							
							} break;
						}
					}
				}
				if (return_code)
				{
					Texture_write_to_file(texture, file_name, file_format, orientation);
				} /* parse error, help */
				if (file_name)
				{
					DEALLOCATE(file_name);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_write_texture.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_write_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_texture */
#endif /* defined (IMAGEMAGICK) */

static int execute_command_gfx_write(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
Executes a GFX WRITE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_write);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "curve", NULL,
				command_data_void, gfx_write_Control_curve);
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "element_layout", NULL,
				command_data_void, gfx_write_element_layout);
			Option_table_add_entry(option_table, "elements", NULL,
				command_data_void, gfx_write_elements);
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "texture", NULL,
				command_data_void, gfx_write_texture);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx write", command_data->command_window);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_write.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_gfx(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"change_identifier",NULL,NULL,gfx_change_identifier},
		{"create",NULL,NULL,execute_command_gfx_create},
#if !defined (WINDOWS_DEV_FLAG)
		{"data_tool",NULL,NULL,execute_command_gfx_node_tool},
		{"define",NULL,NULL,execute_command_gfx_define},
		{"destroy",NULL,NULL,execute_command_gfx_destroy},
		{"draw",NULL,NULL,execute_command_gfx_draw},
		{"edit",NULL,NULL,execute_command_gfx_edit},
		{"element_creator",NULL,NULL,execute_command_gfx_element_creator},
		{"element_tool",NULL,NULL,execute_command_gfx_element_tool},
		{"erase",NULL,NULL,execute_command_gfx_erase},
		{"evaluate",NULL,NULL,gfx_evaluate},
		{"export",NULL,NULL,execute_command_gfx_export},
		{"filter",NULL,NULL,execute_command_gfx_filter},
		{"grab_frame",NULL,NULL,execute_command_gfx_grab_frame},
		{"list",NULL,NULL,execute_command_gfx_list},
		{"modify",NULL,NULL,execute_command_gfx_modify},
#if defined (SGI_MOVIE_FILE)
		{"movie",NULL,NULL,gfx_movie},
#endif /* defined (SGI_MOVIE_FILE) */
		{"node_tool",NULL,NULL,execute_command_gfx_node_tool},
		{"print",NULL,NULL,execute_command_gfx_print},
		{"project",NULL,NULL,open_projection_window},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{"read",NULL,NULL,execute_command_gfx_read},
#if !defined (WINDOWS_DEV_FLAG)
		{"select",NULL,NULL,execute_command_gfx_select},
		{"set",NULL,NULL,execute_command_gfx_set},
		{"smooth",NULL,NULL,execute_command_gfx_smooth},
		{"timekeeper",NULL,NULL,gfx_timekeeper},
		{"transform_tool",NULL,NULL,gfx_transform_tool},
		{"unselect",NULL,NULL,execute_command_gfx_unselect},
		{"update",NULL,NULL,execute_command_gfx_update},
		{"warp",NULL,NULL,execute_command_gfx_warp},
		{"write",NULL,NULL,execute_command_gfx_write},
#endif /* !defined (WINDOWS_DEV_FLAG) */
		{NULL,NULL,NULL,NULL}
	};
	struct Open_projection_window_data open_projection_window_data;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				i=0;
				/* change_identifier */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* create */
				(option_table[i]).user_data=command_data_void;
				i++;
#if !defined (WINDOWS_DEV_FLAG)
				/* data_tool */
				(option_table[i]).to_be_modified=(void *)1;
				(option_table[i]).user_data=command_data_void;
				i++;
				/* define */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* destroy */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* draw */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* edit */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* element_creator */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* element_tool */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* erase */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* evaluate */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* export */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* filter */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* grab_frame */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* list */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* modify */
				(option_table[i]).user_data=command_data_void;
				i++;
#if defined (SGI_MOVIE_FILE)
				/* movie */
				(option_table[i]).user_data=command_data_void;
				i++;
#endif /* defined (SGI_MOVIE_FILE) */
				/* node_tool */
				(option_table[i]).to_be_modified=(void *)0;
				(option_table[i]).user_data=command_data_void;
				i++;
				/* print */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* project */
				(option_table[i]).to_be_modified= &(command_data->projection_window);
				open_projection_window_data.user_interface=command_data->user_interface;
				open_projection_window_data.fe_field_manager=
					command_data->fe_field_manager;
				open_projection_window_data.element_manager=
					command_data->element_manager;
				open_projection_window_data.element_group_manager=
					command_data->element_group_manager;
				open_projection_window_data.spectrum_manager=
					command_data->spectrum_manager;
				open_projection_window_data.default_spectrum=
					command_data->default_spectrum;
				(option_table[i]).user_data= &open_projection_window_data;
				i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
				/* read */
				(option_table[i]).user_data=command_data_void;
				i++;
#if !defined (WINDOWS_DEV_FLAG)
				/* select */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* set */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* smooth */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* timekeeper */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* transform_tool */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* unselect */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* update */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* warp */
				(option_table[i]).user_data=command_data_void;
				i++;
				/* write */
				(option_table[i]).user_data=command_data_void;
				i++;
#endif /* !defined (WINDOWS_DEV_FLAG) */
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("gfx",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_cm(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 October 2001

DESCRIPTION :
Executes a cm (back end) command.
==============================================================================*/
{
	char *current_token,*prompt;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_cm);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
#if defined (LINK_CMISS)
				if (CMISS)
				{
					/* somehow extract the whole command */
					return_code=CMISS_connection_process_command(&CMISS,
						state->command_string);
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						display_message(ERROR_MESSAGE,
							"execute_command_cm.	No CMISS connection");
						return_code=0;
					}
					else
					{
						return_code=1;
					}
				}
#else /* defined (LINK_CMISS) */
				display_message(ERROR_MESSAGE,"execute_command_cm.	Define LINK_CMISS");
				return_code=0;
#endif /* defined (LINK_CMISS) */
#if defined (OLD_CODE)
				if (open_socket(command_data->command_window,
					command_data->basis_manager,command_data->node_manager,
					command_data->element_group_manager,command_data->node_group_manager,
					&(command_data->prompt_window),command_data->user_interface))
				{
#if !defined (WINDOWS_DEV_FLAG)
					write_socket(state->command_string,CONN_ID1);
#endif /* !defined (WINDOWS_DEV_FLAG) */
					return_code=1;
				}
				else
				{
					return_code=0;
				}
#endif /* defined (OLD_CODE) */
			}
			else
			{
				if (prompt=(char *)prompt_void)
				{
					set_command_prompt(prompt,command_data->command_window);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cm.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cm.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cm */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_connection(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 3 October 1996

DESCRIPTION :
Executes a CONNECTION command.
???RC Obsolete.
==============================================================================*/
{
	char *current_token,*host_name;
	int port_number,return_code;

	ENTER(execute_command_connection);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				set_host(current_token);
				shift_Parse_state(state,1);
				if (current_token=state->current_token)
				{
					set_port(atoi(current_token));
				}
				return_code=1;
			}
			else
			{
				if (get_host(&host_name))
				{
					display_message(INFORMATION_MESSAGE," HOSTNAME[%s]",host_name);
					DEALLOCATE(host_name);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," HOSTNAME[none]");
				}
				if (get_port(&port_number))
				{
					display_message(INFORMATION_MESSAGE," PORT_ADDRESS[%d]",port_number);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," PORT_ADDRESS[none]");
				}
				return_code=1;
			}
		}
		else
		{
			if (get_port(&port_number)&&get_host(&host_name))
			{
				display_message(INFORMATION_MESSAGE,"host name : %s\n",host_name);
				display_message(INFORMATION_MESSAGE,"port address : %d\n",port_number);
				DEALLOCATE(host_name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not get connection information");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cm.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_connection */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
#if defined (UNEMAP)
static int execute_command_unemap_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Executes a UNEMAP OPEN command.
==============================================================================*/
{	
	char *current_token,*default_torso_group_name;
	int return_code;
	Dimension window_width=0,window_height=0;
	struct Cmiss_command_data *command_data;
	struct Colour colour;
	struct FE_field *map_fit_field;
	struct Graphical_material *electrode_selected_material;
	struct GROUP(FE_element) *torso_element_group;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct System_window *system;
	struct System_window_data
	{
		Position x;
		Position y;
	} system_window_data;
	struct Standard_torso_defaults
	{
		char *standard_torso_file;
	};
	struct Standard_torso_defaults standard_torso_defaults;
#define XmNstandardTorso "standardTorso"
#define XmCStandardTorso "StandardTorso"

	static XtResource resources[]=
	{
		{
			XmNstandardTorso,
			XmCStandardTorso,
			XmRString,
			sizeof(char *),
			XtOffsetOf(struct Standard_torso_defaults,standard_torso_file),
			XmRString,
			""
		}
	};
	static XtResource system_window_resources[]=
	{
		{
			XmNx,
			XmCPosition,
			XmRPosition,
			sizeof(Position),
			XtOffsetOf(struct System_window_data,x),
			XmRImmediate,
			(XtPointer) -1
		},
		{
			XmNy,
			XmCPosition,
			XmRPosition,
			sizeof(Position),
			XtOffsetOf(struct System_window_data,y),
			XmRImmediate,
			(XtPointer) -1
		}
	};
	Widget shell;


	ENTER(execute_command_unemap_open);
	map_fit_field=(struct FE_field *)NULL;
	default_torso_group_name=(char *)NULL;
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	electrode_selected_material=(struct Graphical_material *)NULL;		
	torso_element_group=(struct GROUP(FE_element) *)NULL;
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{	
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				/* create material "electrode_selected" to be bright white for highlighting
					 electrode graphics */
				if(!(FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)
					("electrode_selected",command_data->graphical_material_manager)))
				{
					if (electrode_selected_material=CREATE(Graphical_material)("electrode_selected"))
					{
						colour.red=1.0;
						colour.green=1.0;
						colour.blue=1.0;
						Graphical_material_set_ambient(electrode_selected_material,&colour);
						Graphical_material_set_diffuse(electrode_selected_material,&colour);
						/* ACCESS so can never be destroyed */
						ACCESS(Graphical_material)(electrode_selected_material);
						if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(electrode_selected_material,
							command_data->graphical_material_manager))
						{
							DEACCESS(Graphical_material)(&electrode_selected_material);
						}
					}		
				}	
				/* create the unemap_package */
				if(!(command_data->unemap_package))
				{
					computed_field_manager=Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
					command_data->unemap_package = CREATE(Unemap_package)(
						command_data->fe_field_manager,command_data->element_group_manager,
						command_data->node_manager,command_data->data_manager,
						command_data->data_group_manager,
						command_data->node_group_manager,command_data->basis_manager,
						command_data->element_manager,computed_field_manager,
						command_data->interactive_tool_manager, 
						command_data->node_selection);
				}
				if(command_data->unemap_package)
				{
					if (!(system=command_data->unemap_system_window))
					{
						/* create a shell */
						if (shell=XtVaCreatePopupShell("system_window_shell",
							topLevelShellWidgetClass,
							command_data->user_interface->application_shell,
							XmNallowShellResize,False,NULL))
						{
							if (system=create_System_window(shell,close_emap,
								command_data->default_time_keeper,command_data->user_interface 
								,command_data->unemap_package,	
								command_data->element_point_ranges_selection,
								command_data->element_selection,
								command_data->node_selection,
								command_data->data_selection,
								command_data->texture_manager,
								command_data->interactive_tool_manager,
								command_data->scene_manager,
								command_data->light_model_manager,
								command_data->light_manager,
								command_data->spectrum_manager,
								command_data->graphical_material_manager,
								command_data->data_manager,
								command_data->glyph_list,						
								command_data->default_graphical_material,
								command_data->computed_field_package,
								command_data->default_light,
								command_data->default_light_model
																							))
							{
								command_data->unemap_system_window=system;
								create_Shell_list_item(&(system->window_shell),
									command_data->user_interface);
								XtAddCallback(system->window_shell,XmNdestroyCallback,
									close_emap,(XtPointer)system);
								/* manage the system window */
								XtManageChild(system->window);
								/* realize the system window shell */
								XtRealizeWidget(system->window_shell);
								/* determine placement */
								XtVaGetValues(system->window_shell,
									XmNwidth,&window_width,
									XmNheight,&window_height,
									NULL);
								/* Do all this to allow backward compatibility but still allow the
									 resources to be set */
								system_window_data.x = -1; /* These defaults match with the */
								system_window_data.y = -1; /* default resources above */
								XtVaGetApplicationResources(system->window_shell,
									&system_window_data,system_window_resources,
									XtNumber(system_window_resources),NULL);
								if (system_window_data.x == -1)
								{
									system_window_data.x = ((command_data->user_interface->
										screen_width)-window_width)/2;
								}
								if (system_window_data.y == -1)
								{
									system_window_data.y = ((command_data->user_interface->
										screen_height)-window_height)/2;
								}
								XtVaSetValues(system->window_shell,
									XmNx, system_window_data.x,
									XmNy, system_window_data.y,
									XmNmappedWhenManaged, True,
									NULL);
								return_code=1;
								/* turn off graphics in default scene, as updating these slows things down */
								/* for unemap 3d window */
								Scene_set_graphical_element_mode(command_data->default_scene,
									GRAPHICAL_ELEMENT_EMPTY,
									Computed_field_package_get_computed_field_manager(
										command_data->computed_field_package),
									command_data->element_manager,
									command_data->element_group_manager,
									command_data->fe_field_manager,
									command_data->node_manager,
									command_data->node_group_manager,
									command_data->data_manager,
									command_data->data_group_manager,
									command_data->element_point_ranges_selection,
									command_data->element_selection,
									command_data->node_selection,
									command_data->data_selection,
									command_data->user_interface);								
								/* create and store the map fit field  */
								map_fit_field=create_mapping_type_fe_field("fit",
									command_data->fe_field_manager);
								set_unemap_package_map_fit_field(command_data->unemap_package,
									map_fit_field);
								/* get the location of the default_torso file from Xresoures*/
								standard_torso_defaults.standard_torso_file= "";			
								XtVaGetApplicationResources(system->window_shell,
									&standard_torso_defaults,resources,XtNumber(resources),NULL);
								/* do nothing if no default torso file specified */
								if(strcmp(standard_torso_defaults.standard_torso_file,""))
								{									
									/* read in the default torso node and element groups */
									if(read_FE_node_and_elem_groups_and_return_name_given_file_name(
										standard_torso_defaults.standard_torso_file,
										command_data->fe_field_manager,command_data->node_manager,
										command_data->element_manager,command_data->node_group_manager,
										command_data->data_group_manager,
										command_data->element_group_manager,command_data->basis_manager,
										&default_torso_group_name))
									{
										/* offset default torso  node and element groups */
										offset_FE_node_and_element_identifiers_in_group(
											default_torso_group_name,(INT_MAX/2),
											command_data->node_manager,command_data->element_manager,
											command_data->node_group_manager,
											command_data->element_group_manager);									
										/*put in name unemap_package*/
										set_unemap_package_default_torso_name(command_data->unemap_package,
											default_torso_group_name);
										/* define the fit field on  the defaut torso*/
										torso_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),
											name)(default_torso_group_name,command_data->element_group_manager);
										define_fit_field_at_quad_elements_and_nodes(torso_element_group,
											map_fit_field,command_data->basis_manager,
											command_data->element_manager,command_data->node_manager);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Can't find standardTorso specified in Cmgui/Unemap file");
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"execute_command_unemap_open.  Could not create unemap_system_window");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_unemap_open."
								"  Could not create unemap_system_window shell");
							return_code=0;
						}
					}
					if (system&&return_code)
					{						
						/* pop up the system window shell */
						XtPopup(system->window_shell,XtGrabNone);
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_unemap_open. Couldn't create unemap_package");
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_unemap_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_unemap_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_unemap_open */
#endif /* defined (UNEMAP) */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_unemap(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 9 December 1996

DESCRIPTION :
Executes a UNEMAP command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
#if defined (UNEMAP)
		{"open",NULL,NULL,execute_command_unemap_open},
#endif /* defined (UNEMAP) */
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_unemap);
	/* check argument */
	if (state)
	{
#if defined (UNEMAP)
		(option_table[0]).user_data=command_data_void;
		(option_table[1]).user_data=command_data_void;
		(option_table[1]).to_be_modified=prompt_void;
#else
		(option_table[0]).user_data=command_data_void;
		(option_table[0]).to_be_modified=prompt_void;
#endif /* defined (UNEMAP) */
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_unemap.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_unemap */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (CELL)
static int execute_command_cell_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a CELL OPEN command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	
	ENTER(execute_command_cell_open);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				if (!(cell_interface=command_data->cell_interface))
				{
					/* add Cell 3D */
					if (cell_interface = CREATE(Cell_interface)(
						command_data->any_object_selection,
						&(command_data->background_colour),
						command_data->default_graphical_material,
						command_data->default_light,
						command_data->default_light_model,
						command_data->default_scene,
						command_data->default_spectrum,
						command_data->default_time_keeper,
						command_data->graphics_object_list,
						command_data->glyph_list,
						command_data->interactive_tool_manager,
						command_data->light_manager,
						command_data->light_model_manager,
						command_data->graphical_material_manager,
						command_data->scene_manager,
						command_data->spectrum_manager,
						command_data->texture_manager,
						command_data->user_interface,
						close_cell_window
#if defined (CELL_DISTRIBUTED)
						,command_data->element_point_ranges_selection,
						command_data->computed_field_package,
						command_data->element_manager,
						command_data->element_group_manager,
						command_data->fe_field_manager
#endif /* defined (CELL_DISTRIBUTED) */
						))
					{
						command_data->cell_interface=cell_interface;
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"execute_command_cell_open.  "
							"Could not create the cell_interface");
					}
				}
        else
        {
          /* Cell already exists, so pop it up */
          return_code = Cell_interface_pop_up(command_data->cell_interface);
        }
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_open */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_close(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL CLOSE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
  
	ENTER(execute_command_cell_close);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
        if (cell_interface=command_data->cell_interface)
        {
          return_code = DESTROY(Cell_interface)(&cell_interface);
          command_data->cell_interface = (struct Cell_interface *)NULL;
        }
        else
        {
          display_message(ERROR_MESSAGE,"execute_command_close.  "
            "Missing Cell interface");
          return_code = 0;
        }
      }
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_close.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_close.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_close() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_read_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL READ MODEL command.
==============================================================================*/
{
	char *current_token,*file_name;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;
  
	ENTER(execute_command_cell_read_model);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
      file_name=(char *)NULL;
      while (entry->option)
      {
        entry->to_be_modified= &file_name;
        entry++;
      }
      entry->to_be_modified= &file_name;
      if (return_code=process_multiple_options(state,
        command_data->set_file_name_option_table))
      {
        if (!((current_token=state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
        {
          if (!file_name)
          {
            if (!(file_name = confirmation_get_read_filename(".cell.xml",
              command_data->user_interface)))
            {
              return_code = 0;
            }
          }
          if (file_name)
          {
            if (check_suffix(&file_name,".cell.xml"))
            {
              if (cell_interface=command_data->cell_interface)
              {
                return_code = Cell_interface_read_model(cell_interface,
                  file_name);
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "execute_command_cell_read_model. "
                  "Missing Cell interface");
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "execute_command_cell_read_model. "
                "Invalid file name: %s",file_name);
              return_code = 0;
            }
            DEALLOCATE(file_name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_read_model. "
              "Unable to get the file name");
            return_code = 0;
          }
        }
        else
        {
          /* no help */
          return_code=1;
        }
      }
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_read_model.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_read_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_read_model */
#endif /* defined (CELL) */
        
#if defined (CELL)
static int execute_command_cell_read(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Executes a CELL READ command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_read);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
		option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"model",NULL,
      command_data_void,execute_command_cell_read_model);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_read */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_write_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL WRITE MODEL command.
==============================================================================*/
{
	char *current_token,*file_name;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;
  
	ENTER(execute_command_cell_write_model);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
      file_name=(char *)NULL;
      while (entry->option)
      {
        entry->to_be_modified= &file_name;
        entry++;
      }
      entry->to_be_modified= &file_name;
      if (return_code=process_multiple_options(state,
        command_data->set_file_name_option_table))
      {
        if (!((current_token=state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
        {
          if (!file_name)
          {
            if (!(file_name = confirmation_get_write_filename(".cell.xml",
              command_data->user_interface)))
            {
              return_code = 0;
            }
          }
          if (file_name)
          {
            if (check_suffix(&file_name,".cell.xml"))
            {
              if (cell_interface=command_data->cell_interface)
              {
                return_code = Cell_interface_write_model(cell_interface,
                  file_name);
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "execute_command_cell_write_model. "
                  "Missing Cell interface");
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "execute_command_cell_write_model. "
                "Invalid file name: %s",file_name);
              return_code = 0;
            }
            DEALLOCATE(file_name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_write_model. "
              "Unable to get the file name");
            return_code = 0;
          }
        }
        else
        {
          /* no help */
          return_code=1;
        }
      }
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_write_model.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_write_model.  Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* execute_command_cell_write_model */
#endif /* defined (CELL) */
        
#if defined (CELL)
static int execute_command_cell_write_ipcell(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL WRITE IPCELL command.
==============================================================================*/
{
	char *current_token,*file_name;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry *entry;
  
	ENTER(execute_command_cell_write_ipcell);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void)&&
			(entry=command_data->set_file_name_option_table))
		{
      file_name=(char *)NULL;
      while (entry->option)
      {
        entry->to_be_modified= &file_name;
        entry++;
      }
      entry->to_be_modified= &file_name;
      if (return_code=process_multiple_options(state,
        command_data->set_file_name_option_table))
      {
        if (!((current_token=state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
        {
          if (!file_name)
          {
            if (!(file_name = confirmation_get_write_filename(".ipcell",
              command_data->user_interface)))
            {
              return_code = 0;
            }
          }
          if (file_name)
          {
            if (check_suffix(&file_name,".ipcell"))
            {
              if (cell_interface=command_data->cell_interface)
              {
                return_code = Cell_interface_write_model_to_ipcell_file(
                  cell_interface,file_name);
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "execute_command_cell_write_ipcell. "
                  "Missing Cell interface");
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "execute_command_cell_write_ipcell. "
                "Invalid file name: %s",file_name);
              return_code = 0;
            }
            DEALLOCATE(file_name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_write_ipcell. "
              "Unable to get the file name");
            return_code = 0;
          }
        }
        else
        {
          /* no help */
          return_code=1;
        }
      }
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_write_ipcell.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_write_ipcell.  Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* execute_command_cell_write_ipcell */
#endif /* defined (CELL) */
        
#if defined (CELL)
static int execute_command_cell_write(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL WRITE command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_write);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
		option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"model",NULL,
      command_data_void,execute_command_cell_write_model);
    Option_table_add_entry(option_table,"ipcell",NULL,
      command_data_void,execute_command_cell_write_ipcell);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_write.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_write */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_XMLParser_properties(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL LIST XMLPARSER_PROPERTIES command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_XMLParser_properties);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          /* ??? Should just do a get_XMLParser_properties and then do the
           * listing here ???
           */
          return_code =
            Cell_interface_list_XMLParser_properties(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_XMLParser_properties.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_XMLParser_properties.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_XMLParser_properties.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_XMLParser_properties() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_set_XMLParser_properties(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Executes a CELL SET XMLPARSER_PROPERTIES command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  static struct Option_table *option_table;
  int *XMLParser_properties,i;

	ENTER(cell_set_XMLParser_properties);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    /* Leave this check here as we require an existing Cell interface to be
     * able to get the default values for the option table
     */
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
      strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          /* initialise defaults */
          if (XMLParser_properties =
            Cell_interface_get_XMLParser_properties(cell_interface))
          {
            option_table = CREATE(Option_table)();
            i = 0;
            /* validation */
            Option_table_add_entry(option_table,"do_validation",
              XMLParser_properties+i,NULL,set_int);
            i++;
            /* namespaces */
            Option_table_add_entry(option_table,"do_namespaces",
              XMLParser_properties+i,NULL,set_int);
            i++;
            /* entity expansion */
            Option_table_add_entry(option_table,"do_expand",
              XMLParser_properties+i,NULL,set_int);
            if (return_code = Option_table_multi_parse(option_table,state))
            {
              if (!Cell_interface_set_XMLParser_properties(cell_interface,
                XMLParser_properties))
              {
                display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
                  "Unable to set the current XML parser properties");
                return_code=0;
              }
            }
            DESTROY(Option_table)(&option_table);
            DEALLOCATE(XMLParser_properties);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
              "Unable to get the current XML parser properties");
            return_code=0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_set_XMLParser_properties.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help ?? */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_set_XMLParser_properties() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_set_variable_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Executes a CELL SET VARIABLE_VALUE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char *component_name,*variable_name,*value_string;

	ENTER(cell_set_variable_value);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      component_name = (char *)NULL;
      Option_table_add_entry(option_table,"component",&component_name,
        (void *)1,set_name);
      variable_name = (char *)NULL;
      Option_table_add_entry(option_table,"name",&variable_name,
        (void *)1,set_name);
      value_string = (char *)NULL;
      Option_table_add_entry(option_table,"value",&value_string,
        (void *)1,set_name);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
            Cell_interface_set_variable_value_from_string(cell_interface,
              component_name,variable_name,value_string);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_set_variable_value.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
      if (component_name)
      {
        DEALLOCATE(component_name);
      }
      if (variable_name)
      {
        DEALLOCATE(variable_name);
      }
      if (value_string)
      {
        DEALLOCATE(value_string);
      }
      DESTROY(Option_table)(&option_table);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_set_variable_value.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_variable_value.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_set_variable_value() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_set_calculate(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Executes a CELL SET CALCULATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  float Tstart,Tend,dT,tabT;
  char *model_routine_name,*model_dso_name,*intg_routine_name,*intg_dso_name,
    *data_file_name;

	ENTER(cell_set_calculate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      if (!((state->current_token)&&
        !(strcmp(PARSER_HELP_STRING,state->current_token)&&
          strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
      {
        if (cell_interface = command_data->cell_interface)
        {
          option_table = CREATE(Option_table)();
          Tstart = Cell_interface_get_start_time(cell_interface);
          Option_table_add_entry(option_table,"start_time",&Tstart,
            NULL,set_float);
          Tend = Cell_interface_get_end_time(cell_interface);
          Option_table_add_entry(option_table,"end_time",&Tend,
            NULL,set_float);
          dT = Cell_interface_get_dt(cell_interface);
          Option_table_add_entry(option_table,"dt",&dT,
            NULL,set_float);
          tabT = Cell_interface_get_tabt(cell_interface);
          Option_table_add_entry(option_table,"tab",&tabT,
            NULL,set_float);
          model_routine_name =
            Cell_interface_get_model_routine_name(cell_interface);
          Option_table_add_entry(option_table,"model_routine",
            &model_routine_name,(void *)1,set_name);
          model_dso_name =
            Cell_interface_get_model_dso_name(cell_interface);
          Option_table_add_entry(option_table,"model_dso",
            &model_dso_name,NULL,set_file_name);
          intg_routine_name =
            Cell_interface_get_intg_routine_name(cell_interface);
          Option_table_add_entry(option_table,"integrator_routine",
            &intg_routine_name,(void *)1,set_name);
          intg_dso_name =
            Cell_interface_get_intg_dso_name(cell_interface);
          Option_table_add_entry(option_table,"integrator_dso",
            &intg_dso_name,NULL,set_file_name);
          data_file_name =
            Cell_interface_get_data_file_name(cell_interface);
          Option_table_add_entry(option_table,"data_file",
            &data_file_name,NULL,set_file_name);
          if (return_code = Option_table_multi_parse(option_table,state))
          {
            /* check for essential parameters */
            if (model_routine_name && intg_routine_name && (Tstart <= Tend) &&
              (dT > 0) && (tabT > 0))
            {
              return_code = Cell_interface_set_calculate(cell_interface,Tstart,
                Tend,dT,tabT,model_routine_name,model_dso_name,
                intg_routine_name,intg_dso_name,data_file_name);
            }
            else
            {
              display_message(ERROR_MESSAGE,"cell_set_calculate.  "
                "Invalid parameters");
              return_code = 0;
            }
          }
          DESTROY(Option_table)(&option_table);
          if (model_routine_name)
          {
            DEALLOCATE(model_routine_name);
          }
          if (model_dso_name)
          {
            DEALLOCATE(model_dso_name);
          }
          if (intg_routine_name)
          {
            DEALLOCATE(intg_routine_name);
          }
          if (intg_dso_name)
          {
            DEALLOCATE(intg_dso_name);
          }
          if (data_file_name)
          {
            DEALLOCATE(data_file_name);
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_set_calculate.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        /* ??? Is this a good way to get the help out ???
         */
        option_table = CREATE(Option_table)();
        Tstart = 0.0;
        Option_table_add_entry(option_table,"start_time",&Tstart,
          NULL,set_float);
        Tend = 0.0;
        Option_table_add_entry(option_table,"end_time",&Tend,
          NULL,set_float);
        dT = 0.0;
        Option_table_add_entry(option_table,"dt",&dT,
          NULL,set_float);
        tabT = 0.0;
        Option_table_add_entry(option_table,"tab",&tabT,
          NULL,set_float);
        model_routine_name = (char *)NULL;
        Option_table_add_entry(option_table,"model_routine",
          &model_routine_name,(void *)1,set_name);
        model_dso_name = (char *)NULL;
        Option_table_add_entry(option_table,"model_dso",
          &model_dso_name,NULL,set_file_name);
        intg_routine_name = (char *)NULL;
        Option_table_add_entry(option_table,"integrator_routine",
          &intg_routine_name,(void *)1,set_name);
        intg_dso_name = (char *)NULL;
        Option_table_add_entry(option_table,"integrator_dso",
          &intg_dso_name,NULL,set_file_name);
        data_file_name = (char *)NULL;
        Option_table_add_entry(option_table,"data_file",
          &data_file_name,NULL,set_file_name);
        return_code = Option_table_multi_parse(option_table,state);
        DESTROY(Option_table)(&option_table);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_set_calculate.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_calculate.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_set_calculate() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_copy_tags(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL LIST COPY_TAGS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_copy_tags);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          return_code =
            Cell_interface_list_copy_tags(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_copy_tags.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_copy_tags.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_copy_tags.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_list_copy_tags() */
#endif /* defined (CELL) */

#if defined (HOW_CAN_I_DO_THIS)
#if defined (CELL)
static int cell_set_copy_tags(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Executes a CELL SET COPY_TAGS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  static struct Option_table *option_table;
  int i;
  char **copy_tags;

	ENTER(cell_set_copy_tags);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
      strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          /* initialise defaults */
          if (copy_tags =
            Cell_interface_get_copy_tags(cell_interface))
          {
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
              "Unable to get the current XML parser properties");
            return_code=0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_set_XMLParser_properties.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help ?? */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_set_XMLParser_properties() */
#endif /* defined (CELL) */
#endif /* defined (HOW_CAN_I_DO_THIS) */

#if defined (CELL)
static int cell_list_ref_tags(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL LIST REF_TAGS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_ref_tags);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          return_code =
            Cell_interface_list_ref_tags(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_ref_tags.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_ref_tags.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_ref_tags.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_list_ref_tags() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_calculate(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Executes a CELL LIST CALCULATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_calculate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          return_code =
            Cell_interface_list_calculate(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_calculate.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_calculate.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_calculate.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_calculate() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_components(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Executes a CELL LIST COMPONENTS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char full;

	ENTER(cell_list_components);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      full = 0;
      Option_table_add_entry(option_table,"full",&full,NULL,set_char_flag);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
            Cell_interface_list_components(cell_interface,(int)full);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_list_components.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
      DESTROY(Option_table)(&option_table);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_list_components.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_components.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_components() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_variables(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Executes a CELL LIST VARIABLES command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char full,*component_name,*variable_name;

	ENTER(cell_list_variables);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      full = 0;
      Option_table_add_entry(option_table,"full",&full,NULL,set_char_flag);
      component_name = (char *)NULL;
      Option_table_add_entry(option_table,"component",&component_name,
        (void *)1,set_name);
      variable_name = (char *)NULL;
      Option_table_add_entry(option_table,"name",&variable_name,
        (void *)1,set_name);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (cell_interface = command_data->cell_interface)
        {
          if (!((state->current_token)&&
            !(strcmp(PARSER_HELP_STRING,state->current_token)&&
              strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
          {
            Cell_interface_list_variables(cell_interface,component_name,
              variable_name,(int)full);
          }
          else
          {
            /* no help */
            return_code = 1;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_variables.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      DESTROY(Option_table)(&option_table);
      if (component_name)
      {
        DEALLOCATE(component_name);
      }
      if (variable_name)
      {
        DEALLOCATE(variable_name);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_list_variables.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_variables.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_variables() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_hierarchy(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Executes a CELL LIST HIERARCHY command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char full,*name;

	ENTER(cell_list_hierarchy);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      full = 0;
      Option_table_add_entry(option_table,"full",&full,NULL,set_char_flag);
      name = (char *)NULL;
      Option_table_add_entry(option_table,"name",&name,(void *)1,set_name);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
            Cell_interface_list_hierarchy(cell_interface,(int)full,name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_list_hierarchy.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
      DESTROY(Option_table)(&option_table);
      if (name)
      {
        DEALLOCATE(name);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_list_hierarchy.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_hierarchy.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_hierarchy() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_list(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2904 April 2001

DESCRIPTION :
Executes a CELL LIST command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_list);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
    option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"XMLParser_properties",NULL,
      command_data_void,cell_list_XMLParser_properties);
    Option_table_add_entry(option_table,"copy_tags",NULL,
      command_data_void,cell_list_copy_tags);
    Option_table_add_entry(option_table,"ref_tags",NULL,
      command_data_void,cell_list_ref_tags);
    Option_table_add_entry(option_table,"components",NULL,
      command_data_void,cell_list_components);
    Option_table_add_entry(option_table,"variables",NULL,
      command_data_void,cell_list_variables);
    Option_table_add_entry(option_table,"hierarchy",NULL,
      command_data_void,cell_list_hierarchy);
    Option_table_add_entry(option_table,"calculate",NULL,
      command_data_void,cell_list_calculate);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_list.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_list() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_set(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Executes a CELL SET command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_set);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
    option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"XMLParser_properties",NULL,
      command_data_void,cell_set_XMLParser_properties);
    Option_table_add_entry(option_table,"variable_value",NULL,
      command_data_void,cell_set_variable_value);
    Option_table_add_entry(option_table,"calculate",NULL,
      command_data_void,cell_set_calculate);
#if defined (HOW_CAN_I_DO_THIS)
    Option_table_add_entry(option_table,"copy_tags",NULL,
      command_data_void,cell_set_copy_tags);
#endif /* defined (HOW_CAN_I_DO_THIS) */
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_set() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_calculate(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Executes a CELL SOLVE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
/*    struct Option_table *option_table; */
/*    char *data_file_name; */

	ENTER(execute_command_cell_calculate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
/*        option_table = CREATE(Option_table)(); */
/*        data_file_name = (char *)NULL; */
/*        Option_table_add_entry(option_table,"data_file",&data_file_name, */
/*          (void *)1,set_name); */
/*        if (return_code = Option_table_multi_parse(option_table,state)) */
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
/*              if (data_file_name) */
/*              { */
/*                return_code = Cell_interface_set_data_file_name( */
/*                  cell_interface,data_file_name); */
/*              } */
/*              if (return_code) */
            {
              return_code = Cell_interface_calculate_model(cell_interface);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_calculate.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
/*        DESTROY(Option_table)(&option_table); */
/*        if (data_file_name) */
/*        { */
/*          DEALLOCATE(data_file_name); */
/*        } */
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "execute_command_cell_calculate.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_calculate.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* execute_command_cell_calculate() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
    option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"open",NULL,command_data_void,
      execute_command_cell_open);
    Option_table_add_entry(option_table,"close",NULL,command_data_void,
      execute_command_cell_close);
    Option_table_add_entry(option_table,"list",NULL,command_data_void,
      execute_command_cell_list);
    Option_table_add_entry(option_table,"set",NULL,command_data_void,
      execute_command_cell_set);
    Option_table_add_entry(option_table,"read",NULL,command_data_void,
      execute_command_cell_read);
    Option_table_add_entry(option_table,"write",NULL,command_data_void,
      execute_command_cell_write);
    Option_table_add_entry(option_table,"calculate",NULL,command_data_void,
      execute_command_cell_calculate);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell */
#endif /* defined (CELL) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_assign(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 

DESCRIPTION :
Executes an ASSIGN command.
First implementation of a small subset of the assign command which exists 
in the "new interpreter".
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_assign);
	USE_PARAMETER(prompt_void);
	if (state)
	{
		if (state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				if (fuzzy_string_compare(state->current_token, "variable"))
				{
					shift_Parse_state(state,1);
					return_code=execute_assign_variable(state,(void *)NULL,
						(void *)NULL);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_assign:  Only \"assign variable VARIABLE_NAME value\" implemented currently");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					"\n      variable");
				/* By not shifting the parse state the rest of the help should come out */
				return_code=execute_assign_variable(state,(void *)NULL,
					(void *)NULL);
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				set_command_prompt("assign",command_data->command_window);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_assign.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_assign.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_assign */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_create(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 March 2001

DESCRIPTION :
Executes a CREATE command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"cm",NULL,NULL,gfx_create_cmiss},
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_create);
	/* check argument */
	if (state)
	{
		(option_table[0]).user_data=command_data_void;
		(option_table[1]).user_data=command_data_void;
		(option_table[1]).to_be_modified=prompt_void;
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_create */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (NO_HELP)
static int execute_command_help(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Executes a HELP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_help);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
#if !defined (HYPERTEXT_HELP) && !defined (NETSCAPE_HELP)
#if !defined (WINDOWS_DEV_FLAG)
					do_help(current_token);
#endif /* !defined (WINDOWS_DEV_FLAG) */
#else
					strcpy(global_temp_string,command_data->help_url);
					strcat(global_temp_string,current_token);
					strcat(global_temp_string,"/");
					do_help(global_temp_string,command_data->examples_directory,
						command_data->execute_command,command_data->user_interface);
#endif
				}
				else
				{
					display_message(INFORMATION_MESSAGE," WORD");
					return_code=1;
				}
			}
			else
			{
#if !defined (HYPERTEXT_HELP) && !defined (NETSCAPE_HELP)
#if !defined (WINDOWS_DEV_FLAG)
				do_help(" ",command_data->execute_command,command_data->user_interface);
#endif /* !defined (WINDOWS_DEV_FLAG) */
#else
				do_help(command_data->help_url,command_data->examples_directory,
					command_data->execute_command,command_data->user_interface);
#endif
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_help.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_help.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_help */
#endif /* !defined (NO_HELP) */

#if !defined (WINDOWS_DEV_FLAG)
#if defined (INCLUDE_XVG)
static int execute_command_imp_draw_number(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Executes a IMP DRAW_NUMBER command.
???DB.  Should be set_char_flag ?
???RC.  Is this used?
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(execute_command_imp_draw_number);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (data_draw_number)
			{
				data_draw_number=0;
			}
			else
			{
				data_draw_number=1;
			}
			return_code=1;
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp_draw_number */

static int execute_command_imp_load(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a IMP LOAD command.
???RC.  Is this used?
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(execute_command_imp_load);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			return_code=LoadFile(current_token);
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp_load */

static int execute_command_imp_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 February 1999

DESCRIPTION :
Executes a IMP OPEN command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_imp_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				/*???DB.  Does this have a return code */
				xvg_popup_interface(command_data->fe_field_manager,
					command_data->data_manager,command_data->data_group_manager,
					command_data->user_interface);
				return_code=1;
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_imp_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_imp_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp_open */
#endif /* defined (INCLUDE_XVG) */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_imp(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a IMP command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
#if defined (INCLUDE_XVG)
		{"draw_number",NULL,NULL,execute_command_imp_draw_number},
		{"load",NULL,NULL,execute_command_imp_load},
		{"open",NULL,NULL,execute_command_imp_open},
#endif /* defined (INCLUDE_XVG) */
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_imp);
	/* check argument */
	if (state)
	{
#if defined (INCLUDE_XVG)
		(option_table[0]).user_data=NULL;
		(option_table[1]).user_data=NULL;
		(option_table[2]).user_data=command_data_void;
		(option_table[3]).user_data=command_data_void;
		(option_table[3]).to_be_modified=prompt_void;
#else
		(option_table[0]).user_data=command_data_void;
		(option_table[0]).to_be_modified=prompt_void;
#endif /* defined (INCLUDE_XVG) */
		return_code=process_option(state,option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_imp.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_imp */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int execute_command_list_memory(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a LIST_MEMORY command.
==============================================================================*/
{
	char increment_counter, suppress_pointers;
	int count_number,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"increment_counter",NULL,NULL,set_char_flag},
		{"suppress_pointers",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_int}
	};

	ENTER(execute_command_list_memory);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		count_number=0;
		increment_counter = 0;
		suppress_pointers = 0;
		(option_table[0]).to_be_modified= &increment_counter;
		(option_table[1]).to_be_modified= &suppress_pointers;
		(option_table[2]).to_be_modified= &count_number;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (suppress_pointers)
			{
				return_code=list_memory(count_number, /*show_pointers*/0,
					increment_counter, /*show_structures*/0);
			}
			else
			{
				return_code=list_memory(count_number, /*show_pointers*/1,
					increment_counter, /*show_structures*/1);
			}
		} /* parse error, help */
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_list_memory.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_list_memory */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_open_comfile(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Executes a OPEN COMFILE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_open_comfile);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=open_comfile(state,command_data->execute_command,
				command_data->set_file_name_option_table,command_data->user_interface);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open_comfile.  Missing command_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_open_comfile.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open_comfile */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_open_menu(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Executes a OPEN MENU command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_open_menu);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=open_menu(state,command_data->execute_command,
				command_data->set_file_name_option_table,command_data->user_interface);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open_menu.  Missing command_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open_menu.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open_menu */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a READ command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"comfile",NULL,NULL,open_comfile},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;

	ENTER(execute_command_read);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=1;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.file_extension=".com";
				open_comfile_data.user_interface=command_data->user_interface;
				(option_table[0]).user_data=(void *)&open_comfile_data;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("read",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_read.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_read */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a OPEN command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"comfile",NULL,NULL,open_comfile},
		{"menu",NULL,NULL,execute_command_open_menu},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;

	ENTER(execute_command_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=0;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.file_extension=".com";
				open_comfile_data.user_interface=command_data->user_interface;
				(option_table[0]).user_data=(void *)&open_comfile_data;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("open",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_quit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a QUIT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_quit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				User_interface_end_application_loop(command_data->user_interface);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_quit.  Invalid command_data");
				return_code=0;
			}
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_quit.	Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_quit */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_set_dir_example(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
==============================================================================*/
{
	char *current_token,*example_directory,*temp_char;
	int current_token_length,file_name_length,i,return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_set_dir_example);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					/* construct the example directory path */
					if ((current_token_length=strlen(current_token))>0)
					{
						file_name_length=
							1+(current_token_length*(current_token_length+3))/2;
						if (command_data->examples_directory)
						{
							file_name_length += strlen(command_data->examples_directory);
						}
						if (ALLOCATE(example_directory,char,file_name_length))
						{
							*example_directory='\0';
							if (command_data->examples_directory)
							{
								strcat(example_directory,command_data->examples_directory);
							}
							temp_char=example_directory+strlen(example_directory);
							for (i=1;i<=current_token_length;i++)
							{
								strncpy(temp_char,current_token,i);
								temp_char += i;
								*temp_char='/';
								temp_char++;
							}
							*temp_char='\0';
							DEALLOCATE(command_data->example_directory);
							command_data->example_directory=example_directory;
							/* send command to the back end */
							return_code=execute_command_cm(state,(void *)NULL,
								command_data_void);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_set_dir_example.  Insufficient memory");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_set_dir_example.  Invalid example name");
					}
#if defined (OLD_CODE)
					file_name_length=strlen(current_token)+2;
					if (command_data->examples_directory)
					{
						file_name_length += strlen(command_data->examples_directory);
					}
					if (ALLOCATE(example_directory,char,file_name_length))
					{
						*example_directory='\0';
						if (command_data->examples_directory)
						{
							strcat(example_directory,command_data->examples_directory);
						}
						strcat(example_directory,current_token);
						strcat(example_directory,"/");
						DEALLOCATE(command_data->example_directory);
						command_data->example_directory=example_directory;
						return_code=execute_command_cm(state,(void *)NULL,
							command_data_void);
					}
					else
					{
						display_message(ERROR_MESSAGE,
"execute_command_set_dir_example.  Insufficient memory for relative example directory");
						return_code=0;
					}
#endif /* defined (OLD_CODE) */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_set_dir_example.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," RELATIVE_EXAMPLE_DIRECTORY");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_set_dir_example.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set_dir_example */
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

#if !defined (WINDOWS_DEV_FLAG)
static int set_dir(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Executes a SET DIR command.
==============================================================================*/
{
	char *comfile_name, *directory_name, *example_directory, example_flag, *token;
	int file_name_length, return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,NULL,
			set_char_flag},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(set_dir);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (token = state->current_token)
			{
				directory_name = (char *)NULL;
				example_flag = 0;
				(option_table[0]).to_be_modified = &example_flag;
				(option_table[1]).to_be_modified = &directory_name;
				return_code=process_multiple_options(state,option_table);
				if (return_code)
				{
					if (example_flag)
					{
						if (directory_name)
						{							
							/* Lookup the example path */
							if (example_directory = 
								resolve_example_path(command_data->examples_directory, 
								directory_name, &comfile_name))
							{
								if (command_data->example_directory)
								{
									DEALLOCATE(command_data->example_directory);
								}
								command_data->example_directory=example_directory;
								if (command_data->example_comfile)
								{
									DEALLOCATE(command_data->example_comfile);
								}
								if (comfile_name)
								{
									command_data->example_comfile = comfile_name;
								}
								else
								{
									command_data->example_comfile = (char *)NULL;
								}
#if defined (PERL_INTERPRETER)
								/* Set the interpreter variable */
								interpreter_set_string("example", example_directory, 
									&return_code);
#endif /* defined (PERL_INTERPRETER) */
								
#if defined (LINK_CMISS)
								if (CMISS)
								{
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
#endif /* defined (LINK_CMISS) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Unable to resolve example path.");
								return_code = 0;
							}
#if defined (OLD_CODE)
							/* construct the example directory path */
							if ((directory_name_length=strlen(directory_name))>0)
							{
								file_name_length=
									1+(directory_name_length*(directory_name_length+3))/2;
								if (command_data->examples_directory)
								{
									file_name_length += strlen(command_data->examples_directory);
								}
								if (ALLOCATE(example_directory,char,file_name_length))
								{
									*example_directory='\0';
									if (command_data->examples_directory)
									{
										strcat(example_directory,command_data->examples_directory);
									}
									temp_char=example_directory+strlen(example_directory);
									for (i=1;i<=directory_name_length;i++)
									{
										strncpy(temp_char,directory_name,i);
										temp_char += i;
										*temp_char='/';
										temp_char++;
									}
									*temp_char='\0';
									DEALLOCATE(command_data->example_directory);
									command_data->example_directory=example_directory;
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_dir.  Insufficient memory");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Invalid example name");
							}
#endif /* defined (OLD_CODE) */
						}
						else
						{
							file_name_length = 1;
							if (command_data->examples_directory)
							{
								file_name_length += strlen(command_data->examples_directory);
							}
							if (ALLOCATE(example_directory,char,file_name_length))
							{
								*example_directory='\0';
								if (command_data->examples_directory)
								{
									strcat(example_directory,command_data->examples_directory);
								}
								DEALLOCATE(command_data->example_directory);
								command_data->example_directory=example_directory;
#if defined (LINK_CMISS)
								if (CMISS)
								{
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
#endif /* defined (LINK_CMISS) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Insufficient memory");
							}
						}
					} 
					else
					{
						if(chdir(directory_name))
						{
							display_message(ERROR_MESSAGE,
								"set_dir.  Unable to change to directory %s",
								directory_name);
							
						}
						return_code=execute_command_cm(state,(void *)NULL,
							command_data_void);
					}
				}
				if (directory_name)
				{
					DEALLOCATE(directory_name);
				}
			}
			else
			{
				set_command_prompt("set dir",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set_dir.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set_dir.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_dir */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"directory","dir",NULL,set_dir},
		{NULL,NULL,NULL,execute_command_cm}
	};

	ENTER(execute_command_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				(option_table[0]).user_data=command_data_void;
				(option_table[1]).user_data=command_data_void;
				return_code=process_option(state,option_table);
			}
			else
			{
				set_command_prompt("set",command_data->command_window);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_system(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
???RC Obsolete?
==============================================================================*/
{
	char *command,*current_token,*system_command;
	int return_code;

	ENTER(execute_command_system);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	return_code=0;
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				command=strstr(state->command_string,current_token);
				if (ALLOCATE(system_command,char,strlen(command)+1))
				{
					strcpy(system_command,command);
					parse_variable(&system_command);
					system(system_command);
					DEALLOCATE(system_command);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_system.  Insufficient memory");
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_system.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_system */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
static int execute_command_variable(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a VARIABLE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_variable);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (state->current_token)
		{
			return_code=execute_variable_command(state,(void *)NULL,
				(void *)NULL);
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				set_command_prompt("var",command_data->command_window);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_variable.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_variable.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_variable */
#endif /* !defined (WINDOWS_DEV_FLAG) */

/*
Global functions
----------------
*/
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
void execute_command(char *command_string,void *command_data_void, int *quit,
  int *error)
/*******************************************************************************
LAST MODIFIED : 28 March 2001

DESCRIPTION:
==============================================================================*/
{
#if !defined (WINDOWS_DEV_FLAG)
	char **token;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	int i,return_code;
	struct Cmiss_command_data *command_data;
#if !defined (WINDOWS_DEV_FLAG)
	static struct Modifier_entry option_table[]=
	{
		{"assign","assign",NULL,execute_command_assign},
#if defined (SELECT_DESCRIPTORS)
		{"attach",NULL,NULL,execute_command_attach},
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (CELL)
		{"cell","cell",NULL,execute_command_cell},
#endif /* defined (CELL) */
		{"command_window","command_window",NULL,modify_Command_window},
		{"create","create",NULL,execute_command_create},
#if defined (SELECT_DESCRIPTORS)
		{"detach",NULL,NULL,execute_command_detach},
#endif /* !defined (SELECT_DESCRIPTORS) */
		{"fem","fem",NULL,execute_command_cm},
		{"gen","gen",NULL,execute_command_cm},
		{"gfx",NULL,NULL,execute_command_gfx},
#if !defined (NO_HELP)
		{"help",NULL,NULL,execute_command_help},
#endif /* !defined (NO_HELP) */
		{"imp","imp",NULL,execute_command_imp},
		{"open",NULL,NULL,execute_command_open},
		{"quit",NULL,NULL,execute_command_quit},
		{"list_memory",NULL,NULL,execute_command_list_memory},
		{"read",NULL,NULL,execute_command_read},
		{"set","set",NULL,execute_command_set},
		{"system","system",NULL,execute_command_system},
		{"unemap","unemap",NULL,execute_command_unemap},
		{"var","var",NULL,execute_command_variable},
		{NULL,NULL,NULL,execute_command_cm}
	};
	struct Parse_state *state;
#endif /* !defined (WINDOWS_DEV_FLAG) */

	ENTER(execute_command);
	USE_PARAMETER(quit);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
#if !defined (WINDOWS_DEV_FLAG)
		if (state=create_Parse_state(command_string))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
#if defined (OLD_CODE)
				/* add command to command history */			  
				/*???RC put out processed tokens instead? */
				display_message(INFORMATION_MESSAGE,
					"%s\n", command_string);
#endif /* defined (OLD_CODE) */
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data->command_window);
				}
				else
				{
					/* assign */
					(option_table[i]).user_data=command_data_void;
					i++;
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (CELL)
					/* cell */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* defined (CELL) */
					/* command_window */
					(option_table[i]).user_data=command_data->command_window;
					i++;
					/* create */
					(option_table[i]).user_data=command_data_void;
					i++;
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* fem */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gen */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gfx */
					(option_table[i]).user_data=command_data_void;
					i++;
#if !defined (NO_HELP)
					/* help */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* !defined (NO_HELP) */
					/* imp */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* open */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* quit */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* list_memory */
					(option_table[i]).user_data=NULL;
					i++;
					/* read */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* set */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* system */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* unemap */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* var */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* default */
					(option_table[i]).user_data=command_data_void;
					i++;
					return_code=process_option(state,option_table);
				}
			}
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
#else
		return_code=1;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}

	*error = return_code;

	LEAVE;

} /* execute_command */

int cmiss_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
	int quit,return_code;
	struct Cmiss_command_data *command_data;

	ENTER(cmiss_execute_command);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
		if (command_data->command_window)
		{
			add_to_command_list(command_string,command_data->command_window);
		}
		quit = 0;

		interpret_command(command_string, (void *)command_data, 
		  &quit, &execute_command, &return_code);

		if (quit)
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Proper quit for cmgui not implemented yet");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#else /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
int cmiss_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION:
Execute a <command_string>. If there is a command
==============================================================================*/
{
#if !defined (WINDOWS_DEV_FLAG)
	char **token;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	int i,return_code;
	struct Cmiss_command_data *command_data;
#if !defined (WINDOWS_DEV_FLAG)
	static struct Modifier_entry option_table[]=
	{
		{"assign","assign",NULL,execute_command_assign},
		{"cell","cell",NULL,execute_command_cell},
		{"command_window","command_window",NULL,modify_Command_window},
		{"create","create",NULL,execute_command_create},
		{"fem","fem",NULL,execute_command_cm},
		{"gen","gen",NULL,execute_command_cm},
		{"gfx",NULL,NULL,execute_command_gfx},
#if !defined (NO_HELP)
		{"help",NULL,NULL,execute_command_help},
#endif /* !defined (NO_HELP) */
		{"imp","imp",NULL,execute_command_imp},
		{"open",NULL,NULL,execute_command_open},
		{"quit",NULL,NULL,execute_command_quit},
		{"list_memory",NULL,NULL,execute_command_list_memory},
		{"read",NULL,NULL,execute_command_read},
		{"set","set",NULL,execute_command_set},
		{"system","system",NULL,execute_command_system},
		{"unemap","unemap",NULL,execute_command_unemap},
		{"var","var",NULL,execute_command_variable},
		{NULL,NULL,NULL,execute_command_cm}
	};
	struct Parse_state *state;
#endif /* !defined (WINDOWS_DEV_FLAG) */

	ENTER(cmiss_execute_command);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
#if !defined (WINDOWS_DEV_FLAG)
		if (state=create_Parse_state(command_string))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* add command to command history */
				/*???RC put out processed tokens instead? */
				if (command_data->command_window)
				{
					add_to_command_list(command_string,command_data->command_window);
				}
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data->command_window);
				}
				else
				{
					/* assign */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* cell */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* command_window */
					(option_table[i]).user_data=command_data->command_window;
					i++;
					/* create */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* fem */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gen */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* gfx */
					(option_table[i]).user_data=command_data_void;
					i++;
#if !defined (NO_HELP)
					/* help */
					(option_table[i]).user_data=command_data_void;
					i++;
#endif /* !defined (NO_HELP) */
					/* imp */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* open */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* quit */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* list_memory */
					(option_table[i]).user_data=NULL;
					i++;
					/* read */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* set */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* system */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* unemap */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* var */
					(option_table[i]).user_data=command_data_void;
					i++;
					/* default */
					(option_table[i]).user_data=command_data_void;
					i++;
					return_code=process_option(state,option_table);
				}
			}
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
#else
		return_code=1;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#endif  /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing and entering. If there is no command_window, does nothing.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(cmiss_set_command);
	if (command_string&&
		(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (command_data->command_window)
		{
			return_code=Command_window_set_command_string(
				command_data->command_window,command_string);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmiss_set_command.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_set_command */

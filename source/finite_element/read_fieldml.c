/*******************************************************************************
FILE : read_fieldml.c

LAST MODIFIED : 19 February 2003

DESCRIPTION :
The function for importing finite element data into CMISS.
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
#if defined (HAVE_XML2)
#if defined (UNIX)
#include <ctype.h>
#endif /* defined (UNIX) */
#include <math.h>

#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_time.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/read_fieldml.h"
#include "general/debug.h"
#include "general/list.h"
#include "general/list_private.h"
#include "general/math.h"
#include "general/multi_range.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "general/object.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"


/*
Module types
------------
*/

enum Fieldml_label_type
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	FIELDML_LABEL_TYPE_UNKNOWN,
	FIELDML_LABEL_TYPE_LABEL,
	FIELDML_LABEL_TYPE_LABELS_TEMPLATE,
	FIELDML_LABEL_TYPE_LABEL_LOOKUP,
	FIELDML_LABEL_TYPE_FIELD_REF,
	FIELDML_LABEL_TYPE_COMPONENT_REF,
	FIELDML_LABEL_TYPE_BASIS_MAPPING,
	FIELDML_LABEL_TYPE_COEFFICIENTS,
	FIELDML_LABEL_TYPE_PRODUCT,
	FIELDML_LABEL_TYPE_ELEMENT_INTERPOLATION,
	FIELDML_LABEL_TYPE_NODE_LOOKUP,
	FIELDML_LABEL_TYPE_NODE_INDEX,
	FIELDML_LABEL_TYPE_ELEMENT_LOOKUP,
	FIELDML_LABEL_TYPE_FIELD_LOOKUP,
	FIELDML_LABEL_TYPE_COMPONENT_LOOKUP,
	FIELDML_LABEL_TYPE_ASSIGN_LABELS,
	FIELDML_LABEL_TYPE_NODE
}; /* enum Fieldml_label_type */

struct Fieldml_label_name
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *name;
	char *value;
	enum Fieldml_label_type type;
	struct LIST(Fieldml_label_name) *child_labels;

	/* For FIELDML_LABEL_TYPE_FIELD_REF */
	struct FE_field *field;

	/* For FIELDML_LABEL_TYPE_FIELD_REF in a node */
	/* When working on a node then the first time through all the labels are
		compiled into this node_field_creator and the children removed */
	struct FE_node_field_creator *node_field_creator;

	/* FIELDML_LABEL_TYPE_BASIS_MAPPING */
	struct FE_basis *basis;
	FE_element_field_component_modify modify;

	/* FIELDML_LABEL_TYPE_NODE_INDEX and FIELDML_LABEL_TYPE_ELEMENT_LOOKUP */
	int local_index;
	int current_node_number_of_derivatives;
	int current_node_number_of_versions;
	enum FE_nodal_value_type *current_node_derivative_types;

	/* FIELDML_LABEL_TYPE_NODE_INDEX, FIELDML_LABEL_TYPE_ELEMENT_LOOKUP and
	 FIELDML_LABEL_TYPE_NODE */
	struct FE_node *current_node;

	/* FIELDML_LABEL_TYPE_ELEMENT_LOOKUP */
	void *scale_factor_list_identifier;
	int scale_factor_list_size;
	int scale_factor_list_offset;

	/* FIELDML_LABEL_TYPE_ELEMENT_INTERPOLATION */
	struct FE_element *element_template;
	struct LIST(Fieldml_label_name) *element_template_node_index_list;	
	struct LIST(Fieldml_label_name) *element_template_element_lookup_list;	

	/* FIELDML_LABEL_TYPE_ASSIGN_LABELS */
	char *character_buffer;

	int access_count;
};

DECLARE_LIST_TYPES(Fieldml_label_name);
/* We need to maintain the order, so we do not want an indexed list */
FULL_DECLARE_LIST_TYPE(Fieldml_label_name);

struct Fieldml_sax_data
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_val;
	int fieldml_version;
	int fieldml_subversion;
	int unknown_depth; /* Record the depth of unknown entities so we can start
								 parsing again when we end */
	struct Cmiss_region *root_region;
	struct FE_region *current_fe_region, *root_fe_region;
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	struct LIST(Fieldml_label_name) *label_templates;
	struct LIST(Fieldml_label_name) *basis_mappings;
	struct LIST(Fieldml_label_name) *element_interpolations;
	/* For storing templates of nodes to speed up creating equivalent nodes */
	struct LIST(Fieldml_label_name) *node_label_templates;

	char *character_buffer;
	char *character_index;
	int buffer_length;
	int buffer_allocated_length;
	int expecting_characters;

	struct Fieldml_label_name *current_label_name;

	struct LIST(Fieldml_label_name) *label_name_stack;
	/* Only for defining a new field */
	struct FE_field *current_field;
	char *current_field_component_name;
	/* When referencing an existing field */
	struct FE_field *current_field_ref;

	struct FE_node *current_node;
	struct FE_node_field_creator *current_node_field_creator;
	int current_component_number;
	int current_component_has_value_defined;
	int current_component_number_of_versions;

	struct Fieldml_label_name *current_basis_mapping;

	struct FE_element *current_element;
	int current_element_number;
	struct FE_element_shape *current_element_shape;
	struct Fieldml_label_name *current_element_interpolation_ref;
	struct LIST(Fieldml_label_name) *current_element_labels;
	struct LIST(Fieldml_label_name) *element_lookup_list;
	struct LIST(Fieldml_label_name) *element_node_index_list;

	int number_of_faces;
	int *face_numbers;

	/* Flag to indicate that the node was created from a template and so
		should already have all the fields correctly defined */
	int fields_already_defined;

	struct LIST(Fieldml_label_name) *current_value_list;

	int number_of_scale_factor_lists;
	int scale_factor_list_offset;
	void **scale_factor_list_identifiers;
	int *scale_factor_list_sizes;

	struct FE_element_field_component **current_element_field_components;
	int current_element_field_component_nodes;
	struct Standard_node_to_element_map **current_element_field_standard_map;
	int current_element_field_standard_map_node_index;
	int current_element_field_standard_map_number_of_values;
	int *current_element_field_standard_map_indices;
	
	int current_scale_factor_list_offset;
	int current_scale_factor_list_size;

};

/*
Module functions
----------------
*/

PROTOTYPE_LIST_FUNCTIONS(Fieldml_label_name);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Fieldml_label_name,name, \
	char *);

static struct Fieldml_label_name *CREATE(Fieldml_label_name)(char *name,
	enum Fieldml_label_type type)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_label_name *label_name;

	ENTER(CREATE(Fieldml_label_name));
	if (name)
	{
		if (ALLOCATE(label_name,struct Fieldml_label_name,1)&&
			ALLOCATE(label_name->name,char,strlen(name)+1))
		{
			strcpy(label_name->name, name);
			label_name->value = (char *)NULL;
			label_name->type = type;
			label_name->child_labels = (struct LIST(Fieldml_label_name) *)NULL;
			label_name->field = (struct FE_field *)NULL;
			label_name->basis = (struct FE_basis *)NULL;
			label_name->modify = (FE_element_field_component_modify)NULL;
			label_name->node_field_creator = (struct FE_node_field_creator *)NULL;
			label_name->local_index = -1;
			label_name->scale_factor_list_identifier = NULL;
			label_name->scale_factor_list_size = 0;
			label_name->scale_factor_list_offset = 0;
			label_name->current_node = (struct FE_node *)NULL;
			label_name->current_node_number_of_derivatives = 0;
			label_name->current_node_derivative_types =
				(enum FE_nodal_value_type *)NULL;
			label_name->element_template = (struct FE_element *)NULL;
			label_name->element_template_node_index_list = 
				(struct LIST(Fieldml_label_name) *)NULL;
			label_name->element_template_element_lookup_list = 
				(struct LIST(Fieldml_label_name) *)NULL;
			label_name->character_buffer = (char *)NULL;
			label_name->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Fieldml_label_name).  Could not allocate memory for node field");
			DEALLOCATE(label_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Fieldml_label_name).  Invalid argument(s)");
		label_name=(struct Fieldml_label_name *)NULL;
	}
	LEAVE;

	return (label_name);
} /* CREATE(Fieldml_label_name) */

static int DESTROY(Fieldml_label_name)(struct Fieldml_label_name **label_name_address)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Frees the memory for the node field and sets <*label_name_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Fieldml_label_name *label_name;

	ENTER(DESTROY(Fieldml_label_name));
	if ((label_name_address)&&(label_name= *label_name_address))
	{
		if (0==label_name->access_count)
		{
			if (label_name->node_field_creator)
			{
				DESTROY(FE_node_field_creator)(&label_name->node_field_creator);
			}
			if (label_name->child_labels)
			{
				DESTROY(LIST(Fieldml_label_name))(&label_name->child_labels);
			}
			if (label_name->current_node_derivative_types)
			{
				DEALLOCATE(label_name->current_node_derivative_types);
			}
			if (label_name->element_template_node_index_list)
			{
				DESTROY(LIST(Fieldml_label_name))
					(&label_name->element_template_node_index_list);
			}
			if (label_name->element_template_element_lookup_list)
			{
				DESTROY(LIST(Fieldml_label_name))
					(&label_name->element_template_element_lookup_list);
			}
			if (label_name->value)
			{
				DEALLOCATE(label_name->value);
			}
			if (label_name->character_buffer)
			{
				DEALLOCATE(label_name->character_buffer);
			}
			DEALLOCATE(label_name->name);
			DEALLOCATE(*label_name_address);
		}
		else
		{
			*label_name_address=(struct Fieldml_label_name *)NULL;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Fieldml_label_name) */

DECLARE_OBJECT_FUNCTIONS(Fieldml_label_name)
DECLARE_LIST_FUNCTIONS(Fieldml_label_name)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Fieldml_label_name,name, \
	char *,strcmp)

static int Last_Fieldml_label_name_in_list_iterator(
   struct Fieldml_label_name *label_name, void *label_name_pointer_void)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_label_name **label_name_pointer;
	int return_code;

	ENTER(Last_Fieldml_label_name_in_list_iterator);
	if (label_name && (label_name_pointer =
		(struct Fieldml_label_name **)label_name_pointer_void))
	{
		*label_name_pointer = label_name;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Last_Fieldml_label_name_in_list_iterator.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Last_Fieldml_label_name_in_list_iterator */

static struct Fieldml_label_name *Last_Fieldml_label_name_in_list(
   struct LIST(Fieldml_label_name) *label_name_list)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_label_name *label_name;

	ENTER(CREATE(Fieldml_label_name));
	if (label_name_list)
	{
		label_name = (struct Fieldml_label_name *)NULL;
		/* Iterate through the list and we will be left with the last one */
		FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
			Last_Fieldml_label_name_in_list_iterator, (void *)&label_name,
			label_name_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Last_Fieldml_label_name_in_list.  Invalid argument(s)");
		label_name=(struct Fieldml_label_name *)NULL;
	}
	LEAVE;

	return (label_name);
} /* Last_Fieldml_label_name_in_list */

static int Fieldml_label_name_process_label_in_node(
   struct Fieldml_label_name *label, void *fieldml_data_void)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *component_name, *field_name;
	enum FE_nodal_value_type derivative_type;	
	enum Value_type value_type;
	int bytes_processed, j, k, length, node_number, number_of_components,
		number_of_values, return_code, version_number;
	struct FE_field *field;
	struct FE_node *node;
   struct Fieldml_label_name *current_assign_labels;
	struct Fieldml_sax_data *fieldml_data;
	struct FE_import_time_index *time_index;

	ENTER(Fieldml_label_name_process_label);

	if (fieldml_data = (struct Fieldml_sax_data *)fieldml_data_void)
	{
		return_code = 1;
		if (FIELDML_LABEL_TYPE_ASSIGN_LABELS == label->type)
		{
			fieldml_data->character_buffer = label->character_buffer;
			fieldml_data->buffer_length = strlen(fieldml_data->character_buffer);
			fieldml_data->character_index = label->character_buffer;
			if (current_assign_labels = FIND_BY_IDENTIFIER_IN_LIST
				(Fieldml_label_name,name)(label->name, fieldml_data->label_templates))
			{
				Fieldml_label_name_process_label_in_node(current_assign_labels,
					fieldml_data);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
					"Unable to find a labels template named %s.", label->name);
			}
			fieldml_data->character_index = (char *)NULL;
			fieldml_data->character_buffer = (char *)NULL;
			fieldml_data->buffer_length = 0;
		}
		else
		{
			if (FIELDML_LABEL_TYPE_FIELD_REF == label->type)
			{
				if (label->field)
				{
					if (!(fieldml_data->current_field_ref))
					{
						fieldml_data->current_field_ref = label->field;
						number_of_components = get_FE_field_number_of_components(label->field);
						if (!label->node_field_creator)
						{
							fieldml_data->current_node_field_creator = 
								CREATE(FE_node_field_creator)(number_of_components);
						}
						fieldml_data->current_component_number = 0;
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
							"Already referencing a field.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
						"Label is field ref type but does not have a field.");
				}
			}
			else if (FIELDML_LABEL_TYPE_COMPONENT_REF == label->type)
			{
				fieldml_data->current_component_has_value_defined = 0;
				fieldml_data->current_component_number_of_versions = 0;
				if (fieldml_data->current_field_ref)
				{
					if (component_name = get_FE_field_component_name(
							 fieldml_data->current_field_ref,
							 fieldml_data->current_component_number))
					{
						if (!(strcmp(component_name, label->name)))
						{
							/* This component is OK */
						}
						else
						{
							display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
								"Referenced component does not match the field.");
						}
						DEALLOCATE(component_name);
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
							"Unable to get component name.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
						"Not referencing a field when component encountered.");
				}
			}
			else if (FIELDML_LABEL_TYPE_LABEL == label->type)
			{
				if (!strncmp("version", label->name, 7))
				{
					if (sscanf(label->name, "version_%d", &version_number))
					{
						if ((fieldml_data->current_component_number_of_versions + 1)
							== version_number)
						{
							fieldml_data->current_component_number_of_versions++;
						}
						else
						{
							display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
								"Unexpected label for version number.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
							"Unable to parse a version label.");
					}
				}
			}
			if (!label->node_field_creator)
			{
				if (label->child_labels)
				{
					ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
					return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(Fieldml_label_name_process_label_in_node,
						fieldml_data_void, label->child_labels);
					REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
				}
				else
				{
					/* Try to find an equivalent cmgui type and then define that */
					if (STRING_TO_ENUMERATOR(FE_nodal_value_type)(label->name,
							&derivative_type))
					{
						if (derivative_type == FE_NODAL_VALUE)
						{
							if (!fieldml_data->current_component_has_value_defined)
							{
								fieldml_data->current_component_has_value_defined = 1;
							}
							else
							{
								if (fieldml_data->current_component_number_of_versions < 2)
								{
									display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
										"More than one nodal value in component.");
									return_code = 0;
								}
							}
						}
						else
						{
							if (fieldml_data->current_component_number_of_versions < 2)
							{
								FE_node_field_creator_define_derivative(
									fieldml_data->current_node_field_creator,
									fieldml_data->current_component_number, derivative_type);
							}
						}
					}
					else
					{
						if (!fieldml_data->current_component_has_value_defined)
						{
							fieldml_data->current_component_has_value_defined = 1;
						}
						else
						{
							if (fieldml_data->current_component_number_of_versions < 2)
							{
								FE_node_field_creator_define_derivative(
									fieldml_data->current_node_field_creator,
									fieldml_data->current_component_number,
									FE_NODAL_UNKNOWN);
							}
						}				
					}
				}
			}
			if (FIELDML_LABEL_TYPE_COMPONENT_REF == label->type)
			{
				if (fieldml_data->current_component_number_of_versions)
				{
					FE_node_field_creator_define_versions(
						fieldml_data->current_node_field_creator,
						fieldml_data->current_component_number,
						fieldml_data->current_component_number_of_versions);
				}
				fieldml_data->current_component_number++;
				if (!fieldml_data->current_component_has_value_defined)
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
						"No value defined for component %s.", label->name);
					return_code = 0;
				}
			}
			else if (FIELDML_LABEL_TYPE_FIELD_REF == label->type)
			{
				GET_NAME(FE_field)(label->field,&field_name);
				number_of_components = get_FE_field_number_of_components(label->field);
				if (!fieldml_data->fields_already_defined)
				{
					if (label->node_field_creator)
					{
						if (fieldml_data->current_node)
						{
							/* Then define this field on the node */
							define_FE_field_at_node(fieldml_data->current_node, label->field,
								(struct FE_time_sequence *)NULL, 
								label->node_field_creator);
						}
					}
					else
					{
						if (number_of_components == fieldml_data->current_component_number)
						{
							if (fieldml_data->current_node)
							{
								/* Then define this field on the node */
								define_FE_field_at_node(fieldml_data->current_node, label->field,
									(struct FE_time_sequence *)NULL, 
									fieldml_data->current_node_field_creator);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
								"Not all components of field %s were referenced.", label->name);
							return_code = 0;
						}
						if (fieldml_data->current_node_field_creator)
						{
							label->node_field_creator = fieldml_data->current_node_field_creator;
							/* We no longer require the child fields as they are compiled into
								the node field creator */
							DESTROY(LIST(Fieldml_label_name))(&label->child_labels);
							fieldml_data->current_node_field_creator = 
								(struct FE_node_field_creator *)NULL;
						}
					}
				}
				if (fieldml_data->current_node && fieldml_data->current_field_ref &&
					fieldml_data->character_buffer && fieldml_data->buffer_length)
				{
					node = fieldml_data->current_node;
					node_number = get_FE_node_identifier(node);
					field = fieldml_data->current_field_ref;
					time_index = (struct FE_import_time_index *)NULL;
					number_of_values = 0;
					for (j = 0 ; j < number_of_components ; j++)
					{
						number_of_values += get_FE_node_field_component_number_of_versions(
							node,field,j)*
							(1+get_FE_node_field_component_number_of_derivatives(node,
								field,j));
					}
					value_type=get_FE_field_value_type(field);
					if (0<number_of_values)
					{
						switch (value_type)
						{
							case ELEMENT_XI_VALUE:
							{
								FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
								struct FE_element *element;

								if (number_of_values==number_of_components)
								{
									for (k=0;(k<number_of_values)&&return_code;k++)
									{
										USE_PARAMETER(element);
										USE_PARAMETER(xi);
										/* Not implemented yet */
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Derivatives/versions not supported for element_xi.");
									return_code=0;
								}
							} break;
							case FE_VALUE_VALUE:
							{
								FE_value *values;

								if (ALLOCATE(values,FE_value,number_of_values))
								{
									for (k=0;(k<number_of_values)&&return_code;k++)
									{
										if (1 == sscanf(fieldml_data->character_index,
												FE_VALUE_INPUT_STRING "%n",
												&(values[k]), &bytes_processed))
										{
											fieldml_data->character_index += bytes_processed;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Error reading nodal value from file.");
											return_code=0;
										}
										if (!finite(values[k]))
										{
											display_message(ERROR_MESSAGE,
												"Infinity or NAN read from node file.");
											return_code=0;
										}
									}
									if (return_code)
									{
										if (time_index)
										{
											if (return_code=set_FE_nodal_field_FE_values_at_time(
													 field,node,values,&length,time_index->time))
											{
												if (length != number_of_values)
												{
													display_message(ERROR_MESSAGE,
														"node %d field '%s' took %d values from %d"
														" expected.",node_number,field_name,
														length,number_of_values);
													return_code=0;
												}
											}
										}
										else
										{
											if (return_code=set_FE_nodal_field_FE_value_values(
													 field,node,values,&length))
											{
												if (length != number_of_values)
												{
													display_message(ERROR_MESSAGE,
														"node %d field '%s' took %d values from %d"
														" expected.",node_number,field_name,
														length,number_of_values);
													return_code=0;
												}
											}
										}
									}
									DEALLOCATE(values);
								}
								else
								{
									display_message(ERROR_MESSAGE,"read_FE_node.  "
										"Insufficient memory for FE_value_values");
									return_code=0;
								}
							} break;
							case INT_VALUE:
							{
								int *values;

								if (ALLOCATE(values,int,number_of_values))
								{
									for (k=0;(k<number_of_values)&&return_code;k++)
									{
										if (1 == sscanf(fieldml_data->character_index,
												"%d%n",
												&(values[k]), &bytes_processed))
										{
											fieldml_data->character_index += bytes_processed;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Error reading nodal value from file.");
											return_code=0;
										}
									}
									if (return_code)
									{
										if (return_code=set_FE_nodal_field_int_values(field,
												node,values,&length))
										{
											if (length != number_of_values)
											{
												display_message(ERROR_MESSAGE,
													"node %d field '%s' took %d values from %d"
													" expected.",node_number,field_name,
													length,number_of_values);
												return_code=0;
											}
										}
									}
									DEALLOCATE(values);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_FE_node.  Insufficient memory for int_values");
									return_code=0;
								}
							} break;
							case STRING_VALUE:
							{
								char *the_string;

								if (number_of_values==number_of_components)
								{
									for (k=0;(k<number_of_values)&&return_code;k++)
									{
										USE_PARAMETER(the_string);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Derivatives/versions not supported for string.");
									return_code=0;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"Unsupported value_type %s.",
									Value_type_string(value_type));
								return_code=0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No nodal values for field '%s'.",field_name);
						return_code=0;
					}
				}
				DEALLOCATE(field_name);
				fieldml_data->current_field_ref = (struct FE_field *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_process_label.  Missing fieldml data.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_process_label */

static int Fieldml_label_references_same_value(
   struct Fieldml_label_name *label_in_list, void *label_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int number_in_child_labels_one, number_in_child_labels_two, return_code;
	struct Fieldml_label_name *label;

	ENTER(Fieldml_label_references_same_value);

	if (label_in_list && (label = (struct Fieldml_label_name *)label_void))
	{
		return_code = 1;
		if (label->type == label_in_list->type)
		{
			switch (label->type)
			{
				case FIELDML_LABEL_TYPE_LABEL_LOOKUP:
				case FIELDML_LABEL_TYPE_ASSIGN_LABELS:
				{
					if (strcmp(label->name, label_in_list->name))
					{
						return_code = 0;
					}
				} break;
			}
			if (return_code)
			{
				if (!label->child_labels && !label_in_list->child_labels)
				{
					return_code = 1;
				}
				else
				{
					number_in_child_labels_one = NUMBER_IN_LIST(Fieldml_label_name)
						(label->child_labels);
					number_in_child_labels_two = NUMBER_IN_LIST(Fieldml_label_name)
						(label_in_list->child_labels);
					if ((0 == number_in_child_labels_one) &&
						(0 == number_in_child_labels_two))
					{
						return_code = 1;
					}
					else if ((1 == number_in_child_labels_one) &&
						(1 == number_in_child_labels_two))
					{
						label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
							((LIST_ITERATOR_FUNCTION(Fieldml_label_name) *)NULL, NULL,
								label->child_labels);
						label_in_list = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
							((LIST_ITERATOR_FUNCTION(Fieldml_label_name) *)NULL, NULL,
								label_in_list->child_labels);
						return_code = Fieldml_label_references_same_value
							(label_in_list, (void *)label);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Fieldml_label_references_same_value.  "
							"To many children in an node index element.");
						return_code = 0;
					}
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
			"Fieldml_label_references_same_value.  "
			"Missing label structures.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_references_same_value */

static int Fieldml_label_is_type(struct Fieldml_label_name *label,
	void *label_type_address_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
==============================================================================*/
{
	enum Fieldml_label_type label_type;
	int return_code;

	ENTER(Fieldml_label_references_same_value);

	if (label)
	{
		label_type = *((enum Fieldml_label_type *)label_type_address_void);
		if (label->type == label_type)
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
			"Fieldml_label_references_same_value.  "
			"Missing label structures.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_references_same_value */

static int Fieldml_label_name_assign_local_index(
   struct Fieldml_label_name *label, void *local_index_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code, *local_index;

	ENTER(Fieldml_label_name_assign_local_index);

	if (local_index = (int *)local_index_void)
	{
		label->local_index = *local_index;
		label->current_node = (struct FE_node *)NULL;
		if (label->current_node_derivative_types)
		{
			DEALLOCATE(label->current_node_derivative_types);
		}
		(*local_index)++;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_assign_local_index.  "
			"Missing node number pointer.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_assign_local_index */

struct Fieldml_label_reference
{
	struct Fieldml_label_name *label;
	struct Fieldml_label_name *referenced_label;
	struct Fieldml_sax_data *fieldml_data;
};

static int Fieldml_label_looks_up_value(
   struct Fieldml_label_name *label_in_list, void *label_reference_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *component_name;
	int number_in_child_labels_one, return_code;
	struct Fieldml_label_reference *label_reference;
	struct Fieldml_label_name *label;
	struct Fieldml_sax_data *fieldml_data;

	ENTER(Fieldml_label_looks_up_value);

	if (label_in_list &&
		(label_reference = (struct Fieldml_label_reference *)label_reference_void) &&
		(label = label_reference->label) &&
		(fieldml_data = (struct Fieldml_sax_data *)label_reference->fieldml_data))
	{
		return_code = 1;
		/* Are the label types consistent */
		if (((FIELDML_LABEL_TYPE_FIELD_LOOKUP == label->type) &&
				 (FIELDML_LABEL_TYPE_FIELD_REF == label_in_list->type))
			|| ((FIELDML_LABEL_TYPE_COMPONENT_LOOKUP == label->type) &&
				(FIELDML_LABEL_TYPE_COMPONENT_REF == label_in_list->type))
			|| ((FIELDML_LABEL_TYPE_LABEL_LOOKUP == label->type) &&
				(FIELDML_LABEL_TYPE_LABEL == label_in_list->type)))
		{
			if (FIELDML_LABEL_TYPE_LABEL_LOOKUP == label->type)
			{
				if (strcmp(label->name, label_in_list->name))
				{
					return_code = 0;
				}
			}
			if (FIELDML_LABEL_TYPE_FIELD_LOOKUP == label->type)
			{
				if (fieldml_data->current_field_ref)
				{
					if (fieldml_data->current_field_ref != label_in_list->field)
					{
						/* Wrong field */
						return_code = 0;
					}
				}
				else
				{
					/* Can't resolve this now even though we may be able to in
						another context */
					return_code = 0;
				}
			}
			if (FIELDML_LABEL_TYPE_COMPONENT_LOOKUP == label->type)
			{
				if (fieldml_data->current_field_ref)
				{
					if (component_name = get_FE_field_component_name(
							 fieldml_data->current_field_ref,
							 fieldml_data->current_component_number))
					{
						if (!(strcmp(component_name, label_in_list->name)))
						{
							/* This component is OK */
						}
						else
						{
							return_code = 0;
						}
						DEALLOCATE(component_name);
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
							"Unable to get component name.");
						return_code = 0;
					}
				}
				else
				{
					/* Can't resolve this now even though we may be able to in
						another context */
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (!label->child_labels)
				{
					/* Success when we have followed as far as the lookups go */
					label_reference->referenced_label = label_in_list;
				}
				else
				{
					/* If we got down to here then this is the correct label at 
						this level and so we always return 1 even if the label isn't
						found so that the list iterators will not continue to look 
						through the list */
					number_in_child_labels_one = NUMBER_IN_LIST(Fieldml_label_name)
						(label->child_labels);
					if (0 == number_in_child_labels_one)
					{
						/* Success when we have followed as far as the lookups go */
						label_reference->referenced_label = label_in_list;
					}
					else if (1 == number_in_child_labels_one)
					{
						if (label_in_list->child_labels)
						{
							label_reference->label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
								((LIST_ITERATOR_FUNCTION(Fieldml_label_name) *)NULL, NULL,
									label->child_labels);
							/* Look down the tree, if found the referenced_label
								pointer will be set. */
							FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
								(Fieldml_label_looks_up_value, label_reference_void,
									label_in_list->child_labels);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Fieldml_label_looks_up_value.  "
							"To many children in a lookup element.");
					}
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
			"Fieldml_label_looks_up_value.  "
			"Missing label structures.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_looks_up_value */

static struct Fieldml_label_name *Fieldml_label_name_resolve_element_lookup(
   struct Fieldml_label_name *label, struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_label_name *child_label, *return_label;
	struct Fieldml_label_reference label_reference;

	ENTER(Fieldml_label_name_resolve_element_lookup);

	return_label = (struct Fieldml_label_name *)NULL;
	if (label && fieldml_data)
	{
		/* Both the elements own list and the immediate children of the
			elements interpolation are top level to the element */
		if (label->child_labels && 
			(1 == NUMBER_IN_LIST(Fieldml_label_name)(label->child_labels)))
		{
			child_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
				((LIST_ITERATOR_FUNCTION(Fieldml_label_name) *)NULL, NULL,
					label->child_labels);
			label_reference.label = child_label;
			label_reference.referenced_label = (struct Fieldml_label_name *)NULL;
			label_reference.fieldml_data = fieldml_data;
			FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
				Fieldml_label_looks_up_value, (void *)&label_reference,
				fieldml_data->current_element_labels);
			if (!label_reference.referenced_label)
			{
				label_reference.label = child_label;
				FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
					Fieldml_label_looks_up_value, (void *)&label_reference,
					fieldml_data->current_element_interpolation_ref->child_labels);
			}
			if (label_reference.referenced_label)
			{
				return_label = label_reference.referenced_label;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Fieldml_label_name_resolve_element_lookup.  "
				"Only exactly one child permitted in an element lookup.");			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_resolve_element_lookup.  "
			"Missing label or fieldml_data.");
	}
	
	LEAVE;

	return(return_label);
} /* Fieldml_label_name_resolve_element_lookup */

static int Fieldml_label_name_create_node_and_element_lookup_lists(
   struct Fieldml_label_name *label, void *fieldml_data_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Fieldml_sax_data *fieldml_data;

	ENTER(Fieldml_label_name_create_node_and_element_lookup_lists);

	if (fieldml_data = (struct Fieldml_sax_data *)fieldml_data_void)
	{
		return_code = 1;
		if (FIELDML_LABEL_TYPE_NODE_INDEX == label->type)
		{
			if (!FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
				Fieldml_label_references_same_value, (void *)label,
				fieldml_data->element_node_index_list))
			{
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
					fieldml_data->element_node_index_list);
			}
		}
		else if (FIELDML_LABEL_TYPE_ELEMENT_LOOKUP == label->type)
		{
			if (!FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
				Fieldml_label_references_same_value, (void *)label,
				fieldml_data->element_lookup_list))
			{
				/* If we can resolve it now then the values are stored in 
					the element */
				if (Fieldml_label_name_resolve_element_lookup(
					label, fieldml_data))
				{
					ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
						fieldml_data->element_lookup_list);
				}
			}
		}
		/* Don't follow this down as we treat the element_lookups referenced by
			the node_lookup differently. */
		else if (label->child_labels)
		{
			ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
			return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
				Fieldml_label_name_create_node_and_element_lookup_lists,
				fieldml_data_void, label->child_labels);
			REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_create_node_and_element_lookup_lists.  "
			"Missing fieldml data.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_create_node_and_element_lookup_lists */

static int Fieldml_label_name_assign_scale_factor_lists(
   struct Fieldml_label_name *label, void *fieldml_data_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Fieldml_label_name *returned_label;
	struct Fieldml_sax_data *fieldml_data;

	ENTER(Fieldml_label_name_assign_scale_factor_lists);

	if (fieldml_data = (struct Fieldml_sax_data *)fieldml_data_void)
	{
		/* We need the offsets in the parent as we are going to use this
			as a template with many referenced lookups */
		label->scale_factor_list_offset = fieldml_data->scale_factor_list_offset;
		if (returned_label = Fieldml_label_name_resolve_element_lookup(
			label, fieldml_data))
		{
			returned_label->scale_factor_list_offset = label->scale_factor_list_offset;
			returned_label->scale_factor_list_size = label->scale_factor_list_size;
			returned_label->scale_factor_list_identifier = 
				label->scale_factor_list_identifier;
			fieldml_data->scale_factor_list_identifiers[
				fieldml_data->number_of_scale_factor_lists] = 
				returned_label->scale_factor_list_identifier;
			fieldml_data->scale_factor_list_sizes[
				fieldml_data->number_of_scale_factor_lists] = 
				returned_label->scale_factor_list_size;
			fieldml_data->number_of_scale_factor_lists++;
			fieldml_data->scale_factor_list_offset += returned_label->scale_factor_list_size;
		}
		/* We always want to work through the whole list irrespective 
			of whether this label was resolved or not */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_assign_scale_factor_lists.  "
			"Missing node number pointer.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_assign_scale_factor_lists */

static int Fieldml_label_name_process_label_in_element(
   struct Fieldml_label_name *label, void *fieldml_data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
==============================================================================*/
{
	char *component_name;
	enum Fieldml_label_type label_type;
	enum FE_nodal_value_type derivative_type;
	int follow_children, index, i, j, node_index_value, node_number, number_of_components,
		number_of_nodal_values, return_code, version_number;
	struct Fieldml_label_name *node_index, *returned_label, *sub_label, *value_type_label;
	struct Fieldml_sax_data *fieldml_data;

	ENTER(Fieldml_label_name_process_label_in_element);

	if (fieldml_data = (struct Fieldml_sax_data *)fieldml_data_void)
	{
		follow_children = 1;
		return_code = 1;
		if (FIELDML_LABEL_TYPE_LABEL == label->type)
		{
			if (label->value && fieldml_data->current_value_list)
			{
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
					fieldml_data->current_value_list);
			}
		}
		else if (FIELDML_LABEL_TYPE_FIELD_REF == label->type)
		{
			if (label->field)
			{
				if (!(fieldml_data->current_field_ref))
				{
					if (!fieldml_data->current_element_field_components)
					{
						fieldml_data->current_field_ref = label->field;
						fieldml_data->current_component_number = 0;
						if (number_of_components = get_FE_field_number_of_components(
								 label->field))
						{
							if(ALLOCATE(fieldml_data->current_element_field_components,
									struct FE_element_field_component *, number_of_components))
							{
								for (j = 0 ; j < number_of_components ; j++)
								{
									fieldml_data->current_element_field_components[j] =
										(struct FE_element_field_component *)NULL;
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Already defining element field components.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Already referencing a field.");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
					"Label is field ref type but does not have a field.");
			}
		}
		else if (FIELDML_LABEL_TYPE_COMPONENT_REF == label->type)
		{
			if (fieldml_data->current_field_ref)
			{
				if (component_name = get_FE_field_component_name(
					fieldml_data->current_field_ref, 
					fieldml_data->current_component_number))
				{
					if (!(strcmp(component_name, label->name)))
					{
						/* This component is OK */
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
							"Referenced component does not match the field.");
					}
					DEALLOCATE(component_name);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Unable to get component name.");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
					"Not referencing a field when component encountered.");
			}
		}
		else if (FIELDML_LABEL_TYPE_BASIS_MAPPING == label->type)
		{
			if (!fieldml_data->current_basis_mapping)
			{
				fieldml_data->current_basis_mapping = label;
				if (!fieldml_data->current_field_ref)
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Expect to be in a field_ref when defining a mapping.");
				}
				fieldml_data->current_element_field_component_nodes = 0;
				fieldml_data->current_element_field_standard_map =
					(struct Standard_node_to_element_map **)NULL;
				fieldml_data->current_element_field_standard_map_node_index = -1;
				fieldml_data->current_element_field_standard_map_number_of_values = 0;
				fieldml_data->current_element_field_standard_map_indices = (int *)NULL;
				fieldml_data->current_scale_factor_list_size = 0;
				fieldml_data->current_scale_factor_list_offset = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
					"Already processing a basis mapping.");
			}
		}
		else if (FIELDML_LABEL_TYPE_ELEMENT_LOOKUP == label->type)
		{
			if (returned_label = Fieldml_label_name_resolve_element_lookup(
					 label, fieldml_data))
			{
				if (returned_label->scale_factor_list_size)
				{
					if (!fieldml_data->current_scale_factor_list_size)
					{
						fieldml_data->current_scale_factor_list_size = 
							returned_label->scale_factor_list_size;
						fieldml_data->current_scale_factor_list_offset = 
							returned_label->scale_factor_list_offset;
					}
				}
				else
				{
					/* Process this label as if it was right here */
					Fieldml_label_name_process_label_in_element(returned_label,
						fieldml_data_void);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
					"Unable to resolve element_lookup.");
			}
			/* The children specified the lookup */
			follow_children = 0;
		}
		else if (FIELDML_LABEL_TYPE_NODE_INDEX == label->type)
		{
			if (!label->current_node)
			{
				if (!fieldml_data->current_value_list)
				{
					fieldml_data->current_value_list =
						CREATE(LIST(Fieldml_label_name))();
					if (label->child_labels && (1 ==
							 NUMBER_IN_LIST(Fieldml_label_name)(label->child_labels)))
					{
						if (sub_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
							((LIST_ITERATOR_FUNCTION(Fieldml_label_name) *)NULL, NULL,
								label->child_labels))
						{
							Fieldml_label_name_process_label_in_element(sub_label,
								fieldml_data_void);
							if (1 == NUMBER_IN_LIST(Fieldml_label_name)
								(fieldml_data->current_value_list))
							{
								sub_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)
									((LIST_ITERATOR_FUNCTION(Fieldml_label_name) *)NULL, NULL,
										fieldml_data->current_value_list);
								if (sub_label && sub_label->value &&
									(1 == sscanf(sub_label->value, "%d", &node_number)))
								{
									if (label->current_node = 
										FE_region_get_FE_node_from_identifier(fieldml_data->current_fe_region,
											node_number))
									{
										/* OK */
									}
									else
									{
										display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
											"Unable to find node number %d.", node_number);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
										"Unable to parse node number from node index value.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
									"Unable to get value for node index.");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
								"Unable to get child from node index.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
							"Too many children for a node index.");
					}
					DESTROY(LIST(Fieldml_label_name))(&fieldml_data->current_value_list);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Unable to get node_index element from node_lookup.");
				}
			}
			if (label->current_node && fieldml_data->current_field_ref)
			{
				label->current_node_number_of_derivatives = 
					get_FE_node_field_component_number_of_derivatives(
						label->current_node, fieldml_data->current_field_ref,
						fieldml_data->current_component_number);
				label->current_node_number_of_versions =
					get_FE_node_field_component_number_of_versions(
						label->current_node, fieldml_data->current_field_ref,
						fieldml_data->current_component_number);
				if (label->current_node_derivative_types)
				{
					DEALLOCATE(label->current_node_derivative_types);
				}
				label->current_node_derivative_types =
					get_FE_node_field_component_nodal_value_types(
						label->current_node, fieldml_data->current_field_ref,
						fieldml_data->current_component_number);
			}
			else
			{
				label->current_node_number_of_derivatives = 0;
				label->current_node_number_of_versions = 0;
				if (label->current_node_derivative_types)
				{
					DEALLOCATE(label->current_node_derivative_types);
				}
			}
			follow_children = 0;
		}
		else if (FIELDML_LABEL_TYPE_NODE_LOOKUP == label->type)
		{
			node_index_value = -1;
			/* Find the node_index label */
			label_type = FIELDML_LABEL_TYPE_NODE_INDEX;
			if (node_index = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
				Fieldml_label_is_type, (void *)&label_type, label->child_labels))
			{
				node_index_value = node_index->local_index;
				if (node_index_value < 0)
				{
					if (node_index = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
						Fieldml_label_references_same_value, (void *)node_index,
						fieldml_data->element_node_index_list))
					{
						node_index_value = node_index->local_index;
					}
				}
				if (node_index_value >= 0)
				{
					Fieldml_label_name_process_label_in_element(node_index,
						fieldml_data_void);
					if (node_index->current_node)
					{
						/* Values in the node must be in the field and component */
						version_number = 0;
						index = 0;
						label_type = FIELDML_LABEL_TYPE_FIELD_LOOKUP;
						if (value_type_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
								 Fieldml_label_is_type, (void *)&label_type, label->child_labels))
						{
							label_type = FIELDML_LABEL_TYPE_COMPONENT_LOOKUP;
							if (value_type_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
									 Fieldml_label_is_type, (void *)&label_type, value_type_label->child_labels))
							{
								label_type = FIELDML_LABEL_TYPE_LABEL_LOOKUP;
								if (value_type_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
										 Fieldml_label_is_type, (void *)&label_type, value_type_label->child_labels))
								{
									if (!strncmp("version", value_type_label->name, 7))
									{
										if (sscanf(value_type_label->name, "version_%d", &version_number))
										{
											if ((version_number > 0) && (version_number <= 
												node_index->current_node_number_of_versions))
											{
												/* Index from zero internally */
												version_number--;
											}
											else
											{
												version_number = 0;
											}
											
										}
										else
										{
											display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
												"Unable to parse a version label.");
										}
										label_type = FIELDML_LABEL_TYPE_LABEL_LOOKUP;
										value_type_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
											Fieldml_label_is_type, (void *)&label_type, value_type_label->child_labels);
									}
									if (value_type_label && STRING_TO_ENUMERATOR(
										FE_nodal_value_type)(value_type_label->name, &derivative_type))
									{
										j = 0;
										while ((derivative_type != node_index->current_node_derivative_types[j]) &&
											(j <= node_index->current_node_number_of_derivatives))
										{
											j++;
										}
										if (j <= node_index->current_node_number_of_derivatives)
										{
											index = j;
										}
										else
										{
											display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
												"Unable to find nodal value type in node.");
										}
									}
									index += (node_index->current_node_number_of_derivatives + 1) *
										version_number;


									if (fieldml_data->current_basis_mapping)
									{
										/* Now we know what we have store it in the fieldml_data
											unfortunately we have to have correct number of node_maps
											requiring me to compile together the values from one node
											into a single Standard_node_to_element_map but if the same
											node is repeated starting again and the only way I can tell
											is if the node index restarts */
										if ((fieldml_data->current_element_field_standard_map_node_index
												 != node_index_value) ||
											(derivative_type == FE_NODAL_VALUE))
										{
											if (fieldml_data->current_element_field_standard_map_number_of_values)
											{
												if (REALLOCATE(fieldml_data->current_element_field_standard_map,
														 fieldml_data->current_element_field_standard_map,
														 struct Standard_node_to_element_map *, fieldml_data->current_element_field_component_nodes+1)
													&& (fieldml_data->current_element_field_standard_map[
															 fieldml_data->current_element_field_component_nodes] = 
														CREATE(Standard_node_to_element_map)(
															fieldml_data->current_element_field_standard_map_node_index,
															fieldml_data->current_element_field_standard_map_number_of_values)))
												{
													for (j = 0 ; j < fieldml_data->current_element_field_standard_map_number_of_values ; j++)
													{
														Standard_node_to_element_map_set_nodal_value_index(
															fieldml_data->current_element_field_standard_map[
																fieldml_data->current_element_field_component_nodes],
															/*nodal_value_number*/j, 
															fieldml_data->current_element_field_standard_map_indices[j]);
													}
													fieldml_data->current_element_field_component_nodes++;
												}
												fieldml_data->current_element_field_standard_map_number_of_values = 0;
											}
											fieldml_data->current_element_field_standard_map_node_index
												= node_index_value;
										}
										REALLOCATE(fieldml_data->current_element_field_standard_map_indices,
											fieldml_data->current_element_field_standard_map_indices,
											int, fieldml_data->current_element_field_standard_map_number_of_values+1);
										fieldml_data->current_element_field_standard_map_indices[
											fieldml_data->current_element_field_standard_map_number_of_values]
											= index;
										fieldml_data->current_element_field_standard_map_number_of_values++;
									}
								}
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Invalid node index number.");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
					"Unable to get node_index element from node_lookup.");
			}

			/* The children specified the lookup */
			follow_children = 0;
		}
		if (follow_children && label->child_labels)
		{
			ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
			return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
				Fieldml_label_name_process_label_in_element,
				fieldml_data_void, label->child_labels);
			REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
		}
		if (FIELDML_LABEL_TYPE_COMPONENT_REF == label->type)
		{
			fieldml_data->current_component_number++;
		}
		else if (FIELDML_LABEL_TYPE_FIELD_REF == label->type)
		{
			if (fieldml_data->current_element_field_components)
			{
				if (!define_FE_field_at_element(fieldml_data->current_element,
					fieldml_data->current_field_ref,
					fieldml_data->current_element_field_components))
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Unable to define FE_field at element.");
				}
				if (number_of_components = get_FE_field_number_of_components(
					label->field))
				{
					for (j = 0 ; j < number_of_components ; j++)
					{
						if (fieldml_data->current_element_field_components[j])
						{
							DESTROY(FE_element_field_component)(
								&fieldml_data->current_element_field_components[j]);
						}
					}
				}
				DEALLOCATE(fieldml_data->current_element_field_components);
			}
			fieldml_data->current_field_ref = (struct FE_field *)NULL;
		}
		else if (FIELDML_LABEL_TYPE_BASIS_MAPPING == label->type)
		{
			if (fieldml_data->current_element_field_standard_map_number_of_values)
			{
				REALLOCATE(fieldml_data->current_element_field_standard_map,
					fieldml_data->current_element_field_standard_map,
					struct Standard_node_to_element_map *,
					fieldml_data->current_element_field_component_nodes+1);
				fieldml_data->current_element_field_standard_map[
					fieldml_data->current_element_field_component_nodes] = 
					CREATE(Standard_node_to_element_map)(
						fieldml_data->current_element_field_standard_map_node_index,
						fieldml_data->current_element_field_standard_map_number_of_values);
				for (j = 0 ; j < fieldml_data->current_element_field_standard_map_number_of_values ; j++)
				{
					Standard_node_to_element_map_set_nodal_value_index(
						fieldml_data->current_element_field_standard_map[
							fieldml_data->current_element_field_component_nodes],
						/*nodal_value_number*/j, 
						fieldml_data->current_element_field_standard_map_indices[j]);
				}
				fieldml_data->current_element_field_standard_map_number_of_values = 0;
				fieldml_data->current_element_field_component_nodes++;
			}
			if (fieldml_data->current_scale_factor_list_size)
			{
				for (i = 0 ; i < fieldml_data->current_element_field_component_nodes ; i++)
				{
					Standard_node_to_element_map_get_number_of_nodal_values(
						fieldml_data->current_element_field_standard_map[i],
						&number_of_nodal_values);
					for (j = 0 ; j < number_of_nodal_values ; j++)
					{
						Standard_node_to_element_map_set_scale_factor_index(
							fieldml_data->current_element_field_standard_map[i],
							/*nodal_value_number*/j,
							fieldml_data->current_scale_factor_list_offset);
						fieldml_data->current_scale_factor_list_offset++;
					}
				}
			}
			if (fieldml_data->current_element_field_component_nodes &&
				fieldml_data->current_element_field_components)
			{
				if (!fieldml_data->current_element_field_components[
					fieldml_data->current_component_number])
				{
					if (fieldml_data->current_element_field_components[
						fieldml_data->current_component_number] =
						CREATE(FE_element_field_component)(
						STANDARD_NODE_TO_ELEMENT_MAP,
						fieldml_data->current_element_field_component_nodes,
						label->basis, label->modify))
					{
						for (j = 0 ; j < fieldml_data->current_element_field_component_nodes ; j++)
						{
							FE_element_field_component_set_standard_node_map(
								fieldml_data->current_element_field_components[
									fieldml_data->current_component_number],
								/*node_number*/j,
								fieldml_data->current_element_field_standard_map[j]);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
						"Already defined an element field component for this component.");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label_in_element.  "
					"Missing field component nodes or field component storage.");
			}
			fieldml_data->current_element_field_component_nodes = 0;
			if (fieldml_data->current_element_field_standard_map_indices)
			{
				DEALLOCATE(fieldml_data->current_element_field_standard_map_indices);
			}
			if (fieldml_data->current_element_field_standard_map)
			{
				DEALLOCATE(fieldml_data->current_element_field_standard_map);
			}
				
			fieldml_data->current_basis_mapping = 
				(struct Fieldml_label_name *)NULL;
			fieldml_data->current_scale_factor_list_size = 0;
			fieldml_data->current_scale_factor_list_offset = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_process_label_in_element.  Missing fieldml data.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_process_label_in_element */

static int Fieldml_label_name_set_nodes_in_element(
   struct Fieldml_label_name *label, void *element_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct FE_element *element;

	ENTER(Fieldml_label_name_set_nodes_in_element);

	if (element = (struct FE_element *)element_void)
	{
		set_FE_element_node(element,label->local_index,label->current_node);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_set_nodes_in_element.  "
			"Missing node number pointer.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_set_nodes_in_element */

static int Fieldml_label_name_set_scale_factors_in_element(
   struct Fieldml_label_name *label, void *fieldml_data_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	float scale_factor;
	int follow_children, return_code;
	struct Fieldml_label_name *returned_label;
	struct Fieldml_sax_data *fieldml_data;

	ENTER(Fieldml_label_name_set_scale_factors_in_element);

	if (fieldml_data = (struct Fieldml_sax_data *)fieldml_data_void)
	{
		follow_children = 1;
		if (FIELDML_LABEL_TYPE_ELEMENT_LOOKUP == label->type)
		{
			if (returned_label = Fieldml_label_name_resolve_element_lookup(
				label, fieldml_data))
			{
				/* The element_lookup list is used as a template for
					many elements so the offset must be fetched from there */
				fieldml_data->scale_factor_list_offset = 
					label->scale_factor_list_offset;
				Fieldml_label_name_set_scale_factors_in_element(
					returned_label, fieldml_data_void);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Fieldml_label_name_set_scale_factors_in_element.  "
					"Unable to find scale factor set in element.");
			}
			follow_children = 0;
		}
		else if (FIELDML_LABEL_TYPE_LABEL == label->type)
		{
			if (label->value)
			{
				sscanf(label->value, "%f", &scale_factor);
				set_FE_element_scale_factor(fieldml_data->current_element,
					/*scale_factor_number*/fieldml_data->scale_factor_list_offset,
					scale_factor);
				fieldml_data->scale_factor_list_offset++;				
			}
		}
		if (follow_children && label->child_labels)
		{
			return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
				Fieldml_label_name_set_scale_factors_in_element,
				fieldml_data_void, label->child_labels);
		}
		/* We always want to work through the whole list irrespective 
			of whether this label was resolved or not */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_set_scale_factors_in_element.  "
			"Missing fieldml data.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_set_scale_factors_in_element */

static int Fieldml_label_names_check_equivalent_label_in_list(
   struct Fieldml_label_name *label, void *check_label_list_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct LIST(Fieldml_label_name) *check_label_list;

	ENTER(Fieldml_label_names_define_equivalent_nodes);

	if (label &&
		(check_label_list = (struct LIST(Fieldml_label_name) *)check_label_list_void))
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
			Fieldml_label_references_same_value, label, check_label_list))
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
			"Fieldml_label_references_same_value.  "
			"Missing label structures.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_references_same_value */

static int Fieldml_label_names_define_equivalent_nodes(
   struct Fieldml_label_name *label_in_list, void *label_void)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	int number_in_child_labels_one, number_in_child_labels_two, return_code;
	struct Fieldml_label_name *label;

	ENTER(Fieldml_label_names_define_equivalent_nodes);

	if (label_in_list && (label = (struct Fieldml_label_name *)label_void))
	{
		return_code = 0;
		if ((label->type == FIELDML_LABEL_TYPE_NODE) &&
			(label_in_list->type == FIELDML_LABEL_TYPE_NODE))
		{
			number_in_child_labels_one = NUMBER_IN_LIST(Fieldml_label_name)
				(label->child_labels);
			number_in_child_labels_two = NUMBER_IN_LIST(Fieldml_label_name)
				(label_in_list->child_labels);
			if (number_in_child_labels_one == number_in_child_labels_two)
			{
				return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
					Fieldml_label_names_check_equivalent_label_in_list,
					label->child_labels, label_in_list->child_labels);	
			}
			else
			{
				return_code = 0;
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
			"Fieldml_label_references_same_value.  "
			"Missing label structures.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_references_same_value */

static void fieldml_start_fieldml(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *version_string, *attribute_name, *attribute_value;
	int i;

	ENTER(fieldml_start_fieldml);

	if (0 > fieldml_data->fieldml_version)
	{
		i = 0;
		if (attributes)
		{
			while (attributes[i])
			{
				attribute_name = (char *)attributes[i];
				attribute_value = (char *)attributes[i + 1];
				if (!strcmp(attribute_name, "xmlns"))
				{
					if (version_string = strstr(attribute_value, "fieldml/"))
					{
						sscanf(version_string + 8, "%d.%d", &fieldml_data->fieldml_version,
							&fieldml_data->fieldml_subversion);
					}
				}
				i += 2;
			}
		}
		if (0 > fieldml_data->fieldml_version)
		{
			display_message(WARNING_MESSAGE,
				"fieldml_start_fieldml.  Unable to read version number, assuming version 0.1");		
			fieldml_data->fieldml_version = 0; /* In fieldml but we don't know what version */
			fieldml_data->fieldml_subversion = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_fieldml.  Already in a fieldml block.");		
	}
	
	LEAVE;
} /* fieldml_start_fieldml */

static void fieldml_end_fieldml(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_fieldml);

	if (0 <= fieldml_data->fieldml_version)
	{
 		fieldml_data->fieldml_version = -1;
 		fieldml_data->fieldml_subversion = -1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_fieldml.  Not in a fieldml block.");
	}
	
	LEAVE;
} /* fieldml_end_fieldml */

static struct Fieldml_label_name *fieldml_create_label_name_in_hierarchy(
	char *name, enum Fieldml_label_type type, struct Fieldml_label_name *parent,
	struct LIST(Fieldml_label_name) *label_name_stack)
/*******************************************************************************
LAST MODIFIED : 21 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_label_name *label;

	ENTER(fieldml_create_label_name_in_hierarchy);

	if (name && label_name_stack)
	{
		label = CREATE(Fieldml_label_name)(name, type);
		if (parent)
		{
			/* This is a child of the current label name */
			if (!(parent->child_labels))
			{
				parent->child_labels = CREATE(LIST(Fieldml_label_name))();
			}
			/* Add this label to the parent */
			ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, 
				parent->child_labels);
		}
		/* Add this label to the stack */
		ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE, "fieldml_create_label_name_in_hierarchy.  "
			"Invalid arguments.");		
		label = (struct Fieldml_label_name *)NULL;
	}
	
	return (label);

	LEAVE;
} /* fieldml_create_label_name_in_hierarchy */

static void fieldml_start_label_name(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *label_name;
	int i;

	ENTER(fieldml_start_label_name);

	label_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "name"))
			{
				label_name = attribute_value;
			}
			i += 2;
		}
	}
	if (label_name)
	{
		/* If there isn't a current label name this will just add it in the stack,
			we then process it accordingly when this label ends. */
		fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
			label_name, FIELDML_LABEL_TYPE_LABEL,
			fieldml_data->current_label_name, fieldml_data->label_name_stack);
		fieldml_data->expecting_characters = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_label_name.  Unable to parse label name.");
	}
	
	LEAVE;
} /* fieldml_start_label_name */

static void fieldml_start_field_ref(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *label_name;
	int i;
	struct FE_field *field;
	
	ENTER(fieldml_start_field_ref);

	label_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "ref"))
			{
				label_name = attribute_value;
			}
			i += 2;
		}
	}
	if (label_name)
	{
		if (field = FE_region_get_FE_field_from_name(fieldml_data->current_fe_region,
			label_name))
		{
			fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
				label_name, FIELDML_LABEL_TYPE_FIELD_REF, 
				fieldml_data->current_label_name, fieldml_data->label_name_stack);
			fieldml_data->current_label_name->field = field;
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_field_ref.  "
				"<field_ref> for a field %s that hasn't been declared.", label_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_field_ref.  Unable to parse field_ref ref.");
	}
	
	LEAVE;
} /* fieldml_start_field_ref */

static void fieldml_start_component_ref(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *label_name;
	int i;

	ENTER(fieldml_start_component_ref);

	label_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "ref"))
			{
				label_name = attribute_value;
			}
			i += 2;
		}
	}
	if (label_name)
	{
		fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
			label_name, FIELDML_LABEL_TYPE_COMPONENT_REF,
			fieldml_data->current_label_name, fieldml_data->label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_component_ref.  Unable to parse component_ref ref.");
	}
	
	LEAVE;
} /* fieldml_start_component_ref */

static void fieldml_start_element_interpolation(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 21 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *name;
	int i;

	ENTER(fieldml_start_element_interpolation);

	if (!(fieldml_data->current_basis_mapping))
	{
	   name = (char *)NULL;
		i = 0;
		if (attributes)
		{
			while (attributes[i])
			{
				attribute_name = (char *)attributes[i];
				attribute_value = (char *)attributes[i + 1];
				if (!strcmp(attribute_name, "name"))
				{
					name = attribute_value;
				}
				i += 2;
			}
		}
		if (name)
		{
			fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
				name, FIELDML_LABEL_TYPE_ELEMENT_INTERPOLATION,
				fieldml_data->current_label_name, fieldml_data->label_name_stack);
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_element_interpolation.  "
				"Missing mapping name or basis string.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_element_interpolation.  Already working on a basis mapping.");
	}
	
	LEAVE;
} /* fieldml_start_element_interpolation */

static void fieldml_start_mapping_ref(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *label_name;
	int i;
	struct Fieldml_label_name *label;

	ENTER(fieldml_start_mapping_ref);

	label_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "ref"))
			{
				label_name = attribute_value;
			}
			i += 2;
		}
	}
	if (label_name)
	{
		if (label = FIND_BY_IDENTIFIER_IN_LIST(Fieldml_label_name,name)(
			label_name, fieldml_data->basis_mappings))
		{
			if (fieldml_data->current_label_name)
			{
				if (!fieldml_data->current_label_name->child_labels)
				{
					fieldml_data->current_label_name->child_labels = 
						CREATE(LIST(Fieldml_label_name))();
				}
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
					fieldml_data->current_label_name->child_labels);
			}
			else
			{
				display_message(ERROR_MESSAGE, "fieldml_start_mapping_ref.  "
					"Not in a label tree.");
			}
			ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
				fieldml_data->label_name_stack);
			fieldml_data->current_label_name = label;
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_mapping_ref.  "
				"Unable to find mapping_ref %s.", label_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_mapping_ref.  Unable to parse mapping_ref ref.");
	}
	
	LEAVE;
} /* fieldml_start_mapping_ref */

static void fieldml_start_coefficients(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_start_coefficients);

	if (attributes && attributes[0])
	{
		display_message(ERROR_MESSAGE, "fieldml_start_coefficients.  "
			"Not expecting attributes for coefficients.");
	}
	fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
		"coefficients", FIELDML_LABEL_TYPE_COEFFICIENTS,
		fieldml_data->current_label_name, fieldml_data->label_name_stack);
	
	LEAVE;
} /* fieldml_start_coefficients */

static void fieldml_start_product(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_start_product);

	if (attributes && attributes[0])
	{
		display_message(ERROR_MESSAGE, "fieldml_start_product.  "
			"Not expecting attributes for product.");
	}
	fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
		"product", FIELDML_LABEL_TYPE_PRODUCT,
		fieldml_data->current_label_name, fieldml_data->label_name_stack);
	
	LEAVE;
} /* fieldml_start_product */

static void fieldml_start_label_lookup(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *label_name;
	int i;

	ENTER(fieldml_start_label_name);

	label_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "indices"))
			{
				label_name = attribute_value;
			}
			i += 2;
		}
	}
	if (label_name)
	{
		/* If there isn't a current label name this will just add it in the stack,
			we then process it accordingly when this label ends. */
		fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
			label_name, FIELDML_LABEL_TYPE_LABEL_LOOKUP,
			fieldml_data->current_label_name, fieldml_data->label_name_stack);
		fieldml_data->expecting_characters = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_label_lookup.  Unable to parse label name.");
	}
	
	LEAVE;
} /* fieldml_start_label_name */

static void fieldml_start_node_lookup(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_start_node_lookup);

	if (!(fieldml_data->current_basis_mapping))
	{
		if (attributes && attributes[0])
		{
			display_message(ERROR_MESSAGE, "fieldml_start_node_lookup.  "
				"Not expecting attributes for node lookup.");
		}
		fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
			"node_lookup", FIELDML_LABEL_TYPE_NODE_LOOKUP,
			fieldml_data->current_label_name, fieldml_data->label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_node_lookup.  Already working on a basis mapping.");
	}
	
	LEAVE;
} /* fieldml_start_node_lookup */

static void fieldml_start_node_index(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_start_node_index);

	if (!(fieldml_data->current_basis_mapping))
	{
		if (attributes && attributes[0])
		{
			display_message(ERROR_MESSAGE, "fieldml_start_node_index.  "
				"Not expecting attributes for node lookup.");
		}
		fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
			"node_index", FIELDML_LABEL_TYPE_NODE_INDEX,
			fieldml_data->current_label_name, fieldml_data->label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_node_index.  Already working on a basis mapping.");
	}
	
	LEAVE;
} /* fieldml_start_node_index */

static void fieldml_start_field_lookup(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_start_field_lookup);

	if (attributes && attributes[0])
	{
		display_message(ERROR_MESSAGE, "fieldml_start_field_lookup.  "
			"Not expecting attributes for node lookup.");
	}
	fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
		"field_lookup", FIELDML_LABEL_TYPE_FIELD_LOOKUP,
		fieldml_data->current_label_name, fieldml_data->label_name_stack);
	
	LEAVE;
} /* fieldml_start_field_lookup */

static void fieldml_start_component_lookup(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_start_component_lookup);

	if (attributes && attributes[0])
	{
		display_message(ERROR_MESSAGE, "fieldml_start_component_lookup.  "
			"Not expecting attributes for node lookup.");
	}
	fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
		"component_lookup", FIELDML_LABEL_TYPE_COMPONENT_LOOKUP,
		fieldml_data->current_label_name, fieldml_data->label_name_stack);
	
	LEAVE;
} /* fieldml_start_component_lookup */

static void fieldml_start_element_lookup(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_start_element_lookup);

	if (attributes && attributes[0])
	{
		display_message(ERROR_MESSAGE, "fieldml_start_element_lookup.  "
			"Not expecting attributes for node lookup.");
	}
	fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
		"element_lookup", FIELDML_LABEL_TYPE_ELEMENT_LOOKUP,
		fieldml_data->current_label_name, fieldml_data->label_name_stack);
	if (fieldml_data->current_basis_mapping &&
		fieldml_data->current_basis_mapping->basis)
	{
		fieldml_data->current_label_name->scale_factor_list_identifier =
			(void *)fieldml_data->current_basis_mapping->basis;
		FE_basis_get_number_of_basis_functions(
			fieldml_data->current_basis_mapping->basis,
			&(fieldml_data->current_label_name->scale_factor_list_size));
	}
	LEAVE;
} /* fieldml_start_element_lookup */

static void fieldml_end_label_name(struct Fieldml_sax_data *fieldml_data,
	enum Fieldml_label_type type)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{
	char name[15], *index, *index2;
	int count, length, number_in_list;
	struct Fieldml_label_name *label;

	ENTER(fieldml_end_label_name);

	if (fieldml_data->current_label_name)
	{
		if (type != fieldml_data->current_label_name->type)
		{
			display_message(ERROR_MESSAGE, "fieldml_end_label_name.  "
				"Label name doesn't match element type.");
		}

		/* Process any characters */
		if (index = fieldml_data->character_buffer)
		{
			count = fieldml_data->buffer_length;
			while ((count > 0) && (*index == ' ') || (*index == '\n') || (*index == '\t'))
			{
				index++;
				count--;
			}
			while (count > 0)
			{
				if (index2 = strpbrk(index, " \n\t"))
				{
					length = index2 - index;
				}
				else
				{
					length = count;
				}
				if (fieldml_data->current_label_name->child_labels)
				{
					number_in_list = NUMBER_IN_LIST(Fieldml_label_name)(
						fieldml_data->current_label_name->child_labels);
					sprintf(name, "%d", number_in_list + 1);
					if (label = CREATE(Fieldml_label_name)(name,
							 FIELDML_LABEL_TYPE_LABEL))
					{
						ADD_OBJECT_TO_LIST(Fieldml_label_name)(
							label, fieldml_data->current_label_name->child_labels);
					}			
				}
				else
				{
					if (fieldml_data->current_label_name->value)
					{
						fieldml_data->current_label_name->child_labels =
							CREATE(LIST(Fieldml_label_name))();
						sprintf(name, "%d", 1);
						if (label = CREATE(Fieldml_label_name)(name,
								 FIELDML_LABEL_TYPE_LABEL))
						{
							label->value = fieldml_data->current_label_name->value;
							fieldml_data->current_label_name->value = (char *)NULL;
							ADD_OBJECT_TO_LIST(Fieldml_label_name)(
								label, fieldml_data->current_label_name->child_labels);
						}
						sprintf(name, "%d", 2);
						if (label = CREATE(Fieldml_label_name)(name,
								 FIELDML_LABEL_TYPE_LABEL))
						{
							ADD_OBJECT_TO_LIST(Fieldml_label_name)(
								label, fieldml_data->current_label_name->child_labels);
						}
					}
					else
					{
						label = fieldml_data->current_label_name;
					}
				}
				if (ALLOCATE(label->value, char, length + 1))
				{
					memcpy(label->value, index, length);
					label->value[length] = 0;
				}
				index += length;
				count -= length;
				while ((count > 0) && (*index == ' ') || (*index == '\n') || (*index == '\t'))
				{
					index++;
					count--;
				}
			}
		}
		fieldml_data->expecting_characters = 0;
			
		if (1 < NUMBER_IN_LIST(Fieldml_label_name)(fieldml_data->label_name_stack))
		{
			REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
				fieldml_data->label_name_stack);
			fieldml_data->current_label_name = Last_Fieldml_label_name_in_list(
				fieldml_data->label_name_stack);
		}
		else
		{
			/* Then we are at the top, add this into the appropriate list 
				BEFORE removing it from the label name stack */
			if (FIELDML_LABEL_TYPE_ELEMENT_INTERPOLATION ==
				fieldml_data->current_label_name->type)
			{
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(
					fieldml_data->current_label_name,
					fieldml_data->element_interpolations);
			}
			else if (fieldml_data->current_element_labels)
			{
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(
					fieldml_data->current_label_name,
					fieldml_data->current_element_labels);
			}
			REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
				fieldml_data->label_name_stack);
			fieldml_data->current_label_name = (struct Fieldml_label_name *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_label_name.  Not in a label name.");
	}
	
	LEAVE;
} /* fieldml_end_label_name */

static void fieldml_start_assign_labels(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *template_name;
	int i;

	ENTER(fieldml_start_assign_labels);

	template_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "template_name"))
			{
				template_name = attribute_value;
			}
			i += 2;
		}
	}
	if (template_name)
	{
		fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
			template_name, FIELDML_LABEL_TYPE_ASSIGN_LABELS,
			fieldml_data->current_label_name, fieldml_data->label_name_stack);
		fieldml_data->expecting_characters = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_assign_labels.  Unable to parse label template name.");		
	}
	
	LEAVE;
} /* fieldml_start_assign_labels */

static void fieldml_end_assign_labels(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_end_assign_labels);

	if (fieldml_data->current_label_name->type == FIELDML_LABEL_TYPE_ASSIGN_LABELS)
	{
		/* Copy the character data */
		if (fieldml_data->character_buffer && fieldml_data->buffer_length)
		{
			if (ALLOCATE(fieldml_data->current_label_name->character_buffer, char,
				strlen(fieldml_data->character_buffer) + 1))
			{
				strcpy(fieldml_data->current_label_name->character_buffer,
					fieldml_data->character_buffer);
			}
		}
		fieldml_data->expecting_characters = 0;

		REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
			fieldml_data->label_name_stack);
		fieldml_data->current_label_name = Last_Fieldml_label_name_in_list(
			fieldml_data->label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_assign_labels.  Not assigning a labels template.");
	}
	
	LEAVE;
} /* fieldml_end_assign_labels */

static void fieldml_start_labels_template(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *template_name;
	int i;

	ENTER(fieldml_start_labels_template);

	if (!(fieldml_data->current_label_name))
	{
		template_name = (char *)NULL;
		i = 0;
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "name"))
			{
				template_name = attribute_value;
			}
			i += 2;
		}
		if (template_name)
		{
			fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
				template_name, FIELDML_LABEL_TYPE_LABELS_TEMPLATE,
				(struct Fieldml_label_name *)NULL, fieldml_data->label_name_stack);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fieldml_start_labels_template.  Unable to parse label template name.");		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_labels_template.  Already working on a label template.");		
	}
	
	LEAVE;
} /* fieldml_start_labels_template */

static void fieldml_end_labels_template(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_labels_template);

	if (fieldml_data->current_label_name)
	{
		if (!(ADD_OBJECT_TO_LIST(Fieldml_label_name)(
			fieldml_data->current_label_name, fieldml_data->label_templates)))
		{
			display_message(ERROR_MESSAGE, "fieldml_end_labels_template.  "
				"Unable to add template to list, possibly duplicate name.");
		}
		REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
			fieldml_data->label_name_stack);
		fieldml_data->current_label_name = (struct Fieldml_label_name *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_labels_template.  Not working on a labels template.");		
	}
	
	LEAVE;
} /* fieldml_end_labels_template */

static void fieldml_start_labels_template_ref(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *label_name;
	int i;
	struct Fieldml_label_name *label;

	ENTER(fieldml_start_labels_template_ref);

	label_name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "ref"))
			{
				label_name = attribute_value;
			}
			i += 2;
		}
	}
	if (label_name)
	{
		if (label = FIND_BY_IDENTIFIER_IN_LIST(Fieldml_label_name,name)(
			label_name, fieldml_data->label_templates))
		{
			if (fieldml_data->current_label_name)
			{
				if (!fieldml_data->current_label_name->child_labels)
				{
					fieldml_data->current_label_name->child_labels = 
						CREATE(LIST(Fieldml_label_name))();
				}
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
					fieldml_data->current_label_name->child_labels);
			}
			else
			{
				display_message(ERROR_MESSAGE, "fieldml_start_labels_template_ref.  "
					"Not in a label tree.");
			}
			ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
				fieldml_data->label_name_stack);
			fieldml_data->current_label_name = label;
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_labels_template_ref.  "
				"Unable to find labels_template_ref %s.", label_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_labels_template_ref.  Unable to parse labels_template_ref ref.");
	}
	
	LEAVE;
} /* fieldml_start_labels_template_ref */

static void fieldml_start_field(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *field_name, *value_type_string,
		*coordinate_system_string, *coordinate_system_focus_string;
	enum Value_type value_type;
	int i;
	struct Coordinate_system coordinate_system;

	ENTER(fieldml_start_field);

	if (!(fieldml_data->current_field))
	{
		field_name = (char *)NULL;
		value_type_string = (char *)NULL;
		coordinate_system_string = (char *)NULL;
		coordinate_system_focus_string = (char *)NULL;
		i = 0;	
		if (attributes)
		{
			while (attributes[i])
			{
				attribute_name = (char *)attributes[i];
				attribute_value = (char *)attributes[i + 1];
				if (!strcmp(attribute_name, "name"))
				{
					field_name = attribute_value;
				}
				else if (!strcmp(attribute_name, "value_type"))
				{
					value_type_string = attribute_value;
				}
				else if (!strcmp(attribute_name, "coordinate_system"))
				{
					coordinate_system_string = attribute_value;
				}
				else if (!strcmp(attribute_name, "focus"))
				{
					coordinate_system_focus_string = attribute_value;
				}
				i += 2;
			}
		}
		if (field_name)
		{
			fieldml_data->current_field = CREATE(FE_field)(field_name,
				fieldml_data->current_fe_region);
			set_FE_field_CM_field_type(fieldml_data->current_field, CM_COORDINATE_FIELD);
			if (value_type_string &&
				(value_type = Value_type_from_string(value_type_string)))
			{
				set_FE_field_value_type(fieldml_data->current_field, value_type);
			}
			if (coordinate_system_string)
			{
				if (fuzzy_string_compare_same_length(coordinate_system_string,
					"rectangular cartesian"))
				{
					coordinate_system.type=RECTANGULAR_CARTESIAN;
				}
				else if (fuzzy_string_compare_same_length(coordinate_system_string,
				   "cylindrical polar"))
				{
					coordinate_system.type=CYLINDRICAL_POLAR;
				}
				else if (fuzzy_string_compare_same_length(coordinate_system_string,
               "spherical polar"))
				{
					coordinate_system.type=SPHERICAL_POLAR;
				}
				else if (fuzzy_string_compare_same_length(coordinate_system_string,
					"prolate spheroidal"))
				{
					coordinate_system.type=PROLATE_SPHEROIDAL;
				}
				else if (fuzzy_string_compare_same_length(coordinate_system_string,
					"oblate spheroidal"))
				{
					coordinate_system.type=OBLATE_SPHEROIDAL;
				}
				else if (fuzzy_string_compare_same_length(coordinate_system_string,
					"fibre"))
				{
					coordinate_system.type=FIBRE;
				}
				else
				{
					coordinate_system.type=UNKNOWN_COORDINATE_SYSTEM;
					display_message(ERROR_MESSAGE, "fieldml_start_fieldml.  "
						"Unable to parse coordinate system name.");		
				}
				coordinate_system.parameters.focus = 0;
				if (coordinate_system_focus_string)
				{
					sscanf(coordinate_system_focus_string, "%f",
						&coordinate_system.parameters.focus);
				}
				set_FE_field_coordinate_system(fieldml_data->current_field,
					&coordinate_system);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fieldml_start_field.  Unable to parse field name.");		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_field.  Already working on a field.");		
	}
	
	LEAVE;
} /* fieldml_start_field */

static void fieldml_end_field(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_field);

	if (fieldml_data->current_field)
	{
		FE_region_merge_FE_field(fieldml_data->current_fe_region,
			fieldml_data->current_field);
		fieldml_data->current_field = (struct FE_field *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_field.  Not working on a field.");		
	}
	
	LEAVE;
} /* fieldml_end_field */

static void fieldml_start_field_component(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *component_name;
	int i;

	ENTER(fieldml_start_field);

	if (fieldml_data->current_field && !fieldml_data->current_field_component_name)
	{
		i = 0;
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "name"))
			{
				component_name = attribute_value;
			}
			i += 2;
		}
		if (component_name)
		{
			fieldml_data->current_field_component_name = 
				duplicate_string(component_name);
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_field_component.  "
				"Unable to parse field component name.");		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "fieldml_start_field_component.  "
			"Not working on a field or already in a component.");
	}
	
	LEAVE;
} /* fieldml_start_field_component */

static void fieldml_end_field_component(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	int component_number;

	ENTER(fieldml_end_field);
	if (fieldml_data->current_field && fieldml_data->current_field_component_name)
	{
		component_number = get_FE_field_number_of_components(
			fieldml_data->current_field);
		set_FE_field_number_of_components(fieldml_data->current_field,
			component_number + 1);
		set_FE_field_component_name(fieldml_data->current_field, component_number,
			fieldml_data->current_field_component_name);
		DEALLOCATE(fieldml_data->current_field_component_name);		
		fieldml_data->current_field_component_name = (char *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_field_component.  Not working on a field component.");
	}
	
	LEAVE;
} /* fieldml_end_field_component */

static void fieldml_start_node(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *node_name;
	int i;

	ENTER(fieldml_start_node);

	if (!(fieldml_data->current_node))
	{
		node_name = (char *)NULL;
		i = 0;
		if (attributes)
		{
			while (attributes[i])
			{
				attribute_name = (char *)attributes[i];
				attribute_value = (char *)attributes[i + 1];
				if (!strcmp(attribute_name, "name"))
				{
					node_name = attribute_value;
				}
				i += 2;
			}
		}
		if (node_name)
		{
			fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
				node_name, FIELDML_LABEL_TYPE_NODE,
				fieldml_data->current_label_name, fieldml_data->label_name_stack);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fieldml_start_node.  Unable to parse node name.");		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_node.  Already working on a node.");		
	}
	
	LEAVE;
} /* fieldml_start_node */

static void fieldml_end_node(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	int node_number;
	struct Fieldml_label_name *equivalent_node_label;

	ENTER(fieldml_end_node);

	if (fieldml_data->current_label_name->type == FIELDML_LABEL_TYPE_NODE)
	{
		node_number = 0;
		sscanf(fieldml_data->current_label_name->name, "%d", &node_number);
		if (node_number)
		{
			if (equivalent_node_label = FIRST_OBJECT_IN_LIST_THAT(Fieldml_label_name)(
				Fieldml_label_names_define_equivalent_nodes, 
				(void *)fieldml_data->current_label_name,
				fieldml_data->node_label_templates))
			{
				/* An equivalent node already exist so copy that one */
				fieldml_data->current_node = CREATE(FE_node)(
					node_number, (struct FE_region *)NULL, 
					equivalent_node_label->current_node);
				fieldml_data->fields_already_defined = 1;
			}
			else
			{
				fieldml_data->current_node = CREATE(FE_node)(
					node_number, fieldml_data->current_fe_region, (struct FE_node *)NULL);
			}
			if (FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
				Fieldml_label_name_process_label_in_node,
				fieldml_data, fieldml_data->current_label_name->child_labels))
			{
				FE_region_merge_FE_node(fieldml_data->current_fe_region,
					fieldml_data->current_node);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"fieldml_end_node.  Error defining node name %s.",
					fieldml_data->current_label_name->name);		
			}
			if (!equivalent_node_label)
			{
				fieldml_data->current_label_name->current_node = fieldml_data->current_node;
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
					fieldml_data->node_label_templates);	
			}
			fieldml_data->fields_already_defined = 0;
			fieldml_data->current_node = (struct FE_node *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fieldml_end_node.  Unable to parse node name %s.",
				fieldml_data->current_label_name->name);		
		}

		REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
			fieldml_data->label_name_stack);
		fieldml_data->current_label_name = Last_Fieldml_label_name_in_list(
			fieldml_data->label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_node.  Not working on a node.");		
	}
	
	LEAVE;
} /* fieldml_end_node */

static struct FE_element_shape *fieldml_read_FE_element_shape(
	char *shape_string, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *index;
	int component, dimension, i, j, *first_simplex, 
		number_of_polygon_vertices, previous_component, return_code, *type, 
		*temp_entry, *type_entry, xi_number;
	struct FE_element_shape *shape;

	ENTER(fieldml_read_FE_element_shape);

	return_code = 1;
	dimension=1;

	index = shape_string;
	while (index = strchr(index,'*'))
	{
		index++;
		dimension++;
	}
	index = shape_string;
	if (ALLOCATE(type, int, (dimension*(dimension+1))/2))
	{
		type_entry=type;
		for (xi_number = 0 ; return_code && (xi_number < dimension) ; xi_number++)
		{
			if (0==strncmp(index,"line",4))
			{
				index += 4;
				*type_entry=LINE_SHAPE;
				type_entry++;
				for (i=dimension-xi_number-1;i>0;i--)
				{
					*type_entry=0;
					type_entry++;
				}
			}
			else
			{
				if (0==strncmp(index,"polygon",7))
				{
					index += 7;
					while (' ' == *index)
					{
						index++;
					}
					if ('('== *index)
					{
						/* assign link to second polygon coordinate */
						if ((2==sscanf(index,"(%d ;%d )%n",
								  &number_of_polygon_vertices,&component,&i))&&
							(3<=number_of_polygon_vertices)&&
							(xi_number+1<component)&&(component<=dimension)&&
							('*'==index[i]))
						{
							index += i;
							*type_entry=POLYGON_SHAPE;
							type_entry++;
							i=xi_number+2;
							while (i<component)
							{
								*type_entry=0;
								type_entry++;
								i++;
							}
							*type_entry=number_of_polygon_vertices;
							type_entry++;
							while (i<dimension)
							{
								*type_entry=0;
								type_entry++;
								i++;
							}
						}
						else
						{
							return_code = 0;
						}
					}
					else
					{
						/* check for link to first polygon coordinate */
						temp_entry=type_entry;
						i=xi_number;
						j=dimension-xi_number-1;
						number_of_polygon_vertices=0;
						while (type&&(i>0))
						{
							j++;
							temp_entry -= j;
							if (*temp_entry)
							{
								if (0<number_of_polygon_vertices)
								{
									return_code = 0;
								}
								else
								{
									if (!((POLYGON_SHAPE==temp_entry[i-xi_number-1])&&
										((number_of_polygon_vertices= *temp_entry)>=3)))
									{
										return_code = 0;
									}
								}
							}
							i--;
						}
						if (type&&(3<=number_of_polygon_vertices))
						{
							*type_entry=POLYGON_SHAPE;
							type_entry++;
							for (i=dimension-xi_number-1;i>0;i--)
							{
								*type_entry=0;
								type_entry++;
							}
						}
						else
						{
							return_code = 0;
						}
					}
				}
				else
				{
					if (0==strncmp(index,"simplex",7))
					{
						index += 7;
						while (' '== *index)
						{
							index++;
						}
						if ('('== *index)
						{
							/* assign link to succeeding simplex coordinate */
							previous_component=xi_number+2;
							if ((1 == sscanf(index, "(%d %n",
								&component, &i)) &&
								(previous_component <= component) &&
								(component <= dimension))
							{
								*type_entry=SIMPLEX_SHAPE;
								type_entry++;
								do
								{
									index += i;
									while (previous_component<component)
									{
										*type_entry=0;
										type_entry++;
										previous_component++;
									}
									*type_entry=1;
									type_entry++;
									previous_component++;
								} while ((')'!=index[0])&&
									(1==sscanf(index,"%*[; ]%d %n",
										&component,&i))&&(previous_component<=component)&&
									(component<=dimension));
								if (')'==index[0])
								{
									/* fill rest of shape_type row with zeroes */
									while (previous_component <= dimension)
									{
										*type_entry=0;
										type_entry++;
										previous_component++;
									}
									index++;
								}
								else
								{
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							/* check for link to previous simplex coordinate */
							temp_entry=type_entry;
							i=xi_number;
							j=dimension-xi_number-1;
							first_simplex=(int *)NULL;
							while (type&&(i>0))
							{
								j++;
								temp_entry -= j;
								if (*temp_entry)
								{
									if (SIMPLEX_SHAPE==temp_entry[i-xi_number-1])
									{
										first_simplex=temp_entry;
									}
									else
									{
										return_code = 0;
									}
								}
								i--;
							}
							if (type&&first_simplex)
							{
								*type_entry=SIMPLEX_SHAPE;
								type_entry++;
								first_simplex++;
								for (i=dimension-xi_number-1;i>0;i--)
								{
									*type_entry= *first_simplex;
									type_entry++;
									first_simplex++;
								}
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
			}
			while (' ' == *index)
			{
				index++;
			}
			if ('*' == *index)
			{
				index++;
			}
			else
			{
				if ('\0' != *index)
				{
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			if (!(shape=CREATE(FE_element_shape)(dimension,type, fe_region)))
			{
				display_message(ERROR_MESSAGE,
					"fieldml_read_FE_element_shape.  Error creating shape");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fieldml_read_FE_element_shape.  Error parsing shape description");
			shape = (struct FE_element_shape *)NULL;
		}
		DEALLOCATE(type);
	}

	LEAVE;

	return(shape);
} /* fieldml_read_FE_element_shape */

static void fieldml_start_element(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value;
	int i, element_number;
	struct FE_element_shape *shape;

	ENTER(fieldml_start_element);

	if (!(fieldml_data->current_element_number))
	{
		element_number = 0;
		shape = (struct FE_element_shape *)NULL;
		i = 0;
		if (attributes)
		{
			while (attributes[i])
			{
				attribute_name = (char *)attributes[i];
				attribute_value = (char *)attributes[i + 1];
				if (!strcmp(attribute_name, "name"))
				{
					sscanf(attribute_value, "%d", &element_number);
				}
				if (!strcmp(attribute_name, "shape"))
				{
					shape = fieldml_read_FE_element_shape(attribute_value,
						fieldml_data->current_fe_region);
				}
				i += 2;
			}
		}
		if (element_number && shape)
		{
			fieldml_data->current_element_number = element_number;
			fieldml_data->current_element_shape = shape;
			fieldml_data->current_element_labels =
				CREATE(LIST(Fieldml_label_name))();
			fieldml_data->current_element_interpolation_ref =
				(struct Fieldml_label_name *)NULL;
			fieldml_data->face_numbers = (int *)NULL;
			fieldml_data->number_of_faces = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fieldml_start_element.  Unable to parse element name or shape.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_element.  Already working on a element.");
	}
	
	LEAVE;
} /* fieldml_start_element */

static void fieldml_end_element(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
==============================================================================*/
{
	int j, number_of_local_nodes, maximum_number_of_scale_factor_lists;
	struct CM_element_information cm, face_identifier;
	struct FE_element *element_template, *face_element;

	ENTER(fieldml_end_element);

	if (fieldml_data->current_element_number)
	{
		/* Everything is an element */
		cm.type = CM_ELEMENT;
		cm.number = fieldml_data->current_element_number;
		if (fieldml_data->current_element_interpolation_ref &&
			fieldml_data->current_element_interpolation_ref->element_template)
		{
			element_template = 
				fieldml_data->current_element_interpolation_ref->element_template;
			fieldml_data->current_element = CREATE(FE_element)(
				&cm, (struct FE_element_shape *)NULL, 
				(struct FE_region *)NULL, element_template);
		}
		else
		{
			element_template = (struct FE_element *)NULL;
			fieldml_data->current_element = CREATE(FE_element)(
				&cm, fieldml_data->current_element_shape, 
				fieldml_data->current_fe_region, element_template);
		}
		if (fieldml_data->face_numbers)
		{
			face_identifier.type = CM_ELEMENT;
			for (j = 0 ; j < fieldml_data->number_of_faces ; j++)
			{
				if (fieldml_data->face_numbers[j])
				{
					face_identifier.number = fieldml_data->face_numbers[j];
					if (face_element = FE_region_get_FE_element_from_identifier(
						fieldml_data->current_fe_region, &face_identifier))
					{
						if (!set_FE_element_face(fieldml_data->current_element,j,
							face_element))
						{
							display_message(ERROR_MESSAGE,"read_FE_element.  "
								"Could not set face %d of element");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"read_FE_element.  "
							"Could not find face %d of element");
					}
				}
			}
			DEALLOCATE(fieldml_data->face_numbers);
			fieldml_data->number_of_faces = 0;
		}
		if (NUMBER_IN_LIST(Fieldml_label_name)(fieldml_data->current_element_labels))
		{
			if (fieldml_data->current_element_interpolation_ref)
			{
				if (fieldml_data->current_element_interpolation_ref->element_template
					&& fieldml_data->current_element_interpolation_ref->element_template_node_index_list
					&& fieldml_data->current_element_interpolation_ref->element_template_element_lookup_list)
				{
					number_of_local_nodes = 0;
					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_assign_local_index, (void *)&number_of_local_nodes,
						fieldml_data->current_element_interpolation_ref->element_template_node_index_list);

					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_process_label_in_element, (void *)fieldml_data,
						fieldml_data->current_element_interpolation_ref->element_template_node_index_list);
					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_set_nodes_in_element,
						(void *)fieldml_data->current_element,
						fieldml_data->current_element_interpolation_ref->element_template_node_index_list);					

					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_set_scale_factors_in_element,
						(void *)fieldml_data,
						fieldml_data->current_element_interpolation_ref->element_template_element_lookup_list);
				}
				else
				{
					fieldml_data->element_lookup_list =
						CREATE(LIST(Fieldml_label_name))();
					fieldml_data->element_node_index_list =
						CREATE(LIST(Fieldml_label_name))();
					Fieldml_label_name_create_node_and_element_lookup_lists(
						fieldml_data->current_element_interpolation_ref,
						(void *)fieldml_data);

					number_of_local_nodes = 0;
					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_assign_local_index, (void *)&number_of_local_nodes,
						fieldml_data->element_node_index_list);
				
					maximum_number_of_scale_factor_lists =
						NUMBER_IN_LIST(Fieldml_label_name)(fieldml_data->element_lookup_list);
					fieldml_data->scale_factor_list_identifiers = (void **)NULL;
					fieldml_data->scale_factor_list_sizes = (int *)NULL;
					if (maximum_number_of_scale_factor_lists)
					{
						if (ALLOCATE(fieldml_data->scale_factor_list_identifiers, void *,
								 maximum_number_of_scale_factor_lists) &&
							ALLOCATE(fieldml_data->scale_factor_list_sizes, int,
								maximum_number_of_scale_factor_lists))
						{
							fieldml_data->number_of_scale_factor_lists = 0;
							fieldml_data->scale_factor_list_offset = 0;
							FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
								Fieldml_label_name_assign_scale_factor_lists, 
								(void *)fieldml_data, fieldml_data->element_lookup_list);
							/* When we get Richard's API separate the node scale field
								info call */
						}
					}

					set_FE_element_number_of_nodes(fieldml_data->current_element,
						number_of_local_nodes);
					set_FE_element_number_of_scale_factor_sets(fieldml_data->current_element,
						fieldml_data->number_of_scale_factor_lists, 
						fieldml_data->scale_factor_list_identifiers,
						fieldml_data->scale_factor_list_sizes);

					Fieldml_label_name_process_label_in_element(
						fieldml_data->current_element_interpolation_ref,
						(void *)fieldml_data);

					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_set_nodes_in_element,
						(void *)fieldml_data->current_element,
						fieldml_data->element_node_index_list);

					FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
						Fieldml_label_name_set_scale_factors_in_element,
						(void *)fieldml_data,
						fieldml_data->element_lookup_list);

					if (fieldml_data->scale_factor_list_identifiers)
					{
						DEALLOCATE(fieldml_data->scale_factor_list_identifiers);
					}
					if (fieldml_data->scale_factor_list_sizes)
					{
						DEALLOCATE(fieldml_data->scale_factor_list_sizes);
					}

					/* Keep references to this element so we can use it for a 
						template */
					fieldml_data->current_element_interpolation_ref->element_template =
						fieldml_data->current_element;
					fieldml_data->current_element_interpolation_ref->element_template_node_index_list =
						fieldml_data->element_node_index_list;
					fieldml_data->current_element_interpolation_ref->element_template_element_lookup_list =
						fieldml_data->element_lookup_list;
					fieldml_data->element_lookup_list = (struct LIST (Fieldml_label_name) *)NULL;
					fieldml_data->element_node_index_list = (struct LIST (Fieldml_label_name) *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "fieldml_end_element.  "
					"Element has label lists but no interpolation.");
			}
		}
		FE_region_merge_FE_element(fieldml_data->current_fe_region,
			fieldml_data->current_element);
		if (fieldml_data->current_element_labels)
		{
			DESTROY(LIST(Fieldml_label_name))(&fieldml_data->current_element_labels);
		}
		fieldml_data->current_element_interpolation_ref =
			(struct Fieldml_label_name *)NULL;
		fieldml_data->current_element = (struct FE_element *)NULL;
		fieldml_data->current_element_number = 0;
		fieldml_data->current_element_shape = (struct FE_element_shape *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_element.  Not working on a element.");
	}
	
	LEAVE;
} /* fieldml_end_element */

static void fieldml_start_element_faces(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_start_element_faces);

	if (fieldml_data->current_element_number)
	{
		if (attributes && attributes[0])
		{
			display_message(ERROR_MESSAGE,
				"fieldml_start_element_faces.  Not expecting attributes for faces.");
		}
		/* Otherwise just wait for the completion of the faces element and
			then process the character buffer */
		fieldml_data->expecting_characters = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_element_faces.  Not working on an element.");
	}
	
	LEAVE;
} /* fieldml_start_element_faces */

static void fieldml_end_element_faces(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *index;
	int face_element_number, i, *face_number_list;

	ENTER(fieldml_end_element_faces);

	if (fieldml_data->current_element_number)
	{
		index = fieldml_data->character_buffer;
		face_element_number = 0;
		while ((index < fieldml_data->character_buffer + fieldml_data->buffer_length)
			&& (1 == sscanf(index, " %d%n", &face_element_number, &i)))
		{
			if (REALLOCATE(face_number_list, fieldml_data->face_numbers,
				int, fieldml_data->number_of_faces + 1))
			{
				face_number_list[fieldml_data->number_of_faces] = face_element_number;
				fieldml_data->face_numbers = face_number_list;
				fieldml_data->number_of_faces++;
			}
			index += i;
		}
		fieldml_data->expecting_characters = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_element_faces.  Not working on an element.");
	}
	
	LEAVE;
} /* fieldml_end_element_faces */

static struct FE_basis *fieldml_read_FE_basis(
	struct Fieldml_sax_data *fieldml_data,char *basis_description_string)
/*******************************************************************************
LAST MODIFIED : 27 October 2004

DESCRIPTION :
Reads a basis description from a string.
If the basis does not exist, it is created.  The basis is returned.
<basis_type> should be allocated outside the function to the following size:
1 + (number_of_xi_coordinates*(1 + number_of_xi_coordinates))/2
and on exit will contain the a copy of the type for the basis.
Some examples of basis descriptions in an input file are:
1. c.Hermite*c.Hermite*l.Lagrange  This has cubic variation in xi1 and xi2 and
	linear variation in xi3.
2. c.Hermite*l.simplex(3)*l.simplex  This has cubic variation in xi1 and 2-D
	linear simplex variation for xi2 and xi3.
3. polygon(5,3)*l.Lagrange*polygon  This has linear variation in xi2 and a 2-D
	5-gon for xi1 and xi3.
==============================================================================*/
{
	int *basis_type;
	struct FE_basis *basis;

	ENTER(fieldml_read_FE_basis);
	basis=(struct FE_basis *)NULL;
	if (fieldml_data&&basis_description_string)
	{
		if (basis_type=FE_basis_string_to_type_array(basis_description_string))
		{
			basis=FE_region_get_FE_basis_matching_basis_type(
				fieldml_data->current_fe_region,basis_type);
			DEALLOCATE(basis_type);
		}
		else
		{
			display_message(ERROR_MESSAGE,"fieldml_read_FE_basis.  "
				"Unable to convert basis string to type array");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_read_FE_basis.  Invalid argument(s)");
	}
	LEAVE;

	return (basis);
} /* fieldml_read_FE_basis */

static void fieldml_start_mapping(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *basis_string, *mapping_name,
		*modification_string;
	FE_element_field_component_modify modify;
	int i;
	struct FE_basis *basis;

	ENTER(fieldml_start_mapping);

	if (!(fieldml_data->current_basis_mapping))
	{
		basis_string = (char *)NULL;
	   mapping_name = (char *)NULL;
		modification_string = (char *)NULL;
		modify = (FE_element_field_component_modify)NULL;
		i = 0;
		if (attributes)
		{
			while (attributes[i])
			{
				attribute_name = (char *)attributes[i];
				attribute_value = (char *)attributes[i + 1];
				if (!strcmp(attribute_name, "name"))
				{
					mapping_name = attribute_value;
				}
				if (!strcmp(attribute_name, "basis"))
				{
					basis_string = attribute_value;
				}
				if (!strcmp(attribute_name, "modification"))
				{
					modification_string = attribute_value;
				}
				i += 2;
			}
		}
		if (mapping_name && basis_string)
		{
			if (modification_string)
			{
				/* determine the modify function */
				if (0 == strcmp("no modify", modification_string))
				{
					modify = (FE_element_field_component_modify)NULL;
				}
				else if (0 == strcmp("increasing in xi1",
								modification_string))
				{
					modify = theta_increasing_in_xi1;
				}
				else if (0 == strcmp("decreasing in xi1",
								modification_string))
				{
					modify = theta_decreasing_in_xi1;
				}
				else if (0 == strcmp("non-increasing in xi1",
								modification_string))
				{
					modify = theta_non_increasing_in_xi1;
				}
				else if (0 == strcmp("non-decreasing in xi1",
								modification_string))
				{
					modify = theta_non_decreasing_in_xi1;
				}
				else if (0 == strcmp("closest in xi1",
								modification_string))
				{
					modify = theta_closest_in_xi1;
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_start_mapping.  "
						"Invalid modify function");
				}
			}
			if (basis = fieldml_read_FE_basis(fieldml_data, basis_string))
			{
				if (fieldml_data->current_label_name = fieldml_create_label_name_in_hierarchy(
				mapping_name, FIELDML_LABEL_TYPE_BASIS_MAPPING,
				fieldml_data->current_label_name, fieldml_data->label_name_stack))
				{
					fieldml_data->current_basis_mapping =
						fieldml_data->current_label_name;
					fieldml_data->current_basis_mapping->basis = basis;
					fieldml_data->current_basis_mapping->modify = modify;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_mapping.  "
				"Missing mapping name or basis string.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_mapping.  Already working on a basis mapping.");
	}
	
	LEAVE;
} /* fieldml_start_mapping */

static void fieldml_end_mapping(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_mapping);

	if (fieldml_data->current_basis_mapping)
	{
		if (!(ADD_OBJECT_TO_LIST(Fieldml_label_name)(
			fieldml_data->current_basis_mapping, fieldml_data->basis_mappings)))
		{
			display_message(ERROR_MESSAGE, "fieldml_end_basis_mapping.  "
				"Unable to add template to list, possibly duplicate name.");		
			DESTROY(Fieldml_label_name)(&fieldml_data->current_basis_mapping);
		}
		if (fieldml_data->current_label_name)
		{
			REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
				fieldml_data->label_name_stack);
		}
		fieldml_data->current_basis_mapping = (struct Fieldml_label_name *)NULL;
		fieldml_data->current_label_name = (struct Fieldml_label_name *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_mapping.  Not working on an element.");
	}
	
	LEAVE;
} /* fieldml_end_mapping */

static void fieldml_start_element_interpolation_ref(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 21 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *name;
	int i;
	struct Fieldml_label_name *label;

	ENTER(fieldml_start_element_interpolation_ref);

	if (!(fieldml_data->current_element_interpolation_ref))
	{
		if (fieldml_data->current_element_number)
		{
			name = (char *)NULL;
			i = 0;
			if (attributes)
			{
				while (attributes[i])
				{
					attribute_name = (char *)attributes[i];
					attribute_value = (char *)attributes[i + 1];
					if (!strcmp(attribute_name, "ref"))
					{
						name = attribute_value;
					}
					i += 2;
				}
			}
			if (name)
			{
				if (label = FIND_BY_IDENTIFIER_IN_LIST(Fieldml_label_name,name)(
					name, fieldml_data->element_interpolations))
				{
					fieldml_data->current_element_interpolation_ref = label;
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_start_element_interpolation_ref.  "
						"Unable to find referenced element interpolation\"%s\".",
						name);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "fieldml_start_element_interpolation_ref.  "
					"Missing mapping name or basis string.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_start_element_interpolation_ref.  "
				"Not in an element which is required for an element_interpolation_ref.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "fieldml_start_element_interpolation_ref.  "
			"Already referencing an element interpolation for this element.");
	}
	
	LEAVE;
} /* fieldml_start_element_interpolation_ref */

static void fieldml_end_element_interpolation_ref(
	struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_element_interpolation_ref);

	if (fieldml_data->current_element_interpolation_ref)
	{
		/* Keep this reference for the element */
	}
	else
	{
		display_message(ERROR_MESSAGE, "fieldml_end_element_interpolation_ref.  "
			"Not in an element interpolation reference.");
	}
	
	LEAVE;
} /* fieldml_end_element_interpolation_ref */

static void fieldml_start_xml_element(struct Fieldml_sax_data *fieldml_data, 
	char *name, char **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_start_element);

	if (fieldml_data->current_field)
	{
		/* Fieldml field */
		switch (*name)
		{
			case 'c':
			{
				if (!strcmp(name, "component"))
				{
					fieldml_start_field_component(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			default:
			{
				fieldml_data->unknown_depth = 1;
			} break;
		}
	}
	else if (fieldml_data->current_label_name)
	{
		/* Fieldml label template */
		switch (*name)
		{
			case 'a':
			{
				if (!strcmp(name, "assign_labels"))
				{
					fieldml_start_assign_labels(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'c':
			{
				if (!strcmp(name, "coefficients"))
				{
					fieldml_start_coefficients(fieldml_data, attributes);
				}
				else if (!strcmp(name, "component_lookup"))
				{
					fieldml_start_component_lookup(fieldml_data, attributes);
				}
				else if (!strcmp(name, "component_ref"))
				{
					fieldml_start_component_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'e':
			{
				if (!strcmp(name, "element_lookup"))
				{
					fieldml_start_element_lookup(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "field_lookup"))
				{
					fieldml_start_field_lookup(fieldml_data, attributes);
				}
				else if (!strcmp(name, "field_ref"))
				{
					fieldml_start_field_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'l':
			{
				if (!strcmp(name, "label"))
				{
					fieldml_start_label_name(fieldml_data, attributes);
				}
				else if (!strcmp(name, "label_lookup"))
				{
					fieldml_start_label_lookup(fieldml_data, attributes);
				}
				else if (!strcmp(name, "labels_template_ref"))
				{
					fieldml_start_labels_template_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'm':
			{
				if (!strcmp(name, "mapping_ref"))
				{
					fieldml_start_mapping_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'n':
			{
				if (!strcmp(name, "node_index"))
				{
					fieldml_start_node_index(fieldml_data, attributes);
				}
				else if (!strcmp(name, "node_lookup"))
				{
					fieldml_start_node_lookup(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'p':
			{
				if (!strcmp(name, "product"))
				{
					fieldml_start_product(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			default:
			{
				fieldml_data->unknown_depth = 1;
			} break;
		}
	}
	else if (fieldml_data->current_element_number)
	{
		/* Fieldml node */
		switch (*name)
		{
			case 'e':
			{
				if (!strcmp(name, "element_interpolation_ref"))
				{
					fieldml_start_element_interpolation_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "faces"))
				{
					fieldml_start_element_faces(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'l':
			{
				if (!strcmp(name, "label"))
				{
					fieldml_start_label_name(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			default:
			{
				fieldml_data->unknown_depth = 1;
			} break;
		}
	}
	else
	{
		/* Fieldml top level */
		switch (*name)
		{
			case 'e':
			{
				if (!strcmp(name, "element"))
				{
					fieldml_start_element(fieldml_data, attributes);
				}
				else if (!strcmp(name, "element_interpolation"))
				{
					fieldml_start_element_interpolation(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "field"))
				{
					fieldml_start_field(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'l':
			{
				if (!strcmp(name, "labels_template"))
				{
					fieldml_start_labels_template(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'm':
			{
				if (!strcmp(name, "mapping"))
				{
					fieldml_start_mapping(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'n':
			{
				if (!strcmp(name, "node"))
				{
					fieldml_start_node(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			default:
			{
				fieldml_data->unknown_depth = 1;
			} break;
		}
		/* Clear out the character buffer */
		if (fieldml_data->character_buffer)
		{
			DEALLOCATE(fieldml_data->character_buffer);
			fieldml_data->character_buffer = (char *)NULL;
			fieldml_data->buffer_length = 0;
			fieldml_data->buffer_allocated_length = 0;
		}
	}
	LEAVE;
} /* fieldml_start_xml_element */

static void fieldml_end_xml_element(struct Fieldml_sax_data *fieldml_data,
	char *name)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(fieldml_end_element);

	if (fieldml_data && name)
	{
		return_code = 1;
		if (fieldml_data->current_field)
		{
			/* Fieldml field */
			switch (*name)
			{
				case 'c':
				{
					if (!strcmp(name, "component"))
					{
						fieldml_end_field_component(fieldml_data);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'f':
				{
					if (!strcmp(name, "field"))
					{
						fieldml_end_field(fieldml_data);
					}
					else
					{
						return_code = 0;
					}
				} break;
				default:
				{
					return_code = 0;
				} break;
			}
		}
		else if (fieldml_data->current_label_name)
		{
			/* Fieldml labels template */
			switch (*name)
			{
				case 'a':
				{
					if (!strcmp(name, "assign_labels"))
					{
						fieldml_end_assign_labels(fieldml_data);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'c':
				{
					if (!strcmp(name, "coefficients"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_COEFFICIENTS);
					}
					else if (!strcmp(name, "component_lookup"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_COMPONENT_LOOKUP);
					}
					else if (!strcmp(name, "component_ref"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_COMPONENT_REF);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'e':
				{
					if (!strcmp(name, "element_interpolation"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_ELEMENT_INTERPOLATION);
					}
					else if (!strcmp(name, "element_lookup"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_ELEMENT_LOOKUP);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'f':
				{
					if (!strcmp(name, "field_lookup"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_FIELD_LOOKUP);
					}
					else if (!strcmp(name, "field_ref"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_FIELD_REF);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'l':
				{
					if (!strcmp(name, "label"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_LABEL);
					}
					else if (!strcmp(name, "label_lookup"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_LABEL_LOOKUP);
					}
					else if (!strcmp(name, "labels_template"))
					{
						fieldml_end_labels_template(fieldml_data);
					}
					else if (!strcmp(name, "labels_template_ref"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_LABELS_TEMPLATE);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'm':
				{
					if (!strcmp(name, "mapping"))
					{
						fieldml_end_mapping(fieldml_data);
					}
					else if (!strcmp(name, "mapping_ref"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_BASIS_MAPPING);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'n':
				{
					if (!strcmp(name, "node"))
					{
						fieldml_end_node(fieldml_data);
					}
					else if (!strcmp(name, "node_index"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_NODE_INDEX);
					}
					else if (!strcmp(name, "node_lookup"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_NODE_LOOKUP);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'p':
				{
					if (!strcmp(name, "product"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_PRODUCT);
					}
					else
					{
						return_code = 0;
					}
				} break;
				default:
				{
					return_code = 0;
				} break;
			}
		}
		else if (fieldml_data->current_element_number)
		{
			/* Fieldml node */
			switch (*name)
			{
				case 'e':
				{
					if (!strcmp(name, "element"))
					{
						fieldml_end_element(fieldml_data);
					}
					else if (!strcmp(name, "element_interpolation_ref"))
					{
						fieldml_end_element_interpolation_ref(fieldml_data);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'f':
				{
					if (!strcmp(name, "faces"))
					{
						fieldml_end_element_faces(fieldml_data);
					}
					else
					{
						return_code = 0;
					}
				} break;
				case 'l':
				{
					if (!strcmp(name, "label"))
					{
						fieldml_end_label_name(fieldml_data,
							FIELDML_LABEL_TYPE_LABEL);
					}
					else
					{
						return_code = 0;
					}
				} break;
				default:
				{
					return_code = 0;
				} break;
			}
		}
		else
		{
			/* Fieldml top level */
			if (!strcmp(name, "fieldml"))
			{
				fieldml_end_fieldml(fieldml_data);
			}
			else
			{
				return_code = 0;
			}
		}
		/* Clear out the character buffer */
		if (fieldml_data->character_buffer)
		{
			DEALLOCATE(fieldml_data->character_buffer);
			fieldml_data->character_buffer = (char *)NULL;
			fieldml_data->buffer_length = 0;
			fieldml_data->buffer_allocated_length = 0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
				"Closing with an element \"%s\"which wasn't opened.", name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
			"Invalid arguments.");
	}

	LEAVE;
} /* fieldml_end_xml_element */

static void fieldml_start_group(
	struct Fieldml_sax_data *fieldml_data, char **attributes)
/*******************************************************************************
LAST MODIFIED : 4 September 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *name;
	int i;
	struct Cmiss_region *region;
	struct FE_region *fe_region;

	ENTER(fieldml_start_group);

	name = (char *)NULL;
	i = 0;
	if (attributes)
	{
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "name"))
			{
				name = attribute_value;
			}
			i += 2;
		}
	}
	if (name)
	{
		if (!(region = Cmiss_region_get_child_region_from_name(
					fieldml_data->root_region, name)))
		{
			region = CREATE(Cmiss_region)();
			if (!Cmiss_region_add_child_region(fieldml_data->root_region,
					 region, name, /*child_position*/-1))
			{
				display_message(ERROR_MESSAGE,
					"fieldml_start_group.  Could not add child region");
				DESTROY(Cmiss_region)(&region);
				region = (struct Cmiss_region *)NULL;
			}
		}
		if (region)
		{
			if (!(fe_region = Cmiss_region_get_FE_region(region)))
			{
				/*???RC Later allow separate namespace for group,
				  Use "Region name : NAME" token instead? */
				fe_region = CREATE(FE_region)(fieldml_data->root_fe_region, 
					fieldml_data->basis_manager, fieldml_data->element_shape_list);
				if (!Cmiss_region_attach_FE_region(region, fe_region))
				{
					display_message(ERROR_MESSAGE, "fieldml_start_group.  "
						"Could not attach finite element region");
					DESTROY(FE_region)(&fe_region);
					fe_region = (struct FE_region *)NULL;
				}
			}
		}
		if (region && fe_region)
		{
			fieldml_data->current_fe_region = fe_region;
			FE_region_begin_change(fieldml_data->current_fe_region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "fieldml_start_group.  "
			"Missing group name.");
	}
	
	LEAVE;
} /* fieldml_start_group */

static void fieldml_end_group(
	struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 4 September 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_group);

	FE_region_end_change(fieldml_data->current_fe_region);
	fieldml_data->current_fe_region = fieldml_data->root_fe_region;

	LEAVE;
} /* fieldml_end_group */

static void general_start_xml_element(void *user_data, const xmlChar *name,
	const xmlChar **const_attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char **attributes;
	struct Fieldml_sax_data *fieldml_data;

	ENTER(general_start_xml_element);

	if (fieldml_data = (struct Fieldml_sax_data *)user_data)
	{
		attributes = (char **)const_attributes;
		if (fieldml_data->unknown_depth > 0)
		{
			fieldml_data->unknown_depth++;
		}
		else
		{
			if (0 <= fieldml_data->fieldml_version)
			{
				fieldml_start_xml_element(fieldml_data, (char *)name, attributes);
			}
			else
			{
				switch (*name)
				{
					case 'f':
					{
						if (!strcmp((char *)name, "fieldml"))
						{
							fieldml_start_fieldml(fieldml_data, attributes);
						}
						else
						{
							fieldml_data->unknown_depth = 1;
						}
					} break;
					case 'g':
					{
						if (!strcmp((char *)name, "group"))
						{
							fieldml_start_group(fieldml_data, attributes);
						}
						else
						{
							fieldml_data->unknown_depth = 1;
						}
					} break;
					case 'r':
					{
						if (!strcmp((char *)name, "regionml"))
						{
							/* ignore for now */
						}
						else
						{
							fieldml_data->unknown_depth = 1;
						}
					} break;
					default:
					{
						fieldml_data->unknown_depth = 1;
					} break;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"general_start_xml_element.  Fieldml_data not defined.");
	}
	LEAVE;
} /* general_start_xml_element */

static void general_end_xml_element(void *user_data, const xmlChar *name)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_sax_data *fieldml_data;

	ENTER(general_end_xml_element);

	if (fieldml_data = (struct Fieldml_sax_data *)user_data)
	{
		if (fieldml_data->unknown_depth > 0)
		{
			fieldml_data->unknown_depth--;
		}
		else
		{
			if (0 <= fieldml_data->fieldml_version)
			{
				fieldml_end_xml_element(fieldml_data, (char *)name);
			}
			else
			{
				switch (*name)
				{
					case 'g':
					{
						if (!strcmp((char *)name, "group"))
						{
							fieldml_end_group(fieldml_data);
						}
						else
						{
							display_message(ERROR_MESSAGE, "general_end_xml_element.  "
								"Closing with an unknown element which wasn't opened.");
						}
					} break;
					case 'r':
					{
						if (!strcmp((char *)name, "regionml"))
						{
							/* ignore for now */
						}
						else
						{
							display_message(ERROR_MESSAGE, "general_end_xml_element.  "
								"Closing with an unknown element which wasn't opened.");
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE, "general_end_xml_element.  "
							"Closing with an unknown element which wasn't opened.");
					} break;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"general_end_xml_element.  Fieldml_data not defined.");
	}
	LEAVE;
} /* general_end_xml_element */

static void fieldml_sax_characters(struct Fieldml_sax_data *fieldml_data,
	char *characters, int length)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *buffer, *index;
	int allocate_blocksize = 1000, i, new_allocate, return_code;

	ENTER(fieldml_sax_characters);

	if (fieldml_data->expecting_characters)
	{
		/* Fieldml node */
		if (characters && length)
		{
			new_allocate = 0;
			while (fieldml_data->buffer_length + length > 
				fieldml_data->buffer_allocated_length + new_allocate)
			{
				new_allocate += allocate_blocksize;
			}
			if (new_allocate)
			{
				if (REALLOCATE(buffer, fieldml_data->character_buffer, char,
					fieldml_data->buffer_allocated_length + new_allocate + 1))
				{
					fieldml_data->character_buffer = buffer;
					fieldml_data->buffer_allocated_length += new_allocate;
				}
			}
			memcpy(fieldml_data->character_buffer + fieldml_data->buffer_length,
				characters, length);
			fieldml_data->buffer_length += length;
			fieldml_data->character_buffer[fieldml_data->buffer_length] = 0;
		}
	}
	else
	{
		return_code = 1;
		index = characters;
		for (i = 0 ; i < length ; i++)
		{
			if ((*index != ' ') && (*index != '\n') && (*index != '\t'))
			{
				return_code = 0;
			}
			index++;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"fieldml_sax_characters.  Unexpected non whitespace characters.");
		}
	}
	
	LEAVE;
} /* fieldml_sax_characters */

static void general_sax_characters(void *user_data, const xmlChar *ch, int len)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_sax_data *fieldml_data;

	ENTER(general_sax_characters);
	if (fieldml_data = (struct Fieldml_sax_data *)user_data)
	{
		if (fieldml_data->unknown_depth > 0)
		{
			/* ignore */
		}
		else
		{
			if (0 <= fieldml_data->fieldml_version)
			{
				fieldml_sax_characters(fieldml_data, (char *)ch, len);
			}
			else
			{
				/* ignore */
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"general_sax_characters.  Fieldml_data not defined.");
	}
	LEAVE;
} /* general_sax_characters */

static void fieldml_sax_warning(void *user_data, const char *msg, ...)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	va_list args;

	ENTER(fieldml_sax_warning);
	USE_PARAMETER(user_data);
	va_start(args, msg);

	display_message(WARNING_MESSAGE,
		"fieldml_sax_warning.  %s", msg, args);
	va_end(args);
	LEAVE;
} /* fieldml_sax_warning */

static void fieldml_sax_error(void *user_data, const char *msg, ...)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *sax_error;
	va_list args;

	ENTER(fieldml_sax_error);
	USE_PARAMETER(user_data);
	ALLOCATE(sax_error, char, strlen(msg) + 3000);
	va_start(args, msg);
	vsprintf(sax_error, msg, args);
	display_message(ERROR_MESSAGE,
		"fieldml_sax_error.  %s", sax_error);
	va_end(args);
	DEALLOCATE(sax_error);
	LEAVE;
} /* fieldml_sax_error */

static void fieldml_sax_fatalError(void *user_data, const char *msg, ...)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	va_list args;

	ENTER(fieldml_sax_fatalError);
	USE_PARAMETER(user_data);
	va_start(args, msg);
	display_message(ERROR_MESSAGE,
		"fieldml_sax_fatalError.  %s", msg, args);
	va_end(args);

	LEAVE;
} /* fieldml_sax_fatalError */

#if defined (OLD_CODE)
Now there is a function to use in libxml2 (although this has also been deprecated
	by SAX2).
static int specialXmlSAXParseFile(xmlSAXHandlerPtr sax, void *user_data, char *filename)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	int ret = 0;
	xmlParserCtxtPtr ctxt;

	ctxt = xmlCreateFileParserCtxt(filename);
	if (ctxt == NULL) return -1;
	ctxt->sax = sax;
	ctxt->userData = user_data;

	xmlParseDocument(ctxt);

	if (ctxt->wellFormed)
		ret = 0;

	else
		ret = -1;
	if (sax != NULL)
		ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
    
	return ret;
} /* specialXmlSAXParseFile */
#endif /* defined (OLD_CODE) */

struct Cmiss_region *parse_fieldml_file(char *filename,
	struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list)
/*******************************************************************************
LAST MODIFIED : 19 April 2007

DESCRIPTION :
Reads fieldml file <filename> and returns a Cmiss_region containing its
contents. A NULL object return indicates an error.
Up to the calling function to check, merge and destroy the returned
Cmiss_region.
==============================================================================*/
{
	int return_code;
	static xmlSAXHandler fieldml_handler;
	struct Cmiss_region *root_region;
	struct Fieldml_sax_data fieldml_data;

	ENTER(parse_fieldml_file);
	root_region = (struct Cmiss_region *)NULL;
	if (filename && basis_manager)
	{
		root_region = CREATE(Cmiss_region)();
		fieldml_data.unknown_depth = 0;
		fieldml_data.return_val = 0;
		fieldml_data.fieldml_version = -1; /* Not in fieldml yet */
		fieldml_data.fieldml_subversion = -1; /* Not in fieldml yet */
		fieldml_data.basis_manager = basis_manager;
		fieldml_data.element_shape_list = element_shape_list;
		fieldml_data.root_region = root_region;
		fieldml_data.root_fe_region =
			CREATE(FE_region)((struct FE_region *)NULL, basis_manager, element_shape_list);
		Cmiss_region_attach_FE_region(fieldml_data.root_region,
			fieldml_data.root_fe_region);

		Cmiss_region_begin_change(fieldml_data.root_region);
		FE_region_begin_change(fieldml_data.root_fe_region);

		fieldml_data.label_templates = CREATE(LIST(Fieldml_label_name))();
		fieldml_data.label_name_stack = CREATE(LIST(Fieldml_label_name))();
		fieldml_data.basis_mappings = CREATE(LIST(Fieldml_label_name))();
		fieldml_data.node_label_templates = CREATE(LIST(Fieldml_label_name))();
		fieldml_data.element_interpolations = CREATE(LIST(Fieldml_label_name))();
		fieldml_data.character_buffer = (char *)NULL;
		fieldml_data.buffer_length = 0;
		fieldml_data.buffer_allocated_length = 0;
		fieldml_data.expecting_characters = 0;
	
		fieldml_data.current_label_name = (struct Fieldml_label_name *)NULL;
		fieldml_data.current_fe_region = fieldml_data.root_fe_region;
		fieldml_data.current_field = (struct FE_field *)NULL;
		fieldml_data.current_field_ref = (struct FE_field *)NULL;
		fieldml_data.current_field_component_name = (char *)NULL;

		fieldml_data.current_node = (struct FE_node *)NULL;
		fieldml_data.current_node_field_creator = (struct FE_node_field_creator *)NULL;

		fieldml_data.current_basis_mapping = (struct Fieldml_label_name *)NULL;
		fieldml_data.current_element_interpolation_ref = (struct Fieldml_label_name *)NULL;
		fieldml_data.current_element = (struct FE_element *)NULL;
		fieldml_data.current_element_number = 0;
		fieldml_data.current_element_shape = (struct FE_element_shape *)NULL;
		fieldml_data.current_element_labels = (struct LIST(Fieldml_label_name) *)NULL;

		fieldml_data.current_element_field_components =
			(struct FE_element_field_component **)NULL;

		fieldml_data.number_of_scale_factor_lists = 0;
		fieldml_data.scale_factor_list_offset = 0;
		fieldml_data.scale_factor_list_sizes = (int *)NULL;

		fieldml_data.current_scale_factor_list_size = 0;
		fieldml_data.current_scale_factor_list_offset = 0;

		fieldml_data.number_of_faces = 0;
		fieldml_data.face_numbers = (int *)NULL;

		fieldml_data.fields_already_defined = 0;

		fieldml_data.current_value_list = (struct LIST(Fieldml_label_name) *)NULL;

		fieldml_handler.internalSubset = (internalSubsetSAXFunc)NULL;
		fieldml_handler.isStandalone = (isStandaloneSAXFunc)NULL;
		fieldml_handler.hasInternalSubset = (hasInternalSubsetSAXFunc)NULL;
		fieldml_handler.hasExternalSubset = (hasExternalSubsetSAXFunc)NULL;
		fieldml_handler.resolveEntity = (resolveEntitySAXFunc)NULL;
		fieldml_handler.getEntity = (getEntitySAXFunc)NULL;
		fieldml_handler.entityDecl = (entityDeclSAXFunc)NULL;
		fieldml_handler.notationDecl = (notationDeclSAXFunc)NULL;
		fieldml_handler.attributeDecl = (attributeDeclSAXFunc)NULL;
		fieldml_handler.elementDecl = (elementDeclSAXFunc)NULL;
		fieldml_handler.unparsedEntityDecl = (unparsedEntityDeclSAXFunc)NULL;
		fieldml_handler.setDocumentLocator = (setDocumentLocatorSAXFunc)NULL;
		fieldml_handler.startDocument = (startDocumentSAXFunc)NULL;
		fieldml_handler.endDocument = (endDocumentSAXFunc)NULL;

		fieldml_handler.startElement = general_start_xml_element;
		fieldml_handler.endElement = general_end_xml_element;

		fieldml_handler.reference = (referenceSAXFunc)NULL;

		fieldml_handler.characters = general_sax_characters;

		fieldml_handler.ignorableWhitespace = (ignorableWhitespaceSAXFunc)NULL;
		fieldml_handler.processingInstruction = (processingInstructionSAXFunc)NULL;
		fieldml_handler.comment = (commentSAXFunc)NULL;
		fieldml_handler.warning = fieldml_sax_warning;
		fieldml_handler.error = fieldml_sax_error;
		fieldml_handler.fatalError = fieldml_sax_fatalError;
	
		return_code = xmlSAXUserParseFile(&fieldml_handler, &fieldml_data,
			filename);
		FE_region_end_change(fieldml_data.root_fe_region);
		Cmiss_region_end_change(fieldml_data.root_region);
		if (return_code != 0)
		{
			DESTROY(Cmiss_region)(&root_region);
			root_region = (struct Cmiss_region *)NULL;
		}

		/* Clean up */
		DESTROY(LIST(Fieldml_label_name))(&fieldml_data.label_templates);
		DESTROY(LIST(Fieldml_label_name))(&fieldml_data.label_name_stack);
		DESTROY(LIST(Fieldml_label_name))(&fieldml_data.basis_mappings);
		DESTROY(LIST(Fieldml_label_name))(&fieldml_data.element_interpolations);
		DESTROY(LIST(Fieldml_label_name))(&fieldml_data.node_label_templates);
	}
	else
	{
		display_message(ERROR_MESSAGE, "parse_fieldml_file.  Invalid argument(s)");
	}
	LEAVE;

	return (root_region);
} /* parse_fieldml_file */
#endif /* defined (HAVE_XML2) */

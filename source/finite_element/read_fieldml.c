/*******************************************************************************
FILE : read_fieldml.c

LAST MODIFIED : 19 February 2003

DESCRIPTION :
The function for importing finite element data into CMISS.
==============================================================================*/
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
#include "finite_element/write_fieldml.h" /* SAB For temporary regions stuff */
#include "general/debug.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
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

struct Fieldml_label_name
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *name;
	enum FE_nodal_value_type nodal_value_type;
	struct FE_field *field;
	int field_component_flag; /* Indicates that this label represents a component */
	struct LIST(Fieldml_label_name) *child_labels;
	int access_count;
};

DECLARE_LIST_TYPES(Fieldml_label_name);
/* We need to maintain the order, so we do not want an indexed list */
FULL_DECLARE_LIST_TYPE(Fieldml_label_name);

struct Fieldml_labels_template
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *name;
	struct LIST(Fieldml_label_name) *labels;
	int access_count;
};

DECLARE_LIST_TYPES(Fieldml_labels_template);
/* We want a hash indexed by name so use the indexed list type */
FULL_DECLARE_INDEXED_LIST_TYPE(Fieldml_labels_template);

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
	struct FE_region *root_fe_region;
	struct LIST(Fieldml_labels_template) *label_templates;
	char *character_buffer;
	char *character_index;
	int buffer_length;
	int buffer_allocated_length;

	struct Fieldml_labels_template *current_assign_labels;
	struct Fieldml_labels_template *current_labels_template;
	struct Fieldml_label_name *current_label_name;
	struct LIST(Fieldml_label_name) *label_name_stack;
	struct FE_node *current_node;
	/* Only for defining a new field */
	struct FE_field *current_field;
	char *current_field_component_name;
	/* When referencing an existing field */
	struct FE_field *current_field_ref;
	struct FE_node_field_creator *current_node_field_creator;
	int current_component_number;
	int current_component_has_value_defined;
	int current_component_number_of_versions;
};

/*
Module functions
----------------
*/

/* Temporary delarations so that the code compiles before regions exist.
   Till end */
struct Cmiss_region *CREATE(Cmiss_region)(void)
{
	struct Cmiss_region *cmiss_region;

	ALLOCATE(cmiss_region, struct Cmiss_region, 1);
	cmiss_region->fe_region = (struct FE_region *)NULL;
	return (cmiss_region);
}
struct FE_region *CREATE(FE_region)(struct FE_region *master_fe_region,
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_node) *node_manager, struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager)
{
	struct FE_region *fe_region;

	USE_PARAMETER(master_fe_region);
	ALLOCATE(fe_region, struct FE_region, 1);
	fe_region->basis_manager = basis_manager;
	fe_region->node_manager = node_manager;
	fe_region->element_manager = element_manager;
	fe_region->fe_field_manager = fe_field_manager;
	return (fe_region);
}
int Cmiss_region_attach_FE_region(struct Cmiss_region *cmiss_region,
	struct FE_region *fe_region)
{
	cmiss_region->fe_region = fe_region;
	return(1);
}
int DESTROY(Cmiss_region)(struct Cmiss_region **region_address)
{
	DEALLOCATE(*region_address);
	return (1);
}
int DESTROY(FE_region)(struct FE_region **fe_region_address)
{
	DEALLOCATE(*fe_region_address);
	return (1);
}
struct FE_field *FE_region_create_FE_field(struct FE_region *fe_region,
	char *name)
{
	struct FE_field *field;
	USE_PARAMETER(fe_region);
	field = CREATE(FE_field)((struct FE_time *)NULL);
	set_FE_field_name(field, name);
	return(field);
}
struct FE_field *FE_region_merge_FE_field(struct FE_region *fe_region,
	struct FE_field **field_address)
{
	char *field_name;
	struct FE_field *field;

	field = *field_address;
	if (field_name = get_FE_field_name(field))
	{
		if (FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,
				 name)(field_name,fe_region->fe_field_manager))
		{
			display_message(ERROR_MESSAGE,
				"read_FE_field.  Cannot merge fe_fields of the same name %s",
				field_name);
			DESTROY(FE_field)(&field);
		}
		else
		{
			if (!ADD_OBJECT_TO_MANAGER(FE_field)(field,fe_region->fe_field_manager))
			{				
				display_message(ERROR_MESSAGE,
					"read_FE_field.  Error adding field %s to field_manager",
					field_name);
				DESTROY(FE_field)(&field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_FE_field.  Unable to get fe_field name");
		DESTROY(FE_field)(&field);
	}
	if (!field)
	{
		*field_address = (struct FE_field *)NULL;
	}
	return (field);
}
struct FE_field *FE_region_get_FE_field_from_name(struct FE_region *fe_region,
	char *field_name)
{
	struct FE_field *field;
	field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,
		name)(field_name,fe_region->fe_field_manager);
	return(field);
}
struct FE_node *FE_region_create_FE_node(struct FE_region *fe_region,
	int cm_node_identifier, struct FE_node *template_node)
{
	struct FE_node *node;
	USE_PARAMETER(fe_region);
	node = CREATE(FE_node)(cm_node_identifier, template_node);
	return(node);
}
struct FE_node *FE_region_merge_FE_node(struct FE_region *fe_region,
	struct FE_node **node_address)
{
	int node_number;
	struct FE_node *existing_node, *node;

	node = *node_address;
	node_number = get_FE_node_cm_node_identifier(node);
	if (existing_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
		cm_node_identifier)(node_number,fe_region->node_manager))
	{
		/* merge the values from the existing to the new */
		if (merge_FE_node(node,existing_node))
		{
			if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
				existing_node,node,fe_region->node_manager))
			{
				DESTROY(FE_node)(&node);
				node=existing_node;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_FE_node.  Error modifying node %d in node_manager", 
					node_number);
				DESTROY(FE_node)(&node);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_FE_node.  Error merging new information for node %d",
				node_number);
			DESTROY(FE_node)(&node);
		}
	}
	else
	{
		if (!ADD_OBJECT_TO_MANAGER(FE_node)(node,fe_region->node_manager))
		{				
			display_message(ERROR_MESSAGE,
				"read_FE_node.  Error adding node %d to node_manager",
				node_number);
			DESTROY(FE_node)(&node);
		}
	}
	if (!node)
	{
		*node_address = (struct FE_node *)NULL;
	}
	return (node);
}
/* end Temporary delarations so that the code compiles before regions exist. */

PROTOTYPE_LIST_FUNCTIONS(Fieldml_label_name);

static struct Fieldml_label_name *CREATE(Fieldml_label_name)(char *name)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

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
			label_name->field = (struct FE_field *)NULL;
			label_name->field_component_flag = 0;
			label_name->nodal_value_type = FE_NODAL_UNKNOWN;
			label_name->child_labels = (struct LIST(Fieldml_label_name) *)NULL;
			label_name->access_count=0;
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
			if (label_name->child_labels)
			{
				DESTROY(LIST(Fieldml_label_name))(&label_name->child_labels);
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

static int Fieldml_label_name_process_label(
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
	struct Fieldml_sax_data *fieldml_data;
	struct Node_time_index *node_time_index;

	ENTER(Fieldml_label_name_process_label);

	if (fieldml_data = (struct Fieldml_sax_data *)fieldml_data_void)
	{
		return_code = 1;
		if (label->field)
		{
			if (!(fieldml_data->current_field_ref))
			{
				fieldml_data->current_field_ref = label->field;
				number_of_components = get_FE_field_number_of_components(label->field);
				fieldml_data->current_node_field_creator = CREATE(FE_node_field_creator)
					(number_of_components);
				fieldml_data->current_component_number = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldml_label_name_process_label.  "
					"Already referencing a field.");
			}
		}
		if (label->field_component_flag)
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
		if (!(strncmp("version", label->name, 7)))
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
		if (label->child_labels)
		{
			ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
			return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(Fieldml_label_name_process_label,
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
		if (label->field_component_flag)
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
		if (label->field)
		{
			GET_NAME(FE_field)(label->field,&field_name);
			number_of_components = get_FE_field_number_of_components(label->field);
			if (number_of_components == fieldml_data->current_component_number)
			{
				if (fieldml_data->current_node)
				{
					/* Then define this field on the node */
					define_FE_field_at_node(fieldml_data->current_node, label->field,
						(struct FE_time_version *)NULL, 
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
				DESTROY(FE_node_field_creator)(&fieldml_data->current_node_field_creator);
				fieldml_data->current_node_field_creator = 
					(struct FE_node_field_creator *)NULL;
			}
			if (fieldml_data->current_node && fieldml_data->current_field_ref &&
				fieldml_data->character_buffer && fieldml_data->buffer_length)
			{
				node = fieldml_data->current_node;
				node_number = get_FE_node_cm_node_identifier(node);
				field = fieldml_data->current_field_ref;
				node_time_index = (struct Node_time_index *)NULL;
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
									if (node_time_index)
									{
										if (return_code=set_FE_nodal_field_FE_values_at_time(
												 field,node,values,&length,node_time_index->time))
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
	else
	{
		display_message(ERROR_MESSAGE,
			"Fieldml_label_name_process_label.  Missing fieldml data.");
		return_code = 0;
	}
	
	LEAVE;

	return(return_code);
} /* Fieldml_label_name_process_label */

PROTOTYPE_LIST_FUNCTIONS(Fieldml_labels_template);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Fieldml_labels_template,name, \
	char *);

static struct Fieldml_labels_template *CREATE(Fieldml_labels_template)(char *name)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_labels_template *label_template;

	ENTER(CREATE(Fieldml_labels_template));
	if (name)
	{
		if (ALLOCATE(label_template,struct Fieldml_labels_template,1)&&
			ALLOCATE(label_template->name,char,strlen(name)+1))
		{
			strcpy(label_template->name, name);
			label_template->labels = (struct LIST(Fieldml_label_name) *)NULL;
			label_template->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Fieldml_labels_template).  Could not allocate memory for node field");
			DEALLOCATE(label_template);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Fieldml_labels_template).  Invalid argument(s)");
		label_template=(struct Fieldml_labels_template *)NULL;
	}
	LEAVE;

	return (label_template);
} /* CREATE(Fieldml_labels_template) */

static int DESTROY(Fieldml_labels_template)(struct Fieldml_labels_template **label_template_address)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Frees the memory for the node field and sets <*label_template_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Fieldml_labels_template *label_template;

	ENTER(DESTROY(Fieldml_labels_template));
	if ((label_template_address)&&(label_template= *label_template_address))
	{
		if (0==label_template->access_count)
		{
			if (label_template->labels)
			{
				DESTROY(LIST(Fieldml_label_name))(&label_template->labels);
			}
			DEALLOCATE(label_template->name);
			DEALLOCATE(*label_template_address);
		}
		else
		{
			*label_template_address=(struct Fieldml_labels_template *)NULL;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Fieldml_labels_template) */

DECLARE_OBJECT_FUNCTIONS(Fieldml_labels_template)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Fieldml_labels_template,name,char *, \
	strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Fieldml_labels_template)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Fieldml_labels_template,name, \
	char *,strcmp)

static int Fieldml_labels_template_process_labels(
   struct Fieldml_labels_template *template, struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Fieldml_labels_template_process_labels);

	if (template && fieldml_data->character_buffer && fieldml_data->buffer_length)
	{
		if (template->labels)
		{
			return_code = FOR_EACH_OBJECT_IN_LIST(Fieldml_label_name)(
				Fieldml_label_name_process_label,
				(void *)fieldml_data, template->labels);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_fieldml.  Not in a fieldml block.");
		return_code = 0;
	}
	
	LEAVE;

	return (return_code);
} /* Fieldml_labels_template_process_labels */

void fieldml_start_fieldml(struct Fieldml_sax_data *fieldml_data,
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

void fieldml_end_fieldml(struct Fieldml_sax_data *fieldml_data)
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

struct Fieldml_label_name *fieldml_create_label_name_in_hierarchy(
	char *name, struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_label_name *label;

	ENTER(fieldml_create_label_name_in_hierarchy);

	if (name)
	{
		label = CREATE(Fieldml_label_name)(name);
		if (fieldml_data->current_node)
		{
			/* Adding the label to the stack will be sufficient */
		}
		else if (fieldml_data->current_labels_template)
		{
			if (fieldml_data->current_label_name)
			{
				/* This is a child of the current label name */
				if (!(fieldml_data->current_label_name->child_labels))
				{
					fieldml_data->current_label_name->child_labels = 
						CREATE(LIST(Fieldml_label_name))();
				}
				/* Add this label to the parent */
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, 
					fieldml_data->current_label_name->child_labels);
			}
			else 			/* This is at the top level of a template */
			{
				if (!(fieldml_data->current_labels_template->labels))
				{
					fieldml_data->current_labels_template->labels = 
						CREATE(LIST(Fieldml_label_name))();
				}
				/* Add this label to the template */
				ADD_OBJECT_TO_LIST(Fieldml_label_name)(label,
					fieldml_data->current_labels_template->labels);	
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_create_label_name_in_hierarchy.  "
				"Not in a labels template or a node.");
		}
		/* Add this label to the stack */
		ADD_OBJECT_TO_LIST(Fieldml_label_name)(label, fieldml_data->label_name_stack);
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

void fieldml_start_label_name(struct Fieldml_sax_data *fieldml_data,
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
	if (label_name)
	{
		fieldml_data->current_label_name = 
			fieldml_create_label_name_in_hierarchy(label_name, fieldml_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_label_name.  Unable to parse label template name.");		
	}
	
	LEAVE;
} /* fieldml_start_label_name */

void fieldml_start_field_ref(struct Fieldml_sax_data *fieldml_data,
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
	if (label_name)
	{
		if (field = FE_region_get_FE_field_from_name(fieldml_data->root_fe_region,
			label_name))
		{
			fieldml_data->current_label_name = 
				fieldml_create_label_name_in_hierarchy(label_name, fieldml_data);
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

void fieldml_start_component_ref(struct Fieldml_sax_data *fieldml_data,
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
	if (label_name)
	{
		fieldml_data->current_label_name = 
			fieldml_create_label_name_in_hierarchy(label_name, fieldml_data);
		fieldml_data->current_label_name->field_component_flag = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_component_ref.  Unable to parse component_ref ref.");
	}
	
	LEAVE;
} /* fieldml_start_component_ref */

void fieldml_end_label_name(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_label_name);

	if (fieldml_data->current_label_name)
	{
		REMOVE_OBJECT_FROM_LIST(Fieldml_label_name)(fieldml_data->current_label_name,
			fieldml_data->label_name_stack);
		/* Go up the stack of label names, if it is empty then set to NULL */
		fieldml_data->current_label_name = Last_Fieldml_label_name_in_list(
			fieldml_data->label_name_stack);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_label_name.  Not in a label name.");
	}
	
	LEAVE;
} /* fieldml_end_label_name */

void fieldml_start_assign_labels(struct Fieldml_sax_data *fieldml_data,
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
	if (template_name)
	{
		if (fieldml_data->current_assign_labels)
		{
			display_message(ERROR_MESSAGE, "fieldml_start_assign_labels.  "
				"Already assigning a labels template.");
		}
		else
		{
			if (!(fieldml_data->current_assign_labels = FIND_BY_IDENTIFIER_IN_LIST
				(Fieldml_labels_template,name)(template_name, fieldml_data->label_templates)))
			{
				display_message(ERROR_MESSAGE, "fieldml_start_assign_labels.  "
					"Unable to find a template name %s.", template_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_start_assign_labels.  Unable to parse label template name.");		
	}
	
	LEAVE;
} /* fieldml_start_assign_labels */

void fieldml_end_assign_labels(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_end_assign_labels);

	if (fieldml_data->current_assign_labels)
	{
		/* Process the character data */
		if (fieldml_data->character_buffer && fieldml_data->buffer_length)
		{
			fieldml_data->character_index = fieldml_data->character_buffer;
			Fieldml_labels_template_process_labels(fieldml_data->current_assign_labels,
				fieldml_data);
		}
		fieldml_data->current_assign_labels = (struct Fieldml_labels_template *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_assign_labels.  Not assigning a labels template.");
	}
	
	LEAVE;
} /* fieldml_end_assign_labels */

void fieldml_start_labels_template(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value, *template_name;
	int i;

	ENTER(fieldml_start_labels_template);

	if (!(fieldml_data->current_labels_template))
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
			fieldml_data->current_labels_template =
				CREATE(Fieldml_labels_template)(template_name);
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

void fieldml_end_labels_template(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_labels_template);

	if (fieldml_data->current_labels_template)
	{
		if (!(ADD_OBJECT_TO_LIST(Fieldml_labels_template)(
			fieldml_data->current_labels_template, fieldml_data->label_templates)))
		{
			display_message(ERROR_MESSAGE, "fieldml_end_labels_template.  "
				"Unable to add template to list, possibly duplicate name.");		
			DESTROY(Fieldml_labels_template)(&fieldml_data->current_labels_template);
		}
		fieldml_data->current_labels_template = (struct Fieldml_labels_template *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_labels_template.  Not working on a labels template.");		
	}
	
	LEAVE;
} /* fieldml_end_labels_template */

void fieldml_start_field(struct Fieldml_sax_data *fieldml_data,
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
		if (field_name)
		{
			fieldml_data->current_field = FE_region_create_FE_field(
				fieldml_data->root_fe_region, field_name);
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

void fieldml_end_field(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_field);

	if (fieldml_data->current_field)
	{
		FE_region_merge_FE_field(fieldml_data->root_fe_region,
			&fieldml_data->current_field);
		fieldml_data->current_field = (struct FE_field *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_field.  Not working on a field.");		
	}
	
	LEAVE;
} /* fieldml_end_field */

void fieldml_start_field_component(struct Fieldml_sax_data *fieldml_data,
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

void fieldml_end_field_component(struct Fieldml_sax_data *fieldml_data)
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

void fieldml_start_node(struct Fieldml_sax_data *fieldml_data,
	char **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *attribute_name, *attribute_value;
	int i, node_number;

	ENTER(fieldml_start_node);

	if (!(fieldml_data->current_node))
	{
		node_number = 0;
		i = 0;
		while (attributes[i])
		{
			attribute_name = (char *)attributes[i];
			attribute_value = (char *)attributes[i + 1];
			if (!strcmp(attribute_name, "name"))
			{
				sscanf(attribute_value, "%d", &node_number);
			}
			i += 2;
		}
		if (node_number)
		{
			fieldml_data->current_node = FE_region_create_FE_node(
			 	fieldml_data->root_fe_region, node_number, (struct FE_node *)NULL);
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

void fieldml_end_node(struct Fieldml_sax_data *fieldml_data)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{

	ENTER(fieldml_end_node);

	if (fieldml_data->current_node)
	{
		FE_region_merge_FE_node(fieldml_data->root_fe_region,
			&fieldml_data->current_node);
		fieldml_data->current_node = (struct FE_node *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fieldml_end_node.  Not working on a node.");		
	}
	
	LEAVE;
} /* fieldml_end_node */

void fieldml_start_xml_element(struct Fieldml_sax_data *fieldml_data, 
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
	else if (fieldml_data->current_labels_template)
	{
		/* Fieldml label template */
		switch (*name)
		{
			case 'c':
			{
				if (!strcmp(name, "component_ref"))
				{
					fieldml_start_component_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "field_ref"))
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
	else if (fieldml_data->current_node)
	{
		/* Fieldml node */
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
				if (!strcmp(name, "component_ref"))
				{
					fieldml_start_component_ref(fieldml_data, attributes);
				}
				else
				{
					fieldml_data->unknown_depth = 1;
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "field_ref"))
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

void fieldml_end_xml_element(struct Fieldml_sax_data *fieldml_data,
	char *name)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	ENTER(fieldml_end_element);

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
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
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
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
					"Closing with an unknown element which wasn't opened.");
			} break;
		}
	}
	else if (fieldml_data->current_labels_template)
	{
		/* Fieldml labels template */
		switch (*name)
		{
			case 'c':
			{
				if (!strcmp(name, "component_ref"))
				{
					fieldml_end_label_name(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "field_ref"))
				{
					fieldml_end_label_name(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			case 'l':
			{
				if (!strcmp(name, "label"))
				{
					fieldml_end_label_name(fieldml_data);
				}
				else if (!strcmp(name, "labels_template"))
				{
					fieldml_end_labels_template(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
					"Closing with an unknown element which wasn't opened.");
			} break;
		}
	}
	else if (fieldml_data->current_node)
	{
		/* Fieldml node */
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
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			case 'c':
			{
				if (!strcmp(name, "component_ref"))
				{
					fieldml_end_label_name(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			case 'f':
			{
				if (!strcmp(name, "field_ref"))
				{
					fieldml_end_label_name(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			case 'l':
			{
				if (!strcmp(name, "label"))
				{
					fieldml_end_label_name(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			case 'n':
			{
				if (!strcmp(name, "node"))
				{
					fieldml_end_node(fieldml_data);
				}
				else
				{
					display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
						"Closing with an unknown element which wasn't opened.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
					"Closing with an unknown element which wasn't opened.");
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
	else
	{
		/* Fieldml top level */
		if (!strcmp(name, "fieldml"))
		{
			fieldml_end_fieldml(fieldml_data);
		}
		else
		{
			display_message(ERROR_MESSAGE, "fieldml_end_xml_element.  "
				"Closing with an unknown element which wasn't opened.");
		}
	}
	LEAVE;
} /* fieldml_end_xml_element */

void general_start_xml_element(void *user_data, const xmlChar *name,
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

void general_end_xml_element(void *user_data, const xmlChar *name)
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

void fieldml_sax_characters(struct Fieldml_sax_data *fieldml_data,
	char *characters, int length)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *buffer;
	int allocate_blocksize = 1000, new_allocate;

	ENTER(fieldml_sax_characters);

	if (fieldml_data->current_node)
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
					fieldml_data->buffer_allocated_length + new_allocate))
				{
					fieldml_data->character_buffer = buffer;
					fieldml_data->buffer_allocated_length += new_allocate;
				}
			}
			memcpy(fieldml_data->character_buffer + fieldml_data->buffer_length,
				characters, length);
			fieldml_data->buffer_length += length;
		}
	}
	else
	{
		/* Ignore until field values and element values are implemented */
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
	va_list args;

	ENTER(fieldml_sax_error);
	USE_PARAMETER(user_data);
	va_start(args, msg);
	display_message(ERROR_MESSAGE,
		"fieldml_sax_error.  %s", msg, args);
	va_end(args);
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

int specialXmlSAXParseFile(xmlSAXHandlerPtr sax, void *user_data, char *filename)
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

int parse_fieldml_file(char *filename, struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_node) *node_manager, struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static xmlSAXHandler fieldml_handler;
	struct Fieldml_sax_data fieldml_data;

	ENTER(parse_fieldml_file);

	fieldml_data.unknown_depth = 0;
	fieldml_data.return_val = 0;
	fieldml_data.fieldml_version = -1; /* Not in fieldml yet */
	fieldml_data.fieldml_subversion = -1; /* Not in fieldml yet */
	fieldml_data.root_region = CREATE(Cmiss_region)();
	fieldml_data.root_fe_region = CREATE(FE_region)((struct FE_region *)NULL,
		basis_manager, node_manager, element_manager, fe_field_manager);
	Cmiss_region_attach_FE_region(fieldml_data.root_region,
		fieldml_data.root_fe_region);
	fieldml_data.label_templates = CREATE(LIST(Fieldml_labels_template))();
	fieldml_data.label_name_stack = CREATE(LIST(Fieldml_label_name))();
	fieldml_data.character_buffer = (char *)NULL;
	fieldml_data.buffer_length = 0;
	fieldml_data.buffer_allocated_length = 0;

	fieldml_data.current_assign_labels = (struct Fieldml_labels_template *)NULL;
	fieldml_data.current_labels_template = (struct Fieldml_labels_template *)NULL;
	fieldml_data.current_label_name = (struct Fieldml_label_name *)NULL;
	fieldml_data.current_node = (struct FE_node *)NULL;
	fieldml_data.current_field = (struct FE_field *)NULL;
	fieldml_data.current_field_ref = (struct FE_field *)NULL;
	fieldml_data.current_field_component_name = (char *)NULL;
	fieldml_data.current_node_field_creator = (struct FE_node_field_creator *)NULL;

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
	
	if (specialXmlSAXParseFile(&fieldml_handler, &fieldml_data, filename) < 0)
	{
		return_code = 1;
	} 
	else
	{
		return_code = 0;
	}

	/* Regions can be merged */
	/* Merge regions */

	/* Clean up */
	DESTROY(LIST(Fieldml_labels_template))(&fieldml_data.label_templates);
	DESTROY(LIST(Fieldml_label_name))(&fieldml_data.label_name_stack);

	DESTROY(FE_region)(&fieldml_data.root_fe_region);
	DESTROY(Cmiss_region)(&fieldml_data.root_region);

	LEAVE;
	return(return_code);
} /* parse_fieldml_file */
#endif /* defined (HAVE_XML2) */

/*******************************************************************************
FILE : read_fieldml.c

LAST MODIFIED : 10 February 2003

DESCRIPTION :
The function for importing finite element data into CMISS.
==============================================================================*/
#if defined (UNIX)
#include <ctype.h>
#endif /* defined (UNIX) */
#include <math.h>

#include <libxml/parser.h>
#include <libxml/parserInternals.h>

#include "finite_element/finite_element.h"
#include "finite_element/finite_element_time.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/read_fieldml.h"
#include "finite_element/write_fieldml.h" /* SAB For temporary regions stuff */
#include "general/debug.h"
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

struct Fieldml_sax_data
/*******************************************************************************
LAST MODIFIED : 10 February 2003

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

	struct FE_node *current_node;
	struct FE_field *current_field;
	char *current_field_component_name;
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
	struct FE_field *existing_field, *field;

	field = *field_address;
	if (field_name = get_FE_field_name(field))
	{
		if (existing_field=FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,
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

void fieldml_start_fieldml(struct Fieldml_sax_data *fieldml_data,
	const xmlChar **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	char *version_string, *attribute_name, *attribute_value;
	int i, fieldml_number;

	ENTER(fieldml_start_fieldml);

	if (0 > fieldml_data->fieldml_version)
	{
		fieldml_number = 0;
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

void fieldml_start_field(struct Fieldml_sax_data *fieldml_data,
	const xmlChar **attributes)
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
	const xmlChar **attributes)
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
	const xmlChar **attributes)
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
	const xmlChar *name, const xmlChar **attributes)
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
	else if (fieldml_data->current_node)
	{
		/* Fieldml node */
		fieldml_data->unknown_depth = 1;
	}
	else
	{
		/* Fieldml top level */
		switch (*name)
		{
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
			default:
			{
				fieldml_data->unknown_depth = 1;
			} break;
		}
	}
	LEAVE;
} /* fieldml_start_xml_element */

void fieldml_end_xml_element(struct Fieldml_sax_data *fieldml_data,
	const xmlChar *name)
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
	else if (fieldml_data->current_node)
	{
		/* Fieldml node */
		switch (*name)
		{
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
	const xmlChar **attributes)
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
==============================================================================*/
{
	struct Fieldml_sax_data *fieldml_data;

	ENTER(general_start_xml_element);

	if (fieldml_data = (struct Fieldml_sax_data *)user_data)
	{
		if (fieldml_data->unknown_depth > 0)
		{
			fieldml_data->unknown_depth++;
		}
		else
		{
			if (0 <= fieldml_data->fieldml_version)
			{
				fieldml_start_xml_element(fieldml_data, name, attributes);
			}
			else
			{
				switch (*name)
				{
					case 'f':
					{
						if (!strcmp(name, "fieldml"))
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
						if (!strcmp(name, "regionml"))
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
				fieldml_end_xml_element(fieldml_data, name);
			}
			else
			{
				switch (*name)
				{
					case 'r':
					{
						if (!strcmp(name, "regionml"))
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

	fieldml_data.current_node = (struct FE_node *)NULL;
	fieldml_data.current_field = (struct FE_field *)NULL;
	fieldml_data.current_field_component_name = (char *)NULL;

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
	fieldml_handler.characters = (charactersSAXFunc)NULL;
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

	LEAVE;
	return(return_code);
} /* parse_fieldml_file */

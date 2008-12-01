/*******************************************************************************
FILE : write_fieldml.c

LAST MODIFIED : 16 May 2003

DESCRIPTION :
Functions for exporting finite element data to a file.
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
#include <stdio.h>
#include "finite_element/finite_element.h"
#include "finite_element/write_fieldml.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_write_info.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module Defines
--------------
*/

/* the number of spaces each child object is indented from its parent by in the
	 output file */
#define EXPORT_INDENT_SPACES (2)

/*
Module types
------------
*/

struct Write_FE_region_field_data
{
	FILE *output_file;
	int indent;
}; /* struct Write_FE_region_element_data */

struct Basis_mapping
{
	struct FE_element_field_component *field_component;
	char basis_mapping_name[100];
	char element_nodal_values_name[100];
	char scale_factor_list_name[100];
}; /* struct Basis_mapping */

struct Element_template
{
	struct FE_element_field_component *field_component;
	struct FE_element *element; /* Needed so we can check nodal value names */
	struct FE_field *field;
	int component_number;
	char element_template_name[100];
}; /* struct Element_template */

struct Element_interpolation
{
	struct FE_element *element;
	char element_interpolation_name[100];
}; /* struct Element_interpolation */

struct Write_FE_region_element_data
{
	FILE *output_file;
	int indent;
	int dimension, output_number_of_nodes, *output_node_indices,
		output_number_of_scale_factors, *output_scale_factor_indices;
	int number_of_basis_mappings;
	struct Basis_mapping *basis_mappings;
	int number_of_element_templates;
	struct Element_template *element_templates;
	int number_of_element_interpolations;
	struct Element_interpolation *element_interpolations;
	struct FE_field_order_info *field_order_info;
	struct FE_element *last_element;
	struct FE_region *fe_region;
}; /* struct Write_FE_region_element_data */

struct Write_FE_node_field_values
{
	FILE *output_file;
	/* store number of values for writing nodal values in columns */
	int number_of_values;
};

struct Write_FE_node_field_info_sub
{
	/* Initialised to the current file indent number of spaces */
	int indent;
	/* value_index is incremented by write_FE_node_field so
		 single and multiple field output can be handled appropriately. Must be
		 initialised to 1 before the first time write_FE_node_field is called */
	int value_index;
	FILE *output_file;
}; /* Write_FE_node_field_info_sub */

struct Write_FE_element_field_sub
{
	FILE *output_file;
	int indent;
	struct Element_interpolation *element_interpolation;
	int *number_of_basis_mappings;
	struct Basis_mapping **basis_mappings;
	int *number_of_element_templates;
	struct Element_template **element_templates;
	int field_number,output_number_of_nodes,*output_node_indices,
		output_number_of_scale_factors,*output_scale_factor_indices;
}; /* struct Write_FE_element_field_sub */


struct Node_template
{
	struct FE_node *node;
	char template_name[100];
}; /* struct Node_template */

struct Write_FE_region_node_data
{
	FILE *output_file;
	int indent;
	struct FE_field_order_info *field_order_info;
	struct FE_node *last_node;
	int number_of_node_templates;
	struct Node_template *node_templates;
}; /* struct Write_FE_region_node_data */

/*
Module functions
----------------
*/

/*
Module functions
----------------
*/

static int indent_fprintf(FILE *file, int indent, char *format, ... )
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Writes to a file prepending the indent level number of spaces to the front.
==============================================================================*/
{
	int return_code;
	va_list ap;

	ENTER(indent_fprintf);

	va_start(ap,format);
	fprintf(file, "%*s", indent, " ");
	return_code=vfprintf(file, format, ap);
	va_end(ap);

	LEAVE;

	return (return_code);
} /* indent_fprintf */

static int write_element_xi_value(FILE *output_file,struct FE_element *element,
	FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes to <output_file> the element_xi position in the format:
E<lement>/F<ace>/L<ine> ELEMENT_NUMBER DIMENSION xi1 xi2... xiDIMENSION
==============================================================================*/
{
	char element_char;
	int dimension, i, return_code;
	struct CM_element_information cm;

	ENTER(write_element_xi_value);
	if (output_file && element && get_FE_element_identifier(element, &cm)
		&& (dimension = get_FE_element_dimension(element)))
	{
		display_message(ERROR_MESSAGE,
			"write_element_xi_value.  Not implemented yet");
		switch (cm.type)
		{
			case CM_FACE:
			{
				element_char = 'F';
			} break;
			case CM_LINE:
			{
				element_char = 'L';
			} break;
			default:
			{
				element_char = 'E';
			} break;
		}
		fprintf(output_file, " %c %d %d", element_char, cm.number, dimension);
		for (i = 0; i < dimension; i++)
		{
			fprintf(output_file, " %"FE_VALUE_STRING, xi[i]);
		}
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_element_xi_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_element_xi_value */

static int write_FE_region_field(FILE *output_file, int indent, 
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 28 January 2003

DESCRIPTION :
Writes the part of the field header that is common to exnode and exelem files.
Examples:
1) coordinates, coordinate, rectangular cartesian, #Components=3
2) variable, field, indexed, Index_field=bob, #Values=3, real, #Components=1
3) fixed, field, constant, integer, #Components=3
4) an_array, field, real, #Values=10, #Components=1
==============================================================================*/
{
	char *component_name, *name;
	int i, number_of_components,return_code;
	struct Coordinate_system *coordinate_system;

	ENTER(write_FE_field_header);
	if (output_file&&field)
	{
		/* write the field name */
		if (GET_NAME(FE_field)(field,&name))
		{
			indent_fprintf(output_file, indent, "<field name=\"%s\"\n", name);
			DEALLOCATE(name);
		}
		indent_fprintf(output_file, indent + 7, "value_type=\"%s\"",
			Value_type_string(get_FE_field_value_type(field)));
		if (coordinate_system=get_FE_field_coordinate_system(field))
		{
			switch (coordinate_system->type)
			{
				case CYLINDRICAL_POLAR:
				{
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent + 7, "coordinate_system=\"cylindrical polar\"");
				} break;
				case FIBRE:
				{
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent + 7, "coordinate_system=\"fibre\"");
				} break;
				case OBLATE_SPHEROIDAL:
				{
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent + 7, "coordinate_system=\"oblate spheroidal\"\n");
					indent_fprintf(output_file, indent + 7, "focus=\"%"FE_VALUE_STRING"\"",
						coordinate_system->parameters.focus);
				} break;
				case PROLATE_SPHEROIDAL:
				{
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent + 7, "coordinate_system=\"prolate spheroidal\"\n");
					indent_fprintf(output_file, indent + 7, "focus=\"%"FE_VALUE_STRING"\"",
						coordinate_system->parameters.focus);
				} break;
				case RECTANGULAR_CARTESIAN:
				{
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent + 7, "coordinate_system=\"rectangular cartesian\"");
				} break;
				case SPHERICAL_POLAR:
				{
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent + 7, "coordinate_system=\"spherical polar\"");
				} break;
				default:
				{
					/* write nothing */
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_FE_element_field.  Missing field coordinate system");
		}
		fprintf(output_file, ">\n");

		indent += EXPORT_INDENT_SPACES;
		number_of_components=get_FE_field_number_of_components(field);
		for (i = 0 ; i < number_of_components ; i++)
		{
			if (component_name = get_FE_field_component_name(field, i))
			{
				indent_fprintf(output_file, indent, "<component name=\"%s\"/>\n",
					component_name);
				DEALLOCATE(component_name);
			}
		}
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</field>\n\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_field_header.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_field_header */

static int write_FE_field_values(FILE *output_file,struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Writes the field values to <output_file> - if field is of type constant or
indexed. Each component or version starts on a new line.
==============================================================================*/
{
	enum Value_type value_type;
	int k,number_of_values,return_code;

	ENTER(write_FE_field_values);
	if (output_file&&field)
	{
		return_code=1;
		number_of_values=get_FE_field_number_of_values(field);
		/* only output values for fields with them; ie. not for GENERAL_FE_FIELD */
		if (0<number_of_values)
		{
			display_message(ERROR_MESSAGE,
				"write_FE_field_values.  Not implemented yet");
			value_type=get_FE_field_value_type(field);
			switch (value_type)
			{
				case ELEMENT_XI_VALUE:
				{
					FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
					struct FE_element *element;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_element_xi_value(field,k,&element,xi))
						{
							write_element_xi_value(output_file,element,xi);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_field_values.  Error getting element_xi value");
						}
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value value;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_FE_value_value(field,k,&value))
						{
							fprintf(output_file," %"FE_VALUE_STRING,value);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_field_values.  Error getting FE_value");
						}
					}
				} break;
				case INT_VALUE:
				{
					int value;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_int_value(field,k,&value))
						{
							fprintf(output_file," %d",value);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_field_values.  Error getting int");
						}
					}
				} break;
				case STRING_VALUE:
				{
					char *the_string;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_string_value(field,k,&the_string))
						{
							if (the_string)
							{
								make_valid_token(&the_string);
								fprintf(output_file," %s",the_string);
								DEALLOCATE(the_string);
							}
							else
							{
								/* empty string */
								fprintf(output_file," \"\"");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_field_values.  Could not get string");
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_FE_field_values.  Value type %s not supported",
						Value_type_string(value_type));
				} break;
			}
			fprintf(output_file,"\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_field_values */

static int write_FE_region_field_sub(struct FE_field *field,
	void *write_fields_data_void)
/*******************************************************************************
LAST MODIFIED : 30 January 2003

DESCRIPTION :
Iterator function for fields defined at a element. Wrapper for call to
write_FE_region_field.
==============================================================================*/
{
	struct Write_FE_region_field_data *write_fields_data;
	int return_code;

	ENTER(write_FE_region_field_sub);

	if (field && (write_fields_data = (struct Write_FE_region_field_data *)
		write_fields_data_void))
	{
		return_code=write_FE_region_field(write_fields_data->output_file,
			write_fields_data->indent, field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_field_values.  write_FE_region_field_sub");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_region_field_sub */

static int write_FE_element_field_FE_field_values(struct FE_element *element,
	struct FE_field *field,void *output_file_void)
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Iterator function for fields defined at a element. Wrapper for call to
write_FE_field_values.
==============================================================================*/
{
	int return_code;

	ENTER(write_FE_element_field_FE_field_values);
	USE_PARAMETER(element);
	return_code=write_FE_field_values((FILE *)output_file_void,field);
	LEAVE;

	return (return_code);
} /* write_FE_element_field_FE_field_values */

static int write_FE_node_field_FE_field_values(struct FE_node *node,
	struct FE_field *field,void *output_file_void)
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Iterator function for fields defined at a node. Wrapper for call to
write_FE_field_values.
==============================================================================*/
{
	int return_code;

	ENTER(write_FE_node_field_FE_field_values);
	USE_PARAMETER(node);
	return_code=write_FE_field_values((FILE *)output_file_void,field);
	LEAVE;

	return (return_code);
} /* write_FE_node_field_FE_field_values */

static int write_FE_node_field_template(FILE *output_file,int indent,
	struct FE_node *node,struct FE_field *field,int *value_index)
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Writes a node field to an <output_file>. After the field header is written with
write_FE_field_header, value index, derivative and version info are output on
following lines, one per component:
  COMPONENT_NAME.  Value_index=~, #Derivatives=~ (NAMES), #Versions=~
???RC Added value index - which should be initialised to 1 before the first
time this function is called - this function adds on
(#derivatives+1)*(#versions) to it for each component output.
==============================================================================*/
{
	char *component_name, *name;
	enum FE_field_type fe_field_type;
	enum FE_nodal_value_type *nodal_value_types;
	int i,j,k,number_of_components,number_of_derivatives,number_of_versions,
		return_code;

	ENTER(write_FE_node_field);
	return_code=0;
	if (output_file&&node&&field&&value_index)
	{
		if (GET_NAME(FE_field)(field,&name))
		{
			indent_fprintf(output_file, indent, "<field_ref ref=\"%s\">\n",
				name);
			DEALLOCATE(name);
		}
		indent += EXPORT_INDENT_SPACES;
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;i<number_of_components;i++)
		{
			if (component_name=get_FE_field_component_name(field,i))
			{
				indent_fprintf(output_file, indent, "<component_ref ref=\"%s\">\n",
					component_name);
				DEALLOCATE(component_name);
			}
			indent += EXPORT_INDENT_SPACES;
			fe_field_type=get_FE_field_FE_field_type(field);
			if (GENERAL_FE_FIELD==fe_field_type)
			{
				number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
				number_of_versions=
				get_FE_node_field_component_number_of_versions(node,field,i);
				if (nodal_value_types=
					get_FE_node_field_component_nodal_value_types(node,field,i))
				{
					for (j = 0 ; j < number_of_versions ; j++)
					{
						if (number_of_versions > 1)
						{
							indent_fprintf(output_file, indent, "<label name=\"version_%d\">\n",
								j + 1);
							indent += EXPORT_INDENT_SPACES;
						}
						indent_fprintf(output_file, indent, "<label name=\"value\"/>\n");
						for (k=1;k<1+number_of_derivatives;k++)
						{
							switch (nodal_value_types[k])
							{
								case FE_NODAL_UNKNOWN:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"unknown\"/>\n");
								} break;
								case FE_NODAL_D_DS1:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d/ds1\"/>\n");
								} break;
								case FE_NODAL_D_DS2:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d/ds2\"/>\n");
								} break;
								case FE_NODAL_D_DS3:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d/ds3\"/>\n");
								} break;
								case FE_NODAL_D2_DS1DS2:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d2/ds1ds2\"/>\n");
								} break;
								case FE_NODAL_D2_DS1DS3:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d2/ds1ds3\"/>\n");
								} break;
								case FE_NODAL_D2_DS2DS3:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d2/ds2ds3\"/>\n");
								} break;
								case FE_NODAL_D3_DS1DS2DS3:
								{
									indent_fprintf(output_file, indent,
										"<label name=\"d3/ds1ds2ds3\"/>\n");
								} break;
							}
						}
						if (number_of_versions > 1)
						{
							indent -= EXPORT_INDENT_SPACES;
							indent_fprintf(output_file, indent, "</label>\n");
						}
					}
					DEALLOCATE(nodal_value_types);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_FE_node_field.  Could not get nodal value types");
				}
				(*value_index) += number_of_versions*(1+number_of_derivatives);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_FE_node_field.  Not implemented yet");
			}
			indent -= EXPORT_INDENT_SPACES;
			indent_fprintf(output_file, indent, "</component_ref>\n");
		}
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</field_ref>\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_node_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_field */

static int write_FE_node_field_info_sub(struct FE_node *node,
	struct FE_field *field,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 September 1999

DESCRIPTION :
Calls the write_FE_node_field routine for each FE_node_field
==============================================================================*/
{
	int return_code;
	struct Write_FE_node_field_info_sub *write_data;

	ENTER(write_FE_node_field_info_sub);
	if (write_data=(struct Write_FE_node_field_info_sub *)user_data)
	{
		return_code=write_FE_node_field_template(write_data->output_file,
			write_data->indent,node,field,&(write_data->value_index));
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_field_info_sub */

static int write_FE_node_field_values(struct FE_node *node,
	struct FE_field *field,void *values_data_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Writes out the nodal values. Each component or version starts on a new line.
==============================================================================*/
{
	enum FE_field_type fe_field_type;
	enum Value_type value_type;
	FILE *output_file;
	int i,j,k,number_of_components,number_of_derivatives,number_of_values,
		number_of_versions,return_code;
	struct Write_FE_node_field_values *values_data;

	ENTER(write_FE_node_field_values);
	if (node&&field&&(values_data=(struct Write_FE_node_field_values *)
		values_data_void)&&(output_file=values_data->output_file))
	{
		fe_field_type=get_FE_field_FE_field_type(field);
		if (GENERAL_FE_FIELD==fe_field_type)
		{
			number_of_components=get_FE_field_number_of_components(field);
			value_type=get_FE_field_value_type(field);
			switch (value_type)
			{
				case ELEMENT_XI_VALUE:
				{
					FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
					struct FE_element *element;

					/* only allow as many values as components */
					for (i=0;i<number_of_components;i++)
					{
						if (get_FE_nodal_element_xi_value(node,field,/*component_number*/i,
							/*version*/0,FE_NODAL_VALUE,&element,xi))
						{
							write_element_xi_value(output_file,element,xi);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_node_field_values.  Could not get element_xi value");
						}
						fprintf(output_file,"\n");
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value *value,*values;

					if (get_FE_nodal_field_FE_value_values(field,node,&number_of_values,
						&values))
					{
						value=values;
						for (i=0;i<number_of_components;i++)
						{
							number_of_derivatives=
								get_FE_node_field_component_number_of_derivatives(node,field,i);
							number_of_versions=
								get_FE_node_field_component_number_of_versions(node,field,i);
							for (j=number_of_versions;0<j;j--)
							{
								for (k=0;k<=number_of_derivatives;k++)
								{
									fprintf(output_file," %"FE_VALUE_STRING,*value);
									value++;
								}
								fprintf(output_file,"\n");
							}
						}
						DEALLOCATE(values);
					}
				} break;
				case INT_VALUE:
				{
					int *value,*values;

					if (get_FE_nodal_field_int_values(field,node,&number_of_values,
						&values))
					{
						value=values;
						for (i=0;i<number_of_components;i++)
						{
							number_of_derivatives=
								get_FE_node_field_component_number_of_derivatives(node,field,i);
							number_of_versions=
								get_FE_node_field_component_number_of_versions(node,field,i);
							for (j=number_of_versions;0<j;j--)
							{
								for (k=0;k<=number_of_derivatives;k++)
								{
									fprintf(output_file," %d",*value);
									value++;
								}
								fprintf(output_file,"\n");
							}
						}
						DEALLOCATE(values);
					}
				} break;
				case STRING_VALUE:
				{
					char *the_string;

					/* only allow as many values as components */
					for (i=0;i<number_of_components;i++)
					{
						if (get_FE_nodal_string_value(node,field,/*component_number*/i,
							/*version*/0,FE_NODAL_VALUE,&the_string))
						{
							if (the_string)
							{
								make_valid_token(&the_string);
								fprintf(output_file," %s",the_string);
								DEALLOCATE(the_string);
							}
							else
							{
								/* empty string */
								fprintf(output_file," \"\"");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_node_field_values.  Could not get string");
						}
						fprintf(output_file,"\n");
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_FE_node_field_values.  Value type %s not supported",
						Value_type_string(value_type));
				} break;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_node_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_field_values */

static int write_FE_node(FILE *output_file, int indent, struct FE_node *node,
	char *template_name, struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Writes out a node to an <output_file>. Unless <field_order_info> is non-NULL and
contains the fields to be written if defined, all fields defined at the node are
written.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;
	struct Write_FE_node_field_values values_data;

	ENTER(write_FE_node);
	if (output_file && node)
	{
		indent_fprintf(output_file, indent, "<node name=\"%d\">\n", 
			get_FE_node_identifier(node));
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<assign_labels template_name=\"%s\">\n",
			template_name);
		values_data.output_file = output_file;
		values_data.number_of_values = 0;
		if (field_order_info)
		{
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (i = 0; i < number_of_fields; i++)
			{
				if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
					FE_field_is_defined_at_node(field, node))
				{
					for_FE_field_at_node(field, write_FE_node_field_values,
						(void *)&values_data, node);
				}
			}
		}
		else
		{
			for_each_FE_field_at_node_indexer_first(write_FE_node_field_values,
				(void *)&values_data,node);
		}
		/* add extra carriage return for not multiple of
			 FE_VALUE_MAX_OUTPUT_COLUMNS values */
		if ((0 < values_data.number_of_values) &&
			((0 >= FE_VALUE_MAX_OUTPUT_COLUMNS) ||
				(0 != (values_data.number_of_values % FE_VALUE_MAX_OUTPUT_COLUMNS))))
		{
			fprintf(output_file, "\n");
		}
		indent_fprintf(output_file, indent, "</assign_labels>\n");
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</node>\n\n", 
			get_FE_node_identifier(node));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_FE_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node */

static int FE_nodes_have_same_template(struct FE_node *node_1,
	struct FE_node *node_2, struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 30 January 2003

DESCRIPTION :
Returns true if <node_1> and <node_2> can be written to file with the
same fields header. If <field_order_info> is supplied only those fields
listed in it are compared.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_nodes_have_same_template);
	if (node_1 && node_2)
	{
		if (field_order_info)
		{
			return_code = 1;
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (i = 0; (i < number_of_fields) && return_code; i++)
			{
				if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
					equivalent_FE_field_at_nodes(field, node_1, node_2)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = equivalent_FE_fields_at_nodes(node_1, node_2);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodes_have_same_template.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_nodes_have_same_template */

static int write_FE_region_node(struct FE_node *node, void *user_data)
/*******************************************************************************
LAST MODIFIED : 30 January 2003

DESCRIPTION :
Writes a node to the given file.  If the fields defined at the node are
different from the last node (taking into account whether a selection of fields
has been selected for output) then the header is written out.
==============================================================================*/
{
	char *template_name;
	FILE *output_file;
	int i, indent, number_of_fields, number_of_fields_in_header, return_code,
		write_field_values;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct Node_template *templates;
	struct Write_FE_region_node_data *write_data;
	struct Write_FE_node_field_info_sub field_data;

	ENTER(write_FE_region_node);
	if (node && (write_data = (struct Write_FE_region_node_data *)user_data) &&
		(output_file = write_data->output_file))
	{
		return_code = 1;
		indent = write_data->indent;
		field_order_info = write_data->field_order_info;
		template_name = (char *)NULL;
		for (i = 0 ; !template_name && (i < write_data->number_of_node_templates) ; i++)
		{
			if (FE_nodes_have_same_template(write_data->node_templates[i].node, 
				node, field_order_info))
			{
				template_name = write_data->node_templates[i].template_name;
			}
		}
		if (!template_name)
		{
			/* Generate a new name */
			if (REALLOCATE(templates, write_data->node_templates, struct Node_template,
				write_data->number_of_node_templates + 1))
			{
				write_data->node_templates = templates;
				templates[write_data->number_of_node_templates].node = node;
				sprintf(templates[write_data->number_of_node_templates].template_name,
					"NodeTemplate%d", write_data->number_of_node_templates + 1);
				template_name = 
					templates[write_data->number_of_node_templates].template_name;
				write_data->number_of_node_templates++;

				/* get number of fields in header */
				if (field_order_info)
				{
					number_of_fields_in_header = 0;
					write_field_values = 0;
					number_of_fields = get_FE_field_order_info_number_of_fields(
						field_order_info);
					for (i = 0; i < number_of_fields; i++)
					{
						if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
							FE_field_is_defined_at_node(field, node))
						{
							number_of_fields_in_header++;
							if (0 < get_FE_field_number_of_values(field))
							{
								write_field_values = 1;
							}
						}
					}
				}
				else
				{
					number_of_fields_in_header = get_FE_node_number_of_fields(node);
					write_field_values = FE_node_has_FE_field_values(node);
				}
				indent_fprintf(output_file, indent, "<labels_template name=\"%s\">\n",
					template_name);
				indent += EXPORT_INDENT_SPACES;
				field_data.indent = indent;
				field_data.value_index = 1;
				field_data.output_file = output_file;
				if (field_order_info)
				{
					for (i = 0; i < number_of_fields; i++)
					{
						if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
							FE_field_is_defined_at_node(field, node))
						{
							for_FE_field_at_node(field, write_FE_node_field_info_sub,
								&field_data, node);
						}
					}
				}
				else
				{
					for_each_FE_field_at_node_indexer_first(write_FE_node_field_info_sub,
						&field_data, node);
				}
				indent -= EXPORT_INDENT_SPACES;
				indent_fprintf(output_file, indent, "</labels_template>\n\n");
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_FE_region_node.  Unable to allocate memory for template names.");
				return_code = 0;
			}
		}
		if (write_field_values)
		{
			/* write values for constant and indexed fields */
			fprintf(output_file, " Values :\n");
			if (field_order_info)
			{
				for (i = 0; i < number_of_fields; i++)
				{
					if ((field = get_FE_field_order_info_field(field_order_info, i))
						&& FE_field_is_defined_at_node(field, node) &&
						(0 < get_FE_field_number_of_values(field)))
					{
						for_FE_field_at_node(field, write_FE_node_field_FE_field_values,
							(void *)output_file, node);
					}
				}
			}
			else
			{
				for_each_FE_field_at_node_indexer_first(
					write_FE_node_field_FE_field_values, (void *)output_file, node);
			}
		}
		write_FE_node(output_file, indent, node, template_name, field_order_info);
		write_data->last_node = node;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_region_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_region_node */

static int write_FE_element_identifier(FILE *output_file,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes out the <element> identifier to <output_file> as the triplet:
ELEMENT_NUMBER FACE_NUMBER LINE_NUMBER, with only one number non-zero. The
output contains no characters before or after the printed numbers.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm;

	ENTER(write_FE_element_identifier);
	if (output_file && element && get_FE_element_identifier(element, &cm))
	{
		/* file output */
		return_code = 1;
		switch (cm.type)
		{
			case CM_ELEMENT:
			{
				fprintf(output_file, "%d", cm.number);
			} break;
			case CM_FACE:
			{
				fprintf(output_file, "%d", cm.number+200000);
			} break;
			case CM_LINE:
			{
				fprintf(output_file, "%d", cm.number+400000);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"write_FE_element_identifier.  Unknown CM_element_type");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_identifier.  Invalid element");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_identifier */

static int write_FE_element_shape(FILE *output_file,
	struct FE_element_shape *element_shape)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Writes out the <element_shape> to <output_file>.
???RC Currently limited to handling one polygon or one simplex. Will have to
be rewritten for 4-D and above elements.
==============================================================================*/
{
	enum FE_element_shape_type shape_type;
	int dimension, linked_dimensions, next_xi_number, number_of_polygon_vertices,
		return_code, xi_number;

	ENTER(write_FE_element_shape);
	if (output_file && element_shape &&
		get_FE_element_shape_dimension(element_shape, &dimension))
	{
		return_code = 1;
		linked_dimensions = 0;
		for (xi_number = 0; (xi_number < dimension) && return_code; xi_number++)
		{
			if (get_FE_element_shape_xi_shape_type(element_shape, xi_number,
				&shape_type))
			{
				switch (shape_type)
				{
					case LINE_SHAPE:
					{
						fprintf(output_file, "line");
					} break;
					case POLYGON_SHAPE:
					{
						/* logic currently limited to one polygon in shape - ok up to 3D */
						fprintf(output_file, "polygon");
						if (0 == linked_dimensions)
						{
							if (get_FE_element_shape_next_linked_xi_number(element_shape,
								xi_number, &next_xi_number, &number_of_polygon_vertices) &&
								(0 < next_xi_number))
							{
								if (number_of_polygon_vertices >= 3)
								{
									fprintf(output_file, "(%d;%d)", number_of_polygon_vertices,
										next_xi_number + 1);
								}
								else
								{
									display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
										"Invalid number of vertices in polygon: %d",
										number_of_polygon_vertices);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
									"No second linked dimensions in polygon");
								return_code = 0;
							}
						}
						linked_dimensions++;
						if (2 < linked_dimensions)
						{
							display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
								"Too many linked dimensions in polygon");
							return_code = 0;
						}
					} break;
					case SIMPLEX_SHAPE:
					{
						/* logic currently limited to one simplex in shape - ok up to 3D */
						fprintf(output_file, "simplex");
						if (0 == linked_dimensions)
						{
							linked_dimensions++;
							/* for first linked simplex dimension write (N1[;N2]) where N1 is
								 first linked dimension, N2 is the second - for tetrahedra */
							fprintf(output_file,"(");
							next_xi_number = xi_number;
							while (return_code && (next_xi_number < dimension))
							{
								if (get_FE_element_shape_next_linked_xi_number(element_shape,
									next_xi_number, &next_xi_number, &number_of_polygon_vertices))
								{
									if (0 < next_xi_number)
									{
										linked_dimensions++;
										if (2 < linked_dimensions)
										{
											fprintf(output_file, ";");
										}
										fprintf(output_file, "%d", next_xi_number + 1);
									}
									else
									{
										next_xi_number = dimension;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
										"Could not get next linked xi number for simplex");
									return_code = 0;
								}
							}
							fprintf(output_file,")");
							if (1 == linked_dimensions)
							{
								display_message(ERROR_MESSAGE,"write_FE_element_shape.  "
									"Too few linked dimensions in simplex");
								return_code = 0;
							}
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"write_FE_element_shape.  Unknown shape type");
						return_code = 0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_FE_element_shape.  Could not get shape type");
				return_code = 0;
			}
			if (xi_number < (dimension - 1))
			{
				fprintf(output_file,"*");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_shape.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_shape */

static int write_FE_basis_mapping(FILE *output_file, int indent, 
	struct Basis_mapping *basis_mapping, 
	struct FE_element_field_component *component)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Writes out the <basis> to <output_file>.
==============================================================================*/
{
	char *basis_type_string;
	enum FE_basis_type basis_type;
	FE_element_field_component_modify modify;
	int dimension, linked_dimensions, next_xi_number, number_of_polygon_vertices,
		return_code, xi_number;
	struct FE_basis *basis;

	ENTER(write_FE_basis);
	if (output_file && component)
	{
		return_code = 1;
		FE_element_field_component_get_basis(component, &basis);
		FE_element_field_component_get_modify(component, &modify);
		FE_basis_get_dimension(basis, &dimension);
		linked_dimensions = 0;
		indent_fprintf(output_file, indent, "<mapping name=\"%s\"\n",
			basis_mapping->basis_mapping_name);
		indent_fprintf(output_file, indent + 9, "basis=\"");
		for (xi_number = 0; (xi_number < dimension) && return_code; xi_number++)
		{
			if (FE_basis_get_xi_basis_type(basis, xi_number, &basis_type))
			{
				if (basis_type_string = FE_basis_type_string(basis_type))
				{
					fprintf(output_file, "%s", basis_type_string);
					switch (basis_type)
					{
						case CUBIC_HERMITE:
						case CUBIC_LAGRANGE:
						case HERMITE_LAGRANGE:
						case LAGRANGE_HERMITE:
						case LINEAR_LAGRANGE:
						case QUADRATIC_LAGRANGE:
						{
							/* no linking between dimensions */
						} break;
						case POLYGON:
						{
							/* write (number_of_polygon_vertices;linked_xi) */
							/* logic currently limited to one polygon in basis - ok to 3D */
							if (0 == linked_dimensions)
							{
								if (FE_basis_get_next_linked_xi_number(basis,
										 xi_number, &next_xi_number, &number_of_polygon_vertices) &&
									(0 < next_xi_number))
								{
									if (number_of_polygon_vertices >= 3)
									{
										fprintf(output_file, "(%d;%d)", number_of_polygon_vertices,
											next_xi_number + 1);
									}
									else
									{
										display_message(ERROR_MESSAGE, "write_FE_basis.  "
											"Invalid number of vertices in polygon: %d",
											number_of_polygon_vertices);
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "write_FE_basis.  "
										"No second linked dimensions in polygon");
									return_code = 0;
								}
							}
							linked_dimensions++;
							if (2 < linked_dimensions)
							{
								display_message(ERROR_MESSAGE, "write_FE_basis.  "
									"Too many linked dimensions in polygon");
								return_code = 0;
							}
						} break;
						case LINEAR_SIMPLEX:
						case QUADRATIC_SIMPLEX:
						case SERENDIPITY:
						{
							/* write (linked_xi[;linked_xi]) */
							/* logic currently limited to one simplex in shape - ok to 3D */
							if (0 == linked_dimensions)
							{
								linked_dimensions++;
								/* for first linked simplex dimension write (N1[;N2]) where N1
									is first linked dimension, N2 is second - for tetrahedra */
								fprintf(output_file,"(");
								next_xi_number = xi_number;
								while (return_code && (next_xi_number < dimension))
								{
									if (FE_basis_get_next_linked_xi_number(basis,
											 next_xi_number, &next_xi_number,
											 &number_of_polygon_vertices))
									{
										if (0 < next_xi_number)
										{
											linked_dimensions++;
											if (2 < linked_dimensions)
											{
												fprintf(output_file, ";");
											}
											fprintf(output_file, "%d", next_xi_number + 1);
										}
										else
										{
											next_xi_number = dimension;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE, "write_FE_basis.  "
											"Could not get next linked xi number for simplex");
										return_code = 0;
									}
								}
								fprintf(output_file,")");
								if (1 == linked_dimensions)
								{
									display_message(ERROR_MESSAGE,"write_FE_basis.  "
										"Too few linked dimensions in simplex");
									return_code = 0;
								}
							}
						} break;
						case BSPLINE:
						case FOURIER:
						case SINGULAR:
						case TRANSITION:
						{
							printf("don't know how to link dimensions for %s\n",
								basis_type_string);
							/* don't know what to do! */
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"write_FE_basis.  Unknown basis type: %s", basis_type_string);
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_FE_basis.  Could not get basis type string");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_FE_basis.  Could not get basis type");
				return_code = 0;
			}
			if (xi_number < (dimension - 1))
			{
				fprintf(output_file,"*");
			}
		}
		FE_element_field_component_get_modify(component, &modify);
		if (!modify)
		{
			fprintf(output_file, "\">\n");
		}
		else
		{
			fprintf(output_file, "\"\n");
			if (modify == theta_increasing_in_xi1)
			{
				indent_fprintf(output_file, indent + 9,
					"modification=\"increasing in xi1\"");
			}
			else if (modify == theta_decreasing_in_xi1)
			{
				indent_fprintf(output_file, indent + 9,
					"modification=\"decreasing in xi1\"");
			}
			else if (modify == theta_non_increasing_in_xi1)
			{
				indent_fprintf(output_file, indent + 9,
					"modification=\"non-increasing in xi1\"");
			}
			else if (modify == theta_non_decreasing_in_xi1)
			{
				indent_fprintf(output_file, indent + 9,
					"modification=\"non-decreasing in xi1\"");
			}
			else
			{
				
				display_message(ERROR_MESSAGE,
					"write_FE_element_field.  Unknown modify function");
			}
			fprintf(output_file, ">\n");
		}
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<coefficients>\n");
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<product>\n");
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<element_lookup>\n");
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<field_lookup>\n");
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<component_lookup>\n");
		indent_fprintf(output_file, indent + EXPORT_INDENT_SPACES,
			"<label_lookup indices=\"%s\"/>\n", basis_mapping->element_nodal_values_name);
		indent_fprintf(output_file, indent, "</component_lookup>\n");
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</field_lookup>\n");
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</element_lookup>\n");
		if (basis_mapping->scale_factor_list_name &&
			*basis_mapping->scale_factor_list_name)
		{
			indent_fprintf(output_file, indent, "<element_lookup>\n");
			indent_fprintf(output_file, indent + EXPORT_INDENT_SPACES,
				"<label_lookup indices=\"%s\"/>\n", basis_mapping->scale_factor_list_name);
			indent_fprintf(output_file, indent, "</element_lookup>\n");
		}
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</product>\n");
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</coefficients>\n");
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</mapping>\n\n");
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_basis.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_basis */

int FE_element_field_components_have_same_mapping(
	struct FE_element_field_component *component_1,
	struct FE_element_field_component *component_2)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns true if <component_1> and <component_2> are represented by the same
basis mapping.
==============================================================================*/
{
	FE_element_field_component_modify modify_1, modify_2;
	int return_code;
	struct FE_basis *basis_1, *basis_2;

	ENTER(FE_element_field_components_have_same_mapping);
	if (component_1 && component_2)
	{
		FE_element_field_component_get_basis(component_1, &basis_1);
		FE_element_field_component_get_modify(component_1, &modify_1);
		FE_element_field_component_get_basis(component_2, &basis_2);
		FE_element_field_component_get_modify(component_2, &modify_2);
		if ((basis_1 == basis_2) && (modify_1 == modify_2))
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
			"FE_element_field_components_have_same_mapping.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_components_have_same_mapping */

int FE_element_field_components_have_same_element_template(
	struct FE_element_field_component *component_1, struct FE_element *element_1,
	struct FE_field *field_1, int component_number_1,
	struct FE_element_field_component *component_2, struct FE_element *element_2,
	struct FE_field *field_2, int component_number_2)
/*******************************************************************************
LAST MODIFIED : 4 September 2003

DESCRIPTION :
Returns true if <component_1> and <component_2> are represented by the same
element template.  <element_1>, <field_1> and <component_number_1> and 
<element_2>,  <field_2> and <component_number_2> are required so we can check the
names for the nodal_value indices.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_types_1, *nodal_value_types_2;
	enum Global_to_element_map_type component_type_1, component_type_2;
	int derivatives_1, derivatives_2, j, k, nodal_value_index_1,
		nodal_value_index_2, node_index_1, node_index_2,
		number_of_nodal_values_1, number_of_nodal_values_2, number_of_nodes_1,
		number_of_nodes_2, return_code, versions_1, versions_2, version_1, version_2;
	struct FE_node *node_1, *node_2;
	struct Standard_node_to_element_map *standard_node_map_1,
		*standard_node_map_2;

	ENTER(FE_element_field_components_have_same_mapping);
	if (component_1 && component_2)
	{
		FE_element_field_component_get_type(component_1, &component_type_1);
		FE_element_field_component_get_type(component_2, &component_type_2);
		if (component_type_1 == component_type_2)
		{
			switch (component_type_1)
			{
				case STANDARD_NODE_TO_ELEMENT_MAP:
				{
					return_code = 1;
					if (FE_element_field_component_get_number_of_nodes(component_1,
						&number_of_nodes_1) &&
						FE_element_field_component_get_number_of_nodes(component_2,
						&number_of_nodes_2))
					{
						if (number_of_nodes_1 == number_of_nodes_2)
						{
							for (j = 0; return_code && (j < number_of_nodes_1) ; j++)
							{
								if (FE_element_field_component_get_standard_node_map(
										 component_1, j, &standard_node_map_1) &&
									FE_element_field_component_get_standard_node_map(
										component_2, j, &standard_node_map_2) &&
									Standard_node_to_element_map_get_node_index(
										standard_node_map_1, &node_index_1) &&
									Standard_node_to_element_map_get_node_index(
										standard_node_map_2, &node_index_2) &&
									Standard_node_to_element_map_get_number_of_nodal_values(
										standard_node_map_1, &number_of_nodal_values_1) && 
									Standard_node_to_element_map_get_number_of_nodal_values(
										standard_node_map_2, &number_of_nodal_values_2))
								{
									if ((node_index_1 == node_index_2) && 
										(number_of_nodal_values_1 == number_of_nodal_values_2))
									{
										get_FE_element_node(element_1, node_index_1, &node_1);
										get_FE_element_node(element_2, node_index_2, &node_2);
										derivatives_1 = get_FE_node_field_component_number_of_derivatives(
											node_1, field_1, component_number_1);
										derivatives_2 = get_FE_node_field_component_number_of_derivatives(
											node_2, field_2, component_number_2);
										versions_1 = get_FE_node_field_component_number_of_versions(
											node_1, field_1, component_number_1);
										versions_2 = get_FE_node_field_component_number_of_versions(
											node_2, field_2, component_number_2);
										if ((derivatives_1 == derivatives_2) && (versions_1 == versions_2))
										{
											nodal_value_types_1 = get_FE_node_field_component_nodal_value_types(
												node_1, field_1, component_number_1);
											nodal_value_types_2 = get_FE_node_field_component_nodal_value_types(
												node_2, field_2, component_number_2);
											for (k = 0; return_code && (k < number_of_nodal_values_1) ; k++)
											{
												Standard_node_to_element_map_get_nodal_value_index(
													standard_node_map_1, k, &nodal_value_index_1);
												Standard_node_to_element_map_get_nodal_value_index(
													standard_node_map_2, k, &nodal_value_index_2);
												version_1 = nodal_value_index_1 / (derivatives_1 + 1);
												version_2 = nodal_value_index_2 / (derivatives_2 + 1);
												nodal_value_index_1 = nodal_value_index_1 % (derivatives_1 + 1);
												nodal_value_index_2 = nodal_value_index_2 % (derivatives_2 + 1);
												if ((version_1 != version_2) ||
													(nodal_value_types_1[nodal_value_index_1] !=
													nodal_value_types_2[nodal_value_index_2]))
												{
													return_code = 0;
												}
											}
											if (nodal_value_types_1)
											{
												DEALLOCATE(nodal_value_types_1);
											}
											if (nodal_value_types_2)
											{
												DEALLOCATE(nodal_value_types_2);
											}
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
						"FE_element_field_components_have_same_element_template.  "
						"Unable to get number of nodes from element field components.");
						return_code = 0;
					}
				} break;
				case GENERAL_NODE_TO_ELEMENT_MAP:
				{
					return_code = 0;
				} break;
				case FIELD_TO_ELEMENT_MAP:
				{
					return_code = 0;
				} break;
				case ELEMENT_GRID_MAP:
				{
					return_code = 0;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_components_have_same_element_template.  "
						"Unknown field component type");
					return_code = 0;
				} break;
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
			"FE_element_field_components_have_same_mapping.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_components_have_same_mapping */

int write_FE_element_template(FILE *output_file, int indent,
	struct Element_template *element_template,
	struct FE_element *element, struct FE_field *field, int field_component_number, 
	struct FE_element_field_component *component)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Writes out the element nodal template represented in <component> to <output_file>.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_types;
	enum Global_to_element_map_type component_type;
	int j, k, nodal_value_index, node_index, number_of_derivatives,
		number_of_nodal_values, number_of_nodes, number_of_versions, return_code,
		version;
	struct FE_node *node;
	struct Standard_node_to_element_map *standard_node_map;

	ENTER(write_FE_element_template);
	if (output_file && indent && component)
	{
		indent_fprintf(output_file, indent, "<labels_template name=\"%s\">\n",
			element_template->element_template_name);
		indent += EXPORT_INDENT_SPACES;
		FE_element_field_component_get_type(component, &component_type);
		switch (component_type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				return_code = 1;
				if (FE_element_field_component_get_number_of_nodes(component,
					&number_of_nodes))
				{
					for (j = 0; return_code && (j < number_of_nodes) ; j++)
					{
						if (FE_element_field_component_get_standard_node_map(
							component, j, &standard_node_map) &&
							Standard_node_to_element_map_get_node_index(
							standard_node_map, &node_index) &&
							Standard_node_to_element_map_get_number_of_nodal_values(
							standard_node_map, &number_of_nodal_values))
						{
							get_FE_element_node(element, node_index, &node);
							number_of_derivatives=
								get_FE_node_field_component_number_of_derivatives(node,
									field, field_component_number);
							number_of_versions=
								get_FE_node_field_component_number_of_versions(node,
									field, field_component_number);
							nodal_value_types = get_FE_node_field_component_nodal_value_types
								(node, field, field_component_number);

							for (k = 0; return_code && (k < number_of_nodal_values) ; k++)
							{
								Standard_node_to_element_map_get_nodal_value_index(
									standard_node_map, k, &nodal_value_index);

								indent_fprintf(output_file, indent, "<node_lookup>\n");
								indent += EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "<node_index>\n");
								indent += EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "<element_lookup>\n");
								indent += EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "<label_lookup indices=\"ElementNodeList\">\n");
								indent_fprintf(output_file, indent + EXPORT_INDENT_SPACES,

									"<label_lookup indices=\"%d\"/>\n", node_index + 1);
								indent_fprintf(output_file, indent, "</label_lookup>\n");
								indent -= EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "</element_lookup>\n");
								indent -= EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "</node_index>\n");
								indent_fprintf(output_file, indent, "<field_lookup>\n");
								indent += EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "<component_lookup>\n");
								indent += EXPORT_INDENT_SPACES;
								if (number_of_versions > 1)
								{
									version = nodal_value_index / (number_of_derivatives + 1);
									indent_fprintf(output_file, indent,
										"<label_lookup indices=\"version_%d\">\n", version + 1);
									indent += EXPORT_INDENT_SPACES;
									nodal_value_index -= (number_of_derivatives + 1) *
										version;
								}
								if (nodal_value_types)
								{
									switch (nodal_value_types[nodal_value_index])
									{
										case FE_NODAL_VALUE:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"value\"/>\n");
										} break;
										case FE_NODAL_D_DS1:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d/ds1\"/>\n");
										} break;
										case FE_NODAL_D_DS2:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d/ds2\"/>\n");
										} break;
										case FE_NODAL_D_DS3:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d/ds3\"/>\n");
										} break;
										case FE_NODAL_D2_DS1DS2:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d2/ds1ds2\"/>\n");
										} break;
										case FE_NODAL_D2_DS1DS3:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d2/ds1ds3\"/>\n");
										} break;
										case FE_NODAL_D2_DS2DS3:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d2/ds2ds3\"/>\n");
										} break;
										case FE_NODAL_D3_DS1DS2DS3:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"d3/ds1ds2ds3\"/>\n");
										} break;
										case FE_NODAL_UNKNOWN:
										default:
										{
											indent_fprintf(output_file, indent,
												"<label_lookup indices=\"%d\"/>\n", nodal_value_index + 1);
										} break;
									}
								}
								else
								{
									indent_fprintf(output_file, indent,
										"<label_lookup indices=\"%d\"/>\n", nodal_value_index + 1);
								}
								if (number_of_versions > 1)
								{
									indent -= EXPORT_INDENT_SPACES;
									indent_fprintf(output_file, indent, "</label_lookup>\n");
								}
								indent -= EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "</component_lookup>\n");
								indent -= EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "</field_lookup>\n");
								indent -= EXPORT_INDENT_SPACES;
								indent_fprintf(output_file, indent, "</node_lookup>\n");
							}
						}
						if (nodal_value_types)
						{
							DEALLOCATE(nodal_value_types);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_FE_element_template.  "
						"Unable to get number of nodes from element field components.");
					return_code = 0;
				}
			} break;
			case GENERAL_NODE_TO_ELEMENT_MAP:
			{
				return_code = 0;
			} break;
			case FIELD_TO_ELEMENT_MAP:
			{
				return_code = 0;
			} break;
			case ELEMENT_GRID_MAP:
			{
				return_code = 0;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"write_FE_element_template.  "
					"Unknown field component type");
				return_code = 0;
			} break;
		}
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</labels_template>\n\n");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_template.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_template */

static int write_FE_element_basis_and_templates_sub(struct FE_element *element,
	struct FE_field *field,void *write_element_field_data_void)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Writes information describing how <field> is defined at <element>.
==============================================================================*/
{
	enum FE_field_type fe_field_type;
	enum Global_to_element_map_type component_type;
	FE_element_field_component_modify modify;
	FILE *output_file;
	int i, j, number_of_components, number_of_scale_factor_sets, return_code,
		scale_factor_set_used;
	struct Basis_mapping *basis_mapping, *basis_mappings;
	struct Element_template *element_template, *element_templates;
	struct FE_basis *basis;
	struct FE_element_field_component *component;
	struct Write_FE_element_field_sub *data;
	void *scale_factor_set_identifier;

	ENTER(write_FE_element_basis_and_templates_sub);
	if (element && field && (data =
		(struct Write_FE_element_field_sub *)write_element_field_data_void) &&
		(output_file = data->output_file) &&
		((0 == data->output_number_of_nodes) ||
			data->output_node_indices) &&
		((0 == data->output_number_of_scale_factors) ||
			data->output_scale_factor_indices))
	{
		return_code = 1;
		fe_field_type=get_FE_field_FE_field_type(field);
		number_of_components=get_FE_field_number_of_components(field);
		for (i = 0; i < number_of_components; i++)
		{
			if (GENERAL_FE_FIELD == fe_field_type)
			{
				if (get_FE_element_field_component(element, field, i, &component))
				{
					/* Try and find this basis */
					basis_mapping = (struct Basis_mapping *)NULL;
					for (j = 0 ; (!basis_mapping) && (j < *(data->number_of_basis_mappings)) ; j++)
					{
						if (FE_element_field_components_have_same_mapping(component,
								 (*data->basis_mappings)[j].field_component))
						{
							basis_mapping = (*data->basis_mappings) + j;
						}
					}
					if (!basis_mapping)
					{
						if (REALLOCATE(basis_mappings, *data->basis_mappings,
							struct Basis_mapping, *data->number_of_basis_mappings + 1))
						{
							*data->basis_mappings = basis_mappings;
							basis_mappings[*(data->number_of_basis_mappings)].field_component =
								component;
							sprintf(basis_mappings[*(data->number_of_basis_mappings)].basis_mapping_name,
								"BasisMapping%d", *(data->number_of_basis_mappings) + 1);
							sprintf(basis_mappings[*(data->number_of_basis_mappings)].
								element_nodal_values_name,
								"ElementNodalValues%d", *(data->number_of_basis_mappings) + 1);
							/* Find which scale factor set to use */
							if (get_FE_element_number_of_scale_factor_sets(element,
								&number_of_scale_factor_sets) &&
								FE_element_field_component_get_basis(component, &basis))
							{
								scale_factor_set_used = 0;
								for (j = 0; !scale_factor_set_used &&
									 (j < number_of_scale_factor_sets) ; j++)
								{
									if (get_FE_element_scale_factor_set_identifier(
										element, j, &scale_factor_set_identifier))
									{
										if ((struct FE_basis *)scale_factor_set_identifier
											== basis)
										{
											scale_factor_set_used = j + 1;
										}
									}
								}
								if (scale_factor_set_used)
								{
									sprintf(basis_mappings[*(data->number_of_basis_mappings)].
										scale_factor_list_name,
										"ScaleFactorList%d", scale_factor_set_used);
								}
								else
								{
									basis_mappings[*(data->number_of_basis_mappings)].
										scale_factor_list_name[0] = 0;
								}
							}
							else
							{
								basis_mappings[*(data->number_of_basis_mappings)].
									scale_factor_list_name[0] = 0;
							}
							basis_mapping = basis_mappings + *(data->number_of_basis_mappings);
							(*data->number_of_basis_mappings)++;

							write_FE_basis_mapping(output_file, data->indent,
								basis_mapping, component);	
						}
					}
					
					/* Try and find this element node lookup template */
					element_template = (struct Element_template *)NULL;
					for (j = 0 ; (!element_template) && (j < *(data->number_of_element_templates)) ; j++)
					{
						if (FE_element_field_components_have_same_element_template(
							component, element, field, i,
							(*data->element_templates)[j].field_component,
							(*data->element_templates)[j].element,
							(*data->element_templates)[j].field,
							(*data->element_templates)[j].component_number))
						{
							element_template = (*data->element_templates) + j;
						}
					}
					if (!element_template)
					{
						if (REALLOCATE(element_templates, *data->element_templates,
							struct Element_template, *data->number_of_element_templates + 1))
						{
							*data->element_templates = element_templates;
							element_templates[*(data->number_of_element_templates)].field_component =
								component;
							element_templates[*(data->number_of_element_templates)].element =
								element;
							element_templates[*(data->number_of_element_templates)].field =
								field;
							element_templates[*(data->number_of_element_templates)].component_number =
								i;
							sprintf(element_templates[*(data->number_of_element_templates)].element_template_name,
								"ElementTemplate%d", *(data->number_of_element_templates) + 1);
							element_template = 
								element_templates + *(data->number_of_element_templates);
							(*data->number_of_element_templates)++;

							FE_element_field_component_get_basis(component, &basis);
							FE_element_field_component_get_modify(component, &modify);
							FE_element_field_component_get_type(component, &component_type);
							write_FE_element_template(output_file, data->indent,
								element_template, element, field, i, component);	
						}
					}					
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_FE_element_basis_and_templates_sub.  Missing element field component");
				}
			}
			else
			{
				/* constant and indexed fields: no further component information */
				fprintf(output_file,"\n");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_basis_and_templates_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_basis_and_templates_sub */

static int write_FE_element_field_sub(struct FE_element *element,
	struct FE_field *field,void *write_element_field_data_void)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes information describing how <field> is defined at <element>.
==============================================================================*/
{
	char *component_name, *field_name;
	FILE *output_file;
	int i, indent, j, number_of_components, return_code;
	struct Basis_mapping *basis_mapping;
	struct Element_template *element_template;
	struct FE_element_field_component *component;
	struct Write_FE_element_field_sub *data;

	ENTER(write_FE_element_field_sub);
	if (element && field && (data =
		(struct Write_FE_element_field_sub *)write_element_field_data_void) &&
		(output_file = data->output_file) &&
		((0 == data->output_number_of_nodes) ||
			data->output_node_indices) &&
		((0 == data->output_number_of_scale_factors) ||
			data->output_scale_factor_indices))
	{
		indent = data->indent;
		return_code = 1;
		data->field_number++;
		if (GET_NAME(FE_field)(field,&field_name))
		{
			indent_fprintf(output_file, indent, "<field_ref ref=\"%s\">\n",
				field_name);
			DEALLOCATE(field_name);
			indent += EXPORT_INDENT_SPACES;

			number_of_components=get_FE_field_number_of_components(field);
			for (i = 0; i < number_of_components; i++)
			{
				if (get_FE_element_field_component(element, field, i, &component))
				{
					/* Find the correct basis mapping and element nodal template */
					basis_mapping = (struct Basis_mapping *)NULL;
					for (j = 0 ; (!basis_mapping) && (j < *(data->number_of_basis_mappings)) ; j++)
					{
						if (FE_element_field_components_have_same_mapping(component,
								 (*data->basis_mappings)[j].field_component))
						{
							basis_mapping = (*data->basis_mappings) + j;
						}
					}			
					element_template = (struct Element_template *)NULL;
					for (j = 0 ; (!element_template) && (j < *(data->number_of_element_templates)) ; j++)
					{
						if (FE_element_field_components_have_same_element_template(
							component, element, field, i,
							(*data->element_templates)[j].field_component,
							(*data->element_templates)[j].element,
							(*data->element_templates)[j].field,
							(*data->element_templates)[j].component_number))
						{
							element_template = (*data->element_templates) + j;
						}
					}
					if (basis_mapping && element_template)
					{
						if (component_name = get_FE_field_component_name(field, i))
						{
							indent_fprintf(output_file, indent, "<component_ref ref=\"%s\">\n",
								component_name);
							DEALLOCATE(component_name);
							indent += EXPORT_INDENT_SPACES;
					
							indent_fprintf(output_file, indent, "<mapping_ref ref=\"%s\"/>\n",
								basis_mapping->basis_mapping_name);
							indent_fprintf(output_file, indent, "<label name=\"%s\">\n",
								basis_mapping->element_nodal_values_name);
							indent_fprintf(output_file, indent + EXPORT_INDENT_SPACES,
								"<labels_template_ref ref=\"%s\"/>\n", 
								element_template->element_template_name);
							indent_fprintf(output_file, indent, "</label>\n");

							indent -= EXPORT_INDENT_SPACES;
							indent_fprintf(output_file, indent, "</component_ref>\n");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_FE_element_field_sub.  "
							"Unable to find a valid basis mapping or element template.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "write_FE_element_field_sub.  "
						"Unable to get component name.");
				}
			}
			indent -= EXPORT_INDENT_SPACES;
			indent_fprintf(output_file, indent, "</field_ref>\n");
		}
		else
		{
			display_message(ERROR_MESSAGE, "write_FE_element_field_sub.  "
				"Unable to get field name.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_field_sub.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_field_sub */

static int write_FE_element_element_interpolation(FILE *output_file,
	int indent,	struct FE_element *element,
	struct Element_interpolation *element_interpolation,
	struct FE_field_order_info *field_order_info,
	int *output_number_of_nodes,int **output_node_indices,
	int *output_number_of_scale_factors,int **output_scale_factor_indices,
	int *number_of_basis_mappings, struct Basis_mapping **basis_mappings,
	int *number_of_element_templates, struct Element_template **element_templates)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Writes the element field information header for <element>. If the
<field_order_info> is supplied the header is restricted to include only
components/fields/bases for it. To handle both this and the all-field case, the
function builds up and returns the following:
- output_number_of_nodes and output_node_indices for reducing the number of
  nodes output if not all nodes in element are used for the field. In the array
  of indices, -1 indicates that the node is not used, otherwise the new index
  into the output node list is recorded.
- output_number_of_scale_factors and output_scale_factor_indices for reducing
  the number of scale_factors output. In the array of indices, -1 indicates
  that the scale_factor is not used, otherwise the new index into the output
  scale_factor list is recorded.
The indices arrays are reallocated here, so should be NULL the first time they
are passed to this function.
==============================================================================*/
{
	enum FE_field_type fe_field_type;
	enum Global_to_element_map_type component_type;
	int field_no, i, j, node_index, number_of_components, 
		number_of_nodes_in_component, number_of_fields,
		number_of_fields_in_header, number_of_nodes, number_of_scale_factor_sets,
		number_of_scale_factors, return_code, *scale_factor_set_in_use,
		*temp_indices, write_field_values;
	struct FE_basis *basis;
	struct FE_element_field_component *component;
	struct FE_field *field;
	struct General_node_to_element_map *general_node_map;
	struct Standard_node_to_element_map *standard_node_map;
	void *scale_factor_set_identifier;
	struct Write_FE_element_field_sub write_element_field_data;

	ENTER(write_FE_element_element_interpolation);
	if (output_file && element && output_number_of_nodes && output_node_indices &&
		output_number_of_scale_factors && output_scale_factor_indices)
	{
		return_code = 1;
		/* output basis mappings */
		write_element_field_data.output_file=output_file;
		write_element_field_data.indent=indent;
		write_element_field_data.field_number=1;
		write_element_field_data.output_number_of_nodes=
			*output_number_of_nodes;
		write_element_field_data.output_node_indices=
			*output_node_indices;
		write_element_field_data.output_number_of_scale_factors=
			*output_number_of_scale_factors;
		write_element_field_data.output_scale_factor_indices=
			*output_scale_factor_indices;
		write_element_field_data.number_of_basis_mappings = 
			number_of_basis_mappings;
		write_element_field_data.basis_mappings = basis_mappings;
		write_element_field_data.number_of_element_templates = 
			number_of_element_templates;
		write_element_field_data.element_templates = element_templates;
			
		if (field_order_info)
		{
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (field_no = 0; field_no < number_of_fields; field_no++)
			{
				if ((field =
						 get_FE_field_order_info_field(field_order_info, field_no)) &&
					FE_field_is_defined_in_element(field, element))
				{
					for_FE_field_at_element(field,
						write_FE_element_basis_and_templates_sub,
						(void *)&write_element_field_data, element);
				}
			}
		}
		else
		{
			for_each_FE_field_at_element_indexer_first(
				write_FE_element_basis_and_templates_sub,
				(void *)&write_element_field_data, element);
		}

		/* reallocate/clear output parameters */
		*output_number_of_nodes = 0;
		if (get_FE_element_number_of_nodes(element, &number_of_nodes) &&
			(0 < number_of_nodes))
		{
			if (REALLOCATE(temp_indices, *output_node_indices, int,
					 number_of_nodes))
			{
				*output_node_indices = temp_indices;
				for (i = 0; i < number_of_nodes; i++)
				{
					temp_indices[i] = -1;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			*output_node_indices = (int *)NULL;
		}
		*output_number_of_scale_factors = 0;
		if (get_FE_element_number_of_scale_factors(element,
				 &number_of_scale_factors) && (0 < number_of_scale_factors))
		{
			if (REALLOCATE(temp_indices, *output_scale_factor_indices, int,
					 number_of_scale_factors))
			{
				*output_scale_factor_indices = temp_indices;
				for (i = 0; i < number_of_scale_factors; i++)
				{
					temp_indices[i] = -1;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			*output_scale_factor_indices = (int *)NULL;
		}
		number_of_fields_in_header = 0;
		write_field_values = 0;
		if (return_code &&
			(return_code = get_FE_element_number_of_scale_factor_sets(element,
				&number_of_scale_factor_sets)))
		{
			scale_factor_set_in_use = (int *)NULL;
			if ((0 == number_of_scale_factor_sets) ||
				ALLOCATE(scale_factor_set_in_use, int, number_of_scale_factor_sets))
			{
				if (field_order_info)
				{
					/* output only scale factor sets used by field(s) */
					for (j = 0; j < number_of_scale_factor_sets; j++)
					{
						scale_factor_set_in_use[j] = 0;
					}
					number_of_fields =
						get_FE_field_order_info_number_of_fields(field_order_info);
					for (field_no = 0; field_no < number_of_fields; field_no++)
					{
						if (field =
							get_FE_field_order_info_field(field_order_info, field_no))
						{
							number_of_fields_in_header++;
							if (0 < get_FE_field_number_of_values(field))
							{
								write_field_values = 1;
							}
							fe_field_type = get_FE_field_FE_field_type(field);
							/* no components and no scale factors for non-general fields */
							if (GENERAL_FE_FIELD == fe_field_type)
							{
								for (i = 0; i < number_of_components; i++)
								{
									if (get_FE_element_field_component(element, field, i,
											 &component) &&
										FE_element_field_component_get_type(component,
											&component_type))
									{
										/* determine which scale_factor_sets are in use. From
											this can determine scale_factor renumbering. Note
											that grid-based comp'ts DO NOT use scale factors */
										if (ELEMENT_GRID_MAP != component_type)
										{
											if (FE_element_field_component_get_basis(component,
													 &basis))
											{
												for (j = 0; j < number_of_scale_factor_sets; j++)
												{
													if (get_FE_element_scale_factor_set_identifier(
															 element, j, &scale_factor_set_identifier))
													{
														if ((struct FE_basis *)scale_factor_set_identifier
															== basis)
														{
															scale_factor_set_in_use[j] = 1;
														}
													}
												}
											}
										}
										switch (component_type)
										{
											case STANDARD_NODE_TO_ELEMENT_MAP:
											{
												/* work out which nodes to output */
												if (FE_element_field_component_get_number_of_nodes(
														 component, &number_of_nodes_in_component))
												{
													for (j = 0; j < number_of_nodes_in_component; j++)
													{
														if (FE_element_field_component_get_standard_node_map(
																 component, j, &standard_node_map) &&
															Standard_node_to_element_map_get_node_index(
																standard_node_map, &node_index))
														{
															if ((0 <= node_index) &&
																(node_index < number_of_nodes))
															{
																(*output_node_indices)[node_index] = 1;
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"write_FE_element_element_interpolation.  "
																	"Invalid node_index for "
																	"standard node to element map");
																return_code=0;
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"write_FE_element_element_interpolation.  "
																"Missing standard node to element map");
															return_code=0;
														}
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,"write_FE_element.  "
														"Invalid standard node to element component");
													return_code=0;
												}
											} break;
											case GENERAL_NODE_TO_ELEMENT_MAP:
											{
												/* work out which nodes to output */
												if (FE_element_field_component_get_number_of_nodes(
														 component, &number_of_nodes_in_component))
												{
													for (j = 0; j < number_of_nodes_in_component; j++)
													{
														if (FE_element_field_component_get_general_node_map(
																 component, j, &general_node_map) &&
															General_node_to_element_map_get_node_index(
																general_node_map, &node_index))
														{
															if ((0 <= node_index) &&
																(node_index < number_of_nodes))
															{
																(*output_node_indices)[node_index] = 1;
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"write_FE_element_element_interpolation.  "
																	"Invalid node_index for "
																	"general node to element map");
																return_code=0;
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"write_FE_element_element_interpolation.  "
																"Missing general node to element map");
															return_code=0;
														}
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"write_FE_element_element_interpolation.  "
														"Invalid general node to element component");
													return_code=0;
												}
											} break;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"write_FE_element_element_interpolation.  "
											"Missing element field component");
										return_code=0;
									}
								}
							}
						}
					}
					/* work out correct output_node_indices - currently the nodes in
						use are marked with a 1, otherwise they are -1. Change the 1s
						to the new index to be output */
					*output_number_of_nodes = 0;
					for (i = 0; i < number_of_nodes; i++)
					{
						if (0 < (*output_node_indices)[i])
						{
							(*output_node_indices)[i]= *output_number_of_nodes;
							(*output_number_of_nodes)++;
						}
					}
				}
				else
				{
					number_of_fields_in_header =
						get_FE_element_number_of_fields(element);
					write_field_values = FE_element_has_FE_field_values(element);
					/* output all scale factor sets */
					for (j = 0; j < number_of_scale_factor_sets; j++)
					{
						scale_factor_set_in_use[j] = 1;
					}
					/* output all nodes */
					*output_number_of_nodes = number_of_nodes;
					for (i = 0; i < number_of_nodes; i++)
					{
						(*output_node_indices)[i] = i;
					}
				}

#if defined (OLD_CODE)
				/* work out the number of scale factor sets to output */
				output_number_of_scale_factor_sets = 0;
				for (j = 0; j < number_of_scale_factor_sets; j++)
				{
					if (scale_factor_set_in_use[j])
					{
						output_number_of_scale_factor_sets++;
					}
				}
				/* output scale factor sets and work out scale_factor renumbering
					information */
				fprintf(output_file, " #Scale factor sets=%d\n",
					output_number_of_scale_factor_sets);
				*output_number_of_scale_factors = 0;
				output_scale_factor_index=scale_factor_index = 0;
				for (j = 0; j < number_of_scale_factor_sets; j++)
				{
					if (scale_factor_set_in_use[j])
					{
						fprintf(output_file," ");
						get_FE_element_scale_factor_set_identifier(element, j,
							&scale_factor_set_identifier);
						get_FE_element_numbers_in_scale_factor_set(element, j,
							&numbers_in_scale_factor_set);
						fprintf(output_file,", #Scale factors=%d\n",
							numbers_in_scale_factor_set);
						/* set output scale factor indices */
						for (i = numbers_in_scale_factor_set; 0 < i; i--)
						{
							(*output_scale_factor_indices)[scale_factor_index]=
								output_scale_factor_index;
							scale_factor_index++;
							output_scale_factor_index++;
						}
						*output_number_of_scale_factors += numbers_in_scale_factor_set;
					}
					else
					{
						scale_factor_index += numbers_in_scale_factor_set;
					}
				}
#endif /* defined (OLD_CODE) */

				/* Write the element_interpolation */
				indent_fprintf(output_file, indent,
					"<element_interpolation name=\"%s\">\n",
					element_interpolation->element_interpolation_name);
				indent += EXPORT_INDENT_SPACES;
				write_element_field_data.output_file=output_file;
				write_element_field_data.indent=indent;
				write_element_field_data.field_number=1;
				write_element_field_data.output_number_of_nodes=
					*output_number_of_nodes;
				write_element_field_data.output_node_indices=
					*output_node_indices;
				write_element_field_data.output_number_of_scale_factors=
					*output_number_of_scale_factors;
				write_element_field_data.output_scale_factor_indices=
					*output_scale_factor_indices;
				write_element_field_data.element_interpolation = 
					element_interpolation;
				write_element_field_data.number_of_basis_mappings = 
					number_of_basis_mappings;
				write_element_field_data.basis_mappings = 
					basis_mappings;
				write_element_field_data.number_of_element_templates = 
					number_of_element_templates;
				write_element_field_data.element_templates = element_templates;
					
				if (field_order_info)
				{
					for (field_no = 0; field_no < number_of_fields; field_no++)
					{
						if ((field =
								 get_FE_field_order_info_field(field_order_info, field_no)) &&
							FE_field_is_defined_in_element(field, element))
						{
							for_FE_field_at_element(field, write_FE_element_field_sub,
								(void *)&write_element_field_data, element);
						}
					}
				}
				else
				{
					for_each_FE_field_at_element_indexer_first(
						write_FE_element_field_sub,(void *)&write_element_field_data,
						element);
				}
				indent -= EXPORT_INDENT_SPACES;
				indent_fprintf(output_file, indent, "</element_interpolation>\n\n");

				if (write_field_values)
				{
					fprintf(output_file," Values :\n");
					if (field_order_info)
					{
						for (field_no = 0; field_no < number_of_fields; field_no++)
						{
							if ((field =
									 get_FE_field_order_info_field(field_order_info, field_no))
								&& FE_field_is_defined_in_element(field, element))
							{
								for_FE_field_at_element(field,
									write_FE_element_field_FE_field_values,
									(void *)output_file, element);
							}
						}
					}
					else
					{
						for_each_FE_field_at_element_indexer_first(
							write_FE_element_field_FE_field_values,
							(void *)output_file,element);
					}
				}
				DEALLOCATE(scale_factor_set_in_use);
			}
			else
			{
				display_message(ERROR_MESSAGE,"write_FE_element_element_interpolation.  "
					"Not enough memory for scale_factor_set_in_use");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_FE_element_element_interpolation.  Not enough memory for indices");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_element_interpolation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_element_interpolation */

static int write_FE_element_field_values(struct FE_element *element,
	struct FE_field *field,void *output_file_void)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes grid-based values stored with the element.
==============================================================================*/
{
	enum FE_field_type fe_field_type;
	enum Global_to_element_map_type component_type;
	enum Value_type value_type;
	FILE *output_file;
	int i, j, number_of_components, number_of_values, return_code;
	int number_of_columns,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_element_field_component *component;

	if (element && field && (output_file = (FILE *)output_file_void))
	{
		return_code = 1;
		fe_field_type = get_FE_field_FE_field_type(field);
		if (GENERAL_FE_FIELD == fe_field_type)
		{
			value_type = get_FE_field_value_type(field);
			number_of_components = get_FE_field_number_of_components(field);
			for (i = 0; (i < number_of_components) && return_code; i++)
			{
				if (get_FE_element_field_component(element, field, i, &component) &&
					FE_element_field_component_get_type(component, &component_type))
				{
					if (ELEMENT_GRID_MAP == component_type)
					{
						if ((0<(number_of_values=
							get_FE_element_field_number_of_grid_values(element,field)))&&
							get_FE_element_field_grid_map_number_in_xi(element,
								field,number_in_xi)&&(0<(number_of_columns=number_in_xi[0]+1)))
						{
							switch (value_type)
							{
								case FE_VALUE_VALUE:
								{
									FE_value *values;

									if (get_FE_element_field_component_grid_FE_value_values(
										element, field, /*component_number*/i, &values))
									{
										/* have new line every number-of-grid-points-in-xi1 */
										for (j=0;j<number_of_values;j++)
										{
											fprintf(output_file," %"FE_VALUE_STRING,values[j]);
											if (0==((j+1)%number_of_columns))
											{
												fprintf(output_file,"\n");
											}
										}
										/* extra newline if not multiple of number_of_columns */
										if (0 != (number_of_values % number_of_columns))
										{
											fprintf(output_file,"\n");
										}
										DEALLOCATE(values);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"write_FE_element_field_values.  "
											"Could not get component grid FE_value values");
										return_code=0;
									}
								} break;
								case INT_VALUE:
								{
									int *values;

									if (get_FE_element_field_component_grid_int_values(
										element, field, /*component_number*/i, &values))
									{
										/* have new line every number-of-grid-points-in-xi1 */
										for (j=0;j<number_of_values;j++)
										{
											fprintf(output_file," %d",values[j]);
											if (0==((j+1)%number_of_columns))
											{
												fprintf(output_file,"\n");
											}
										}
										/* extra newline if not multiple of number_of_columns */
										if (0 != (number_of_values % number_of_columns))
										{
											fprintf(output_file,"\n");
										}
										DEALLOCATE(values);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"write_FE_element_field_values.  "
											"Could not get component grid int values");
										return_code=0;
									}
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"write_FE_element_field_values.  Unsupported value type %s",
										Value_type_string(value_type));
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_element_field_values.  Invalid number_of_values");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_FE_element_field_values.  Missing element field component");
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_field_values */

static int write_FE_element(FILE *output_file,
	int indent, struct FE_element *element,
	struct Element_interpolation *element_interpolation,
	struct FE_field_order_info *field_order_info,
	int output_number_of_nodes,int *output_node_indices,
	int output_number_of_scale_factors,int *output_scale_factor_indices)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
==============================================================================*/
{
	FE_value scale_factor;
	int field_no, first_grid_field, i, j, number_in_scale_factor_set,
		number_of_faces, number_of_fields, number_of_nodes, 
		number_of_scale_factor_sets, return_code, scale_factor_index;
	struct FE_element *face;
	struct FE_element_shape *shape;
	struct FE_field *field;
	struct FE_node *node;

	ENTER(write_FE_element);
	if (output_file && element &&
		((0 == output_number_of_nodes) || output_node_indices) &&
		((0 == output_number_of_scale_factors) || output_scale_factor_indices))
	{
		return_code = 1;
		/* write the element identifier */
		indent_fprintf(output_file, indent, "<element name=\"");
		write_FE_element_identifier(output_file, element);
		fprintf(output_file, "\"\n");
		indent_fprintf(output_file, indent + 9, "shape=\"");
		get_FE_element_shape(element, &shape);
		write_FE_element_shape(output_file, shape);
		fprintf(output_file,"\">\n");
		indent += EXPORT_INDENT_SPACES;
		if (get_FE_element_number_of_faces(element, &number_of_faces) &&
			(0 < number_of_faces))
		{
			/* write the faces */
			indent_fprintf(output_file, indent, "<faces>\n");
			for (i = 0; i < number_of_faces; i++)
			{
				if (get_FE_element_face(element, i, &face) && face)
				{
					write_FE_element_identifier(output_file, face);
				}
				else
				{
					fprintf(output_file,"0");
				}
				if ((i == (number_of_faces - 1)) || !((i + 1) % 8))
				{
					fprintf(output_file,"\n");
				}
				else
				{
					fprintf(output_file," ");
				}
			}
			indent_fprintf(output_file, indent, "</faces>\n");
		}
		if (field_order_info)
		{
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (field_no = 0; field_no < number_of_fields; field_no++)
			{
				first_grid_field = 1;
				if ((field =
					get_FE_field_order_info_field(field_order_info, field_no)) &&
					FE_element_field_is_grid_based(element,field))
				{
					if (first_grid_field)
					{
						fprintf(output_file, " Values :\n");
						first_grid_field = 0;
					}
					for_FE_field_at_element(field, write_FE_element_field_values,
						(void *)output_file, element);
				}
			}
		}
		else
		{
			if (FE_element_has_grid_based_fields(element))
			{
				fprintf(output_file," Values :\n");
				for_each_FE_field_at_element_indexer_first(
					write_FE_element_field_values,(void *)output_file,element);
			}
		}
		if (element_interpolation)
		{
			indent_fprintf(output_file, indent, "<element_interpolation_ref ref=\"%s\"/>\n",
				element_interpolation->element_interpolation_name);
			if (0 < output_number_of_nodes)
			{
				/* write the nodes */
				if (get_FE_element_number_of_nodes(element, &number_of_nodes))
				{
					indent_fprintf(output_file, indent, "<label name=\"ElementNodeList\">\n");
					for (i = 0; i < number_of_nodes; i++)
					{
						if (0 <= output_node_indices[i])
						{
							if (get_FE_element_node(element, i, &node))
							{
								fprintf(output_file, " %d", get_FE_node_identifier(node));
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"write_FE_element.  Missing node");
								return_code = 0;
							}
						}
					}
					fprintf(output_file, "\n");
					indent_fprintf(output_file, indent, "</label>\n");
				}
				else
				{
					display_message(ERROR_MESSAGE, "write_FE_element.  Invalid nodes");
					return_code = 0;
				}
			}
		}
		if (get_FE_element_number_of_scale_factor_sets(element,
			&number_of_scale_factor_sets))
		{
			scale_factor_index = 0;
			for (i = 0 ; i < number_of_scale_factor_sets ; i++)
			{
				indent_fprintf(output_file, indent, "<label name=\"ScaleFactorList%d\">\n",
					i + 1);
				get_FE_element_numbers_in_scale_factor_set(element, i,
					&number_in_scale_factor_set);
				for (j = 0 ; j < number_in_scale_factor_set ; j++)
				{
					get_FE_element_scale_factor(element, scale_factor_index,
						&scale_factor);
					fprintf(output_file, " %"FE_VALUE_STRING, scale_factor);
					if ((0<FE_VALUE_MAX_OUTPUT_COLUMNS)&&
						(0==((j+1)%FE_VALUE_MAX_OUTPUT_COLUMNS)))
					{
						fprintf(output_file,"\n");
					}
					scale_factor_index++;
				}
				/* add extra carriage return for not multiple of
					 FE_VALUE_MAX_OUTPUT_COLUMNS values */
				if ((0 >= FE_VALUE_MAX_OUTPUT_COLUMNS) ||
					(0 != (j % FE_VALUE_MAX_OUTPUT_COLUMNS)))
				{
					fprintf(output_file,"\n");
				}
				indent_fprintf(output_file, indent, "</label>\n");
			}
		}
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</element>\n\n");
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_element.  Invalid element");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element */

static int FE_elements_have_same_element_interpolation(
	struct FE_element *element_1, struct FE_element *element_2,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns true if <element_1> and <element_2> can be written to file with the
same field header. If <field_order_info> is supplied only those fields
listed in it are compared.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_elements_have_same_element_interpolation);
	if (element_1 && element_2)
	{
		if (field_order_info)
		{
			return_code = 1;
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (i = 0; (i < number_of_fields) && return_code; i++)
			{
				if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
					equivalent_FE_field_in_elements(field, element_1, element_2)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = equivalent_FE_fields_in_elements(element_1, element_2);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_elements_have_same_element_interpolation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_elements_have_same_element_interpolation */

static int write_FE_region_element(struct FE_element *element,
	void *write_data_void)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Writes the <element> to the given file. If the fields defined at the element are
different from the last element then the header is written out. If the element
has no node_scale_field_info - ie. lines and faces - only the shape is output
in the header.
==============================================================================*/
{
	FILE *output_file;
	int j, return_code;
	struct Element_interpolation *element_interpolation, *element_interpolations;
	struct Write_FE_region_element_data *data;

	ENTER(write_FE_region_element);
	if (element &&
		(data = (struct Write_FE_region_element_data *)write_data_void) &&
			(output_file = data->output_file))
	{
		return_code = 1;
		/* only output the element if it has the specified dimension and fields
			 appropriate to the write_criterion */
		if ((data->dimension == get_FE_element_dimension(element)))
		{
			element_interpolation = (struct Element_interpolation *)NULL;
			if (0 < get_FE_element_number_of_fields(element))
			{
				/* Try and find a valid element_interpolation */
				for (j = 0 ; j < data->number_of_element_interpolations ; j++)
				{
					if (FE_elements_have_same_element_interpolation(element,
						data->element_interpolations[j].element, data->field_order_info))
					{
						element_interpolation = data->element_interpolations + j;
					}
				}
				if (!element_interpolation)
				{
					if (REALLOCATE(element_interpolations, data->element_interpolations,
						struct Element_interpolation, data->number_of_element_interpolations + 1))
					{
						data->element_interpolations = element_interpolations;
						element_interpolations[data->number_of_element_interpolations].
							element = element;
						sprintf(element_interpolations[data->number_of_element_interpolations].element_interpolation_name,
							"ElementInterpolation%d", data->number_of_element_interpolations + 1);
						element_interpolation = 
							element_interpolations + data->number_of_element_interpolations;
						data->number_of_element_interpolations++;

						write_FE_element_element_interpolation(output_file, 
							data->indent, element, element_interpolation,
							data->field_order_info,
							&(data->output_number_of_nodes),
							&(data->output_node_indices),
							&(data->output_number_of_scale_factors),
							&(data->output_scale_factor_indices),
							&(data->number_of_basis_mappings),
							&(data->basis_mappings),
							&(data->number_of_element_templates),
							&(data->element_templates));
					}
				}
			}
			write_FE_element(output_file, data->indent, element,
				element_interpolation, data->field_order_info,
				data->output_number_of_nodes,
				data->output_node_indices,
				data->output_number_of_scale_factors,
				data->output_scale_factor_indices);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_region_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_region_element */

static int write_FE_region(FILE *output_file, int indent, struct FE_region *fe_region,
	int write_elements, int write_nodes, struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Writes <fe_region> to the <output_file>.
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only element identifiers are output.
If <field_order_info> contains fields, they are written in that order.
==============================================================================*/
{
	int dimension, i, number_of_fields, return_code;
	struct FE_field *field;
	struct Write_FE_region_element_data write_elements_data;
	struct Write_FE_region_field_data write_fields_data;
	struct Write_FE_region_node_data write_nodes_data;

	ENTER(write_FE_region);
	if (output_file && fe_region)
	{
		return_code = 1;
		indent_fprintf(output_file, indent, "<fieldml xmlns=\"http://www.physiome.org.nz/fieldml/0.1#\"\n");
      indent_fprintf(output_file, indent + 9, "xmlns:fieldml=\"http://www.physiome.org.nz/fieldml/0.1#\">\n");
		indent += EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "<!-- Generated by Cmiss -->\n");
		if (field_order_info)
		{
			number_of_fields = get_FE_field_order_info_number_of_fields(
				field_order_info);
			for (i = 0; i < number_of_fields; i++)
			{
				if (field = get_FE_field_order_info_field(field_order_info, i))
				{
					write_FE_region_field(output_file, indent, field);
				}
			}
		}
		else
		{
			write_fields_data.output_file = output_file;
			write_fields_data.indent = indent;
			return_code = FE_region_for_each_FE_field(fe_region,
				write_FE_region_field_sub, &write_fields_data);
		}
		if (write_nodes)
		{
			write_nodes_data.output_file = output_file;
			write_nodes_data.indent = indent;
			write_nodes_data.field_order_info = field_order_info;
			write_nodes_data.number_of_node_templates = 0;
			write_nodes_data.node_templates = (struct Node_template *)NULL;
			return_code = FE_region_for_each_FE_node(fe_region,
				write_FE_region_node, &write_nodes_data);
			if (write_nodes_data.node_templates)
			{
				DEALLOCATE(write_nodes_data.node_templates);
			}
		}
		if (write_elements)
		{
			write_elements_data.output_file = output_file;
			write_elements_data.indent = indent;
			write_elements_data.output_number_of_nodes = 0;
			write_elements_data.output_node_indices = (int *)NULL;
			write_elements_data.output_number_of_scale_factors = 0;
			write_elements_data.output_scale_factor_indices = (int *)NULL;
			write_elements_data.field_order_info = field_order_info;
			write_elements_data.fe_region = fe_region;
			write_elements_data.last_element = (struct FE_element *)NULL;
			write_elements_data.number_of_basis_mappings = 0;
			write_elements_data.basis_mappings = (struct Basis_mapping *)NULL;
			write_elements_data.number_of_element_templates = 0;
			write_elements_data.element_templates = (struct Element_template *)NULL;
			write_elements_data.number_of_element_interpolations = 0;
			write_elements_data.element_interpolations = 
				(struct Element_interpolation *)NULL;
			/* write 1-D, 2-D then 3-D so lines and faces precede elements */
			for (dimension = 1; dimension <= 3; dimension++)
			{
				write_elements_data.dimension = dimension;
				if (!FE_region_for_each_FE_element(fe_region,
					write_FE_region_element, &write_elements_data))
				{
					return_code = 0;
				}
			}
			if (write_elements_data.basis_mappings)
			{
				DEALLOCATE(write_elements_data.basis_mappings);
			}
			if (write_elements_data.element_templates)
			{
				DEALLOCATE(write_elements_data.element_templates);
			}
			if (write_elements_data.element_interpolations)
			{
				DEALLOCATE(write_elements_data.element_interpolations);
			}
			DEALLOCATE(write_elements_data.output_node_indices);
			DEALLOCATE(write_elements_data.output_scale_factor_indices);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "write_FE_region.  Failed");
			return_code = 0;
		}
		indent -= EXPORT_INDENT_SPACES;
		indent_fprintf(output_file, indent, "</fieldml>\n");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_region */

static int write_Cmiss_region(FILE *output_file, struct Cmiss_region *region,
	struct Cmiss_region *root_region, int force_no_master_region,
	char *write_path, struct Cmiss_region *write_region, int indent,
	int write_elements, int write_nodes, struct FE_field_order_info *field_order_info,
	struct LIST(Cmiss_region_write_info) *write_info_list, char *path)
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Writes the contents of <region> to <output_file>. If <write_path> is supplied,
output is restricted to this path -- relative to <region>, and once the path is
complete the <write_region> is set to the final region in the path and
everything below it is output. Up to one of <write_path> and <write_region> may
be specified, and if both are omitted, everything from <region> down is written
to the file.
Where regions have master regions, full output is only made for the master and
only identifiers of nodes and elements in the servant region are output. If the
master region will not be written to the file because it is not under
<root_region> or the <write_path>/<write_region>, servant regions are upgraded
to full regions without masters for the benefit of output.
<indent_level> is increased by EXPORT_INDENT_SPACES with every recursive call to
this function.
<write_info_list> is used to store current information about how regions have
been written to date -- whether they are WRITTEN, NOT_WRITTEN or DECLARED; the
latter indicating that the region headers have been added to the file but the
content of the region is to be with another path to the region that follows
the master region being written. This eases writing information for
reconstructing the DAG when file is imported.
<write_elements>, <write_nodes>, <write_criterion> and <field_order_info> are
parameters for <write_FE_region>, called by this function.
<path> is the full currently-being-output path from <root_region> to <region>.
Notes:
* the master region must be a parent of the regions it is master to.
* element_xi values currently restricted to being in the root_region.
==============================================================================*/
{
	char *write_path_copy;
	int return_code;
	struct Cmiss_region *write_path_region;
	struct FE_region *fe_region;

	ENTER(write_Cmiss_region);
	USE_PARAMETER(force_no_master_region);
	if (output_file && region && root_region &&
		(!(write_path && write_region)) &&
		(0 <= indent) && write_info_list && path)
	{
		return_code = 1;
		if (write_path)
		{
			/* if <write_path> is "" or "/", the following clears it to NULL and
				 sets write_region to region */
			write_path_copy = duplicate_string(write_path);
			if (Cmiss_region_get_child_region_from_path(region, write_path,
				&write_path_region, &write_path))
			{
				if ((char *)NULL == write_path)
				{
					write_region = write_path_region;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_Cmiss_region.  Invalid write path");
				return_code = 0;
			}
			if (write_path_copy)
			{
				fprintf(output_file, "<group name=\"%s\">\n", write_path_copy);
			}
		}
		if (return_code)
		{
			if (fe_region = Cmiss_region_get_FE_region(region))
			{
				return_code = write_FE_region(output_file, indent, fe_region,
					write_elements, write_nodes, field_order_info);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"write_Cmiss_region.  Error writing finite element region");
				return_code = 0;
			}
		}
		if (write_path_copy)
		{
			fprintf(output_file, "</group>\n");
			DEALLOCATE(write_path_copy);
		}
#if defined (NEW_CODE)
		if (return_code)
		{
			if (return_code =
				Cmiss_region_get_number_of_child_regions(region, &number_of_children))
			{
				for (i = 0; (i < number_of_children) && return_code; i++)
				{
					child_region = Cmiss_region_get_child_region(root_region, i);
					if (return_code = (child_region != NULL) &&
						Cmiss_region_get_child_region_name(region, i, &child_region_name))
					{
						/* determine if child_region is to be written */
						if (((!write_path) || (child_region == write_path_region)) &&
							((!write_region) || Cmiss_region_contains_Cmiss_region(
								write_region, child_region)))
						{
							/* get info about whether child_region has already been written */
							if (return_code = get_Cmiss_region_write_info(write_info_list,
								child_region, &write_status, &first_path))
							{
								/* build the new_path to the child */
								if (ALLOCATE(child_path, char,
									strlen(path) + strlen(child_region_name) + 2))
								{
									strcpy(child_path, path);
									strcat(child_path, "/");
									strcat(child_path, child_region_name);
								}
								else
								{
									return_code = 0;
								}
								master_region = (struct Cmiss_region *)NULL;
								master_path = (char *)NULL;
								master_region_not_in_file = 0;
								if (write_status != CMISS_REGION_WRITTEN)
								{
									/* determine if child_region has a master Cmiss_region */
									if ((fe_region = Cmiss_region_get_FE_region(child_region)) &&
										(return_code = FE_region_get_master_FE_region(fe_region,
											&master_fe_region)) && master_fe_region &&
										(return_code = FE_region_get_Cmiss_region(master_fe_region,
											&master_region)) &&
										(return_code = get_Cmiss_region_write_info(write_info_list,
											master_region, &master_write_status, &master_path)))
									{
										if (CMISS_REGION_WRITTEN != master_write_status)
										{
											if (write_path) 
											{
												master_region_not_in_file = 1;
											}
											else if (write_region)
											{
												master_region_not_in_file =
													!Cmiss_region_contains_Cmiss_region(write_region,
														master_region);
											}
											else
											{
												master_region_not_in_file =
													!Cmiss_region_contains_Cmiss_region(root_region,
														master_region);
											}
										}
									}
								}
								if (return_code)
								{
									/* output indent */
									for (j = 0; j < indent_level; j++)
									{
										fputc(' ', output_file);
									}
									fprintf(output_file, "<region name=\"%s\"",
										child_region_name);
									/* if child_region has already been declared or written, write
										 its alternative_path to coordinate multiple parentage */
									if (write_status != CMISS_REGION_NOT_WRITTEN)
									{
										fprintf(output_file, " alternative_path=\"%s\"",
											first_path);
									}
									/* if child_region is not already written and has a master
										 region that is written, it will be fully written now and
										 we need to output its master_path */
									if ((CMISS_REGION_WRITTEN != write_status) && master_region &&
										(CMISS_REGION_WRITTEN == master_write_status))
									{
										fprintf(output_file, " master_path=\"%s\"", master_path);
									}
									fprintf(output_file, ">\n");
									/* write child_region if it has not been written yet and has
										 no master_region or its master_region is either already
										 written or will not appear in the file */
									if ((CMISS_REGION_WRITTEN != write_status) &&
										((!master_region) ||
											((CMISS_REGION_WRITTEN == master_write_status) ||
												master_region_not_in_file)))
									{
										return_code = write_Cmiss_region(output_file,
											child_region, root_region,
											((!master_region) || master_region_not_in_file),
											write_path, write_region,
											indent_level + EXPORT_INDENT_SPACES,
											write_elements, write_nodes, write_criterion,
											field_order_info, write_info_list, child_path);
										write_status = CMISS_REGION_WRITTEN;
									}
									else
									{
										write_status = CMISS_REGION_DECLARED;
									}
									/* output indent */
									for (j = 0; j < indent_level; j++)
									{
										fputc(' ', output_file);
									}
									fprintf(output_file, "</region>");
									if (return_code)
									{
										return_code = set_Cmiss_region_write_info(write_info_list,
											child_region, write_status, path);
									}
								}
								DEALLOCATE(child_path);
							}
						}
						DEALLOCATE(child_region_name);
					}
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE, "write_Cmiss_region.  Failed");
				return_code = 0;
			}
		}
#endif /* defined (NEW_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_Cmiss_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_Cmiss_region */

/*
Global functions
----------------
*/

int write_fieldml_file(FILE *output_file,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 3 January 2008

DESCRIPTION :
Writes an exregion file to <output_file> with <root_region> at the top level of
the file.  Optionally the <write_path> restricts the output to only that part 
of the hierarchy.
If the structure of the file permits it to be written in the old exnode/exelem
format this is done so; this is only possible if the output hierarchy is
two-deep and the second level contains only regions which use their parent's
name space for fields etc. In all other cases, <region> and </region> elements
are used to start and end regions so that they can be nested.
The <write_elements>, <write_nodes>, <write_criterion> and <field_order_info>
control what part of the regions are written:
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only object identifiers are output.
If <field_order_info> contains fields, they are written in that order.
==============================================================================*/
{
	int return_code;
	struct LIST(Cmiss_region_write_info) *write_info_list;

	ENTER(write_exregion_file);
	if (output_file && root_region)
	{
		return_code = 1;
		write_info_list = CREATE(LIST(Cmiss_region_write_info))();
		fprintf(output_file, "<regionml>\n");
		return_code = write_Cmiss_region(output_file,
			root_region, root_region, /*no_master*/1, write_path,
			/*write_region*/(struct Cmiss_region *)NULL, EXPORT_INDENT_SPACES,
			write_elements, write_nodes, field_order_info,
			write_info_list, /*path*/"");
		fprintf(output_file, "</regionml>\n");
		DESTROY(LIST(Cmiss_region_write_info))(&write_info_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_exregion_file */

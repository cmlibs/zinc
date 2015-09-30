/*******************************************************************************
FILE : export_finite_element.c

LAST MODIFIED : 16 May 2003

DESCRIPTION :
Functions for exporting finite element data to a file.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "zinc/element.h"
#include "zinc/field.h"
#include "zinc/fieldgroup.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/node.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/export_finite_element.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "region/cmiss_region_write_info.h"
#include "general/message.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

/* the number of spaces each child object is indented from its parent by in the
	 output file */
#define EXPORT_INDENT_SPACES 2

/*
Module types
------------
*/

struct Write_FE_region_element_data
{
	ostream *output_file;
	int output_number_of_nodes, *output_node_indices,
		output_number_of_scale_factors, *output_scale_factor_indices;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct FE_element *last_element;
	struct FE_region *fe_region;
	FE_value time;
}; /* struct Write_FE_region_element_data */

struct Write_FE_node_field_values
{
	ostream *output_file;
	/* store number of values for writing nodal values in columns */
	int number_of_values;
	FE_value time;
};

struct Write_FE_node_field_info_sub
{
	/* field_number and value_index are incremented by write_FE_node_field so
		 single and multiple field output can be handled appropriately. Both must be
		 initialised to 1 before the first time write_FE_node_field is called */
	int field_number,value_index;
	ostream *output_file;
}; /* Write_FE_node_field_info_sub */

struct Write_FE_region_node_data
{
	ostream *output_file;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct FE_node *last_node;
	FE_value time;
}; /* struct Write_FE_region_node_data */

/*
Module functions
----------------
*/

static int write_element_xi_value(ostream *output_file,struct FE_element *element,
	FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes to <output_file> the element_xi position in the format:
E<lement>/F<ace>/L<ine> ELEMENT_NUMBER DIMENSION xi1 xi2... xiDIMENSION
==============================================================================*/
{
	char element_char;
	int i, return_code;

	ENTER(write_element_xi_value);
	int dimension = get_FE_element_dimension(element);
	if (output_file && (0 < dimension))
	{
		int identifier = get_FE_element_identifier(element);
		if (dimension == 2)
			element_char = 'F';
		else if (dimension == 1)
			element_char = 'L';
		else
			element_char = 'E';
		(*output_file) << " " << element_char << " " <<  identifier << " " << dimension;
		for (i = 0; i < dimension; i++)
		{
			char num_string[100];
			sprintf(num_string, " %"FE_VALUE_STRING, xi[i]);
			(*output_file) << num_string;
		}
		return_code = 1;
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

static int write_FE_field_header(ostream *output_file,int field_number,
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Writes the part of the field header that is common to exnode and exelem files.
Examples:
1) coordinates, coordinate, rectangular cartesian, #Components=3
2) variable, field, indexed, Index_field=bob, #Values=3, real, #Components=1
3) fixed, field, constant, integer, #Components=3
4) an_array, field, real, #Values=10, #Components=1
Value_type ELEMENT_XI_VALUE has optional Mesh Dimension=#.
==============================================================================*/
{
	char *name;
	enum FE_field_type fe_field_type;
	enum Value_type value_type;
	int number_of_components,return_code;
	struct Coordinate_system *coordinate_system;

	ENTER(write_FE_field_header);
	if (output_file&&field)
	{
		(*output_file) << " " << field_number << ") ";
		/* write the field name */
		if (GET_NAME(FE_field)(field,&name))
		{
			(*output_file) << name;
			DEALLOCATE(name);
		}
		else
		{
			(*output_file) << "unknown";
		}
		(*output_file) << ", " << ENUMERATOR_STRING(CM_field_type)(get_FE_field_CM_field_type(field));
		/* optional constant/indexed, Index_field=~, #Values=# */
		fe_field_type=get_FE_field_FE_field_type(field);
		switch (fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				(*output_file) << ", constant";
			} break;
			case GENERAL_FE_FIELD:
			{
				/* default; nothing to write */
			} break;
			case INDEXED_FE_FIELD:
			{
				struct FE_field *indexer_field;
				int number_of_indexed_values;

				(*output_file) << ", indexed, Index_field=";
				if (get_FE_field_type_indexed(field,&indexer_field,
					&number_of_indexed_values))
				{
					if (GET_NAME(FE_field)(indexer_field,&name))
					{
						(*output_file) << name;
						DEALLOCATE(name);
					}
					else
					{
						(*output_file) << "unknown";
					}
					(*output_file) << ", #Values=" << number_of_indexed_values;
				}
				else
				{
					(*output_file) << "unknown, #Values=0";
					display_message(ERROR_MESSAGE,
						"write_FE_field_header.  Invalid indexed field");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"write_FE_field_header.  Invalid FE_field_type");
			} break;
		}
		if (NULL != (coordinate_system=get_FE_field_coordinate_system(field)))
		{
			switch (coordinate_system->type)
			{
				case CYLINDRICAL_POLAR:
				{
					(*output_file) << ", cylindrical polar";
				} break;
				case FIBRE:
				{
					(*output_file) << ", fibre";
				} break;
				case OBLATE_SPHEROIDAL:
				{
					char num_string[100];
					sprintf(num_string, "%"FE_VALUE_STRING, coordinate_system->parameters.focus);
					(*output_file) << ", oblate spheroidal, focus=" << num_string;
				} break;
				case PROLATE_SPHEROIDAL:
				{
					char num_string[100];
					sprintf(num_string, "%"FE_VALUE_STRING, coordinate_system->parameters.focus);
					(*output_file) << ", prolate spheroidal, focus=" << num_string;
				} break;
				case RECTANGULAR_CARTESIAN:
				{
					(*output_file) << ", rectangular cartesian";
				} break;
				case SPHERICAL_POLAR:
				{
					(*output_file) << ", spherical polar";
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
		value_type=get_FE_field_value_type(field);
		/* for backwards compatibility with old file formats, if there is a valid
			 coordinate system only write value_type if it is not FE_VALUE_VALUE */
		if ((FE_VALUE_VALUE!=value_type) || (!coordinate_system) ||
			(NOT_APPLICABLE==coordinate_system->type))
		{
			(*output_file) << ", " << Value_type_string(value_type);
		}
		number_of_components=get_FE_field_number_of_components(field);
		(*output_file) << ", #Components=" << number_of_components;
		if (ELEMENT_XI_VALUE == value_type)
		{
			int element_xi_mesh_dimension = FE_field_get_element_xi_mesh_dimension(field);
			if (element_xi_mesh_dimension != 0)
			{
				(*output_file) << "; mesh dimension=" << element_xi_mesh_dimension;
			}
		}
		(*output_file) << "\n";
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

static int write_FE_field_values(ostream *output_file,struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

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
							char num_string[100];
							sprintf(num_string, "%"FE_VALUE_STRING, value);
							(*output_file) << " " << num_string;
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
							(*output_file) << " " << value;
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
								(*output_file) << " " << the_string;
								DEALLOCATE(the_string);
							}
							else
							{
								/* empty string */
								(*output_file) << " \"\"";
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
			(*output_file) << "\n";
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

static int write_FE_element_field_FE_field_values(struct FE_element *element,
	struct FE_field *field,void *output_file_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Iterator function for fields defined at a element. Wrapper for call to
write_FE_field_values.
==============================================================================*/
{
	int return_code;

	ENTER(write_FE_element_field_FE_field_values);
	USE_PARAMETER(element);
	return_code=write_FE_field_values((ostream *)output_file_void,field);
	LEAVE;

	return (return_code);
} /* write_FE_element_field_FE_field_values */

static int write_FE_node_field_FE_field_values(struct FE_node *node,
	struct FE_field *field,void *output_file_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Iterator function for fields defined at a node. Wrapper for call to
write_FE_field_values.
==============================================================================*/
{
	int return_code;

	ENTER(write_FE_node_field_FE_field_values);
	USE_PARAMETER(node);
	return_code=write_FE_field_values((ostream *)output_file_void,field);
	LEAVE;

	return (return_code);
} /* write_FE_node_field_FE_field_values */

/***************************************************************************//**
 * Writes out the <element_shape> to <output_file>.
 */
static int write_FE_element_shape(ostream *output_file,
	struct FE_element_shape *element_shape)
{
	int return_code = 1;

	ENTER(write_FE_element_shape);
	if (output_file && element_shape)
	{
		const int dimension = get_FE_element_shape_dimension(element_shape);
		(*output_file) << " Shape. Dimension=" << dimension << ", ";
		char *shape_description = FE_element_shape_get_EX_description(element_shape);
		if (shape_description)
		{
			(*output_file) << shape_description << "\n";
			DEALLOCATE(shape_description);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_FE_element_shape.  Invalid shape");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_shape.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int write_FE_element_identifier(ostream *output_file,
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

	ENTER(write_FE_element_identifier);
	if (output_file && element)
	{
		return_code = 1;
		int dimension = get_FE_element_dimension(element);
		int identifier = get_FE_element_identifier(element);
		if ((3 == dimension) || FE_element_is_top_level(element, (void *)NULL))
		{
			(*output_file) << identifier << " 0 0";
		}
		else if (2 == dimension)
		{
			(*output_file) << "0 " << identifier << " 0";
		}
		else if (1 == dimension)
		{
			(*output_file) << "0 0 " << identifier;
		}
		else
		{
			(*output_file) << identifier << " 0 0";
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

static int write_FE_basis(ostream *output_file,struct FE_basis *basis)
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Writes out the <basis> to <output_file>.
==============================================================================*/
{
	char *basis_string;
	int return_code;

	ENTER(write_FE_basis);
	if (output_file && basis)
	{
		return_code = 1;
		basis_string = FE_basis_get_description_string(basis);
		if (basis_string)
		{
			(*output_file) << basis_string;
			DEALLOCATE(basis_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_FE_basis.  Invalid basis");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_basis.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_basis */

struct Write_FE_element_field_sub
{
	ostream *output_file;
	int field_number,output_number_of_nodes,*output_node_indices,
		output_number_of_scale_factors,*output_scale_factor_indices;
}; /* struct Write_FE_element_field_sub */

static int write_FE_element_field_sub(struct FE_element *element,
	struct FE_field *field,void *write_element_field_data_void)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes information describing how <field> is defined at <element>.
==============================================================================*/
{
	char *component_name;
	enum FE_field_type fe_field_type;
	enum Global_to_element_map_type component_type;
	FE_element_field_component_modify modify;
	ostream *output_file;
	int i, j, k, node_index, number_in_xi,
		number_of_components, number_of_nodal_values,
		number_of_nodes, number_of_xi_coordinates, return_code, scale_factor_index;
	struct FE_basis *basis;
	struct FE_element_field_component *component;
	struct Standard_node_to_element_map *standard_node_map;
	struct Write_FE_element_field_sub *write_element_field_data;

	ENTER(write_FE_element_field_sub);
	write_element_field_data = (struct Write_FE_element_field_sub *)write_element_field_data_void;
	if (element && field && (NULL != write_element_field_data) &&
		(NULL != (output_file = write_element_field_data->output_file)) &&
		((0 == write_element_field_data->output_number_of_nodes) ||
			write_element_field_data->output_node_indices) &&
		((0 == write_element_field_data->output_number_of_scale_factors) ||
			write_element_field_data->output_scale_factor_indices))
	{
		return_code = 1;
		write_FE_field_header(output_file,write_element_field_data->field_number,
			field);
		fe_field_type=get_FE_field_FE_field_type(field);
		write_element_field_data->field_number++;
		number_of_components=get_FE_field_number_of_components(field);
		for (i = 0; i < number_of_components; i++)
		{
			if (NULL != (component_name = get_FE_field_component_name(field, i)))
			{
				(*output_file) << " " << component_name << ". ";
				DEALLOCATE(component_name);
			}
			else
			{
				(*output_file) << "  " << i + 1 << ".";
			}
			if (GENERAL_FE_FIELD == fe_field_type)
			{
				if (get_FE_element_field_component(element, field, i, &component))
				{
					FE_element_field_component_get_basis(component, &basis);
					write_FE_basis(output_file, basis);
					FE_element_field_component_get_modify(component, &modify);
					if (!modify)
					{
						(*output_file) << ", no modify";
					}
					else if (modify == theta_increasing_in_xi1)
					{
						(*output_file) << ", increasing in xi1";
					}
					else if (modify == theta_decreasing_in_xi1)
					{
						(*output_file) << ", decreasing in xi1";
					}
					else if (modify == theta_non_increasing_in_xi1)
					{
						(*output_file) << ", non-increasing in xi1";
					}
					else if (modify == theta_non_decreasing_in_xi1)
					{
						(*output_file) << ", non-decreasing in xi1";
					}
					else
					{
						(*output_file) << ", unknown modify function";
						display_message(ERROR_MESSAGE,
							"write_FE_element_field.  Unknown modify function");
					}
					if (FE_element_field_component_get_type(component, &component_type))
					{
						switch (component_type)
						{
							case STANDARD_NODE_TO_ELEMENT_MAP:
							{
								(*output_file) << ", standard node based.\n";
								if (FE_element_field_component_get_number_of_nodes(component,
									&number_of_nodes))
								{
									(*output_file) << "   #Nodes=" << number_of_nodes << "\n";
									for (j = 0; j < number_of_nodes; j++)
									{
										if (FE_element_field_component_get_standard_node_map(
											component, j, &standard_node_map) &&
											Standard_node_to_element_map_get_node_index(
												standard_node_map, &node_index) &&
											Standard_node_to_element_map_get_number_of_nodal_values(
												standard_node_map, &number_of_nodal_values))
										{
											/* node indices are renumbered */
											(*output_file) << "   " <<
												write_element_field_data->output_node_indices[node_index] + 1 <<
												". #Values=" << number_of_nodal_values << "\n";
#ifdef CMZN_OLD_VALUE_INDICES
											// this code is kept for documenting old EX format only
											/* nodal value indices are all relative so output as is */
											(*output_file) << "     Value indices:";
											for (k = 0; k < number_of_nodal_values; k++)
											{
												int nodal_value_index;
												Standard_node_to_element_map_get_nodal_value_index(
													standard_node_map, k, &nodal_value_index);
												(*output_file) << " " << nodal_value_index + 1;
											}
											(*output_file) << "\n";
#endif // CMZN_OLD_VALUE_INDICES
											/* nodal value labels(versions) e.g. d/ds1(2) */
											(*output_file) << "     Value labels:";
											for (k = 0; k < number_of_nodal_values; k++)
											{
												FE_nodal_value_type valueType =
													Standard_node_to_element_map_get_nodal_value_type(standard_node_map, k);
												if (FE_NODAL_UNKNOWN == valueType)
												{
													(*output_file) << " zero";
												}
												else
												{
													const char *valueTypeString = ENUMERATOR_STRING(FE_nodal_value_type)(valueType);
													(*output_file) << " " << valueTypeString;
													int nodalVersion = Standard_node_to_element_map_get_nodal_version(standard_node_map, k);
													if (1 != nodalVersion)
														(*output_file) << "(" << nodalVersion << ")";
												}
											}
											(*output_file) << "\n";
											/* scale factor indices are renumbered */
											(*output_file) << "     Scale factor indices:";
											for (k = 0; k < number_of_nodal_values; k++)
											{
												scale_factor_index = Standard_node_to_element_map_get_scale_factor_index(standard_node_map, k);
												if (0 <= scale_factor_index)
												{
													(*output_file) << " " << write_element_field_data->
														output_scale_factor_indices[scale_factor_index]+1;
												}
												else
												{
													/* non-positive index means no scale factor so value
														 of 1.0 assumed */
													(*output_file) << " 0";
												}
											}
											(*output_file) << "\n";
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"write_FE_element_field_sub.  "
												"Missing standard node to element map");
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "write_FE_element_field_sub.  "
										"Could not get number of nodes");
								}
							} break;
							case GENERAL_ELEMENT_MAP:
							{
								(*output_file) << ", general map based.\n";
								display_message(ERROR_MESSAGE,"write_FE_element_field_sub.  "
									"general map not supported");
							} break;
							case ELEMENT_GRID_MAP:
							{
								(*output_file) << ", grid based.\n";
								FE_basis_get_dimension(basis, &number_of_xi_coordinates);
								(*output_file) << " ";
								for (j = 0; j < number_of_xi_coordinates; j++)
								{
									if (0 < j)
									{
										(*output_file) << ", ";
									}
									FE_element_field_component_get_grid_map_number_in_xi(
										component, j, &number_in_xi);
									(*output_file) << "#xi" << j + 1 << "=" << number_in_xi;
								}
								(*output_file) << "\n";
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_FE_element_field_sub.  Could not get element map type");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_FE_element_field_sub.  Missing element field component");
				}
			}
			else
			{
				/* constant and indexed fields: no further component information */
				(*output_file) << "\n";
			}
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

static int write_FE_element_field_info(ostream *output_file,
	struct FE_element *element,
	struct FE_field_order_info *field_order_info,
	int *output_number_of_nodes,int **output_node_indices,
	int *output_number_of_scale_factors,int **output_scale_factor_indices)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

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
	int field_no, i, j, number_of_components, number_of_fields = 0,
		number_of_fields_in_header, number_of_nodes,
		number_of_scale_factor_sets, number_of_scale_factors,
		number_in_scale_factor_set,
		output_number_of_scale_factor_sets, output_scale_factor_index, return_code,
		scale_factor_index, *scale_factor_set_in_use,
		write_field_values;
	struct FE_element_field_component *component;
	struct FE_field *field;
	struct Write_FE_element_field_sub write_element_field_data;

	ENTER(write_FE_element_field_info);
	if (output_file && element && output_number_of_nodes && output_node_indices &&
		output_number_of_scale_factors && output_scale_factor_indices)
	{
		return_code = 1;
		if (0 < get_FE_element_number_of_fields(element))
		{
			/* reallocate/clear output parameters */
			*output_number_of_nodes = 0;
			if (get_FE_element_number_of_nodes(element, &number_of_nodes) &&
				(0 < number_of_nodes))
			{
				int *temp_indices;
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
				int *temp_indices;
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
							if ((field =
								get_FE_field_order_info_field(field_order_info, field_no)) &&
								FE_field_is_defined_in_element(field, element) &&
								(number_of_components =
									get_FE_field_number_of_components(field)))
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
												cmzn_mesh_scale_factor_set *scale_factor_set =
													FE_element_field_component_get_scale_factor_set(component);
												if (scale_factor_set)
												{
													for (j = 0; j < number_of_scale_factor_sets; j++)
													{
														if (get_FE_element_scale_factor_set_identifier_at_index(element, j) ==
															scale_factor_set)
														{
															scale_factor_set_in_use[j] = 1;
														}
													}
												}
											}
											FE_element_field_component_get_local_node_in_use(
												component, number_of_nodes, *output_node_indices);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"write_FE_element_field_info.  "
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
					(*output_file) << " #Scale factor sets=" << output_number_of_scale_factor_sets << "\n";
					*output_number_of_scale_factors = 0;
					output_scale_factor_index=scale_factor_index = 0;
					for (j = 0; j < number_of_scale_factor_sets; j++)
					{
						number_in_scale_factor_set = get_FE_element_number_in_scale_factor_set_at_index(element, j);
						if (scale_factor_set_in_use[j])
						{
							(*output_file) << " ";
							cmzn_mesh_scale_factor_set *scale_factor_set =
								get_FE_element_scale_factor_set_identifier_at_index(element, j);
							(*output_file) << scale_factor_set->getName();
							(*output_file) << ", #Scale factors=" << number_in_scale_factor_set << "\n";
							/* set output scale factor indices */
							for (i = number_in_scale_factor_set; 0 < i; i--)
							{
								(*output_scale_factor_indices)[scale_factor_index]=
									output_scale_factor_index;
								scale_factor_index++;
								output_scale_factor_index++;
							}
							*output_number_of_scale_factors += number_in_scale_factor_set;
						}
						else
						{
							scale_factor_index += number_in_scale_factor_set;
						}
					}
					/* output number of nodes */
					(*output_file) << " #Nodes=" << *output_number_of_nodes << "\n";
					/* output fields / components */
					write_element_field_data.output_file=output_file;
					write_element_field_data.field_number=1;
					write_element_field_data.output_number_of_nodes=
						*output_number_of_nodes;
					write_element_field_data.output_node_indices=
						*output_node_indices;
					write_element_field_data.output_number_of_scale_factors=
						*output_number_of_scale_factors;
					write_element_field_data.output_scale_factor_indices=
						*output_scale_factor_indices;
					
					(*output_file) << " #Fields=" << number_of_fields_in_header << "\n";
					if (field_order_info)
					{
						for (field_no = 0; field_no < number_of_fields; field_no++)
						{
							if ((field =
								get_FE_field_order_info_field(field_order_info, field_no)) &&
								FE_field_is_defined_in_element(field, element))
							{
								write_FE_element_field_sub(element, field,
									(void *)&write_element_field_data);
							}
						}
					}
					else
					{
						for_each_FE_field_at_element_alphabetical_indexer_priority(
							write_FE_element_field_sub,(void *)&write_element_field_data,
							element);
					}

					if (write_field_values)
					{
						(*output_file) << " Values :\n";
						if (field_order_info)
						{
							for (field_no = 0; field_no < number_of_fields; field_no++)
							{
								if ((field =
									get_FE_field_order_info_field(field_order_info, field_no))
									&& FE_field_is_defined_in_element(field, element))
								{
									write_FE_element_field_FE_field_values(element, field,
										(void *)output_file);
								}
							}
						}
						else
						{
							for_each_FE_field_at_element_alphabetical_indexer_priority(
								write_FE_element_field_FE_field_values,
								(void *)output_file,element);
						}
					}
					DEALLOCATE(scale_factor_set_in_use);
				}
				else
				{
					display_message(ERROR_MESSAGE,"write_FE_element_field_info.  "
						"Not enough memory for scale_factor_set_in_use");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_FE_element_field_info.  Not enough memory for indices");
			}
		}
		else
		{
			/* no scale factors, nodes or fields */
			(*output_file) << " #Scale factor sets=0\n";
			(*output_file) << " #Nodes=0\n";
			(*output_file) << " #Fields=0\n";
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_field_info */

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
	ostream *output_file;
	int i, j, number_of_components, number_of_values, return_code;
	int number_of_columns,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_element_field_component *component;

	if (element && field && (NULL != (output_file = (ostream *)output_file_void)))
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
							get_FE_element_field_component_number_of_grid_values(element,field,i)))&&
							get_FE_element_field_component_grid_map_number_in_xi(element,
								field,/*component_number*/i,number_in_xi) &&
								(0 < (number_of_columns=number_in_xi[0]+1)))
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
											char num_string[100];
											sprintf(num_string," %"FE_VALUE_STRING,values[j]);
											(* output_file) << num_string;
											if (0==((j+1)%number_of_columns))
											{
												(* output_file) << "\n";
											}
										}
										/* extra newline if not multiple of number_of_columns */
										if (0 != (number_of_values % number_of_columns))
										{
											(* output_file) << "\n";
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
											(* output_file) << " " << values[j];
											if (0==((j+1)%number_of_columns))
											{
												(* output_file) << "\n";
											}
										}
										/* extra newline if not multiple of number_of_columns */
										if (0 != (number_of_values % number_of_columns))
										{
											(* output_file) << "\n";
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

static int write_FE_element(ostream *output_file,struct FE_element *element,
	struct FE_field_order_info *field_order_info,
	int output_number_of_nodes,int *output_node_indices,
	int output_number_of_scale_factors,int *output_scale_factor_indices)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Writes out an element to <output_file> or the socket (if <output_file> is NULL).
Output is in the order:
 Element: 1 0 0
 Faces: 
 0 1 0
 0 2 0
 0 3 0
 0 4 0
 0 5 0
 0 6 0
 Values:
 1 2 4 16 32 64 128 6 7 8 7 8 9 8 9 10 9 10 11 10 11 12 7 8 9 8 9 10 9 10 11 10
 11 12 11 12 13 8 9 10 9 10 11 10 11 12 11 12 13 12 13 14 9 10 11 10 11 12 11 12
 13 12 13 14 13 14 15
 Nodes:
 1 2 3 4 5 6 7 8
 Scale factors:
  0.100000E+01  0.100000E+01  0.100000E+01  0.100000E+01  0.100000E+01
  0.100000E+01  0.100000E+01  0.100000E+01
Notes:
- Faces are only output for elements with the faces array allocated. Missing
  faces are given the identifier 0 0 0 as expected by read_FE_element.
- Values, Nodes and Scale Factors are only output for elements with
  node_scale_field_information, and for which their respective number, in the
	function arguments is greater than 0.
- If <field_order_info> is specified then only values for the listed fields
  are output.
- Function uses output_* parameters set up by write_FE_element_field_info.
==============================================================================*/
{
	FE_value scale_factor;
	int field_no, i, number_of_fields,
		number_of_nodes, number_of_scale_factors, return_code,
		total_number_of_scale_factors;
	struct FE_element *face;
	struct FE_field *field;
	struct FE_node *node;

	ENTER(write_FE_element);
	if (output_file && element &&
		((0 == output_number_of_nodes) || output_node_indices) &&
		((0 == output_number_of_scale_factors) || output_scale_factor_indices))
	{
		return_code = 1;
		/* write the element identifier */
		(*output_file) << " Element: ";
		write_FE_element_identifier(output_file, element);
		(*output_file) << "\n";
		/* only write faces if writing fields i.e. not with groups */
		if (!field_order_info || (get_FE_field_order_info_number_of_fields(field_order_info) > 0))
		{
			const int number_of_faces = FE_element_shape_get_number_of_faces(get_FE_element_shape(element));
			if (0 < number_of_faces)
			{
				/* write the faces */
				(*output_file) << " Faces:\n";
				for (i = 0; i < number_of_faces; i++)
				{
					(*output_file) << " ";
					if (get_FE_element_face(element, i, &face) && face)
					{
						write_FE_element_identifier(output_file, face);
					}
					else
					{
						/* no face = no number, for compatibility with read_FE_element */
						(*output_file) << "0 0 0";
					}
					(*output_file) << "\n";
				}
			}
		}
		if (field_order_info)
		{
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			int first_grid_field = 1;
			for (field_no = 0; field_no < number_of_fields; field_no++)
			{
				if ((field =
					get_FE_field_order_info_field(field_order_info, field_no)) &&
					FE_element_field_is_grid_based(element,field))
				{
					if (first_grid_field)
					{
						(*output_file) << " Values :\n";
						first_grid_field = 0;
					}
					write_FE_element_field_values(element, field, (void *)output_file);
				}
			}
		}
		else
		{
			if (FE_element_has_grid_based_fields(element))
			{
				(*output_file) << " Values :\n";
				for_each_FE_field_at_element_alphabetical_indexer_priority(
					write_FE_element_field_values,(void *)output_file,element);
			}
		}
		if (0 < output_number_of_nodes)
		{
			/* write the nodes */
			if (get_FE_element_number_of_nodes(element, &number_of_nodes))
			{
				if (0 < number_of_nodes)
				{
					(*output_file) << " Nodes:\n";
					for (i = 0; i < number_of_nodes; i++)
					{
						if (0 <= output_node_indices[i])
						{
							if (get_FE_element_node(element, i, &node))
							{
								(*output_file) << " " << get_FE_node_identifier(node);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"write_FE_element.  Missing node");
								return_code = 0;
							}
						}
					}
					(*output_file) << "\n";
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "write_FE_element.  Invalid nodes");
				return_code = 0;
			}
		}
		if (0<output_number_of_scale_factors)
		{
			/* write the scale_factors */
			if (get_FE_element_number_of_scale_factors(element,
				&total_number_of_scale_factors))
			{
				if (0 < total_number_of_scale_factors)
				{
					(*output_file) << " Scale factors:\n";
					number_of_scale_factors=0;
					/* output in columns if FE_VALUE_MAX_OUTPUT_COLUMNS > 0 */
					for (i = 0; i < total_number_of_scale_factors; i++)
					{
						if (0 <= output_scale_factor_indices[i])
						{
							number_of_scale_factors++;
							get_FE_element_scale_factor(element, i, &scale_factor);
							char num_string[100];
							sprintf(num_string, "%"FE_VALUE_STRING, scale_factor);
							(*output_file) << " " << num_string;
							if ((0<FE_VALUE_MAX_OUTPUT_COLUMNS)&&
								(0==(number_of_scale_factors%FE_VALUE_MAX_OUTPUT_COLUMNS)))
							{
								(*output_file) << "\n";
							}
						}
					}
					/* add extra carriage return for not multiple of
						FE_VALUE_MAX_OUTPUT_COLUMNS values */
					if ((0 >= FE_VALUE_MAX_OUTPUT_COLUMNS) ||
						(0 != (number_of_scale_factors % FE_VALUE_MAX_OUTPUT_COLUMNS)))
					{
						(*output_file) << "\n";
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_FE_element.  Invalid scale_factors");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_element.  Invalid element");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element */

static int FE_element_passes_write_criterion(struct FE_element *element,
	struct FE_region *fe_region,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if the <write_criterion> -- some options of which require the
<field_order_info> -- indicates the <element> is to be written.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_element_passes_write_criterion);
	switch (write_criterion)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
			return_code = 1;
		} break;
		case FE_WRITE_WITH_ALL_LISTED_FIELDS:
		{
			if (element && fe_region && field_order_info &&
				(0 < (number_of_fields =
					get_FE_field_order_info_number_of_fields(field_order_info))))
			{
				return_code = 1;
				for (i = 0; (i < number_of_fields) && return_code; i++)
				{
					if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
						FE_element_or_parent_has_field(element, field,
							(LIST_CONDITIONAL_FUNCTION(FE_element) *)0, (void *)0)))
					{
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_passes_write_criterion.  Invalid argument(s)");
				return_code = 0;
			}
		} break;
		case FE_WRITE_WITH_ANY_LISTED_FIELDS:
		{
			if (element && fe_region && field_order_info &&
				(0 < (number_of_fields =
					get_FE_field_order_info_number_of_fields(field_order_info))))
			{
				return_code = 0;
				for (i = 0; (i < number_of_fields) && (!return_code); i++)
				{
					if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
						FE_element_or_parent_has_field(element, field,
							(LIST_CONDITIONAL_FUNCTION(FE_element) *)0, (void *)0))
					{
						return_code = 1;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_passes_write_criterion.  Invalid argument(s)");
				return_code = 0;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_element_passes_write_criterion.  Unknown write_criterion");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* FE_element_passes_write_criterion */


/**
 * Return true if any fields are to be written for the element.
 */
static bool FE_element_has_fields_to_write(
	struct FE_element *element, struct FE_field_order_info *field_order_info)
{
	if (field_order_info)
	{
		struct FE_field *field;
		int number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
		for (int i = 0; i < number_of_fields; ++i)
		{
			field = get_FE_field_order_info_field(field_order_info, i);
			if (FE_field_is_defined_in_element_not_inherited(field, element))
				return true;
		}
		return false;
	}
	return (0 < get_FE_element_number_of_fields(element));
}

/**
 * Returns true if <element_1> and <element_2> can be written to file with the
 * same field header. If <field_order_info> is supplied only those fields
 * listed in it are compared.
 */
static bool FE_elements_have_same_header(
	struct FE_element *element_1, struct FE_element *element_2,
	struct FE_field_order_info *field_order_info)
{
	if (element_1 && element_2)
	{
		if (field_order_info)
		{
			struct FE_field *field;
			int number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
			for (int i = 0; i < number_of_fields; ++i)
			{
				if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
					equivalent_FE_field_in_elements(field, element_1, element_2)))
				{
					return false;
				}
			}
			return true;
		}
		return (0 != equivalent_FE_fields_in_elements(element_1, element_2));
	}
	display_message(ERROR_MESSAGE,
		"FE_elements_have_same_header.  Invalid argument(s)");
	return false;
}

static int write_FE_region_element(struct FE_element *element,
	Write_FE_region_element_data *write_elements_data)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Writes the <element> to the given file. If the fields defined at the element are
different from the last element then the header is written out. If the element
has no node_scale_field_info - ie. lines and faces - only the shape is output
in the header.
==============================================================================*/
{
	ostream *output_file;
	int return_code;
	struct FE_element_shape *element_shape, *last_element_shape;

	ENTER(write_FE_region_element);
	if (element && write_elements_data &&
		(0 != (output_file = write_elements_data->output_file)))
	{
		return_code = 1;
		/* only output the element if it has the specified dimension and fields
			 appropriate to the write_criterion */
		if (FE_element_passes_write_criterion(element, write_elements_data->fe_region,
				write_elements_data->write_criterion, write_elements_data->field_order_info))
		{
			/* work out if shape or field header have changed from last element */
			element_shape = get_FE_element_shape(element);
			if (element_shape)
			{
				bool new_shape = true;
				bool new_field_header = true;
				if (write_elements_data->last_element)
				{
					last_element_shape = get_FE_element_shape(write_elements_data->last_element);
					new_shape = (element_shape != last_element_shape);
					if (new_shape)
					{
						new_field_header = FE_element_has_fields_to_write(element,
							write_elements_data->field_order_info);
					}
					else
					{
						new_field_header = !FE_elements_have_same_header(element,
							write_elements_data->last_element, write_elements_data->field_order_info);
					}
				}
				if (new_shape)
				{
					write_FE_element_shape(output_file, element_shape);
				}
				if (new_field_header)
				{
					write_FE_element_field_info(output_file, element,
						write_elements_data->field_order_info,
						&(write_elements_data->output_number_of_nodes),
						&(write_elements_data->output_node_indices),
						&(write_elements_data->output_number_of_scale_factors),
						&(write_elements_data->output_scale_factor_indices));
				}
				write_FE_element(output_file, element,
					write_elements_data->field_order_info,
					write_elements_data->output_number_of_nodes,
					write_elements_data->output_node_indices,
					write_elements_data->output_number_of_scale_factors,
					write_elements_data->output_scale_factor_indices);
				write_elements_data->last_element = element;
			}
			else
			{
				return_code = 0;
			}
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
}

static int write_FE_node_field(ostream *output_file,int field_number,
	struct FE_node *node,struct FE_field *field,int *value_index)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

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
	char *component_name;
	enum FE_field_type fe_field_type;
	enum FE_nodal_value_type *nodal_value_types;
	int i,j,number_of_components,number_of_derivatives,number_of_versions,
		return_code;

	ENTER(write_FE_node_field);
	return_code=0;
	if (output_file&&node&&field&&value_index)
	{
		write_FE_field_header(output_file,field_number,field);
		fe_field_type=get_FE_field_FE_field_type(field);
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;i<number_of_components;i++)
		{
			if (NULL != (component_name=get_FE_field_component_name(field,i)))
			{
				(*output_file) << "  "<< component_name << ".";
				DEALLOCATE(component_name);
			}
			else
			{
				(*output_file) << "  "<< i+1 << ".";
			}
			if (GENERAL_FE_FIELD==fe_field_type)
			{
				number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
				number_of_versions=
				get_FE_node_field_component_number_of_versions(node,field,i);
				(*output_file) << "  Value index=" << *value_index << ", #Derivatives=" << number_of_derivatives;
				if (0<number_of_derivatives)
				{
					if (NULL != (nodal_value_types=
						get_FE_node_field_component_nodal_value_types(node,field,i)))
					{
						(*output_file) << " (";
						for (j=1;j<1+number_of_derivatives;j++)
						{
							if (1!=j)
							{
								(*output_file) << ",";
							}
							(*output_file) << ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_types[j]);
						}
						(*output_file) << ")";
						DEALLOCATE(nodal_value_types);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_FE_node_field.  Could not get nodal value types");
					}
				}
				(*output_file) << ", #Versions=" << number_of_versions << "\n";
				(*value_index) += number_of_versions*(1+number_of_derivatives);
			}
			else
			{
				/* constant and indexed fields: 1 version, no derivatives */
				(*output_file) << "\n";
			}
		}
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
	struct FE_field *field,void *write_nodes_data_void)
/*******************************************************************************
LAST MODIFIED : 20 September 1999

DESCRIPTION :
Calls the write_FE_node_field routine for each FE_node_field
==============================================================================*/
{
	int return_code;
	struct Write_FE_node_field_info_sub *write_nodes_data;

	ENTER(write_FE_node_field_info_sub);
	if (NULL != (write_nodes_data=(struct Write_FE_node_field_info_sub *)write_nodes_data_void))
	{
		return_code=write_FE_node_field(write_nodes_data->output_file,
			write_nodes_data->field_number,node,field,&(write_nodes_data->value_index));
		write_nodes_data->field_number++;
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
	ostream *output_file;
	int i,j,k,number_of_components,number_of_derivatives,number_of_values,
		number_of_versions,return_code;
	struct Write_FE_node_field_values *values_data;

	ENTER(write_FE_node_field_values);
	values_data = (struct Write_FE_node_field_values *)values_data_void;
	output_file = values_data->output_file;
	if (node && field && (NULL != values_data) && (NULL != output_file))
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
						(*output_file) << "\n";
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value *value,*values;
					if (get_FE_nodal_field_FE_value_values(field,node,&number_of_values,
							values_data->time, &values))
					{
						value=values;
						for (i=0;i<number_of_components;i++)
						{
							number_of_versions=
								get_FE_node_field_component_number_of_versions(node,field,i);
							number_of_derivatives=
								get_FE_node_field_component_number_of_derivatives(node,field,i);
							for (j=number_of_versions;0<j;j--)
							{
								for (k=0;k<=number_of_derivatives;k++)
								{
									char num_string[100];
									sprintf(num_string, "%"FE_VALUE_STRING, *value);

									(*output_file) << " " << num_string;
									value++;
								}
								(*output_file) << "\n";
							}
						}
						DEALLOCATE(values);
					}
				} break;
				case INT_VALUE:
				{
					int *value,*values;

					if (get_FE_nodal_field_int_values(field,node,&number_of_values,
							values_data->time, &values))
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
									(*output_file) << " " << *value;
									value++;
								}
								(*output_file) << "\n";
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
								(*output_file) << " " << the_string;
								DEALLOCATE(the_string);
							}
							else
							{
								/* empty string */
								(*output_file) << " \"\"";
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_node_field_values.  Could not get string");
						}
						(*output_file) << "\n";
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

static int write_FE_node(ostream *output_file,struct FE_node *node,
	struct FE_field_order_info *field_order_info, FE_value time)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

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
		(*output_file) << " Node: " << get_FE_node_identifier(node) << "\n";
		values_data.output_file = output_file;
		values_data.number_of_values = 0;
		values_data.time = time;
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
			for_each_FE_field_at_node_alphabetical_indexer_priority(write_FE_node_field_values,
				(void *)&values_data,node);
		}
		/* add extra carriage return for not multiple of
			 FE_VALUE_MAX_OUTPUT_COLUMNS values */
		if ((0 < values_data.number_of_values) &&
			((0 >= FE_VALUE_MAX_OUTPUT_COLUMNS) ||
				(0 != (values_data.number_of_values % FE_VALUE_MAX_OUTPUT_COLUMNS))))
		{
			(*output_file) << "\n";
		}
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

static int FE_node_passes_write_criterion(struct FE_node *node,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
Returns true if the <write_criterion> -- some options of which require the
<field_order_info> -- indicates the <node> is to be written.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_node_passes_write_criterion);
	switch (write_criterion)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
			return_code = 1;
		} break;
		case FE_WRITE_WITH_ALL_LISTED_FIELDS:
		{
			if (node && field_order_info && (0 < (number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info))))
			{
				return_code = 1;
				for (i = 0; (i < number_of_fields) && return_code; i++)
				{
					if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
						FE_field_is_defined_at_node(field, node)))
					{
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_passes_write_criterion.  Invalid argument(s)");
				return_code = 0;
			}
		} break;
		case FE_WRITE_WITH_ANY_LISTED_FIELDS:
		{
			if (node && field_order_info && (0 < (number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info))))
			{
				return_code = 0;
				for (i = 0; (i < number_of_fields) && (!return_code); i++)
				{
					if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
						FE_field_is_defined_at_node(field, node))
					{
						return_code = 1;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_passes_write_criterion.  Invalid argument(s)");
				return_code = 0;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_node_passes_write_criterion.  Unknown write_criterion");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* FE_node_passes_write_criterion */

static int FE_nodes_have_same_header(struct FE_node *node_1,
	struct FE_node *node_2, struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if <node_1> and <node_2> can be written to file with the
same fields header. If <field_order_info> is supplied only those fields
listed in it are compared.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_nodes_have_same_header);
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
			"FE_nodes_have_same_header.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_nodes_have_same_header */

static int write_FE_region_node(struct FE_node *node,
	Write_FE_region_node_data *write_nodes_data)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Writes a node to the given file.  If the fields defined at the node are
different from the last node (taking into account whether a selection of fields
has been selected for output) then the header is written out.
==============================================================================*/
{
	ostream *output_file;
	int i, number_of_fields = 0, number_of_fields_in_header, return_code,
		write_field_values;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct Write_FE_node_field_info_sub field_data;

	ENTER(write_FE_region_node);
	if (node && write_nodes_data && (0 != (output_file = write_nodes_data->output_file)))
	{
		return_code = 1;
		field_order_info = write_nodes_data->field_order_info;
		/* write this node? */
		if (FE_node_passes_write_criterion(node, write_nodes_data->write_criterion,
			field_order_info))
		{
			/* need to write new header? */
			if (((struct FE_node *)NULL == write_nodes_data->last_node) ||
				(!FE_nodes_have_same_header(node, write_nodes_data->last_node,
					field_order_info)))
			{
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
				(*output_file) << " #Fields=" << number_of_fields_in_header << "\n";
				field_data.field_number = 1;
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
					for_each_FE_field_at_node_alphabetical_indexer_priority(write_FE_node_field_info_sub,
						&field_data, node);
				}
				if (write_field_values)
				{
					/* write values for constant and indexed fields */
					(*output_file) << " Values :\n";
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
						for_each_FE_field_at_node_alphabetical_indexer_priority(
							write_FE_node_field_FE_field_values, (void *)output_file, node);
					}
				}
			}
			write_FE_node(output_file, node, field_order_info, write_nodes_data->time);
			/* remember the last node to check if header needs to be re-output */
			write_nodes_data->last_node = node;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_region_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int write_cmzn_region_content(ostream *output_file,
	struct cmzn_region *region, cmzn_field_group_id group,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, int *field_names_counter,
	FE_value time,
	enum FE_write_criterion write_criterion)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Writes <base_fe_region> to the <output_file>.
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only element identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all elements in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only elements with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only elements with any listed fields defined.
==============================================================================*/
{
	int return_code;

	ENTER(write_cmzn_region_content);
	if (output_file && region)
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
		FE_region *fe_region = cmzn_region_get_FE_region(region);
		return_code = 1;
		FE_field_order_info *field_order_info = CREATE(FE_field_order_info)();

		if (write_fields_mode != FE_WRITE_NO_FIELDS)
		{
			if (write_fields_mode == FE_WRITE_ALL_FIELDS)
			{
				// get list of all fields in default alphabetical order
				return_code = FE_region_for_each_FE_field(fe_region,
					FE_field_add_to_FE_field_order_info, (void *)field_order_info);
				FE_field_order_info_prioritise_indexer_fields(field_order_info);
			}
			else if ((0 < number_of_field_names) && field_names && field_names_counter)
			{
				for (int i = 0; (i < number_of_field_names) && return_code; i++)
				{
					if (field_names[i])
					{
						struct FE_field *fe_field = FE_region_get_FE_field_from_name(fe_region, field_names[i]);
						if (fe_field)
						{
							++(field_names_counter[i]);
							return_code = add_FE_field_order_info_field(field_order_info, fe_field);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "write_cmzn_region_content.  NULL field name");
						return_code = 0;
					}
				}
			}
			else if (0 < number_of_field_names)
			{
				display_message(ERROR_MESSAGE, "write_cmzn_region_content.  Missing field names array");
				return_code = 0;
			}
		}

		if (return_code)
		{
			if (write_nodes || write_data)
			{
				Write_FE_region_node_data write_nodes_data;
				write_nodes_data.output_file = output_file;
				write_nodes_data.write_criterion = write_criterion;
				write_nodes_data.field_order_info = field_order_info;
				write_nodes_data.last_node = (struct FE_node *)NULL;
				write_nodes_data.time = time;
				if (write_data)
				{
					cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
						CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
					if (group)
					{
						cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
						cmzn_nodeset_destroy(&nodeset);
						nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
						cmzn_field_node_group_destroy(&node_group);
					}
					if (nodeset && (cmzn_nodeset_get_size(nodeset) > 0))
					{
						(*output_file) << " !#nodeset datapoints\n";
						cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
						cmzn_node_id node = 0;
						while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
						{
							if (!write_FE_region_node(node, &write_nodes_data))
							{
								return_code = 0;
								break;
							}
						}
						cmzn_nodeiterator_destroy(&iter);
						cmzn_nodeset_destroy(&nodeset);
					}
				}
				if (write_nodes)
				{
					write_nodes_data.last_node = (struct FE_node *)NULL;
					cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
						CMZN_FIELD_DOMAIN_TYPE_NODES);
					if (group)
					{
						cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
						cmzn_nodeset_destroy(&nodeset);
						nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
						cmzn_field_node_group_destroy(&node_group);
					}
					if (nodeset && (cmzn_nodeset_get_size(nodeset) > 0))
					{
						(*output_file) << " !#nodeset nodes\n";
						cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
						cmzn_node_id node = 0;
						while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
						{
							if (!write_FE_region_node(node, &write_nodes_data))
							{
								return_code = 0;
								break;
							}
						}
						cmzn_nodeiterator_destroy(&iter);
						cmzn_nodeset_destroy(&nodeset);
					}
				}
			}
			if (write_elements)
			{
				Write_FE_region_element_data write_elements_data;
				write_elements_data.output_file = output_file;
				write_elements_data.output_number_of_nodes = 0;
				write_elements_data.output_node_indices = (int *)NULL;
				write_elements_data.output_number_of_scale_factors = 0;
				write_elements_data.output_scale_factor_indices = (int *)NULL;
				write_elements_data.write_criterion = write_criterion;
				write_elements_data.field_order_info = field_order_info;
				write_elements_data.fe_region = fe_region;
				write_elements_data.last_element = (struct FE_element *)NULL;
				write_elements_data.time = time;
				int highest_dimension = FE_region_get_highest_dimension(fe_region);
				if (0 >= highest_dimension)
					highest_dimension = 3;
				/* write 1-D, 2-D then 3-D so lines and faces precede elements */
				for (int dimension = 1; dimension <= highest_dimension; dimension++)
				{
					if ((dimension == 1 && (write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH1D)) ||
						(dimension == 2 && (write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH2D)) ||
						(dimension == 3 && (write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH3D)) ||
						(dimension == highest_dimension &&
							(write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION)))
					{
						cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
						if (group)
						{
							cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, mesh);
							cmzn_mesh_destroy(&mesh);
							mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
							cmzn_field_element_group_destroy(&element_group);
						}
						if (mesh)
						{
							cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(mesh);
							cmzn_element_id element = 0;
							while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
							{
								if (!write_FE_region_element(element, &write_elements_data))
								{
									return_code = 0;
									break;
								}
							}
							cmzn_elementiterator_destroy(&iter);
							cmzn_mesh_destroy(&mesh);
						}
					}
				}
				DEALLOCATE(write_elements_data.output_node_indices);
				DEALLOCATE(write_elements_data.output_scale_factor_indices);
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "write_cmzn_region_content.  Failed");
			return_code = 0;
		}
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		cmzn_fieldmodule_destroy(&field_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_cmzn_region_content.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Recursively writes the finite element fields in region tree to file.
 * Notes:
 * - the master region for a group must be the parent region.
 * - element_xi values currently restricted to being in the root_region.
 * 
 * @param output_file  The file to write region and field data to.
 * @param region  The region to write.
 * @param parent_region_output  Pointer to parent region if just output to same
 *   file. Caller should set to NULL on first call.
 * @param root_region  The root region of any data to be written. Need not be
 *   the true root of region hierarchy, but region paths in file are relative to
 *   this region.
 * @param write_elements  If set, write elements and element fields to file.
 * @param write_nodes  If set, write nodes and node fields to file.
 * @param write_data  If set, write data and data fields to file. May only use
 *   if write_elements and write_nodes are 0.
 * @param write_fields_mode  Controls which fields are written to file.
 *   If mode is FE_WRITE_LISTED_FIELDS then:
 *   - Number/list of field_names must be supplied;
 *   - Field names not used in a region are ignored;
 *   - Warnings are given for any field names not used in any output region.
 * @param number_of_field_names  The number of names in the field_names array.
 * @param field_names  Array of field names.
 * @param field_names_counter  Array of integers of same length as field_names
 *   incremented whenever the respective field name is matched.
 * @param write_criterion  Controls which objects are written. Some modes
 *   limit output to nodes or objects with any or all listed fields defined.
 * @param write_recursion  Controls whether sub-regions and sub-groups are
 *   recursively written.
 */
static int write_cmzn_region(ostream *output_file,
	struct cmzn_region *region, const char * group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, int *field_names_counter,
	FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode)
{
	int return_code;

	ENTER(write_cmzn_region);
	if (output_file && region && root_region)
	{
		return_code = 1;

		// write region path and/or group name */
		cmzn_field_group_id group = 0;
		if (group_name)
		{
			cmzn_fieldmodule_id fieldmodule =  cmzn_region_get_fieldmodule(region);
			cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, group_name);
			if (field)
			{
				group = cmzn_field_cast_group(field);
				cmzn_field_destroy(&field);
			}
			cmzn_fieldmodule_destroy(&fieldmodule);
		}

		if (!group_name || group)
		{
			if (!group || (region != root_region))
			{
				char *region_path = cmzn_region_get_relative_path(region, root_region);
				int len = strlen(region_path);
				if ((1 < len) && (region_path[len - 1] == CMZN_REGION_PATH_SEPARATOR_CHAR))
				{
					region_path[len - 1] = '\0';
				}
				(*output_file) << "Region: " << region_path << "\n";
				DEALLOCATE(region_path);
			}
			if (group)
			{
				char *group_name = cmzn_field_get_name(cmzn_field_group_base_cast(group));
				(*output_file) << " Group name: " << group_name << "\n";
				DEALLOCATE(group_name);
			}

			// write finite element fields for this region
			if (return_code)
			{
				return_code = write_cmzn_region_content(output_file, region, group,
					write_elements, write_nodes, write_data,
					write_fields_mode, number_of_field_names, field_names,
					field_names_counter, time, write_criterion);
			}
		}

		if (return_code && !group_name && recursion_mode == CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON)
		{
			// write group members
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_fielditerator_id field_iter = cmzn_fieldmodule_create_fielditerator(field_module);
			cmzn_field_id field = 0;
			while ((0 != (field = cmzn_fielditerator_next_non_access(field_iter))) && return_code)
			{
				cmzn_field_group_id output_group = cmzn_field_cast_group(field);
				if (output_group)
				{
					char *group_name = cmzn_field_get_name(field);
					(*output_file) << " Group name: " << group_name << "\n";
					DEALLOCATE(group_name);
					return_code = write_cmzn_region_content(output_file, region, output_group,
						write_elements, write_nodes, write_data,
						FE_WRITE_NO_FIELDS, number_of_field_names, field_names,
						field_names_counter, time, write_criterion);
					cmzn_field_group_destroy(&output_group);
				}
			}
			cmzn_fielditerator_destroy(&field_iter);
			cmzn_fieldmodule_destroy(&field_module);
		}

		if (recursion_mode == CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON)
		{
			// write child regions
			cmzn_region *child_region = cmzn_region_get_first_child(region);
			while (child_region)
			{
				return_code = write_cmzn_region(output_file,
					child_region, group_name, root_region,
					write_elements, write_nodes, write_data,
					write_fields_mode, number_of_field_names, field_names,
					field_names_counter, time, write_criterion, recursion_mode);
				if (!return_code)
				{
					cmzn_region_destroy(&child_region);
					break;
				}
				cmzn_region_reaccess_next_sibling(&child_region);
			}
		}
		if (group)
		{
			cmzn_field_group_destroy(&group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_cmzn_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_cmzn_region */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(FE_write_criterion)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(FE_write_criterion));
	switch (enumerator_value)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
			enumerator_string = "complete_group";
		} break;
		case FE_WRITE_WITH_ALL_LISTED_FIELDS:
		{
			enumerator_string = "with_all_listed_fields";
		} break;
		case FE_WRITE_WITH_ANY_LISTED_FIELDS:
		{
			enumerator_string = "with_any_listed_fields";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(FE_write_criterion) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(FE_write_criterion)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(FE_write_recursion)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(FE_write_recursion));
	switch (enumerator_value)
	{
		case FE_WRITE_RECURSIVE:
		{
			enumerator_string = "recursive";
		} break;
		case FE_WRITE_RECURSE_SUBGROUPS:
		{
			enumerator_string = "recurse_subgroups";
		} break;
		case FE_WRITE_NON_RECURSIVE:
		{
			enumerator_string = "non_recursive";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(FE_write_recursion) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(FE_write_recursion)

/***************************************************************************//**
 * Writes an EX file with supplied root_region at the top level of the file.
 *
 * @param output_file  The file to write region and field data to.
 * @param region  The region to output.
 * @param group  Optional subgroup to output.
 * @param root_region  The region which will become the root region in the EX
 *   file. Need not be the true root of region hierarchy, but must contain
 *   <region>.
 * @param write_elements  If set, write elements and element fields to file.
 * @param write_nodes  If set, write nodes and node fields to file.
 * @param write_data  If set, write data and data fields to file. May only use
 *   if write_elements and write_nodes are 0.
 * @param write_fields_mode  Controls which fields are written to file.
 *   If mode is FE_WRITE_LISTED_FIELDS then:
 *   - Number/list of field_names must be supplied;
 *   - Field names not used in a region are ignored;
 *   - Warnings are given for any field names not used in any output region.
 * @param number_of_field_names  The number of names in the field_names array.
 * @param field_names  Array of field names.
 * @param time  Field values at <time> will be written out if field is time
 *    dependent. If fields are time dependent but <time> is out of range then
 *    the values at nearest time will be written out. If fields are not time
 *    dependent, this parameter is ignored.
 * @param write_criterion  Controls which objects are written. Some modes
 *   limit output to nodes or objects with any or all listed fields defined.
 * @param write_recursion  Controls whether sub-regions and sub-groups are
 *   recursively written.
 */
int write_exregion_to_stream(ostream *output_file,
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode)
{
	int return_code;

	ENTER(write_exregion_to_stream);
	if (output_file && region && root_region &&
		((write_data || write_elements || write_nodes)) &&
		((write_fields_mode != FE_WRITE_LISTED_FIELDS) ||
			((0 < number_of_field_names) && field_names)))
	{
		if (cmzn_region_contains_subregion(root_region, region))
		{
			int *field_names_counter = NULL;
			if (0 < number_of_field_names)
			{
				/* count number of times each field name is matched for later warning */
				if (ALLOCATE(field_names_counter, int, number_of_field_names))
				{
					for (int i = 0; i < number_of_field_names; i++)
					{
						field_names_counter[i] = 0;
					}
				}
			}
			return_code = write_cmzn_region(output_file,
				region, group_name, root_region,
				write_elements, write_nodes, write_data,
				write_fields_mode, number_of_field_names, field_names, field_names_counter,
				time, write_criterion, recursion_mode);
			if (field_names_counter)
			{
				if (write_fields_mode == FE_WRITE_LISTED_FIELDS)
				{
					for (int i = 0; i < number_of_field_names; i++)
					{
						if (field_names_counter[i] == 0)
						{
							display_message(WARNING_MESSAGE,
								"No field named '%s' found in any region written to EX file",
								field_names[i]);
						}
					}
				}
				DEALLOCATE(field_names_counter);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"write_exregion_to_stream.  Error writing region");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_exregion_to_stream.  Region is not within root region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_to_stream.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_exregion_to_stream */

int write_exregion_file_of_name(const char *file_name,
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode)
{
	int return_code;

	if (file_name)
	{
		ofstream output_file;
		output_file.open(file_name, ios::out);
		if (output_file.is_open())
		{
			return_code = write_exregion_to_stream(&output_file, region, group_name, root_region,
				write_elements, write_nodes, write_data,
				write_fields_mode, number_of_field_names, field_names, time,
				write_criterion, recursion_mode);
			output_file.close();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open for writing exregion file: %s", file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_file_of_name.  Invalid arguments");
		return_code = 0;
	}

	return (return_code);
} /* write_exregion_file_of_name */

int write_exregion_file_to_memory_block(
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region, int write_elements,
	int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode,
	void **memory_block, unsigned int *memory_block_length)
{
	int return_code;

	ENTER(write_exregion_file_of_name);
	if (memory_block)
	{
		ostringstream stringStream;
		if (stringStream)
		{
			return_code = write_exregion_to_stream(&stringStream, region, group_name, root_region,
				write_elements, write_nodes, write_data,
				write_fields_mode, number_of_field_names, field_names, time,
				write_criterion, recursion_mode);
			string sstring = stringStream.str();
			*memory_block_length = sstring.size();
			*memory_block = duplicate_string(sstring.c_str());
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open for writing exregion into memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_file_of_name.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

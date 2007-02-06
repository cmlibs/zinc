/*******************************************************************************
FILE : export_finite_element.c

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
#include "finite_element/export_finite_element.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "region/cmiss_region_write_info.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* the number of spaces each child object is indented from its parent by in the
	 output file */
#define EXPORT_INDENT_SPACES 2

/*
Module types
------------
*/

#if defined (OLD_CODE)
struct Fwrite_FE_element_group_data
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct GROUP(FE_element) *element_group;
}; /* struct File_write_FE_element_group_data */

struct Fwrite_all_FE_element_groups_data
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
}; /* struct File_write_FE_element_group_data */

struct Fwrite_FE_node_group_data
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct GROUP(FE_node) *node_group;
}; /* struct Fwrite_FE_node_group_data */

struct Fwrite_all_FE_node_groups_data
/*******************************************************************************
LAST MODIFIED : 5 September 2001

DESCRIPTION :
==============================================================================*/
{
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
}; /* struct Fwrite_all_FE_node_groups_data */
#endif /* defined (OLD_CODE) */

struct Write_FE_region_element_data
{
	FILE *output_file;
	int dimension, output_number_of_nodes, *output_node_indices,
		output_number_of_scale_factors, *output_scale_factor_indices;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct FE_element *last_element;
	struct FE_region *fe_region;
}; /* struct Write_FE_region_element_data */

#if defined (OLD_CODE)
struct File_write_FE_region_sub
{
	FILE *output_file;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
}; /* struct File_write_FE_region_sub */
#endif /* defined (OLD_CODE) */

struct Write_FE_node_field_values
{
	FILE *output_file;
	/* store number of values for writing nodal values in columns */
	int number_of_values;
};

struct Write_FE_node_field_info_sub
{
	/* field_number and value_index are incremented by write_FE_node_field so
		 single and multiple field output can be handled appropriately. Both must be
		 initialised to 1 before the first time write_FE_node_field is called */
	int field_number,value_index;
	FILE *output_file;
}; /* Write_FE_node_field_info_sub */

struct Write_FE_region_node_data
{
	FILE *output_file;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct FE_node *last_node;
}; /* struct Write_FE_region_node_data */

#if defined (OLD_CODE)
struct File_write_FE_node_group_sub
{
	FILE *output_file;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
}; /* struct File_write_FE_node_group_sub */
#endif /* defined (OLD_CODE) */

/*
Module functions
----------------
*/

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

static int write_FE_field_header(FILE *output_file,int field_number,
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
		fprintf(output_file," %i) ",field_number);
		/* write the field name */
		if (GET_NAME(FE_field)(field,&name))
		{
			fprintf(output_file,name);
			DEALLOCATE(name);
		}
		else
		{
			fprintf(output_file,"unknown");
		}
		fprintf(output_file,", %s",
			ENUMERATOR_STRING(CM_field_type)(get_FE_field_CM_field_type(field)));
		/* optional constant/indexed, Index_field=~, #Values=# */
		fe_field_type=get_FE_field_FE_field_type(field);
		switch (fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				fprintf(output_file,", constant");
			} break;
			case GENERAL_FE_FIELD:
			{
				/* default; nothing to write */
			} break;
			case INDEXED_FE_FIELD:
			{
				struct FE_field *indexer_field;
				int number_of_indexed_values;

				fprintf(output_file,", indexed, Index_field=");
				if (get_FE_field_type_indexed(field,&indexer_field,
					&number_of_indexed_values))
				{
					if (GET_NAME(FE_field)(indexer_field,&name))
					{
						fprintf(output_file,name);
						DEALLOCATE(name);
					}
					else
					{
						fprintf(output_file,"unknown");
					}
					fprintf(output_file,", #Values=%d",number_of_indexed_values);
				}
				else
				{
					fprintf(output_file,"unknown, #Values=0");
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
		if (coordinate_system=get_FE_field_coordinate_system(field))
		{
			switch (coordinate_system->type)
			{
				case CYLINDRICAL_POLAR:
				{
					fprintf(output_file,", cylindrical polar");
				} break;
				case FIBRE:
				{
					fprintf(output_file,", fibre");
				} break;
				case OBLATE_SPHEROIDAL:
				{
					fprintf(output_file,", oblate spheroidal, focus=%"FE_VALUE_STRING,
						coordinate_system->parameters.focus);
				} break;
				case PROLATE_SPHEROIDAL:
				{
					fprintf(output_file,", prolate spheroidal, focus=%"FE_VALUE_STRING,
						coordinate_system->parameters.focus);
				} break;
				case RECTANGULAR_CARTESIAN:
				{
					fprintf(output_file,", rectangular cartesian");
				} break;
				case SPHERICAL_POLAR:
				{
					fprintf(output_file,", spherical polar");
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
			fprintf(output_file,", %s",Value_type_string(value_type));
		}
		number_of_components=get_FE_field_number_of_components(field);
		fprintf(output_file,", #Components=%d\n",number_of_components);
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
	return_code=write_FE_field_values((FILE *)output_file_void,field);
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
	return_code=write_FE_field_values((FILE *)output_file_void,field);
	LEAVE;

	return (return_code);
} /* write_FE_node_field_FE_field_values */

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
		fprintf(output_file, " Shape. Dimension=%d, ", dimension);
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
		fprintf(output_file,"\n");
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
				fprintf(output_file, "%d 0 0", cm.number);
			} break;
			case CM_FACE:
			{
				fprintf(output_file, "0 %d 0", cm.number);
			} break;
			case CM_LINE:
			{
				fprintf(output_file, "0 0 %d", cm.number);
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

static int write_FE_basis(FILE *output_file,struct FE_basis *basis)
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Writes out the <basis> to <output_file>.
???RC Currently limited to handling one polygon or one simplex. Will have to
be rewritten for 4-D and above elements.
==============================================================================*/
{
	char *basis_type_string;
	enum FE_basis_type basis_type;
	int dimension, linked_dimensions, next_xi_number, number_of_polygon_vertices,
		return_code, xi_number;

	ENTER(write_FE_basis);
	if (output_file && basis && FE_basis_get_dimension(basis, &dimension))
	{
		return_code = 1;
		linked_dimensions = 0;
		for (xi_number = 0; (xi_number < dimension) && return_code; xi_number++)
		{
			if (FE_basis_get_xi_basis_type(basis, xi_number, &basis_type))
			{
				if (basis_type_string = FE_basis_type_string(basis_type))
				{
					fprintf(output_file, basis_type_string);
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
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_basis.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_basis */

struct Write_FE_element_field_sub
{
	FILE *output_file;
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
	FILE *output_file;
	int i, j, k, nodal_value_index, node_index, number_in_xi,
		number_of_components, number_of_nodal_values,
		number_of_nodes, number_of_xi_coordinates, return_code, scale_factor_index;
	struct FE_basis *basis;
	struct FE_element_field_component *component;
	struct Standard_node_to_element_map *standard_node_map;
	struct Write_FE_element_field_sub *write_element_field_data;

	ENTER(write_FE_element_field_sub);
	if (element && field && (write_element_field_data =
		(struct Write_FE_element_field_sub *)write_element_field_data_void) &&
		(output_file = write_element_field_data->output_file) &&
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
			if (component_name = get_FE_field_component_name(field, i))
			{
				fprintf(output_file, " %s. ", component_name);
				DEALLOCATE(component_name);
			}
			else
			{
				fprintf(output_file, "  %d.", i + 1);
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
						fprintf(output_file,", no modify");
					}
					else if (modify == theta_increasing_in_xi1)
					{
						fprintf(output_file,", increasing in xi1");
					}
					else if (modify == theta_decreasing_in_xi1)
					{
						fprintf(output_file,", decreasing in xi1");
					}
					else if (modify == theta_non_increasing_in_xi1)
					{
						fprintf(output_file,", non-increasing in xi1");
					}
					else if (modify == theta_non_decreasing_in_xi1)
					{
						fprintf(output_file,", non-decreasing in xi1");
					}
					else
					{
						fprintf(output_file,", unknown modify function");
						display_message(ERROR_MESSAGE,
							"write_FE_element_field.  Unknown modify function");
					}
					if (FE_element_field_component_get_type(component, &component_type))
					{
						switch (component_type)
						{
							case STANDARD_NODE_TO_ELEMENT_MAP:
							{
								fprintf(output_file,", standard node based.\n");
								if (FE_element_field_component_get_number_of_nodes(component,
									&number_of_nodes))
								{
									fprintf(output_file,"   #Nodes=%d\n",number_of_nodes);
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
											fprintf(output_file,"   %d. #Values=%d\n",
												write_element_field_data->
												output_node_indices[node_index] + 1,
												number_of_nodal_values);
											/* nodal value indices are all relative so output as is */
											fprintf(output_file,"     Value indices:");
											for (k = 0; k < number_of_nodal_values; k++)
											{
												Standard_node_to_element_map_get_nodal_value_index(
													standard_node_map, k, &nodal_value_index);
												fprintf(output_file, " %d", nodal_value_index + 1);
											}
											fprintf(output_file, "\n");
											/* scale factor indices are renumbered */
											fprintf(output_file,"     Scale factor indices:");
											for (k = 0; k < number_of_nodal_values; k++)
											{
												Standard_node_to_element_map_get_scale_factor_index(
													standard_node_map, k, &scale_factor_index);
												if (0 <= scale_factor_index)
												{
													fprintf(output_file, " %d", write_element_field_data->
														output_scale_factor_indices[scale_factor_index]+1);
												}
												else
												{
													/* non-positive index means no scale factor so value
														 of 1.0 assumed */
													fprintf(output_file," 0");
												}
											}
											fprintf(output_file,"\n");
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
							case GENERAL_NODE_TO_ELEMENT_MAP:
							{
								fprintf(output_file,", general node based.\n");
								display_message(ERROR_MESSAGE,"write_FE_element_field_sub.  "
									"general node based map not supported");
							} break;
							case FIELD_TO_ELEMENT_MAP:
							{
								fprintf(output_file,", field to element.\n");
								display_message(ERROR_MESSAGE,
									"write_FE_element_field.  field to element map not supported");
							} break;
							case ELEMENT_GRID_MAP:
							{
								fprintf(output_file,", grid based.\n");
								FE_basis_get_dimension(basis, &number_of_xi_coordinates);
								fprintf(output_file, " ");
								for (j = 0; j < number_of_xi_coordinates; j++)
								{
									if (0 < j)
									{
										fprintf(output_file, ", ");
									}
									FE_element_field_component_get_grid_map_number_in_xi(
										component, j, &number_in_xi);
									fprintf(output_file, "#xi%d=%d", j + 1, number_in_xi);
								}
								fprintf(output_file, "\n");
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"write_FE_element_field_sub.  Unknown field component type");
								fprintf(output_file, "\n");
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
				fprintf(output_file,"\n");
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

static int write_FE_element_field_info(FILE *output_file,
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
	int field_no, i, j, node_index,number_of_components, number_of_fields,
		number_of_fields_in_header, number_of_nodes, number_of_nodes_in_component,
		number_of_scale_factor_sets, number_of_scale_factors,
		numbers_in_scale_factor_set,
		output_number_of_scale_factor_sets, output_scale_factor_index, return_code,
		scale_factor_index, *scale_factor_set_in_use, *temp_indices,
		write_field_values;
	struct FE_basis *basis;
	struct FE_element_field_component *component;
	struct FE_field *field;
	struct General_node_to_element_map *general_node_map;
	struct Standard_node_to_element_map *standard_node_map;
	struct Write_FE_element_field_sub write_element_field_data;
	void *scale_factor_set_identifier;

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
							if ((field =
								get_FE_field_order_info_field(field_order_info, field_no)) &&
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
																		"write_FE_element_field_info.  "
																		"Invalid node_index for "
																		"standard node to element map");
																	return_code=0;
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"write_FE_element_field_info.  "
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
																		"write_FE_element_field_info.  "
																		"Invalid node_index for "
																		"general node to element map");
																	return_code=0;
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"write_FE_element_field_info.  "
																	"Missing general node to element map");
																return_code=0;
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"write_FE_element_field_info.  "
															"Invalid general node to element component");
														return_code=0;
													}
												} break;
											}
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
					fprintf(output_file, " #Scale factor sets=%d\n",
						output_number_of_scale_factor_sets);
					*output_number_of_scale_factors = 0;
					output_scale_factor_index=scale_factor_index = 0;
					for (j = 0; j < number_of_scale_factor_sets; j++)
					{
						get_FE_element_numbers_in_scale_factor_set(element, j,
							&numbers_in_scale_factor_set);
						if (scale_factor_set_in_use[j])
						{
							fprintf(output_file," ");
							get_FE_element_scale_factor_set_identifier(element, j,
								&scale_factor_set_identifier);
							write_FE_basis(output_file,
								(struct FE_basis *)scale_factor_set_identifier);
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
					/* output number of nodes */
					fprintf(output_file," #Nodes=%d\n",*output_number_of_nodes);
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
					
					fprintf(output_file, " #Fields=%d\n", number_of_fields_in_header);
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
						for_each_FE_field_at_element_indexer_first(
							write_FE_element_field_sub,(void *)&write_element_field_data,
							element);
					}

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
									write_FE_element_field_FE_field_values(element, field,
										(void *)output_file);
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
			fprintf(output_file, " #Scale factor sets=0\n");
			fprintf(output_file, " #Nodes=0\n");
			fprintf(output_file, " #Fields=0\n");
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

static int write_FE_element(FILE *output_file,struct FE_element *element,
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
	int field_no, first_grid_field, i, number_of_faces, number_of_fields,
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
		fprintf(output_file, " Element: ");
		write_FE_element_identifier(output_file, element);
		fprintf(output_file,"\n");
		if (get_FE_element_number_of_faces(element, &number_of_faces) &&
			(0 < number_of_faces))
		{
			/* write the faces */
			fprintf(output_file," Faces:\n");
			for (i = 0; i < number_of_faces; i++)
			{
				fprintf(output_file," ");
				if (get_FE_element_face(element, i, &face) && face)
				{
					write_FE_element_identifier(output_file, face);
				}
				else
				{
					/* no face = no number, for compatibility with read_FE_element */
					fprintf(output_file,"0 0 0");
				}
				fprintf(output_file,"\n");
			}
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
					write_FE_element_field_values(element, field, (void *)output_file);
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
		if (0 < output_number_of_nodes)
		{
			/* write the nodes */
			if (get_FE_element_number_of_nodes(element, &number_of_nodes))
			{
				if (0 < number_of_nodes)
				{
					fprintf(output_file, " Nodes:\n");
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
					fprintf(output_file," Scale factors:\n");
					number_of_scale_factors=0;
					/* output in columns if FE_VALUE_MAX_OUTPUT_COLUMNS > 0 */
					for (i = 0; i < total_number_of_scale_factors; i++)
					{
						if (0 <= output_scale_factor_indices[i])
						{
							number_of_scale_factors++;
							get_FE_element_scale_factor(element, i, &scale_factor);
							fprintf(output_file, " %"FE_VALUE_STRING, scale_factor);
							if ((0<FE_VALUE_MAX_OUTPUT_COLUMNS)&&
								(0==(number_of_scale_factors%FE_VALUE_MAX_OUTPUT_COLUMNS)))
							{
								fprintf(output_file,"\n");
							}
						}
					}
					/* add extra carriage return for not multiple of
						FE_VALUE_MAX_OUTPUT_COLUMNS values */
					if ((0 >= FE_VALUE_MAX_OUTPUT_COLUMNS) ||
						(0 != (number_of_scale_factors % FE_VALUE_MAX_OUTPUT_COLUMNS)))
					{
						fprintf(output_file,"\n");
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
						FE_region_element_or_parent_has_field(fe_region, element, field)))
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
						FE_region_element_or_parent_has_field(fe_region, element, field))
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

static int FE_elements_have_same_header(
	struct FE_element *element_1, struct FE_element *element_2,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if <element_1> and <element_2> can be written to file with the
same field header. If <field_order_info> is supplied only those fields
listed in it are compared.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_elements_have_same_header);
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
			"FE_elements_have_same_header.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_elements_have_same_header */

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
	int new_field_header, new_shape, return_code;
	struct FE_element_shape *element_shape, *last_element_shape;
	struct Write_FE_region_element_data *write_data;

	ENTER(write_FE_region_element);
	if (element &&
		(write_data = (struct Write_FE_region_element_data *)write_data_void) &&
			(output_file = write_data->output_file))
	{
		return_code = 1;
		/* only output the element if it has the specified dimension and fields
			 appropriate to the write_criterion */
		if ((write_data->dimension == get_FE_element_dimension(element)) &&
			FE_element_passes_write_criterion(element, write_data->fe_region,
				write_data->write_criterion, write_data->field_order_info))
		{
			/* work out if shape or field header have changed from last element */
			if (get_FE_element_shape(element, &element_shape))
			{
				new_shape = 1;
				new_field_header = 1;
				if (write_data->last_element)
				{
					if (get_FE_element_shape(write_data->last_element,
						&last_element_shape))
					{
						new_shape = (element_shape != last_element_shape);
					}
					new_field_header = !FE_elements_have_same_header(element,
						write_data->last_element, write_data->field_order_info);
				}
				if (new_shape)
				{
					write_FE_element_shape(output_file, element_shape);
				}
				if (new_field_header)
				{
					write_FE_element_field_info(output_file, element,
						write_data->field_order_info,
						&(write_data->output_number_of_nodes),
						&(write_data->output_node_indices),
						&(write_data->output_number_of_scale_factors),
						&(write_data->output_scale_factor_indices));
				}
				write_FE_element(output_file, element,
					write_data->field_order_info,
					write_data->output_number_of_nodes,
					write_data->output_node_indices,
					write_data->output_number_of_scale_factors,
					write_data->output_scale_factor_indices);
				write_data->last_element = element;
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
} /* write_FE_region_element */

#if defined (OLD_CODE)
static int file_write_FE_region_sub(
	struct FE_region *fe_region, void *data_void)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Writes an element group to a file.
==============================================================================*/
{
	int return_code;
	struct File_write_FE_region_sub *data;

	ENTER(file_write_FE_region_sub);
	data = (struct File_write_FE_region_sub *)data_void;
	return_code = write_FE_region(data->output_file, fe_region,
		data->write_criterion, data->field_order_info);
	LEAVE;

	return (return_code);
} /* file_write_FE_region_sub */
#endif /* defined (OLD_CODE) */

static int write_FE_node_field(FILE *output_file,int field_number,
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
			if (component_name=get_FE_field_component_name(field,i))
			{
				fprintf(output_file,"  %s.",component_name);
				DEALLOCATE(component_name);
			}
			else
			{
				fprintf(output_file,"  %d.",i+1);
			}
			if (GENERAL_FE_FIELD==fe_field_type)
			{
				number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
				number_of_versions=
				get_FE_node_field_component_number_of_versions(node,field,i);
				fprintf(output_file,"  Value index=%d, #Derivatives=%d",*value_index,
					number_of_derivatives);
				if (0<number_of_derivatives)
				{
					if (nodal_value_types=
						get_FE_node_field_component_nodal_value_types(node,field,i))
					{
						fprintf(output_file," (");
						for (j=1;j<1+number_of_derivatives;j++)
						{
							if (1!=j)
							{
								fprintf(output_file,",");
							}
							switch (nodal_value_types[j])
							{
								case FE_NODAL_UNKNOWN:
								{
									fprintf(output_file,"unknown");
								} break;
								case FE_NODAL_D_DS1:
								{
									fprintf(output_file,"d/ds1");
								} break;
								case FE_NODAL_D_DS2:
								{
									fprintf(output_file,"d/ds2");
								} break;
								case FE_NODAL_D_DS3:
								{
									fprintf(output_file,"d/ds3");
								} break;
								case FE_NODAL_D2_DS1DS2:
								{
									fprintf(output_file,"d2/ds1ds2");
								} break;
								case FE_NODAL_D2_DS1DS3:
								{
									fprintf(output_file,"d2/ds1ds3");
								} break;
								case FE_NODAL_D2_DS2DS3:
								{
									fprintf(output_file,"d2/ds2ds3");
								} break;
								case FE_NODAL_D3_DS1DS2DS3:
								{
									fprintf(output_file,"d3/ds1ds2ds3");
								} break;
							}
						}
						fprintf(output_file,")");
						DEALLOCATE(nodal_value_types);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_FE_node_field.  Could not get nodal value types");
					}
				}
				fprintf(output_file,", #Versions=%d\n",number_of_versions);
				(*value_index) += number_of_versions*(1+number_of_derivatives);
			}
			else
			{
				/* constant and indexed fields: 1 version, no derivatives */
				fprintf(output_file,"\n");
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
		return_code=write_FE_node_field(write_data->output_file,
			write_data->field_number,node,field,&(write_data->value_index));
		write_data->field_number++;
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

static int write_FE_node(FILE *output_file,struct FE_node *node,
	struct FE_field_order_info *field_order_info)
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
		fprintf(output_file, " Node: %d\n", get_FE_node_identifier(node));
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

static int write_FE_region_node(struct FE_node *node, void *user_data)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Writes a node to the given file.  If the fields defined at the node are
different from the last node (taking into account whether a selection of fields
has been selected for output) then the header is written out.
==============================================================================*/
{
	FILE *output_file;
	int i, number_of_fields, number_of_fields_in_header, return_code,
		write_field_values;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct Write_FE_region_node_data *write_data;
	struct Write_FE_node_field_info_sub field_data;

	ENTER(write_FE_region_node);
	if (node && (write_data = (struct Write_FE_region_node_data *)user_data) &&
		(output_file = write_data->output_file))
	{
		return_code = 1;
		field_order_info = write_data->field_order_info;
		/* write this node? */
		if (FE_node_passes_write_criterion(node, write_data->write_criterion,
			field_order_info))
		{
			/* need to write new header? */
			if (((struct FE_node *)NULL == write_data->last_node) ||
				(!FE_nodes_have_same_header(node, write_data->last_node,
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
				fprintf(output_file, " #Fields=%d\n", number_of_fields_in_header);
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
					for_each_FE_field_at_node_indexer_first(write_FE_node_field_info_sub,
						&field_data, node);
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
			}
			write_FE_node(output_file, node, field_order_info);
			/* remember the last node to check if header needs to be re-output */
			write_data->last_node = node;
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
} /* write_FE_region_node */

#if defined (OLD_CODE)
static int file_write_FE_node_group_sub(struct GROUP(FE_node) *node_group,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
Writes a node group to a file.
==============================================================================*/
{
	int return_code;
	struct File_write_FE_node_group_sub *data;

	ENTER(file_write_FE_node_group_sub);
	data = (struct File_write_FE_node_group_sub *)data_void;
	return_code = write_FE_node_group(data->output_file, node_group,
		data->write_criterion, data->field_order_info);
	LEAVE;

	return (return_code);
} /* file_write_FE_node_group_sub */
#endif /* defined (OLD_CODE) */

static int write_FE_region(FILE *output_file, struct FE_region *fe_region,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Writes <fe_region> to the <output_file>.
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
	int dimension, return_code;
	struct Write_FE_region_element_data write_elements_data;
	struct Write_FE_region_node_data write_nodes_data;

	ENTER(write_FE_region);
	if (output_file && fe_region)
	{
		return_code = 1;
		if (write_nodes)
		{
			write_nodes_data.output_file = output_file;
			write_nodes_data.write_criterion = write_criterion;
			write_nodes_data.field_order_info = field_order_info;
			write_nodes_data.last_node = (struct FE_node *)NULL;
			return_code = FE_region_for_each_FE_node(fe_region,
				write_FE_region_node, &write_nodes_data);
		}
		if (write_elements)
		{
			write_elements_data.output_file = output_file;
			write_elements_data.output_number_of_nodes = 0;
			write_elements_data.output_node_indices = (int *)NULL;
			write_elements_data.output_number_of_scale_factors = 0;
			write_elements_data.output_scale_factor_indices = (int *)NULL;
			write_elements_data.write_criterion = write_criterion;
			write_elements_data.field_order_info = field_order_info;
			write_elements_data.fe_region = fe_region;
			write_elements_data.last_element = (struct FE_element *)NULL;
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
			DEALLOCATE(write_elements_data.output_node_indices);
			DEALLOCATE(write_elements_data.output_scale_factor_indices);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "write_FE_region.  Failed");
			return_code = 0;
		}
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
	char *write_path, struct Cmiss_region *write_region, int indent_level,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info,
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
	char *child_path, *child_region_name, *first_path, *master_path;
	enum Cmiss_region_write_status master_write_status, write_status;
	enum FE_write_criterion use_write_criterion;
	int i, j, master_region_not_in_file, number_of_children, return_code;
	struct Cmiss_region *child_region, *master_region, *write_path_region;
	struct FE_region *fe_region, *master_fe_region;

	ENTER(write_Cmiss_region);
	if (output_file && region && root_region &&
		((write_path && (!write_region)) || ((!write_path) && write_region)) &&
		(0 <= indent_level) && write_info_list && path)
	{
		return_code = 1;
		if (write_path)
		{
			/* if <write_path> is "" or "/", the following clears it to NULL and
				 sets write_region to region */
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
		}
		if (return_code)
		{
			if ((fe_region = Cmiss_region_get_FE_region(region)) &&
				(return_code = FE_region_get_immediate_master_FE_region(fe_region,
					&master_fe_region)))
			{
				use_write_criterion = write_criterion;
				if (master_fe_region && (!force_no_master_region))
				{
					use_write_criterion = FE_WRITE_NO_FIELDS;
				}
				for (j = 0; j < indent_level; j++)
				{
					fputc(' ', output_file);
				}
				fprintf(output_file, "<exelem>");
				return_code = write_FE_region(output_file, fe_region,
					write_elements, write_nodes, use_write_criterion, field_order_info);
				for (j = 0; j < indent_level; j++)
				{
					fputc(' ', output_file);
				}
				fprintf(output_file, "</exelem>");
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"write_Cmiss_region.  Error writing finite element region");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (return_code =
				Cmiss_region_get_number_of_child_regions(region, &number_of_children))
			{
				for (i = 0; (i < number_of_children) && return_code; i++)
				{
					if (return_code =
						Cmiss_region_get_child_region(region, i, &child_region) &&
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
										(return_code = FE_region_get_immediate_master_FE_region(
											fe_region, &master_fe_region)) && master_fe_region &&
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

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(FE_write_criterion)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(FE_write_criterion));
	switch (enumerator_value)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
			enumerator_string = "complete_group";
		} break;
		case FE_WRITE_NO_FIELDS:
		{
			enumerator_string = "no_fields";
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
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(FE_write_criterion) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(FE_write_criterion)

int write_exregion_file(FILE *output_file,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Writes an exregion file to <output_file> with <root_region> at the top level of
the file. If <write_path> is supplied, only its contents are written, except
that all paths from <root_region> to it are complete in the file.
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
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all object in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only objects with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only objects with any listed fields defined.
==============================================================================*/
{
	char *child_region_name;
	int child_number_of_children, i, number_of_children, region_format,
		return_code;
	struct Cmiss_region *child_region, *write_region;
	struct FE_region *fe_region;
	struct LIST(Cmiss_region_write_info) *write_info_list;

	ENTER(write_exregion_file);
	write_region = (struct Cmiss_region *)NULL;
	if (output_file && root_region && 
		Cmiss_region_get_number_of_child_regions(root_region, &number_of_children)
		&& ((!write_path) || (Cmiss_region_get_region_from_path(root_region,
			write_path, &write_region) && write_region)))
	{
		return_code = 1;
		/* determine if the region hierarchy can be written in the old format
			 with one flat list of regions stemming from the root_region and with
			 the root_region owning all fields, nodes and elements */
		if (0 < number_of_children)
		{
			region_format = 0;
			for (i = 0; (i < number_of_children) && (!region_format) && return_code;
				i++)
			{
				if ((return_code =
					Cmiss_region_get_child_region(root_region, i, &child_region)) &&
					((!write_region) || (write_region == child_region)) &&
					(return_code = Cmiss_region_get_number_of_child_regions(child_region,
						&child_number_of_children)))
				{
					if (0 < child_number_of_children)
					{
						region_format = 1;
					}
				}
			}
		}
		else
		{
			/* cannot output a region with itself as root in old_format */
			region_format = 1;
		}
		if (return_code)
		{
			if (region_format)
			{
				write_info_list = CREATE(LIST(Cmiss_region_write_info))();
				fprintf(output_file, "<regionml>\n");
				return_code = write_Cmiss_region(output_file,
					root_region, root_region, /*no_master*/1, write_path,
					/*write_region*/(struct Cmiss_region *)NULL, EXPORT_INDENT_SPACES,
					write_elements, write_nodes, write_criterion, field_order_info,
					write_info_list, /*path*/"");
				fprintf(output_file, "</regionml>\n");
				DESTROY(LIST(Cmiss_region_write_info))(&write_info_list);
			}
			else
			{
				for (i = 0; (i < number_of_children) && return_code; i++)
				{
					if ((return_code = Cmiss_region_get_child_region(root_region, i,
						&child_region)) &&
						((!write_region) || (write_region == root_region) ||
							(child_region == write_region)) &&
						(return_code = Cmiss_region_get_child_region_name(root_region,
							i, &child_region_name)))
					{
						fprintf(output_file, " Group name: %s\n", child_region_name);
						if (fe_region = Cmiss_region_get_FE_region(child_region))
						{
							return_code = write_FE_region(output_file, fe_region,
								write_elements, write_nodes, write_criterion, field_order_info);
						}
						DEALLOCATE(child_region_name);
					}
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"write_exregion_file.  Error writing region");
		}
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

int write_exregion_file_of_name(char *file_name,
	struct Cmiss_region *root_region, char *write_path,
	int write_elements, int write_nodes, enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 18 March 2003

DESCRIPTION :
Opens <file_name> for writing, calls write_exregion_file and then closes the file.
==============================================================================*/
{
	FILE *output_file;
	int return_code;

	ENTER(write_exregion_file_of_name);
	if (file_name)
	{
		/* open the input file */
		if (output_file = fopen(file_name, "w"))
		{
			return_code = write_exregion_file(output_file,
				root_region, write_path, write_elements, write_nodes,
				write_criterion, field_order_info);
			fclose(output_file);
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
	LEAVE;

	return (return_code);
} /* write_exregion_file_of_name */

#if defined (OLD_CODE)
int write_FE_element_group(FILE *output_file,
	struct GROUP(FE_element) *element_group,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Writes an element group to an <output_file> or the socket (if <output_file> is
NULL).
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
	char *group_name;
	int dimension, return_code;
	struct Write_FE_element_group_sub write_data;

	ENTER(write_FE_element_group);
	if (output_file && element_group)
	{
		if (return_code = GET_NAME(GROUP(FE_element))(element_group, &group_name))
		{
			fprintf(output_file, " Group name: %s\n", group_name);
			write_data.output_file = output_file;
			write_data.output_number_of_nodes = 0;
			write_data.output_node_indices = (int *)NULL;
			write_data.output_number_of_scale_factors = 0;
			write_data.output_scale_factor_indices = (int *)NULL;
			write_data.write_criterion = write_criterion;
			write_data.field_order_info = field_order_info;
			write_data.element_group = element_group;
			write_data.last_element = (struct FE_element *)NULL;
			/* write 1-D, 2-D then 3-D so lines and faces precede elements */
			for (dimension = 1; dimension <= 3; dimension++)
			{
				write_data.dimension = dimension;
				if (!FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					write_FE_element_group_sub, &write_data, element_group))
				{
					return_code = 0;
				}
			}
			DEALLOCATE(write_data.output_node_indices);
			DEALLOCATE(write_data.output_scale_factor_indices);
			DEALLOCATE(group_name);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "write_FE_element_group.  Failed");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_element_group */

int file_write_FE_element_group(char *file_name, void *data_void)
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Writes an element group to a file.
<data_void> should point to a struct Fwrite_FE_element_group_data.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct Fwrite_FE_element_group_data *data;
	struct GROUP(FE_element) *element_group;

	ENTER(file_write_FE_element_group);
	if (file_name && (data = (struct Fwrite_FE_element_group_data *)data_void) &&
		(element_group = data->element_group))
	{
		/* open the input file */
		if (output_file = fopen(file_name,"w"))
		{
			return_code = write_FE_element_group(output_file, element_group,
				data->write_criterion, data->field_order_info);
			fclose(output_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not create element group file: %s", file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_write_FE_element_group.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* file_write_FE_element_group */

int file_write_all_FE_element_groups(char *file_name,void *data_void)
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Writes all existing element groups to a file.
<data_void> should point to a struct Fwrite_all_FE_element_groups_data.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct File_write_FE_element_group_sub data_sub;
	struct Fwrite_all_FE_element_groups_data *data;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;

	ENTER(file_write_all_FE_element_groups);
	if (file_name && (data =
		(struct Fwrite_all_FE_element_groups_data *)data_void) &&
		(element_group_manager = data->element_group_manager))
	{
		/* open the input file */
		if (output_file = fopen(file_name,"w"))
		{
			data_sub.output_file = output_file;
			data_sub.write_criterion = data->write_criterion;
			data_sub.field_order_info = data->field_order_info;
			return_code = FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_element))(
				file_write_FE_element_group_sub,
				(void *)&data_sub, element_group_manager);
			fclose(output_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open element group file: %s", file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_write_all_FE_element_groups.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* file_write_all_FE_element_groups */

int write_FE_node_group(FILE *output_file, struct GROUP(FE_node) *node_group,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
Writes a node group to an <output_file>.
If <field_order_info> is NULL, all node fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only node identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all nodes in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only nodes with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only nodes with any listed fields defined.
==============================================================================*/
{
	char *group_name;
	int return_code;
	struct Write_FE_node_group_sub temp_data;

	ENTER(write_FE_node_group);
	if (output_file && node_group)
	{
		if (return_code = GET_NAME(GROUP(FE_node))(node_group, &group_name))
		{
			/* file output */
			fprintf(output_file, " Group name: %s\n", group_name);
			temp_data.output_file = output_file;
			temp_data.write_criterion = write_criterion;
			temp_data.field_order_info = field_order_info;
			temp_data.last_node = (struct FE_node *)NULL;
			return_code = FOR_EACH_OBJECT_IN_GROUP(FE_node)(write_FE_node_group_sub,
				&temp_data, node_group);
			DEALLOCATE(group_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_FE_node_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_group */

int file_write_FE_node_group(char *file_name, void *data_void)
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
Writes a node group to a file.
<data_void> should point to a struct Fwrite_FE_node_group_data.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct Fwrite_FE_node_group_data *data;
	struct GROUP(FE_node) *node_group;

	ENTER(file_write_FE_node_group);
	if (file_name && (data = (struct Fwrite_FE_node_group_data *)data_void) &&
		(node_group = data->node_group))
	{
		/* open the input file */
		if (output_file = fopen(file_name,"w"))
		{
			return_code = write_FE_node_group(output_file, node_group,
				data->write_criterion, data->field_order_info);
			fclose(output_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not create node group file: %s",file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_write_FE_node_group.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* file_write_FE_node_group */

int file_write_all_FE_node_groups(char *file_name, void *data_void)
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Writes all existing node groups to a file.
<data_void> should point to a struct Fwrite_all_FE_node_groups_data.
==============================================================================*/
{
	FILE *output_file;
	int return_code;
	struct File_write_FE_node_group_sub data_sub;
	struct Fwrite_all_FE_node_groups_data *data;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;

	ENTER(file_write_all_FE_node_groups);
	if (file_name && (data = (struct Fwrite_all_FE_node_groups_data *)data_void)
		&& (node_group_manager = data->node_group_manager))
	{
		/* open the input file */
		if (output_file = fopen(file_name,"w"))
		{
			data_sub.output_file = output_file;
			data_sub.write_criterion = data->write_criterion;
			data_sub.field_order_info = data->field_order_info;
			return_code = FOR_EACH_OBJECT_IN_MANAGER(GROUP(FE_node))(
				file_write_FE_node_group_sub, (void *)&data_sub, node_group_manager);
			fclose(output_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open node group file: %s",file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_write_all_FE_node_groups.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_write_all_FE_node_groups */

#endif /* defined (OLD_CODE) */

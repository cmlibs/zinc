/*******************************************************************************
FILE : finite_element_to_graphics_object.c

LAST MODIFIED : 14 March 2003

DESCRIPTION :
The functions for creating graphical objects from finite elements.
==============================================================================*/
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_adjacent_elements.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/geometry.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/random.h"
#include "general/statistics.h"
#include "graphics/graphics_object.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"
#include "graphics/mcubes.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* following used for encoding a CM_element_information as an integer */
#define HALF_INT_MAX (INT_MAX/2)

/*
Module types
------------
*/

struct Computed_fields_of_node
/*******************************************************************************
LAST MODIFIED : 14 May 1999

DESCRIPTION :
For checking fields of a node.
???GMH. To generalise, we should add general component checking.
==============================================================================*/
{
	int num_required_fields,number_of_nodes,num_coordinate,num_field;
	struct Computed_field **required_fields;
	struct FE_node *field_node;
}; /* struct Computed_fields_of_node */

struct Node_to_glyph_set_data
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Used with iterators for building glyph sets from nodes.
==============================================================================*/
{
	char **label;
	FE_value base_size[3], centre[3], scale_factors[3], time;
	GTDATA *data;
	int n_data_components, *name;
	struct Computed_field *coordinate_field, *data_field, *label_field,
		*orientation_scale_field, *variable_scale_field;
	Triple *point, *axis1, *axis2, *axis3, *scale;
}; /* struct Node_to_glyph_set_data */

/*
Module functions
----------------
*/

struct Computed_fields_of_node *CREATE(Computed_fields_of_node)(void)
/*******************************************************************************
LAST MODIFIED : 14 May 1999

DESCRIPTION :
Initialises the fields.  Does not allocate memory for components of the
structure - assumes they will be statically allocated by the calling procedure.
==============================================================================*/
{
	struct Computed_fields_of_node *computed_fields_of_node;

	ENTER(CREATE(Computed_fields_of_node));
	if (ALLOCATE(computed_fields_of_node,struct Computed_fields_of_node,1))
	{
		computed_fields_of_node->num_required_fields=0;
		computed_fields_of_node->number_of_nodes=0;	
		computed_fields_of_node->num_field=0;
		computed_fields_of_node->num_coordinate=0;
		computed_fields_of_node->required_fields=(struct Computed_field **)NULL;
		computed_fields_of_node->field_node=(struct FE_node *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_fields_of_node).  Could not allocate structure");
		computed_fields_of_node=(struct Computed_fields_of_node *)NULL;
	}
	LEAVE;

	return (computed_fields_of_node);
} /* CREATE(Computed_fields_of_node) */

int DESTROY(Computed_fields_of_node)(struct Computed_fields_of_node **computed_fields_of_node)
/*******************************************************************************
LAST MODIFIED : 14 May 1999

DESCRIPTION :
Destroys the structure.  DOES NOT DESTROY THE FIELDS!!!
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_fields_of_node));
	/* check the arguments */
	if (computed_fields_of_node)
	{
		DEALLOCATE(*computed_fields_of_node);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_fields_of_node).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_fields_of_node) */

static int FE_node_computed_fields_defined(struct FE_node *node,int num_of_fields,
	struct Computed_field **current_field_address)
/*******************************************************************************
LAST MODIFIED : 17 May 1999

DESCRIPTION :
function for checking that a node has the computed fields in current_field_address 
defined.
==============================================================================*/
{
	int fields_all_defined,i;
	struct Computed_field *current_field;

	ENTER(FE_node_computed_fields_defined);
	/* check arguments */	
	if(node&&current_field_address)
	{
		fields_all_defined=1;
		i=num_of_fields;

		while (fields_all_defined&&(i>0))
		{
			if (current_field= *current_field_address)
			{
				if(!Computed_field_is_defined_at_node(current_field,node))
				{	
					fields_all_defined=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"FE_node_computed_fields_defined"
					" Current_field is NULL  ");
				fields_all_defined=0;
			}		
			i--;
			current_field_address++;
		}
	}
	else
	{		
		display_message(ERROR_MESSAGE,"FE_node_computed_fields_defined."
			" Invalid arguments");
		fields_all_defined=0;
	}
	LEAVE;
	return (fields_all_defined);
} /* FE_node_computed_fields_defined*/

static int FE_node_count_if_computed_fields_defined(struct FE_node *node,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 17 May 1999

DESCRIPTION :
Iterator function for counting how many of the supplied computed fields are defined 
and can be evaluated at a node. Count returned in 
computed_fields_of_node->number_of_nodes++.
==============================================================================*/
{
	int computed_fields_all_defined,return_code;
	struct Computed_field **current_field_address;
	struct Computed_fields_of_node *computed_fields_of_node;

	ENTER(FE_node_count_if_computed_fields_defined);
	/* check arguments */
	if (node&&(computed_fields_of_node=(struct Computed_fields_of_node *)user_data))
	{
		return_code=1;
		/* a match here means that fields_of_node->field_node has already been
			 checked */
		if (!equivalent_computed_fields_at_nodes(node,computed_fields_of_node->field_node))
		{
			/* check for specific fields */
			current_field_address=computed_fields_of_node->required_fields;

			computed_fields_all_defined =FE_node_computed_fields_defined(node,
				computed_fields_of_node->num_required_fields,current_field_address);
		}
		else
		{
			computed_fields_all_defined =1;
		}
		if (computed_fields_all_defined)
		{
			computed_fields_of_node->number_of_nodes++;
			/* remember this info for the next one */
			computed_fields_of_node->field_node=node;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_count_if_computed_fields_defined */

static int node_to_glyph_set(struct FE_node *node,
	void *node_to_glyph_set_data_void)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Iterator function for adding a glyph showing a node and fields to a glyph_set.
Up to calling function to call Computed_field_clear_cache for any computed
fields used here.
==============================================================================*/
{
	FE_value a[3], b[3], c[3], coordinates[3], orientation_scale[9], size[3],
		time, variable_scale[3];
	struct Computed_field **current_field_address, *coordinate_field, *data_field,
		*label_field, *orientation_scale_field, *required_fields[4],
		*variable_scale_field;
	int j, number_of_fields, number_of_orientation_scale_components,
		number_of_variable_scale_components, return_code;
	struct Node_to_glyph_set_data *node_to_glyph_set_data;
	Triple *axis1, *axis2, *axis3, *point, *scale;

	ENTER(node_to_glyph_set);
	/* must set following to 0 = valid if no orientation_scale_field */
	number_of_orientation_scale_components = 0;
	number_of_variable_scale_components = 0;
	if (node && (node_to_glyph_set_data =
		(struct Node_to_glyph_set_data *)node_to_glyph_set_data_void) &&
		(coordinate_field=node_to_glyph_set_data->coordinate_field) &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)) && 
		((!(orientation_scale_field =
			node_to_glyph_set_data->orientation_scale_field)) ||
			(9 >= (number_of_orientation_scale_components =
				Computed_field_get_number_of_components(orientation_scale_field)))) &&
		((!(data_field = node_to_glyph_set_data->data_field)) ||
			node_to_glyph_set_data->data) &&
		((!(label_field = node_to_glyph_set_data->label_field)) ||
			node_to_glyph_set_data->label) &&
		((!(variable_scale_field = node_to_glyph_set_data->variable_scale_field)) ||
			(3 >= (number_of_variable_scale_components =
				Computed_field_get_number_of_components(variable_scale_field)))) &&
		(point = node_to_glyph_set_data->point) &&
		(axis1 = node_to_glyph_set_data->axis1) &&
		(axis2 = node_to_glyph_set_data->axis2) &&
		(axis3 = node_to_glyph_set_data->axis3) &&
		(scale = node_to_glyph_set_data->scale))
	{
		return_code=1;
		/* prepare fields for check with FE_node_computed_fields_defined */
		number_of_fields = 0;	
		if (coordinate_field)
		{ 
			required_fields[number_of_fields]=coordinate_field;
			number_of_fields++;			
		}	
		if (data_field)
		{ 
			required_fields[number_of_fields]=data_field;
			number_of_fields++;			
		}			
		if (label_field)
		{ 
			required_fields[number_of_fields]=label_field;
			number_of_fields++;			
		}			
		if (orientation_scale_field)
		{ 
			required_fields[number_of_fields]=orientation_scale_field;
			number_of_fields++;			
		}	
		if (variable_scale_field)
		{ 
			required_fields[number_of_fields]=variable_scale_field;
			number_of_fields++;			
		}
		time = node_to_glyph_set_data->time;
		current_field_address = required_fields;		
		/* if there's no node coordinate field, do nothing, but no error */
		if (FE_node_computed_fields_defined(node,number_of_fields,
			current_field_address)) 
		{
			/* clear coordinates in case coordinate field is not 3 component */
			coordinates[0] = 0.0;
			coordinates[1] = 0.0;
			coordinates[2] = 0.0;
			/* evaluate the fields at the node */
			if (Computed_field_evaluate_at_node(coordinate_field,node,time,
				coordinates)&&
		      ((!orientation_scale_field) || Computed_field_evaluate_at_node(
					orientation_scale_field,node,time,orientation_scale))&&
				((!variable_scale_field) || Computed_field_evaluate_at_node(
					variable_scale_field, node, time, variable_scale)) &&
				((!data_field)||Computed_field_evaluate_at_node(data_field,node,
					time, node_to_glyph_set_data->data))&&
				((!label_field)||(*(node_to_glyph_set_data->label) =
					Computed_field_evaluate_as_string_at_node(label_field,
					/*component_number*/-1, node, time)))&&
				make_glyph_orientation_scale_axes(
					number_of_orientation_scale_components, orientation_scale,
					a, b, c, size))
			{
				for (j = 0; j < 3; j++)
				{
					(*scale)[j] = node_to_glyph_set_data->base_size[j] +
						size[j]*node_to_glyph_set_data->scale_factors[j];
				}
				for (j = 0; j < number_of_variable_scale_components; j++)
				{
					(*scale)[j] *= variable_scale[j];
				}
				for (j = 0; j < 3; j++)
				{
					(*point)[j] = coordinates[j] -
						node_to_glyph_set_data->centre[0]*(*scale)[0]*a[j] -
						node_to_glyph_set_data->centre[1]*(*scale)[1]*b[j] -
						node_to_glyph_set_data->centre[2]*(*scale)[2]*c[j];
					(*axis1)[j] = a[j];
					(*axis2)[j] = b[j];
					(*axis3)[j] = c[j];
				}
				(node_to_glyph_set_data->point)++;
				(node_to_glyph_set_data->axis1)++;
				(node_to_glyph_set_data->axis2)++;
				(node_to_glyph_set_data->axis3)++;
				(node_to_glyph_set_data->scale)++;

				if (node_to_glyph_set_data->data_field)
				{
					node_to_glyph_set_data->data +=
						node_to_glyph_set_data->n_data_components;
				}
				if (node_to_glyph_set_data->name)
				{
					*(node_to_glyph_set_data->name) = get_FE_node_identifier(node);
					(node_to_glyph_set_data->name)++;
				}
				if (node_to_glyph_set_data->label_field)
				{
					(node_to_glyph_set_data->label)++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"node_to_glyph_set.  Error calculating node fields");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_to_glyph_set.  Invalid argument(s)");
		return_code=0;
	}
	
	LEAVE;

	return (return_code);
} /* node_to_glyph_set */

static int fill_table(struct FE_element **element_block,int *adjacency_table,
	struct FE_element *element,int i,int j,int k,int n_xi[3])
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Recursive routine used to fill a volume with elements which may not adjacent,
but are indirectly connected (e.g. mesh with slit)
*******************************************************************************/
{
	int number_of_elements, return_code;
	struct CM_element_information cm;
	struct FE_element **elements, *element_ptr;

	ENTER(fill_table);
	/* check arguments */
	if (element_block&&adjacency_table&&n_xi)
	{
		return_code=1;
		/* if already visited, skip */
		if (!(element_block[k*n_xi[0]*n_xi[1]+j*n_xi[0]+i])&&(i<n_xi[0])&&
			(j<n_xi[1])&&(k<n_xi[2]))
		{
			/* add element to block */
			element_block[k*n_xi[0]*n_xi[1]+j*n_xi[0]+i]=element;
			/* +ve xi1 direction */
			if (adjacent_FE_element(element,1,&number_of_elements,&elements))
			{
				/* Just use the first one */
				element_ptr=elements[0];
				get_FE_element_identifier(element_ptr, &cm);
				adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+1] = cm.number;
				fill_table(element_block,adjacency_table,element_ptr,i+1,j,k,n_xi);
				DEALLOCATE(elements);
			}
			else
			{
				adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+1]=0;
			}
			/* +ve xi2 direction */
			if (adjacent_FE_element(element,3,&number_of_elements,&elements))
			{
				/* Just use the first one */
				element_ptr=elements[0];
				get_FE_element_identifier(element_ptr, &cm);
				adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+3] = cm.number;
				fill_table(element_block,adjacency_table,element_ptr,i,j+1,k,n_xi);
				DEALLOCATE(elements);
			}
			else
			{
				adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+3]=0;
			}
			/* +ve xi3 direction */
			if (adjacent_FE_element(element,5,&number_of_elements,&elements))
			{
				/* Just use the first one */
				element_ptr=elements[0];
				get_FE_element_identifier(element_ptr, &cm);
				adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+5] = cm.number;
				fill_table(element_block,adjacency_table,element_ptr,i,j,k+1,n_xi);
				DEALLOCATE(elements);
			}
			else
			{
				adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+5]=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"fill_table.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fill_table */

/*
Global functions
----------------
*/

int make_glyph_orientation_scale_axes(
	int number_of_orientation_scale_values, FE_value *orientation_scale_values,
	FE_value *axis1,FE_value *axis2, FE_value *axis3, FE_value *size)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Computes the three glyph orientation axes from the <orientation_scale_values>.

The orientation is understood from the number_of_orientation_scale_values as:
0 = zero scalar (no vector/default orientation);
1 = scalar (no vector/default orientation);
2 = 1 2-D vector (2nd glyph axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd glyph axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd glyph axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd glyph axis found from cross product);
9 = 3 3-D vectors = complete definition of glyph axes;

The scaling behaviour depends on the number of vectors interpreted above, where:
0 = isotropic scaling on all three axes by scalar;
1 = isotropic scaling on all three axes by magnitude of vector;
2 = scaling in direction of 2 vectors, ie. they keep their current length, unit
    vector in 3rd axis;
3 = scaling in direction of 3 vectors - ie. they keep their current length.

Function returns the axes as unit vectors with their magnitudes in the <size>
array. This is always possible if there is a scalar (or zero scalar), but where
zero vectors are either read or calculated from the <orientation_scale_values>,
these are simply returned, since no valid direction can be produced.
==============================================================================*/
{
	float magnitude;
	int return_code;

	ENTER(make_glyph_orientation_scale_axes);
	if (((0 == number_of_orientation_scale_values) ||
		((0<number_of_orientation_scale_values) && orientation_scale_values)) &&
		axis1 && axis2 && axis3 && size)
	{
		return_code=1;
		switch (number_of_orientation_scale_values)
		{
			case 0:
			{
				/* zero scalar; axes = x,y,z */
				size[0]=size[1]=size[2]=0.0;
				axis1[0]=1.0;
				axis1[1]=0.0;
				axis1[2]=0.0;
				axis2[0]=0.0;
				axis2[1]=1.0;
				axis2[2]=0.0;
				axis3[0]=0.0;
				axis3[1]=0.0;
				axis3[2]=1.0;
			} break;
			case 1:
			{
				/* scalar; axes = x,y,z */
				size[0]=size[1]=size[2]=orientation_scale_values[0];
				axis1[0]=1.0;
				axis1[1]=0.0;
				axis1[2]=0.0;
				axis2[0]=0.0;
				axis2[1]=1.0;
				axis2[2]=0.0;
				axis3[0]=0.0;
				axis3[1]=0.0;
				axis3[2]=1.0;
			} break;
			case 2:
			{
				/* 1 2-D vector */
				axis1[0]=orientation_scale_values[0];
				axis1[1]=orientation_scale_values[1];
				axis1[2]=0.0;
				if (0.0<(magnitude=sqrt(axis1[0]*axis1[0]+axis1[1]*axis1[1])))
				{
					axis1[0] /= magnitude;
					axis1[1] /= magnitude;
				}
				size[0]=size[1]=size[2]=magnitude;
				/* get axis2 orthogonal to axis 1 in x-y plane (in right hand sense) */
				axis2[0]=-axis1[1];
				axis2[1]=axis1[0];
				axis2[2]=0.0;
				/* axis3 is along the z-axis, of same length as other axes */
				axis3[0]=0.0;
				axis3[1]=0.0;
				axis3[2]=1.0;
			} break;
			case 3:
			{
				/* 1 3-D vector */
				axis1[0]=orientation_scale_values[0];
				axis1[1]=orientation_scale_values[1];
				axis1[2]=orientation_scale_values[2];
				/* get magnitude of axis1 vector to make axis2 and axis3 this size */
				if (0.0<(magnitude=
					sqrt(axis1[0]*axis1[0]+axis1[1]*axis1[1]+axis1[2]*axis1[2])))
				{
					axis1[0] /= magnitude;
					axis1[1] /= magnitude;
					axis1[2] /= magnitude;
					size[0]=size[1]=size[2]=magnitude;
					/* get axis3, non-colinear with axis1 */
					axis3[0]=0.0;
					axis3[1]=0.0;
					axis3[2]=0.0;
					if (fabs(axis1[0]) < fabs(axis1[1]))
					{
						if (fabs(axis1[2]) < fabs(axis1[0]))
						{
							axis3[2]=1.0;
						}
						else
						{
							axis3[0]=1.0;
						}
					}
					else
					{
						if (fabs(axis1[2]) < fabs(axis1[1]))
						{
							axis3[2]=1.0;
						}
						else
						{
							axis3[1]=1.0;
						}
					}
					/* get axis2 = axis3 (x) axis1 = vector orthogonal to axis1 */
					axis2[0]=axis3[1]*axis1[2]-axis3[2]*axis1[1];
					axis2[1]=axis3[2]*axis1[0]-axis3[0]*axis1[2];
					axis2[2]=axis3[0]*axis1[1]-axis3[1]*axis1[0];
					/* make axis2 unit length */
					magnitude=sqrt(axis2[0]*axis2[0]+axis2[1]*axis2[1]+axis2[2]*axis2[2]);
					axis2[0] /= magnitude;
					axis2[1] /= magnitude;
					axis2[2] /= magnitude;
					/* get axis3 = axis1 (x) axis2 = unit vector */
					axis3[0]=axis1[1]*axis2[2]-axis1[2]*axis2[1];
					axis3[1]=axis1[2]*axis2[0]-axis1[0]*axis2[2];
					axis3[2]=axis1[0]*axis2[1]-axis1[1]*axis2[0];
				}
				else
				{
					/* magnitude of axis1 zero, so clear axis2 and axis3 */
					axis2[0]=0.0;
					axis2[1]=0.0;
					axis2[2]=0.0;
					axis3[0]=0.0;
					axis3[1]=0.0;
					axis3[2]=0.0;
					size[0]=size[1]=size[2]=0.0;
				}
			} break;
			case 4:
			{
				/* 2 2-D vectors */
				/* axis 1 */
				axis1[0]=orientation_scale_values[0];
				axis1[1]=orientation_scale_values[1];
				axis1[2]=0.0;
				if (0.0<(magnitude=sqrt(axis1[0]*axis1[0]+axis1[1]*axis1[1])))
				{
					axis1[0] /= magnitude;
					axis1[1] /= magnitude;
				}
				size[0]=magnitude;
				/* axis 2 */
				axis2[0]=orientation_scale_values[2];
				axis2[1]=orientation_scale_values[3];
				axis2[2]=0.0;
				if (0.0<(magnitude=sqrt(axis2[0]*axis2[0]+axis2[1]*axis2[1])))
				{
					axis2[0] /= magnitude;
					axis2[1] /= magnitude;
				}
				size[1]=magnitude;
				/* axis3 is a unit vector along the z-axis with zero size */
				axis3[0]=0.0;
				axis3[1]=0.0;
				axis3[2]=1.0;
				size[2]=0.0;
			} break;
			case 6:
			{
				/* 2 3-D vectors */
				/* axis 1 */
				axis1[0]=orientation_scale_values[0];
				axis1[1]=orientation_scale_values[1];
				axis1[2]=orientation_scale_values[2];
				if (0.0<(magnitude=
					sqrt(axis1[0]*axis1[0]+axis1[1]*axis1[1]+axis1[2]*axis1[2])))
				{
					axis1[0] /= magnitude;
					axis1[1] /= magnitude;
					axis1[2] /= magnitude;
				}
				size[0]=magnitude;
				/* axis 2 */
				axis2[0]=orientation_scale_values[3];
				axis2[1]=orientation_scale_values[4];
				axis2[2]=orientation_scale_values[5];
				if (0.0<(magnitude=
					sqrt(axis2[0]*axis2[0]+axis2[1]*axis2[1]+axis2[2]*axis2[2])))
				{
					axis2[0] /= magnitude;
					axis2[1] /= magnitude;
					axis2[2] /= magnitude;
				}
				size[1]=magnitude;
				/* get axis3 = axis1 (x) axis2 */
				axis3[0]=axis1[1]*axis2[2]-axis1[2]*axis2[1];
				axis3[1]=axis1[2]*axis2[0]-axis1[0]*axis2[2];
				axis3[2]=axis1[0]*axis2[1]-axis1[1]*axis2[0];
				/* make axis3 a unit vector, with zero size */
				if (0.0<(magnitude=
					sqrt(axis3[0]*axis3[0]+axis3[1]*axis3[1]+axis3[2]*axis3[2])))
				{
					axis3[0] /= magnitude;
					axis3[1] /= magnitude;
					axis3[2] /= magnitude;
				}
				size[2]=0.0;
			} break;
			case 9:
			{
				/* 3 3-D vectors */
				/* axis 1 */
				axis1[0]=orientation_scale_values[0];
				axis1[1]=orientation_scale_values[1];
				axis1[2]=orientation_scale_values[2];
				if (0.0<(magnitude=
					sqrt(axis1[0]*axis1[0]+axis1[1]*axis1[1]+axis1[2]*axis1[2])))
				{
					axis1[0] /= magnitude;
					axis1[1] /= magnitude;
					axis1[2] /= magnitude;
				}
				size[0]=magnitude;
				/* axis 2 */
				axis2[0]=orientation_scale_values[3];
				axis2[1]=orientation_scale_values[4];
				axis2[2]=orientation_scale_values[5];
				if (0.0<(magnitude=
					sqrt(axis2[0]*axis2[0]+axis2[1]*axis2[1]+axis2[2]*axis2[2])))
				{
					axis2[0] /= magnitude;
					axis2[1] /= magnitude;
					axis2[2] /= magnitude;
				}
				size[1]=magnitude;
				/* axis 3 */
				axis3[0]=orientation_scale_values[6];
				axis3[1]=orientation_scale_values[7];
				axis3[2]=orientation_scale_values[8];
				if (0.0<(magnitude=
					sqrt(axis3[0]*axis3[0]+axis3[1]*axis3[1]+axis3[2]*axis3[2])))
				{
					axis3[0] /= magnitude;
					axis3[1] /= magnitude;
					axis3[2] /= magnitude;
				}
				size[2]=magnitude;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"make_glyph_orientation_scale_axes.  "
					"Invalid number_of_orientation_scale_values");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_orientation_scale_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_glyph_orientation_scale_axes */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Use_element_type)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Use_element_type));
	switch (enumerator_value)
	{
		case USE_ELEMENTS:
		{
			enumerator_string = "use_elements";
		} break;
		case USE_FACES:
		{
			enumerator_string = "use_faces";
		} break;
		case USE_LINES:
		{
			enumerator_string = "use_lines";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Use_element_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Use_element_type)

enum CM_element_type Use_element_type_CM_element_type(
	enum Use_element_type use_element_type)
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Returns the CM_element_type expected for the <use_element_type>. Note that a
match is found if either the dimension or the CM_element_type matches the
element.
==============================================================================*/
{
	enum CM_element_type cm_element_type;

	ENTER(Use_element_type_CM_element_type);
	switch (use_element_type)
	{
		case USE_ELEMENTS:
		{
			cm_element_type=CM_ELEMENT;
		} break;
		case USE_FACES:
		{
			cm_element_type=CM_FACE;
		} break;
		case USE_LINES:
		{
			cm_element_type=CM_LINE;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Use_element_type_CM_element_type.  Unknown use_element_type");
			cm_element_type=CM_ELEMENT;
		} break;
	}
	LEAVE;

	return (cm_element_type);
} /* Use_element_type_CM_element_type */

int Use_element_type_dimension(enum Use_element_type use_element_type)
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Returns the dimension expected for the <use_element_type>. Note that a match is
found if either the dimension or the CM_element_type matches the element.
==============================================================================*/
{
	int dimension;

	ENTER(Use_element_type_dimension);
	switch (use_element_type)
	{
		case USE_ELEMENTS:
		{
			dimension=3;
		} break;
		case USE_FACES:
		{
			dimension=2;
		} break;
		case USE_LINES:
		{
			dimension=1;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Use_element_type_dimension.  Unknown use_element_type");
			dimension=0;
		} break;
	}
	LEAVE;

	return (dimension);
} /* Use_element_type_dimension */

struct FE_element *FE_region_get_element_with_Use_element_type(
	struct FE_region *fe_region, enum Use_element_type use_element_type,
	int element_number)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Because USE_FACES can refer to either a 2-D CM_FACE or a 2-D CM_ELEMENT, and
USE_LINES can refer to a 1-D CM_LINE or a 1-D CM_ELEMENT, this function handles
the logic for getting the most appropriate element from <fe_region> with
the given the <use_element_type> and <element_number>.
==============================================================================*/
{
	struct CM_element_information element_identifier;
	struct FE_element *element;

	ENTER(FE_region_get_element_with_Use_element_type);
	if (fe_region)
	{
		element_identifier.type =
			Use_element_type_CM_element_type(use_element_type);
		element_identifier.number = element_number;
		if (!(element = FE_region_get_FE_element_from_identifier(fe_region,
			&element_identifier)))
		{
			if (USE_ELEMENTS != use_element_type)
			{
				element_identifier.type = CM_ELEMENT;
				if (element = FE_region_get_FE_element_from_identifier(fe_region,
					&element_identifier))
				{
					if (get_FE_element_dimension(element) !=
						Use_element_type_dimension(use_element_type))
					{
						element = (struct FE_element *)NULL;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_element_with_Use_element_type.  "
			"Invalid argument(s)");
		element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
} /* FE_region_get_element_with_Use_element_type */

int CM_element_information_to_graphics_name(struct CM_element_information *cm)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Encodes <cm> as a single integer that can be converted back to the element with
CM_element_information_from_graphics_name. cm->number must be non-negative,
and for CM_ELEMENT and CM_FACE must be less than INT_MAX/2, since they share the
positive integers. Note: keeps element numbers contiguous for a CM_type.
Used for selection and highlighting of elements.
==============================================================================*/
{
	int graphics_name;

	ENTER(CM_element_information_to_graphics_name);
	if (cm)
	{
		switch (cm->type)
		{
			case CM_ELEMENT:
			{
				if ((0 <= cm->number)&&(cm->number <= HALF_INT_MAX))
				{
					graphics_name = cm->number;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CM_element_information_to_graphics_name.  "
						"CM_ELEMENT number must be from 0 to %d",HALF_INT_MAX);
					graphics_name = 0;
				}
			} break;
			case CM_FACE:
			{
				if ((0 <= cm->number)&&(cm->number <= HALF_INT_MAX))
				{
					graphics_name = HALF_INT_MAX + 1 + cm->number;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CM_element_information_to_graphics_name.  "
						"CM_FACE number must be from 0 to %d",HALF_INT_MAX);
					graphics_name = HALF_INT_MAX + 1;
				}
			} break;
			case CM_LINE:
			{
				/* allow twice as many line numbers as tend to have more of them */
				if ((0 <= cm->number)&&(cm->number <= INT_MAX))
				{
					graphics_name = INT_MIN + cm->number;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CM_element_information_to_graphics_name.  "
						"CM_LINE number must be from 0 to %d",INT_MAX);
					graphics_name = INT_MIN;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"CM_element_information_to_graphics_name.  Invalid CM_type");
				graphics_name = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CM_element_information_to_graphics_name.  Invalid argument(s)");
		graphics_name = 0;
	}
	LEAVE;

	return (graphics_name);
} /* CM_element_information_to_graphics_name */

int CM_element_information_from_graphics_name(struct CM_element_information *cm,
	int graphics_name)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Fills <cm> with the CM_type and number determined from the the integer
<graphics_name>, encoded with CM_element_information_to_graphics_name.
Used for selection and highlighting of elements.
==============================================================================*/
{
	int return_code;

	ENTER(CM_element_information_from_graphics_name);
	if (cm)
	{
		return_code=1;
		if (0 > graphics_name)
		{
			/* line number */
			cm->type = CM_LINE;
			cm->number = graphics_name - INT_MIN;
		}
		else if (HALF_INT_MAX < graphics_name)
		{
			/* face number */
			cm->type = CM_FACE;
			cm->number = graphics_name - HALF_INT_MAX - 1;
		}
		else
		{
			/* element number */
			cm->type = CM_ELEMENT;
			cm->number = graphics_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CM_element_information_from_graphics_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* CM_element_information_from_graphics_name */

struct GT_glyph_set *create_GT_glyph_set_from_FE_region_nodes(
	struct FE_region *fe_region,
	struct Computed_field *coordinate_field, struct GT_object *glyph,
	FE_value *base_size, FE_value *centre, FE_value *scale_factors,
	FE_value time, struct Computed_field *orientation_scale_field,
	struct Computed_field *variable_scale_field,
	struct Computed_field *data_field, struct Computed_field *label_field,
	enum Graphics_select_mode select_mode,
	struct LIST(FE_node) *selected_node_list)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a GT_glyph_set displaying a <glyph> of at least <base_size>, with the
given glyph <centre> at each node in <fe_region>.
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field. The
optional <variable_scale_field> can provide signed scaling independently of the
glyph axis directions. See function make_glyph_orientation_scale_axes for
details. The combined scale from the above 2 fields is multiplied in each axis
by the <scale_factors> then added to the base_size.
The optional <data_field> is calculated as data over the glyph_set, for later
colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The <select_mode> controls whether node cmiss numbers are output as integer
names with the glyph_set. If <select_mode> is DRAW_SELECTED or DRAW_UNSELECTED,
only nodes in (or not in) the <selected_node_list> are rendered.
Notes:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
- the coordinate system of the variable_scale_field is ignored/not used.
==============================================================================*/
{
	char **labels;
	GTDATA *data;
	int i, n_data_components, *names, number_of_fields, number_of_points,
		return_code;
	struct GT_glyph_set *glyph_set;
	struct Node_to_glyph_set_data node_to_glyph_set_data;
	Triple *axis1_list, *axis2_list, *axis3_list, *point_list, *scale_list;
	struct Computed_field *required_fields[4];
	struct Computed_fields_of_node *computed_fields_of_node;

	ENTER(create_GT_glyph_set_from_FE_region_nodes);
	glyph_set = (struct GT_glyph_set *)NULL;
	if (fe_region && coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field))&&
		((!orientation_scale_field) ||
			(9 >= Computed_field_get_number_of_components(orientation_scale_field)))&&
		glyph && centre && base_size && scale_factors &&
		((!variable_scale_field) ||
			(3 >=	Computed_field_get_number_of_components(variable_scale_field))) &&
		(((GRAPHICS_DRAW_SELECTED!=select_mode)&&
			(GRAPHICS_DRAW_UNSELECTED!=select_mode))||selected_node_list))
	{
		if (computed_fields_of_node=CREATE(Computed_fields_of_node)())
		{
			number_of_fields = 0;	
			computed_fields_of_node->num_coordinate=1;
			computed_fields_of_node->required_fields=required_fields;	
			if (coordinate_field)
			{
				required_fields[number_of_fields]=coordinate_field;
				number_of_fields++;
			}
			if (data_field)
			{
				required_fields[number_of_fields]=data_field;
				number_of_fields++;
			}
			if (label_field)
			{
				required_fields[number_of_fields]=label_field;
				number_of_fields++;
			}
			if (orientation_scale_field)
			{
				required_fields[number_of_fields]=orientation_scale_field;
				number_of_fields++;			
			}
			if (variable_scale_field)
			{
				required_fields[number_of_fields]=variable_scale_field;
				number_of_fields++;			
			}
			computed_fields_of_node->num_required_fields=number_of_fields;
			switch (select_mode)
			{
				case GRAPHICS_DRAW_SELECTED:
				{
					return_code = FE_region_for_each_FE_node_conditional(fe_region,
						FE_node_is_in_list, selected_node_list,
						FE_node_count_if_computed_fields_defined,
						(void *)computed_fields_of_node);
				} break;
				case GRAPHICS_DRAW_UNSELECTED:
				{
					return_code = FE_region_for_each_FE_node_conditional(fe_region,
						FE_node_is_not_in_list, selected_node_list,
						FE_node_count_if_computed_fields_defined,
						(void *)computed_fields_of_node);
				} break;
				default:
				{
					return_code = FE_region_for_each_FE_node(fe_region,
						FE_node_count_if_computed_fields_defined,
						(void *)computed_fields_of_node);
				} break;
			}
			if (return_code) 
			{
				number_of_points = computed_fields_of_node->number_of_nodes;
				if (0<number_of_points)
				{
					point_list = (Triple *)NULL;
					axis1_list = (Triple *)NULL;
					axis2_list = (Triple *)NULL;
					axis3_list = (Triple *)NULL;
					scale_list = (Triple *)NULL;
					labels = (char **)NULL;
					n_data_components = 0;
					data = (GTDATA *)NULL;
					names = (int *)NULL;
					if (data_field)
					{
						n_data_components =
							Computed_field_get_number_of_components(data_field);
						ALLOCATE(data, GTDATA, number_of_points*n_data_components);
					}
					if (label_field)
					{
						ALLOCATE(labels, char *, number_of_points);
						/* clear labels array pointers so new glyph_set not corrupted */
						for (i=0;i<number_of_points;i++)
						{
							labels[i] = (char *)NULL;
						}
					}
					if (GRAPHICS_NO_SELECT != select_mode)
					{
						ALLOCATE(names,int,number_of_points);
					}
					if (((!n_data_components)||data)&&((!label_field)||labels)&&
						((GRAPHICS_NO_SELECT==select_mode)||names) &&
						ALLOCATE(point_list, Triple, number_of_points) &&
						ALLOCATE(axis1_list, Triple, number_of_points) &&
						ALLOCATE(axis2_list, Triple, number_of_points) &&
						ALLOCATE(axis3_list, Triple, number_of_points) &&
						ALLOCATE(scale_list, Triple, number_of_points) &&
						(glyph_set=CREATE(GT_glyph_set)(number_of_points,point_list,
							axis1_list,axis2_list,axis3_list,scale_list,glyph,labels,
							n_data_components,data,/*object_name*/0,names)))
					{
						/* set up information for the iterator */
						for (i = 0; i < 3; i++)
						{
							node_to_glyph_set_data.base_size[i] = base_size[i];
							node_to_glyph_set_data.centre[i] = centre[i];
							node_to_glyph_set_data.scale_factors[i] = scale_factors[i];
						}
						node_to_glyph_set_data.point = point_list;
						node_to_glyph_set_data.axis1 = axis1_list;
						node_to_glyph_set_data.axis2 = axis2_list;
						node_to_glyph_set_data.axis3 = axis3_list;
						node_to_glyph_set_data.scale = scale_list;
						node_to_glyph_set_data.data = data;
						node_to_glyph_set_data.label = labels;
						node_to_glyph_set_data.coordinate_field = coordinate_field;
						node_to_glyph_set_data.orientation_scale_field =
							orientation_scale_field;
						node_to_glyph_set_data.variable_scale_field = variable_scale_field;
						node_to_glyph_set_data.data_field = data_field;
						node_to_glyph_set_data.n_data_components = n_data_components;
						node_to_glyph_set_data.label_field = label_field;
						node_to_glyph_set_data.name = names;
						node_to_glyph_set_data.time = time;
						switch (select_mode)
						{
							case GRAPHICS_DRAW_SELECTED:
							{
								return_code = FE_region_for_each_FE_node_conditional(fe_region,
									FE_node_is_in_list, selected_node_list,
									node_to_glyph_set, (void *)(&node_to_glyph_set_data));
							} break;
							case GRAPHICS_DRAW_UNSELECTED:
							{
								return_code = FE_region_for_each_FE_node_conditional(fe_region,
									FE_node_is_not_in_list, selected_node_list,
									node_to_glyph_set, (void *)(&node_to_glyph_set_data));
							} break;
							default:
							{
								return_code = FE_region_for_each_FE_node(fe_region,
									node_to_glyph_set, (void *)(&node_to_glyph_set_data));
							} break;
						}
						/* clear Computed_field caches so nodes not accessed */
						Computed_field_clear_cache(coordinate_field);
						if (orientation_scale_field)
						{
							Computed_field_clear_cache(orientation_scale_field);
						}
						if (variable_scale_field)
						{
							Computed_field_clear_cache(variable_scale_field);
						}
						if (data_field)
						{
							Computed_field_clear_cache(data_field);
						}
						if (label_field)
						{
							Computed_field_clear_cache(label_field);
						}
						if (!return_code)
						{
							DESTROY(GT_glyph_set)(&glyph_set);
						}
					}
					else
					{
						DEALLOCATE(point_list);
						DEALLOCATE(axis1_list);
						DEALLOCATE(axis2_list);
						DEALLOCATE(axis3_list);
						DEALLOCATE(scale_list);
						DEALLOCATE(data);
						DEALLOCATE(labels);
						DEALLOCATE(names);
					}
					if (!glyph_set)
					{
						display_message(ERROR_MESSAGE,
							"create_GT_glyph_set_from_FE_region_nodes.  Failed");
					}
				} /* no points, hence no glyph_set */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_GT_glyph_set_from_FE_region_nodes.  "
					"Error counting nodes with all fields defined");
			}
			DESTROY(Computed_fields_of_node)(&computed_fields_of_node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_GT_glyph_set_from_FE_region_nodes.  "
				"Could not create fields_of_node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_glyph_set_from_FE_region_nodes.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph_set);
} /* create_GT_glyph_set_from_FE_region_nodes */

struct GT_polyline *create_GT_polyline_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *data_field,int number_of_segments,
	struct FE_element *top_level_element, FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a <GT_polyline> from the <coordinate_field> for the 1-D finite <element>
using <number_of_segments> spaced equally in xi.
The optional <data_field> (currently only a scalar) is calculated as data over
the polyline, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/
{
	FE_value coordinates[3],distance,xi;
	GTDATA *data;
	int i,n_data_components;
	struct CM_element_information cm;
	struct GT_polyline *polyline;
	Triple *point,*points;

	ENTER(create_GT_polyline_from_FE_element);
	if (element && (1 == get_FE_element_dimension(element)) &&
		(0 < number_of_segments) && coordinate_field &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)))
	{
		/* clear coordinates in case coordinate field is not 3 component */
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		polyline=(struct GT_polyline *)NULL;
		points=(Triple *)NULL;
		n_data_components=0;
		data=(GTDATA *)NULL;
		if (data_field)
		{
			n_data_components = Computed_field_get_number_of_components(data_field);
			if (!ALLOCATE(data,GTDATA,(number_of_segments+1)*n_data_components))
			{
				display_message(ERROR_MESSAGE,
					"create_GT_polyline_from_FE_element.  Could allocate data");
			}
		}
		if ((data||(!n_data_components))&&
			ALLOCATE(points,Triple,number_of_segments+1)&&
			(polyline=CREATE(GT_polyline)(g_PLAIN,number_of_segments+1,
				points,/* normalpoints */NULL,n_data_components,data)))
		{
			/* for selective editing of GT_object primitives, record element ID */
			get_FE_element_identifier(element, &cm);
			polyline->object_name = CM_element_information_to_graphics_name(&cm);
			point=points;
			distance=(FE_value)number_of_segments;
			i=0;
			for (i=0;(i<=number_of_segments)&&polyline;i++)
			{
				xi=((FE_value)i)/distance;
				/* evaluate the fields */
				if (Computed_field_evaluate_in_element(coordinate_field,element,&xi,
					time,top_level_element,coordinates,(FE_value *)NULL)&&
					((!data_field)||Computed_field_evaluate_in_element(
					data_field,element,&xi,time,top_level_element,data,
					(FE_value *)NULL)))
				{
					(*point)[0]=coordinates[0];
					(*point)[1]=coordinates[1];
					(*point)[2]=coordinates[2];
					point++;
					if (data_field)
					{
						data+=n_data_components;
					}
				}
				else
				{
					/* error evaluating fields */
					DESTROY(GT_polyline)(&polyline);
				}
			}
			/* clear Computed_field caches so elements not accessed */
			Computed_field_clear_cache(coordinate_field);
			if (data_field)
			{
				Computed_field_clear_cache(data_field);
			}
		}
		else
		{
			DEALLOCATE(points);
			DEALLOCATE(data);
		}
		if (!polyline)
		{
			display_message(ERROR_MESSAGE,
				"create_GT_polyline_from_FE_element.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_polyline_from_FE_element.  Invalid argument(s)");
		polyline=(struct GT_polyline *)NULL;
	}
	LEAVE;

	return (polyline);
} /* create_GT_polyline_from_FE_element */

struct GT_surface *create_cylinder_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	float constant_radius,float scale_factor,struct Computed_field *radius_field,
	int number_of_segments_along,int number_of_segments_around,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element,FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a <GT_surface> from the <coordinate_field> and the radius for the 1-D
finite <element> using a grid of points.  The cylinder is built out of an array
of rectangles <number_of_segments_along> by <number_of_segments_around> the
cylinder.The actual radius is calculated as:
radius = constant_radius + scale_factor*radius_field(a scalar field)
The optional <data_field> (currently only a scalar) is calculated as data over
the length of the cylinder, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
The first component of <texture_coordinate_field> is used to control the
texture coordinates along the element. If not supplied it will match Xi. The
texture coordinate around the cylinders is always from 0 to 1.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/
{
	FE_value coordinates[3], cos_theta, derivative_xi[3], distance, dS_dxi,
		end_aligned_normal[3], facet_angle, jacobian[9], length, normal_1, normal_2,
		normal_3, *radius_array, radius_derivative, radius_value, sin_theta,
		tex_coordinates[3], theta, theta_change, xi, x, y;
	float texture_coordinate1;
	GTDATA *data, *datum;
	int facet_offset,i,j,k,n_data_components,number_of_points;
	struct CM_element_information cm;
	struct GT_surface *surface;
	Triple *derivative, *normal, *normalpoints, *point, *points, *previous_point,
		*previous_normal, *texturepoints, *texture_coordinate;

	ENTER(create_cylinder_from_FE_element);
	if (element && (1 == get_FE_element_dimension(element)) &&
		(0 < number_of_segments_along) && (1 < number_of_segments_around) &&
		coordinate_field &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)) &&
		((!radius_field) ||
			(1 == Computed_field_get_number_of_components(radius_field))) &&
		((!texture_coordinate_field) ||
			(3 >= Computed_field_get_number_of_components(texture_coordinate_field))))
	{
		/* clear coordinates and derivatives not set if coordinate field is not
			 3 component */
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		derivative_xi[1]=0.0;
		derivative_xi[2]=0.0;
		surface=(struct GT_surface *)NULL;
		points=(Triple *)NULL;
		normalpoints=(Triple *)NULL;
		texturepoints=(Triple *)NULL;
		n_data_components=0;
		data=(GTDATA *)NULL;
		number_of_points=(number_of_segments_along+1)*(number_of_segments_around+1);
		if (data_field)
		{
			n_data_components = Computed_field_get_number_of_components(data_field);
			if (!ALLOCATE(data,GTDATA,number_of_points*n_data_components))
			{
				display_message(ERROR_MESSAGE,
					"create_cylinder_from_FE_element.  Could allocate data");
			}
		}
		if ((data||(!n_data_components))&&
			ALLOCATE(points,Triple,number_of_points)&&
			ALLOCATE(normalpoints,Triple,number_of_points)&&
			ALLOCATE(texturepoints,Triple,number_of_points)&&
			ALLOCATE(radius_array,FE_value,3*(number_of_segments_along+1))&&
			(surface=CREATE(GT_surface)(/*g_SHADED*/g_SHADED_TEXMAP,g_QUADRILATERAL,
				number_of_segments_around+1,number_of_segments_along+1,
				points,normalpoints,(Triple *)NULL,texturepoints,n_data_components,data)))
		{
			/* for selective editing of GT_object primitives, record element ID */
			get_FE_element_identifier(element, &cm);
			surface->object_name = CM_element_information_to_graphics_name(&cm);
			point=points;
			derivative=normalpoints;
			texture_coordinate=texturepoints;
			/* Calculate the points and radius and data at the each point */
			for (i=0;(i<=number_of_segments_along)&&surface;i++)
			{
				xi=(float)i/(float)number_of_segments_along;
				/* evaluate the fields */
				if (Computed_field_evaluate_in_element(coordinate_field,element,&xi,
					time,top_level_element,coordinates,derivative_xi)&&
					((!data_field)||Computed_field_evaluate_in_element(
					data_field,element,&xi,time,top_level_element,data,
					(FE_value *)NULL)) &&
					((!radius_field) ||
						Computed_field_evaluate_in_element(radius_field,element,&xi,
							time,top_level_element,&radius_value,&radius_derivative)) &&
					((!texture_coordinate_field) ||
						Computed_field_evaluate_in_element(texture_coordinate_field,
							element, &xi, time, top_level_element, tex_coordinates,
							/*derivatives*/(FE_value *)NULL)))
				{
					/* store the coordinates in the point */
					(*point)[0]=coordinates[0];
					(*point)[1]=coordinates[1];
					(*point)[2]=coordinates[2];
					/* normalize the line direction (derivative) */
					/* keep dS/dxi in the radius_array for converting derivatives later */
					dS_dxi = sqrt(derivative_xi[0]*derivative_xi[0]+
						derivative_xi[1]*derivative_xi[1]+
						derivative_xi[2]*derivative_xi[2]);
					radius_array[3*i+2] = dS_dxi;
					if (0.0 < dS_dxi)
					{
						derivative_xi[0] /= dS_dxi;
						derivative_xi[1] /= dS_dxi;
						derivative_xi[2] /= dS_dxi;
					}
					/* store the derivative in the normal */
					(*derivative)[0]=derivative_xi[0];
					(*derivative)[1]=derivative_xi[1];
					(*derivative)[2]=derivative_xi[2];
					/* store the radius and derivative in the radius array */
					if (radius_field)
					{
						radius_array[3*i] = constant_radius+scale_factor*radius_value;
						radius_array[3*i+1] = radius_derivative * scale_factor;
					}
					else
					{
						radius_array[3*i] = constant_radius;
						radius_array[3*i+1] = 0.0;
					}
					/* store the data and then we are done with it */
					if (data_field)
					{
						datum=data;
						for (j=number_of_segments_around;j>=0;j--)
						{
							for (k=0;k<n_data_components;k++)
							{
								data[k] = datum[k];
							}
							data+=n_data_components;
						}
					}
					/* store the first texture coordinate */
					if (texture_coordinate_field)
					{
						(*texture_coordinate)[0] = (float)(tex_coordinates[0]);
					}
					else
					{
						/* default is to use xi for the first texture coordinate */
						(*texture_coordinate)[0] = (float)(xi);
					}
				}
				else
				{
					/* error evaluating fields */
					DESTROY(GT_surface)(&surface);
				}
				point += number_of_segments_around+1;
				derivative += number_of_segments_around+1;
				texture_coordinate += number_of_segments_around+1;
			}

			if (surface)
			{
				/* Calculate the normals at the first and last points as we must line
					up at these points so that the next elements join on correctly */
				for (i=0; i<=number_of_segments_along; i+=number_of_segments_along)
				{
					point = points + i * (number_of_segments_around+1);
					derivative = normalpoints + i * (number_of_segments_around+1);
					normal = normalpoints + i * (number_of_segments_around+1) + 1;
					derivative_xi[0] = (*derivative)[0];
					derivative_xi[1] = (*derivative)[1];
					derivative_xi[2] = (*derivative)[2];
					dS_dxi = radius_array[3*i+2];
					/* if the derivative is zero, use the change in location between
						 this and the nearest point along the element so that the normal
						 will at least remain normal to the axis of the cylinder */
					if (0.0 == dS_dxi)
					{
						if (0 == i)
						{
							previous_point = point;
							point = points + (i + 1) * (number_of_segments_around + 1);
						}
						else
						{
							previous_point =
								points + (i - 1) * (number_of_segments_around + 1);
						}
						derivative_xi[0] = (*point)[0] - (*previous_point)[0];
						derivative_xi[1] = (*point)[1] - (*previous_point)[1];
						derivative_xi[2] = (*point)[2] - (*previous_point)[2];
						/* normalise this pseudo derivative */
						if (0.0 < (length = sqrt(derivative_xi[0]*derivative_xi[0] +
							derivative_xi[1]*derivative_xi[1] +
							derivative_xi[2]*derivative_xi[2])))
						{
							derivative_xi[0] /= length;
							derivative_xi[1] /= length;
							derivative_xi[2] /= length;
						}
						/* put it back in the derivatives; we know it is a zero derivative
							 from the stored dS_dxi */
						(*derivative)[0] = derivative_xi[0];
						(*derivative)[1] = derivative_xi[1];
						(*derivative)[2] = derivative_xi[2];
					}
					/* get any vector not aligned with derivative */
					jacobian[0] = 0.0;
					jacobian[1] = 0.0;
					jacobian[2] = 0.0;
					/* make jacobian have 1.0 in the component with the least absolute
						value in derivative_xi */
					if (fabs(derivative_xi[0]) < fabs(derivative_xi[1]))
					{
						if (fabs(derivative_xi[2]) < fabs(derivative_xi[0]))
						{
							jacobian[2] = 1.0;
						}
						else
						{
							jacobian[0] = 1.0;
						}
					}
					else
					{
						if (fabs(derivative_xi[2]) < fabs(derivative_xi[1]))
						{
							jacobian[2] = 1.0;
						}
						else
						{
							jacobian[1] = 1.0;
						}
					}
					/* get cross product of the derivative and this vector
						= vector normal to derivative */
					/* Put this in the normal, we don't need the derivative anymore */
					jacobian[3] =
						derivative_xi[1]*jacobian[2] - derivative_xi[2]*jacobian[1];
					jacobian[4] =
						derivative_xi[2]*jacobian[0] - derivative_xi[0]*jacobian[2];
					jacobian[5] =
						derivative_xi[0]*jacobian[1] - derivative_xi[1]*jacobian[0];
					/* make normal into a unit vector */
					if (0.0 < (length = (float)sqrt((double)(jacobian[3]*jacobian[3] +
						jacobian[4]*jacobian[4] + jacobian[5]*jacobian[5]))))
					{
						jacobian[3] /= length;
						jacobian[4] /= length;
						jacobian[5] /= length;
					}
					(*normal)[0] = jacobian[3];
					(*normal)[1] = jacobian[4];
					(*normal)[2] = jacobian[5];
				}
				end_aligned_normal[0] = (*normal)[0];
				end_aligned_normal[1] = (*normal)[1];
				end_aligned_normal[2] = (*normal)[2];

				/* Propogate the first normal along the segments keeping it in
					the same plane each step */
				for (i=1;i<=number_of_segments_along;i++)
				{
					point = points + i * (number_of_segments_around+1);
					derivative = normalpoints + i * (number_of_segments_around+1);
					normal = normalpoints + i * (number_of_segments_around+1) + 1;
					previous_point = points + (i-1) * (number_of_segments_around+1);
					previous_normal = normalpoints + (i-1) * (number_of_segments_around+1) + 1;

					/* Get the change in position */
					jacobian[0] = (*point)[0] - (*previous_point)[0];
					jacobian[1] = (*point)[1] - (*previous_point)[1];
					jacobian[2] = (*point)[2] - (*previous_point)[2];
				
					/* Get the normal to plane which contains change_in_position
						vector and previous_normal_vector */
					jacobian[3] = jacobian[1] * (*previous_normal)[2] -
						jacobian[2] * (*previous_normal)[1];
					jacobian[4] = jacobian[2] * (*previous_normal)[0] -
						jacobian[0] * (*previous_normal)[2];
					jacobian[5] = jacobian[0] * (*previous_normal)[1] -
						jacobian[1] * (*previous_normal)[0];

					/* Get the new normal vector which lies in the plane */
					jacobian[0] = jacobian[4] * (*derivative)[2] -
						jacobian[5] * (*derivative)[1];
					jacobian[1] = jacobian[5] * (*derivative)[0] -
						jacobian[3] * (*derivative)[2];
					jacobian[2] = jacobian[3] * (*derivative)[1] -
						jacobian[4] * (*derivative)[0];

					/* Store this in the other normal space and normalise */
					(*normal)[0] = jacobian[0];
					(*normal)[1] = jacobian[1];
					(*normal)[2] = jacobian[2];
					if (0.0<(distance=
						sqrt((*normal)[0]*(*normal)[0] + (*normal)[1]*(*normal)[1]+
							(*normal)[2]*(*normal)[2])))
					{
						(*normal)[0]/=distance;
						(*normal)[1]/=distance;
						(*normal)[2]/=distance;
					}
				}

				/* Find the closest correlation between the end_aligned_normal and the 
					propogated normal */
				derivative = normalpoints + number_of_segments_along * (number_of_segments_around+1);
				normal = normalpoints + number_of_segments_along * (number_of_segments_around+1) + 1;

				/* calculate theta, the angle from the end_aligned_normal to the
					 propagated normal in a right hand sense about the derivative */
				jacobian[0] = end_aligned_normal[0];
				jacobian[1] = end_aligned_normal[1];
				jacobian[2] = end_aligned_normal[2];

				jacobian[3]= (*derivative)[1]*jacobian[2]-(*derivative)[2]*jacobian[1];
				jacobian[4]= (*derivative)[2]*jacobian[0]-(*derivative)[0]*jacobian[2];
				jacobian[5]= (*derivative)[0]*jacobian[1]-(*derivative)[1]*jacobian[0];

				x = (*normal)[0] * jacobian[0] +
				    (*normal)[1] * jacobian[1] +
				    (*normal)[2] * jacobian[2];
				y = (*normal)[0] * jacobian[3] +
				    (*normal)[1] * jacobian[4] +
				    (*normal)[2] * jacobian[5];
				theta = atan2(y, x);
				if (theta < 0.0)
				{
					theta += 2*PI;
				}
				facet_angle = 2*PI/number_of_segments_around;
				/* calculate the number of times facet_angle can occur before theta */
				facet_offset = theta / facet_angle;
				/* get angle from the next lowest whole facet to propagated normal */
				theta -= facet_offset*facet_angle;
				/* nearest facet could be on the otherside of the propagated normal so
					 handle this case; theta_change to nearest facet has opposite sign */
				if (theta > 0.5*facet_angle)
				{
					theta_change = facet_angle - theta;
					facet_offset++;
				}
				else
				{
					theta_change = -theta;
				}

				/* Calculate the actual points and normals */
				for (i=0;(i<=number_of_segments_along);i++)
				{
					/* Get the two normals */
					point = points + i * (number_of_segments_around+1);
					derivative = normalpoints + i * (number_of_segments_around+1);
					normal = normalpoints + i * (number_of_segments_around+1) + 1;

					if (i < number_of_segments_along)
					{
						jacobian[0] = (*normal)[0];
						jacobian[1] = (*normal)[1];
						jacobian[2] = (*normal)[2];
					}
					else
					{
						jacobian[0] = end_aligned_normal[0];
						jacobian[1] = end_aligned_normal[1];
						jacobian[2] = end_aligned_normal[2];
					}

					derivative_xi[0] = (*derivative)[0];
					derivative_xi[1] = (*derivative)[1];
					derivative_xi[2] = (*derivative)[2];

					jacobian[3]=derivative_xi[1]*jacobian[2]-derivative_xi[2]*jacobian[1];
					jacobian[4]=derivative_xi[2]*jacobian[0]-derivative_xi[0]*jacobian[2];
					jacobian[5]=derivative_xi[0]*jacobian[1]-derivative_xi[1]*jacobian[0];

					/* Get the other stored values */
					radius_value = radius_array[3*i];
					radius_derivative = radius_array[3*i+1];
					dS_dxi = radius_array[3*i+2];

					/* Write the true normals and positions */
					normal = normalpoints + i * (number_of_segments_around+1);				
					for (j=number_of_segments_around;0<=j;j--)
					{
						if (i < number_of_segments_along)
						{
							theta = theta_change * ((float) i)/ ((float)number_of_segments_along)
								+ PI*2.0*((float)j)/((float)number_of_segments_around);
						}
						else
						{
							theta = PI*2.*((float)(j + facet_offset))/
								((float)number_of_segments_around);
						}
						cos_theta=cos(theta);
						sin_theta=sin(theta);
						(normal[j])[0]=cos_theta*jacobian[0]+sin_theta*jacobian[3];
						(normal[j])[1]=cos_theta*jacobian[1]+sin_theta*jacobian[4];
						(normal[j])[2]=cos_theta*jacobian[2]+sin_theta*jacobian[5];
						(point[j])[0]=(point[0])[0]+radius_value*(normal[j])[0];
						(point[j])[1]=(point[0])[1]+radius_value*(normal[j])[1];
						(point[j])[2]=(point[0])[2]+radius_value*(normal[j])[2];
						if (radius_field && (0.0 != radius_derivative))
						{
							if (0.0 < dS_dxi)
							{
								(normal[j])[0] -= (radius_derivative/dS_dxi)*derivative_xi[0];
								(normal[j])[1] -= (radius_derivative/dS_dxi)*derivative_xi[1];
								(normal[j])[2] -= (radius_derivative/dS_dxi)*derivative_xi[2];
							}
							else
							{
								/* a finite change of radius is happening in an infinitessimal
									 space. Hence, make normal aligned with derivative */
								if (radius_derivative < 0.0)
								{
									(normal[j])[0] = derivative_xi[0];
									(normal[j])[1] = derivative_xi[1];
									(normal[j])[2] = derivative_xi[2];
								}
								else
								{
									(normal[j])[0] = -derivative_xi[0];
									(normal[j])[1] = -derivative_xi[1];
									(normal[j])[2] = -derivative_xi[2];
								}
							}
						}
					}
				}
			}
			DEALLOCATE(radius_array);
			if (surface)
			{
				/* normalize the normals */
				normal=normalpoints;
				for (i=number_of_points;i>0;i--)
				{
					normal_1=(*normal)[0];
					normal_2=(*normal)[1];
					normal_3=(*normal)[2];
					if (0.0<(distance=
						sqrt(normal_1*normal_1+normal_2*normal_2+normal_3*normal_3)))
					{
						(*normal)[0]=normal_1/distance;
						(*normal)[1]=normal_2/distance;
						(*normal)[2]=normal_3/distance;
					}
					normal++;
				}
				/* calculate the texture coordinates */
				/* the texture coordinate along the length has been set above but must
					 be propagated to vertices around the cylinder. The second texture
					 coordinate ranges from from 0 to 1 around the circumference */
				texture_coordinate = texturepoints;
				for (i = 0; i <= number_of_segments_along; i++)
				{
					texture_coordinate1 = (*texture_coordinate)[0];
					for (j = 0; j <= number_of_segments_around; j++)
					{
						(*texture_coordinate)[0] = texture_coordinate1;
						(*texture_coordinate)[1] =
							(float)j / (float)number_of_segments_around;
						(*texture_coordinate)[2] = 0.0;
						texture_coordinate++;
					}
				}
			}
			/* clear Computed_field caches so elements not accessed */
			Computed_field_clear_cache(coordinate_field);
			if (data_field)
			{
				Computed_field_clear_cache(data_field);
			}
			if (radius_field)
			{
				Computed_field_clear_cache(radius_field);
			}
		}
		else
		{
			DEALLOCATE(points);
			DEALLOCATE(normalpoints);
			DEALLOCATE(texturepoints);
			DEALLOCATE(data);
		}
		if (!surface)
		{
			display_message(ERROR_MESSAGE,
				"create_cylinder_from_FE_element.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_cylinder_from_FE_element.  Invalid argument(s)");
		surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (surface);
} /* create_cylinder_from_FE_element */

struct GT_nurbs *create_GT_nurb_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element, FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
==============================================================================*/
{
	double sign1, sign2, *sknots, *tknots, *control_points,
		*texture_control_points;
	FE_value derivative_xi[6],coordinates[3], xi[3];
	struct CM_element_information cm;
	struct GT_nurbs *nurbs;
	int i, j, offset0, offset1, offset2, number_of_points, sorder, torder,
		sknotcount, tknotcount, scontrolcount, tcontrolcount,
		texture_coordinate_components;

	ENTER(create_GT_nurb_from_FE_element);
	/* check the arguments */
	if (element && (2 == get_FE_element_dimension(element)) &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)))
	{
#if defined (OLD_CODE)
		collapsed_nodes=0;
		number_of_polygon_vertices=0;
		polygon_xi2_zero=0;
		modified_reverse_normals=reverse_normals;
		/* check for polygon */
		switch (*(element->shape->type))
		{
			case POLYGON_SHAPE:
			{
				/* polygon */
				number_of_polygon_vertices=(element->shape->type)[1];
				number_of_points_in_xi1=((number_of_segments_in_xi1_requested)/
					number_of_polygon_vertices+1)*number_of_polygon_vertices+1;
				polygon_xi2_zero=1;
				number_of_points_in_xi2=number_of_segments_in_xi2_requested+1;
				number_of_points=number_of_points_in_xi1*number_of_points_in_xi2;
				polygon_type=g_QUADRILATERAL;
			} break;
			case SIMPLEX_SHAPE:
			{
				/* simplex */
				if (number_of_segments_in_xi1_requested >
					number_of_segments_in_xi2_requested)
				{
					number_of_points_in_xi1=number_of_segments_in_xi1_requested+1;
					number_of_points_in_xi2=number_of_segments_in_xi1_requested+1;
				}
				else
				{
					number_of_points_in_xi1=number_of_segments_in_xi2_requested+1;
					number_of_points_in_xi2=number_of_segments_in_xi2_requested+1;
				}
				number_of_points=
					(number_of_points_in_xi1*(number_of_points_in_xi1+1))/2;
				polygon_type=g_TRIANGLE;
				/* to get the surface colouring right, the points need to be given in a
					certain order to GL_TRIANGLE_STRIP.  To keep the lighting correct,
					the normals need to be reversed */
				if (reverse_normals)
				{
					modified_reverse_normals=0;
				}
				else
				{
					modified_reverse_normals=1;
				}
			} break;
			default:
			{
				number_of_points_in_xi1=number_of_segments_in_xi1_requested+1;
				/* check for bi-quadratic Lagrange with collapse along xi2=0 */
				/*???DB.  Extend ? */
				if ((element->information)&&(9==element->information->number_of_nodes)&&
					(get_FE_node_identifier((element->information->nodes)[0])==
						get_FE_node_identifier((element->information->nodes)[2])))
				{
					collapsed_nodes=1;
				}
				number_of_points_in_xi2=number_of_segments_in_xi2_requested+1;
				number_of_points=number_of_points_in_xi1*number_of_points_in_xi2;
				polygon_type=g_QUADRILATERAL;
			} break;
		}
#endif /* defined (OLD_CODE) */
		/* allocate memory for the surface */
		sorder = 4;
		torder = 4;
		sknotcount = 8;
		tknotcount = 8;
		scontrolcount = 4;
		tcontrolcount = 4;
		number_of_points = scontrolcount * tcontrolcount;
		if (ALLOCATE(sknots, double, sknotcount) && 
			ALLOCATE(tknots, double, tknotcount)&&
			ALLOCATE(control_points, double, 4 * number_of_points))
		{
			if (nurbs=CREATE(GT_nurbs)())
			{
				/* for selective editing of GT_object primitives, record element ID */
				get_FE_element_identifier(element, &cm);
				nurbs->object_name = CM_element_information_to_graphics_name(&cm);
				if (GT_nurbs_set_surface(nurbs, sorder, torder,
					sknotcount, tknotcount, sknots, tknots, 
					scontrolcount, tcontrolcount, control_points))
				{
					if (texture_coordinate_field)
					{
						if (ALLOCATE(texture_control_points, double, 4 * number_of_points))
						{
							GT_nurbs_set_texture_control_points(nurbs,
								texture_control_points);
							texture_coordinate_components = 
								Computed_field_get_number_of_components(texture_coordinate_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_GT_nurb_from_FE_element.   Unable to allocate texture control points");
							texture_coordinate_field = (struct Computed_field *)NULL;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_GT_nurb_from_FE_element.   Could not set surface");
					DESTROY(GT_nurbs)(&nurbs);
					DEALLOCATE(sknots);
					DEALLOCATE(tknots);
					DEALLOCATE(control_points);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_GT_nurb_from_FE_element.   Could not create surface");
				DEALLOCATE(sknots);
				DEALLOCATE(tknots);
				DEALLOCATE(control_points);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_GT_nurb_from_FE_element.   Could not allocate memory for points");
			DEALLOCATE(sknots);
			DEALLOCATE(tknots);
			DEALLOCATE(control_points);
			nurbs=(struct GT_nurbs *)NULL;
		}
		if (nurbs)
		{
			for(i = 0 ; i < sknotcount ; i++)
			{
				if(i < sknotcount / 2)
				{
					sknots[i] = 0.0;
				}
				else
				{
					sknots[i] = 1.0;
				}
			}
			for(i = 0 ; i < tknotcount ; i++)
			{
				if(i < tknotcount / 2)
				{
					tknots[i] = 0.0;
				}
				else
				{
					tknots[i] = 1.0;
				}
			}
		}
			
		if (nurbs)
		{
#if defined (OLD_CODE)
			/* calculate the xi coordinates and store in "normals" */
			point_a=points+number_of_points;
			point_b=point_a;
			distance=(FE_value)(number_of_points_in_xi1-1);
			if (SIMPLEX_SHAPE== *(element->shape->type))
			{
				for (i=0;i<number_of_points_in_xi1;i++)
				{
					(*point_a)[0]=(float)i/distance;
					point_a++;
				}
				for (j=number_of_points_in_xi2-1;j>0;j--)
				{
					for (i=0;i<j;i++)
					{
						(*point_a)[0]=(*point_b)[0];
						point_a++;
						point_b++;
					}
					point_b++;
				}
				point=points+number_of_points;
				distance=(float)(number_of_points_in_xi2-1);
				for (j=0;j<number_of_points_in_xi2;j++)
				{
					xi2=(float)j/distance;
					for (i=number_of_points_in_xi1-j;i>0;i--)
					{
						(*point)[1]=xi2;
						point++;
					}
				}
			}
			else
			{
				for (i=0;i<number_of_points_in_xi1;i++)
				{
					(*point_a)[0]=(float)i/distance;
					point_a++;
				}
				for (j=1;j<number_of_points_in_xi2;j++)
				{
					for (i=0;i<number_of_points_in_xi1;i++)
					{
						(*point_a)[0]=(*point_b)[0];
						point_a++;
						point_b++;
					}
				}
				point=points+number_of_points;
				distance=(float)(number_of_points_in_xi2-1);
				for (j=0;j<number_of_points_in_xi2;j++)
				{
					xi2=(float)j/distance;
					for (i=0;i<number_of_points_in_xi1;i++)
					{
						(*point)[1]=xi2;
						point++;
					}
				}
			}
#endif /* defined (OLD_CODE) */
			/* Assuming a HERMITE nurb definition at the moment */
			if(sorder == 4 && torder == 4)
			{
				i = 0;
				while ( i < 4 && nurbs)
				{
					switch(i)
					{
						case 0:
						{
							xi[0] = 0.0;
							xi[1] = 0.0;
							sign1 = 1.0;
							sign2 = 1.0;
							offset0 = 0;
							offset1 = 1;
							offset2 = 4;
						} break;
						case 1:
						{
							xi[0] = 1.0;
							xi[1] = 0.0;
							sign1 = -1.0;
							sign2 = 1.0;
							offset0 = 3;
							offset1 = -1;
							offset2 = 4;
						} break;
						case 2:
						{
							xi[0] = 0.0;
							xi[1] = 1.0;
							sign1 = 1.0;
							sign2 = -1.0;
							offset0 = 12;
							offset1 = 1;
							offset2 = -4;
						} break;
						case 3:
						{
							xi[0] = 1.0;
							xi[1] = 1.0;
							sign1 = -1.0;
							sign2 = -1.0;
							offset0 = 15;
							offset1 = -1;
							offset2 = -4;
						} break;
					}
					xi[2] = 0.0;
					
					if (Computed_field_evaluate_in_element(coordinate_field,element,xi,
						time,top_level_element,coordinates,derivative_xi))
					{
						for (j = 0 ; j < 3 ; j++)
						{							
							control_points[4 * offset0 + j] = coordinates[j];
							control_points[4 * (offset0 + offset1) + j] = coordinates[j]
								+ sign1 * derivative_xi[2 * j] / 3.0;
							control_points[4 * (offset0 + offset2) + j] = coordinates[j]
								+ sign2 * derivative_xi[2 * j + 1] / 3.0;
							control_points[4 * (offset0 + offset1 + offset2) + j] = coordinates[j]
								+ sign1 * derivative_xi[2 * j] / 3.0
								+ sign2 * derivative_xi[2 * j + 1] / 3.0;
						}

						control_points[4 * offset0 + 3] = 1.0;
						control_points[4 * (offset0 + offset1) + 3] = 1.0;
						control_points[4 * (offset0 + offset2) + 3] = 1.0;
						control_points[4 * (offset0 + offset1 + offset2) + 3] = 1.0;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_GT_nurb_from_FE_element.  Error calculating coordinate field");
						DESTROY(GT_nurbs)(&nurbs);
					}

					if (texture_coordinate_field)
					{
						if (Computed_field_evaluate_in_element(texture_coordinate_field,
							element,xi,time,(struct FE_element *)NULL,
							coordinates,derivative_xi))
						{
							for (j = 0 ; j < texture_coordinate_components ; j++)
							{
								texture_control_points[4 * offset0 + j] = coordinates[j];
								texture_control_points[4 * (offset0 + offset1) + j] = coordinates[j]
									+ sign1 * derivative_xi[2 * j] / 3.0;
								texture_control_points[4 * (offset0 + offset2) + j] = coordinates[j]
									+ sign2 * derivative_xi[2 * j + 1] / 3.0;
								texture_control_points[4 * (offset0 + offset1 + offset2) + j] = coordinates[j]
									+ sign1 * derivative_xi[2 * j] / 3.0
									+ sign2 * derivative_xi[2 * j + 1] / 3.0;
							}
							for ( ; j < 3 ; j++)
							{
								texture_control_points[4 * offset0 + j] = 0.0;
								texture_control_points[4 * (offset0 + offset1) + j] = 0.0;
								texture_control_points[4 * (offset0 + offset2) + j] = 0.0;
								texture_control_points[4 * (offset0 + offset1 + offset2) + j] = 0.0;
							}
							texture_control_points[4 * offset0 + 3] = 1.0;
							texture_control_points[4 * (offset0 + offset1) + 3] = 1.0;
							texture_control_points[4 * (offset0 + offset2) + 3] = 1.0;
							texture_control_points[4 * (offset0 + offset1 + offset2) + 3] = 1.0;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_GT_nurb_from_FE_element.  Error calculating texture coordinate field");
							DESTROY(GT_nurbs)(&nurbs);
						}
					}
					i++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_GT_nurb_from_FE_element.  Only forth order nurbs calculated at"
					" the moment using Hermite Bezier patches");
				DESTROY(GT_nurbs)(&nurbs);
			}
			/* clear Computed_field caches so elements not accessed */
			Computed_field_clear_cache(coordinate_field);
#if defined (OLD_CODE)
			if (nurbs)
			{
				/* for collapsed nodes on xi2=0, copy normals from next row */
				/*???DB.  Extend ? */
				if (collapsed_nodes)
				{
					normal=points+number_of_points;
					for (i=number_of_points_in_xi1;i>0;i--)
					{
						(*normal)[0]=(*(normal+number_of_points_in_xi1))[0];
						(*normal)[1]=(*(normal+number_of_points_in_xi1))[1];
						(*normal)[2]=(*(normal+number_of_points_in_xi1))[2];
						normal++;
					}
				}
				/* for polygons, calculate the normals for the xi2=0 edge */
				if (number_of_polygon_vertices>=3)
				{
					/* fix the normal in the centre by copying it from the next row of
						points */
					normal=points+number_of_points;
					for (i=number_of_points_in_xi1;i>0;i--)
					{
						(*normal)[0]=(*(normal+number_of_points_in_xi1))[0];
						(*normal)[1]=(*(normal+number_of_points_in_xi1))[1];
						(*normal)[2]=(*(normal+number_of_points_in_xi1))[2];
						normal++;
					}
				}
				/* normalize the normals */
				normal=points+number_of_points;
				if (modified_reverse_normals)
				{
					for (i=number_of_points;i>0;i--)
					{
						normal_1=(*normal)[0];
						normal_2=(*normal)[1];
						normal_3=(*normal)[2];
						if (0<(distance=normal_1*normal_1+normal_2*normal_2+
							normal_3*normal_3))
						{
							distance=sqrt(distance);
							(*normal)[0]= -normal_1/distance;
							(*normal)[1]= -normal_2/distance;
							(*normal)[2]= -normal_3/distance;
						}
						normal++;
					}
				}
				else
				{
					for (i=number_of_points;i>0;i--)
					{
						normal_1=(*normal)[0];
						normal_2=(*normal)[1];
						normal_3=(*normal)[2];
						if (0<(distance=normal_1*normal_1+normal_2*normal_2+
							normal_3*normal_3))
						{
							distance=sqrt(distance);
							(*normal)[0]=normal_1/distance;
							(*normal)[1]=normal_2/distance;
							(*normal)[2]=normal_3/distance;
						}
						normal++;
					}
				}
				/* calculate the texture coordinates */
				/*???DB.  Should be able to use scale factors, but don't know for
				  linear.  Locate in structure ? */
				/*???DB.  Shouldn't use Triples for texture coordinates because
				  onlytwo */
				point=points;
				point_s=point;
				point_t=point;
				texture_coordinate=points+(2*number_of_points);
				texture_coordinate_s=texture_coordinate;
				texture_coordinate_t=texture_coordinate;
				(*texture_coordinate)[0]=0.;
				(*texture_coordinate)[1]=0.;
				if (SIMPLEX_SHAPE== *(element->shape->type))
				{
					for (j=number_of_points_in_xi1-1;j>0;j--)
					{
						point++;
						texture_coordinate++;
						coordinate_1=(*point)[0]-(*point_s)[0];
						coordinate_2=(*point)[1]-(*point_s)[1];
						coordinate_3=(*point)[2]-(*point_s)[2];
						(*texture_coordinate)[0]=(*texture_coordinate_s)[0]+
							sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
								coordinate_3*coordinate_3);
						(*texture_coordinate)[1]=0.;
						point_s++;
						texture_coordinate_s++;
					}
					for (i=number_of_points_in_xi2-1;i>0;i--)
					{
						point++;
						texture_coordinate++;
						point_s++;
						texture_coordinate_s++;
						(*texture_coordinate)[0]=0.;
						coordinate_1=(*point)[0]-(*point_t)[0];
						coordinate_2=(*point)[1]-(*point_t)[1];
						coordinate_3=(*point)[2]-(*point_t)[2];
						(*texture_coordinate)[1]=(*texture_coordinate_t)[1]+
							sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
								coordinate_3*coordinate_3);
						point_t++;
						texture_coordinate_t++;
						for (j=i-1;j>0;j--)
						{
							point++;
							texture_coordinate++;
							coordinate_1=(*point)[0]-(*point_s)[0];
							coordinate_2=(*point)[1]-(*point_s)[1];
							coordinate_3=(*point)[2]-(*point_s)[2];
							(*texture_coordinate)[0]=(*texture_coordinate_s)[0]+
								sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
									coordinate_3*coordinate_3);
							coordinate_1=(*point)[0]-(*point_t)[0];
							coordinate_2=(*point)[1]-(*point_t)[1];
							coordinate_3=(*point)[2]-(*point_t)[2];
							(*texture_coordinate)[1]=(*texture_coordinate_t)[1]+
								sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
									coordinate_3*coordinate_3);
							point_s++;
							texture_coordinate_s++;
						}
						point_t++;
						texture_coordinate_t++;
					}
				}
				else
				{
					for (j=number_of_points_in_xi1-1;j>0;j--)
					{
						point++;
						texture_coordinate++;
						coordinate_1=(*point)[0]-(*point_s)[0];
						coordinate_2=(*point)[1]-(*point_s)[1];
						coordinate_3=(*point)[2]-(*point_s)[2];
						(*texture_coordinate)[0]=(*texture_coordinate_s)[0]+
							sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
								coordinate_3*coordinate_3);
						(*texture_coordinate)[1]=0.;
						point_s++;
						texture_coordinate_s++;
					}
					for (i=number_of_points_in_xi2-1;i>0;i--)
					{
						point++;
						texture_coordinate++;
						point_s++;
						texture_coordinate_s++;
						(*texture_coordinate)[0]=0.;
						coordinate_1=(*point)[0]-(*point_t)[0];
						coordinate_2=(*point)[1]-(*point_t)[1];
						coordinate_3=(*point)[2]-(*point_t)[2];
						(*texture_coordinate)[1]=(*texture_coordinate_t)[1]+
							sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
								coordinate_3*coordinate_3);
						point_t++;
						texture_coordinate_t++;
						for (j=number_of_points_in_xi1-1;j>0;j--)
						{
							point++;
							texture_coordinate++;
							coordinate_1=(*point)[0]-(*point_s)[0];
							coordinate_2=(*point)[1]-(*point_s)[1];
							coordinate_3=(*point)[2]-(*point_s)[2];
							(*texture_coordinate)[0]=(*texture_coordinate_s)[0]+
								sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
									coordinate_3*coordinate_3);
							coordinate_1=(*point)[0]-(*point_t)[0];
							coordinate_2=(*point)[1]-(*point_t)[1];
							coordinate_3=(*point)[2]-(*point_t)[2];
							(*texture_coordinate)[1]=(*texture_coordinate_t)[1]+
								sqrt(coordinate_1*coordinate_1+coordinate_2*coordinate_2+
									coordinate_3*coordinate_3);
							point_s++;
							texture_coordinate_s++;
							point_t++;
							texture_coordinate_t++;
						}
					}
				}
			}
#endif /* defined (OLD_CODE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_nurb_from_FE_element.  Invalid argument(s)");
		nurbs=(struct GT_nurbs *)NULL;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave create_GT_nurb_from_FE_element\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (nurbs);
} /* create_GT_nurb_from_FE_element */

int get_surface_element_segmentation(struct FE_element *element,
	int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,int reverse_normals,
	int *number_of_points_in_xi1,int *number_of_points_in_xi2,
	int *number_of_points,int *number_of_polygon_vertices,
	gtPolygonType *polygon_type,enum Collapsed_element_type *collapsed_element,
	char *modified_reverse_normals,
	enum FE_element_shape_type *shape_type_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Sorts out how standard, polygon and simplex elements are segmented, based on
numbers of segments requested for "square" elements.
==============================================================================*/
{
	int i, number_of_faces, return_code;
	struct FE_element *faces[4];
	struct FE_element_shape *element_shape;

	ENTER(get_surface_element_segmentation);
	return_code = 0;
	if (element && (2 == get_FE_element_dimension(element)) &&
		get_FE_element_shape(element, &element_shape) && shape_type_address)
	{
		if (get_FE_element_shape_xi_shape_type(element_shape, /*xi_number*/0,
			shape_type_address))
		{
			return_code = 1;
			*collapsed_element = ELEMENT_NOT_COLLAPSED;
			*number_of_polygon_vertices = 0;
			*modified_reverse_normals = reverse_normals;
			switch (*shape_type_address)
			{
				case POLYGON_SHAPE:
				{
					/* polygon */
					if (get_FE_element_shape_xi_linkage_number(element_shape,
						/*xi_number1*/0, /*xi_number2*/1, number_of_polygon_vertices) &&
						(2 < *number_of_polygon_vertices))
					{
						*number_of_points_in_xi1=((number_of_segments_in_xi1_requested)/
							(*number_of_polygon_vertices)+1)*(*number_of_polygon_vertices)+1;
						*collapsed_element=ELEMENT_COLLAPSED_XI2_0;
						*number_of_points_in_xi2=number_of_segments_in_xi2_requested+1;
						*number_of_points =
							*number_of_points_in_xi1*(*number_of_points_in_xi2);
						*polygon_type=g_QUADRILATERAL;
					}
					else
					{
						return_code = 0;
					}
				} break;
				case SIMPLEX_SHAPE:
				{
					/* simplex */
					if (number_of_segments_in_xi1_requested >
						number_of_segments_in_xi2_requested)
					{
						*number_of_points_in_xi1=number_of_segments_in_xi1_requested+1;
						*number_of_points_in_xi2=number_of_segments_in_xi1_requested+1;
					}
					else
					{
						*number_of_points_in_xi1=number_of_segments_in_xi2_requested+1;
						*number_of_points_in_xi2=number_of_segments_in_xi2_requested+1;
					}
					*number_of_points=
						(*number_of_points_in_xi1*(*number_of_points_in_xi1+1))/2;
					*polygon_type=g_TRIANGLE;
					/* to get the surface colouring right, the points need to be given in a
						 certain order to GL_TRIANGLE_STRIP.  To keep the lighting correct,
						 the normals need to be reversed */
					if (reverse_normals)
					{
						*modified_reverse_normals=0;
					}
					else
					{
						*modified_reverse_normals=1;
					}
				} break;
				default:
				{
					*number_of_points_in_xi1 = number_of_segments_in_xi1_requested + 1;
					/* check for collapsed elements */
					if ((LINE_SHAPE == (*shape_type_address)) &&
						get_FE_element_number_of_faces(element, &number_of_faces))
					{
						for (i = 0; (i < 4) && return_code; i++)
						{
							if (i < number_of_faces)
							{
								if (!get_FE_element_face(element, i, &(faces[i])))
								{
									return_code = 0;
								}
							}
							else
							{
								faces[i] = (struct FE_element *)NULL;
							}
						}
						if (return_code)
						{
							if (!faces[0])
							{
								if (faces[1]&&faces[2]&&faces[3])
								{
									*collapsed_element=ELEMENT_COLLAPSED_XI1_0;
								}
							}
							else if (!faces[1])
							{
								if (faces[0]&&faces[2]&&faces[3])
								{
									*collapsed_element=ELEMENT_COLLAPSED_XI1_1;
								}
							}
							else if (!faces[2])
							{
								if (faces[0]&&faces[1]&&faces[3])
								{
									*collapsed_element=ELEMENT_COLLAPSED_XI2_0;
								}
							}
							else if (!faces[3])
							{
								if (faces[0]&&faces[1]&&faces[2])
								{
									*collapsed_element=ELEMENT_COLLAPSED_XI2_1;
								}
							}
						}
					}
					*number_of_points_in_xi2=number_of_segments_in_xi2_requested+1;
					*number_of_points =
						(*number_of_points_in_xi1)*(*number_of_points_in_xi2);
					*polygon_type = g_QUADRILATERAL;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_surface_element_segmentation.  Could not get shape type");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_surface_element_segmentation.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* get_surface_element_segmentation */

struct GT_surface *create_GT_surface_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *data_field,int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,char reverse_normals,
	struct FE_element *top_level_element,enum Render_type render_type,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a <GT_surface> from the <coordinate_field> for the 2-D finite <element>
using an array of <number_of_segments_in_xi1> by <number_of_segments_in_xi2>
rectangles in xi space.  The spacing is constant in each of xi1 and xi2.
The optional <texture_coordinate_field> is evaluated at each vertex for the
corresonding texture coordinates.  If none is supplied then a length based
algorithm is used instead.
The optional <data_field> (currently only a scalar) is calculated as data over
the surface, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
- checks for collapsed elements (by looking for NULL faces)
???DB.  30 July 2001.  Could be extended by
1 Adding a "surface_normal" computed_field to
	computed_field/computed_field_derivatives.  This would cache the type of
	collapsing and the collapsed normal(s) and use xi to determine if on collapsed
	face.
2 Add a normal field to "gfx create surfaces" (would be passed down to here).
This would allow the "collapsed normals" code to be shared and allow the user
to specify normal.  I didn't do it because this is the only place that collapsed
normals are used.
???DB.  Always calculates texture coordinates.  Calculate as required ?  Finite
	element graphical object ?
???DB.  18 September 1993.  Only the side with the normal pointing out is
	textured correctly (even in TWOSIDE mode).  For 2-D finite elements which
	are faces of 3-D finite elements make the normals point out of the element.
==============================================================================*/
{
	char modified_reverse_normals,special_normals;
	enum Collapsed_element_type collapsed_element;
	enum FE_element_shape_type shape_type;
	enum GT_surface_type surface_type;
	FE_value coordinates[3],derivative_xi[6],texture_values[3],xi[2],xi2,
		texture_derivative[6], texture_determinant;
	float distance;
	GTDATA *data;
	gtPolygonType polygon_type;
	struct GT_surface *surface;
	int i,j,n_data_components,number_of_points,number_of_points_in_xi1,
		number_of_points_in_xi2,number_of_polygon_vertices;
	struct CM_element_information cm;
	Triple *normal,*normalpoints,*point,*point_a,*point_b,*points,
		*tangent,*tangentpoints,*texturepoints,*texture_coordinate;

	ENTER(create_GT_surface_from_FE_element);
	if (element && (2 == get_FE_element_dimension(element)) &&
		(0<number_of_segments_in_xi1_requested)&&
		(0<number_of_segments_in_xi2_requested)&&coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field))&&
		((!texture_coordinate_field)||
		(3>=Computed_field_get_number_of_components(texture_coordinate_field))))
	{
		/* clear coordinates and derivatives not set if coordinate field is not
			 3 component */
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		derivative_xi[2]=0.0;
		derivative_xi[3]=0.0;
		derivative_xi[4]=0.0;
		derivative_xi[5]=0.0;
		/* clear texture_values not set if texture_coordinate field is not
			 3 component */
		texture_values[1]=0.0;
		texture_values[2]=0.0;
		get_surface_element_segmentation(element,
			number_of_segments_in_xi1_requested,number_of_segments_in_xi2_requested,
			reverse_normals,&number_of_points_in_xi1,&number_of_points_in_xi2,
			&number_of_points,&number_of_polygon_vertices,&polygon_type,
			&collapsed_element,&modified_reverse_normals, &shape_type);
		/* create the GT_surface */
		surface=(struct GT_surface *)NULL;
		points=(Triple *)NULL;
		normalpoints=(Triple *)NULL;
		tangentpoints=(Triple *)NULL;
		texturepoints=(Triple *)NULL;
		n_data_components = 0;
		data=(GTDATA *)NULL;
		if (data_field)
		{
			n_data_components = Computed_field_get_number_of_components(data_field);
			if (!ALLOCATE(data,GTDATA,number_of_points*n_data_components))
			{
				display_message(ERROR_MESSAGE,
					"create_GT_surface_from_FE_element.  Could allocate data");
			}
		}
		switch (render_type)
		{
			case RENDER_TYPE_WIREFRAME:
			{
				surface_type = g_WIREFRAME_SHADED_TEXMAP;
			} break;
			default:
			{
				surface_type = g_SHADED_TEXMAP;
			} break;
		}
		if ((data||(0==n_data_components))&&
			ALLOCATE(points,Triple,number_of_points)&&
			ALLOCATE(normalpoints,Triple,number_of_points)&&
			(!texture_coordinate_field || (ALLOCATE(tangentpoints,Triple,number_of_points)&&
			ALLOCATE(texturepoints,Triple,number_of_points)))&&
			(surface=CREATE(GT_surface)(surface_type,polygon_type,
			number_of_points_in_xi1,number_of_points_in_xi2, points,
			normalpoints, tangentpoints, texturepoints, n_data_components,data)))
		{
			/* for selective editing of GT_object primitives, record element ID */
			get_FE_element_identifier(element, &cm);
			surface->object_name = CM_element_information_to_graphics_name(&cm);
			/* calculate the xi coordinates and store in "normals" */
			point_a=normalpoints;
			point_b=point_a;
			distance=(FE_value)(number_of_points_in_xi1-1);
			if (SIMPLEX_SHAPE == shape_type)
			{
				for (i=0;i<number_of_points_in_xi1;i++)
				{
					(*point_a)[0]=(float)i/distance;
					point_a++;
				}
				for (j=number_of_points_in_xi2-1;j>0;j--)
				{
					for (i=0;i<j;i++)
					{
						(*point_a)[0]=(*point_b)[0];
						point_a++;
						point_b++;
					}
					point_b++;
				}
				point=normalpoints;
				distance=(float)(number_of_points_in_xi2-1);
				for (j=0;j<number_of_points_in_xi2;j++)
				{
					xi2=(float)j/distance;
					for (i=number_of_points_in_xi1-j;i>0;i--)
					{
						(*point)[1]=xi2;
						point++;
					}
				}
			}
			else
			{
				for (i=0;i<number_of_points_in_xi1;i++)
				{
					(*point_a)[0]=(float)i/distance;
					point_a++;
				}
				for (j=1;j<number_of_points_in_xi2;j++)
				{
					for (i=0;i<number_of_points_in_xi1;i++)
					{
						(*point_a)[0]=(*point_b)[0];
						point_a++;
						point_b++;
					}
				}
				point=normalpoints;
				distance=(float)(number_of_points_in_xi2-1);
				for (j=0;j<number_of_points_in_xi2;j++)
				{
					xi2=(float)j/distance;
					for (i=0;i<number_of_points_in_xi1;i++)
					{
						(*point)[1]=xi2;
						point++;
					}
				}
			}
			/* calculate the points, normals and data */
			point=points;
			normal=normalpoints;
			if (texture_coordinate_field)
			{
				tangent=tangentpoints;
				texture_coordinate=texturepoints;
			}
			if ((g_QUADRILATERAL==polygon_type)&&
				((ELEMENT_COLLAPSED_XI1_0==collapsed_element)||
				(ELEMENT_COLLAPSED_XI1_1==collapsed_element)||
				(ELEMENT_COLLAPSED_XI2_0==collapsed_element)||
				(ELEMENT_COLLAPSED_XI2_1==collapsed_element)))
			{
				special_normals=1;
			}
			else
			{
				special_normals=0;
			}
			i=0;
			while ((i<number_of_points)&&surface)
			{
				/* extract xi from the normals */
				xi[0]=(*normal)[0];
				xi[1]=(*normal)[1];
				/* evaluate the fields */
				if (Computed_field_evaluate_in_element(coordinate_field,element,xi,
					time,top_level_element,coordinates,derivative_xi)&&
					((!data_field)||Computed_field_evaluate_in_element(
					data_field,element,xi,time,top_level_element,data,
					(FE_value *)NULL))&&((!texture_coordinate_field)||
					Computed_field_evaluate_in_element(texture_coordinate_field,
					element,xi,time,top_level_element,texture_values,
					texture_derivative)))
				{
					(*point)[0]=coordinates[0];
					(*point)[1]=coordinates[1];
					(*point)[2]=coordinates[2];
					point++;
					/* calculate the normals */
					/* calculate the normal=d/d_xi1 x d/d_xi2 */
					(*normal)[0]=derivative_xi[2]*derivative_xi[5]-
						derivative_xi[3]*derivative_xi[4];
					(*normal)[1]=derivative_xi[4]*derivative_xi[1]-
						derivative_xi[5]*derivative_xi[0];
					(*normal)[2]=derivative_xi[0]*derivative_xi[3]-
						derivative_xi[1]*derivative_xi[2];
					if (texture_coordinate_field)
					{
						/* tangent is dX/d_xi * inv(dT/dxi) */
						texture_determinant = texture_derivative[0] * texture_derivative[3]
							- texture_derivative[1] * texture_derivative[2];
						if ((texture_determinant < FE_VALUE_ZERO_TOLERANCE) && 
							(texture_determinant > -FE_VALUE_ZERO_TOLERANCE))
						{
							/* Cannot invert the texture derivative so just use the first xi derivative */
							(*tangent)[0] = derivative_xi[0];
							(*tangent)[1] = derivative_xi[2];
							(*tangent)[2] = derivative_xi[4];
						}
						else
						{
							(*tangent)[0] = (derivative_xi[0] * texture_derivative[0]
								- derivative_xi[1] * texture_derivative[2]) / texture_determinant;
							(*tangent)[1] = (derivative_xi[2] * texture_derivative[0]
								- derivative_xi[3] * texture_derivative[2]) / texture_determinant;
							(*tangent)[2] = (derivative_xi[4] * texture_derivative[0]
								- derivative_xi[5] * texture_derivative[2]) / texture_determinant;
						}
						tangent++;
					}
					if (special_normals)
					{
						if (ELEMENT_COLLAPSED_XI1_0==collapsed_element)
						{
							if (0==i%number_of_points_in_xi1)
							{
								/* save xi1 derivatives, get normal from cross product of
									these */
								(*normal)[0]=derivative_xi[0];
								(*normal)[1]=derivative_xi[2];
								(*normal)[2]=derivative_xi[4];
							}
						}
						else if (ELEMENT_COLLAPSED_XI2_0==collapsed_element)
						{
							if (0==i/number_of_points_in_xi1)
							{
								/* save xi2 derivatives, get normal from cross product of
									these */
								(*normal)[0]=derivative_xi[1];
								(*normal)[1]=derivative_xi[3];
								(*normal)[2]=derivative_xi[5];
							}
						}
						else if (ELEMENT_COLLAPSED_XI1_1==collapsed_element)
						{
							if (number_of_points_in_xi2-1==i%number_of_points_in_xi1)
							{
								/* save xi1 derivatives, get normal from cross product of
									these */
								(*normal)[0]=derivative_xi[0];
								(*normal)[1]=derivative_xi[2];
								(*normal)[2]=derivative_xi[4];
							}
						}
						else if (ELEMENT_COLLAPSED_XI2_1==collapsed_element)
						{
							if (number_of_points_in_xi2-1==i/number_of_points_in_xi1)
							{
								/* save xi2 derivatives, get normal from cross product of
									these */
								(*normal)[0]=derivative_xi[1];
								(*normal)[1]=derivative_xi[3];
								(*normal)[2]=derivative_xi[5];
							}
						}
					}
					normal++;
					if (data_field)
					{
						data+=n_data_components;
					}
					if (texture_coordinate_field)
					{
						(*texture_coordinate)[0]=texture_values[0];
						(*texture_coordinate)[1]=texture_values[1];
						(*texture_coordinate)[2]=texture_values[2];
						texture_coordinate++;
					}
				}
				else
				{
					/* error evaluating fields */
					DESTROY(GT_surface)(&surface);
				}
				i++;
			}
			/* clear Computed_field caches so elements not accessed */
			Computed_field_clear_cache(coordinate_field);
			if (data_field)
			{
				Computed_field_clear_cache(data_field);
			}
			if (surface)
			{
				if (special_normals)
				{
					if (number_of_polygon_vertices>0)
					{
						normal=normalpoints+number_of_points_in_xi1-2;
						derivative_xi[1]=(*normal)[0];
						derivative_xi[3]=(*normal)[1];
						derivative_xi[5]=(*normal)[2];
						normal=normalpoints;
						for (i=number_of_points_in_xi1;i>0;i--)
						{
							derivative_xi[0]=(*normal)[0];
							derivative_xi[2]=(*normal)[1];
							derivative_xi[4]=(*normal)[2];
							(*normal)[0]=derivative_xi[2]*derivative_xi[5]-
								derivative_xi[3]*derivative_xi[4];
							(*normal)[1]=derivative_xi[4]*derivative_xi[1]-
								derivative_xi[5]*derivative_xi[0];
							(*normal)[2]=derivative_xi[0]*derivative_xi[3]-
								derivative_xi[1]*derivative_xi[2];
							derivative_xi[1]=derivative_xi[0];
							derivative_xi[3]=derivative_xi[2];
							derivative_xi[5]=derivative_xi[4];
							normal++;
						}
					}
					else if (ELEMENT_COLLAPSED_XI1_0==collapsed_element)
					{
						/* calculate the normals for the xi1=0 edge */
						normal=normalpoints+((number_of_points_in_xi2-1)*
							number_of_points_in_xi1);
						derivative_xi[0]=(*normal)[0];
						derivative_xi[2]=(*normal)[1];
						derivative_xi[4]=(*normal)[2];
						normal=normalpoints;
						derivative_xi[1]=(*normal)[0];
						derivative_xi[3]=(*normal)[1];
						derivative_xi[5]=(*normal)[2];
						(*normal)[0]=derivative_xi[2]*derivative_xi[5]-
							derivative_xi[3]*derivative_xi[4];
						(*normal)[1]=derivative_xi[4]*derivative_xi[1]-
							derivative_xi[5]*derivative_xi[0];
						(*normal)[2]=derivative_xi[0]*derivative_xi[3]-
							derivative_xi[1]*derivative_xi[2];
						derivative_xi[0]=(*normal)[0];
						derivative_xi[1]=(*normal)[1];
						derivative_xi[2]=(*normal)[2];
						for (i=number_of_points_in_xi2-1;i>0;i--)
						{
							normal += number_of_points_in_xi1;
							(*normal)[0]=derivative_xi[0];
							(*normal)[1]=derivative_xi[1];
							(*normal)[2]=derivative_xi[2];
						}
					}
					else if (ELEMENT_COLLAPSED_XI2_0==collapsed_element)
					{
						/* calculate the normals for the xi2=0 edge */
						normal=normalpoints+(number_of_points_in_xi1-1);
						derivative_xi[0]=(*normal)[0];
						derivative_xi[2]=(*normal)[1];
						derivative_xi[4]=(*normal)[2];
						normal=normalpoints;
						derivative_xi[1]=(*normal)[0];
						derivative_xi[3]=(*normal)[1];
						derivative_xi[5]=(*normal)[2];
						(*normal)[0]=derivative_xi[2]*derivative_xi[5]-
							derivative_xi[3]*derivative_xi[4];
						(*normal)[1]=derivative_xi[4]*derivative_xi[1]-
							derivative_xi[5]*derivative_xi[0];
						(*normal)[2]=derivative_xi[0]*derivative_xi[3]-
							derivative_xi[1]*derivative_xi[2];
						derivative_xi[0]=(*normal)[0];
						derivative_xi[1]=(*normal)[1];
						derivative_xi[2]=(*normal)[2];
						for (i=number_of_points_in_xi1-1;i>0;i--)
						{
							normal++;
							(*normal)[0]=derivative_xi[0];
							(*normal)[1]=derivative_xi[1];
							(*normal)[2]=derivative_xi[2];
						}
					}
					else if (ELEMENT_COLLAPSED_XI1_1==collapsed_element)
					{
						/* calculate the normals for the xi1=1 edge */
						normal=normalpoints+(number_of_points_in_xi1*
							number_of_points_in_xi2-1);
						derivative_xi[1]=(*normal)[0];
						derivative_xi[3]=(*normal)[1];
						derivative_xi[5]=(*normal)[2];
						normal=normalpoints+(number_of_points_in_xi1-1);
						derivative_xi[0]=(*normal)[0];
						derivative_xi[2]=(*normal)[1];
						derivative_xi[4]=(*normal)[2];
						(*normal)[0]=derivative_xi[2]*derivative_xi[5]-
							derivative_xi[3]*derivative_xi[4];
						(*normal)[1]=derivative_xi[4]*derivative_xi[1]-
							derivative_xi[5]*derivative_xi[0];
						(*normal)[2]=derivative_xi[0]*derivative_xi[3]-
							derivative_xi[1]*derivative_xi[2];
						derivative_xi[0]=(*normal)[0];
						derivative_xi[1]=(*normal)[1];
						derivative_xi[2]=(*normal)[2];
						for (i=number_of_points_in_xi2-1;i>0;i--)
						{
							normal += number_of_points_in_xi1;
							(*normal)[0]=derivative_xi[0];
							(*normal)[1]=derivative_xi[1];
							(*normal)[2]=derivative_xi[2];
						}
					}
					else if (ELEMENT_COLLAPSED_XI2_1==collapsed_element)
					{
						/* calculate the normals for the xi2=1 edge */
						normal=normalpoints+(number_of_points_in_xi1*
							number_of_points_in_xi2-1);
						derivative_xi[1]=(*normal)[0];
						derivative_xi[3]=(*normal)[1];
						derivative_xi[5]=(*normal)[2];
						normal=normalpoints+(number_of_points_in_xi1*
							(number_of_points_in_xi2-1));
						derivative_xi[0]=(*normal)[0];
						derivative_xi[2]=(*normal)[1];
						derivative_xi[4]=(*normal)[2];
						(*normal)[0]=derivative_xi[2]*derivative_xi[5]-
							derivative_xi[3]*derivative_xi[4];
						(*normal)[1]=derivative_xi[4]*derivative_xi[1]-
							derivative_xi[5]*derivative_xi[0];
						(*normal)[2]=derivative_xi[0]*derivative_xi[3]-
							derivative_xi[1]*derivative_xi[2];
						derivative_xi[0]=(*normal)[0];
						derivative_xi[1]=(*normal)[1];
						derivative_xi[2]=(*normal)[2];
						for (i=number_of_points_in_xi1-1;i>0;i--)
						{
							normal++;
							(*normal)[0]=derivative_xi[0];
							(*normal)[1]=derivative_xi[1];
							(*normal)[2]=derivative_xi[2];
						}
					}
				}
				/* normalize the normals */
				normal=normalpoints;
				if (modified_reverse_normals)
				{
					for (i=number_of_points;i>0;i--)
					{
						if (0.0<(distance=sqrt((*normal)[0]*(*normal)[0]+
							(*normal)[1]*(*normal)[1]+(*normal)[2]*(*normal)[2])))
						{
							(*normal)[0] = -(*normal)[0]/distance;
							(*normal)[1] = -(*normal)[1]/distance;
							(*normal)[2] = -(*normal)[2]/distance;
						}
						normal++;
					}
				}
				else
				{
					for (i=number_of_points;i>0;i--)
					{
						if (0.0<(distance=sqrt((*normal)[0]*(*normal)[0]+
							(*normal)[1]*(*normal)[1]+(*normal)[2]*(*normal)[2])))
						{
							(*normal)[0] /= distance;
							(*normal)[1] /= distance;
							(*normal)[2] /= distance;
						}
						normal++;
					}
				}
				if (texture_coordinate_field)
				{
					/* normalize the tangents */
					tangent=tangentpoints;
					for (i=number_of_points;i>0;i--)
					{
						if (0.0<(distance=sqrt((*tangent)[0]*(*tangent)[0]+
										(*tangent)[1]*(*tangent)[1]+(*tangent)[2]*(*tangent)[2])))
						{
							(*tangent)[0] /= distance;
							(*tangent)[1] /= distance;
							(*tangent)[2] /= distance;
						}
						tangent++;
					}
				}
			}
		}
		else
		{
			DEALLOCATE(points);
			DEALLOCATE(normalpoints);
			if (tangentpoints)
			{
				DEALLOCATE(tangentpoints);
			}
			if (texturepoints)
			{
				DEALLOCATE(texturepoints);
			}
			if (data)
			{
				DEALLOCATE(data);
			}
		}
		if (!surface)
		{
			display_message(ERROR_MESSAGE,
				"create_GT_surface_from_FE_element.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_surface_from_FE_element.  Invalid argument(s)");
		surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (surface);
} /* create_GT_surface_from_FE_element */

int Set_element_and_local_xi(struct FE_element **element_block,
	int *n_xi, FE_value *xi, struct FE_element **element)
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Uses the global xi to select an element from the <element_block> and
returns this pointer in <element>.  The <n_xi> represent the maximum number of
elements in each of the three directions.
The <xi> are set to their local values within the returned <element>.
==============================================================================*/
{
	int a, b, c, return_code;

	ENTER(Set_element_and_local_xi);
	if (element_block&&n_xi&&xi&&element)
	{
		a=(int)(floor(((float)xi[0])));
		b=(int)(floor(((float)xi[1])));
		c=(int)(floor(((float)xi[2])));
		/* set any slight outliers to boundary of volume */
		if (a>n_xi[0]-1)
		{
			a=n_xi[0]-1;
		}
		if (b>n_xi[1]-1)
		{
			b=n_xi[1]-1;
		}
		if (c>n_xi[2]-1)
		{
			c=n_xi[2]-1;
		}
		if (a<0)
		{
			a=0;
		}
		if (b<0)
		{
			b=0;
		}
		if (c<0)
		{
			c=0;
		}
		xi[0] -= ((FE_value)a);
		xi[1] -= ((FE_value)b);
		xi[2] -= ((FE_value)c);
	
		*element = element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Set_element_and_local_xi.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
}

struct GT_voltex *create_GT_voltex_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	struct VT_volume_texture *vtexture, enum Render_type render_type,
	struct Computed_field *displacement_field, int displacement_map_xi_direction,
	struct Computed_field *blur_field, struct Computed_field *texture_coordinate_field,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a <GT_voltex> from a 3-D finite <element> <block> and volume texture
The volume texture contains a list of coloured triangular polygons representing
an isosurface calculated from the volumetric data. These polygons are stored in
local xi1,2,3 space and undergo a free form deformation (FFD) when mapped onto
the finite <element> <block>. The output contains the deformed polygon & normal
list.  Only the vertices are transformed. The polygons are then generated from a
list of pointers to the vertices.  Normals are calculated from the resulting
deformed vertices by taking the average of the cross products of the surrounding
faces.
==============================================================================*/
{
	enum GT_voltex_type voltex_type;
	FE_value *coordinate_field_derivatives,coordinate_1,coordinate_2,coordinate_3,
		*data,distance,minimum_distance,xi[3],xi_step[3],xi_stepsize[3],
		/*???DB.  Merge */
		rmag,vector1[3],vector2[3],result[3],vectorsum[3],vertex0[3],vertex1[3],
		vertex2[3];
	FE_value colour_data[4];
	int *adjacency_table,data_index,*deform,i,ii,j,jj,k,kk,m,n_iso_polys,
		n_data_components,number_of_elements,number_of_faces,n_xi_rep[3],return_code,
		/*???DB.  Merge ?  Better names ? */
		a,aaa,b,bbb,c,ccc,n_xi[3],index,triangle,v_count,n_vertices,
		*triangle_list,*triangle_list2,acc_triangle_index,
		*dont_draw;
	struct CM_element_information cm;
	struct Environment_map *environment_map;
	struct FE_element **element_block,**element_block_ptr;
	struct Graphical_material *material;
	struct GT_voltex *voltex;
	struct VT_iso_vertex *vertex_list;

	double *iso_poly_cop;
	float *texturemap_coord;
	int *texturemap_index;
	int tex_x_i,tex_y_i,tex_width_texels,tex_height_texels,
		tex_number_of_components;
	double intensity1,intensity2,intensity3;
	int outside_block;
	int vectorsumc, blur = 1;

	ENTER(create_GT_voltex_from_FE_element);
#if defined (DEBUG)
	/*???debug */
	printf("enter create_GT_voltex_from_FE_element\n");
#endif /* defined (DEBUG) */
	voltex=(struct GT_voltex *)NULL;
	if (element && (3 == get_FE_element_dimension(element)) && vtexture &&
		coordinate_field&&
		Computed_field_has_3_components(coordinate_field,NULL)&&
		((!blur_field)||
			Computed_field_has_up_to_4_numerical_components(blur_field,NULL))&&
		((!displacement_field)||
			Computed_field_has_up_to_4_numerical_components(displacement_field,NULL)))
	{
		if (vtexture->mc_iso_surface->n_triangles)
		{
			if (displacement_field&&((12==displacement_map_xi_direction)||
				(13==displacement_map_xi_direction)||
				(23==displacement_map_xi_direction)))
			{
				if (ALLOCATE(coordinate_field_derivatives,FE_value,9))
				{
					return_code=1;
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,
						"create_GT_voltex_from_FE_element.  "
						"Insufficient memory for coordinate_field_derivatives");
				}
			}
			else
			{
				coordinate_field_derivatives=(FE_value *)NULL;
				return_code=1;
			}
			if (return_code)
			{
				deform=vtexture->mc_iso_surface->deform;
				/* examine min & max xi values of voltex. If 0<xi<1 possibly repeat.  If
					xi>1 then will need to draw over other elements, so must find
					neighbouring ones */
					/*???MS.  27 August 1996.  At present put in a hack to subtrat the
						vtexture->ximin values of the xi values.  Should formalize this
						later */
				number_of_elements=1;
				for (i=0;i<3;i++)
				{
					/* MACRO TEXTURES : spread over several elements */
					/* round to nearest integer above if non integer (eg 1->1,1.2 ->2) */
					n_xi[i]=(int)ceil(vtexture->ximax[i]);
#if defined (OLD_CODE)
					/* SAB 2003 I don't really want this to repeat when I subdivide an element 
						so that the computation is spread out */
					/* MICRO TEXTURES : repeated within one element */
					/* if texture is to be repeated then calc # of repetitions for each
						value */
					if (vtexture->ximax[i] < 1.0)
					{
						n_xi_rep[i]=(int) floor(0.5+1.0/vtexture->ximax[i]);
						xi_stepsize[i]=(double) (1.0/((double)n_xi_rep[i]));
					}
					else
#endif /* defined (OLD_CODE) */
					{
						n_xi_rep[i]=1;
						xi_stepsize[i]=1.0;
					}
					number_of_elements *= n_xi[i]; /* n_xi_rep[i]? */
				}
				/* allocate memory for elements */
				ALLOCATE(element_block,struct FE_element *,number_of_elements);
				ALLOCATE(adjacency_table,int,6*number_of_elements);
				if (data_field)
				{
					n_data_components = Computed_field_get_number_of_components(data_field);
				}
				else
				{
					n_data_components = 0;
				}
				if (element_block&&adjacency_table)
				{
					/* pick seed element starting at minimum xi1,2,3 position (ie give
						closest bottom leftmost element ) */
					/* calculate elements sitting in block which texture extends into */
					/* count the number of faces */
						/*???DB.  Where is the counting ? */
					number_of_faces=number_of_elements*6;
						/*???MS.  number_of_elements*6 */
						/*???DB.  Not necessarily */
					element_block_ptr=element_block;
					for (i=0;i<n_xi[0];i++)
					{
						for (j=0;j<n_xi[1];j++)
						{
							for (k=0;k<n_xi[2];k++)
							{
								element_block_ptr[k*n_xi[0]*n_xi[1]+j*n_xi[0]+i]=
									(struct FE_element *)NULL;
								for (m=0;m<6;m++)
								{
									adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+m]=0;
								}
							}
						}
					}
					/* recursively filling element block */
					fill_table(element_block,adjacency_table,element,0,0,0,n_xi);
					ALLOCATE(dont_draw,int,number_of_faces);
					if (dont_draw)
					{
						/* coordinates calculated using Q=G(geometry vector).
							M(blending matrix).T(monomial basis (of xi values)) */
						/* calculate G.M [using calc_FE_elem_field_vals] (to multiply the
							monomial basis T) for each element in the block - when an xi
							coordinate in the vertex list extends over the first element, the
							next element is used */
						return_code=1;
						/* check the voltex can be completely drawn inside mesh */
						for (i=0;return_code&&(i<number_of_elements);i++)
						{
							if (!element_block[i])
							{
								display_message(WARNING_MESSAGE,
									"create_GT_voltex_from_FE_element.  "
									"voltex extends beyond elements");
								return_code=0;
							}
						}
#if defined (DONT_DRAW)
						i=number_of_elements;
						while (return_code&&(i>0))
						{
							element_ptr=element_block[i-1];
							if (element_ptr)
							{
								/* calculate whether element has face visible by checking each
									of its faces for the number of parents. If 1 => external.
									If > 1 => internal */
								for (j=(*element_block_ptr)->shape->number_of_faces;j>0;)
								{
									j--;
									if (NUMBER_IN_LIST(FE_element_parent)(
										((*element_block_ptr)->faces)[j]->parent_list) > 1)
									{
										/* this face is internal => dont draw it: record
											iso_poly_indices not to draw between */
										*dont_draw_ptr=1;
									}
									else
									{
										*dont_draw_ptr=0;
									}
									dont_draw_ptr++;
								}
							}
							else
							{
								*dont_draw_ptr=0;
								dont_draw_ptr++;
							}
							i--;
						}
#endif /* defined (DONT_DRAW) */
						if (return_code)
						{
							/* allocate memory for the VOLTEX */
							n_iso_polys = vtexture->mc_iso_surface->n_triangles;
							n_vertices = vtexture->mc_iso_surface->n_vertices;
							ALLOCATE(triangle_list, int, 3*n_iso_polys);
							ALLOCATE(triangle_list2, int, 3*n_iso_polys);
							ALLOCATE(iso_poly_cop, double, 3*n_iso_polys*3);
							ALLOCATE(texturemap_coord, float, 3*n_iso_polys*3);
							ALLOCATE(texturemap_index, int, 3*n_iso_polys);
							ALLOCATE(vertex_list, struct VT_iso_vertex,
								n_vertices*n_xi_rep[0]*n_xi_rep[1]*n_xi_rep[2]);
							/*???DB.  n_iso_polys=n_xi_rep[0]*n_xi_rep[1]*n_xi_rep[2] */
							if (data_field)
							{
								ALLOCATE(data, GTDATA, n_vertices*n_xi_rep[0]*n_xi_rep[1]*
									n_xi_rep[2]*n_data_components);
							}
							else
							{
								n_data_components = 0;
								data = (GTDATA *)NULL;
							}
							if (!(triangle_list && triangle_list2 && iso_poly_cop &&
								texturemap_coord && texturemap_index && vertex_list &&
								((!data_field) || data)))
							{
								display_message(ERROR_MESSAGE,
									"create_GT_voltex_from_FE_element.  "
									"Could not allocate memory for points");
								/*???debug*/
								display_message(ERROR_MESSAGE,
									"n_iso_polys=%d, n_vertices=%d, n_xi_rep[0]=%d, "
									"n_xi_rep[1]=%d, n_xi_rep[2]=%d\n",
									n_iso_polys,n_vertices,n_xi_rep[0],n_xi_rep[1],n_xi_rep[2]);
								return_code = 0;
							}
							switch (render_type)
							{
								case RENDER_TYPE_SHADED:
								{
									voltex_type = g_VOLTEX_SHADED_TEXMAP;
								} break;
								case RENDER_TYPE_WIREFRAME:
								{
									voltex_type = g_VOLTEX_WIREFRAME_SHADED_TEXMAP;
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"create_GT_voltex_from_FE_element.  Unknown render type");
									return_code = 0;
								} break;
							}
							if (return_code)
							{
								if (!(voltex = CREATE(GT_voltex)(n_iso_polys ,n_vertices,
									triangle_list2, vertex_list, iso_poly_cop,
									texturemap_coord, texturemap_index,
									n_xi_rep[0]*n_xi_rep[1]*n_xi_rep[2] ,n_data_components,
									data, voltex_type)))
								{
									display_message(ERROR_MESSAGE,
										"create_GT_voltex_from_FE_element.  "
										"Could not create voltex");
									return_code = 0;
								}
							}
							if (!voltex)
							{
								DEALLOCATE(triangle_list);
								DEALLOCATE(triangle_list2);
								DEALLOCATE(iso_poly_cop);
								DEALLOCATE(texturemap_coord);
								DEALLOCATE(texturemap_index);
								DEALLOCATE(vertex_list);
								if (data)
								{
									DEALLOCATE(data);
								}
							}
							if (voltex)
							{
								/* for selective editing of GT_object primitives,
									 record element ID */
								get_FE_element_identifier(element, &cm);
								voltex->object_name =
									CM_element_information_to_graphics_name(&cm);

								/*???Mark.  THIS CODE NEEDS TO BE DOCTORED.  begin */
								/* copy volume texture values into triangle_list */
								/* acceptable triangle index */
								acc_triangle_index=0;
#if defined (DEBUG)
								/*???debug */
								printf("create_GT_voltex_from_FE_element.  copying %d polys\n",
									n_iso_polys);
#endif /* defined (DEBUG) */
								for (i=0;i<n_iso_polys;i++)
								{
									/* need this list for calculating normals from surrounds */
									for (j=0;j<3;j++)
									{
										triangle_list[3*i+j]=vtexture->mc_iso_surface->
											compiled_triangle_list[i]->vertex_index[j];
#if defined (DEBUG)
										/*???debug */
										if (triangle_list[3*i+j] > n_vertices)
										{
											printf("n_xirep = %d %d %d\n",n_xi_rep[0],n_xi_rep[1],
												n_xi_rep[2]);
											printf("######### ERROR:triangle_list[3*%d+%d] > %d\n",i,
												j,n_vertices);
										}
#endif /* defined (DEBUG) */
									}
								}
								for (i=0;i<n_iso_polys;i++)
								{
									/* dont use polys on face */
									/*???RC This comment has a preservation orfer on it */
									/*???Mark.  This needs to be redone - have to think of way of
									  doing this properly - want to only copy triangles not on
									  face ! taint triangle_list and do to that? so if triangle
									  list see bit of paper */
									/*???Mark.  fix this! */
#if defined (OLD_CODE)
									if (((i>=0)&&(i<vtexture->iso_surface->xi_face_poly_index[0])&&(dont_draw[0]!=1))||
										((i>=vtexture->mc_iso_surface->xi_face_poly_index[0])&&
											(i<vtexture->mc_iso_surface->xi_face_poly_index[1])&&
											(dont_draw[1]!=1))||
										((i>=vtexture->mc_iso_surface->xi_face_poly_index[1])&&
											(i<vtexture->mc_iso_surface->xi_face_poly_index[2])&&
											(dont_draw[2]!=1))||
										((i>=vtexture->mc_iso_surface->xi_face_poly_index[2])&&
											(i<vtexture->mc_iso_surface->xi_face_poly_index[3])&&
											(dont_draw[3]!=1))||
										((i>=vtexture->mc_iso_surface->xi_face_poly_index[3])&&
											(i<vtexture->mc_iso_surface->xi_face_poly_index[4])&&
											(dont_draw[4]!=1))||
										((i>=vtexture->mc_iso_surface->xi_face_poly_index[4])&&
											(i<vtexture->mc_iso_surface->xi_face_poly_index[5])&&
											(dont_draw[5]!=1)))
									{
#endif /* defined (OLD_CODE) */

										for (j = 0; j < 3; j++)
										{
											triangle_list2[3*acc_triangle_index+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->vertex_index[j];
											if (material = vtexture->mc_iso_surface->
												compiled_triangle_list[i]->material[j])
											{
												GT_voltex_set_triangle_vertex_material(voltex,
													acc_triangle_index, j, material);
											}
											if (environment_map = vtexture->mc_iso_surface->
												compiled_triangle_list[i]->env_map[j])
											{
												GT_voltex_set_triangle_vertex_environment_map(voltex,
													acc_triangle_index, j, environment_map);
											}
											texturemap_index[3*acc_triangle_index+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->env_map_index[j];
											texturemap_coord[3*(3*acc_triangle_index+0)+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->texture_coord[0][j];
											texturemap_coord[3*(3*acc_triangle_index+1)+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->texture_coord[1][j];
											texturemap_coord[3*(3*acc_triangle_index+2)+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->texture_coord[2][j];
											iso_poly_cop[3*(3*acc_triangle_index+0)+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->iso_poly_cop[0][j];
											iso_poly_cop[3*(3*acc_triangle_index+1)+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->iso_poly_cop[1][j];
											iso_poly_cop[3*(3*acc_triangle_index+2)+j]=
												vtexture->mc_iso_surface->
												compiled_triangle_list[i]->iso_poly_cop[2][j];
										}
										acc_triangle_index++;
#if defined (OLD_CODE)
									}
#endif /* defined (OLD_CODE) */
								}
#if defined (DEBUG)
								/*???debug */
								printf("n_iso_polys = %d acc_triangle_index = %d\n",n_iso_polys,
									acc_triangle_index);
#endif /* defined (DEBUG) */
								voltex->n_iso_polys=acc_triangle_index;
								for (i=0;i<n_vertices;i++)
								{
									vertex_list[i].n_ptrs=(vtexture->mc_iso_surface->
										compiled_vertex_list)[i]->n_triangle_ptrs;
									for (j=0;j<vertex_list[i].n_ptrs;j++)
									{
										vertex_list[i].ptrs[j]=(vtexture->mc_iso_surface->
											compiled_vertex_list[i]->triangle_ptrs)[j]->
											triangle_index;
									}
									vertex_list[i].data_index = 0;
									vertex_list[i].blur=0;
								}
								data_index = 0;
								/*???Mark.  THIS CODE NEEDS TO BE DOCTORED.  end */
								/* calculate the points in the specified coordinate system */
								v_count=0;
								ii=n_xi_rep[0];
								xi_step[0]=0;
								while (voltex&&(ii>0))
								{
									jj=n_xi_rep[1];
									xi_step[1]=0;
									while (voltex&&(jj>0))
									{
										kk=n_xi_rep[2];
										xi_step[2]=0;
										while (voltex&&(kk>0))
										{
											i=0;
											while (voltex&&(i<n_vertices))
											{
												/* get triangle coord values from volume texture */
												/* vertex */
												xi[0]=(FE_value)((vtexture->mc_iso_surface->
													compiled_vertex_list)[i]->coord)[0]+xi_step[0];
												xi[1]=(FE_value)((vtexture->mc_iso_surface->
													compiled_vertex_list)[i]->coord)[1]+xi_step[1];
												xi[2]=(FE_value)((vtexture->mc_iso_surface->
													compiled_vertex_list)[i]->coord)[2]+xi_step[2];
												/* here it must be decided which element the xi point
													exists in.  Once the element is chosen, the xi value
													is renormalized to lie within that element */
												/* if there is no adjacent element in the xi direction
													1. End of block => extrapolate from last element
													2. ???MS.  Not implemented yet
													Slit or hole => extrapolate from nearest element
													e.g xi < 0.5 */
												/* the element indices */
												a=(int)(floor(((float)xi[0])));
												b=(int)(floor(((float)xi[1])));
												c=(int)(floor(((float)xi[2])));
#if defined (OLD_CODE)
												if (a>n_xi[0]-1)
												{
													a=n_xi[0]-1;
												}
												if (b>n_xi[1]-1)
												{
													b=n_xi[1]-1;
												}
												if (c>n_xi[2]-1)
												{
													c=n_xi[2]-1;
												}
												if (a < 0)
												{
													a = 0;
												}
												if (b < 0)
												{
													b = 0;
												}
												if (c < 0)
												{
													c = 0;
												}
#endif /* defined (OLD_CODE) */
												/* if a vertex is outside of the element block, then
													do not deform it */
												outside_block=0;
												/* if not deformable material point, treat as outside
													block (i.e. do not deform) */
												if (deform!=NULL)
												{
													if (deform[i])
													{
														outside_block=0;
													}
													else
													{
														outside_block=1;
													}
													if (a>n_xi[0]-1)
													{
														outside_block=1;
													}
													if (b>n_xi[1]-1)
													{
														outside_block=1;
													}
													if (c>n_xi[2]-1)
													{
														outside_block=1;
													}
													if (a<0)
													{
														outside_block=1;
													}
													if (b<0)
													{
														outside_block=1;
													}
													if (c<0)
													{
														outside_block=1;
													}
												}
												else
												{
													/* set any slight outliers to boundary of volume */
													if (a>n_xi[0]-1)
													{
														a=n_xi[0]-1;
													}
													if (b>n_xi[1]-1)
													{
														b=n_xi[1]-1;
													}
													if (c>n_xi[2]-1)
													{
														c=n_xi[2]-1;
													}
													if (a<0)
													{
														a=0;
													}
													if (b<0)
													{
														b=0;
													}
													if (c<0)
													{
														c=0;
													}
												}
												if (!outside_block)
												{
													/* normalize the xi values to lie in element
														[a][b][c] */
													if (!(element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a]))
													{
														/* find nearest non-null element to xi */
														minimum_distance=(xi[0])*(xi[0])+(xi[1])*(xi[1])+
															(xi[2])*(xi[2]);
														for (aaa=0;aaa<n_xi[0];aaa++)
														{
															for (bbb=0;bbb<n_xi[1];bbb++)
															{
																for (ccc=0;ccc<n_xi[2];ccc++)
																{
																	distance=(xi[0]-aaa)*(xi[0]-aaa)+
																		(xi[1]-bbb)*(xi[1]-bbb)+
																		(xi[2]-ccc)*(xi[2]-ccc);
																	if ((distance<minimum_distance)&&
																		element_block[ccc*n_xi[0]*n_xi[1]+
																			bbb*n_xi[0]+aaa])
																	{
																		minimum_distance=distance;
																		a=aaa;
																		b=bbb;
																		c=ccc;
																	}
																}
															}
														}
													}
													xi[0] -= ((FE_value)a);
													xi[1] -= ((FE_value)b);
													xi[2] -= ((FE_value)c);
													/* displacement map */
													if (displacement_field)
													{
														tex_number_of_components=
															Computed_field_get_number_of_components(
																displacement_field);
														Computed_field_evaluate_in_element(
															displacement_field,
															element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a],
															xi,time,(struct FE_element *)NULL,
															colour_data,(FE_value *)NULL);
														intensity1 = colour_data[0];
														intensity2 = 0;
														intensity3 = 0;
														if (tex_number_of_components > 1)
														{
															intensity2 = colour_data[1];
															if (tex_number_of_components > 2)
															{
																intensity3 = colour_data[2];
															}
														}
														/* stop any boundary displacement */
														if ((xi[0]+a==0)||(xi[0]+a==n_xi[0])||
															(xi[1]+b==0)||(xi[1]+b==n_xi[1]))
														{
#if defined (DEBUG)
															/*???debug */
															printf("Hit boundary at %lf %lf %lf\n",
																(xi[0]+a), (xi[1]+b), (xi[2]+c));
#endif /* defined (DEBUG) */
															intensity1=0;
															intensity2=0;
															intensity3=0;
														}
														/* ignore border */
														if ((tex_x_i>0)&&(tex_x_i<tex_width_texels)&&
															(tex_y_i>0)&&(tex_y_i<tex_height_texels))
														{
															/* For 12, 13 and 23 we displace after evaluation in
																the direction of the derivative cross products */
															if ((1==displacement_map_xi_direction)||
																(2==displacement_map_xi_direction)||
																(3==displacement_map_xi_direction))
															{
																xi[displacement_map_xi_direction-1] += intensity1;
															}
														}
														/* Keep intensity2 and intensity3 in case we do some
															other displacement stuff with them */
														USE_PARAMETER(intensity2);
														USE_PARAMETER(intensity3);
													}
													/* texture coordinates */
													if (texture_coordinate_field)
													{
														tex_number_of_components=
															Computed_field_get_number_of_components(
																texture_coordinate_field);
														Computed_field_evaluate_in_element(
															texture_coordinate_field,
															element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a],
															xi,time,(struct FE_element *)NULL,
															vertex_list[i+n_vertices*v_count].texture_coordinates,
															(FE_value *)NULL);
														if (tex_number_of_components < 3)
														{
															vertex_list[i+n_vertices*v_count].texture_coordinates[2]
																= 0;
															if (tex_number_of_components < 2)
															{
																vertex_list[i+n_vertices*v_count].texture_coordinates[1]
																	= 0;
															}
														}
													}
													if (blur_field)
													{
														if ((xi[0]+a==0)||(xi[0]+a==n_xi[0])||
															(xi[1]+b==0)||(xi[1]+b==n_xi[1]))
														{
															/* Don't touch the boundary */
															vertex_list[i+n_vertices*v_count].blur = 0;
														}
														else
														{
															Computed_field_evaluate_in_element(
																blur_field,
																element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a],
																xi,time,(struct FE_element *)NULL,
																colour_data,(FE_value *)NULL);
															vertex_list[i+n_vertices*v_count].blur =
																(unsigned char)colour_data[0];
														}
													}
													/* coordinate_n_value contains the blended matrix
														(M.G (m=blending (eg Cubic Hermite),G=geometry)
														for each element dimension. To calculate the
														monomial values (dependent on xi1,2,3) the
														standard basis fn (which happens to be monomial
														(T(xi1,2,3)) in this case) is called. This need
														only be calculated once for each set of xi coords
														and then used for each coordinate by obtaining the
														dot product (Q(xi1,2,3)=M.G.T) <I think this is
														actually Q=G.M.T> */
													index=i+n_vertices*v_count;
													if (Computed_field_evaluate_in_element(
														coordinate_field,
														element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a],
														xi,time,(struct FE_element *)NULL,
														(FE_value *)(vertex_list[index].coord),
														coordinate_field_derivatives))
													{
														/* record original xi values of these for 2 & 3d
															texture mapping applications */
														/*???DB.  To be done ? */
														if ((12==displacement_map_xi_direction)||
															(13==displacement_map_xi_direction)||
															(23==displacement_map_xi_direction))
														{
															if (coordinate_field_derivatives)
															{
																/* For the other displacements we displace the 
																	xi coordinates (coordinate_field_derivatives IS NULL) */
																switch (displacement_map_xi_direction)
																{
																	case 13:
																	{
																		coordinate_field_derivatives[1]=
																			coordinate_field_derivatives[2];
																		coordinate_field_derivatives[4]=
																			coordinate_field_derivatives[5];
																		coordinate_field_derivatives[7]=
																			coordinate_field_derivatives[8];
																	} break;
																	case 23:
																	{
																		coordinate_field_derivatives[0]=
																			coordinate_field_derivatives[1];
																		coordinate_field_derivatives[3]=
																			coordinate_field_derivatives[4];
																		coordinate_field_derivatives[6]=
																			coordinate_field_derivatives[7];
																		coordinate_field_derivatives[1]=
																			coordinate_field_derivatives[2];
																		coordinate_field_derivatives[4]=
																			coordinate_field_derivatives[5];
																		coordinate_field_derivatives[7]=
																			coordinate_field_derivatives[8];
																	} break;
																}
																coordinate_1=
																	coordinate_field_derivatives[3]*
																	coordinate_field_derivatives[7]-
																	coordinate_field_derivatives[4]*
																	coordinate_field_derivatives[6];
																coordinate_2=
																	coordinate_field_derivatives[6]*
																	coordinate_field_derivatives[1]-
																	coordinate_field_derivatives[0]*
																	coordinate_field_derivatives[7];
																coordinate_3=
																	coordinate_field_derivatives[0]*
																	coordinate_field_derivatives[4]-
																	coordinate_field_derivatives[1]*
																	coordinate_field_derivatives[3];
																vertex_list[index].coord[0] += coordinate_1*intensity1;
																vertex_list[index].coord[1] += coordinate_2*intensity1;
																vertex_list[index].coord[2] += coordinate_3*intensity1;
															}
														}
														if (data_field)
														{
															if (Computed_field_evaluate_in_element(data_field,
																element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a],
																xi,time,(struct FE_element *)NULL,
																data+data_index, (FE_value *)NULL))
															{
																vertex_list[index].data_index = data_index;
																data_index+=n_data_components;
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"create_GT_voltex_from_FE_element.  Error calculating data field");
																DESTROY(GT_voltex)(&voltex);
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"create_GT_voltex_from_FE_element.  Error calculating coordinate field");
														DESTROY(GT_voltex)(&voltex);
													}
												} /* if outside_block */
												else
												{
													index=i+n_vertices*v_count;
													vertex_list[index].coord[0]=((vtexture->
														mc_iso_surface->compiled_vertex_list)[i]->
														coord)[0];
													vertex_list[index].coord[1]=((vtexture->
														mc_iso_surface->compiled_vertex_list)[i]->
														coord)[1];
													vertex_list[index].coord[2]=((vtexture->
														mc_iso_surface->compiled_vertex_list)[i]->
														coord)[2];
												}
												i++;
											} /* i */
											v_count++;
											xi_step[2] += xi_stepsize[2];
											kk--;
										} /* kk */
										xi_step[1] += xi_stepsize[1];
										jj--;
									} /* jj */
									xi_step[0] += xi_stepsize[0];
									ii--;
								} /* ii */
								if (voltex)
									/* SAB Code from Mark Sagar */
								{
									/* now calculate vertex normals in cartesian space by
										averaging normals of surrounding faces */
									/*???DB.  Can the normals be transformed ? */
									/* ii repeats over fixed triangle vertex list, pointing to
										current vertex_list+n_vertices for repeating textures */
									acc_triangle_index = 0;
									for (ii=0;ii< n_xi_rep[0]*n_xi_rep[1]*n_xi_rep[2];ii++)
									{
										if (texture_coordinate_field)
										{
											for (i=0;i<n_iso_polys;i++)
											{
												for (j=0;j<3;j++)
												{
													texturemap_index[3*acc_triangle_index+j]=0;
													texturemap_coord[3*(3*acc_triangle_index+j)+0]=
														vertex_list[triangle_list[i*3+j]+
															n_vertices*ii].texture_coordinates[0];
													texturemap_coord[3*(3*acc_triangle_index+j)+1]=
														vertex_list[triangle_list[i*3+j]+
															n_vertices*ii].texture_coordinates[1];
													texturemap_coord[3*(3*acc_triangle_index+j)+2]=
														vertex_list[triangle_list[i*3+j]+
															n_vertices*ii].texture_coordinates[2];
												}
												acc_triangle_index++;
											}
										}

										/* Blurring values */
										if (displacement_field) 
										{
											do
											{
												blur=0;
												for (i=0;i<n_vertices;i++)
												{
													if (vertex_list[i].blur > 0)
													{
														if (vertex_list[i].blur > 1)
														{
															/* i.e. more blurring to do */
															blur=1;
														}
														vertex_list[i].blur--;
														for (k=0;k<3;k++)
														{
															vectorsum[k]=0;
														}
														vectorsumc=0;
														for (j=0;j<vertex_list[i].n_ptrs;j++)
														{
															triangle=vertex_list[i].ptrs[j];
																
															for (k=0;k<3;k++)
															{
																vertex0[k]=vertex_list[ii*n_vertices+
																	triangle_list[triangle*3+0]].coord[k];
																vertex1[k]=vertex_list[ii*n_vertices+
																	triangle_list[triangle*3+1]].coord[k];
																vertex2[k]=vertex_list[ii*n_vertices+
																	triangle_list[triangle*3+2]].coord[k];
																/* set vertex to be the average of its neighbours */
															}
															if (!(vertex0[0] == vertex_list[ii*n_vertices+i].coord[0] &&
																vertex0[1] == vertex_list[ii*n_vertices+i].coord[1] &&
																vertex0[2] == vertex_list[ii*n_vertices+i].coord[2]))
															{
																for (k=0;k<3;k++)
																{
																	vectorsum[k] += vertex0[k]; 
																}
																vectorsumc++;
															}
																
															if (!(vertex1[0] == vertex_list[ii*n_vertices+i].coord[0] &&
																vertex1[1] == vertex_list[ii*n_vertices+i].coord[1] &&
																vertex1[2] == vertex_list[ii*n_vertices+i].coord[2]))
															{
																for (k=0;k<3;k++)
																{
																	vectorsum[k] += vertex1[k];
																}
																vectorsumc++;
															}
																
															if (!(vertex2[0] == vertex_list[ii*n_vertices+i].coord[0] &&
																vertex2[1] == vertex_list[ii*n_vertices+i].coord[1] &&
																vertex2[2] == vertex_list[ii*n_vertices+i].coord[2]))
															{
																for (k=0;k<3;k++)
																{
																	vectorsum[k] += vertex2[k];
																}
																vectorsumc++;
															}
																
																
														}
														for (k=0;k<3;k++)
														{
															if (vectorsumc !=0)
															{
																vertex_list[ii*n_vertices+i].coord[k]=vectorsum[k]/vectorsumc;
															}
														}
													}
													
												} /* i */
											} while (blur);
										}
										
										for (i=0;i<n_vertices;i++)
										{
											for (k=0;k<3;k++)
											{
												vectorsum[k]=0;
											}
											for (j=0;j<vertex_list[i].n_ptrs;j++)
											{
												triangle=vertex_list[i].ptrs[j];
												for (k=0;k<3;k++)
												{
													vertex0[k]=vertex_list[ii*n_vertices+
														triangle_list[triangle*3+0]].coord[k];
													vertex1[k]=vertex_list[ii*n_vertices+
														triangle_list[triangle*3+1]].coord[k];
													vertex2[k]=vertex_list[ii*n_vertices+
														triangle_list[triangle*3+2]].coord[k];
													vector1[k]=vertex1[k]-vertex0[k];
													vector2[k]=vertex2[k]-vertex0[k];
												}
												cross_product_float3(vector1,vector2,result);
												rmag=sqrt((double)(result[0]*result[0]+
													result[1]*result[1]+result[2]*result[2]));
												for (k=0;k<3;k++)
												{
													vectorsum[k] += result[k];
												}
#if defined (OLD_CODE)
												if (fabs(vector1[0]*vector1[0]+vector1[1]*vector1[1]+
													vector1[2]*vector1[2]) < 1e-8)
												{
													bad_triangle = 1;
												}
												if (fabs(vector2[0]*vector2[0]+vector2[1]*vector2[1]+
													vector2[2]*vector2[2]) < 1e-8)
												{
													bad_triangle = 1;
												}
												if (fabs(result[0]*result[0]+result[1]*result[1]+
													result[2]*result[2]) < 1e-8)
												{
													bad_triangle = 1;
												}
#endif /* defined (OLD_CODE) */
											}
											/* now set normal as the average & normalize */
											rmag=sqrt((double)(vectorsum[0]*vectorsum[0]+
												vectorsum[1]*vectorsum[1]+vectorsum[2]*vectorsum[2]));
											if (rmag > 1e-10)
											{
												for (k=0;k<3;k++)
												{
													vertex_list[ii*n_vertices+i].normal[k]=
														-vectorsum[k]/rmag;
													/*???Mark.  This should be + */
												}
											}
											else
											{
												vertex_list[ii*n_vertices+i].normal[0]=1.0;
												vertex_list[ii*n_vertices+i].normal[1]=0.0;
												vertex_list[ii*n_vertices+i].normal[2]=0.0;
											}
#if defined (DEBUG)
											if (bad_triangle)
											{
												printf ("\nVertex %d\n", i);
												printf (" coord %f %f %f\n",
													vertex_list[i].coord[0],
													vertex_list[i].coord[1],
													vertex_list[i].coord[2]);			
												printf (" normal %f %f %f\n",
													vertex_list[i].normal[0],
													vertex_list[i].normal[1],
													vertex_list[i].normal[2]);			
												printf (" Triangles\n");
												for (j=0;j<vertex_list[i].n_ptrs;j++)
												{
													triangle=vertex_list[i].ptrs[j];
													printf ("  (%f %f %f),",
														vertex_list[ii*n_vertices+triangle_list[triangle*3+0]].coord[0],
														vertex_list[ii*n_vertices+triangle_list[triangle*3+0]].coord[1],
														vertex_list[ii*n_vertices+triangle_list[triangle*3+0]].coord[2]);
													printf ("  (%f %f %f),\n\t",
														vertex_list[ii*n_vertices+triangle_list[triangle*3+1]].coord[0],
														vertex_list[ii*n_vertices+triangle_list[triangle*3+1]].coord[1],
														vertex_list[ii*n_vertices+triangle_list[triangle*3+1]].coord[2]);
													printf ("  (%f %f %f)\n",
														vertex_list[ii*n_vertices+triangle_list[triangle*3+2]].coord[0],
														vertex_list[ii*n_vertices+triangle_list[triangle*3+2]].coord[1],
														vertex_list[ii*n_vertices+triangle_list[triangle*3+2]].coord[2]);
													for (k=0;k<3;k++)
													{
														vertex0[k]=vertex_list[ii*n_vertices+
															triangle_list[triangle*3+0]].coord[k];
														vertex1[k]=vertex_list[ii*n_vertices+
															triangle_list[triangle*3+1]].coord[k];
														vertex2[k]=vertex_list[ii*n_vertices+
															triangle_list[triangle*3+2]].coord[k];
														vector1[k]=vertex1[k]-vertex0[k];
														vector2[k]=vertex2[k]-vertex0[k];
													}
													cross_product_float3(vector1,vector2,result);
													printf ("  Normal (%f %f %f)\n",
														result[0], result[1], result[2]);
												}				
												vertex_list[ii*n_vertices+i].normal[0]=1.0;
												vertex_list[ii*n_vertices+i].normal[1]=0.0;
												vertex_list[ii*n_vertices+i].normal[2]=0.0;
											}
#endif /* defined (DEBUG) */
										} /* i */
									} /* ii */
								}
							
								Computed_field_clear_cache(coordinate_field);
								if(data_field)
								{
									Computed_field_clear_cache(data_field);
								}
								if(blur_field)
								{
									Computed_field_clear_cache(blur_field);
								}
								if(texture_coordinate_field)
								{
									Computed_field_clear_cache(texture_coordinate_field);
								}
								if(displacement_field)
								{
									Computed_field_clear_cache(displacement_field);
								}
								DEALLOCATE(triangle_list);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
	"create_GT_voltex_from_FE_element.  Invalid number of coordinate components");
							voltex=(struct GT_voltex *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
			"create_GT_voltex_from_FE_element.  Could not allocate memory for faces");
						voltex=(struct GT_voltex *)NULL;
					}
					DEALLOCATE(dont_draw);
				}
				else
				{
					display_message(ERROR_MESSAGE,
	"create_GT_voltex_from_FE_element.  Could not allocate memory for elements");
					voltex=(struct GT_voltex *)NULL;
				}
				DEALLOCATE(element_block);
				DEALLOCATE(adjacency_table);
				if (coordinate_field_derivatives)
				{
					DEALLOCATE(coordinate_field_derivatives);
				}
			}
		}
		else
		{
/*			display_message(ERROR_MESSAGE,
				"create_GT_voltex_from_FE_element.  n_iso_polys = 0");*/
			voltex=(struct GT_voltex *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_voltex_from_FE_element.  Invalid argument(s)");
		voltex=(struct GT_voltex *)NULL;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave create_GT_voltex_from_FE_element %p\n",voltex);
#endif /* defined (DEBUG) */
	LEAVE;

	return (voltex);
} /* create_GT_voltex_from_FE_element */

int create_FE_nodes_on_GT_voltex_surface(struct GT_voltex *voltex,
	struct FE_element *element, struct Computed_field *coordinate_field,
	struct VT_volume_texture *vtexture, struct FE_region *fe_region,
	struct Computed_field *data_density_field,
	struct Computed_field *data_coordinate_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
This function takes a <voltex> and the corresponding <vtexture> and creates
a randomly placed set of FE_nodes over the surface the <voltex> describes.
The vtexture is used to define an element_xi field at each of the data_points.
These can then be used to draw axes or gradient vectors over the surface of an
isosurface or to create hair streamlines starting from the surface of a volume
texture. If the <data_coordinate_field> is supplied, it - ie. the FE_field it is
calculated from -  will be defined at the data points, and the field is given
the position of the point, with appropriate coordinate conversion.
==============================================================================*/
{
	FE_value area, coordinate_1, coordinate_2, coordinate_3,
		density_data[4], expected_number, position[3], xi1, xi2, xi[3];
	int aaa, *adjacency_table, bbb, ccc,i, j, k, m, node_number, n_iso_polys, 
		number_of_elements, n_xi[3], number_of_points, return_code, *triangle_list2;
	struct Computed_field *rc_data_coordinate_field;
	struct Coordinate_system rect_cart_coords;
	struct FE_element **element_block,**element_block_ptr,*local_element;
	struct FE_node *new_node, *template_node;
	struct FE_field *fe_coordinate_field, *fe_element_xi_field;
	struct LIST(FE_field) *fe_coordinate_field_list;
	struct VT_iso_vertex *vertex_list;
	struct FE_node_field_creator *coordinate_node_field_creator,
		*element_xi_node_field_creator;
	char *element_xi_component_names[1]=
	{
		"value"
	};

	ENTER(create_FE_nodes_on_GT_voltex_surface);
#if defined (DEBUG)
	/*???debug */
	printf("enter create_FE_nodes_on_GT_voltex_surface\n");
#endif /* defined (DEBUG) */
	if (element && (3 == get_FE_element_dimension(element)) && vtexture &&
		vtexture && fe_region && coordinate_field &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)) &&
		data_density_field &&
		(4 >= Computed_field_get_number_of_components(data_density_field)) &&
		vtexture->mc_iso_surface && voltex &&
		(vtexture->mc_iso_surface->n_triangles == voltex->n_iso_polys) &&
		(vtexture->mc_iso_surface->n_vertices == voltex->n_vertices))
	{
		FE_region_begin_change(fe_region);
		fe_coordinate_field_list = (struct LIST(FE_field) *)NULL;
		fe_coordinate_field = (struct FE_field *)NULL;
		rc_data_coordinate_field = (struct Computed_field *)NULL;
		if ((!data_coordinate_field) || (
			(3 == Computed_field_get_number_of_components(data_coordinate_field)) &&
			(fe_coordinate_field_list =
				Computed_field_get_defining_FE_field_list(data_coordinate_field)) &&
			(1 == NUMBER_IN_LIST(FE_field)(fe_coordinate_field_list)) &&
			(fe_coordinate_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
				fe_coordinate_field_list)) &&
			(3 == get_FE_field_number_of_components(fe_coordinate_field)) &&
			(rc_data_coordinate_field = Computed_field_begin_wrap_coordinate_field(
				data_coordinate_field))))
		{
			template_node = (struct FE_node *)NULL;
			if (vtexture->mc_iso_surface->n_triangles)
			{
				/* examine min & max xi values of voltex. If 0<xi<1 possibly repeat.  If
					 xi>1 then will need to draw over other elements, so must find
					 neighbouring ones */
				/*???MS.  27 August 1996.  At present put in a hack to subtrat the
					vtexture->ximin values of the xi values.  Should formalize this
					later */
				number_of_elements=1;
				node_number = 1;
				for (i=0;i<3;i++)
				{
					/* MACRO TEXTURES : spread over several elements */
					/* round to nearest integer above if non integer (eg 1->1,1.2 ->2) */
					n_xi[i]=(int)ceil(vtexture->ximax[i]);
					/* MICRO TEXTURES : repeated within one element */
					/* if texture is to be repeated then calc # of repetitions for each
						 value */
					number_of_elements *= n_xi[i]; /* n_xi_rep[i]? */
				}
				/* allocate memory for elements */
				ALLOCATE(element_block,struct FE_element *,number_of_elements);
				ALLOCATE(adjacency_table,int,6*number_of_elements);
				if (element_block&&adjacency_table)
				{
					/* pick seed element starting at minimum xi1,2,3 position (ie give
						 closest bottom leftmost element ) */
					/* calculate elements sitting in block which texture extends into */
					/* count the number of faces */
					element_block_ptr=element_block;
					for (i=0;i<n_xi[0];i++)
					{
						for (j=0;j<n_xi[1];j++)
						{
							for (k=0;k<n_xi[2];k++)
							{
								element_block_ptr[k*n_xi[0]*n_xi[1]+j*n_xi[0]+i]=
									(struct FE_element *)NULL;
								for (m=0;m<6;m++)
								{
									adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+m]=0;
								}
							}
						}
					}
					/* recursively filling element block */
					fill_table(element_block,adjacency_table,element,0,0,0,n_xi);
					return_code=1;
					/* check the voltex can be completely drawn inside mesh */
					for (i=0;return_code&&(i<number_of_elements);i++)
					{
						if (!element_block[i])
						{
							display_message(WARNING_MESSAGE,
								"create_FE_nodes_on_GT_voltex_surface.  "
								"voltex extends beyond elements");
							return_code=0;
						}
					}
					if (return_code)
					{
						n_iso_polys=vtexture->mc_iso_surface->n_triangles;
						triangle_list2 = voltex->triangle_list;
						vertex_list = voltex->vertex_list;
										
						for ( i = 0 ; return_code && (i < n_iso_polys) ; i++)
						{
							/* Find centre of triangle in xi space and evaluate surface_point_map */
							xi[0] = 0.0;
							xi[1] = 0.0;
							xi[2] = 0.0;
							for (j=0;j<3;j++)
							{
								xi[0] += ((vtexture->mc_iso_surface->
									compiled_vertex_list)[triangle_list2[3*i+j]]->coord)[0];
								xi[1] += ((vtexture->mc_iso_surface->
									compiled_vertex_list)[triangle_list2[3*i+j]]->coord)[1];
								xi[2] += ((vtexture->mc_iso_surface->
									compiled_vertex_list)[triangle_list2[3*i+j]]->coord)[2];
							}
							xi[0] /= 3.0;
							xi[1] /= 3.0;
							xi[2] /= 3.0;

							Set_element_and_local_xi(element_block, n_xi, xi, &local_element);

							Computed_field_evaluate_in_element(
								data_density_field, local_element,
								xi,time,(struct FE_element *)NULL,
								density_data,(FE_value *)NULL);

#if defined (DEBUG)
							printf("create_FE_nodes_on_GT_voltex_surface.  colour %f\n",
								texture_data[0]);
#endif /* defined (DEBUG) */

							/* Evaluate area of triangle in real space */
							aaa = triangle_list2[3*i];
							bbb = triangle_list2[3*i + 1];
							ccc = triangle_list2[3*i + 2];
							coordinate_1 = sqrt(
								(vertex_list[aaa].coord[0] - vertex_list[bbb].coord[0]) *
								(vertex_list[aaa].coord[0] - vertex_list[bbb].coord[0]) +
								(vertex_list[aaa].coord[1] - vertex_list[bbb].coord[1]) *
								(vertex_list[aaa].coord[1] - vertex_list[bbb].coord[1]) +
								(vertex_list[aaa].coord[2] - vertex_list[bbb].coord[2]) *
								(vertex_list[aaa].coord[2] - vertex_list[bbb].coord[2]));
							coordinate_2 = sqrt(
								(vertex_list[bbb].coord[0] - vertex_list[ccc].coord[0]) *
								(vertex_list[bbb].coord[0] - vertex_list[ccc].coord[0]) +
								(vertex_list[bbb].coord[1] - vertex_list[ccc].coord[1]) *
								(vertex_list[bbb].coord[1] - vertex_list[ccc].coord[1]) +
								(vertex_list[bbb].coord[2] - vertex_list[ccc].coord[2]) *
								(vertex_list[bbb].coord[2] - vertex_list[ccc].coord[2]));
							coordinate_3 = sqrt(
								(vertex_list[ccc].coord[0] - vertex_list[aaa].coord[0]) *
								(vertex_list[ccc].coord[0] - vertex_list[aaa].coord[0]) +
								(vertex_list[ccc].coord[1] - vertex_list[aaa].coord[1]) *
								(vertex_list[ccc].coord[1] - vertex_list[aaa].coord[1]) +
								(vertex_list[ccc].coord[2] - vertex_list[aaa].coord[2]) *
								(vertex_list[ccc].coord[2] - vertex_list[aaa].coord[2]));
							area = 0.5 * ( coordinate_1 + coordinate_2 + coordinate_3 );
							area = sqrt ( area * (area - coordinate_1) *
								(area - coordinate_2) * (area - coordinate_3));
							expected_number = area * density_data[0];
							/* get actual_number = sample from Poisson distribution with
								 mean given by expected_number */
							number_of_points =
								sample_Poisson_distribution((double)expected_number);
#if defined (DEBUG)
							printf("create_FE_nodes_on_GT_voltex_surface.\n"
								"\tarea %f expected %f sample number_of_points %d\n",
								area, expected_number, number_of_points);
#endif /* defined (DEBUG) */
							for (j = 0; (j < number_of_points) && return_code; j++)
							{
								xi1 = CMGUI_RANDOM(float);
								xi2 = CMGUI_RANDOM(float);
								if(xi1 + xi2 > 1.0)
								{
									xi1 = 1.0 - xi1;
									xi2 = 1.0 - xi2;
								}

								xi[0] = vtexture->mc_iso_surface->
									compiled_vertex_list[triangle_list2[3*i+0]]->coord[0]
									+ xi1 * (vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+1]]->coord[0]
										- vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+0]]->coord[0])
									+ xi2 * (vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+2]]->coord[0]
										- vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+0]]->coord[0]);
								xi[1] = vtexture->mc_iso_surface->
									compiled_vertex_list[triangle_list2[3*i+0]]->coord[1]
									+ xi1 * (vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+1]]->coord[1]
										- vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+0]]->coord[1])
									+ xi2 * (vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+2]]->coord[1]
										- vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+0]]->coord[1]);
								xi[2] = vtexture->mc_iso_surface->
									compiled_vertex_list[triangle_list2[3*i+0]]->coord[2]
									+ xi1 * (vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+1]]->coord[2]
										- vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+0]]->coord[2])
									+ xi2 * (vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+2]]->coord[2]
										- vtexture->mc_iso_surface->
										compiled_vertex_list[triangle_list2[3*i+0]]->coord[2]);

								Set_element_and_local_xi(element_block, n_xi, xi, &local_element);

								if (!(Computed_field_evaluate_in_element(
									coordinate_field, local_element,
									xi,time,(struct FE_element *)NULL, position, 
									(FE_value *)NULL)))
								{
									display_message(ERROR_MESSAGE,
										"create_FE_nodes_on_GT_voltex_surface."
										"  Unable to evaluate node coordinate position");
									return_code = 0;
								}

								/* If this is the first one create the template */
								if (!template_node)
								{
									/* create or find the element_xi field */
									rect_cart_coords.type=RECTANGULAR_CARTESIAN;
									if (fe_element_xi_field = 
										FE_region_get_FE_field_with_properties(fe_region,
											"element_xi",GENERAL_FE_FIELD,
											/*indexer_field*/(struct FE_field *)NULL,
											/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
											&rect_cart_coords,ELEMENT_XI_VALUE,
											/*number_of_components*/1,element_xi_component_names,
											/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
											(struct FE_field_external_information *)NULL))
									{
										if ((coordinate_node_field_creator = CREATE(
											FE_node_field_creator)(/*number_of_components*/3))&&
											(element_xi_node_field_creator = CREATE(
												FE_node_field_creator)(/*number_of_components*/1)))
										{
											/* create the node */
											if ((template_node = CREATE(FE_node)(0, fe_region,
												(struct FE_node *)NULL)) &&
												((!fe_coordinate_field) ||
													define_FE_field_at_node(template_node,
														fe_coordinate_field,
														(struct FE_time_version *)NULL,
														coordinate_node_field_creator)) &&
												define_FE_field_at_node(template_node,
													fe_element_xi_field,
													(struct FE_time_version *)NULL,
													element_xi_node_field_creator))
											{
												return_code=1;	
											}
											DESTROY(FE_node_field_creator)(
												&coordinate_node_field_creator);
											DESTROY(FE_node_field_creator)(
												&element_xi_node_field_creator);
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
								}
								node_number = FE_region_get_next_FE_node_identifier(fe_region,
									node_number);
								if (new_node = CREATE(FE_node)(node_number,
									(struct FE_region *)NULL, template_node))
								{	
									ACCESS(FE_node)(new_node);
									if (((!fe_coordinate_field) ||
										Computed_field_set_values_at_node(rc_data_coordinate_field,
											new_node, position)) &&
										set_FE_nodal_element_xi_value(new_node,
											fe_element_xi_field, 0/*component_number*/, 0/*version*/,
											FE_NODAL_VALUE, local_element, xi))
									{
										if (!FE_region_merge_FE_node(fe_region, new_node))
										{
											display_message(ERROR_MESSAGE,
												"create_FE_nodes_on_GT_voltex_surface.  "
												"Could not merge node into region");
											return_code = 0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"create_FE_nodes_on_GT_voltex_surface.  "
											"Could not set fields at node");
										return_code = 0;
									}
									DEACCESS(FE_node)(&new_node);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_FE_nodes_on_GT_voltex_surface.  "
										"Could not create FE_node");
									return_code = 0;
								}
#if defined (DEBUG)
								printf("create_FE_nodes_on_GT_voltex_surface.  "
									"point_coords %f %f %f\n", position[0], position[1], position[2]);
								printf("create_FE_nodes_on_GT_voltex_surface.  rand xi %f %f\n",
									xi1, xi2);
								printf("create_FE_nodes_on_GT_voltex_surface.  element %p\n",
									element);
								printf("create_FE_nodes_on_GT_voltex_surface.  xi %f %f %f\n",
									xi[0], xi[1], xi[2]);
#endif /* defined (DEBUG) */
							}
						}
						Computed_field_clear_cache(coordinate_field);
						Computed_field_clear_cache(data_density_field);
						if(template_node)
						{
							DESTROY(FE_node)(&template_node);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_FE_nodes_on_GT_voltex_surface.  Could not allocate memory for elements");
					return_code = 0;
				}
				DEALLOCATE(element_block);
				DEALLOCATE(adjacency_table);
			}
			else
			{
				/* display_message(ERROR_MESSAGE,
					 "create_FE_nodes_on_GT_voltex_surface.  n_iso_polys = 0");*/
				return_code = 0;
			}
			if (rc_data_coordinate_field)
			{
				Computed_field_end_wrap(&rc_data_coordinate_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_FE_nodes_on_GT_voltex_surface.  "
				"Could not use surface data coordinate field");
			return_code = 0;
		}
		if (fe_coordinate_field_list)
		{
			DESTROY(LIST(FE_field))(&fe_coordinate_field_list);
		}
		FE_region_end_change(fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_FE_nodes_on_GT_voltex_surface.  Invalid argument(s)");
		return_code = 0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave create_FE_nodes_on_GT_voltex_surface %p\n",voltex);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* create_FE_nodes_on_GT_voltex_surface */

struct GT_glyph_set *create_GT_glyph_set_from_FE_element(
	struct FE_element *element, struct FE_element *top_level_element,
	struct Computed_field *coordinate_field,
	int number_of_xi_points, Triple *xi_points, struct GT_object *glyph,
	FE_value *base_size, FE_value *centre, FE_value *scale_factors,
	struct Computed_field *orientation_scale_field,
	struct Computed_field *variable_scale_field,
	struct Computed_field *data_field, struct Computed_field *label_field,
	enum Graphics_select_mode select_mode, int element_selected,
	struct Multi_range *selected_ranges, int *point_numbers, FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information
about fields defined over it.
At each of the <number_of_xi_points> <xi_points> the <glyph> of at least
<base_size> with the given glyph <centre> is displayed.
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field. The
optional <variable_scale_field> can provide signed scaling independently of the
glyph axis directions. See function make_glyph_orientation_scale_axes for
details. The combined scale from the above 2 fields is multiplied in each axis
by the <scale_factors> then added to the base_size.
The optional <data_field> (currently only a scalar) is calculated as data over
the glyph_set, for later colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
<select_mode> is used in combination with the <element_selected> and
<selected_ranges> to draw only those points with numbers in or out of the given
ranges when given value GRAPHICS_DRAW_SELECTED or GRAPHICS_DRAW_UNSELECTED.
If <element_selected> is true, all points are selected, otherwise selection is
determined from the <selected_ranges>, and if <selected_ranges> is NULL, no
numbers are selected.
If <point_numbers> are supplied then points numbers for OpenGL picking are taken
from this array, otherwise they are sequential, starting at 0.
Note:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
- the coordinate system of the variable_scale_field is ignored/not used.
==============================================================================*/
{
	char **label, **labels;
	FE_value a[3], b[3], c[3], coordinates[3], orientation_scale[9], size[3],
		variable_scale[3], xi[3];
	GTDATA *data;
	int draw_all, i, j, n_data_components, *name, *names,
		number_of_orientation_scale_components, number_of_variable_scale_components,
		point_number, point_selected,	points_to_draw;
	struct CM_element_information cm;
	struct GT_glyph_set *glyph_set;
	Triple *axis1, *axis1_list, *axis2, *axis2_list, *axis3, *axis3_list,
		*point, *point_list, *scale, *scale_list;

	ENTER(create_GT_glyph_set_from_FE_element);
	/* must set following to 0 in case fields not supplied */
	number_of_orientation_scale_components = 0;
	number_of_variable_scale_components = 0;
	if (element && coordinate_field &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)) &&
		(0 < number_of_xi_points) && xi_points && glyph &&
		centre && base_size && scale_factors &&
		((!orientation_scale_field)||((9>=(number_of_orientation_scale_components=
			Computed_field_get_number_of_components(orientation_scale_field)))&&
			Computed_field_is_orientation_scale_capable(orientation_scale_field,
				(void *)NULL))) &&
		((!variable_scale_field)||(3>=(number_of_variable_scale_components=
			Computed_field_get_number_of_components(variable_scale_field)))))
	{
		/* clear coordinates in case coordinate field is not 3 component */
		coordinates[0] = 0.0;
		coordinates[1] = 0.0;
		coordinates[2] = 0.0;
		glyph_set = (struct GT_glyph_set *)NULL;
		point_list = (Triple *)NULL;
		axis1_list = (Triple *)NULL;
		axis2_list = (Triple *)NULL;
		axis3_list = (Triple *)NULL;
		scale_list = (Triple *)NULL;
		labels = (char **)NULL;
		n_data_components = 0;
		data = (GTDATA *)NULL;
		names = (int *)NULL;
		if ((GRAPHICS_SELECT_ON == select_mode) ||
			(GRAPHICS_NO_SELECT == select_mode) ||
			((GRAPHICS_DRAW_SELECTED == select_mode) && element_selected))
		{
			points_to_draw = number_of_xi_points;
		}
		else if ((GRAPHICS_DRAW_UNSELECTED == select_mode) && element_selected)
		{
			points_to_draw = 0;
		}
		else
		{
			points_to_draw = 0;
			if (selected_ranges)
			{
				for (i = 0; i < number_of_xi_points; i++)
				{
					if (point_numbers)
					{
						point_number = point_numbers[i];
					}
					else
					{
						point_number = i;
					}
					if (Multi_range_is_value_in_range(selected_ranges, point_number))
					{
						points_to_draw++;
					}
				}
			}
			if (GRAPHICS_DRAW_UNSELECTED == select_mode)
			{
				points_to_draw = number_of_xi_points - points_to_draw;
			}
		}
		if (0 < points_to_draw)
		{
			draw_all = (points_to_draw == number_of_xi_points);
			if (data_field)
			{
				n_data_components = Computed_field_get_number_of_components(data_field);
				ALLOCATE(data, GTDATA, points_to_draw*n_data_components);
			}
			if (label_field)
			{
				if (ALLOCATE(labels, char *, points_to_draw))
				{
					/* clear labels array pointers so new glyph_set not corrupted */
					for (i = 0; i < points_to_draw; i++)
					{
						labels[i] = (char *)NULL;
					}
				}
			}
			if (GRAPHICS_NO_SELECT != select_mode)
			{
				ALLOCATE(names,int,points_to_draw);
			}
			/* store element number as object_name for editing GT_object primitives */
			get_FE_element_identifier(element, &cm);
			if ((data || (!n_data_components)) && ((!label_field) || labels) &&
				((GRAPHICS_NO_SELECT == select_mode) || names) &&
				ALLOCATE(point_list, Triple, points_to_draw) &&
				ALLOCATE(axis1_list, Triple, points_to_draw) &&
				ALLOCATE(axis2_list, Triple, points_to_draw) &&
				ALLOCATE(axis3_list, Triple, points_to_draw) &&
				ALLOCATE(scale_list, Triple, points_to_draw) &&
				(glyph_set = CREATE(GT_glyph_set)(points_to_draw, point_list,
					axis1_list, axis2_list, axis3_list, scale_list, glyph, labels,
					n_data_components, data,
					CM_element_information_to_graphics_name(&cm), names)))
			{
				point = point_list;
				axis1 = axis1_list;
				axis2 = axis2_list;
				axis3 = axis3_list;
				scale = scale_list;
				name = names;
				label = labels;
				for (i = 0; (i < number_of_xi_points) && glyph_set; i++)
				{
					if (point_numbers)
					{
						point_number = point_numbers[i];
					}
					else
					{
						point_number = i;
					}
					if (!draw_all)
					{
						if (selected_ranges)
						{
							point_selected =
								Multi_range_is_value_in_range(selected_ranges, point_number);
						}
						else
						{
							point_selected = 0;
						}
					}
					if (draw_all ||
						((GRAPHICS_DRAW_SELECTED == select_mode) && point_selected) ||
						((GRAPHICS_DRAW_UNSELECTED == select_mode) && (!point_selected)))
					{
						xi[0] = (FE_value)xi_points[i][0];
						xi[1] = (FE_value)xi_points[i][1];
						xi[2] = (FE_value)xi_points[i][2];
						/* evaluate all the fields in order orientation_scale, coordinate
							 then data (if each specified). Reason for this order is that the
							 orientation_scale field very often requires the evaluation of the
							 same coordinate_field with derivatives, meaning that values for
							 the coordinate_field will already be cached = more efficient. */
						if (((!orientation_scale_field) ||
							Computed_field_evaluate_in_element(orientation_scale_field,
							element, xi, time, top_level_element, orientation_scale,
							(FE_value *)NULL)) &&
							((!variable_scale_field) ||
							Computed_field_evaluate_in_element(variable_scale_field,
							element, xi, time,top_level_element, variable_scale,
							(FE_value *)NULL)) &&
							Computed_field_evaluate_in_element(coordinate_field,element,xi,
							time,top_level_element,coordinates,(FE_value *)NULL)&&
							((!data_field) || Computed_field_evaluate_in_element(
							data_field, element, xi, time, top_level_element, data,
							(FE_value *)NULL)) && ((!label_field) || (*label =
							Computed_field_evaluate_as_string_in_element(label_field,
							/*component_number*/-1, element, xi, time, top_level_element))) &&
							make_glyph_orientation_scale_axes(
								number_of_orientation_scale_components, orientation_scale,
								a, b, c, size))
						{
							for (j = 0; j < 3; j++)
							{
								(*scale)[j] = base_size[j] + size[j]*scale_factors[j];
							}
							for (j = 0; j < number_of_variable_scale_components; j++)
							{
								(*scale)[j] *= variable_scale[j];
							}
							for (j = 0; j < 3; j++)
							{
								(*point)[j] = coordinates[j] -
									centre[0]*(*scale)[0]*a[j] -
									centre[1]*(*scale)[1]*b[j] -
									centre[2]*(*scale)[2]*c[j];
								(*axis1)[j] = a[j];
								(*axis2)[j] = b[j];
								(*axis3)[j] = c[j];
							}
							point++;
							axis1++;
							axis2++;
							axis3++;
							scale++;

							if (data_field)
							{
								data += n_data_components;
							}
							if (names)
							{
								*name = point_number;
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
						else
						{
							/* error evaluating fields */
							DESTROY(GT_glyph_set)(&glyph_set);
						}
					}
				}
				/* clear Computed_field caches so elements not accessed */
				Computed_field_clear_cache(coordinate_field);
				if (orientation_scale_field)
				{
					Computed_field_clear_cache(orientation_scale_field);
				}
				if (variable_scale_field)
				{
					Computed_field_clear_cache(variable_scale_field);
				}
				if (data_field)
				{
					Computed_field_clear_cache(data_field);
				}
				if (label_field)
				{
					Computed_field_clear_cache(label_field);
				}
			}
			else
			{
				DEALLOCATE(point_list);
				DEALLOCATE(axis1_list);
				DEALLOCATE(axis2_list);
				DEALLOCATE(axis3_list);
				DEALLOCATE(scale_list);
				DEALLOCATE(data);
				DEALLOCATE(labels);
				DEALLOCATE(names);
			}
			if (!glyph_set)
			{
				display_message(ERROR_MESSAGE,
					"create_GT_glyph_set_from_FE_element.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_glyph_set_from_FE_element.  Invalid argument(s)");
		glyph_set=(struct GT_glyph_set *)NULL;
	}
	LEAVE;

	return (glyph_set);
} /* create_GT_glyph_set_from_FE_element */

struct VT_vector_field *interpolate_vector_field_on_FE_element(double ximax[3],
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct VT_vector_field *vector_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Interpolates xi points (triples in vector field) over the finite <element>
<block>.  Outputs the updated field.
==============================================================================*/
{
	FE_value coordinate[3],distance, minimum_distance,xi[3];
	int a,aa,*adjacency_table,b,bb,c,cc,i,j,k,m,number_of_elements,
		n_vertices,n_xi[3],outside_block,return_code;
	struct FE_element **element_block,**element_block_ptr;
	struct VT_vector_field *new_field;

	ENTER(interpolate_vector_field_on_FE_element);
#if defined (DEBUG)
	/*???debug */
	printf("enter interpolate_vector_field_on_FE_element\n");
#endif /* defined (DEBUG) */
	if (element && (3 == get_FE_element_dimension(element)) && vector_field &&
		coordinate_field &&
		(3 >= Computed_field_get_number_of_components(coordinate_field)))
	{
		/* examine min & max xi values of voltex. If 0<xi<1 possibly repeat.  If
			xi>1 then will need to draw over other elements, so must find neighbouring
			ones */
		number_of_elements=1;
		for (i=0;i<3;i++)
		{
			/* MACRO TEXTURES : spread over several elements */
			/* round to nearest integer above if non integer (eg 1->1,1.2 ->2) */
			n_xi[i]=(int)ceil(ximax[i]);
			number_of_elements *= n_xi[i];
			/* MICRO TEXTURES : repeated within one element ???*/
		}
		/* allocate memory for elements */
		ALLOCATE(element_block,struct FE_element *,number_of_elements);
		ALLOCATE(adjacency_table, int, 6*number_of_elements);
		if (element_block&&adjacency_table)
		{
			/* pick seed element starting at minimum xi1,2,3 position (ie give closet
				bottom leftmost element ) */
			/* calculute elements sitting in block which texture extends into */
			element_block_ptr=element_block;
			for (i=0;i<n_xi[0];i++)
			{
				for (j=0;j<n_xi[1];j++)
				{
					for (k=0;k<n_xi[2];k++)
					{
						element_block_ptr[k*n_xi[0]*n_xi[1]+j*n_xi[0]+i]=
							(struct FE_element *)NULL;
						for (m=0;m<6;m++)
						{
							adjacency_table[(k*n_xi[0]*n_xi[1]+j*n_xi[0]+i)*6+m]=0;
						}
					}
				}
			}
			/* recursively filling element block */
			fill_table(element_block,adjacency_table,element,0,0,0,n_xi);
			/* coordinates calculated using Q=G(geometry vector).M(blending matrix).
				T(monomial basis (of xi values)) */
			/* calculate G.M [using calc_FE_elem_field_vals] (to multiply the
				monomial basis T) for each element in the block - when an xi
				coordinate in the vertex list extends over the first element, the next
				element is used */
			return_code=1;
			if (return_code)
			{
				n_vertices=(vector_field->dimension[0]+1)*
					(vector_field->dimension[1]+1)*(vector_field->dimension[2]+1);
				if (ALLOCATE(new_field,struct VT_vector_field,1))
				{
					new_field->dimension[0]=vector_field->dimension[0];
					new_field->dimension[1]=vector_field->dimension[1];
					new_field->dimension[2]=vector_field->dimension[2];
					if (!ALLOCATE(new_field->vector,double,3*n_vertices))
					{
						display_message(ERROR_MESSAGE,
							"interpolate_vector_field_on_FE_element.  "
							"Could not allocate memory for points");
						new_field=(struct VT_vector_field *)NULL;
					}
					else
					{
						i=0;
						while ((i<n_vertices)&&new_field)
						{
							/* get coord values from volume texture */
								/* vertex */
							xi[0]=(FE_value) vector_field->vector[3*i+0];
							xi[1]=(FE_value) vector_field->vector[3*i+1];
							xi[2]=(FE_value) vector_field->vector[3*i+2];
							/* here it must be decided which element the xi point
							exists in.  Once the element is chosen, the xi value is
							renormalized to lie within that element */
							/* the element indices */
							a=(int)(floor(((float)xi[0])));
							b=(int)(floor(((float)xi[1])));
							c=(int)(floor(((float)xi[2])));
							/* if a vertex is outside of the element block, then do not
								deform it */
							outside_block=0;
							if ((a>n_xi[0]-1)&&(xi[0]))
							{
								outside_block=1;
							}
							if ((b>n_xi[1]-1)&&(xi[1]))
							{
								outside_block=1;
							}
							if ((c>n_xi[2]-1)&&(xi[2]))
							{
								outside_block=1;
							}
							if (a<0)
							{
								outside_block=1;
							}
							if (b<0)
							{
								outside_block=1;
							}
							if (c<0)
							{
								outside_block=1;
							}
							if (!outside_block)
							{
								/* normalize the xi values to lie in element [a][b][c] */
								if (!(element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a]))
								{
									/* find nearest non-null element to xi */
									minimum_distance=(xi[0])*(xi[0])+(xi[1])*(xi[1])+
										(xi[2])*(xi[2]);
									for (aa=0;aa<n_xi[0];aa++)
									{
										for (bb=0;bb<n_xi[1];bb++)
										{
											for (cc=0;cc<n_xi[2];cc++)
											{
												distance=(xi[0]-aa)*(xi[0]-aa)+(xi[1]-bb)*(xi[1]-bb)+
													(xi[2]-cc)*(xi[2]-cc);
												if ((distance<minimum_distance)&&
													element_block[cc*n_xi[0]*n_xi[1]+bb*n_xi[0]+aa])
												{
													minimum_distance=distance;
													a=aa;
													b=bb;
													c=cc;
												}
											}
										}
									}
								}
								xi[0] -= ((FE_value)a);
								xi[1] -= ((FE_value)b);
								xi[2] -= ((FE_value)c);
								/* coordinate_n_value contains the blended matrix (M.G
									(m=blending (eg Cubic Hermite),G=geometry) for each
									element dimension. To calculate the monomial values
									(dependent on xi1,2,3) the standard basis fn (which
									happens to be monomial (T(xi1,2,3)) in this case) is
									called. This need only be calculated once for each set
									of xi coords and then used for each coordinate by
									obtaining the dot product (Q(xi1,2,3)=M.G.T) <I think
									this is actually Q=G.M.T> */
								if (Computed_field_evaluate_in_element(
									coordinate_field,
									element_block[c*n_xi[0]*n_xi[1]+b*n_xi[0]+a],
									xi,time,(struct FE_element *)NULL, coordinate, 
									(FE_value *)NULL))
								{
									/* assign the coordinates to the point */
									new_field->vector[3*i+0]=(double)coordinate[0];
									new_field->vector[3*i+1]=(double)coordinate[1];
									new_field->vector[3*i+2]=(double)coordinate[2];
								}
								else
								{
									display_message(ERROR_MESSAGE,
				"interpolate_vf_on_FE_element.  Error calculating coordinate field");
									DEALLOCATE(new_field->vector);
									DEALLOCATE(new_field);
								}
							}
							else /* if outside_block */
							{
								new_field->vector[3*i+0]=vector_field->vector[3*i+0];
								new_field->vector[3*i+1]=vector_field->vector[3*i+1];
								new_field->vector[3*i+2]=vector_field->vector[3*i+2];
							}
							i++;
						} /* i */
						Computed_field_clear_cache(coordinate_field);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"interpolate_vt_on_FE_element. Couldn't allocate new field ");
				}
			} /*if return code */
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"interpolate_vt_on_FE_element.  Could not allocate memory for elements");
			new_field=(struct VT_vector_field *)NULL;
		}
		DEALLOCATE(element_block);
		DEALLOCATE(adjacency_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"interpolate_vt_on_FE_element  Invalid argument(s)");
		new_field=(struct VT_vector_field *)NULL;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave interpolate_vector_field_on_FE_element\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (new_field);
} /* interpolate_vector_field_on_FE_element */

struct GT_voltex *generate_clipped_GT_voltex_from_FE_element(
	struct Clipping *clipping,struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	struct VT_volume_texture *texture, enum Render_type render_type,
	struct Computed_field *displacement_map_field, int displacement_map_xi_direction,
	struct Computed_field *blur_field, struct Computed_field *texture_coordinate_field,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Generates clipped voltex from <volume texture> and <clip_function> over
<element><block>
==============================================================================*/
{
	int i,nx,ny,nz;
	int n_scalar_fields,face_index;
	struct VT_scalar_field *scalar_field_list[MAX_SCALAR_FIELDS];
	double isovalue_list[MAX_SCALAR_FIELDS];
	double vector[3];
	struct VT_vector_field *new_field;
	struct MC_triangle *triangle;
	struct GT_voltex *voltex;

	ENTER(generate_clipped_GT_voltex_from_FE_element);
	if (texture)
	{
		if (!(texture->disable_volume_functions))
		{
			nx=texture->dimension[0]+1;
			ny=texture->dimension[1]+1;
			nz=texture->dimension[2]+1;
			/* calculate scalar field using filter */
			/* at present this also calculates a regular grid coordinate field  - in
				future may want to use irregular grid coordinates) */
			if (update_scalars(texture,texture->cutting_plane))
			{
				scalar_field_list[0]=texture->scalar_field;
				isovalue_list[0]=texture->isovalue;
				if (texture->hollow_mode_on)
				{
					/*???Mark.  Should combine clip field and this field in some way */
					for (i=(texture->clip_field2->dimension[0]+1)*
						(texture->clip_field2->dimension[1]+1)*
						(texture->clip_field2->dimension[2]+1)-1;i>=0;i--)
					{
						texture->clip_field2->scalar[i]=
							1.0-texture->scalar_field->scalar[i];
					}
				}
				n_scalar_fields=1;
				if (texture->clipping_field_on)
				{
#if defined (DEBUG)
					/*???debug */
					printf("texture->clipping_field_on\n");
#endif /* defined (DEBUG) */
					/* cutting isosurface is defined by a field */
					scalar_field_list[n_scalar_fields]=texture->clip_field;
					isovalue_list[n_scalar_fields]=texture->cut_isovalue;
					n_scalar_fields++;
				}
				else
				{
					if (clipping)
					{
#if defined (DEBUG)
						/*???debug */
						printf("clipping\n");
#endif /* defined (DEBUG) */
						/* calculate the clip fn over the voltex coordinate field */
						/* calculate deformed coordinate field */
						new_field=interpolate_vector_field_on_FE_element(texture->ximax,
							element,coordinate_field,texture->coordinate_field, time);
						if (new_field)
						{
#if defined (DEBUG)
							/*???debug */
							printf("new_field\n");
#endif /* defined (DEBUG) */
							for (i=0;i<nx*ny*nz;i++)
							{
								vector[0]=new_field->vector[3*i];
								vector[1]=new_field->vector[3*i+1];
								vector[2]=new_field->vector[3*i+2];
								texture->clip_field->scalar[i]=
									(clipping->function)(vector,clipping->parameters);
#if defined (DEBUG)
								/*???debug */
								if (0<=texture->clip_field->scalar[i])
								{
									printf("clip %g %g %g %g\n",vector[0],vector[1],vector[2],
									texture->clip_field->scalar[i]);
								}
#endif /* defined (DEBUG) */
							}
							scalar_field_list[n_scalar_fields]=texture->clip_field;
							isovalue_list[n_scalar_fields]=0;
							n_scalar_fields++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
"generate_clipped_GT_voltex_from_FE_element.  Could not create new coordinate field");
						}
					}
				}
#if defined (CODE_FRAGMENTS)
				if (texture->cutting_plane_on)
				{
					scalar_field_list[n_scalar_fields]=texture->clip_field;
					isovalue_list[n_scalar_fields]=texture->cutting_plane[3];
					n_scalar_fields++;
				}
#endif
				if (texture->hollow_mode_on)
				{
					scalar_field_list[n_scalar_fields]=texture->clip_field2;
					isovalue_list[n_scalar_fields]=
						texture->hollow_isovalue*texture->isovalue;
					n_scalar_fields++;
				}
				if (texture->mc_iso_surface)
				{
					/* calculate detail map here */
					calculate_detail_map(texture,texture->mc_iso_surface);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"generate_clipped_GT_voltex_from_FE_element.  Missing mc_iso_surface");
				}
#if defined (DEBUG)
				/*???debug */
				printf("Clipped Marching cubes: isovalue=%lf : n_scalar_fields=%d : mode [hollow=%d] [closed=%d] [clip=%d]\n",
					texture->isovalue,n_scalar_fields,texture->hollow_mode_on,
					texture->closed_surface,texture->cutting_plane_on);
#endif /* defined (DEBUG) */
				/* create isosurface */
				/* ( cutting plane[3]=clip isovalue ) */
				if (texture->recalculate)
				{
#if defined (DEBUG)
					/*???debug */
					printf("Creating Isosurface for texture %s\n", texture->name);
#endif /* defined (DEBUG) */
					if (marching_cubes(scalar_field_list,n_scalar_fields,
						texture->coordinate_field,texture->mc_iso_surface,isovalue_list,
						texture->closed_surface,
						1 /*(texture->cutting_plane_on)||(texture->hollow_mode_on)*/,
						texture->decimation))
					{
						texture->recalculate=0;
						/* set polygon materials from texture data */
						if (calculate_mc_material(texture,texture->mc_iso_surface))
						{
							for (i=0;i<texture->mc_iso_surface->n_triangles;i++)
							{
								triangle = texture->mc_iso_surface->compiled_triangle_list[i];
								face_index = triangle->triangle_list_index + 1;
								if (face_index > 0 && face_index <= 6)
								{
									/* special case - texture projection on actual face */
									mc_face_cube_map_function(&(triangle->env_map_index[0]),
										triangle->texture_coord[0],
										triangle->vertices[0],
										triangle->iso_poly_cop[0],face_index,
										texture->ximin,texture->ximax);
									mc_face_cube_map_function(&(triangle->env_map_index[1]),
										triangle->texture_coord[1],
										triangle->vertices[1],
										triangle->iso_poly_cop[1],face_index,
										texture->ximin,texture->ximax);
									mc_face_cube_map_function(&(triangle->env_map_index[2]),
										triangle->texture_coord[2],
										triangle->vertices[2],
										triangle->iso_poly_cop[2],face_index,
										texture->ximin,texture->ximax);
								}
								else
								{
									/* general case */
									mc_cube_map_function(&(triangle->env_map_index[0]),
										triangle->texture_coord[0],
										triangle->vertices[0],
										triangle->iso_poly_cop[0],
										texture->ximin,texture->ximax);
									mc_cube_map_function(&(triangle->env_map_index[1]),
										triangle->texture_coord[1],
										triangle->vertices[1],
										triangle->iso_poly_cop[1],
										texture->ximin,texture->ximax);
									mc_cube_map_function(&(triangle->env_map_index[2]),
										triangle->texture_coord[2],
										triangle->vertices[2],
										triangle->iso_poly_cop[2],
										texture->ximin,texture->ximax);
								}
							}

#if defined (OLD_CODE)
							/* calculate projected texture */
							for (i=0;i<texture->iso_surface->n_iso_polys;i++)
							{
								face_index = 0;
								if (i<texture->iso_surface->xi_face_poly_index[0])
									face_index = 1;
								else if (i<texture->iso_surface->xi_face_poly_index[1])
									face_index = 2;
								else if (i<texture->iso_surface->xi_face_poly_index[2])
									face_index = 3;
								else if (i<texture->iso_surface->xi_face_poly_index[3])
									face_index = 4;
								else if (i<texture->iso_surface->xi_face_poly_index[4])
									face_index = 5;
								else if (i<texture->iso_surface->xi_face_poly_index[5])
									face_index = 6;
								if (face_index)
								{
									/* special case - texture projection on actual face */
									face_cube_map_function(&(texture->texturemap_index[3*i]),
										texture->texturemap_coord[3*i],
										&(texture->iso_surface->vertex_list[texture->iso_surface->
										triangle_list[i][0]]),&(texture->iso_poly_cop[i*3][0]),
										face_index,texture->ximin,texture->ximax);
									face_cube_map_function(&(texture->texturemap_index[3*i+1]),
										texture->texturemap_coord[3*i+1],
										&(texture->iso_surface->vertex_list[texture->iso_surface->
										triangle_list[i][1]]),&(texture->iso_poly_cop[i*3+1][0]),
										face_index,texture->ximin,texture->ximax);
									face_cube_map_function(&(texture->texturemap_index[3*i+2]),
										texture->texturemap_coord[3*i+2],
										&(texture->iso_surface->vertex_list[texture->iso_surface->
										triangle_list[i][2]]),&(texture->iso_poly_cop[i*3+2][0]),
										face_index,texture->ximin,texture->ximax);
								}
								else
								{
									/* general case */
									cube_map_function(&(texture->texturemap_index[3*i]),
										texture->texturemap_coord[3*i],
										&(texture->iso_surface->vertex_list[texture->iso_surface->
										triangle_list[i][0]]),&(texture->iso_poly_cop[i*3][0]),
										texture->ximin,texture->ximax);
									cube_map_function(&(texture->texturemap_index[3*i+1]),
										texture->texturemap_coord[3*i+1],
										&(texture->iso_surface->vertex_list[texture->iso_surface->
										triangle_list[i][1]]),&(texture->iso_poly_cop[i*3+1][0]),
										texture->ximin,texture->ximax);
									cube_map_function(&(texture->texturemap_index[3*i+2]),
										texture->texturemap_coord[3*i+2],
										&(texture->iso_surface->vertex_list[texture->iso_surface->
										triangle_list[i][2]]),&(texture->iso_poly_cop[i*3+2][0]),
										texture->ximin,texture->ximax);
								}
							}
#endif /* defined (OLD_CODE) */
							voltex=create_GT_voltex_from_FE_element(element,
								coordinate_field,data_field,texture,render_type,
								displacement_map_field, displacement_map_xi_direction,
								blur_field, texture_coordinate_field, time);
						}
						else
						{
							display_message(ERROR_MESSAGE,
			"generate_clipped_GT_voltex_from_FE_element.  Error calculate_materials");
							voltex=(struct GT_voltex *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"generate_clipped_GT_voltex_from_FE_element.  Error marching_cubes");
						voltex=(struct GT_voltex *)NULL;
					}
				}
				else
				{
#if defined (DEBUG)
					/*???debug */
					printf("Skipping: Isosurface already created for texture %s\n",
						texture->name);
#endif /* defined (DEBUG) */
					voltex=create_GT_voltex_from_FE_element(element,coordinate_field,
						data_field, texture, render_type,
						displacement_map_field, displacement_map_xi_direction,
						blur_field, texture_coordinate_field, time);
#if defined (DEBUG)
					/*???debug */
					printf("After create GT Voltex (2)\n");
#endif /* defined (DEBUG) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"generate_clipped_GT_voltex_from_FE_element.  Error updating scalars");
				voltex=(struct GT_voltex *)NULL;
			}
		}
		else
		{
#if defined (DEBUG)
			/*???debug */
			printf("Skipping: Obj surface already created for texture %s\n",
				texture->name);
#endif /* defined (DEBUG) */
			voltex=create_GT_voltex_from_FE_element(element,
				coordinate_field, data_field, texture, render_type,
				displacement_map_field, displacement_map_xi_direction,
				blur_field, texture_coordinate_field, time);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"generate_clipped_GT_voltex_from_FE_element.  Missing texture");
		voltex=(struct GT_voltex *)NULL;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave generate_clipped_GT_voltex_from_FE_element %p\n",voltex);
#endif /* defined (DEBUG) */
	LEAVE;

	return (voltex);
} /* generate_clipped_GT_voltex_from_FE_element */

int element_to_cylinder(struct FE_element *element,
	void *void_element_to_cylinder_data)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Converts a finite element into a cylinder.
==============================================================================*/
{
	struct GT_surface *surface_element;
	int return_code;
	struct Element_to_cylinder_data *element_to_cylinder_data;

	ENTER(element_to_cylinder);
	/* check arguments */
	if (element&&(element_to_cylinder_data=
		(struct Element_to_cylinder_data *)void_element_to_cylinder_data))
	{
		if (FE_region_FE_element_meets_topological_criteria(
			element_to_cylinder_data->fe_region, element, 1, CM_LINE,
			element_to_cylinder_data->exterior,
			element_to_cylinder_data->face_number))
		{
			if (surface_element=create_cylinder_from_FE_element(element,
				element_to_cylinder_data->coordinate_field,
				element_to_cylinder_data->data_field,
				element_to_cylinder_data->constant_radius,
				element_to_cylinder_data->scale_factor,
				element_to_cylinder_data->radius_field,
				element_to_cylinder_data->number_of_segments_along,
				element_to_cylinder_data->number_of_segments_around,
				element_to_cylinder_data->texture_coordinate_field,
				/*top_level_element*/(struct FE_element *)NULL,
				element_to_cylinder_data->time))
			{
				if (!(return_code=GT_OBJECT_ADD(GT_surface)(
					element_to_cylinder_data->graphics_object,
					element_to_cylinder_data->time,surface_element)))
				{
					DESTROY(GT_surface)(&surface_element);
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"element_to_cylinder.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_cylinder */

int element_to_polyline(struct FE_element *element,
	void *element_to_polyline_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Converts a finite element into a polyline and adds it to a graphics_object.
==============================================================================*/
{
	int return_code;
	struct Element_to_polyline_data *element_to_polyline_data;
	struct GT_polyline *polyline;

	ENTER(element_to_polyline);
	if (element&&(element_to_polyline_data=
		(struct Element_to_polyline_data *)element_to_polyline_data_void))
	{
		if (FE_region_FE_element_meets_topological_criteria(
			element_to_polyline_data->fe_region, element, 1, CM_LINE,
			element_to_polyline_data->exterior,
			element_to_polyline_data->face_number))
		{
			if (polyline=create_GT_polyline_from_FE_element(element,
				element_to_polyline_data->coordinate_field,
				element_to_polyline_data->data_field,
				element_to_polyline_data->number_of_segments_in_xi1,
				/*top_level_element*/(struct FE_element *)NULL,
				element_to_polyline_data->time))
			{
				if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
					element_to_polyline_data->graphics_object,
					element_to_polyline_data->time,polyline)))
				{
					DESTROY(GT_polyline)(&polyline);
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"element_to_polyline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_polyline */

int element_to_surface(struct FE_element *element,
	void *void_element_to_surface_data)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Converts a finite element into a surface.
==============================================================================*/
{
	struct GT_nurbs *nurb;
	struct GT_surface *surface;
	int return_code;
	struct Element_to_surface_data *element_to_surface_data;

	ENTER(element_to_surface);
	if (element&&(element_to_surface_data=
		(struct Element_to_surface_data *)void_element_to_surface_data))
	{
		if (FE_region_FE_element_meets_topological_criteria(
			element_to_surface_data->fe_region, element, 2, CM_FACE,
			element_to_surface_data->exterior, element_to_surface_data->face_number))
		{
			switch(element_to_surface_data->object_type)
			{
				case g_SURFACE:
				{
					if (surface=create_GT_surface_from_FE_element(element,
						element_to_surface_data->coordinate_field,
						element_to_surface_data->texture_coordinate_field,
						element_to_surface_data->data_field,
						element_to_surface_data->number_of_segments_in_xi1,
						element_to_surface_data->number_of_segments_in_xi2,
						element_to_surface_data->reverse_normals,
						/*top_level_element*/(struct FE_element *)NULL,
					   element_to_surface_data->render_type,
						element_to_surface_data->time))
					{
						if (!(return_code=GT_OBJECT_ADD(GT_surface)(
							element_to_surface_data->graphics_object,
							element_to_surface_data->time,surface)))
						{
							DESTROY(GT_surface)(&surface);
						}
					}
					else
					{
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
					if (nurb=create_GT_nurb_from_FE_element(element,
						element_to_surface_data->coordinate_field,
						element_to_surface_data->texture_coordinate_field,
						/*top_level_element*/(struct FE_element *)NULL,
						element_to_surface_data->time))
					{
						if (!(return_code=GT_OBJECT_ADD(GT_nurbs)(
							element_to_surface_data->graphics_object,
							element_to_surface_data->time,nurb)))
						{
							DESTROY(GT_nurbs)(&nurb);
						}
					}
					else
					{
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"element_to_surface.  Unknown object type");
					return_code=0;
				}
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"element_to_surface.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_surface */

int set_Displacement_map(struct Parse_state *state,
	void *displacement_map_void,void *texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1998

DESCRIPTION :
A modifier function for setting the displacement map.
???RC Move to texture? volume_texture?
==============================================================================*/
{
	char *current_token,*texture_name;
	int return_code;
	struct Displacement_map *displacement_map;

	ENTER(set_Displacement_map);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (displacement_map=(struct Displacement_map *)displacement_map_void)
				{
					if (return_code=set_Texture(state,
						(void *)(&(displacement_map->texture)),texture_manager_void))
					{
						if (return_code=set_double(state,
							(void *)(&(displacement_map->scale)),(void *)NULL))
						{
							if (return_code=set_int(state,
								(void *)(&(displacement_map->xi_direction)),(void *)NULL))
							{
								if (!((1==displacement_map->xi_direction)||
									(2==displacement_map->xi_direction)||
									(3==displacement_map->xi_direction)||
									(12==displacement_map->xi_direction)||
									(13==displacement_map->xi_direction)||
									(23==displacement_map->xi_direction)))
								{
									display_message(ERROR_MESSAGE,"Invalid xi direction");
									display_parse_state_location(state);
									return_code=0;
								}
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Displacement_map.  Missing displacement_map");
					return_code=0;
				}
			}
			else
			{
				displacement_map=(struct Displacement_map *)displacement_map_void;
				display_message(INFORMATION_MESSAGE," TEXTURE_NAME");
				if (displacement_map&&(displacement_map->texture))
				{
					if (GET_NAME(Texture)(displacement_map->texture,&texture_name))
					{
						display_message(INFORMATION_MESSAGE,"[%s]",texture_name);
						DEALLOCATE(texture_name);
					}
				}
				display_message(INFORMATION_MESSAGE," SCALE");
				if (displacement_map)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",displacement_map->scale);
				}
				display_message(INFORMATION_MESSAGE,"{real} XI_DIRECTION");
				if (displacement_map)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",
						displacement_map->xi_direction);
				}
				display_message(INFORMATION_MESSAGE, "{1,2,3,12,13,23 ,integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Displacement_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Displacement_map */

int element_to_glyph_set(struct FE_element *element,
	void *element_to_glyph_set_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information about the
fields defined over it.
==============================================================================*/
{
	struct GT_glyph_set *glyph_set;
	int i, number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], number_of_xi_points,
		return_code, top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Element_to_glyph_set_data *element_to_glyph_set_data;
	struct FE_element *top_level_element;
	Triple *xi_points;

	ENTER(element_to_glyph_set);
	if (element && (element_to_glyph_set_data=
		(struct Element_to_glyph_set_data *)element_to_glyph_set_data_void))
	{
		return_code=1;
		/* determine if the element is required */
		if (FE_region_FE_element_meets_topological_criteria(
			element_to_glyph_set_data->fe_region, element,
			Use_element_type_dimension(element_to_glyph_set_data->use_element_type),
			Use_element_type_CM_element_type(
				element_to_glyph_set_data->use_element_type),
			element_to_glyph_set_data->exterior,
			element_to_glyph_set_data->face_number))
		{
			top_level_element = (struct FE_element *)NULL;
			for (i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
			{
				top_level_number_in_xi[i] = element_to_glyph_set_data->number_in_xi[i];
			}
			/* determine discretization of element for graphic */
			if (FE_region_get_FE_element_discretization(
				element_to_glyph_set_data->fe_region, element,
				element_to_glyph_set_data->face_number,
				element_to_glyph_set_data->native_discretization_field,
				top_level_number_in_xi, &top_level_element, number_in_xi))
			{
				if (FE_element_get_xi_points(element,
					element_to_glyph_set_data->xi_discretization_mode, number_in_xi,
					element_to_glyph_set_data->exact_xi,
					element_to_glyph_set_data->coordinate_field,
					element_to_glyph_set_data->xi_point_density_field,
					&number_of_xi_points, &xi_points, element_to_glyph_set_data->time))
				{
					if (glyph_set = create_GT_glyph_set_from_FE_element(
						element, top_level_element,
						element_to_glyph_set_data->coordinate_field,
						number_of_xi_points, xi_points,
						element_to_glyph_set_data->glyph,
						element_to_glyph_set_data->base_size,
						element_to_glyph_set_data->centre,
						element_to_glyph_set_data->scale_factors,
						element_to_glyph_set_data->orientation_scale_field,
						element_to_glyph_set_data->variable_scale_field,
						element_to_glyph_set_data->data_field,
						element_to_glyph_set_data->label_field,
						element_to_glyph_set_data->select_mode,
						/*element_selected*/0,
						(struct Multi_range *)NULL,
						/*point_numbers*/(int *)NULL,
						element_to_glyph_set_data->time))
					{
						if (!GT_OBJECT_ADD(GT_glyph_set)(
							element_to_glyph_set_data->graphics_object,
							element_to_glyph_set_data->time,glyph_set))
						{
							DESTROY(GT_glyph_set)(&glyph_set);
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
					DEALLOCATE(xi_points);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"element_to_glyph_set.  Error getting xi points");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"element_to_glyph_set.  Error getting discretization");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_to_glyph_set.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_glyph_set */

int element_to_volume(struct FE_element *element,
	void *void_element_to_volume_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2000

DESCRIPTION :
Converts a 3-D element into a volume.
==============================================================================*/
{
	struct GT_voltex *volume_element;
	int return_code;
	struct Element_to_volume_data *element_to_volume_data;

	ENTER(element_to_volume);
/*???debug */
/*printf("enter element_to_volume\n");*/
	/* check arguments */
	if (element&&(element_to_volume_data=
		(struct Element_to_volume_data *)void_element_to_volume_data))
	{
		/* determine if the element is required */
		if (3 == get_FE_element_dimension(element))
		{
			if (volume_element= generate_clipped_GT_voltex_from_FE_element(
				element_to_volume_data->clipping,
				element,
				element_to_volume_data->coordinate_field,
				element_to_volume_data->data_field,
				element_to_volume_data->volume_texture,
				element_to_volume_data->render_type,
				element_to_volume_data->displacement_map_field,
				element_to_volume_data->displacement_map_xi_direction,
				element_to_volume_data->blur_field,
				element_to_volume_data->texture_coordinate_field,
				element_to_volume_data->time))
			{
				if (return_code=GT_OBJECT_ADD(GT_voltex)(
					element_to_volume_data->graphics_object,
					element_to_volume_data->time,volume_element))
				{
					if(element_to_volume_data->surface_data_density_field &&
						element_to_volume_data->surface_data_fe_region)
					{
						return_code = create_FE_nodes_on_GT_voltex_surface(
							volume_element,
							element,
							element_to_volume_data->coordinate_field,
							element_to_volume_data->volume_texture,
							element_to_volume_data->surface_data_fe_region,
							element_to_volume_data->surface_data_density_field,
							element_to_volume_data->surface_data_coordinate_field,
							element_to_volume_data->time);
					}
					else
					{
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"element_to_volume.  Unable to add voltex to graphics object");
					return_code=0;
					DESTROY(GT_voltex)(&volume_element);
				}
			}
			else
			{
				/* the element has been totally clipped */
				return_code=1;
#if defined (OLD_CODE)
/*???debug */
printf("generate_clipped_GT_voltex_from_FE_element failed\n");
				DESTROY(GT_object)(&(element_to_volume_data->graphics_object));
				return_code=0;
#endif /* defined (OLD_CODE) */
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"element_to_volume.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
/*???debug */
/*printf("leave element_to_volume %d\n",return_code);*/

	return (return_code);
} /* element_to_volume */

int element_to_iso_scalar(struct FE_element *element,
	void *element_to_iso_scalar_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Computes iso-surfaces/lines/points graphics from <element>.
==============================================================================*/
{
	int i,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],return_code,
		top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Element_to_iso_scalar_data *element_to_iso_scalar_data;
	struct FE_element *top_level_element;

	ENTER(element_to_iso_scalar);
	if (element && (element_to_iso_scalar_data=
		(struct Element_to_iso_scalar_data *)element_to_iso_scalar_data_void))
	{
		return_code=1;
		/* determine if the element is required */
		if (FE_region_FE_element_meets_topological_criteria(
			element_to_iso_scalar_data->fe_region, element,
			Use_element_type_dimension(element_to_iso_scalar_data->use_element_type),
			Use_element_type_CM_element_type(
				element_to_iso_scalar_data->use_element_type),
			element_to_iso_scalar_data->exterior,
			element_to_iso_scalar_data->face_number))
		{
			top_level_element=(struct FE_element *)NULL;
			for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
			{
				top_level_number_in_xi[i]=element_to_iso_scalar_data->number_in_xi[i];
			}
			/* determine discretization of element for graphic */
			if (FE_region_get_FE_element_discretization(
				element_to_iso_scalar_data->fe_region, element,
				element_to_iso_scalar_data->face_number,
				element_to_iso_scalar_data->native_discretization_field,
				top_level_number_in_xi,&top_level_element,number_in_xi))
			{
				switch (element_to_iso_scalar_data->graphics_object->object_type)
				{
					case g_VOLTEX:
					{
						if (3==get_FE_element_dimension(element))
						{
							return_code=create_iso_surfaces_from_FE_element(element,
								element_to_iso_scalar_data->iso_value,
								element_to_iso_scalar_data->time,
								element_to_iso_scalar_data->clipping,
								element_to_iso_scalar_data->coordinate_field,
								element_to_iso_scalar_data->data_field,
								element_to_iso_scalar_data->scalar_field,
								element_to_iso_scalar_data->surface_data_density_field,
								element_to_iso_scalar_data->surface_data_coordinate_field,
								element_to_iso_scalar_data->texture_coordinate_field,
								number_in_xi,
								element_to_iso_scalar_data->graphics_object,
								element_to_iso_scalar_data->render_type,
								element_to_iso_scalar_data->surface_data_fe_region);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Cannot add iso_lines of 2-D element to voltex");
						}
					} break;
					case g_POLYLINE:
					{
						if (2==get_FE_element_dimension(element))
						{
							return_code=create_iso_lines_from_FE_element(element,
								element_to_iso_scalar_data->coordinate_field,
								element_to_iso_scalar_data->scalar_field,
								element_to_iso_scalar_data->iso_value,
								element_to_iso_scalar_data->time,
								element_to_iso_scalar_data->data_field,
								number_in_xi[0],number_in_xi[1],top_level_element,
								element_to_iso_scalar_data->graphics_object,
								/*graphics_object_time*/0.0);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Cannot add iso_surfaces of 3-D element to polyline");
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,"element_to_iso_scalar.  "
							"Invalid graphic type for iso_scalar");
						return_code=0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"element_to_iso_scalar.  Error getting discretization");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_to_iso_scalar.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_iso_scalar */

int create_iso_surfaces_from_FE_element(struct FE_element *element,
	double iso_value, FE_value time,struct Clipping *clipping,
	struct Computed_field *coordinate_field,
	struct Computed_field *data_field,struct Computed_field *scalar_field,
	struct Computed_field *surface_data_density_field,
	struct Computed_field *surface_data_coordinate_field,
	struct Computed_field *texture_coordinate_field,
	int *number_in_xi,
	struct GT_object *graphics_object,enum Render_type render_type,
	struct FE_region *surface_data_fe_region)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Converts a 3-D element into an iso_surface (via a volume_texture).
==============================================================================*/
{
	enum FE_element_shape_type shape_type;
	FE_value scalar_value,xi[3];
	struct FE_element_shape *element_shape;
	struct GT_voltex *iso_surface_voltex;
	int cell_limit, *detail_map, i, linked_xi1, linked_xi2, number_of_passes,
		number_of_polygon_vertices, number_in_xi1, number_in_xi2, number_in_xi3, number_of_volume_texture_cells,
		number_of_volume_texture_nodes, pass, return_code, xi3_step;
	struct MC_cell **mc_cell;
	struct MC_iso_surface *mc_iso_surface;
	struct VT_texture_cell *cell,*cell_block,**cell_list;
	struct VT_texture_node *node,*node_block,**node_list;
	struct VT_volume_texture *volume_texture;

	ENTER(create_iso_surfaces_from_FE_element);
	/* default return_code */
	return_code=0;
	if (element && (3 == get_FE_element_dimension(element)) &&
		get_FE_element_shape(element, &element_shape) &&
		Computed_field_has_3_components(coordinate_field,NULL)&&
		number_in_xi&&(0<number_in_xi[0])&&(0<number_in_xi[1])&&(0<number_in_xi[2])
		&&scalar_field&&(1==Computed_field_get_number_of_components(scalar_field)))
	{
		number_in_xi1=number_in_xi[0];
		number_in_xi2=number_in_xi[1];
		number_in_xi3=number_in_xi[2];
		/* check for polygons */
		/*???DB.  Only for linear polygons */
		/* make sure that polygon vertices land on grid points */
		/*???DB.  For a polygon the "radial" component has to be second ? */
		linked_xi1 = -1;
		if (get_FE_element_shape_xi_shape_type(element_shape, /*xi_number*/0,
					&shape_type) && (POLYGON_SHAPE == shape_type))
		{
			linked_xi1 = 0;
		}
		if (get_FE_element_shape_xi_shape_type(element_shape, /*xi_number*/1,
					&shape_type) && (POLYGON_SHAPE == shape_type))
		{
			if (0 <= linked_xi1)
			{
				linked_xi2 = 1;
			}
			else
			{
				linked_xi1 = 1;
				linked_xi2 = 2;
			}
		}
		else if (0 <= linked_xi1)
		{
			linked_xi2 = 2;
		}
		if (0 <= linked_xi1)
		{
			if (get_FE_element_shape_xi_linkage_number(element_shape,
						linked_xi1, linked_xi2, &number_of_polygon_vertices) &&
				(0 < number_of_polygon_vertices))
			{
				if (0 == linked_xi1)
				{
					number_in_xi1 = (1 + (number_in_xi1 - 1)/number_of_polygon_vertices)
						* number_of_polygon_vertices;
				}
				else
				{
					number_in_xi2 = (1 + (number_in_xi2 - 1)/number_of_polygon_vertices)
						* number_of_polygon_vertices;
				}
			}
		}

		/* SAB 17 March 2003  To reduce the maximum amount of memory used in generating
			these isosurfaces I am dividing up the xi3 direction if the number of cells is
			greater than this limit.  A sensible value for this threshhold is dependent
			on the memory commonly available in computers being used and so will need to be
			increased in the future */
		cell_limit = 100000;
		/* If the number in xi is large we will divide it up into slices */
		if (number_in_xi1 * number_in_xi2 * number_in_xi3 > cell_limit)
		{
			xi3_step = number_in_xi3;
			while ((xi3_step > 1) && 
				(number_in_xi1 * number_in_xi2 * xi3_step > cell_limit))
			{
				xi3_step /= 2;
			}
			number_of_passes = (number_in_xi3 + xi3_step - 1) / xi3_step;
		}
		else
		{
			number_of_passes = 1;
			xi3_step = number_in_xi3;
		}
		for (pass = 0 ; pass < number_of_passes ; pass++)
		{
			if (number_of_passes > 1)
			{
				if (pass == (number_of_passes - 1))
				{
					number_in_xi3 = number_in_xi[2] - (number_of_passes - 1) * xi3_step;
				}
				else
				{
					number_in_xi3 = xi3_step;
				}
			}
			/* create the volume texture */
			if (volume_texture=CREATE(VT_volume_texture)((char *)NULL))
			{
				/* fill in the volume texture */
				number_of_volume_texture_cells=
					number_in_xi1*number_in_xi2*number_in_xi3;
				number_of_volume_texture_nodes=
					(number_in_xi1+1)*(number_in_xi2+1)*(number_in_xi3+1);
				if (ALLOCATE(volume_texture->scalar_field->scalar,double,
						 number_of_volume_texture_nodes)&&
					ALLOCATE(volume_texture->clip_field->scalar,double,
						number_of_volume_texture_nodes)&&
					ALLOCATE(volume_texture->clip_field2->scalar,double,
						number_of_volume_texture_nodes)&&
					ALLOCATE(volume_texture->coordinate_field->vector,double,
						3*number_of_volume_texture_nodes)&&
					ALLOCATE(mc_iso_surface,struct MC_iso_surface,1)&&
					ALLOCATE(detail_map,int,(number_in_xi1+2)*(number_in_xi2+2)*
						(number_in_xi3+2))&&
					ALLOCATE(mc_cell,struct MC_cell *,(number_in_xi1+3)*
						(number_in_xi2+3)*(number_in_xi3+3))&&
					ALLOCATE(cell_list,struct VT_texture_cell *,
						number_of_volume_texture_cells)&&
					ALLOCATE(node_list,struct VT_texture_node *,
						number_of_volume_texture_nodes) &&
					ALLOCATE(cell_block,struct VT_texture_cell,
						number_of_volume_texture_cells) &&
					ALLOCATE(node_block,struct VT_texture_node,
						number_of_volume_texture_nodes))
					/*???DB.  Not freeing properly */
				{
					/* set up iso-surface */
					(mc_iso_surface->dimension)[0]=number_in_xi1;
					(mc_iso_surface->dimension)[1]=number_in_xi2;
					(mc_iso_surface->dimension)[2]=number_in_xi3;
					mc_iso_surface->detail_map=detail_map;
					mc_iso_surface->mc_cells=mc_cell;
					for (i=(number_in_xi1+2)*(number_in_xi2+2)*(number_in_xi3+2);i>0;i--)
					{
						*detail_map=0;
						detail_map++;
					}
					for (i=(number_in_xi1+3)*(number_in_xi2+3)*(number_in_xi3+3);i>0;i--)
					{
						*mc_cell=(struct MC_cell *)NULL;
						mc_cell++;
					}
					/* all vertices deformable */
					mc_iso_surface->deform=(int *)NULL;
					mc_iso_surface->compiled_triangle_list=(struct MC_triangle **)NULL;
					mc_iso_surface->compiled_vertex_list=(struct MC_vertex **)NULL;
					mc_iso_surface->n_scalar_fields=0;
					mc_iso_surface->n_vertices=0;
					mc_iso_surface->n_triangles=0;
					(mc_iso_surface->active_block)[0]=1;
					(mc_iso_surface->active_block)[1]=number_in_xi1;
					(mc_iso_surface->active_block)[2]=1;
					(mc_iso_surface->active_block)[3]=number_in_xi2;
					(mc_iso_surface->active_block)[4]=1;
					(mc_iso_surface->active_block)[5]=number_in_xi3;
					volume_texture->mc_iso_surface=mc_iso_surface;
					/* set the cell values */
					i=0;
					while (i<number_of_volume_texture_cells)
					{
						cell=cell_block+i;
						cell_list[i]=cell;
						(cell->cop)[0]=0.5;
						(cell->cop)[1]=0.5;
						(cell->cop)[2]=0.5;
						cell->material=(struct Graphical_material *)NULL;
						cell->scalar_value=1.0;
						cell->env_map=(struct Environment_map *)NULL;
						cell->detail=0;
						cell->interpolation_fn=0;
						i++;
					}
					if (i==number_of_volume_texture_cells)
					{
						/* set the node values */
						i=0;
						xi[0]=0;
						xi[1]=0;
						return_code=1;
						while (return_code&&(i<number_of_volume_texture_nodes))
						{
							xi[0] = (float)(i % (number_in_xi1+1)) / (float)number_in_xi1; 
							xi[1] = (float)((i / (number_in_xi1+1)) % (number_in_xi2+1)) / (float)number_in_xi2; 
							/* We may start part way through, use number_in_xi2 as this is the total dimension */
							xi[2] = (float)(((i / ((number_in_xi1+1) * (number_in_xi2+1))))
								+ pass * xi3_step) / (float)number_in_xi[2];
							if (Computed_field_evaluate_in_element(scalar_field,
									 element,xi,time,/*top_level_element*/(struct FE_element *)NULL,
									 &scalar_value,(FE_value *)NULL))
							{
								node=node_block+i;
								node_list[i]=node;
								(node->cop)[0]=0.5;
								(node->cop)[1]=0.5;
								(node->cop)[2]=0.5;
								node->material=(struct Graphical_material *)NULL;
								node->dominant_material=(struct Graphical_material *)NULL;
								node->scalar_value=(double)scalar_value;
								/*???RC no clipping function for now */
								node->clipping_fn_value=(double)1.0;
								i++;
							}
							else
							{
								return_code=0;
							}
						}
						/* clear Computed_field caches so elements not accessed */
						Computed_field_clear_cache(scalar_field);
						if (i==number_of_volume_texture_nodes)
						{
							volume_texture->texture_cell_list=cell_list;
							volume_texture->global_texture_node_list=node_list;
							volume_texture->isovalue=
								iso_value;
							volume_texture->hollow_isovalue=
								0.75*(volume_texture->isovalue);
							volume_texture->hollow_mode_on=0;
							volume_texture->cutting_plane_on=0;
							/* allow the clipping function to be defined nodally like
								the scalar field */
							volume_texture->clipping_field_on=1;
							volume_texture->cut_isovalue=0;
							volume_texture->cutting_plane[0]=0;
							volume_texture->cutting_plane[1]=0;
							volume_texture->cutting_plane[2]=0;
							volume_texture->cutting_plane[3]=0;
							volume_texture->closed_surface=0;
							volume_texture->decimation=0;
							/* set the xi ranges */
							(volume_texture->ximin)[0]=0;
							(volume_texture->ximax)[0]=1;
							(volume_texture->ximin)[1]=0;
							(volume_texture->ximax)[1]=1;
							(volume_texture->ximin)[2]=(float)(pass * xi3_step) / (float)number_in_xi[2];
							(volume_texture->ximax)[2]=(float)((pass + 1) * xi3_step) / (float)number_in_xi[2];
							if (volume_texture->ximax[2] > 1.0)
							{
								volume_texture->ximax[2] = 1.0;
							}
							/* set the discretization */
							(volume_texture->dimension)[0]=number_in_xi1;
							(volume_texture->dimension)[1]=number_in_xi2;
							(volume_texture->dimension)[2]=number_in_xi3;
							(volume_texture->scalar_field->dimension)[0]=number_in_xi1;
							(volume_texture->scalar_field->dimension)[1]=number_in_xi2;
							(volume_texture->scalar_field->dimension)[2]=number_in_xi3;
							(volume_texture->clip_field->dimension)[0]=number_in_xi1;
							(volume_texture->clip_field->dimension)[1]=number_in_xi2;
							(volume_texture->clip_field->dimension)[2]=number_in_xi3;
							(volume_texture->clip_field2->dimension)[0]=number_in_xi1;
							(volume_texture->clip_field2->dimension)[1]=number_in_xi2;
							(volume_texture->clip_field2->dimension)[2]=number_in_xi3;
							(volume_texture->coordinate_field->dimension)[0]=
								number_in_xi1;
							(volume_texture->coordinate_field->dimension)[1]=
								number_in_xi2;
							(volume_texture->coordinate_field->dimension)[2]=
								number_in_xi3;
							volume_texture->calculate_nodal_values=0;
							volume_texture->disable_volume_functions=0;
							if (iso_surface_voltex=generate_clipped_GT_voltex_from_FE_element(
									 clipping,element,coordinate_field,data_field,
									 volume_texture,render_type,NULL,0,NULL,texture_coordinate_field,
									 time))
							{
								if (return_code=GT_OBJECT_ADD(GT_voltex)(
										 graphics_object,
										 /*time*/0,iso_surface_voltex))
								{
									if (surface_data_fe_region && surface_data_density_field)
									{
										return_code = create_FE_nodes_on_GT_voltex_surface(
											iso_surface_voltex,
											element, coordinate_field,
											volume_texture,
											surface_data_fe_region,
											surface_data_density_field,
											surface_data_coordinate_field, time);
									}
									else
									{
										return_code=1;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_iso_surfaces_from_FE_element.  "
										"Unable to add voltex to graphics object");
									return_code=0;
									DESTROY(GT_voltex)(&iso_surface_voltex);
								}
							}
							else
							{
								/* the element has been totally clipped */
								return_code=1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_iso_surfaces_from_FE_element.  "
								"Could not allocate memory for volume_texture nodes");
							while (i>0)
							{
								i--;
								DEALLOCATE(node_list[i]);
							}
							i=number_of_volume_texture_cells;
							while (i>0)
							{
								i--;
								DEALLOCATE(cell_list[i]);
							}
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_iso_surfaces_from_FE_element.  "
							"Could not allocate memory for volume_texture cells");
						while (i>0)
						{
							i--;
							DEALLOCATE(cell_list[i]);
						}
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"create_iso_surfaces_from_FE_element.  "
						"Could not allocate memory for volume_texture");
					return_code=0;
				}
				DESTROY(VT_volume_texture)(&volume_texture);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_iso_surfaces_from_FE_element.  "
					"Could not create volume texture");
				DESTROY(GT_object)(&(graphics_object));
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_iso_surfaces_from_FE_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_iso_surfaces_from_FE_element */

/*******************************************************************************
FILE : computed_field_fibres.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Implements a number of basic continuum mechanics fibres operations on
computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

struct Computed_field_fibres_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_fibre_axes_type_string[] = "fibre_axes";

int Computed_field_is_type_fibre_axes(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_fibre_axes);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_fibre_axes_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_fibre_axes.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_fibre_axes */

static int Computed_field_fibre_axes_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_fibre_axes_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_fibre_axes_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fibre_axes_clear_type_specific */

static void *Computed_field_fibre_axes_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination;

	ENTER(Computed_field_fibre_axes_copy_type_specific);
	if (field)
	{
		/* Return a TRUE value */
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_fibre_axes_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_fibre_axes_copy_type_specific */

#define Computed_field_fibre_axes_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_fibre_axes_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_fibre_axes_type_specific_contents_match);
	if (field && other_computed_field)
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fibre_axes_type_specific_contents_match */

int Computed_field_fibre_axes_is_defined_in_element(
	struct Computed_field *field, struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_fibre_axes_is_defined_in_element);
	if (field && element)
	{
		/* fibre_axes requires at least 2-D elements */
		if (2 <= get_FE_element_dimension(element))
		{
			/* check the source fields */
			return_code = Computed_field_default_is_defined_in_element(field,element);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_fibre_axes_is_defined_in_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fibre_axes_is_defined_in_element */

int Computed_field_fibre_axes_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Returns 0.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_is_defined_at_node);
	USE_PARAMETER(field);
	USE_PARAMETER(node);
	/* fibre_axes can only be calculated in elements */
	return_code=0;
	LEAVE;

	return (return_code);
} /* Computed_field_default_is_defined_at_node */

#define Computed_field_fibre_axes_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_fibre_axes_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

#define Computed_field_fibre_axes_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/

static int Computed_field_fibre_axes_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 30 January 2001

DESCRIPTION :
Compute the three 3-component fibre axes in the order fibre, sheet, normal from
the source fibre and coordinate fields. Function reads the coordinate field and
derivatives in rectangular cartesian coordinates. The 1 to 3 fibre angles in
the fibre_field are used as follows (2 values omit step 3, 1 only does step 1):
1 = fibre_angle in xi1-xi2 plane measured from xi1;
2 = sheet_angle, inclination of the fibres from xi1-xi2 plane after step 1.
3 = imbrication_angle, rotation of the sheet about the fibre vector.
coordinates from the source_field values in an arbitrary
<element_dimension> may be 2 or 3 only.
Derivatives may not be computed for this type of Computed_field [yet].
==============================================================================*/
{
	FE_value a_x, a_y, a_z, alpha, b_x, b_y, b_z, beta, c_x, c_y, c_z, cos_alpha,
		cos_beta, cos_gamma ,dx_dxi[9], f11, f12, f13, f21, f22, f23, f31, f32, f33,
		gamma, length, sin_alpha, sin_beta, sin_gamma,
		top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], x[3];
	int element_dimension,return_code,top_level_element_dimension;
	struct Computed_field *coordinate_field, *fibre_field;

	ENTER(Computed_field_fibre_axes_evaluate_cache_in_element);
	if (field && element && xi)
	{
		if (0 == calculate_derivatives)
		{
			field->derivatives_valid = 0;
			element_dimension = get_FE_element_dimension(element);
			/* 1. Precalculate any source fields that this field depends on.
				 Always need derivatives of coordinate field and further, it must be
				 calculated on the top_level_element */
			fibre_field = field->source_fields[0];
			coordinate_field = field->source_fields[1];
			if (return_code =
				Computed_field_get_top_level_element_and_xi(element,xi,
					element_dimension,&top_level_element,top_level_xi,
					&top_level_element_dimension) &&
				Computed_field_evaluate_cache_in_element(fibre_field,
					element,xi,time,top_level_element,0) &&
				Computed_field_evaluate_cache_in_element(coordinate_field,
					top_level_element,top_level_xi,time,top_level_element,1) &&
				Computed_field_extract_rc(coordinate_field,
					top_level_element_dimension,x,dx_dxi))
			{
				/* 2. Calculate the field */
				/* get f1~ = vector in xi1 direction */
				f11=dx_dxi[0];
				f12=dx_dxi[3];
				f13=dx_dxi[6];
				/* get f2~ = vector in xi2 direction */
				f21=dx_dxi[1];
				f22=dx_dxi[4];
				f23=dx_dxi[7];
				/* get f3~ = vector normal to xi1-xi2 plane */
				f31=f12*f23-f13*f22;
				f32=f13*f21-f11*f23;
				f33=f11*f22-f12*f21;
				/* normalise vectors f1~ and f3~ */
				if (0.0<(length=sqrt(f11*f11+f12*f12+f13*f13)))
				{
					f11 /= length;
					f12 /= length;
					f13 /= length;
				}
				if (0.0<(length=sqrt(f31*f31+f32*f32+f33*f33)))
				{
					f31 /= length;
					f32 /= length;
					f33 /= length;
				}
				/* get vector f2~ = f3~ (x) f1~ = normal to xi1 in xi1-xi2 plane */
				f21=f32*f13-f33*f12;
				f22=f33*f11-f31*f13;
				f23=f31*f12-f32*f11;
				/* get sin/cos of fibre angles alpha, beta and gamma */
				alpha = fibre_field->values[0];
				sin_alpha = sin(alpha);
				cos_alpha = cos(alpha);
				if (1 < fibre_field->number_of_components)
				{
					beta = fibre_field->values[1];
					sin_beta = sin(beta);
					cos_beta = cos(beta);
				}
				else
				{
					/* default beta is 0 */
					sin_beta = 0;
					cos_beta = 1;
				}
				if (2 < fibre_field->number_of_components)
				{
					gamma = fibre_field->values[2];
					sin_gamma = sin(gamma);
					cos_gamma = cos(gamma);
				}
				else
				{
#if defined (OLD_CODE)
					/* default gamma is pi/2 */
					sin_gamma = 1;
					cos_gamma = 0;
#endif /* defined (OLD_CODE) */
					/* default gamma is 0 */
					sin_gamma = 0;
					cos_gamma = 1;
				}
				/* calculate the fibre axes a=fibre, b=sheet, c=normal */
				a_x =  cos_alpha*f11 + sin_alpha*f21;
				a_y =  cos_alpha*f12 + sin_alpha*f22;
				a_z =  cos_alpha*f13 + sin_alpha*f23;
				b_x = -sin_alpha*f11 + cos_alpha*f21;
				b_y = -sin_alpha*f12 + cos_alpha*f22;
				b_z = -sin_alpha*f13 + cos_alpha*f23;
				f11 = a_x;
				f12 = a_y;
				f13 = a_z;
				f21 = b_x;
				f22 = b_y;
				f23 = b_z;
				/* as per KATs change 30Nov00 in back-end function ROT_COORDSYS,
					rotate anticlockwise about axis2, not -axis2 */
#if defined (OLD_CODE)
				a_x =  cos_beta*f11 + sin_beta*f31;
				a_y =  cos_beta*f12 + sin_beta*f32;
				a_z =  cos_beta*f13 + sin_beta*f33;
				c_x = -sin_beta*f11 + cos_beta*f31;
				c_y = -sin_beta*f12 + cos_beta*f32;
				c_z = -sin_beta*f13 + cos_beta*f33;
#endif /* defined (OLD_CODE) */
				c_x =  cos_beta*f31 + sin_beta*f11;
				c_y =  cos_beta*f32 + sin_beta*f12;
				c_z =  cos_beta*f33 + sin_beta*f13;
				a_x = -sin_beta*f31 + cos_beta*f11;
				a_y = -sin_beta*f32 + cos_beta*f12;
				a_z = -sin_beta*f33 + cos_beta*f13;
				f31 = c_x;
				f32 = c_y;
				f33 = c_z;
#if defined (OLD_CODE)
				/* note rearrangement of sin/cos to give equivalent rotation of
					gamma - PI/2.  Note we will probably remove the -PI/2 factor at some
					stage */
				b_x = sin_gamma*f21 - cos_gamma*f31;
				b_y = sin_gamma*f22 - cos_gamma*f32;
				b_z = sin_gamma*f23 - cos_gamma*f33;
				c_x = cos_gamma*f21 + sin_gamma*f31;
				c_y = cos_gamma*f22 + sin_gamma*f32;
				c_z = cos_gamma*f23 + sin_gamma*f33;
#endif /* defined (OLD_CODE) */
				b_x =  cos_gamma*f21 + sin_gamma*f31;
				b_y =  cos_gamma*f22 + sin_gamma*f32;
				b_z =  cos_gamma*f23 + sin_gamma*f33;
				c_x = -sin_gamma*f21 + cos_gamma*f31;
				c_y = -sin_gamma*f22 + cos_gamma*f32;
				c_z = -sin_gamma*f23 + cos_gamma*f33;
				/* put fibre, sheet then normal in field values */
				field->values[0]=a_x;
				field->values[1]=a_y;
				field->values[2]=a_z;
				field->values[3]=b_x;
				field->values[4]=b_y;
				field->values[5]=b_z;
				field->values[6]=c_x;
				field->values[7]=c_y;
				field->values[8]=c_z;
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_fibre_axes_evaluate_cache_in_element.  "
					"Could not evaluate source fields");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_fibre_axes_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of fibre axes");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_fibre_axes_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fibre_axes_evaluate_cache_in_element */

#define Computed_field_fibre_axes_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_fibre_axes_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_fibre_axes_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_fibre_axes_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_fibre_axes_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_fibre_axes_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_fibre_axes(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_fibre_axes);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    fibre field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_fibre_axes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_fibre_axes */

static char *Computed_field_fibre_axes_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_fibre_axes_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_fibre_axes_type_string, &error);
		append_string(&command_string, " coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " fibre ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_fibre_axes_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_fibre_axes_get_command_string */

#define Computed_field_fibre_axes_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_fibre_axes(struct Computed_field *field,
	struct Computed_field *fibre_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FIBRE_AXES, combining a fibre and
coordinate field to return the 3, 3-component fibre axis vectors:
fibre  = fibre direction,
sheet  = fibre normal in the plane of the sheet,
normal = normal to the fibre sheet.
Sets the number of components to 9.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.

Both the fibre and coordinate fields must have no more than 3 components. The
fibre field is expected to have a FIBRE coordinate_system, although this is not
enforced.
???RC To enforce the fibre field to have a FIBRE coordinate_system, must make
the MANAGER_COPY_NOT_IDENTIFIER fail if it would change the coordinate_system
while the field is in use. Not sure if we want that restriction.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **temp_source_fields;

	ENTER(Computed_field_set_type_fibre_axes);
	if (field&&fibre_field&&(3>=fibre_field->number_of_components)&&
		coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(temp_source_fields,struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_fibre_axes_type_string;
			field->number_of_components=9;
			/* source_fields: 0=fibre, 1=coordinate */
			temp_source_fields[0]=ACCESS(Computed_field)(fibre_field);
			temp_source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=temp_source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(fibre_axes);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_fibre_axes.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_fibre_axes */

int Computed_field_get_type_fibre_axes(struct Computed_field *field,
	struct Computed_field **fibre_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FIBRE_AXES, the fibre and coordinate
fields used by it are returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_fibre_axes);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_fibre_axes_type_string) &&
		fibre_field && coordinate_field)
	{
		/* source_fields: 0=fibre, 1=coordinate */
		*fibre_field=field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_fibre_axes */

static int define_Computed_field_type_fibre_axes(struct Parse_state *state,
	void *field_void,void *computed_field_fibres_package_void)
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FIBRE_AXES (if it is not already) and
allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field, *fibre_field, *field;
	struct Computed_field_fibres_package *computed_field_fibres_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fibre_field_data;

	ENTER(define_Computed_field_type_fibre_axes);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_fibres_package=(struct Computed_field_fibres_package *)
		computed_field_fibres_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		fibre_field=(struct Computed_field *)NULL;
		if (computed_field_fibre_axes_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_fibre_axes(field,
				&fibre_field, &coordinate_field);
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (fibre_field)
			{
				ACCESS(Computed_field)(fibre_field);
			}
			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				computed_field_fibres_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&coordinate_field,&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* fibre */
			set_fibre_field_data.computed_field_manager=
				computed_field_fibres_package->computed_field_manager;
			set_fibre_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_fibre_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"fibre",
				&fibre_field,&set_fibre_field_data,set_Computed_field_conditional);
			if (return_code = Option_table_multi_parse(option_table,state))
			{
				return_code = Computed_field_set_type_fibre_axes(field,
					fibre_field, coordinate_field);
			}
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (fibre_field)
			{
				DEACCESS(Computed_field)(&fibre_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_fibre_axes */

int Computed_field_register_types_fibres(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_fibres_package 
		computed_field_fibres_package;

	ENTER(Computed_field_register_types_fibres);
	if (computed_field_package)
	{
		computed_field_fibres_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_fibre_axes_type_string,
			define_Computed_field_type_fibre_axes,
			&computed_field_fibres_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_fibres.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_fibres */

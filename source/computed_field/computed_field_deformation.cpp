/*******************************************************************************
FILE : computed_field_deformation.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Implements a number of basic continuum mechanics deformation operations on
computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

struct Computed_field_deformation_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_2d_strain_type_string[] = "2d_strain";

int Computed_field_is_type_2d_strain(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_2d_strain);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_2d_strain_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_2d_strain.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_2d_strain */

static int Computed_field_2d_strain_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_2d_strain_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_2d_strain_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_2d_strain_clear_type_specific */

static void *Computed_field_2d_strain_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_2d_strain_type_specific_data *destination;

	ENTER(Computed_field_2d_strain_copy_type_specific);
	if (field)
	{
		/* Return a TRUE value */
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_2d_strain_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_2d_strain_copy_type_specific */

#define Computed_field_2d_strain_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_2d_strain_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_2d_strain_type_specific_contents_match);
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
} /* Computed_field_2d_strain_type_specific_contents_match */

int Computed_field_2d_strain_is_defined_in_element(
	struct Computed_field *field, struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_2d_strain_is_defined_in_element);
	if (field && element)
	{
		/* 2d_strain requires at least 2-D elements */
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
			"Computed_field_2d_strain_is_defined_in_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_2d_strain_is_defined_in_element */

int Computed_field_2d_strain_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Returns 0.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_is_defined_at_node);
	USE_PARAMETER(field);
	USE_PARAMETER(node);
	/* 2d_strain can only be calculated in elements */
	return_code=0;
	LEAVE;

	return (return_code);
} /* Computed_field_default_is_defined_at_node */

#define Computed_field_2d_strain_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_2d_strain_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

#define Computed_field_2d_strain_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/

static int Computed_field_2d_strain_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Computes the wrinkle strain. Derivatives are not available.
Source fields are coordinates, undeformed_coordinates and fibre angle.
==============================================================================*/
{
	double A,A2,B,B2,cos_fibre_angle,C2,D,dxi_dnu[6],E[4],fibre_angle, 
		F_x[6],F_X[6],sin_fibre_angle;
	FE_value def_derivative_xi[9], undef_derivative_xi[9];
	int element_dimension,return_code;

	ENTER(Computed_field_2d_strain_evaluate_cache_in_element);
	if (field && element && xi)
	{
		if (0==calculate_derivatives)
		{
			/* 1. Precalculate any source fields that this field depends on.
				 Always need derivatives of deformed and undeformed coordinate fields */
			if (return_code=
				Computed_field_evaluate_cache_in_element(field->source_fields[0],
					element,xi,time,top_level_element,1)&&
				Computed_field_evaluate_cache_in_element(field->source_fields[1],
					element,xi,time,top_level_element,1)&&
				Computed_field_evaluate_cache_in_element(field->source_fields[2],
					element,xi,time,top_level_element,0))
			{
				/* 2. Calculate the field */
				element_dimension=get_FE_element_dimension(element);
				field->derivatives_valid = 0;
				switch(element_dimension)
				{
					case 2:
					{
						def_derivative_xi[0] = field->source_fields[0]->derivatives[0];
						def_derivative_xi[1] = field->source_fields[0]->derivatives[1];
						def_derivative_xi[3] = field->source_fields[0]->derivatives[2];
						def_derivative_xi[4] = field->source_fields[0]->derivatives[3];
						def_derivative_xi[6] = field->source_fields[0]->derivatives[4];
						def_derivative_xi[7] = field->source_fields[0]->derivatives[5];
						undef_derivative_xi[0] = field->source_fields[1]->derivatives[0];
						undef_derivative_xi[1] = field->source_fields[1]->derivatives[1];
						undef_derivative_xi[3] = field->source_fields[1]->derivatives[2];
						undef_derivative_xi[4] = field->source_fields[1]->derivatives[3];
						undef_derivative_xi[6] = field->source_fields[1]->derivatives[4];
						undef_derivative_xi[7] = field->source_fields[1]->derivatives[5];
					} break;
					case 3:
					{
						/* Convert to 2D ignoring xi3, should be able to choose the
							 direction that is ignored */
						def_derivative_xi[0] = field->source_fields[0]->derivatives[0];
						def_derivative_xi[1] = field->source_fields[0]->derivatives[1];
						def_derivative_xi[3] = field->source_fields[0]->derivatives[3];
						def_derivative_xi[4] = field->source_fields[0]->derivatives[4];
						def_derivative_xi[6] = field->source_fields[0]->derivatives[6];
						def_derivative_xi[7] = field->source_fields[0]->derivatives[7];
						undef_derivative_xi[0] = field->source_fields[1]->derivatives[0];
						undef_derivative_xi[1] = field->source_fields[1]->derivatives[1];
						undef_derivative_xi[3] = field->source_fields[1]->derivatives[3];
						undef_derivative_xi[4] = field->source_fields[1]->derivatives[4];
						undef_derivative_xi[6] = field->source_fields[1]->derivatives[6];
						undef_derivative_xi[7] = field->source_fields[1]->derivatives[7];
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_2d_strain.  Unknown element dimension");
						return_code=0;
					} break;
				}
				if (return_code)
				{
					/* do 2D_strain calculation */
					fibre_angle=field->source_fields[2]->values[0];

					/* X is the undeformed coordinates
						 x is the deformed coordinates
						 nu is the fibre coordinate system (2-D,within sheet) */
					/* calculate F_X=dX_dnu and dxi_dnu */
					cos_fibre_angle=cos(fibre_angle);
					sin_fibre_angle=sin(fibre_angle);
					A2=undef_derivative_xi[0]*undef_derivative_xi[0]+
						undef_derivative_xi[3]*undef_derivative_xi[3]+
						undef_derivative_xi[6]*undef_derivative_xi[6];
					A=sqrt(A2);
					B2=undef_derivative_xi[1]*undef_derivative_xi[1]+
						undef_derivative_xi[4]*undef_derivative_xi[4]+
						undef_derivative_xi[7]*undef_derivative_xi[7];
					B=sqrt(B2);
					C2=undef_derivative_xi[0]*undef_derivative_xi[1]+
						undef_derivative_xi[3]*undef_derivative_xi[4]+
						undef_derivative_xi[6]*undef_derivative_xi[7];
					D=sin_fibre_angle*B/(A2*B2-C2*C2);
					dxi_dnu[0] = cos_fibre_angle/A-sin_fibre_angle*C2*D;
					dxi_dnu[1]=D*A2;
					F_X[0]=dxi_dnu[0]*undef_derivative_xi[0]+
						dxi_dnu[1]*undef_derivative_xi[1];
					F_X[2]=dxi_dnu[0]*undef_derivative_xi[3]+
						dxi_dnu[1]*undef_derivative_xi[4];
					F_X[4]=dxi_dnu[0]*undef_derivative_xi[6]+
						dxi_dnu[1]*undef_derivative_xi[7];
					D=cos_fibre_angle*B/(A2*B2-C2*C2);
					dxi_dnu[2]=
						-(sin_fibre_angle/A+cos_fibre_angle*C2*D);
					dxi_dnu[3]=D*A2;
					F_X[1]=dxi_dnu[2]*undef_derivative_xi[0]+
						dxi_dnu[3]*undef_derivative_xi[1];
					F_X[3]=dxi_dnu[2]*undef_derivative_xi[3]+
						dxi_dnu[3]*undef_derivative_xi[4];
					F_X[5]=dxi_dnu[2]*undef_derivative_xi[6]+
						dxi_dnu[3]*undef_derivative_xi[7];
					/* calculate F_x=dx_dnu=dx_dxi*dxi_dnu */
					F_x[0]=dxi_dnu[0]*def_derivative_xi[0]+
						dxi_dnu[1]*def_derivative_xi[1];
					F_x[1]=dxi_dnu[2]*def_derivative_xi[0]+
						dxi_dnu[3]*def_derivative_xi[1];
					F_x[2]=dxi_dnu[0]*def_derivative_xi[3]+
						dxi_dnu[1]*def_derivative_xi[4];
					F_x[3]=dxi_dnu[2]*def_derivative_xi[3]+
						dxi_dnu[3]*def_derivative_xi[4];
					F_x[4]=dxi_dnu[0]*def_derivative_xi[6]+
						dxi_dnu[1]*def_derivative_xi[7];
					F_x[5]=dxi_dnu[2]*def_derivative_xi[6]+
						dxi_dnu[3]*def_derivative_xi[7];
					/* calculate the strain tensor
						 E=0.5*(trans(F_x)*F_x-trans(F_X)*F_X) */
					E[0]=0.5*((F_x[0]*F_x[0]+F_x[2]*F_x[2]+
						F_x[4]*F_x[4])-
						(F_X[0]*F_X[0]+F_X[2]*F_X[2]+
							F_X[4]*F_X[4]));
					E[1]=0.5*((F_x[0]*F_x[1]+F_x[2]*F_x[3]+
						F_x[4]*F_x[5])-
						(F_X[0]*F_X[1]+F_X[2]*F_X[3]+
							F_X[4]*F_X[5]));
					E[2]=E[1];
					E[3]=0.5*((F_x[1]*F_x[1]+F_x[3]*F_x[3]+
						F_x[5]*F_x[5])-
						(F_X[1]*F_X[1]+F_X[3]*F_X[3]+
							F_X[5]*F_X[5]));
					field->values[0] = E[0];
					field->values[1] = E[1];
					field->values[2] = E[2];
					field->values[3] = E[3];
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_2d_strain_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of strains");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_2d_strain_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_2d_strain_evaluate_cache_in_element */

#define Computed_field_2d_strain_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_2d_strain_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_2d_strain_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_2d_strain_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_2d_strain_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_2d_strain_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_2d_strain(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_2d_strain);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    deformed coordinate field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    undeformed coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    fibre angle field : %s\n",field->source_fields[2]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_2d_strain.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_2d_strain */

static char *Computed_field_2d_strain_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_2d_strain_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_2d_strain_type_string, &error);
		append_string(&command_string, " deformed_coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " undeformed_coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " fibre_angle ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_2d_strain_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_2d_strain_get_command_string */

#define Computed_field_2d_strain_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_2d_strain(struct Computed_field *field,
	struct Computed_field *deformed_coordinate_field,
	struct Computed_field *undeformed_coordinate_field,
	struct Computed_field *fibre_angle_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_2D_STRAIN, combining a 
deformed_coordinate_field, undeformed_coordinate_field and
fibre_angle_field.  Sets the number of components to 4.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
The <coordinate_field>s must have no more than 3 components.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **temp_source_fields;

	ENTER(Computed_field_set_type_2d_strain);
	if (field&&deformed_coordinate_field&&
		(3>=deformed_coordinate_field->number_of_components)&&
		undeformed_coordinate_field&&
		(3>=undeformed_coordinate_field->number_of_components)
		&&fibre_angle_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=3;
		if (ALLOCATE(temp_source_fields,struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_2d_strain_type_string;
			field->number_of_components=4;
			/* source_fields:
				 0=deformed_coordinate, 1=undeformed_coordinate, 2=fibre_angle */
			temp_source_fields[0]=ACCESS(Computed_field)(deformed_coordinate_field);
			temp_source_fields[1]=ACCESS(Computed_field)(undeformed_coordinate_field);
			temp_source_fields[2]=ACCESS(Computed_field)(fibre_angle_field);
			field->source_fields=temp_source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(2d_strain);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_2d_strain.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_2d_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_2d_strain */

int Computed_field_get_type_2d_strain(struct Computed_field *field,
	struct Computed_field **deformed_coordinate_field,
	struct Computed_field **undeformed_coordinate_field,
	struct Computed_field **fibre_angle_field)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_2D_STRAIN, the undeformed and deformed
coordinate fields and the fibre angle field used by it are returned 
- otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_2d_strain);
	if (field && (field->type_string == computed_field_2d_strain_type_string) &&
		undeformed_coordinate_field && deformed_coordinate_field &&
		fibre_angle_field)
	{
		/* source_fields: 0=deformed_coordinate_field,
			1=undeformed_coordinate_field,
			2=fibre_angle_field */
		*deformed_coordinate_field=field->source_fields[0];
		*undeformed_coordinate_field=field->source_fields[1];
		*fibre_angle_field=field->source_fields[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_2d_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_2d_strain */

static int define_Computed_field_type_2d_strain(struct Parse_state *state,
	void *field_void,void *computed_field_deformation_package_void)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_2D_STRAIN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *deformed_coordinate_field, *fibre_angle_field, *field,
		*undeformed_coordinate_field;
	struct Computed_field_deformation_package *computed_field_deformation_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_deformed_coordinate_field_data,
		set_fibre_angle_field_data,set_undeformed_coordinate_field_data;

	ENTER(define_Computed_field_type_2d_strain);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_deformation_package=
		(struct Computed_field_deformation_package *)
		computed_field_deformation_package_void))
	{
		return_code=1;
		deformed_coordinate_field=(struct Computed_field *)NULL;
		undeformed_coordinate_field=(struct Computed_field *)NULL;
		fibre_angle_field=(struct Computed_field *)NULL;
		if (computed_field_2d_strain_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_2d_strain(field,
				&deformed_coordinate_field, &undeformed_coordinate_field,
				&fibre_angle_field);
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			if (deformed_coordinate_field)
			{
				ACCESS(Computed_field)(deformed_coordinate_field);
			}
			if (undeformed_coordinate_field)
			{
				ACCESS(Computed_field)(undeformed_coordinate_field);
			}
			if (fibre_angle_field)
			{
				ACCESS(Computed_field)(fibre_angle_field);
			}
			option_table = CREATE(Option_table)();
			/* deformed coordinate */
			set_deformed_coordinate_field_data.computed_field_manager=
				computed_field_deformation_package->computed_field_manager;
			set_deformed_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_deformed_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"deformed_coordinate",
				&deformed_coordinate_field,&set_deformed_coordinate_field_data,
				set_Computed_field_conditional);
			/* undeformed coordinate */
			set_undeformed_coordinate_field_data.computed_field_manager=
				computed_field_deformation_package->computed_field_manager;
			set_undeformed_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_undeformed_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"undeformed_coordinate",
				&undeformed_coordinate_field,&set_undeformed_coordinate_field_data,
				set_Computed_field_conditional);
			/* fibre_angle */
			set_fibre_angle_field_data.computed_field_manager=
				computed_field_deformation_package->computed_field_manager;
			set_fibre_angle_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_fibre_angle_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"fibre_angle",
				&fibre_angle_field,&set_fibre_angle_field_data,
				set_Computed_field_conditional);
			if (return_code = Option_table_multi_parse(option_table,state))
			{
				return_code = Computed_field_set_type_2d_strain(field,
					deformed_coordinate_field, undeformed_coordinate_field,
					fibre_angle_field);
			}
			DESTROY(Option_table)(&option_table);
			if (deformed_coordinate_field)
			{
				DEACCESS(Computed_field)(&deformed_coordinate_field);
			}
			if (undeformed_coordinate_field)
			{
				DEACCESS(Computed_field)(&undeformed_coordinate_field);
			}
			if (fibre_angle_field)
			{
				DEACCESS(Computed_field)(&fibre_angle_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_2d_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_2d_strain */

int Computed_field_register_types_deformation(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_deformation_package 
		computed_field_deformation_package;

	ENTER(Computed_field_register_types_deformation);
	if (computed_field_package)
	{
		computed_field_deformation_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_2d_strain_type_string,
			define_Computed_field_type_2d_strain,
			&computed_field_deformation_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_deformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_deformation */

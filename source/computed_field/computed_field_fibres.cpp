/*******************************************************************************
FILE : computed_field_fibres.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a number of basic continuum mechanics fibres operations on
computed fields.
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
extern "C" {
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_fibres.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}

struct Computed_field_fibres_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

namespace {

char computed_field_fibre_axes_type_string[] = "fibre_axes";

class Computed_field_fibre_axes : public Computed_field_core
{
public:
	Computed_field_fibre_axes(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_fibre_axes(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_fibre_axes_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_fibre_axes*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location);
};

int Computed_field_fibre_axes::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_fibre_axes::is_defined_at_location);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;
		/* Only works for element_xi locations */
		if ((element_xi_location  = 
				dynamic_cast<Field_element_xi_location*>(location))
			/* 2d_strain requires at least 2-D elements */
  			&& (2 <= get_FE_element_dimension(element_xi_location->get_element())))
		{
			/* check the source fields */
			return_code = Computed_field_core::is_defined_at_location(location);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_2d_strain::is_defined_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fibre_axes::is_defined_at_location */

int Computed_field_fibre_axes::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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
	FE_element* top_level_element;
	FE_value a_x, a_y, a_z, alpha, b_x, b_y, b_z, beta, c_x, c_y, c_z, cos_alpha,
		cos_beta, cos_gamma ,dx_dxi[9], f11, f12, f13, f21, f22, f23, f31, f32, f33,
		gamma, length, sin_alpha, sin_beta, sin_gamma,
		top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], x[3];
	int return_code,top_level_element_dimension;
	struct Computed_field *coordinate_field, *fibre_field;

	ENTER(Computed_field_fibre_axes::evaluate_cache_at_location);

	Field_element_xi_location* element_xi_location;
	/* Only works for element_xi locations */
	if (field && location && (element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location)))
	{
		if (!location->get_number_of_derivatives())
		{
			FE_element* element = element_xi_location->get_element();
			int element_dimension=get_FE_element_dimension(element);

			field->derivatives_valid = 0;

			top_level_element = element_xi_location->get_top_level_element();
			FE_element_get_top_level_element_and_xi(element_xi_location->get_element(),
				element_xi_location->get_xi(), element_dimension,
				&top_level_element, top_level_xi, &top_level_element_dimension);

			Field_element_xi_location location2(
				element,element_xi_location->get_xi(),
				element_xi_location->get_time(),top_level_element);
			Field_element_xi_location top_level_location(
				top_level_element,
				top_level_xi, element_xi_location->get_time(), top_level_element,
				get_FE_element_dimension(top_level_element));

			/* 1. Precalculate any source fields that this field depends on.
				 Always need derivatives of coordinate field and further, it must be
				 calculated on the top_level_element */
			fibre_field = field->source_fields[0];
			coordinate_field = field->source_fields[1];
			if (return_code =
				Computed_field_evaluate_cache_at_location(fibre_field,
					&location2) &&
				Computed_field_evaluate_cache_at_location(coordinate_field,
					&top_level_location) &&
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
					"Computed_field_fibre_axes::evaluate_cache_at_location.  "
					"Could not evaluate source fields");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_fibre_axes::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of fibre axes");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_fibre_axes::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fibre_axes::evaluate_cache_at_location */


int Computed_field_fibre_axes::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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

char *Computed_field_fibre_axes::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_fibre_axes::get_command_string);
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
			"Computed_field_fibre_axes::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_fibre_axes::get_command_string */

} //namespace

int Computed_field_set_type_fibre_axes(struct Computed_field *field,
	struct Computed_field *fibre_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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
			field->number_of_components=9;
			/* source_fields: 0=fibre, 1=coordinate */
			temp_source_fields[0]=ACCESS(Computed_field)(fibre_field);
			temp_source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=temp_source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->core = new Computed_field_fibre_axes(field);
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
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FIBRE_AXES, the fibre and coordinate
fields used by it are returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_fibre_axes);
	if (field && (dynamic_cast<Computed_field_fibre_axes*>(field->core)) &&
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

int define_Computed_field_type_fibre_axes(struct Parse_state *state,
	void *field_void,void *computed_field_fibres_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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
LAST MODIFIED : 24 August 2006

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



#include "api/cmiss_zinc_configure.h"
#if 1
#include "configure/cmgui_configure.h"
#endif

#include "image_processing/computed_field_image_filter.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_derivatives.h"

class Computed_field_derivatives_package : public Computed_field_type_package
{
};

const char computed_field_derivative_type_string[] = "derivative";

const char computed_field_curl_type_string[] = "curl";

const char computed_field_divergence_type_string[] = "divergence";

const char computed_field_gradient_type_string[] = "gradient";

int Computed_field_get_type_derivative(struct Computed_field *field,
	struct Computed_field **source_field, int *xi_index);

int Computed_field_get_type_curl(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field);

int Computed_field_get_type_divergence(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field);

int Computed_field_get_type_gradient(struct Computed_field *field,
	struct Computed_field **source_field,struct Computed_field **coordinate_field);

int define_Computed_field_type_derivative(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DERIVATIVE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code, xi_index;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_derivative);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		xi_index = 1;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_derivative_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_derivative(field_modify->get_field(),
				&source_field, &xi_index);
			xi_index++;
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The derivative field has two modes of operation.  For normal "
				"finite element fields it simply promotes the derivative "
				"values corresponding to <xi_index> calculated by the input "
				"<field> to be the field values.  These derivatives are with "
				"respect to xi. "
				"If the input <field> cannot cannot calculate element based "
				"derivatives then if the input field has a native resolution "
				"then this field uses the ITK DerivativeImageFilter to calculate "
				"a pixel based derivative at that same resolution.  "
				"The derivative filter will use the image pixel physical spacing "
				"if that is defined for ITK.  Note that as the derivative is a "
				"signed value you may want to offset and scale the resultant "
				"values if you intend to store them in an unsigned pixel format.");

			/* field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* xi_index */
			Option_table_add_entry(option_table,"xi_index",&xi_index,
				NULL,set_int_positive);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_derivative(field_modify->get_field_module(),
						source_field, xi_index - 1));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_derivative.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_derivative */


int define_Computed_field_type_curl(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURL (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*vector_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_curl);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field = (struct Computed_field *)NULL;
		vector_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_curl_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_curl(field_modify->get_field(),
				&vector_field,&coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_vector_field_data.conditional_function=
				Computed_field_has_3_components;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				(void *)&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_curl(field_modify->get_field_module(),
						vector_field, coordinate_field));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_curl.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_curl */


int define_Computed_field_type_divergence(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DIVERGENCE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*vector_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_divergence);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		vector_field=(struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_divergence_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_divergence(field_modify->get_field(),
				&vector_field,&coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_vector_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				(void *)&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_divergence(field_modify->get_field_module(),
						vector_field, coordinate_field));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_divergence.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divergence */



int define_Computed_field_type_gradient(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type 'gradient', if not already, and allows its contents
to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_source_field_data;

	ENTER(define_Computed_field_type_gradient);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		source_field=(struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_gradient_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_gradient(field_modify->get_field(),
				&source_field, &coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The gradient field calculates the partial derivatives of each of the "
				"source field components with respect to each of the coordinate field components.  "
				"The first values are each of the source field components with respect to the first "
				"coordinate component and then each of the source field components wrt to the second "
				"coordinate component and so on.  "
				"This field can now be used at both element_xi and nodal locations.  At element_xi "
				"locations the basis function supplied xi derivative is multiplied by the basis function "
				"coordinate field jacobian.  For nodal locations a finite difference approximation is "
				"calculated by perturbing each of the coordinate field values and reevaluating the source field.  "
				"See a/graph_axes for an example of using the gradient field to calculate the number of pixels "
				"per ndc coordinate.");

			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				(void *)&set_source_field_data,set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_gradient(field_modify->get_field_module(),
						source_field, coordinate_field));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_gradient.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_gradient */

int Computed_field_register_types_derivatives(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_derivatives_package
		*computed_field_derivatives_package =
		new Computed_field_derivatives_package;

	ENTER(Computed_field_register_type_derivative);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_derivative_type_string,
			define_Computed_field_type_derivative,
			computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_curl_type_string,
			define_Computed_field_type_curl,
			computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_divergence_type_string,
			define_Computed_field_type_divergence,
			computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_gradient_type_string,
			define_Computed_field_type_gradient,
			computed_field_derivatives_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_derivative */

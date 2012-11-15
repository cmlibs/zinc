
#include "zinc/fieldvectoroperators.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_vector_operators.hpp"

class Computed_field_vector_operators_package : public Computed_field_type_package
{
};

const char computed_field_cubic_texture_coordinates_type_string[] = "cubic_texture_coordinates";
const char computed_field_magnitude_type_string[] = "magnitude";
const char computed_field_dot_product_type_string[] = "dot_product";
const char computed_field_cross_product_type_string[] = "cross_product";
const char computed_field_normalise_type_string[] = "normalise";

int Computed_field_get_type_normalise(struct Computed_field *field,
	struct Computed_field **source_field);

int Computed_field_get_type_cross_product(struct Computed_field *field,
	int *dimension, struct Computed_field ***source_fields);

int Computed_field_get_type_dot_product(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two);

int Computed_field_get_type_magnitude(struct Computed_field *field,
	struct Computed_field **source_field);

int Computed_field_get_type_cubic_texture_coordinates(struct Computed_field *field,
	struct Computed_field **source_field);

struct Computed_field *Computed_field_create_cubic_texture_coordinates(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

int define_Computed_field_type_normalise(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NORMALISE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_normalise);
	USE_PARAMETER(computed_field_vector_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_normalise_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_normalise(field_modify->get_field(), &source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_normalise(field_modify->get_field_module(),
						source_field));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_normalise.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_normalise */


int define_Computed_field_type_cross_product(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CROSS_PRODUCT (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	int dimension, i, number_of_source_fields, return_code,
		temp_number_of_source_fields;
	struct Computed_field **source_fields, **temp_source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_cross_product);
	USE_PARAMETER(computed_field_vector_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_cross_product_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_cross_product(field_modify->get_field(), &dimension,
				&source_fields);
			number_of_source_fields = dimension - 1;
		}
		else
		{
			dimension = 3;
			number_of_source_fields = dimension - 1;
			if (ALLOCATE(source_fields, struct Computed_field *,
				number_of_source_fields))
			{
				for (i = 0; i < number_of_source_fields; i++)
				{
					source_fields[i] = (struct Computed_field *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_cross_product.  "
					"Could not allocate source fields array");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* try to handle help first */
			current_token = state->current_token;
			if (current_token != 0)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table, "dimension", &dimension,
						NULL, set_int_positive);
					set_field_data.conditional_function = Computed_field_has_n_components;
					set_field_data.conditional_function_user_data = (void *)&dimension;
					set_field_data.computed_field_manager =
						field_modify->get_field_manager();
					set_field_array_data.number_of_fields = number_of_source_fields;
					set_field_array_data.conditional_data = &set_field_data;
					Option_table_add_entry(option_table, "fields", source_fields,
						&set_field_array_data, set_Computed_field_array);
					return_code = Option_table_multi_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					/* ... only if the "dimension" token is next */
					if (fuzzy_string_compare(current_token, "dimension"))
					{
						option_table = CREATE(Option_table)();
						/* dimension */
						Option_table_add_entry(option_table, "dimension", &dimension,
							NULL, set_int_positive);
						return_code = Option_table_parse(option_table, state);
						DESTROY(Option_table)(&option_table);
						if (number_of_source_fields != dimension - 1)
						{
							temp_number_of_source_fields = dimension - 1;
							if (REALLOCATE(temp_source_fields, source_fields,
								struct Computed_field *, temp_number_of_source_fields))
							{
								source_fields = temp_source_fields;
								/* make all the new source fields NULL */
								for (i = number_of_source_fields;
									i < temp_number_of_source_fields; i++)
								{
									source_fields[i] = (struct Computed_field *)NULL;
								}
								number_of_source_fields = temp_number_of_source_fields;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_cross_product.  "
									"Could not reallocate source fields array");
								return_code = 0;
							}
						}
						if ((dimension < 2) || (dimension > 4))
						{
							display_message(ERROR_MESSAGE,
								"Only dimensions from 2 to 4 are supported");
							return_code = 0;
						}
					}
					if (return_code)
					{
						/* ACCESS the source fields for set_Computed_field_array */
						for (i = 0; i < number_of_source_fields; i++)
						{
							if (source_fields[i])
							{
								ACCESS(Computed_field)(source_fields[i]);
							}
						}
						option_table = CREATE(Option_table)();
						set_field_data.conditional_function =
							Computed_field_has_n_components;
						set_field_data.conditional_function_user_data = (void *)&dimension;
						set_field_data.computed_field_manager =
							field_modify->get_field_manager();
						set_field_array_data.number_of_fields = number_of_source_fields;
						set_field_array_data.conditional_data = &set_field_data;
						Option_table_add_entry(option_table, "fields", source_fields,
							&set_field_array_data, set_Computed_field_array);
						return_code = Option_table_multi_parse(option_table, state);
						if (return_code)
						{
							return_code = field_modify->update_field_and_deaccess(
								Cmiss_field_module_create_cross_product(field_modify->get_field_module(),
									dimension, source_fields));
						}
						for (i = 0; i < number_of_source_fields; i++)
						{
							if (source_fields[i])
							{
								DEACCESS(Computed_field)(&source_fields[i]);
							}
						}
						DESTROY(Option_table)(&option_table);
					}
					if (!return_code)
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_cross_product.  Failed");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing command options.");
			}
		}
		if (source_fields)
		{
			DEALLOCATE(source_fields);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cross_product */


int define_Computed_field_type_dot_product(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DOT_PRODUCT (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_dot_product);
	USE_PARAMETER(computed_field_vector_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_dot_product_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_dot_product(field_modify->get_field(),
					source_fields, source_fields + 1);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}
				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_dot_product(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_dot_product.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_dot_product.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_dot_product */


int define_Computed_field_type_magnitude(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MAGNITUDE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_magnitude);
	USE_PARAMETER(computed_field_vector_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_magnitude_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_magnitude(field_modify->get_field(), &source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_magnitude(field_modify->get_field_module(),
						source_field));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_magnitude.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_magnitude */


int define_Computed_field_type_cubic_texture_coordinates(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_cubic_texture_coordinates);
	USE_PARAMETER(computed_field_vector_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;

		if ((NULL != field_modify->get_field()) &&
			(computed_field_cubic_texture_coordinates_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_cubic_texture_coordinates(
				field_modify->get_field(), &source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_cubic_texture_coordinates(
						field_modify->get_field_module(), source_field));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_cubic_texture_coordinates.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cubic_texture_coordinates */

int Computed_field_register_types_vector_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_vector_operators_package
		*computed_field_vector_operators_package =
		new Computed_field_vector_operators_package;

	ENTER(Computed_field_register_types_vector_operators);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_magnitude_type_string,
			define_Computed_field_type_magnitude,
			computed_field_vector_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_normalise_type_string,
			define_Computed_field_type_normalise,
			computed_field_vector_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cross_product_type_string,
			define_Computed_field_type_cross_product,
			computed_field_vector_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_dot_product_type_string,
			define_Computed_field_type_dot_product,
			computed_field_vector_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cubic_texture_coordinates_type_string,
			define_Computed_field_type_cubic_texture_coordinates,
			computed_field_vector_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_vector_operators.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_vector_operators */

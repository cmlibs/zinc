/*******************************************************************************
FILE : computed_field_vector_operations.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a number of basic vector operations on computed fields.
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
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_vector_operations.h"
}

class Computed_field_vector_operations_package : public Computed_field_type_package
{
};

namespace {

char computed_field_normalise_type_string[] = "normalise";

class Computed_field_normalise : public Computed_field_core
{
public:
	Computed_field_normalise(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_normalise(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_normalise_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_normalise*>(other_field))
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
};

int Computed_field_normalise::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	double size;
	FE_value *derivative, *source_derivative;
	int i, number_of_xi, return_code;

	ENTER(Computed_field_normalise::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			size = 0.0;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				size += field->source_fields[0]->values[i] *
					field->source_fields[0]->values[i];
			}
			size = sqrt(size);
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] / size;
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				source_derivative = field->source_fields[0]->derivatives;
				for (i = 0 ; i < field->number_of_components * number_of_xi ; i++)
				{
					*derivative = *source_derivative / size;
					derivative++;
					source_derivative++;
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_normalise::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_normalise::evaluate_cache_at_location */


int Computed_field_normalise::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_normalise);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_normalise.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_normalise */

char *Computed_field_normalise::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_normalise::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_normalise_type_string, &error);
		append_string(&command_string, " field ", &error);
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
			"Computed_field_normalise::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_normalise::get_command_string */

} //namespace

int Computed_field_set_type_normalise(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NORMALISE with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_normalise);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_normalise(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_normalise */

int Computed_field_get_type_normalise(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NORMALISE, the 
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_normalise);
	if (field && (dynamic_cast<Computed_field_normalise*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_normalise */

int define_Computed_field_type_normalise(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NORMALISE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_normalise);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_vector_operations_package=
		(Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_normalise_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_normalise(field, &source_field);
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
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_normalise(field,
					source_field);
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

namespace {

char computed_field_cross_product_type_string[] = "cross_product";

class Computed_field_cross_product : public Computed_field_core
{
public:
	Computed_field_cross_product(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_cross_product(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_cross_product_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cross_product*>(other_field))
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
};

int Computed_field_cross_product::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *derivative, *source_derivative, *temp_vector;
	int i, j, number_of_derivatives, return_code;

	ENTER(Computed_field_cross_product::evaluate_cache_at_location);
	if (field && location &&
		(field->number_of_source_fields == field->number_of_components - 1))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			switch (field->number_of_components)
			{
				case 1:
				{
					field->values = 0;
				} break;
				case 2:
				{
					field->values[0] = -field->source_fields[0]->values[1];
					field->values[1] = field->source_fields[0]->values[0];
				} break;
				case 3:
				{
					cross_product_FE_value_vector3(field->source_fields[0]->values,
						field->source_fields[1]->values, field->values);
				} break;
				case 4:
				{
					cross_product_FE_value_vector4(field->source_fields[0]->values,
						field->source_fields[1]->values, 
						field->source_fields[2]->values, field->values);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_cross_product_evaluate_cache_at_node.  "
						"Unsupported number of components.");
					return_code = 0;
				} break;
			}
			number_of_derivatives = location->get_number_of_derivatives();
			if (0 < number_of_derivatives)
			{
				if (ALLOCATE(temp_vector, FE_value, field->number_of_components *
					field->number_of_components))
				{
					switch (field->number_of_components)
					{
						case 1:
						{
							derivative = field->derivatives;
							for (i = 0 ; i < number_of_derivatives ; i++)
							{
								*derivative = 0;
								derivative++;
							}
						} break;
						case 2:
						{
							derivative = field->derivatives;
							source_derivative = field->derivatives + number_of_derivatives;
							for (i = 0 ; i < number_of_derivatives ; i++)
							{
								*derivative = -*source_derivative;
								derivative++;
								source_derivative++;
							}
							source_derivative = field->derivatives + number_of_derivatives;
							for (i = 0 ; i < number_of_derivatives ; i++)
							{
								*derivative = *source_derivative;
								derivative++;
								source_derivative++;
							}
						} break;
						case 3:
						{
							for (j = 0 ; j < number_of_derivatives ; j++)
							{
								for (i = 0 ; i < 3 ; i++)
								{
									temp_vector[i] = field->source_fields[0]->
										derivatives[i * number_of_derivatives + j];
									temp_vector[i + 3] = field->source_fields[1]->
										derivatives[i * number_of_derivatives + j];
								}
								cross_product_FE_value_vector3(temp_vector,
									field->source_fields[1]->values, temp_vector + 6);
								for (i = 0 ; i < 3 ; i++)
								{
									field->derivatives[i * number_of_derivatives + j] = 
										temp_vector[i + 6];
								}
								cross_product_FE_value_vector3(
									field->source_fields[0]->values,
									temp_vector + 3, temp_vector + 6);
								for (i = 0 ; i < 3 ; i++)
								{
									field->derivatives[i * number_of_derivatives + j] += 
										temp_vector[i + 6];
								}
							}
						} break;
						case 4:
						{
							for (j = 0 ; j < number_of_derivatives ; j++)
							{
								for (i = 0 ; i < 4 ; i++)
								{
									temp_vector[i] = field->source_fields[0]->
										derivatives[i * number_of_derivatives + j];
									temp_vector[i + 4] = field->source_fields[1]->
										derivatives[i * number_of_derivatives + j];
									temp_vector[i + 8] = field->source_fields[2]->
										derivatives[i * number_of_derivatives + j];
								}
								cross_product_FE_value_vector4(temp_vector,
									field->source_fields[1]->values, 
									field->source_fields[2]->values, temp_vector + 12);
								for (i = 0 ; i < 4 ; i++)
								{
									field->derivatives[i * number_of_derivatives + j] = 
										temp_vector[i + 12];
								}
								cross_product_FE_value_vector4(
									field->source_fields[0]->values, temp_vector + 4,
									field->source_fields[2]->values, temp_vector + 12);
								for (i = 0 ; i < 4 ; i++)
								{
									field->derivatives[i * number_of_derivatives + j] += 
										temp_vector[i + 12];
								}
								cross_product_FE_value_vector4(
									field->source_fields[0]->values, 
									field->source_fields[1]->values, 
									temp_vector + 8, temp_vector + 12);
								for (i = 0 ; i < 4 ; i++)
								{
									field->derivatives[i * number_of_derivatives + j] += 
										temp_vector[i + 12];
								}
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_cross_product_evaluate_cache_at_node.  "
								"Unsupported number of components.");
							field->derivatives_valid = 0;
							return_code = 0;
						} break;
					}
					DEALLOCATE(temp_vector);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_cross_product_evaluate_cache_at_node.  "
						"Unable to allocate temporary vector.");
					field->derivatives_valid = 0;
					return_code = 0;
				}
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cross_product_evaluate_cache_in_element */


int Computed_field_cross_product::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_cross_product);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    dimension : %d\n",field->number_of_components);
		display_message(INFORMATION_MESSAGE,"    source fields :");
		for (i = 0 ; i < field->number_of_components - 1 ; i++)
		{
			display_message(INFORMATION_MESSAGE," %s",
				field->source_fields[i]->name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cross_product.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cross_product */

char *Computed_field_cross_product::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_cross_product::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cross_product_type_string, &error);
		sprintf(temp_string, " dimension %d", field->number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " fields", &error);
		for (i = 0 ; i < field->number_of_components - 1 ; i++)
		{
			if (GET_NAME(Computed_field)(field->source_fields[i], &field_name))
			{
				make_valid_token(&field_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, field_name, &error);
				DEALLOCATE(field_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cross_product::get_command_string */

} //namespace

int Computed_field_set_type_cross_product(struct Computed_field *field,
	int dimension, struct Computed_field **source_fields)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CROSS_PRODUCT with the supplied
<dimension> and the corresponding (dimension-1) <source_fields>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i, number_of_source_fields,return_code;
	struct Computed_field **temp_source_fields;

	ENTER(Computed_field_set_type_cross_product);
	if (field && (2 <= dimension) && (4 >= dimension) && source_fields)
	{
		return_code = 1;
		for (i = 0 ; return_code && (i < dimension - 1) ; i++)
		{
			if (!source_fields[i] || 
				(source_fields[i]->number_of_components != dimension))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_cross_product.  "
					"The number of components of the %s field does not match the dimension",
					source_fields[i]->name);
				return_code = 0;
			}
		}
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=dimension - 1;
		if (return_code &&
			ALLOCATE(temp_source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = dimension;
			for (i = 0 ; i < number_of_source_fields ; i++)
			{
				temp_source_fields[i]=ACCESS(Computed_field)(source_fields[i]);
			}
			field->source_fields=temp_source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_cross_product(field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_cross_product.  Unable to allocate memory");
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cross_product */

int Computed_field_get_type_cross_product(struct Computed_field *field,
	int *dimension, struct Computed_field ***source_fields)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CROSS_PRODUCT, the 
<dimension> and <source_fields> used by it are returned.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_get_type_cross_product);
	if (field&&(dynamic_cast<Computed_field_cross_product*>(field->core))
		&&source_fields)
	{
		*dimension = field->number_of_components;
		if (ALLOCATE(*source_fields,struct Computed_field *,
			field->number_of_source_fields))
		{
			for (i=0;i<field->number_of_source_fields;i++)
			{
				(*source_fields)[i]=field->source_fields[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_cross_product.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cross_product */

int define_Computed_field_type_cross_product(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CROSS_PRODUCT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int dimension, i, number_of_source_fields, return_code,
		temp_number_of_source_fields;
	struct Computed_field *field, **source_fields, **temp_source_fields;
	Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_cross_product);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_vector_operations_package=
		(Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (computed_field_cross_product_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_cross_product(field, &dimension,
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
			if (current_token = state->current_token)
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
						Cmiss_region_get_Computed_field_manager(field_modify->region);
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
							Cmiss_region_get_Computed_field_manager(field_modify->region);
						set_field_array_data.number_of_fields = number_of_source_fields;
						set_field_array_data.conditional_data = &set_field_data;
						Option_table_add_entry(option_table, "fields", source_fields,
							&set_field_array_data, set_Computed_field_array);
						if (return_code = Option_table_multi_parse(option_table, state))
						{
							return_code = Computed_field_set_type_cross_product(field,
								dimension, source_fields);
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

namespace {

char computed_field_dot_product_type_string[] = "dot_product";

class Computed_field_dot_product : public Computed_field_core
{
public:
	Computed_field_dot_product(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_dot_product(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_dot_product_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_dot_product*>(other_field))
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
};

int Computed_field_dot_product::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int i, j, number_of_derivatives, return_code;

	ENTER(Computed_field_dot_product::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			number_of_derivatives = location->get_number_of_derivatives();
			field->values[0] = 0.0;
			if (0 < number_of_derivatives)
			{
				for (j=0;j<number_of_derivatives;j++)
				{
					field->derivatives[j]=0.0;
				}
			}
			temp=field->source_fields[0]->values;
			temp2=field->source_fields[1]->values;
			for (i=0;i < field->source_fields[0]->number_of_components;i++)
			{
				field->values[0] += (*temp) * (*temp2);
				temp++;
				temp2++;
			}
			if (0 < number_of_derivatives)
			{
				temp=field->source_fields[0]->values;
				temp2=field->source_fields[1]->derivatives;
				for (i=0;i < field->source_fields[0]->number_of_components;i++)
				{
					for (j=0;j<number_of_derivatives;j++)
					{
						field->derivatives[j] += (*temp)*(*temp2);
						temp2++;
					}
					temp++;
				}
				temp=field->source_fields[1]->values;
				temp2=field->source_fields[0]->derivatives;
				for (i=0;i < field->source_fields[0]->number_of_components;i++)
				{
					for (j=0;j<number_of_derivatives;j++)
					{
						field->derivatives[j] += (*temp)*(*temp2);
						temp2++;
					}
					temp++;
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dot_product::evaluate_cache_at_location */


int Computed_field_dot_product::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_dot_product);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field 1 : %s\n    field 2 : %s\n",
			field->source_fields[0]->name, field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_dot_product.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_dot_product */

char *Computed_field_dot_product::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_dot_product::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_dot_product_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_dot_product::get_command_string */

} //namespace

int Computed_field_set_type_dot_product(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DOT_PRODUCT with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components to one.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_dot_product);
	if (field&&source_field_one&&source_field_two&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = 1;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_dot_product(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_dot_product */

int Computed_field_get_type_dot_product(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DOT_PRODUCT, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_dot_product);
	if (field && (dynamic_cast<Computed_field_dot_product*>(field->core)) &&
		source_field_one && source_field_two)
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_dot_product */

int define_Computed_field_type_dot_product(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DOT_PRODUCT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_dot_product);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_vector_operations_package=
		(Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_dot_product_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_dot_product(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_dot_product(field,
						source_fields[0], source_fields[1]);
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

namespace {

char computed_field_magnitude_type_string[] = "magnitude";

class Computed_field_magnitude : public Computed_field_core
{
public:
	Computed_field_magnitude(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_magnitude(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_magnitude_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_magnitude*>(other_field))
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

	int set_values_at_location(Field_location* location, FE_value *values);
};

int Computed_field_magnitude::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *source_derivatives, *source_values, sum;
	int number_of_derivatives, i, j, return_code, source_number_of_components;

	ENTER(Computed_field_magnitude::evaluate_cache_at_location);
	if (field && location && (0 < field->number_of_source_fields))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			source_number_of_components =
				field->source_fields[0]->number_of_components;
			source_values = field->source_fields[0]->values;
			sum = 0.0;
			for (i=0;i<source_number_of_components;i++)
			{
				sum += source_values[i]*source_values[i];
			}
			field->values[0] = sqrt(sum);

			number_of_derivatives = location->get_number_of_derivatives();
			if (0 < number_of_derivatives)
			{
				source_derivatives=field->source_fields[0]->derivatives;
				for (j=0;j<number_of_derivatives;j++)
				{
					sum = 0.0;
					for (i=0;i<source_number_of_components;i++)
					{
						sum += source_values[i]*source_derivatives[i*number_of_derivatives+j];
					}
					field->derivatives[j] = sum / field->values[0];
				}
				field->derivatives_valid = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude::evaluate_cache_at_location */

int Computed_field_magnitude::set_values_at_location(
   Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	FE_value magnitude,*source_values;
	int k,return_code,source_number_of_components;

	ENTER(Computed_field_magnitude::set_values_at_location);
	if (field && location && values)
	{
		return_code=1;
		if (ALLOCATE(source_values, FE_value,
			field->source_fields[0]->number_of_components))
		{
			/* need current field values to "magnify" */
			if (Computed_field_evaluate_cache_at_location(field->source_fields[0],
					location))
			{
				source_number_of_components =
					field->source_fields[0]->number_of_components;
				/* if the source field is not a zero vector, set its magnitude to
					the given value */
				magnitude = 0.0;
				for (k=0;k<source_number_of_components;k++)
				{
					magnitude += field->source_fields[0]->source_values[k] *
						field->source_fields[0]->source_values[k];
				}
				if (0.0 < magnitude)
				{
					magnitude = values[0] / sqrt(magnitude);
					for (k=0;k<source_number_of_components;k++)
					{
						source_values[k] = field->source_fields[0]->source_values[k]
							* magnitude;
					}
				}
				else
				{
					/* not an error; just a warning */
					display_message(WARNING_MESSAGE,
						"Magnitude field %s cannot be inverted for zero vectors",
						field->name);
				}
				return_code=Computed_field_set_values_at_location(
					field->source_fields[0],location,source_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_magnitude::set_values_at_location.  "
					"Could not evaluate source field for %s.",field->name);
				return_code=0;
			}
			DEALLOCATE(source_values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude::set_values_at_location.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude::set_values_at_location */


int Computed_field_magnitude::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_magnitude);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_magnitude.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_magnitude */

char *Computed_field_magnitude::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_magnitude::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_magnitude_type_string, &error);
		append_string(&command_string, " field ", &error);
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
			"Computed_field_magnitude::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_magnitude::get_command_string */

} //namespace

int Computed_field_set_type_magnitude(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MAGNITUDE with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_magnitude);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = 1;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_magnitude(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_magnitude */

int Computed_field_get_type_magnitude(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MAGNITUDE, the 
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_magnitude);
	if (field && (dynamic_cast<Computed_field_magnitude*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_magnitude */

int define_Computed_field_type_magnitude(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MAGNITUDE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_magnitude);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_vector_operations_package=
		(Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_magnitude_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_magnitude(field, &source_field);
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
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_magnitude(field, source_field);
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

namespace {

char computed_field_cubic_texture_coordinates_type_string[] = "cubic_texture_coordinates";

class Computed_field_cubic_texture_coordinates : public Computed_field_core
{
public:
	Computed_field_cubic_texture_coordinates(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_cubic_texture_coordinates(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_cubic_texture_coordinates_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cubic_texture_coordinates*>(other_field))
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
};

int Computed_field_cubic_texture_coordinates::evaluate_cache_at_location(
   Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp;
	int i, j, number_of_components, return_code;

	ENTER(Computed_field_cubic_texture_coordinates::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			number_of_components = field->number_of_components;
			temp=field->source_fields[0]->values;
			field->values[number_of_components - 1] = fabs(*temp);
			temp++;
			j = 0;
			for (i=1;i<number_of_components;i++)
			{
				if (fabs(*temp) > field->values[number_of_components - 1])
				{
					field->values[number_of_components - 1] = fabs(*temp);
					j = i;
				}
				temp++;
			}
			temp=field->source_fields[0]->values;
			for (i=0;i < number_of_components - 1;i++)
			{
				if ( i == j )
				{
					/* Skip over the maximum coordinate */
					temp++;
				}
				field->values[i] = *temp / field->values[number_of_components - 1];
				temp++;
			}
			field->derivatives_valid = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cubic_texture_coordinates::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cubic_texture_coordinates::evaluate_cache_at_location */


int Computed_field_cubic_texture_coordinates::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cubic_texture_coordinates);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cubic_texture_coordinates.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cubic_texture_coordinates */

char *Computed_field_cubic_texture_coordinates::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_cubic_texture_coordinates::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cubic_texture_coordinates_type_string, &error);
		append_string(&command_string, " field ", &error);
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
			"Computed_field_cubic_texture_coordinates::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cubic_texture_coordinates::get_command_string */

} // namespace

int Computed_field_set_type_cubic_texture_coordinates(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_cubic_texture_coordinates);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_magnitude(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cubic_texture_coordinates */

int Computed_field_get_type_cubic_texture_coordinates(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type CUBIC_TEXTURE_COORDINATES, the source field used
by it is returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_cubic_texture_coordinates);
	if (field && (dynamic_cast<Computed_field_cubic_texture_coordinates*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cubic_texture_coordinates */

int define_Computed_field_type_cubic_texture_coordinates(struct Parse_state *state,
	void *field_modify_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_cubic_texture_coordinates);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_vector_operations_package=
		(Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (dynamic_cast<Computed_field_cubic_texture_coordinates*>(field->core))
		{
			return_code = Computed_field_get_type_cubic_texture_coordinates(field, &source_field);
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
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_cubic_texture_coordinates(field,
					source_field);
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

int Computed_field_register_types_vector_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_vector_operations_package
		*computed_field_vector_operations_package =
		new Computed_field_vector_operations_package;

	ENTER(Computed_field_register_types_vector_operations);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_magnitude_type_string,
			define_Computed_field_type_magnitude,
			computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_normalise_type_string,
			define_Computed_field_type_normalise,
			computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cross_product_type_string,
			define_Computed_field_type_cross_product,
			computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_dot_product_type_string,
			define_Computed_field_type_dot_product,
			computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cubic_texture_coordinates_type_string,
			define_Computed_field_type_cubic_texture_coordinates,
			computed_field_vector_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_vector_operations.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_vector_operations */

/*******************************************************************************
FILE : computed_field_control_curve.c

LAST MODIFIED : 17 December 2001

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_control_curve.h"

struct Computed_field_control_curve_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Control_curve) *control_curve_manager;
};

struct Computed_field_curve_lookup_type_specific_data
{
	struct Control_curve *curve;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Control_curve) *control_curve_manager;
	void *control_curve_manager_callback_id;
};

static char computed_field_curve_lookup_type_string[] = "curve_lookup";

static void Computed_field_curve_lookup_Control_curve_change(
	struct MANAGER_MESSAGE(Control_curve) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 24 May 2001

DESCRIPTION :
Something has changed globally in the Control_curve manager. Passes on messages
about changes as stemming from computed_field_manager for fields of type
COMPUTED_FIELD_CURVE_LOOKUP.
???RC Review Manager Messages Here
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_curve_lookup_Control_curve_change);
	if (message && (field = (struct Computed_field *)field_void) &&
		(COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_curve_lookup_type_string) &&
		(data = (struct Computed_field_curve_lookup_type_specific_data *)
			field->type_specific_data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Control_curve):
			case MANAGER_CHANGE_OBJECT(Control_curve):
			{
				/*???debug*/
				if (IS_OBJECT_IN_LIST(Control_curve)(data->curve,
					message->changed_object_list))
				{
					Computed_field_changed(field, data->computed_field_manager);
				}
			} break;
			case MANAGER_CHANGE_ADD(Control_curve):
			case MANAGER_CHANGE_REMOVE(Control_curve):
			case MANAGER_CHANGE_IDENTIFIER(Control_curve):
			{
				/* do nothing */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_curve_lookup_Control_curve_change.  "
					"Unknown manager message");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup_Control_curve_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_curve_lookup_Control_curve_change */

int Computed_field_is_type_curve_lookup(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_curve_lookup);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_curve_lookup_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_curve_lookup.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_curve_lookup */

static int Computed_field_curve_lookup_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_curve_lookup_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data))
	{
		if (data->control_curve_manager_callback_id)
		{
			MANAGER_DEREGISTER(Control_curve)(
				data->control_curve_manager_callback_id,
				data->control_curve_manager);
			data->control_curve_manager_callback_id = (void *)NULL;
		}
		if (data->curve)
		{
			DEACCESS(Control_curve)(&(data->curve));
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curve_lookup_clear_type_specific */

static void *Computed_field_curve_lookup_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 May 2001

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_curve_lookup_type_specific_data *destination, *source;

	ENTER(Computed_field_curve_lookup_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_curve_lookup_type_specific_data, 1))
		{
			destination->curve = ACCESS(Control_curve)(source->curve);
			destination->control_curve_manager_callback_id = (void *)NULL;
			destination->computed_field_manager = source->computed_field_manager;
			destination->control_curve_manager = source->control_curve_manager;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_curve_lookup_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_curve_lookup_copy_type_specific */

#define Computed_field_curve_lookup_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_curve_lookup_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_curve_lookup_type_specific_data *data, *other_data;

	ENTER(Computed_field_curve_lookup_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_curve_lookup_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->curve == other_data->curve)
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
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curve_lookup_type_specific_contents_match */

#define Computed_field_curve_lookup_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_curve_lookup_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_curve_lookup_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_curve_lookup_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
No special criteria on the destroy.
==============================================================================*/

static int Computed_field_curve_lookup_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_curve_lookup_evaluate_cache_at_node);
	if (field && node && (data = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			return_code = Control_curve_get_values_at_parameter(data->curve,
				field->source_fields[0]->values[0], field->values,
				/*derivatives*/(FE_value *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curve_lookup_evaluate_cache_at_node */

static int Computed_field_curve_lookup_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value dx_dt, *jacobian, *temp;
	int i, j, element_dimension, return_code;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_curve_lookup_evaluate_cache_in_element);
	if (field && element && xi && (data = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			element_dimension = get_FE_element_dimension(element);
			if (calculate_derivatives)
			{
				jacobian = field->derivatives;
			}
			else
			{
				jacobian = (FE_value *)NULL;
			}
			/* only slightly dodgy - stores derivatives of curve in start
				 of derivatives space - must be at least big enough */
			if (return_code = Control_curve_get_values_at_parameter(data->curve,
				field->source_fields[0]->values[0], field->values, jacobian))
			{
				if (jacobian)
				{
					/* use product rule to get derivatives */
					temp = field->source_fields[0]->derivatives;
					/* count down in following loop because of slightly dodgy bit */
					for (j = field->number_of_components - 1; 0 <= j; j--)
					{
						dx_dt = jacobian[j];
						for (i = 0; i < element_dimension; i++)
						{
							jacobian[j*element_dimension + i] = dx_dt*temp[i];
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curve_lookup_evaluate_cache_in_element */

#define Computed_field_curve_lookup_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_curve_lookup_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_curve_lookup_set_values_at_node \
	(Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Unavailable for this field type.
==============================================================================*/

#define Computed_field_curve_lookup_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Unavailable for this field type.
==============================================================================*/

#define Computed_field_curve_lookup_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_curve_lookup_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_curve_lookup(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
==============================================================================*/
{
	char *curve_name;
	int return_code;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(List_Computed_field_curve_lookup);
	if (field && (data = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data))
	{
		if (return_code = GET_NAME(Control_curve)(data->curve, &curve_name))
		{
			display_message(INFORMATION_MESSAGE, "    curve : %s\n", curve_name);
			display_message(INFORMATION_MESSAGE, "    source field : %s\n",
				field->source_fields[0]->name);
			DEALLOCATE(curve_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_curve_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_curve_lookup */

static char *Computed_field_curve_lookup_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *curve_name, *field_name;
	int error;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_curve_lookup_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_curve_lookup_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_curve_lookup_type_string, &error);
		append_string(&command_string, " curve ", &error);
		if (GET_NAME(Control_curve)(data->curve, &curve_name))
		{
			make_valid_token(&curve_name);
			append_string(&command_string, curve_name, &error);
			DEALLOCATE(curve_name);
		}
		append_string(&command_string, " source ", &error);
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
			"Computed_field_curve_lookup_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_curve_lookup_get_command_string */

int Computed_field_set_type_curve_lookup(struct Computed_field *field,
	struct Computed_field *source_field, struct Control_curve *curve,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Control_curve) *control_curve_manager)
/*******************************************************************************
LAST MODIFIED : 24 May 2001

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CURVE_LOOKUP, returning the value of
<curve> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <curve>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
???RC In future may not need to pass computed_field_manager it all fields
maintain pointer to it. Only have it to invoke computed field manager messages
in response to changes in the curve from the control curve manager.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_set_type_curve_lookup);
	if (field && source_field &&
		Computed_field_is_scalar(source_field, (void *)NULL) &&
		curve && computed_field_manager && control_curve_manager)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_curve_lookup_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type = COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_curve_lookup_type_string;
			field->number_of_components =
				Control_curve_get_number_of_components(curve);
			source_fields[0] = ACCESS(Computed_field)(source_field);
			field->source_fields = source_fields;
			field->number_of_source_fields = number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->curve = ACCESS(Control_curve)(curve);
			data->computed_field_manager = computed_field_manager;
			data->control_curve_manager = control_curve_manager;
			data->control_curve_manager_callback_id = MANAGER_REGISTER(Control_curve)(
				Computed_field_curve_lookup_Control_curve_change, (void *)field,
				control_curve_manager);

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_curve_lookup_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_curve_lookup_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_curve_lookup_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_curve_lookup_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_curve_lookup_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_curve_lookup_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_curve_lookup_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_curve_lookup_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_curve_lookup_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_curve_lookup_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_curve_lookup_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_curve_lookup_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_curve_lookup_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_curve_lookup_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_curve_lookup_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_curve_lookup_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_curve_lookup;
			field->computed_field_get_command_string_function = 
				Computed_field_curve_lookup_get_command_string;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_curve_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_curve_lookup */

int Computed_field_get_type_curve_lookup(struct Computed_field *field,
	struct Computed_field **source_field, struct Control_curve **curve)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURVE_LOOKUP, the source_field and curve
used by it are returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;
	struct Computed_field_curve_lookup_type_specific_data *data;

	ENTER(Computed_field_get_type_curve_lookup);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_curve_lookup_type_string) &&
		(data = (struct Computed_field_curve_lookup_type_specific_data *)
			field->type_specific_data) && source_field && curve)
	{
		*source_field = field->source_fields[0];
		*curve = data->curve;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_curve_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_curve_lookup */

static int define_Computed_field_type_curve_lookup(struct Parse_state *state,
	void *field_void, void *computed_field_control_curve_package_void)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURVE_LOOKUP (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_control_curve_package
		*computed_field_control_curve_package;
	struct Control_curve *curve;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_curve_lookup);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_control_curve_package =
			(struct Computed_field_control_curve_package *)
			computed_field_control_curve_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		curve = (struct Control_curve *)NULL;
		if (computed_field_curve_lookup_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_curve_lookup(field, &source_field, &curve);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (curve)
			{
				ACCESS(Control_curve)(curve);
			}

			option_table = CREATE(Option_table)();
			/* curve */
			Option_table_add_entry(option_table, "curve", &curve,
				computed_field_control_curve_package->control_curve_manager,
				set_Control_curve);
			/* source */
			set_source_field_data.computed_field_manager =
				computed_field_control_curve_package->computed_field_manager;
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "source", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_curve_lookup(field, source_field,
					curve, computed_field_control_curve_package->computed_field_manager,
					computed_field_control_curve_package->control_curve_manager);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_curve_lookup.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (curve)
			{
				DEACCESS(Control_curve)(&curve);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_curve_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_curve_lookup */

int Computed_field_register_types_control_curve(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Control_curve) *control_curve_manager)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_control_curve_package 
		computed_field_control_curve_package;

	ENTER(Computed_field_register_types_control_curve);
	if (computed_field_package && control_curve_manager)
	{
		computed_field_control_curve_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_control_curve_package.control_curve_manager =
			control_curve_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_curve_lookup_type_string, 
			define_Computed_field_type_curve_lookup,
			&computed_field_control_curve_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_control_curve.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_control_curve */

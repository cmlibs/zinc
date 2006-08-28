/*******************************************************************************
FILE : computed_field_curve.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
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
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_curve.h"
}

struct Computed_field_curve_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Curve) *curve_manager;
};

namespace {

void Computed_field_curve_lookup_Curve_change(
	struct MANAGER_MESSAGE(Curve) *message, void *field_void);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Something has changed globally in the Curve manager. Passes on messages
about changes as stemming from computed_field_manager for fields of type
COMPUTED_FIELD_CURVE_LOOKUP.
???RC Review Manager Messages Here
==============================================================================*/

char computed_field_curve_lookup_type_string[] = "curve_lookup";

class Computed_field_curve_lookup : public Computed_field_core
{
public:
	Curve *curve;
	MANAGER(Computed_field) *computed_field_manager;
	MANAGER(Curve) *curve_manager;
	void *curve_manager_callback_id;

	Computed_field_curve_lookup(Computed_field *field,
		Curve *curve, MANAGER(Computed_field) *computed_field_manager,
		MANAGER(Curve) *curve_manager) : Computed_field_core(field),
		curve(ACCESS(Curve)(curve)), computed_field_manager(computed_field_manager),
		curve_manager(curve_manager)
	{
		curve_manager_callback_id = MANAGER_REGISTER(Curve)(
			Computed_field_curve_lookup_Curve_change, (void *)field,
			curve_manager);
	};

	~Computed_field_curve_lookup();

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_curve_lookup_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

Computed_field_curve_lookup::~Computed_field_curve_lookup()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_curve_lookup::~Computed_field_curve_lookup);
	if (field)
	{
		if (curve_manager_callback_id)
		{
			MANAGER_DEREGISTER(Curve)(
				curve_manager_callback_id,
				curve_manager);
			curve_manager_callback_id = (void *)NULL;
		}
		if (curve)
		{
			DEACCESS(Curve)(&(curve));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup::~Computed_field_curve_lookup.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_curve_lookup::~Computed_field_curve_lookup */

Computed_field_core* Computed_field_curve_lookup::copy(Computed_field* new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_curve_lookup* core;

	ENTER(Computed_field_curve_lookup::copy);
	if (new_parent)
	{
		core = new Computed_field_curve_lookup(new_parent, curve,
			computed_field_manager, curve_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup::copy.  Invalid argument(s)");
		core = (Computed_field_curve_lookup*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_curve_lookup::copy */

int Computed_field_curve_lookup::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_curve_lookup* other;
	int return_code;

	ENTER(Computed_field_curve_lookup::compare);
	if (field && (other = dynamic_cast<Computed_field_curve_lookup*>(other_core)))
	{
		if (curve == other->curve)
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
} /* Computed_field_curve_lookup::compare */

int Computed_field_curve_lookup::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value dx_dt, *jacobian, *temp;
	int i, j, number_of_derivatives, return_code;

	ENTER(Computed_field_curve_lookup::evaluate_cache_at_location);
	if (field && location)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			number_of_derivatives = location->get_number_of_derivatives();
			if (number_of_derivatives)
			{
				jacobian = field->derivatives;
			}
			else
			{
				jacobian = (FE_value *)NULL;
			}
			/* only slightly dodgy - stores derivatives of curve in start
				 of derivatives space - must be at least big enough */
			if (return_code = Curve_get_values_at_parameter(curve,
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
						for (i = 0; i < number_of_derivatives ; i++)
						{
							jacobian[j*number_of_derivatives + i] = dx_dt*temp[i];
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curve_lookup::evaluate_cache_at_location */


int Computed_field_curve_lookup::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *curve_name;
	int return_code;

	ENTER(List_Computed_field_curve_lookup);
	if (field)
	{
		if (return_code = GET_NAME(Curve)(curve, &curve_name))
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

char *Computed_field_curve_lookup::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *curve_name, *field_name;
	int error;

	ENTER(Computed_field_curve_lookup::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_curve_lookup_type_string, &error);
		append_string(&command_string, " curve ", &error);
		if (GET_NAME(Curve)(curve, &curve_name))
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
			"Computed_field_curve_lookup::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_curve_lookup::get_command_string */

void Computed_field_curve_lookup_Curve_change(
	struct MANAGER_MESSAGE(Curve) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Something has changed globally in the Curve manager. Passes on messages
about changes as stemming from computed_field_manager for fields of type
COMPUTED_FIELD_CURVE_LOOKUP.
???RC Review Manager Messages Here
==============================================================================*/
{
	Computed_field_curve_lookup* core;
	Computed_field* field;

	ENTER(Computed_field_curve_lookup_Curve_change);
	if (message && (field = (Computed_field *)field_void) &&
		(core = dynamic_cast<Computed_field_curve_lookup*>(field->core)))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Curve):
			case MANAGER_CHANGE_OBJECT(Curve):
			{
				if (IS_OBJECT_IN_LIST(Curve)(core->curve,
					message->changed_object_list))
				{
					Computed_field_changed(field, core->computed_field_manager);
				}
			} break;
			case MANAGER_CHANGE_ADD(Curve):
			case MANAGER_CHANGE_REMOVE(Curve):
			case MANAGER_CHANGE_IDENTIFIER(Curve):
			{
				/* do nothing */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_curve_lookup_Curve_change.  "
					"Unknown manager message");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curve_lookup_Curve_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_curve_lookup_Curve_change */
} //namespace

int Computed_field_set_type_curve_lookup(struct Computed_field *field,
	struct Computed_field *source_field, struct Curve *curve,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Curve) *curve_manager)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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

	ENTER(Computed_field_set_type_curve_lookup);
	if (field && source_field &&
		Computed_field_is_scalar(source_field, (void *)NULL) &&
		curve && computed_field_manager && curve_manager)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components =
				Curve_get_number_of_components(curve);
			source_fields[0] = ACCESS(Computed_field)(source_field);
			field->source_fields = source_fields;
			field->number_of_source_fields = number_of_source_fields;			
			field->core = new Computed_field_curve_lookup(field,
				curve, computed_field_manager, curve_manager);
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
	struct Computed_field **source_field, struct Curve **curve)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURVE_LOOKUP, the source_field and curve
used by it are returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_curve_lookup* core;
	int return_code;

	ENTER(Computed_field_get_type_curve_lookup);
	if (field && (core = dynamic_cast<Computed_field_curve_lookup*>(field->core))
		&& source_field && curve)
	{
		*source_field = field->source_fields[0];
		*curve = core->curve;
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

int define_Computed_field_type_curve_lookup(struct Parse_state *state,
	void *field_void, void *computed_field_curve_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURVE_LOOKUP (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_curve_package
		*computed_field_curve_package;
	struct Curve *curve;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_curve_lookup);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_curve_package =
			(struct Computed_field_curve_package *)
			computed_field_curve_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		curve = (struct Curve *)NULL;
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
				ACCESS(Curve)(curve);
			}

			option_table = CREATE(Option_table)();
			/* curve */
			Option_table_add_entry(option_table, "curve", &curve,
				computed_field_curve_package->curve_manager,
				set_Curve);
			/* source */
			set_source_field_data.computed_field_manager =
				computed_field_curve_package->computed_field_manager;
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "source", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_curve_lookup(field, source_field,
					curve, computed_field_curve_package->computed_field_manager,
					computed_field_curve_package->curve_manager);
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
				DEACCESS(Curve)(&curve);
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

int Computed_field_register_types_curve(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Curve) *curve_manager)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_curve_package 
		computed_field_curve_package;

	ENTER(Computed_field_register_types_curve);
	if (computed_field_package && curve_manager)
	{
		computed_field_curve_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_curve_package.curve_manager =
			curve_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_curve_lookup_type_string, 
			define_Computed_field_type_curve_lookup,
			&computed_field_curve_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_curve.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_curve */


#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_curve.h"
#include "curve/curve_app.h"

class Computed_field_curve_package : public Computed_field_type_package
{
public:
	struct MANAGER(Curve) *curve_manager;
};

const char computed_field_curve_lookup_type_string[] = "curve_lookup";

int Computed_field_get_type_curve_lookup(struct Computed_field *field,
	struct Computed_field **source_field, struct Curve **curve);

int define_Computed_field_type_curve_lookup(struct Parse_state *state,
	void *field_modify_void, void *computed_field_curve_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURVE_LOOKUP (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_curve_package *computed_field_curve_package;
	Computed_field_modify_data *field_modify;
	struct Curve *curve;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_curve_lookup);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_curve_package =
			(Computed_field_curve_package *)
			computed_field_curve_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		curve = (struct Curve *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_curve_lookup_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_curve_lookup(field_modify->get_field(), &source_field, &curve);
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
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "source", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_curve_lookup(field_modify->get_field_module(),
						source_field, curve, computed_field_curve_package->curve_manager));
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
	Computed_field_curve_package
		*computed_field_curve_package =
		new Computed_field_curve_package;

	ENTER(Computed_field_register_types_curve);
	if (computed_field_package && curve_manager)
	{
		computed_field_curve_package->curve_manager = curve_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_curve_lookup_type_string,
			define_Computed_field_type_curve_lookup,
			computed_field_curve_package);
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

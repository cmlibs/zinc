
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_fibres.h"

class Computed_field_fibres_package : public Computed_field_type_package
{
};

const char computed_field_fibre_axes_type_string[] = "fibre_axes";

int Computed_field_get_type_fibre_axes(struct Computed_field *field,
	struct Computed_field **fibre_field,struct Computed_field **coordinate_field);

int define_Computed_field_type_fibre_axes(struct Parse_state *state,
	void *field_modify_void,void *computed_field_fibres_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FIBRE_AXES (if it is not already) and
allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field, *fibre_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fibre_field_data;

	ENTER(define_Computed_field_type_fibre_axes);
	USE_PARAMETER(computed_field_fibres_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		fibre_field=(struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_fibre_axes_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_fibre_axes(field_modify->get_field(),
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
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&coordinate_field,&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* fibre */
			set_fibre_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_fibre_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_fibre_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"fibre",
				&fibre_field,&set_fibre_field_data,set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_fibre_axes(
						field_modify->get_field_module(), fibre_field, coordinate_field));
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
	Computed_field_fibres_package
		*computed_field_fibres_package =
		new Computed_field_fibres_package;

	ENTER(Computed_field_register_types_fibres);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_fibre_axes_type_string,
			define_Computed_field_type_fibre_axes,
			computed_field_fibres_package);
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

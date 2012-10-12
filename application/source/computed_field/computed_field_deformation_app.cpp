
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_deformation.h"

class Computed_field_deformation_package : public Computed_field_type_package
{
};

char computed_field_2d_strain_type_string[] = "2d_strain";

int Computed_field_get_type_2d_strain(struct Computed_field *field,
	struct Computed_field **deformed_coordinate_field,
	struct Computed_field **undeformed_coordinate_field,
	struct Computed_field **fibre_angle_field);

struct Computed_field *Computed_field_create_2d_strain(
	struct Cmiss_field_module *field_module,
	struct Computed_field *deformed_coordinate_field,
	struct Computed_field *undeformed_coordinate_field,
	struct Computed_field *fibre_angle_field);

int define_Computed_field_type_2d_strain(struct Parse_state *state,
	void *field_modify_void,void *computed_field_deformation_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_2D_STRAIN (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *deformed_coordinate_field, *fibre_angle_field,
		*undeformed_coordinate_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_deformed_coordinate_field_data,
		set_fibre_angle_field_data,set_undeformed_coordinate_field_data;

	ENTER(define_Computed_field_type_2d_strain);
	USE_PARAMETER(computed_field_deformation_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		deformed_coordinate_field=(struct Computed_field *)NULL;
		undeformed_coordinate_field=(struct Computed_field *)NULL;
		fibre_angle_field=(struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_2d_strain_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_2d_strain(field_modify->get_field(),
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
				field_modify->get_field_manager();
			set_deformed_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_deformed_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"deformed_coordinate",
				&deformed_coordinate_field,&set_deformed_coordinate_field_data,
				set_Computed_field_conditional);
			/* undeformed coordinate */
			set_undeformed_coordinate_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_undeformed_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_undeformed_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"undeformed_coordinate",
				&undeformed_coordinate_field,&set_undeformed_coordinate_field_data,
				set_Computed_field_conditional);
			/* fibre_angle */
			set_fibre_angle_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_fibre_angle_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_fibre_angle_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"fibre_angle",
				&fibre_angle_field,&set_fibre_angle_field_data,
				set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_2d_strain(
						field_modify->get_field_module(),
						deformed_coordinate_field, undeformed_coordinate_field,
						fibre_angle_field));
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
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_deformation_package
		*computed_field_deformation_package =
		new Computed_field_deformation_package;

	ENTER(Computed_field_register_types_deformation);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_2d_strain_type_string,
			define_Computed_field_type_2d_strain,
			computed_field_deformation_package);
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

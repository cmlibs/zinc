

#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/geometry.h"
#include "general/geometry_app.h"

int set_Coordinate_system(struct Parse_state *state,
	void *coordinate_system_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Sets the coordinate system (including focus for prolate spheroidal and oblate
spheroidal).
==============================================================================*/
{
	char cylindrical_polar_flag,fibre_flag,not_applicable_flag,
		normalised_window_coordinates_flag,oblate_spheroidal_flag,
		prolate_spheroidal_flag,spherical_polar_flag,read_focus,
		rectangular_cartesian_flag;
	int return_code;
	struct Option_table *focus_option_table, *option_table;
	struct Coordinate_system *coordinate_system,coordinate_system_copy;

	ENTER(set_Coordinate_system);
	USE_PARAMETER(dummy_void);
	if (state)
	{
		if ((coordinate_system=(struct Coordinate_system *)coordinate_system_void)&&
			COPY(Coordinate_system)(&coordinate_system_copy,coordinate_system))
		{
			cylindrical_polar_flag=0;
			fibre_flag=0;
			not_applicable_flag=0;
			normalised_window_coordinates_flag=0;
			oblate_spheroidal_flag=0;
			prolate_spheroidal_flag=0;
			rectangular_cartesian_flag=0;
			spherical_polar_flag=0;
			option_table = CREATE(Option_table)();
			Option_table_add_char_flag_entry(option_table, "cylindrical_polar", &cylindrical_polar_flag);
			Option_table_add_char_flag_entry(option_table, "fibre", &fibre_flag);
			Option_table_add_char_flag_entry(option_table, "not_applicable",
				&not_applicable_flag);
			Option_table_add_char_flag_entry(option_table, "normalised_window_coordinates",
				&normalised_window_coordinates_flag);
			Option_table_add_char_flag_entry(option_table, "oblate_spheroidal", &oblate_spheroidal_flag);
			Option_table_add_char_flag_entry(option_table, "prolate_spheroidal", &prolate_spheroidal_flag);
			Option_table_add_char_flag_entry(option_table, "rectangular_cartesian", &rectangular_cartesian_flag);
			Option_table_add_char_flag_entry(option_table, "spherical_polar", &spherical_polar_flag);
			return_code=Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			if (return_code)
			{
				read_focus=0;
				coordinate_system_copy.parameters.focus=(FE_value)1.0;
				if (cylindrical_polar_flag)
				{
					coordinate_system_copy.type=CYLINDRICAL_POLAR;
				}
				else if (fibre_flag)
				{
					coordinate_system_copy.type=FIBRE;
				}
				else if (not_applicable_flag)
				{
					coordinate_system_copy.type=NOT_APPLICABLE;
				}
				else if (normalised_window_coordinates_flag)
				{
					coordinate_system_copy.type=NORMALISED_WINDOW_COORDINATES;
				}
				else if (oblate_spheroidal_flag)
				{
					coordinate_system_copy.type=OBLATE_SPHEROIDAL;
					read_focus=1;
				}
				else if (prolate_spheroidal_flag)
				{
					coordinate_system_copy.type=PROLATE_SPHEROIDAL;
					read_focus=1;
				}
				else if (rectangular_cartesian_flag)
				{
					coordinate_system_copy.type=RECTANGULAR_CARTESIAN;
				}
				else if (spherical_polar_flag)
				{
					coordinate_system_copy.type=SPHERICAL_POLAR;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Coordinate_system.  Unknown coordinate_system_type");
					return_code=0;
				}
				if (return_code&&read_focus&&state->current_token)
				{
					if (fuzzy_string_compare(state->current_token,"focus")||
						(!(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
					{
						focus_option_table = CREATE(Option_table)();
						Option_table_add_entry(focus_option_table,
							"focus", &coordinate_system_copy.parameters.focus,
							NULL,set_FE_value_positive);
						return_code=Option_table_parse(focus_option_table, state);
						DESTROY(Option_table)(&focus_option_table);
					}
				}
				if (return_code)
				{
					return_code=COPY(Coordinate_system)(coordinate_system,
						&coordinate_system_copy);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Coordinate_system.  Missing coordinate_system_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Coordinate_system.  Missing state");
		return_code=0;
	}

	return (return_code);
} /* set_Coordinate_system */

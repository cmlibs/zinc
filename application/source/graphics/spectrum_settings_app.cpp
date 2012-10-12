
#include <math.h>

#include "api/cmiss_zinc_configure.h"
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "computed_field/computed_field.h"
#include "general/enumerator.h"
#include "general/enumerator_private.h"
//#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/debug.h"
#include "command/parser.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum_settings_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "general/enumerator_private_app.h"

DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(Spectrum_settings_colour_mapping);

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LINEAR command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char ambient,amb_diff,diffuse,emission,extend_above,
		extend_below,fix_maximum,fix_minimum,reverse,specular,
		transparent_above,transparent_below;
	enum Spectrum_settings_colour_mapping colour_mapping;
	int black_band_int,component,number_of_bands,range_components,return_code;
	float band_ratio,step_value;
	FE_value colour_range[2],range[2];
	struct Modify_spectrum_data *modify_spectrum_data;
	struct Option_table *option_table, *render_option_table;
	struct Spectrum_settings *settings;

	ENTER(gfx_modify_spectrum_settings_linear);
	USE_PARAMETER(spectrum_command_data_void);
	if (state)
	{
		modify_spectrum_data=
			(struct Modify_spectrum_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->settings=CREATE(Spectrum_settings)();
			if (settings)
			{
				/* access since deaccessed in gfx_modify_spectrum */
				ACCESS(Spectrum_settings)(modify_spectrum_data->settings);

				Spectrum_settings_set_type(settings,SPECTRUM_LINEAR);

				colour_mapping = Spectrum_settings_get_colour_mapping(settings);
				ambient = 0;
				amb_diff = 0;
				band_ratio = 0.01;
				diffuse = 0;
				emission = 0;
				extend_above = 0;
				extend_below = 0;
				fix_maximum=0;
				fix_minimum=0;
				number_of_bands = 10;
				reverse = 0;
				specular = 0;
				step_value = 0.5;
				transparent_above = 0;
				transparent_below = 0;
				range_components = 2;
				colour_range[0] = 0.0;
				colour_range[1] = 1.0;
				range[0] = (FE_value)(modify_spectrum_data->spectrum_minimum);
				range[1] = (FE_value)(modify_spectrum_data->spectrum_maximum);
				component = Spectrum_settings_get_component_number(settings);

				option_table = CREATE(Option_table)();
				/* band_ratio */
				Option_table_add_float_entry(option_table, "band_ratio",
					&band_ratio);
				/* colour_range */
				Option_table_add_FE_value_vector_entry(option_table, "colour_range",
					colour_range, &range_components);
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&component);
				/* extend_above */
				Option_table_add_char_flag_entry(option_table, "extend_above",
					&extend_above);
				/* extend_below */
				Option_table_add_char_flag_entry(option_table, "extend_below",
					&extend_below);
				/* fix_maximum */
				Option_table_add_char_flag_entry(option_table, "fix_maximum",
					&fix_maximum);
				/* fix_minimum */
				Option_table_add_char_flag_entry(option_table, "fix_minimum",
					&fix_minimum);
				/* number_of_bands */
				Option_table_add_int_positive_entry(option_table, "number_of_bands",
					&number_of_bands);
				/* position */
				Option_table_add_int_non_negative_entry(option_table, "position",
					&(modify_spectrum_data->position));
				/* range */
				Option_table_add_FE_value_vector_entry(option_table, "range",
					range, &range_components);
				/* reverse */
				Option_table_add_char_flag_entry(option_table, "reverse",
					&reverse);
				/* step_value */
				Option_table_add_float_entry(option_table, "step_value",
					&step_value);
				/* transparent_above */
				Option_table_add_char_flag_entry(option_table, "transparent_above",
					&transparent_above);
				/* transparent_below */
				Option_table_add_char_flag_entry(option_table, "transparent_below",
					&transparent_below);
				/* conversion_mode */
				OPTION_TABLE_ADD_ENUMERATOR(Spectrum_settings_colour_mapping)(
					option_table, &colour_mapping);
				/* render_option */
				render_option_table = CREATE(Option_table)();
				Option_table_add_char_flag_entry(render_option_table, "ambient",
					&ambient);
				Option_table_add_char_flag_entry(render_option_table, "amb_diff",
					&amb_diff);
				Option_table_add_char_flag_entry(render_option_table, "diffuse",
					&diffuse);
				Option_table_add_char_flag_entry(render_option_table, "emission",
					&emission);
				Option_table_add_char_flag_entry(render_option_table, "specular",
					&specular);
				Option_table_add_suboption_table(option_table, render_option_table);


				if (!(return_code=Option_table_multi_parse(option_table,state)))
				{
					DEACCESS(Spectrum_settings)(&(modify_spectrum_data->settings));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					Spectrum_settings_set_colour_mapping(settings,
						colour_mapping);
				}
				if ( return_code )
				{
					if (specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"All spectrums are ambient diffuse.  The specular and emission flags are ignored.");
						return_code=0;
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;
					}
					else if (extend_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 1);
					}
					else if (transparent_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;
					}
					else if (extend_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 1);
					}
					else if (transparent_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					Spectrum_settings_set_component_number(settings,
						component);
					Spectrum_settings_set_colour_value_minimum(settings,
						colour_range[0]);
					Spectrum_settings_set_colour_value_maximum(settings,
						colour_range[1]);
					Spectrum_settings_set_range_minimum(settings,
						range[0]);
					Spectrum_settings_set_range_maximum(settings,
						range[1]);
					Spectrum_settings_set_reverse_flag(settings,
						reverse);
					Spectrum_settings_set_number_of_bands(settings,
						number_of_bands);
					black_band_int = (band_ratio * 1000.0 + 0.5);
					Spectrum_settings_set_black_band_proportion(settings,
						black_band_int);
					/* Must set step value after setting minimum and maximum range */
					Spectrum_settings_set_step_value(settings, step_value);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						Spectrum_settings_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						Spectrum_settings_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_settings_linear */

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LOG command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char ambient,amb_diff,diffuse,emission,extend_above,
		extend_below,fix_maximum,fix_minimum,left,reverse,right,
		specular,transparent_above,transparent_below;
	enum Spectrum_settings_colour_mapping colour_mapping;
	int black_band_int,component,number_of_bands,range_components,return_code;
	float band_ratio,exaggeration,step_value;
	FE_value colour_range[2],range[2];
	struct Modify_spectrum_data *modify_spectrum_data;
	struct Option_table *option_table, *render_option_table;
	struct Spectrum_settings *settings;

	ENTER(gfx_modify_spectrum_settings_log);
	USE_PARAMETER(spectrum_command_data_void);
	if (state)
	{
		modify_spectrum_data=(struct Modify_spectrum_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->settings=CREATE(Spectrum_settings)();
			if (settings)
			{
				/* access since deaccessed in gfx_modify_spectrum */
				ACCESS(Spectrum_settings)(modify_spectrum_data->settings);

				Spectrum_settings_set_type(settings,SPECTRUM_LOG);

				colour_mapping = Spectrum_settings_get_colour_mapping(settings);
				ambient = 0;
				amb_diff = 0;
				band_ratio = 0.01;
				diffuse = 0;
				emission = 0;
				extend_above = 0;
				extend_below = 0;
				fix_maximum=0;
				fix_minimum=0;
				left = 0;
				number_of_bands = 10;
				reverse = 0;
				right = 0;
				specular = 0;
				step_value = 0.5;
				transparent_above = 0;
				transparent_below = 0;
				range_components = 2;
				colour_range[0] = 0.0;
				colour_range[1] = 1.0;
				range[0] = (FE_value)(modify_spectrum_data->spectrum_minimum);
				range[1] = (FE_value)(modify_spectrum_data->spectrum_maximum);
				component = Spectrum_settings_get_component_number(settings);
				exaggeration = Spectrum_settings_get_exaggeration(settings);

				option_table = CREATE(Option_table)();
				/* band_ratio */
				Option_table_add_float_entry(option_table, "band_ratio",
					&band_ratio);
				/* colour_range */
				Option_table_add_FE_value_vector_entry(option_table, "colour_range",
					colour_range, &range_components);
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&component);
				/* exaggeration */
				Option_table_add_float_entry(option_table, "exaggeration",
					&exaggeration);
				/* extend_above */
				Option_table_add_char_flag_entry(option_table, "extend_above",
					&extend_above);
				/* extend_below */
				Option_table_add_char_flag_entry(option_table, "extend_below",
					&extend_below);
				/* fix_maximum */
				Option_table_add_char_flag_entry(option_table, "fix_maximum",
					&fix_maximum);
				/* fix_minimum */
				Option_table_add_char_flag_entry(option_table, "fix_minimum",
					&fix_minimum);
				/* left */
				Option_table_add_char_flag_entry(option_table, "left",
					&left);
				/* number_of_bands */
				Option_table_add_int_positive_entry(option_table, "number_of_bands",
					&number_of_bands);
				/* position */
				Option_table_add_int_non_negative_entry(option_table, "position",
					&(modify_spectrum_data->position));
				/* range */
				Option_table_add_FE_value_vector_entry(option_table, "range",
					range, &range_components);
				/* reverse */
				Option_table_add_char_flag_entry(option_table, "reverse",
					&reverse);
				/* right */
				Option_table_add_char_flag_entry(option_table, "right",
					&right);
				/* step_value */
				Option_table_add_float_entry(option_table, "step_value",
					&step_value);
				/* transparent_above */
				Option_table_add_char_flag_entry(option_table, "transparent_above",
					&transparent_above);
				/* transparent_below */
				Option_table_add_char_flag_entry(option_table, "transparent_below",
					&transparent_below);
				/* conversion_mode */
				Option_table_add_enumerator_Spectrum_settings_colour_mapping(
					option_table, &colour_mapping);
				/* render_option */
				render_option_table = CREATE(Option_table)();
				Option_table_add_char_flag_entry(render_option_table, "ambient",
					&ambient);
				Option_table_add_char_flag_entry(render_option_table, "amb_diff",
					&amb_diff);
				Option_table_add_char_flag_entry(render_option_table, "diffuse",
					&diffuse);
				Option_table_add_char_flag_entry(render_option_table, "emission",
					&emission);
				Option_table_add_char_flag_entry(render_option_table, "specular",
					&specular);
				Option_table_add_suboption_table(option_table, render_option_table);


				if (!(return_code=Option_table_multi_parse(option_table,state)))
				{
					DEACCESS(Spectrum_settings)(&(modify_spectrum_data->settings));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					Spectrum_settings_set_colour_mapping(settings,
						colour_mapping);
					Spectrum_settings_set_number_of_bands(settings,
						number_of_bands);
					black_band_int = (band_ratio * 1000.0 + 0.5);
					Spectrum_settings_set_black_band_proportion(settings,
						black_band_int);
					Spectrum_settings_set_step_value(settings, step_value);
				}
				if ( return_code )
				{
					if (specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"All spectrums are ambient diffuse.  The specular and emission flags are ignored.");
						return_code=0;
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;
					}
					else if (extend_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 1);
					}
					else if (transparent_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;
					}
					else if (extend_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 1);
					}
					else if (transparent_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					if (left && right)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of left or right");
						return_code=0;
					}
					else if (left)
					{
						exaggeration = fabs(exaggeration);
					}
					else if (right)
					{
						exaggeration = -fabs(exaggeration);
					}
					Spectrum_settings_set_exaggeration(settings,
						exaggeration);
				}
				if ( return_code )
				{
					Spectrum_settings_set_component_number(settings,
						component);
					Spectrum_settings_set_colour_value_minimum(settings,
						colour_range[0]);
					Spectrum_settings_set_colour_value_maximum(settings,
						colour_range[1]);
					Spectrum_settings_set_range_minimum(settings,
						range[0]);
					Spectrum_settings_set_range_maximum(settings,
						range[1]);
					Spectrum_settings_set_reverse_flag(settings,
						reverse);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						Spectrum_settings_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						Spectrum_settings_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_log */

int gfx_modify_spectrum_settings_field(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM FIELD command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	enum Spectrum_settings_colour_mapping colour_mapping;
	int component, return_code;
	struct Computed_field *input_field, *output_field;
	struct Modify_spectrum_data *modify_spectrum_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_input_field_data,
		set_output_field_data;
	struct Spectrum_settings *settings;

	ENTER(gfx_modify_spectrum_settings_field);
	USE_PARAMETER(spectrum_command_data_void);
	if (state)
	{
		modify_spectrum_data=(struct Modify_spectrum_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->settings=CREATE(Spectrum_settings)();
			if (settings)
			{
				/* access since deaccessed in gfx_modify_spectrum */
				ACCESS(Spectrum_settings)(modify_spectrum_data->settings);

				Spectrum_settings_set_type(settings,SPECTRUM_FIELD);

				colour_mapping = Spectrum_settings_get_colour_mapping(settings);
				component = Spectrum_settings_get_component_number(settings);

				input_field = (struct Computed_field *)NULL;
				output_field = (struct Computed_field *)NULL;
				if (settings->input_field)
				{
					input_field = ACCESS(Computed_field)(settings->input_field);
				}
				if (settings->output_field)
				{
					output_field = ACCESS(Computed_field)(settings->output_field);
				}

				option_table = CREATE(Option_table)();
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&component);
				/* input_field */
				set_input_field_data.computed_field_manager=
					modify_spectrum_data->computed_field_manager;
				set_input_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_input_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"input",
					&input_field ,&set_input_field_data,
					set_Computed_field_conditional);
				/* output_field */
				set_output_field_data.computed_field_manager=
					modify_spectrum_data->computed_field_manager;
				set_output_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_output_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"output",
					&output_field, &set_output_field_data,
					set_Computed_field_conditional);
				/* conversion_mode */
				Option_table_add_enumerator_Spectrum_settings_colour_mapping(
					option_table, &colour_mapping);

				if (!(return_code=Option_table_multi_parse(option_table,state)))
				{
					DEACCESS(Spectrum_settings)(&(modify_spectrum_data->settings));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (!input_field)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
							"You must specify a numerical input field.");
						return_code=0;
					}
					if (!output_field)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
							"You must specify a numerical input field.");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (input_field != settings->input_field)
					{
						REACCESS(Computed_field)(&settings->input_field,
							input_field);
						settings->settings_changed = 1;
					}
					if (output_field != settings->output_field)
					{
						REACCESS(Computed_field)(&settings->output_field,
							output_field);
						settings->settings_changed = 1;
					}
					Spectrum_settings_set_component_number(settings,
						component);
					Spectrum_settings_set_colour_mapping(settings,
						colour_mapping);
				}
				DEACCESS(Computed_field)(&input_field);
				DEACCESS(Computed_field)(&output_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_field */

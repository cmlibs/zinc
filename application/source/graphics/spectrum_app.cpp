

#include "general/message.h"
#include "general/mystring.h"
#include "command/parser.h"
#include "graphics/scene.h"
#include "graphics/graphics_module.h"
#include "graphics/spectrum.h"
#include "graphics/render_gl.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum_settings_app.h"
#include "graphics/scene_app.h"
#include "user_interface/process_list_or_write_command.hpp"


int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Spectrum *temp_spectrum,**spectrum_address;
	struct MANAGER(Spectrum) *spectrum_manager;

	ENTER(set_Spectrum);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				spectrum_address=(struct Spectrum **)spectrum_address_void;
				spectrum_manager=(struct MANAGER(Spectrum) *)spectrum_manager_void;
				if (spectrum_address && spectrum_manager)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*spectrum_address)
						{
							DEACCESS(Spectrum)(spectrum_address);
							*spectrum_address=(struct Spectrum *)NULL;
						}
						return_code=1;
					}
					else
					{
						temp_spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,	name)(current_token,spectrum_manager);
						if (temp_spectrum)
						{
							if (*spectrum_address!=temp_spectrum)
							{
								DEACCESS(Spectrum)(spectrum_address);
								*spectrum_address=ACCESS(Spectrum)(temp_spectrum);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"set_Spectrum.  Invalid argument(s).  spectrum_address %p.  spectrum_manager %p",
						spectrum_address_void,spectrum_manager_void);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SPECTRUM_NAME|none");
				/* if possible, then write the name */
				spectrum_address = (struct Spectrum **)spectrum_address_void;
				if (spectrum_address)
				{
					temp_spectrum = *spectrum_address;
					if (temp_spectrum)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_spectrum->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum */

int set_Spectrum_minimum_command(struct Parse_state *state,
	void *spectrum_ptr_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum minimum.
==============================================================================*/
{
	const char *current_token;
	float value;
	int return_code;
	struct Spectrum *spectrum;

	ENTER(set_Spectrum_minimum);
	if (state && (!dummy_user_data))
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				spectrum = *((struct Spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						set_Spectrum_minimum(spectrum,value);

						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid spectrum minimum : %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Spectrum_minimum_command.  Missing spectrum_ptr_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MINIMUM_VALUE#");
				spectrum = *((struct Spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",spectrum->minimum);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[calculated]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum minimum");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum_minimum_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_minimum_command */

int set_Spectrum_maximum_command(struct Parse_state *state,
	void *spectrum_ptr_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum maximum.
==============================================================================*/
{
	const char *current_token;
	float value;
	int return_code;
	struct Spectrum *spectrum;

	ENTER(set_Spectrum_maximum);
	if (state && (!dummy_user_data))
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				spectrum = *((struct Spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						set_Spectrum_maximum(spectrum,value);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(WARNING_MESSAGE,"Invalid spectrum maximum : %s",
							current_token);
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Spectrum_maximum_command.  Missing spectrum_ptr_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MAXIMUM_VALUE#");
				spectrum = *((struct Spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",spectrum->maximum);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[calculated]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum maximum");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum_maximum_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_maximum_command */

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum;

	ENTER(gfx_destroy_spectrum);
	if (state && (!dummy_to_be_modified))
	{
		spectrum_manager = (struct MANAGER(Spectrum) *)spectrum_manager_void;
		if (spectrum_manager)
		{
			current_token=state->current_token;
			if (current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)(current_token,spectrum_manager);
					if (spectrum)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Spectrum)(spectrum,
							spectrum_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown spectrum: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing spectrum name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_spectrum.  Missing spectrum_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_spectrum */

int process_list_or_write_Spectrum_commands(struct Spectrum *spectrum,
	 const char *command_prefix,char *command_suffix,
	 class Process_list_or_write_command_class *process_message)
/*******************************************************************************
LAST MODIFIED : 24 August 2007

DESCRIPTION :
List the properties of the <spectrum> to the command window or write
to the comfile.
==============================================================================*/
{
	char *line_prefix,*name;
	int return_code;
	struct Spectrum_settings_list_data list_data;
	ENTER(Spectrum_list_commands);
	/* check the arguments */
	if (spectrum)
	{
		name=duplicate_string(spectrum->name);
		if (name)
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			/* add spectrum name to command_prefix */
			if (strcmp(name, "default") != 0)
			{
				 process_message->process_command(INFORMATION_MESSAGE,"gfx create spectrum %s",name);
				 process_message->process_command(INFORMATION_MESSAGE,";\n");
			}
			if (ALLOCATE(line_prefix,char,strlen(command_prefix)+strlen(name)+3))
			{
				sprintf(line_prefix,"%s %s ",command_prefix,name);
				process_message->process_command(INFORMATION_MESSAGE,line_prefix);
				process_message->process_command(INFORMATION_MESSAGE,"clear");
				if (spectrum->clear_colour_before_settings)
				{
					process_message->process_command(INFORMATION_MESSAGE," overwrite_colour");
				}
				else
				{
					process_message->process_command(INFORMATION_MESSAGE," overlay_colour");
				}
				if (command_suffix)
				{
					process_message->process_command(INFORMATION_MESSAGE,command_suffix);
				}
				process_message->process_command(INFORMATION_MESSAGE,";\n");
				list_data.settings_string_detail=SPECTRUM_SETTINGS_STRING_COMPLETE;
				list_data.line_prefix=line_prefix;
				list_data.line_suffix=command_suffix;
				return_code = process_message->write_enabled();
				if (return_code == 0)
				{
					 return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
							Spectrum_settings_list_contents, (void *)&list_data,
							spectrum->list_of_settings);
				}
				else
				{
					 return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
							Spectrum_settings_write_contents, (void *)&list_data,
							spectrum->list_of_settings);
				}
				DEALLOCATE(line_prefix);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_list_commands.  Unable to allocate string");
				return_code=0;
			}
			DEALLOCATE(name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_list_commands.  Unable to duplicate string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_list_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_list_commands */

int for_each_spectrum_list_or_write_commands(struct Spectrum *spectrum,void *write_enabled_void)
/*******************************************************************************
LAST MODIFIED : 18 August 2007

DESCRIPTION :
For each spectrum in manager, list the spectrum commands to the command windows or write them out.
==============================================================================*/
{
	 static const char *command_prefix="gfx modify spectrum";
	 char *write_enabled;
	 int return_code;
	 ENTER(for_each_spectrum_list_or_write_commands);
	 write_enabled = (char *)write_enabled_void;
	 if (write_enabled)
	 {
			if (strcmp(write_enabled, "false") == 0)
			{
				 if (Process_list_command_class *list_message =
						new Process_list_command_class())
				 {
						return_code = process_list_or_write_Spectrum_commands(spectrum, command_prefix,
							 (char *)NULL, list_message);
						delete list_message;
				 }
				 else
				 {
						return_code = 0;
						display_message(ERROR_MESSAGE,"Spectrum_list_commands. Cannot create a process_command_class");
				 }
			}
			else
			{
				 if (Process_write_command_class *write_message =
						new Process_write_command_class())
				 {
						return_code = process_list_or_write_Spectrum_commands(spectrum, command_prefix,
							 (char *)NULL, write_message);
						delete write_message;
				 }
				 else
				 {
						return_code = 0;
						display_message(ERROR_MESSAGE,"Spectrum_list_commands. Cannot create a process_command_class");
				 }
			}
	 }
	 else
	 {
			return_code = 0;
			display_message(ERROR_MESSAGE,"for_each_spectrum_list_or_write_commands.  Invalid argument(s)");
	 }
	 LEAVE;

	 return (return_code);
}

int Spectrum_list_commands(struct Spectrum *spectrum,
	const char *command_prefix,char *command_suffix)
/*******************************************************************************
LAST MODIFIED : 24 Aug 2007

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/
{
	 int return_code;
	 ENTER(Spectrum_list_commands);
	 if (spectrum)
	 {
			if (Process_list_command_class *list_message =
				 new Process_list_command_class())
			{
				 return_code = process_list_or_write_Spectrum_commands(spectrum, command_prefix,
						command_suffix, list_message);
				 delete list_message;
			}
			else
			{
				 return_code = 0;
				 display_message(ERROR_MESSAGE,"Spectrum_list_commands. Cannot create a process_command_class");
			}
	 }
	 else
	 {
			return_code = 0;
			display_message(ERROR_MESSAGE,"Spectrum_list_commands.  Invalid argument(s)");
	 }
	 return (return_code);
}


int Cmiss_spectrum_execute_command(Cmiss_spectrum_id spectrum, const char *command_string)
{
	int return_code = 0;

	if (spectrum && command_string)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		if (state)
		{
			struct Cmiss_graphics_module *graphics_module =	MANAGER_GET_OWNER(Spectrum)(
				spectrum->manager);
			if(graphics_module)
			{
				char autorange, blue_to_red, blue_white_red, clear, lg_blue_to_red,
					lg_red_to_blue, overlay_colour, overwrite_colour, red_to_blue;
				struct Modify_spectrum_data modify_spectrum_data;
				float maximum, minimum;
				struct Scene *autorange_scene;
				struct Spectrum_command_data spectrum_command_data;
				struct Option_table *option_table = NULL;
				int range_set = 0;;
				autorange = 0;
				autorange_scene = Cmiss_graphics_module_get_default_scene(graphics_module);
				blue_to_red = 0;
				clear = 0;
				lg_blue_to_red = 0;
				lg_red_to_blue = 0;
				overlay_colour = 0;
				overwrite_colour = 0;
				red_to_blue = 0;
				blue_white_red = 0;
				modify_spectrum_data.position = 0;
				modify_spectrum_data.settings = (struct Spectrum_settings *)NULL;
				modify_spectrum_data.spectrum_minimum = get_Spectrum_minimum(
					spectrum);
				modify_spectrum_data.spectrum_maximum = get_Spectrum_maximum(
					spectrum);
				modify_spectrum_data.computed_field_manager	= NULL;
				spectrum_command_data.spectrum_manager = spectrum->manager;
				option_table=CREATE(Option_table)();
				Option_table_add_entry(option_table,"autorange",&autorange,NULL,
					set_char_flag);
				Option_table_add_entry(option_table,"blue_to_red",&blue_to_red,NULL,
					set_char_flag);
				Option_table_add_entry(option_table,"blue_white_red",&blue_white_red,NULL,
					set_char_flag);
				Option_table_add_entry(option_table,"clear",&clear,NULL,
					set_char_flag);
				Option_table_add_entry(option_table,"field",&modify_spectrum_data,
					&spectrum_command_data,gfx_modify_spectrum_settings_field);
				Option_table_add_entry(option_table,"linear",&modify_spectrum_data,
					&spectrum_command_data,gfx_modify_spectrum_settings_linear);
				Option_table_add_entry(option_table,"log",&modify_spectrum_data,
					&spectrum_command_data,gfx_modify_spectrum_settings_log);
				Option_table_add_entry(option_table,"lg_blue_to_red",&lg_blue_to_red,
					NULL,set_char_flag);
				Option_table_add_entry(option_table,"lg_red_to_blue",&lg_red_to_blue,
					NULL,set_char_flag);
				Option_table_add_entry(option_table,"maximum",&spectrum,
					NULL,set_Spectrum_maximum_command);
				Option_table_add_entry(option_table,"minimum",&spectrum,
					NULL,set_Spectrum_minimum_command);
				Option_table_add_entry(option_table,"overlay_colour",&overlay_colour,
					NULL,set_char_flag);
				Option_table_add_entry(option_table,"overwrite_colour",&overwrite_colour,
					NULL,set_char_flag);
				Option_table_add_entry(option_table,"scene_for_autorange",&autorange_scene,
					Cmiss_graphics_module_get_scene_manager(graphics_module),set_Scene);
				Option_table_add_entry(option_table,"red_to_blue",&red_to_blue,
					NULL,set_char_flag);
				return_code=Option_table_multi_parse(option_table,state);
				if (return_code)
				{
					if (return_code)
					{
						if ( clear )
						{
							Spectrum_remove_all_settings(spectrum);
						}
						if (blue_to_red + blue_white_red +red_to_blue + lg_red_to_blue +
							lg_blue_to_red > 1 )
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Specify only one simple spectrum type\n "
								"   (blue_to_red, blue_white_red, red_to_blue, lg_red_to_blue, lg_blue_to_red)");
							return_code=0;
						}
						else if (red_to_blue)
						{
							Spectrum_set_simple_type(spectrum, RED_TO_BLUE_SPECTRUM);
						}
						else if (blue_to_red)
						{
							Spectrum_set_simple_type(spectrum, BLUE_TO_RED_SPECTRUM);
						}
						else if (blue_white_red)
						{
							Spectrum_set_simple_type(spectrum, BLUE_WHITE_RED_SPECTRUM);
						}
						else if (lg_red_to_blue)
						{
							Spectrum_set_simple_type(spectrum, LOG_RED_TO_BLUE_SPECTRUM);
						}
						else if (lg_blue_to_red)
						{
							Spectrum_set_simple_type(spectrum, LOG_BLUE_TO_RED_SPECTRUM);
						}
						if ( modify_spectrum_data.settings )
						{
							/* add new settings */
							return_code=Spectrum_add_settings(spectrum,modify_spectrum_data.settings,
								modify_spectrum_data.position);
						}
						if (overlay_colour && overwrite_colour)
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Specify only one colour mode, overwrite_colour or overlay_colour");
							return_code=0;
						}
						else if (overlay_colour)
						{
							Spectrum_set_opaque_colour_flag(spectrum,	0);
						}
						else if (overwrite_colour)
						{
							Spectrum_set_opaque_colour_flag(spectrum,	1);
						}
						if (autorange)
						{
							/* Could also do all scenes */
							range_set = 0;
							Scene_get_data_range_for_spectrum(autorange_scene, spectrum,
								&minimum, &maximum, &range_set);
							if ( range_set )
							{
								Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum );
							}
						}
					}
				}
				if(option_table)
				{
					DESTROY(Option_table)(&option_table);
				}
				if ( modify_spectrum_data.settings )
				{
					DEACCESS(Spectrum_settings)(&(modify_spectrum_data.settings));
				}
				DEACCESS(Scene)(&autorange_scene);
			}
		}
	}

	return return_code;
}


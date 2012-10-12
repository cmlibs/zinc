
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/scene.h"
#include "graphics/render_gl.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "graphics/tessellation_app.hpp"
#include "user_interface/process_list_or_write_command.hpp"
#include "finite_element/finite_element_region_app.h"
#include "graphics/font.h"
// insert app headers here
#include "graphics/graphic_app.h"

int gfx_modify_rendition_general(struct Parse_state *state,
	void *cmiss_region_void, void *group_void)
{
	int return_code = 0;

	ENTER(gfx_modify_rendition_general);
	Cmiss_region_id cmiss_region = reinterpret_cast<Cmiss_region_id>(cmiss_region_void);
	Cmiss_field_group_id group = reinterpret_cast<Cmiss_field_group_id>(group_void);
	if (state && cmiss_region)
	{
		/* if possible, get defaults from element_group on default scene */
		Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(cmiss_region);
		if (rendition)
		{
			Cmiss_field_id default_coordinate_field = rendition->default_coordinate_field;
			if (default_coordinate_field)
			{
				Cmiss_field_access(default_coordinate_field);
			}
			int clear_flag = 0;

			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The clear option removes all graphics from the rendition. "
				"Option 'circle_discretization' is deprecated: use circle_discretization option on cylinders instead. "
				"Option 'default_coordinate' is deprecated: use coordinate option on individual graphics instead. "
				"Option 'element_discretization' is deprecated: use tessellation option on individual graphics instead. "
				"Option 'native_discretization' is deprecated: use native_discretization option on individual graphics instead. ");
			/* circle_discretization */
			Option_table_add_entry(option_table, "circle_discretization",
				(void *)&rendition->circle_discretization, (void *)NULL,
				set_Circle_discretization);
			/* clear */
			Option_table_add_entry(option_table, "clear",
				(void *)&clear_flag, NULL, set_char_flag);
			/* default_coordinate */
			Set_Computed_field_conditional_data set_coordinate_field_data;
			set_coordinate_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(cmiss_region);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table, "default_coordinate",
				(void *)&default_coordinate_field, (void *)&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* element_discretization */
			Option_table_add_divisions_entry(option_table, "element_discretization",
				&(rendition->element_divisions), &(rendition->element_divisions_size));
			/* native_discretization */
			Set_FE_field_conditional_FE_region_data native_discretization_field_conditional_data;
			native_discretization_field_conditional_data.conditional_function =
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
			native_discretization_field_conditional_data.user_data = (void *)NULL;
			native_discretization_field_conditional_data.fe_region =
				Cmiss_region_get_FE_region(cmiss_region);
			Option_table_add_entry(option_table, "native_discretization",
				(void *)&rendition->native_discretization_field,
				(void *)&native_discretization_field_conditional_data,
				set_FE_field_conditional_FE_region);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (clear_flag)
				{
					if (group)
					{
						// remove only graphics using group as subgroup field
						Cmiss_rendition_begin_change(rendition);
						Cmiss_field_id group_field = Cmiss_field_group_base_cast(group);
						// support legacy command files by changing visibility of each graphic using group as its subgroup field
						Cmiss_graphic_id graphic = Cmiss_rendition_get_first_graphic(rendition);
						while (graphic)
						{
							Cmiss_graphic_id this_graphic = graphic;
							graphic = Cmiss_rendition_get_next_graphic(rendition, this_graphic);
							Cmiss_field_id subgroup_field = 0;
							Cmiss_graphic_get_subgroup_field(this_graphic, &subgroup_field);
							if (subgroup_field == group_field)
							{
								Cmiss_rendition_remove_graphic(rendition, this_graphic);
							}
							Cmiss_graphic_destroy(&this_graphic);
						}
						Cmiss_rendition_end_change(rendition);
					}
					else
					{
						return_code = Cmiss_rendition_remove_all_graphics(rendition);
					}
				}
				if (default_coordinate_field)
				{
					if (rendition->default_coordinate_field && default_coordinate_field &&
						(rendition->default_coordinate_field != default_coordinate_field))
					{
						display_message(WARNING_MESSAGE,
							"Change of default_coordinate field can have unexpected results. "
							"Please specify coordinate field for each graphic instead.");
						display_parse_state_location(state);
					}
					Cmiss_rendition_set_default_coordinate_field(rendition, default_coordinate_field);
				}
			}
			if (default_coordinate_field)
			{
				DEACCESS(Computed_field)(&default_coordinate_field);
			}
			Cmiss_rendition_destroy(&rendition);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_rendition_general.  Missing rendition");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_general.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}


struct Cmiss_rendition_process_temp_data
{
	 class Process_list_or_write_command_class *process_message;
	 struct Cmiss_graphic_list_data *list_data;
};

/***************************************************************************//**
 * Writes out the <graphic> as a text string in the command window with the
 * <graphic_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int Cmiss_rendition_process_Cmiss_graphic_list_contents(
	 struct Cmiss_graphic *graphic,	void *process_temp_data_void)
{
	int return_code;
	char *graphic_string; /*,line[40];*/
	struct Cmiss_graphic_list_data *list_data;
	class Process_list_or_write_command_class *process_message;
	struct Cmiss_rendition_process_temp_data *process_temp_data;

	ENTER(Cmiss_rendition_process_Cmiss_graphic_list_contents);
	if (graphic&&
		 (process_temp_data=(struct Cmiss_rendition_process_temp_data *)process_temp_data_void))
	{
		 if (NULL != (process_message = process_temp_data->process_message) &&
			 NULL != (list_data = process_temp_data->list_data))
		 {
			 if (NULL != (graphic_string=Cmiss_graphic_string(graphic,
						 list_data->graphic_string_detail)))
				{
					 if (list_data->line_prefix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_prefix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,graphic_string);
					 if (list_data->line_suffix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_suffix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,"\n");
					 DEALLOCATE(graphic_string);
					 return_code=1;
				}
			 return_code= 1;
		 }
		 else
		 {
				return_code=0;
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Cmiss_rendition_process_Cmiss_graphic_list_contents.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Will write commands to comfile or list commands to the command windows
 * depending on the class.
 */
int Cmiss_rendition_process_list_or_write_window_commands(struct Cmiss_rendition *rendition,
	 const char *command_prefix, const char *command_suffix, class Process_list_or_write_command_class *process_message)
{
	int return_code;
	struct Cmiss_graphic_list_data list_data;

	ENTER(Cmiss_rendition_process_list_or_write_window_command);
	if (rendition && command_prefix)
	{
		process_message->process_command(INFORMATION_MESSAGE,command_prefix);
		process_message->process_command(INFORMATION_MESSAGE,"general clear");
		if (command_suffix)
		{
			process_message->process_command(INFORMATION_MESSAGE,command_suffix);
		}
		process_message->process_command(INFORMATION_MESSAGE,"\n");
		list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE;
		list_data.line_prefix=command_prefix;
		list_data.line_suffix=command_suffix;
		struct Cmiss_rendition_process_temp_data *process_temp_data;
		if (ALLOCATE(process_temp_data,struct Cmiss_rendition_process_temp_data,1))
		{
			 process_temp_data->process_message = process_message;
			 process_temp_data->list_data = &list_data;
			 return_code=FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
					Cmiss_rendition_process_Cmiss_graphic_list_contents,(void *)process_temp_data,
					rendition->list_of_graphics);
			 DEALLOCATE(process_temp_data);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_list_commands.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_process_list_or_write_window_command*/

int Cmiss_rendition_list_commands(struct Cmiss_rendition *rendition,
	 const char *command_prefix, const char *command_suffix)
{
	 int return_code = 0;

	 ENTER(Cmiss_rendition_list_commands);
	 Process_list_command_class *list_message =
			new Process_list_command_class();
	 if (list_message)
	 {
			return_code = Cmiss_rendition_process_list_or_write_window_commands(
				rendition, command_prefix, command_suffix, list_message);
			delete list_message;
	 }
	 LEAVE;

	 return (return_code);
}

int Cmiss_rendition_list_contents(struct Cmiss_rendition *rendition)
{
	char *name = 0;
	int return_code;
	struct Cmiss_graphic_list_data list_data;

	ENTER(Cmiss_rendition_list_contents);
	if (rendition)
	{
		if (rendition->circle_discretization)
		{
			display_message(INFORMATION_MESSAGE,"  Legacy circle discretization: %d\n",
				rendition->circle_discretization);
		}
		if (rendition->default_coordinate_field)
		{
			if (GET_NAME(Computed_field)(rendition->default_coordinate_field,
				&name))
			{
				display_message(INFORMATION_MESSAGE,
					"  Legacy default coordinate field: %s\n",name);
				DEALLOCATE(name);
			}
		}
		if (rendition->element_divisions)
		{
			display_message(INFORMATION_MESSAGE, "  Legacy element discretization: \"");
			for (int i = 0; i < rendition->element_divisions_size; i++)
			{
				if (i)
					display_message(INFORMATION_MESSAGE, "*");
				display_message(INFORMATION_MESSAGE, "%d", rendition->element_divisions[i]);
			}
			display_message(INFORMATION_MESSAGE, "\"\n");
		}
		if (rendition->native_discretization_field)
		{
			if (GET_NAME(FE_field)(rendition->native_discretization_field,
				&name))
			{
				display_message(INFORMATION_MESSAGE,"  Legacy native discretization field: %s\n",name);
				DEALLOCATE(name);
			}
		}
		if (0 < NUMBER_IN_LIST(Cmiss_graphic)(
			rendition->list_of_graphics))
		{
			display_message(INFORMATION_MESSAGE,"  graphics objects defined:\n");
			list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE_PLUS;
			list_data.line_prefix="  ";
			list_data.line_suffix="";
			return_code=FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_list_contents,(void *)&list_data,
				rendition->list_of_graphics);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no graphics graphic defined\n");
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_rendition_list_contents.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_rendition_list_contents */


int Cmiss_rendition_execute_command_internal(Cmiss_rendition_id rendition,
	Cmiss_field_group_id group, struct Parse_state *state)
{
	int return_code = 0;
	if (rendition && state)
	{
		struct Cmiss_graphics_module *graphics_module =	rendition->graphics_module;
		if(graphics_module)
		{
			struct Option_table *option_table;
			struct Modify_rendition_data modify_rendition_data;
			modify_rendition_data.delete_flag = 0;
			modify_rendition_data.position = -1;
			modify_rendition_data.graphic = (struct Cmiss_graphic *)NULL;
			modify_rendition_data.modify_this_graphic = 0;
			modify_rendition_data.group = group;
			int previous_state_index;
			if (state && (previous_state_index = state->current_index))
			{
				option_table = CREATE(Option_table)();
				/* as */
				char *graphic_name = (char *)NULL;
				Option_table_add_name_entry(option_table, "as", &graphic_name);
				/* default to absorb everything else */
				char *dummy_string = (char *)NULL;
				Option_table_add_name_entry(option_table, (char *)NULL, &dummy_string);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (graphic_name && (modify_rendition_data.graphic = first_graphic_in_Cmiss_rendition_that(
								rendition, Cmiss_graphic_has_name, (void *)graphic_name)))
					{
						ACCESS(Cmiss_graphic)(modify_rendition_data.graphic);
					}
				}
				if (dummy_string)
				{
					DEALLOCATE(dummy_string);
				}
				if (graphic_name)
				{
					DEALLOCATE(graphic_name);
				}
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}

			struct Rendition_command_data rendition_command_data;
			Cmiss_rendition_fill_rendition_command_data(rendition,
				&rendition_command_data);

			option_table = CREATE(Option_table)();
			/* cylinders */
			Option_table_add_entry(option_table, "cylinders",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_cylinders);
			/* data_points */
			Option_table_add_entry(option_table, "data_points",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_data_points);
			/* element_points */
			Option_table_add_entry(option_table, "element_points",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_element_points);
			/* general */
			Option_table_add_entry(option_table, "general",
				(void *)rendition->region, (void *)group, gfx_modify_rendition_general);
			/* iso_surfaces */
			Option_table_add_entry(option_table, "iso_surfaces",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_iso_surfaces);
			/* lines */
			Option_table_add_entry(option_table, "lines",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_lines);
			/* node_points */
			Option_table_add_entry(option_table, "node_points",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_node_points);
			/* point */
			Option_table_add_entry(option_table, "point",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_point);
			/* streamlines */
			Option_table_add_entry(option_table, "streamlines",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_streamlines);
			/* surfaces */
			Option_table_add_entry(option_table, "surfaces",
				(void *)&modify_rendition_data, (void *)&rendition_command_data,
				gfx_modify_rendition_surfaces);

			return_code = Option_table_parse(option_table, state);
			if (return_code && (modify_rendition_data.graphic))
			{
				return_code = Cmiss_region_modify_rendition(rendition->region,
					modify_rendition_data.graphic,
					modify_rendition_data.delete_flag,
					modify_rendition_data.position);
			} /* parse error,help */
			DESTROY(Option_table)(&option_table);
			if (modify_rendition_data.graphic)
			{
				DEACCESS(Cmiss_graphic)(&(modify_rendition_data.graphic));
			}
			if (rendition_command_data.default_font)
			{
				DEACCESS(Graphics_font)(&rendition_command_data.default_font);
			}
			if (rendition_command_data.default_spectrum)
			{
				DEACCESS(Spectrum)(&rendition_command_data.default_spectrum);
			}
			Cmiss_region_destroy(&(rendition_command_data.root_region));
		}
	}
	return return_code;
}

int Cmiss_rendition_execute_command(Cmiss_rendition_id rendition, const char *command_string)
{
	int return_code = 0;
	if (rendition && command_string)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		return_code = Cmiss_rendition_execute_command_internal(rendition, (Cmiss_field_group_id)0, state);
		destroy_Parse_state(&state);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Cmiss_rendition_transformation_commands.  Invalid argument(s)");
		return_code=0;
	}
	return return_code;
}


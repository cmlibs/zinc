/*******************************************************************************
FILE : unemap_command.c

LAST MODIFIED : 22 July 2002

DESCRIPTION :
Functions and for executing unemap commands.
==============================================================================*/
#include "command/command.h"
#include "command/parser.h"
#include "general/debug.h"
#include "general/object.h"
#if defined (UNEMAP_USE_3D)
#include "graphics/material.h"
#include "graphics/scene.h"
#include "node/node_tool.h"
#endif /* defined (UNEMAP_USE_3D) */
#include "unemap/unemap_command.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Unemap_command_data
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION :
Data required for Unemap commands.
==============================================================================*/
{
	struct Event_dispatcher *event_dispatcher;
	struct Execute_command *execute_command;
	struct User_interface *user_interface;
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#if defined (MOTIF)
	struct Node_tool *node_tool;
	struct Interactive_tool *transform_tool;
#endif /* defined (MOTIF) */
	/* list of glyphs = simple graphics objects with only geometry */
	struct LIST(GT_object) *glyph_list;
	struct FE_time *fe_time;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(FE_basis) *basis_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *data_manager,*node_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_graphical_material;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	/* global list of selected objects */
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection, *node_selection;
#endif /* defined (UNEMAP_USE_3D) */
	struct System_window *unemap_system_window;
	/* flag indicating whether the unemap_system_window can be destroyed when
		 the command data is destroyed.
		 Set only if system window created by commands, not passed in */ 
	int unemap_system_window_destroyable;
	struct Time_keeper *default_time_keeper;
#else /* defined (NOT_ACQUISITION_ONLY) */
	struct Page_window *page_window;
#endif /* defined (NOT_ACQUISITION_ONLY) */
}; /* struct Unemap_command_data */

/*
Module functions
----------------
*/

#if defined (NOT_ACQUISITION_ONLY)
void Unemap_system_window_destroy_in_unemap_command_data(
	struct System_window *system_window, void *unemap_command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION :
Callback function for when the close button is clicked in the unemap system
window -- but only if unemap system window is run as a sub-application.
==============================================================================*/
{
	struct Unemap_command_data *unemap_command_data;

	ENTER(Unemap_system_window_destroy_in_unemap_command_data);
	USE_PARAMETER(system_window);
	if (unemap_command_data =
		(struct Unemap_command_data *)unemap_command_data_void)
	{
		if (unemap_command_data->unemap_system_window_destroyable)
		{
			DESTROY(System_window)(&(unemap_command_data->unemap_system_window));
			unemap_command_data->unemap_system_window = (struct System_window *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Unemap_system_window_destroy_in_unemap_command_data.  "
				"System window is not destroyable");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Unemap_system_window_destroy_in_unemap_command_data.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Unemap_system_window_destroy_in_unemap_command_data */
#endif /* defined (NOT_ACQUISITION_ONLY) */

static int execute_command_unemap_open(struct Parse_state *state,
	void *dummy_to_be_modified, void *unemap_command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 July 2002

DESCRIPTION :
Executes a UNEMAP OPEN command.
==============================================================================*/
{	
	char *current_token;
	int return_code;
	struct Unemap_command_data *unemap_command_data;
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
	struct Colour colour;
	struct Graphical_material *electrode_selected_material;
	struct Scene *default_scene;
#endif /* defined (UNEMAP_USE_3D) */
	struct System_window *system;
	Widget shell;
#endif /* defined (NOT_ACQUISITION_ONLY) */

	ENTER(execute_command_unemap_open);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (unemap_command_data =
		(struct Unemap_command_data *)unemap_command_data_void))
	{
		if (!((current_token = state->current_token) &&
			!(strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
		{
#if defined (NOT_ACQUISITION_ONLY)
			if (system = unemap_command_data->unemap_system_window)
			{
				System_window_pop_up(unemap_command_data->unemap_system_window);
			}
			else
			{
#if defined (UNEMAP_USE_3D)
				/* create material "electrode_selected" to be bright white for
					 highlighting electrode graphics */
				if (!(FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
					"electrode_selected",
					unemap_command_data->graphical_material_manager)))
				{
					if (electrode_selected_material =
						CREATE(Graphical_material)("electrode_selected"))
					{
						colour.red = 1.0;
						colour.green = 1.0;
						colour.blue = 1.0;
						Graphical_material_set_ambient(electrode_selected_material
							,&colour);
						Graphical_material_set_diffuse(electrode_selected_material,
							&colour);
						/* ACCESS so can never be destroyed */
						ACCESS(Graphical_material)(electrode_selected_material);
						if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
							electrode_selected_material,
							unemap_command_data->graphical_material_manager))
						{
							DEACCESS(Graphical_material)(&electrode_selected_material);
						}
					}
				}
#endif /* defined (UNEMAP_USE_3D) */
				/* create a shell */
				if (shell = XtVaCreatePopupShell("system_window_shell",
					topLevelShellWidgetClass,
					User_interface_get_application_shell(
						unemap_command_data->user_interface),
					XmNallowShellResize, False,
					NULL))
				{
					if (system = create_System_window(shell,
						Unemap_system_window_destroy_in_unemap_command_data,
						(void *)unemap_command_data,
#if defined (UNEMAP_USE_3D)
						unemap_command_data->element_point_ranges_selection,
						unemap_command_data->element_selection,
						unemap_command_data->fe_field_manager,
						unemap_command_data->node_selection,
						unemap_command_data->data_selection,
						unemap_command_data->fe_time,
						unemap_command_data->basis_manager,
						unemap_command_data->element_manager,
						unemap_command_data->data_manager,
						unemap_command_data->node_manager,
						unemap_command_data->element_group_manager,
						unemap_command_data->data_group_manager,
						unemap_command_data->node_group_manager,
						unemap_command_data->texture_manager,
						unemap_command_data->interactive_tool_manager,
						unemap_command_data->scene_manager,
						unemap_command_data->light_model_manager,
						unemap_command_data->light_manager,
						unemap_command_data->spectrum_manager,
						unemap_command_data->graphical_material_manager,
						unemap_command_data->glyph_list,						
						unemap_command_data->default_graphical_material,
						unemap_command_data->computed_field_package,
						unemap_command_data->default_light,
						unemap_command_data->default_light_model,
#endif /* defined (UNEMAP_USE_3D) */
						unemap_command_data->default_time_keeper,
						unemap_command_data->user_interface))
					{
						unemap_command_data->unemap_system_window = system;
						/* need to set destroyable flag for system window created here */
						unemap_command_data->unemap_system_window_destroyable = 1;
						/* turn off default lines for graphics in default scene,
							 as updating these slows down unemap 3d window */
#if defined (UNEMAP_USE_3D)
						if (default_scene = FIND_BY_IDENTIFIER_IN_MANAGER(Scene, name)(
							"default", unemap_command_data->scene_manager))
						{
							Scene_set_graphical_element_mode(default_scene,
								GRAPHICAL_ELEMENT_EMPTY,
								Computed_field_package_get_computed_field_manager(
									unemap_command_data->computed_field_package),
								unemap_command_data->element_manager,
								unemap_command_data->element_group_manager,
								unemap_command_data->fe_field_manager,
								unemap_command_data->node_manager,
								unemap_command_data->node_group_manager,
								unemap_command_data->data_manager,
								unemap_command_data->data_group_manager,
								unemap_command_data->element_point_ranges_selection,
								unemap_command_data->element_selection,
								unemap_command_data->node_selection,
								unemap_command_data->data_selection,
								unemap_command_data->user_interface);
						}
#endif /* defined (UNEMAP_USE_3D) */
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE, "execute_command_unemap_open.  "
							"Could not create unemap_system_window");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_unemap_open.  "
						"Could not create unemap system window shell");
					return_code = 0;
				}
			}
#else /* defined (NOT_ACQUISITION_ONLY) */
			USE_PARAMETER(unemap_command_data);
#endif /* defined (NOT_ACQUISITION_ONLY) */
		}
		else
		{
			/* no help */
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_unemap_open.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_unemap_open */

/*
Global functions
----------------
*/

struct Unemap_command_data *CREATE(Unemap_command_data)(
	struct Event_dispatcher *event_dispatcher,
	struct Execute_command *execute_command,
	struct User_interface *user_interface,
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#if defined (MOTIF)
	struct Node_tool *node_tool,
	struct Interactive_tool *transform_tool,
#endif /* defined (MOTIF) */
	struct LIST(GT_object) *glyph_list,
	struct FE_time *fe_time,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *data_selection,
	struct FE_node_selection *node_selection,
#endif /* defined (UNEMAP_USE_3D) */
	struct System_window *unemap_system_window,
	struct Time_keeper *default_time_keeper
#else /* defined (NOT_ACQUISITION_ONLY) */
	struct Page_window *page_window
#endif /* defined (NOT_ACQUISITION_ONLY) */
	)
/*******************************************************************************
LAST MODIFIED : 22 July 2002

DESCRIPTION :
Creates a Unemap_command_data structure containing pointers to the passed
objects.
Note only the unemap_system_window is optional. If it is passed into this
structure here then it will not be destroyed when the command data is destroyed.
If the unemap_system_window is created by commands within this structure, it
will be destroyed with it.
==============================================================================*/
{
  struct Unemap_command_data *unemap_command_data;

	ENTER(CREATE(Unemap_command_data));
	unemap_command_data = (struct Unemap_command_data *)NULL;
	if (event_dispatcher && execute_command &&
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#if defined (MOTIF)
		node_tool && transform_tool &&
#endif /* defined (MOTIF) */
		glyph_list && fe_time && computed_field_package && basis_manager &&
		element_manager && fe_field_manager && data_manager && node_manager &&
		graphical_material_manager && default_graphical_material &&
		element_group_manager && data_group_manager && node_group_manager &&
		interactive_tool_manager && light_manager && default_light &&
		light_model_manager && default_light_model && texture_manager &&
		scene_manager && spectrum_manager && element_point_ranges_selection &&
		element_selection && data_selection && node_selection &&
#endif /* defined (UNEMAP_USE_3D) */
		default_time_keeper
#else /* defined (NOT_ACQUISITION_ONLY) */
		page_window
#endif /* defined (NOT_ACQUISITION_ONLY) */
		)
	{
		if (ALLOCATE(unemap_command_data, struct Unemap_command_data, 1))
		{
			unemap_command_data->event_dispatcher = event_dispatcher;
			unemap_command_data->execute_command = execute_command;
			unemap_command_data->user_interface = user_interface;
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#if defined (MOTIF)
			unemap_command_data->node_tool = node_tool;
			unemap_command_data->transform_tool = transform_tool;
#endif /* defined (MOTIF) */
			unemap_command_data->glyph_list = glyph_list;
			unemap_command_data->fe_time = fe_time;
			unemap_command_data->computed_field_package = computed_field_package;
			unemap_command_data->basis_manager = basis_manager;
			unemap_command_data->element_manager = element_manager;
			unemap_command_data->fe_field_manager = fe_field_manager;
			unemap_command_data->data_manager = data_manager;
			unemap_command_data->node_manager = node_manager;
			unemap_command_data->graphical_material_manager =
				graphical_material_manager;
			unemap_command_data->default_graphical_material =
				default_graphical_material;
			unemap_command_data->element_group_manager = element_group_manager;
			unemap_command_data->data_group_manager = data_group_manager;
			unemap_command_data->node_group_manager = node_group_manager;
			unemap_command_data->interactive_tool_manager = interactive_tool_manager;
			unemap_command_data->light_manager = light_manager;
			unemap_command_data->default_light = default_light;
			unemap_command_data->light_model_manager = light_model_manager;
			unemap_command_data->default_light_model = default_light_model;
			unemap_command_data->texture_manager = texture_manager;
			unemap_command_data->scene_manager = scene_manager;
			unemap_command_data->spectrum_manager = spectrum_manager;
			unemap_command_data->element_point_ranges_selection = element_point_ranges_selection;
			unemap_command_data->element_selection = element_selection;
			unemap_command_data->data_selection = data_selection;
			unemap_command_data->node_selection = node_selection;
#endif /* defined (UNEMAP_USE_3D) */
			unemap_command_data->default_time_keeper = default_time_keeper;
			unemap_command_data->unemap_system_window = unemap_system_window;
			unemap_command_data->unemap_system_window_destroyable = 0;
#else /* defined (NOT_ACQUISITION_ONLY) */
			unemap_command_data->page_window=page_window;
#endif /* defined (NOT_ACQUISITION_ONLY) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Unemap_command_data).  Could not allocate");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Unemap_command_data).  Missing argument(s)");
	}
	LEAVE;

	return (unemap_command_data);
} /* CREATE(Unemap_command_data) */

int DESTROY(Unemap_command_data)(
	struct Unemap_command_data **unemap_command_data_address)
/*******************************************************************************
LAST MODIFIED : 18 July 2002

DESCRIPTION :
Destroys the Unemap_command_data at <unemap_command_data_address>.
==============================================================================*/
{
	int return_code;
  struct Unemap_command_data *unemap_command_data;

	ENTER(CREATE(Unemap_command_data));
	if (unemap_command_data_address &&
		(unemap_command_data = *unemap_command_data_address))
	{
#if defined (NOT_ACQUISITION_ONLY)
		if (unemap_command_data->unemap_system_window_destroyable)
		{
			DESTROY(System_window)(&(unemap_command_data->unemap_system_window));
		}
#else /* defined (NOT_ACQUISITION_ONLY) */
		USE_PARAMETER(unemap_command_data);
#endif /* defined (NOT_ACQUISITION_ONLY) */
		DEALLOCATE(*unemap_command_data_address);
		*unemap_command_data_address = (struct Unemap_command_data *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Unemap_command_data).  Missing unemap_command_data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Unemap_command_data) */

int execute_command_unemap(struct Parse_state *state,
	void *prompt_void, void *unemap_command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 July 2002

DESCRIPTION :
Executes a UNEMAP command.
Pass the command stem string in <prompt_void>, for example "unemap" in Cmgui,
to allow the command prompt to remain.
==============================================================================*/
{
	char *prompt;
	int return_code;
	struct Option_table *option_table;
	struct Unemap_command_data *unemap_command_data;

	ENTER(execute_command_unemap);
	if (state && (unemap_command_data =
		(struct Unemap_command_data *)unemap_command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "open", NULL,
				unemap_command_data_void, execute_command_unemap_open);
			return_code = Option_table_parse(option_table, state);
		}
		else
		{
			if (prompt = (char *)prompt_void)
			{
				USE_PARAMETER(unemap_command_data);
				USE_PARAMETER(prompt);
#if defined (FUTURE_CODE)
				/*???RC I think the Execute_command should handle this, eg: */
				Execute_command_set_command_prompt(
					unemap_command_data->execute_command, prompt);
				/*set_command_prompt(prompt, command_data);*/
#endif /* defined (FUTURE_CODE) */
			}
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_unemap.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_unemap */

int unemap_execute_command(char *command_string, void *unemap_command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 July 2002

DESCRIPTION:
Execute a <command_string>.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(unemap_execute_command);
	if (unemap_command_data_void)
	{
		if (state = create_Parse_state(command_string))
		{
			option_table = CREATE(Option_table)();
			/* unemap */
			Option_table_add_entry(option_table, "unemap", NULL,
				unemap_command_data_void, execute_command_unemap);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_execute_command.  Could not create parse state");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_execute_command.  Missing unemap_command_data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* unemap_execute_command */

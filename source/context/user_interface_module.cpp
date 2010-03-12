/*******************************************************************************
FILE : User_intrefac_module.c

LAST MODIFIED : 7 January 2003

DESCRIPTION :
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
#if defined (MOTIF_USER_INTERFACE)
#include "io_devices/conversion.h"
#include "view/coord.h"
#endif
#include "time/time_keeper.h"
#include "comfile/comfile.h"
#include "command/command_window.h"
#include "command/cmiss.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/colour.h"
#include "graphics/cmiss_rendition.h"
#include "graphics/scene.h"
#include "interaction/interactive_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
}
#if defined (MOTIF_USER_INTERFACE)
typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
==============================================================================*/
{
	Pixel background_colour,foreground_colour;
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;
#endif /* defined (MOTIF_USER_INTERFACE) */

#if !defined (WIN32_USER_INTERFACE)
struct User_interface_module *User_interface_module_create(
	struct Context *context, int in_argc, const char *in_argv[])
#else
struct User_interface_module *User_interface_module_create(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance, 
	LPSTR command_line,int initial_main_window_state)
#endif
{
	struct User_interface_module  *UI_module = NULL;
	int return_code = 1;
	Cmiss_region *root_region;

#if defined (MOTIF_USER_INTERFACE)
	Display *display;
#define XmNbackgroundColour "backgroundColour"
#define XmCBackgroundColour "BackgroundColour"
#define XmNforegroundColour "foregroundColour"
#define XmCForegroundColour "ForegroundColour"
#define XmNexamplesDirectory "examplesDirectory"
#define XmCExamplesDirectory "ExamplesDirectory"
#define XmNstartupComfile "startupComfile"
#define XmCStartupComfile "StartupComfile"
#define XmNhelpDirectory "helpDirectory"
#define XmCHelpDirectory "HelpDirectory"
#define XmNhelpUrl "helpUrl"
#define XmCHelpUrl "HelpUrl"
	static XtResource resources[]=
	{
		{
			const_cast<char *>(XmNbackgroundColour),
			const_cast<char *>(XmCBackgroundColour),
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,background_colour),
			XmRString,
			const_cast<char *>("black")
		},
		{
			const_cast<char *>(XmNforegroundColour),
			const_cast<char *>(XmCForegroundColour),
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,foreground_colour),
			XmRString,
			const_cast<char *>("white")
		},
		{
			const_cast<char *>(XmNexamplesDirectory),
			const_cast<char *>(XmCExamplesDirectory),
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,examples_directory),
			XmRString,
			const_cast<char *>("")
		},
		{
			const_cast<char *>(XmNstartupComfile),
			const_cast<char *>(XmCStartupComfile),
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,startup_comfile),
			XmRImmediate,
			(XtPointer)0
		},
		{
			const_cast<char *>(XmNhelpDirectory),
			const_cast<char *>(XmCHelpDirectory),
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_directory),
			XmRString,
			const_cast<char *>("")
		},
		{
			const_cast<char *>(XmNhelpUrl),
			const_cast<char *>(XmCHelpUrl),
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_url),
			XmRString,
			const_cast<char *>("http://www.bioeng.auckland.ac.nz/cmiss/help/user_help.php")
		},
	};
	XColor rgb;
	User_settings user_settings;
#endif /* defined (MOTIF_USER_INTERFACE) */

	if (context && ALLOCATE(UI_module, struct User_interface_module, 1))
	{
		root_region = Cmiss_context_get_default_region(context);
#if defined (CELL)
		UI_module->cell_interface = NULL;
#endif /* defined (CELL) */
		UI_module->event_dispatcher = NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		UI_module->graphics_window_manager = NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
#if defined (MOTIF_USER_INTERFACE)
		UI_module->prompt_window = NULL;
		UI_module->projection_window = NULL;
#endif /* defined (MOTIF_USER_INTERFACE) */
#if defined (MOTIF_USER_INTERFACE)
		UI_module->select_tool = NULL;
		UI_module->curve_editor_dialog = NULL;
		UI_module->data_grabber_dialog = NULL;
		UI_module->grid_field_calculator_dialog = NULL; 
		UI_module->input_module_dialog  = NULL;
		UI_module->sync_2d_3d_dialog = NULL;
		UI_module->element_creator = NULL;
		UI_module->time_editor_dialog = NULL;
#endif /* defined (MOTIF_USER_INTERFACE) */
		UI_module->default_time_keeper = NULL;
		UI_module->user_interface = NULL;
		UI_module->emoter_slider_dialog = NULL;
#if defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		UI_module->comfile_window_manager = NULL;
		UI_module->data_viewer = NULL; 
		UI_module->node_viewer = NULL;
		UI_module->element_point_viewer = NULL;
		UI_module->material_editor_dialog = NULL;
		UI_module->scene_editor = NULL;
		UI_module->spectrum_editor_dialog = NULL;
#endif /* defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		UI_module->scene_viewer_package = NULL;
		/* graphics window manager.  Note there is no default window. */
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		UI_module->graphics_buffer_package = NULL;
		UI_module->interactive_tool_manager = NULL;
		UI_module->background_colour.red=(float)0;
		UI_module->background_colour.green=(float)0;
		UI_module->background_colour.blue=(float)0;
		UI_module->foreground_colour.red=(float)1;
		UI_module->foreground_colour.green=(float)1;
		UI_module->foreground_colour.blue=(float)1;
		UI_module->access_count = 1;

		UI_module->argc = in_argc;
		UI_module->argv = NULL;
		UI_module->cleanup_argc = in_argc;
		UI_module->cleanup_argv = NULL;
		if (0 < in_argc)
		{
			ALLOCATE(UI_module->argv, char *, in_argc);
			ALLOCATE(UI_module->cleanup_argv, char *, in_argc);
			for (int ai = 0; ai < in_argc; ai++)
			{
				UI_module->cleanup_argv[ai] = UI_module->argv[ai] =
					duplicate_string(in_argv[ai]);
			}
		}

		if (NULL != (UI_module->event_dispatcher = Cmiss_context_get_default_event_dispatcher(
									 context)))
		{
#if !defined (WIN32_USER_INTERFACE)
			if (NULL == (UI_module->user_interface = CREATE(User_interface)
					(&(UI_module->argc), UI_module->argv, UI_module->event_dispatcher, "Cmgui",
						"cmgui")))
			{
				display_message(ERROR_MESSAGE,"Could not create User interface");
				return_code=0;
			}
#else /* !defined (WIN32_USER_INTERFACE) */
			if (NULL == (UI_module->user_interface = CREATE(User_interface)
					(current_instance, previous_instance, command_line,
						initial_main_window_state, UI_module->event_dispatcher)))
			{
				display_message(ERROR_MESSAGE,"Could not create User interface");
				return_code=0;
			}
#endif /* !defined (WIN32_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not create Event dispatcher.");
			return_code = 0;
		}
#if defined (MOTIF_USER_INTERFACE)
		if (UI_module->user_interface)
		{
			/* retrieve application specific constants */
			display = User_interface_get_display(UI_module->user_interface);
			XtVaGetApplicationResources(User_interface_get_application_shell(
			   UI_module->user_interface),&user_settings,resources,
			   XtNumber(resources),NULL);
			/*???DB.  User settings should be divided among tools */
			/* retrieve the background rgb settings */
			rgb.pixel=user_settings.background_colour;
			XQueryColor(display,DefaultColormap(display,DefaultScreen(display)),&rgb);
			/*???DB.  Get rid of 65535 ? */
			UI_module->background_colour.red=(float)(rgb.red)/(float)65535;
			UI_module->background_colour.green=(float)(rgb.green)/(float)65535;
			UI_module->background_colour.blue=(float)(rgb.blue)/(float)65535;
			/* retrieve the foreground rgb settings */
			rgb.pixel=user_settings.foreground_colour;
			XQueryColor(display,DefaultColormap(display,DefaultScreen(display)),&rgb);
			/*???DB.  Get rid of 65535 ? */
			UI_module->foreground_colour.red=(float)(rgb.red)/(float)65535;
			UI_module->foreground_colour.green=(float)(rgb.green)/(float)65535;
			UI_module->foreground_colour.blue=(float)(rgb.blue)/(float)65535;
			if ((UI_module->foreground_colour.red ==
					 UI_module->background_colour.red) &&
				(UI_module->foreground_colour.green ==
					UI_module->background_colour.green) &&
				(UI_module->foreground_colour.blue ==
					UI_module->background_colour.blue))
			{
				UI_module->foreground_colour.red =
					1 - UI_module->background_colour.red;
				UI_module->foreground_colour.green =
					1 - UI_module->background_colour.green;
				UI_module->foreground_colour.blue =
					1 - UI_module->background_colour.blue;
			}
		}
#endif /* defined (MOTIF_USER_INTERFACE) */
#if defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		/* comfile window manager */
		UI_module->comfile_window_manager = CREATE(MANAGER(Comfile_window))();
#endif /* defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			UI_module->graphics_buffer_package = CREATE(Graphics_buffer_package)(
				UI_module->user_interface);
		}
		/* graphics window manager.  Note there is no default window. */
		UI_module->graphics_window_manager=CREATE(MANAGER(Graphics_window))();
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		UI_module->interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
#if defined (MOTIF_USER_INTERFACE)
		/* now set up the conversion routines */
		/*???DB.  Can this be put elsewhere ? */
		conversion_init();
		/* initialize the coordinate widget manager */
		/*???DB.  Still needs to be turned into a manager */
		coord_widget_init();
#endif /* defined (MOTIF_USER_INTERFACE) */
#if defined (SGI_MOVIE_FILE) && defined (MOTIF_USER_INTERFACE)
		UI_module->movie_graphics_manager = CREATE(MANAGER(Movie_graphics))();
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF_USER_INTERFACE) */
		if (UI_module->user_interface)
		{
			UI_module->default_time_keeper=ACCESS(Time_keeper)(
				CREATE(Time_keeper)("default", UI_module->event_dispatcher,
				UI_module->user_interface));
		}
		if (UI_module->user_interface)
		{
			/* set up image library */
#if defined (UNIX) /* switch (Operating_System) */
			Open_image_environment(*(UI_module->argv));
#elif defined (WIN32_USER_INTERFACE) /* switch (Operating_System) */
			/* SAB Passing a string to this function so that it
				starts up, should get the correct thing from
				the windows system */
			Open_image_environment("cmgui");
#endif /* switch (Operating_System) */
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			struct Cmiss_graphics_module *graphics_module = 
				Cmiss_context_get_default_graphics_module(context);
			if (graphics_module)
			{
				struct Light *default_light = 
					Cmiss_graphics_module_get_default_light(graphics_module);
				struct Light_model *default_light_model = 
					Cmiss_graphics_module_get_default_light_model(graphics_module);
				struct Scene *default_scene =
					Cmiss_graphics_module_get_default_scene(graphics_module);
				UI_module->scene_viewer_package = CREATE(Cmiss_scene_viewer_package)
					(UI_module->graphics_buffer_package,
						&UI_module->background_colour,
						UI_module->interactive_tool_manager,
						Cmiss_graphics_module_get_light_manager(graphics_module), default_light,
						Cmiss_graphics_module_get_light_model_manager(graphics_module), default_light_model,
						Cmiss_graphics_module_get_scene_manager(graphics_module), default_scene,
						Cmiss_graphics_module_get_texture_manager(graphics_module), 
						UI_module->user_interface);
				DEACCESS(Light_model)(&default_light_model);
				DEACCESS(Light)(&default_light);
				DEACCESS(Scene)(&default_scene);
			}
			Cmiss_graphics_module_destroy(&graphics_module);
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		Cmiss_region_destroy(&root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_module_create.  Invalid argument(s)");
	}

	return UI_module;
}

struct User_interface_module *User_interface_module_access(
	struct User_interface_module *UI_module)
{
	if (UI_module)
	{
		UI_module->access_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_module_access.  Invalid argument(s)");
	}

	return UI_module;
}

int User_interface_module_destroy(
	struct User_interface_module **UI_module_address)
{
	int return_code = 0;
	struct User_interface_module *UI_module  = NULL;

	if (NULL != (UI_module = *UI_module_address))
	{
		UI_module->access_count--;
		if (0 == UI_module->access_count)
		{
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (UI_module->scene_viewer_package)
			{
				DESTROY(Cmiss_scene_viewer_package)(&UI_module->scene_viewer_package);		
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			DESTROY(MANAGER(Graphics_window))(&UI_module->graphics_window_manager);
			/* Must destroy the graphics_buffer_package after the windows which use it */
			if (UI_module->graphics_buffer_package)
			{
				DESTROY(Graphics_buffer_package)(&UI_module->graphics_buffer_package);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			DESTROY(MANAGER(Interactive_tool))(&(UI_module->interactive_tool_manager));

#if defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			DESTROY(MANAGER(Comfile_window))(&UI_module->comfile_window_manager);
#endif /* defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
#if defined (SGI_MOVIE_FILE) && defined (MOTIF_USER_INTERFACE)
			DESTROY(MANAGER(Movie_graphics))(&UI_module->movie_graphics_manager);
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF_USER_INTERFACE) */
			if (UI_module->default_time_keeper)
				DEACCESS(Time_keeper)(&UI_module->default_time_keeper);
			if (UI_module->user_interface)
			{
				/* reset up messages */
				set_display_message_function(ERROR_MESSAGE,
					(Display_message_function *)NULL, NULL);
				set_display_message_function(INFORMATION_MESSAGE,
					(Display_message_function *)NULL, NULL);
				set_display_message_function(WARNING_MESSAGE,
					(Display_message_function *)NULL, NULL);
				/* close the user interface */
				DESTROY(User_interface)(&(UI_module->user_interface));
			}

			if (UI_module->cleanup_argv != NULL)
			{
				for (int ai = 0; ai < UI_module->cleanup_argc; ai++)
				{
					DEALLOCATE(UI_module->cleanup_argv[ai]);
				}
				DEALLOCATE(UI_module->cleanup_argv);
				DEALLOCATE(UI_module->argv);
			}
			DEALLOCATE(*UI_module_address);
		}
		*UI_module_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"User_interface_module_destroy.  Missing address of user interface module");
		return_code = 0;
	}
		
	return return_code;
}

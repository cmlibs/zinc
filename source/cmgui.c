/*******************************************************************************
FILE : cmgui.c

LAST MODIFIED : 5 July 2000

DESCRIPTION :
???DB.  Prototype main program for an application that uses the "cmgui tools".
	It is imagined that this will include - cmgui, emap and dfn.  Started as
	general/cmgui.c .
???DB.  Can all the #if defined (MOTIF) and #if defined (WINDOWS) be hidden in
	macros defined in user_interface.h ?
???DB.  global_database.h has been removed.  It was specific to cmgui, and
	couldn't be extended to a mix and match use of tools.  I see two possibilities
	1 Managers/globals are passed.  I am going to try this, but I'm not sure what
		happens with automatic creation of managers.
	2 Managers/globals have externs in the modules that need them.  I don't like
		this because it fixes the names and is less clear.
???DB.  Add an argument which causes the command tree to be written to stdout
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
/*SAB I have concatenated the correct version file for each version
  externally in the shell with cat #include "version.h"*/
#include "command/cmiss.h"
#include "command/command_window.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "element/element_creator.h"
#include "element/element_point_tool.h"
#include "element/element_point_viewer.h"
#include "element/element_tool.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_sample_texture.h"
#include "computed_field/computed_field_window_projection.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "finite_element/grid_field_calculator.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "general/debug.h"
#include "general/error_handler.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "graphics/environment_map.h"
#include "graphics/glyph.h"
#include "graphics/graphics_window.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/movie_graphics.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/transform_tool.h"
#include "graphics/volume_texture.h"
#include "interaction/interactive_tool.h"
#include "io_devices/conversion.h"
#include "node/node_tool.h"
#include "node/node_viewer.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#if defined (OLD_CODE)
#if defined (MOTIF)
#include "socket/socket.h"
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */
#if defined (SGI_MOVIE_FILE)
#include "three_d_drawing/movie_extensions.h"
#endif /* defined (SGI_MOVIE_FILE) */
#include "time/time_keeper.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "user_interface/filedir.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "curve/control_curve.h"
#if !defined (WINDOWS_DEV_FLAG)
#include "view/coord.h"
#endif /* !defined (WINDOWS_DEV_FLAG) */
#include "unemap/unemap_package.h"
#if defined (F90_INTERPRETER)
#include "command/f90_interpreter.h"
#else /* defined (F90_INTERPRETER) */
#if defined (PERL_INTERPRETER)
#include "command/perl_interpreter.h"
#endif /* defined (PERL_INTERPRETER) */
#endif /* defined (F90_INTERPRETER) */

/*
Global variables
----------------
*/
#if defined (OLD_CODE)
#if defined (MOTIF)
User_settings user_settings;
	/*???DB.  Temp ? */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

/*
Module types
------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF)
	Pixel background_colour,foreground_colour;
#endif /* defined (MOTIF) */
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;

/*
Module variables
----------------
*/
#if defined (OLD_CODE)
#if !defined (WINDOWS_DEV_FLAG)
/*???DB.  The plan is that these will all be passed */
/* environment maps */
static struct Environment_map
	*default_Environment_map=(struct Environment_map *)NULL;
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* defined (OLD_CODE) */

/*
Module functions
----------------
*/
static int display_error_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui error message.
==============================================================================*/
{
	int return_code;

	ENTER(display_error_message);
#if !defined (WINDOWS_DEV_FLAG)
	if (command_window_void)
	{
		write_command_window("ERROR: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
#endif /* !defined (WINDOWS_DEV_FLAG) */
	{
		printf("ERROR: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_error_message */

static int display_information_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui information message.
==============================================================================*/
{
	int return_code;

	ENTER(display_error_message);
#if !defined (WINDOWS_DEV_FLAG)
	if (command_window_void)
	{
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
	}
	else
#endif /* !defined (WINDOWS_DEV_FLAG) */
	{
		printf(message);
	}
	LEAVE;

	return (return_code);
} /* display_information_message */

static int display_warning_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui warning message.
???DB.  write_output is for the command_window - needs a better name.
==============================================================================*/
{
	int return_code;

	ENTER(display_warning_message);
#if !defined (WINDOWS_DEV_FLAG)
	if (command_window_void)
	{
		write_command_window("WARNING: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
#endif /* !defined (WINDOWS_DEV_FLAG) */
	{
		printf("WARNING: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_warning_message */

static int cmgui_execute_comfile(char *comfile_name,char *example_id,
	char *examples_directory,char *example_symbol,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes the comfile specified on the command line.
==============================================================================*/
{
#if defined (OLD_CODE)
	char *command_string,*temp_char,*temp_char_2;
	int count,i;
#endif /* defined (OLD_CODE) */
	int return_code;

	ENTER(cmgui_execute_comfile);
	return_code=0;
	if ((comfile_name||example_id)&&execute_command)
	{
		if (example_id)
		{
			if (examples_directory&&example_symbol)
			{
				/* set the examples directory */
				sprintf(global_temp_string,"set dir ");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string,"=");
				strcat(global_temp_string,example_id);
				Execute_command_execute_string(execute_command,global_temp_string);
				sprintf(global_temp_string,"open comfile ");
				if (comfile_name)
				{
					strcat(global_temp_string,comfile_name);
				}
				else
				{
					strcat(global_temp_string,"example_");
					strcat(global_temp_string,example_id);
				}
				strcat(global_temp_string,";");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string," execute");
				return_code=Execute_command_execute_string(execute_command,global_temp_string);
#if defined (OLD_CODE)
				/* strip the path */
				temp_char_2=comfile_name;
#if defined (UNIX)
				while (temp_char=strchr(temp_char_2,'/'))
				{
					temp_char_2=temp_char+1;
				}
#endif /* defined (UNIX) */
#if defined (VMS)
				if (temp_char=strchr(temp_char_2,']'))
				{
					temp_char_2=temp_char+1;
				}
#endif /* defined (VMS) */
				if (temp_char=strchr(temp_char_2,'_'))
				{
					temp_char++;
				}
				else
				{
					temp_char=temp_char_2;
				}
				count=0;
				while ((*(temp_char+count))&&('?'!= *(temp_char+count)))
				{
					count++;
				}
				if (count>0)
				{
					if (ALLOCATE(command_string,char,strlen(examples_directory)+
						strlen(example_symbol)+13+(count*(count+5))/2))
					{
						/* set the examples directory (back end) */
						strcpy(command_string,"set dir ");
						strcat(command_string,example_symbol);
						strcat(command_string,"=");
						temp_char_2=command_string+strlen(command_string);
						for (i=1;i<=count;i++)
						{
							strncpy(temp_char_2,temp_char,i);
							temp_char_2 += i;
							if (i<count)
							{
								*temp_char_2='/';
								temp_char_2++;
							}
						}
						*temp_char_2='\0';
						temp_char_2[count]='\0';
						Execute_command_execute_string(execute_command, command_string);
						/* open the comfile */
						temp_char_2=command_string;
						strcpy(temp_char_2,"example_");
						temp_char_2 += 8;
						strncpy(temp_char_2,temp_char,count);
						temp_char_2[count]='\0';
						sprintf(global_temp_string,"open comfile ");
						strcat(global_temp_string,command_string);
						strcat(global_temp_string,";");
						strcat(global_temp_string,example_symbol);
						strcat(global_temp_string," execute");
						Execute_command_execute_string(execute_command, global_temp_string);
						DEALLOCATE(command_string);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"cmgui_execute_comfile.  Insufficient memory");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"cmgui_execute_comfile.  Invalid example name");
				}
#endif /* defined (OLD_CODE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"cmgui_execute_comfile.  Missing examples_directory or example_symbol");
			}
		}
		else
		{
			/* open the command line comfile */
			sprintf(global_temp_string,"open comfile ");
			strcat(global_temp_string,comfile_name);
			strcat(global_temp_string," execute");
			return_code=Execute_command_execute_string(execute_command, global_temp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmgui_execute_comfile.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* cmgui_execute_comfile */

/*
Main program
------------
*/
#if defined (MOTIF)
int main(int argc,char *argv[])
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
	/* using WinMain as the entry point tells Windows that it is a gui and to use
		the graphics device interface functions for I/O */
	/*???DB.  WINDOWS a zero return code if WinMain does get into the message
		loop.  Other application interfaces may expect something else.  Should this
		failure code be #define'd ? */
	/*???DB. Win32 SDK says that don't have to call it WinMain */
#endif /* defined (WINDOWS) */
/*******************************************************************************
LAST MODIFIED : 21 June 2000

DESCRIPTION :
Main program for the CMISS Graphical User Interface
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*comfile_name,
		*example_id,*examples_directory,*version_command_id,version_id_string[100],
		*version_ptr,version_temp[20];
	float default_light_direction[3]={0.0,-0.5,-1.0};
	int
#if defined (WINDOWS)
		WINAPI
#endif /* defined (WINDOWS) */
		return_code;
	int batch_mode, command_list,no_display,non_random,start_cm,start_mycm;
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
#if defined (MOTIF)
	char *arg;
	int i;
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
			XmNbackgroundColour,
			XmCBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,background_colour),
			XmRString,
			"black"
		},
		{
			XmNforegroundColour,
			XmCForegroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,foreground_colour),
			XmRString,
			"white"
		},
		{
			XmNexamplesDirectory,
			XmCExamplesDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,examples_directory),
			XmRString,
			""
		},
		{
			XmNstartupComfile,
			XmCStartupComfile,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,startup_comfile),
			XmRImmediate,
			(XtPointer)0
		},
		{
			XmNhelpDirectory,
			XmCHelpDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_directory),
			XmRString,
			""
		},
		{
			XmNhelpUrl,
			XmCHelpUrl,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_url),
			XmRString,
			"http://www.esc.auckland.ac.nz/Groups/Bioengineering/CMISS/help/index_user.html"
		},
	};
#if defined (OLD_CODE)
/*???DB.  User settings should be divided among tools */
#define XmNprinterForegroundColour "printerForegroundColour"
#define XmCPrinterForegroundColour "PrinterForegroundColour"
#define XmNprinterBackgroundColour "printerBackgroundColour"
#define XmCPrinterBackgroundColour "PrinterBackgroundColour"
#define XmNprinterPageBottomMarginMm "printerPageBottomMarginMm"
#define XmCPrinterPageBottomMarginMm "PrinterPageBottomMarginMm"
#define XmNprinterPageHeightMm "printerPageHeightMm"
#define XmCPrinterPageHeightMm "PrinterPageHeightMm"
#define XmNprinterPageLeftMarginMm "printerPageLeftMarginMm"
#define XmCPrinterPageLeftMarginMm "PrinterPageLeftMarginMm"
#define XmNprinterPageRightMarginMm "printerPageRightMarginMm"
#define XmCPrinterPageRightMarginMm "PrinterPageRightMarginMm"
#define XmNprinterPageTopMarginMm "printerPageTopMarginMm"
#define XmCPrinterPageTopMarginMm "PrinterPageTopMarginMm"
#define XmNprinterPageWidthMm "printerPageWidthMm"
#define XmCPrinterPageWidthMm "PrinterPageWidthMm"
	static XtResource resources[]=
	{
		{
			XmNprinterForegroundColour,
			XmCPrinterForegroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,foreground_printer_colour),
			XmRString,
			"black"
		},
		{
			XmNprinterBackgroundColour,
			XmCPrinterBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,background_printer_colour),
			XmRString,
			"white"
		},
		{
			XmNprinterPageBottomMarginMm,
			XmCPrinterPageBottomMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,printer_page_bottom_margin_mm),
			XmRString,
			"25"
		},
		{
			XmNprinterPageHeightMm,
			XmCPrinterPageHeightMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,printer_page_height_mm),
			XmRString,
			"297"
		},
		{
			XmNprinterPageLeftMarginMm,
			XmCPrinterPageLeftMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,printer_page_left_margin_mm),
			XmRString,
			"10"
		},
		{
			XmNprinterPageRightMarginMm,
			XmCPrinterPageRightMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,printer_page_right_margin_mm),
			XmRString,
			"10"
		},
		{
			XmNprinterPageTopMarginMm,
			XmCPrinterPageTopMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,printer_page_top_margin_mm),
			XmRString,
			"25"
		},
		{
			XmNprinterPageWidthMm,
			XmCPrinterPageWidthMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,printer_page_width_mm),
			XmRString,
			"210"
		},
	};
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
#if defined (MOTIF)
/*???DB.  Need a setup routine for file I/O ? */
/*???DB.  Put in open_user_interface and always have ? */
	static MrmRegisterArg callbacks[]=
	{
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"open_file_and_write",(XtPointer)open_file_and_write}
	};
#if defined (OLD_CODE)
	static MrmRegisterArg identifiers[]=
	{
		{"widget_spacing",(XtPointer)NULL},
		{"menu_font",(XtPointer)NULL},
		{"heading_font",(XtPointer)NULL},
		{"normal_font",(XtPointer)NULL},
		{"normal_non_proportional_font",(XtPointer)NULL},
		{"button_font",(XtPointer)NULL},
		{"list_font",(XtPointer)NULL},
		{"small_font",(XtPointer)NULL}
	};
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
	struct CM_field_information cm_field_information;
	struct Cmiss_command_data command_data;
	struct Colour ambient_colour,default_colour;
	struct Colour no_interpolation_colour={0.65,0.65,0.65};
	struct Computed_field *computed_field;
	struct Command_window *command_window;
	struct Coordinate_system rect_coord_system,temp_coordinate_system;
	struct Execute_command *execute_command, *set_command;
	struct FE_field *fe_field;
	struct Graphical_material *default_selected_material;
	struct GT_object *glyph;
	struct MANAGER(Computed_field) *computed_field_manager;
#if !defined (WINDOWS_DEV_FLAG)
	struct Modifier_entry set_file_name_option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,NULL,NULL,set_file_name},
		{NULL,NULL,NULL,set_file_name}
	};
#endif /* !defined (WINDOWS_DEV_FLAG) */
	struct User_interface user_interface;
	User_settings user_settings;

#if defined (MOTIF)
	XColor rgb;
#endif /* defined (MOTIF) */

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	ENTER(WinMain);
#endif /* defined (WINDOWS) */
	return_code=1;
	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");

	/* check for command_list (needed because don't open_user_interface for
		command_list) */
	command_list=0;
	batch_mode = 0;
	no_display = 0;
	i=1;
	while ((i<argc)&&strcmp("-command_list",argv[i]))
	{
		i++;
	}
	if (i<argc)
	{
		command_list=1;
	}
	i=1;
	while ((i<argc)&&strncmp("-batch",argv[i],3))
	{
		i++;
	}
	if (i<argc)
	{
		batch_mode = 1;
	}
	i=1;
	while ((i<argc)&&strncmp("-no_display",argv[i],4))
	{
		i++;
	}
	if (i<argc)
	{
		no_display = 1;
	}

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
	create_interpreter(&status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

	/* initialize application specific global variables */
	execute_command = CREATE(Execute_command)(cmiss_execute_command,
		(void *)(&command_data));
	command_data.execute_command= execute_command;
	set_command = CREATE(Execute_command)(cmiss_set_command,
		(void *)(&command_data));
	command_data.set_command= set_command;
	if(no_display || command_list)
	{
		command_data.user_interface= (struct User_interface *)NULL;
	}
	else
	{
		user_interface.local_machine_info=(struct Machine_information *)NULL;
#if defined (MOTIF)
		user_interface.application_context=(XtAppContext)NULL;
		user_interface.application_name="cmgui";
		user_interface.application_shell=(Widget)NULL;
		user_interface.argc_address= &argc;
		user_interface.argv=argv;
		user_interface.class_name="Cmgui";
		user_interface.display=(Display *)NULL;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		user_interface.instance=current_instance;
		user_interface.main_window=(HWND)NULL;
		user_interface.main_window_state=initial_main_window_state;
		user_interface.command_line=command_line;
#endif /* defined (WINDOWS) */
		command_data.user_interface= &user_interface;
	}
#if !defined (WINDOWS_DEV_FLAG)
	command_data.control_curve_editor_dialog=(Widget)NULL;
	command_data.data_grabber_dialog=(Widget)NULL;
	command_data.sync_2d_3d_dialog=(Widget)NULL;
	command_data.emoter_slider_dialog=(Widget)NULL;
	command_data.grid_field_calculator_dialog=(Widget)NULL;
	command_data.input_module_dialog=(Widget)NULL;
	command_data.interactive_data_editor_dialog=(Widget)NULL;
	command_data.interactive_node_editor_dialog=(Widget)NULL;
	command_data.interactive_streamlines_dialog=(Widget)NULL;
	command_data.material_editor_dialog=(Widget)NULL;
	command_data.node_group_slider_dialog=(Widget)NULL;
	command_data.data_viewer=(struct Node_viewer *)NULL;
	command_data.node_viewer=(struct Node_viewer *)NULL;
	command_data.element_point_viewer=(struct Element_point_viewer *)NULL;
	command_data.transformation_editor_dialog=(Widget)NULL;
	command_data.prompt_window=(struct Prompt_window *)NULL;
	command_data.projection_window=(struct Projection_window *)NULL;
	command_data.element_group_editor_dialog=(Widget)NULL;
		/*???RC.  Temporary - should allow more than one */
	command_data.spectrum_editor_dialog=(Widget)NULL;
	command_data.time_editor_dialog=(Widget)NULL;
		/*???RC.  Temporary - should allow more than one */
#if defined (UNEMAP)
	command_data.unemap_system_window=(struct System_window *)NULL;
#endif /* defined (UNEMAP) */
#if defined (CELL)
	command_data.cell_window=(struct Cell_window *)NULL;
#endif /* defined (CELL) */
#if defined (MIRAGE)
	command_data.tracking_editor_dialog=(struct Tracking_editor_dialog *)NULL;
#if defined (OLD_CODE)
	command_data.mirage_movie=(struct Mirage_movie *)NULL;
	command_data.digitiser_window_manager=
		(struct MANAGER(Digitiser_window) *)NULL;
#endif /* defined (OLD_CODE) */
#endif /* defined (MIRAGE) */
	command_data.example_directory=(char *)NULL;
	(set_file_name_option_table[0]).user_data=
		&(command_data.example_directory);
	command_data.set_file_name_option_table=set_file_name_option_table;
	/* create the managers */
	/* light manager */
	command_data.default_light=(struct Light *)NULL;
	if (command_data.light_manager=CREATE(MANAGER(Light))())
	{
		if (command_data.default_light=CREATE(Light)("default"))
		{
			set_Light_type(command_data.default_light,INFINITE_LIGHT);
			default_colour.red=1.0;
			default_colour.green=1.0;
			default_colour.blue=1.0;		
			set_Light_colour(command_data.default_light,&default_colour); 
				
			set_Light_direction(command_data.default_light,default_light_direction);
			/*???DB.  Include default as part of manager ? */
			ACCESS(Light)(command_data.default_light);
			if (!ADD_OBJECT_TO_MANAGER(Light)(command_data.default_light,
				command_data.light_manager))
			{
				DEACCESS(Light)(&(command_data.default_light));
			}
		}
	}
	command_data.default_light_model=(struct Light_model *)NULL;
	if (command_data.light_model_manager=CREATE(MANAGER(Light_model))())
	{
		if (command_data.default_light_model=CREATE(Light_model)("default"))
		{
			ambient_colour.red=0.2;
			ambient_colour.green=0.2;
			ambient_colour.blue=0.2;
			Light_model_set_ambient(command_data.default_light_model,&ambient_colour);
			Light_model_set_side_mode(command_data.default_light_model,
				LIGHT_MODEL_TWO_SIDED);
			/*???DB.  Include default as part of manager ? */
			ACCESS(Light_model)(command_data.default_light_model);
			if (!ADD_OBJECT_TO_MANAGER(Light_model)(
				command_data.default_light_model,command_data.light_model_manager))
			{
				DEACCESS(Light_model)(&(command_data.default_light_model));
			}			
		}
	}
	/* environment map manager */
	command_data.environment_map_manager=CREATE(MANAGER(Environment_map))();
	/* texture manager */
	command_data.texture_manager=CREATE(MANAGER(Texture))();
	/* volume texture manager */
	command_data.volume_texture_manager=CREATE(MANAGER(VT_volume_texture))();
	/* graphical material manager */
	command_data.default_graphical_material=(struct Graphical_material *)NULL;
	if (command_data.graphical_material_manager=
		CREATE(MANAGER(Graphical_material))())
	{
		struct Colour colour;

		if (command_data.default_graphical_material=
			CREATE(Graphical_material)("default"))
		{
			/* ACCESS so can never be destroyed */
			ACCESS(Graphical_material)(command_data.default_graphical_material);
			if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
				command_data.default_graphical_material,
				command_data.graphical_material_manager))
			{
				DEACCESS(Graphical_material)(
					&(command_data.default_graphical_material));
			}
		}
		/* create material "default_selected" to be bright red for highlighting
			 selected graphics */
		if (default_selected_material=CREATE(Graphical_material)(
			"default_selected"))
		{
			colour.red=1.0;
			colour.green=0.0;
			colour.blue=0.0;
			Graphical_material_set_ambient(default_selected_material,&colour);
			Graphical_material_set_diffuse(default_selected_material,&colour);
			/* ACCESS so can never be destroyed */
			ACCESS(Graphical_material)(default_selected_material);
			if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(default_selected_material,
				command_data.graphical_material_manager))
			{
				DEACCESS(Graphical_material)(&default_selected_material);
			}
		}	
	}
	/* spectrum manager */
	command_data.default_spectrum=(struct Spectrum *)NULL;
	if (command_data.spectrum_manager=CREATE(MANAGER(Spectrum))())
	{
		if (command_data.default_spectrum=CREATE(Spectrum)("default"))
		{
			Spectrum_set_simple_type(command_data.default_spectrum,
				BLUE_TO_RED_SPECTRUM);
			Spectrum_set_minimum_and_maximum(command_data.default_spectrum,0,1);
			/* ACCESS so can never be destroyed */
			ACCESS(Spectrum)(command_data.default_spectrum);
			if (!ADD_OBJECT_TO_MANAGER(Spectrum)(command_data.default_spectrum,
				command_data.spectrum_manager))
			{
				DEACCESS(Spectrum)(&(command_data.default_spectrum));
			}
		}
	}
	/* FE_element_field_info manager */
		/*???DB.  To be done */
	all_FE_element_field_info=CREATE(LIST(FE_element_field_info))();
	/* FE_element_shape manager */
		/*???DB.  To be done */
	all_FE_element_shape=CREATE(LIST(FE_element_shape))();
	/* FE_field manager */
	command_data.fe_field_manager=CREATE(MANAGER(FE_field))();
#if defined (OLD_CODE)
	/*???RC.  all_FE_field is gone, replaced by above manager */
	all_FE_field=CREATE(LIST(FE_field))();
#endif /* defined (OLD_CODE) */
	/* FE_element manager */
#if defined (OLD_CODE)
		/*???DB.  In transistion */
	all_FE_element=CREATE(LIST(FE_element))();
#endif /* defined (OLD_CODE) */
	rect_coord_system.type = RECTANGULAR_CARTESIAN;

	command_data.control_curve_manager=CREATE(MANAGER(Control_curve))();

	command_data.basis_manager=CREATE(MANAGER(FE_basis))();
	command_data.element_manager=CREATE(MANAGER(FE_element))();
		/*???DB.  Also manages faces and lines */
	command_data.element_group_manager=CREATE(MANAGER(GROUP(FE_element)))();
	command_data.node_manager=CREATE(MANAGER(FE_node))();
	command_data.node_group_manager=CREATE(MANAGER(GROUP(FE_node)))();
	command_data.data_manager=CREATE(MANAGER(FE_node))();
	command_data.data_group_manager=CREATE(MANAGER(GROUP(FE_node)))();

	command_data.interactive_streamline_manager=
		CREATE(MANAGER(Interactive_streamline))();

	command_data.streampoint_list=(struct Streampoint *)NULL;
	/* create graphics object list */
		/*???RC.  Eventually want graphics object manager */
	command_data.graphics_object_list=CREATE(LIST(GT_object))();

	/* computed field manager and default computed fields zero, xi,
		 default_coordinate, etc. */
	/*???RC should the default computed fields be established in
		CREATE(Computed_field_package)? */
	if ((command_data.computed_field_package=CREATE(Computed_field_package)(
		command_data.fe_field_manager,command_data.element_manager,
		command_data.control_curve_manager))&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			command_data.computed_field_package)))
	{
		if (!((computed_field=CREATE(Computed_field)("cmiss_number"))&&
			Computed_field_set_coordinate_system(computed_field,
				&rect_coord_system)&&
			Computed_field_set_type_cmiss_number(computed_field)&&
			Computed_field_set_read_only(computed_field)&&
			ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
				computed_field_manager)))
		{
			DESTROY(Computed_field)(&computed_field);
		}
		if (!((computed_field=CREATE(Computed_field)("default_coordinate"))&&
			Computed_field_set_coordinate_system(computed_field,
				&rect_coord_system)&&
			Computed_field_set_type_default_coordinate(computed_field,
				computed_field_manager)&&
			Computed_field_set_read_only(computed_field)&&
			ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
				computed_field_manager)))
		{
			DESTROY(Computed_field)(&computed_field);
		}
		if (!((computed_field=CREATE(Computed_field)("xi"))&&
			Computed_field_set_coordinate_system(computed_field,
			&rect_coord_system)&&
			Computed_field_set_type_xi_coordinates(computed_field)&&
			Computed_field_set_read_only(computed_field)&&
			ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
				computed_field_manager)))
		{
			DESTROY(Computed_field)(&computed_field);
		}
	}
	/* create the grid_point_number field and add to FE_field_manager - 
		 wrapper Computed_field will automatically be made for it. Created here
		 as has special meaning for setting grid_points in Element_point_viewer */
	temp_coordinate_system.type = NOT_APPLICABLE;
	if (!((fe_field=CREATE(FE_field)())&&
		set_FE_field_name(fe_field,"grid_point_number")&&
		set_FE_field_value_type(fe_field,INT_VALUE)&&
		set_FE_field_number_of_components(fe_field,1)&&
		set_FE_field_type_general(fe_field)&&
		set_CM_field_information(&cm_field_information,CM_FIELD,(int *)NULL)&&
		set_FE_field_CM_field_information(fe_field,&cm_field_information)&&
		set_FE_field_coordinate_system(fe_field,&temp_coordinate_system)&&
		set_FE_field_component_name(fe_field,0,"grid_point_number")&&
		ADD_OBJECT_TO_MANAGER(FE_field)(fe_field,command_data.fe_field_manager)))
	{
		DESTROY(FE_field)(&fe_field);
	}
	/* create glyph list */
		/*???RC.  Eventually want glyph manager */
	if (command_data.glyph_list=CREATE(LIST(GT_object))())
	{
		/* add standard glyphs */
		if (glyph=make_glyph_arrow_line("arrow_line",0.25,0.125))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_arrow_solid("arrow_solid",12,2./3.,1./6.))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_axes("axes",0.1,0.025,0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cone("cone",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cross("cross"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder6",6))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder_hires",48))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sphere("diamond",4,2))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_line("line"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_point("point",g_POINT_MARKER,0))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sheet("sheet"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sphere("sphere",12,6))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sphere("sphere_hires",48,24))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
	}

	/* global list of selected objects */
	command_data.element_point_ranges_selection=
		CREATE(Element_point_ranges_selection)();
	command_data.element_selection=CREATE(FE_element_selection)();
	command_data.data_selection=CREATE(FE_node_selection)();
	command_data.node_selection=CREATE(FE_node_selection)();

	/* interactive_tool manager */
	command_data.interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();

	/* scene manager */
		/*???RC.   LOTS of managers need to be created before this */
	command_data.default_scene=(struct Scene *)NULL;
	if (command_data.scene_manager=CREATE(MANAGER(Scene))())
	{
		if (command_data.default_scene=CREATE(Scene)("default"))
		{
			Scene_enable_graphics(command_data.default_scene,command_data.glyph_list,
				command_data.graphical_material_manager,
				command_data.default_graphical_material,command_data.light_manager,
				command_data.spectrum_manager,command_data.default_spectrum,
				command_data.texture_manager);
			Scene_enable_interactive_streamlines(command_data.default_scene,
				command_data.interactive_streamline_manager);
			Scene_set_graphical_element_mode(command_data.default_scene,
				GRAPHICAL_ELEMENT_LINES,
				Computed_field_package_get_computed_field_manager(
				command_data.computed_field_package),command_data.element_manager,
				command_data.element_group_manager,command_data.fe_field_manager,
				command_data.node_manager,command_data.node_group_manager,
				command_data.data_manager,command_data.data_group_manager,
				command_data.element_point_ranges_selection,
				command_data.element_selection,command_data.node_selection,
				command_data.data_selection,command_data.user_interface);
			/*???RC.  May want to use functions to modify default_scene here */
			/* eg. to add model lights, etc. */
			/* ACCESS default so can never be destroyed */
			/*???RC.  Should be able to change: eg. gfx set default scene NAME */
			/*???DB.  Include default as part of manager ? */
			ACCESS(Scene)(command_data.default_scene);
			if (!ADD_OBJECT_TO_MANAGER(Scene)(command_data.default_scene,
				command_data.scene_manager))
			{
				DEACCESS(Scene)(&(command_data.default_scene));
			}
		}
	}
	/* graphics window manager.  Note there is no default window. */
	command_data.graphics_window_manager=CREATE(MANAGER(Graphics_window))();

	/* Add Computed_fields to the Computed_field_package */
	if (command_data.computed_field_package)
	{
		if (command_data.graphics_window_manager)
		{
			Computed_field_register_type_window_projection(
				command_data.computed_field_package, 
				command_data.graphics_window_manager);
		}
		if (command_data.texture_manager)
		{
			Computed_field_register_type_sample_texture(
				command_data.computed_field_package, 
				command_data.texture_manager);
		}
	}
#if defined (UNEMAP)	
	command_data.unemap_package = CREATE(Unemap_package)(
		command_data.fe_field_manager,command_data.element_group_manager,
		command_data.node_manager,command_data.data_group_manager,
		command_data.node_group_manager,command_data.basis_manager,
		command_data.element_manager,command_data.element_point_ranges_selection,
		command_data.element_selection,command_data.node_selection,
		command_data.data_selection,
		computed_field_manager,
		command_data.texture_manager,command_data.interactive_tool_manager,
		command_data.scene_manager,
		command_data.light_model_manager,command_data.light_manager,
		command_data.spectrum_manager,command_data.graphical_material_manager,
		command_data.data_manager,command_data.glyph_list,&no_interpolation_colour);	
	set_unemap_package_background_colour(command_data.unemap_package,
		&(command_data.background_colour));
	set_unemap_package_light(command_data.unemap_package,command_data.default_light);
	set_unemap_package_light_model(command_data.unemap_package,
		command_data.default_light_model);
	set_unemap_package_user_interface(command_data.unemap_package,
		command_data.user_interface);
	set_unemap_package_map_graphical_material(command_data.unemap_package,
		command_data.default_graphical_material);	
	set_unemap_package_computed_field_package(command_data.unemap_package,
		command_data.computed_field_package);
#endif /* defined (UNEMAP) */
#if defined (MIRAGE)
/*	command_data.digitiser_window_manager=CREATE(MANAGER(Digitiser_window))();*/
#endif /* defined (MIRAGE) */
	/* now set up the conversion routines */
		/*???DB.  Can this be put elsewhere ? */
	conversion_init();
	/* initialize the coordinate widget manager */
		/*???DB.  Still needs to be turned into a manager */
	coord_widget_init();
	command_data.command_window=(struct Command_window *)NULL;
#endif /* !defined (WINDOWS_DEV_FLAG) */
#if defined (SGI_MOVIE_FILE)
	command_data.movie_graphics_manager=CREATE(MANAGER(Movie_graphics))();
#endif /* defined (SGI_MOVIE_FILE) */
	/* parse the command line */
		/*???DB.  Is this the right place for this ? */
		/*???DB.  Can this be done with command_parser ? */
		/*???GMH.  We need to change to REAL argument parsing */
	/* flag to say randomise */
	non_random= -1;
	/* flag for starting cm */
	start_cm=0;
	/* flag for starting mycm */
	start_mycm=0;
	/* to over-ride all other example directory settings */
	examples_directory=(char *)NULL;
	/* back-end examples directory */
	cm_examples_directory=(char *)NULL;
	/* back-end parameters file */
	cm_parameters_file_name=(char *)NULL;
	/* the comfile is in the examples directory */
	example_id=(char *)NULL;
	/* set no command id supplied */
	version_command_id=(char *)NULL;

	if (!command_list)
	{
		if (no_display)
		{
			return_code = 1;
		}
		else
		{
			if (open_user_interface(&user_interface))
			{
				return_code = 1;
				/* get the name of the machine we are running on */
				if (!(user_interface.local_machine_info=CREATE(Machine_information)()))
				{
					display_message(WARNING_MESSAGE,
						"Could not determine local machine information");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open user interface");
				return_code=0;
			}
		}
	}
	command_data.transform_tool=CREATE(Transform_tool)(
		command_data.interactive_tool_manager);
	command_data.node_tool=CREATE(Node_tool)(
		command_data.interactive_tool_manager,
		command_data.node_manager,/*data_manager*/0,
		command_data.node_selection,
		command_data.computed_field_package);
	command_data.element_tool=CREATE(Element_tool)(
		command_data.interactive_tool_manager,
		command_data.element_selection);
	command_data.data_tool=CREATE(Node_tool)(
		command_data.interactive_tool_manager,
		command_data.data_manager,/*data_manager*/1,
		command_data.data_selection,
		command_data.computed_field_package);
	command_data.element_point_tool=CREATE(Element_point_tool)(
		command_data.interactive_tool_manager,
		command_data.element_point_ranges_selection);
	command_data.element_creator=(struct Element_creator *)NULL;

	if (return_code)
	{
		if (command_list)
		{
#if defined (MOTIF)
			command_data.examples_directory=(char *)NULL;
			command_data.cm_examples_directory=(char *)NULL;
			command_data.cm_parameters_file_name=(char *)NULL;
#endif /* defined (MOTIF) */
#if !defined (WINDOWS_DEV_FLAG)
			cmiss_execute_command("??",(void *)(&command_data));
#endif /* !defined (WINDOWS_DEV_FLAG) */
		}
		else
		{
#if defined (MOTIF)
#if defined (OLD_CODE)
			/*???DB.  Not needed ? */
			program_name=remove_before_first(remove_after_last(argv[0], '.'), ']');
#endif /* defined (OLD_CODE)*/
			i=1;
			/* the name of the comfile to be run on startup */
			comfile_name=(char *)NULL;
			while (return_code&&(i<argc))
			{
				arg=argv[i];
				if ('-'==arg[0])
				{
					switch (arg[1])
					{
						case 'c':
						{
							switch (arg[2])
							{
								case 'o':
								{
									/* write a list of the commands */
									command_list=1;
								} break;
								case 'm':
								{
									switch (arg[3])
									{
										case '\0':
										{
											start_cm=1;
										} break;
										case '_':
										{
											switch (arg[4])
											{
												case 'e':
												{
													/* back-end examples directory path */
													i++;
													if (i<argc)
													{
														if (cm_examples_directory)
														{
															DEALLOCATE(cm_examples_directory);
														}
														if (ALLOCATE(cm_examples_directory,char,
															strlen(argv[i])+1))
														{
															strcpy(cm_examples_directory,argv[i]);
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"main.  Insufficient memory for back-end examples directory path");
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												} break;
												case 'p':
												{
													/* cm parameters file name */
													i++;
													if (i<argc)
													{
														if (cm_parameters_file_name)
														{
															DEALLOCATE(cm_parameters_file_name);
														}
														if (ALLOCATE(cm_parameters_file_name,char,
															strlen(argv[i])+1))
														{
															strcpy(cm_parameters_file_name,argv[i]);
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"main.  Insufficient memory for parameters file name");
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												} break;
												default:
												{
													return_code=0;
												} break;
											}
										} break;
										default:
										{
											return_code=0;
										} break;
									}
								} break;
								default:
								{
									return_code=0;
								} break;
							}
						} break;
						case 'd':
						{
							/* skip the display name */
							i++;
						} break;
						case 'e':
						{
							switch (arg[2])
							{
								case 'p':
								{
									/* examples directory path */
									i++;
									if (i<argc)
									{
										if (examples_directory)
										{
											DEALLOCATE(examples_directory);
										}
										if (ALLOCATE(examples_directory,char,strlen(argv[i])+1))
										{
											strcpy(examples_directory,argv[i]);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"main.  Insufficient memory for examples directory path");
											return_code=0;
										}
									}
									else
									{
										return_code=0;
									}
								} break;
								case 'x':
								{
									/* example id */
									i++;
									if (i<argc)
									{
										if (example_id)
										{
											DEALLOCATE(example_id);
										}
										if (ALLOCATE(example_id,char,strlen(argv[i])+1))
										{
											strcpy(example_id,argv[i]);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"main.  Insufficient memory for example_id");
											return_code=0;
										}
									}
									else
									{
										return_code=0;
									}
								} break;
							}
						} break;
#if defined (OLD_CODE)
						case 'h':
						{
							if ('o'==arg[2])
							{
								i++;
								if (i<argc)
								{
									set_host(argv[i]);
								}
								else
								{
									return_code=0;
								}
							}
							else
							{
								return_code=0;
							}
						} break;
						case 'p':
						{
							i++;
							if (i<argc)
							{
								set_port(atoi(argv[i]));
							}
							else
							{
								return_code=0;
							}
						} break;
#endif /* defined (OLD_CODE) */
						case 'm':
						{
							start_mycm=1;
						} break;
						case 'r':
						{
							/* set a flag to say don't randomise */
							i++;
							if (i<argc)
							{
								non_random=atoi(argv[i]);
							}
							else
							{
								return_code=0;
							}
						} break;
						case 'i':
						{
							i++;
							if (i<argc)
							{
								version_command_id=argv[i];
							}
							else
							{
								return_code=0;
							}
						} break;
						case 'b':
						case 'n':
						{
						} break;
						default:
						{
							return_code=0;
						} break;
					}
				}
				else
				{
					if (comfile_name)
					{
						DEALLOCATE(comfile_name);
					}
					if (ALLOCATE(comfile_name,char,strlen(argv[i])+1))
					{
						strcpy(comfile_name,argv[i]);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"main.  Insufficient memory for comfile name");
						return_code=0;
					}
				}
				i++;
			}
#endif /* defined (MOTIF) */
			if(no_display)
			{
				command_data.default_time_keeper = (struct Time_keeper *)NULL;
			}
			else
			{
				command_data.default_time_keeper=ACCESS(Time_keeper)(
					CREATE(Time_keeper)("default", &user_interface));
				if(command_data.default_scene)
				{
					Scene_enable_time_behaviour(command_data.default_scene,
						command_data.default_time_keeper);
				}
#if defined (UNEMAP)
				set_unemap_package_time_keeper(command_data.unemap_package,
					command_data.default_time_keeper);
#endif
			}
			if (return_code)
			{
				/* initialize random number generator */
				if (-1==non_random)
				{
					/* randomise */
					srand(time(NULL));
					/*???DB.  time is not ANSI */
				}
				else
				{
					/* randomise using given seed */
					srand(non_random);
				}
				user_settings.startup_comfile = (char *)NULL;
				if (no_display)
				{
					command_data.background_colour.red=0.0;
					command_data.background_colour.green=0.0;
					command_data.background_colour.blue=0.0;
					command_data.foreground_colour.red=1.0;
					command_data.foreground_colour.green=1.0;
					command_data.foreground_colour.blue=1.0;
				}
				else
				{
					/* retrieve application specific constants */
#if defined (MOTIF)
					XtVaGetApplicationResources(user_interface.application_shell,
						&user_settings,resources,XtNumber(resources),NULL);
					/*???DB.  User settings should be divided among tools */
					/* retrieve the background rgb settings */
					rgb.pixel=user_settings.background_colour;
					XQueryColor(user_interface.display,DefaultColormap(
						user_interface.display,DefaultScreen(user_interface.display)),&rgb);
					/*???DB.  Get rid of 65535 ? */
					command_data.background_colour.red=(float)(rgb.red)/(float)65535;
					command_data.background_colour.green=(float)(rgb.green)/(float)65535;
					command_data.background_colour.blue=(float)(rgb.blue)/(float)65535;
					/* retrieve the foreground rgb settings */
					rgb.pixel=user_settings.foreground_colour;
					XQueryColor(user_interface.display,DefaultColormap(
						user_interface.display,DefaultScreen(user_interface.display)),&rgb);
					/*???DB.  Get rid of 65535 ? */
					command_data.foreground_colour.red=(float)(rgb.red)/(float)65535;
					command_data.foreground_colour.green=(float)(rgb.green)/(float)65535;
					command_data.foreground_colour.blue=(float)(rgb.blue)/(float)65535;
					if ((command_data.foreground_colour.red==
						command_data.background_colour.red)&&
						(command_data.foreground_colour.green==
							command_data.background_colour.green)&&
						(command_data.foreground_colour.blue==
							command_data.background_colour.blue))
					{
						command_data.foreground_colour.red=
							1-command_data.background_colour.red;
						command_data.foreground_colour.green=
							1-command_data.background_colour.green;
						command_data.foreground_colour.blue=
							1-command_data.background_colour.blue;
					}
				}
				/*???DB.  Temporary */
				sprintf(global_temp_string,
					"gfx modify material default alpha 1 ambient %g %g %g diffuse %g %g %g",
					command_data.foreground_colour.red,
					command_data.foreground_colour.green,
					command_data.foreground_colour.blue,
					command_data.foreground_colour.red,
					command_data.foreground_colour.green,
					command_data.foreground_colour.blue);
				cmiss_execute_command(global_temp_string,(void *)(&command_data));
				if (no_display || examples_directory)
				{
					command_data.examples_directory=examples_directory;
				}
				else
				{
					command_data.examples_directory=user_settings.examples_directory;
				}
				command_data.cm_examples_directory=cm_examples_directory;
				command_data.cm_parameters_file_name=cm_parameters_file_name;
				if(no_display)
				{
					command_data.help_directory=(char *)NULL;
					command_data.help_url=(char *)NULL;
				}
				else
				{
					command_data.help_directory=user_settings.help_directory;
					command_data.help_url=user_settings.help_url;
				}
#endif /* defined (MOTIF) */
#if defined (MOTIF)
				if (no_display)
				{
					return_code = 1;
				}
				else
				{
					/* register the callbacks in the global name table */
					if (MrmSUCCESS==MrmRegisterNames(callbacks,XtNumber(callbacks)))
					{
#if defined (OLD_CODE)
						identifiers[0].value=(XtPointer)5;
						identifiers[1].value=(XtPointer)XmFontListCreate(
							user_settings.menu_font,XmSTRING_DEFAULT_CHARSET);
						identifiers[2].value=(XtPointer)XmFontListCreate(
							user_settings.heading_font,XmSTRING_DEFAULT_CHARSET);
						identifiers[3].value=(XtPointer)XmFontListCreate(
							user_settings.normal_font,XmSTRING_DEFAULT_CHARSET);
						identifiers[4].value=(XtPointer)XmFontListCreate(
							user_settings.normal_non_proportional_font,
							XmSTRING_DEFAULT_CHARSET);
						identifiers[5].value=(XtPointer)XmFontListCreate(
							user_settings.button_font,XmSTRING_DEFAULT_CHARSET);
						identifiers[6].value=(XtPointer)XmFontListCreate(
							user_settings.list_font,XmSTRING_DEFAULT_CHARSET);
						identifiers[7].value=(XtPointer)XmFontListCreate(
							user_settings.small_font,XmSTRING_DEFAULT_CHARSET);
						/* register the identifiers in the global name table*/
						if (MrmSUCCESS==MrmRegisterNames(identifiers,XtNumber(identifiers)))
						{
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
							/* create the main window */
							/* construct the version ID string which is exported in the command
								windows version atom */
							strcpy(version_id_string,"cmiss*");
							/* version number */
							if (version_ptr=strstr(VERSION,"version "))
							{
								strncpy(version_temp,version_ptr+8,11);
								version_temp[11] = 0;
								strcat(version_id_string,version_temp);
							}
							strcat(version_id_string,"*");
							/* id from runtime */
							if (version_command_id)
							{
								strcat(version_id_string,version_command_id);
							}
							strcat(version_id_string,"*");
							/* link and runtime options */
							if ( start_mycm )
							{
								strcat(version_id_string, "mycm ");
							}
							else 
							{
								if (start_cm)
								{
									strcat(version_id_string, "cm ");
								}
							}
#if defined (UNEMAP)
							strcat(version_id_string,"unemap ");
#endif /* defined (UNEMAP) */
#if defined (MIRAGE)
							strcat(version_id_string,"mirage ");
#endif /* defined (MIRAGE) */
#if defined (INCLUDE_XVG)
							strcat(version_id_string,"xvg ");
#endif /* defined (INCLUDE_XVG) */

							if (command_window=CREATE(Command_window)(execute_command,
								&user_interface,version_id_string))
							{
								command_data.command_window=command_window;
								if (!batch_mode)
								{
									/* set up messages */
									set_display_message_function(ERROR_MESSAGE,
										display_error_message,command_window);
									set_display_message_function(INFORMATION_MESSAGE,
										display_information_message,command_window);
									set_display_message_function(WARNING_MESSAGE,
										display_warning_message,command_window);
								}

								XSetErrorHandler(x_error_handler);
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unable to create command window");
								return_code=0;
							}
#if defined (MOTIF)
#if defined (OLD_CODE)
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unable to register identifiers");
							return_code=0;
						}
#endif /* defined (OLD_CODE) */
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unable to register callbacks");
						return_code=0;
					}
#endif /* defined (MOTIF) */
				}
#if !defined (WINDOWS_DEV_FLAG)
				if(return_code)
				{
					if (start_cm||start_mycm)
					{
						sprintf(global_temp_string,"create cm");
						if (start_mycm)
						{
							strcat(global_temp_string," mycm");
						}
						if (cm_parameters_file_name)
						{
							strcat(global_temp_string," parameters ");
							strcat(global_temp_string,cm_parameters_file_name);
						}
						if (cm_examples_directory)
						{
							strcat(global_temp_string," examples_directory ");
							strcat(global_temp_string,cm_examples_directory);
						}
						/* start the back-end */
						cmiss_execute_command(global_temp_string,
							(void *)(&command_data));
					}
					if (user_settings.startup_comfile)
					{
						/* Can't get the startupComfile name without X at the moment */
						cmgui_execute_comfile(user_settings.startup_comfile, NULL,
							NULL, NULL, execute_command);
					}
					if (example_id||comfile_name)
					{
						/* open the command line comfile */
						cmgui_execute_comfile(comfile_name,example_id,
							command_data.examples_directory,
							CMGUI_EXAMPLE_DIRECTORY_SYMBOL,execute_command);
						DEALLOCATE(comfile_name);
						DEALLOCATE(example_id);
					}
#endif /* !defined (WINDOWS_DEV_FLAG) */
					/*							START_ERROR_HANDLING;*/
					switch (signal_code)
					{
#if !defined (WINDOWS)
						/*???DB.  SIGBUS is not POSIX */
						case SIGBUS:
						{
							printf("Bus error occurred\n");
							display_message(ERROR_MESSAGE,"Bus error occurred");
						} break;
#endif /* !defined (WINDOWS) */
						case SIGFPE:
						{
							printf("Floating point exception occurred\n");
							display_message(ERROR_MESSAGE,
								"Floating point exception occurred");
						} break;
						case SIGILL:
						{
							printf("Illegal instruction occurred\n");
							display_message(ERROR_MESSAGE,
								"Illegal instruction occurred");
						} break;
						case SIGSEGV:
						{
							printf("Invalid memory reference occurred\n");
							display_message(ERROR_MESSAGE,
								"Invalid memory reference occurred");
						} break;
					}
					if(!batch_mode)
					{
						/* DESTROY(Command_window)(&command_data.command_window); */
						/* user interface loop */						
						return_code=application_main_loop(&user_interface);						
						/*???DB.  Need better way to stop error handling because
						  application_main_loop is infinite.  Alternatively make sure that
						  application_main_loop is not infinite */
						/*							END_ERROR_HANDLING;*/
						/*???RC.  Need clean up routines, eg.
						  X3dThreeDDrawingCleanUp(display); */
						/* free application memory */
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"Usage :\n");
					display_message(INFORMATION_MESSAGE,
						"  cmgui\n");
					display_message(INFORMATION_MESSAGE,
						"    <-batch>                                batch mode\n");
					display_message(INFORMATION_MESSAGE,
						"    <-cm>                                   start cm on entry\n");
					display_message(INFORMATION_MESSAGE,
						"    <-cm_epath PATH_TO_EXAMPLES_DIRECTORY>  override the cm examples directory\n");
					display_message(INFORMATION_MESSAGE,
						"    <-id ID>                                an id for this instance\n");
					display_message(INFORMATION_MESSAGE,
						"    <-cm_parameters PARAMETER_FILE_NAME>    parameters for cm\n");
					display_message(INFORMATION_MESSAGE,
						"    <-command_list>                         write a list of the commands\n");
					display_message(INFORMATION_MESSAGE,
						"    <-display HOST:0>                       override host to display on\n");
					display_message(INFORMATION_MESSAGE,
						"    <-epath PATH_TO_EXAMPLES_DIRECTORY>     override the examples directory\n");
					display_message(INFORMATION_MESSAGE,
						"    <-example EXAMPLE_ID>                   command file is an example\n");
#if defined (OLD_CODE)
					display_message(INFORMATION_MESSAGE,
						"    <-host HOSTNAME>                        host that Cmiss is running on\n");
					display_message(INFORMATION_MESSAGE,
						"    <-port NUMBER>                          socket port number to use\n");
#endif /* defined (OLD_CODE) */
					display_message(INFORMATION_MESSAGE,
						"    <-mycm>                                 start mycm on entry\n");
					display_message(INFORMATION_MESSAGE,
						"    <-random NUMBER>                        random number seed (else random)\n");
					display_message(INFORMATION_MESSAGE,
						"    <COMMAND_FILE_NAME>                     execute the command file\n");
					return_code=0;
				}
				if(!batch_mode)
				{
#if defined (OLD_CODE)
					/* Close_user_interface is called by execute_command_quit and
						stops the main loop */
					/* close the user interface */
					close_user_interface(&user_interface);
					/*???DB.  Should this actually be inside the application and be used to
					  set a flag that terminates the main loop ? */
#endif /* defined (OLD_CODE) */
					DESTROY(Machine_information)(&user_interface.local_machine_info);
					DEACCESS(Time_keeper)(&command_data.default_time_keeper);
				}

#if defined (SGI_MOVIE_FILE)
				DESTROY(MANAGER(Movie_graphics))(&command_data.movie_graphics_manager);
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (UNEMAP)
				DESTROY(Unemap_package)(&command_data.unemap_package); 
#endif /* defined (UNEMAP) */

				/* viewers */
				if (command_data.data_viewer)
				{
					DESTROY(Node_viewer)(&(command_data.data_viewer));
				}
				if (command_data.node_viewer)
				{
					DESTROY(Node_viewer)(&(command_data.node_viewer));
				}
				if (command_data.element_point_viewer)
				{
					DESTROY(Element_point_viewer)(&(command_data.element_point_viewer));
				}

				DESTROY(MANAGER(Graphics_window))(
					&command_data.graphics_window_manager);

				if (command_data.element_creator)
				{
					DESTROY(Element_creator)(&command_data.element_creator);
				}
				/* destroy Interactive_tools and manager */
				DESTROY(Transform_tool)(&command_data.transform_tool);
				DESTROY(Element_point_tool)(&command_data.element_point_tool);
				DESTROY(Node_tool)(&command_data.data_tool);
				DESTROY(Element_tool)(&command_data.element_tool);
				DESTROY(Node_tool)(&command_data.node_tool);
				DESTROY(MANAGER(Interactive_tool))(
					&(command_data.interactive_tool_manager));

				DEACCESS(Scene)(&(command_data.default_scene));
				DESTROY(MANAGER(Scene))(&command_data.scene_manager);

				DESTROY(FE_node_selection)(&(command_data.data_selection));
				DESTROY(FE_node_selection)(&(command_data.node_selection));
				DESTROY(FE_element_selection)(&(command_data.element_selection));
				DESTROY(Element_point_ranges_selection)(
					&(command_data.element_point_ranges_selection));

				DESTROY(LIST(GT_object))(&command_data.graphics_object_list);
				DESTROY(LIST(GT_object))(&command_data.glyph_list);

				DESTROY(MANAGER(Interactive_streamline))
					(&command_data.interactive_streamline_manager);

				DESTROY(Computed_field_package)(&command_data.computed_field_package);

				DESTROY(MANAGER(FE_field))(&command_data.fe_field_manager);

				DESTROY(MANAGER(Control_curve))(&command_data.control_curve_manager);

				DESTROY(MANAGER(GROUP(FE_element)))(&command_data.element_group_manager);
				DESTROY(MANAGER(FE_element))(&command_data.element_manager);
				DESTROY(MANAGER(GROUP(FE_node)))(&command_data.node_group_manager);
				DESTROY(MANAGER(FE_node))(&command_data.node_manager);
				DESTROY(MANAGER(GROUP(FE_node)))(&command_data.data_group_manager);
				DESTROY(MANAGER(FE_node))(&command_data.data_manager);
				DESTROY(MANAGER(FE_basis))(&command_data.basis_manager);

				DESTROY_LIST(FE_element_field_info)(&all_FE_element_field_info);
				DESTROY_LIST(FE_element_shape)(&all_FE_element_shape);

				DEACCESS(Spectrum)(&(command_data.default_spectrum));
				DESTROY(MANAGER(Spectrum))(&command_data.spectrum_manager);
				DEACCESS(Graphical_material)(&(command_data.default_graphical_material));			
				DEACCESS(Graphical_material)(&(default_selected_material));			
				DESTROY(MANAGER(Graphical_material))(&command_data.graphical_material_manager);

				DESTROY(MANAGER(VT_volume_texture))(&command_data.volume_texture_manager);
				DESTROY(MANAGER(Texture))(&command_data.texture_manager);
				DESTROY(MANAGER(Environment_map))(&command_data.environment_map_manager);

				DEACCESS(Light_model)(&(command_data.default_light_model));
				DESTROY(MANAGER(Light_model))(&command_data.light_model_manager);
				DEACCESS(Light)(&(command_data.default_light));
				DESTROY(MANAGER(Light))(&command_data.light_manager);

				if (examples_directory)
				{
					DEALLOCATE(examples_directory);
				}
				if (command_data.example_directory)
				{
					DEALLOCATE(command_data.example_directory);
				}

				coord_widget_finish();
				destroy_assign_variable_list();

				DESTROY(Execute_command)(&execute_command);
				DESTROY(Execute_command)(&set_command);

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
				destroy_interpreter(&status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

		

				/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
					on.  When MEMORY_CHECKING is off this function does nothing */
				list_memory(/*count_number*/1, /*show_pointers*/0, /*increment_counter*/0, /*show_structures*/1);
			}
		}
	}
	LEAVE;

	return (return_code);
} /* main */

/*******************************************************************************
FILE : cell.c

LAST MODIFIED : 18 August 1999

DESCRIPTION :
Main program for cell.  Based on unemap.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/error_handler.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "cell/cell_window.h"
#include "variable/variable_editor_dialog.h"

#if defined (MOTIF)
/*???DB.  Until can remove CMISS link from user_interface */
#include "link/cmiss.h"
struct CMISS_connection *CMISS = (struct CMISS_connection *)NULL;

int DESTROY(CMISS_connection)(struct CMISS_connection **connection_address)
/*******************************************************************************
LAST MODIFIED : 11 April 1997

DESCRIPTION :
Dummy
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(CMISS_connection));
	return_code=0;
	LEAVE;

	return (return_code);
} /* DESTROY(CMISS_connection) */

int CMISS_connection_update(struct CMISS_connection *connection)
/*******************************************************************************
LAST MODIFIED : 11 April 1997

DESCRIPTION :
Dummy
==============================================================================*/
{
	int return_code;

	ENTER(CMISS_connection_update);
	return_code=0;
	LEAVE;

	return (return_code);
} /* CMISS_connection_update */
#endif /* defined (MOTIF) */

/*
Module constants
----------------
*/
#define CHARSET XmSTRING_DEFAULT_CHARSET
#define MAX_NUMBER_MESSAGES 10

/*
Module variables
----------------
*/
static struct Cell_window *cell = (struct Cell_window *)NULL;

/*
Module types
------------
*/
#if defined (MOTIF)
typedef struct
/*******************************************************************************
LAST MODIFIED : 11 April 1997

DESCRIPTION :
==============================================================================*/
{
	Pixel background_colour,foreground_colour;
} User_settings;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static int display_error_message(char *message)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Display a Cell error message.
==============================================================================*/
{
	int return_code;

  ENTER(display_error_message);
  write_cell_window("ERROR: ",cell);
  return_code = write_cell_window(message,cell);
  write_cell_window("\n",cell);
  if (cell && cell->output_file)
  {
    fprintf(cell->output_file,"ERROR: ");
    fprintf(cell->output_file,"%s",message);
    fprintf(cell->output_file,"\n");
  }
  LEAVE;
  return (return_code);
} /* display_error_message */

static int display_information_message(char *message)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Display a Cell information message.
==============================================================================*/
{
	int return_code;

  ENTER(display_error_message);
  return_code = write_cell_window(message,cell);
  if (cell && cell->output_file)
  {
    fprintf(cell->output_file,message);
  }
  LEAVE;
  return (return_code);
} /* display_information_message */

static int display_warning_message(char *message)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Display a Cell warning message.
==============================================================================*/
{
	int return_code;

  ENTER(display_warning_message);
  write_cell_window("Warning: ",cell);
  return_code = write_cell_window(message,cell);
  write_cell_window("\n",cell);
  if (cell && cell->output_file)
  {
    fprintf(cell->output_file,"Warning: ");
    fprintf(cell->output_file,"%s",message);
    fprintf(cell->output_file,"\n");
  }
  LEAVE;
  return (return_code);
} /* display_warning_message */

#if defined (MOTIF)
static void exit_cell(Widget widget,XtPointer user_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 April 1998

DESCRIPTION :
Exits cell
==============================================================================*/
{
  if (cell && cell->output_file)
  {
    fclose(cell->output_file);
  }
	exit(0);
} /* exit_cell */
#endif /* defined (MOTIF) */

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
LAST MODIFIED : 26 May 1998

DESCRIPTION :
Main program for cell
==============================================================================*/
{
#if defined (MOTIF)
	Dimension window_height,window_width;
#endif /* defined (MOTIF) */
	int i,return_code;
	struct User_interface user_interface;
#if defined (MOTIF)
#define XmNbackgroundColour "backgroundColour"
#define XmCBackgroundColour "BackgroundColour"
#define XmNforegroundColour "foregroundColour"
#define XmCForegroundColour "ForegroundColour"
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
	};
	User_settings user_settings;
#endif /* defined (MOTIF) */
  char *filename;
  struct MANAGER(Time_variable) *time_variable_manager;
  struct Time_variable *default_time_variable;

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	ENTER(WinMain);
#endif /* defined (WINDOWS) */
	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");
	/* open the user interface */
#if defined (MOTIF)
	user_interface.application_context=(XtAppContext)NULL;
	user_interface.application_name="cell";
	user_interface.application_shell=(Widget)NULL;
	user_interface.argc_address= &argc;
	user_interface.argv=argv;
	user_interface.class_name="Cell";
	user_interface.display=(Display *)NULL;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	user_interface.instance=current_instance;
	user_interface.main_window=(HWND)NULL;
	user_interface.main_window_state=initial_main_window_state;
	user_interface.command_line=command_line;
#endif /* defined (WINDOWS) */
	if (open_user_interface(&user_interface))
	{
		/* set up messages */
    set_display_message_function(ERROR_MESSAGE,display_error_message);
    set_display_message_function(INFORMATION_MESSAGE,
			display_information_message);
		set_display_message_function(WARNING_MESSAGE,display_warning_message);

    /* set the filename for display_message output */
    if (argc > 1)
    {
      filename = argv[1];
    }
    else
    {
      filename = (char *)NULL;
    }
    /* initialise the time variable stuff */
    all_FE_element_field_info=CREATE_LIST(FE_element_field_info)();
    all_FE_element_shape=CREATE_LIST(FE_element_shape)();
    all_FE_field=CREATE_LIST(FE_field)();
    if (time_variable_manager = CREATE_MANAGER(Time_variable)())
    {
      /* create single component zero variable as default */
      if (default_time_variable = CREATE(Time_variable)("default",
        LINEAR_LAGRANGE,1))
      {
        /* ACCESS so can never be destroyed */
        ACCESS(Time_variable)(default_time_variable);
        if (!ADD_OBJECT_TO_MANAGER(Time_variable)(
          default_time_variable,
          time_variable_manager))
        {
          DEACCESS(Time_variable)(&(default_time_variable));
        }
      }
    }
    /* create the cell window */
    if (cell = create_Cell_window(&user_interface,filename,
      time_variable_manager,default_time_variable))
    {
      /* display the version */
      display_message(INFORMATION_MESSAGE, VERSION "\n");
      display_message(INFORMATION_MESSAGE,
        "Cell window successfully created\n");
      return_code=application_main_loop(&user_interface);
    }
    else
    {
      display_message(ERROR_MESSAGE,"main. "
        "Unable to create Cell window");
    }
#if defined (OLD_CODE)
    if (argc > 1)
    {
      return_code = go(argc,argv);
    }
    else
    {
      display_message(ERROR_MESSAGE,"No input file name specified");
    }
#endif /* defined (OLD_CODE) */
    close_user_interface(&user_interface);
    /*???DB.  Should this actually be inside the application and be
      used to set a flag that terminates the main loop ? */
  }
	else
	{
		display_message(ERROR_MESSAGE,"Could not open user interface");
		return_code=0;
	}
  LEAVE;

  return (return_code);
} /* main */

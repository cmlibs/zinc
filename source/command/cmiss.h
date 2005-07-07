/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 16 September 2004

DESCRIPTION :
Functions and types for executing cmiss commands.

This should only be included in cmgui.c and command/cmiss.c
==============================================================================*/
#if !defined (COMMAND_CMISS_H)
#define COMMAND_CMISS_H

#include "command/command.h"
#include "general/io_stream.h"
#include "region/cmiss_region.h"
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Global types
------------
*/
struct Cmiss_command_data;
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Shifted the Cmiss_command_data to be internal to cmiss.c
==============================================================================*/

/*
Global functions
----------------
*/

int cmiss_execute_command(char *command_string,void *command_data);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Execute a <command_string>.
==============================================================================*/

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
void execute_command(char *command_string,void *command_data_void, int *quit,
  int *error);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION:
==============================================================================*/
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing or entering. If there is no command_window, does nothing.
==============================================================================*/

#if !defined (WIN32_USER_INTERFACE)
struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[],
	char *version_string);
#else /* !defined (WIN32_USER_INTERFACE) */
struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[],
	char *version_string, HINSTANCE current_instance, 
        HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state);
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/

int Cmiss_command_data_main_loop(struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
==============================================================================*/

struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/

struct Time_keeper *Cmiss_command_data_get_default_time_keeper(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Returns the default time_keeper from the <command_data>.
==============================================================================*/

struct Execute_command *Cmiss_command_data_get_execute_command(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 28 May 2003

DESCRIPTION :
Returns the execute command structure from the <command_data>, useful for 
executing cmiss commands from C.
==============================================================================*/

struct IO_stream_package *Cmiss_command_data_get_IO_stream_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the io_stream_package structure from the <command_data>
==============================================================================*/

struct Cmiss_fdio_package* Cmiss_command_data_get_fdio_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
Gets an Fdio_package for this <command_data>
==============================================================================*/

struct Cmiss_idle_package* Cmiss_command_data_get_idle_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Gets an Idle_package for this <command_data>
==============================================================================*/

struct Cmiss_timer_package* Cmiss_command_data_get_timer_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Gets a Timer_package for this <command_data>
==============================================================================*/

struct FE_element_selection *Cmiss_command_data_get_element_selection(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Returns the selected_element object from the <command_data>.
==============================================================================*/

struct FE_node_selection *Cmiss_command_data_get_node_selection(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Returns the selected_node object from the <command_data>.
==============================================================================*/
#endif /* !defined (COMMAND_CMISS_H) */

/*******************************************************************************
FILE : cmgui.c

LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/
/*SAB I have concatenated the correct version file for each version
  externally in the shell with cat #include "version.h"*/
#include "command/cmiss.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

#if !defined (WIN32_USER_INTERFACE)
int main(int argc,char *argv[])
#else /* !defined (WIN32_USER_INTERFACE) */
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
	/* using WinMain as the entry point tells Windows that it is a gui and to use
		the graphics device interface functions for I/O */
	/*???DB.  WINDOWS a zero return code if WinMain does get into the message
		loop.  Other application interfaces may expect something else.  Should this
		failure code be #define'd ? */
	/*???DB. Win32 SDK says that don't have to call it WinMain */
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Main program for the CMISS Graphical User Interface
==============================================================================*/
{
	int return_code;
#if defined (WIN32_USER_INTERFACE)
	int argc = 1;
	char *argv[] = {"cmgui_win32"};
#endif /* defined (WIN32_USER_INTERFACE) */
	struct Cmiss_command_data *command_data;

#if !defined (WIN32_USER_INTERFACE)
	ENTER(main);
#else /* !defined (WIN32_USER_INTERFACE) */
	ENTER(WinMain);
#endif /* !defined (WIN32_USER_INTERFACE) */

	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");

	if (command_data = CREATE(Cmiss_command_data)(argc, argv, VERSION))
	{
		Cmiss_command_data_main_loop(command_data);
		DESTROY(Cmiss_command_data)(&command_data);
		return_code = 0;
	}
	else
	{
		return_code = 1;
	}
	LEAVE;

	return (return_code);
} /* main */

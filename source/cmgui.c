/*******************************************************************************
FILE : cmgui.c

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
/*SAB I have concatenated the correct version file for each version
  externally in the shell with cat #include "version.h"*/
#include "command/cmiss.h"
#include "general/debug.h"
#include "user_interface/message.h"
#if defined (UNEMAP)
#include "unemap_application/unemap_command.h"
#include "command/cmiss_unemap_link.h"
#endif /* defined (UNEMAP) */

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
	int argc = 1, i;
	char **argv, *p, *q;
#endif /* defined (WIN32_USER_INTERFACE) */
	struct Cmiss_command_data *command_data;

#if !defined (WIN32_USER_INTERFACE)
	ENTER(main);
#else /* !defined (WIN32_USER_INTERFACE) */
	ENTER(WinMain);

	for (p = command_line; p != NULL && *p != 0;)
	{
		p = strchr(p, ' ');
		if (p != NULL)
			p++;
		argc++;
	}

	argv = malloc(sizeof(*argv) * argc);

	argv[0] = "cmgui";

	for (i = 1, p = command_line; p != NULL && *p != 0;)
	{
		q = strchr(p, ' ');
		if (q != NULL)
			*q++ = 0;
		if (p != NULL)
			argv[i++] = p;
		p = q;
	}
#endif /* !defined (WIN32_USER_INTERFACE) */

	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");

#if !defined (WIN32_USER_INTERFACE)
	if (command_data = CREATE(Cmiss_command_data)(argc, argv, VERSION))
#else /* !defined (WIN32_USER_INTERFACE) */
	if (command_data = CREATE(Cmiss_command_data)(argc, argv, VERSION, 
                           current_instance, previous_instance, command_line, initial_main_window_state))
#endif /* !defined (WIN32_USER_INTERFACE) */
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

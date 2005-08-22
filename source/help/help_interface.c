/*******************************************************************************
FILE : help_interface.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
Interface routines for CMISS commands to work with the help window.
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
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/WinUtil.h>
#else /* defined (NETSCAPE_HELP) */
#if !defined (HYPERTEXT_HELP)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/TextF.h>
#endif
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

#include "general/debug.h"
#include "general/mystring.h"
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
#include "command/command.h"
#else /* defined (NETSCAPE_HELP) */
#include "help/help_window.h"
#if defined (HYPERTEXT_HELP)
#include "command/command.h"
#include "hypertext_help/hthelp.h"
#else
#include "help/help_work.h"
#endif
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */
#include "help/help_interface.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
#define MOZILLA_VERSION_PROPERTY "_MOZILLA_VERSION"
#define MOZILLA_LOCK_PROPERTY "_MOZILLA_LOCK"
#define MOZILLA_COMMAND_PROPERTY "_MOZILLA_COMMAND"
#define MOZILLA_RESPONSE_PROPERTY "_MOZILLA_RESPONSE"
#define WINDOW_NAME "WM_NAME"
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

/*
Module variables
----------------
*/
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
static Atom XA_MOZILLA_VERSION=0;
static Atom XA_MOZILLA_LOCK=0;
static Atom XA_MOZILLA_COMMAND=0;
static Atom XA_MOZILLA_RESPONSE=0;
static Atom WM_NAME=0;

static char *lock_data=(char *)NULL;
#endif /* defined (NETSCAPE_HELP) */

#if defined (HYPERTEXT_HELP)
char *examples_directory=(char *)NULL;
struct Execute_command *help_execute_command=(struct Execute_command *)NULL;
	/*??DB.  Not good, but can't do otherwise without changing HYPERTEXT_HELP */
#else
struct Help_window *help_window_structure=(struct Help_window *)NULL;
#endif
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (MOTIF)
#if defined (HYPERTEXT_HELP)
static void example_url_callback(char *example_string)
/*******************************************************************************
LAST MODIFIED : 4 March 1997

DESCRIPTION :
Callback for when the "Load example" button is clicked in the help.  It receives
the <example_string>, parses it and opens the appropriate comfile.
???DB.  UNIX only at present.
==============================================================================*/
{
	char *command_string,*temp_char,*temp_char_2;
	int count,i;

	ENTER(example_url_callback);
	if (example_string&&(examples_directory)&&help_execute_command)
	{
		if (!strncmp(example_string,"example:",8))
		{
			if (temp_char=strchr(example_string,'_'))
			{
				temp_char++;
			}
			else
			{
				temp_char=example_string+8;
			}
			count=0;
			while ((*(temp_char+count))&&('?'!= *(temp_char+count)))
			{
				count++;
			}
			if (count>0)
			{
				if (ALLOCATE(command_string,char,
					count+strlen(CMGUI_EXAMPLE_DIRECTORY_SYMBOL)+20))
				{
					strcpy(command_string,"open comfile ");
					strcat(command_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
					strcat(command_string," name ");
					*(command_string+(strlen(command_string)+count))='\0';
					strncpy(command_string+strlen(command_string),temp_char,count);
					(*(help_execute_command->function))(command_string,
						help_execute_command->data);
					DEALLOCATE(command_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"example_url_callback.  Insufficient memory");
				}
#if defined (OLD_CODE)
				if (ALLOCATE(command_string,char,
					strlen(examples_directory)+22+(count*(count+5))/2))
				{
					/* set the examples directory (back end) */
					strcpy(command_string,"set dir doc=");
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
					(*(help_execute_command->function))(command_string,
						help_execute_command->data);
					/* open the comfile (front end) */
					strcpy(command_string,"open comfile ");
					strcat(command_string,examples_directory);
#if defined (UNIX)
					temp_char_2=command_string+strlen(command_string);
#endif
#if defined (VMS)
					temp_char_2=command_string+(strlen(command_string)-1);
#endif
					for (i=1;i<=count;i++)
					{
#if defined (VMS)
						*temp_char_2='.';
						temp_char_2++;
#endif
						strncpy(temp_char_2,temp_char,i);
						temp_char_2 += i;
#if defined (UNIX)
						*temp_char_2='/';
						temp_char_2++;
#endif
					}
#if defined (VMS)
					*temp_char_2=']';
					temp_char_2++;
#endif
					strcpy(temp_char_2,"example_");
					temp_char_2 += 8;
					strncpy(temp_char_2,temp_char,count);
					temp_char_2[count]='\0';
					(*(help_execute_command->function))(command_string,
						help_execute_command->data);
					DEALLOCATE(command_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"example_url_callback.  Insufficient memory");
				}
#endif /* defined (OLD_CODE) */
			}
		}
	}
	LEAVE;
} /* example_url_callback */
#endif
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
static Window get_netscape_window(Display *display)
/*******************************************************************************
LAST MODIFIED : 30 January 1998

DESCRIPTION :
Determines if there is a netscape on the <display>.
==============================================================================*/
{
	Atom property_type;
	float current_version_number, greatest_version_number;
	int data_unit, navigator_window;
	unsigned char *version, *name;
	unsigned int i,number_of_child_windows;
	unsigned long bytes_left,number_of_units;
	Window child_window,*child_windows,netscape_window,parent_window,
		root_window_1,root_window_2;

	netscape_window=(Window)NULL;
	greatest_version_number = -1.0;
	if (display)
	{
		root_window_1=RootWindowOfScreen(DefaultScreenOfDisplay(display));
		if (XQueryTree(display,root_window_1,&root_window_2,&parent_window,
			&child_windows,&number_of_child_windows))
		{
			if (child_windows&&(0<number_of_child_windows))
			{
				i=0;
				while (i<number_of_child_windows)
				{
					version=(unsigned char *)NULL;
					child_window=XmuClientWindow(display,child_windows[i]);
						/*???DB.  XmuClientWindow is required.  Don't know why */
					if (Success==XGetWindowProperty(display,child_window,
						XA_MOZILLA_VERSION,0,(65536/sizeof(long)),False,XA_STRING,
						&property_type,&data_unit,&number_of_units,&bytes_left,&version))
					{
						if (None!=property_type)
						{ 
							/* Try and get a navigator client rather than a mail window or
								address book, annoying dependence on window name */
							navigator_window = 0;
							if (Success==XGetWindowProperty(display,child_window,
								WM_NAME,0,(65536/sizeof(long)),False,XA_STRING,
								&property_type,&data_unit,&number_of_units,&bytes_left,&name))
							{
								if(!strncmp((char *)name, "Netscape:", 9))
								{
									navigator_window = 1;
								}
							}
							if (version)
							{
								if(!sscanf((char *)version, "%f", &current_version_number))
								{
									current_version_number = 0.0;
								}
								if(current_version_number > greatest_version_number)
								{
									greatest_version_number = current_version_number;
									netscape_window=child_window;
								}
								if((current_version_number == greatest_version_number)
									&& navigator_window)
								{
									netscape_window=child_window;										
								}
							}
						}
						if (version)
						{
							XFree(version);
						}
					}
					i++;
				}
			}
			XFree(child_windows);
		}
	}

	return (netscape_window);
} /* get_cmiss_netscape_window */
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
static int lock_netscape(Display *display,Window netscape_window)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Locks netscape so that other processes cannot interact with it.
==============================================================================*/
{
	Atom property_type;
	char locked;
	int data_unit,result,return_code;
	unsigned char *data;
	unsigned long bytes_left,number_of_units;
	XEvent event;
#if defined (DEBUG)
	/*???debug */
	char out_string[180];
#endif /* defined (DEBUG) */

	return_code=0;
	/* check arguments */
	if (display&&netscape_window)
	{
		if (!lock_data)
		{
			if (ALLOCATE(lock_data,char,255))
			{
				sprintf(lock_data,"pid%x@",(int)getpid());
				if (gethostname(lock_data+strlen(lock_data),100))
				{
					free(lock_data);
					lock_data=(char *)NULL;
				}
			}
		}
#if defined (DEBUG)
		/*???debug */
		sprintf(out_string,"echo \"lock_data %s\"",lock_data);
		system(out_string);
#endif /* defined (DEBUG) */
		if (lock_data)
		{
			return_code=1;
			locked=0;
			do
			{
				XGrabServer(display);
				result=XGetWindowProperty(display,netscape_window,XA_MOZILLA_LOCK,0,
					(65536/sizeof(long)),True,XA_STRING,&property_type,&data_unit,
					&number_of_units,&bytes_left,&data);
				if (data)
				{
					XFree(data);
				}
				if ((Success!=result)||(None==property_type))
				{
					/* not currently locked.  Lock it */
					XChangeProperty(display,netscape_window,XA_MOZILLA_LOCK,XA_STRING,8,
						PropModeReplace,(unsigned char *)lock_data,strlen(lock_data));
					locked=1;
				}
				XUngrabServer(display);
				XSync(display,False);
				if (!locked)
				{
					/* another process is using netscape.  Wait for a property delete
						event */
					locked=1;
					do
					{
						XNextEvent(display,&event);
						if ((DestroyNotify==event.xany.type)&&(netscape_window==
							event.xdestroywindow.window))
						{
							return_code=0;
						}
						else
						{
							if ((PropertyNotify==event.xany.type)&&(PropertyDelete==
								event.xproperty.state)&&(netscape_window==
								event.xproperty.window)&&
								(XA_MOZILLA_LOCK==event.xproperty.atom))
							{
								locked=0;
							}
						}
					} while (return_code&&locked);
				}
			} while (return_code&&(!locked));
		}
	}

	return (return_code);
} /* lock_netscape */
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
static int unlock_netscape(Display *display,Window netscape_window)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Unlocks netscape so that other processes can interact with it.
==============================================================================*/
{
	Atom property_type;
	int data_unit,result,return_code;
	char *data;
	unsigned long bytes_left,number_of_units;

	return_code=0;
	/* check arguments */
	if (display&&netscape_window)
	{
		data=(char *)NULL;
		result=XGetWindowProperty(display,netscape_window,XA_MOZILLA_LOCK,0,
			(65536/sizeof(long)),True,XA_STRING,&property_type,&data_unit,
			&number_of_units,&bytes_left,(unsigned char **)&data);
		if (Success==result)
		{
			if (data&&(*data))
			{
				if (0==strcmp(data,lock_data))
				{
					return_code=1;
				}
			}
		}
		if (data)
		{
			XFree(data);
		}
	}

	return (return_code);
} /* lock_netscape */
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
static int set_netscape_url(Display *display,Window netscape_window,char *url)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Executes the netscape url.
==============================================================================*/
{
	char *url_command;
	int return_code;

	return_code=0;
	/* check arguments */
	if (display&&netscape_window&&url)
	{
		if (ALLOCATE(url_command,char,strlen(url)+40))
		{
			sprintf(url_command,"openURL(%s)",url);
			XChangeProperty(display,netscape_window,XA_MOZILLA_COMMAND,XA_STRING,8,
				PropModeReplace,(unsigned char *)url_command,strlen(url_command));
			DEALLOCATE(url_command);
			return_code=1;
			/*???DB.  Could wait for response, but won't at present */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_netscape_url.  Could not allocate url_command");
		}
	}

	return (return_code);
} /* set_netscape_url */
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/

void do_help(char *help_string,char *help_examples_directory,
	struct Execute_command *execute_command,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Gets help for help_string.
If the help window hasn't been created,it creates it.
If the help window is popped down it pops it up.
Then it sets the help topic edit field and gets the help on the string.
==============================================================================*/
{
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
	char *temp_string;
	Display *display;
	int count,max_attempts;
	static char *netscape_command=(char *)NULL;
#define XmNnetscapeCommand "netscapeCommand"
#define XmCNetscapeCommand "NetscapeCommand"
	static XtResource resources[]=
	{
		{
			XmNnetscapeCommand,
			XmCNetscapeCommand,
			XmRString,
			sizeof(char *),
			0,
			XmRString,
			"netscape"
		}
	};
	Window netscape_window;
#else /* defined (NETSCAPE_HELP) */
#if defined (HYPERTEXT_HELP)
	static int hypertext_help_initialized=0;
#else /* defined (HYPERTEXT_HELP) */
	char **the_strings;
	short num_strings;
#endif /* defined (HYPERTEXT_HELP) */
#endif /* defined (NETSCAPE_HELP) */
#endif /* defined (MOTIF) */

	ENTER(do_help);
#if defined (MOTIF)
#if !defined (HYPERTEXT_HELP)
	USE_PARAMETER(help_examples_directory);
#endif /* !defined (HYPERTEXT_HELP) */
#if defined (DEBUG)
	/*???debug */
	printf("enter do_help %s\n",help_string);
#endif /* defined (DEBUG) */
#if defined (NETSCAPE_HELP)
	/* check arguments */
	if (user_interface&&execute_command)
	{
		display=User_interface_get_display(user_interface);
		/* initialize atoms */
		if (!XA_MOZILLA_VERSION)
		{
			XA_MOZILLA_VERSION=XInternAtom(display,MOZILLA_VERSION_PROPERTY,False);
		}
		if (!XA_MOZILLA_LOCK)
		{
			XA_MOZILLA_LOCK=XInternAtom(display,MOZILLA_LOCK_PROPERTY,False);
		}
		if (!XA_MOZILLA_COMMAND)
		{
			XA_MOZILLA_COMMAND=XInternAtom(display,MOZILLA_COMMAND_PROPERTY,False);
		}
		if (!XA_MOZILLA_RESPONSE)
		{
			XA_MOZILLA_RESPONSE=XInternAtom(display,MOZILLA_RESPONSE_PROPERTY,False);
		}
		if (!WM_NAME)
		{
			WM_NAME=XInternAtom(display,WINDOW_NAME,False);
		}
		/* check if Netscape is running */
		netscape_window=get_netscape_window(display);
		if (!netscape_window)
		{
			if (!netscape_command)
			{
				XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
					&netscape_command,resources,XtNumber(resources),NULL);
				if (ALLOCATE(temp_string,char,strlen(netscape_command)+3))
				{
					strcpy(temp_string,netscape_command);
					netscape_command=temp_string,
					strcat(netscape_command," &");
				}
				else
				{
					netscape_command=(char *)NULL;
				}
			}
			if (netscape_command)
			{
				if (-1!=system(netscape_command))
				{
					max_attempts=10;
					count=0;
					while (!(netscape_window=get_netscape_window(display))&&
						(count<max_attempts))
					{
						sleep(5);
						count++;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"do_help.  Could not retrieve netscape_command");
			}
		}
		if (netscape_window)
		{
			XSelectInput(display,netscape_window,(PropertyChangeMask|
				StructureNotifyMask));
			lock_netscape(display,netscape_window);
			set_netscape_url(display,netscape_window,help_string);
			unlock_netscape(display,netscape_window);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not start netscape");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"do_help.  Invalid argument(s)");
	}
#else /* defined (NETSCAPE_HELP) */
#if defined (HYPERTEXT_HELP)
	if (help_string&&execute_command&&user_interface&&help_examples_directory)
	{
		if (0==hypertext_help_initialized)
		{
			examples_directory=help_examples_directory;
			help_execute_command=execute_command;
			initHelp(User_interface_get_application_shell(user_interface));
			setUnknownURLCallback(example_url_callback);
			hypertext_help_initialized=1;
		}
		setHelp(help_string);
	}
#else /* defined (HYPERTEXT_HELP) */
	if (!help_window_structure)
	{
		create_help_window((D_FUNC)0,&help_window_structure);
	}
	if (help_window_structure)
	{
		if (!(help_window_structure->popped_up))
		{
			pop_up_help_window(help_window_structure);
		}
		if (help_string)
		{
			XmTextFieldSetString(help_window_structure->help_topic,help_string);
			num_strings=0;
			the_strings=break_string(help_string,&num_strings);
			do_strings_help(help_window_structure,the_strings,num_strings);
			DEALLOCATE(the_strings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"do_help.  Couldn't create help window");
	}
#endif /* defined (HYPERTEXT_HELP) */
#endif /* defined (NETSCAPE_HELP) */
#else /* defined (MOTIF) */
	USE_PARAMETER(help_string);
	USE_PARAMETER(help_examples_directory);
	USE_PARAMETER(execute_command);
	USE_PARAMETER(user_interface);
	display_message(ERROR_MESSAGE,"do_help.  "
		"Not implemented for the user interface.");
#endif /* defined (MOTIF) */
#if defined (DEBUG)
	/*???debug */
	printf("leave do_help\n");
#endif /* defined (DEBUG) */
	LEAVE;
} /* do_help */

void pop_down_help(void)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Pops down the help window.
==============================================================================*/
{
	ENTER(pop_down_help);
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
#else /* defined (NETSCAPE_HELP) */
#if !defined (HYPERTEXT_HELP)
	if (help_window_structure)
	{
		if (help_window_structure->popped_up)
		{
			pop_down_help_window(help_window_structure);
		}
	}
#endif /* !defined (HYPERTEXT_HELP) */
#endif /* defined (NETSCAPE_HELP) */
#else /* defined (MOTIF) */
	display_message(ERROR_MESSAGE,"pop_down_help.  "
		"Not implemented for the user interface.");
#endif /* defined (MOTIF) */
	LEAVE;
} /* pop_down_help */

void destroy_help(void)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Destroys the help window.
==============================================================================*/
{
	ENTER(destroy_help);
#if defined (MOTIF)
#if defined (NETSCAPE_HELP)
#else /* defined (NETSCAPE_HELP) */
#if !defined (HYPERTEXT_HELP)
	if (help_window_structure)
	{
		close_help_window(help_window_structure);
	}
#endif /* !defined (HYPERTEXT_HELP) */
#endif /* defined (NETSCAPE_HELP) */
#else /* defined (MOTIF) */
	display_message(ERROR_MESSAGE,"destroy_help.  "
		"Not implemented for the user interface.");
#endif /* defined (MOTIF) */
	LEAVE;
} /* destroy_help */

/*******************************************************************************
FILE : comfile_window.c

LAST MODIFIED : 18 April 2002

DESCRIPTION :
Management routines for the comfile window.
==============================================================================*/
#include <stdio.h>
#include <string.h>
#if defined (MOTIF)
#include <Xm/Protocols.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/MwmUtil.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "comfile/comfile_window.h"
#if defined (MOTIF)
#include "comfile/comfile_window.uidh"
#endif /* defined (MOTIF) */
#include "command/command.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/

struct Comfile_window
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Command file or "comfile" window structure which lists a cmiss command file and
by user selection passes commands to the command window.
???DB.  Should there be a "window_structure" which contains shell information
etc ?
==============================================================================*/
{
	char *name;
	/* need to keep comfile window manager so window can be destroyed by self */
	struct MANAGER(Comfile_window) *comfile_window_manager;
	char *file_name;
	int number_of_commands;
	char **commands;
	Widget command_list;
	Widget window;
	Widget shell;
	struct User_interface *user_interface;
	/* for executing commands */
	struct Execute_command *execute_command;
	/* for setting command text ready to edit and enter */
	struct Execute_command *set_command;
	/* the number of objects accessing this window. The window cannot be removed
		from manager unless it is 1 (ie. only the manager is accessing it) */
	int access_count;
}; /* struct Comfile_window */

FULL_DECLARE_INDEXED_LIST_TYPE(Comfile_window);

FULL_DECLARE_MANAGER_TYPE(Comfile_window);

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int comfile_window_hierarchy_open=0;
static MrmHierarchy comfile_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Comfile_window, name, char *, strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Comfile_window)

#if defined (MOTIF)
static void identify_command_list(Widget widget,
	XtPointer comfile_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 June 1996

DESCRIPTION :
Finds the id of the command list widget in the comfile window and reads the
commands from the comfile into the list.
==============================================================================*/
{
	char **command,*command_string,*line;
	FILE *comfile;
	int i,number_of_commands;
	struct Comfile_window *comfile_window;

	ENTER(identify_command_list);
	USE_PARAMETER(call_data);
	if (comfile_window=(struct Comfile_window *)comfile_window_structure)
	{
		comfile_window->command_list=widget;
		if ((comfile_window->file_name)&&
			(comfile=fopen(comfile_window->file_name,"r")))
		{
			number_of_commands=0;
			while ((EOF!=fscanf(comfile," "))&&!feof(comfile)&&
				read_string(comfile,"[^\n]",&line))
			{
				if (command_string=trim_string(line))
				{
					if (*command_string)
					{
						XmListAddItem(comfile_window->command_list,
							XmStringCreateSimple(command_string),0);
						number_of_commands++;
					}
					DEALLOCATE(command_string);
				}
				DEALLOCATE(line);
			}
			if (!feof(comfile))
			{
				display_message(ERROR_MESSAGE,
					"identify_command_list.  Error reading comfile");
			}
			if (number_of_commands>0)
			{
				if (ALLOCATE(command,char *,number_of_commands))
				{
					comfile_window->number_of_commands=number_of_commands;
					comfile_window->commands=command;
					for (i=number_of_commands;i>0;i--)
					{
						*command=(char *)NULL;
						command++;
					}
					XtVaSetValues(comfile_window->window,
						XmNdialogTitle, XmStringCreateSimple(comfile_window->name),
						NULL);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"identify_command_list.  Could not allocate memory for commands");
				}
			}
			fclose(comfile);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"identify_command_list.  Could not open file");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_command_list.  Missing comfile window");
	}
	LEAVE;
} /* identify_command_list */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void close_comfile_window(Widget widget,
	XtPointer comfile_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Called when "close" is selected from the window menu, or it is double clicked.
How this is made to occur is as follows. The comfile_window dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE(Comfile_window) for more
details.
Since the Comfile_window is a managed object, we simply remove it from its
manager. If this is successful it will be DESTROYed automatically.
If it is not managed, can't destroy it here.
==============================================================================*/
{
	struct Comfile_window *comfile_window;

	ENTER(close_comfile_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (comfile_window=(struct Comfile_window *)comfile_window_structure)
	{
		if (comfile_window->comfile_window_manager)
		{
			REMOVE_OBJECT_FROM_MANAGER(Comfile_window)(comfile_window,
				comfile_window->comfile_window_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_comfile_window.  Missing comfile_window structure");
	}
	LEAVE;
} /* close_comfile_window */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void item_selected(Widget widget,XtPointer comfile_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Called when a command is selected (clicked).
==============================================================================*/
{
	char *command_string;
	int position;
	struct Comfile_window *comfile_window;
	XmListCallbackStruct *list_callback;

	ENTER(item_selected);
	USE_PARAMETER(widget);
	/* check arguments */
	if ((list_callback=(XmListCallbackStruct *)call_data)&&
		(comfile_window=(struct Comfile_window *)comfile_window_structure))
	{
		/* find the position of the selected command string */
		position=(list_callback->item_position)-1;
		/* if the command string has not been retrieved */
		if (!((comfile_window->commands)[position]))
		{
			/* retrieve the command string */
			command_string=(char *)NULL;
			XmStringGetLtoR(list_callback->item,XmSTRING_DEFAULT_CHARSET,
				&command_string);
			(comfile_window->commands)[position]=command_string;
		}
		/* put command in command window's command entry box */
		Execute_command_execute_string(comfile_window->set_command,
			(comfile_window->commands)[position]);
	}
	else
	{
		display_message(ERROR_MESSAGE,"item_selected.  Invalid argument(s)");
	}
	LEAVE;
} /* item_selected */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void execute_one(Widget widget,XtPointer comfile_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Called when a command is double-clicked.
==============================================================================*/
{
	char *command_string;
	int position;
	struct Comfile_window *comfile_window;
	XmListCallbackStruct *list_callback;

	ENTER(execute_one);
	USE_PARAMETER(widget);
	if ((list_callback=(XmListCallbackStruct *)call_data)&&
		(comfile_window=(struct Comfile_window *)comfile_window_structure))
	{
		/* find the position of the selected command string */
		position=(list_callback->item_position)-1;
		/* if the command string has not been retrieved */
		if (!((comfile_window->commands)[position]))
		{
			/* retrieve the command string */
			command_string=(char *)NULL;
			XmStringGetLtoR(list_callback->item,XmSTRING_DEFAULT_CHARSET,
				&command_string);
			(comfile_window->commands)[position]=command_string;
		}
		/* execute the command */
		Execute_command_execute_string(comfile_window->execute_command,
			(comfile_window->commands)[position]);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_one.  Invalid argument(s)");
	}
	LEAVE;
} /* execute_one */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void execute_all(Widget widget,XtPointer comfile_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Executes all commands in a comfile window.
==============================================================================*/
{
	char **command,*command_string;
	int number_of_commands,position;
	struct Comfile_window *comfile_window;
	XmStringTable command_strings=(XmStringTable)NULL;

	ENTER(execute_all);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (comfile_window=(struct Comfile_window *)comfile_window_structure)
	{
		/* execute each command in the file */
		command=comfile_window->commands;
		number_of_commands=comfile_window->number_of_commands;
		for (position=0;position<number_of_commands;position++)
		{
			/* if the command string has not been parsed */
			if (!(*command))
			{
				if (!command_strings)
				{
					XtVaGetValues(comfile_window->command_list,
						XmNitems,&command_strings,
						NULL);
				}
				if (command_strings)
				{
					/* retrieve the command string */
					command_string=(char *)NULL;
					XmStringGetLtoR(command_strings[position],XmSTRING_DEFAULT_CHARSET,
						&command_string);
					*command=command_string;
				}
			}
			if (*command)
			{
				/* execute the command */
				Execute_command_execute_string(comfile_window->execute_command,
					*command);
			}
			command++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_all.  Invalid argument(s)");
	}
	LEAVE;
} /* execute_all */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void execute_selected(Widget widget,XtPointer comfile_window_structure,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1993

DESCRIPTION :
Executes selected commands in a comfile window.
==============================================================================*/
{
	char *command_string;
	int i,number_of_selected_commands,position,*selected_commands;
	struct Comfile_window *comfile_window;
	XmStringTable command_strings=(XmStringTable)NULL;

	ENTER(execute_selected);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (comfile_window=(struct Comfile_window *)comfile_window_structure)
	{
		/* get the number of selected commands and their positions */
		if (XmListGetSelectedPos(comfile_window->command_list,&selected_commands,
			&number_of_selected_commands))
		{
			/* execute the selected commands */
			for (i=0;i<number_of_selected_commands;i++)
			{
				position=selected_commands[i]-1;
				/* if the command string has not been parsed */
				if (!(comfile_window->commands)[position])
				{
					if (!command_strings)
					{
						XtVaGetValues(comfile_window->command_list,
							XmNitems,&command_strings,
							NULL);
					}
					if (command_strings)
					{
						/* retrieve the command string */
						command_string=(char *)NULL;
						XmStringGetLtoR(command_strings[position],
							XmSTRING_DEFAULT_CHARSET,&command_string);
						/* parse the command string */
						(comfile_window->commands)[position]=command_string;
					}
				}
				if ((comfile_window->commands)[position])
				{
					/* execute the command */
					Execute_command_execute_string(comfile_window->execute_command,					
						(comfile_window->commands)[position]);
				}
			}
			XtFree((char *)selected_commands);
		}
		else
		{
			display_message(ERROR_MESSAGE,"execute_selected.  No commands selected");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_selected.  Invalid argument(s)");
	}
	LEAVE;
} /* execute_selected */
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
#if defined (MOTIF)
struct Comfile_window *CREATE(Comfile_window)(char *name,
	char *file_name,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Creates the structures and retrieves a comfile window widget from the Motif
resource manager hierarchy.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	FILE *comfile;
	MrmType comfile_window_class;
	static MrmRegisterArg callback_list[]=
	{
		{"close_comfile_window",(XtPointer)close_comfile_window},
		{"execute_all",(XtPointer)execute_all},
		{"execute_one",(XtPointer)execute_one},
		{"execute_selected",(XtPointer)execute_selected},
		{"identify_command_list",(XtPointer)identify_command_list},
		{"item_selected",(XtPointer)item_selected}
	};
	static MrmRegisterArg identifier_list[1];
	struct Comfile_window *comfile_window;

	ENTER(CREATE(Comfile_window));
	if (name && file_name && (comfile = fopen(file_name,"r")) &&
		execute_command && set_command && user_interface)
	{
		fclose(comfile);
		if (MrmOpenHierarchy_base64_string(comfile_window_uidh,
			&comfile_window_hierarchy,&comfile_window_hierarchy_open))
		{
			/* create the structure */
			if (ALLOCATE(comfile_window, struct Comfile_window, 1) &&
				(comfile_window->name = duplicate_string(name)) &&
				(comfile_window->file_name = duplicate_string(file_name)))
			{
				comfile_window->comfile_window_manager=
					(struct MANAGER(Comfile_window) *)NULL;
				comfile_window->command_list=(Widget)NULL;
				comfile_window->window=(Widget)NULL;
				comfile_window->shell=(Widget)NULL;
				comfile_window->number_of_commands=0;
				comfile_window->commands=(char **)NULL;
				comfile_window->execute_command=execute_command;
				comfile_window->set_command=set_command;
				comfile_window->user_interface=user_interface;
				comfile_window->access_count = 0;
				/* create the window shell */
				if (comfile_window->shell = XtVaCreatePopupShell(
					"comfile_window_shell", topLevelShellWidgetClass,
					User_interface_get_application_shell(user_interface),
					XmNallowShellResize,True,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNmwmDecorations,MWM_DECOR_ALL,
					XmNmwmFunctions,MWM_FUNC_ALL,
#if defined (OLD_CODE)
					/* Many of us want to close the window with a top left double click */
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions,MWM_FUNC_RESIZE|MWM_FUNC_MOVE|MWM_FUNC_MINIMIZE,
#endif /* defined (OLD_CODE) */
					NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW = XmInternAtom(
						XtDisplay(comfile_window->shell), "WM_DELETE_WINDOW", False);
					XmAddWMProtocolCallback(comfile_window->shell,
						WM_DELETE_WINDOW, close_comfile_window, comfile_window);
					/* register callbacks with Motif resource manager */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(comfile_window_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						identifier_list[0].name="comfile_window_structure";
						identifier_list[0].value=(XtPointer)comfile_window;
						/* register identifiers with Motif resource manager */
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							comfile_window_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* retrieve a comfile window widget */
							if (MrmSUCCESS==MrmFetchWidget(comfile_window_hierarchy,
								"comfile_window",comfile_window->shell,
								&(comfile_window->window),&comfile_window_class))
							{
								XtManageChild(comfile_window->window);
								XtRealizeWidget(comfile_window->shell);
								XtPopup(comfile_window->shell,XtGrabNone);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Comfile_window).  Could not retrieve widget");
								DEALLOCATE(comfile_window->file_name);
								DEALLOCATE(comfile_window->name);
								DEALLOCATE(comfile_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Comfile_window).  Could not register identifiers");
							DEALLOCATE(comfile_window->file_name);
							DEALLOCATE(comfile_window->name);
							DEALLOCATE(comfile_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Comfile_window).  Could not register callbacks");
						DEALLOCATE(comfile_window->file_name);
						DEALLOCATE(comfile_window->name);
						DEALLOCATE(comfile_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Comfile_window).  Could not create window shell");
					DEALLOCATE(comfile_window->file_name);
					DEALLOCATE(comfile_window->name);
					DEALLOCATE(comfile_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Comfile_window).  Could not allocate memory for structure");
				DEALLOCATE(comfile_window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Comfile_window).  Could not open hierarchy");
			comfile_window=(struct Comfile_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open: %s",file_name);
		comfile_window=(struct Comfile_window *)NULL;
	}
	LEAVE;

	return (comfile_window);
} /* CREATE(Comfile_window) */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int DESTROY(Comfile_window)(struct Comfile_window **comfile_window_address)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION:
Frees the contents of the Comfile_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the comfile_window from a global list of windows is left with the
calling routine. See also Comfile_window_close_CB and
Comfile_window_destroy_CB.
==============================================================================*/
{
	char **command;
	int i, return_code;
	struct Comfile_window *comfile_window;

	ENTER(DESTROY(Comfile_window));
	if (comfile_window_address && (comfile_window = *comfile_window_address))
	{
		/* destroy the comfile window widget */
		XtDestroyWidget(comfile_window->shell);
		/* free the memory for the file name */
		DEALLOCATE(comfile_window->file_name);
		DEALLOCATE(comfile_window->name);
		/* free the memory for the commands */
		if (command = comfile_window->commands)
		{
			for (i = comfile_window->number_of_commands; i > 0; i--)
			{
				XtFree(*command);
				command++;
			}
			DEALLOCATE(comfile_window->commands);
		}
		DEALLOCATE(*comfile_window_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Comfile_window).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Comfile_window) */
#endif /* defined (MOTIF) */

DECLARE_OBJECT_FUNCTIONS(Comfile_window)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Comfile_window)

DECLARE_INDEXED_LIST_FUNCTIONS(Comfile_window)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Comfile_window, \
	name,char *,strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Comfile_window,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Comfile_window,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Comfile_window,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Comfile_window,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name));
	if (source&&destination)
	{
		/*???RC have problems with copying scene_manager? messages? */
		printf("MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name).  "
			"Not used\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Comfile_window,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Comfile_window,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Comfile_window,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			if (destination->name)
			{
				DEALLOCATE(destination->name);
			}
			destination->name=destination_name;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Comfile_window,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Comfile_window,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Comfile_window,name) */

/* NOTE: Using special ADD_OBJECT_TO_MANAGER function so that object keeps
	pointer to its manager while it is managed. */
DECLARE_MANAGER_FUNCTIONS(Comfile_window)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Comfile_window)

DECLARE_OBJECT_WITH_MANAGER_MANAGER_IDENTIFIER_FUNCTIONS(Comfile_window,name, \
	char *,comfile_window_manager)

char *Comfile_window_manager_make_unique_name(
	struct MANAGER(Comfile_window) *comfile_window_manager, char *file_name)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Allocates and returns a name based on <file_name> that is not currently in use
in the <comfile_window_manager>. Does so by appending a number in angle
brackets.
Up to the calling routine to deallocate the returned string.
==============================================================================*/
{
	char *return_name, temp_string[20];
	int error, length, number;

	ENTER(Comfile_window_manager_make_unique_name);
	if (comfile_window_manager && file_name)
	{
		if (return_name = duplicate_string(file_name))
		{
			error = 0;
			length = strlen(file_name);
			number = 1;
		}
		else
		{
			error = 1;
		}
		while (return_name && ((struct Comfile_window *)NULL !=
			FIND_BY_IDENTIFIER_IN_MANAGER(Comfile_window,name)(
				return_name, comfile_window_manager)))
		{
			number++;
			return_name[length] = '\0';
			sprintf(temp_string, "<%d>", number);
			append_string(&return_name, temp_string, &error);
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"Comfile_window_manager_make_unique_name.  Could not allocate name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Comfile_window_manager_make_unique_name.  Invalid argument(s)");
		return_name = (char *)NULL;
	}
	LEAVE;

	return (return_name);
} /* Comfile_window_manager_make_unique_name */

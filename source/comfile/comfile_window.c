/*******************************************************************************
FILE : comfile_window.c

LAST MODIFIED : 22 June 1999

DESCRIPTION :
Management routines for the comfile window.
==============================================================================*/
#include <stdio.h>
#include <string.h>
#if defined (MOTIF)
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/MwmUtil.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "comfile/comfile_window.h"
#include "comfile/comfile_window.uid64"
#include "command/command.h"
#include "general/mystring.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

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
#if defined (MOTIF)
static void destroy_Comfile_window(Widget widget,
	XtPointer comfile_window_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 June 1996

DESCRIPTION :
Destroy the comfile_window structure.
==============================================================================*/
{
	int i;
	struct Comfile_window *comfile_window;
	char **command;

	ENTER(destroy_Comfile_window);
	/* check the arguments */
	if (comfile_window=(struct Comfile_window *)comfile_window_structure)
	{
		/* free the memory for the file name */
		DEALLOCATE(comfile_window->file_name);
		/* free the memory for the commands */
		if (command=comfile_window->commands)
		{
			for (i=comfile_window->number_of_commands;i>0;i--)
			{
				XtFree(*command);
				command++;
			}
			DEALLOCATE(comfile_window->commands);
		}
		DEALLOCATE(comfile_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Comfile_window.  Missing comfile window structure");
	}
	LEAVE;
} /* destroy_Comfile_window */
#endif /* defined (MOTIF) */

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
						XmNdialogTitle,XmStringCreateSimple(comfile_window->file_name),
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
LAST MODIFIED : 24 June 1993

DESCRIPTION :
Closes the comfile window.
==============================================================================*/
{
	struct Comfile_window *comfile_window;

	ENTER(close_comfile_window);
	if (comfile_window=(struct Comfile_window *)comfile_window_structure)
	{
		XtPopdown(comfile_window->shell);
		XtDestroyWidget(comfile_window->shell);
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
		(*(comfile_window->set_command->function))(
			(comfile_window->commands)[position],
			comfile_window->set_command->data);
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
		/* execute the command */
		(*(comfile_window->execute_command->function))(
			(comfile_window->commands)[position],
			comfile_window->execute_command->data);
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
	/* check arguments */
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
				(*(comfile_window->execute_command->function))(*command,
					comfile_window->execute_command->data);
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
	/* check arguments */
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
					(*(comfile_window->execute_command->function))(
						(comfile_window->commands)[position],
						comfile_window->execute_command->data);
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

#if defined (MOTIF)
static int open_comfile_via_selection_box(char *file_name,
	void *open_comfile_data_void)
/*******************************************************************************
LAST MODIFIED : 25 February 1997

DESCRIPTION :
Submits a command to open a comfile.
==============================================================================*/
{
	char *file_extension;
	int length,return_code;
	struct Open_comfile_data *open_comfile_data;

	ENTER(open_comfile_via_selection_box);
	/* check arguments */
	if (file_name&&
		(open_comfile_data=(struct Open_comfile_data *)open_comfile_data_void))
	{
		/* remove the file extension */
		if (file_extension=strrchr(file_name,'.'))
		{
			length=file_extension-file_name;
		}
		else
		{
			length=strlen(file_name);
		}
		if (ALLOCATE(open_comfile_data->file_name,char,length+1))
		{
			strncpy(open_comfile_data->file_name,file_name,length);
			(open_comfile_data->file_name)[length]='\0';
			return_code=open_comfile((struct Parse_state *)NULL,(void *)NULL,
				open_comfile_data_void);
		}
		else
		{
			display_message(ERROR_MESSAGE,
"open_comfile_via_selection_box.  Could not allocate memory for command string"
				);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_comfile_via_selection_box.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile_via_selection_box */
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
#if defined (MOTIF)
struct Comfile_window *create_Comfile_window(char *file_name,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Creates the structures and retrieves a comfile window widget from the Motif
resource manager hierarchy.
==============================================================================*/
{
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

	ENTER(create_Comfile_window);
	/* check the arguments */
	if (file_name&&(comfile=fopen(file_name,"r"))&&
		execute_command&&(execute_command->function)&&
		set_command&&(set_command->function)&&
		user_interface)
	{
		fclose(comfile);
		if (MrmOpenHierarchy_base64_string(comfile_window_uid64,
			&comfile_window_hierarchy,&comfile_window_hierarchy_open))
		{
			/* create the structure */
			if (ALLOCATE(comfile_window,struct Comfile_window,1)&&
				ALLOCATE(comfile_window->file_name,char,strlen(file_name)+1))
			{
				strcpy(comfile_window->file_name,file_name);
				comfile_window->command_list=(Widget)NULL;
				comfile_window->window=(Widget)NULL;
				comfile_window->shell=(Widget)NULL;
				comfile_window->number_of_commands=0;
				comfile_window->commands=(char **)NULL;
				comfile_window->execute_command=execute_command;
				comfile_window->set_command=set_command;
				comfile_window->user_interface=user_interface;
				/* create the window shell */
				if (comfile_window->shell=XtVaCreatePopupShell("comfile_window_shell",
					topLevelShellWidgetClass,user_interface->application_shell,
					XmNallowShellResize,True,
#if defined (OLD_CODE)
					/* Many of us want to close the window with a top left double click */
					XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions,MWM_FUNC_RESIZE|MWM_FUNC_MOVE|MWM_FUNC_MINIMIZE,
#endif /* defined (OLD_CODE) */
					NULL))
				{
					/* add destroy callback */
					XtAddCallback(comfile_window->shell,XmNdestroyCallback,
						destroy_Comfile_window,(XtPointer)comfile_window);
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
									"create_Comfile_window.  Could not retrieve widget");
								DEALLOCATE(comfile_window->file_name);
								DEALLOCATE(comfile_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Comfile_window.  Could not register identifiers");
							DEALLOCATE(comfile_window->file_name);
							DEALLOCATE(comfile_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Comfile_window.  Could not register callbacks");
						DEALLOCATE(comfile_window->file_name);
						DEALLOCATE(comfile_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Comfile_window.  Could not create window shell");
					DEALLOCATE(comfile_window->file_name);
					DEALLOCATE(comfile_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Comfile_window.  Could not allocate memory for structure");
				DEALLOCATE(comfile_window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Comfile_window.  Could not open hierarchy");
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
} /* create_Comfile_window */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int execute_comfile(char *file_name,struct Execute_command *execute_command)
/******************************************************************************
LAST MODIFIED : 5 November 1997

DESCRIPTION :
Opens, executes and then closes a com file.  No window is created.
=============================================================================*/
{
	char *command_string;
	FILE *comfile;
	int return_code;

	ENTER(execute_comfile);
	if (file_name)
  {
    if (execute_command&&(execute_command->function))
    {
      if (comfile=fopen(file_name,"r"))
      {
        fscanf(comfile," ");
        while (!feof(comfile)&&(read_string(comfile,"[^\n]",&command_string)))
        {
          (*(execute_command->function))(command_string,execute_command->data);
          DEALLOCATE(command_string);
          fscanf(comfile," ");
        }
        fclose(comfile);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Could not open: %s",file_name);
        return_code=1;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"execute_comfile.  "
        "Invalid execute command");
      return_code=0;
    }
  }
	else
	{
		display_message(ERROR_MESSAGE,"execute_comfile.  Missing file name");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_comfile */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int open_comfile(struct Parse_state *state,void *dummy_to_be_modified,
	void *open_comfile_data_void)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
==============================================================================*/
{
	char *command_string;
	int i,length,return_code;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_char_flag},
		{"execute",NULL,NULL,set_int_optional},
		{"name",NULL,(void *)1,set_name},
		{NULL,NULL,NULL,set_name}
	};
	struct File_open_data *file_open_data;
	struct Open_comfile_data *file_open_comfile_data,*open_comfile_data;
#if defined (OLD_CODE)
	char *temp_char,*temp_char_2;
	int count;
#endif /* defined (OLD_CODE) */

	ENTER(open_comfile);
	/* check arguments */
	if (open_comfile_data=(struct Open_comfile_data *)open_comfile_data_void)
	{
		if (open_comfile_data->file_name)
		{
			if (open_comfile_data->example_flag)
			{
				/* include the relative path */
				length=strlen(open_comfile_data->file_name)+1;
				if (open_comfile_data->examples_directory)
				{
					length += strlen(open_comfile_data->examples_directory);
				}
				if (ALLOCATE(command_string,char,length))
				{
					*command_string='\0';
					if (open_comfile_data->examples_directory)
					{
						strcat(command_string,open_comfile_data->examples_directory);
					}
					strcat(command_string,open_comfile_data->file_name);
					DEALLOCATE(open_comfile_data->file_name);
					open_comfile_data->file_name=command_string;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_comfile.  Insufficient memory");
				}
#if defined (OLD_CODE)
				/* strip the path */
				temp_char_2=open_comfile_data->file_name;
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
				/*???DB.  Richard added */
/*				temp_char=temp_char_2;*/
				count=0;
				while ((*(temp_char+count))&&('?'!= *(temp_char+count)))
				{
					count++;
				}
				if (count>0)
				{
					if (ALLOCATE(command_string,char,strlen(open_comfile_data->
						examples_directory)+strlen(open_comfile_data->example_symbol)+13+
						(count*(count+5))/2))
					{
						/* set the examples directory (back end) */
						strcpy(command_string,"set dir ");
						strcat(command_string,open_comfile_data->example_symbol);
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
						(*(open_comfile_data->execute_command->function))(command_string,
							open_comfile_data->execute_command->data);
						/* construct the full comfile name */
						strcpy(command_string,open_comfile_data->examples_directory);
#if defined (UNIX)
						temp_char_2=command_string+strlen(command_string);
#endif /* defined (UNIX) */
#if defined (VMS)
						temp_char_2=command_string+(strlen(command_string)-1);
#endif /* defined (VMS) */
						for (i=1;i<=count;i++)
						{
#if defined (VMS)
							*temp_char_2='.';
							temp_char_2++;
#endif /* defined (VMS) */
							strncpy(temp_char_2,temp_char,i);
							temp_char_2 += i;
#if defined (UNIX)
							*temp_char_2='/';
							temp_char_2++;
#endif /* defined (UNIX) */
						}
#if defined (VMS)
						*temp_char_2=']';
						temp_char_2++;
#endif /* defined (VMS) */
						strcpy(temp_char_2,"example_");
						temp_char_2 += 8;
						strncpy(temp_char_2,temp_char,count);
						temp_char_2[count]='\0';
						DEALLOCATE(open_comfile_data->file_name);
						open_comfile_data->file_name=command_string;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_comfile.  Insufficient memory");
					}
				}
#endif /* defined (OLD_CODE) */
			}
			/* open the file */
			if (return_code=check_suffix(&(open_comfile_data->file_name),
				open_comfile_data->file_extension))
			{
				if (0<open_comfile_data->execute_count)
				{
					for (i=open_comfile_data->execute_count;i>0;i--)
					{
						execute_comfile(open_comfile_data->file_name,
							open_comfile_data->execute_command);
					}
				}
				else
				{
					if (create_Comfile_window(open_comfile_data->file_name,
						open_comfile_data->execute_command,open_comfile_data->set_command,
						open_comfile_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_comfile.  Could not create comfile window");
						return_code=0;
					}
				}
			}
		}
		else
		{
			if (state)
			{
				/* initialize defaults */
				(option_table[0]).option=open_comfile_data->example_symbol;
				(option_table[0]).to_be_modified= &(open_comfile_data->example_flag);
				(option_table[1]).to_be_modified= &(open_comfile_data->execute_count);
				(option_table[2]).to_be_modified= &(open_comfile_data->file_name);
				(option_table[3]).to_be_modified= &(open_comfile_data->file_name);
				return_code=process_multiple_options(state,option_table);
				/* no errors, not asking for help */
				if (return_code)
				{
					if (open_comfile_data->file_name)
					{
						return_code=open_comfile(state,dummy_to_be_modified,
							open_comfile_data_void);
					}
					else
					{
						/* open file selection box */
						if (ALLOCATE(file_open_comfile_data,struct Open_comfile_data,1))
						{
							if (file_open_data=create_File_open_data(
								open_comfile_data->file_extension,REGULAR,
								open_comfile_via_selection_box,(void *)file_open_comfile_data,0,
								open_comfile_data->user_interface))
							{
								memcpy((void *)file_open_comfile_data,open_comfile_data_void,
									sizeof(struct Open_comfile_data));
								open_file_and_read((Widget)NULL,(XtPointer)file_open_data,
									(XtPointer)NULL);
								/*???DB.  How to free file_open_data ? */
								return_code=1;
							}
							else
							{
								DEALLOCATE(file_open_comfile_data);
								display_message(ERROR_MESSAGE,
									"open_comfile.  Could not allocate file open data");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_comfile.  Could not allocate file open comfile data");
							return_code=0;
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"open_comfile.  Missing state");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_comfile.  Missing open_comfile_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile */
#endif /* defined (MOTIF) */

#if defined (OLD_CODE)
#if defined (MOTIF)
int open_comfile(struct Parse_state *state,
	struct Execute_command *execute_command,
	struct Modifier_entry *set_file_name_option_table,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 25 August 1997

DESCRIPTION :
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
???DB.  Make more like other commands - option_table etc
==============================================================================*/
{
	char *current_token,*file_name,*temp_file_name;
	int i,repeat_count,return_code;
	struct File_open_data *file_open_data;
	struct Modifier_entry *entry;

	ENTER(open_comfile);
	/* default return code */
	return_code=0;
	/* check arguments */
	if (state&&execute_command&&(execute_command->function)&&
		(entry=set_file_name_option_table)&&user_interface)
	{
		if (state->current_token)
		{
			file_name=(char *)NULL;
			while (entry->option)
			{
				entry->to_be_modified= &file_name;
				entry++;
			}
			entry->to_be_modified= &file_name;
			if (return_code=process_option(state,set_file_name_option_table))
			{
				if (file_name)
				{
					/* open the file */
					if (REALLOCATE(temp_file_name,file_name,char,strlen(file_name)+
						strlen(open_comfile_data->file_extension)))
					{
						file_name=temp_file_name;
						strcat(file_name,open_comfile_data->file_extension);
						if ((current_token=state->current_token)&&
							fuzzy_string_compare(current_token,"EXECUTE"))
						{
							shift_Parse_state(state,1);
							if (current_token=state->current_token)
							{
								/* read the number of times to be repeated */
								if (1==sscanf(current_token,"%i",&repeat_count))
								{
									for (i=repeat_count;i>0;i--)
									{
										execute_comfile(file_name,execute_command);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"Invalid repeat count: %s",
										current_token);
									return_code=0;
								}
							}
							else
							{
								return_code=execute_comfile(file_name,execute_command);
							}
						}
						else
						{
							if (create_Comfile_window(file_name,execute_command,
								user_interface))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"open_comfile.  Could not create comfile window");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_comfile.  Could not reallocate file name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"open_comfile.  Missing file name");
					return_code=0;
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			/* open file selection box */
			if (file_open_data=create_File_open_data(
				open_comfile_data->file_extension,REGULAR,
				open_comfile_via_selection_box,execute_command,0,user_interface))
			{
				open_file_and_read((Widget)NULL,(XtPointer)file_open_data,
					(XtPointer)NULL);
				/*???DB.  How to free file_open_data ? */
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"open_comfile.  Could not allocate file open data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_comfile.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int open_comfile_via_selection_box(char *file_name,void *execute_command_void)
/*******************************************************************************
LAST MODIFIED : 5 June 1996

DESCRIPTION :
Submits a command to open a comfile.
==============================================================================*/
{
	char *command_string,*file_extension;
	int length,return_code;
	struct Execute_command *execute_command;

	ENTER(open_comfile_via_selection_box);
	/* check arguments */
	if (file_name&&
		(execute_command=(struct Execute_command *)execute_command_void)&&
		(execute_command->function))
	{
		/* remove the file extension */
		if (file_extension=strrchr(file_name,'.'))
		{
			length=file_extension-file_name;
		}
		else
		{
			length=strlen(file_name);
		}
		if (ALLOCATE(command_string,char,length+14))
		{
			strcpy(command_string,"open comfile ");
			strncpy(command_string+13,file_name,length);
			command_string[length+13]='\0';
			(*(execute_command->function))(command_string,execute_command->data);
			DEALLOCATE(command_string);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
"open_comfile_via_selection_box.  Could not allocate memory for command string"
				);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_comfile_via_selection_box.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile_via_selection_box */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

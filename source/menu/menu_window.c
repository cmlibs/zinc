/*******************************************************************************
FILE : menu_window.c

LAST MODIFIED : 25 August 1997

DESCRIPTION :
Management routines for the menu window.
==============================================================================*/
#if defined (MOTIF)
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#endif /* defined (MOTIF) */
#include "command/parser.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "menu/menu_window.h"
#include "menu/menu_window.uidh"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int menu_window_hierarchy_open=0;
static MrmHierarchy menu_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
void close_menu_window(Widget widget_id, XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION:
	Closes the menu window.
==============================================================================*/
{
	struct Menu_window *menu_window;

	ENTER(close_menu_window);
	USE_PARAMETER(widget_id);
	USE_PARAMETER(call_data);
	if (menu_window=(struct Menu_window *)client_data)
	{
		XtPopdown(menu_window->shell);
		XtDestroyWidget(menu_window->shell);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Menu window structure missing");
	}
	LEAVE;
} /* close_menu_window */

struct Macro *create_Macro(char *file_name)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Create and parse the macro structure for a given <file_name>.
==============================================================================*/
{
	char **command,*line,*title_end;
	FILE *macro_file;
	int i,length,number_of_commands;
	struct Macro *macro;

	ENTER(create_Macro);
	if (file_name)
	{
		if (macro_file=fopen(file_name,"r"))
		{
			if (ALLOCATE(macro,struct Macro,1)&&
				ALLOCATE(macro->file_name,char,strlen(file_name)+1))
			{
				strcpy(macro->file_name,file_name);
				/* skip blank lines at the beginning of the file and leading blanks
					for the first non blank line */
				fscanf(macro_file," ");
				if (!feof(macro_file))
				{
					/* read the first non blank line which may contain the macro name */
					if (read_string(macro_file,"[^\n]",&line))
					{
						/* check if it is the macro name */
						if ('['== *line)
						{
							if (title_end=strrchr(line,']'))
							{
								length=(title_end-line)+1;
							}
							else
							{
								length=strlen(line);
							}
							if (ALLOCATE(macro->name,char,length+1))
							{
								strncpy(macro->name,line,length);
								(macro->name)[length]='\0';
								number_of_commands=0;
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"create_Macro.  Insufficient memory for macro name");
								DEALLOCATE(macro->file_name);
								DEALLOCATE(macro);
							}
						}
						else
						{
							/* use the file name as the macro name */
							if (ALLOCATE(macro->name,char,strlen(file_name)+1))
							{
								strcpy(macro->name,file_name);
								number_of_commands=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"create_Macro.  Insufficient memory for macro name");
								DEALLOCATE(macro->file_name);
								DEALLOCATE(macro);
							}
						}
						DEALLOCATE(line);
						if (macro)
						{
							/* count the number of commands */
							while (EOF!=fscanf(macro_file," %*[^\n]"))
							{
								number_of_commands++;
							}
							if (number_of_commands>0)
							{
								if (ALLOCATE(command,char *,number_of_commands))
								{
									macro->commands=command;
									macro->number_of_commands=number_of_commands;
									/* read the commands */
									i=number_of_commands;
									rewind(macro_file);
									/* skip blank lines at the beginning of the file and leading
										blanks for the first non blank line */
									fscanf(macro_file," ");
									/* read the first non blank line which may contain the macro
										name */
									if (read_string(macro_file,"[^\n]",&line))
									{
										/* check if it is the macro name */
										if ('['!= *line)
										{
											/* create the first command */
											*command=line;
											command++;
											i--;
										}
										else
										{
											DEALLOCATE(line);
										}
										while (macro&&(i>0)&&(EOF!=fscanf(macro_file," ")))
										{
											if (read_string(macro_file,"[^\n]",&line))
											{
												/* create the command */
												*command=line;
												command++;
												i--;
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"create_Macro.  Error reading line");
												while (i<number_of_commands)
												{
													command--;
													DEALLOCATE(*command);
													i++;
												}
												DEALLOCATE(macro->commands);
												DEALLOCATE(macro->name);
												DEALLOCATE(macro->file_name);
												DEALLOCATE(macro);
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"create_Macro.  Error reading line");
										DEALLOCATE(macro->commands);
										DEALLOCATE(macro->name);
										DEALLOCATE(macro->file_name);
										DEALLOCATE(macro);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"create_Macro.  Insufficient memory for commands");
									DEALLOCATE(macro->name);
									DEALLOCATE(macro->file_name);
									DEALLOCATE(macro);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_Macro.  No commands in file");
								DEALLOCATE(macro->name);
								DEALLOCATE(macro->file_name);
								DEALLOCATE(macro);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Macro.   Error reading line");
						DEALLOCATE(macro->file_name);
						DEALLOCATE(macro);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"create_Macro.  Insufficient memory");
					DEALLOCATE(macro);
				}
				fclose(macro_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Macro.  Insufficient memory");
				DEALLOCATE(macro);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open macro file : %s",file_name);
			macro=(struct Macro *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Macro.  Missing file name");
		macro=(struct Macro *)NULL;
	}
	LEAVE;

	return (macro);
} /* create_Macro */

struct Menu_window *create_Menu_window(char *file_name,
	struct Execute_command *execute_command,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Create the structures and retrieve the menu window from the uil file.
==============================================================================*/
{
	struct Menu_window *menu_window;
	MrmType menu_window_class;
	static MrmRegisterArg callback_list[]=
	{
		{"close_menu_window",(XtPointer)close_menu_window},
		{"execute_menu_item",(XtPointer)execute_menu_item},
		{"identify_menu_list",(XtPointer)identify_menu_list}
	};
	static MrmRegisterArg identifier_list[1];
	FILE *menu_handle;

	ENTER(create_Menu_window);
	/* check arguments */
	if (file_name&&user_interface&&execute_command)
	{
		if (MrmOpenHierarchy_base64_string(menu_window_uidh,
			&menu_window_hierarchy,&menu_window_hierarchy_open))
		{
			if (menu_handle=fopen(file_name,"r"))
			{
				fclose(menu_handle);
				if (ALLOCATE(menu_window, struct Menu_window,1)&&
					ALLOCATE(menu_window->file_name,char,strlen(file_name)+1))
				{
					menu_window->execute_command=execute_command;
					menu_window->menu_list=(Widget)NULL;
					menu_window->window=(Widget)NULL;
					menu_window->shell=(Widget)NULL;
					menu_window->macros=(struct Macro **)NULL;
					menu_window->num_macros=0;
					strcpy(menu_window->file_name, file_name);
					if (menu_window->shell=XtVaCreatePopupShell("menu_window_shell",
						xmDialogShellWidgetClass,User_interface_get_application_shell(user_interface),
						XmNallowShellResize,TRUE,
						XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
						XmNmwmFunctions,MWM_FUNC_RESIZE|MWM_FUNC_MOVE|MWM_FUNC_MINIMIZE,
						XmNtransient,FALSE,
						NULL))
					{
						/* Add destroy callback */
						XtAddCallback(menu_window->shell,XmNdestroyCallback,
							destroy_Menu_window,(XtPointer)menu_window);
						/* Register callbacks in UIL */
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(menu_window_hierarchy,
							callback_list,XtNumber(callback_list)))
						{
							identifier_list[0].name="menu_window_structure";
							identifier_list[0].value=(XtPointer)menu_window;
							if (MrmSUCCESS==MrmRegisterNamesInHierarchy(menu_window_hierarchy,
								identifier_list,XtNumber(identifier_list)))
							{
								if (MrmSUCCESS==MrmFetchWidget(menu_window_hierarchy,
									"menu_window",menu_window->shell,&(menu_window->window),
									&menu_window_class))
								{
									XtManageChild(menu_window->window);
									XtRealizeWidget(menu_window->shell);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_Menu_window.  Could not retrieve widget");
									DEALLOCATE(menu_window);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_Menu_window.  Could not register identifiers");
								DEALLOCATE(menu_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Menu_window.  Could not register callbacks");
							DEALLOCATE(menu_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Menu_window.  Could not create shell");
						DEALLOCATE(menu_window);
					}
				}
				else
				{
					if (menu_window)
					{
						DEALLOCATE(menu_window);
					}
					display_message(ERROR_MESSAGE,
						"create_Menu_window.  Insufficient memory");
					menu_window=(struct Menu_window *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open menu file : %s",
					file_name);
				menu_window=(struct Menu_window *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Menu_window.  Could not open hierarchy");
			menu_window=(struct Menu_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Menu_window.  Invalid argument(s)");
		menu_window=(struct Menu_window *)NULL;
	}
	LEAVE;

	return (menu_window);
} /* create_Menu_window */

int destroy_Macro(struct Macro **macro)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Free the memory associated with the fields of <**macro>, free the memory for
<**macro> and set <*macro> to NULL;
==============================================================================*/
{
	int i,return_code;
	char **command;

	ENTER(destroy_Macro);
	if (macro)
	{
		if (*macro)
		{
			DEALLOCATE((*macro)->name);
			DEALLOCATE((*macro)->file_name);
			if (((i=(*macro)->number_of_commands)>0)&&(command=(*macro)->commands))
			{
				while (i>0)
				{
					DEALLOCATE(*command);
					command++;
					i--;
				}
			}
			DEALLOCATE((*macro)->commands);
			DEALLOCATE(*macro);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "destroy_Macro.  Missing macro");
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* destroy_Macro */

void destroy_Menu_window(Widget widget, XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
Destroy the menu_window structure.
==============================================================================*/
{
	struct Menu_window *menu_window;
	int macro;

	ENTER(destroy_Menu_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (menu_window=(struct Menu_window *)client_data)
	{
		if (menu_window->macros)
		{
			for (macro=0;macro<menu_window->num_macros;macro++)
			{
				if (menu_window->macros[macro])
				{
					destroy_Macro(&(menu_window->macros[macro]));
				}
			}
			DEALLOCATE(menu_window->macros);
		}
		DEALLOCATE(menu_window->file_name);
		DEALLOCATE(menu_window);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Menu window structure missing");
	}
	LEAVE;
} /* destroy_Menu_window */

void execute_menu_item(Widget widget,XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Executes a double-clicked menu item.
==============================================================================*/
{
	int comm,num_items,position;
	struct Menu_window *menu_window;
	XmListCallbackStruct *list_callback;

	ENTER(execute_menu_item);
	if ((list_callback=(XmListCallbackStruct *)call_data)&&
		(menu_window=(struct Menu_window *)client_data))
	{
		position=(list_callback->item_position)-1;
		if (menu_window->macros[position])
		{
			for (comm=0;comm<menu_window->macros[position]->number_of_commands;comm++)
			{
				Execute_command_execute_string(menu_window->execute_command,
					menu_window->macros[position]->commands[comm]);
			}
			XtVaGetValues(widget,XmNitemCount,&num_items,NULL);
			if (list_callback->item_position>=num_items)
			{
				XmListSelectPos(widget,num_items,FALSE);
			}
			else
			{
				XmListSelectPos(widget,(list_callback->item_position)+1,FALSE);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_menu_item.  Invalid macro structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_menu_item.  Menu window structure missing");
	}
	LEAVE;
} /* execute_menu_item */

void identify_menu_list(Widget widget,XtPointer client_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 1993

DESCRIPTION :
Stores the id of the message areas.
==============================================================================*/
{
	char *line,no_error,*title;
	FILE *menu_file;
	int i,length,number_of_macros;
	struct Macro **macro;
	struct Menu_window *menu_window;

	ENTER(identify_menu_list);
	USE_PARAMETER(call_data);
	/* check the arguments */
	if (menu_window=(struct Menu_window *)client_data)
	{
		menu_window->menu_list=widget;
		if (menu_window->file_name)
		{
			if (menu_file=fopen(menu_window->file_name,"r"))
			{
				number_of_macros=0;
				/* skip blank lines at the beginning of the file and leading blanks
					for the first non blank line */
				fscanf(menu_file," ");
				if (!feof(menu_file))
				{
					/* read the first non blank line which may contain the title */
					if (read_string(menu_file,"[^\n]",&line))
					{
						/* check if it is the title */
						if ('['== *line)
						{
							if (title=strrchr(line,']'))
							{
								length=(title-line)+1;
							}
							else
							{
								length=strlen(line);
							}
							if (ALLOCATE(title,char,length+1))
							{
								strncpy(title,line,length);
								title[length]='\0';
								number_of_macros=0;
								no_error=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"identify_menu_list.  Insufficient memory for macro name");
								no_error=0;
							}
						}
						else
						{
							/* use the file name as the title */
							if (ALLOCATE(title,char,strlen(menu_window->file_name)+1))
							{
								strcpy(title,menu_window->file_name);
								number_of_macros=1;
								no_error=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"identify_menu_list.  Insufficient memory for title");
								no_error=0;
							}
						}
						DEALLOCATE(line);
						if (no_error)
						{
							/* count the number of macros */
							while (EOF!=fscanf(menu_file," %*[^\n]"))
							{
								number_of_macros++;
							}
							if (number_of_macros>0)
							{
								if (ALLOCATE(macro,struct Macro *,number_of_macros))
								{
									menu_window->macros=macro;
									menu_window->num_macros=number_of_macros;
									/* read the macros */
									i=number_of_macros;
									rewind(menu_file);
									/* skip blank lines at the beginning of the file and leading
										blanks for the first non blank line */
									fscanf(menu_file," ");
									/* read the first non blank line which may contain the
										title */
									if (read_string(menu_file,"[^\n]",&line))
									{
										/* check if it is the title */
										if ('['!= *line)
										{
											/* create the first macro */
											if (*macro=create_Macro(line))
											{
												XmListAddItem(menu_window->menu_list,
													XmStringCreateSimple((*macro)->name),0);
												macro++;
												i--;
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"identify_menu_list.  Error creating macro");
												DEALLOCATE(menu_window->macros);
												no_error=0;
											}
										}
										DEALLOCATE(line);
										while (no_error&&(i>0)&&(EOF!=fscanf(menu_file," ")))
										{
											if (read_string(menu_file,"[^\n]",&line))
											{
												/* create the macro */
												if (*macro=create_Macro(line))
												{
													XmListAddItem(menu_window->menu_list,
														XmStringCreateSimple((*macro)->name),0);
													macro++;
													i--;
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"identify_menu_list.  Error creating macro");
													while (i<number_of_macros)
													{
														macro--;
														destroy_Macro(macro);
														i++;
													}
													DEALLOCATE(menu_window->macros);
													no_error=0;
												}
												DEALLOCATE(line);
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"identify_menu_list.  Error reading line");
												while (i<number_of_macros)
												{
													macro--;
													destroy_Macro(macro);
													i++;
												}
												DEALLOCATE(menu_window->macros);
												no_error=0;
											}
										}
										if (no_error)
										{
											XtVaSetValues(menu_window->window,
												XmNdialogTitle,XmStringCreateSimple(title),
												NULL);
											XtPopup(menu_window->shell, XtGrabNone);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"identify_menu_list.  Error reading line");
										DEALLOCATE(menu_window->macros);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"identify_menu_list.  Insufficient memory for macros");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"identify_menu_list.  No macros in file");
							}
							DEALLOCATE(title);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"identify_menu_list.  Error reading line");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"identify_menu_list.  No macros in file");
				}
				fclose(menu_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open menu file : %s",
					menu_window->file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"identify_menu_list.  Missing file name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_menu_list.  Menu window structure missing");
	}
	LEAVE;
} /* identify_menu_list */

int open_menu(struct Parse_state *state,struct Execute_command *execute_command,
	struct Modifier_entry *set_file_name_option_table,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 December 1996

DESCRIPTION :
Opens a menu, and a window if it is to be executed.  If a menu is not specified
on the command line, a file selection box is presented to the user.
==============================================================================*/
{
	char *file_name,*temp_file_name;
	int return_code;
	struct File_open_data *file_open_data;
	struct Modifier_entry *entry;

	ENTER(open_menu);
	/* check arguments */
	if (state&&execute_command&&
		(entry=set_file_name_option_table))
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
				if (user_interface)
				{
					if (file_name)
					{
						/* open the file */
						if (REALLOCATE(temp_file_name,file_name,char,strlen(file_name)+6))
						{
							file_name=temp_file_name;
							strcat(file_name,".menu");
							if (create_Menu_window(file_name,execute_command,user_interface))
							{
								return_code=1;
							}
							else
							{
								display_message(INFORMATION_MESSAGE,
									"Could not create file name for: %s",file_name);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_menu.  Could not reallocate menu name");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"open_menu.  Missing menu name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_menu.  Missing user_interface.");
					return_code=0;
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			/* open file selection box */
			if (file_open_data=create_File_open_data(".menu",REGULAR,
				open_menu_from_fsb,execute_command,0,user_interface))
			{
				open_file_and_read((Widget)NULL,(XtPointer)file_open_data,
					(XtPointer)NULL);
				/*???DB.  How to free file_open_data ? */
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"open_menu.  Insufficient memory");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_menu.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_menu */

int open_menu_from_fsb(char *file_name,void *execute_command_void)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
Submits a command to open a menu.
==============================================================================*/
{
	char *command_string,*file_extension;
	int length,return_code;
	struct Execute_command *execute_command;

	ENTER(open_menu_from_fsb);
	/* check arguments */
	if (file_name&&
		(execute_command=(struct Execute_command *)execute_command_void))
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
		if (ALLOCATE(command_string,char,length+11))
		{
			strcpy(command_string,"open menu ");
			strncpy(command_string+10,file_name,length);
			command_string[length+10]='\0';
			Execute_command_execute_string(execute_command,command_string);
			DEALLOCATE(command_string);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_menu_from_fsb.  Could not allocate memory for command string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_menu_from_fsb.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_menu_from_fsb */

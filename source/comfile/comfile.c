/*******************************************************************************
FILE : comfile.c

LAST MODIFIED : 29 June 2002

DESCRIPTION :
Commands for comfiles.
==============================================================================*/
#include <stdio.h>
#include <string.h>
#include "general/debug.h"
#include "comfile/comfile.h"
#if defined (MOTIF)
#include "comfile/comfile_window.h"
#endif /* defined (MOTIF) */
#include "command/command.h"
#include "general/mystring.h"
#include "general/object.h"
#include "user_interface/confirmation.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"

int open_comfile(struct Parse_state *state,void *dummy_to_be_modified,
	void *open_comfile_data_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
==============================================================================*/
{
	char *command_string, *filename, *name;
	int i,length,return_code;
#if defined (MOTIF)
	struct Comfile_window *comfile_window;
#endif /* defined (MOTIF) */
	struct Open_comfile_data *open_comfile_data;
	struct Option_table *option_table;

	ENTER(open_comfile);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (open_comfile_data=(struct Open_comfile_data *)open_comfile_data_void)
	{
		if (state)
		{
			if (open_comfile_data->file_name)
			{
				filename = duplicate_string(open_comfile_data->file_name);
			}
			else
			{
				filename = (char *)NULL;
			}
			option_table = CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table, open_comfile_data->example_symbol,
				&(open_comfile_data->example_flag), NULL, set_char_flag);
			/* execute */
			Option_table_add_entry(option_table, "execute",
				&(open_comfile_data->execute_count), NULL, set_int_optional);
			/* name */
			Option_table_add_entry(option_table, "name",
				&filename, (void *)1, set_name);
			/* default */
			Option_table_add_entry(option_table, (char *)NULL,
				&filename, NULL, set_name);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* Prompt if we don't have a filename */
				if (!filename)
				{
					if (!(filename = confirmation_get_read_filename(
						open_comfile_data->file_extension, open_comfile_data->user_interface)))
					{
						return_code = 0;
					}
				}
			}
			if (return_code)
			{
				if (open_comfile_data->example_flag)
				{
					/* include the relative path */
					length=strlen(filename)+1;
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
						strcat(command_string,filename);
						DEALLOCATE(filename);
						filename=command_string;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"open_comfile.  Insufficient memory");
					}
				}
				/* open the file */
				if (return_code = check_suffix(&filename,
					open_comfile_data->file_extension))
				{
					if (0 < open_comfile_data->execute_count)
					{
						for (i=open_comfile_data->execute_count;i>0;i--)
						{
							execute_comfile(filename,
								open_comfile_data->execute_command);
						}
					}
					else
					{
#if defined (MOTIF)
						if (name = Comfile_window_manager_make_unique_name(
							open_comfile_data->comfile_window_manager,
							filename))
						{
							if (comfile_window = CREATE(Comfile_window)(name,
								filename,
								open_comfile_data->execute_command,
								open_comfile_data->set_command,
								open_comfile_data->user_interface))
							{
								if (ADD_OBJECT_TO_MANAGER(Comfile_window)(comfile_window,
										 open_comfile_data->comfile_window_manager))
								{
									return_code = 1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"open_comfile.  Could not manage comfile window");
									DESTROY(Comfile_window)(&comfile_window);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"open_comfile.  Could not create comfile window");
								return_code=0;
							}
							DEALLOCATE(name);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_comfile.  Could not allocate window name");
							return_code=0;
						}
#else /* defined (MOTIF) */
						display_message(ERROR_MESSAGE,
							"open_comfile.  Cannot create a comfile dialog, use execute.");
						return_code=0;
#endif /* defined (MOTIF) */
					}
				}
			}
			if (filename)
			{
				DEALLOCATE(filename);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"open_comfile.  Missing state");
			return_code=0;
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

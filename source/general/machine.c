/*******************************************************************************
FILE : machine.c

LAST MODIFIED : 22 June 1999

DESCRIPTION :
==============================================================================*/
#if defined (UNIX)
#include <sys/utsname.h>
#endif /* UNIX */
#if defined (VMS)
#include <stdlib.h>
#endif /* defined (VMS) */
#include "general/machine.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/
struct Machine_information *CREATE(Machine_information)(void)
/*******************************************************************************
LAST MODIFIED : 3 March 1997

DESCRIPTION :
Creates a machine information structure.
???GMH.  Perhaps this could be extended to tell it a machine to interrogate?
==============================================================================*/
{
	int name_length;
	struct Machine_information *return_struct;
#if defined (UNIX)
	struct utsname local_information;
#endif /* defined (UNIX) */
#if defined (VMS)
	char *node_name,*temp_char;
#endif /* defined (VMS) */

	ENTER(CREATE(Machine_information));
	if (ALLOCATE(return_struct,struct Machine_information,1))
	{
		/* determine what machine we are */
		name_length=0;
#if defined (UNIX)
		if (-1==uname(&local_information))
		{
			display_message(WARNING_MESSAGE,"CREATE(Machine_information).  %s",
				"Could not determine local machine name");
			/* NULL terminate the string */
			local_information.nodename[0]='\0';
			name_length=0;
		}
		else
		{
			name_length=strlen(local_information.nodename);
		}
#endif /* defined (UNIX) */
#if defined (VMS)
		if (node_name=getenv("sys$node"))
		{
			if (temp_char=strchr(node_name,':'))
			{
				name_length=temp_char-node_name;
			}
			else
			{
				name_length=strlen(node_name);
			}
		}
#endif /* defined (VMS) */
		if (ALLOCATE(return_struct->name,char,name_length+1))
		{
#if defined (UNIX)
			strcpy(return_struct->name,local_information.nodename);
#endif /* UNIX */
#if defined (VMS)
			strncpy(return_struct->name,node_name,name_length);
#endif /* VMS */
			(return_struct->name)[name_length]='\0';
			return_struct->type=MACHINE_UNKNOWN;
#if defined (UNIX)
			return_struct->type=MACHINE_UNIX;
#endif /* UNIX */
#if defined (VMS)
			return_struct->type=MACHINE_VMS;
#endif /* VMS */
#if defined (WIN32)
			return_struct->type=MACHINE_WINDOWS;
#endif /* VMS */
			return_struct->num_processors=1;
			return_struct->processor_types=(char **)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Machine_information).  %s",
				"Could not allocate memory name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Machine_information).  %s",
			"Could not allocate memory");
	}
	LEAVE;

	return return_struct;
} /* CREATE(Machine_information) */

int DESTROY(Machine_information)(
	struct Machine_information **machine_information_address)
/*******************************************************************************
LAST MODIFIED : 18 February 1997

DESCRIPTION :
Creates a machine information structure.
???GMH.  Perhaps this could be extended to tell it a machine to interrogate?
==============================================================================*/
{
	struct Machine_information *machine_information;
	int return_code;

	ENTER(DESTROY(Machine_information));
	return_code=0;
	if (machine_information_address&&
		(machine_information=*machine_information_address))
	{
		DEALLOCATE(machine_information->name);
		DEALLOCATE(machine_information_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROYMachine_information).  %s",
			"Invalid arguments");
	}
	LEAVE;

	return return_code;
} /* DESTROY(Machine_information) */

int set_machine_type(struct Parse_state *state,void *machine_type_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Parses the name, and turns it into a type.
==============================================================================*/
{
#if !defined (WINDOWS_DEV_FLAG)
	enum Machine_type *machine_type_address;
	char *current_token,*directory_name,**name_address;
	int file_name_length;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	int return_code;

	ENTER(set_file_name);
	if (state)
	{
#if !defined (WINDOWS_DEV_FLAG)
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (machine_type_address=(enum Machine_type *)machine_type_address_void)
				{
					if (fuzzy_string_compare(current_token,"UNKNOWN"))
					{
						*machine_type_address=MACHINE_UNKNOWN;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						if (fuzzy_string_compare(current_token,"UNIX"))
						{
							*machine_type_address=MACHINE_UNIX;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							if (fuzzy_string_compare(current_token,"VMS"))
							{
								*machine_type_address=MACHINE_VMS;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								if (fuzzy_string_compare(current_token,"WINDOWS"))
								{
									*machine_type_address=MACHINE_WINDOWS;
									return_code=shift_Parse_state(state,1);
								}
								else
								{
									display_message(ERROR_MESSAGE,"Unkown machine type: %s",
										current_token);
									return_code=0;
								}
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_machine_type.  Missing machine_type_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," <UNKNOWN|UNIX|VMS|WINDOWS>");
				if (machine_type_address=(enum Machine_type *)machine_type_address_void)
				{
					switch(*machine_type_address)
					{
						case MACHINE_UNKNOWN:
						{
							display_message(INFORMATION_MESSAGE,"[UNKNOWN]");
						} break;
						case MACHINE_UNIX:
						{
							display_message(INFORMATION_MESSAGE,"[UNIX]");
						} break;
						case MACHINE_VMS:
						{
							display_message(INFORMATION_MESSAGE,"[VMS]");
						} break;
						case MACHINE_WINDOWS:
						{
							display_message(INFORMATION_MESSAGE,"[WINDOWS]");
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"set_machine_type.  Invalid machine type");
						}
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_machine_type.  Missing file name");
			display_parse_state_location(state);
			return_code=0;
		}
#else
		return_code=0;
#endif /* !defined (WINDOWS_DEV_FLAG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_machine_type.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_machine_type */

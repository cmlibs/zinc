/*******************************************************************************
FILE : perl_interpreter.c

LAST MODIFIED : 12 June 2003

DESCRIPTION :
Provides an interface between cmiss and a Perl interpreter.
==============================================================================*/

#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "perl_interpreter.h"

static int interpreter_display_message(enum Message_type message_type,
	char *format, ... )
/*******************************************************************************
LAST MODIFIED : 26 March 2003

DESCRIPTION :
The default interpreter_display_message_function.
==============================================================================*/
{
	int return_code;
	va_list ap;

	va_start(ap,format);
	return_code=vfprintf(stderr,format,ap);
	va_end(ap);
	fprintf(stderr,"\n");

	return (return_code);
} /* interpreter_display_message */

static Interpreter_display_message_function *display_message_function =
   interpreter_display_message;

static char *interpreter_duplicate_string(char *source_string, size_t length)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION :
Returns an allocated copy of <source_string>, or NULL in case of error.  If
<length> is greater than zero than this is the maximum number of characters
copied and the NULL termination is added after that length.
==============================================================================*/
{
	char *copy_of_string;

	if (source_string)
	{
		if (length)
		{
			/* Can't use ALLOCATE as this library is used by CM as well */
			if (copy_of_string = (char *)malloc(length+1))
			{
				strncpy(copy_of_string,source_string,length);
				copy_of_string[length] = 0;
			}
			else
			{
				(*display_message_function)(ERROR_MESSAGE,"interpreter_duplicate_string.  "
					 "Not enough memory");
			}
		}
		else
		{
			if (copy_of_string = (char *)malloc(strlen(source_string)+1))
			{
				strcpy(copy_of_string,source_string);
			}
			else
			{
				(*display_message_function)(ERROR_MESSAGE,"interpreter_duplicate_string.  "
					 "Not enough memory");
			}
		}
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_duplicate_string.  "
			 "Invalid argument(s)");
		copy_of_string=(char *)NULL;
	}

	return (copy_of_string);
} /* interpreter_duplicate_string */

void interpreter_destroy_string_(char *string)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION :
Frees the memory associated with a string allocated by the interpreter.
==============================================================================*/
{
	if (string)
	{
		free(string);
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_duplicate_string.  Invalid argument(s)");
	}
} /* interpreter_duplicate_string */

void create_interpreter_(int argc, char **argv, const char *initial_comfile, int *status)
/*******************************************************************************
LAST MODIFIED : 24 July 2001

DESCRIPTION:
We already are running perl - do nothing
==============================================================================*/
{
  int return_code;

  return_code = 1;


  *status = return_code;

}

void destroy_interpreter_(int *status)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
   *status = 1;
}

void interpreter_set_display_message_function_(Interpreter_display_message_function *function,
	int *status)
/*******************************************************************************
LAST MODIFIED : 26 March 2003

DESCRIPTION:
Sets the function that will be called whenever the Interpreter wants to report
information.
==============================================================================*/
{
	 int return_code;

	 return_code = 1;


	 *status = return_code;
}

void redirect_interpreter_output_(int *status)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION:
==============================================================================*/
{
  int return_code;

  return_code = 1;

  *status = return_code;
}

void interpret_command_(char *command_string, void *user_data, int *quit,
  execute_command_function_type execute_command_function, int *status)
/*******************************************************************************
LAST MODIFIED : 19 May 2000

DESCRIPTION:
Takes a <command_string>, processes this through the Perl interpreter.
==============================================================================*/
{
	char *escaped_command, *new_pointer, *old_pointer, *quote_pointer,
		*slash_pointer, *wrapped_command;
	int escape_symbols, return_code;
	STRLEN n_a;
	dSP ;
 
	ENTER ;
	SAVETMPS;
	
	if (command_string)
	{
		PUSHMARK(sp) ;

		return_code = 1;

		escape_symbols = 0;
		if ((quote_pointer = strchr (command_string, '\'')) ||
			(slash_pointer = strchr (command_string, '\\')))
		{
			/* Count how many things we are going to escape */
			quote_pointer = command_string;
			while (quote_pointer = strchr (quote_pointer, '\\'))
			{
				quote_pointer++;
				escape_symbols++;
			}
			quote_pointer = command_string;
			while (quote_pointer = strchr (quote_pointer, '\''))
			{
				quote_pointer++;
				escape_symbols++;
			}
		}

		if (wrapped_command = (char *)malloc(strlen(command_string) + 
				 escape_symbols + 100))
		{
			/* Escape any 's in the string */
			if (escape_symbols)
			{
				if (escaped_command = (char *)malloc(strlen(command_string) + 
						 escape_symbols + 10))
				{
					slash_pointer = strchr (command_string, '\\');
					new_pointer = escaped_command;
					old_pointer = command_string;
					strcpy(new_pointer, old_pointer);
					while (slash_pointer)
					{
						new_pointer += slash_pointer - old_pointer;
						old_pointer = slash_pointer;
						*new_pointer = '\\';
						new_pointer++;
					  
						strcpy(new_pointer, old_pointer);

						slash_pointer = strchr (slash_pointer + 1, '\\');
					}
					strcpy(wrapped_command, escaped_command);
					new_pointer = escaped_command;
					old_pointer = wrapped_command;
					quote_pointer = strchr (wrapped_command, '\'');
					while (quote_pointer)
					{
						new_pointer += quote_pointer - old_pointer;
						old_pointer = quote_pointer;
						*new_pointer = '\\';
						new_pointer++;
					  
						strcpy(new_pointer, old_pointer);

						quote_pointer = strchr (quote_pointer + 1, '\'');
					}
					sprintf(wrapped_command, "Perl_cmiss::execute_command('%s')",
						escaped_command);

					free (escaped_command);
				}
				else
				{
					(*display_message_function)(ERROR_MESSAGE,"cmiss_perl_execute_command.  "
						"Unable to allocate escaped_string");
					return_code=0;
				}
			}
			else
			{
				sprintf(wrapped_command, "Perl_cmiss::execute_command('%s')",
					command_string);
			}
#if defined (CMISS_DEBUG)
			printf("cmiss_perl_execute_command: %s\n", wrapped_command);
#endif /* defined (CMISS_DEBUG) */

			perl_eval_pv(wrapped_command, FALSE);

			if (SvTRUE(ERRSV))
			{
				(*display_message_function)(ERROR_MESSAGE,
					"%s", SvPV(ERRSV, n_a)) ;
				POPs ;
				return_code = 0;
			}

			*quit = 0;
 
			/*  This command needs to get the correct response from a 
				 partially complete command before it is useful
				 if (!SvTRUE(cvrv))
				 {
				 (*display_message_function)(ERROR_MESSAGE,
				 "Unable to compile command: %s\n", wrapped_command) ;
				 POPs ;
				 }*/

			free (wrapped_command);
		}
		else
		{
			(*display_message_function)(ERROR_MESSAGE,"interpret_command.  "
				"Unable to allocate wrapped_string");
			return_code=0;
		}
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpret_command.  "
			"Missing command_data");
		return_code=0;
	}
	
	FREETMPS ;
	LEAVE ;	
	
	*status = return_code;
} /* interpret_command_ */

void interpreter_evaluate_integer_(char *expression, int *result, int *status)
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION:
Use the perl_interpreter to evaluate the given string <expression> and return 
its value as an integer <result>.  If the string <expression> does not evaluate
as an integer then <status> will be set to zero.
==============================================================================*/
{
	int return_code;

	STRLEN n_a;
	dSP ;
	SV *sv_result;

	return_code = 1;

	ENTER ;
	SAVETMPS;
	
	if (expression && result && status)
	{
				
		sv_result = perl_eval_pv(expression, FALSE);

		if (SvTRUE(ERRSV))
		{
			(*display_message_function)(ERROR_MESSAGE,
				"%s", SvPV(ERRSV, n_a)) ;
			POPs ;
			return_code = 0;
		}
		else
		{
			if (SvIOK(sv_result))
			{
				*result = SvIV(sv_result);
				return_code = 1;
			}
			else
			{
				(*display_message_function)(ERROR_MESSAGE,"interpreter_evaluate_integer.  "
					"String \"%s\" does not evaluate to an integer.", expression);
				return_code = 0;
			}
		}
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_evaluate_integer.  "
			"Invalid arguments.") ;
		return_code = 0;
	}
		 
	FREETMPS ;
	LEAVE ;

	*status = return_code;
} /* interpreter_evaluate_integer_ */

void interpreter_set_integer_(char *variable_name, int *value, int *status)
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
	int return_code;

	SV *sv_variable;

	return_code = 1;

	ENTER ;
	SAVETMPS;
	
	if (variable_name && value && status)
	{
		sv_variable = perl_get_sv(variable_name, TRUE);
		sv_setiv(sv_variable, *value);
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_set_integer.  "
			"Invalid arguments.") ;
		return_code = 0;
	}
	
	FREETMPS ;
	LEAVE ;
 
	*status = return_code;
} /* interpreter_set_integer_ */

void interpreter_evaluate_double_(char *expression, double *result, int *status)
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION:
Use the perl_interpreter to evaluate the given string <expression> and return 
its value as an double <result>.  If the string <expression> does not evaluate
as an double then <status> will be set to zero.
==============================================================================*/
{
	int return_code;

	STRLEN n_a;
	dSP ;
	SV *sv_result;
 
	return_code = 1;

	ENTER ;
	SAVETMPS;

	if (expression && result && status)
	{
		sv_result = perl_eval_pv(expression, FALSE);

		if (SvTRUE(ERRSV))
		{
			(*display_message_function)(ERROR_MESSAGE,
				"%s", SvPV(ERRSV, n_a)) ;
			POPs ;
			return_code = 0;
		}
		else
		{
			if (SvNOK(sv_result))
			{
				*result = SvNV(sv_result);
				return_code = 1;
			}
			else
			{
				(*display_message_function)(ERROR_MESSAGE,"interpreter_evaluate_double.  "
					"String \"%s\" does not evaluate to a double.", expression);
				return_code = 0;
			}
		}
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_evaluate_double.  "
			"Invalid arguments.") ;
		return_code = 0;
	}

	FREETMPS ;
	LEAVE ;

	*status = return_code;
} /* interpreter_evaluate_double_ */

void interpreter_set_double_(char *variable_name, double *value, int *status)
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
	int return_code;

	SV *sv_variable;

	return_code = 1;

	ENTER ;
	SAVETMPS;
	
	if (variable_name && value && status)
	{
		sv_variable = perl_get_sv(variable_name, TRUE);
		sv_setnv(sv_variable, *value);
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_set_double.  "
			"Invalid arguments.") ;
		return_code = 0;
	}
 
	FREETMPS ;
	LEAVE ;

	*status = return_code;
} /* interpreter_set_double_ */

void interpreter_evaluate_string_(char *expression, char **result, int *status)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION:
Use the perl_interpreter to evaluate the given string <expression> and return 
its value as an string in <result>.  The string is allocated and it is up to 
the calling routine to release the string with Interpreter_destroy_string when
it is done.  If the string <expression> does not evaluate
as an string then <status> will be set to zero and <*result> will be NULL.
==============================================================================*/
{
	char *internal_string;
	int return_code;

	STRLEN n_a, string_length;
	dSP ;
	SV *sv_result;

	return_code = 1;

	*result = (char *)NULL;
	ENTER ;
	SAVETMPS;

	if (expression && result && status)
	{
		sv_result = perl_eval_pv(expression, FALSE);

		if (SvTRUE(ERRSV))
		{
			(*display_message_function)(ERROR_MESSAGE,
				"%s", SvPV(ERRSV, n_a)) ;
			POPs ;
			return_code = 0;
		}
		else
		{
			if (SvPOK(sv_result))
			{
				internal_string = SvPV(sv_result, string_length);
				if (*result = interpreter_duplicate_string(internal_string, string_length))
				{
					return_code = 1;
				}
				else
				{
					return_code = 0;
				}
			}
			else
			{
				(*display_message_function)(ERROR_MESSAGE,"interpreter_evaluate_string.  "
					"String \"%s\" does not evaluate to a string.", expression);
				return_code = 0;
			}
		}
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_evaluate_string.  "
			"Invalid arguments.") ;
		return_code = 0;
	}
		 
	FREETMPS ;
	LEAVE ;

	*status = return_code;
} /* interpreter_evaluate_string_ */

void interpreter_set_string_(char *variable_name, char *value, int *status)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
	int return_code;

	SV *sv_variable;

	return_code = 1;

	ENTER ;
	SAVETMPS;

	if (variable_name && value && status)
	{
		sv_variable = perl_get_sv(variable_name, TRUE);
		sv_setpv(sv_variable, value);
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_set_string.  "
			"Invalid arguments.") ;
		return_code = 0;
	}

	FREETMPS ;
	LEAVE ;

	*status = return_code;
} /* interpreter_set_string_ */

void interpreter_set_pointer_(char *variable_name, char *class, void *value,
	 int *status)
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value> and 
sets the class of that variable to be <class>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
	int return_code;

	SV *sv_variable;

	return_code = 1;

	ENTER ;
	SAVETMPS;

	if (variable_name && value && status)
	{
		sv_variable = perl_get_sv(variable_name, TRUE);
		sv_setref_pv(sv_variable, class, value);
	}
	else
	{
		(*display_message_function)(ERROR_MESSAGE,"interpreter_set_string.  "
			"Invalid arguments.") ;
		return_code = 0;
	}

	FREETMPS ;
	LEAVE ;

	*status = return_code;
} /* interpreter_set_string_ */

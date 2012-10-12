#include <Python.h>
#include "perl_interpreter.h"

void interpreter_destroy_string_(char *string)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION :
Frees the memory associated with a string allocated by the interpreter.
==============================================================================*/
{
} /* interpreter_duplicate_string */

void create_interpreter_(int argc, char **argv, const char *initial_comfile, int *status)
/*******************************************************************************
LAST MODIFIED : 24 July 2001

DESCRIPTION:
Creates the interpreter for processing commands.
==============================================================================*/
{
}

void destroy_interpreter_(int *status)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
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
}

void redirect_interpreter_output_(int *status)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION:
This redirects the output from stdout to a pipe so that the handle_output
routine can write this to the command window.
==============================================================================*/
{
}

void interpret_command_(char *command_string, void *user_data, int *quit,
  execute_command_function_type execute_command_function, int *status)
/*******************************************************************************
LAST MODIFIED : 19 May 2000

DESCRIPTION:
Takes a <command_string>, processes this through the Perl interpreter.
==============================================================================*/
{
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
} /* interpreter_evaluate_integer_ */

void interpreter_set_integer_(char *variable_name, int *value, int *status)
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
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
} /* interpreter_evaluate_double_ */

void interpreter_set_double_(char *variable_name, double *value, int *status)
/*******************************************************************************
LAST MODIFIED : 6 September 2000

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
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
} /* interpreter_evaluate_string_ */

void interpreter_set_string_(char *variable_name, char *value, int *status)
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION:
Sets the value of the scalar variable cmiss::<variable_name> to be <value>.
To override the cmiss:: package specify the full name in the string.
==============================================================================*/
{
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
} /* interpreter_set_string_ */

static PyMethodDef CmissInterpreter_methods[] = {
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initInterpreter(void) 
{
	Py_InitModule("Interpreter", CmissInterpreter_methods);
}

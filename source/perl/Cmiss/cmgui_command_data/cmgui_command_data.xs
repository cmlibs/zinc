#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "command/cmiss.h"
#include "typemap.h"

MODULE = Cmiss::cmgui_command_data		PACKAGE = Cmiss::cmgui_command_data		PREFIX = Cmiss_cmgui_command_data_

PROTOTYPES: DISABLE

Cmiss::cmgui_command_data
create(argv)
   AV *argv;
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
	   {
	      char **c_argv;
	      int argc, i;
			SV **elem;

			argc = av_len(argv) + 1;
			New(0, c_argv, argc, char *);
			for (i = 0 ; i < argc ; i++)
			{
				elem = av_fetch(argv, i, 0);
				c_argv[i] = SvPV(*elem, PL_na);
			}
			RETVAL=CREATE(Cmiss_command_data)(argc, c_argv, "cmgui perl");
			Safefree(c_argv);
	   }
	OUTPUT:
		RETVAL

int
destroy(Cmiss::cmgui_command_data value)
	CODE:
		{
			struct Cmiss_command_data *temp_value;

			temp_value=value;
			RETVAL=DESTROY(Cmiss_command_data)(&temp_value);
		}
	OUTPUT:
		RETVAL

int
execute_command(Cmiss::cmgui_command_data cmgui_command_data, char *name)
	CODE:
	{
	   int quit = 0;
		execute_command(name, (void *)cmgui_command_data, &quit, &RETVAL);
	}
	OUTPUT:
	RETVAL
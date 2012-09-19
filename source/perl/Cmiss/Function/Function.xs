#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function.h"
#include "typemap.h"

MODULE = Cmiss::Function		PACKAGE = Cmiss::Function		PREFIX = Cmiss_function_

PROTOTYPES: DISABLE

int
DESTROY(Cmiss::Function function)
	CODE:
		{
			Cmiss_function_id temp_function;

			temp_function=function;
			RETVAL=Cmiss_function_destroy(&temp_function);
		}
	OUTPUT:
		RETVAL

SV *
string_convert_xs(Cmiss::Function function)
	CODE:
		RETVAL=(SV *)NULL;
		{
			char *string_representation;

			string_representation=(char *)NULL;
			if (Cmiss_function_get_string_representation(function,
				&string_representation))
			{
				RETVAL=newSVpv(string_representation,0);
				free(string_representation);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
input_xs(Cmiss::Function function)
	CODE:
		RETVAL=Cmiss_function_input(function);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
output_xs(Cmiss::Function function)
	CODE:
		RETVAL=Cmiss_function_output(function);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

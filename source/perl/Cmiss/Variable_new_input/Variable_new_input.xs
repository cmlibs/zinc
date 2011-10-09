#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_variable_new.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new_input		PACKAGE = Cmiss::Variable_new_input		PREFIX = Cmiss_variable_new_input_

PROTOTYPES: DISABLE

int
DESTROY(Cmiss::Variable_new_input input)
	CODE:
		{
			Cmiss_variable_new_input_id temp_input;

			temp_input=input;
			RETVAL=Cmiss_variable_new_input_destroy(&temp_input);
		}
	OUTPUT:
		RETVAL

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_variable_exclusion.h"
#include "typemap.h"

MODULE = Cmiss::Function_variable::Exclusion  PACKAGE = Cmiss::Function_variable::Exclusion  PREFIX = Cmiss_function_variable_exclusion_

PROTOTYPES: DISABLE

Cmiss::Function_variable
new_xs(Cmiss::Function_variable universe,Cmiss::Function_variable exclusion)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function_variable structure).  This means that don't need to worry
			about ACCESSing for Perl assignment/copy,
			$cmiss_variable_2=$cmiss_variable_1, because this increments the reference
			count for the stash (DESTROY is called when the stash reference count gets
			to zero) */
		RETVAL=Cmiss_function_variable_exclusion_create(universe,exclusion);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

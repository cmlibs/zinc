#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_gradient.h"
#include "typemap.h"

MODULE = Cmiss::Function::Gradient  PACKAGE = Cmiss::Function::Gradient  PREFIX = Cmiss_function_gradient_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable dependent_variable, \
	Cmiss::Function_variable independent_variable)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_function_2=$cmiss_function_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		RETVAL=Cmiss_function_gradient_create(dependent_variable,
			independent_variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

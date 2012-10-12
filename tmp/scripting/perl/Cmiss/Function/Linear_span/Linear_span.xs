#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_linear_span.h"
#include "typemap.h"

MODULE = Cmiss::Function::Linear_span  PACKAGE = Cmiss::Function::Linear_span  PREFIX = Cmiss_function_linear_span_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable spanned_variable, \
	Cmiss::Function_variable spanning_variable)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_function_2=$cmiss_function_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		RETVAL=Cmiss_function_linear_span_create(spanned_variable,
			spanning_variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

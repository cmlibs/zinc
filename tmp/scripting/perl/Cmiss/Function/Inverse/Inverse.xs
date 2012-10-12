#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_inverse.h"
#include "typemap.h"

MODULE = Cmiss::Function::Inverse  PACKAGE = Cmiss::Function::Inverse  PREFIX = Cmiss_function_inverse_

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
		RETVAL=Cmiss_function_inverse_create(dependent_variable,
			independent_variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
independent_xs(Cmiss::Function::Inverse function_inverse)
	CODE:
		RETVAL=Cmiss_function_inverse_independent(function_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
step_tolerance_xs(Cmiss::Function::Inverse function_inverse)
	CODE:
		RETVAL=Cmiss_function_inverse_step_tolerance(function_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
value_tolerance_xs(Cmiss::Function::Inverse function_inverse)
	CODE:
		RETVAL=Cmiss_function_inverse_value_tolerance(function_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
maximum_iterations_xs(Cmiss::Function::Inverse function_inverse)
	CODE:
		RETVAL=Cmiss_function_inverse_maximum_iterations(function_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
dependent_estimate_xs(Cmiss::Function::Inverse function_inverse)
	CODE:
		RETVAL=Cmiss_function_inverse_dependent_estimate(function_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_integral.h"
#include "typemap.h"

MODULE = Cmiss::Function::Integral  PACKAGE = Cmiss::Function::Integral  PREFIX = Cmiss_function_integral_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable integrand_output, \
	Cmiss::Function_variable integrand_input, \
	Cmiss::Function_variable independent_output, \
	Cmiss::Function_variable independent_input, \
	Cmiss::Region domain,char *quadrature_scheme)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_function_2=$cmiss_function_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		RETVAL=Cmiss_function_integral_create(integrand_output,integrand_input,
			independent_output,independent_input,domain,quadrature_scheme);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

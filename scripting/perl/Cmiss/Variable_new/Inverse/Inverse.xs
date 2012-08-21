#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_inverse.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Inverse  PACKAGE = Cmiss::Variable_new::Inverse  PREFIX = Cmiss_variable_new_inverse_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(Cmiss::Variable_new_input dependent_variable, \
	Cmiss::Variable_new independent_variable)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		RETVAL=Cmiss_variable_new_inverse_create(dependent_variable,
			independent_variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_independent_xs(Cmiss::Variable_new::Inverse variable_inverse)
	CODE:
		RETVAL=Cmiss_variable_new_input_inverse_independent(variable_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_step_tolerance_xs(Cmiss::Variable_new::Inverse variable_inverse)
	CODE:
		RETVAL=Cmiss_variable_new_input_inverse_step_tolerance(variable_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_value_tolerance_xs(Cmiss::Variable_new::Inverse variable_inverse)
	CODE:
		RETVAL=Cmiss_variable_new_input_inverse_value_tolerance(variable_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_maximum_iterations_xs(Cmiss::Variable_new::Inverse variable_inverse)
	CODE:
		RETVAL=Cmiss_variable_new_input_inverse_maximum_iterations(
			variable_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_dependent_estimate_xs(Cmiss::Variable_new::Inverse variable_inverse)
	CODE:
		RETVAL=Cmiss_variable_new_input_inverse_dependent_estimate(
			variable_inverse);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

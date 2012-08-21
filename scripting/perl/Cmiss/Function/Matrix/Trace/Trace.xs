#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_trace.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Trace		PACKAGE = Cmiss::Function::Matrix::Trace		PREFIX = Cmiss_function_matrix_trace_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable matrix_variable)
	CODE:
		RETVAL=Cmiss_function_matrix_trace_create(matrix_variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

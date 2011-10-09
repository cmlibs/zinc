#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_transpose.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Transpose		PACKAGE = Cmiss::Function::Matrix::Transpose		PREFIX = Cmiss_function_matrix_transpose_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable matrix_variable)
	CODE:
		RETVAL=Cmiss_function_matrix_transpose_create(matrix_variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

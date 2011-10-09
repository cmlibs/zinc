#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_resize.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Resize		PACKAGE = Cmiss::Function::Matrix::Resize		PREFIX = Cmiss_function_matrix_resize_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable matrix_variable,unsigned int number_of_columns)
	CODE:
		RETVAL=Cmiss_function_matrix_resize_create(matrix_variable,
			number_of_columns);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

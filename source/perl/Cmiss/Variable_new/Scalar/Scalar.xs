#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_variable_new_basic.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Scalar		PACKAGE = Cmiss::Variable_new::Scalar		PREFIX = Cmiss_variable_new_scalar_

PROTOTYPES: DISABLE

Cmiss::Variable_new
create(char *name,Scalar value)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable_new structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_1=$cmiss_variable_2,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=Cmiss_variable_new_scalar_create(name,value))
		{
			/*???DB.  ACCESSing? */
		}
	OUTPUT:
		RETVAL

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_identity.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Identity  PACKAGE = Cmiss::Variable::Identity  PREFIX = Cmiss_variable_identity_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name,Cmiss::Variable variable)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		RETVAL=CREATE(Cmiss_variable_identity)(name,variable);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

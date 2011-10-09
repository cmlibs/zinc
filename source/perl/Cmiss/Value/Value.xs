#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/cmiss_value.h"
#include "computed_variable/computed_value.h"
#include "typemap.h"

MODULE = Cmiss::Value		PACKAGE = Cmiss::Value		PREFIX = Cmiss_value_

PROTOTYPES: DISABLE

Cmiss::Value
create()
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(RETVAL);
		}
	OUTPUT:
		RETVAL

int
DESTROY(Cmiss::Value value)
	CODE:
		{
			struct Cmiss_value *temp_value;

			temp_value=value;
			RETVAL=DEACCESS(Cmiss_value)(&temp_value);
		}
	OUTPUT:
		RETVAL

NO_OUTPUT int
Cmiss_value_get_string(IN Cmiss::Value value, OUTLIST char *string)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;


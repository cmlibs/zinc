#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "computed_variable/computed_value.h"
#include "typemap.h"

MODULE = Cmiss::Value::FE_value		PACKAGE = Cmiss::Value::FE_value		PREFIX = Cmiss_value_FE_value_

PROTOTYPES: DISABLE

Cmiss::Value
create(fe_value)
	FE_value fe_value
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(RETVAL);
			if (!Cmiss_value_FE_value_set_type(RETVAL,fe_value))
			{
				DEACCESS(Cmiss_value)(&RETVAL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

FE_value
get_type(Cmiss::Value cmiss_value)
	CODE:
		if (!Cmiss_value_FE_value_get_type(cmiss_value,&RETVAL))
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

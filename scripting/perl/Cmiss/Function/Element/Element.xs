#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Function::Element  PACKAGE = Cmiss::Function::Element  PREFIX = Cmiss_function_element_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Element element)
	CODE:
		{
			RETVAL=Cmiss_function_element_create(element);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

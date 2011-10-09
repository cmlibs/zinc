#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_coordinates.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Spheroidal_coordinates_focus  PACKAGE = Cmiss::Variable::Spheroidal_coordinates_focus  PREFIX = Cmiss_variable_spheroidal_coordinates_focus_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		RETVAL=CREATE(Cmiss_variable_spheroidal_coordinates_focus)(name);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

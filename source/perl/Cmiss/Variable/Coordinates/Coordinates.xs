#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "computed_variable/computed_variable_coordinates.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Coordinates  PACKAGE = Cmiss::Variable::Coordinates  PREFIX = Cmiss_variable_coordinates_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name=(char *)NULL,int dimension=0)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
			name))
		{
			ACCESS(Cmiss_variable)(RETVAL);
			if (!Cmiss_variable_coordinates_set_type(RETVAL,dimension))
			{
				DEACCESS(Cmiss_variable)(&RETVAL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

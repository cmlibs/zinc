#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Nodal_value  PACKAGE = Cmiss::Variable::Nodal_value  PREFIX = Cmiss_variable_nodal_value_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name,Cmiss::Variable::Finite_element fe_variable,Cmiss::Node node,char *type,int version)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (name&&fe_variable)
		{
			enum FE_nodal_value_type value_type;
			int value_type_valid;

			value_type_valid=0;
			if (!strcmp("all",type))
			{
				value_type=FE_NODAL_UNKNOWN;
				value_type_valid=1;
			}
			else if (!strcmp("value",type))
			{
				value_type=FE_NODAL_VALUE;
				value_type_valid=1;
			}
			else if (!strcmp("d/ds1",type))
			{
				value_type=FE_NODAL_D_DS1;
				value_type_valid=1;
			}
			else if (!strcmp("d/ds2",type))
			{
				value_type=FE_NODAL_D_DS2;
				value_type_valid=1;
			}
			else if (!strcmp("d/ds3",type))
			{
				value_type=FE_NODAL_D_DS3;
				value_type_valid=1;
			}
			else if (!strcmp("d2/ds1ds2",type))
			{
				value_type=FE_NODAL_D2_DS1DS2;
				value_type_valid=1;
			}
			else if (!strcmp("d2/ds1ds3",type))
			{
				value_type=FE_NODAL_D2_DS1DS3;
				value_type_valid=1;
			}
			else if (!strcmp("d2/ds2ds3",type))
			{
				value_type=FE_NODAL_D2_DS2DS3;
				value_type_valid=1;
			}
			else if (!strcmp("d3/ds1ds2ds3",type))
			{
				value_type=FE_NODAL_D3_DS1DS2DS3;
				value_type_valid=1;
			}
			if (value_type_valid)
			{
				RETVAL = CREATE(Cmiss_variable_nodal_value)(name,fe_variable,
					node,value_type,version);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "computed_variable/computed_variable_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Finite_element  PACKAGE = Cmiss::Variable::Finite_element  PREFIX = Cmiss_variable_finite_element_

PROTOTYPES: DISABLE

Cmiss::Variable
create(Cmiss::FE_field fe_field,char *component_name=(char *)NULL)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (fe_field)
		{
			char *name;
			int component_number;

			name=(char *)NULL;
			if (GET_NAME(FE_field)(fe_field,&name)&&(RETVAL=CREATE(Cmiss_variable)(
				(struct Cmiss_variable_package *)NULL,name)))
			{
				ACCESS(Cmiss_variable)(RETVAL);
				if (name)
				{
					free(name);
				}
				if (component_name)
				{
					component_number=get_FE_field_number_of_components(fe_field);
					if (0<component_number)
					{
						name=(char *)NULL;
						do
						{
							component_number--;
							if (name)
							{
								free(name);
							}
							name=get_FE_field_component_name(fe_field,component_number);
						} while ((component_number>0)&&strcmp(name,component_name));
						if (!name||strcmp(name,component_name))
						{
							DEACCESS(Cmiss_variable)(&RETVAL);
						}
						if (name)
						{
							free(name);
						}
					}
				}
				else
				{
					component_number= -1;
				}
				if (RETVAL)
				{
					if (!Cmiss_variable_finite_element_set_type(RETVAL,fe_field,
						component_number))
					{
						DEACCESS(Cmiss_variable)(&RETVAL);
					}
				}
			}
			else
			{
				if (name)
				{
					free(name);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

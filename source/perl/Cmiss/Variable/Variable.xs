#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_variable.h"
#include "typemap.h"

MODULE = Cmiss::Variable		PACKAGE = Cmiss::Variable		PREFIX = Cmiss_variable_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_1=$cmiss_variable_2,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
			name))
		{
			ACCESS(Cmiss_variable)(RETVAL);
		}
	OUTPUT:
		RETVAL

int
DESTROY(Cmiss::Variable variable)
	CODE:
		{
			struct Cmiss_variable *temp_variable;

			temp_variable=variable;
			RETVAL=DEACCESS(Cmiss_variable)(&temp_variable);
		}
	OUTPUT:
		RETVAL

SV *
get_type(Cmiss::Variable cmiss_variable)
	CODE:
		{
			char *name;

			name=(char *)NULL;
			if (get_name_Cmiss_variable(cmiss_variable,&name))
			{
				RETVAL=newSVpv(name,0);
				free(name);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Value
evaluate(Cmiss::Variable variable,...)
	PPCODE:
		if (1==items%2)
		{
			int i;
			struct Cmiss_variable_value *variable_value;
			struct LIST(Cmiss_variable_value) *values;

			if (values=CREATE_LIST(Cmiss_variable_value)())
			{
				i=1;
				while ((i<items)&&values)
				{
					if (sv_derived_from(ST(i),"Cmiss::Variable")&&
						sv_derived_from(ST(i+1),"Cmiss::Value")&&
						(variable_value=CREATE(Cmiss_variable_value)(
						INT2PTR(Cmiss__Variable,SvIV((SV *)SvRV(ST(i)))),
						INT2PTR(Cmiss__Value,SvIV((SV *)SvRV(ST(i+1)))))))
					{
						if (ADD_OBJECT_TO_LIST(Cmiss_variable_value)(variable_value,values))
						{
							i += 2;
						}
						else
						{
							DESTROY(Cmiss_variable_value)(&variable_value);
							DESTROY_LIST(Cmiss_variable_value)(&values);
						}
					}
					else
					{
						DESTROY_LIST(Cmiss_variable_value)(&values);
					}
				}
				if (values&&(RETVAL=CREATE(Cmiss_value)()))
				{
					ACCESS(Cmiss_value)(RETVAL);
					if (variable_value=CREATE(Cmiss_variable_value)(variable,RETVAL))
					{
						if (!Cmiss_variable_evaluate(variable_value,values))
						{
							DEACCESS(Cmiss_value)(&RETVAL);
						}
						DESTROY(Cmiss_variable_value)(&variable_value);
					}
					else
					{
						DEACCESS(Cmiss_value)(&RETVAL);
					}
				}
				DESTROY_LIST(Cmiss_variable_value)(&values);
			}
		}
		if (RETVAL)
		{
			char *class_string,*type_id_string;

			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			class_string=(char *)NULL;
			if (type_id_string=Cmiss_value_get_type_id_string(RETVAL))
			{
				if (class_string=(char *)malloc(strlen(type_id_string)+15))
				{
					strcpy(class_string,"Cmiss::Value::");
					strcat(class_string,type_id_string);
				}
			}
			if (class_string)
			{
				sv_setref_pv(ST(0), class_string, (void*)RETVAL);
				free(class_string);
			}
			else
			{
				sv_setref_pv(ST(0), "Cmiss::Value", (void*)RETVAL);
			}
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_variable_new.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new		PACKAGE = Cmiss::Variable_new		PREFIX = Cmiss_variable_new_

PROTOTYPES: DISABLE

int
DESTROY(Cmiss::Variable_new variable)
	CODE:
		{
			Cmiss_variable_new_id temp_variable;

			temp_variable=variable;
			RETVAL=Cmiss_variable_new_destroy(&temp_variable);
		}
	OUTPUT:
		RETVAL

SV *
variable_get_string_representation(Cmiss::Variable_new cmiss_variable)
	CODE:
		{
			char *name;

			name=(char *)NULL;
			if (Cmiss_variable_new_get_string_representation(cmiss_variable,&name))
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

Cmiss::Variable_new
evaluate(Cmiss::Variable_new variable,...)
	PPCODE:
		RETVAL=(Cmiss_variable_new_id)NULL;
		if (1==items%2)
		{
			int i;
			Cmiss_variable_new_input_value_list_id values;

			if (values=Cmiss_variable_new_input_value_list_create())
			{
				i=1;
				while ((i<items)&&values)
				{
					if (sv_derived_from(ST(i),"Cmiss::Variable_new_input")&&
						sv_derived_from(ST(i+1),"Cmiss::Variable_new"))
					{
						if (Cmiss_variable_new_input_value_list_add(values,
							INT2PTR(Cmiss__Variable_new_input,SvIV((SV *)SvRV(ST(i)))),
							INT2PTR(Cmiss__Variable_new,SvIV((SV *)SvRV(ST(i+1))))))
						{
							i += 2;
						}
						else
						{
							Cmiss_variable_new_input_value_list_destroy(&values);
						}
					}
					else
					{
						Cmiss_variable_new_input_value_list_destroy(&values);
						Perl_croak(aTHX_ "Evaluate.  Invalid arguments");
					}
				}
				if (values)
				{
					RETVAL=Cmiss_variable_new_evaluate(variable,values);
					Cmiss_variable_new_input_value_list_destroy(&values);
				}
			}
		}
		if (RETVAL)
		{
			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			sv_setref_pv(ST(0), "Cmiss::Variable_new", (void*)RETVAL);
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

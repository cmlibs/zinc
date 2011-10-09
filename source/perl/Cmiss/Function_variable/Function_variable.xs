#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"
#include "typemap.h"

MODULE = Cmiss::Function_variable		PACKAGE = Cmiss::Function_variable		PREFIX = Cmiss_function_variable_

PROTOTYPES: DISABLE

int
DESTROY(Cmiss::Function_variable variable)
	CODE:
		RETVAL=0;
		{
			Cmiss_function_variable_id temp_variable;

			temp_variable=variable;
			RETVAL=Cmiss_function_variable_destroy(&temp_variable);
		}
	OUTPUT:
		RETVAL

SV *
string_convert_xs(Cmiss::Function_variable variable)
	CODE:
		RETVAL=(SV *)NULL;
		{
			char *string_representation;

			string_representation=(char *)NULL;
			if (Cmiss_function_variable_get_string_representation(variable,
				&string_representation))
			{
				RETVAL=newSVpv(string_representation,0);
				free(string_representation);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function
evaluate_xs( \
	Cmiss::Function_variable variable=(Cmiss_function_variable_id)NULL, \
	Cmiss::Function_variable input=(Cmiss_function_variable_id)NULL, \
	Cmiss::Function value=(Cmiss_function_id)NULL)
	PPCODE:
		RETVAL=(Cmiss_function_id)NULL;
		if (variable)
		{
			RETVAL=Cmiss_function_variable_evaluate(variable,input,value);
		}
		else
		{
			Perl_croak(aTHX_ "Evaluate.  Missing variable");
		}
		if (RETVAL)
		{
			char *class_string;
			const char *type_id_string;

			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			class_string=(char *)NULL;
			if ((type_id_string=Cmiss_function_get_type_id_string(RETVAL))&&
				(0==strncmp(type_id_string,"Function_",9)))
			{
				if (class_string=(char *)malloc(strlen(type_id_string)+7))
				{
					strcpy(class_string,"Cmiss::Function::");
					class_string[17]=toupper(type_id_string[9]);
					class_string[18]='\0';
					strcat(class_string,type_id_string+10);
				}
			}
			if (class_string)
			{
				sv_setref_pv(ST(0), class_string, (void*)RETVAL);
				free(class_string);
			}
			else
			{
				sv_setref_pv(ST(0), "Cmiss::Function", (void*)RETVAL);
			}
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

Cmiss::Function
evaluate_derivative_xs( \
	Cmiss::Function_variable variable=(Cmiss_function_variable_id)NULL, \
	AV *independent_variables_array=(AV *)NULL, \
	Cmiss::Function_variable input=(Cmiss_function_variable_id)NULL, \
	Cmiss::Function value=(Cmiss_function_id)NULL)
	PPCODE:
		RETVAL=(Cmiss_function_id)NULL;
		if (variable&&independent_variables_array)
		{
			int i,order;
			IV tmp_IV;
			Cmiss_function_variable_list_id independent_variables;

			order=av_len(independent_variables_array)+1;
			if (0<order)
			{
				if (independent_variables=Cmiss_function_variable_list_create())
				{
					i=0;
					while ((i<order)&&independent_variables)
					{
						if (sv_derived_from(AvARRAY(independent_variables_array)[i],
							"Cmiss::Function_variable"))
						{
							tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
							if (Cmiss_function_variable_list_add(independent_variables,
								INT2PTR(Cmiss__Function_variable,tmp_IV)))
							{
								i++;
							}
							else
							{
								Cmiss_function_variable_list_destroy(&independent_variables);
							}
						}
						else
						{
							Cmiss_function_variable_list_destroy(&independent_variables);
							Perl_croak(aTHX_ "Evaluate_derivative.  "
								"Independent is not a variable");
						}
					}
					if (independent_variables)
					{
						RETVAL=Cmiss_function_variable_evaluate_derivative(variable,
							independent_variables,input,value);
						Cmiss_function_variable_list_destroy(&independent_variables);
					}
				}
			}
			else
			{
				Perl_croak(aTHX_ "Evaluate_derivative.  No independents");
			}
		}
		else
		{
			Perl_croak(aTHX_ "Evaluate_derivative.  "
				"Missing variable or independents");
		}
		if (RETVAL)
		{
			char *class_string;
			const char *type_id_string;

			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			class_string=(char *)NULL;
			if ((type_id_string=Cmiss_function_get_type_id_string(RETVAL))&&
				(0==strncmp(type_id_string,"Function_",9)))
			{
				if (class_string=(char *)malloc(strlen(type_id_string)+7))
				{
					strcpy(class_string,"Cmiss::Function::");
					class_string[17]=toupper(type_id_string[9]);
					class_string[18]='\0';
					strcat(class_string,type_id_string+10);
				}
			}
			if (class_string)
			{
				sv_setref_pv(ST(0), class_string, (void*)RETVAL);
				free(class_string);
			}
			else
			{
				sv_setref_pv(ST(0), "Cmiss::Function", (void*)RETVAL);
			}
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

int
set_value(Cmiss::Function_variable variable,Cmiss::Function value)
	CODE:
		RETVAL=Cmiss_function_variable_set_value(variable,value);
	OUTPUT:
		RETVAL

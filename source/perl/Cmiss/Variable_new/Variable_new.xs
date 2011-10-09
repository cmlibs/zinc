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
			char *class_string;
			const char *type_id_string;

			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			class_string=(char *)NULL;
			if ((type_id_string=Cmiss_variable_new_get_type_id_string(RETVAL))&&
				(0==strncmp(type_id_string,"Variable_",9)))
			{
				if (class_string=(char *)malloc(strlen(type_id_string)+11))
				{
					strcpy(class_string,"Cmiss::Variable_new::");
					class_string[21]=toupper(type_id_string[9]);
					class_string[22]='\0';
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
				sv_setref_pv(ST(0), "Cmiss::Variable_new", (void*)RETVAL);
			}
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

Cmiss::Variable_new
evaluate_derivative_xs( \
	Cmiss::Variable_new variable=(Cmiss_variable_new_id)NULL, \
	AV *independent_variables_array=(AV *)NULL, \
	AV *input_value_array=(AV *)NULL)
	PPCODE:
		RETVAL=(Cmiss_variable_new_id)NULL;
		if (variable&&independent_variables_array)
		{
			int i,number_of_items,order;
			IV tmp_IV;
			Cmiss_variable_new_input_list_id independent_variables;
			Cmiss_variable_new_input_value_list_id values;

			order=av_len(independent_variables_array)+1;
			number_of_items=0;
			if (input_value_array)
			{
				number_of_items=av_len(input_value_array)+1;
			}
			if ((0<order)&&(0==number_of_items%2))
			{
				if (independent_variables=Cmiss_variable_new_input_list_create())
				{
					i=0;
					while ((i<order)&&independent_variables)
					{
						if (sv_derived_from(AvARRAY(independent_variables_array)[i],
							"Cmiss::Variable_new_input"))
						{
							tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
							if (Cmiss_variable_new_input_list_add(independent_variables,
								INT2PTR(Cmiss__Variable_new_input,tmp_IV)))
							{
								i++;
							}
							else
							{
								Cmiss_variable_new_input_list_destroy(&independent_variables);
							}
						}
						else
						{
							Cmiss_variable_new_input_list_destroy(&independent_variables);
							Perl_croak(aTHX_ "Evaluate_derivative.  "
								"Independent variable is not an input");
						}
					}
					if (independent_variables&&
						(values=Cmiss_variable_new_input_value_list_create()))
					{
						i=0;
						while ((i<number_of_items)&&values)
						{
							if (sv_derived_from(
								AvARRAY(input_value_array)[i],"Cmiss::Variable_new_input")&&
								sv_derived_from(AvARRAY(input_value_array)[i+1],
								"Cmiss::Variable_new"))
							{
								if (Cmiss_variable_new_input_value_list_add(values,
									INT2PTR(Cmiss__Variable_new_input,SvIV((SV *)SvRV(
									AvARRAY(input_value_array)[i]))),
									INT2PTR(Cmiss__Variable_new,SvIV((SV *)SvRV(AvARRAY(
									input_value_array)[i+1])))))
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
							}
						}
						if (values)
						{
							RETVAL=Cmiss_variable_new_evaluate_derivative(variable,
								independent_variables,values);
						}
						Cmiss_variable_new_input_value_list_destroy(&values);
					}
					Cmiss_variable_new_input_list_destroy(&independent_variables);
				}
			}
			else
			{
				Perl_croak(aTHX_ "Evaluate_derivative.  "
					"No derivative variables or do not have input-value pairs");
			}
		}
		else
		{
			Perl_croak(aTHX_ "Evaluate_derivative.  "
				"Missing variable or derivative variables");
		}
		if (RETVAL)
		{
			char *class_string;
			const char *type_id_string;

			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			class_string=(char *)NULL;
			if ((type_id_string=Cmiss_variable_new_get_type_id_string(RETVAL))&&
				(0==strncmp(type_id_string,"Variable_",9)))
			{
				if (class_string=(char *)malloc(strlen(type_id_string)+11))
				{
					strcpy(class_string,"Cmiss::Variable_new::");
					class_string[21]=toupper(type_id_string[9]);
					class_string[22]='\0';
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
				sv_setref_pv(ST(0), "Cmiss::Variable_new", (void*)RETVAL);
			}
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

Cmiss::Variable_new
get_input_value(Cmiss::Variable_new variable,Cmiss::Variable_new_input input)
	PPCODE:
		RETVAL=Cmiss_variable_new_get_input_value(variable,input);
		if (RETVAL)
		{
			char *class_string;
			const char *type_id_string;

			EXTEND(SP,1);
			PUSHs(sv_newmortal());
			class_string=(char *)NULL;
			if ((type_id_string=Cmiss_variable_new_get_type_id_string(RETVAL))&&
				(0==strncmp(type_id_string,"Variable_",9)))
			{
				if (class_string=(char *)malloc(strlen(type_id_string)+11))
				{
					strcpy(class_string,"Cmiss::Variable_new::");
					class_string[21]=toupper(type_id_string[9]);
					class_string[22]='\0';
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
				sv_setref_pv(ST(0), "Cmiss::Variable_new", (void*)RETVAL);
			}
			XSRETURN(1);
		}
		else
		{
			XSRETURN_UNDEF;
		}

int
set_input_value(Cmiss::Variable_new variable,Cmiss::Variable_new_input input, \
	Cmiss::Variable_new value)
	CODE:
		RETVAL=Cmiss_variable_new_set_input_value(variable,input,value);
	OUTPUT:
		RETVAL

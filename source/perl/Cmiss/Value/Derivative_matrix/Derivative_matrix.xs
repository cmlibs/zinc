#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "general/debug.h"
#include "typemap.h"

MODULE = Cmiss::Value::Derivative_matrix		PACKAGE = Cmiss::Value::Derivative_matrix		PREFIX = Cmiss_value_Derivative_matrix_

PROTOTYPES: DISABLE

Cmiss::Value
create(Cmiss::Variable dependent_variable,AV *independent_variables_array, \
	AV *matrices_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_value)())
		{
			int i,number_of_matrices,order;
			IV tmp_IV;
			Cmiss_value_id *matrices;
			Cmiss_variable_id *independent_variables;

			ACCESS(Cmiss_value)(RETVAL);
			if (dependent_variable&&independent_variables_array&&
				(0<(order=av_len(independent_variables_array)+1)))
			{
				if (independent_variables=(Cmiss_variable_id *)malloc(order*
					sizeof(Cmiss_variable_id)))
				{
					i=0;
					number_of_matrices=0;
					while ((i<order)&&sv_derived_from(
						AvARRAY(independent_variables_array)[i],"Cmiss::Variable"))
					{
						tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
						independent_variables[i]=INT2PTR(Cmiss__Variable,tmp_IV);
						i++;
						number_of_matrices=2*number_of_matrices+1;
					}
					if (i==order)
					{
						if (matrices_array)
						{
							if ((number_of_matrices==av_len(matrices_array)+1)&&
								(matrices=(Cmiss_value_id *)malloc(number_of_matrices*
								sizeof(Cmiss_value_id))))
							{
								i=0;
								while ((i<number_of_matrices)&&sv_derived_from(
									AvARRAY(matrices_array)[i],"Cmiss::Value::Matrix"))
								{
									tmp_IV=SvIV((SV*)(SvRV(AvARRAY(matrices_array)[i])));
									matrices[i]=INT2PTR(Cmiss__Value__Matrix,tmp_IV);
									i++;
								}
								if (i!=number_of_matrices)
								{
									free(independent_variables);
									DEACCESS(Cmiss_value)(&RETVAL);
								}
							}
							else
							{
								free(independent_variables);
								DEACCESS(Cmiss_value)(&RETVAL);
							}
						}
						else
						{
							matrices=(Cmiss_value_id *)NULL;
						}
						if (RETVAL)
						{
							if (!Cmiss_value_derivative_matrix_set_type(RETVAL,
								dependent_variable,order,independent_variables,matrices))
							{
								free(matrices);
								free(independent_variables);
								DEACCESS(Cmiss_value)(&RETVAL);
							}
						}
					}
					else
					{
						free(independent_variables);
						DEACCESS(Cmiss_value)(&RETVAL);
					}
				}
				else
				{
					DEACCESS(Cmiss_value)(&RETVAL);
				}
			}
			else
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

void
get_type(Cmiss::Value::Derivative_matrix cmiss_value)
	PREINIT:
		AV *av;
		char *key;
		Cmiss_value_id *matrices;
		Cmiss_variable_id dependent_variable,*independent_variables;
		int i,number_of_matrices,offset,order;
		SV *sv;
	PPCODE:
		if (Cmiss_value_derivative_matrix_get_type(cmiss_value,
			&dependent_variable,&order,&independent_variables,&matrices)&&
			dependent_variable&&(0<order)&&independent_variables)
		{
			offset=0;
			EXTEND(SP,2);
			key="dependent";
			PUSHs(sv_2mortal(newSVpv(key,strlen(key))));
			offset++;
			PUSHs(sv_newmortal());
			ACCESS(Cmiss_variable)(dependent_variable);
			sv_setref_pv(ST(offset),"Cmiss::Variable",(void*)dependent_variable);
			offset++;
			EXTEND(SP,2);
			key="independent";
			PUSHs(sv_2mortal(newSVpv(key,strlen(key))));
			offset++;
			av=newAV();
			number_of_matrices=0;
			for (i=0;i<order;i++)
			{
				sv=newSV(0);
				ACCESS(Cmiss_variable)(independent_variables[i]);
				sv_setref_pv(sv,"Cmiss::Variable",(void *)independent_variables[i]);
				av_store(av,i,sv);
				number_of_matrices=2*number_of_matrices+1;
			}
			sv=newRV((SV *)av);
			PUSHs(sv_2mortal(sv));
			offset++;
			if (matrices)
			{
				EXTEND(SP,2);
				key="matrices";
				PUSHs(sv_2mortal(newSVpv(key,strlen(key))));
				offset++;
				av=newAV();
				for (i=0;i<number_of_matrices;i++)
				{
					sv=newSV(0);
					ACCESS(Cmiss_value)(matrices[i]);
					sv_setref_pv(sv,"Cmiss::Value::Matrix",(void *)matrices[i]);
					av_store(av,i,sv);
				}
				sv=newRV((SV *)av);
				PUSHs(sv_2mortal(sv));
				offset++;
			}
			XSRETURN(offset);
		}
		else
		{
			XSRETURN_UNDEF;
		}

Cmiss::Value::Matrix
get_matrix(Cmiss::Value::Derivative_matrix derivative_matrix, \
	AV *independent_variables_array)
	PREINIT:
		Cmiss_variable_id *independent_variables;
		int i,order;
		IV tmp_IV;
	CODE:
		if (independent_variables_array&&
			(0<(order=av_len(independent_variables_array)+1)))
		{
			if (independent_variables=(Cmiss_variable_id *)malloc(order*
				sizeof(Cmiss_variable_id)))
			{
				i=0;
				while ((i<order)&&sv_derived_from(
					AvARRAY(independent_variables_array)[i],"Cmiss::Variable"))
				{
					tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
					independent_variables[i]=INT2PTR(Cmiss__Variable,tmp_IV);
					i++;
				}
				if (i==order)
				{
					Cmiss_value_derivative_matrix_get_matrix(derivative_matrix,order,
						independent_variables,&RETVAL);
				}
				free(independent_variables);
			}
		}
		if (RETVAL)
		{
			ACCESS(Cmiss_value)(RETVAL);
		}
		else
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

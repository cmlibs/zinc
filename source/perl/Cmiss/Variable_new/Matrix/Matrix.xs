#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_variable_new_matrix.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Matrix		PACKAGE = Cmiss::Variable_new::Matrix		PREFIX = Cmiss_variable_new_matrix_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(unsigned int number_of_columns,AV *values_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			Scalar *values;
			unsigned int i,number_of_values,number_of_rows;

			if (values_array&&(0<(number_of_values=av_len(values_array)+1)))
			{
				if (0==number_of_values%number_of_columns)
				{
					number_of_rows=number_of_values/number_of_columns;
					if (values=(Scalar *)malloc(number_of_values*sizeof(Scalar)))
					{
						for (i=0;i<number_of_values;i++)
						{
							values[i]=(Scalar)SvNV(AvARRAY(values_array)[i]);
						}
						RETVAL=Cmiss_variable_new_matrix_create(number_of_rows,
							number_of_columns,values);
						free(values);
					}
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new::Matrix
sub_matrix_xs(Cmiss::Variable_new::Matrix matrix,HV *args=(HV *)NULL)
	PREINIT:
		char *key;
		unsigned int column_high,column_low,number_of_columns,number_of_rows,
			row_high,row_low;
		SV **svp;
	CODE:
		if (matrix&&Cmiss_variable_new_matrix_get_dimensions(matrix,&number_of_rows,
			&number_of_columns))
		{
			row_low=1;
			row_high=number_of_rows;
			column_low=1;
			column_high=number_of_columns;
			if (args)
			{
				key="row_low";
				if (svp=hv_fetch(args,key,strlen(key),FALSE))
				{
					row_low=SvIV(*svp);
					if (row_low<1)
					{
						row_low=1;
					}
				}
				key="row_high";
				if (svp=hv_fetch(args,key,strlen(key),FALSE))
				{
					row_high=SvIV(*svp);
					if (row_high<1)
					{
						row_high=number_of_rows;
					}
				}
				key="column_low";
				if (svp=hv_fetch(args,key,strlen(key),FALSE))
				{
					column_low=SvIV(*svp);
					if (column_low<1)
					{
						column_low=1;
					}
				}
				key="column_high";
				if (svp=hv_fetch(args,key,strlen(key),FALSE))
				{
					column_high=SvIV(*svp);
					if (column_high<1)
					{
						column_high=number_of_columns;
					}
				}
			}
			RETVAL=Cmiss_variable_new_matrix_get_sub_matrix(matrix,row_low,
				row_high,column_low,column_high);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_values_xs(Cmiss::Variable_new::Matrix variable_matrix,AV *indices_array)
	CODE:
		if (variable_matrix)
		{
			unsigned int *column_indices,i,j,number_of_indices,*row_indices;

			if (indices_array&&(number_of_indices=av_len(indices_array)+1))
			{
				if (0==number_of_indices%2)
				{
					number_of_indices /= 2;
					if (row_indices=(unsigned int *)malloc(number_of_indices*
						sizeof(unsigned int)))
					{
						if (column_indices=(unsigned int *)malloc(number_of_indices*
							sizeof(unsigned int)))
						{
							j=0;
							for (i=0;i<number_of_indices;i++)
							{
								row_indices[i]=(unsigned int)SvIV(AvARRAY(indices_array)[j]);
								j++;
								column_indices[i]=(unsigned int)SvIV(AvARRAY(indices_array)[j]);
								j++;
							}
							RETVAL=Cmiss_variable_new_input_matrix_values(variable_matrix,
								number_of_indices,row_indices,column_indices);
							free(column_indices);
						}
						free(row_indices);
					}
				}
			}
			else
			{
				RETVAL=Cmiss_variable_new_input_matrix_values(variable_matrix,0,0,
					(unsigned int *)NULL);
			}
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new
solve_xs(Cmiss::Variable_new::Matrix matrix,Cmiss::Variable_new rhs)
	CODE:
		if (matrix&&rhs)
		{
			RETVAL=Cmiss_variable_new_matrix_solve(matrix,rhs);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

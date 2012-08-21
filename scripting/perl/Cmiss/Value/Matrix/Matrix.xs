#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_value_matrix.h"
#include "computed_variable/computed_value.h"
#include "typemap.h"

MODULE = Cmiss::Value::Matrix		PACKAGE = Cmiss::Value::Matrix		PREFIX = Cmiss_value_matrix_

PROTOTYPES: DISABLE

Cmiss::Value
create(number_of_columns,values_array)
   int number_of_columns
	AV *values_array
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (0<number_of_columns)
		{
			int i,j,k,number_of_values,number_of_rows;
			double *value,*values;
			struct Matrix *matrix;

			if (values_array&&(0<(number_of_values=av_len(values_array)+1)))
			{
				if (0==number_of_values%number_of_columns)
				{
					number_of_rows=number_of_values/number_of_columns;
					if (values=(double *)malloc(number_of_values*
						sizeof(double)))
					{
						value=values;
						for (j=0;j<number_of_values;j++)
						{
							*value=(double)SvNV(AvARRAY(values_array)[j]);
							value++;
						}
						RETVAL = CREATE(Cmiss_value_matrix)(number_of_rows,
							number_of_columns, values);
						free(values);
					}
				}
			}
			else
			{
				RETVAL = CREATE(Cmiss_value_matrix)(/*number_of_rows*/0,
					/*number_of_columns*/0, (double *)NULL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Value::Matrix
get_sub_matrix(Cmiss::Value::Matrix matrix,HV *args=(HV *)NULL)
	PREINIT:
		char *key;
		int column_high,column_low,number_of_columns,number_of_rows,row_high,
			row_low;
		SV **svp;
	CODE:
		if (matrix&&Cmiss_value_matrix_get_dimensions(matrix,&number_of_rows,
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
			if (RETVAL=Cmiss_value_matrix_get_submatrix(matrix,row_low,row_high,
				column_low,column_high))
			{
				ACCESS(Cmiss_value)(RETVAL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

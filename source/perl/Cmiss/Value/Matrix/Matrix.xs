#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "computed_variable/computed_value_matrix.h"
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
		if ((0<number_of_columns)&&(RETVAL=CREATE(Cmiss_value)()))
		{
			int i,j,k,number_of_values,number_of_rows;
			Matrix_value *value,*values;
			struct Matrix *matrix;

			ACCESS(Cmiss_value)(RETVAL);
			if (values_array&&(0<(number_of_values=av_len(values_array)+1)))
			{
				if (0==number_of_values%number_of_columns)
				{
					number_of_rows=number_of_values/number_of_columns;
					if (values=(Matrix_value *)malloc(number_of_values*
						sizeof(Matrix_value)))
					{
						/* swap column fastest to row fastest */
						value=values;
						for (j=0;j<number_of_columns;j++)
						{
							k=j;
							for (i=number_of_rows;i>0;i--)
							{
								*value=(Matrix_value)SvNV(AvARRAY(values_array)[k]);
								value++;
								k += number_of_columns;
							}
						}
						matrix=CREATE(Matrix)("matrix",DENSE,number_of_rows,
							number_of_columns);
						if (!(matrix&&Matrix_set_values(matrix,values,1,number_of_rows,1,
							number_of_columns)&&Cmiss_value_matrix_set_type(RETVAL,matrix)))
						{
							if (matrix)
							{
								DESTROY(Matrix)(&matrix);
							}
							DEACCESS(Cmiss_value)(&RETVAL);
						}
						free(values);
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
			else
			{
				matrix=CREATE(Matrix)("matrix",DENSE,0,0);
				if (!(matrix&&Cmiss_value_matrix_set_type(RETVAL,matrix)))
				{
					if (matrix)
					{
						DESTROY(Matrix)(&matrix);
					}
					DEACCESS(Cmiss_value)(&RETVAL);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
	
SV *
get_string_convert(Cmiss::Value cmiss_value,int max_rows,int max_columns)
	CODE:
		if (RETVAL=newSVpv("[",0))
		{
			int i,j,number_of_columns,number_of_rows;
			Matrix_value *values;
			struct Matrix *matrix;

			if (Cmiss_value_matrix_get_type(cmiss_value,&matrix)&&
				Matrix_get_dimensions(matrix,&number_of_rows,&number_of_columns))
			{
				if ((0<number_of_rows)&&(0<number_of_columns))
				{
					values=(Matrix_value *)malloc(number_of_rows*number_of_columns*
						sizeof(Matrix_value));
					if (values&&Matrix_get_values(matrix,values,1,number_of_rows,1,
						number_of_columns))
					{
						if (number_of_rows<max_rows)
						{
							for (i=0;i<number_of_rows;i++)
							{
								sv_catpvf(RETVAL,"%g",values[i]);
								if (number_of_columns<max_columns)
								{
									for (j=1;j<number_of_columns;j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
								}
								else
								{
									for (j=1;j<max_columns/2;j++)
									{
										sv_catpvf(RETVAL,",%g",
											values[i+j*number_of_rows]);
									}
									sv_catpv(RETVAL,",...");
									for (j=number_of_columns-max_columns/2;j<number_of_columns;
										j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
								}
								if (i+1<number_of_rows)
								{
									sv_catpv(RETVAL,"\n");
								}
							}
						}
						else
						{
							for (i=0;i<max_rows/2;i++)
							{
								sv_catpvf(RETVAL,"%g",values[i]);
								if (number_of_columns<max_columns)
								{
									for (j=1;j<number_of_columns;j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
								}
								else
								{
									for (j=1;j<max_columns/2;j++)
									{
										sv_catpvf(RETVAL,",%g",
											values[i+j*number_of_rows]);
									}
									sv_catpv(RETVAL,",...");
									for (j=number_of_columns-max_columns/2;j<number_of_columns;
										j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
								}
								sv_catpv(RETVAL,"\n");
							}
							sv_catpv(RETVAL,"...\n");
							for (i=number_of_rows-max_rows/2;i<number_of_rows;i++)
							{
								sv_catpvf(RETVAL,"%g",values[i]);
								if (number_of_columns<max_columns)
								{
									for (j=1;j<number_of_columns;j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
								}
								else
								{
									for (j=1;j<max_columns/2;j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
									sv_catpv(RETVAL,",...");
									for (j=number_of_columns-max_columns/2;j<number_of_columns;
										j++)
									{
										sv_catpvf(RETVAL,",%g",values[i+j*number_of_rows]);
									}
								}
								if (i+1<number_of_rows)
								{
									sv_catpv(RETVAL,"\n");
								}
							}
						}
					}
					else
					{
						sv_free(RETVAL);
						RETVAL=0;
					}
					if (values)
					{
						free(values);
					}
				}
				if (RETVAL)
				{
					sv_catpv(RETVAL,"]");
				}
			}
			else
			{
				sv_free(RETVAL);
				RETVAL=0;
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

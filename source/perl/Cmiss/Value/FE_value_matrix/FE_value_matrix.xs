#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include "computed_variable/computed_value.h"
#include "typemap.h"

MODULE = Cmiss::Value::FE_value_matrix		PACKAGE = Cmiss::Value::FE_value_matrix		PREFIX = Cmiss_value_FE_value_matrix_

PROTOTYPES: DISABLE

Cmiss::Value
create(int number_of_columns,AV *fe_values_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if ((0<number_of_columns)&&(RETVAL=CREATE(Cmiss_value)()))
		{
			FE_value *fe_value_matrix;
			int i,number_of_fe_values,number_of_rows;

			ACCESS(Cmiss_value)(RETVAL);
			if (fe_values_array&&(0<(number_of_fe_values=av_len(fe_values_array)+1)))
			{
				if (0==number_of_fe_values%number_of_columns)
				{
					number_of_rows=number_of_fe_values/number_of_columns;
					if (fe_value_matrix=(FE_value *)malloc(number_of_fe_values*
						sizeof(FE_value)))
					{
						for (i=0;i<number_of_fe_values;i++)
						{
							fe_value_matrix[i]=(FE_value)SvNV(AvARRAY(fe_values_array)[i]);
						}
						if (!Cmiss_value_FE_value_matrix_set_type(RETVAL,number_of_rows,
							number_of_columns,fe_value_matrix))
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
					DEACCESS(Cmiss_value)(&RETVAL);
				}
			}
			else
			{
				if (!Cmiss_value_FE_value_matrix_set_type(RETVAL,0,0,(FE_value *)NULL))
				{
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
get_type(Cmiss::Value cmiss_value)
	CODE:
		if (RETVAL=newSVpv("[",0))
		{
			int i,j,number_of_columns,number_of_rows;
			FE_value *fe_value_matrix;

			if (Cmiss_value_FE_value_matrix_get_type(cmiss_value,
				&number_of_rows,&number_of_columns,&fe_value_matrix))
			{
				if ((0<number_of_rows)&&(0<number_of_columns))
				{
					if (number_of_rows<8)
					{
						for (i=0;i<number_of_rows;i++)
						{
							sv_catpvf(RETVAL,"%g",fe_value_matrix[i*number_of_columns]);
							if (number_of_columns<8)
							{
								for (j=1;j<number_of_columns;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
							}
							else
							{
								for (j=1;j<3;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
								sv_catpv(RETVAL,",...");
								for (j=number_of_columns-3;j<number_of_columns;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
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
						for (i=0;i<3;i++)
						{
							sv_catpvf(RETVAL,"%g",fe_value_matrix[i*number_of_columns]);
							if (number_of_columns<8)
							{
								for (j=1;j<number_of_columns;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
							}
							else
							{
								for (j=1;j<3;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
								sv_catpv(RETVAL,",...");
								for (j=number_of_columns-3;j<number_of_columns;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
							}
							sv_catpv(RETVAL,"\n");
						}
						sv_catpv(RETVAL,"...\n");
						for (i=number_of_rows-3;i<number_of_rows;i++)
						{
							sv_catpvf(RETVAL,"%g",fe_value_matrix[i*number_of_columns]);
							if (number_of_columns<8)
							{
								for (j=1;j<number_of_columns;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
							}
							else
							{
								for (j=1;j<3;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
								sv_catpv(RETVAL,",...");
								for (j=number_of_columns-3;j<number_of_columns;j++)
								{
									sv_catpvf(RETVAL,",%g",
										fe_value_matrix[i*number_of_columns+j]);
								}
							}
							if (i+1<number_of_rows)
							{
								sv_catpv(RETVAL,"\n");
							}
						}
					}
				}
				sv_catpv(RETVAL,"]");
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

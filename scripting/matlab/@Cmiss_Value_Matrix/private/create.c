#include "mex.h"
#include "api/cmiss_value_matrix.h"
#include "types.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  Cmiss_value_id cmiss_value;
  const int dims[]={1,1};
  double *values_array, *value, *values;
  int i, j, number_of_columns,number_of_values,number_of_rows;

  /* Check for proper number of arguments. */
  if (nrhs != 1) 
    mexErrMsgTxt("Exactly one argument expected.");
  else if (nlhs != 1) 
    mexErrMsgTxt("Too many output argument requested.");

  /* Set C-style string output_buf to MATLAB mexFunction output*/
  plhs[0] = mxCreateNumericArray(2,dims,mxUINT32_CLASS,mxREAL);

  if (!mxIsDouble(prhs[0]))
    mexErrMsgTxt("Input argument must be a double array.");

  number_of_rows = mxGetM(prhs[0]);
  number_of_columns = mxGetN(prhs[0]);

  cmiss_value = (Cmiss_value_id)NULL;
  if ((0<number_of_columns) && (0<number_of_rows))
  {
	 number_of_values = number_of_rows * number_of_columns;
	 if (values=(double *)malloc(number_of_values*
		sizeof(double)))
	 {
		values_array = (double *)mxGetPr(prhs[0]);
		value = values;
		for (i = 0 ; i < number_of_rows ; i++)
		{
		  for (j = 0 ; j < number_of_columns ; j++)
		  {
			 *value = values_array[j * number_of_rows + i];
			 value++;
		  }
		}
	 }
	 cmiss_value = CREATE(Cmiss_value_matrix)(number_of_rows,
		number_of_columns,values);
	 free(values);
  }
  else
  {
	 cmiss_value = CREATE(Cmiss_value_matrix)(/*number_of_rows*/0,
		/*number_of_columns*/0, (double *)NULL);
  }
  if (cmiss_value)
  {
	 *(PTR_CTYPE *)mxGetData(plhs[0]) = (PTR_CTYPE)cmiss_value;
  }
  else
  {
	 mexErrMsgTxt("Error creating value.");
  }

  return;
}

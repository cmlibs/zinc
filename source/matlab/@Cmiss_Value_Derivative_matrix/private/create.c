#include "mex.h"
#include "api/cmiss_value_derivative_matrix.h"
#include "types.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  Cmiss_value_id *matrices;
  Cmiss_variable_id dependent_variable, *independent_variables;
  const int dims[]={1,1};
  int i, number_of_matrices, order, return_code;
  const mxArray *value_pointer;
  struct Cmiss_value *cmiss_value; 

  /* Check for proper number of arguments. */
  if (nrhs != 3) 
    mexErrMsgTxt("Exactly three arguments expected.");
  else if (nlhs != 1) 
    mexErrMsgTxt("Too many output argument requested.");

  plhs[0] = mxCreateNumericArray(2,dims,mxUINT32_CLASS,mxREAL);

  if (!mxIsClass(prhs[0],"Cmiss_Variable"))
    mexErrMsgTxt("First input argument must be the dependent Cmiss_Variable.");

  if (!(mxIsClass(prhs[1],"Cmiss_Variable") && (mxGetM(prhs[1]) == 1)))
    mexErrMsgTxt("Second input argument must be a list of independent Cmiss_Variables.");

  if (!(mxIsClass(prhs[2],"Cmiss_Value_Matrix") && (mxGetM(prhs[2]) == 1)))
    mexErrMsgTxt("Third input argument must be a list of Cmiss_value_matrices.");

   dependent_variable = (Cmiss_variable_id)(*(PTR_CTYPE *)mxGetData(prhs[0]));

	return_code = 1;
	if (0 < (order = mxGetN(prhs[1])))
	{
	  if (independent_variables=(Cmiss_variable_id *)malloc(order*
		 sizeof(Cmiss_variable_id)))
	  {
		 i=0;
		 number_of_matrices=0;
		 while ((i < order)  && 
			((mxIsClass(prhs[1], "Cmiss_Variable") &&
			  (value_pointer = mxGetField(prhs[1], i, "variable_pointer")))
			|| ((value_pointer = mxGetField(prhs[1], i, "Cmiss_Variable")) &&
			(value_pointer = mxGetField(value_pointer, 0, "variable_pointer")))))
		 {
			independent_variables[i] = (Cmiss_variable_id)(*(PTR_CTYPE *)mxGetData(value_pointer));
			i++;
			number_of_matrices=2*number_of_matrices+1;
		 }
		 if (return_code && (i == order))
		 {
			if ((number_of_matrices==mxGetN(prhs[2])) &&
			  (matrices=(Cmiss_value_id *)malloc(number_of_matrices*
				 sizeof(Cmiss_value_id))))
			{
			  i=0;
			  if (!(mxIsClass(prhs[2], "Cmiss_Value_Matrix")))
				 mexErrMsgTxt("Inorrect matrix array type.");
				 
			  while ((i < number_of_matrices) &&
				 (value_pointer = mxGetField(prhs[2], i, "Cmiss_Value")) &&
				 (value_pointer = mxGetField(value_pointer, 0, "value_pointer")))
			  {
				 matrices[i] = (Cmiss_value_id)(*(PTR_CTYPE *)mxGetData(value_pointer));
				 i++;
			  }
			  if (i != number_of_matrices)
			  {
				 return_code = 0;
				 free(independent_variables);
				 mexErrMsgTxt("Invalid type in matrix array.");
			  }
			}
			else
			{
			  matrices=(Cmiss_value_id *)NULL;
			}
			if (return_code)
			{
			  if (cmiss_value = CREATE(Cmiss_value_derivative_matrix)(
				 dependent_variable, order, independent_variables, number_of_matrices,
				 matrices))
			  {
				 *(PTR_CTYPE *)mxGetData(plhs[0]) = (PTR_CTYPE)cmiss_value;
			  }
			  else
			  {
				 mexErrMsgTxt("Unable to create Cmiss_value_derivative_matrix.");
				 if (matrices)
				 {
					free(matrices);
				 }
				 free(independent_variables);
			  }
			}
		 }
		 else
		 {
			free(independent_variables);
			mexErrMsgTxt("Invalid type in dependent variable array.");
		 }
	  }
	}

  return;
}

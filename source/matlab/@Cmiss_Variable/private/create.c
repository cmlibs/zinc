#include "mex.h"
#include "computed_variable/computed_variable.h"
#include "types.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  char *input_buf;
  const int dims[]={1,1};
  int buflen, status;
  struct Cmiss_variable *variable;

  /* Check for proper number of arguments. */
  if (nrhs != 1) 
    mexErrMsgTxt("One input required.");
  else if (nlhs > 1) 
    mexErrMsgTxt("Too many output arguments.");

  /* Input must be a string. */
  if (mxIsChar(prhs[0]) != 1)
    mexErrMsgTxt("Input must be a string.");

  /* Input must be a row vector. */
  if (mxGetM(prhs[0]) != 1)
    mexErrMsgTxt("Input must be a row vector.");
    
  /* Get the length of the input string. */
  buflen = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;

  /* Allocate memory for input and output strings. */
  input_buf = mxCalloc(buflen, sizeof(char));

  /* Set C-style string output_buf to MATLAB mexFunction output*/
  plhs[0] = mxCreateNumericArray(2,dims,PTR_MATLAB_CLASS,mxREAL);

  /* Copy the string data from prhs[0] into a C string 
   * input_buf. If the string array contains several rows, 
   * they are copied, one column at a time, into one long 
   * string array. */
  status = mxGetString(prhs[0], input_buf, buflen);
  if (status != 0) 
    mexWarnMsgTxt("Not enough space. String is truncated.");

  if (variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL, input_buf))
  {
    *(PTR_CTYPE *)mxGetData(plhs[0]) = (PTR_CTYPE)ACCESS(Cmiss_variable)(variable);
  }

  return;
}

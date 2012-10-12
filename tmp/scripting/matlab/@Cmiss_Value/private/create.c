#include "mex.h"
#include "computed_variable/computed_value.h"
#include "types.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  const int dims[]={1,1};
  struct Cmiss_value *value;

  /* Check for proper number of arguments. */
  if (nrhs != 0) 
    mexErrMsgTxt("No inputs.");
  else if (nlhs != 1) 
    mexErrMsgTxt("Too many output arguments.");

  plhs[0] = mxCreateNumericArray(2,dims,mxUINT32_CLASS,mxREAL);

  if (value=CREATE(Cmiss_value)())
  {
    *(PTR_CTYPE *)mxGetData(plhs[0]) = (PTR_CTYPE)ACCESS(Cmiss_value)(value);
  }

  return;
}

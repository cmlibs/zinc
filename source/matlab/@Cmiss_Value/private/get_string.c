#include "mex.h"
#include "api/cmiss_core.h"
#include "api/cmiss_value.h"
#include "types.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  char *string;
  Cmiss_value_id value;

  /* Check for proper number of arguments. */
  if (nrhs != 1) 
    mexErrMsgTxt("Must specify an input argument to get string from.");
  else if ((1 != mxGetM(prhs[0])) || (1 != mxGetN(prhs[0])))
    mexErrMsgTxt("Argument must be a scalar value.");
  else if (nlhs != 1) 
    mexErrMsgTxt("Too many output arguments.");

  value = (Cmiss_value_id)(*(PTR_CTYPE *)mxGetData(prhs[0]));

  if (Cmiss_value_get_string(value, &string))
  {
	 plhs[0] = mxCreateString(string);
	 Cmiss_deallocate(string);
  }
  else
  {
 	 mexErrMsgTxt("Unable to fetch string.");
  }

  return;
}

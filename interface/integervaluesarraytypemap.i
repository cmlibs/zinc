
%typemap(in) (int numberOfValues, int *values)
{
	/* Check if is a list */
	if (PyList_Check($input)) 
	{
		int i;
		$1 = PyList_Size($input);
		$2 = (int *) malloc(($1)*sizeof(int));
		for (i = 0; i < $1; i++) 
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyInt_Check(o))
				$2[i] = PyInt_AsLong(PyList_GetItem($input,i));
			else 
			{
				PyErr_SetString(PyExc_TypeError,"list must contain long");
				free($2);
				return NULL;
			}
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError,"not a list");
		return NULL;
	}
}

%typemap(freearg) (int numberOfValues, int *values)
{
	free((int*) $2);
}

%typemap(typecheck) (int numberOfValues, int *values) {
$1 = PyList_Check($input) ? 1 : 0;
}

%typemap(in) (int size, int *minimumDivisions) = (int numberOfValues, int *values);
%typemap(freearg) (int size, int *minimumDivisions) = (int numberOfValues, int *values);
%typemap(typecheck) (int size, int *minimumDivisions) = (int numberOfValues, int *values);

%typemap(in) (int size, int *refinementFactors) = (int numberOfValues, int *values);
%typemap(freearg) (int size, int *refinementFactors) = (int numberOfValues, int *values);
%typemap(typecheck) (int size, int *refinementFactors) = (int numberOfValues, int *values);

%typemap(in) (int basisNumberOfNodes, int *localNodeIndexes) = (int numberOfValues, int *values);
%typemap(freearg) (int basisNumberOfNodes, int *localNodeIndexes) = (int numberOfValues, int *values);
%typemap(typecheck) (int basisNumberOfNodes, int *localNodeIndexes) = (int numberOfValues, int *values);

%typemap(argout)(int numberOfValues, int *outValues)
{
	PyObject *o, *o2, *o3, *o4;
	if($1 == 1)
		o = PyInt_FromLong(*$2);
	else
	{
		o = PyList_New($1);
		for (int i = 0 ; i < $1; i++)
		{
			o4 = PyInt_FromLong($2[i]);
			PyList_SetItem(o, i, o4);
		}
	
	}
	if ((!$result) || ($result == Py_None))
	{
		$result = o;
	}
	else 
	{
		if (!PyTuple_Check($result))
		{
			PyObject *o2 = $result;
			$result = PyTuple_New(1);
			PyTuple_SetItem($result,0,o2);
		}
		o3 = PyTuple_New(1);
		PyTuple_SetItem(o3,0,o);
		o2 = $result;
		$result = PySequence_Concat(o2,o3);
		Py_DECREF(o2);
		Py_DECREF(o3);
	}
	delete $2;
}

%typemap(in,numinputs=0) (int *outValues)
{
	$1 = new int[100];
}

%typemap(argout) (int size, int *outRefinementFactors) = (int numberOfValues, int *outValues);
%typemap(in,numinputs=0) (int *outRefinementFactorss) = (int *outValues);

%typemap(argout) (int size, int *outRefinementFactors) = (int numberOfValues, int *outValues);
%typemap(in,numinputs=0) (int *outRefinementFactors) = (int *outValues);


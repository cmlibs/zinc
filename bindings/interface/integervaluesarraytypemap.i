/**
 * integervaluesarraytypemap.i
 * 
 * Swig interface file for integer array.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%typemap(in) (int valuesCount, const int *valuesIn)
{
	/* Check if is a list */
	if (PyInt_Check($input) || PyLong_Check($input))
	{
		$1 = 1;
		$2 = new int[$1];
		PyObject *o = $input;
		if (PyLong_Check(o))
		{
			$2[0] = PyLong_AsLong(o);
		}
		else if (PyInt_Check(o))
		{
			$2[0] = PyInt_AsLong(o);
		}
		else 
		{
			PyErr_SetString(PyExc_TypeError,"value must be a long");
			delete[] $2;
			return NULL;
		}
	}
	else if (PyList_Check($input)) 
	{
		$1 = PyList_Size($input);
		$2 = new int[$1];
		for (int i = 0; i < $1; i++) 
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyInt_Check(o))
			{
				$2[i] = PyInt_AsLong(PyList_GetItem($input,i));
			}
			else 
			{
				PyErr_SetString(PyExc_TypeError,"list must contain long");
				delete[] $2;
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

%typemap(freearg) (int valuesCount, const int *valuesIn)
{
	delete[] $2;
}

// array getter in-handler expects an integer array size only
// and allocates array to accept output; see argout-handler
%typemap(in, numinputs=1) (int valuesCount, int *valuesOut)
{
	if (!PyInt_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting an integer");
		return NULL;
	}
	$1 = PyInt_AsLong($input);
	if ($1 < 0)
	{
		PyErr_SetString(PyExc_ValueError, "Positive integer expected");
		return NULL;
	}
	$2 = new int[$1];
}

%typemap(argout)(int valuesCount, int *valuesOut)
{
	PyObject *o;
	if ($1 == 1)
	{
		o = PyInt_FromLong(*$2);
	}
	else
	{
		o = PyList_New($1);
		for (int i = 0 ; i < $1; i++)
		{
			PyList_SET_ITEM(o, i, PyInt_FromLong($2[i])); // steals reference
		}
	}
	if ((!$result) || ($result == Py_None))
	{
		$result = o;
	}
	else 
	{
		// note: code considers that tuples are supposed to be immutable
		// should only modify if you have only reference to them!
		if (!PyTuple_Check($result))
		{
			PyObject *previousResult = $result;
			$result = PyTuple_New(2);
			PyTuple_SET_ITEM($result, 0, previousResult); // steals reference
			PyTuple_SET_ITEM($result, 1, o); // steals reference
		}
		else
		{
			PyObject *previousResult = $result;
			PyObject *addResult = PyTuple_New(1);
			PyTuple_SET_ITEM(addResult, 0, o); // steals reference
			$result = PySequence_Concat(previousResult, addResult);
			Py_DECREF(previousResult);
			Py_DECREF(addResult);
		}
	}
	delete[] $2;
}

%typemap(in) (int radiusSizesCount, const int *radiusSizesIn) = (int valuesCount, const int *valuesIn);
%typemap(freearg) (int radiusSizesCount, const int *radiusSizesIn) = (int valuesCount, const int *valuesIn);
%typemap(in) (int nodeIndexesCount, const int *nodeIndexesIn) = (int valuesCount, const int *valuesIn);
%typemap(freearg) (int nodeIndexesCount, const int *nodeIndexesIn) = (int valuesCount, const int *valuesIn);
%typemap(in) (int sourceComponentIndexesCount, const int *sourceComponentIndexesIn) = (int valuesCount, const int *valuesIn);
%typemap(freearg) (int sourceComponentIndexesCount, const int *sourceComponentIndexesIn) = (int valuesCount, const int *valuesIn);
%typemap(in) (int identifiersCount, const int *identifiersIn) = (int valuesCount, const int *valuesIn);
%typemap(freearg) (int identifiersCount, const int *identifiersIn) = (int valuesCount, const int *valuesIn);

// ignore int *valuesOut3 on input; it's an output argument only
%typemap(in, numinputs=0) int *valuesOut3
{
	$1 = new int[3];
};

%typemap(argout)(int *valuesOut3)
{
	/* Applying out array of size 3 typemap */
	PyObject *o;
	o = PyList_New(3);
	for (int i = 0 ; i < 3; i++)
	{
		PyList_SET_ITEM(o, i, PyInt_FromLong($1[i])); // steals reference
	}
	if ((!$result) || ($result == Py_None))
	{
		$result = o;
	}
	else
	{
		// note: code considers that tuples are supposed to be immutable
		// should only modify if you have only reference to them!
		if (!PyTuple_Check($result))
		{
			PyObject *previousResult = $result;
			$result = PyTuple_New(2);
			PyTuple_SET_ITEM($result, 0, previousResult); // steals reference
			PyTuple_SET_ITEM($result, 1, o); // steals reference
		}
		else
		{
			PyObject *previousResult = $result;
			PyObject *addResult = PyTuple_New(1);
			PyTuple_SET_ITEM(addResult, 0, o); // steals reference
			$result = PySequence_Concat(previousResult, addResult);
			Py_DECREF(previousResult);
			Py_DECREF(addResult);
		}
	}
	delete[] $1;
};

%apply (int *valuesOut3) { (int *versionOut3) };


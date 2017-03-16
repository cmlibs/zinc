/**
 * doublevaluesarraytypemap.i
 *
 * Swig interface file for double array.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%typemap(in) (int valuesCount, const double *valuesIn)
{
	/* Applying in array of variable size typemap */
	if (PyInt_Check($input) || PyFloat_Check($input) || PyLong_Check($input))
	{
		$1 = 1;
		$2 = new double[$1];
		PyObject *o = $input;
		if (PyFloat_Check(o))
		{
			$2[0] = PyFloat_AsDouble(o);
		}
		else if (PyLong_Check(o))
		{
			$2[0] = PyLong_AsDouble(o);
		}
		else if (PyInt_Check(o))
		{
			$2[0] = static_cast<double>(PyInt_AsLong(o));
		}
		else
		{
			PyErr_SetString(PyExc_TypeError,"value must be a number");
			delete[] $2;
			return NULL;
		}
	}
	else if (PyList_Check($input))
	{
		$1 = PyList_Size($input);
		$2 = new double[$1];
		for (int i = 0; i < $1; i++)
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyFloat_Check(o))
			{
				$2[i] = PyFloat_AsDouble(o);
			}
			else if (PyLong_Check(o))
			{
				$2[i] = PyLong_AsDouble(o);
			}
			else if (PyInt_Check(o))
			{
				$2[i] = static_cast<double>(PyInt_AsLong(o));
			}
			else
			{
				PyErr_SetString(PyExc_TypeError,"list may only contain numbers");
				delete[] $2;
				return NULL;
			}
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError,"not a list, or single value");
		return NULL;
	}
};

%typemap(freearg) (int valuesCount, double const *valuesIn)
{
	delete[] $2;
};

// array getter in-handler expects an integer array size only
// and allocates array to accept output; see argout-handler
%typemap(in, numinputs=1) (int valuesCount, double *valuesOut)
{
	/* Applying out array of variable size typemap */
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
	$2 = new double[$1];
};

%typemap(argout)(int valuesCount, double *valuesOut)
{
	PyObject *o;
	if ($1 == 1)
	{
		o = PyFloat_FromDouble(*$2);
	}
	else
	{
		o = PyList_New($1);
		for (int i = 0 ; i < $1; i++)
		{
			PyList_SET_ITEM(o, i, PyFloat_FromDouble($2[i])); // steals reference
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
};

%apply (int valuesCount, const double *valuesIn) { (int coordinatesCount, const double *coordinatesIn)};
%apply (int valuesCount, double *valuesOut) { (int coordinatesCount, double *coordinatesOut)};

%apply (int valuesCount, const double *valuesIn) { (int timesCount, const double *timesIn) };

%apply (int valuesCount, const double *valuesIn) { (int seedPointsCount, const double *seedPoints) };

%typemap(in, numinputs=1) (int valuesCount, double *minimumValuesOut, double *maximumValuesOut)
{
	/* Applying minimum, maximum array out typemap */
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
	$2 = new double[$1];
	$3 = new double[$1];
};

%typemap(argout)(int valuesCount, double *minimumValuesOut, double *maximumValuesOut)
{
	PyObject *o1, *o2;
	if ($1 == 1)
	{
		o1 = PyFloat_FromDouble(*$2);
		o2 = PyFloat_FromDouble(*$3);
	}
	else
	{
		o1 = PyList_New($1);
		o2 = PyList_New($1);
		for (int i = 0 ; i < $1; i++)
		{
			PyList_SET_ITEM(o1, i, PyFloat_FromDouble($2[i])); // steals reference
			PyList_SET_ITEM(o2, i, PyFloat_FromDouble($3[i])); // steals reference
		}
	}

	if ((!$result) || ($result == Py_None))
	{
		$result = PyTuple_New(2);
		PyTuple_SET_ITEM($result, 0, o1); // steals reference
		PyTuple_SET_ITEM($result, 1, o2); // steals reference
	}
	else
	{
		// note: code considers that tuples are supposed to be immutable
		// should only modify if you have only reference to them!
		if (!PyTuple_Check($result))
		{
			PyObject *previousResult = $result;
			$result = PyTuple_New(3);
			PyTuple_SET_ITEM($result, 0, previousResult); // steals reference
			PyTuple_SET_ITEM($result, 1, o1); // steals reference
			PyTuple_SET_ITEM($result, 2, o2); // steals reference
		}
		else
		{
			PyObject *previousResult = $result;
			PyObject *addResult = PyTuple_New(2);
			PyTuple_SET_ITEM(addResult, 0, o1); // steals reference
			PyTuple_SET_ITEM(addResult, 1, o2); // steals reference
			$result = PySequence_Concat(previousResult, addResult);
			Py_DECREF(previousResult);
			Py_DECREF(addResult);
		}
	}
	delete[] $2;
	delete[] $3;
};

%typemap(in) (const double *valuesIn3)
{
	/* Applying in array of size 3 typemap */
	if (PyList_Check($input) && PyList_Size($input) == 3)
	{
		$1 = new double[3];
		for (int i = 0; i < 3; i++)
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyFloat_Check(o))
			{
				$1[i] = PyFloat_AsDouble(o);
			}
			else if (PyLong_Check(o))
			{
				$1[i] = PyLong_AsDouble(o);
			}
			else if (PyInt_Check(o))
			{
				$1[i] = static_cast<double>(PyInt_AsLong(o));
			}
			else
			{
				PyErr_SetString(PyExc_TypeError,"list may only contain a numbers");
				delete[] $1;
				return NULL;
			}
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError,"not a list of size 3");
		return NULL;
	}
};

%typemap(freearg) (double const *valuesIn3)
{
	delete[] $1;
};

%apply (double const *valuesIn3) { (double const *eyeValuesIn3), (double const *lookatValuesIn3), (double const *upVectorValuesIn3) };

// array getter in-handler expects an integer array size only
// and allocates array to accept output; see argout-handler
%typemap(in, numinputs=0) ( double *valuesOut3)
{
	$1 = new double[3];
};

%typemap(argout)(double *valuesOut3)
{
	/* Applying out array of size 3 typemap */
	PyObject *o;
	o = PyList_New(3);
	for (int i = 0 ; i < 3; i++)
	{
		PyList_SET_ITEM(o, i, PyFloat_FromDouble($1[i])); // steals reference
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

%apply (double *valuesOut3) { (double *eyeValuesOut3), (double *lookatValuesOut3), (double *upVectorValuesOut3), (double *coordinateValuesOut3), (double *minimumValuesOut3), (double *maximumValuesOut3) };


/*******************************************************************************
 * doublevaluesarraytypemap.i
 * 
 * Swig interface file for double array.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
 
%typemap(in) (int valuesCount, const double *values)
{
	/* Check if is a list */
	if (PyList_Check($input)) 
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
				PyErr_SetString(PyExc_TypeError,"list must contain float");
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

%typemap(freearg) (int valuesCount, const double *values)
{
	delete[] $2;
}

// array getter in-handler expects an integer array size only
// and allocates array to accept output; see argout-handler
%typemap(in) (int valuesCount, double *values)
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
	$2 = new double[$1];
}

%typemap(argout)(int valuesCount, double *values)
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
			PyTuple_SET_ITEM(tmp, 0, o); // steals reference
			$result = PySequence_Concat(previousResult, addResult);
			Py_DECREF(previousResult);
			Py_DECREF(addResult);
		}
	}
	delete[] $2;
}

%typemap(in) (int coordinatesCount, const double *coordinates) = (int valuesCount, const double *values);
%typemap(freearg) (int coordinatesCount, const double *coordinates) = (int valuesCount, const double *values);
%typemap(in) (int coordinatesCount, double *coordinates) = (int valuesCount, double *values);
%typemap(argout) (int coordinatesCount, double *coordinates) = (int valuesCount, double *values);

%typemap(in) (int timesCount, const double *times) = (int valuesCount, const double *values);
%typemap(freearg) (int timesCount, const double *times) = (int valuesCount, const double *values);

%typemap(in) (double *weights)
{
	/* Check if is a list */
	if (PyList_Check($input)) 
	{
		int i, arraySize;
		arraySize = PyList_Size($input);
		$1 = (double *) malloc((arraySize)*sizeof(double));
		for (i = 0; i < arraySize; i++) 
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyFloat_Check(o))
				$1[i] = PyFloat_AsDouble(o);
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
				PyErr_SetString(PyExc_TypeError,"list must contain float");
				free($1);
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

%typemap(freearg) (double *weights)
{
	free((double*) $1);
}

%typemap(typecheck) (double *weights) {
	$1 = PyList_Check($input) ? 1 : 0;
}

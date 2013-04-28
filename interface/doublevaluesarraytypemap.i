/*******************************************************************************
 * ZnDoubleValuesArrayTypemap.i
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
 
%typemap(in) (int numberOfValues, double *values)
{
	/* Check if is a list */
	if (PyList_Check($input)) 
	{
		int i;
		$1 = PyList_Size($input);
		$2 = (double *) malloc(($1)*sizeof(double));
		for (i = 0; i < $1; i++) 
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyFloat_Check(o))
				$2[i] = PyFloat_AsDouble(o);
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

%typemap(freearg) (int numberOfValues, double *values)
{
	free((double*) $2);
}

%typemap(typecheck) (int numberOfValues, double *values) {
$1 = PyList_Check($input) ? 1 : 0;
}

%typemap(in) (int numberOfChartCoordinates, double *chartCoordinates) = (int numberOfValues, double *values);
%typemap(freearg) (int numberOfChartCoordinates, double *chartCoordinates) = (int numberOfValues, double *values);
%typemap(typecheck) (int numberOfChartCoordinates, double *chartCoordinates) = (int numberOfValues, double *values);

%typemap(in) (int numberOfTimes, double *times) = (int numberOfValues, double *values);
%typemap(freearg) (int numberOfTimes, double *times) = (int numberOfValues, double *values);
%typemap(typecheck) (int numberOfTimes, double *times) = (int numberOfValues, double *values);

%typemap(in) (int number, double *baseSize) = (int numberOfValues, double *values);
%typemap(freearg) (int number, double *baseSize) = (int numberOfValues, double *values);
%typemap(typecheck) (int number, double *baseSize) = (int numberOfValues, double *values);

%typemap(in) (int number, double *scaleFactors) = (int numberOfValues, double *values);
%typemap(freearg) (int number, double *scaleFactors) = (int numberOfValues, double *values);
%typemap(typecheck) (int number, double *scaleFactors) = (int numberOfValues, double *values);

%typemap(in) (int number, double *offset) = (int numberOfValues, double *values);
%typemap(freearg) (int number, double *offset) = (int numberOfValues, double *values);
%typemap(typecheck) (int number, double *offset) = (int numberOfValues, double *values);

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


%typemap(argout)(int numberOfValues, double *outValues)
{
	PyObject *o, *o2, *o3, *o4;
	if($1 == 1)
		o = PyFloat_FromDouble(*$2);
	else
	{
		o = PyList_New($1);
		for (int i = 0 ; i < $1; i++)
		{
			o4 = PyFloat_FromDouble($2[i]);
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

%typemap(in,numinputs=0) (double *outValues)
{
	$1 = new double[100];
}

%typemap(argout) (int numberOfChartCoordinates, double *outChartCoordinates) = (int numberOfValues, double *outValues);
%typemap(in,numinputs=0) (double *outChartCoordinates) = (double *outValues);

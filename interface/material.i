/*******************************************************************************
 * Material.i
 * 
 * Swig interface file for cmiss material.
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

%module Material

%typemap(in) (const double *values)
{
	/* Check if is a list */
	if (PyList_Check($input)) 
	{
		int i;
		$1 = (double *) malloc(3*sizeof(double));
		for (i = 0; i < 3; i++) 
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyFloat_Check(o))
				$1[i] = PyFloat_AsDouble(PyList_GetItem($input,i));
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

%typemap(freearg) (const double *values)
{
	free((double*) $1);
}

%typemap(argout)(double *outValues)
{
	PyObject *o, *o2, *o3, *o4;
	o = PyList_New(3);
	for (int i = 0 ; i < 3; i++)
	{
		o4 = PyFloat_FromDouble($1[i]);
		PyList_SetItem(o, i, o4);
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
	delete $1;
}

%typemap(in,numinputs=0) (double *outValues)
{
	$1 = new double[3];
}

%{
#include "zinc/graphicsmaterial.hpp"
%}

%include "zinc/graphicsmaterial.hpp"

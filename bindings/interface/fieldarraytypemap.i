/**
 * fieldarraytypemap.i
 * 
 * Swig interface file for field array.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%typemap(in) (int fieldsCount, const OpenCMISS::Zinc::Field *sourceFields)
{
	if (PyList_Check($input))
	{
		$1 = PyList_Size($input);
		$2 = new OpenCMISS::Zinc::Field[$1];
		$2_ltype field = 0;
		for (int i = 0; i < $1; i++)
		{
			PyObject *o = PyList_GetItem($input,i);
			if ((SWIG_ConvertPtr(o,(void **) &field, $2_descriptor,SWIG_POINTER_EXCEPTION)) != -1)
			{
					$2[i] = *field;
			}
			else
			{
				PyErr_SetString(PyExc_TypeError,"list must contain OpenCMISS::Zinc::Field");
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

%typemap(freearg) (int fieldsCount, const OpenCMISS::Zinc::Field *sourceFields)
{
	delete[] $2;
}

%typemap(typecheck) (int fieldsCount, const OpenCMISS::Zinc::Field *sourceFields)
{
	$1 = PyList_Check($input) ? 1 : 0;
}

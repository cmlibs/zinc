/*******************************************************************************
 * fieldarraytypemap.i
 * 
 * Swig interface file for field array.
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

%typemap(in) (int fieldsCount, OpenCMISS::Zinc::Field *sourceFields)
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

%typemap(freearg) (int fieldsCount, OpenCMISS::Zinc::Field *sourceFields)
{
	delete[] $2;
}

%typemap(typecheck) (int fieldsCount, OpenCMISS::Zinc::Field *sourceFields)
{
	$1 = PyList_Check($input) ? 1 : 0;
}

#include <Python.h>
#include "api/cmiss_variable_composition.h"

static PyObject*
CmissVariableComposition_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id dependent_variable_ptr, *independent_variable_ptrs, variable,
		*source_variable_ptrs;
	int i, number_of_source_variables, return_code;
	PyObject *cmiss_variable, *dependent_variable, *independent_variable, 
		*independent_source_variables_array, *variable_cpointer, *variable_module;

	if (!PyArg_ParseTuple(args,"sOO:new", &name, &dependent_variable, &independent_source_variables_array)) 
		return NULL;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (!((variable_cpointer = PyObject_CallMethod(dependent_variable, "get_variable_cpointer", (char *)NULL)) &&
			 PyCObject_Check(variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from dependent_variable.");
		return NULL;			 
	}
	dependent_variable_ptr = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);

	if (!PyList_Check(independent_source_variables_array))
	{
		PyErr_SetString(PyExc_AttributeError, "Third argument must be a list");
		return NULL;
	}

	cmiss_variable = (PyObject *)NULL;
	return_code = 1;
	if (dependent_variable&&independent_source_variables_array&&
			(0<(number_of_source_variables=PyList_Size(independent_source_variables_array)))&&
		 !(number_of_source_variables%2))
	{
		number_of_source_variables /= 2;
		if ((independent_variable_ptrs=(Cmiss_variable_id *)malloc(number_of_source_variables*
				 sizeof(Cmiss_variable_id)))
			&& (source_variable_ptrs=(Cmiss_variable_id *)malloc(number_of_source_variables*
					 sizeof(Cmiss_variable_id))))
		{
			i=0;
			while (return_code && (i<number_of_source_variables))
			{
				if ((variable_cpointer = PyObject_CallMethod(
					PySequence_GetItem(independent_source_variables_array, 2 * i),
					"get_variable_cpointer", (char *)NULL)) &&
					PyCObject_Check(variable_cpointer))
				{
					independent_variable_ptrs[i] = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);
				}
				else
				{
					return_code = 0;
				}
				if ((variable_cpointer = PyObject_CallMethod(
					PySequence_GetItem(independent_source_variables_array, 2 * i + 1),
					"get_variable_cpointer", (char *)NULL)) &&
					PyCObject_Check(variable_cpointer))
				{
					source_variable_ptrs[i] = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);
				}
				else
				{
					return_code = 0;
				}
				i++;
			}
			if (return_code)
			{
				if (variable = CREATE(Cmiss_variable_composition)(name, dependent_variable_ptr,
					number_of_source_variables,source_variable_ptrs,independent_variable_ptrs))
				{
					cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
						PyCObject_FromVoidPtr(variable, NULL));
				}
				else
				{
					free(independent_variable_ptrs);
					free(source_variable_ptrs);
				}
			}
			else
			{
				PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from at one of the values in the independent_source_variable array.");
				free(independent_variable_ptrs);
				free(source_variable_ptrs);
			}
		}
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableCompositionModule_methods[] = {
    {"new", CmissVariableComposition_new, METH_VARARGS,
     "Create a new Cmiss VariableComposition object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initComposition(void) 
{
	Py_InitModule("Cmiss.Variable.C.Composition", CmissVariableCompositionModule_methods);
}

#include <Python.h>
#include "api/cmiss_variable_derivative.h"

static PyObject*
CmissVariableDerivative_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id dependent_variable_ptr, *independent_variable_ptrs, variable;
	int i, order;
	PyObject *cmiss_variable, *dependent_variable, *independent_variable, 
		*independent_variables_array, *variable_cpointer, *variable_module;

	if (!PyArg_ParseTuple(args,"sOO:new", &name, &dependent_variable, &independent_variables_array)) 
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

	if (!PyList_Check(independent_variables_array))
	{
		PyErr_SetString(PyExc_AttributeError, "Third argument must be a list");
		return NULL;
	}

	cmiss_variable = (PyObject *)NULL;
	if (dependent_variable&&independent_variables_array&&
			(0<(order=PyList_Size(independent_variables_array))))
	{
		if (independent_variable_ptrs=(Cmiss_variable_id *)malloc(order*
				 sizeof(Cmiss_variable_id)))
		{
			i=0;
			while ((i<order)&&(variable_cpointer = PyObject_CallMethod(
				PySequence_GetItem(independent_variables_array, i),
				"get_variable_cpointer", (char *)NULL)) &&
				PyCObject_Check(variable_cpointer))
			{
				independent_variable_ptrs[i] = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);
				i++;
			}
			if (i >= order)
			{
				if (variable = CREATE(Cmiss_variable_derivative)(
					name,dependent_variable_ptr, order, independent_variable_ptrs))
				{
					cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
						PyCObject_FromVoidPtr(variable, NULL));
				}
				else
				{
					free(independent_variable_ptrs);
				}
			}
			else
			{
				PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from at one of the values in the independent_variable array.");
				free(independent_variable_ptrs);
			}
		}
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableDerivativeModule_methods[] = {
    {"new", CmissVariableDerivative_new, METH_VARARGS,
     "Create a new Cmiss VariableDerivative object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initDerivative(void) 
{
	Py_InitModule("Cmiss.Variable.C.Derivative", CmissVariableDerivativeModule_methods);
}

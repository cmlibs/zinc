#include <Python.h>
#include "api/cmiss_variable_composite.h"

static PyObject*
CmissVariableComposite_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id variable, *variables;
	int i, number_of_variables, return_code;
	PyObject *cmiss_variable, *variable_module, *variables_array, *variable_cpointer;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (!PyArg_ParseTuple(args,"sO|:new", &name, &variables_array)) 
		return NULL;

	cmiss_variable = (PyObject *)NULL;
	return_code = 1;
	if (variables_array && (0<(number_of_variables=PyList_Size(variables_array))) &&
	  (variables=(Cmiss_variable_id *)malloc(number_of_variables*sizeof(Cmiss_variable_id))))
	{
		i=0;
		while (return_code && (i<number_of_variables))
		{
			if ((variable_cpointer = PyObject_CallMethod(
				PySequence_GetItem(variables_array, i),
				"get_variable_cpointer", (char *)NULL)) &&
				PyCObject_Check(variable_cpointer))
			{
				variables[i] = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);
			}
			else
			{
				return_code = 0;
			}
			i++;
		}
		if (return_code)
		{
			if (variable = CREATE(Cmiss_variable_composite)(name, number_of_variables, variables))
			{
				cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
					PyCObject_FromVoidPtr(variable, NULL));
			}
			else
			{
				cmiss_variable = (PyObject *)NULL;
			}
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from one of the variables in the array.");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "Missing variables array.");
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableCompositeModule_methods[] = {
    {"new", CmissVariableComposite_new, METH_VARARGS,
     "Create a new Cmiss VariableComposite object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initComposite(void) 
{
	Py_InitModule("Cmiss.Variable.C.Composite", CmissVariableCompositeModule_methods);
}

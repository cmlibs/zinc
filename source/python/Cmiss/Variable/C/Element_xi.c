#include <Python.h>
#include "api/cmiss_variable_finite_element.h"

static PyObject*
CmissVariableElementxi_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id variable;
	int dimension;
	PyObject *cmiss_variable, *variable_module;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	dimension = 0;
	if (!PyArg_ParseTuple(args,"s|d:new", &name, &dimension)) 
		return NULL;

	if (variable = CREATE(Cmiss_variable_element_xi)(name, dimension))
	{
		cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
			PyCObject_FromVoidPtr(variable, NULL));
	}
	else
	{
		cmiss_variable = (PyObject *)NULL;
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableElementxiModule_methods[] = {
    {"new", CmissVariableElementxi_new, METH_VARARGS,
     "Create a new Cmiss VariableElementxi object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initElement_xi(void) 
{
	Py_InitModule("Cmiss.Variable.C.Element_xi", CmissVariableElementxiModule_methods);
}

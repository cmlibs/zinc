#include <Python.h>
#include "api/cmiss_variable_coordinates.h"

static PyObject*
CmissVariableCoordinates_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id variable;
	int dimension;
	PyObject *cmiss_variable, *variable_module;

	if (!PyArg_ParseTuple(args,"si:new", &name, &dimension)) 
		return NULL;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	cmiss_variable = (PyObject *)NULL;
	if (variable = CREATE(Cmiss_variable_coordinates)(name, dimension))
	{
		cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
			PyCObject_FromVoidPtr(variable, NULL));
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableCoordinatesModule_methods[] = {
    {"new", CmissVariableCoordinates_new, METH_VARARGS,
     "Create a new Cmiss VariableCoordinates object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initCoordinates(void) 
{
	Py_InitModule("Cmiss.Variable.C.Coordinates", CmissVariableCoordinatesModule_methods);
}

#include <Python.h>
#include "api/cmiss_variable_coordinates.h"

static PyObject*
CmissVariableSpheroidal_Coordinates_Focus_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id variable;
	PyObject *cmiss_variable, *variable_module;

	if (!PyArg_ParseTuple(args,"s:new", &name)) 
		return NULL;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	cmiss_variable = (PyObject *)NULL;
	if (variable = CREATE(Cmiss_variable_spheroidal_coordinates_focus)(name))
	{
		cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
			PyCObject_FromVoidPtr(variable, NULL));
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableSpheroidal_Coordinates_FocusModule_methods[] = {
    {"new", CmissVariableSpheroidal_Coordinates_Focus_new, METH_VARARGS,
     "Create a new Cmiss VariableSpheroidal_Coordinates_Focus object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initSpheroidal_coordinates_focus(void) 
{
	Py_InitModule("Cmiss.Variable.C.Spheroidal_coordinates_focus", CmissVariableSpheroidal_Coordinates_FocusModule_methods);
}

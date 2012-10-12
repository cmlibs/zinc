#include <Python.h>
#include "api/cmiss_variable_coordinates.h"

static PyObject*
CmissVariableProlate_Spheroidal_To_Rectangular_Cartesian_new(PyObject* self, PyObject* args)
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
	if (variable = CREATE(Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian)(name))
	{
		cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
			PyCObject_FromVoidPtr(variable, NULL));
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableProlate_Spheroidal_To_Rectangular_CartesianModule_methods[] = {
    {"new", CmissVariableProlate_Spheroidal_To_Rectangular_Cartesian_new, METH_VARARGS,
     "Create a new Cmiss VariableProlate_Spheroidal_To_Rectangular_Cartesian object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initProlate_spheroidal_to_rectangular_cartesian(void) 
{
	Py_InitModule("Cmiss.Variable.C.Prolate_spheroidal_to_rectangular_cartesian", CmissVariableProlate_Spheroidal_To_Rectangular_CartesianModule_methods);
}

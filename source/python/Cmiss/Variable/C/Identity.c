#include <Python.h>
#include "api/cmiss_variable_identity.h"

static PyObject*
CmissVariableIdentity_new(PyObject* self, PyObject* args)
{
	char *name;
	Cmiss_variable_id variable, reference_variable_ptr;
	PyObject *cmiss_variable, *variable_module, *reference_variable, *variable_cpointer;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (!PyArg_ParseTuple(args,"sO|:new", &name, &reference_variable)) 
		return NULL;

	if (!((variable_cpointer = PyObject_CallMethod(reference_variable, "get_variable_cpointer", (char *)NULL)) &&
			 PyCObject_Check(variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from referenced variable.");
		return NULL;			 
	}
	reference_variable_ptr = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);

	if (variable = CREATE(Cmiss_variable_identity)(name, reference_variable_ptr))
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

static PyMethodDef CmissVariableIdentityModule_methods[] = {
    {"new", CmissVariableIdentity_new, METH_VARARGS,
     "Create a new Cmiss VariableIdentity object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initIdentity(void) 
{
	Py_InitModule("Cmiss.Variable.C.Identity", CmissVariableIdentityModule_methods);
}

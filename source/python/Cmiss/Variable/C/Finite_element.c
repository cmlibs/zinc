#include <Python.h>
#include "api/cmiss_variable_finite_element.h"

static PyObject*
CmissVariableFiniteelement_new(PyObject* self, PyObject* args)
{
	char *component_name;
	Cmiss_variable_id variable;
	struct Cmiss_FE_field *fe_field_ptr;
	int dimension;
	PyObject *cmiss_variable, *fe_field, *fe_field_module, *fe_field_cpointer, *variable_module;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	component_name = (char *)NULL;
	if (!PyArg_ParseTuple(args,"O|s:new", &fe_field, &component_name)) 
		return NULL;

	if (!(fe_field_module = PyImport_ImportModule("Cmiss.FE_field")))
	{
		PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.FE_field module");
		return NULL;
	}
	 
	if (!(PyObject_IsTrue(PyObject_CallMethod(fe_field_module, "check", "O", fe_field))))
	{
		PyErr_SetString(PyExc_AttributeError, "First argument must be a Cmiss.FE_field");
		return NULL;
	}

	if (!((fe_field_cpointer = PyObject_CallMethod(fe_field, "get_fe_field_cpointer", (char *)NULL)) &&
			 PyCObject_Check(fe_field_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract fe_field pointer from fe_field.");
		return NULL;			 
	}
	fe_field_ptr = (struct Cmiss_FE_field *)PyCObject_AsVoidPtr(fe_field_cpointer);

	if (variable = CREATE(Cmiss_variable_finite_element)(fe_field_ptr, component_name))
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

static PyMethodDef CmissVariableFiniteelementModule_methods[] = {
    {"new", CmissVariableFiniteelement_new, METH_VARARGS,
     "Create a new Cmiss VariableFiniteelement object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initFinite_element(void) 
{
	Py_InitModule("Cmiss.Variable.C.Finite_element", CmissVariableFiniteelementModule_methods);
}


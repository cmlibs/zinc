#include <Python.h>
#include "api/cmiss_variable_finite_element.h"

static PyObject*
CmissVariableNodalvalue_new(PyObject* self, PyObject* args)
{
	char *name, default_type[] = "all", *type;
	Cmiss_variable_id fe_variable_ptr, variable;
	enum FE_nodal_value_type value_type;
	int value_type_valid, version;
	PyObject *cmiss_variable, *fe_variable, *fe_node_module, *finite_element_module, *node,
		*variable_cpointer, *variable_module;
	struct FE_node *node_ptr;

	type = default_type;
	version = -1;
	node = (PyObject *)NULL;
	if (!PyArg_ParseTuple(args,"sO|Osi:new", &name, &fe_variable, &node, &type, &version)) 
		return NULL;

	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.C.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.C.Variable module");
		return NULL;
	}

	if (!(finite_element_module = PyImport_ImportModule("Cmiss.Variable.C.Finite_element")))
	{
		PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.Variable.C.Finite_element module");
		return NULL;
	}

	if (node)
	{
		if (!(fe_node_module = PyImport_ImportModule("Cmiss.FE_node")))
		{
			PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.FE_node module");
			return NULL;
		}

		if (!((variable_cpointer = PyObject_CallMethod(node, "get_fe_node_cpointer", (char *)NULL)) &&
				 PyCObject_Check(variable_cpointer)))
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from variable.");
			return NULL;
		}
		node_ptr = PyCObject_AsVoidPtr(variable_cpointer);
	}
	else
	{
		node_ptr = (struct FE_node *)NULL;
	}

	cmiss_variable = (PyObject *)NULL;

	if (!((variable_cpointer = PyObject_CallMethod(fe_variable, "get_variable_cpointer", (char *)NULL)) &&
			 PyCObject_Check(variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from variable.");
		return NULL;
	}
	fe_variable_ptr = PyCObject_AsVoidPtr(variable_cpointer);
		
	value_type_valid=0;
	value_type=FE_NODAL_UNKNOWN;
	if (!strcmp("all",type))
	{
		value_type=FE_NODAL_UNKNOWN;
		value_type_valid=1;
	}
	else if (!strcmp("value",type))
	{
		value_type=FE_NODAL_VALUE;
		value_type_valid=1;
	}
	else if (!strcmp("d/ds1",type))
	{
		value_type=FE_NODAL_D_DS1;
		value_type_valid=1;
	}
	else if (!strcmp("d/ds2",type))
	{
		value_type=FE_NODAL_D_DS2;
		value_type_valid=1;
	}
	else if (!strcmp("d/ds3",type))
	{
		value_type=FE_NODAL_D_DS3;
		value_type_valid=1;
	}
	else if (!strcmp("d2/ds1ds2",type))
	{
		value_type=FE_NODAL_D2_DS1DS2;
		value_type_valid=1;
	}
	else if (!strcmp("d2/ds1ds3",type))
	{
		value_type=FE_NODAL_D2_DS1DS3;
		value_type_valid=1;
	}
	else if (!strcmp("d2/ds2ds3",type))
	{
		value_type=FE_NODAL_D2_DS2DS3;
		value_type_valid=1;
	}
	else if (!strcmp("d3/ds1ds2ds3",type))
	{
		value_type=FE_NODAL_D3_DS1DS2DS3;
		value_type_valid=1;
	}
	if (value_type_valid)
	{
		if (variable = CREATE(Cmiss_variable_nodal_value)(
			name, fe_variable_ptr, node_ptr, value_type, version))
		{
			cmiss_variable = PyObject_CallMethod(variable_module, "wrap", "O",
				PyCObject_FromVoidPtr(variable, NULL));
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "Invalid nodal value type.");
	}

	return cmiss_variable;
}

static PyMethodDef CmissVariableNodalvalueModule_methods[] = {
    {"new", CmissVariableNodalvalue_new, METH_VARARGS,
     "Create a new Cmiss VariableNodalvalue object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initNodal_value(void) 
{
	Py_InitModule("Cmiss.Variable.C.Nodal_value", CmissVariableNodalvalueModule_methods);
}


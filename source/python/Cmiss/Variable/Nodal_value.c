#include <Python.h>
#include "computed_variable/computed_variable.h"
#include "computed_variable/computed_variable_finite_element.h"

staticforward PyTypeObject CmissVariableNodalvalueType;

/* Internal definition */
#define _CmissVariableNodalvalue_check(object)  ((object)->ob_type == &CmissVariableNodalvalueType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_variable *variable;
} CmissVariableNodalvalueObject;

/* Object Methods */

static PyObject*
CmissVariableNodalvalue_get_variable_cpointer(PyObject* self, PyObject* args)
{
	CmissVariableNodalvalueObject *cmiss_variable;
	PyObject *object, *return_code;

	if (_CmissVariableNodalvalue_check(self))
	{
		cmiss_variable = (CmissVariableNodalvalueObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_variable->variable, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissVariableNodalvalue_methods[] =
	{
		{"get_variable_cpointer", CmissVariableNodalvalue_get_variable_cpointer, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissVariableNodalvalue_new(PyObject* self, PyObject* args)
{
	char *name, default_type[] = "all", *type;
	CmissVariableNodalvalueObject *cmiss_variable;
	Cmiss_variable_id fe_variable_ptr;
	enum FE_nodal_value_type value_type;
	int value_type_valid, version;
	PyObject *fe_variable, *fe_node_module, *finite_element_module, *node, *variable_cpointer;
	struct FE_node *node_ptr;

	type = default_type;
	version = -1;
	node = (PyObject *)NULL;
	if (!PyArg_ParseTuple(args,"sO|Osi:new", &name, &fe_variable, &node, &type, &version)) 
		return NULL;

	 if (!(finite_element_module = PyImport_ImportModule("Cmiss.Variable.Finite_element")))
	 {
		 PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.Variable.Finite_element module");
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

	cmiss_variable = PyObject_New(CmissVariableNodalvalueObject, &CmissVariableNodalvalueType);
	if (cmiss_variable->variable = CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
		name))
	{
		ACCESS(Cmiss_variable)(cmiss_variable->variable);

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
			if (!Cmiss_variable_nodal_value_set_type(cmiss_variable->variable,fe_variable_ptr, node_ptr,
					 value_type,version))
			{
				DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
				Py_DECREF(cmiss_variable);
				cmiss_variable = (CmissVariableNodalvalueObject *)NULL;
			}
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Invalid nodal value type.");
			DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
			Py_DECREF(cmiss_variable);
			cmiss_variable = (CmissVariableNodalvalueObject *)NULL;
		}
	}

	return (PyObject *)cmiss_variable;
}

static void
CmissVariableNodalvalue_dealloc(PyObject* self)
{
   CmissVariableNodalvalueObject *cmiss_variable;
 	if (_CmissVariableNodalvalue_check(self))
	{
		cmiss_variable = (CmissVariableNodalvalueObject *)self;
		DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
	}
	PyObject_Del(self);
}

static PyObject *
CmissVariableNodalvalue_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissVariableNodalvalue_methods, (PyObject *)self, name);
}

static PyObject*
CmissVariableNodalvalue_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	if (_CmissVariableNodalvalue_check(object))
	{
		return_code = PyInt_FromLong(1);
	}
	else
	{
		return_code = PyInt_FromLong(0);
	}

	return(return_code);
}

static PyObject*
CmissVariableNodalvalue_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableNodalvalueObject *cmiss_variable;
	PyObject *cmiss_variable_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_variable_cpointer)
		&& PyCObject_Check(cmiss_variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_variable = PyObject_New(CmissVariableNodalvalueObject, &CmissVariableNodalvalueType);
	if (!(cmiss_variable->variable = ACCESS(Cmiss_variable)(
		(Cmiss_variable_id)PyCObject_AsVoidPtr(cmiss_variable_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.VariableNodalvalue pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_variable;
}

static PyObject *
CmissVariableNodalvalue_repr(PyObject* self)
{
	char *name;
	CmissVariableNodalvalueObject *cmiss_variable;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissVariableNodalvalue_check(self))
	{		
		cmiss_variable = (CmissVariableNodalvalueObject *)self;
		if (get_name_Cmiss_variable(cmiss_variable->variable, &name))
		{
			string = PyString_FromString(name);
			free(name);
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Derivativematrix");
	}
	return (string);
}

static PyTypeObject CmissVariableNodalvalueType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Nodal_value",
    sizeof(CmissVariableNodalvalueObject),
    0,
    CmissVariableNodalvalue_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissVariableNodalvalue_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissVariableNodalvalue_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissVariableNodalvalueType_methods[] = {
    {"new", CmissVariableNodalvalue_new, METH_VARARGS,
     "Create a new Cmiss VariableNodalvalue object."},
    {"check", CmissVariableNodalvalue_check, METH_VARARGS,
     "Check if object is of type Cmiss VariableNodalvalue object."},
    {"wrap", CmissVariableNodalvalue_wrap, METH_VARARGS,
     "Wrap a C CmissVariableNodalvalue in a python Cmiss VariableNodalvalue object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initNodal_value(void) 
{
	CmissVariableNodalvalueType.ob_type = &PyType_Type;
	
	Py_InitModule("Nodal_value", CmissVariableNodalvalueType_methods);
}

#include <Python.h>
#include "computed_variable/computed_variable.h"

staticforward PyTypeObject CmissVariableElementxiType;

/* Internal definition */
#define _CmissVariableElementxi_check(object)  ((object)->ob_type == &CmissVariableElementxiType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_variable *variable;
} CmissVariableElementxiObject;

/* Object Methods */

static PyObject*
CmissVariableElementxi_get_variable_cpointer(PyObject* self, PyObject* args)
{
	CmissVariableElementxiObject *cmiss_variable;
	PyObject *object, *return_code;

	printf("CmissVariableElementxi_get_variable_cpointer\n");

	if (_CmissVariableElementxi_check(self))
	{
		cmiss_variable = (CmissVariableElementxiObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_variable->variable, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissVariableElementxi_methods[] =
	{
		{"get_variable_cpointer", CmissVariableElementxi_get_variable_cpointer, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissVariableElementxi_new(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableElementxiObject *cmiss_variable;

	if (!PyArg_ParseTuple(args,"s:new", &name)) 
		return NULL;

	cmiss_variable = PyObject_New(CmissVariableElementxiObject, &CmissVariableElementxiType);
	if (cmiss_variable->variable = CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
		name))
	{
		ACCESS(Cmiss_variable)(cmiss_variable->variable);
		if (!Cmiss_variable_element_xi_set_type(cmiss_variable->variable))
		{
			DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
			PyDECREF(cmiss_variable);
			cmiss_variable = (CmissVariableElementxiObject *)NULL;
		}
	}

	printf("Creating new CmissVariableElementxi\n");

	return (PyObject *)cmiss_variable;
}

static void
CmissVariableElementxi_dealloc(PyObject* self)
{
   CmissVariableElementxiObject *cmiss_variable;
 	if (_CmissVariableElementxi_check(self))
	{
		cmiss_variable = (CmissVariableElementxiObject *)self;
		DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
	}
	PyObject_Del(self);
}

static PyObject *
CmissVariableElementxi_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissVariableElementxi_methods, (PyObject *)self, name);
}

static PyObject*
CmissVariableElementxi_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissVariableElementxi\n");

	if (_CmissVariableElementxi_check(object))
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
CmissVariableElementxi_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableElementxiObject *cmiss_variable;
	PyObject *cmiss_variable_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_variable_cpointer)
		&& PyCObject_Check(cmiss_variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_variable = PyObject_New(CmissVariableElementxiObject, &CmissVariableElementxiType);
	if (!(cmiss_variable->variable = ACCESS(Cmiss_variable)(
		(Cmiss_variable_id)PyCObject_AsVoidPtr(cmiss_variable_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.VariableElementxi pointer.");
		return NULL;			 
	}

	printf("Wrapping CmissVariableElementxi\n");

	return (PyObject*)cmiss_variable;
}

static PyObject *
CmissVariableElementxi_repr(PyObject* self)
{
	char *name;
	CmissVariableElementxiObject *cmiss_variable;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissVariableElementxi_check(self))
	{		
		cmiss_variable = (CmissVariableElementxiObject *)self;
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

static PyTypeObject CmissVariableElementxiType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Element_xi",
    sizeof(CmissVariableElementxiObject),
    0,
    CmissVariableElementxi_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissVariableElementxi_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissVariableElementxi_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissVariableElementxiType_methods[] = {
    {"new", CmissVariableElementxi_new, METH_VARARGS,
     "Create a new Cmiss VariableElementxi object."},
    {"check", CmissVariableElementxi_check, METH_VARARGS,
     "Check if object is of type Cmiss VariableElementxi object."},
    {"wrap", CmissVariableElementxi_wrap, METH_VARARGS,
     "Wrap a C CmissVariableElementxi in a python Cmiss VariableElementxi object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initElement_xi(void) 
{
	CmissVariableElementxiType.ob_type = &PyType_Type;
	
	printf ("In initElement_xi\n");

	Py_InitModule("Element_xi", CmissVariableElementxiType_methods);
}

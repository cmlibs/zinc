#include <Python.h>
#include "computed_variable/computed_variable.h"

staticforward PyTypeObject CmissVariableType;

/* Internal definition */
#define _CmissVariable_check(object)  ((object)->ob_type == &CmissVariableType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_variable *variable;
} CmissVariableObject;

/* Object Methods */

static PyObject*
CmissVariable_get_variable_cpointer(PyObject* self, PyObject* args)
{
	CmissVariableObject *cmiss_variable;
	PyObject *object, *return_code;

	printf("CmissVariable_get_variable_cpointer\n");

	if (_CmissVariable_check(self))
	{
		cmiss_variable = (CmissVariableObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_variable->variable, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissVariable_methods[] =
	{
		{"get_variable_cpointer", CmissVariable_get_variable_cpointer, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissVariable_new(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableObject *cmiss_variable;

	if (!PyArg_ParseTuple(args,"s:new", &name)) 
		return NULL;

	cmiss_variable = PyObject_New(CmissVariableObject, &CmissVariableType);
	if (cmiss_variable->variable = CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
		name))
	{
		ACCESS(Cmiss_variable)(cmiss_variable->variable);
	}

	printf("Creating new CmissVariable\n");

	return (PyObject*)cmiss_variable;
}

static void
CmissVariable_dealloc(PyObject* self)
{
   CmissVariableObject *cmiss_variable;
 	if (_CmissVariable_check(self))
	{
		cmiss_variable = (CmissVariableObject *)self;
		DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
	}
	PyObject_Del(self);
}

static PyObject *
CmissVariable_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissVariable_methods, (PyObject *)self, name);
}

static PyObject*
CmissVariable_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissVariable\n");

	if (_CmissVariable_check(object))
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
CmissVariable_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableObject *cmiss_variable;
	PyObject *cmiss_variable_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_variable_cpointer)
		&& PyCObject_Check(cmiss_variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_variable = PyObject_New(CmissVariableObject, &CmissVariableType);
	if (!(cmiss_variable->variable = ACCESS(Cmiss_variable)(
		(Cmiss_variable_id)PyCObject_AsVoidPtr(cmiss_variable_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Variable pointer.");
		return NULL;			 
	}

	printf("Wrapping CmissVariable\n");

	return (PyObject*)cmiss_variable;
}

static PyObject *
CmissVariable_repr(PyObject* self)
{
	char *name;
	CmissVariableObject *cmiss_variable;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissVariable_check(self))
	{		
		cmiss_variable = (CmissVariableObject *)self;
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

static PyTypeObject CmissVariableType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Variable",
    sizeof(CmissVariableObject),
    0,
    CmissVariable_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissVariable_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissVariable_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissVariableType_methods[] = {
    {"new", CmissVariable_new, METH_VARARGS,
     "Create a new Cmiss Variable object."},
    {"check", CmissVariable_check, METH_VARARGS,
     "Check if object is of type Cmiss Variable object."},
    {"wrap", CmissVariable_wrap, METH_VARARGS,
     "Wrap a C CmissVariable in a python Cmiss Variable object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initVariable(void) 
{
	CmissVariableType.ob_type = &PyType_Type;
	
	printf ("In initVariable\n");

	Py_InitModule("Variable", CmissVariableType_methods);
}

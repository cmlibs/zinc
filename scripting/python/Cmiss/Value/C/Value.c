#include <Python.h>
#include "api/cmiss_core.h"
#include "api/cmiss_value.h"
#include "computed_variable/computed_value.h"

staticforward PyTypeObject CmissValueType;

/* Internal definition */
#define _CmissValue_check(object)  ((object)->ob_type == &CmissValueType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_value *value;
} CmissValueObject;

/* Object Methods */

static PyObject*
CmissValue_get_value_cpointer(PyObject* self, PyObject* args)
{
	CmissValueObject *cmiss_value;
	PyObject *return_code;

	return_code = (PyObject *)NULL;
	if (_CmissValue_check(self))
	{
		cmiss_value = (CmissValueObject *)self;
		
		return_code = PyCObject_FromVoidPtr(cmiss_value->value, NULL);
	}

	return(return_code);
}

static PyObject*
CmissValue_get_string(PyObject* self, PyObject* args)
{
	char *string;
	CmissValueObject *cmiss_value;
	PyObject *return_code;

	return_code = (PyObject *)NULL;
	if (_CmissValue_check(self))
	{
		cmiss_value = (CmissValueObject *)self;
		if (Cmiss_value_get_string(cmiss_value->value,
			&string))
		{
			return_code = PyString_FromString(string);
			Cmiss_deallocate(string);
		}
	}

	return(return_code);
}

static struct PyMethodDef CmissValue_methods[] =
	{
		{"get_value_cpointer", CmissValue_get_value_cpointer, 1},
		{"get_string", CmissValue_get_string, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissValue_new(PyObject* self, PyObject* args)
{
    CmissValueObject *cmiss_value;

    if (!PyArg_ParseTuple(args,":new")) 
        return NULL;

    cmiss_value = PyObject_New(CmissValueObject, &CmissValueType);
	 if (cmiss_value->value = CREATE(Cmiss_value)())
	 {
		 ACCESS(Cmiss_value)(cmiss_value->value);
	 }

    return (PyObject*)cmiss_value;
}

static PyObject*
CmissValue_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissValueObject *cmiss_value;
	PyObject *cmiss_value_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_value_cpointer)
		&& PyCObject_Check(cmiss_value_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_value = PyObject_New(CmissValueObject, &CmissValueType);
	if (!(cmiss_value->value = ACCESS(Cmiss_value)(
		(Cmiss_value_id)PyCObject_AsVoidPtr(cmiss_value_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Value pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_value;
}

static PyObject *
CmissValue_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissValue_methods, (PyObject *)self, name);
}

static void
CmissValue_dealloc(PyObject* self)
{
	/* How do I check that this is really a CmissValue before the cast? */
    CmissValueObject *cmiss_value;
	 cmiss_value = (CmissValueObject *)self;
	 DEACCESS(Cmiss_value)(&cmiss_value->value);
    PyObject_Del(self);
}

static PyTypeObject CmissValueType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Cmiss.Value.C.Value",
    sizeof(CmissValueObject),
    0,
    CmissValue_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissValue_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    0,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissValueType_methods[] = {
    {"new", CmissValue_new, METH_VARARGS,
     "Create a new Cmiss Value object."},
    {"wrap", CmissValue_wrap, METH_VARARGS,
     "Wrap a cmiss value in a new python Cmiss Value object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initValue(void) 
{
	CmissValueType.ob_type = &PyType_Type;
	
	Py_InitModule("Cmiss.Value.C.Value", CmissValueType_methods);
}

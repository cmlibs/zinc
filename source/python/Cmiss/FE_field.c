#include <Python.h>
#include "finite_element/finite_element.h"

staticforward PyTypeObject CmissFEfieldType;

/* Internal definition */
#define _CmissFEfield_check(object)  ((object)->ob_type == &CmissFEfieldType)

typedef struct {
    PyObject_HEAD
    struct FE_field *fe_field;
} CmissFEfieldObject;

/* Object Methods */

static PyObject*
CmissFEfield_get_fe_field_cpointer(PyObject* self, PyObject* args)
{
	CmissFEfieldObject *cmiss_fe_field;
	PyObject *object, *return_code;

	printf("CmissFEfield_get_fe_field_cpointer\n");

	if (_CmissFEfield_check(self))
	{
		cmiss_fe_field = (CmissFEfieldObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_fe_field->fe_field, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissFEfield_methods[] =
	{
		{"get_fe_field_cpointer", CmissFEfield_get_fe_field_cpointer, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static void
CmissFEfield_dealloc(PyObject* self)
{
   CmissFEfieldObject *cmiss_fe_field;
 	if (_CmissFEfield_check(self))
	{
		cmiss_fe_field = (CmissFEfieldObject *)self;
		DEACCESS(FE_field)(&cmiss_fe_field->fe_field);
	}
	PyObject_Del(self);
}

static PyObject *
CmissFEfield_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissFEfield_methods, (PyObject *)self, name);
}

static PyObject*
CmissFEfield_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissFEfield\n");

	if (_CmissFEfield_check(object))
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
CmissFEfield_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissFEfieldObject *cmiss_fe_field;
	PyObject *cmiss_fe_field_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_fe_field_cpointer)
		&& PyCObject_Check(cmiss_fe_field_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_fe_field = PyObject_New(CmissFEfieldObject, &CmissFEfieldType);
	if (!(cmiss_fe_field->fe_field = ACCESS(FE_field)(
		(struct FE_field *)PyCObject_AsVoidPtr(cmiss_fe_field_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract FE_field pointer.");
		return NULL;			 
	}

	printf("Wrapping Cmiss.FE_field\n");

	return (PyObject*)cmiss_fe_field;
}

static PyObject *
CmissFEfield_repr(PyObject* self)
{
	char *name;
	CmissFEfieldObject *cmiss_fe_field;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissFEfield_check(self))
	{		
		cmiss_fe_field = (CmissFEfieldObject *)self;
		if (GET_NAME(FE_field)(cmiss_fe_field->fe_field, &name))
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

static PyTypeObject CmissFEfieldType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "FE_field",
    sizeof(CmissFEfieldObject),
    0,
    CmissFEfield_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissFEfield_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissFEfield_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissFEfieldType_methods[] = {
    {"check", CmissFEfield_check, METH_VARARGS,
     "Check if object is of type Cmiss FEfield object."},
    {"wrap", CmissFEfield_wrap, METH_VARARGS,
     "Wrap a C CmissFEfield in a python Cmiss FEfield object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initFE_field(void) 
{
	CmissFEfieldType.ob_type = &PyType_Type;
	
	printf ("In initFE_field\n");

	Py_InitModule("FE_field", CmissFEfieldType_methods);
}

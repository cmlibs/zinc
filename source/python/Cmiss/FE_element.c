#include <Python.h>
#include "finite_element/finite_element.h"

staticforward PyTypeObject CmissFEelementType;

/* Internal definition */
#define _CmissFEelement_check(object)  ((object)->ob_type == &CmissFEelementType)

typedef struct {
    PyObject_HEAD
    struct FE_element *fe_element;
} CmissFEelementObject;

/* Object Methods */

static PyObject*
CmissFEelement_get_fe_element_cpointer(PyObject* self, PyObject* args)
{
	CmissFEelementObject *cmiss_fe_element;
	PyObject *object, *return_code;

	if (_CmissFEelement_check(self))
	{
		cmiss_fe_element = (CmissFEelementObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_fe_element->fe_element, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissFEelement_methods[] =
	{
		{"get_fe_element_cpointer", CmissFEelement_get_fe_element_cpointer, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static void
CmissFEelement_dealloc(PyObject* self)
{
   CmissFEelementObject *cmiss_fe_element;
 	if (_CmissFEelement_check(self))
	{
		cmiss_fe_element = (CmissFEelementObject *)self;
		DEACCESS(FE_element)(&cmiss_fe_element->fe_element);
	}
	PyObject_Del(self);
}

static PyObject *
CmissFEelement_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissFEelement_methods, (PyObject *)self, name);
}

static PyObject*
CmissFEelement_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	if (_CmissFEelement_check(object))
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
CmissFEelement_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissFEelementObject *cmiss_fe_element;
	PyObject *cmiss_fe_element_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_fe_element_cpointer)
		&& PyCObject_Check(cmiss_fe_element_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_fe_element = PyObject_New(CmissFEelementObject, &CmissFEelementType);
	if (!(cmiss_fe_element->fe_element = ACCESS(FE_element)(
		(struct FE_element *)PyCObject_AsVoidPtr(cmiss_fe_element_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract FE_element pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_fe_element;
}

static PyObject *
CmissFEelement_repr(PyObject* self)
{
	char *name;
	CmissFEelementObject *cmiss_fe_element;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissFEelement_check(self))
	{		
		cmiss_fe_element = (CmissFEelementObject *)self;
		if (GET_NAME(FE_element)(cmiss_fe_element->fe_element, &name))
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

static PyTypeObject CmissFEelementType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "FE_element",
    sizeof(CmissFEelementObject),
    0,
    CmissFEelement_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissFEelement_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissFEelement_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissFEelementType_methods[] = {
    {"check", CmissFEelement_check, METH_VARARGS,
     "Check if object is of type Cmiss FEelement object."},
    {"wrap", CmissFEelement_wrap, METH_VARARGS,
     "Wrap a C CmissFEelement in a python Cmiss FEelement object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initFE_element(void) 
{
	CmissFEelementType.ob_type = &PyType_Type;
	
	Py_InitModule("FE_element", CmissFEelementType_methods);
}

#include <Python.h>
#include "computed_variable/computed_value.h"

staticforward PyTypeObject CmissValueType;

typedef struct {
    PyObject_HEAD
    struct Cmiss_value *value;
} CmissValueObject;

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

	 printf("Creating new CmissValue\n");

    return (PyObject*)cmiss_value;
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
    "Value",
    sizeof(CmissValueObject),
    0,
    CmissValue_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    0,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    0,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissValue_methods[] = {
    {"new", CmissValue_new, METH_VARARGS,
     "Create a new Cmiss Value object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initValue(void) 
{
	CmissValueType.ob_type = &PyType_Type;
	
	printf ("In initValue\n");

	Py_InitModule("Value", CmissValue_methods);
}

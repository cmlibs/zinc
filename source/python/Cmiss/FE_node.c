#include <Python.h>
#include "finite_element/finite_element.h"

staticforward PyTypeObject CmissFEnodeType;

/* Internal definition */
#define _CmissFEnode_check(object)  ((object)->ob_type == &CmissFEnodeType)

typedef struct {
    PyObject_HEAD
    struct FE_node *fe_node;
} CmissFEnodeObject;

/* Object Methods */

static PyObject*
CmissFEnode_get_fe_node_cpointer(PyObject* self, PyObject* args)
{
	CmissFEnodeObject *cmiss_fe_node;
	PyObject *object, *return_code;

	printf("CmissFEnode_get_fe_node_cpointer\n");

	if (_CmissFEnode_check(self))
	{
		cmiss_fe_node = (CmissFEnodeObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_fe_node->fe_node, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissFEnode_methods[] =
	{
		{"get_fe_node_cpointer", CmissFEnode_get_fe_node_cpointer, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static void
CmissFEnode_dealloc(PyObject* self)
{
   CmissFEnodeObject *cmiss_fe_node;
 	if (_CmissFEnode_check(self))
	{
		cmiss_fe_node = (CmissFEnodeObject *)self;
		DEACCESS(FE_node)(&cmiss_fe_node->fe_node);
	}
	PyObject_Del(self);
}

static PyObject *
CmissFEnode_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissFEnode_methods, (PyObject *)self, name);
}

static PyObject*
CmissFEnode_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissFEnode\n");

	if (_CmissFEnode_check(object))
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
CmissFEnode_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissFEnodeObject *cmiss_fe_node;
	PyObject *cmiss_fe_node_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_fe_node_cpointer)
		&& PyCObject_Check(cmiss_fe_node_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_fe_node = PyObject_New(CmissFEnodeObject, &CmissFEnodeType);
	if (!(cmiss_fe_node->fe_node = ACCESS(FE_node)(
		(struct FE_node *)PyCObject_AsVoidPtr(cmiss_fe_node_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract FE_node pointer.");
		return NULL;			 
	}

	printf("Wrapping Cmiss.FE_node\n");

	return (PyObject*)cmiss_fe_node;
}

static PyObject *
CmissFEnode_repr(PyObject* self)
{
	char *name;
	CmissFEnodeObject *cmiss_fe_node;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissFEnode_check(self))
	{		
		cmiss_fe_node = (CmissFEnodeObject *)self;
		if (GET_NAME(FE_node)(cmiss_fe_node->fe_node, &name))
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

static PyTypeObject CmissFEnodeType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "FE_node",
    sizeof(CmissFEnodeObject),
    0,
    CmissFEnode_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissFEnode_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissFEnode_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissFEnodeType_methods[] = {
    {"check", CmissFEnode_check, METH_VARARGS,
     "Check if object is of type Cmiss FEnode object."},
    {"wrap", CmissFEnode_wrap, METH_VARARGS,
     "Wrap a C CmissFEnode in a python Cmiss FEnode object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initFE_node(void) 
{
	CmissFEnodeType.ob_type = &PyType_Type;
	
	printf ("In initFE_node\n");

	Py_InitModule("FE_node", CmissFEnodeType_methods);
}

#include <Python.h>
#include "api/cmiss_value.h"
#include "api/cmiss_value_element_xi.h"
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_finite_element.h"
#include "computed_variable/computed_value_matrix.h"
#include "finite_element/finite_element.h"

PyTypeObject CmissValueElementxiType;

/* Internal definition */
#define _CmissValueElementxi_check(object)  ((object)->ob_type == &CmissValueElementxiType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_value *value;
} CmissValueElementxiObject;

/* Object Methods */
static PyObject*
CmissValueElementxi_get_value_cpointer(PyObject* self, PyObject* args)
{
	CmissValueElementxiObject *cmiss_value_elementxi;
	PyObject *return_code;
	struct Elementxi *value;

	if (_CmissValueElementxi_check(self))
	{
		cmiss_value_elementxi = (CmissValueElementxiObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_value_elementxi->value, NULL);	
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissValueElementxi_methods[] =
	{
		{"get_value_cpointer", CmissValueElementxi_get_value_cpointer, 1, NULL},
		{NULL, NULL, 0, NULL}
	};

/* Type Methods */

static PyObject*
CmissValueElementxi_new(PyObject* self, PyObject* args)
{
    CmissValueElementxiObject *cmiss_value;
	 FE_value *xi;
	 int i, number_of_xi;
	 struct FE_element *element_ptr;
	 PyObject *element, *element_cpointer, *float_item, *fe_element_module, *xi_array, *xi_array_item;

    if (!PyArg_ParseTuple(args,"OO:new", &element, &xi_array)) 
        return NULL;

	 if (!(fe_element_module = PyImport_ImportModule("Cmiss.FE_element")))
	 {
		 PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.FE_element module");
		 return NULL;
	 }
	 
	 if (!(PyObject_IsTrue(PyObject_CallMethod(fe_element_module, "check", "O", element))))
	 {
		 PyErr_SetString(PyExc_AttributeError, "First argument must be a Cmiss.FE_element");
		 return NULL;
	 }

	 if (!PyList_Check(xi_array))
	 {
		 PyErr_SetString(PyExc_AttributeError, "Second argument must be a list");
		 return NULL;
	 }

    cmiss_value = PyObject_New(CmissValueElementxiObject, &CmissValueElementxiType);
	 if (cmiss_value->value = CREATE(Cmiss_value)())
	 {
		 ACCESS(Cmiss_value)(cmiss_value->value);
		 if (!((element_cpointer = PyObject_CallMethod(element, "get_fe_element_cpointer", (char *)NULL)) &&
			 PyCObject_Check(element_cpointer)))
		 {
			 PyErr_SetString(PyExc_AttributeError, "Unable to extract element pointer from element.");
			 return NULL;			 
		 }
		 element_ptr = (struct FE_element *)PyCObject_AsVoidPtr(element_cpointer);

		 if (xi_array && (0<(number_of_xi=PyList_Size(xi_array))) &&
			 (get_FE_element_dimension(element_ptr)==number_of_xi))
		 {
			 if (xi=(FE_value *)malloc(number_of_xi*sizeof(FE_value)))
			 {
				 for (i = 0 ; i < number_of_xi ; i++)
				 {
					 xi_array_item = PyList_GetItem(xi_array, i);
					 if (!(float_item = PyNumber_Float(xi_array_item)))
					 {
						 PyErr_SetString(PyExc_AttributeError, "Second argument must be a list containing only numeric values");
						 return NULL;
					 }
					 xi[i] = (FE_value)PyFloat_AsDouble(float_item);
					 Py_DECREF(float_item);
				 }
				 if (!Cmiss_value_element_xi_set_type(cmiss_value->value, number_of_xi, element_ptr, xi))
				 {
					 free(xi);
					 DEACCESS(Cmiss_value)(&cmiss_value->value);
				 }
			 }
			 else
			 {
				 PyErr_SetString(PyExc_MemoryError, "Unable to allocate Elementxi_value storage for CmissValueElementxi.");
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
		 else
		 {
			 if (!Cmiss_value_element_xi_set_type(cmiss_value->value, 0, element_ptr, (FE_value *)NULL))
			 {
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
	 }

    return (PyObject*)cmiss_value;
}

static void
CmissValueElementxi_dealloc(PyObject* self)
{
	/* How do I check that this is really a CmissValueElementxi before the cast? */
    CmissValueElementxiObject *cmiss_value;
	 cmiss_value = (CmissValueElementxiObject *)self;
	 DEACCESS(Cmiss_value)(&cmiss_value->value);
    PyObject_Del(self);
}

static PyObject *
CmissValueElementxi_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissValueElementxi_methods, (PyObject *)self, name);
}

static PyObject*
CmissValueElementxi_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	if (_CmissValueElementxi_check(object))
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
CmissValueElementxi_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissValueElementxiObject *cmiss_value;
	PyObject *cmiss_value_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_value_cpointer)
		&& PyCObject_Check(cmiss_value_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_value = PyObject_New(CmissValueElementxiObject, &CmissValueElementxiType);
	if (!(cmiss_value->value = ACCESS(Cmiss_value)(
		(Cmiss_value_id)PyCObject_AsVoidPtr(cmiss_value_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Value pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_value;
}

static PyObject *
CmissValueElementxi_str(PyObject* self)
{
	char *element_name;
	CmissValueElementxiObject *cmiss_value;
	int dimension, i, number_of_xi;
	FE_value *xi;
	struct FE_element *element;
	PyObject *string;
	
	element_name=(char *)NULL;
	string = (PyObject *)NULL;
 	if (_CmissValueElementxi_check(self))
	{		
		cmiss_value = (CmissValueElementxiObject *)self;
		if (Cmiss_value_element_xi_get_type(cmiss_value->value, &dimension, &element, &xi)&&
			FE_element_to_any_element_string(element, &element_name))
		{
			string = PyString_FromFormat("[dimension = %d, element=%s, xi=[", dimension, element_name);
			if (0<(number_of_xi=get_FE_element_dimension(element)))
			{
				for (i = 0 ; i < number_of_xi ; i++)
				{
					PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
						xi[i])));
					if (i < number_of_xi - 1)
					{
						PyString_Concat(&string, PyString_FromString(","));
					}
				}
				PyString_Concat(&string, PyString_FromString("]"));
			}
			if (xi)
			{
				free(xi);
			}
			PyString_Concat(&string, PyString_FromString("]"));
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Element_xi");
	}
	return (string);
}

static PyObject *
CmissValueElementxi_repr(PyObject* self)
{
	CmissValueElementxiObject *cmiss_value;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissValueElementxi_check(self))
	{		
		string = PyString_FromString("Cmiss.Value.Element_xi.new=");
		PyString_Concat(&string, CmissValueElementxi_str(self));
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Element_xi");
	}
	return (string);
}

PyTypeObject CmissValueElementxiType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Element_xi",
    sizeof(CmissValueElementxiObject),
    0,
    CmissValueElementxi_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissValueElementxi_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissValueElementxi_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
	 0,          /*tp_call */
	 CmissValueElementxi_str,         /* tp_str */
};

static PyMethodDef CmissValueElementxiType_methods[] = {
    {"new", CmissValueElementxi_new, METH_VARARGS,
     "Create a new Cmiss Value Elementxi object."},
    {"check", CmissValueElementxi_check, METH_VARARGS,
     "Check if object is of type Cmiss Value Elementxi object."},
    {"wrap", CmissValueElementxi_wrap, METH_VARARGS,
     "Wrap a C CmissValue in a python Cmiss Value Elementxi object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initElement_xi(void) 
{
	CmissValueElementxiType.ob_type = &PyType_Type;
	
	Py_InitModule("Element_xi", CmissValueElementxiType_methods);
}

#include <Python.h>
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_fe_value.h"

PyTypeObject CmissValueFEvaluevectorType;

/* Internal definition */
#define _CmissValueFEvaluevector_check(object)  ((object)->ob_type == &CmissValueFEvaluevectorType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_value *value;
} CmissValueFEvaluevectorObject;

/* Object Methods */
#if defined (OLD_CODE)
static PyObject*
CmissValueFEvaluevector_get_matrix_cpointer(PyObject* self, PyObject* args)
{
	CmissValueFEvaluevectorObject *cmiss_value_matrix;
	PyObject *object, *return_code;
	struct FEvaluevector *matrix;

	if (_CmissValueFEvaluevector_check(self))
	{
		cmiss_value_matrix = (CmissValueFEvaluevectorObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_value_matrix->value, NULL);	
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}
#endif /* defined (OLD_CODE) */

static struct PyMethodDef CmissValueFEvaluevector_methods[] =
	{
#if defined (OLD_CODE)
		{"get_matrix_cpointer", CmissValueFEvaluevector_get_matrix_cpointer, 1},
#endif /* defined (OLD_CODE) */
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissValueFEvaluevector_new(PyObject* self, PyObject* args)
{
    CmissValueFEvaluevectorObject *cmiss_value;
	 FE_value *values;
	 int i, number_of_values;
	 PyObject *float_item, *values_array, *values_array_item;

    if (!PyArg_ParseTuple(args,"O:new", &values_array)) 
        return NULL;

	 if (!PyList_Check(values_array))
	 {
		 PyErr_SetString(PyExc_AttributeError, "Argument must be a list");
		 return NULL;
	 }

    cmiss_value = PyObject_New(CmissValueFEvaluevectorObject, &CmissValueFEvaluevectorType);
	 if (cmiss_value->value = CREATE(Cmiss_value)())
	 {
		 ACCESS(Cmiss_value)(cmiss_value->value);
		 if (values_array && (0<(number_of_values = PyList_Size(values_array))))
		 {
			 if (values = (FE_value *)malloc(number_of_values*sizeof(FE_value)))
			 {
				 for (i = 0 ; i < number_of_values ; i++)
				 {
					 values_array_item = PyList_GetItem(values_array, i);
					 if (!(float_item = PyNumber_Float(values_array_item)))
					 {
						 PyErr_SetString(PyExc_AttributeError, "Argument must be a list containing only numeric values");
						 return NULL;
					 }
					 values[i] = (FE_value)PyFloat_AsDouble(float_item);
					 Py_DECREF(float_item);
				 }
				 if (!Cmiss_value_FE_value_vector_set_type(cmiss_value->value, number_of_values, values))
				 {
					 free(values);
					 DEACCESS(Cmiss_value)(&cmiss_value->value);
				 }
			 }
			 else
			 {
				 PyErr_SetString(PyExc_MemoryError, "Unable to allocate FE_value_vector storage for CmissValueFEvaluevector.");
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
		 else
		 {
			 if (!Cmiss_value_FE_value_vector_set_type(cmiss_value->value, 0, (FE_value *)NULL))
			 {
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
	 }

    return (PyObject*)cmiss_value;
}

static void
CmissValueFEvaluevector_dealloc(PyObject* self)
{
	/* How do I check that this is really a CmissValueFEvaluevector before the cast? */
    CmissValueFEvaluevectorObject *cmiss_value;
	 cmiss_value = (CmissValueFEvaluevectorObject *)self;
	 DEACCESS(Cmiss_value)(&cmiss_value->value);
    PyObject_Del(self);
}

static PyObject *
CmissValueFEvaluevector_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissValueFEvaluevector_methods, (PyObject *)self, name);
}

static PyObject*
CmissValueFEvaluevector_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	if (_CmissValueFEvaluevector_check(object))
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
CmissValueFEvaluevector_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissValueFEvaluevectorObject *cmiss_value;
	PyObject *cmiss_value_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_value_cpointer)
		&& PyCObject_Check(cmiss_value_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_value = PyObject_New(CmissValueFEvaluevectorObject, &CmissValueFEvaluevectorType);
	if (!(cmiss_value->value = ACCESS(Cmiss_value)(
		(Cmiss_value_id)PyCObject_AsVoidPtr(cmiss_value_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Value pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_value;
}

static PyObject *
CmissValueFEvaluevector_str(PyObject* self)
{
	CmissValueFEvaluevectorObject *cmiss_value;
	int i, number_of_values;
	FE_value *values;
	PyObject *string;
	
	string = (PyObject *)NULL;
 	if (_CmissValueFEvaluevector_check(self))
	{		
		cmiss_value = (CmissValueFEvaluevectorObject *)self;
		if (Cmiss_value_FE_value_vector_get_type(cmiss_value->value, &number_of_values, &values))
		{
			string = PyString_FromString("[");
			for (i = 0 ; i < number_of_values ; i++)
			{
				PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
					values[i])));
				if (i < number_of_values - 1)
				{
					PyString_Concat(&string, PyString_FromString(","));
				}
			}
			PyString_Concat(&string, PyString_FromString("]"));
		}
		if (values)
		{
			free(values);
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.FE_value_vector");
	}
	return (string);
}

static PyObject *
CmissValueFEvaluevector_repr(PyObject* self)
{
	CmissValueFEvaluevectorObject *cmiss_value;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissValueFEvaluevector_check(self))
	{		
		string = PyString_FromString("Cmiss.Value.FE_value_vector.new=");
		PyString_Concat(&string, CmissValueFEvaluevector_str(self));
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.FE_value_vector");
	}
	return (string);
}

PyTypeObject CmissValueFEvaluevectorType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "FE_value_vector",
    sizeof(CmissValueFEvaluevectorObject),
    0,
    CmissValueFEvaluevector_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissValueFEvaluevector_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissValueFEvaluevector_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
	 0,          /*tp_call */
	 CmissValueFEvaluevector_str,         /* tp_str */
};

static PyMethodDef CmissValueFEvaluevectorType_methods[] = {
    {"new", CmissValueFEvaluevector_new, METH_VARARGS,
     "Create a new Cmiss Value FE_value_vector object."},
    {"check", CmissValueFEvaluevector_check, METH_VARARGS,
     "Check if object is of type Cmiss Value FE_value_vector object."},
    {"wrap", CmissValueFEvaluevector_wrap, METH_VARARGS,
     "Wrap a C CmissValue in a python Cmiss Value FE_value_vector object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initFE_value_vector(void) 
{
	CmissValueFEvaluevectorType.ob_type = &PyType_Type;
	
	Py_InitModule("FE_value_vector", CmissValueFEvaluevectorType_methods);
}

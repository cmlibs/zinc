#include <Python.h>
#include "api/cmiss_finite_element.h"
#include "api/cmiss_value.h"
#include "api/cmiss_value_element_xi.h"

static PyObject*
CmissValueElementxi_new(PyObject* self, PyObject* args)
{
	Cmiss_value_id value;
	float *xi;
	int i, number_of_xi;
	struct FE_element *element_ptr;
	PyObject *cmiss_value, *element, *element_cpointer, *float_item, *fe_element_module, 
		*value_module, *xi_array, *xi_array_item;

	if (!(value_module = PyImport_ImportModule("Cmiss.Value.C.Value")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.C.Value module");
		return NULL;
	}

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

	cmiss_value = (PyObject *)NULL;
	if (!((element_cpointer = PyObject_CallMethod(element, "get_fe_element_cpointer", (char *)NULL)) &&
			 PyCObject_Check(element_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract element pointer from element.");
		return NULL;			 
	}
	element_ptr = (struct FE_element *)PyCObject_AsVoidPtr(element_cpointer);

	if (xi_array && (0<(number_of_xi=PyList_Size(xi_array))))
	{
		if (xi=(float *)malloc(number_of_xi*sizeof(float)))
		{
			for (i = 0 ; i < number_of_xi ; i++)
			{
				xi_array_item = PyList_GetItem(xi_array, i);
				if (!(float_item = PyNumber_Float(xi_array_item)))
				{
					PyErr_SetString(PyExc_AttributeError, "Second argument must be a list containing only numeric values");
					return NULL;
				}
				xi[i] = PyFloat_AsDouble(float_item);
				Py_DECREF(float_item);
			}
			if (value = CREATE(Cmiss_value_element_xi)(number_of_xi, element_ptr, xi))
			{
				cmiss_value = PyObject_CallMethod(value_module, "wrap", "O",
					PyCObject_FromVoidPtr(value, NULL));
			}
			else
			{
				free(xi);
			}
		}
		else
		{
			PyErr_SetString(PyExc_MemoryError, "Unable to allocate Elementxi_value storage for CmissValueElementxi.");
		}
	}
	else
	{
		if (value = CREATE(Cmiss_value_element_xi)(0, element_ptr, (float *)NULL))
		{
			cmiss_value = PyObject_CallMethod(value_module, "wrap", "O",
				PyCObject_FromVoidPtr(value, NULL));
		}
	}

	return cmiss_value;
}

static PyMethodDef CmissValueElementxiModule_methods[] = {
    {"new", CmissValueElementxi_new, METH_VARARGS,
     "Create a new Cmiss Value Elementxi object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initElement_xi(void) 
{
	Py_InitModule("Cmiss.Value.C.Element_xi", CmissValueElementxiModule_methods);
}

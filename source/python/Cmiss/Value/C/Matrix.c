#include <Python.h>
#include "api/cmiss_value.h"
#include "api/cmiss_value_matrix.h"

/* Module Methods */

static PyObject*
CmissValueMatrix_sub_matrix(PyObject* self, PyObject* args, PyObject *keywds)
{
	Cmiss_value_id matrix, sub_matrix;
	int column_high,column_low,number_of_columns,number_of_rows,row_high,
		row_low;
	static char *kwlist[] = {"matrix", "row_low", "row_high", "column_low", "column_high", NULL};
	PyObject *matrix_value, *value_cpointer, *sub_matrix_value, *value_c_pointer, 
		*value_C_module, *value_C_object, *value_module;

	if (!(value_C_module = PyImport_ImportModule("Cmiss.Value.C.Value")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.C.Value module");
		return NULL;
	}
	 
	if (!(value_module = PyImport_ImportModule("Cmiss.Value")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value module");
		return NULL;
	}
	 
	sub_matrix_value = (PyObject *)NULL;

	row_low=1;
	row_high=0;
	column_low=1;
	column_high=0;

	PyArg_ParseTupleAndKeywords(args, keywds, "O|iiii:sub_matrix", kwlist,
		&matrix_value, &row_low, &row_high, &column_low, &column_high);
	
	if ((value_cpointer = PyObject_CallMethod(matrix_value,
			  "get_value_cpointer", (char *)NULL)) && PyCObject_Check(value_cpointer))
	{
		matrix = PyCObject_AsVoidPtr(value_cpointer);
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract value pointer from Matrix.");
		return NULL;
	}

	if (Cmiss_value_matrix_get_dimensions(matrix, &number_of_rows,
		&number_of_columns))
	{
		if (!row_high)
		{
			row_high = number_of_rows;
		}
		if (!column_high)
		{
			column_high = number_of_columns;
		}
		if (sub_matrix = Cmiss_value_matrix_get_submatrix(matrix,row_low,row_high,
			column_low,column_high))
		{
			/* Wrap the Cmiss_value in a python C api object */
			if (value_C_object = PyObject_CallMethod(value_C_module, "wrap", "O",
				PyCObject_FromVoidPtr(sub_matrix, NULL)))
			{
				/* Pass the C api object to the python object constructor of the correct type */
				if (!(sub_matrix_value = PyObject_CallMethod(value_module, "Matrix", "O",
					value_C_object)))
				{
					PyErr_SetString(PyExc_AttributeError,
						"Unable to create sub matrix python Cmiss.Value.Matrix");
				}
			}
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError,
				"Unable to extract sub matrix");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError,
			"Unable to get matrix dimensions");
	}

	return(sub_matrix_value);
}

static PyObject*
CmissValueMatrix_new(PyObject* self, PyObject* args)
{
	double *values;
	int i,number_of_columns,number_of_rows,number_of_values;
	Cmiss_value_id value, *matrices;
	PyObject *cmiss_value, *float_item, *value_module, *values_array, *values_array_item;

	if (!(value_module = PyImport_ImportModule("Cmiss.Value.C.Value")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.C.Value module");
		return NULL;
	}

	if (!PyArg_ParseTuple(args,"Oi:new", &values_array, &number_of_columns)) 
		return NULL;

	if (!PyList_Check(values_array))
	{
		PyErr_SetString(PyExc_AttributeError, "First argument must be a list");
		return NULL;
	}

	cmiss_value = (PyObject *)NULL;
	if (values_array && (0<(number_of_values = PyList_Size(values_array))))
	{
		number_of_rows = number_of_values / number_of_columns;
		if (values = (double *)malloc(number_of_values*sizeof(double)))
		{
			for (i = 0 ; i < number_of_values ; i++)
			{
				values_array_item = PyList_GetItem(values_array, i);
				if (!(float_item = PyNumber_Float(values_array_item)))
				{
					PyErr_SetString(PyExc_AttributeError, "Argument must be a list containing only numeric values");
					return NULL;
				}
				values[i] = (double)PyFloat_AsDouble(float_item);
				Py_DECREF(float_item);
			}
			if (value = CREATE(Cmiss_value_matrix)(number_of_rows, number_of_columns, values))
			{
				cmiss_value = PyObject_CallMethod(value_module, "wrap", "O",
					PyCObject_FromVoidPtr(value, NULL));
			}
			else
			{
				PyErr_SetString(PyExc_MemoryError, "Unable to create Cmiss_value_matrix.");
			}
		}
		else
		{
			PyErr_SetString(PyExc_MemoryError, "Unable to allocate FE_value_vector storage for CmissValueFEvaluevector.");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "Missing values array.");
	}

	return cmiss_value;
}

static PyMethodDef CmissValueMatrixModule_methods[] = {
    {"new", CmissValueMatrix_new, METH_VARARGS,
     "Create a new Cmiss Value Matrix object."},
    {"sub_matrix", (PyCFunction)CmissValueMatrix_sub_matrix, METH_VARARGS|METH_KEYWORDS,
     "Extract a sub matrix from a Cmiss Value Matrix."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initMatrix(void) 
{
	Py_InitModule("Cmiss.Value.C.Matrix", CmissValueMatrixModule_methods);
}

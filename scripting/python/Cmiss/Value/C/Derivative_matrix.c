#include <Python.h>
#include "api/cmiss_value.h"
#include "api/cmiss_value_derivative_matrix.h"

/* Module Methods */

static PyObject*
CmissValueDerivativematrix_matrix(PyObject* self, PyObject* args)
{
	Cmiss_value_id derivative_matrix, matrix;
	Cmiss_variable_id *independent_variable_ptrs;
	int i, order;
	PyObject *cmiss_derivative_matrix, *independent_variable, *independent_variables_array,
		*return_code, *value_C_module, *value_C_object, *value_module, *variable_cpointer,
		*variable_module;

	return_code = (PyObject *)NULL;

	if (!PyArg_ParseTuple(args,"OO:matrix", &cmiss_derivative_matrix, &independent_variables_array))
		return NULL;

	/* Make sure we load the variable module */
	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

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
	 
	if (!PyList_Check(independent_variables_array))
	{
		PyErr_SetString(PyExc_AttributeError, "Argument must be a list");
		return NULL;
	}

	if ((variable_cpointer = PyObject_CallMethod(cmiss_derivative_matrix,
		"get_value_cpointer", (char *)NULL)) && PyCObject_Check(variable_cpointer))
	{
		derivative_matrix = PyCObject_AsVoidPtr(variable_cpointer);
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract value pointer from Derivative_matrix.");
		return NULL;
	}

	if ( 0 < (order = PyList_Size(independent_variables_array)))
	{
		if (independent_variable_ptrs=(Cmiss_variable_id *)malloc(order*
				 sizeof(Cmiss_variable_id)))
		{
			i=0;
			while ((i < order) && (independent_variable = PySequence_GetItem(independent_variables_array, i)))
			{
				if (!((variable_cpointer = PyObject_CallMethod(independent_variable,
							 "get_variable_cpointer", (char *)NULL)) &&
						 PyCObject_Check(variable_cpointer)))
				{
					PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from variable value array.");
					return NULL;
				}
				independent_variable_ptrs[i] = PyCObject_AsVoidPtr(variable_cpointer);
				i++;
			}
			if (i == order)
			{
				Cmiss_value_derivative_matrix_get_matrix(derivative_matrix,
					order, independent_variable_ptrs, &matrix);
				/* Wrap the Cmiss_value in a python C api object */
				if (value_C_object = PyObject_CallMethod(value_C_module, "wrap", "O",
					PyCObject_FromVoidPtr(matrix, NULL)))
				{
					/* Pass the C api object to the python object constructor of the correct type */
					if (!(return_code  = PyObject_CallMethod(value_module, "Matrix", "O",
						value_C_object)))
					{
						PyErr_SetString(PyExc_AttributeError,
							"Unable to create python Cmiss.Value.Matrix");
					}
				}
			}
			free(independent_variable_ptrs);
		}
	}

	return(return_code);
}

static PyObject*
CmissValueDerivativematrix_new(PyObject* self, PyObject* args)
{
	int i,number_of_matrices,order,return_code;
	Cmiss_value_id value, *matrices;
	Cmiss_variable_id *independent_variables;
	Cmiss_variable_id dependent_variable_ptr;
	PyObject *cmiss_value, *dependent_variable, *dependent_variable_cpointer, *independent_variable,
		*independent_variable_cpointer, *independent_variables_array,
		*matrices_array, *matrix, *matrix_cpointer, *value_module;


	if (!(value_module = PyImport_ImportModule("Cmiss.Value.C.Value")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.C.Value module");
		return NULL;
	}

	if (!PyArg_ParseTuple(args,"OOO:new", &dependent_variable, &independent_variables_array,
			 &matrices_array)) 
		return NULL;

	if (!PyList_Check(independent_variables_array))
	{
		PyErr_SetString(PyExc_AttributeError, "Second argument must be a list");
		return NULL;
	}

	if (!((dependent_variable_cpointer = PyObject_CallMethod(dependent_variable,
				 "get_variable_cpointer", (char *)NULL)) &&
			 PyCObject_Check(dependent_variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from dependent variable.");
		return NULL;			 
	}
	dependent_variable_ptr = (Cmiss_variable_id)PyCObject_AsVoidPtr(dependent_variable_cpointer);

	cmiss_value = (PyObject *)NULL;
	return_code = 1;
	if ( 0 < (order = PyList_Size(independent_variables_array)))
	{
		if (independent_variables=(Cmiss_variable_id *)malloc(order*
				 sizeof(Cmiss_variable_id)))
		{
			i=0;
			number_of_matrices=0;
			while ((i < order) &&
				(independent_variable = PySequence_GetItem(independent_variables_array, i)))
			{						
				if (!((independent_variable_cpointer = PyObject_CallMethod(independent_variable,
							 "get_variable_cpointer", (char *)NULL)) &&
						 PyCObject_Check(independent_variable_cpointer)))
				{
					PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from variable value array.");
					return_code = 0;
				}
				independent_variables[i] = PyCObject_AsVoidPtr(independent_variable_cpointer);
				i++;
				number_of_matrices=2*number_of_matrices+1;
			}
			if (return_code && (i == order))
			{
				if (PyList_Check(matrices_array) &&
					(number_of_matrices==PyList_Size(matrices_array)) &&
					(matrices=(Cmiss_value_id *)malloc(number_of_matrices*
						sizeof(Cmiss_value_id))))
				{
					i=0;
					while ((i < number_of_matrices) &&
						(matrix = PySequence_GetItem(matrices_array, i))
						&& (matrix_cpointer = PyObject_CallMethod(matrix,
						"get_value_cpointer", (char *)NULL)))
					{
						matrices[i] = (Cmiss_value_id)PyCObject_AsVoidPtr(matrix_cpointer);
						i++;
					}
					if (i != number_of_matrices)
					{
						return_code = 0;
						free(independent_variables);
						PyErr_SetString(PyExc_AttributeError, "Invalid type in matrix array.");
					}
				}
				else
				{
					matrices=(Cmiss_value_id *)NULL;
				}
				if (return_code)
				{
					if (value = CREATE(Cmiss_value_derivative_matrix)(
						dependent_variable_ptr, order, independent_variables, number_of_matrices,
						matrices))
					{
						cmiss_value = PyObject_CallMethod(value_module, "wrap", "O",
							PyCObject_FromVoidPtr(value, NULL));
					}
					else
					{
						if (matrices)
						{
							free(matrices);
						}
						free(independent_variables);
					}
				}
			}
			else
			{
				free(independent_variables);
				PyErr_SetString(PyExc_AttributeError, "Invalid type in dependent variable array.");
			}
		}
	}

	return cmiss_value;
}

static PyMethodDef CmissValueDerivativematrixModule_methods[] = {
    {"new", CmissValueDerivativematrix_new, METH_VARARGS,
     "Create a new Cmiss Value Derivative_matrix object."},
    {"matrix", CmissValueDerivativematrix_matrix, METH_VARARGS,
     "Extracts a Matrix value from a Derivative_matrix value."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initDerivative_matrix(void) 
{
	Py_InitModule("Cmiss.Value.C.Derivative_matrix", CmissValueDerivativematrixModule_methods);
}

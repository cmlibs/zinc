#include <Python.h>
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_variable.h"
#include "python/Cmiss/Value/Value.h"
#include "python/Cmiss/Value/Matrix.h"
#include "python/Cmiss/Variable/Variable.h"

staticforward PyTypeObject CmissValueDerivativematrixType;

/* Internal definition */
#define _CmissValueDerivativematrix_check(object)  ((object)->ob_type == &CmissValueDerivativematrixType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_value *value;
} CmissValueDerivativematrixObject;

/* Object Methods */
static PyObject*
CmissValueDerivativematrix_matrix(PyObject* self, PyObject* args)
{
	CmissValueDerivativematrixObject *cmiss_derivative_matrix;
	Cmiss_value_id matrix;
	Cmiss_variable_id *independent_variable_ptrs;
	int i, order;
	PyObject *independent_variable, *independent_variables_array, *return_code,
		*value_matrix_module, *variable_cpointer, *variable_module;

	return_code = (PyObject *)NULL;

	if (!PyArg_ParseTuple(args,"O:new", &independent_variables_array))
		return NULL;

	/* Make sure we load the variable module */
	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (!(value_matrix_module = PyImport_ImportModule("Cmiss.Value.Matrix")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.Matrix module");
		return NULL;
	}
	 
	if (!PyList_Check(independent_variables_array))
	{
		PyErr_SetString(PyExc_AttributeError, "Second argument must be a list");
		return NULL;
	}

	if (_CmissValueDerivativematrix_check(self))
	{
		cmiss_derivative_matrix = (CmissValueDerivativematrixObject *)self;
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
					Cmiss_value_derivative_matrix_get_matrix(cmiss_derivative_matrix->value,
						order, independent_variable_ptrs, &matrix);
					return_code = PyObject_CallMethod(value_matrix_module, "wrap", "O",
						PyCObject_FromVoidPtr(matrix, NULL));
				}
				free(independent_variable_ptrs);
			}
		}
	}

	return(return_code);
}

static struct PyMethodDef CmissValueDerivativematrix_methods[] =
	{
		{"matrix", CmissValueDerivativematrix_matrix, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissValueDerivativematrix_new(PyObject* self, PyObject* args)
{
    CmissValueDerivativematrixObject *cmiss_value;
	 int i,number_of_matrices,order;
	 Cmiss_value_id *matrices;
	 Cmiss_variable_id *independent_variables;
	 Cmiss_variable_id dependent_variable_ptr;
	 PyObject *dependent_variable, *dependent_variable_cpointer, *independent_variable,
		 *independent_variable_cpointer, *independent_variables_array,
		 *matrices_array, *matrix, *matrix_cpointer, *value_matrix_module, *variable_module;

    if (!PyArg_ParseTuple(args,"OOO:new", &dependent_variable, &independent_variables_array,
			  &matrices_array)) 
        return NULL;

	 /* Make sure we load the variable module */
	 if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	 {
		 PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		 return NULL;
	 }

	 if (!(value_matrix_module = PyImport_ImportModule("Cmiss.Value.Matrix")))
	 {
		 PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.Matrix module");
		 return NULL;
	 }
	 
	 if (!(PyObject_IsTrue(PyObject_CallMethod(variable_module, "check", "O", dependent_variable))))
	 {
		 PyErr_SetString(PyExc_AttributeError, "First argument must be a Cmiss.Variable.Variable");
		 return NULL;
	 }

	 if (!PyList_Check(independent_variables_array))
	 {
		 PyErr_SetString(PyExc_AttributeError, "Second argument must be a list");
		 return NULL;
	 }

    cmiss_value = PyObject_New(CmissValueDerivativematrixObject, &CmissValueDerivativematrixType);
	 if (cmiss_value->value = CREATE(Cmiss_value)())
	 {

		 ACCESS(Cmiss_value)(cmiss_value->value);

		 if (!((dependent_variable_cpointer = PyObject_CallMethod(dependent_variable,
			 "get_variable_cpointer", (char *)NULL)) &&
			 PyCObject_Check(dependent_variable_cpointer)))
		 {
			 PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from dependent variable.");
			 return NULL;			 
		 }
				  
		 dependent_variable_ptr = (Cmiss_variable_id)PyCObject_AsVoidPtr(dependent_variable_cpointer);
		 
		 if ( 0 < (order = PyList_Size(independent_variables_array)))
		 {
			 if (independent_variables=(Cmiss_variable_id *)malloc(order*
				 sizeof(Cmiss_variable_id)))
			 {
				 i=0;
				 number_of_matrices=0;
				 while ((i < order) &&
					 (independent_variable = PySequence_GetItem(independent_variables_array, i))
					 && PyObject_IsTrue(PyObject_CallMethod(variable_module, "check", "O", dependent_variable)))
					{						
						if (!((independent_variable_cpointer = PyObject_CallMethod(independent_variable,
							"get_variable_cpointer", (char *)NULL)) &&
							 PyCObject_Check(independent_variable_cpointer)))
						{
							PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from variable value array.");
							return NULL;
						}
						independent_variables[i] = PyCObject_AsVoidPtr(independent_variable_cpointer);
						i++;
						number_of_matrices=2*number_of_matrices+1;
					}
					if (i == order)
					{
						if (PyList_Check(matrices_array) &&
							(number_of_matrices==PyList_Size(matrices_array)) &&
							(matrices=(Cmiss_value_id *)malloc(number_of_matrices*
								sizeof(Cmiss_value_id))))
						{
							i=0;
							while ((i < number_of_matrices) &&
								(matrix = PySequence_GetItem(matrices_array, i))
								&& PyObject_IsTrue(PyObject_CallMethod(value_matrix_module, "check", "O", matrix)))
							{
								if (!((matrix_cpointer = PyObject_CallMethod(matrix,
									"get_matrix_cpointer", (char *)NULL)) &&
									PyCObject_Check(matrix_cpointer)))
								{
									PyErr_SetString(PyExc_AttributeError, "Unable to extract matrix pointer from matrices array.");
									return NULL;
								}
								matrices[i] = (Cmiss_value_id)PyCObject_AsVoidPtr(matrix_cpointer);
								i++;
							}
							if (i != number_of_matrices)
							{
								free(independent_variables);
								DEACCESS(Cmiss_value)(&cmiss_value->value);
								PyErr_SetString(PyExc_AttributeError, "Invalid type in matrix array.");
							}
						}
						else
						{
							matrices=(Cmiss_value_id *)NULL;
						}
						if (cmiss_value->value)
						{
							if (!Cmiss_value_derivative_matrix_set_type(cmiss_value->value,
								dependent_variable_ptr, order, independent_variables, matrices))
							{
								free(matrices);
								free(independent_variables);
								DEACCESS(Cmiss_value)(&cmiss_value->value);
							}
						}
					}
					else
					{
						free(independent_variables);
						DEACCESS(Cmiss_value)(&cmiss_value->value);
						PyErr_SetString(PyExc_AttributeError, "Invalid type in dependent variable array.");
					}
			 }
			 else
			 {
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
		 else
		 {
			 DEACCESS(Cmiss_value)(&cmiss_value->value);
		 }
	 }

    return (PyObject*)cmiss_value;
}

static void
CmissValueDerivativematrix_dealloc(PyObject* self)
{
    CmissValueDerivativematrixObject *cmiss_value;

	 if (_CmissValueDerivativematrix_check(self))
	 {
		 cmiss_value = (CmissValueDerivativematrixObject *)self;
		 DEACCESS(Cmiss_value)(&cmiss_value->value);
	 }
    PyObject_Del(self);
}

static PyObject*
CmissValueDerivativematrix_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissValueDerivativematrixObject *cmiss_value;
	PyObject *cmiss_value_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_value_cpointer)
		&& PyCObject_Check(cmiss_value_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_value = PyObject_New(CmissValueDerivativematrixObject, &CmissValueDerivativematrixType);
	if (!(cmiss_value->value = ACCESS(Cmiss_value)(
		(Cmiss_value_id)PyCObject_AsVoidPtr(cmiss_value_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Value pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_value;
}

static PyObject *
CmissValueDerivativematrix_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissValueDerivativematrix_methods, (PyObject *)self, name);
}

static PyObject *
CmissValueDerivativematrix_repr(PyObject* self)
{
	CmissValueDerivativematrixObject *cmiss_value;
	Cmiss_value_id *matrices;
	Cmiss_variable_id dependent_variable_ptr,*independent_variables;
	int i,number_of_matrices,offset,order;
	PyObject *dependent_variable, *independent_variable, *string, *variable_module;

	/* Make sure we load the variable module */
	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	string = (PyObject *)NULL;
 	if (_CmissValueDerivativematrix_check(self))
	{		
		cmiss_value = (CmissValueDerivativematrixObject *)self;
		if (Cmiss_value_derivative_matrix_get_type(cmiss_value->value,
			&dependent_variable_ptr,&order,&independent_variables,&matrices)&&
			dependent_variable&&(0<order)&&independent_variables)
		{
			/* SAB Should we create new wrappers for all these objects? */
			string = PyString_FromString("Cmiss.Value.Derivative_matrix.new(");
			dependent_variable = PyObject_CallMethod(variable_module, "wrap", "O",
				PyCObject_FromVoidPtr(dependent_variable_ptr, NULL));
			PyString_Concat(&string, PyObject_Str(dependent_variable));
			PyString_Concat(&string, PyString_FromString(", ["));
			for (i = 0 ; i < order ; i++)
			{
				independent_variable = PyObject_CallMethod(variable_module, "wrap", "O",
					PyCObject_FromVoidPtr(independent_variables[i], NULL));
				PyString_Concat(&string, PyObject_Str(independent_variable));
				if (i < order - 1)
				{
					PyString_Concat(&string, PyString_FromString(","));
				}
			}
			PyString_Concat(&string, PyString_FromString("])"));
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to get type from Cmiss.Value.Derivativematrix");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Derivativematrix");
	}
	return (string);
}

static PyObject *
CmissValueDerivativematrix_str(PyObject* self)
{
	CmissValueDerivativematrixObject *cmiss_value;
	Cmiss_value_id *matrices;
	Cmiss_variable_id dependent_variable_ptr,*independent_variables;
	int i,number_of_matrices,offset,order;
	PyObject *dependent_variable, *independent_variable, *matrix, *string, *value_matrix_module,
		*variable_module;

	/* Make sure we load the variable module */
	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (!(value_matrix_module = PyImport_ImportModule("Cmiss.Value.Matrix")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Value.Matrix module");
		return NULL;
	}
	 
	string = (PyObject *)NULL;
 	if (_CmissValueDerivativematrix_check(self))
	{		
		cmiss_value = (CmissValueDerivativematrixObject *)self;
		if (Cmiss_value_derivative_matrix_get_type(cmiss_value->value,
			&dependent_variable_ptr,&order,&independent_variables,&matrices)&&
			dependent_variable&&(0<order)&&independent_variables)
		{
			number_of_matrices = 0;
			/* SAB Should we create new wrappers for all these objects? */
			string = PyString_FromFormat("d(%d)", order);
			dependent_variable = PyObject_CallMethod(variable_module, "wrap", "O",
				PyCObject_FromVoidPtr(dependent_variable_ptr, NULL));
			PyString_Concat(&string, PyObject_Str(dependent_variable));
			PyString_Concat(&string, PyString_FromString("/d("));
			for (i = 0 ; i < order ; i++)
			{
				independent_variable = PyObject_CallMethod(variable_module, "wrap", "O",
					PyCObject_FromVoidPtr(independent_variables[i], NULL));
				PyString_Concat(&string, PyObject_Str(independent_variable));
				if (i < order - 1)
				{
					PyString_Concat(&string, PyString_FromString(","));
				}
				number_of_matrices = 2 * number_of_matrices + 1;
			}
			PyString_Concat(&string, PyString_FromString(")="));
			/* Write out just the last matrix */
			matrix = PyObject_CallMethod(value_matrix_module, "wrap", "O",
				PyCObject_FromVoidPtr(matrices[number_of_matrices - 1], NULL));
			PyString_Concat(&string, PyObject_Str(matrix));
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to get type from Cmiss.Value.Derivativematrix");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Derivativematrix");
	}
	return (string);
}

static PyTypeObject CmissValueDerivativematrixType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Derivative_matrix",
    sizeof(CmissValueDerivativematrixObject),
    0,
    CmissValueDerivativematrix_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissValueDerivativematrix_getattr,  /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissValueDerivativematrix_repr,    /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
	 0,          /*tp_call */
	 CmissValueDerivativematrix_str,    /* tp_str */
};

static PyMethodDef CmissValueDerivativematrixType_methods[] = {
    {"new", CmissValueDerivativematrix_new, METH_VARARGS,
     "Create a new Cmiss Value Derivative_matrix object."},
    {"wrap", CmissValueDerivativematrix_wrap, METH_VARARGS,
     "Wrap a C CmissValue in a python Cmiss Value Derivative_matrix object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initDerivative_matrix(void) 
{
	CmissValueDerivativematrixType.ob_type = &PyType_Type;
	
	Py_InitModule("Derivative_matrix", CmissValueDerivativematrixType_methods);
}

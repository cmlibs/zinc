#include <Python.h>
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_matrix.h"
#include "matrix/matrix.h"

PyTypeObject CmissValueMatrixType;

/* Internal definition */
#define _CmissValueMatrix_check(object)  ((object)->ob_type == &CmissValueMatrixType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_value *value;
} CmissValueMatrixObject;

/* Object Methods */

static PyObject*
CmissValueMatrix_get_matrix_cpointer(PyObject* self, PyObject* args)
{
	CmissValueMatrixObject *cmiss_value_matrix;
	PyObject *return_code;
	struct Matrix *matrix;

	if (_CmissValueMatrix_check(self))
	{
		cmiss_value_matrix = (CmissValueMatrixObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_value_matrix->value, NULL);	
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static PyObject*
CmissValueMatrix_sub_matrix(PyObject* self, PyObject* args, PyObject *keywds)
{
	CmissValueMatrixObject *cmiss_value_matrix, *return_code;
	int column_high,column_low,number_of_columns,number_of_rows,row_high,
		row_low;
	static char *kwlist[] = {"row_low", "row_high", "column_low", "column_high", NULL};


	return_code = (CmissValueMatrixObject *)NULL;
	if (_CmissValueMatrix_check(self))
	{
		cmiss_value_matrix = (CmissValueMatrixObject *)self;

		if (Cmiss_value_matrix_get_dimensions(cmiss_value_matrix->value, &number_of_rows,
			&number_of_columns))
		{
			row_low=1;
			row_high=number_of_rows;
			column_low=1;
			column_high=number_of_columns;

			PyArg_ParseTupleAndKeywords(args, keywds, "|iiii:sub_matrix", kwlist,
				&row_low, &row_high, &column_low, &column_high);

			return_code = PyObject_New(CmissValueMatrixObject, &CmissValueMatrixType);
			if (!(return_code->value = ACCESS(Cmiss_value)(
				Cmiss_value_matrix_get_submatrix(cmiss_value_matrix->value,row_low,row_high,
				column_low,column_high))))
			{
				PyErr_SetString(PyExc_AttributeError, "Unable to create sub_matrix Cmiss value.");
				Py_DECREF(return_code);
				return_code = (CmissValueMatrixObject *)NULL;
			}
		}
	}

	return((PyObject *)return_code);
}

static struct PyMethodDef CmissValueMatrix_methods[] =
	{
		{"get_matrix_cpointer", CmissValueMatrix_get_matrix_cpointer, METH_VARARGS},
		{"sub_matrix", (PyCFunction)CmissValueMatrix_sub_matrix, METH_VARARGS|METH_KEYWORDS},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissValueMatrix_new(PyObject* self, PyObject* args)
{
    CmissValueMatrixObject *cmiss_value;
	 int i,j,k,number_of_columns,number_of_values,number_of_rows;
	 Matrix_value *value,*values;
	 PyObject *float_item, *values_array, *values_array_item;
	 struct Matrix *matrix;

    if (!PyArg_ParseTuple(args,"Oi:new", &values_array, &number_of_columns)) 
        return NULL;

	 if (!PyList_Check(values_array))
	 {
		 PyErr_SetString(PyExc_AttributeError, "First argument must be a list");
		 return NULL;
	 }

    cmiss_value = PyObject_New(CmissValueMatrixObject, &CmissValueMatrixType);
	 if (cmiss_value->value = CREATE(Cmiss_value)())
	 {

		 ACCESS(Cmiss_value)(cmiss_value->value);
		 if (values_array&&(0<(number_of_values=PyList_Size(values_array))))
		 {
			 if (0==number_of_values%number_of_columns)
			 {
				 number_of_rows=number_of_values/number_of_columns;
				 if (values=(Matrix_value *)malloc(number_of_values*
						  sizeof(Matrix_value)))
				 {
					 /* swap column fastest to row fastest */
					 value=values;
					 for (j=0;j<number_of_columns;j++)
					 {
						 k=j;
						 for (i=number_of_rows;i>0;i--)
						 {
							 values_array_item = PyList_GetItem(values_array, k);
							 if (!(float_item = PyNumber_Float(values_array_item)))
							 {
								 PyErr_SetString(PyExc_AttributeError, "First argument must be a list containing only numeric values");
								 return NULL;
							 }
							 *value = (Matrix_value)PyFloat_AsDouble(float_item);
							 value++;
							 k += number_of_columns;
							 Py_DECREF(float_item);
						 }
					 }
					 matrix=CREATE(Matrix)("matrix",DENSE,number_of_rows,
						 number_of_columns);
					 if (!(matrix&&Matrix_set_values(matrix,values,1,number_of_rows,1,
								 number_of_columns)&&Cmiss_value_matrix_set_type(cmiss_value->value,matrix)))
					 {
						 if (matrix)
						 {
							 DESTROY(Matrix)(&matrix);
						 }
						 DEACCESS(Cmiss_value)(&cmiss_value->value);
					 }
					 free(values);
				 }
				 else
				 {
					 PyErr_SetString(PyExc_MemoryError, "Unable to allocate Matrix_value storage for CmissValueMatrix.");
					 DEACCESS(Cmiss_value)(&cmiss_value->value);
				 }
			 }
			 else
			 {
				 PyErr_SetString(PyExc_AttributeError, "The number of values in the array must be a multiple of the number_of_columns argument.");
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
		 else
		 {
			 matrix=CREATE(Matrix)("matrix",DENSE,0,0);
			 if (!(matrix&&Cmiss_value_matrix_set_type(cmiss_value->value,matrix)))
			 {
				 if (matrix)
				 {
					 DESTROY(Matrix)(&matrix);
				 }
				 DEACCESS(Cmiss_value)(&cmiss_value->value);
			 }
		 }
	 }

    return (PyObject*)cmiss_value;
}

static void
CmissValueMatrix_dealloc(PyObject* self)
{
	/* How do I check that this is really a CmissValueMatrix before the cast? */
    CmissValueMatrixObject *cmiss_value;
	 cmiss_value = (CmissValueMatrixObject *)self;
	 DEACCESS(Cmiss_value)(&cmiss_value->value);
    PyObject_Del(self);
}

static PyObject *
CmissValueMatrix_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissValueMatrix_methods, (PyObject *)self, name);
}

static PyObject*
CmissValueMatrix_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	if (_CmissValueMatrix_check(object))
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
CmissValueMatrix_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissValueMatrixObject *cmiss_value;
	PyObject *cmiss_value_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_value_cpointer)
		&& PyCObject_Check(cmiss_value_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_value = PyObject_New(CmissValueMatrixObject, &CmissValueMatrixType);
	if (!(cmiss_value->value = ACCESS(Cmiss_value)(
		(Cmiss_value_id)PyCObject_AsVoidPtr(cmiss_value_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Value pointer.");
		return NULL;			 
	}

	return (PyObject*)cmiss_value;
}

static PyObject *
CmissValueMatrix_str(PyObject* self)
{
	char *name;
	CmissValueMatrixObject *cmiss_value;
	int i, j, max_rows = 20, max_columns = 20, number_of_rows, number_of_columns;
	Matrix_value *values;
	struct Matrix *matrix;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissValueMatrix_check(self))
	{		
		cmiss_value = (CmissValueMatrixObject *)self;
		if (Cmiss_value_matrix_get_type(cmiss_value->value, &matrix)&&
			Matrix_get_dimensions(matrix, &number_of_rows, &number_of_columns))
		{
			if ((0<number_of_rows)&&(0<number_of_columns))
			{
				values=(Matrix_value *)malloc(number_of_rows*number_of_columns*
					sizeof(Matrix_value));
				if (values&&Matrix_get_values(matrix,values,1,number_of_rows,1,
						 number_of_columns))
				{
					string  = PyString_FromString("[");
					if (number_of_rows<max_rows)
					{
						for (i=0;i<number_of_rows;i++)
						{
							PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(values[i])));
							if (number_of_columns<max_columns)
							{
								for (j=1;j<number_of_columns;j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
										values[i+j*number_of_rows])));
								}
							}
							else
							{
								for (j=1;j<max_columns/2;j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
										values[i+j*number_of_rows])));
								}
								PyString_Concat(&string, PyString_FromString(",..."));
								for (j=number_of_columns-max_columns/2;j<number_of_columns;
									  j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
										values[i+j*number_of_rows])));
								}
							}
							if (i+1<number_of_rows)
							{
								PyString_Concat(&string, PyString_FromString("\n"));
							}
						}
					}
					else
					{
						for (i=0;i<max_rows/2;i++)
						{
							PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(values[i])));
							if (number_of_columns<max_columns)
							{
								for (j=1;j<number_of_columns;j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
																						  values[i+j*number_of_rows])));
								}
							}
							else
							{
								for (j=1;j<max_columns/2;j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
																						  values[i+j*number_of_rows])));
								}
								PyString_Concat(&string, PyString_FromString(",..."));
								for (j=number_of_columns-max_columns/2;j<number_of_columns;
									  j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
																						  values[i+j*number_of_rows])));
								}
							}
							PyString_Concat(&string, PyString_FromString("\n"));
						}
						PyString_Concat(&string, PyString_FromString("...\n"));
						for (i=number_of_rows-max_rows/2;i<number_of_rows;i++)
						{
							PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(values[i])));
							if (number_of_columns<max_columns)
							{
								for (j=1;j<number_of_columns;j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
																						  values[i+j*number_of_rows])));
								}
							}
							else
							{
								for (j=1;j<max_columns/2;j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
																						  values[i+j*number_of_rows])));
								}
								PyString_Concat(&string, PyString_FromString(",..."));
								for (j=number_of_columns-max_columns/2;j<number_of_columns;
									  j++)
								{
									PyString_Concat(&string, PyString_FromFormat(","));
									PyString_Concat(&string, PyObject_Str(PyFloat_FromDouble(
																						  values[i+j*number_of_rows])));
								}
							}
							if (i+1<number_of_rows)
							{
								PyString_Concat(&string, PyString_FromString("\n"));
							}
						}
					}
				}
				if (values)
				{
					free(values);
				}
			}
			else
			{
				Py_DECREF(string);
				string = (PyObject *)NULL;
			}
		}
		if (string)
		{
			PyString_Concat(&string, PyString_FromString("]"));
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Derivativematrix");
	}
	return (string);
}

static PyObject *
CmissValueMatrix_repr(PyObject* self)
{
	char *name;
	CmissValueMatrixObject *cmiss_value;
	int i, j, max_rows = 20, max_columns = 20, number_of_rows, number_of_columns;
	Matrix_value *values;
	struct Matrix *matrix;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissValueMatrix_check(self))
	{		
		string = PyString_FromString("Cmiss.Value.Matrix.new=");
		PyString_Concat(&string, CmissValueMatrix_str(self));
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Value.Derivativematrix");
	}
	return (string);
}

PyTypeObject CmissValueMatrixType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Matrix",
    sizeof(CmissValueMatrixObject),
    0,
    CmissValueMatrix_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissValueMatrix_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissValueMatrix_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
	 0,          /*tp_call */
	 CmissValueMatrix_str,         /* tp_str */
};

static PyMethodDef CmissValueMatrixType_methods[] = {
    {"new", CmissValueMatrix_new, METH_VARARGS,
     "Create a new Cmiss Value Matrix object."},
    {"check", CmissValueMatrix_check, METH_VARARGS,
     "Check if object is of type Cmiss Value Matrix object."},
    {"wrap", CmissValueMatrix_wrap, METH_VARARGS,
     "Wrap a C CmissValue in a python Cmiss Value Matrix object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initMatrix(void) 
{
	CmissValueMatrixType.ob_type = &PyType_Type;
	
	Py_InitModule("Matrix", CmissValueMatrixType_methods);
}

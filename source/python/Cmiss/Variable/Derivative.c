#include <Python.h>
#include "computed_variable/computed_variable.h"
#include "computed_variable/computed_variable_finite_element.h"

staticforward PyTypeObject CmissVariableDerivativeType;

/* Internal definition */
#define _CmissVariableDerivative_check(object)  ((object)->ob_type == &CmissVariableDerivativeType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_variable *variable;
} CmissVariableDerivativeObject;

/* Object Methods */

static PyObject*
CmissVariableDerivative_get_variable_cpointer(PyObject* self, PyObject* args)
{
	CmissVariableDerivativeObject *cmiss_variable;
	PyObject *object, *return_code;

	printf("CmissVariableDerivative_get_variable_cpointer\n");

	if (_CmissVariableDerivative_check(self))
	{
		cmiss_variable = (CmissVariableDerivativeObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_variable->variable, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static PyObject*
CmissVariableDerivative_evaluate(PyObject* self, PyObject* args)
{
	CmissVariableDerivativeObject *cmiss_variable;
	PyObject *generic_variable, *object, *return_code, *variable_module, *value_variable_list;

	printf("CmissVariableDerivative_evaluate\n");

	if (!PyArg_ParseTuple(args,"O:evaluate", &value_variable_list)) 
		return NULL;

	/* Make sure we load the variable module */
	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (_CmissVariableDerivative_check(self))
	{
		cmiss_variable = (CmissVariableDerivativeObject *)self;
		generic_variable = PyObject_CallMethod(variable_module, "wrap", "O",
			PyCObject_FromVoidPtr(cmiss_variable->variable, NULL));
		return_code = PyObject_CallMethod(generic_variable, "evaluate", "O",
			args, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static struct PyMethodDef CmissVariableDerivative_methods[] =
	{
		{"get_variable_cpointer", CmissVariableDerivative_get_variable_cpointer, 1},
		{"evaluate", CmissVariableDerivative_evaluate, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissVariableDerivative_new(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableDerivativeObject *cmiss_variable;
	Cmiss_variable_id dependent_variable_ptr, *independent_variable_ptrs;
	int i, order;
	PyObject *dependent_variable, *independent_variable, *independent_variables_array,
		*variable_cpointer, *variable_module;

	if (!PyArg_ParseTuple(args,"sOO:new", &name, &dependent_variable, &independent_variables_array)) 
		return NULL;

	if (!((variable_cpointer = PyObject_CallMethod(dependent_variable, "get_variable_cpointer", (char *)NULL)) &&
			 PyCObject_Check(variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from dependent_variable.");
		return NULL;			 
	}
	dependent_variable_ptr = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);

	if (!PyList_Check(independent_variables_array))
	{
		PyErr_SetString(PyExc_AttributeError, "Third argument must be a list");
		return NULL;
	}

	cmiss_variable = PyObject_New(CmissVariableDerivativeObject, &CmissVariableDerivativeType);
	if (cmiss_variable->variable = CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL, name))
	{
		ACCESS(Cmiss_variable)(cmiss_variable->variable);
		if (dependent_variable&&independent_variables_array&&
			(0<(order=PyList_Size(independent_variables_array))))
		{
			if (independent_variable_ptrs=(Cmiss_variable_id *)malloc(order*
					 sizeof(Cmiss_variable_id)))
			{
				i=0;
				while ((i<order)&&(variable_cpointer = PyObject_CallMethod(
					 PySequence_GetItem(independent_variables_array, i),
					 "get_variable_cpointer", (char *)NULL)) &&
					PyCObject_Check(variable_cpointer))
				{
					independent_variable_ptrs[i] = (Cmiss_variable_id)PyCObject_AsVoidPtr(variable_cpointer);
					i++;
				}
				if (i >= order)
				{
					if (!Cmiss_variable_derivative_set_type(cmiss_variable->variable,
								dependent_variable_ptr, order, independent_variable_ptrs))
					{
						free(independent_variable_ptrs);
						DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
						Py_DECREF(cmiss_variable);
						cmiss_variable = (CmissVariableDerivativeObject *)NULL;
					}
				}
				else
				{
					PyErr_SetString(PyExc_AttributeError, "Unable to extract variable pointer from at one of the values in the independent_variable array.");
					free(independent_variable_ptrs);
					DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
					Py_DECREF(cmiss_variable);
					cmiss_variable = (CmissVariableDerivativeObject *)NULL;
				}
			}
			else
			{
				DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
				Py_DECREF(cmiss_variable);
				cmiss_variable = (CmissVariableDerivativeObject *)NULL;
			}
		}
		else
		{
			DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
			Py_DECREF(cmiss_variable);
			cmiss_variable = (CmissVariableDerivativeObject *)NULL;
		}
	}
	else
	{
		if (name)
		{
			free(name);
		}
	}

	printf("Creating new CmissVariableDerivative\n");

	return (PyObject*)cmiss_variable;
}

static void
CmissVariableDerivative_dealloc(PyObject* self)
{
   CmissVariableDerivativeObject *cmiss_variable;
 	if (_CmissVariableDerivative_check(self))
	{
		cmiss_variable = (CmissVariableDerivativeObject *)self;
		DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
	}
	PyObject_Del(self);
}

static PyObject *
CmissVariableDerivative_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissVariableDerivative_methods, (PyObject *)self, name);
}

static PyObject*
CmissVariableDerivative_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissVariableDerivative\n");

	if (_CmissVariableDerivative_check(object))
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
CmissVariableDerivative_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableDerivativeObject *cmiss_variable;
	PyObject *cmiss_variable_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_variable_cpointer)
		&& PyCObject_Check(cmiss_variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_variable = PyObject_New(CmissVariableDerivativeObject, &CmissVariableDerivativeType);
	if (!(cmiss_variable->variable = ACCESS(Cmiss_variable)(
		(Cmiss_variable_id)PyCObject_AsVoidPtr(cmiss_variable_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.VariableDerivative pointer.");
		return NULL;			 
	}

	printf("Wrapping CmissVariableDerivative\n");

	return (PyObject*)cmiss_variable;
}

static PyObject *
CmissVariableDerivative_repr(PyObject* self)
{
	char *name;
	CmissVariableDerivativeObject *cmiss_variable;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissVariableDerivative_check(self))
	{		
		cmiss_variable = (CmissVariableDerivativeObject *)self;
		if (get_name_Cmiss_variable(cmiss_variable->variable, &name))
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

static PyTypeObject CmissVariableDerivativeType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Derivative",
    sizeof(CmissVariableDerivativeObject),
    0,
    CmissVariableDerivative_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissVariableDerivative_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissVariableDerivative_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissVariableDerivativeType_methods[] = {
    {"new", CmissVariableDerivative_new, METH_VARARGS,
     "Create a new Cmiss VariableDerivative object."},
    {"check", CmissVariableDerivative_check, METH_VARARGS,
     "Check if object is of type Cmiss VariableDerivative object."},
    {"wrap", CmissVariableDerivative_wrap, METH_VARARGS,
     "Wrap a C CmissVariableDerivative in a python Cmiss VariableDerivative object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initDerivative(void) 
{
	CmissVariableDerivativeType.ob_type = &PyType_Type;
	
	printf ("In initDerivative\n");

	Py_InitModule("Derivative", CmissVariableDerivativeType_methods);
}

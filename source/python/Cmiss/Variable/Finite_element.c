#include <Python.h>
#include "computed_variable/computed_variable.h"
#include "computed_variable/computed_variable_finite_element.h"

staticforward PyTypeObject CmissVariableFiniteelementType;

/* Internal definition */
#define _CmissVariableFiniteelement_check(object)  ((object)->ob_type == &CmissVariableFiniteelementType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_variable *variable;
} CmissVariableFiniteelementObject;

/* Object Methods */

static PyObject*
CmissVariableFiniteelement_get_variable_cpointer(PyObject* self, PyObject* args)
{
	CmissVariableFiniteelementObject *cmiss_variable;
	PyObject *object, *return_code;

	printf("CmissVariableFiniteelement_get_variable_cpointer\n");

	if (_CmissVariableFiniteelement_check(self))
	{
		cmiss_variable = (CmissVariableFiniteelementObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_variable->variable, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static PyObject*
CmissVariableFiniteelement_evaluate(PyObject* self, PyObject* args)
{
	CmissVariableFiniteelementObject *cmiss_variable;
	PyObject *generic_variable, *object, *return_code, *variable_module, *value_variable_list;

	printf("CmissVariableFiniteelement_evaluate\n");

	if (!PyArg_ParseTuple(args,"O:evaluate", &value_variable_list)) 
		return NULL;

	/* Make sure we load the variable module */
	if (!(variable_module = PyImport_ImportModule("Cmiss.Variable.Variable")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to import Cmiss.Variable.Variable module");
		return NULL;
	}

	if (_CmissVariableFiniteelement_check(self))
	{
		cmiss_variable = (CmissVariableFiniteelementObject *)self;
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

static struct PyMethodDef CmissVariableFiniteelement_methods[] =
	{
		{"get_variable_cpointer", CmissVariableFiniteelement_get_variable_cpointer, 1},
		{"evaluate", CmissVariableFiniteelement_evaluate, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissVariableFiniteelement_new(PyObject* self, PyObject* args)
{
	char *component_name, *name;
	CmissVariableFiniteelementObject *cmiss_variable;
	PyObject *fe_field, *fe_field_cpointer, *fe_field_module;
	int component_number;
	struct FE_field *fe_field_ptr;

	component_name = (char *)NULL;
	if (!PyArg_ParseTuple(args,"O|s:new", &fe_field, &component_name)) 
		return NULL;

	if (!(fe_field_module = PyImport_ImportModule("Cmiss.FE_field")))
	{
		PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.FE_field module");
		return NULL;
	}
	 
	if (!(PyObject_IsTrue(PyObject_CallMethod(fe_field_module, "check", "O", fe_field))))
	{
		PyErr_SetString(PyExc_AttributeError, "First argument must be a Cmiss.FE_field");
		return NULL;
	}

	if (!((fe_field_cpointer = PyObject_CallMethod(fe_field, "get_fe_field_cpointer", (char *)NULL)) &&
			 PyCObject_Check(fe_field_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract fe_field pointer from fe_field.");
		return NULL;			 
	}
	fe_field_ptr = (struct FE_field *)PyCObject_AsVoidPtr(fe_field_cpointer);

	name = (char *)NULL;
	cmiss_variable = PyObject_New(CmissVariableFiniteelementObject, &CmissVariableFiniteelementType);
	if (GET_NAME(FE_field)(fe_field_ptr,&name) && (cmiss_variable->variable = CREATE(Cmiss_variable)
		((struct Cmiss_variable_package *)NULL, name)))
	{
		ACCESS(Cmiss_variable)(cmiss_variable->variable);
		if (name)
		{
			free(name);
		}
		if (component_name)
		{
			component_number = get_FE_field_number_of_components(fe_field_ptr);
			if (0<component_number)
			{
				name=(char *)NULL;
				do
				{
					component_number--;
					if (name)
					{
						free(name);
					}
					name=get_FE_field_component_name(fe_field_ptr,component_number);
				} while ((component_number>0) && strcmp(name,component_name));
				if (!name||strcmp(name,component_name))
				{
					DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
				}
				if (name)
				{
					free(name);
				}
			}
		}
		else
		{
			component_number= -1;
		}
		if (cmiss_variable->variable)
		{
			if (!Cmiss_variable_finite_element_set_type(cmiss_variable->variable,fe_field_ptr,
				component_number))
			{
				DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
			}
		}
	}
	else
	{
		if (name)
		{
			free(name);
		}
	}

	printf("Creating new CmissVariableFiniteelement\n");

	return (PyObject*)cmiss_variable;
}

static void
CmissVariableFiniteelement_dealloc(PyObject* self)
{
   CmissVariableFiniteelementObject *cmiss_variable;
 	if (_CmissVariableFiniteelement_check(self))
	{
		cmiss_variable = (CmissVariableFiniteelementObject *)self;
		DEACCESS(Cmiss_variable)(&cmiss_variable->variable);
	}
	PyObject_Del(self);
}

static PyObject *
CmissVariableFiniteelement_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissVariableFiniteelement_methods, (PyObject *)self, name);
}

static PyObject*
CmissVariableFiniteelement_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissVariableFiniteelement\n");

	if (_CmissVariableFiniteelement_check(object))
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
CmissVariableFiniteelement_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissVariableFiniteelementObject *cmiss_variable;
	PyObject *cmiss_variable_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_variable_cpointer)
		&& PyCObject_Check(cmiss_variable_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_variable = PyObject_New(CmissVariableFiniteelementObject, &CmissVariableFiniteelementType);
	if (!(cmiss_variable->variable = ACCESS(Cmiss_variable)(
		(Cmiss_variable_id)PyCObject_AsVoidPtr(cmiss_variable_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.VariableFiniteelement pointer.");
		return NULL;			 
	}

	printf("Wrapping CmissVariableFiniteelement\n");

	return (PyObject*)cmiss_variable;
}

static PyObject *
CmissVariableFiniteelement_repr(PyObject* self)
{
	char *name;
	CmissVariableFiniteelementObject *cmiss_variable;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissVariableFiniteelement_check(self))
	{		
		cmiss_variable = (CmissVariableFiniteelementObject *)self;
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

static PyTypeObject CmissVariableFiniteelementType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Finite_element",
    sizeof(CmissVariableFiniteelementObject),
    0,
    CmissVariableFiniteelement_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissVariableFiniteelement_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissVariableFiniteelement_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissVariableFiniteelementType_methods[] = {
    {"new", CmissVariableFiniteelement_new, METH_VARARGS,
     "Create a new Cmiss VariableFiniteelement object."},
    {"check", CmissVariableFiniteelement_check, METH_VARARGS,
     "Check if object is of type Cmiss VariableFiniteelement object."},
    {"wrap", CmissVariableFiniteelement_wrap, METH_VARARGS,
     "Wrap a C CmissVariableFiniteelement in a python Cmiss VariableFiniteelement object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initFinite_element(void) 
{
	CmissVariableFiniteelementType.ob_type = &PyType_Type;
	
	printf ("In initFinite_element\n");

	Py_InitModule("Finite_element", CmissVariableFiniteelementType_methods);
}

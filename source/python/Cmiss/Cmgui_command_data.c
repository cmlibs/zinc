#include <Python.h>
#include "command/cmiss.h"

staticforward PyTypeObject CmissCmguicommanddataType;

/* Internal definition */
#define _CmissCmguicommanddata_check(object)  ((object)->ob_type == &CmissCmguicommanddataType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_command_data *command_data;
} CmissCmguicommanddataObject;

/* Object Methods */

static PyObject*
CmissCmguicommanddata_get_cmgui_command_data_cpointer(PyObject* self, PyObject* args)
{
	CmissCmguicommanddataObject *object;
	PyObject *return_code;
	struct Cmiss_command_data *command_data;

	if (_CmissCmguicommanddata_check(self))
	{
		object = (CmissCmguicommanddataObject *)self;
		return_code = PyCObject_FromVoidPtr(object->command_data, NULL);
	}
	else
	{
		return_code = NULL;
	}

	return(return_code);
}

static PyObject*
CmissCmguicommanddata_execute_command(PyObject* self, PyObject* args)
{
	char *command;
	CmissCmguicommanddataObject *cmgui_command_data;
	int error, quit;
	PyObject *cmgui_command_data_cpointer, *cmgui_command_data_module, *return_code;
	struct Cmiss_command_data *cmgui_command_data_ptr;

	if (!(PyArg_ParseTuple(args,"s:execute_command", &command)))
	{
		PyErr_SetString(PyExc_AttributeError, "String argument required execute_command method.");
		return NULL;			 
	}

	if (_CmissCmguicommanddata_check(self))
	{
		cmgui_command_data = (CmissCmguicommanddataObject *)self;
		error = 0;
		quit = 0;

		execute_command(command, (void *)cmgui_command_data->command_data, &quit, &error);
		return_code = PyInt_FromLong(!error);
	}
	else
	{
		return_code = PyInt_FromLong(0);
	}

	return (return_code);
}

static struct PyMethodDef CmissCmguicommanddata_methods[] =
	{
		{"get_cmgui_command_data_cpointer", CmissCmguicommanddata_get_cmgui_command_data_cpointer, 1},
		{"execute_command", CmissCmguicommanddata_execute_command, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissCmguicommanddata_new(PyObject* self, PyObject* args)
{
	char *argv[] = {"python"};
	int argc;
	CmissCmguicommanddataObject *cmgui_command_data;

	if (!PyArg_ParseTuple(args,":new", NULL)) 
		return NULL;

	argc = sizeof (argv) / sizeof (char *);
	cmgui_command_data = PyObject_New(CmissCmguicommanddataObject, &CmissCmguicommanddataType);
	if (!(cmgui_command_data->command_data = CREATE(Cmiss_command_data)(argc, argv, "python version")))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to create Cmgui_command_data.");
		return NULL;
	}

	return (PyObject*)cmgui_command_data;
}

static void
CmissCmguicommanddata_dealloc(PyObject* self)
{
   CmissCmguicommanddataObject *cmgui_command_data;
 	if (_CmissCmguicommanddata_check(self))
	{
		cmgui_command_data = (CmissCmguicommanddataObject *)self;
		DESTROY(Cmiss_command_data)(&cmgui_command_data->command_data);
	}
	PyObject_Del(self);
}

static PyObject *
CmissCmguicommanddata_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissCmguicommanddata_methods, (PyObject *)self, name);
}

static PyObject*
CmissCmguicommanddata_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	if (_CmissCmguicommanddata_check(object))
	{
		return_code = PyInt_FromLong(1);
	}
	else
	{
		return_code = PyInt_FromLong(0);
	}

	return(return_code);
}

static PyObject *
CmissCmguicommanddata_repr(PyObject* self)
{
	CmissCmguicommanddataObject *cmgui_command_data;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissCmguicommanddata_check(self))
	{		
		cmgui_command_data = (CmissCmguicommanddataObject *)self;
		string = PyString_FromString("=Cmiss.Cmgui_command_data.new()");
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not a Cmiss.Cmgui_command_data");
	}
	return (string);
}

static PyTypeObject CmissCmguicommanddataType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Cmgui_command_data",
    sizeof(CmissCmguicommanddataObject),
    0,
    CmissCmguicommanddata_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissCmguicommanddata_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissCmguicommanddata_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissCmguicommanddataType_methods[] = {
    {"new", CmissCmguicommanddata_new, METH_VARARGS,
     "Create a new Cmiss Cmguicommanddata object."},
    {"check", CmissCmguicommanddata_check, METH_VARARGS,
     "Check if object is of type Cmiss Cmguicommanddata object."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initCmgui_command_data(void) 
{
	CmissCmguicommanddataType.ob_type = &PyType_Type;
	
	Py_InitModule("Cmgui_command_data", CmissCmguicommanddataType_methods);
}

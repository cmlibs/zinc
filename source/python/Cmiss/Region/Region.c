#include <Python.h>
#include "command/cmiss.h"
#include "region/cmiss_region.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/import_finite_element.h"

staticforward PyTypeObject CmissRegionType;

/* Internal definition */
#define _CmissRegion_check(object)  ((object)->ob_type == &CmissRegionType)

typedef struct {
    PyObject_HEAD
    struct Cmiss_region *region;
} CmissRegionObject;

/* Object Methods */

static PyObject*
CmissRegion_get_region_cpointer(PyObject* self, PyObject* args)
{
	CmissRegionObject *cmiss_region;
	PyObject *object, *return_code;

	printf("CmissRegion_get_region_cpointer\n");

	if (_CmissRegion_check(self))
	{
		cmiss_region = (CmissRegionObject *)self;
		return_code = PyCObject_FromVoidPtr(cmiss_region->region, NULL);
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not type Cmiss.Region.");
		return_code = NULL;
	}

	return(return_code);
}

static PyObject*
CmissRegion_get_sub_region(PyObject* self, PyObject* args)
{
	char *region_path;
	CmissRegionObject *cmiss_region, *sub_cmiss_region;
	PyObject *object;

	if (!(PyArg_ParseTuple(args,"s:get_sub_region", &region_path)))
	{
		PyErr_SetString(PyExc_AttributeError, "String argument required execute_command method.");
		return NULL;			 
	}

	sub_cmiss_region = (CmissRegionObject *)NULL;
	if (_CmissRegion_check(self))
	{
		cmiss_region = (CmissRegionObject *)self;
		printf("CmissRegion_get_sub_region %p %s\n", cmiss_region->region, region_path);

		sub_cmiss_region = PyObject_New(CmissRegionObject, &CmissRegionType);
		if (Cmiss_region_get_region_from_path(cmiss_region->region, region_path,
			&sub_cmiss_region->region) && sub_cmiss_region->region)
		{
			ACCESS(Cmiss_region)(sub_cmiss_region->region);
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to find sub region");
			Py_DECREF(sub_cmiss_region);
			sub_cmiss_region = (CmissRegionObject *)NULL;
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not type Cmiss.Region.");
	}

	return((PyObject *)sub_cmiss_region);
}

static PyObject*
CmissRegion_get_field(PyObject* self, PyObject* args)
{
	char *name;
	CmissRegionObject *cmiss_region;
	PyObject *fe_field, *fe_field_module;
	struct FE_field *fe_field_ptr;
	struct FE_region *fe_region;

	if (!(fe_field_module = PyImport_ImportModule("Cmiss.FE_field")))
	{
		PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.FE_field module");
		return NULL;
	}
	 
	if (!(PyArg_ParseTuple(args,"s:get_field", &name)))
	{
		PyErr_SetString(PyExc_AttributeError, "String argument required for get_field method.");
		return NULL;			 
	}


	fe_field = (PyObject *)NULL;
	if (_CmissRegion_check(self))
	{
		cmiss_region = (CmissRegionObject *)self;
		printf("CmissRegion_get_field %p\n", cmiss_region->region);
		if (fe_region = Cmiss_region_get_FE_region(cmiss_region->region))
		{
			fe_field_ptr = FE_region_get_FE_field_from_name(fe_region, name);
			fe_field = PyObject_CallMethod(fe_field_module, "wrap", "O",
				PyCObject_FromVoidPtr(fe_field_ptr, NULL));
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to get fe_region.");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not type Cmiss.Region.");
	}

	return((PyObject *)fe_field);
}

static PyObject*
CmissRegion_get_element(PyObject* self, PyObject* args)
{
	CmissRegionObject *cmiss_region;
	int element_number;
	PyObject *fe_element, *fe_element_module;
	struct CM_element_information identifier;
	struct FE_element *fe_element_ptr;
	struct FE_region *fe_region;

	if (!(fe_element_module = PyImport_ImportModule("Cmiss.FE_element")))
	{
		PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.FE_element module");
		return NULL;
	}
	 
	if (!(PyArg_ParseTuple(args,"i:get_element", &element_number)))
	{
		PyErr_SetString(PyExc_AttributeError, "String argument required for get_element method.");
		return NULL;			 
	}


	fe_element = (PyObject *)NULL;
	if (_CmissRegion_check(self))
	{
		cmiss_region = (CmissRegionObject *)self;
		printf("CmissRegion_get_element %p\n", cmiss_region->region);
		if (fe_region = Cmiss_region_get_FE_region(cmiss_region->region))
		{
			identifier.type = CM_ELEMENT;
			identifier.number = element_number;
			fe_element_ptr = FE_region_get_FE_element_from_identifier(fe_region, &identifier);
			fe_element = PyObject_CallMethod(fe_element_module, "wrap", "O",
				PyCObject_FromVoidPtr(fe_element_ptr, NULL));
		}
		else
		{
			PyErr_SetString(PyExc_AttributeError, "Unable to get fe_region.");
		}
	}
	else
	{
		PyErr_SetString(PyExc_AttributeError, "self is not type Cmiss.Region.");
	}

	return((PyObject *)fe_element);
}

static struct PyMethodDef CmissRegion_methods[] =
	{
		{"get_region_cpointer", CmissRegion_get_region_cpointer, 1},
		{"get_sub_region", CmissRegion_get_sub_region, 1},
		{"get_field", CmissRegion_get_field, 1},
		{"get_element", CmissRegion_get_element, 1},
		{NULL, NULL, 0}
	};

/* Type Methods */

static PyObject*
CmissRegion_new(PyObject* self, PyObject* args)
{
	CmissRegionObject *cmiss_region;
	struct FE_region *fe_region;
	struct MANAGER(FE_basis) *basis_manager;

	if (!PyArg_ParseTuple(args,":new", NULL)) 
		return NULL;

	cmiss_region = PyObject_New(CmissRegionObject, &CmissRegionType);
	if (cmiss_region->region = CREATE(Cmiss_region)())
	{
		ACCESS(Cmiss_region)(cmiss_region->region);
		if (basis_manager=CREATE_MANAGER(FE_basis)())
		{
			if (fe_region=CREATE(FE_region)((struct FE_region *)NULL,basis_manager))
			{
				if (!Cmiss_region_attach_FE_region(cmiss_region->region, fe_region))
				{
					DEACCESS(Cmiss_region)(&cmiss_region->region);
				}
			}
			else
			{
				DEACCESS(Cmiss_region)(&cmiss_region->region);
			}
		}
		else
		{
			DEACCESS(Cmiss_region)(&cmiss_region->region);
		}
	}

	printf("Creating new CmissRegion\n");

	return (PyObject*)cmiss_region;
}

static void
CmissRegion_dealloc(PyObject* self)
{
   CmissRegionObject *cmiss_region;
 	if (_CmissRegion_check(self))
	{
		cmiss_region = (CmissRegionObject *)self;
		DEACCESS(Cmiss_region)(&cmiss_region->region);
	}
	PyObject_Del(self);
}

static PyObject *
CmissRegion_getattr(PyObject *self, char *name)
{
	return Py_FindMethod(CmissRegion_methods, (PyObject *)self, name);
}

static PyObject*
CmissRegion_check(PyObject* self, PyObject* args)
{
	PyObject *object, *return_code;

	if (!PyArg_ParseTuple(args,"O:check", &object)) 
		return NULL;

	printf("Checking CmissRegion\n");

	if (_CmissRegion_check(object))
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
CmissRegion_wrap(PyObject* self, PyObject* args)
{
	char *name;
	CmissRegionObject *cmiss_region;
	PyObject *cmiss_region_cpointer;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmiss_region_cpointer)
		&& PyCObject_Check(cmiss_region_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	cmiss_region = PyObject_New(CmissRegionObject, &CmissRegionType);
	if (!(cmiss_region->region = ACCESS(Cmiss_region)(
		(struct Cmiss_region *)PyCObject_AsVoidPtr(cmiss_region_cpointer))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Region pointer.");
		return NULL;			 
	}

	printf("Wrapping CmissRegion\n");

	return (PyObject*)cmiss_region;
}

static PyObject *
CmissRegion_repr(PyObject* self)
{
	char *name;
	CmissRegionObject *cmiss_region;
	PyObject *string;

	string = (PyObject *)NULL;
 	if (_CmissRegion_check(self))
	{		
		cmiss_region = (CmissRegionObject *)self;
		if (get_name_Cmiss_region(cmiss_region->region, &name))
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

static PyObject*
CmissRegion_command_data_get_root_region(PyObject* self, PyObject* args)
{
	char *name;
	CmissRegionObject *cmiss_region;
	PyObject *cmiss_region_cpointer, *cmgui_command_data_module, *cmgui_command_data,
		*cmgui_command_data_cpointer;
	struct Cmiss_command_data *cmgui_command_data_ptr;

	if (!(PyArg_ParseTuple(args,"O:wrap", &cmgui_command_data)))
	{
		PyErr_SetString(PyExc_AttributeError, "Incorrect argument for wrap function.");
		return NULL;			 
	}

	if (!(cmgui_command_data_module = PyImport_ImportModule("Cmiss.Cmgui_command_data")))
	{
		PyErr_SetString(PyExc_ImportError, "Unable to import Cmiss.cmgui_command_data module");
		return NULL;
	}

	if (!(PyObject_IsTrue(PyObject_CallMethod(cmgui_command_data_module, "check", "O", cmgui_command_data))))
	{
		PyErr_SetString(PyExc_AttributeError, "First argument must be a Cmiss.cmgui_command_data");
		return NULL;
	}

	if (!((cmgui_command_data_cpointer = PyObject_CallMethod(cmgui_command_data, "get_cmgui_command_data_cpointer", (char *)NULL)) &&
			 PyCObject_Check(cmgui_command_data_cpointer)))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract cmgui_command_data pointer from cmgui_command_data.");
		return NULL;			 
	}
	cmgui_command_data_ptr = (struct Cmiss_command_data *)PyCObject_AsVoidPtr(cmgui_command_data_cpointer);

	cmiss_region = PyObject_New(CmissRegionObject, &CmissRegionType);
	if (!(cmiss_region->region = ACCESS(Cmiss_region)(
		Cmiss_command_data_get_root_region(cmgui_command_data_ptr))))
	{
		PyErr_SetString(PyExc_AttributeError, "Unable to extract Cmiss.Region pointer.");
		return NULL;			 
	}

	printf("command_data_get_root_region CmissRegion\n");

	return (PyObject*)cmiss_region;
}


static PyTypeObject CmissRegionType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Region",
    sizeof(CmissRegionObject),
    0,
    CmissRegion_dealloc, /*tp_dealloc*/
    0,          /*tp_print*/
    CmissRegion_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    CmissRegion_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
};

static PyMethodDef CmissRegionType_methods[] = {
    {"new", CmissRegion_new, METH_VARARGS,
     "Create a new Cmiss Region object."},
    {"check", CmissRegion_check, METH_VARARGS,
     "Check if object is of type Cmiss Region object."},
    {"wrap", CmissRegion_wrap, METH_VARARGS,
     "Wrap a C CmissRegion in a python Cmiss Region object."},
    {"command_data_get_root_region", CmissRegion_command_data_get_root_region, METH_VARARGS,
     "Returns a Python CmissRegion object containing the root region from the cmgui_command_data."},
    {NULL, NULL, 0, NULL}
};

DL_EXPORT(void)
initRegion(void) 
{
	CmissRegionType.ob_type = &PyType_Type;
	
	printf ("In initRegion\n");

	Py_InitModule("Region", CmissRegionType_methods);
}

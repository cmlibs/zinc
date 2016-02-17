/**
 * glyph.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") glyph

%extend OpenCMISS::Zinc::Glyphmodulenotifier {
	int setCallback(PyObject *callbackObject)
	{
		PyObject *my_callback = NULL;
		if (!PyCallable_Check(callbackObject))
		{
			PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
			return 0;
		}
		Py_XINCREF(callbackObject);         /* Add a reference to new callback */     /* Remember new callback */
		return cmzn_glyphmodulenotifier_set_callback(($self)->getId(), glyphmoduleCallbackToPython, (void *)callbackObject);
	}

	int clearCallback()
	{
		void *user_data = cmzn_glyphmodulenotifier_get_callback_user_data(($self)->getId());
		PyObject *callbackObject = static_cast<PyObject *>(user_data);
		Py_XDECREF(callbackObject);         /* Decrease a reference count */
		return cmzn_glyphmodulenotifier_clear_callback(($self)->getId());
	}
}

%ignore OpenCMISS::Zinc::Glyphmodulenotifier::clearCallback();

%import "material.i"
%import "spectrum.i"

%extend OpenCMISS::Zinc::Glyph {
	bool operator==(const OpenCMISS::Zinc::Glyph& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/glyph.hpp"
#include "opencmiss/zinc/graphics.hpp"

static void glyphmoduleCallbackToPython(cmzn_glyphmoduleevent_id glyphmoduleevent, 
	void *user_data)
{
	PyObject *arglist = NULL;
	PyObject *result = NULL;
	PyObject *my_callback = (PyObject *)user_data;
	/* convert glyphmoduleevent to python object */
	PyObject *obj = NULL;
	OpenCMISS::Zinc::Glyphmoduleevent *glyphmoduleEvent = 
		new OpenCMISS::Zinc::Glyphmoduleevent(cmzn_glyphmoduleevent_access(glyphmoduleevent));
	obj = SWIG_NewPointerObj(SWIG_as_voidptr(glyphmoduleEvent), SWIGTYPE_p_OpenCMISS__Zinc__Glyphmoduleevent, 1);
	/* Time to call the callback */
	arglist = Py_BuildValue("(N)", obj);
	result = PyObject_CallObject(my_callback, arglist);
	Py_DECREF(arglist);
	if (result)
	{
		Py_DECREF(result);
	}
}
%}

%include "opencmiss/zinc/glyph.hpp"

/**
 * sceneviewer.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") sceneviewer
%include "typemaps.i"
%apply double *OUTPUT { double *eyex, double *eyey, double *eyez, double *lookatx, double *lookaty, double *lookatz, double *upx, double *upy, double *upz};
%apply double *OUTPUT { double *left, double *right, double *bottom, double *top, double *near_plane, double *far_plane};
%apply double *OUTPUT { double *x, double *y, double *z};
%apply int *OUTPUT {int *width, int *height};

%import "light.i"
%import "scene.i"

%extend OpenCMISS::Zinc::Sceneviewernotifier {

	int setCallback(PyObject *callbackObject)
	{
		PyObject *my_callback = NULL;
		if (!PyCallable_Check(callbackObject))
		{
			PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
			return 0;
		}
		Py_XINCREF(callbackObject);         /* Add a reference count to new callback */
		/* Remember new callback */
		return cmzn_sceneviewernotifier_set_callback(($self)->getId(),callbackToPython, (void *)callbackObject);
	}

	int clearCallback()
	{
		void *user_data = cmzn_sceneviewernotifier_get_callback_user_data(($self)->getId());
		PyObject *callbackObject =  static_cast<PyObject *>(user_data);
		Py_XDECREF(callbackObject);         /* Decrease a reference count */
		return cmzn_sceneviewernotifier_clear_callback(($self)->getId());
	}

}

%ignore OpenCMISS::Zinc::Sceneviewernotifier::clearCallback();

%{
#include "zinc/sceneviewer.hpp"
#include "zinc/sceneviewerinput.hpp"

static void callbackToPython(cmzn_sceneviewerevent_id sceneviewernotifier_event, void *user_data)
{
	PyObject *arglist = NULL;
	PyObject *result = NULL;
	PyObject *my_callback = (PyObject *)user_data;
	/* convert sceneviewernotifier to python object */
	/* Time to call the callback */
	OpenCMISS::Zinc::Sceneviewerevent *sceneviewerevent = new OpenCMISS::Zinc::Sceneviewerevent(
	cmzn_sceneviewerevent_access(sceneviewernotifier_event));
	PyObject *obj = SWIG_NewPointerObj(SWIG_as_voidptr(sceneviewerevent), SWIGTYPE_p_OpenCMISS__Zinc__Sceneviewerevent, 1);
	arglist = Py_BuildValue("(N)", obj);
	result = PyObject_CallObject(my_callback, arglist);
	Py_DECREF(arglist);
	if (result)
		Py_DECREF(result);
}
%}

%include "zinc/sceneviewer.hpp"


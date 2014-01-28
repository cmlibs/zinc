/**
 * timenotifier.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") timenotifier

%extend OpenCMISS::Zinc::Timenotifier {

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
        return cmzn_timenotifier_set_callback(($self)->getId(),callbackToPython, (void *)callbackObject);
    }

    int clearCallback()
    {
	    void *user_data = cmzn_timenotifier_get_callback_user_data(($self)->getId());
	    PyObject *callbackObject =  static_cast<PyObject *>(user_data);
	    Py_XDECREF(callbackObject);         /* Decrease a reference count */
        return cmzn_timenotifier_clear_callback(($self)->getId());
    }
}

%ignore OpenCMISS::Zinc::Timenotifier::clearCallback();

%import "timekeeper.i"

%{
#include "zinc/timenotifier.hpp"

static void callbackToPython(cmzn_timenotifierevent_id timenotifier_event, void *user_data)
{
    PyObject *arglist = NULL;
    PyObject *result = NULL;
    PyObject *my_callback = (PyObject *)user_data;
    /* convert timenotifier to python object */
    /* Time to call the callback */
    OpenCMISS::Zinc::Timenotifierevent *timenotifierevent = new OpenCMISS::Zinc::Timenotifierevent(cmzn_timenotifierevent_access(timenotifier_event));
    PyObject *obj = SWIG_NewPointerObj(SWIG_as_voidptr(timenotifierevent), SWIGTYPE_p_OpenCMISS__Zinc__Timenotifierevent, 1);
    arglist = Py_BuildValue("(N)", obj);
    result = PyObject_CallObject(my_callback, arglist);
    Py_DECREF(arglist);
    if (result)
        Py_DECREF(result);
}
%}

%include "zinc/timenotifier.hpp"


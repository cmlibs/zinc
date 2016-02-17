/**
 * spectrum.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") spectrum

%extend OpenCMISS::Zinc::Spectrummodulenotifier {

    int setCallback(PyObject *callbackObject)
    {
        PyObject *my_callback = NULL;
        if (!PyCallable_Check(callbackObject))
        {
            PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
            return 0;
        }
        Py_XINCREF(callbackObject);         /* Add a reference to new callback */     /* Remember new callback */
        return cmzn_spectrummodulenotifier_set_callback(($self)->getId(), spectrummoduleCallbackToPython, (void *)callbackObject);
    }

    int clearCallback()
    {
      	void *user_data = cmzn_spectrummodulenotifier_get_callback_user_data(($self)->getId());
	    PyObject *callbackObject = static_cast<PyObject *>(user_data);
	    Py_XDECREF(callbackObject);         /* Decrease a reference count */
        return cmzn_spectrummodulenotifier_clear_callback(($self)->getId());
    }
}

%ignore OpenCMISS::Zinc::Spectrummodulenotifier::clearCallback();

%include "pyzincstringhandling.i"

%extend OpenCMISS::Zinc::Spectrum {
	bool operator==(const OpenCMISS::Zinc::Spectrum& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/spectrum.hpp"

static void spectrummoduleCallbackToPython(cmzn_spectrummoduleevent_id spectrummoduleevent, void *user_data)
{
    PyObject *arglist = NULL;
    PyObject *result = NULL;
    PyObject *my_callback = (PyObject *)user_data;
    /* convert spectrummoduleevent to python object */
    PyObject *obj = NULL;
    OpenCMISS::Zinc::Spectrummoduleevent *spectrummoduleEvent = new OpenCMISS::Zinc::Spectrummoduleevent(cmzn_spectrummoduleevent_access(spectrummoduleevent));
    obj = SWIG_NewPointerObj(SWIG_as_voidptr(spectrummoduleEvent), SWIGTYPE_p_OpenCMISS__Zinc__Spectrummoduleevent, 1);
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

%include "opencmiss/zinc/spectrum.hpp"

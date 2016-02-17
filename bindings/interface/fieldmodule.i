/**
 * fieldmodule.i
 *
 * Swig interface file for wrapping api functions in api/fieldmodule.hpp
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") fieldmodule

%extend OpenCMISS::Zinc::Fieldmodulenotifier {

    int setCallback(PyObject *callbackObject)
    {
        PyObject *my_callback = NULL;
        if (!PyCallable_Check(callbackObject))
        {
            PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
            return 0;
        }
        Py_XINCREF(callbackObject);         /* Add a reference to new callback */     /* Remember new callback */
        return cmzn_fieldmodulenotifier_set_callback(($self)->getId(), fieldmoduleCallbackToPython, (void *)callbackObject);
    }

    int clearCallback()
    {
      	void *user_data = cmzn_fieldmodulenotifier_get_callback_user_data(($self)->getId());
	    PyObject *callbackObject = static_cast<PyObject *>(user_data);
	    Py_XDECREF(callbackObject);         /* Decrease a reference count */
        return cmzn_fieldmodulenotifier_clear_callback(($self)->getId());
    }
}

%ignore OpenCMISS::Zinc::Fieldmodulenotifier::clearCallback();
// ignore following as array overload works the same
%ignore OpenCMISS::Zinc::Fieldmodule::createFieldComponent(const Field& sourceField, int sourceComponentIndex);

%include "doublevaluesarraytypemap.i"
%include "integervaluesarraytypemap.i"
%include "fieldarraytypemap.i"
%include "pyzincstringhandling.i"

%import "scenecoordinatesystem.i"
%import "timesequence.i"
%import "optimisation.i"
%import "field.i"
%import "fieldcache.i"
%import "fieldsmoothing.i"

%{
#include "opencmiss/zinc/fieldalias.hpp"
#include "opencmiss/zinc/fieldarithmeticoperators.hpp"
#include "opencmiss/zinc/fieldcache.hpp"
#include "opencmiss/zinc/fieldcomposite.hpp"
#include "opencmiss/zinc/fieldconditional.hpp"
#include "opencmiss/zinc/fieldconstant.hpp"
#include "opencmiss/zinc/fieldcoordinatetransformation.hpp"
#include "opencmiss/zinc/fieldderivatives.hpp"
#include "opencmiss/zinc/fieldfibres.hpp"
#include "opencmiss/zinc/fieldfiniteelement.hpp"
#include "opencmiss/zinc/fieldsubobjectgroup.hpp"
#include "opencmiss/zinc/fieldgroup.hpp"
#include "opencmiss/zinc/fieldimage.hpp"
#include "opencmiss/zinc/fieldimageprocessing.hpp"
#include "opencmiss/zinc/fieldlogicaloperators.hpp"
#include "opencmiss/zinc/fieldmatrixoperators.hpp"
#include "opencmiss/zinc/fieldmeshoperators.hpp"
#include "opencmiss/zinc/fieldnodesetoperators.hpp"
#include "opencmiss/zinc/fieldsceneviewerprojection.hpp"
#include "opencmiss/zinc/fieldtime.hpp"
#include "opencmiss/zinc/fieldtrigonometry.hpp"
#include "opencmiss/zinc/fieldvectoroperators.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"
#include "opencmiss/zinc/fieldsmoothing.hpp"
#include "opencmiss/zinc/optimisation.hpp"

static void fieldmoduleCallbackToPython(cmzn_fieldmoduleevent_id fieldmoduleevent, void *user_data)
{
    PyObject *arglist = NULL;
    PyObject *result = NULL;
    PyObject *my_callback = (PyObject *)user_data;
    /* convert fieldmoduleevent to python object */
    PyObject *obj = NULL;
    OpenCMISS::Zinc::Fieldmoduleevent *fieldmoduleEvent = new OpenCMISS::Zinc::Fieldmoduleevent(cmzn_fieldmoduleevent_access(fieldmoduleevent));
    obj = SWIG_NewPointerObj(SWIG_as_voidptr(fieldmoduleEvent), SWIGTYPE_p_OpenCMISS__Zinc__Fieldmoduleevent, 1);
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

%include "opencmiss/zinc/fieldmodule.hpp"


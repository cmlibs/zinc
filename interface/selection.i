/*******************************************************************************
 * CmissSelection.i
 *
 * Swig interface file for cmgui selection API.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

%module(package="opencmiss.zinc") selection

%extend OpenCMISS::Zinc::SelectionHandler {

    int setCallback(PyObject *callbackObject)
    {
        PyObject *my_callback = NULL;
        if (!PyCallable_Check(callbackObject))
        {
            PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
            return 0;
        }
        Py_XINCREF(callbackObject);         /* Add a reference to new callback */
        my_callback = callbackObject;       /* Remember new callback */
        return cmzn_selection_handler_set_callback(($self)->getId(), selectionCallbackToPython, (void *)my_callback);
    }

    int clearCallback()
    {
        // Py_XDECREF(callbackObject);
        return cmzn_selection_handler_clear_callback(($self)->getId());
    }
}

%{
#include "zinc/selection.hpp"

static void selectionCallbackToPython(cmzn_selection_event_id selection_event,	void *user_data)
{
    PyObject *arglist = NULL;
    PyObject *result = NULL;
    PyObject *my_callback = (PyObject *)user_data;
    /* convert time_notifier to python object */
    PyObject *obj = NULL;
    OpenCMISS::Zinc::SelectionEvent *selectionEvent = new OpenCMISS::Zinc::SelectionEvent(cmzn_selection_event_access(selection_event));
    obj = SWIG_NewPointerObj(SWIG_as_voidptr(selectionEvent), SWIGTYPE_p_OpenCMISS__Zinc__SelectionEvent, 1);
    /* Time to call the callback */
    arglist = Py_BuildValue("(N)", obj);
    result = PyObject_CallObject(my_callback, arglist);
    Py_DECREF(arglist);
    if (result)
        Py_DECREF(result);
}
%}

%include "zinc/selection.hpp"

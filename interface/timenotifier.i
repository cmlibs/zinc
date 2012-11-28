/*******************************************************************************
 * Time.i
 *
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1mesh_name
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

%module(package="zinc") timenotifier

%extend zinc::TimeNotifier {

    int addCallback(PyObject *callbackObject)
    {
        PyObject *my_callback = NULL;
        if (!PyCallable_Check(callbackObject))
        {
            PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
            return 0;
        }
        Py_XINCREF(callbackObject);         /* Add a reference to new callback */
        my_callback = callbackObject;       /* Remember new callback */
        return Cmiss_time_notifier_add_callback(($self)->getId(),callbackToPython, (void *)my_callback);
    }

    int removeCallback(PyObject *callbackObject)
    {
        if (!PyCallable_Check(callbackObject))
        {
            PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
            return 0;
        }

        Py_XDECREF(callbackObject);         /* Add a reference to new callback */
        return Cmiss_time_notifier_remove_callback(($self)->getId(),callbackToPython,
            (void *)callbackObject);
    }
}

%{
#include "zinc/timenotifier.hpp"

struct TimeNotifierPyDataObject
{
    PyObject *callbackObject;
    PyObject *userObject;
};

static int callbackToPython(Cmiss_time_notifier_id time_notifier,
    double current_time, void *user_data)
{
    PyObject *arglist = NULL;
    PyObject *result = NULL;
    PyObject *my_callback = (PyObject *)user_data;
    /* convert time_notifier to python object */
    PyObject *obj = NULL;
    zinc::TimeNotifier *timeNotifier = new zinc::TimeNotifier(Cmiss_time_notifier_access(time_notifier));
    obj = SWIG_NewPointerObj(SWIG_as_voidptr(timeNotifier), SWIGTYPE_p_zinc__TimeNotifier, 1);
    /* Time to call the callback */
    arglist = Py_BuildValue("(Nd)", obj, current_time);
    result = PyObject_CallObject(my_callback, arglist);
    Py_DECREF(arglist);
    if (result)
        Py_DECREF(result);
    return 1;
}
%}

%include "zinc/timenotifier.hpp"


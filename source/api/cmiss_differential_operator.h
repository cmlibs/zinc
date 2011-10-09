/***************************************************************************//**
 * FILE : cmiss_differential_operator.h
 * 
 * Public interface to differential operator objects used to specify which
 * field derivative to evaluate.
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef __CMISS_DIFFERENTIAL_OPERATOR_H__
#define __CMISS_DIFFERENTIAL_OPERATOR_H__

#include "api/types/cmiss_differential_operator_id.h"

/***************************************************************************//**
 * Returns a new reference to the differential operator with reference count
 * incremented. Caller is responsible for destroying the new reference.
 * 
 * @param differential_operator  The differential operator to obtain a new
 * reference to.
 * @return  New differential operator reference with incremented reference
 * count.
 */
Cmiss_differential_operator_id Cmiss_differential_operator_access(
	Cmiss_differential_operator_id differential_operator);

/***************************************************************************//**
 * Destroys reference to the differential operator and sets pointer/handle to
 * NULL. Internally this just decrements the reference count.
 *
 * @param differential_operator_address  Address of differential operator
 * reference.
 * @return  1 on success, 0 if invalid arguments.
 */
int Cmiss_differential_operator_destroy(
	Cmiss_differential_operator_id *differential_operator_address);

#endif /* __CMISS_DIFFERENTIAL_OPERATOR_H__ */

/*******************************************************************************
FILE : api/cmiss_idle.h

LAST MODIFIED : 21 March, 2005

DESCRIPTION :
The public interface to idle callbacks
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#ifndef __API_CMISS_IDLE_H__
#define __API_CMISS_IDLE_H__

#include "general/object.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Cmiss_idle_package * Cmiss_idle_package_id;
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
An identifier for an idle package object.
==============================================================================*/

typedef struct Cmiss_idle_callback * Cmiss_idle_callback_id;
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
An identifier for an idle callback object.
==============================================================================*/

typedef int Cmiss_idle_callback_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
The type used for idle callback data.
==============================================================================*/

Cmiss_idle_callback_id Cmiss_idle_package_add_callback(Cmiss_idle_package_id pkg,
 Cmiss_idle_callback_function *callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Sets an idle callback.
==============================================================================*/

int DESTROY(Cmiss_idle_callback)(Cmiss_idle_package_id pkg,
	Cmiss_idle_callback_id *callback);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys an idle callback, causing it to not be called any longer.
==============================================================================*/

int DESTROY(Cmiss_idle_package)(Cmiss_idle_package_id *pkg);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys the idle package object.
==============================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* __API_CMISS_IDLE_H__ */

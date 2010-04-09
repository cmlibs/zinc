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

#ifndef __CMISS_GRAPHICS_MODULE_H__
#define __CMISS_GRAPHICS_MODULE_H__

#include "api/cmiss_material.h"

struct Cmiss_graphics_module;
#ifndef CMISS_GRAPHICS_MODULE_ID_DEFINED
typedef struct Cmiss_graphics_module * Cmiss_graphics_module_id;
#define CMISS_GRAPHICS_MODULE_ID_DEFINED
#endif /* CMISS_GRAPHICS_MODULE_ID_DEFINED */

/***************************************************************************//**
 * Find the material with the supplied name in graphics module, if any.
 *
 * @param graphics_module  The handle to the graphics module to find the
 * material in.
 * @param name  The name of the material.
 * @return  Handle to the material with that name, or NULL if not found.
 */
Cmiss_material_id Cmiss_graphics_module_find_material_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name);

/***************************************************************************//**
 * Create and return a handle to a new graphics material.
 *
 * @param graphics_module  The handle to the graphics module the material will
 * belong to.
 * @return  Handle to the newly created material if successful, otherwise NULL.
 */
Cmiss_material_id Cmiss_graphics_module_create_material(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Return an additional handle to the graphics module. Increments the
 * internal 'access count' of the module.
 *
 * @param graphics_module  Existing handle to the graphics module.
 * @return  Additional handle to graphics module.
 */
Cmiss_graphics_module_id Cmiss_graphics_module_access(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Destroy this handle to the graphics module. The graphics module itself will
 * only be destroyed when all handles to it are destroyed.
 *
 * @param graphics_module_address  Address of the graphics module handle to be
 * destroyed. 
 * @return  1 if handle is destroyed, otherwise 0.
 */
int Cmiss_graphics_module_destroy(
	Cmiss_graphics_module_id *graphics_module_address);

#endif /*__CMISS_GRAPHICS_MODULE_H__*/

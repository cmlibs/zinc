/*******************************************************************************
FILE : cmiss_graphics_package.h

LAST MODIFIED : 05 Nov 2009

DESCRIPTION :
The public interface to the cmiss_graphics_package.
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

#ifndef __CMISS_GRAPHICS_PACKAGE_H__
#define __CMISS_GRAPHICS_PACKAGE_H__

#include "api/cmiss_material.h"

struct Cmiss_graphics_package;
#ifndef CMISS_GRAPHICS_PACKAGE_ID_DEFINED
typedef struct Cmiss_graphics_package * Cmiss_graphics_package_id;
#define CMISS_GRAPHICS_PACKAGE_ID_DEFINED
#endif /* CMISS_GRAPHICS_PACKAGE_ID_DEFINED */

/***************************************************************************//**
 * Get the material with <name> from graphics_package if graphics_package has
 * an existing material with the name provided.
 *
 * @param graphics_package  The handle to cmiss grpahics package, it contains
 *   internal cmgui structure required by cmiss_graphical_material.
 * @param name  The name of the material.
 * @return  1 if successfully get material, otherwise 0.
 */
Cmiss_graphics_material_id Cmiss_graphics_package_find_material_by_name(
	Cmiss_graphics_package_id graphics_package, const char *name);

#endif /*__CMISS_GRAPHICS_PACKAGE_H__*/

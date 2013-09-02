/***************************************************************************//**
 * FILE : cmiss_graphics_material_id.h
 *
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
 * Portions created by the Initial Developer are Copyright (C) 2010-2011
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

#ifndef CMZN_GRAPHICSMATERIALID_H__
#define CMZN_GRAPHICSMATERIALID_H__

/***************************************************************************//**
 * A handle to cmiss material. cmiss material describes the
 * colour, shading and other graphical properties of a material, it is highly
 * similar to material described by OpenGL.
 * User can get a handle to material either through create new material using
 * cmzn_graphics_mateial_module_create_material or use existing materials in the
 * material_module provided by the context with
 * cmzn_graphics_material_module_find_material_by_name.
 * LibZinc also provide a number of preset materials in the default
 * graphics_packge.
 * Preset graphical materials are:
 * black, blue, bone, gray50, gold, green, muscle, red, silver, tissue,
 * transparent_gray50 and white.
 *
 * Please see available cmzn_graphics_material API functions belong for
 * configurable properties.
 */
	struct cmzn_graphics_material;
	typedef struct cmzn_graphics_material * cmzn_graphics_material_id;

	struct cmzn_graphics_material_module;
	typedef struct cmzn_graphics_material_module * cmzn_graphics_material_module_id;

#endif

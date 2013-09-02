/***************************************************************************//**
 * FILE : fontid.h
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
 * Portions created by the Initial Developer are Copyright (C) 2010-2012
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

#ifndef CMZN_FONTID_H__
#define CMZN_FONTID_H__

#include "zinc/zincsharedobject.h"

enum cmzn_font_type
{
	CMZN_FONT_TYPE_INVALID = 0,
	CMZN_FONT_TYPE_OpenSans = 1,
};

enum cmzn_font_render_type
{
	CMZN_FONT_RENDER_TYPE_INVALID = 0,
	CMZN_FONT_RENDER_TYPE_BITMAP = 1,
	CMZN_FONT_RENDER_TYPE_PIXMAP = 2,
	CMZN_FONT_RENDER_TYPE_POLYGON = 3,
	CMZN_FONT_RENDER_TYPE_OUTLINE = 4,
	CMZN_FONT_RENDER_TYPE_EXTRUDE = 5,
};

struct cmzn_font;
typedef struct cmzn_font *cmzn_font_id;

struct cmzn_font_module;
typedef struct cmzn_font_module *cmzn_font_module_id;

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_font_type
	cmzn_font_type_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param system  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_font_type_enum_to_string(
	enum cmzn_font_type font_type);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_font_render_type
	cmzn_font_render_type_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param system  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_font_render_type_enum_to_string(
	enum cmzn_font_render_type render_type);

#ifdef __cplusplus
}
#endif

#endif

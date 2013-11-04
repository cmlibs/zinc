/**
 * FILE : fontid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FONTID_H__
#define CMZN_FONTID_H__

#include "zinc/zincsharedobject.h"

enum cmzn_font_typeface_type
{
	CMZN_FONT_TYPEFACE_TYPE_INVALID = 0,
	CMZN_FONT_TYPEFACE_TYPE_OPENSANS = 1
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

struct cmzn_fontmodule;
typedef struct cmzn_fontmodule *cmzn_fontmodule_id;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_font_typeface_type
	cmzn_font_typeface_type_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param system  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_font_typeface_type_enum_to_string(
	enum cmzn_font_typeface_type typeface_type);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_font_render_type
	cmzn_font_render_type_enum_from_string(const char *string);

/**
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

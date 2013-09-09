/*******************************************************************************
FILE : import_graphics_object.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
Function prototype for reading graphics object data from a file.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (IMPORT_GRAPHICS_OBJECT_H)
#define IMPORT_GRAPHICS_OBJECT_H

struct IO_stream_package;
struct cmzn_graphics_material_module;
struct cmzn_glyph_module;

/*
Global functions
----------------
*/

int file_read_graphics_objects(char *file_name,
	struct IO_stream_package *io_stream_package,
	struct cmzn_graphics_material_module *material_module,
	struct cmzn_glyph_module *glyph_module);

int file_read_voltex_graphics_object_from_obj(char *file_name,
	struct IO_stream_package *io_stream_package,
	char *graphics_object_name, enum cmzn_graphic_render_polygon_mode render_polygon_mode,
	ZnReal time, struct cmzn_graphics_material_module *material_module,
	struct cmzn_glyph_module *glyph_module);

#endif

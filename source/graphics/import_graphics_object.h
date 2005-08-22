/*******************************************************************************
FILE : import_graphics_object.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
Function prototype for reading graphics object data from a file.
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
#if !defined (IMPORT_GRAPHICS_OBJECT_H)
#define IMPORT_GRAPHICS_OBJECT_H

#include "graphics/graphics_object.h"
#include "graphics/material.h"

/*
Global functions
----------------
*/
int file_read_graphics_objects(char *file_name,
	struct IO_stream_package *io_stream_package,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *object_list);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
==============================================================================*/

int file_read_voltex_graphics_object_from_obj(char *file_name,
	struct IO_stream_package *io_stream_package,
	char *graphics_object_name, enum Render_type render_type,
	float time, struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *object_list);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
==============================================================================*/
#endif

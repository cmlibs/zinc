/*******************************************************************************
FILE : graphics_font.h

LAST MODIFIED : 17 November 2005

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
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
#if !defined (GRAPHICS_FONT_H)
#define GRAPHICS_FONT_H

#include "zinc/types/graphicsfontid.h"
#include "general/callback.h"
#include "general/manager.h"
#include "general/enumerator.h"

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphics_font_type);
PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphics_font_true_type);

/*
Global types
------------
*/

struct Cmiss_graphics_font;

DECLARE_LIST_TYPES(Cmiss_graphics_font);

DECLARE_MANAGER_TYPES(Cmiss_graphics_font);

struct Cmiss_graphics_module;
#include "general/manager_private.h"
PROTOTYPE_MANAGER_GET_OWNER_FUNCTION(Cmiss_graphics_font, struct Cmiss_graphics_module);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_graphics_font);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_graphics_font);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_graphics_font);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_graphics_font,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Cmiss_graphics_font,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_graphics_font);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_graphics_font,name,const char *);

struct Cmiss_graphics_font *CREATE(Cmiss_graphics_font)(const char *name);

int DESTROY(Cmiss_graphics_font)(struct Cmiss_graphics_font **font_address);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/


int Cmiss_graphics_font_compile(struct Cmiss_graphics_font *font);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
Compiles the specified <font> so it can be used by the graphics.  The
<buffer> is required as the Win32 API requires a window context.
==============================================================================*/

int Cmiss_graphics_font_rendergl_text(struct Cmiss_graphics_font *font, char *text,
	float x, float y, float z);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/

int Cmiss_graphics_font_manager_set_owner(struct MANAGER(Cmiss_graphics_font) *manager,
	struct Cmiss_graphics_module *graphics_module);
#endif /* !defined (GRAPHICS_FONT_H) */


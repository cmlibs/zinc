/*******************************************************************************
FILE : material_editor_dialog_wx.cpp

LAST MODIFIED : 5 Nov 2007

DESCRIPTION :
This module creates a free material_editor_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
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

#if !defined (MATERIAL_EDITOR_DIALOG_WX_H)
#define MATERIAL_EDITOR_DIALOG_WX_H

#include "graphics/material.h"
#include "user_interface/user_interface.h"

struct Material_editor_dialog;

int DESTROY(Material_editor_dialog)(
	struct Material_editor_dialog **material_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_dialog_address> and sets
<*material_editor_dialog_address> to NULL.
==============================================================================*/

int bring_up_material_editor_dialog_wx(
	struct Material_editor_dialog **material_editor_dialog_address,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *material,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
If there is a material_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/

int DESTROY(Material_editor_dialog)(
	 struct Material_editor_dialog **material_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_dialog_address> and sets
<*material_editor_dialog_address> to NULL.
==============================================================================*/

#endif

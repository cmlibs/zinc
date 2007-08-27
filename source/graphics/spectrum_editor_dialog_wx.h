/*******************************************************************************
FILE : spectrum_editor_dialog.h

LAST MODIFIED : 23 Aug 2007

DESCRIPTION :
Header description for spectrum_editor_dialog widget.
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
#if !defined (SPECTRUM_EDITOR_DIALOG_H)
#define SPECTRUM_EDITOR_DIALOG_H

#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

struct Spectrum_editor_dialog;

/*
Global Functions
----------------
*/

int bring_up_spectrum_editor_dialog(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum, 
	struct Graphics_font *font,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct User_interface *user_interface, struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then de-iconify it and
bring it to the front, otherwise create a new one.
==============================================================================*/

int DESTROY(Spectrum_editor_dialog)(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroy the <*spectrum_editor_dialog_address> and sets
<*spectrum_editor_dialog_address> to NULL.
==============================================================================*/

struct Spectrum *spectrum_editor_dialog_get_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the spectrum edited by the <spectrum_editor_dialog>.
==============================================================================*/

int spectrum_editor_dialog_set_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> for the <spectrum_editor_dialog>.
==============================================================================*/

#endif

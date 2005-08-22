/*******************************************************************************
FILE : input_module_dialog.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
This is the control center for the input_module.  Things like setting
resolution, refresh rate and origins may be done from this widget.
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
#if !defined (INPUT_MODULE_DIALOG_H)
#define INPUT_MODULE_DIALOG_H

#include "io_devices/input_module.h"

#ifdef EXT_INPUT
/*
Global functions
----------------
*/
int bring_up_input_module_dialog(Widget *input_module_dialog_address,
	Widget parent,struct Graphical_material *material,
	struct MANAGER(Graphical_material) *material_manager,struct Scene *scene,
	struct MANAGER(Scene) *scene_manager, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
If there is a input_module dialog in existence, then bring it to the front, else
create a new one.  Assumes we will only ever want one input_module controller at
a time.  This implementation may be changed later.
==============================================================================*/
#endif /* EXT_INPUT */

#endif /* INPUT_MODULE_DIALOG_H */

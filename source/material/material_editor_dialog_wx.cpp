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
extern "C"
{
#include <stdio.h>
#include "three_d_drawing/graphics_buffer.h"
#include "general/debug.h"
#include "graphics/material.h"
#include "material/material_editor_wx.h"
#include "material/material_editor_dialog_wx.h"
#include "user_interface/message.h"
}

/*
Global Types
------------
*/
struct Material_editor_dialog
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Contains all the information carried by the material_editor_dialog widget.
Note that we just hold a pointer to the material_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct Graphical_material *current_value;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Material_editor *material_editor;
	struct Material_editor_dialog **material_editor_dialog_address;
	struct User_interface *user_interface;
}; /* material_editor_dialog_struct */

int material_editor_dialog_set_material(
	struct Material_editor_dialog *material_editor_dialog,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 6 November 2007

DESCRIPTION :
Set the <material> for the <material_editor_dialog>.
==============================================================================*/
{
	int return_code;

	ENTER(material_editor_dialog_set_material);
	return_code=0;
	/* check arguments */
	if (material_editor_dialog)
	{
		return_code=1;
		if (material)
		{
			if (!IS_MANAGED(Graphical_material)(material,
				material_editor_dialog->graphical_material_manager))
			{
				display_message(ERROR_MESSAGE,
					"material_editor_dialog_set_material.  Material not managed");
				material=(struct Graphical_material *)NULL;
				return_code=0;
			}
		}
		if (!material)
		{
			material=FIRST_OBJECT_IN_MANAGER_THAT(Graphical_material)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
				(void *)NULL,
				material_editor_dialog->graphical_material_manager);
		}
// 		if (return_code=SELECT_SET_SELECT_ITEM(Graphical_material)(
// 			material_editor_dialog->select_widget,material))
// 		{
// 			material_editor_dialog->current_value=material;
// 			material_editor_wx_set_material(
// 				material_editor_dialog->material_editor,
// 				material_editor_dialog->current_value);
// 		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_set_material.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_dialog_set_material */

static struct Material_editor_dialog *CREATE(Material_editor_dialog)(
	struct Material_editor_dialog **material_editor_dialog_address,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *init_data,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/
{
	int init_widgets;
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(CREATE(Material_editor_dialog));
	material_editor_dialog = (struct Material_editor_dialog *)NULL;
	if (material_editor_dialog_address && graphical_material_manager &&
		texture_manager && user_interface)
	{
		 /* allocate memory */
		 if (ALLOCATE(material_editor_dialog,
					 struct Material_editor_dialog,1))
		 {
				/* initialise the structure */
				material_editor_dialog->material_editor_dialog_address=
					 material_editor_dialog_address;
				material_editor_dialog->graphical_material_manager=
					graphical_material_manager;
				/* current_value set in material_editor_dialog_set_material */
				material_editor_dialog->current_value=
					(struct Graphical_material *)NULL;
				material_editor_dialog->material_editor =
					(struct Material_editor *)NULL;
				material_editor_dialog->user_interface=user_interface;
				init_widgets=1;
				/* make the dialog shell */
				if (!(material_editor_dialog->material_editor =
							CREATE(Material_editor)(
								 material_editor_dialog_address,
								 graphical_material_manager,
								 texture_manager,
								 (struct Graphical_material *)NULL, graphics_buffer_package,
								 user_interface)))
				{
					 display_message(ERROR_MESSAGE,
							"CREATE(Material_editor_dialog).  Could not create editor widget.");
					 init_widgets=0;
				}
				if (init_widgets)
				{
					 /* set current_value to init_data */
					 material_editor_dialog_set_material(
							material_editor_dialog, init_data);
				}
				else
				{
					 DEALLOCATE(material_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"CREATE(Material_editor_dialog).  Could not allocate material_editor_dialog");
			}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_editor_dialog).  Invalid argument(s)");
	}
	if (material_editor_dialog_address && material_editor_dialog)
	{
		*material_editor_dialog_address = material_editor_dialog;
	}
	LEAVE;

	return (material_editor_dialog);
} /* CREATE(Material_editor_dialog) */

int bring_up_material_editor_dialog_wx(
	struct Material_editor_dialog **material_editor_dialog_address,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Texture) *texture_manager,struct Graphical_material *material,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
If there is a material_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(bring_up_material_editor_dialog);
	if (material_editor_dialog_address)
	{
		if (material_editor_dialog = *material_editor_dialog_address)
		{
// 			material_editor_dialog_set_material(material_editor_dialog, material);
// 			XtPopup(material_editor_dialog->dialog, XtGrabNone);
// 			XtVaSetValues(material_editor_dialog->dialog, XmNiconic,
// 			False, NULL);
			 material_editor_bring_up_editor(material_editor_dialog->material_editor);
			return_code = 1;
		}
		else
		{
			if (CREATE(Material_editor_dialog)(material_editor_dialog_address,
				graphical_material_manager, texture_manager, material, 
				graphics_buffer_package, user_interface))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_material_editor_dialog.  Error creating dialog");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_material_editor_dialog.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_material_editor_dialog */

int DESTROY(Material_editor_dialog)(
	struct Material_editor_dialog **material_editor_dialog_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_dialog_address> and sets
<*material_editor_dialog_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Material_editor_dialog *material_editor_dialog;

	ENTER(DESTROY(Material_editor_dialog));
	if (material_editor_dialog_address &&
		(material_editor_dialog = *material_editor_dialog_address))
	{
		DESTROY(Material_editor)(&(material_editor_dialog->material_editor));
		DEALLOCATE(*material_editor_dialog_address);
		*material_editor_dialog_address = (struct Material_editor_dialog *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Material_editor_dialog).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Material_editor_dialog) */

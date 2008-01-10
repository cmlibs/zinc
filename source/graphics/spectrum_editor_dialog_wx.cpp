/*******************************************************************************
FILE : spectrum_editor_dialog_wx.cpp

LAST MODIFIED : 23 Aug 2007

DESCRIPTION :
This module creates a spectrum_editor_dialog.
???SAB.  Basically pillaged from material/material_editor_dialog.c
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

extern "C" {
#include <stdio.h>
#include "three_d_drawing/graphics_buffer.h"
#include "general/debug.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/light.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#include "user_interface/message.h"
}

struct Spectrum_editor_dialog
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Contains all the information carried by the spectrum_editor_dialog widget.
Note that we just hold a pointer to the spectrum_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
// 	struct Callback_data update_callback;
	struct Spectrum *current_value;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Scene *autorange_scene;
	struct Spectrum_editor *spectrum_editor;
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address;
	struct User_interface *user_interface;
};

static struct Spectrum_editor_dialog *CREATE(Spectrum_editor_dialog)(
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	 struct MANAGER(Spectrum) *spectrum_manager,
	 struct Spectrum *init_data, 
	 struct Graphics_font *font, 
	 struct Graphics_buffer_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct LIST(GT_object) *glyph_list,
	 struct MANAGER(Graphical_material) *graphical_material_manager,
	 struct MANAGER(Light) *light_manager,
	 struct MANAGER(Texture) *texture_manager,
	 struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the spectrums contained in the global list.
==============================================================================*/
{
	int init_widgets;
// 	struct Callback_data callback;
	struct Spectrum_editor_dialog *spectrum_editor_dialog=NULL;

	ENTER(CREATE(Spectrum_editor_dialog));
	spectrum_editor_dialog = (struct Spectrum_editor_dialog *)NULL;
	/* check arguments */
	if (spectrum_manager&&user_interface)
	{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor_dialog,
				struct Spectrum_editor_dialog,1))
			{
				/* initialise the structure */
				spectrum_editor_dialog->spectrum_editor_dialog_address=
					spectrum_editor_dialog_address;
				spectrum_editor_dialog->spectrum_manager=
					spectrum_manager;
				/* current_value set in spectrum_editor_dialog_set_spectrum */
				spectrum_editor_dialog->current_value=
					(struct Spectrum *)NULL;
				spectrum_editor_dialog->spectrum_editor =
					(struct Spectrum_editor *)NULL;
				spectrum_editor_dialog->autorange_scene = (struct Scene *)NULL;
				spectrum_editor_dialog->user_interface = user_interface;
				/* set the mode toggle to the correct position */
				init_widgets=1;
				if (!(spectrum_editor_dialog->spectrum_editor =
							CREATE(Spectrum_editor)(
								 spectrum_editor_dialog_address,
								 init_data, font,
								 graphics_buffer_package,
								 user_interface, glyph_list,
								 graphical_material_manager, light_manager,
								 spectrum_manager, texture_manager,
								 scene_manager)))
				{
									 display_message(ERROR_MESSAGE,
											"CREATE(Spectrum_editor_dialog).  "
											"Could not create spectrum editor");
									 init_widgets = 0;
				}
#if defined (OLD_CODE)
				spectrum_editor_dialog_set_spectrum(
					 spectrum_editor_dialog,init_data);
#endif /* defined (OLD_CODE) */
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"CREATE(Spectrum_editor_dialog).  Could not allocate spectrum_editor_dialog");
			}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "CREATE(Spectrum_editor_dialog).  Invalid argument(s)");
	}
	if (spectrum_editor_dialog_address && spectrum_editor_dialog)
	{
		 *spectrum_editor_dialog_address = spectrum_editor_dialog;
	}
	LEAVE;
	
	return (spectrum_editor_dialog);
} /* CREATE(Spectrum_editor_dialog) */

struct Spectrum *spectrum_editor_dialog_get_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Returns the spectrum edited by the <spectrum_editor_dialog>.
==============================================================================*/
{
	struct Spectrum *spectrum;

	ENTER(spectrum_editor_dialog_get_spectrum);
	if (spectrum_editor_dialog)
	{
		spectrum = spectrum_editor_dialog->current_value;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_get_spectrum.  Invalid argument(s)");
		spectrum = (struct Spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* spectrum_editor_dialog_get_spectrum */

#if defined (OLD_CODE)
int spectrum_editor_dialog_set_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Set the <spectrum> for the <spectrum_editor_dialog>.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_dialog_set_spectrum);
	return_code=0;
	/* check arguments */
	if (spectrum_editor_dialog)
	{
		return_code=1;
		if (spectrum)
		{
			if (!IS_MANAGED(Spectrum)(spectrum,
				spectrum_editor_dialog->spectrum_manager))
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_dialog_set_spectrum.  Spectrum not managed");
				spectrum=(struct Spectrum *)NULL;
				return_code=0;
			}
		}
		if (!spectrum)
		{
			spectrum=FIRST_OBJECT_IN_MANAGER_THAT(Spectrum)(
				(MANAGER_CONDITIONAL_FUNCTION(Spectrum) *)NULL,
				(void *)NULL,
				spectrum_editor_dialog->spectrum_manager);
		}
		spectrum_editor_dialog->current_value=spectrum;
		spectrum_editor_wx_bring_up_editor(spectrum_editor_dialog->spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_set_spectrum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_dialog_set_spectrum */
#endif /* defined (OLD_CODE) */

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
	struct MANAGER(Scene) *scene_manager)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then de-iconify it and
bring it to the front, otherwise create a new one.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(bring_up_spectrum_editor_dialog);
	if (spectrum_editor_dialog_address)
	{
		if (spectrum_editor_dialog = *spectrum_editor_dialog_address)
		{		
			spectrum_editor_wx_bring_up_editor(spectrum_editor_dialog->spectrum_editor);
			return_code = 1;
		}
		else
		{
			if (CREATE(Spectrum_editor_dialog)(spectrum_editor_dialog_address,
				spectrum_manager, spectrum, font, graphics_buffer_package,
				user_interface, glyph_list,
				graphical_material_manager, light_manager,
				texture_manager, scene_manager))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_spectrum_editor_dialog.  Error creating dialog");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_spectrum_editor_dialog.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_spectrum_editor_dialog */

int DESTROY(Spectrum_editor_dialog)(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroy the <*spectrum_editor_dialog_address> and sets
<*spectrum_editor_dialog_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_destroy);
	if (spectrum_editor_dialog_address &&
		(spectrum_editor_dialog= *spectrum_editor_dialog_address))
	{
		return_code = 1;
		DESTROY(Spectrum_editor)(&(spectrum_editor_dialog->spectrum_editor));
		DEALLOCATE(*spectrum_editor_dialog_address);
		*spectrum_editor_dialog_address = (struct Spectrum_editor_dialog *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Spectrum_editor_dialog).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_editor_dialog) */

/*******************************************************************************
FILE : colour_editor_wx.hpp

LAST MODIFIED : 6 November 2007

DESCRIPTION :
Creates a window that allows the user to create a colour.  Each colour component
ranges between 0-1.
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

#if !defined (COLOUR_EDITOR_WX_HPP)
#define COLOUR_EDITOR_WX_HPP

extern "C" {
#include <math.h>
#include "graphics/colour.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}
#include "wx/wx.h"

enum Colour_editor_mode
/*******************************************************************************
LAST MODIFIED : 22 December 1994

DESCRIPTION :
Contains the different types of colour_editor input methods.
==============================================================================*/
{
	COLOUR_EDITOR_RGB,
	COLOUR_EDITOR_HSV,
	COLOUR_EDITOR_CMY
}; /* Colour_Editor_mode */

class Colour_editor : public wxPanel
{

public:

	 Colour_editor(wxPanel *parent, const char *panel_name, enum Colour_editor_mode mode, struct Colour *colour, 
			void *material_editor_temp);
	 int colour_editor_wx_set_colour(struct Colour *colour);
	 Colour *colour_editor_wx_get_colour();
	 void colour_editor_wx_get_colour_from_interface_textctrl(struct Colour *colour);
	 void colour_editor_wx_get_colour_from_interface_slider(struct Colour *colour);
	 void colour_editor_wx_change_mode_from_choice();

private:
	 
    void set_properties();
    void do_layout();
	 void colour_editor_wx_conversion(enum Colour_editor_mode old_mode,
			enum Colour_editor_mode new_mode,struct Colour *old_data,
			struct Colour *new_data);
	 void colour_editor_wx_update_value(
			int item_num);
	 void colour_editor_wx_update();
	 void colour_editor_wx_update_panel_colour();
 	 void OnColourEditorTextEntered(wxCommandEvent& event);
 	 void OnColourEditorSliderChanged(wxCommandEvent& event);
	 void OnColourEditorColourModeChoiceChanged(wxCommandEvent& event);

protected:
	 wxStaticBox *colour_editor_staticbox;
	 wxChoice *colour_mode_choice;
	 wxStaticText *title, *colour_editor_colour_text_1, *colour_editor_colour_text_2, *colour_editor_colour_text_3;
	 wxTextCtrl *colour_editor_text_ctrl_1, *colour_editor_text_ctrl_2, *colour_editor_text_ctrl_3;
	 wxSlider *colour_editor_slider_1, *colour_editor_slider_2, *colour_editor_slider_3;
	 wxPanel *colour_editor_panel, *colour_palette_panel;
	 struct Colour current;
	 enum Colour_editor_mode current_mode,return_mode;
	 wxString colour_text_choice[3][3];
	 void *material_editor_void;
};
#endif /* !defined (COLOUR_EDITOR_WX_HPP) */

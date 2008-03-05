/*******************************************************************************
FILE : transformation_editor_wx.hpp

LAST MODIFIED : 27 February 2008

DESCRIPTION :
Create a cpp class that act as a transformation editor for the wx widgets.
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

#if !defined (TRANSFORMATION_EDITOR_WX_HPP)
#define TRANSFROMATION_EDITOR_WX_HPP

#include "wx/wx.h"
#include "wx/image.h"
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/spinbutt.h>

extern "C"{
#include "io_devices/conversion.h"
#include "graphics/graphics_library.h"
#include "graphics/scene.h"
}

class Transformation_editor : public wxPanel
{

public:

	 Transformation_editor(wxPanel *parent, const char *panel_name, struct Scene_object *scene_object,
			int *auto_apply);
	 int transformation_editor_wx_set_transformation(gtMatrix *transformation_matrix);
	 void transformation_editor_wx_set_current_object(struct Scene_object *scene_object);
	 void ApplyTransformation();

private:

	 void connect_callback();
	 void set_properties();
	 void do_layout();
	 int position_direction_to_transformation_matrix(
			struct Dof3_data *position, struct Dof3_data *direction,
			gtMatrix *transformation_matrix);
	 void OnTransformationEditorTextEntered(wxCommandEvent& event);
	 void transformation_editor_wx_update_position_and_direction();
	 void transformation_editor_wx_get_rate_of_change_from_interface_slider();
	 void OnTransformationEditorRateofChangeSliderChanged(wxCommandEvent& event);
	 void OnTransformationEditor_spin_button_up(wxCommandEvent& event);
	 void OnTransformationEditor_spin_button_down(wxCommandEvent& event);
	 void transformation_editor_wx_spin_button_change_value(wxSpinButton *temp_object, int flag);

protected:
 	 int *auto_apply_flag;
	 struct Scene_object *current_object;
	 gtMatrix transformation_editor_transformation_matrix;
	 int rate_of_change;
	 struct Dof3_data global_direction, global_position;
	 wxPanel *transformation_editor_panel;
	 wxStaticLine *Transformation_editor_wx_staticline;
	 wxStaticBox* direction_sizer_8_staticbox;
	 wxStaticBox* direction_sizer_7_staticbox;
	 wxStaticBox* direction_sizer_6_staticbox;
	 wxStaticBox* position_sizer_8_staticbox;
	 wxStaticBox* position_sizer_7_staticbox;
	 wxStaticBox* position_sizer_6_staticbox;
	 wxStaticBox* Transformation_editor_wx_position_staticbox;
	 wxStaticBox* Transformation_editor_wx_direction_staticbox;
	 wxChoice* Transformation_editor_wx_global_choice;
	 wxButton* Transformation_editor_wx_position_save_button;
	 wxButton* Transformation_editor_wx_position_reset_button;
	 wxToggleButton* Transformation_editor_wx_position_lock_data_toggle_button;
	 wxToggleButton* Transformation_editor_wx_position_link_resolution_toggle_button;
	 wxStaticText* Transformation_editor_wx_position_label_1;
	 wxChoice* Transformation_editor_wx_position_coord_system_choice;
	 wxStaticText* Transformation_editor_wx_position_label_2;
	 wxSlider* Transformation_editor_wx_position_slider_1;
	 wxStaticText* Transformation_editor_wx_position_label_3;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_1;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_1;
	 wxStaticText* Transformation_editor_wx_position_label_4;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_2;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_2;
	 wxStaticText* Transformation_editor_wx_position_label_5;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_3;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_3;
	 wxSlider* Transformation_editor_wx_position_slider_2;
	 wxButton* Transformation_editor_wx_direction_save_button;
	 wxButton* Transformation_editor_wx_direction_reset_button;
	 wxToggleButton* Transformation_editor_wx_direction_lock_data_toggle_button;
	 wxToggleButton* Transformation_editor_wx_direction_link_resolution_toggle_button;
	 wxStaticText* Transformation_editor_wx_direction_label_1;
	 wxChoice* Transformation_editor_wx_direction_coord_system_choice;
	 wxStaticText* Transformation_editor_wx_direction_label_3;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_1;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_1;
	 wxStaticText* Transformation_editor_wx_direction_label_4;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_2;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_2;
	 wxStaticText* Transformation_editor_wx_direction_label_5;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_3;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_3;
	 wxSlider* Transformation_editor_wx_direction_slider_2;
};
#endif /* !defined (TRANSFORMATION_EDITOR_WX_HPP) */

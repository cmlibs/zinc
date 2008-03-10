/*******************************************************************************
FILE : transformation_editor_wx.cpp

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

#include "transformation_editor_wx.hpp"

extern "C"{
#include "io_devices/matrix.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}

Transformation_editor::Transformation_editor(wxPanel *parent, const char *panel_name, 
	 struct Scene_object *scene_object, int *auto_apply)
/*******************************************************************************
LAST MODIFIED : 5 March 2008

DESCRIPTION :
Initialised the variables and set up the widgets used in the 
transformation_editor;
==============================================================================*/
{
	 direction_system_index = 0;
	 transformation_editor_panel = parent;
	 current_object = scene_object;
	 auto_apply_flag = auto_apply;
	 rate_of_change = 0;
	 transformation_editor_quaternion = new Quaternion();
	 position_sizer_6_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 position_sizer_7_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 position_sizer_8_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 position_sizer_13_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_6_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_7_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_8_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_13_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 if (ALLOCATE(global_quat, float, 4))
	 {
			global_quat[0] = 1;
			global_quat[1] = 0;
			global_quat[2] = 0;
			global_quat[3] = 0;	
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "transformation_editor. Cannot allocate sufficient memory");
	 }
	 transformation_editor_direction_text[0][0] = wxT("   Azimuth");
	 transformation_editor_direction_text[0][1] = wxT("   Elevation");
	 transformation_editor_direction_text[0][2] = wxT("   Roll");
	 transformation_editor_direction_text[1][0] = wxT("   X");
	 transformation_editor_direction_text[1][1] = wxT("   Y");
	 transformation_editor_direction_text[1][2] = wxT("   Z");
	 
	 Transformation_editor_wx_position_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxT("Position"));
	 Transformation_editor_wx_direction_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxT("Direction"));
	 const wxString Transformation_editor_wx_global_choice_choices[] = {
			wxT("Global"),
			wxT("POI")
	 };
	 Transformation_editor_wx_staticline = new wxStaticLine(transformation_editor_panel, wxID_ANY);
	 Transformation_editor_wx_global_choice = new wxChoice(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,25), 2, Transformation_editor_wx_global_choice_choices, 0);
	 Transformation_editor_wx_position_save_button = new wxButton(transformation_editor_panel, wxID_ANY, wxT("Save"), wxPoint(-1,-1), wxSize(-1,25));
	 Transformation_editor_wx_position_reset_button = new wxButton(transformation_editor_panel, wxID_ANY, wxT("Reset"), wxPoint(-1,-1), wxSize(-1,25));
	 Transformation_editor_wx_position_lock_data_toggle_button = new wxToggleButton(transformation_editor_panel, wxID_ANY, wxT("Lock Data"), wxPoint(-1,-1), wxSize(-1,25));
	 Transformation_editor_wx_position_link_resolution_toggle_button = new wxToggleButton(transformation_editor_panel, wxID_ANY, wxT("Link Resolution"), wxPoint(-1,-1), wxSize(-1,25));
	 Transformation_editor_wx_position_label_1 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("Coord System"), wxPoint(-1,-1), wxSize(-1,20));
    const wxString Transformation_editor_wx_position_coord_system_choice_choices[] = {
			 wxT("Rectangular Cartesian"),
			 wxT("Cylindrical Polar"),
			 wxT("Spherical Polar")
    };
    Transformation_editor_wx_position_coord_system_choice = new wxChoice(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,25), 3, Transformation_editor_wx_position_coord_system_choice_choices, 0);
    Transformation_editor_wx_position_label_2 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("Rate_of_change:"), wxPoint(-1,-1), wxSize(-1,30));
    Transformation_editor_wx_position_slider_1 = new wxSlider(transformation_editor_panel, wxID_ANY, 0, -10, 10,  wxPoint(-1,-1), wxSize(-1,30), wxSL_HORIZONTAL|wxSL_LABELS);
    Transformation_editor_wx_position_label_3 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  X Axis"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_position_text_ctrl_1 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_position_spin_button_1 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_position_label_4 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  Y Axis"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_position_text_ctrl_2 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
    Transformation_editor_wx_position_spin_button_2 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_position_label_5 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  Z Axis"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_position_text_ctrl_3 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
    Transformation_editor_wx_position_spin_button_3 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_position_label_6 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  Scale Factor"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_position_text_ctrl_4 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
    Transformation_editor_wx_position_spin_button_4 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_direction_save_button = new wxButton(transformation_editor_panel, wxID_ANY, wxT("Save"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_reset_button = new wxButton(transformation_editor_panel, wxID_ANY, wxT("Reset"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_lock_data_toggle_button = new wxToggleButton(transformation_editor_panel, wxID_ANY, wxT("Lock Data"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_link_resolution_toggle_button = new wxToggleButton(transformation_editor_panel, wxID_ANY, wxT("Link Resolution"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_label_1 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("Coord System"), wxPoint(-1,-1), wxSize(-1,20));
    const wxString Transformation_editor_wx_direction_coord_system_choice_choices[] = {
			 wxT("Euler"),
			 wxT("Quaternion")
    };
    Transformation_editor_wx_direction_coord_system_choice = new wxChoice(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,25), 2, Transformation_editor_wx_direction_coord_system_choice_choices, 0);
    Transformation_editor_wx_direction_label_3 = new wxStaticText(
			 transformation_editor_panel, wxID_ANY, transformation_editor_direction_text[0][0], wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_1 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_1 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_direction_label_4 = new wxStaticText(transformation_editor_panel, wxID_ANY, transformation_editor_direction_text[0][1], wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_2 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString,wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_2 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_direction_label_5 = new wxStaticText(transformation_editor_panel, wxID_ANY, transformation_editor_direction_text[0][2], wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_3 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_3 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_direction_label_6 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  W"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_4 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_4 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);

		connect_callback();
    set_properties();
    do_layout();
}


void Transformation_editor::set_properties()
/*******************************************************************************
LAST MODIFIED : 5 March 2008

DESCRIPTION :
Set properties for some of the widgets component;
==============================================================================*/
{
	 Transformation_editor_wx_global_choice->Hide();
	 Transformation_editor_wx_position_save_button->Hide();
	 Transformation_editor_wx_position_reset_button->Hide();
	 Transformation_editor_wx_position_lock_data_toggle_button->Hide();
	 Transformation_editor_wx_position_link_resolution_toggle_button->Hide();
	 Transformation_editor_wx_position_label_1->Hide();
	 Transformation_editor_wx_position_coord_system_choice->Hide();
	 Transformation_editor_wx_direction_save_button->Hide();
	 Transformation_editor_wx_direction_reset_button->Hide();
	 Transformation_editor_wx_direction_lock_data_toggle_button->Hide();
	 Transformation_editor_wx_direction_link_resolution_toggle_button->Hide();
	 Transformation_editor_wx_direction_label_1->Hide();
	 //Transformation_editor_wx_direction_coord_system_choice->Hide();
	 Transformation_editor_wx_global_choice->SetSelection(0);
	 Transformation_editor_wx_position_coord_system_choice->SetSelection(0);
	 Transformation_editor_wx_direction_coord_system_choice->SetSelection(0);
	 Transformation_editor_wx_position_spin_button_1->SetRange(-100000,100000);
	 Transformation_editor_wx_position_spin_button_2->SetRange(-100000,100000);
	 Transformation_editor_wx_position_spin_button_3->SetRange(-100000,100000);
	 Transformation_editor_wx_position_spin_button_4->SetRange(-100000,100000);
	 Transformation_editor_wx_direction_spin_button_1->SetRange(-100000,100000);
	 Transformation_editor_wx_direction_spin_button_2->SetRange(-100000,100000);
	 Transformation_editor_wx_direction_spin_button_3->SetRange(-100000,100000);
 	 Transformation_editor_wx_direction_label_6->Hide();
 	 Transformation_editor_wx_direction_text_ctrl_4->Hide();
 	 Transformation_editor_wx_direction_spin_button_4->Hide();
	 direction_sizer_13_staticbox->Hide();
	 Transformation_editor_wx_direction_spin_button_4->SetRange(-100000,100000);
 	 Transformation_editor_wx_position_spin_button_4->Hide();
}

void Transformation_editor::connect_callback()
/*******************************************************************************
LAST MODIFIED : 5 March 2008

DESCRIPTION :
Connect the widgets to a callback function.
==============================================================================*/
{
    Transformation_editor_wx_position_slider_1->SetClientData((void *)this);
		Transformation_editor_wx_position_slider_1->Connect(wxEVT_SCROLL_THUMBTRACK, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorRateofChangeSliderChanged));
		Transformation_editor_wx_position_slider_1->Connect(wxEVT_SCROLL_THUMBRELEASE, 
			wxCommandEventHandler(Transformation_editor::OnTransformationEditorRateofChangeSliderChanged));

		Transformation_editor_wx_position_text_ctrl_1->SetClientData((void *)this);
		Transformation_editor_wx_position_text_ctrl_1->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_position_text_ctrl_2->SetClientData((void *)this);
		Transformation_editor_wx_position_text_ctrl_2->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_position_text_ctrl_3->SetClientData((void *)this);
		Transformation_editor_wx_position_text_ctrl_3->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_position_text_ctrl_4->SetClientData((void *)this);
		Transformation_editor_wx_position_text_ctrl_4->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));

		Transformation_editor_wx_direction_text_ctrl_1->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_1->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_direction_text_ctrl_2->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_2->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_direction_text_ctrl_3->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_3->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_direction_text_ctrl_4->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_4->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));

		Transformation_editor_wx_position_spin_button_1->SetClientData((void *)this);
		Transformation_editor_wx_position_spin_button_1->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_position_spin_button_1->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));
		Transformation_editor_wx_position_spin_button_2->SetClientData((void *)this);
		Transformation_editor_wx_position_spin_button_2->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_position_spin_button_2->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));
		Transformation_editor_wx_position_spin_button_3->SetClientData((void *)this);
		Transformation_editor_wx_position_spin_button_3->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_position_spin_button_3->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));

		Transformation_editor_wx_direction_spin_button_1->SetClientData((void *)this);
		Transformation_editor_wx_direction_spin_button_1->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_direction_spin_button_1->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));
		Transformation_editor_wx_direction_spin_button_2->SetClientData((void *)this);
		Transformation_editor_wx_direction_spin_button_2->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_direction_spin_button_2->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));
		Transformation_editor_wx_direction_spin_button_3->SetClientData((void *)this);
		Transformation_editor_wx_direction_spin_button_3->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_direction_spin_button_3->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));
		Transformation_editor_wx_direction_spin_button_4->SetClientData((void *)this);
		Transformation_editor_wx_direction_spin_button_4->Connect(wxEVT_SCROLL_LINEUP,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_up));
		Transformation_editor_wx_direction_spin_button_4->Connect(wxEVT_SCROLL_LINEDOWN,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditor_spin_button_down));
		
		Transformation_editor_wx_direction_coord_system_choice->SetClientData((void *)this);
		Transformation_editor_wx_direction_coord_system_choice->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorDirectionSystemChoice));

}

void Transformation_editor::do_layout()
/*******************************************************************************
LAST MODIFIED : 5 March 2008

DESCRIPTION :
Setup the layout for transformation editor.
==============================================================================*/
{
	 wxBoxSizer* Transformation_editor_wx_top_sizer = new wxBoxSizer(wxVERTICAL);
	 wxBoxSizer* direction_sizer_1 = new wxStaticBoxSizer(
			Transformation_editor_wx_direction_staticbox, wxVERTICAL);
	 wxBoxSizer* direction_sizer_9 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* direction_sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* direction_sizer_13 = new wxStaticBoxSizer(direction_sizer_13_staticbox, wxVERTICAL);
	 wxBoxSizer* direction_sizer_14 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* direction_sizer_8 = new wxStaticBoxSizer(direction_sizer_8_staticbox, wxVERTICAL);
	 wxBoxSizer* direction_sizer_12 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* direction_sizer_7 = new wxStaticBoxSizer(direction_sizer_7_staticbox, wxVERTICAL);
	 wxBoxSizer* direction_sizer_11 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* direction_sizer_6 = new wxStaticBoxSizer(direction_sizer_6_staticbox, wxVERTICAL);
	 wxBoxSizer* direction_sizer_10 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* direction_sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* direction_sizer_3 = new wxBoxSizer(wxVERTICAL);
	 wxBoxSizer* position_sizer_1 = new wxStaticBoxSizer(
			Transformation_editor_wx_position_staticbox, wxVERTICAL);
	 wxBoxSizer* position_sizer_9 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* position_sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* position_sizer_13 = new wxStaticBoxSizer(position_sizer_13_staticbox, wxVERTICAL);
	 wxBoxSizer* position_sizer_14 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* position_sizer_8 = new wxStaticBoxSizer(position_sizer_8_staticbox, wxVERTICAL);
	 wxBoxSizer* position_sizer_12 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* position_sizer_7 = new wxStaticBoxSizer(position_sizer_7_staticbox, wxVERTICAL);
	 wxBoxSizer* position_sizer_11 = new wxBoxSizer(wxHORIZONTAL);
	 wxStaticBoxSizer* position_sizer_6 = new wxStaticBoxSizer(position_sizer_6_staticbox, wxVERTICAL);
	 wxBoxSizer* position_sizer_10 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* position_sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* position_sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	 wxBoxSizer* position_sizer_3 = new wxBoxSizer(wxVERTICAL);
	 Transformation_editor_wx_top_sizer->Add(Transformation_editor_wx_staticline, 0, wxEXPAND, 1);
	 Transformation_editor_wx_top_sizer->Add(Transformation_editor_wx_global_choice, 0, wxALIGN_RIGHT|wxALIGN_CENTER_HORIZONTAL, 1);
	 position_sizer_2->Add(Transformation_editor_wx_position_save_button, 0, wxALIGN_BOTTOM, 0);
	 position_sizer_2->Add(Transformation_editor_wx_position_reset_button, 0, wxALIGN_BOTTOM, 0);
	 position_sizer_2->Add(Transformation_editor_wx_position_lock_data_toggle_button, 0, wxALIGN_BOTTOM, 0);
	 position_sizer_2->Add(Transformation_editor_wx_position_link_resolution_toggle_button, 0, wxALIGN_BOTTOM, 0);
	 position_sizer_3->Add(Transformation_editor_wx_position_label_1, 0, 0, 0);
	 position_sizer_3->Add(Transformation_editor_wx_position_coord_system_choice, 0, 0, 0);
	 position_sizer_2->Add(position_sizer_3, 1, wxEXPAND, 0);
	 position_sizer_1->Add(position_sizer_2, 0, wxEXPAND, 0);
	 position_sizer_4->Add(Transformation_editor_wx_position_label_2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	 position_sizer_4->Add(Transformation_editor_wx_position_slider_1, 1, 0, 0);
	 Transformation_editor_wx_top_sizer->Add(position_sizer_4, 0, wxEXPAND, 1);
	 position_sizer_13->Add(Transformation_editor_wx_position_label_6, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 position_sizer_14->Add(Transformation_editor_wx_position_text_ctrl_4, 1, 0, 0);
	 position_sizer_14->Add(Transformation_editor_wx_position_spin_button_4, 0, 0, 0);
 	 position_sizer_13->Add(position_sizer_14, 1, wxEXPAND, 0);
	 position_sizer_5->Add(position_sizer_13, 1, wxEXPAND, 0);
	 position_sizer_6->Add(Transformation_editor_wx_position_label_3, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 position_sizer_10->Add(Transformation_editor_wx_position_text_ctrl_1, 1, 0, 0);
	 position_sizer_10->Add(Transformation_editor_wx_position_spin_button_1, 0, 0, 0);
	 position_sizer_6->Add(position_sizer_10, 1, wxEXPAND, 0);
	 position_sizer_5->Add(position_sizer_6, 1, wxEXPAND, 0);
	 position_sizer_7->Add(Transformation_editor_wx_position_label_4, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 position_sizer_11->Add(Transformation_editor_wx_position_text_ctrl_2, 1, 0, 0);
	 position_sizer_11->Add(Transformation_editor_wx_position_spin_button_2, 0, 0, 0);
	 position_sizer_7->Add(position_sizer_11, 1, wxEXPAND, 0);
	 position_sizer_5->Add(position_sizer_7, 1, wxEXPAND, 0);
	 position_sizer_8->Add(Transformation_editor_wx_position_label_5, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 position_sizer_12->Add(Transformation_editor_wx_position_text_ctrl_3, 1, 0, 0);
	 position_sizer_12->Add(Transformation_editor_wx_position_spin_button_3, 0, 0, 0);
	 position_sizer_8->Add(position_sizer_12, 1, wxEXPAND, 0);
	 position_sizer_5->Add(position_sizer_8, 1, wxEXPAND, 0);
	 position_sizer_1->Add(position_sizer_5, 0, wxEXPAND, 0);
	 position_sizer_1->Add(position_sizer_9, 0, wxEXPAND, 0);
	 Transformation_editor_wx_top_sizer->Add(position_sizer_1, 0, wxEXPAND, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_save_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_reset_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_lock_data_toggle_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_link_resolution_toggle_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_3->Add(Transformation_editor_wx_direction_label_1, 0, 0, 0);
	 direction_sizer_3->Add(Transformation_editor_wx_direction_coord_system_choice, 0, 0, 0);
	 direction_sizer_2->Add(direction_sizer_3, 1, wxEXPAND, 0);
	 direction_sizer_1->Add(direction_sizer_2, 0, wxEXPAND, 0);
	 direction_sizer_13->Add(Transformation_editor_wx_direction_label_6, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 direction_sizer_14->Add(Transformation_editor_wx_direction_text_ctrl_4, 1, 0, 0);
	 direction_sizer_14->Add(Transformation_editor_wx_direction_spin_button_4, 0, 0, 0);
 	 direction_sizer_13->Add(direction_sizer_14, 1, wxEXPAND, 0);
	 direction_sizer_5->Add(direction_sizer_13, 1, wxEXPAND, 0);
	 direction_sizer_6->Add(Transformation_editor_wx_direction_label_3, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 direction_sizer_10->Add(Transformation_editor_wx_direction_text_ctrl_1, 1, 0, 0);
	 direction_sizer_10->Add(Transformation_editor_wx_direction_spin_button_1, 0, 0, 0);
	 direction_sizer_6->Add(direction_sizer_10, 1, wxEXPAND, 0);
	 direction_sizer_5->Add(direction_sizer_6, 1, wxEXPAND, 0);
	 direction_sizer_7->Add(Transformation_editor_wx_direction_label_4, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 direction_sizer_11->Add(Transformation_editor_wx_direction_text_ctrl_2, 1, 0, 0);
	 direction_sizer_11->Add(Transformation_editor_wx_direction_spin_button_2, 0, 0, 0);
	 direction_sizer_7->Add(direction_sizer_11, 1, wxEXPAND, 0);
	 direction_sizer_5->Add(direction_sizer_7, 1, wxEXPAND, 0);
	 direction_sizer_8->Add(Transformation_editor_wx_direction_label_5, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0);
	 direction_sizer_12->Add(Transformation_editor_wx_direction_text_ctrl_3, 1, 0, 0);
	 direction_sizer_12->Add(Transformation_editor_wx_direction_spin_button_3, 0, 0, 0);
	 direction_sizer_8->Add(direction_sizer_12, 1, wxEXPAND, 0);
	 direction_sizer_5->Add(direction_sizer_8, 1, wxEXPAND, 0);
	 direction_sizer_1->Add(direction_sizer_5, 0, wxEXPAND, 0);
	 direction_sizer_1->Add(direction_sizer_9, 0, wxEXPAND, 0);
	 Transformation_editor_wx_top_sizer->Add(direction_sizer_1, 0, wxEXPAND, 0);
	 transformation_editor_panel->SetSizer(Transformation_editor_wx_top_sizer);
	 Transformation_editor_wx_top_sizer->Fit(transformation_editor_panel);
	 Transformation_editor_wx_top_sizer->SetSizeHints(transformation_editor_panel);
	 transformation_editor_panel->Layout();
}

int Transformation_editor::transformation_editor_wx_set_transformation(gtMatrix *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 29 Feb 2008

DESCRIPTION :
Sets the <transformation_editor> to update the value shown in the widgets with the
transformation encoded in 4x4 <transformation_matrix>.
==============================================================================*/
{
	 char temp_str[20], temp_string[50];
	 int i, j, k, return_code;
	 Gmatrix gmatrix;
	 float *values;
	 gtMatrix resolved_transformation_matrix;

	 if (transformation_matrix)
	 {
			return_code = 1;
			/* 1. store the 4x4 transformation_matrix */
			for (i = 0; i < 4; i++)
			{
				 for (j = 0; j < 4; j++)
				 {
						transformation_editor_transformation_matrix[i][j] =
							 (*transformation_matrix)[i][j];
				 }
			}
			
			/* 1. convert gtMatrix into position and direction vectors, if possible */
			
			/* clear direction in case following fails */
			global_direction.data[0] = 0;
			global_direction.data[1] = 0;
			global_direction.data[2] = 0;
			/* convert the gtMatrix into position vector and direction
				 Euler angles. GMATRIX_SIZE is 3; following fails if changed! */
			for (i = 0; i < GMATRIX_SIZE; i++)
			{
				 for (j = 0; j < GMATRIX_SIZE; j++)
				 {
						gmatrix.data[i][j] = (*transformation_matrix)[i][j];
				 }
			}

			if (direction_system_index == 0)
			{
				 /* convert the matrix to Euler angles */
				 matrix_euler(&gmatrix, &global_direction);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_direction.data[0]);
				 Transformation_editor_wx_direction_text_ctrl_1->ChangeValue(temp_str);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_direction.data[1]);
				 Transformation_editor_wx_direction_text_ctrl_2->ChangeValue(temp_str);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_direction.data[2]);
				 Transformation_editor_wx_direction_text_ctrl_3->ChangeValue(temp_str);
			}
			else if (direction_system_index == 1)
			{
				 k = 0;
				 if (ALLOCATE(values, float, 16))
				 {
						for (i = 0;i < 4; i++)
						{
							 for (j = 0; j < 4; j++)
							 {
									values[k] = (*transformation_matrix)[i][j];
									k++;
							 }
						}
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "transformation_editor_wx_set_transformation. Cannot allocate sufficient memory");
				 }
				 transformation_editor_quaternion->matrix_to_quaternion(values, global_quat);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_quat[0]);
				 Transformation_editor_wx_direction_text_ctrl_4->ChangeValue(temp_str);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_quat[1]);
				 Transformation_editor_wx_direction_text_ctrl_1->ChangeValue(temp_str);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_quat[2]);
				 Transformation_editor_wx_direction_text_ctrl_2->ChangeValue(temp_str);
				 sprintf(temp_str, DOF3_NUM_FORMAT, global_quat[3]);
				 Transformation_editor_wx_direction_text_ctrl_3->ChangeValue(temp_str);
				 DEALLOCATE(values);

			}
			/* extract the position from the 4x4 matrix */
			global_position.data[0] = (*transformation_matrix)[3][0];
			global_position.data[1] = (*transformation_matrix)[3][1];
			global_position.data[2] = (*transformation_matrix)[3][2];
			sprintf(temp_str, DOF3_NUM_FORMAT, global_position.data[0]);
			Transformation_editor_wx_position_text_ctrl_1->ChangeValue(temp_str);
			sprintf(temp_str, DOF3_NUM_FORMAT, global_position.data[1]);
			Transformation_editor_wx_position_text_ctrl_2->ChangeValue(temp_str);
			sprintf(temp_str, DOF3_NUM_FORMAT, global_position.data[2]);
			Transformation_editor_wx_position_text_ctrl_3->ChangeValue(temp_str);
			if (position_direction_to_transformation_matrix(
						 &global_position, &global_direction, &resolved_transformation_matrix) &&
				 gtMatrix_match_with_tolerance(transformation_matrix,
						&resolved_transformation_matrix, /*tolerance*/1.0E-6))
			{
				 Transformation_editor_wx_position_text_ctrl_4->SetValue(wxT("1*1*1"));
			}
			else
			{
				 matrix_scalefactor(&gmatrix, global_scale_factor);
				 sprintf(temp_string,"%g*%g*%g",
						global_scale_factor[0],global_scale_factor[1],global_scale_factor[2]);
				 Transformation_editor_wx_position_text_ctrl_4->SetValue(temp_string);	 
			}
			
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_set_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	 return (return_code);
}

int Transformation_editor::position_direction_to_transformation_matrix(
	struct Dof3_data *position, struct Dof3_data *direction,
	gtMatrix *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Takes the 3 position and 3 direction values and puts them into the
4x4 transformation_matrix.
==============================================================================*/
{
	Gmatrix gmatrix;
	int i, j, return_code;

	ENTER(position_direction_to_transformation_matrix);
	if (position && direction && transformation_matrix)
	{
		euler_matrix(direction, &gmatrix);
		for (i = 0; i < GMATRIX_SIZE; i++)
		{
			for (j = 0; j < GMATRIX_SIZE; j++)
			{
				(*transformation_matrix)[i][j] = gmatrix.data[i][j];
			}
		}
		(*transformation_matrix)[3][0] = position->data[0];
		(*transformation_matrix)[3][1] = position->data[1];
		(*transformation_matrix)[3][2] = position->data[2];
		(*transformation_matrix)[0][3] = 0.0;
		(*transformation_matrix)[1][3] = 0.0;
		(*transformation_matrix)[2][3] = 0.0;
		(*transformation_matrix)[3][3] = 1.0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"position_direction_to_transformation_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* position_direction_to_transformation_matrix */

int Transformation_editor::scale_factor_to_transformation_matrix(
	Triple scale_factor, gtMatrix *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Takes the 3 scale factor value and puts them into the
4x4 transformation_matrix.
==============================================================================*/
{
	int i, j, return_code;

	ENTER(scale_factor_to_transformation_matrix);
	if (scale_factor && transformation_matrix)
	{
		for (i = 0; i < GMATRIX_SIZE; i++)
		{
			for (j = 0; j < GMATRIX_SIZE; j++)
			{
				 (*transformation_matrix)[i][j] = (*transformation_matrix)[i][j] * scale_factor[i];
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"scale_factor_to_transformation_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* position_direction_to_transformation_matrix */


void Transformation_editor::transformation_editor_wx_update_position_and_direction()
/*******************************************************************************
LAST MODIFIED : 3 March 2008

DESCRIPTION :
Sets the <transformation_editor> to update the value shown in the widgets with the
transformation encoded in 4x4 <transformation_matrix>.
==============================================================================*/
{
	 Dof3_data direction, position; //, global_direction, global_position;
	 double temp;
	 char *text, *text_entry;
	 Triple scale_factor;
	 struct Parse_state *temp_state;

	 scale_factor[0] = global_scale_factor[0];
	 scale_factor[1] = global_scale_factor[1];
	 scale_factor[2] = global_scale_factor[2];

	 if (direction_system_index == 0)
	 {
			text = NULL;
			text = (char*)Transformation_editor_wx_direction_text_ctrl_1->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%lf",&temp);
				 direction.data[0] = temp;
			}
			text = NULL;
			text = (char*)Transformation_editor_wx_direction_text_ctrl_2->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%lf",&temp);
				 direction.data[1] = temp;
			}
			text = NULL;
			text = (char*)Transformation_editor_wx_direction_text_ctrl_3->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%lf",&temp);
				 direction.data[2] = temp;
			}
	 }
	 else if (direction_system_index == 1)
	 {
			int i,j;
			Gmatrix gmatrix;
			for (i = 0; i < GMATRIX_SIZE; i++)
			{
				 for (j = 0; j < GMATRIX_SIZE; j++)
				 {
						gmatrix.data[i][j] = (transformation_editor_transformation_matrix)[i][j];
				 }
			}
			matrix_euler(&gmatrix, &direction);
	 }
	 text = NULL;
	 text = (char*)Transformation_editor_wx_position_text_ctrl_1->GetValue().mb_str(wxConvUTF8);
	 if (text)
	 {
			sscanf(text,"%lf",&temp);
			position.data[0] = temp;
	 }
	 text = NULL;
	 text = (char*)Transformation_editor_wx_position_text_ctrl_2->GetValue().mb_str(wxConvUTF8);
	 if (text)
	 {
			sscanf(text,"%lf",&temp);
			position.data[1] = temp;
	 }
	 text = NULL;
	 text = (char *)Transformation_editor_wx_position_text_ctrl_3->GetValue().mb_str(wxConvUTF8);
	 if (text)
	 {
			sscanf(text,"%lf",&temp);
			position.data[2] = temp;
	 }
	 text_entry = const_cast<char *>(Transformation_editor_wx_position_text_ctrl_4->
			GetValue().c_str());
	 if (text_entry)
	 {
			if (temp_state=create_Parse_state((char *)text_entry))
			{
				 set_special_float3(temp_state,scale_factor, const_cast<char *>("*"));
			}
	 }
	 position_direction_to_transformation_matrix(
			&position, &direction,&(transformation_editor_transformation_matrix));
	 scale_factor_to_transformation_matrix(scale_factor,
			&(transformation_editor_transformation_matrix));
	 if (temp_state)
			destroy_Parse_state(&temp_state);
	 ApplyTransformation();
}

void Transformation_editor::OnTransformationEditorTextEntered(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 3 March 2008

DESCRIPTION :
Process enter text event. Eventhandler somehow does not return any information of the colour
editor class although this function is inside the class. Therefore, I store the colour
editor inside the control object and get the control object whenever an event is
provoked then use this colour editor to do the settings.
==============================================================================*/
{
	 ENTER(Transformation_editor::OnTransformationEditorTextEntered);

	 wxTextCtrl *temp_object = (wxTextCtrl *)event.GetEventObject();
	 Transformation_editor *temp_editor = (Transformation_editor *)temp_object->GetClientData();
	 temp_editor->transformation_editor_wx_update_position_and_direction();
	 LEAVE;
}

void Transformation_editor::transformation_editor_wx_set_current_object(struct Scene_object *scene_object)
{
	 current_object = scene_object;
}

void Transformation_editor::transformation_editor_wx_get_rate_of_change_from_interface_slider()
{
	 rate_of_change = Transformation_editor_wx_position_slider_1->GetValue();
}


void Transformation_editor::OnTransformationEditorRateofChangeSliderChanged(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 4 March 2008

DESCRIPTION :
Process scroll event. Eventhandler somehow does not return any information of the colour
editor class although this function is inside the class. Therefore, I store the colour
editor inside the control object and get the control object whenever an event is
provoked then use this colour editor to do the settings.
==============================================================================*/
{
	 ENTER(Transformation_editor::OnTransformationEditorSliderChanged);

	 wxSlider *temp_object = (wxSlider *)event.GetEventObject();
	 Transformation_editor *temp_editor = (Transformation_editor *)temp_object->GetClientData();
	 temp_editor->transformation_editor_wx_get_rate_of_change_from_interface_slider();
	 LEAVE;
}

void Transformation_editor::transformation_editor_quaternion_to_gtmatrix()
/*******************************************************************************
LAST MODIFIED : 4 March 2008

DESCRIPTION :
A function in transformation editor to convert the global quaternion to 
global gtmatrix.
==============================================================================*/
{
	 float *values;
	 int i, j, k;

	 transformation_editor_quaternion->set(
			global_quat[0],global_quat[1], global_quat[2],global_quat[3]);
	 if (ALLOCATE(values, float, 16))
	 {
			transformation_editor_quaternion->quaternion_to_matrix(values);
			k = 0;
			for (i = 0;i < 4; i++)
			{
				 for (j = 0; j < 4; j++)
				 {
						transformation_editor_transformation_matrix[i][j] = values[k];
						k++;
				 }
			}
			DEALLOCATE(values);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "transformation_editor_wx_set_transformation. Cannot allocate sufficient memory");
	 }

}

void Transformation_editor::transformation_editor_wx_spin_button_change_value(
	 wxSpinButton *temp_object, int flag)
/*******************************************************************************
LAST MODIFIED : 4 March 2008

DESCRIPTION :
Must only call this function from OnTransformationEditor_spin_button_up function.
==============================================================================*/
{
	 double increment = (double)flag * pow((double)10.0, rate_of_change);
	 if (temp_object == Transformation_editor_wx_position_spin_button_1)
	 {
			global_position.data[0]+= increment;
	 }
	 else if (temp_object == Transformation_editor_wx_position_spin_button_2)
	 {
			global_position.data[1]+= increment;
	 }
	 else if (temp_object == Transformation_editor_wx_position_spin_button_3)
	 {
			global_position.data[2]+= increment;
	 }
	 if (direction_system_index == 0)
	 {
			if (temp_object == Transformation_editor_wx_direction_spin_button_1)
			{
				 global_direction.data[0]+= increment;
			}
			else if (temp_object == Transformation_editor_wx_direction_spin_button_2)
			{
				 global_direction.data[1]+= increment;
			}
			else if (temp_object == Transformation_editor_wx_direction_spin_button_3)
			{
				 global_direction.data[2]+= increment;
			}
	 }
	 else if (direction_system_index == 1)
	 {
			int i,j;
			Gmatrix gmatrix;
			for (i = 0; i < GMATRIX_SIZE; i++)
			{
				 for (j = 0; j < GMATRIX_SIZE; j++)
				 {
						gmatrix.data[i][j] = (transformation_editor_transformation_matrix)[i][j];
				 }
			}
			matrix_euler(&gmatrix, &global_direction);
// 			if (temp_object == Transformation_editor_wx_direction_spin_button_1)
// 			{
// 				 global_quat[1]+= increment;
// 			}
// 			else if (temp_object == Transformation_editor_wx_direction_spin_button_2)
// 			{
// 				 global_quat[2]+= increment;
// 			}
// 			else if (temp_object == Transformation_editor_wx_direction_spin_button_3)
// 			{
// 				 global_quat[3]+= increment;
// 			}
// 			else if (temp_object == Transformation_editor_wx_direction_spin_button_4)
// 			{
// 				 global_quat[0] += increment;
// 			}
// 			transformation_editor_quaternion_to_gtmatrix();
	 }

	 position_direction_to_transformation_matrix(
			&global_position, &global_direction,&(transformation_editor_transformation_matrix));
	 scale_factor_to_transformation_matrix(global_scale_factor,
			&(transformation_editor_transformation_matrix));	 
	 if (gtMatrix_is_identity(&(transformation_editor_transformation_matrix)))
	 {
			transformation_editor_wx_set_transformation(&(transformation_editor_transformation_matrix));
	 }
	 ApplyTransformation();
}

void Transformation_editor::OnTransformationEditor_spin_button_up(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 4 March 2008

DESCRIPTION :
Process spinbutton event. Eventhandler somehow does not return any information of the colour
editor class although this function is inside the class. Therefore, I store the colour
editor inside the control object and get the control object whenever an event is
provoked then use this colour editor to do the settings.
==============================================================================*/
{
	 ENTER(Transformation_editor::OnTransformationEditorSliderChanged);

	 wxSpinButton *temp_object = (wxSpinButton *)event.GetEventObject();
	 Transformation_editor *temp_editor = (Transformation_editor *)temp_object->GetClientData();
	 temp_editor->transformation_editor_wx_spin_button_change_value(temp_object, 1);
	 LEAVE;
}

void Transformation_editor::OnTransformationEditor_spin_button_down(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 4 March 2008

DESCRIPTION :
Process scroll event. Eventhandler somehow does not return any information of the colour
editor class although this function is inside the class. Therefore, I store the colour
editor inside the control object and get the control object whenever an event is
provoked then use this colour editor to do the settings.
==============================================================================*/
{
	 ENTER(Transformation_editor::OnTransformationEditorSliderChanged);

	 wxSpinButton *temp_object = (wxSpinButton *)event.GetEventObject();
	 Transformation_editor *temp_editor = (Transformation_editor *)temp_object->GetClientData();
	 temp_editor->transformation_editor_wx_spin_button_change_value(temp_object, -1);
	 LEAVE;
}

void Transformation_editor::ApplyTransformation()
/*******************************************************************************
LAST MODIFIED : 5 March 2008

DESCRIPTION :
Apply Transformation from the transformation editor.
==============================================================================*/
{
	 ENTER(Transformation_editor::OnTransformationEditorSliderChanged);
	 if (current_object)
	 {
			if (*auto_apply_flag)
			{
				 Scene_object_set_transformation(current_object,
						&transformation_editor_transformation_matrix);
			}
			else
			{
				 transformation_editor_wx_set_transformation(
						&(transformation_editor_transformation_matrix));
			}
	 }

	 LEAVE;
}

void Transformation_editor::transformation_editor_wx_direction_system_choice_changed()
/*******************************************************************************
LAST MODIFIED : 6 March 2008
Process choice selected event.
==============================================================================*/
{
	 int current_choice;

	 ENTER(Transformation_editor::transformation_editor_wx_direction_system_choice_changed);

	 current_choice = Transformation_editor_wx_direction_coord_system_choice->GetSelection();
	 if (direction_system_index != current_choice)
	 {
			direction_system_index = current_choice;
			Transformation_editor_wx_direction_label_3->SetLabel(
				 transformation_editor_direction_text[current_choice][0]);
			Transformation_editor_wx_direction_label_4->SetLabel(
				 transformation_editor_direction_text[current_choice][1]);
			Transformation_editor_wx_direction_label_5->SetLabel(
				 transformation_editor_direction_text[current_choice][2]);
						
			if (direction_system_index == 0)
			{
				 Transformation_editor_wx_direction_label_6->Hide();
				 Transformation_editor_wx_direction_text_ctrl_4->Hide();
				 Transformation_editor_wx_direction_spin_button_4->Hide();
				 direction_sizer_13_staticbox->Hide();
				 Transformation_editor_wx_direction_text_ctrl_1->SetEditable(1);
				 Transformation_editor_wx_direction_text_ctrl_2->SetEditable(1);
				 Transformation_editor_wx_direction_text_ctrl_3->SetEditable(1);
			}
			else if (direction_system_index == 1)
			{
				 Transformation_editor_wx_direction_label_6->Show();
				 Transformation_editor_wx_direction_text_ctrl_4->Show();
				 Transformation_editor_wx_direction_spin_button_4->Show();
				 direction_sizer_13_staticbox->Show();
				 Transformation_editor_wx_direction_text_ctrl_1->SetEditable(0);
				 Transformation_editor_wx_direction_text_ctrl_2->SetEditable(0);
				 Transformation_editor_wx_direction_text_ctrl_3->SetEditable(0);
				 Transformation_editor_wx_direction_text_ctrl_4->SetEditable(0);
			}
			transformation_editor_panel->Layout();
			transformation_editor_wx_set_transformation(
				 &(transformation_editor_transformation_matrix));
	 }
	 
	 LEAVE;
}

void Transformation_editor::OnTransformationEditorDirectionSystemChoice(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 6 March 2008

DESCRIPTION :
Process choice selected event. Eventhandler somehow does not return any information of the colour
editor class although this function is inside the class. Therefore, I store the colour
editor inside the control object and get the control object whenever an event is
provoked then use this colour editor to do the settings.
==============================================================================*/
{
	 ENTER(Transformation_editor::OnTransformationEditorSliderChanged);

	 wxChoice *temp_object = (wxChoice *)event.GetEventObject();
	 Transformation_editor *temp_editor = (Transformation_editor *)temp_object->GetClientData();
	 temp_editor->transformation_editor_wx_direction_system_choice_changed();

	 LEAVE;
}

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
	 transformation_editor_panel = parent;
	 current_object = scene_object;
	 auto_apply_flag = auto_apply;
	 rate_of_change = 0;
	 position_sizer_7_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 position_sizer_8_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_6_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_7_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 direction_sizer_8_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
	 position_sizer_6_staticbox = new wxStaticBox(transformation_editor_panel, -1, wxEmptyString);
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
    Transformation_editor_wx_direction_save_button = new wxButton(transformation_editor_panel, wxID_ANY, wxT("Save"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_reset_button = new wxButton(transformation_editor_panel, wxID_ANY, wxT("Reset"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_lock_data_toggle_button = new wxToggleButton(transformation_editor_panel, wxID_ANY, wxT("Lock Data"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_link_resolution_toggle_button = new wxToggleButton(transformation_editor_panel, wxID_ANY, wxT("Link Resolution"), wxPoint(-1,-1), wxSize(-1,25));
    Transformation_editor_wx_direction_label_1 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("Coord System"), wxPoint(-1,-1), wxSize(-1,20));
    const wxString Transformation_editor_wx_direction_coord_system_choice_choices[] = {
			 wxT("Euler")
    };
    Transformation_editor_wx_direction_coord_system_choice = new wxChoice(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,25), 1, Transformation_editor_wx_direction_coord_system_choice_choices, 0);
    Transformation_editor_wx_direction_label_3 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  Azimuth"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_1 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_1 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_direction_label_4 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  Elevation"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_2 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString,wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_2 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);
    Transformation_editor_wx_direction_label_5 = new wxStaticText(transformation_editor_panel, wxID_ANY, wxT("  Roll"), wxPoint(-1,-1), wxSize(-1,20));
    Transformation_editor_wx_direction_text_ctrl_3 = new wxTextCtrl(transformation_editor_panel, wxID_ANY, wxEmptyString, wxPoint(-1,-1), wxSize(-1,20), wxTE_PROCESS_ENTER);
		Transformation_editor_wx_direction_spin_button_3 = new wxSpinButton(transformation_editor_panel, wxID_ANY, wxPoint(-1,-1), wxSize(-1,20), wxSP_ARROW_KEYS);

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
	 Transformation_editor_wx_direction_coord_system_choice->Hide();
	 Transformation_editor_wx_direction_save_button->Hide();
	 Transformation_editor_wx_direction_reset_button->Hide();
	 Transformation_editor_wx_direction_lock_data_toggle_button->Hide();
	 Transformation_editor_wx_direction_link_resolution_toggle_button->Hide();
	 Transformation_editor_wx_direction_label_1->Hide();
	 Transformation_editor_wx_direction_coord_system_choice->Hide();
	 Transformation_editor_wx_global_choice->SetSelection(0);
	 Transformation_editor_wx_position_coord_system_choice->SetSelection(0);
	 Transformation_editor_wx_direction_coord_system_choice->SetSelection(0);
	 Transformation_editor_wx_position_spin_button_1->SetRange(-100000,100000);
	 Transformation_editor_wx_position_spin_button_2->SetRange(-100000,100000);
	 Transformation_editor_wx_position_spin_button_3->SetRange(-100000,100000);
	 Transformation_editor_wx_direction_spin_button_1->SetRange(-100000,100000);
	 Transformation_editor_wx_direction_spin_button_2->SetRange(-100000,100000);
	 Transformation_editor_wx_direction_spin_button_3->SetRange(-100000,100000);
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
		Transformation_editor_wx_direction_text_ctrl_1->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_1->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_direction_text_ctrl_2->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_2->Connect(wxEVT_COMMAND_TEXT_ENTER, 
			 wxCommandEventHandler(Transformation_editor::OnTransformationEditorTextEntered));
		Transformation_editor_wx_direction_text_ctrl_3->SetClientData((void *)this);
		Transformation_editor_wx_direction_text_ctrl_3->Connect(wxEVT_COMMAND_TEXT_ENTER, 
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
	 position_sizer_1->Add(position_sizer_5, 1, wxEXPAND, 0);
	 position_sizer_1->Add(position_sizer_9, 0, wxEXPAND, 0);
	 Transformation_editor_wx_top_sizer->Add(position_sizer_1, 1, wxEXPAND, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_save_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_reset_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_lock_data_toggle_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_2->Add(Transformation_editor_wx_direction_link_resolution_toggle_button, 0, wxALIGN_BOTTOM, 0);
	 direction_sizer_3->Add(Transformation_editor_wx_direction_label_1, 0, 0, 0);
	 direction_sizer_3->Add(Transformation_editor_wx_direction_coord_system_choice, 0, 0, 0);
	 direction_sizer_2->Add(direction_sizer_3, 1, wxEXPAND, 0);
	 direction_sizer_1->Add(direction_sizer_2, 0, wxEXPAND, 0);
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
	 direction_sizer_1->Add(direction_sizer_5, 1, wxEXPAND, 0);
	 direction_sizer_1->Add(direction_sizer_9, 0, wxEXPAND, 0);
	 Transformation_editor_wx_top_sizer->Add(direction_sizer_1, 1, wxEXPAND, 0);
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
	 char temp_str[20];
	 int i, j, return_code;
	 Gmatrix gmatrix;
	 
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

			/* convert the matrix to Euler angles */
			matrix_euler(&gmatrix, &global_direction);
			sprintf(temp_str, DOF3_NUM_FORMAT, global_direction.data[0]);
			Transformation_editor_wx_direction_text_ctrl_1->ChangeValue(temp_str);
			sprintf(temp_str, DOF3_NUM_FORMAT, global_direction.data[1]);
			Transformation_editor_wx_direction_text_ctrl_2->ChangeValue(temp_str);
			sprintf(temp_str, DOF3_NUM_FORMAT, global_direction.data[2]);
			Transformation_editor_wx_direction_text_ctrl_3->ChangeValue(temp_str);


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

			/*???RC later check transformation by direction and position is
				equivalent to 4x4 matrix; otherwise warn user */

			/* 2. put the direction and position values into widgets */
			
// 			dof3_set_data(transformation_editor->position_widget, DOF3_DATA,
// 				(void *)&position);
// 			dof3_set_data(transformation_editor->direction_widget, DOF3_DATA,
// 				(void *)&direction);
			/* we must change to the global coord system */
// 			transformation_editor->parent_coordinate = global_coordinate_ptr;
// 			coord_set_data(transformation_editor->coord_widget, COORD_COORD_DATA,
// 				transformation_editor->parent_coordinate);

 			/* determine by back calculation whether the transformation is purely
				 a position/rotation. If not, display the warning form */
// 			if (position_direction_to_transformation_matrix(
// 				&position, &direction, &resolved_transformation_matrix) &&
// 				gtMatrix_match_with_tolerance(transformation_matrix,
// 					&resolved_transformation_matrix, /*tolerance*/1.0E-6))
// 			{
// 				XtUnmanageChild(transformation_editor->warning_form);
// 			}
// 			else
// 			{
// 				XtManageChild(transformation_editor->warning_form);
// 			}

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
	 char *text;

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
	 text = (char*)Transformation_editor_wx_position_text_ctrl_3->GetValue().mb_str(wxConvUTF8);
	 if (text)
	 {
			sscanf(text,"%lf",&temp);
			position.data[2] = temp;
	 }
	 position_direction_to_transformation_matrix(
			&position, &direction,&(transformation_editor_transformation_matrix));
	 if (*auto_apply_flag)
	 {
			ApplyTransformation();
	 }
	 else
	 {
			transformation_editor_wx_set_transformation(&(transformation_editor_transformation_matrix));
	 }
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

void Transformation_editor::transformation_editor_wx_spin_button_change_value(
	 wxSpinButton *temp_object, int flag)
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
	 else if (temp_object == Transformation_editor_wx_direction_spin_button_1)
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
	 position_direction_to_transformation_matrix(
			&global_position, &global_direction,&(transformation_editor_transformation_matrix));
	 if (*auto_apply_flag)
	 {
			ApplyTransformation();
	 }
	 transformation_editor_wx_set_transformation(&(transformation_editor_transformation_matrix));
	 
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

	 Scene_object_set_transformation(current_object,
			&transformation_editor_transformation_matrix);

	 LEAVE;
}


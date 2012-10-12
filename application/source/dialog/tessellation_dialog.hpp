/***************************************************************************//**
 * tessellation_dialog.hpp
 *
 * Dialog for describing editing tessellation
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#include <graphics/tessellation.hpp>
#include <wx/wx.h>
#include <map>

#ifndef TESSELATION_DIALOG_HPP
#define TESSELATION_DIALOG_HPP

class TessellationItem : public wxPanel {

public:

 	TessellationItem(wxWindow* parent, MANAGER(Cmiss_tessellation) *tessellation_manager_in,
 		Cmiss_tessellation *tessellation_in);
	void update_global();

private:

	Cmiss_tessellation *tessellation;
	int labelChanged, refinementChanged, divisionsChanged;
  MANAGER(Cmiss_tessellation) *tessellation_manager;
 	void do_layout();
 	void set_callback();
	void update_divisions_string_for_dialog();
	void update_refinement_string_for_dialog();
	void OnTessellationTextEntered(wxCommandEvent& event);
	void OnTessellationApplyPressed(wxCommandEvent& event);

protected:
	wxTextCtrl *tessellationLabel, *refinementTextCtrl, *divisionsTextCtrl;
	wxButton *applyButton;
};

class TessellationDialog: public wxDialog {
public:

    TessellationDialog(struct Cmiss_graphics_module *graphics_module_in, wxWindow* parent, int id,
    	const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize);
    int add_managed_object(Cmiss_tessellation *tessellation);
    void manager_callback(struct MANAGER_MESSAGE(Cmiss_tessellation) *message);
    virtual ~TessellationDialog() {
    	if (tessellation_manager_callback_id)
    	{
    		MANAGER_DEREGISTER(Cmiss_tessellation)(
    			tessellation_manager_callback_id,	tessellation_manager);
    	}
    }

private:
    struct Cmiss_graphics_module *graphics_module;
    MANAGER(Cmiss_tessellation) *tessellation_manager;
    void *tessellation_manager_callback_id;
    void set_properties();
    void do_layout();
    void create_managed_objects_table();
		void OnTessellationDialogAddNewPressed(wxCommandEvent & event);
    std::map<Cmiss_tessellation *, TessellationItem *> itemMap;

protected:
    wxStaticBox* sizer_1_staticbox;
    wxStaticText* label_1;
    wxStaticText* label_2;
    wxStaticText* label_3;
    wxStaticText* label_4;
    wxButton *addNewButton;
    wxScrolledWindow* TessellationItemsPanel;
};
#endif // TESSELATION_DIALOG_HPP

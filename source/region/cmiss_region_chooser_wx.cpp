/*******************************************************************************
FILE : cmiss_region_chooser_wx.cpp

LAST MODIFIED : 22 February 2007

DESCRIPTION :
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
#include "region/cmiss_region_chooser_wx.hpp"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}

wxRegionChooser::wxRegionChooser(wxWindow *parent, 
	Cmiss_region *root_region, const char *initial_path) :
	wxChoice(parent, /*id*/-1, wxPoint(0,0), wxSize(-1,-1)),
	root_region(ACCESS(Cmiss_region)(root_region))
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
==============================================================================*/
{
	build_main_menu(root_region, initial_path);
	
	Connect(wxEVT_COMMAND_CHOICE_SELECTED,
		wxCommandEventHandler(wxRegionChooser::OnChoiceSelected));
	
	Cmiss_region_add_callback(root_region,
		wxRegionChooser::RegionChange, this);

	wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
	sizer->Add(this,
		wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
	parent->SetSizer(sizer);
	
	Show();
	
}

wxRegionChooser::~wxRegionChooser()
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
==============================================================================*/
{
	Cmiss_region_remove_callback(root_region,
		wxRegionChooser::RegionChange, this);
	DEACCESS(Cmiss_region)(&root_region);
}

char *wxRegionChooser::get_path()
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Gets <path> of chosen region in the <chooser>.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/
{
	char *return_name;
	const char *item_name = GetString(GetSelection());
	// Trim the first character which is the root region symbol
	return_name = duplicate_string(item_name);
	return (return_name);
}

Cmiss_region *wxRegionChooser::get_region()
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/
{
	Cmiss_region *child_region = NULL;
	Cmiss_region_get_region_from_path(root_region,
		GetString(GetSelection()), &child_region);
	return (child_region);
}

int wxRegionChooser::set_path(const char *path)
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/
{
	unsigned int found = 0, i;

	for (i = 0 ; !found && (i < GetCount()) ; i++)
	{
		if (!strcmp(path, GetString(i)))
		{
			found = 1;
			SetSelection(i);
		}
	}
	if (!found)
	{
		display_message(ERROR_MESSAGE, "wxRegionChooser::set_path.  "
			"Child region not found for path: %s", path);
	}
	return (found);
}

int wxRegionChooser::append_children(Cmiss_region *current_region,
	const char *current_path, const char *initial_path)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/
{
	char *child_name, *child_path;
	Cmiss_region *child_region;
	int i, number_of_children;

	Cmiss_region_get_number_of_child_regions(current_region,
		&number_of_children);
	for (i = 0 ; i < number_of_children ; i++)
	{
		Cmiss_region_get_child_region_name(current_region, i, &child_name);
		
		ALLOCATE(child_path, char, strlen(current_path) + strlen(child_name) + 2);
		sprintf(child_path, "%s%c%s", current_path, CMISS_REGION_PATH_SEPARATOR_CHAR,
			child_name);
		Append(child_path);
		if (!strcmp(child_path, initial_path))
		{
			SetSelection(GetCount() - 1);
		}

		//Recurse
		child_region = Cmiss_region_get_child_region(current_region, i);
		append_children(child_region, child_path, initial_path);

		DEALLOCATE(child_name);
		DEALLOCATE(child_path);
	}
	return 1;
}

int wxRegionChooser::build_main_menu(Cmiss_region *root_region,
	const char *initial_path)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Updates the menu in the wxRegionChooser to reflect the heirarchy of the <root_region>.
If <initial_path> matches the pathname of one of the children then this object
is selected.
==============================================================================*/
{
	char *root_path;
	Clear();

	Cmiss_region_get_root_region_path(&root_path);
	Append(root_path);
	if (!strcmp(root_path, initial_path))
	{
		SetSelection(GetCount() - 1);
	}
	append_children(root_region, "", initial_path);
	DEALLOCATE(root_path);

	if (wxNOT_FOUND == GetSelection())
	{
		SetSelection(0);
		notify_callback();
	}

	return 1;
}

void wxRegionChooser::RegionChange(struct Cmiss_region *root_region,
	struct Cmiss_region_changes *region_changes, void *region_chooser_void)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/
{
	wxRegionChooser *region_chooser;

	if (region_chooser = static_cast<wxRegionChooser *>(region_chooser_void))
	{
		 char *temp;
		 temp = region_chooser->get_path();
		region_chooser->build_main_menu(root_region, temp);
		DEALLOCATE(temp);
	}
}

void wxRegionChooser::OnChoiceSelected(wxCommandEvent& Event)
{
	notify_callback();
}

int wxRegionChooser::notify_callback()
{
	if (callback)
	{
		callback->callback_function(get_region());
	}
	return (1);
}

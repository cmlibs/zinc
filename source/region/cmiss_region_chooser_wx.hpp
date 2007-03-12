/*******************************************************************************
FILE : cmiss_region_chooser_wx.hpp

LAST MODIFIED : 21 February 2007

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
#if !defined (CMISS_REGION_CHOOSER_WX_HPP)
#define CMISS_REGION_CHOOSER_WX_HPP

#include "wx/wx.h"
#include "general/callback_class.hpp"

struct Cmiss_region;

class wxRegionChooser : public wxChoice
{
private:
	Cmiss_region *root_region;
	Callback_base<Cmiss_region*> *callback;

public:
	wxRegionChooser(wxWindow *parent, 
		Cmiss_region *root_region, const char *initial_path);
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
==============================================================================*/

	~wxRegionChooser();
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/

	int set_callback(Callback_base<Cmiss_region*> *callback_object)
	{
		callback = callback_object;
		return (1);
	}

	char *get_path();
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Gets <path> of chosen region in the <chooser>.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

	Cmiss_region *get_region();
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/

	int set_path(const char *path);
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/

private:
	void OnChoiceSelected(wxCommandEvent& Event);
	
	int notify_callback();

	int build_main_menu(Cmiss_region *root_region,
		const char *initial_path);
	int append_children(Cmiss_region *current_region,
		const char *current_path, const char *initial_path);

	static void RegionChange(struct Cmiss_region *root_region,
		struct Cmiss_region_changes *region_changes, void *region_chooser_void);
};
#endif /* !defined (CMISS_REGION_CHOOSER_H) */

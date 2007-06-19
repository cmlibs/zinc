/*******************************************************************************
FILE : node_viewer.h

LAST MODIFIED : 6 June 2007

DESCRIPTION :
For wxWidgets only, Dialog for selecting an element point, viewing and editing its fields and
applying changes. Works with Element_point_ranges_selection to display the last
selected element point, or set it if entered in this dialog.
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
#if !defined (ELEMENT_POINT_VIEWER_H)
#define ELEMENT_POINT_VIEWER_H

#include "general/callback.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "graphics/element_point_ranges.h"
#include "selection/element_point_ranges_selection.h"
#include "time/time.h"

/*
Global Types
------------
*/

struct Element_Point_viewer;
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Contains all the information carried by the element_point_viewer widget.
The contents of this object are private.
==============================================================================*/

/*
Global Functions
----------------
*/

struct Element_point_viewer *CREATE(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Creates a dialog for choosing element points and displaying and editing their
fields.
==============================================================================*/

int DESTROY(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address);
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION:
Destroys the Element_point_viewer. See also Element_point_viewer_close_CB.
==============================================================================*/

int Element_point_viewer_bring_window_to_front(
	struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Pops the window for <element_point_viewer> to the front of those visible.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_VIEWER_H) */

/*******************************************************************************
FILE : element_point_viewer.h

LAST MODIFIED : 24 May 2000

DESCRIPTION :
Dialog for selecting an element point, viewing and editing its fields and
applying changes. Works with Element_point_ranges_selection to display the last
selected element point, or set it if entered in this dialog.
==============================================================================*/
#if !defined (ELEMENT_POINT_VIEWER_H)
#define ELEMENT_POINT_VIEWER_H

#include "general/callback.h"
#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"
#include "selection/element_point_ranges_selection.h"

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
	struct MANAGER(FE_element) *element_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_field) *fe_field_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 May 2000

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

/*******************************************************************************
FILE : element_selection.h

LAST MODIFIED : 3 April 2003

DESCRIPTION :
Global store of selected elements for group actions and highlighting.
==============================================================================*/
#if !defined (ELEMENT_SELECTION_H)
#define ELEMENT_SELECTION_H

#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/callback.h"
#include "general/object.h"

/*
Global types
------------
*/

struct FE_element_selection;
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Global store of selected elements for group actions and highlighting.
The contents of this object are private.
==============================================================================*/

struct FE_element_selection_changes
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Data broadcast with callbacks from <FE_element_selection> describing the
changes.
==============================================================================*/
{
	/* list of elements selected since the last update callback - includes those
     already selected; excludes those subsequently unselected */
	struct LIST(FE_element) *newly_selected_element_list;
	/* list of elements unselected since the last update callback - excludes both
		 those already unselected and those subsequently reselected */
	struct LIST(FE_element) *newly_unselected_element_list;
}; /* struct FE_element_selection_changes */

DECLARE_CMISS_CALLBACK_TYPES(FE_element_selection_change, \
	struct FE_element_selection *,struct FE_element_selection_changes *);

/*
Global functions
----------------
*/

struct FE_element_selection *CREATE(FE_element_selection)(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Creates the global store of selected elements for group actions and
highlighting.
==============================================================================*/

int DESTROY(FE_element_selection)(
	struct FE_element_selection **element_selection_address);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Destroys the FE_element_selection.
==============================================================================*/

int FE_element_selection_add_callback(
	struct FE_element_selection *element_selection,
	CMISS_CALLBACK_FUNCTION(FE_element_selection_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Adds a callback to <element_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a
struct FE_element_selection, a struct FE_element_selection_changes and the
void *user_data.
==============================================================================*/

int FE_element_selection_remove_callback(
	struct FE_element_selection *element_selection,
	CMISS_CALLBACK_FUNCTION(FE_element_selection_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<element_selection>.
==============================================================================*/

int FE_element_selection_begin_cache(
	struct FE_element_selection *element_selection);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Starts caching for changes in the <element_selection>. No update callbacks will
occur until FE_element_selection_end_cache is called.
==============================================================================*/

int FE_element_selection_end_cache(
	struct FE_element_selection *element_selection);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Ends caching for selected list changes in the <selection>. Calls
FE_element_selection_update to inform clients of the changes that took place
while caching was on.
==============================================================================*/

int FE_element_selection_clear(struct FE_element_selection *element_selection);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Clears all the selected elements in the <element_selection>.
==============================================================================*/

int FE_element_selection_select_element(
	struct FE_element_selection *element_selection,struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Ensures <element> is in <element_selection>. Even if it is already selected,
<element> is added to the newly_selected_element_list and removed from the
newly_unselected_element_list if it is currently there.
Calls FE_element_selection_update.
==============================================================================*/

int FE_element_select_in_FE_element_selection(struct FE_element *element,
	void *element_selection_void);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
FE_element iterator version of FE_element_selection_select_element.
==============================================================================*/

int FE_element_selection_unselect_element(
	struct FE_element_selection *element_selection,struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
If <element> is in <element_selection> it is removed and added to the
newly_unselected_element_list. Note it is added to this list if not currently
selected. Also ensures it is removed from the newly_selected_element_list if it
is currently there. Calls FE_element_selection_update.
==============================================================================*/

int FE_element_unselect_in_FE_element_selection(struct FE_element *element,
	void *element_selection_void);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
FE_element iterator version of FE_element_selection_unselect_element.
==============================================================================*/

struct LIST(FE_element) *FE_element_selection_get_element_list(
	struct FE_element_selection *element_selection);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Returns the list containing the elements currently in <element_selection>.
RETURNED LIST IS NOT TO BE MODIFIED.
==============================================================================*/

int FE_element_selection_is_element_selected(
	struct FE_element_selection *element_selection,struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Returns true if the <element> is in the <element_selection>.
==============================================================================*/

#endif /* !defined (ELEMENT_SELECTION_H) */

/*******************************************************************************
FILE : element_creator.h

LAST MODIFIED : 17 January 2003

DESCRIPTION :
Dialog for choosing the type of element constructed in response to node
selections. Elements are created in this way while dialog is open.
==============================================================================*/
#if !defined (ELEMENT_CREATOR_H)
#define ELEMENT_CREATOR_H

#include "finite_element/finite_element.h"
#include "graphics/scene.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"

/*
Global types
------------
*/

struct Element_creator;

/*
Global functions
----------------
*/

struct Element_creator *CREATE(Element_creator)(
	struct Element_creator **element_creator_address,
	struct Cmiss_region *root_region, char *initial_region_path,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Creates a Element_creator, giving it the element_manager to put new
elements in, and the node_manager for new nodes, and the fe_field_manager to
enable the creation of a coordinate field.
==============================================================================*/

int DESTROY(Element_creator)(
	struct Element_creator **element_creator_address);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Deaccesses objects and frees memory used by the Element_creator at
<*element_creator_address>.
==============================================================================*/

struct FE_field *Element_creator_get_coordinate_field(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Returns the coordinate field interpolated by elements created with
<element_creator>.
==============================================================================*/

int Element_creator_set_coordinate_field(
	struct Element_creator *element_creator,struct FE_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Sets the coordinate field interpolated by elements created with
<element_creator>.
==============================================================================*/

int Element_creator_get_create_enabled(struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Element_creator_set_create_enabled(struct Element_creator *element_creator,
	int create_enabled);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets flag controlling whether elements are created in response to
node selection.
==============================================================================*/

int Element_creator_get_element_dimension(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Returns the dimension of elements to be created by the <element_creator>.
==============================================================================*/

int Element_creator_set_element_dimension(
	struct Element_creator *element_creator,int element_dimension);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Sets the <element_dimension> of any subsequent elements to be created.
==============================================================================*/

int Element_creator_get_region_path(struct Element_creator *element_creator,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 10 January 2003

DESCRIPTION :
Returns in <path_address> the path to the Cmiss_region where elements created by
the <element_creator> are put.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

int Element_creator_set_region_path(struct Element_creator *element_creator,
	char *path);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Sets the <path> to the region/FE_region where elements created by
<element_creator> are placed.
The <element_creator> assumes its nodes are to come from the same FE_region.
==============================================================================*/

int Element_creator_bring_window_to_front(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Pops the window for <element_creator> to the front of those visible.
??? De-iconify as well?
==============================================================================*/

#endif /* !defined (ELEMENT_CREATOR_H) */



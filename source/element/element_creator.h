/*******************************************************************************
FILE : element_creator.h

LAST MODIFIED : 27 June 2000

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
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 26 June 2000

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

struct GROUP(FE_element) *Element_creator_get_element_group(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Returns the group where elements created by the <element_creator> are put.
???RC Eventually this will get the Region.
==============================================================================*/

int Element_creator_set_element_group(struct Element_creator *element_creator,
	struct GROUP(FE_element) *element_group);
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Sets the <element_group> where elements created by <element_creator> are placed.
The <element_creator> will assume its nodes are to be in the node group of the
same name, which enables the nodes and elements to be exported as a group.
???RC Eventually this will set the Region.
==============================================================================*/

int Element_creator_bring_window_to_front(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Pops the window for <element_creator> to the front of those visible.
==============================================================================*/

#endif /* !defined (ELEMENT_CREATOR_H) */



/*******************************************************************************
FILE : graphical_element_creator.h

LAST MODIFIED : 21 October 1999

DESCRIPTION :
Mouse controlled element creator.
???DB.  Includes ?
==============================================================================*/
#if !defined (GRAPHICAL_ELEMENT_CREATOR_H)
#define GRAPHICAL_ELEMENT_CREATOR_H

/*
Global types
------------
*/
struct Graphical_element_creator;

/*
Global functions
----------------
*/
struct Graphical_element_creator *CREATE(Graphical_element_creator)(
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager);
/*******************************************************************************
LAST MODIFIED : 26 March 1999

DESCRIPTION :
Creates a Graphical_element_creator, giving it the element_manager to put new
elements in, and the node_manager for new nodes, and the fe_field_manager to
enable the creation of a coordinate field.
==============================================================================*/

int DESTROY(Graphical_element_creator)(
	struct Graphical_element_creator **element_creator_address);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Deaccesses objects and frees memory used by the Graphical_element_creator at
<*element_creator_address>.
==============================================================================*/

int Graphical_element_creator_can_create_nodes(
	struct Graphical_element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Returns true if the <element_creator> is allowed to create nodes.
==============================================================================*/

int Graphical_element_creator_set_create_nodes(
	struct Graphical_element_creator *element_creator,int create_nodes);
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Sets whether new nodes can be created by the <element_creator>, where
<create_nodes>=0 means it can not, any other value means it can.
==============================================================================*/

int Graphical_element_creator_get_element_dimension(
	struct Graphical_element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Returns the dimension of elements to be created by the <element_creator>.
==============================================================================*/

int Graphical_element_creator_set_element_dimension(
	struct Graphical_element_creator *element_creator,int element_dimension);
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Sets the <element_dimension> of any subsequent elements to be created.
==============================================================================*/

int Graphical_element_creator_get_groups(
	struct Graphical_element_creator *element_creator,
	struct GROUP(FE_element) **element_group,
	struct GROUP(FE_node) **node_group);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Returns the current <element_group> and <node_group> where elements and nodes
created by the <element_creator> are placed.
???RC Eventually this will get the Region.
==============================================================================*/

int Graphical_element_creator_set_groups(
	struct Graphical_element_creator *element_creator,
	struct GROUP(FE_element) *element_group,
	struct GROUP(FE_node) *node_group);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Sets the <element_group> and <node_group> where elements and nodes created by
<element_creator> are placed. Either or both groups may be omitted, however,
it pays to supply a node_group and element_group of the same name with a
corresponding GT_element_group displaying node_points, lines and possibly
surfaces - that way the user will automatically see the new objects. Also, it
enables the user to export the nodes/elements as a group.
???RC Eventually this will set the Region.
==============================================================================*/

int Graphical_element_creator_begin_create_element(
	struct Graphical_element_creator *element_creator,struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Stores current scene input callback for <scene> then diverts it to the
<element_creator>. The <element_creator> then takes as many button click events
as there are nodes in the current element type, picking an existing node or
creating a new one each time. When all the nodes are defined, an element is
constructed between them - with corresponding face and line elements - and
the scene input callback is restored to its previous setting.
???RC currently only supports bilinear elements. Later add other functions to
establish the type of elements that will be added by this function.
==============================================================================*/

#endif /* !defined (GRAPHICAL_ELEMENT_CREATOR_H) */



/*******************************************************************************
FILE : graphical_element_creator.h

LAST MODIFIED : 20 January 2000

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

enum Mesh_editor_create_mode
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Must keep additions and changes in synch with functions:
- Mesh_editor_create_mode_string
- Mesh_editor_create_mode_get_valid_strings
- Mesh_editor_create_mode_from_string
==============================================================================*/
{
	MESH_EDITOR_CREATE_MODE_INVALID,
	MESH_EDITOR_CREATE_MODE_BEFORE_FIRST,
	MESH_EDITOR_CREATE_ELEMENTS,
	MESH_EDITOR_CREATE_ONLY_NODES,
	MESH_EDITOR_CREATE_NODES_AND_ELEMENTS,
	MESH_EDITOR_NO_CREATE,
	MESH_EDITOR_CREATE_MODE_AFTER_LAST
};

/*
Global functions
----------------
*/
char *Mesh_editor_create_mode_string(
	enum Mesh_editor_create_mode mesh_editor_create_mode);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the mesh_editor_create_mode, eg.
MESH_EDITOR_CREATE_ELEMENTS = "create_elements".
This string should match the command used to create that mode.
The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Mesh_editor_create_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Mesh_editor_create_modes - obtained from function
Mesh_editor_create_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Mesh_editor_create_mode Mesh_editor_create_mode_from_string(
	char *mesh_editor_create_mode_string);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Returns the <Mesh_editor_create_mode> described by
<mesh_editor_create_mode_string>.
==============================================================================*/

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

int Graphical_element_creator_get_mesh_editor_create_mode(
	struct Graphical_element_creator *element_creator,
	enum Mesh_editor_create_mode *mesh_editor_create_mode);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
==============================================================================*/

int Graphical_element_creator_set_mesh_editor_create_mode(
	struct Graphical_element_creator *element_creator,
	enum Mesh_editor_create_mode mesh_editor_create_mode,struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
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

#endif /* !defined (GRAPHICAL_ELEMENT_CREATOR_H) */



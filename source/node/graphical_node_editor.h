/*******************************************************************************
FILE : graphical_node_editor.h

LAST MODIFIED : 20 July 1999

DESCRIPTION :
Functions mouse controlled node editing to a Scene.
???DB.  Includes ?
==============================================================================*/
#if !defined (GRAPHICAL_NODE_EDITOR_H)
#define GRAPHICAL_NODE_EDITOR_H

/*
Global types
------------
*/
struct Graphical_node_editor;
/*******************************************************************************
LAST MODIFIED : 19 July 1999

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/
struct Graphical_node_editor *CREATE(Graphical_node_editor)(
	struct Scene *scene,struct GROUP(FE_element) *active_element_group,
	struct GROUP(FE_node) *active_data_group,
	struct GROUP(FE_node) *active_node_group,
	struct MANAGER(FE_node) *node_manager);
/*******************************************************************************
LAST MODIFIED : 20 July 1999

DESCRIPTION :
Creates the structure that needs to be passed to the GNE_scene_input_callback.
==============================================================================*/

int DESTROY(Graphical_node_editor)(
	struct Graphical_node_editor **node_editor_address);
/*******************************************************************************
LAST MODIFIED : 20 July 1999

DESCRIPTION :
Frees and deaccesses objects in the <node_editor> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (GRAPHICAL_NODE_EDITOR_H) */

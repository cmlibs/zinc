/*******************************************************************************
FILE : graphical_node_editor.h

LAST MODIFIED : 21 March 2000

DESCRIPTION :
Functions for mouse controlled node position and vector editing based on
Scene input.
==============================================================================*/
#if !defined (GRAPHICAL_NODE_EDITOR_H)
#define GRAPHICAL_NODE_EDITOR_H

#include "finite_element/finite_element.h"
#include "graphics/scene.h"
#include "selection/node_selection.h"

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
	struct Scene *scene,struct MANAGER(FE_node) *node_manager,
	struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Creates the structure that needs to be passed to the GNE_scene_input_callback.
==============================================================================*/

int DESTROY(Graphical_node_editor)(
	struct Graphical_node_editor **node_editor_address);
/*******************************************************************************
LAST MODIFIED : 21 February 2000

DESCRIPTION :
Frees and deaccesses objects in the <node_editor> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (GRAPHICAL_NODE_EDITOR_H) */

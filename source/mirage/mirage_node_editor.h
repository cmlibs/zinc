/*******************************************************************************
FILE : mirage_node_editor.h

LAST MODIFIED : 19 February 1998

DESCRIPTION :
Special graphical node editor for mirage digitiser windows.
==============================================================================*/
#if !defined (MIRAGE_NODE_EDITOR_H)
#define MIRAGE_NODE_EDITOR_H

/*
Global types
------------
*/
enum Mirage_node_editor_mode
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
==============================================================================*/
{
	MNE_EDIT_NODE,
	MNE_EDIT_OFF
}; /* enum Mirage_node_editor_mode */

struct Mirage_node_editor;

/*
Global functions
----------------
*/
struct Mirage_node_editor *CREATE(Mirage_node_editor)(
	struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Creates a mirage node editor and sets it up so that all view->scenes in the
movie structure send input to this graphical node editor.
This editor is only for use with digitiser_windows.
Note that it may only be created after calling enable_Mirage_movie_graphics
for the movie, and must be destroyed before the views.
==============================================================================*/

int DESTROY(Mirage_node_editor)(
	struct Mirage_node_editor **mirage_node_editor_address);
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Cleans up space used by Mirage_node_editor structure.
==============================================================================*/
#endif /* !defined (MIRAGE_NODE_EDITOR_H) */

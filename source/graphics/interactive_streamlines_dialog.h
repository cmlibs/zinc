/*******************************************************************************
FILE : interactive_streamlines_dialog.h

LAST MODIFIED : 9 November 1998

DESCRIPTION :
This module creates a free streamline_editor_dialog input device, using two
dof3, two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#if !defined (INTERACTIVE_STREAMLINES_DIALOG)
#define INTERACTIVE_STREAMLINES_DIALOG

int bring_up_interactive_streamline_dialog(
	Widget *interactive_streamline_dialog_address,Widget parent,
	struct MANAGER(Interactive_streamline) *interactive_streamline_manager,
	struct Interactive_streamline *streamline,
	struct User_interface *user_interface,struct MANAGER(Scene) *scene_manager);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
If there is a interactive_streamline dialog in existence, then bring it to the
front, else create a new one.
==============================================================================*/
#endif /* !defined (INTERACTIVE_STREAMLINES_DIALOG) */

/*******************************************************************************
FILE : page_window.h

LAST MODIFIED : 10 October 2001

DESCRIPTION :
==============================================================================*/
#if !defined (PAGE_WINDOW_H)
#define PAGE_WINDOW_H

#include "unemap/mapping_window.h"
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/
struct Page_window;
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
The page window object.
==============================================================================*/

/*
Global functions
----------------
*/
int destroy_Page_window(struct Page_window **page_window_address);
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
If the <address> field of the page window is not NULL, <*address> is set to
NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  The function frees the memory associated with the fields of the
page window and frees the memory associated with the page window.
==============================================================================*/

#if defined (MOTIF)
Widget get_page_window_close_button(struct Page_window *page_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
Widget create_page_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface);
		/*???DB.  Position and size should be passed ? */
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Creates a popup shell widget for an page window.  If <address> is not NULL,
<*address> is set to the id of the created shell and on destruction <*address>
will be set to NULL.  The id of the created widget is returned.
???If address is NULL, it won't create an entry in the shell list ?
==============================================================================*/
#endif /* defined (MOTIF) */

struct Page_window *create_Page_window(struct Page_window **address,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HWND parent,
#endif /* defined (WINDOWS) */
	struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	struct Mapping_window **mapping_window_address,int pointer_sensitivity,
	char *signal_file_extension_write,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
This function allocates the memory for a page window and sets the fields to the
specified values (<address>, <activation>, <page_address>).  It then retrieves
a page window widget with the specified <parent> and assigns the widget ids to
the appropriate fields of the structure.  If successful it returns a pointer to
the created page window and, if <address> is not NULL, makes <*address> point to
the created page window.  If unsuccessful, NULL is returned.
==============================================================================*/

int open_Page_window(struct Page_window **address,
	struct Mapping_window **mapping_window_address,struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,
	int screen_width,int screen_height,
#endif /* defined (MOTIF) */
	int pointer_sensitivity,char *signal_file_extension_write,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
#endif /* !defined (PAGE_WINDOW_H) */

/*******************************************************************************
FILE : help_window.h

LAST MODIFIED : 24 June 1996

DESCRIPTION :
Interface file for opening and closing and working a CMISS help window.
==============================================================================*/
#if !defined (HELP_WINDOW_H)
#define HELP_WINDOW_H

#include <X11/Intrinsic.h>
#include <Mrm/MrmPublic.h>

/*
Global types
------------
*/
/*???DB.  Needed ? */
typedef void  (*D_FUNC)(void *);

struct Help_window
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Contains information about a CMISS help window.
==============================================================================*/
{
	char popped_up;
	Widget window_shell;
	char help_file_name[64];
	D_FUNC destroy_func;
	Display *display;
	MrmHierarchy hierarchy;
	short help_from_file;
	void *data_ptr;
	Widget app_shell,help_copy_button,help_do_button,help_find_button,
		help_select_button,help_text,help_topic,main_window;
}; /* struct Help_window */

/*
Global functions
----------------
*/
struct Help_window *create_help_window(D_FUNC destroy_func,void *data_ptr);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Creates a help window, and returns a pointer to the structure describing the
window.  If <destroy_func> is not NULL, it will be called when the window is
destroyed, with <data_ptr> as a parameter.  If destroy_func is NULL, but
<data_ptr> is not, then when the window is created, a pointer to the window
structure will be stored at the location pointed to by <data_ptr>, and when the
window is destroyed, that location will be cleared.
==============================================================================*/

void pop_up_help_window(struct Help_window *the_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops up the (already created) help window described in the structure the_window.
==============================================================================*/

void pop_down_help_window(struct Help_window *the_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops down (but doesn't destroy) the window described in the_window.
==============================================================================*/

void close_help_window(XtPointer help_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Closes and destroys the window pointed to by the window.  After this the
window must be recreated.  The routine relies on close_help_window_callback
to actually dispose of the window description structure and call the
destroy function (if necessary).
==============================================================================*/

void do_topic_text_help(struct Help_window *the_window);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Gets help on the string in the help topic edit field.
==============================================================================*/

void do_strings_help(struct Help_window *the_window,char **the_strings,
	short num_strings);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Gets help on the keywords in the_strings, and displays the help in the_window.
==============================================================================*/
#endif

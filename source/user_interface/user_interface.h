/*******************************************************************************
FILE : user_interface.h

LAST MODIFIED : 6 March 2002

DESCRIPTION :
Function definitions for the user interface.
???DB.  The main purpose is to have a graphical user interface, but the
	possibility of a command line should be kept in mind.
==============================================================================*/
#if !defined (USER_INTERFACE_H)
#define USER_INTERFACE_H

#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#include <windowsx.h>
	/*???DB.  Contains lots of convenience macros */
#endif /* defined (WIN32_USER_INTERFACE) */
#include "general/machine.h"

/*
Global types
------------
*/
struct Event_dispatcher;

struct User_interface;
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

#if defined (MOTIF)
typedef int (*Property_notify_callback)(XPropertyEvent *,void *,
	struct User_interface *);
#endif /* defined (MOTIF) */

/*
Global Functions
----------------
*/
#if defined (MOTIF)
int x_error_handler(Display *display, XErrorEvent *error);
/*******************************************************************************
LAST MODIFIED : 15 September 1999 

DESCRIPTION :
Responds to nonfatal XErrors and allows cmgui to continue.
==============================================================================*/
#endif /* defined (MOTIF) */

struct Shell_list_item *create_Shell_list_item(
#if defined (MOTIF)
	Widget *shell_address,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function allocates memory for a shell list item, initializes the <shell>
field to the specified value and adds the item to the beginning of the shell
list.  It returns a pointer to the created item if successful and NULL if
unsuccessful.
???DB.  Move in with windowing macros ?
==============================================================================*/

int destroy_Shell_list_item(struct Shell_list_item **list_item);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
This function removes the <list_item> from the shell list and frees the memory
for the <list_item>.  <*list_item> is set to NULL.

???SAB.  Seems unnessary to insist that all the windows keep track of the Shell
list item for the shell list, rather pass the respective shell and the list can
find the appropriate list object and remove it.
???DB.  Move in with windowing macros ?
==============================================================================*/

#if defined (MOTIF)
int destroy_Shell_list_item_from_shell(Widget *shell,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
This function removes the list_item which refers to <shell> from the shell list
and frees the memory for the <list_item>.
???DB.  Move in with windowing macros ?
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void destroy_window_shell(Widget widget,XtPointer list_item,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function removes the <list_item> from the shell list, frees the memory
for the <list_item> and sets <*(list_item->address)> to NULL.
???DB.  Move in with windowing macros ?
SAB This is probably not a good way to do this, requires XmNuserData to point
to the User_interface structure.  Prefer to have each window with it's own
structure which it the calls destroy_Shell_list_item
==============================================================================*/
#endif /* defined (MOTIF) */

int busy_cursor_on(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Switchs from the default cursor to the busy cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
==============================================================================*/

int busy_cursor_off(
#if defined (MOTIF)
	Widget excluded_shell,
#endif /* defined (MOTIF) */
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Switchs from the busy cursor to the default cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
==============================================================================*/

#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
struct User_interface *CREATE(User_interface)(int *argc_address, char **argv, 
	struct Event_dispatcher *event_dispatcher, char *class_name, 
	char *application_name);
#elif defined (WIN32_USER_INTERFACE) /* switch (OPERATING_SYSTEM) */
struct User_interface *CREATE(User_interface)(HINSTANCE current_instance,
	HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state,
	struct Event_dispatcher *event_dispatcher);
#endif /* switch (OPERATING_SYSTEM) */
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Open the <user_interface>.
==============================================================================*/

int DESTROY(User_interface)(struct User_interface **user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int User_interface_end_application_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
==============================================================================*/

#if defined (MOTIF)
Widget User_interface_get_application_shell(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
Display *User_interface_get_display(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int User_interface_get_screen_width(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int User_interface_get_screen_height(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int User_interface_get_widget_spacing(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (OPENGL_API)
int User_interface_set_specified_visual_id(struct User_interface *user_interface,
	int specified_visual_id);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Sets a particular Open GL visual to be used by the graphics.
==============================================================================*/
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int User_interface_get_specified_visual_id(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the particular Open GL visual if set to be used by the graphics.
==============================================================================*/
#endif /* defined (OPENGL_API) */

#if defined (MOTIF)
XFontStruct *User_interface_get_normal_font(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
XmFontList User_interface_get_normal_fontlist(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
XmFontList User_interface_get_button_fontlist(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
Pixmap User_interface_get_no_cascade_pixmap(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Returns a pixmap to avoid large gaps on the right of cascade buttons (option menus)
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (WIN32_USER_INTERFACE)
HINSTANCE User_interface_get_instance(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

int User_interface_get_local_machine_name(struct User_interface *user_interface,
	char **name);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
If the local machine name is know ALLOCATES and returns a string containing that
name.
==============================================================================*/

struct Event_dispatcher *User_interface_get_event_dispatcher(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int application_main_step(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Performs one step of the application_main_step update allowing the programmer
to execute the same main loop elsewhere under special conditions (i.e. waiting
for a response from a modal dialog).
==============================================================================*/

int
#if defined (WIN32_USER_INTERFACE)
	WINAPI
#endif /* defined (WIN32_USER_INTERFACE) */
	application_main_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 30 May 1996

DESCRIPTION :
???DB.  Does the main window need to be passed to this ?
???DB.  Should we have our own "WINAPI" (size specifier) ?
==============================================================================*/

#if defined (MOTIF)
int set_property_notify_callback(struct User_interface *user_interface,
	Property_notify_callback property_notify_callback,void *property_notify_data,
	Widget widget);
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Sets the <property_notify_callback> for the <user_interface>.  This is used for
communication with other applications.
???DB.  At present only one (not a list)
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int MrmOpenHierarchy_base64_string(char *base64_string,
	MrmHierarchy *hierarchy,int *hierarchy_open);
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
This wrapper allows the passing of the <base64_string> which is intended to
contain a uid file converted to base64.  This function converts it back to
binary and writes a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int MrmOpenHierarchy_base64_multiple_strings(int number_of_strings, 
	char **base64_strings,
	MrmHierarchy *hierarchy,int *hierarchy_open);
/*******************************************************************************
LAST MODIFIED : 19 April 1999

DESCRIPTION :
This wrapper allows the passing of an array of <base64_strings> which are 
intended to contain a uid files converted to base64.  
This function converts it back to
binary and writes a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
int install_accelerators(Widget widget, Widget top_widget);
/*******************************************************************************
LAST MODIFIED : 24 December 1998

DESCRIPTION :
This travels down the widget tree from <widget> and installs all
accelerators in any subwidgets of <top_widget> in every appropriate subwidget of
<widget>.
==============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (USER_INTERFACE_H) */

/*******************************************************************************
FILE : user_interface.h

LAST MODIFIED : 27 December 1999

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
#if defined (WINDOWS)
#include <windows.h>
#include <windowsx.h>
	/*???DB.  Contains lots of convenience macros */
#endif /* defined (WINDOWS) */
#include "general/machine.h"

/*
Global types
------------
*/
struct Shell_stack_item;
struct Shell_list_item;

#if defined (MOTIF)
/* for communication with other applications */
struct User_interface;

typedef int (*Property_notify_callback)(XPropertyEvent *,void *,
	struct User_interface *);
#endif /* defined (MOTIF) */

struct User_interface
/*******************************************************************************
LAST MODIFIED : 27 December 1999

DESCRIPTION :
The variables that control the interaction of the application with the
application/window manager.
???DB.  Used to be globals.
==============================================================================*/
{
	int continue_interface;
#if defined (MOTIF)
	char *application_name,**argv,*class_name;
	Cursor busy_cursor;
	Display *display;
	int *argc_address,screen_height,screen_width,widget_spacing;
	/* to avoid large gaps on the right of cascade buttons (option menus) */
	Pixmap no_cascade_pixmap;
	/* for communication with other applications */
	Property_notify_callback property_notify_callback;
	void *property_notify_data;
	Widget application_shell;
	XFontStruct *button_font,*heading_font,*list_font,*menu_font,*normal_font,
		*normal_non_proportional_font,*small_font;
	XtAppContext application_context;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	HINSTANCE instance;
	HWND main_window;
	int main_window_state,widget_spacing;
	LPSTR command_line;
#endif /* defined (WINDOWS) */
	struct Machine_information *local_machine_info;
	struct Shell_list_item *shell_list;
	struct Shell_stack_item *active_shell_stack;
}; /* struct User_interface */

#if defined (OLD_CODE)
#if defined (MOTIF)
typedef struct
/*******************************************************************************
LAST MODIFIED : 11 December 1996

DESCRIPTION :
???DB.  Shouldn't really be here because application dependent.
==============================================================================*/
{
	Pixel background_colour,foreground_colour;
	float background_rgb[3],foreground_rgb[3];
	Pixel background_printer_colour,foreground_printer_colour;
	int printer_page_bottom_margin_mm,printer_page_height_mm,
		printer_page_left_margin_mm,printer_page_right_margin_mm,
		printer_page_top_margin_mm,printer_page_width_mm;
	char *examples_directory,*help_directory,*help_url;
} User_settings;
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

/*
Global variables
----------------
*/
#if defined (OLD_CODE)
#if defined (MOTIF)
extern User_settings user_settings;
	/*???DB.  Shouldn't really be here because application dependent */
#endif /* defined (MOTIF) */
#endif /* defined (OLD_CODE) */

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

int open_user_interface(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 27 May 1996

DESCRIPTION :
Open the <user_interface>.
==============================================================================*/

int close_user_interface(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 30 May 1996

DESCRIPTION :
==============================================================================*/

int User_interface_end_application_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

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
#if defined (WINDOWS)
	WINAPI
#endif /* defined (WINDOWS) */
	application_main_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 30 May 1996

DESCRIPTION :
???DB.  Does the main window need to be passed to this ?
???DB.  Should we have our own "WINAPI" (size specifier) ?
==============================================================================*/

#if defined (MOTIF)
int set_property_notify_callback(struct User_interface *user_interface,
	Property_notify_callback property_notify_callback,void *property_notify_data);
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

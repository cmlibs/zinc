/*******************************************************************************
FILE : gui_prototype.h

LAST MODIFIED : 1 March 1997

DESCRIPTION :
Window routines.

Overview:
We wish to write our code to work on more than one windowing platform.
Hopefully these macros will allow us to this in a portable way.  All of the
macros are designed to reflect the functionality - not the implementation.
To use them, think of the steps that must be performed (regardless of the
windowing system), and hopefully there should be macros which correspond to
those steps.
==============================================================================*/
#if !defined (GUI_PROTOTYPE_H)
#define GUI_PROTOTYPE_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#include <X11/Xlib.h>
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include <windows.h>
#endif /* defined (WINDOWS) */

#if defined (MOTIF)
typedef Widget Dialog_handle;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
typedef HWND Dialog_handle;
#endif /* defined (WINDOWS) */

/*
Macros
======
*/

/*
Global types
------------
*/
/* Contains all information necessary for the operation of the dialog -
	independent of the windowing system. */
#if defined (FULL_NAMES)
#define DIALOG_DATA_STRUCT_( dialog_name ) \
	Dialog_data_ ## dialog_name ## _struct
#else
#define DIALOG_DATA_STRUCT_( dialog_name )  DD ## dialog_name
#endif
#define DIALOG_DATA_STRUCT( dialog_name )  DIALOG_DATA_STRUCT_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_DATA_STRUCT_( dialog_name )  dialog_name ## Data
#else
#define WINDOW_DATA_STRUCT_( dialog_name )  WD ## dialog_name
#endif
#define WINDOW_DATA_STRUCT( dialog_name )  WINDOW_DATA_STRUCT_(dialog_name)

/* Contains all information to keep track of items in the dialog */
#if defined (FULL_NAMES)
#define DIALOG_STRUCT_( dialog_name )  Dialog_ ## dialog_name ## _struct
#else
#define DIALOG_STRUCT_( dialog_name )  DP ## dialog_name
#endif
#define DIALOG_STRUCT( dialog_name )  DIALOG_STRUCT_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_STRUCT_( dialog_name )  Dialog_name ## Part
#else
#define WINDOW_STRUCT_( dialog_name )  WP ## dialog_name
#endif
#define WINDOW_STRUCT( dialog_name )  WINDOW_STRUCT_(dialog_name)

/* Contains initialisation information */
#if defined (FULL_NAMES)
#define DIALOG_INITIALISATION_STRUCT_( dialog_name ) \
	Dialog_ ## dialog_name ## _init_struct
#else
#define DIALOG_INITIALISATION_STRUCT_( dialog_name )  DI ## dialog_name
#endif
#define DIALOG_INITIALISATION_STRUCT( dialog_name ) \
	DIALOG_INITIALISATION_STRUCT_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_INITIALISATION_STRUCT_( dialog_name ) \
	dialog_name ## _init_struct
#else
#define WINDOW_INITIALISATION_STRUCT_( dialog_name )  WI ## dialog_name
#endif
#define WINDOW_INITIALISATION_STRUCT( dialog_name ) \
	WINDOW_INITIALISATION_STRUCT_(dialog_name)

#if defined (OLD_CODE)
/* Pick off the dialog data from the dialog struct */
#define DIALOG_GET_DATA( dialog_name )  (&temp_dialog->data)
#endif /* defined (OLD_CODE) */

/* How to name the functions */
#define DIALOG_FUNCTION_( dialog_name , function_name ) \
	dialog_name ## _ ## function_name
#define DIALOG_FUNCTION( dialog_name , function_name ) \
	DIALOG_FUNCTION_(dialog_name,function_name)

#define WINDOW_FUNCTION_( dialog_name , function_name ) \
	dialog_name ## function_name
#define WINDOW_FUNCTION( dialog_name , function_name ) \
	WINDOW_FUNCTION_(dialog_name,function_name)

/* Creation function */
	/*???DB.  Should be CREATE(DIALOG( )) , but haven't got DIALOG (yet) */
#if defined (FULL_NAMES)
#define DIALOG_CREATE_( dialog_name )  create_ ## dialog_name ## _dialog
#else
#define DIALOG_CREATE_( dialog_name )  cd ## dialog_name
#endif
#define DIALOG_CREATE(dialog_name)  DIALOG_CREATE_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_CREATE_( dialog_name )  create_ ## dialog_name ## _window
#else
#define WINDOW_CREATE_( dialog_name )  cw ## dialog_name
#endif
#define WINDOW_CREATE( dialog_name )  WINDOW_CREATE_(dialog_name)

/* Contains parameters passed to all global dialog routines */
#define DIALOG_GLOBAL_PARAM( dialog_name ) \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog
#define WINDOW_GLOBAL_PARAM( dialog_name )  HWND hwndWindow

/* Contains parameters passed to all local dialog routines */
#define DIALOG_LOCAL_PARAM( dialog_name ) \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog
#define WINDOW_LOCAL_PARAM( dialog_name ) \
	WINDOW_STRUCT(dialog_name) *temp_window

/* Local variables used in handler functions */
#define DIALOG_BOTH_VARIABLES( dialog_name ) \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog = \
		(struct DIALOG_STRUCT(dialog_name) *)NULL; \
	struct DIALOG_DATA_STRUCT(dialog_name) *temp_data = \
		(struct DIALOG_DATA_STRUCT(dialog_name) *)NULL
/* Local variables used before calls to dialog functions */
#define DIALOG_WRAPPER_VARIABLES( dialog_name ) \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog = \
		(struct DIALOG_STRUCT(dialog_name) *)NULL

/* Local variables used in dialog functions */
#define DIALOG_VARIABLES( dialog_name ) \
	struct DIALOG_DATA_STRUCT(dialog_name) *temp_data = \
		(struct DIALOG_DATA_STRUCT(dialog_name) *)NULL
#define WINDOW_VARIABLES( dialog_name ) \
	WINDOW_STRUCT(dialog_name) *temp_window = \
		(WINDOW_STRUCT(dialog_name) *)NULL; \
	WINDOW_DATA_STRUCT(dialog_name) *temp_data = \
		(WINDOW_DATA_STRUCT(dialog_name) *)NULL

/* Gets dialog information from the dialog handle */
#if defined (MOTIF)
#define DIALOG_INITIALISE_VARIABLES( dialog_name ) \
	(DIALOG_GET_INFORMATION(dialog_name)(temp_dialog,&temp_data))
#define DIALOG_WRAPPER_INITIALISE_VARIABLES( dialog_name ) \
	(DIALOG_WRAPPER_GET_INFORMATION(dialog_name)(dialog,&temp_dialog))
#define WINDOW_INITIALISE_VARIABLES( dialog_name ) \
	(WINDOW_GET_INFORMATION(dialog_name)(window,&temp_window,&temp_data))
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#define DIALOG_INITIALISE_VARIABLES( dialog_name ) \
	(DIALOG_GET_INFORMATION(dialog_name)(temp_dialog,&temp_data))
#define DIALOG_WRAPPER_INITIALISE_VARIABLES( dialog_name ) \
	(DIALOG_WRAPPER_GET_INFORMATION(dialog_name)(hwndDialog,&temp_dialog))
#define WINDOW_INITIALISE_VARIABLES( dialog_name ) \
	(WINDOW_GET_INFORMATION(dialog_name)(hwndWindow,&temp_window,&temp_data))
#endif /* defined (WINDOWS) */

/* Name of the routine to retrieve structures */
#if defined (FULL_NAMES)
#define DIALOG_GET_INFORMATION_( dialog_name )  dialog_name ## _get_information
#else
#define DIALOG_GET_INFORMATION_( dialog_name )  dg ## dialog_name
#endif
#define DIALOG_GET_INFORMATION( dialog_name ) \
	DIALOG_GET_INFORMATION_(dialog_name)

#if defined (FULL_NAMES)
#define DIALOG_WRAPPER_GET_INFORMATION_( dialog_name ) \
	dialog_name ## _wrapper_get_information
#else
#define DIALOG_WRAPPER_GET_INFORMATION_( dialog_name )  dgw ## dialog_name
#endif
#define DIALOG_WRAPPER_GET_INFORMATION( dialog_name ) \
	DIALOG_WRAPPER_GET_INFORMATION_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_GET_INFORMATION_( dialog_name )  dialog_name ## _get_information
#else
#define WINDOW_GET_INFORMATION_( dialog_name )  wg ## dialog_name
#endif
#define WINDOW_GET_INFORMATION( dialog_name ) \
	WINDOW_GET_INFORMATION_(dialog_name)

/* Contains parameters passed to all user routines */
#define DIALOG_DATA_PARAM( dialog_name ) \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog, \
	struct DIALOG_DATA_STRUCT(dialog_name) *temp_data
#define WINDOW_DATA_PARAM( dialog_name ) \
	WINDOW_DATA_STRUCT(dialog_name) *temp_data

/* Routine to initialise local data */
#if defined (FULL_NAMES)
#define DIALOG_DATA_INITIALISE_( dialog_name ) \
	initialise_ ## dialog_name ## _data
#else
#define DIALOG_DATA_INITIALISE_( dialog_name )  di ## dialog_name
#endif
#define DIALOG_DATA_INITIALISE( dialog_name ) \
	DIALOG_DATA_INITIALISE_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_DATA_INITIALISE_( dialog_name ) \
	initialise_ ## dialog_name ## _data
#else
#define WINDOW_DATA_INITIALISE_( dialog_name )  wi ## dialog_name
#endif
#define WINDOW_DATA_INITIALISE( dialog_name ) \
	WINDOW_DATA_INITIALISE_(dialog_name)

/* Routine to destroy local data */
	/*???DB.  Should be DESTROY( */
#if defined (FULL_NAMES)
#define DIALOG_DATA_DESTROY_( dialog_name ) \
	destroy_ ## dialog_name ## _data
#else
#define DIALOG_DATA_DESTROY_( dialog_name )  dd ## dialog_name
#endif
#define DIALOG_DATA_DESTROY( dialog_name ) \
	DIALOG_DATA_DESTROY_( dialog_name )

#if defined (FULL_NAMES)
#define WINDOW_DATA_DESTROY_( dialog_name ) \
	destroy_ ## dialog_name ## _data
#else
#define WINDOW_DATA_DESTROY_( dialog_name )  wd ## dialog_name
#endif
#define WINDOW_DATA_DESTROY( dialog_name ) \
	WINDOW_DATA_DESTROY_(dialog_name)

/* Routine to identify all sub components of the dialog */
#if defined (FULL_NAMES)
#define DIALOG_WINDOW_INITIALISE_( dialog_name ) \
	dialog_name ## _initialise_window_information
#else
#define DIALOG_WINDOW_INITIALISE_( dialog_name )  dwi ## dialog_name
#endif
#define DIALOG_WINDOW_INITIALISE( dialog_name ) \
	DIALOG_WINDOW_INITIALISE_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_WINDOW_INITIALISE_( dialog_name ) \
	dialog_name ## _initialise_window_information
#else
#define WINDOW_WINDOW_INITIALISE_( dialog_name )  wwi ## dialog_name
#endif
#define WINDOW_WINDOW_INITIALISE( dialog_name ) \
	WINDOW_WINDOW_INITIALISE_(dialog_name)

/* Routine to destroy all sub components of the dialog */
#if defined (FULL_NAMES)
#define DIALOG_WINDOW_DESTROY_( dialog_name ) \
	destroy_ ## dialog_name ## _window_information
#else
#define DIALOG_WINDOW_DESTROY_( dialog_name )  dwd ## dialog_name
#endif
#define DIALOG_WINDOW_DESTROY( dialog_name ) \
	DIALOG_WINDOW_DESTROY_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_WINDOW_DESTROY_( dialog_name ) \
	destroy_ ## dialog_name ## _window_information
#else
#define WINDOW_WINDOW_DESTROY_( dialog_name )  wwd ## dialog_name
#endif
#define WINDOW_WINDOW_DESTROY( dialog_name ) \
	WINDOW_WINDOW_DESTROY_(dialog_name)

#if defined (WINDOWS)
/* Routine to handle a specific windows message */
#if defined (FULL_NAMES)
#define DIALOG_HANDLER_( dialog_name , message_name ) \
	dialog_name ## _handle_ ## message_name
#else
#define DIALOG_HANDLER_( dialog_name , message_name ) \
	dialog_name ## d ## message_name
#endif
#define DIALOG_HANDLER( dialog_name , message_name ) \
	DIALOG_HANDLER_(dialog_name,message_name)

#if defined (FULL_NAMES)
#define WINDOW_HANDLER_( dialog_name , message_name ) \
	dialog_name ## _handle_ ## message_name
#else
#define WINDOW_HANDLER_( dialog_name , message_name ) \
	dialog_name ## w ## message_name
#endif
#define WINDOW_HANDLER( dialog_name , message_name ) \
	WINDOW_HANDLER_(dialog_name,message_name)

/* Class procedure */
#if defined (FULL_NAMES)
#define DIALOG_CLASS_PROCEDURE_( dialog_name ) \
	dialog_name ## _class_procedure
#else
#define DIALOG_CLASS_PROCEDURE_( dialog_name )  dcp ## dialog_name
#endif
#define DIALOG_CLASS_PROCEDURE( dialog_name ) \
	DIALOG_CLASS_PROCEDURE_(dialog_name)

#if defined (FULL_NAMES)
#define WINDOW_CLASS_PROCEDURE_( dialog_name ) \
	dialog_name ## _class_procedure
#else
#define WINDOW_CLASS_PROCEDURE_( dialog_name )  wcp ## dialog_name
#endif
#define WINDOW_CLASS_PROCEDURE( dialog_name ) \
	WINDOW_CLASS_PROCEDURE_(dialog_name)

/* Dialog procedure */
#if defined (FULL_NAMES)
#define DIALOG_PROCEDURE_( dialog_name )  dialog_name ## _procedure
#else
#define DIALOG_PROCEDURE_( dialog_name )  p ## dialog_name
#endif
#define DIALOG_PROCEDURE( dialog_name )  DIALOG_PROCEDURE_(dialog_name)

#define PROTOTYPE_DIALOG_HANDLER_FUNCTION_WM_INITDIALOG(dialog_name) \
BOOL DIALOG_HANDLER(dialog_name,WM_INITDIALOG)(HWND hwndDialog, \
	HWND hwndFocus, LPARAM lParam)
#define PROTOTYPE_DIALOG_HANDLER_FUNCTION_WM_DESTROY(dialog_name) \
void DIALOG_HANDLER(dialog_name,WM_DESTROY)(HWND hwndDialog)
#endif /* defined (WINDOWS) */

/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION:
Creates the dialog.
==============================================================================*/
#if defined (MOTIF)
#define PROTOTYPE_DIALOG_CREATE_FUNCTION(dialog_name) \
struct DIALOG_STRUCT(dialog_name) *DIALOG_CREATE(dialog_name)(Widget parent, \
	DIALOG_INITIALISATION_STRUCT(dialog_name) *initial_data)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#define PROTOTYPE_DIALOG_CREATE_FUNCTION(dialog_name) \
struct DIALOG_STRUCT(dialog_name) *DIALOG_CREATE(dialog_name)(HWND hwndParent, \
	DIALOG_INITIALISATION_STRUCT(dialog_name) *initial_data)
#define PROTOTYPE_WINDOW_CREATE_FUNCTION(dialog_name) \
HWND WINDOW_CREATE(dialog_name)(HWND hwndParent,int identifier, \
	WINDOW_INITIALISATION_STRUCT(dialog_name) *initial_data)
#endif /* defined (WINDOWS) */

/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Gets dialog information from the dialog handle.
==============================================================================*/
#if defined (MOTIF)
#define PROTOTYPE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
int DIALOG_WRAPPER_GET_INFORMATION(dialog_name)(Widget dialog, \
	struct DIALOG_STRUCT(dialog_name) **temp_dialog_address)
#define PROTOTYPE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
int DIALOG_GET_INFORMATION(dialog_name)( \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog, \
	struct DIALOG_DATA_STRUCT(dialog_name) **temp_data_address)
#define PROTOTYPE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name) \
int WINDOW_GET_INFORMATION(dialog_name)(Widget window, \
	WINDOW_STRUCT(dialog_name) **temp_window_address, \
	WINDOW_DATA_STRUCT(dialog_name) **temp_data_address)
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#define PROTOTYPE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name) \
int DIALOG_WRAPPER_GET_INFORMATION(dialog_name)(HWND hwndDialog, \
	struct DIALOG_STRUCT(dialog_name) **temp_dialog_address)
#define PROTOTYPE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name) \
int DIALOG_GET_INFORMATION(dialog_name)( \
	struct DIALOG_STRUCT(dialog_name) *temp_dialog, \
	struct DIALOG_DATA_STRUCT(dialog_name) **temp_data_address)
#define PROTOTYPE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name) \
int WINDOW_GET_INFORMATION(dialog_name)(HWND hwndWindow, \
	WINDOW_STRUCT(dialog_name) **temp_window_address, \
	WINDOW_DATA_STRUCT(dialog_name) **temp_data_address)
#endif /* defined (WINDOWS) */

/* Local functions */
#if defined (WINDOWS)
#define PROTOTYPE_LOCAL_GUI_DIALOG_FUNCTIONS(dialog_name) \
PROTOTYPE_DIALOG_GET_INFORMATION_FUNCTION(dialog_name); \
PROTOTYPE_DIALOG_WRAPPER_GET_INFORMATION_FUNCTION(dialog_name); \
PROTOTYPE_DIALOG_HANDLER_FUNCTION_WM_INITDIALOG(dialog_name); \
PROTOTYPE_DIALOG_HANDLER_FUNCTION_WM_DESTROY(dialog_name)
#define PROTOTYPE_LOCAL_GUI_WINDOW_FUNCTIONS(dialog_name) \
PROTOTYPE_WINDOW_GET_INFORMATION_FUNCTION(dialog_name)
#endif /* defined (WINDOWS) */

/* Global functions */
#define PROTOTYPE_GLOBAL_GUI_DIALOG_FUNCTIONS(dialog_name) \
PROTOTYPE_DIALOG_CREATE_FUNCTION(dialog_name)
#define PROTOTYPE_GLOBAL_GUI_WINDOW_FUNCTIONS(dialog_name) \
PROTOTYPE_WINDOW_CREATE_FUNCTION(dialog_name)

#endif /* !defined (GUI_PROTOTYPE_H) */

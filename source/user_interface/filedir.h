/*******************************************************************************
FILE : filedir.h

LAST MODIFIED : 3 June 1999

DESCRIPTION :
Routines for opening files using Motif widgets.
==============================================================================*/
#if !defined (FILEDIR_H)
#define FILEDIR_H

#include <stddef.h>
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum File_type
{
	REGULAR,
	DIRECTORY
}; /* enum File_type */

typedef int (*File_operation)(char *,void *);

typedef int (*File_cancel_operation)(void *);

struct File_open_data
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
A pointer to a structure of this type should be the client data when either
<file_open_and read> or <file_open_and_write> is registered as a callback.  The
user should specify the public fields.  <filter_or_extension> is the filter used
to construct the list of files when opening a file to read and is the default
extension when specifying a file to write to.  <type> is the file type - either
a <REGULAR> file or <DIRECTORY> <operation> is performed on the chosen file.
The file name and <arguments> are passed to operation.  The private fields are
used to keep track of the widgets involved.
==============================================================================*/
{
	/* public */
	char *filter_extension;
	enum File_type type;
	File_operation operation;
	File_cancel_operation cancel_operation;
	int allow_direct_to_printer;
	void *arguments, *cancel_arguments;
	struct User_interface *user_interface;
	/* private */
#if defined (MOTIF)
	Widget activation,file_list,selection_shell,selection,selection_label,
		selection_text,warning_box,warning_shell;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	OPENFILENAME open_file_name;
#endif /* defined (WIN32_USER_INTERFACE) */
	/* file name is used for passing into callbacks */
	char *file_name;
}; /* struct File_open_data */

/*
Global functions
----------------
*/
struct File_open_data *create_File_open_data(char *filter_extension,
	enum File_type type,File_operation operation,void *arguments,
	int allow_direct_to_printer,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 December 1996

DESCRIPTION :
This function allocates memory for file open data and initializes the fields to
the specified values.  It returns a pointer to the created structure if
successful and NULL if unsuccessful.
==============================================================================*/

int destroy_File_open_data(struct File_open_data **file_open_data);
/*******************************************************************************
LAST MODIFIED : 14 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**file_open_data>,
frees the memeory for <**file_open_data> and changes <*file_open_data> to NULL.
==============================================================================*/

void open_file_and_read(
#if defined (MOTIF)
	Widget widget,XtPointer client_data,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
	);
/*******************************************************************************
LAST MODIFIED : 20 April 1997

DESCRIPTION :
Expects a pointer to a File_open_data structure as the <client_data> (MOTIF).
Displays a list of the file names matching the <filter>.  After the user selects
a file name the <file_operation> is performed on the file with the <arguments>.
???DB.  Make uil callback a different routine ?
==============================================================================*/

void open_file_and_write(
#if defined (MOTIF)
	Widget widget,XtPointer client_data,XtPointer call_data
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
	);
/*******************************************************************************
LAST MODIFIED : 21 April 1997

DESCRIPTION :
Expects a pointer to a File_open_data structure as the <client_data> (MOTIF).
Prompts the user for the name of file, omitting the <extension>, to write to.
The <file_operation> with the <user_arguments> and the output directed to the
specified file.
???DB.  Make uil callback a different routine ?
==============================================================================*/

int register_file_cancel_callback(struct File_open_data *file_open_data,
	File_cancel_operation cancel_operation, void *cancel_arguments);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Register a callback that gets called when the file dialog is cancelled.
==============================================================================*/
#endif /* !defined (FILEDIR_H) */

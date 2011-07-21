/*******************************************************************************
FILE : help.h

LAST MODIFIED : 4 December 1994

DESCRIPTION :
Interface file for opening and closing and working a CMISS help window.
*******************************************************************************/

#ifndef _H_help
#define _H_help    1



/**********Structures**********/

typedef void  (*D_FUNC)(void *);

struct Help_window
{
	MrmHierarchy          hierarchy;
	Display    *display;
	Widget                app_shell,
			window_shell,
												main_window,
			help_text,
			help_topic,
			help_find_button,
			help_do_button,
			help_select_button,
			help_copy_button;
	char            help_file_name[64];
	char                  help_from_file;
	D_FUNC                destroy_func;
	void                  *data_ptr;
};


/**********Function Prototypes**********/

struct Help_window *create_help(
	D_FUNC    destroy_func,
	void      *data_ptr
	);

void close_help(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void identify_help_text(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void identify_help_topic(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void changed_help_topic(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void return_in_help_topic(
	Widget    caller,
	struct Help_window  *the_window,
	XmAnyCallbackStruct  *caller_data
	);

void identify_help_find_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void do_help_find_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void identify_help_do_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void do_help_do_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void identify_help_select_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void do_help_select_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void identify_help_copy_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void do_help_copy_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

void set_other_buttons(
	struct Help_window  *the_window,
	char      set
	);

void do_help_close_button(
	Widget    caller,
	struct Help_window  *the_window,
	void      *caller_data
	);

#endif

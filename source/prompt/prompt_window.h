/*******************************************************************************
FILE : prompt_window.h

LAST MODIFIED : 30 January 1997

DESCRIPTION :
Definitions for prompt window structure, and associated functions
==============================================================================*/
#if !defined (PROMPT_WINDOW_H)
#define PROMPT_WINDOW_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>

struct Prompt_window
{
	struct Prompt_window **address;
	Widget prompt_history;
	Widget prompt_reply;
	Widget window;
	Widget shell;
	int    popup;
	int    replied; /* whether the current prompt has been replied */
};

struct Prompt_window *create_Prompt_window(struct Prompt_window **address,
	Widget parent);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Create the structures and retrieve the prompt window from the uil file.
==============================================================================*/

void destroy_Prompt_window(Widget widget_id, XtPointer prompt_window,
	XtPointer call_data);
void get_reply(Widget widget_id, XtPointer prompt_window, XtPointer call_data);
void identify_prompt_history(Widget widget_id, XtPointer prompt_window,
	XtPointer call_data);
void identify_prompt_reply(Widget widget_id, XtPointer prompt_window,
	XtPointer call_data);

int popdown_prompt_window(struct Prompt_window *prompt_window);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Pops down the prompt window if it is up.
==============================================================================*/

int popup_prompt_window(struct Prompt_window *prompt_window);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Pops up the prompt window.
==============================================================================*/

int write_question(char *question,struct Prompt_window **prompt_window_address,
	Widget parent);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Writes a string to the prompt question label, and pops up the window.
==============================================================================*/
#endif

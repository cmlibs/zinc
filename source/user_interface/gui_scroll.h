/*******************************************************************************
FILE : gui_scroll.h

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Scroll bar routines.
???GMH.  Me thinks this should be a module, with global routines.
==============================================================================*/
#if !defined (GUI_SCROLL_H)
#define GUI_SCROLL_H
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/* These should be global, not local */
#define WINDOW_SCROLL_SET_RANGE(dialog_name,scroll_name,min_value,max_value) \
	dialog_name ## _set_range(temp_dialog->scroll_name,min_value,max_value)
#define WINDOW_SCROLL_SET_POS(dialog_name,scroll_name,value) \
	dialog_name ## _set_pos(temp_dialog->scroll_name,value)
#define WINDOW_SCROLL_GET_POS(dialog_name,scroll_name) \
	dialog_name ## _get_pos(temp_dialog->scroll_name)


#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Sets the range of the scroll bar.
==============================================================================*/
#define DECLARE_GUI_SCROLL_SET_RANGE(dialog_name) \
int dialog_name ## _set_range(HWND scroll_name, \
	int min_value,int max_value) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(dialog_name ## _set_range); \
	if (scroll_name) \
	{ \
		ScrollBar_SetRange(scroll_name,min_value,max_value,1); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"dialog_name ## _set_range.  %s", \
			"Invalid button"); \
	} \
	LEAVE; \
\
	return return_code; \
} /* dialog_name ## _set_range */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Sets the range of the scroll bar.
==============================================================================*/
#define DECLARE_GUI_SCROLL_SET_POS(dialog_name) \
int dialog_name ## _set_pos(HWND scroll_name, \
	int value) \
{ \
	int return_code; \
\
	return_code=0; \
	ENTER(dialog_name ## _set_pos); \
	if (scroll_name) \
	{ \
		ScrollBar_SetPos(scroll_name,value,1); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"dialog_name ## _set_pos.  %s", \
			"Invalid button"); \
	} \
	LEAVE; \
\
	return return_code; \
} /* dialog_name ## _set_pos */
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (MOTIF)
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Sets the range of the scroll bar.
==============================================================================*/
#define DECLARE_GUI_SCROLL_GET_POS(dialog_name) \
int dialog_name ## _get_pos(HWND scroll_name) \
{ \
	int return_pos; \
\
	return_pos=0; \
	ENTER(dialog_name ## _get_pos); \
	if (scroll_name) \
	{ \
		return_pos=ScrollBar_GetPos(scroll_name); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"dialog_name ## _get_pos.  %s", \
			"Invalid button"); \
	} \
	LEAVE; \
\
	return return_pos; \
} /* dialog_name ## _get_pos */
#endif /* defined (WIN32_USER_INTERFACE) */

#endif /* !defined (GUI_SCROLL_H) */

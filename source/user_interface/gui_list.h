/*******************************************************************************
FILE : gui_list.h

LAST MODIFIED : 15 January 1997

DESCRIPTION :
Window routines.
==============================================================================*/
#if !defined (GUI_LIST_H)
#define GUI_LIST_H

#if defined (MOTIF)
#error not implemented
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
#include <windows.h>
#endif /* defined (WINDOWS) */
#include "user_interface/gui_prototype.h"

#if defined (MOTIF)
int dialog_name ## _ ## list_name ## _view_end(DIALOG_PARAM(Command_window)) \
/***************************************************************************** \
LAST MODIFIED : 15 January 1997 \
\
DESCRIPTION : \
Adds an item to a list. \
============================================================================*/ \
{ \
	int return_item; \
\
	return_item= -1; \
	ENTER(dialog_name ## _ ## list_name ## _view_end); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->command_history) \
		{ \
			display_message(WARNING_MESSAGE, \
				"dialog_name ## _ ## list_name ## _view_end.  %s", \
				"Not implemented"); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _view_end.  %s", \
				"Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_selected.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_item); \
} /* dialog_name ## _ ## list_name ## _view_end */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Ensures that the last item is visible at the bottom of the list
==============================================================================*/
#define DECLARE_GUI_LIST_VIEW_END(dialog_name,list_name) \
int dialog_name ## _ ## list_name ## _view_end( \
	DIALOG_LOCAL_PARAM(dialog_name)) \
{ \
	int return_code,num_items,top_item,item_height; \
	long window_height; \
	RECT list_size; \
\
	return_code=FAILURE; \
	ENTER(dialog_name ## _ ## list_name ## _view_end); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->list_name) \
		{ \
			/* must have at least one item so we can get the height */ \
			if ((num_items=ListBox_GetCount(temp_dialog->list_name))!=0) \
			{ \
				top_item=0; \
				/* assume all items the same height (ensure >0) */ \
				if ((item_height=ListBox_GetItemHeight(temp_dialog->list_name,0))>0) \
				{ \
					/* get the height of the window */ \
					if (GetWindowRect(temp_dialog->list_name,&list_size)) \
					{ \
						window_height=list_size.bottom-list_size.top; \
						top_item=(num_items+1)-(window_height/item_height); \
						if (top_item<0) \
						{ \
							top_item=0; \
						} \
					} \
				} \
				ListBox_SetTopIndex(temp_dialog->list_name,top_item); \
			} \
			return_code=SUCCESS; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _view_end.  %s", \
				"Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ##_ ## list_name ## _view_end.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* dialog_name ## _ ## list_name ## _view_end */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Ensures that the last item is visible at the bottom of the list
==============================================================================*/
#define DECLARE_GUI_LIST_VIEW_BEGINNING(dialog_name,list_name) \
int dialog_name ## _ ## list_name ## _view_beginning( \
	DIALOG_LOCAL_PARAM(dialog_name)) \
{ \
	int return_code,num_items; \
\
	return_code=FAILURE; \
	ENTER(dialog_name ## _ ## list_name ## _view_end); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->list_name) \
		{ \
			/* must have at least one item so we can get the height */ \
			if ((num_items=ListBox_GetCount(temp_dialog->list_name))!=0) \
			{ \
				ListBox_SetTopIndex(temp_dialog->list_name,0); \
			} \
			return_code=SUCCESS; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _view_beginning.  %s", \
				"Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ##_ ## list_name ## _view_beginning.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* dialog_name ## _ ## list_name ## _view_beginning */
#endif /* defined (WINDOWS) */

#if defined (MOTIF)
int dialog_name ## _ ## list_name ## _add_item(DIALOG_PARAM(Command_window), \
	char *item) \
/***************************************************************************** \
LAST MODIFIED : 15 January 1997 \
\
DESCRIPTION : \
Adds an item to a list. \
============================================================================*/ \
{ \
	int num_items,max_items,return_code; \
	XmString new_item; \
\
	return_code=FAILURE; \
	ENTER(dialog_name ## _ ## list_name ## _add_item); \
	if (temp_dialog&&item) \
	{ \
		if (temp_dialog->command_history) \
		{ \
			/* create XmString of the command */ \
			new_item=XmStringCreateSimple(item); \
			/* get the number of items and the maximum number to make sure that we \
				don't overflow the list */ \
			XtVaGetValues(temp_dialog->command_history, \
				XmNhistoryItemCount,&num_items, \
				XmNhistoryMaxItems,&max_items, \
				NULL); \
			if (num_items==max_items)  /* Delete first element */ \
			{ \
				XmListDeletePos(temp_dialog->command_history,1); \
			} \
			/* add new command */ \
			XmListAddItem(temp_dialog->command_history,new_item,0); \
			XmStringFree(new_item); \
			/* show last command */ \
			XmListSetBottomPos(temp_dialog->command_history,0); \
			return_code=SUCCESS; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _add_item.  %s", \
				"Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _add_item.  %s","Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* dialog_name ## _ ## list_name ## _add_item */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Adds an item to a list (at the last pos).  Ensures that the last item is
visible.
==============================================================================*/
#define DECLARE_GUI_LIST_ADD_ITEM(dialog_name,list_name) \
int dialog_name ## _ ## list_name ## _add_item( \
	DIALOG_LOCAL_PARAM(dialog_name),char *item) \
{ \
	int return_code; \
\
	return_code=FAILURE; \
	ENTER(dialog_name ## _ ## list_name ## _add_item); \
	if (temp_dialog&&item) \
	{ \
		if (temp_dialog->list_name) \
		{ \
			ListBox_AddString(temp_dialog->list_name,item); \
			/* WINDOW_LIST_VIEW_END(list_name,list_name); */ \
			return_code=SUCCESS; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _add_item.  %s","Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _add_item.  %s","Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* dialog_name ## _ ## list_name ## _add_item */
#endif /* defined (WINDOWS) */

#if defined (MOTIF)
char *dialog_name ## _ ## list_name ## _get_item(DIALOG_PARAM(Command_window), \
	int item_index) \
/***************************************************************************** \
LAST MODIFIED : 15 January 1997 \
\
DESCRIPTION : \
Returns the text of the specfied item. \
============================================================================*/ \
{ \
	char *return_string; \
\
	return_string=(char *)NULL; \
	ENTER(dialog_name ## _ ## list_name ## _get_item); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->command_history) \
		{ \
			display_message(WARNING_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_item.  %s","Not implemented"); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_item.  %s","Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_item.  %s","Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_string); \
} /* dialog_name ## _ ## list_name ## _get_item */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Returns the text of the specified item.  Caller is responsible for freeing
the memory.
==============================================================================*/
#define DECLARE_GUI_LIST_GET_ITEM(dialog_name,list_name) \
char *dialog_name ## _ ## list_name ## _get_item( \
	DIALOG_LOCAL_PARAM(dialog_name),int item_index) \
{ \
	char *return_string; \
	int string_length; \
\
	return_string=(char *)NULL; \
	ENTER(dialog_name ## _ ## list_name ## _get_item); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->list_name) \
		{ \
			string_length=ListBox_GetTextLen(temp_dialog->list_name, \
				item_index); \
			if (ALLOCATE(return_string,char,string_length+1)) \
			{ \
				ListBox_GetText(temp_dialog->list_name,item_index, \
					return_string); \
				/* we must add the trailing NULL */ \
				return_string[string_length]='\0'; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"dialog_name ## _ ## list_name ## _get_item.  %s", \
					"Could not allocate memory for string"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_item.  %s","Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_item.  %s","Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_string); \
} /* dialog_name ## _ ## list_name ## _get_item */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Returns the text of the specified item.  Caller is responsible for freeing
the memory.
==============================================================================*/
#define DECLARE_GUI_LIST_GET_NUM_ITEMS(dialog_name,list_name) \
int dialog_name ## _ ## list_name ## _get_num_items( \
	DIALOG_LOCAL_PARAM(dialog_name)) \
{ \
	int return_items; \
\
	return_items=0; \
	ENTER(dialog_name ## _ ## list_name ## _get_num_items); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->list_name) \
		{ \
				return_items=ListBox_GetCount(temp_dialog->list_name); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_num_items.  %s", \
				"Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_num_items.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_items); \
} /* dialog_name ## _ ## list_name ## _get_num_items */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
#define DECLARE_GUI_LIST_GET_SELECTED_ITEMS(dialog_name,list_name) \
int *dialog_name ## _ ## list_name ## _get_selected_items( \
	DIALOG_LOCAL_PARAM(dialog_name),int *num_items_address) \
{ \
	int *selected_items; \
\
	selected_items=(int *)NULL; \
	*num_items_address=0; \
	ENTER(dialog_name ## _ ## list_name ## _get_selected_items); \
	if (temp_dialog&&num_items_address) \
	{ \
		if (temp_dialog->list_name) \
		{ \
			*num_items_address=ListBox_GetSelCount(temp_dialog->list_name); \
			if (ALLOCATE(selected_items,int,*num_items_address)) \
			{ \
				ListBox_GetSelItems(temp_dialog->list_name, \
					*num_items_address,selected_items); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"dialog_name ## _ ## list_name ## _get_selected_items.  %s", \
					"Could not allocate memory for list"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_selected_items.  %s", \
				"Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_selected_items.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (selected_items); \
} /* dialog_name ## _ ## list_name ## _get_selected_items */
#endif /* defined (WINDOWS) */

#if defined (MOTIF)
int dialog_name ## _ ## list_name ## _get_selected(DIALOG_PARAM( \
	Command_window)) \
/***************************************************************************** \
LAST MODIFIED : 15 January 1997 \
\
DESCRIPTION : \
Adds an item to a list. \
============================================================================*/ \
{ \
	int return_item; \
\
	return_item= -1; \
	ENTER(dialog_name ## _ ## list_name ## _get_selected); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->command_history) \
		{ \
			display_message(WARNING_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_selected.  %s", \
				"Not implemented"); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_selected.  %s","Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_selected.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_item); \
} /* dialog_name ## _ ## list_name ## _get_selected */
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
/*******************************************************************************
LAST MODIFIED : 15 January 1997

DESCRIPTION :
Returns the index of the selected item.
==============================================================================*/
#define DECLARE_GUI_LIST_GET_SELECTED(dialog_name,list_name) \
int dialog_name ## _ ## list_name ## _get_selected( \
	DIALOG_LOCAL_PARAM(dialog_name)) \
{ \
	int return_item; \
\
	return_item= -1; \
	ENTER(dialog_name ## _ ## list_name ## _get_selected); \
	if (temp_dialog) \
	{ \
		if (temp_dialog->list_name) \
		{ \
			return_item=ListBox_GetCurSel(temp_dialog->list_name); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"dialog_name ## _ ## list_name ## _get_selected.  %s","Invalid list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"dialog_name ## _ ## list_name ## _get_selected.  %s", \
			"Invalid arguments"); \
	} \
	LEAVE; \
\
	return (return_item); \
} /* dialog_name ## _ ## list_name ## _get_selected */ 
#endif /* defined (WINDOWS) */

#endif /* !defined (GUI_LIST_H) */

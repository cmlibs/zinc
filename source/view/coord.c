/*******************************************************************************
FILE : coord.c

LAST MODIFIED : 13 December 1996

DESCRIPTION :
This module creates a free coord input device, using two dof3, two control and
one input widget.  The position is given relative to some coordinate system,
and the returned value is a global one.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "command/parser.h"
	/*???DB.  required for set_Cmgui_coordinate .  Should it be here ? */
#include "general/debug.h"
#include "general/list_private.h"
#include "general/object.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/coord.h"
#include "view/coord.uid64"

/*
Global variables
----------------
*/
struct LIST(Cmgui_coordinate) *cmgui_coordinate_list;
struct LIST(Coord_widget) *coord_widget_list;
/*???DB.  Is there a better/more self contained way of doing this ? */
struct Cmgui_coordinate *global_coordinate_ptr;
struct Cmgui_coordinate *poi_coordinate_ptr;

/*
Module types
------------
*/
FULL_DECLARE_LIST_TYPE(Cmgui_coordinate);
FULL_DECLARE_LIST_TYPE(Coord_widget);
FULL_DECLARE_LIST_TYPE(Coord_data);

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int coord_hierarchy_open=0;
static MrmHierarchy coord_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static int compare_widget(Widget widget_1,Widget widget_2)
/*******************************************************************************
LAST MODIFIED : 27 April 1995

DESCRIPTION :
Returns -1 if widget_1 < widget_2, 0 if widget_1=widget_2 and 1 if widget_1 >
widget_2.
==============================================================================*/
{
	int return_code;

	ENTER(compare_widget);
	if (widget_1<widget_2)
	{
		return_code= -1;
	}
	else
	{
		if (widget_1>widget_2)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* compare_widget */

static int compare_transformation(struct Cmgui_coordinate *transformation_1,
	struct Cmgui_coordinate *transformation_2)
/*******************************************************************************
LAST MODIFIED : 27 April 1995

DESCRIPTION :
Returns -1 if transformation_1 < transformation_2, 0 if transformation_1 =
transformation_2 and 1 if transformation_1 > transformation_2.
==============================================================================*/
{
	int return_code;

	ENTER(compare_transformation);
	if (transformation_1<transformation_2)
	{
		return_code= -1;
	}
	else
	{
		if (transformation_1>transformation_2)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* compare_transformation */

DECLARE_DEFAULT_DESTROY_OBJECT_FUNCTION(Cmgui_coordinate)
DECLARE_DEFAULT_DESTROY_OBJECT_FUNCTION(Coord_widget)
DECLARE_DEFAULT_DESTROY_OBJECT_FUNCTION(Coord_data)
DECLARE_OBJECT_FUNCTIONS(Cmgui_coordinate)
DECLARE_LIST_FUNCTIONS(Cmgui_coordinate)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmgui_coordinate,name,char *,strcmp)
DECLARE_OBJECT_FUNCTIONS(Coord_data)
DECLARE_LIST_FUNCTIONS(Coord_data)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Coord_data,coord,
	struct Cmgui_coordinate *,compare_transformation)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Coord_data,button,Widget,
	compare_widget)
DECLARE_OBJECT_FUNCTIONS(Coord_widget)
DECLARE_LIST_FUNCTIONS(Coord_widget)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Coord_widget,coord_widget,Widget,
	compare_widget)

static void coord_update(struct Coord_struct *temp_coord)
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(coord_update);
	if (temp_coord->callback_array[COORD_UPDATE_CB].procedure)
	{
		/* now call the procedure with the user data and the coordinate data */
		(temp_coord->callback_array[COORD_UPDATE_CB].procedure)
			(temp_coord->widget,
			temp_coord->callback_array[COORD_UPDATE_CB].data,
			temp_coord->current_value);
	}
	LEAVE;
} /* coord_update */

static void coord_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the buttons on the coord widget.
==============================================================================*/
{
	struct Coord_struct *temp_coord;

	ENTER(coord_identify_button);
	switch (button_num)
	{
		case coord_menu_ID:
		{
			/* find out which coord widget we are in */
			XtVaGetValues(w,XmNuserData,&temp_coord,NULL);
			temp_coord->menu=w;
		}; break;
		default:
		{
		display_message(WARNING_MESSAGE,
			"coord_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* coord_identify_button */

static void coord_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the coordment dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Coord_struct *temp_coord;

	ENTER(coord_destroy_CB);
	/* Get the pointer to the data for the coord widget */
	XtVaGetValues(w,XmNuserData,&temp_coord,NULL);
	/* clear the list of widgets */
	DESTROY_LIST(Coord_data)(&temp_coord->coord_list);
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	DEALLOCATE(temp_coord);
	LEAVE;
} /* coord_destroy_CB */

static void coord_button_CB(Widget w, int *tag, XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the coord widget - informs whoever which coordinate system we are
now using.
==============================================================================*/
{
	struct Coord_struct *temp_coord;
	struct Coord_data *temp_data;

	ENTER(coord_button_CB);
	XtVaGetValues(XtParent(w),XmNuserData,&temp_coord,NULL);
	if (temp_data=FIND_BY_IDENTIFIER_IN_LIST(Coord_data,button)(w,
		temp_coord->coord_list))
	{
		temp_coord->current_value=temp_data->coord;
		coord_update(temp_coord);
	}
	else
	{
		display_message(ERROR_MESSAGE,"coord_button_CB.  Could not find widget");
	}

	LEAVE;
} /* coord_button_CB */

static int coord_add_button(struct Cmgui_coordinate *new_Cmgui_coordinate,
	void *void_temp_coord)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Adds the coordinate system pointed to by new_Cmgui_coord to the list
of coordinates held by the coord widget.
==============================================================================*/
{
	Arg override_arg;
	int return_code;
	MrmType coord_submenu_class;
	struct Coord_data *new_item;
	struct Coord_struct *temp_coord;
	Widget new_button;
	XmString temp_label;

	ENTER(coord_add_button);
	/* check arguments */
	if (new_Cmgui_coordinate&&(temp_coord=(struct Coord_struct *)void_temp_coord))
	{
		if (coord_hierarchy_open)
		{
			/* fetch the new button, and give it user data of the coord system */
			new_button=(Widget)NULL;
			XtSetArg(override_arg,XmNuserData,new_Cmgui_coordinate);
			if (MrmFetchWidgetOverride(coord_hierarchy,"coord_menu_item",
				temp_coord->menu,NULL,&override_arg,1,
				&new_button,&coord_submenu_class)==MrmSUCCESS)
			{
				XtManageChild(new_button);
				/* now we have to set the name */
				temp_label=XmStringCreateSimple(new_Cmgui_coordinate->name);
				XtVaSetValues(new_button,XmNlabelString,temp_label,NULL);
				XmStringFree(temp_label);
				/* add this widget to the list */
				if (ALLOCATE(new_item,struct Coord_data,1))
				{
					new_item->button=new_button;
					new_item->coord=new_Cmgui_coordinate;
					new_item->access_count=0;
					ADD_OBJECT_TO_LIST(Coord_data)(new_item,temp_coord->coord_list);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"coord_add_button.  Could not allocate new data structure");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"coord_add_button.  Could not fetch submenu widget");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"coord_add_button.  Hierarchy not open");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"coord_add_button.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* coord_add_button */

static void coord_remove_button(struct Coord_struct *temp_coord,
	struct Cmgui_coordinate *old_Cmgui_coordinate)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Adds the coordinate system pointed to by new_Cmgui_coord to the list
of coordinates held by the coord widget.
==============================================================================*/
{
	struct Coord_data *old_item,*new_item;
	Widget current_button;

	ENTER(coord_remove_button);
	/* find out where the coordinate is in the list */
	if (old_item=FIND_BY_IDENTIFIER_IN_LIST(Coord_data,coord)(
		old_Cmgui_coordinate,temp_coord->coord_list))
	{
		/* this assumes there will always be a global system */
		XtVaGetValues(temp_coord->widget,XmNmenuHistory,
			&current_button,NULL);
		/* is this button the current button */
		if (current_button==old_item->button)
		{
			/* set it to the global system */
			if (new_item=FIRST_OBJECT_IN_LIST_THAT(Coord_data)(
				(LIST_CONDITIONAL_FUNCTION(Coord_data) *)NULL,(void *)NULL,
				temp_coord->coord_list))
			{
				XtVaSetValues(temp_coord->widget,XmNmenuHistory,
					new_item->button,NULL);
				/* update the current value */
				temp_coord->current_value=new_item->coord;
				coord_update(temp_coord);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"coord_remove_button.  Missing global system\n");
			}
		}
		XtDestroyWidget(old_item->button);
		REMOVE_OBJECT_FROM_LIST(Coord_data)(old_item,temp_coord->coord_list);
	}
	else
	{
		display_message(WARNING_MESSAGE,"coord_remove_button.  Could not find button");
	}

	LEAVE;
} /* coord_remove_button */

static int add_coordinate_to_coord_widget(struct Coord_widget *coord_widget,
	void *void_new_coord)
/*******************************************************************************
LAST MODIFIED : 25 July 1995

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmgui_coordinate *new_coord;
	struct Coord_struct *temp_coord;

	ENTER(add_coordinate_to_coord_widget);
	/* check arguments */
	if (coord_widget&&(new_coord=(struct Cmgui_coordinate *)void_new_coord))
	{
		XtVaGetValues(coord_widget->coord_widget,XmNuserData,&temp_coord,NULL);
		coord_add_button(new_coord,(void *)temp_coord);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_coordinate_to_coord_widget .  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_coordinate_to_coord_widget */

static int remove_coordinate_from_coord_widget(
	struct Coord_widget *coord_widget,void *void_old_coord)
/*******************************************************************************
LAST MODIFIED : 25 July 1995

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmgui_coordinate *old_coord;
	struct Coord_struct *temp_coord;

	ENTER(remove_coordinate_from_coord_widget);
	/* check arguments */
	if (coord_widget&&(old_coord=(struct Cmgui_coordinate *)void_old_coord))
	{
		XtVaGetValues(coord_widget->coord_widget,XmNuserData,&temp_coord,NULL);
		coord_remove_button(temp_coord,old_coord);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"remove_coordinate_from_coord_widget .  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* remove_coordinate_from_coord_widget */

/*
Global functions
----------------
*/
Widget create_coord_widget(Widget parent)
/*******************************************************************************
LAST MODIFIED : 13 December 1995

DESCRIPTION :
Creates a coord widget that gets a position and orientation from the user.
==============================================================================*/
{
	int i;
	MrmType coord_dialog_class;
	struct Coord_data *coord_data;
	struct Coord_struct *temp_coord=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"coord_identify_button",(XtPointer)coord_identify_button},
		{"coord_destroy_CB",(XtPointer)coord_destroy_CB},
		{"coord_button_CB",(XtPointer)coord_button_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Coord_structure",(XtPointer)NULL},
		{"coord_menu_ID",(XtPointer)coord_menu_ID},
		{"coord_toggle_ID",(XtPointer)coord_toggle_ID}
	};
	Widget return_widget;

	ENTER(create_coord_widget);
	return_widget=(Widget)NULL;
	if (MrmOpenHierarchy_base64_string(coord_uid64,
		&coord_hierarchy,&coord_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_coord,struct Coord_struct,1))
		{
			/* initialise the structure */
			temp_coord->widget_parent=parent;
			temp_coord->widget=(Widget)NULL;
			temp_coord->menu=(Widget)NULL;
			temp_coord->current_value=(struct Cmgui_coordinate *)NULL;
			temp_coord->coord_list=CREATE_LIST(Coord_data)();
			for (i=0;i<COORD_NUM_CALLBACKS;i++)
			{
				temp_coord->callback_array[i].procedure=(Callback_procedure *)NULL;
				temp_coord->callback_array[i].data=NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(coord_hierarchy,callback_list,
				XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_coord;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(coord_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch position window widget */
					if (MrmSUCCESS==MrmFetchWidget(coord_hierarchy,"coord_widget",
						temp_coord->widget_parent,&(temp_coord->widget),
						&coord_dialog_class))
					{
						XtManageChild(temp_coord->widget);
						/* add ourself to the list of coord_widgets */
						coord_widget_add_widget(temp_coord->widget);
						/* ensure that the widget shows the current coord systems */
#if defined (OLD_CODE)
						TRAVERSE_LIST_STATEMENT(Cmgui_coordinate,cmgui_coordinate_list,
							coord_add_button,temp_coord)
#endif
						FOR_EACH_OBJECT_IN_LIST(Cmgui_coordinate)(coord_add_button,
							(void *)temp_coord,cmgui_coordinate_list);
						/* make sure that global is being shown first */
						if (coord_data=FIRST_OBJECT_IN_LIST_THAT(Coord_data)(
							(LIST_CONDITIONAL_FUNCTION(Coord_data) *)NULL,(void *)NULL,
							temp_coord->coord_list))
						{
							temp_coord->current_value=coord_data->coord;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_coord_widget.  Missing global system");
						}
						return_widget=temp_coord->widget;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_coord_widget.  Could not fetch coord dialog");
						DEALLOCATE(temp_coord);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_coord_widget.  Could not register identifiers");
					DEALLOCATE(temp_coord);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_coord_widget.  Could not register callbacks");
				DEALLOCATE(temp_coord);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_coord_widget.  Could not allocate coord widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_coord_widget.  Could not open hierarchy");
	}
	LEAVE;

	return (return_widget);
} /* create_coord_widget */

int coord_set_data(Widget coord_widget,enum Coord_data_type data_type,
	void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the coord widget.
==============================================================================*/
{
	int return_code;
	struct Coord_struct *temp_coord;
	struct Coord_data *temp_data;

	ENTER(coord_set_data);
	/* Get the pointer to the data for the coord dialog */
	XtVaGetValues(coord_widget,XmNuserData,&temp_coord,NULL);
	switch(data_type)
	{
		case COORD_UPDATE_CB:
		{
			temp_coord->callback_array[COORD_UPDATE_CB].procedure=
				((struct Callback_data *)data)->procedure;
			temp_coord->callback_array[COORD_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		}; break;
		case COORD_COORD_DATA:
		{
			temp_coord->current_value=(struct Cmgui_coordinate *)data;
			if (temp_data=FIND_BY_IDENTIFIER_IN_LIST(Coord_data,coord)(
				temp_coord->current_value,temp_coord->coord_list))
			{
				/* now make sure that that widget is displayed */
				XtVaSetValues(temp_coord->widget,XmNmenuHistory,temp_data->button,NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"coord_button_CB.  Could not find widget");
			}
			return_code=1;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,"coord_set_data.  Invalid data type");
			return_code=0;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* coord_set_data */

void *coord_get_data(Widget coord_widget,enum Coord_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the coord widget.
==============================================================================*/
{
	void *return_code;
	struct Coord_struct *temp_coord;
	static struct Callback_data dat_callback;
	static struct Cmgui_coordinate *dat_coord;

	ENTER(coord_get_data);
	/* Get the pointer to the data for the coord dialog */
	XtVaGetValues(coord_widget,XmNuserData,&temp_coord,NULL);
	switch(data_type)
	{
		case COORD_UPDATE_CB:
		{
			dat_callback.procedure=
				temp_coord->callback_array[COORD_UPDATE_CB].procedure;
			dat_callback.data=temp_coord->callback_array[COORD_UPDATE_CB].data;
			return_code= &dat_callback;
		}; break;
		case COORD_COORD_DATA:
		{
			return_code=temp_coord->current_value;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,"coord_get_data.  Invalid data type");
			return_code=NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* coord_get_data */

void coord_add_coordinate(Widget coord_widget,
	struct Cmgui_coordinate *new_Cmgui_coordinate)
/*******************************************************************************
LAST MODIFIED : 25 July 1995

DESCRIPTION :
This routine should only be used in special circumstances, as it will only
add the coordinate system to ONE widget.  This function acts as a wrapper
around coord_add_button.
==============================================================================*/
{
	struct Coord_struct *temp_coord;

	ENTER(coord_add_coordinate);
	/* Get the pointer to the data for the coord dialog */
	XtVaGetValues(coord_widget,XmNuserData,&temp_coord,NULL);
	coord_add_button(new_Cmgui_coordinate,(void *)temp_coord);
	LEAVE;
} /* coord_add_coordinate */

void coord_widget_init(void)
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Initialises the list of coord widgets.
==============================================================================*/
{
	int i;
	struct Cmgui_coordinate *temp_cmgui;

	cmgui_coordinate_list=CREATE_LIST(Cmgui_coordinate)();
	/* we must make sure that there is a global coordinate system */
	if (ALLOCATE(temp_cmgui,struct Cmgui_coordinate,1))
	{
		for (i=0;i<3;i++)
		{
			temp_cmgui->origin.position.data[i]=0.0;
			temp_cmgui->origin.direction.data[i]=0.0;
		}
		if (ALLOCATE(temp_cmgui->name,char,9))
		{
			strcpy(temp_cmgui->name,"Global");
		}
		temp_cmgui->access_count=0;
		global_coordinate_ptr=temp_cmgui;
		ADD_OBJECT_TO_LIST(Cmgui_coordinate)(temp_cmgui,cmgui_coordinate_list);
	}
	/* we must make sure that there is a poi coord system for reference */
	if (ALLOCATE(temp_cmgui,struct Cmgui_coordinate,1))
	{
		for (i=0;i<3;i++)
		{
			temp_cmgui->origin.position.data[i]=0.0;
			temp_cmgui->origin.direction.data[i]=0.0;
		}
		if (ALLOCATE(temp_cmgui->name,char,4))
		{
			strcpy(temp_cmgui->name,"POI");
		}
		temp_cmgui->access_count=0;
		poi_coordinate_ptr=temp_cmgui;
		/* does not get added to the list */
		ADD_OBJECT_TO_LIST(Cmgui_coordinate)(temp_cmgui,cmgui_coordinate_list);
	}
	coord_widget_list=CREATE_LIST(Coord_widget)();
} /* coord_widget_init */

void coord_widget_finish(void)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees up the list of coordinate systems.
==============================================================================*/
{
	DESTROY_LIST(Cmgui_coordinate)(&cmgui_coordinate_list);
	DESTROY_LIST(Coord_widget)(&coord_widget_list);
} /* coord_widget_finish */

void coord_widget_add_coord(struct Cmgui_coordinate *new_coord)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Adds a coordinate to all coord widgets.
==============================================================================*/
{
	ENTER(coord_widget_add_coord);
	ADD_OBJECT_TO_LIST(Cmgui_coordinate)(new_coord,cmgui_coordinate_list);
	/* now traverse the list of coord widgets, and add this coordinate to each */
	(void)FOR_EACH_OBJECT_IN_LIST(Coord_widget)(add_coordinate_to_coord_widget,
		(void *)new_coord,coord_widget_list);
	LEAVE;
} /* coord_widget_add_coord */

void coord_widget_delete_coord(struct Cmgui_coordinate *old_coord)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Removes a coordinate from all coord widgets.
==============================================================================*/
{
	ENTER(coord_widget_delete_coord);
	REMOVE_OBJECT_FROM_LIST(Cmgui_coordinate)(old_coord,cmgui_coordinate_list);
	/* now traverse the list of coord widgets, and remove this coordinate from
		each */
	(void)FOR_EACH_OBJECT_IN_LIST(Coord_widget)(
		remove_coordinate_from_coord_widget,(void *)old_coord,coord_widget_list);
	LEAVE;
} /* coord_widget_delete_coord */

void coord_widget_add_widget(Widget new_widget)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Adds a widget to the list of all coord_widgets.
==============================================================================*/
{
	struct Coord_widget *temp_struct;

	ENTER(coord_widget_add_widget);
	if (ALLOCATE(temp_struct,struct Coord_widget,1))
	{
		temp_struct->coord_widget=new_widget;
		temp_struct->access_count=0;
		ADD_OBJECT_TO_LIST(Coord_widget)(temp_struct,coord_widget_list);
	}
	LEAVE;
} /* coord_widget_add_widget */

void coord_widget_delete_widget(Widget old_widget)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Removes a widget from the list of all coord_widgets.
==============================================================================*/
{
	struct Coord_widget *temp_coord_widget;

	ENTER(coord_widget_delete_widget);
	temp_coord_widget=FIND_BY_IDENTIFIER_IN_LIST(Coord_widget,coord_widget)(
		old_widget,coord_widget_list);
	REMOVE_OBJECT_FROM_LIST(Coord_widget)(temp_coord_widget,coord_widget_list);
	LEAVE;
} /* coord_widget_delete_widget */

int set_Cmgui_coordinate(struct Parse_state *state,void *coordinate_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
Modifier function to set the coordinate from a command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmgui_coordinate *temp_coordinate,**coordinate_address;

	ENTER(set_Cmgui_coordinate);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (coordinate_address=(struct Cmgui_coordinate **)coordinate_void)
				{
					if (temp_coordinate=FIND_BY_IDENTIFIER_IN_LIST(Cmgui_coordinate,name)(
						current_token,cmgui_coordinate_list))
					{
						if (*coordinate_address!=temp_coordinate)
						{
							DEACCESS(Cmgui_coordinate)(coordinate_address);
							*coordinate_address=ACCESS(Cmgui_coordinate)(temp_coordinate);
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Coordinate does not exist: %s",
							current_token);
						return_code=0;
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Cmgui_coordinate.  Missing coordinate void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," NAME");
				/* if possible, then write the name */
				if (coordinate_address=(struct Cmgui_coordinate **)coordinate_void)
				{
					if (temp_coordinate= *coordinate_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_coordinate->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing coordinate name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Cmgui_coordinate.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Cmgui_coordinate */

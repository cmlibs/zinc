/*******************************************************************************
FILE : colour_editor.c

LAST MODIFIED : 26 November 2001

DESCRIPTION :
Creates a window that allows the user to create a colour.  Each colour component
ranges between 0-1.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#if defined (MOTIF)
#include <Xm/Text.h>
#endif /* defined (MOTIF) */
#include "colour/colour_editor.h"
#include "colour/colour_editor.uidh"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
/* UIL Identifiers */
#define colour_editor_text_ID      1
#define colour_editor_slider_ID    2
#define colour_editor_label_ID     3
#define colour_editor_patch_ID     4
#define colour_editor_option_ID    5
#define colour_editor_mode0_ID     6
#define colour_editor_mode1_ID     7
#define colour_editor_mode2_ID     8

/*
Module variables
----------------
*/
static char colour_editor_names[3][3][11]=
{
	{"Red","Green","Blue"},
	{"Hue","Saturation","Value"},
	{"Cyan","Magenta","Yellow"}
};
static int colour_editor_hierarchy_open=0;
static MrmHierarchy colour_editor_hierarchy;

struct Colour_editor_struct
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
Contains information required by the colour_editor control dialog.
==============================================================================*/
{
	enum Colour_editor_mode current_mode,return_mode; /* RGB/HSV etc */
	struct Colour current; /* values of the colour_editor components */
	Widget widget_parent,widget;
	Widget apply,cancel,label[3],ok,patch,slider[3],text[3];
	Widget mode[3],option;
	XColor colour;
	struct Callback_data update_callback;
	struct User_interface *user_interface;
}; /* Colour_Editor_struct */

/*
Module functions
----------------
*/
COLOUR_PRECISION min(COLOUR_PRECISION *data,int number)
/*******************************************************************************
LAST MODIFIED : 12 June 1994

DESCRIPTION :
Finds the minimum of the passed values.
==============================================================================*/
{
	COLOUR_PRECISION return_value;
	int i;

	ENTER(min);
	return_value=1.0e30;
	for (i=0;i<number;i++)
	{
		if (return_value>=data[i])
		{
			return_value=data[i];
		}
	}
	LEAVE;

	return (return_value);
} /* min */

COLOUR_PRECISION max(COLOUR_PRECISION *data,int number)
/*******************************************************************************
LAST MODIFIED : 12 June 1994

DESCRIPTION :
Finds the maximum of the passed values.
==============================================================================*/
{
	COLOUR_PRECISION return_value;
	int i;

	ENTER(max);
	return_value=-1.0e30;
	for (i=0;i<number;i++)
	{
		if (return_value<=data[i])
		{
			return_value=data[i];
		}
	}
	LEAVE;

	return (return_value);
} /* max */

static void colour_editor_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 12 June 1994

DESCRIPTION :
Finds the id of the buttons on the colour_editor control dialog box.
==============================================================================*/
{
	int combo_num;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_identify_button);
	USE_PARAMETER(reason);
	/* find out which colour_editor dialog we are in */
	XtVaGetValues(w,XmNuserData,&temp_colour_editor,NULL);
	switch (button_num)
	{
		case colour_editor_slider_ID:
		{
			/* first,get the user data of the parent */
			XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
			temp_colour_editor->slider[combo_num]=w;
		} break;
		case colour_editor_label_ID:
		{
			/* first,get the user data of the parent */
			XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
			temp_colour_editor->label[combo_num]=w;
		} break;
		case colour_editor_text_ID:
		{
			/* first,get the user data of the parent */
			XtVaGetValues(XtParent(w),XmNuserData,&combo_num,NULL);
			temp_colour_editor->text[combo_num]=w;
		} break;
		case colour_editor_patch_ID:
		{
			temp_colour_editor->patch=w;
		} break;
		case colour_editor_option_ID:
		{
			temp_colour_editor->option=w;
		} break;
		case colour_editor_mode0_ID:
		{
			temp_colour_editor->mode[0]=w;
		} break;
		case colour_editor_mode1_ID:
		{
			temp_colour_editor->mode[1]=w;
		} break;
		case colour_editor_mode2_ID:
		{
			temp_colour_editor->mode[2]=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"colour_editor_identify_button.  Invalid button number");
		} break;
	};
	LEAVE;
} /* colour_editor_identify_button */

static void colour_editor_conversion(enum Colour_editor_mode old_mode,
	enum Colour_editor_mode new_mode,struct Colour *old_data,
	struct Colour *new_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Converts between the different colour_editor modes.  Algorithms taken from
Foley and van Damm p592.
==============================================================================*/
{
	COLOUR_PRECISION temp_delta,temp_f,temp_old0,temp_p,temp_q,temp_t,temp_max,
		temp_min;
	int temp_i;

	ENTER(colour_editor_conversion);
	if (new_mode==old_mode)
	{
		new_data->red=old_data->red;
		new_data->green=old_data->green;
		new_data->blue=old_data->blue;
	}
	else
	{
		/* use nested case statements here as in future we may have more than
			two colour_editor types. */
		switch (old_mode)
		{
			case COLOUR_EDITOR_RGB:
			{
				switch (new_mode)
				{
					case COLOUR_EDITOR_HSV:
					{
						temp_min=min((COLOUR_PRECISION *)old_data,3);
						temp_max=max((COLOUR_PRECISION *)old_data,3);
						new_data->blue=temp_max;
						if (temp_max)
						{
							new_data->green=(temp_max-temp_min)/temp_max;
						}
						else
						{
							new_data->green=0;
						}
						if (new_data->green)
						{
							temp_delta=temp_max-temp_min;
							if (old_data->red==temp_max)
							{
								new_data->red=(old_data->green-old_data->blue)/
									temp_delta;
							}
							else if (old_data->green==temp_max)
							{
								new_data->red=2+(old_data->blue-old_data->red)/
									temp_delta;
							}
							else if (old_data->blue==temp_max)
							{
								new_data->red=4+(old_data->red-old_data->green)/
									temp_delta;
							}
							if (new_data->red<0)
							{
								new_data->red +=6;
							}
							new_data->red /=6; /* bring it down into the range 0-1 */
						}
						else
						{
							new_data->red=0; /* dont use undefined */
						}
					} break;
					case COLOUR_EDITOR_CMY:
					{
						new_data->red=1.0-old_data->red;
						new_data->green=1.0-old_data->green;
						new_data->blue=1.0-old_data->blue;
					} break;
				}
			} break;
			case COLOUR_EDITOR_HSV:
			{
				switch (new_mode)
				{
					case COLOUR_EDITOR_RGB:
					{
						if (old_data->green==0)
						{
							new_data->red=old_data->blue;
							new_data->green=old_data->blue;
							new_data->blue=old_data->blue;
						}
						else
						{
							temp_old0=old_data->red;
							if (temp_old0==1)
							{
								temp_old0=0;
							}
							temp_old0 *=6;
							temp_i=floor(temp_old0);
							temp_f=temp_old0-temp_i;
							temp_p=old_data->blue*(1-old_data->green);
							temp_q=old_data->blue*(1-(old_data->green*temp_f));
							temp_t=old_data->blue*(1-(old_data->green*(1-temp_f)));
							switch (temp_i)
							{
								case 0:
								{
									new_data->red=old_data->blue;
									new_data->green=temp_t;
									new_data->blue=temp_p;
								} break;
								case 1:
								{
									new_data->red=temp_q;
									new_data->green=old_data->blue;
									new_data->blue=temp_p;
								} break;
								case 2:
								{
									new_data->red=temp_p;
									new_data->green=old_data->blue;
									new_data->blue=temp_t;
								} break;
								case 3:
								{
									new_data->red=temp_p;
									new_data->green=temp_q;
									new_data->blue=old_data->blue;
								} break;
								case 4:
								{
									new_data->red=temp_t;
									new_data->green=temp_p;
									new_data->blue=old_data->blue;
								} break;
								case 5:
								{
									new_data->red=old_data->blue;
									new_data->green=temp_p;
									new_data->blue=temp_q;
								} break;
							}
						}
					} break;
					case COLOUR_EDITOR_CMY:
					{
						if (old_data->green==0)
						{
							new_data->red=1-old_data->blue;
							new_data->green=1-old_data->blue;
							new_data->blue=1-old_data->blue;
						}
						else
						{
							temp_old0=old_data->red;
							if (temp_old0==1)
							{
								temp_old0=0;
							}
							temp_old0 *=6;
							temp_i=floor(temp_old0);
							temp_f=temp_old0-temp_i;
							temp_p=old_data->blue*(1-old_data->green);
							temp_q=old_data->blue*(1-(old_data->green*temp_f));
							temp_t=old_data->blue*(1-(old_data->green*(1-temp_f)));
						}
						switch (temp_i)
						{
							case 0:
							{
								new_data->red=1-old_data->blue;
								new_data->green=1-temp_t;
								new_data->blue=1-temp_p;
							} break;
							case 1:
							{
								new_data->red=1-temp_q;
								new_data->green=1-old_data->blue;
								new_data->blue=1-temp_p;
							} break;
							case 2:
							{
								new_data->red=1-temp_p;
								new_data->green=1-old_data->blue;
								new_data->blue=1-temp_t;
							} break;
							case 3:
							{
								new_data->red=1-temp_p;
								new_data->green=1-temp_q;
								new_data->blue=1-old_data->blue;
							} break;
							case 4:
							{
								new_data->red=1-temp_t;
								new_data->green=1-temp_p;
								new_data->blue=1-old_data->blue;
							} break;
							case 5:
							{
								new_data->red=1-old_data->blue;
								new_data->green=1-temp_p;
								new_data->blue=1-temp_q;
							} break;
						}
					} break;
				}
			} break;
			case COLOUR_EDITOR_CMY:
			{
				switch (new_mode)
				{
					case COLOUR_EDITOR_RGB:
					{
						new_data->red=1.0-old_data->red;
						new_data->green=1.0-old_data->green;
						new_data->blue=1.0-old_data->blue;
					} break;
					case COLOUR_EDITOR_HSV:
					{
						temp_max=1-min((COLOUR_PRECISION *)old_data,3);
						temp_min=1.0-max((COLOUR_PRECISION *)old_data,3);
						new_data->blue=temp_max;
						if (temp_max)
						{
							new_data->green=(temp_max-temp_min)/temp_max;
						}
						else
						{
							new_data->green=0;
						}
						if (new_data->green)
						{
							temp_delta=temp_max-temp_min;
							if ((1.0-old_data->red)==temp_max)
							{
								new_data->red=-(old_data->green-old_data->blue)/
									temp_delta;
							}
							else if ((1.0-old_data->green)==temp_max)
							{
								new_data->red=2-(old_data->blue-old_data->red)/
									temp_delta;
							}
							else if ((1.0-old_data->blue)==temp_max)
							{
								new_data->red=4-(old_data->red-old_data->green)/
									temp_delta;
							}
							if (new_data->red<0)
							{
								new_data->red +=6;
							}
							new_data->red /=6; /* bring it down into the range 0-1 */
						}
						else
						{
							new_data->red=0; /* dont use undefined */
						}
					} break;
				}
			} break;
		}
	}
	LEAVE;
} /* colour_editor_conversion */

void colour_editor_update(struct Colour_editor_struct *temp_colour_editor)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	struct Colour colour_editor_temp;

	ENTER(colour_editor_update);
	/* Now send an update to the client if requested */
	if (temp_colour_editor->update_callback.procedure)
	{
		colour_editor_conversion(temp_colour_editor->current_mode,
			temp_colour_editor->return_mode,&temp_colour_editor->current,
			&colour_editor_temp);
		/* now call the procedure with the user data and the colour_editor data */
		(temp_colour_editor->update_callback.procedure)
			(temp_colour_editor->widget,
				temp_colour_editor->update_callback.data,
				&colour_editor_temp);
	}
	LEAVE;
} /* colour_editor_update */

static void colour_editor_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
Callback for the colour_editor dialog - tidies up all memory allocation
==============================================================================*/
{
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the colour_editor dialog */
	XtVaGetValues(w,XmNuserData,&temp_colour_editor,NULL);
	/* deallocate the memory for the user data */
	XFreeColors(temp_colour_editor->user_interface->display,
		DefaultColormapOfScreen(XtScreen(temp_colour_editor->widget_parent)),
		&temp_colour_editor->colour.pixel,
		1,0);
	DEALLOCATE(temp_colour_editor);
	LEAVE;
} /* colour_editor_destroy_CB */

static void colour_editor_update_value(
	struct Colour_editor_struct *temp_colour_editor,int combo_num)
/*******************************************************************************
LAST MODIFIED : 19 August 1995

DESCRIPTION :
Makes the numeric value and the slider agree for the particular combo.
==============================================================================*/
{
	char temp_str[20];
	COLOUR_PRECISION temp;
	int slider_value;

	ENTER(colour_editor_update_value);
	switch (combo_num)
	{
		case 0:
		{
			sprintf(temp_str,COLOUR_NUM_FORMAT,temp_colour_editor->current.red);
			XmTextSetString(temp_colour_editor->text[combo_num],temp_str);
			temp=temp_colour_editor->current.red*100.0;
		} break;
		case 1:
		{
			sprintf(temp_str,COLOUR_NUM_FORMAT,temp_colour_editor->current.green);
			XmTextSetString(temp_colour_editor->text[combo_num],temp_str);
			temp=temp_colour_editor->current.green*100.0;
		} break;
		case 2:
		{
			sprintf(temp_str,COLOUR_NUM_FORMAT,temp_colour_editor->current.blue);
			XmTextSetString(temp_colour_editor->text[combo_num],temp_str);
			temp=temp_colour_editor->current.blue*100.0;
		} break;
	}
	slider_value=(int)(temp+0.5);
	XtVaSetValues(temp_colour_editor->slider[combo_num],XmNvalue,slider_value,
		NULL);
	LEAVE;
} /* colour_editor_update_value */

static void colour_editor_set_names(
	struct Colour_editor_struct *temp_colour_editor)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Writes the correct name on the labels for each of the sliders.
==============================================================================*/
{
	int i;
	XmString temp_label;

	ENTER(colour_editor_set_names);
	for (i=0;i<3;i++)
	{
		temp_label=XmStringCreateSimple(
			colour_editor_names[temp_colour_editor->current_mode][i]);
		XtVaSetValues(temp_colour_editor->label[i],XmNlabelString,temp_label,NULL);
		XmStringFree(temp_label);
	}
	LEAVE;
} /* colour_editor_set_names */

static void colour_editor_update_colour_editor(
	struct Colour_editor_struct *temp_colour_editor)
/*******************************************************************************
LAST MODIFIED : 4 June 1996

DESCRIPTION :
Changes the colour_editor of the colour_editor patch to the current
colour_editor.
==============================================================================*/
{
	struct Colour colour_editor_temp;

	ENTER(colour_editor_update_colour_editor);
	XFreeColors(temp_colour_editor->user_interface->display,
		DefaultColormapOfScreen(XtScreen(temp_colour_editor->widget_parent)),
		&temp_colour_editor->colour.pixel,1,0);
	colour_editor_conversion(temp_colour_editor->current_mode,COLOUR_EDITOR_RGB,
		&temp_colour_editor->current,&colour_editor_temp);
	temp_colour_editor->colour.red=colour_editor_temp.red*65535;
	temp_colour_editor->colour.green=colour_editor_temp.green*65535;
	temp_colour_editor->colour.blue=colour_editor_temp.blue*65535;
	if (!XAllocColor(temp_colour_editor->user_interface->display,
		DefaultColormapOfScreen(XtScreen(temp_colour_editor->widget_parent)),
		&temp_colour_editor->colour))
	{
		display_message(WARNING_MESSAGE,
			"colour_editor_update_colour_editor:  Could not allocate colour");
	}
	XtVaSetValues(temp_colour_editor->patch,XmNbackground,
		temp_colour_editor->colour.pixel,NULL);
	LEAVE;
} /* colour_editor_update_colour_editor */

static void colour_editor_number_CB(Widget w,int *tag,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the text portion of the colour_editor dialog.  Determines which
button has been pressed,and then exits.
==============================================================================*/
{
	char *temp_str_ptr;
	COLOUR_PRECISION temp_value;
	int button_num;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_number_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the colour_editor dialog */
	XtVaGetValues(w,XmNuserData,&temp_colour_editor,NULL);
	/* Find out which combo the number is from */
	XtVaGetValues(XtParent(w),XmNuserData,&button_num,NULL);
	/* get the string */
	XtVaGetValues(w,XmNvalue,&temp_str_ptr,NULL);
	sscanf(temp_str_ptr,"%"COLOUR_PRECISION_STRING,&temp_value);
	XtFree(temp_str_ptr);
	if ((temp_value>=0.0)&&(temp_value<=1.0))
	{
		switch (button_num)
		{
			case 0:
			{
				temp_colour_editor->current.red=temp_value;
			} break;
			case 1:
			{
				temp_colour_editor->current.green=temp_value;
			} break;
			case 2:
			{
				temp_colour_editor->current.blue=temp_value;
			} break;
		}
	}
	/* now print it in the correct format */
	colour_editor_update_value(temp_colour_editor,button_num);
	colour_editor_update_colour_editor(temp_colour_editor);
	colour_editor_update(temp_colour_editor);
	LEAVE;
} /* colour_editor_number_CB */

static void colour_editor_slider_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the slider portion of the colour_editor dialog.  Determines which
button has been pressed,and then exits.
==============================================================================*/
{
	COLOUR_PRECISION temp_value;
	int button_num,temp_int;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_slider_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the colour_editor dialog */
	XtVaGetValues(w,XmNuserData,&temp_colour_editor,NULL);
	/* Find out which combo the number is from */
	XtVaGetValues(XtParent(w),XmNuserData,&button_num,NULL);
	/* get the string */
	XtVaGetValues(w,XmNvalue,&temp_int,NULL);
	temp_value=((COLOUR_PRECISION)temp_int)/100.0;
	if ((temp_value>=0.0)&&(temp_value<=1.0))
	{
		switch (button_num)
		{
			case 0:
			{
				temp_colour_editor->current.red=temp_value;
			} break;
			case 1:
			{
				temp_colour_editor->current.green=temp_value;
			} break;
			case 2:
			{
				temp_colour_editor->current.blue=temp_value;
			} break;
		}
	}
	/* now print it in the correct format */
	colour_editor_update_value(temp_colour_editor,button_num);
	colour_editor_update_colour_editor(temp_colour_editor);
	colour_editor_update(temp_colour_editor);
	LEAVE;
} /* colour_editor_slider_CB */

static void colour_editor_mode_CB(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the option menu - to change between RGB/HSV etc
==============================================================================*/
{
	enum Colour_editor_mode new_mode;
	int i;
	struct Colour new_data;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_mode_CB);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the colour_editor dialog */
	XtVaGetValues(w,XmNuserData,&temp_colour_editor,NULL);
	switch (button_num)
	{
		case colour_editor_mode0_ID:
		{
			new_mode=COLOUR_EDITOR_RGB;
		} break;
		case colour_editor_mode1_ID:
		{
			new_mode=COLOUR_EDITOR_HSV;
		} break;
		case colour_editor_mode2_ID:
		{
			new_mode=COLOUR_EDITOR_CMY;
		} break;
	}
	if (temp_colour_editor->current_mode!=new_mode)
	{
		colour_editor_conversion(temp_colour_editor->current_mode,new_mode,
			&temp_colour_editor->current,&new_data);
		temp_colour_editor->current_mode=new_mode;
		temp_colour_editor->current.red=new_data.red;
		temp_colour_editor->current.green=new_data.green;
		temp_colour_editor->current.blue=new_data.blue;
		for (i=0;i<3;i++)
		{
			colour_editor_update_value(temp_colour_editor,i);
		}
		colour_editor_set_names(temp_colour_editor);
		colour_editor_update_colour_editor(temp_colour_editor);
	}
	LEAVE;
} /* colour_editor_mode_CB */

/*
Global functions
----------------
*/
Widget create_colour_editor_widget(Widget parent,enum Colour_editor_mode mode,
	struct Colour *colour,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Create a colour_editor window that will allow the user to fully specify the
colour via rgb input ie. three sliders.  Initial data in <mode> format is
passed in <colour>.
???RC May now pass NULL instead of a colour. The editor will remain unmanaged
until a non-NULL colour is passed to it. See: colour_editor_set_colour.
==============================================================================*/
{
	int i;
	MrmType colour_editor_dialog_class;
	struct Colour_editor_struct *temp_colour_editor=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"colour_editor_identify_button",(XtPointer)colour_editor_identify_button},
		{"colour_editor_destroy_CB",(XtPointer)colour_editor_destroy_CB},
		{"colour_editor_slider_CB",(XtPointer)colour_editor_slider_CB},
		{"colour_editor_number_CB",(XtPointer)colour_editor_number_CB},
		{"colour_editor_mode_CB",(XtPointer)colour_editor_mode_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Colour_editor_structure",(XtPointer)NULL},
		{"colour_editor_text_ID",(XtPointer)colour_editor_text_ID},
		{"colour_editor_slider_ID",(XtPointer)colour_editor_slider_ID},
		{"colour_editor_label_ID",(XtPointer)colour_editor_label_ID},
		{"colour_editor_patch_ID",(XtPointer)colour_editor_patch_ID},
		{"colour_editor_option_ID",(XtPointer)colour_editor_option_ID},
		{"colour_editor_mode0_ID",(XtPointer)colour_editor_mode0_ID},
		{"colour_editor_mode1_ID",(XtPointer)colour_editor_mode1_ID},
		{"colour_editor_mode2_ID",(XtPointer)colour_editor_mode2_ID}
	};
	Widget return_widget;

	ENTER(create_colour_editor_widget);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(colour_editor_uidh,
			&colour_editor_hierarchy,&colour_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_colour_editor,struct Colour_editor_struct,1))
			{
				/* initialise the structure */
				temp_colour_editor->widget_parent=parent;
				temp_colour_editor->widget=(Widget)NULL;
				for (i=0;i<3;i++)
				{
					temp_colour_editor->slider[i]=(Widget)NULL;
					temp_colour_editor->label[i]=(Widget)NULL;
					temp_colour_editor->text[i]=(Widget)NULL;
				}
				temp_colour_editor->patch=(Widget)NULL;
				temp_colour_editor->ok=(Widget)NULL;
				temp_colour_editor->apply=(Widget)NULL;
				temp_colour_editor->cancel=(Widget)NULL;
				temp_colour_editor->option=(Widget)NULL;
				temp_colour_editor->mode[0]=(Widget)NULL;
				temp_colour_editor->mode[1]=(Widget)NULL;
				/* current colour should only be set by colour_editor_set_colour */
				temp_colour_editor->current.red=0;
				temp_colour_editor->current.green=0;
				temp_colour_editor->current.blue=0;
				temp_colour_editor->user_interface=user_interface;
				temp_colour_editor->update_callback.procedure=
					(Callback_procedure *)NULL;
				temp_colour_editor->update_callback.data=NULL;
				temp_colour_editor->current_mode=mode;
				temp_colour_editor->return_mode=mode;
				temp_colour_editor->colour.flags=DoRed|DoGreen|DoBlue;
				/* reserve a colour */
				XAllocColor(user_interface->display,
					DefaultColormapOfScreen(XtScreen(parent)),
					&(temp_colour_editor->colour));
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(colour_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)temp_colour_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(colour_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch colour_editor control widget */
						if (MrmSUCCESS==MrmFetchWidget(colour_editor_hierarchy,
							"colour_editor_dialog",temp_colour_editor->widget_parent,
							&(temp_colour_editor->widget),&colour_editor_dialog_class))
						{
							/* (un)managing the editor done in colour_editor_set_colour */
							/* XtManageChild(temp_colour_editor->widget); */
							/* ensure that the option menu shows the correct mode */
							XtVaSetValues(temp_colour_editor->option,XmNmenuHistory,
								temp_colour_editor->mode[temp_colour_editor->current_mode],
								NULL);
							/* display the correct names and values */
							colour_editor_set_names(temp_colour_editor);
							colour_editor_set_colour(temp_colour_editor->widget,colour);
#if defined (OLD_CODE)
							for (i=0;i<3;i++)
							{
								colour_editor_update_value(temp_colour_editor,i);
							}
							colour_editor_update_colour_editor(temp_colour_editor);
#endif /* defined (OLD_CODE) */
							return_widget=temp_colour_editor->widget;
						}
						else
						{
							display_message(ERROR_MESSAGE,
					"create_colour_editor_dialog.  Could not fetch colour_editor dialog");
							DEALLOCATE(temp_colour_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_colour_editor_dialog.  Could not register identifiers");
						DEALLOCATE(temp_colour_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_colour_editor_dialog.  Could not register callbacks");
					DEALLOCATE(temp_colour_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_colour_editor_dialog.  Could not allocate control window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_colour_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_colour_editor_dialog.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_colour_editor_widget */

int colour_editor_get_callback(Widget colour_editor_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns the update_callback for the colour editor widget.
==============================================================================*/
{
	int return_code;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_get_callback);
	/* check arguments */
	if (colour_editor_widget&&callback)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(colour_editor_widget,XmNuserData,&temp_colour_editor,NULL);
		if (temp_colour_editor)
		{
			callback->procedure=temp_colour_editor->update_callback.procedure;
			callback->data=temp_colour_editor->update_callback.data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"colour_editor_get_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"colour_editor_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* colour_editor_get_callback */

int colour_editor_set_callback(Widget colour_editor_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes the update_callback for the colour editor widget.
==============================================================================*/
{
	int return_code;
	struct Colour_editor_struct *temp_colour_editor=NULL;

	ENTER(colour_editor_set_callback);
	if (colour_editor_widget&&callback)
	{
		/* Get the pointer to the data for the colour_editor dialog */
		XtVaGetValues(colour_editor_widget,XmNuserData,&temp_colour_editor,NULL);
		if (temp_colour_editor)
		{
			temp_colour_editor->update_callback.procedure=callback->procedure;
			temp_colour_editor->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"colour_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"colour_editor_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* colour_editor_set_callback */

int colour_editor_get_colour(Widget colour_editor_widget,struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns into <colour> the value from the <colour_editor_widget>.
==============================================================================*/
{
	int return_code;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_get_colour);
	if (colour_editor_widget&&colour)
	{
		/* Get the pointer to the colour for the colour_editor dialog */
		XtVaGetValues(colour_editor_widget,XmNuserData,&temp_colour_editor,NULL);
		if (temp_colour_editor)
		{
			colour->red=temp_colour_editor->current.red;
			colour->green=temp_colour_editor->current.green;
			colour->blue=temp_colour_editor->current.blue;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"colour_editor_get_colour.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"colour_editor_get_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* colour_editor_get_colour */

int colour_editor_set_colour(Widget colour_editor_widget,struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Copies <colour> into the value in the <colour_editor_widget>.
==============================================================================*/
{
	int i,return_code;
	struct Colour_editor_struct *temp_colour_editor;

	ENTER(colour_editor_set_colour);
	if (colour_editor_widget)
	{
		/* Get the pointer to the colour for the colour_editor dialog */
		XtVaGetValues(colour_editor_widget,XmNuserData,&temp_colour_editor,NULL);
		if (temp_colour_editor)
		{
			if (colour)
			{
				colour_editor_conversion(temp_colour_editor->return_mode,
					temp_colour_editor->current_mode,colour,&temp_colour_editor->current);
				for (i=0;i<3;i++)
				{
					colour_editor_update_value(temp_colour_editor,i);
				}
				colour_editor_update_colour_editor(temp_colour_editor);
				XtManageChild(temp_colour_editor->widget);
			}
			else
			{
				XtUnmanageChild(temp_colour_editor->widget);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"colour_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"colour_editor_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* colour_editor_set_colour */


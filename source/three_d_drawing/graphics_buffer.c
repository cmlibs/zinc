/*******************************************************************************
FILE : graphics_buffer.c

LAST MODIFIED : 1 July 2002

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
Should be merged with dm_interface.c
******************************************************************************/
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "three_d_drawing/ThreeDDraw.h"
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#if !defined (TEST_WITH_GTK_DRAWING_AREA)
#include <gtkgl/gtkglarea.h>
#endif /* !defined (TEST_WITH_GTK_DRAWING_AREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#if defined (GTK_USER_INTERFACE)
static GtkWidget *share_glarea = (GtkWidget *)NULL;
#endif /* defined (GTK_USER_INTERFACE) */

/*
Module types
------------
*/

FULL_DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, \
	struct Graphics_buffer *, void *);

FULL_DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, \
	struct Graphics_buffer *, struct Graphics_buffer_input *);

struct Graphics_buffer
{
	enum Graphics_buffer_type type;
	int access_count;

	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback))
		  *initialise_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback))
		  *resize_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback))
		  *expose_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_input_callback))
		  *input_callback_list;

/* For GRAPHICS_BUFFER_X3D_TYPE */
#if defined (MOTIF)
	Widget drawing_widget;
	Widget parent;
#endif /* defined (MOTIF) */

/* For GRAPHICS_BUFFER_GTKGLAREA_TYPE */
#if defined (GTK_USER_INTERFACE)
	GtkWidget *glarea;
#endif /* defined (MOTIF) */
};

/*
Module functions
----------------
*/

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Graphics_buffer_callback)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Graphics_buffer_callback, \
	struct Graphics_buffer *,void *)

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Graphics_buffer_input_callback)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Graphics_buffer_input_callback, \
	struct Graphics_buffer *, struct Graphics_buffer_input *)

DECLARE_OBJECT_FUNCTIONS(Graphics_buffer)

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
static struct Graphics_buffer *CREATE(Graphics_buffer)(void)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
This is static as it is designed to be called by the different constructors
contained in the this module only.
==============================================================================*/
{
	struct Graphics_buffer *buffer;

	ENTER(CREATE(Graphics_buffer));

	if (ALLOCATE(buffer, struct Graphics_buffer, 1))
	{
		buffer->type = GRAPHICS_BUFFER_INVALID_TYPE;
		buffer->access_count = 0;

		buffer->initialise_callback_list=
			CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))();
		buffer->resize_callback_list=
			CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))();
		buffer->expose_callback_list=
			CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))();
		buffer->input_callback_list=
			CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_input_callback)))();

/* For GRAPHICS_BUFFER_X3D_TYPE */
#if defined (MOTIF)
		buffer->drawing_widget = (Widget)NULL;
		buffer->parent = (Widget)NULL;
#endif /* defined (MOTIF) */

/* For GRAPHICS_BUFFER_GTKGLAREA_TYPE */
#if defined (GTK_USER_INTERFACE)
		buffer->glarea = (GtkWidget *)NULL;
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_buffer). Unable to allocate buffer structure");
		buffer = (struct Graphics_buffer *)NULL;
	}

	LEAVE;
	return (buffer);
} /* CREATE(Graphics_buffer) */
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

#if defined (MOTIF)
static void Graphics_buffer_X3d_initialize_callback(Widget graphics_buffer_widget,
	XtPointer graphics_buffer_structure, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
This is the configuration callback for the GL widget.
Sets the initial viewing transform for the graphics_buffer.
???RC Needed at all?
???RC Move functionality elsewhere?
???RC Need MakeCurrent?
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(graphics_buffer_X3d_initialize_callback);
	USE_PARAMETER(call_data);
	USE_PARAMETER(graphics_buffer_widget);
	if (graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
	{
		CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->initialise_callback_list, graphics_buffer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_X3d_initialize_callback.  Missing graphics_buffer");
	}
	LEAVE;
} /* Graphics_buffer_X3d_initialize_callback */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Graphics_buffer_X3d_resize_callback(Widget graphics_buffer_widget,
	XtPointer graphics_buffer_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is resized. All it does is notify
callbacks interested in the graphics_buffers transformations.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;
	X3dThreeDDrawCallbackStruct *resize_callback_data;

	ENTER(graphics_buffer_X3d_resize_callback);
	if (graphics_buffer_widget&&
		(graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)&&
		(resize_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_RESIZE==resize_callback_data->reason))
	{
		CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->resize_callback_list, graphics_buffer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_X3d_resize_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* graphics_buffer_X3d_resize_callback */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Graphics_buffer_X3d_expose_callback(Widget graphics_buffer_widget,
	XtPointer graphics_buffer_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is exposed. Does not attempt to
redraw just the exposed area. Instead, it redraws the whole picture, but only
if there are no more expose events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;
	XExposeEvent *expose_event;
	X3dThreeDDrawCallbackStruct *expose_callback_data;

	ENTER(graphics_buffer_X3d_expose_callback);
	if (graphics_buffer_widget&&
		(graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)&&
		(expose_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_EXPOSE==expose_callback_data->reason)&&
		(expose_event=(XExposeEvent *)(expose_callback_data->event)))
	{
		/* if no more expose events in series */
		if (0==expose_event->count)
		{
			CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
				graphics_buffer->expose_callback_list, graphics_buffer, NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_X3d_expose_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* graphics_buffer_X3d_expose_callback */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Graphics_buffer_X3d_input_callback(Widget graphics_buffer_widget,
	XtPointer graphics_buffer_structure, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
The callback for mouse or keyboard input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;
	XButtonEvent *button_event;
	XEvent *event;
	XKeyEvent *key_event;
	XMotionEvent *motion_event;
	X3dThreeDDrawCallbackStruct *input_callback_data;

	ENTER(Graphics_buffer_input_callback);
	USE_PARAMETER(graphics_buffer_widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)&&
		(input_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_INPUT==input_callback_data->reason)&&
		(event=(XEvent *)(input_callback_data->event)))
	{
		return_code = 1;
		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
		input.button_number = 0;
		input.key_code = 0;
		input.position_x = 0;
		input.position_y = 0;
		input.input_modifier = (enum Graphics_buffer_input_modifier)0;
		switch(event->type)
		{
			case MotionNotify:
			{
				input.type = GRAPHICS_BUFFER_MOTION_NOTIFY;
				motion_event = &(event->xmotion);
				input.position_x = motion_event->x;
				input.position_y = motion_event->y;
				if (ShiftMask&(motion_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
				}
				if (ControlMask&(motion_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
				}
				if (Mod1Mask&(motion_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
				}
				if (Button1Mask&(motion_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
				}
			} break;
			case ButtonPress:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
				button_event = &(event->xbutton);
				input.button_number = button_event->button;
				input.position_x = button_event->x;
				input.position_y = button_event->y;
				input.input_modifier = (enum Graphics_buffer_input_modifier)0;
				if (ShiftMask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
				}
				if (ControlMask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
				}
				if (Mod1Mask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
				}
				if (Button1Mask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
				}
			} break;
			case ButtonRelease:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
				button_event = &(event->xbutton);
				input.button_number = button_event->button;
				input.position_x = button_event->x;
				input.position_y = button_event->y;
				if (ShiftMask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
				}
				if (ControlMask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
				}
				if (Mod1Mask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
				}
				if (Button1Mask&(button_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
				}
			} break;
			case KeyPress:
			{
				input.type = GRAPHICS_BUFFER_KEY_PRESS;
				key_event= &(event->xkey);
				input.key_code = key_event->keycode;
				if (ShiftMask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
				}
				if (ControlMask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
				}
				if (Mod1Mask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
				}
				if (Button1Mask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
				}
			} break;
			case KeyRelease:
			{
				input.type = GRAPHICS_BUFFER_KEY_RELEASE;
				key_event= &(event->xkey);
				input.key_code = key_event->keycode;
				if (ShiftMask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
				}
				if (ControlMask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
				}
				if (Mod1Mask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
				}
				if (Button1Mask&(key_event->state))
				{
					input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_buffer_input_callback.  Unknown X event");
				return_code=0;
				/* This event type is not being passed on */
			} break;
		}
		if (return_code)
		{
			CMISS_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_input_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_buffer_input_callback */
#endif /* defined (MOTIF) */

#if defined (GTK_USER_INTERFACE)
static void Graphics_buffer_gtkglarea_initialise_callback(GtkWidget *widget,
	gpointer graphics_buffer_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is initialised. Does not attempt to
redraw just the initialised area. Instead, it redraws the whole picture, but only
if there are no more initialise events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(graphics_buffer_gtkglarea_initialise_callback);
	if (widget && (graphics_buffer = (struct Graphics_buffer *)graphics_buffer_void))
	{
		CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->initialise_callback_list, graphics_buffer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_gtkglarea_initialise_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* graphics_buffer_gtkglarea_initialise_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static void Graphics_buffer_gtkglarea_resize_callback(GtkWidget *widget,
	GtkAllocation *allocation, gpointer graphics_buffer_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is resized. Does not attempt to
redraw just the resized area. Instead, it redraws the whole picture, but only
if there are no more resize events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(graphics_buffer_gtkglarea_resize_callback);
	USE_PARAMETER(allocation);
	if (widget && (graphics_buffer = (struct Graphics_buffer *)graphics_buffer_void))
	{
		CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->resize_callback_list, graphics_buffer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_gtkglarea_resize_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* graphics_buffer_gtkglarea_resize_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_expose_callback(GtkWidget *widget,
	GdkEventExpose *expose_event, gpointer graphics_buffer_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is exposed. Does not attempt to
redraw just the exposed area. Instead, it redraws the whole picture, but only
if there are no more expose events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(graphics_buffer_gtkglarea_expose_callback);
	USE_PARAMETER(expose_event);
	if (widget && (graphics_buffer = (struct Graphics_buffer *)graphics_buffer_void))
	{
		if (0 == expose_event->count)
		{
			CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
				graphics_buffer->expose_callback_list, graphics_buffer, NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_gtkglarea_expose_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* graphics_buffer_gtkglarea_expose_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_button_callback(GtkWidget *widget,
	GdkEventButton *button_event, gpointer graphics_buffer_structure)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for mouse button input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_gtkglarea_button_callback);
	USE_PARAMETER(widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
		&& button_event)
	{
		return_code = 1;
		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
		switch(button_event->type)
		{
			case GDK_BUTTON_PRESS:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
			} break;
			case GDK_BUTTON_RELEASE:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_buffer_gtkglarea_button_callback.  Unknown button event");
				return_code=0;
				/* This event type is not being passed on */
			} break;
		}
		input.key_code = 0;
		input.button_number = button_event->button;
		input.position_x = button_event->x;
		input.position_y = button_event->y;
		input.input_modifier = (enum Graphics_buffer_input_modifier)0;
		if (GDK_SHIFT_MASK&(button_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (GDK_CONTROL_MASK&(button_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (GDK_MOD1_MASK&(button_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (GDK_BUTTON1_MASK&(button_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		if (return_code)
		{
			CMISS_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_button_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_gtkglarea_button_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_key_callback(GtkWidget *widget,
	GdkEventKey *key_event, gpointer graphics_buffer_structure)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for key input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_gtkglarea_key_callback);
	USE_PARAMETER(widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
		&& key_event)
	{
		return_code = 1;
		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
		switch(key_event->type)
		{
			case GDK_KEY_PRESS:
			{
				input.type = GRAPHICS_BUFFER_KEY_PRESS;
			} break;
			case GDK_KEY_RELEASE:
			{
				input.type = GRAPHICS_BUFFER_KEY_RELEASE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_buffer_gtkglarea_key_callback.  Unknown key event");
				return_code=0;
				/* This event type is not being passed on */
			} break;
		}
		input.button_number = 0;
		input.key_code = 0;
		input.position_x = 0;
		input.position_y = 0;
		input.input_modifier = (enum Graphics_buffer_input_modifier)0;
		if (GDK_SHIFT_MASK&(key_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (GDK_CONTROL_MASK&(key_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (GDK_MOD1_MASK&(key_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (GDK_BUTTON1_MASK&(key_event->state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		if (return_code)
		{
			CMISS_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_key_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_gtkglarea_key_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_motion_notify_callback(
	GtkWidget *widget,
	GdkEventMotion *motion_event, gpointer graphics_buffer_structure)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for mouse button input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	GdkModifierType state;
	int return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_gtkglarea_motion_notify_callback);
	USE_PARAMETER(widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
		&& motion_event)
	{
		return_code = 1;
		input.button_number = 0;
		input.type = GRAPHICS_BUFFER_MOTION_NOTIFY;
		input.key_code = 0;
		if (motion_event->is_hint)
		{
			gdk_window_get_pointer(motion_event->window, &input.position_x,
				&input.position_y, &state);
		}
		else
		{
			input.position_x = motion_event->x;
			input.position_y = motion_event->y;
			state = motion_event->state;
		}
		input.input_modifier = (enum Graphics_buffer_input_modifier)0;
		if (GDK_SHIFT_MASK&(state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (GDK_CONTROL_MASK&(state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (GDK_MOD1_MASK&(state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (GDK_BUTTON1_MASK&(state))
		{
			input.input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		if (return_code)
		{
			CMISS_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_motion_notify_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_gtkglarea_motion_notify_callback */
#endif /* defined (GTK_USER_INTERFACE) */

/*
Global functions
----------------
*/

#if defined (MOTIF)
struct Graphics_buffer *create_Graphics_buffer_X3d(Widget parent,
	X3dBufferColourMode colour_mode, X3dBufferingMode buffer_mode,
	int specified_visual_id)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
If <specified_visual_id> is not zero then this visual is required.
==============================================================================*/
{
	struct Graphics_buffer *buffer;

	ENTER(create_Graphics_buffer_X3d);

	if (buffer = CREATE(Graphics_buffer)())
	{
		if (buffer->drawing_widget=XtVaCreateWidget("cm_graphics_buffer_area",
			threeDDrawingWidgetClass,parent,
			X3dNbufferColourMode,colour_mode,
			X3dNbufferingMode,buffer_mode,
			X3dNvisualId, specified_visual_id,
			XmNleftAttachment,XmATTACH_FORM,
			XmNrightAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM,
			XmNtopAttachment,XmATTACH_FORM,
			XmNborderWidth,0,
			XmNleftOffset,0,
			XmNrightOffset,0,
			XmNbottomOffset,0,
			XmNtopOffset,0,
			NULL))
		{
			buffer->type = GRAPHICS_BUFFER_X3D_TYPE;
			buffer->parent = parent;
			XtAddCallback(buffer->drawing_widget ,X3dNinitializeCallback,
				Graphics_buffer_X3d_initialize_callback, buffer);
			XtAddCallback(buffer->drawing_widget, X3dNresizeCallback,
				Graphics_buffer_X3d_resize_callback, buffer);
			XtAddCallback(buffer->drawing_widget, X3dNexposeCallback,
				Graphics_buffer_X3d_expose_callback, buffer);
			XtAddCallback(buffer->drawing_widget, X3dNinputCallback,
				Graphics_buffer_X3d_input_callback, buffer);
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_X3d.  "
				"Unable to initialise X3D widget.");
			DESTROY(Graphics_buffer)(&buffer);
			buffer = (struct Graphics_buffer *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_X3d.  "
			"Unable to create generic Graphics_buffer.");				
		buffer = (struct Graphics_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_X3d */
#endif /* defined (MOTIF) */

#if defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_gtkglarea(GtkContainer *parent)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
==============================================================================*/
{
#if !defined (TEST_WITH_GTK_DRAWING_AREA)
	GtkGLArea *share;
	int attrlist[] = {
		GDK_GL_RGBA,
		GDK_GL_DOUBLEBUFFER,
		GDK_GL_BUFFER_SIZE, 24,
		GDK_GL_ACCUM_RED_SIZE, 16,
		GDK_GL_ACCUM_GREEN_SIZE, 16,
		GDK_GL_ACCUM_BLUE_SIZE, 16,
		GDK_GL_DEPTH_SIZE, 8,
		GDK_GL_NONE
	};
#endif /* !defined (TEST_WITH_GTK_DRAWING_AREA) */
	struct Graphics_buffer *buffer;

	ENTER(create_Graphics_buffer_gtkglarea);

#if !defined (TEST_WITH_GTK_DRAWING_AREA)
	if (gdk_gl_query() == TRUE)
	{
#endif /* !defined (TEST_WITH_GTK_DRAWING_AREA) */
		if (buffer = CREATE(Graphics_buffer)())
		{
#if defined (TEST_WITH_GTK_DRAWING_AREA)
			if (buffer->glarea = gtk_drawing_area_new())
			{
				g_object_set_data(G_OBJECT(buffer->glarea), "graphics_buffer_ptr",
					(gpointer)buffer);
#else /* defined (TEST_WITH_GTK_DRAWING_AREA) */
			if (share_glarea)
			{
				share = GTK_GL_AREA(share_glarea);
			}
			else
			{
				share = (GtkGLArea *)NULL;
			}
			if (buffer->glarea = gtk_gl_area_share_new(attrlist, share))
			{
				if (!share_glarea)
				{
					share_glarea = buffer->glarea;
				}
#endif /* defined (TEST_WITH_GTK_DRAWING_AREA) */
				buffer->type = GRAPHICS_BUFFER_GTKGLAREA_TYPE;
				gtk_widget_set_events(GTK_WIDGET(buffer->glarea),
					GDK_EXPOSURE_MASK|GDK_POINTER_MOTION_MASK|GDK_POINTER_MOTION_HINT_MASK|
					GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|
					GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);
				g_signal_connect(G_OBJECT(buffer->glarea), "realize",
					G_CALLBACK(Graphics_buffer_gtkglarea_initialise_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "size-allocate",
					G_CALLBACK(Graphics_buffer_gtkglarea_resize_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "expose-event",
					G_CALLBACK(Graphics_buffer_gtkglarea_expose_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "button-press-event",
					G_CALLBACK(Graphics_buffer_gtkglarea_button_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "button-release-event",
					G_CALLBACK(Graphics_buffer_gtkglarea_button_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "key-press-event",
					G_CALLBACK(Graphics_buffer_gtkglarea_key_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "key-release-event",
					G_CALLBACK(Graphics_buffer_gtkglarea_key_callback),
					(gpointer)buffer);
				g_signal_connect(G_OBJECT(buffer->glarea), "motion-notify-event",
					G_CALLBACK(Graphics_buffer_gtkglarea_motion_notify_callback),
					(gpointer)buffer);
				gtk_container_add(parent, GTK_WIDGET(buffer->glarea));
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
					"Unable to create gtk gl area.");
				DESTROY(Graphics_buffer)(&buffer);
				buffer = (struct Graphics_buffer *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
				"Unable to create generic Graphics_buffer.");
			buffer = (struct Graphics_buffer *)NULL;
		}
#if !defined (TEST_WITH_GTK_DRAWING_AREA)
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
			"Gdk Open GL not supported.");				
		buffer = (struct Graphics_buffer *)NULL;
	}
#endif /* !defined (TEST_WITH_GTK_DRAWING_AREA) */
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_X3d */
#endif /* defined (GTK_USER_INTERFACE) */

int Graphics_buffer_make_current(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_make_current);

	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				X3dThreeDDrawingMakeCurrent(buffer->drawing_widget);
				return_code = 1;
			} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
#if defined (TEST_WITH_GTK_DRAWING_AREA)
#else /* defined (TEST_WITH_GTK_DRAWING_AREA) */
				gtk_gl_area_make_current(GTK_GL_AREA(buffer->glarea));
#endif /* defined (TEST_WITH_GTK_DRAWING_AREA) */
				return_code = 1;
			} break;
#endif /* defined (GTK_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
			"Graphics_bufffer missing.");				
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_make_current */

int Graphics_buffer_swap_buffers(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_swap_buffers);

	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				X3dThreeDDrawingSwapBuffers();
				return_code = 1;
			} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
#if defined (TEST_WITH_GTK_DRAWING_AREA)
#else /* defined (TEST_WITH_GTK_DRAWING_AREA) */
				gtk_gl_area_swapbuffers(GTK_GL_AREA(buffer->glarea));
#endif /* defined (TEST_WITH_GTK_DRAWING_AREA) */
				return_code = 1;
			} break;
#endif /* defined (GTK_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_swap_buffers.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_swap_buffers.  "
			"Graphics_bufffer missing.");				
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_swap_buffers */

int Graphics_buffer_make_read_current(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Sets this buffer to be the GLX source and the current ThreeDWindow (the one last
made current) to be the GLX destination.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_make_current);

	if (buffer)
	{
		switch (buffer->type)
		{
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
			"Graphics_bufffer missing.");				
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_make_current */

enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Returns the type of buffer represented by <buffer>.
==============================================================================*/
{
	enum Graphics_buffer_type return_code;

	ENTER(Graphics_buffer_get_type);
	if (buffer)
	{
		return_code = buffer->type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_type.  Invalid buffer");
		return_code=GRAPHICS_BUFFER_INVALID_TYPE;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_type */

int Graphics_buffer_get_width(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the width of buffer represented by <buffer>.
==============================================================================*/
{
	int width;

	ENTER(Graphics_buffer_get_width);
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				Dimension xwidth;

				XtVaGetValues(buffer->drawing_widget,
					XmNwidth,&xwidth,
					NULL);
				width = xwidth;
			} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				width = buffer->glarea->allocation.width;
			} break;
#endif /* defined (GTKGLAREA) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_width.  "
					"Graphics_bufffer type unknown or not supported.");				
				width = 0;				
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_width.  Invalid buffer");
		width = 0;
	}
	LEAVE;

	return (width);
} /* Graphics_buffer_get_width */

int Graphics_buffer_set_width(struct Graphics_buffer *buffer, int width)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the width of buffer represented by <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_set_width);
#if !defined (MOTIF)
	USE_PARAMETER(width);
#endif /* !defined (MOTIF) */
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				Dimension xwidth;

				xwidth = width;
				XtVaSetValues(buffer->drawing_widget,
					XmNwidth, xwidth,
					NULL);
				return_code = 1;
			} break;
#endif /* defined (MOTIF) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_set_width.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;				
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_width.  Invalid buffer");
		return_code = 1;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_set_width */

int Graphics_buffer_get_height(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the height of buffer represented by <buffer>.
==============================================================================*/
{
	int height;

	ENTER(Graphics_buffer_get_height);
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				Dimension xheight;

				XtVaGetValues(buffer->drawing_widget,
					XmNheight,&xheight,
					NULL);
				height = xheight;
			} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				height = buffer->glarea->allocation.height;
			} break;
#endif /* defined (GTKGLAREA) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_height.  "
					"Graphics_bufffer type unknown or not supported.");				
				height = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_height.  Invalid buffer");
		height = 0;
	}
	LEAVE;

	return (height);
} /* Graphics_buffer_get_height */

int Graphics_buffer_set_height(struct Graphics_buffer *buffer, int height)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the height of buffer represented by <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_set_height);
#if !defined (MOTIF)
	USE_PARAMETER(height);
#endif /* !defined (MOTIF) */
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				Dimension xheight;

				xheight = height;
				XtVaSetValues(buffer->drawing_widget,
					XmNheight, xheight,
					NULL);
				return_code = 1;
			} break;
#endif /* defined (MOTIF) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_set_height.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;				
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_height.  Invalid buffer");
		return_code = 1;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_set_height */

int Graphics_buffer_get_border_width(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Returns the border width of buffer represented by <buffer>.
==============================================================================*/
{
	int border_width;

	ENTER(Graphics_buffer_get_border_width);
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				Dimension xborder_width;

				XtVaGetValues(buffer->drawing_widget,
					XmNborderWidth,&xborder_width,
					NULL);
				border_width = xborder_width;
			} break;
#endif /* defined (MOTIF) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_border_width.  "
					"Graphics_bufffer type unknown or not supported.");				
				border_width = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_border_width.  Invalid buffer");
		border_width = 0;
	}
	LEAVE;

	return (border_width);
} /* Graphics_buffer_get_border_width */

int Graphics_buffer_set_border_width(struct Graphics_buffer *buffer, int border_width)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Sets the border width of buffer represented by <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_set_border_width);
#if !defined (MOTIF)
	USE_PARAMETER(border_width);
#endif /* !defined (MOTIF) */
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				Dimension xborder_width;

				xborder_width = border_width;
				/*XtVaSetValues(scene_viewer->drawing_widget,
				   XmNborderWidth, xborder_width,NULL);*/
				XtVaSetValues(buffer->drawing_widget,
					XmNleftOffset, xborder_width,
					XmNrightOffset, xborder_width,
					XmNbottomOffset, xborder_width,
					XmNtopOffset, xborder_width,NULL);
				return_code = 1;
			} break;
#endif /* defined (MOTIF) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_set_border_width.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;				
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_border_width.  Invalid buffer");
		return_code = 1;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_set_border_width */

int Graphics_buffer_is_visible(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
routine it will not bother rendering into it, allowing us to avoid rendering
into unmanaged or invisible widgets.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_is_visible);
	if (buffer)
	{
		return_code = 0;
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				return_code = XtIsManaged(buffer->drawing_widget)&&
					XtIsManaged(buffer->parent);
			} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (GTK_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_is_visible.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_is_visible.  Invalid buffer");
		return_code=GRAPHICS_BUFFER_INVALID_TYPE;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_is_visible */

int Graphics_buffer_awaken(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Activates the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		switch (buffer->type)
		{
#if defined (MOTIF)
			case GRAPHICS_BUFFER_X3D_TYPE:
			{
				XtManageChild(buffer->drawing_widget);
			} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				gtk_widget_show(buffer->glarea);
				return_code = 1;
			} break;
#endif /* defined (GTK_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_awaken.  "
					"Graphics_bufffer type unknown or not supported.");				
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_awaken.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_is_visible */

int Graphics_buffer_add_initialise_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an initialise callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->initialise_callback_list, initialise_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_initialise_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_initialise_callback */

int Graphics_buffer_add_resize_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an resize callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->resize_callback_list, resize_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_resize_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_resize_callback */

int Graphics_buffer_add_expose_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an expose callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->expose_callback_list, expose_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_expose_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_expose_callback */

int Graphics_buffer_add_input_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_input_callback) input_callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an input callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_input_callback)(
			buffer->input_callback_list, input_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_input_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_input_callback */

int DESTROY(Graphics_buffer)(struct Graphics_buffer **buffer_ptr)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Closes a Graphics buffer instance
x==============================================================================*/
{
	int return_code;
	struct Graphics_buffer *buffer;

	ENTER(DESTROY(Graphics_buffer));
	if (buffer_ptr && (buffer = *buffer_ptr))
	{
		return_code=1;

#if defined (GTK_USER_INTERFACE)
		if (share_glarea == buffer->glarea)
		{
			share_glarea = (GtkWidget *)NULL;
		}
#endif /* defined (GTK_USER_INTERFACE) */
		if (buffer->initialise_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))(
				&buffer->initialise_callback_list);
		}
		if (buffer->resize_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))(
				&buffer->resize_callback_list);
		}
		if (buffer->expose_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))(
				&buffer->expose_callback_list);
		}
		if (buffer->input_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_input_callback)))(
				&buffer->input_callback_list);
		}

		DEALLOCATE(*buffer_ptr);
		*buffer_ptr = (struct Graphics_buffer *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphics_buffer).  Missing buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphics_buffer) */

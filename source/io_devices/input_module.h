/*******************************************************************************
FILE : input_module.h

LAST MODIFIED : 14 May 1998

DESCRIPTION :
Contains all the code needed to handle input from any of a number of devices,
and sets up callbacks for whatever users are interested.
The callbacks return the relative change since the last call.

NOTE: (If using GL)
The wise guys at GL have made it so that the first time a GL widget is
created, it calls an initialisation routine.  Seems OK right?  However, this
init routine checks for devices on the serial port, and thereby screws up the
polhemus completely.  To rectify the situation, ensure that you create a
GL window first & then destroy it.

NOTE :
Callbacks must have prototype

int *MyCallback(void *identifier,input_module_message message);
(this is the type input_module_callback_proc)

Register the callback for a particular device, and give it a unique identifier -
probably a pointer to your own data area.  This identifier will be passed to the
callback allowing knowledge of 'who you are'.  The return code is not used at
the moment.

NOTE:
The polhemus direction variables are an exception to the 'relative' rule, and
are passed as absolute values.  To check for this, test message->source for
IM_SOURCE_POLHEMUS and then act accordingly.  This is done since I cannot find
a good way of starting the direction from an arbitrary orientation.
==============================================================================*/

#if !defined (INPUT_MODULE_H)
#define INPUT_MODULE_H

#include <Xm/Xm.h>
#include "io_devices/matrix.h"
#include "user_interface/user_interface.h"

/*
Global constants
----------------
*/
#if defined (IBM)
#define IM_SPACEBALL_SCALE 10
#endif
#if defined (SGI)
#define IM_SPACEBALL_SCALE 100
#endif

/*
Global types
------------
*/
enum Input_module_device
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
The different devices you may request input from.
==============================================================================*/
{
	IM_DEVICE_NONE=-1,
	IM_DEVICE_SPACEBALL=0,
	IM_DEVICE_POLHEMUS1=1,
	IM_DEVICE_POLHEMUS2=2,
	IM_DEVICE_POLHEMUS3=3,
	IM_DEVICE_POLHEMUS4=4,
	IM_DEVICE_DIALS=5,
	IM_DEVICE_HAPTIC=6,
	IM_DEVICE_FARO=7
}; /* enum Input_module_device */

#define INPUT_MODULE_NUM_DEVICES 8

enum Input_module_source
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
The different types of devices supported.
==============================================================================*/
{
	IM_SOURCE_SPACEBALL,
	IM_SOURCE_POLHEMUS,
	IM_SOURCE_DIALS,
	IM_SOURCE_HAPTIC,
	IM_SOURCE_FARO
};
enum Input_module_message_type
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
The different types of reasons for a callback.
==============================================================================*/
{
	IM_TYPE_MOTION,
	IM_TYPE_BUTTON_PRESS,
	IM_TYPE_BUTTON_RELEASE
};

struct Input_module_message_struct
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
Contains all the information sent to the callback
==============================================================================*/
{
	enum Input_module_source source;
	enum Input_module_message_type type;
	double data[12];
};
typedef struct Input_module_message_struct *Input_module_message;
typedef int (*Input_module_callback_proc)(void *,Input_module_message);

typedef struct Input_module_callback_info_struct *Input_module_callback_info;
struct Input_module_callback_info_struct
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
Contains all the information in one record of the linked list of
callbacks for each device.
==============================================================================*/
{
	Input_module_callback_info next; /* linked list... */
	Input_module_callback_proc callback;
	Window client_window;    /* to resolve conflicts if >1 client for a device */
	void *identifier;    /* unique identifier for the client*/
};

enum  Input_module_data_types
{
	IM_POSITION_SCALE = 10,
	IM_POSITION_OFFSET = 11,
	IM_POSITION_CURRENT = 12,
	IM_SET_CURRENT_AS_ORIGIN = 13
};

/*
Global functions
----------------
*/
int input_module_init(struct User_interface *user_interface);
/*****************************************************************************
LAST MODIFIED : 23 June 1995

DESCRIPTION :
Sets up any devices correctly, and initialises the callback stack.
============================================================================*/

char *input_module_is_device_valid(enum Input_module_device device);
/*****************************************************************************
LAST MODIFIED : 20 March 1995

DESCRIPTION :
Returns a pointer to a name for the device.  If the device is non-valid, it
returns NULL.
============================================================================*/

int input_module_is_source_valid(enum Input_module_source source);
/*****************************************************************************
LAST MODIFIED : 06 April 1995

DESCRIPTION :
Returns a true if the source is valid.
============================================================================*/

int input_module_add_input_window(Widget widget,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 June 1995

DESCRIPTION :
Let us receive events (ie from spaceball) if the mouse is in this window.
==============================================================================*/

int input_module_close(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Close the input devices.
==============================================================================*/

int input_module_process(XEvent *event,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Checks all events coming into the clients event loop, and searches for any
that are from devices it is interested in.  If any are found, it creates a
message and then sends it to do_callback.
==============================================================================*/

int input_module_update(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Forces the input_module to see if there are any periodic updates
(ie polhemus etc).
==============================================================================*/

int input_module_register(enum Input_module_device device_req,void *ident,
	Widget w,Input_module_callback_proc callback_func);
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
Add a callback to the linked list for the particular device.
==============================================================================*/

int input_module_deregister(enum Input_module_device device_req,void *ident);
/*******************************************************************************
LAST MODIFIED : 11 May 1994

DESCRIPTION :
Remove a callback from the linked list of a particular device.
==============================================================================*/

int input_module_set( enum Input_module_device device,
	enum Input_module_data_types mode, void *data );
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
Changes the setup of the input module device.
Currently supports a scaling and an offset of HAPTIC for mapping it
to world coordinates.
==============================================================================*/


int input_module_get( enum Input_module_device device,
	enum Input_module_data_types mode, void *data );
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
Retrieves the setup of the input device.
Currently supports a scaling and an offset of HAPTIC for mapping it
to world coordinates.
==============================================================================*/

#if defined (FARO)
int input_module_faro_calibrate();
/*******************************************************************************
LAST MODIFIED : 23 October 1998

DESCRIPTION :
Sends commands to calibrate the faro arm
==============================================================================*/
#endif /* defined (FARO) */

#if defined (POLHEMUS)
void input_module_polhemus_set_origin(void);
/*******************************************************************************
LAST MODIFIED : 06 April 1995

DESCRIPTION :
Sets the polhemus so that the current position and origin is defined as the
origin.
==============================================================================*/

void input_module_polhemus_reset_pos(double *position);
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Define the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/

void input_module_polhemus_reset_dir(Gmatrix *direction);
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Define the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/

void input_module_polhemus_revert_pos(double *position);
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Read the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/

void input_module_polhemus_revert_dir(Gmatrix *direction);
/*******************************************************************************
LAST MODIFIED : 8 December 1994

DESCRIPTION :
Read the global origin and orientation for the polhemus so that all stations
will be able to be reset to the same values.
==============================================================================*/

#endif
#endif

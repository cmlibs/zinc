/*******************************************************************************
FILE : io_device.h

LAST MODIFIED : 27 January 2005

DESCRIPTION :
Device structure.  Used to keep information about external devices attached to
cmgui.  Currently just for perl file handle descriptors but could be widened to
include physical devices such as the faro arm or dials which respond to certain
X events or poll frequently with timeouts.
==============================================================================*/
#if !defined (IO_DEVICE_H)
#define IO_DEVICE_H

#include "general/list.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

/* Declare the type here to allow the struct to be used in the
	Io_device_set_perl_action function */
struct Interpreter;

struct Io_device;
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Device structure.
==============================================================================*/

DECLARE_LIST_TYPES(Io_device);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Io_device);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Io_device);

PROTOTYPE_LIST_FUNCTIONS(Io_device);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Io_device,name,char *);

struct Io_device *CREATE(Io_device)(char *name);
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Allocates memory and assigns fields for a device object.
==============================================================================*/

int DESTROY(Io_device)(struct Io_device **device_ptr);
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Frees the memory for the fields of <**device>, frees the memory for <**device>
and sets <*device> to NULL.
==============================================================================*/

int Io_device_start_detection(struct Io_device *device,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Initialises the device to check for active file descriptors.  Useful for 
connecting to external "devices" such as Sockets or widget toolkits.
==============================================================================*/

int Io_device_end_detection(struct Io_device *device);
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Finalises the detection of active file desriptors.  All descriptors activated
between the start and end detection are assumed to belong to the <device>.
==============================================================================*/

int Io_device_set_perl_action(struct Io_device *device,
	struct Interpreter *interpreter, char *perl_action);
/*******************************************************************************
LAST MODIFIED : 27 January 2005

DESCRIPTION :
The string <perl_action> is called "eval"ed in the <interpreter> whenever the
<device> is activated.
==============================================================================*/
#endif /* !defined (DEVICE_H) */

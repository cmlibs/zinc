/*******************************************************************************
FILE : graphics_object.c

LAST MODIFIED : 16 May 2001

DESCRIPTION :
==============================================================================*/
#if defined (SELECT_DESCRIPTORS)
#include <unistd.h>
#include <fcntl.h>
#endif /* defined (SELECT_DESCRIPTORS) */
#include <stdio.h>
#include <string.h>
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "io_devices/io_device.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (PERL_INTERPRETER)
#include "perl_interpreter.h"
#endif /* defined (PERL_INTERPRETER) */

/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
MAXFD is the maximum file descriptor number that we check to.
==============================================================================*/
#define MAXFD (255)

struct Io_device
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Io_device structure.
==============================================================================*/
{
	char *name;
#if defined (SELECT_DESCRIPTORS)
	char *perl_action;
	char *file_descriptor_flags;
	struct User_interface *user_interface;
	struct Event_dispatcher_file_descriptor_handler *callback_id;
#endif /* defined (SELECT_DESCRIPTORS) */
	int access_count;
};

/*
Module types
------------
*/

FULL_DECLARE_INDEXED_LIST_TYPE(Io_device);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Io_device,name,char *,strcmp)

#if defined (SELECT_DESCRIPTORS)
static int Io_device_descriptor_callback(int filehandle, void *device_void)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Called when this device has file descriptors that are waiting.
==============================================================================*/
{
	char *callback_result;
	int return_code;
	struct Io_device *device;

	ENTER(Io_device_descriptor_callback);
	USE_PARAMETER(filehandle);
	if ((device = (struct Io_device *)device_void))
	{
#if defined (DEBUG)
	  printf("Callback from %d\n", *filehandle);
#endif /* defined (DEBUG) */

#if defined (PERL_INTERPRETER)

	  if (device->perl_action)
	  {
#if defined (DEBUG)
		  printf ("Action function %s\n", device->perl_action);
#endif /* defined (DEBUG) */
		  interpreter_evaluate_string(device->perl_action,
			  &callback_result, &return_code);
		  if (callback_result)
		  {
			  DEALLOCATE(callback_result);
		  }
	  }
#endif /* defined (PERL_INTERPRETER) */
	  return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Io_device_descriptor_callback.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Io_device_descriptor_callback */
#endif /* defined (SELECT_DESCRIPTORS) */

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(Io_device)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Io_device)

DECLARE_INDEXED_LIST_FUNCTIONS(Io_device)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Io_device,name,char *, \
	strcmp)

struct Io_device *CREATE(Io_device)(char *name)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Allocates memory and assigns fields for a device.
==============================================================================*/
{
	struct Io_device *device;

	ENTER(CREATE(Io_device));
	if (name)
	{
		if (ALLOCATE(device,struct Io_device,1)&&
			ALLOCATE(device->name,char,strlen(name)+1))
		{
			device->access_count=0;
#if defined (SELECT_DESCRIPTORS)
			device->perl_action = (char *)NULL;
			device->file_descriptor_flags = (char *)NULL;
			device->callback_id = (struct Event_dispatcher_file_descriptor_handler *)NULL;
#endif /* defined (SELECT_DESCRIPTORS) */
			strcpy(device->name,name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Io_device).  Unable to allocate memory for device.");
			device=(struct Io_device *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Io_device).  Invalid argument(s)");
		device=(struct Io_device *)NULL;
	}
	LEAVE;

	return (device);
} /* CREATE(Io_device) */

int DESTROY(Io_device)(struct Io_device **device_ptr)
/*******************************************************************************
LAST MODIFIED : 16 may 2001

DESCRIPTION :
Frees the memory for the fields of <**device>, frees the memory for <**device>
and sets <*device> to NULL.
==============================================================================*/
{
	int return_code;
	struct Io_device *device;

	ENTER(DESTROY(Io_device));
	if (device_ptr&&(device= *device_ptr))
	{
		if (0!=device->access_count)
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Io_device).  Access count = %d",device->access_count);
			return_code=0;
		}
		else
		{
			DEALLOCATE(device->name);
#if defined (SELECT_DESCRIPTORS)
			if (device->perl_action)
			{
				DEALLOCATE(device->perl_action);
			}
			if (device->file_descriptor_flags)
			{
				DEALLOCATE(device->file_descriptor_flags);
			}
			if (device->callback_id)
			{
				Event_dispatcher_remove_file_descriptor_handler(
					User_interface_get_event_dispatcher(device->user_interface),
					device->callback_id);
			}
#endif /* defined (SELECT_DESCRIPTORS) */
			DEALLOCATE(*device_ptr);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Io_device).  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Io_device) */

int Io_device_start_detection(struct Io_device *device,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Initialises the device to check for active file descriptors.  Useful for 
connecting to external "devices" such as Sockets or widget toolkits.
==============================================================================*/
{
	int i, return_code;

	ENTER(Io_device_start_detection);
	if (device && user_interface)
	{
#if defined (SELECT_DESCRIPTORS)
		device->user_interface = user_interface;
		if (!device->file_descriptor_flags)
		{
			ALLOCATE(device->file_descriptor_flags, char, MAXFD);
		}
		if (device->file_descriptor_flags)
		{
			/* See what file descriptors are open */
			for (i = 1 ; i < MAXFD ; i++)
			{
				if (-1 != fcntl(i, F_GETFD))
				{
					device->file_descriptor_flags[i] = 1;
				}
				else
				{
					device->file_descriptor_flags[i] = 0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Io_device_start_detection.  "
				"Unable to allocate file descriptor array.");
			return_code=0;
		}
#else /* defined (SELECT_DESCRIPTORS) */
		return_code = 1;
#endif /* defined (SELECT_DESCRIPTORS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"Io_device_start_detection.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Io_device_start_detection */

int Io_device_end_detection(struct Io_device *device)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
Finalises the detection of active file desriptors.  All descriptors activated
between the start and end detection are assumed to belong to the <device>.
==============================================================================*/
{
	int i, return_code;

	ENTER(Io_device_end_detection);
	if (device)
	{
#if defined (SELECT_DESCRIPTORS)
		if (device->file_descriptor_flags && device->user_interface)
		{
			/* See what file descriptors are open */
			for (i = 1 ; i < MAXFD ; i++)
			{
				if (!device->file_descriptor_flags[i])
				{
					if (-1 != fcntl(i, F_GETFD))
					{
						printf ("Adding Io_device callback %d\n", i);
						device->callback_id = Event_dispatcher_add_file_descriptor_handler(
							User_interface_get_event_dispatcher(device->user_interface),
							i, Io_device_descriptor_callback, (void *)device);
					}
				}
			}
			DEALLOCATE(device->file_descriptor_flags);
			device->file_descriptor_flags = (char *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"You must start_detection on a device before ending it.");
			return_code=0;
		}
#else /* defined (SELECT_DESCRIPTORS) */
		return_code = 1;
#endif /* defined (SELECT_DESCRIPTORS) */		
	}
	else
	{
		display_message(ERROR_MESSAGE,"Io_device_end_detection.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Io_device_end_detection */

int Io_device_set_perl_action(struct Io_device *device, char *perl_action)
/*******************************************************************************
LAST MODIFIED : 16 May 2001

DESCRIPTION :
The string <perl_action> is called "eval"ed in perl whenever the <device> is
activated.
==============================================================================*/
{
	int return_code;

	ENTER(Io_device_set_perl_action);
	if (device && perl_action)
	{
#if defined (SELECT_DESCRIPTORS)
		if (device->perl_action)
		{
			DEALLOCATE(device->perl_action);
		}
		if (ALLOCATE(device->perl_action, char, strlen(perl_action) + 1))
		{
			strcpy(device->perl_action, perl_action);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Io_device_set_perl_action.  "
				"Inable to allocate memory");
			return_code=0;
		}
#else /* defined (SELECT_DESCRIPTORS) */
		return_code = 1;
#endif /* defined (SELECT_DESCRIPTORS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"Io_device_set_perl_action.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Io_device_set_perl_action */

/*******************************************************************************
FILE : fd_io.c

LAST MODIFIED : 10 March 2005

DESCRIPTION :
The public interface to file-descriptor I/O functions of cmiss.
==============================================================================*/
#include "fd_io.h"
#include "general/debug.h"
#include "user_interface/message.h"

struct Fdio_callback_data
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
Contains data for a callback function in the I/O callback API.
==============================================================================*/
{
	Fdio_callback function;
	void *app_user_data;
};

struct Fdio
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This structure is equivalent to a filedescriptor/socket, and holds all the
state we need to perform callbacks when the descriptor is ready to be
read from or written to.
==============================================================================*/
{
	struct Event_dispatcher *event_dispatcher;
	Cmiss_native_socket_t descriptor;
	struct Fdio_callback_data read_data;
	struct Fdio_callback_data write_data;
#if defined(USE_GENERIC_EVENT_DISPATCHER)
	int is_reentrant, signal_to_destroy;
	struct Event_dispatcher_descriptor_callback *callback;
	int ready_to_read, ready_to_write;
#endif /* defined(USE_GENERIC_EVENT_DISPATCHER) */
};

Fdio_package_id CREATE(Fdio_package)(
	struct Event_dispatcher *event_dispatcher)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates a new Fdio_package, given an event dispatcher.
==============================================================================*/
{
	ENTER(CREATE(Fdio_package));
	LEAVE;

	return (Fdio_package_id)(event_dispatcher);
} /* CREATE(Fdio_package) */

int DESTROY(Fdio_package)(Fdio_package_id *pkg)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the Fdio package object. This causes cmgui to forget about the descriptor,
but the descriptor itself must still be closed. This should be called as soon as
the application is notified by the operating system of a closure event.
==============================================================================*/
{
	ENTER(DESTROY(Fdio_package));
	*pkg = NULL;
	LEAVE;

	return (1);
}

/* This implementation is so dependent on USE_GENERIC_EVENT_DISPATCHER that it is
 * easier to write the whole thing again for each option than to put
 * preprocessor conditionals in every single function.
*/
#if defined(USE_GENERIC_EVENT_DISPATCHER)
static int Fdio_event_dispatcher_query_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is the "query" callback function whenever we need to call FD_SET
etc... on a socket using this API.
==============================================================================*/

static int Fdio_event_dispatcher_check_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is called whenever we need to call FD_ISSET etc... on a socket
using this API.
==============================================================================*/

static int Fdio_event_dispatcher_dispatch_function(
	void *user_data
	);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is called whenever we need to dispatch callbacks out to the
application.
==============================================================================*/

static int Fdio_set_callback(struct Fdio_callback_data *callback_data,
	Fdio_callback callback,
	void *app_user_data);
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function sets the callback and user data for a given callback_data structure.
==============================================================================*/

Fdio_id Fdio_package_create_Fdio(Fdio_package_id package,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher.
==============================================================================*/
{
	struct Cmiss_fdio *io;

	ENTER(CREATE(Fdio_package_create_fdio));

	if (ALLOCATE(io, struct Fdio, 1))
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = (struct Event_dispatcher*)package;
		io->descriptor = descriptor;
		if (!(io->callback = Event_dispatcher_add_descriptor_callback(
			io->event_dispatcher,
			Fdio_event_dispatcher_query_function,
			Fdio_event_dispatcher_check_function,
			Fdio_event_dispatcher_dispatch_function,
			io
			)))
		{
			display_message(ERROR_MESSAGE,
				"Fdio_package_create_fdio.  "
				"Event_dispatcher_add_descriptor_callback failed.");
			DEALLOCATE(io);
			io = (Fdio_id)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(Fdio).  "
			"Unable to allocate structure");
	}
	LEAVE;

	return (io);
} /* Fdio_package_create_fdio */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	ENTER(DESTROY(Fdio));
	if ((*io)->is_reentrant)
		(*io)->signal_to_destroy = 1;
	else
	{
		Event_dispatcher_remove_descriptor_callback((*io)->event_dispatcher,
			(*io)->callback);
		DEALLOCATE(*io);
	}

	*io = NULL;

	LEAVE;

	return (1);
} /* DESTROY(Fdio) */

int Fdio_set_read_callback(Fdio_id handle,
	Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after Fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);
	Fdio_set_callback(&handle->read_data, callback, user_data);
	LEAVE;

	return (1);
} /* Fdio_set_read_callback */

int Fdio_set_write_callback(Fdio_id handle,
	Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sets a write callback on the specified Fdio handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback);
	Fdio_set_callback(&handle->read_data, callback, user_data);
	LEAVE;

	return (1);
} /* Fdio_set_write_callback */

static int Fdio_event_dispatcher_query_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	)
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is the "query" callback function whenever we need to call FD_SET
etc... on a socket using this API.
==============================================================================*/
{
	Fdio_id io;

	ENTER(Fdio_event_dispatcher_query_function);
	io = (Fdio_id)user_data;

	if (io->read_data.function)
		FD_SET(io->descriptor, &descriptor_set->read_set);

	if (io->write_data.function)
		FD_SET(io->descriptor, &descriptor_set->write_set);

	LEAVE;

	return (1);
} /* Fdio_event_dispatcher_query_function */


static int Fdio_event_dispatcher_check_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data
	)
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is called whenever we need to call FD_ISSET etc... on a socket
using this API.
==============================================================================*/
{
	Fdio_id io;

	ENTER(Fdio_event_dispatcher_check_function);
	io = (Fdio_id)user_data;

	io->ready_to_read = FD_ISSET(io->descriptor, &descriptor_set->read_set);
	io->ready_to_write = FD_ISSET(io->descriptor, &descriptor_set->write_set);

	LEAVE;

	return (io->ready_to_read || io->ready_to_write);
} /* Fdio_event_dispatcher_check_function */

static int Fdio_event_dispatcher_dispatch_function(
	void *user_data
	)
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function is called whenever we need to dispatch callbacks out to the
application.
==============================================================================*/
{
	Fdio_id io;

	ENTER(Fdio_event_dispatcher_dispatch_function);
	io = (Fdio_id)user_data;

	io->is_reentrant = 1;
	if (io->read_data.function && io->ready_to_read)
		io->read_data.function(io, io->read_data.app_user_data);
	if (io->write_data.function &&
		io->ready_to_write &&
		!io->signal_to_destroy)
		io->write_data.function(io, io->write_data.app_user_data);
	io->is_reentrant = 0;
	if (io->signal_to_destroy)
		DESTROY(Fdio)(&io);
	LEAVE;

	return (1);
} /* Fdio_event_dispatcher_dispatch_function */

static int Fdio_set_callback(struct Fdio_callback_data *callback_data,
	Fdio_callback callback,
	void *app_user_data)
/*******************************************************************************
LAST MODIFIED : 28 February, 2005

DESCRIPTION :
This function sets the callback and user data for a given callback_data structure.
==============================================================================*/
{
	ENTER(Fdio_set_callback);
	callback_data->function = callback;
	callback_data->app_user_data = app_user_data;
	LEAVE;

	return (1);
} /* Fdio_set_callback */

#else /* defined(USE_GENERIC_EVENT_DISPATCHER) */
Fdio_id Fdio_package_create_Fdio(Fdio_package_id package,
	Cmiss_native_socket_t descriptor)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates a new Fdio, given an event dispatcher.
==============================================================================*/
{
	struct Fdio *io;

	ENTER(Fdio_package_create_fdio);
	ALLOCATE(io, struct Fdio, 1);
	if (io)
	{
		memset(io, 0, sizeof(*io));
		io->event_dispatcher = (struct Event_dispatcher*)package;
		io->descriptor = descriptor;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Fdio_package_create_fdio.  "
			"Unable to allocate structure");
	}
	LEAVE;

	return (io);
} /* Fdio_package_create_fdio */

int DESTROY(Fdio)(Fdio_id *io)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the IO object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/
{
	if ((*io)->read_data.function)
		Event_dispatcher_set_socket_read_callback((*io)->event_dispatcher,
			(*io)->descriptor, NULL, NULL);
	if ((*io)->write_data.function)
		Event_dispatcher_set_socket_write_callback((*io)->event_dispatcher,
			(*io)->descriptor, NULL, NULL);
	DEALLOCATE((*io));
	*io = NULL;
	return (1);
} /* DESTROY(Fdio) */

static void Fdio_call_read_callback(SOCKET sock,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Calls the read callback that was previously set on this socket.
==============================================================================*/
{
	Fdio_id handle;

	ENTER(Fdio_call_read_callback);

	handle = (Fdio_id)user_data;
	if (handle->read_data.function)
		handle->read_data.function(handle, handle->read_data.app_user_data);

	LEAVE;
}

static void Fdio_call_write_callback(SOCKET sock,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Calls the write callback that was previously set on this socket.
==============================================================================*/
{
	Fdio_id handle;

	ENTER(Fdio_call_write_callback);

	handle = (Fdio_id)user_data;
	if (handle->write_data.function)
		handle->write_data.function(handle, handle->write_data.app_user_data);

	LEAVE;
}

int Fdio_set_read_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_read_callback);

	if (callback == NULL && handle->read_data.function)
		Event_dispatcher_set_socket_read_callback(handle->event_dispatcher,
			handle->descriptor, NULL, NULL);
	else if (callback != NULL && !handle->read_data.function)
		Event_dispatcher_set_socket_read_callback(handle->event_dispatcher,
			handle->descriptor, Fdio_call_read_callback, handle);

	handle->read_data.function = callback;
	handle->read_data.app_user_data = user_data;

	LEAVE;

	return (1);
} /* Fdio_set_read_callback */

int Fdio_set_write_callback(Fdio_id handle, Fdio_callback callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/
{
	ENTER(Fdio_set_write_callback)

	if (callback == NULL && handle->write_data.function)
		Event_dispatcher_set_socket_read_callback(handle->event_dispatcher,
			handle->descriptor, NULL, NULL);
	else if (callback != NULL && !handle->write_data.function)
		Event_dispatcher_set_socket_read_callback(handle->event_dispatcher,
			handle->descriptor, Fdio_call_write_callback, handle);

	handle->write_data.function = callback;
	handle->write_data.app_user_data = user_data;

	LEAVE;

	return (1);
} /* Fdio_set_write_callback */

#endif /* defined(USE_GENERIC_EVENT_DISPATCHER) else */

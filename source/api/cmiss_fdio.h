/*******************************************************************************
FILE : api/cmiss_io.h

LAST MODIFIED : 15 February, 2005

DESCRIPTION :
The public interface to the Cmiss_IO object.
==============================================================================*/
#ifndef __API_CMISS_FDIO_H__
#define __API_CMISS_FDIO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "general/object.h"
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
typedef SOCKET Cmiss_native_socket_t;
#define INVALID_NATIVE_SOCKET INVALID_SOCKET
#else
typedef int Cmiss_native_socket_t;
#define INVALID_NATIVE_SOCKET ((Cmiss_native_socket_t)-1)
#endif

typedef struct Cmiss_fdio_package * Cmiss_fdio_package_id;
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
An identifier for an fdio package object.
==============================================================================*/

typedef struct Cmiss_fdio * Cmiss_fdio_id;
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
An identifier for an fdio object.
==============================================================================*/

int DESTROY(Cmiss_fdio_package)(Cmiss_fdio_package_id *pkg);
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the fdio package object. This causes cmgui to forget about the descriptor,
but the descriptor itself must still be closed. This should be called as soon as
the application is notified by the operating system of a closure event.
==============================================================================*/

Cmiss_fdio_id Cmiss_fdio_package_create_Cmiss_fdio(Cmiss_fdio_package_id package,
	Cmiss_native_socket_t descriptor);

int DESTROY(Cmiss_fdio)(Cmiss_fdio_id* handle);
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the fdio object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/

typedef void (*Cmiss_fdio_callback)(Cmiss_fdio_id handle, void *user_data);
/*******************************************************************************
LAST MODIFIED : 14 February 2005

DESCRIPTION :
The type used for all I/O callbacks.
==============================================================================*/

int Cmiss_fdio_set_read_callback(Cmiss_fdio_id handle, Cmiss_fdio_callback callback,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 14 February 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after Cmiss_fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/

int Cmiss_fdio_set_write_callback(Cmiss_fdio_id handle, Cmiss_fdio_callback callback,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 14 February 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after Cmiss_fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FDIO_H__ */

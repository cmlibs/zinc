/*******************************************************************************
FILE : fd_io.c

LAST MODIFIED : 26 May 2005

DESCRIPTION :
The private interface to file-descriptor I/O functions of cmiss.

==============================================================================*/
#ifndef __FD_IO_H__
#define __FD_IO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "api/cmiss_fdio.h"

/* Map private names to public function names. */
#define Fdio_package Cmiss_fdio_package
#define Fdio_package_id Cmiss_fdio_package_id
#define Fdio Cmiss_fdio
#define Fdio_id Cmiss_fdio_id
#define destroy_Fdio_package DESTROY(Cmiss_fdio_package)
#define Fdio_package_create_Fdio Cmiss_fdio_package_create_Cmiss_fdio
#define destroy_Fdio DESTROY(Cmiss_Fdio)
#define Fdio_callback Cmiss_fdio_callback
#define Fdio_set_read_callback Cmiss_fdio_set_read_callback
#define Fdio_set_write_callback Cmiss_fdio_set_write_callback

/* Forward declaration to avoid cycle... */
struct Event_dispatcher;

Fdio_package_id CREATE(Fdio_package)(struct Event_dispatcher *event_dispatcher);
/*******************************************************************************
LAST MODIFIED : 15 February 2005

DESCRIPTION :
Creates a new Cmiss_IO_package, given an event dispatcher.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FD_IO_H__ */

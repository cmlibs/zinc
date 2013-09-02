/*******************************************************************************
FILE : api/cmiss_io.h

LAST MODIFIED : 26 May 2005

DESCRIPTION :
The public interface to the cmzn_IO object.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef __API_CMZN_FDIO_H__
#define __API_CMZN_FDIO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined (WIN32_USER_INTERFACE)
//#define WINDOWS_LEAN_AND_MEAN
#if !defined (NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
typedef SOCKET cmzn_native_socket_t;
#define INVALID_NATIVE_SOCKET INVALID_SOCKET
#else
typedef int cmzn_native_socket_t;
#define INVALID_NATIVE_SOCKET ((cmzn_native_socket_t)-1)
#endif

typedef struct cmzn_fdio_package * cmzn_fdio_package_id;
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
An identifier for an fdio package object.
==============================================================================*/

typedef struct cmzn_fdio * cmzn_fdio_id;
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
An identifier for an fdio object.
==============================================================================*/

int cmzn_fdio_package_destroy(cmzn_fdio_package_id *pkg);
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the fdio package object. This causes cmgui to forget about the descriptor,
but the descriptor itself must still be closed. This should be called as soon as
the application is notified by the operating system of a closure event.
==============================================================================*/

cmzn_fdio_id cmzn_fdio_package_create_cmzn_fdio(cmzn_fdio_package_id package,
	cmzn_native_socket_t descriptor);
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Creates and cmzn_fdio object which can be used to add callbacks to the 
<descriptor>.
==============================================================================*/

int cmzn_fdio_destroy(cmzn_fdio_id* handle);
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Destroys the fdio object. This causes cmgui to forget about the descriptor, but the
descriptor itself must still be closed. This should be called as soon as the
application is notified by the operating system of a closure event.
==============================================================================*/

typedef int (*cmzn_fdio_callback)(cmzn_fdio_id handle, void *user_data);
/*******************************************************************************
LAST MODIFIED : 26 May 2005

DESCRIPTION :
The type used for all I/O callbacks.
==============================================================================*/

int cmzn_fdio_set_read_callback(cmzn_fdio_id handle, cmzn_fdio_callback callback,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 14 February 2005

DESCRIPTION :
Sets a read callback on the specified IO handle. This callback is called at
least once after a read function indicates it would block. An application
should not rely upon it being called more than once without attempting a
read between the calls. This read should occur after cmzn_fdio_set_read_callback
is called. The callback will also be called if the underlying descriptor is
closed by the peer. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one read callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the read callback
previously set will be cancelled.
==============================================================================*/

int cmzn_fdio_set_write_callback(cmzn_fdio_id handle, cmzn_fdio_callback callback,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 14 February 2005

DESCRIPTION :
Sets a write callback on the specified IO handle. This callback is called at
least once after a write function indicates it would block. An application
should not rely upon it being called more than once without attempting a
write between the calls. This write should occur after cmzn_fdio_set_write_callback
is called. The callback is not one-shot, and the callback remains in
effect until it is explicitly cancelled.

There may be at most one write callback set per I/O handle at any one time. If
this function is passed NULL as the callback parameter, the write callback
previously set will be cancelled.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMZN_FDIO_H__ */

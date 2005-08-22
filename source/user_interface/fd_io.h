/*******************************************************************************
FILE : fd_io.c

LAST MODIFIED : 26 May 2005

DESCRIPTION :
The private interface to file-descriptor I/O functions of cmiss.

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

/*******************************************************************************
FILE : io_device.h

LAST MODIFIED : 27 January 2005

DESCRIPTION :
Device structure.  Used to keep information about external devices attached to
cmgui.  Currently just for perl file handle descriptors but could be widened to
include physical devices such as the faro arm or dials which respond to certain
X events or poll frequently with timeouts.
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

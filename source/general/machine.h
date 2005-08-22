/*******************************************************************************
FILE : machine.h

LAST MODIFIED : 18 February 1997

DESCRIPTION :
Definitions of machine specific stuff.
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
#if !defined (MACHINE_H)
#define MACHINE_H
#include "general/object.h"
#include "command/parser.h"

/*
Global types
------------
*/
enum Machine_type
/*******************************************************************************
LAST MODIFIED : 20 December 1996

DESCRIPTION :
Different types of machines supported.
==============================================================================*/
{
	MACHINE_UNKNOWN,
	MACHINE_UNIX,
	MACHINE_VMS,
	MACHINE_WINDOWS
}; /* enum Machine_type */

struct Machine_information
/*******************************************************************************
LAST MODIFIED : 20 December 1996

DESCRIPTION :
Different types of machines supported.
==============================================================================*/
{
	char *name;
	enum Machine_type type;
	int num_processors;
	char **processor_types;
}; /* struct Machine_information */

/*
Global functions
----------------
*/
struct Machine_information *CREATE(Machine_information)(void);
/*******************************************************************************
LAST MODIFIED : 20 December 1996

DESCRIPTION :
Creates a machine information structure.
???GMH.  Perhaps this could be extended to tell it a machine to interrogate?
==============================================================================*/

int DESTROY(Machine_information)(
	struct Machine_information **machine_information_address);
/*******************************************************************************
LAST MODIFIED : 20 December 1996

DESCRIPTION :
Creates a machine information structure.
???GMH.  Perhaps this could be extended to tell it a machine to interrogate?
==============================================================================*/

int set_machine_type(struct Parse_state *state,void *machine_type_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 26 September 1996

DESCRIPTION :
Parses the name, and turns it into a type.
==============================================================================*/
#endif /* !defined (MACHINE_H) */

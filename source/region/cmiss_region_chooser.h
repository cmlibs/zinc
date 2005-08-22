/*******************************************************************************
FILE : cmiss_region_chooser.h

LAST MODIFIED : 13 January 2003

DESCRIPTION :
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
#if !defined (CMISS_REGION_CHOOSER_H)
#define CMISS_REGION_CHOOSER_H

/*
Global variables
----------------
*/

#include "general/callback_motif.h"

struct Cmiss_region_chooser;
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/

/*
Global functions
----------------
*/

struct Cmiss_region_chooser *CREATE(Cmiss_region_chooser)(Widget parent,
	struct Cmiss_region *root_region, char *initial_path);
/*******************************************************************************
LAST MODIFIED : 8 January 2003

DESCRIPTION :
Creates a dialog from which a region may be chosen.
<parent> must be an XmForm.
==============================================================================*/

int DESTROY(Cmiss_region_chooser)(
	struct Cmiss_region_chooser **chooser_address);
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/

int Cmiss_region_chooser_set_callback(struct Cmiss_region_chooser *chooser,
	Callback_procedure *procedure, void *data);
/*******************************************************************************
LAST MODIFIED : 8 January 2003

DESCRIPTION :
Sets the callback <procedure> and user <data> in the <chooser>.
==============================================================================*/

int Cmiss_region_chooser_get_path(struct Cmiss_region_chooser *chooser,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Gets <path> of chosen region in the <chooser>.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

int Cmiss_region_chooser_get_region(struct Cmiss_region_chooser *chooser,
	struct Cmiss_region **region_address);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/

int Cmiss_region_chooser_set_path(struct Cmiss_region_chooser *chooser,
	char *path);
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/

#endif /* !defined (CMISS_REGION_CHOOSER_H) */

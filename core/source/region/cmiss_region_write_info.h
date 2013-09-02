/*******************************************************************************
FILE : cmiss_region_write_info.h

LAST MODIFIED : 16 May 2003

DESCRIPTION :
Data structure shared by several region export modules.
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
#if !defined (CMISS_REGION_WRITE_INFO_H)
#define CMISS_REGION_WRITE_INFO_H

#include "region/cmiss_region.h"
#include "general/list.h"
#include "general/object.h"

/*
Module types
------------
*/

enum cmzn_region_write_status
{
	CMISS_REGION_NOT_WRITTEN,
	CMISS_REGION_DECLARED,
	CMISS_REGION_WRITTEN
};

struct cmzn_region_write_info;
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Data structure shared by several region export modules.
==============================================================================*/

DECLARE_LIST_TYPES(cmzn_region_write_info);

/*
Module functions
----------------
*/

struct cmzn_region_write_info *CREATE(cmzn_region_write_info)(void);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/

int DESTROY(cmzn_region_write_info)(
	struct cmzn_region_write_info **write_info_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_region_write_info);

PROTOTYPE_LIST_FUNCTIONS(cmzn_region_write_info);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_region_write_info, \
	region, struct cmzn_region *);

int set_cmzn_region_write_info(
	struct LIST(cmzn_region_write_info) *write_info_list,
	struct cmzn_region *region, enum cmzn_region_write_status write_status,
	char *path);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/

int get_cmzn_region_write_info(
	struct LIST(cmzn_region_write_info) *write_info_list,
	struct cmzn_region *region,
	enum cmzn_region_write_status *write_status_address,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
The returned path is not to be deallocated.
==============================================================================*/

#endif /* !defined (CMISS_REGION_WRITE_INFO_H) */

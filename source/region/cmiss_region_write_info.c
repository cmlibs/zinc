/*******************************************************************************
FILE : cmiss_region_write_info.c

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
#include <stdio.h>
#include "general/compare.h"
#include "general/debug.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_write_info.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Cmiss_region_write_info
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Data structure shared by several region export modules.
==============================================================================*/
{
	struct Cmiss_region *region;
	enum Cmiss_region_write_status status;
	char *path;
	int access_count;
};

FULL_DECLARE_INDEXED_LIST_TYPE(Cmiss_region_write_info);

/*
Module functions
----------------
*/

struct Cmiss_region_write_info *CREATE(Cmiss_region_write_info)(void)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/
{
	struct Cmiss_region_write_info *write_info;

	ENTER(CREATE(Cmiss_region_write_info));
	if (ALLOCATE(write_info, struct Cmiss_region_write_info, 1))
	{
		write_info->region = (struct Cmiss_region *)NULL;
		write_info->status = CMISS_REGION_NOT_WRITTEN;
		write_info->path = (char *)NULL;
		write_info->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_region_write_info).  Invalid argument(s)");
	}
	LEAVE;

	return (write_info);
} /* CREATE(Cmiss_region_write_info) */

int DESTROY(Cmiss_region_write_info)(
	struct Cmiss_region_write_info **write_info_address)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_write_info *write_info;

	ENTER(DESTROY(Cmiss_region_write_info));
	if (write_info_address && (write_info = *write_info_address))
	{
		if (write_info->region)
		{
			DEACCESS(Cmiss_region)(&(write_info->region));
		}
		if (write_info->path)
		{
			DEALLOCATE(write_info->path);
		}
		DEALLOCATE(*write_info_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_region_write_info).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_region_write_info) */

DECLARE_OBJECT_FUNCTIONS(Cmiss_region_write_info)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cmiss_region_write_info, region, \
	struct Cmiss_region *, compare_pointer)

DECLARE_INDEXED_LIST_FUNCTIONS(Cmiss_region_write_info)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cmiss_region_write_info, \
	region, struct Cmiss_region *, compare_pointer)

int set_Cmiss_region_write_info(
	struct LIST(Cmiss_region_write_info) *write_info_list,
	struct Cmiss_region *region, enum Cmiss_region_write_status write_status,
	char *path)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_write_info *write_info;

	ENTER(set_Cmiss_region_write_info);
	if (write_info_list && region && path &&
		((CMISS_REGION_DECLARED == write_status) ||
			(CMISS_REGION_WRITTEN == write_status)))
	{
		return_code = 1;
		if (write_info = FIND_BY_IDENTIFIER_IN_LIST(Cmiss_region_write_info,region)(
			region, write_info_list))
		{
			/* only need to handle updating from DECLARED to WRITTEN */
			if (CMISS_REGION_DECLARED == write_info->status)
			{
				write_info->status = write_status;
			}
		}
		else
		{
			if (write_info = CREATE(Cmiss_region_write_info)())
			{
				write_info->region = ACCESS(Cmiss_region)(region);
				write_info->status = write_status;
				write_info->path = duplicate_string(path);
				if (!(write_info->path && ADD_OBJECT_TO_LIST(Cmiss_region_write_info)(
					write_info,	write_info_list)))
				{
					DESTROY(Cmiss_region_write_info)(&write_info);
					return_code = 0;
				}
			}
			else
			{
				return_code = 0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"set_Cmiss_region_write_info.  Could not set info");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Cmiss_region_write_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Cmiss_region_write_info */

int get_Cmiss_region_write_info(
	struct LIST(Cmiss_region_write_info) *write_info_list,
	struct Cmiss_region *region,
	enum Cmiss_region_write_status *write_status_address,
	char **path_address)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
The returned path is not to be deallocated.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_write_info *write_info;

	ENTER(get_Cmiss_region_write_info);
	if (write_info_list && region && write_status_address && path_address)
	{
		return_code = 1;
		if (write_info = FIND_BY_IDENTIFIER_IN_LIST(Cmiss_region_write_info,region)(
			region, write_info_list))
		{
			*write_status_address = write_info->status;
			*path_address = write_info->path;
		}
		else
		{
			*write_status_address = CMISS_REGION_NOT_WRITTEN;
			*path_address = (char *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Cmiss_region_write_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_Cmiss_region_write_info */

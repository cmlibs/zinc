/*******************************************************************************
FILE : cmiss_region.c

LAST MODIFIED : 02 March 2005

DESCRIPTION :
The public interface to the Cmiss_regions.
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
#include <stdlib.h>
#include "api/cmiss_region.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/import_finite_element.h"
#include "general/io_stream.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_region_id Cmiss_region_access(Cmiss_region_id region)
{
	return (ACCESS(Cmiss_region)(region));
}

int Cmiss_region_destroy(Cmiss_region_id *region)
/*******************************************************************************
LAST MODIFIED : 3 January 2008

DESCRIPTION :
Destroys the <region> and sets the pointer to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_destroy);
	return_code = 0;
	if (region && *region)
	{
		return_code = DEACCESS(Cmiss_region)(region);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_destroy */

int Cmiss_region_read_file(struct Cmiss_region *region, const char *file_name)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *temp_region;
	struct IO_stream_package *io_stream_package;

	ENTER(Cmiss_region_read_file);
	return_code = 0;
	if (region && file_name && (io_stream_package=CREATE(IO_stream_package)()))
	{
		temp_region = Cmiss_region_create_region(region);
		if (read_exregion_file_of_name(temp_region,file_name,io_stream_package,
			(struct FE_import_time_index *)NULL))
		{
			if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
			{
				return_code=Cmiss_regions_merge_FE_regions(region,temp_region);
			}
		}
		DEACCESS(Cmiss_region)(&temp_region);
		DESTROY(IO_stream_package)(&io_stream_package);
	}
	LEAVE;

	return(return_code);
}

Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region, 
	const char *field_name)
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Returns the field of <field_name> from <region> if it is defined.
==============================================================================*/
{
	struct Cmiss_field *field;
	struct MANAGER(Computed_field) *manager;

	ENTER(Cmiss_region_find_field_by_name);
	if (region && field_name && 
		(manager = Cmiss_region_get_Computed_field_manager(region)))
	{
		field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			(char *)field_name, manager);
		if (field)
		{
			ACCESS(Computed_field)(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_find_field_by_name.  Invalid argument(s)");
		field = (struct Cmiss_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Cmiss_region_find_field_by_name */

int Cmiss_region_read_from_memory(struct Cmiss_region *region, const void *memory_buffer,
	const unsigned int memory_buffer_size)
{
	const char block_name[] = "dataBlock";
	const char block_name_uri[] = "memory:dataBlock";
	int return_code;
	struct Cmiss_region *temp_region;
	struct IO_stream_package *io_stream_package;
	struct IO_stream *input_stream;

	ENTER(Cmiss_region_read_file);
	return_code = 0;
	if (region && memory_buffer && memory_buffer_size && (io_stream_package=CREATE(IO_stream_package)()))
	{
		temp_region = Cmiss_region_create_region(region);
		//We should add a way to define a memory block without requiring specifying a name. 
		IO_stream_package_define_memory_block(io_stream_package,
			block_name, memory_buffer, memory_buffer_size);
		input_stream = CREATE(IO_stream)(io_stream_package);
		IO_stream_open_for_read(input_stream, block_name_uri);
		if (read_exregion_file(temp_region, input_stream, (struct FE_import_time_index *)NULL))
		{
			if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
			{
				return_code=Cmiss_regions_merge_FE_regions(region,temp_region);
			}
		}
		IO_stream_close(input_stream);
		DESTROY(IO_stream)(&input_stream);
		IO_stream_package_free_memory_block(io_stream_package,
			block_name);
		DEACCESS(Cmiss_region)(&temp_region);
		DESTROY(IO_stream_package)(&io_stream_package);
	}
	LEAVE;

	return(return_code);
}

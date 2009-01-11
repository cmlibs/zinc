/*******************************************************************************
FILE : cmiss_region.i

LAST MODIFIED : 8 January 2008

DESCRIPTION :
Swig interface file for wrapping api header api/cmiss_region
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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

%module region
%include carrays.i
%array_functions(float, float_array);

%{
#include "api/cmiss_region.h"
%}

%inline %{

	int Cmiss_region_read_file(struct Cmiss_region *region, char *file_name)
	/*******************************************************************************
	LAST MODIFIED : 23 May 2008

	DESCRIPTION :
	==============================================================================*/
	{
		int return_code;
		struct Cmiss_region *temp_region;
		struct IO_stream_package *io_stream_package;

		//ENTER(Cmiss_region_read_file);
		return_code = 0;
		if (region && file_name && (io_stream_package=CREATE(IO_stream_package)()))
		{
			temp_region = Cmiss_region_create_share_globals(region);
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
		//LEAVE;

		return(return_code);
	}

	int Cmiss_region_get_number_of_elements_in_region(struct Cmiss_region *region)
	/*******************************************************************************
	LAST MODIFIED : 02 March 2005

	DESCRIPTION :
	Returns the number of elements in the <region>.
	==============================================================================*/
	{
		int number_of_elements;
		struct FE_region *fe_region;

		//ENTER(Cmiss_region_get_number_of_elements_in_region);
		number_of_elements = 0;
		if (region)
		{
			if (fe_region = Cmiss_region_get_FE_region(region))
			{
				number_of_elements = FE_region_get_number_of_FE_elements(fe_region);
			}
		}
		//LEAVE;

		return (number_of_elements);
	} /* Cmiss_region_get_number_of_elements */

	int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region *region)
	/*******************************************************************************
	LAST MODIFIED : 4 November 2004

	DESCRIPTION :
	Returns the number of nodes in the <region>.
	==============================================================================*/
	{
		int number_of_nodes;
		struct FE_region *fe_region;

		//ENTER(Cmiss_region_get_number_of_nodes_in_region);
		number_of_nodes = 0;
		if (region)
		{
			if (fe_region = Cmiss_region_get_FE_region(region))
			{
				number_of_nodes = FE_region_get_number_of_FE_nodes(fe_region);
			}
		}
		//LEAVE;

		return (number_of_nodes);
	} /* Cmiss_region_get_number_of_elements */

	int Cmiss_region_for_each_node_in_region(struct Cmiss_region *region,
		Cmiss_node_iterator_function iterator_function, void *user_data)
	/*******************************************************************************
	LAST MODIFIED : 4 November 2004

	DESCRIPTION :
	Iterates over each node in <region>.
	==============================================================================*/
	{
		int return_code;
		struct FE_region *fe_region;

		//ENTER(Cmiss_region_for_each_node_in_region);
		return_code = 0;
		if (region&&iterator_function)
		{
			if (fe_region=Cmiss_region_get_FE_region(region))
			{
				return_code=FE_region_for_each_FE_node(fe_region,iterator_function,
					user_data);
			}
		}
		//LEAVE;

		return (return_code);
	} /* Cmiss_region_for_each_node_in_region */

	int Cmiss_region_for_each_element_in_region(struct Cmiss_region *region,
		Cmiss_element_iterator_function iterator_function, void *user_data)
	/*******************************************************************************
	LAST MODIFIED : 02 March 2005

	DESCRIPTION :
	Iterates over each element in <region>.
	==============================================================================*/
	{
		int return_code;
		struct FE_region *fe_region;

		//ENTER(Cmiss_region_for_each_element_in_region);
		return_code = 0;
		if (region&&iterator_function)
		{
			if (fe_region=Cmiss_region_get_FE_region(region))
			{
				return_code=FE_region_for_each_FE_element(fe_region,iterator_function,
					user_data);
			}
		}
		//LEAVE;

		return (return_code);
	} /* Cmiss_region_for_each_element_in_region */

%}

%include "api/cmiss_region.h"


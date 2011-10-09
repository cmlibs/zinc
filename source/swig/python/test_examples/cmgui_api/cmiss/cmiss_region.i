/*******************************************************************************
FILE : cmiss_region.i

LAST MODIFIED : 22 December 2008

DESCRIPTION :
Swig interface file for wrapping api functions in api/cmiss_region
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

%module cmiss_region
%include carrays.i
%array_functions(float, float_array);

%{
typedef struct Cmiss_region *Cmiss_region_id;
typedef struct Cmiss_field *Cmiss_field_id;
typedef struct Cmiss_node *Cmiss_node_id;

extern Cmiss_region_id Cmiss_region_create(void);
extern Cmiss_region_id Cmiss_region_get_sub_region(Cmiss_region_id region,
	const char *path); //returns struct
extern int Cmiss_region_get_number_of_nodes_in_region(Cmiss_region_id 
	region);
extern Cmiss_node_id Cmiss_region_get_node(Cmiss_region_id region,
	const char *name); //returns struct
extern Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region, 
	const char *field_name); //returns struct
%}

%inline %{

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
%}
 	
extern Cmiss_region_id Cmiss_region_create(void);
extern Cmiss_region_id Cmiss_region_get_sub_region(Cmiss_region_id region,
	const char *path); //returns struct
extern int Cmiss_region_get_number_of_nodes_in_region(Cmiss_region_id 
	region);
extern Cmiss_node_id Cmiss_region_get_node(Cmiss_region_id region,
	const char *name); //returns struct
extern Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region, 
	const char *field_name); //returns struct


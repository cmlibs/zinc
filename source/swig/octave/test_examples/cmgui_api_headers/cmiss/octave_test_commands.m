%*******************************************************************************
#FILE : octave_test_commands.m

#LAST MODIFIED : 23 January 2009

#DESCRIPTION :
#octave script for testing wrapped api functions in the cmiss package
#===============================================================================
# * ***** BEGIN LICENSE BLOCK *****
# * Version: MPL 1.1/GPL 2.0/LGPL 2.1
# *
# * The contents of this file are subject to the Mozilla Public License Version
# * 1.1 (the "License"); you may not use this file except in compliance with/
# * the License. You may obtain a copy of the License at
# * http://www.mozilla.org/MPL/
# *
# * Software distributed under the License is distributed on an "AS IS" basis,
# * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# * for the specific language governing rights and limitations under the
# * License.
# *
# * The Original Code is cmiss.
# *
# * The Initial Developer of the Original Code is
# * Auckland Uniservices Ltd, Auckland, New Zealand.
# * Portions created by the Initial Developer are Copyright (C) 2008
# * the Initial Developer. All Rights Reserved.
# *
# * Contributor(s):
# *
# * Alternatively, the contents of this file may be used under the terms of
# * either the GNU General Public License Version 2 or later (the "GPL"), or
# * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# * in which case the provisions of the GPL or the LGPL are applicable instead
# * of those above. If you wish to allow use of your version of this file only
# * under the terms of either the GPL or the LGPL, and not to allow others to
# * use your version of this file under the terms of the MPL, indicate your
# * decision by deleting the provisions above and replace them with the notice
# * and other provisions required by the GPL or the LGPL. If you do not delete
# * the provisions above, a recipient may use your version of this file under
# * the terms of any one of the MPL, the GPL or the LGPL.
# *
# * ***** END LICENSE BLOCK ***** 

cmiss_command_data
cmiss_region
cmiss_field
%cmiss_node
cmiss_element

a = cmiss_command_data.new_Cmiss_command_data();
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx re no cube");
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx re elem cube");

%***************************************************************
%obtain cmiss_region objects to query number of nodes in cube
%***************************************************************
region = "/cube"
root_region_id = cmiss_command_data.Cmiss_command_data_get_root_region(a)
region_1_id = cmiss_region.Cmiss_region_get_sub_region(root_region_id, region)
region_1_number_of_nodes = cmiss_region.Cmiss_region_get_number_of_nodes_in_region(region_1_id)
%printf("Number of nodes in region \n", region, "\n:", region_1_number_of_nodes)

%***************************************************************
%get cmiss_field objects from a region to query number of components of a
%coordinate field  
%***************************************************************
field = "coordinates"
field_1_id = cmiss_region.Cmiss_region_find_field_by_name(region_1_id, field)
field_1_number_of_components = cmiss_field.Cmiss_field_get_number_of_components(field_1_id)
%printf("Number of components in field \n", field, "\n:", field_1_number_of_components)

%***************************************************************
%use carrays.i from swig library to extract array of field values from a node 
%***************************************************************
node ="1";
array_size = field_1_number_of_components;
field_values = [0.0]*array_size;

node_1_id = cmiss_region.Cmiss_region_get_node(region_1_id,node);
node_1_fieldvalues = cmiss_field.new_float_array(array_size) % carrays method
cmiss_field.Cmiss_field_evaluate_at_node(field_1_id, node_1_id, 0, array_size, node_1_fieldvalues);

for i = 0:array_size;
    field_values[i] = cmiss_field.float_array_getitem(node_1_fieldvalues,i); % carrays method
end;

%printf("field values at node", node, ":", field_values)

#*******************************************************************************
#FILE : cmgui_api_header_test.py

#LAST MODIFIED : 02 February 2008

#DESCRIPTION :
#python script for testing wrapped api headers in the cmiss package
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

#================================================#
#=Initiate and load an example femur mesh (mean)=#
#================================================#
print "loading modules..."
import cmiss.command_data
import cmiss.region
import cmiss.field

print "creating cmiss_command_data..."
a = cmiss.command_data.Cmiss_command_data()

print "loading mesh..."
filename = "mean_mag_pc1_fitted"
region_id = cmiss.region.Cmiss_region()
region_id.read_file(filename+".exnode")
region_id.read_file(filename+".exelem")

region_name = "mean_mag_pc1_fitted"
root_region_id = a.get_root_region()
root_region_id.add_child_region(region_id, region_name, -1)

#==============================================================#
#=obtain cmiss_region objects to query number of nodes in mean=#
#==============================================================# 
region_number_of_nodes = region_id.get_number_of_nodes_in_region()
print "Number of nodes in region \"", filename, "\":", region_number_of_nodes

#=============================================================================#
#= use carrays.i from swig library to extract array of field values at nodes =#
#=============================================================================#

def write_field(field_id, region_id, number_of_nodes, components, file_name):
    field_values = [0.0]*components
    f=open(file_name, mode="wt")

    for node in range(1,number_of_nodes+1):
    	node = str(node)
    	node_id = region_id.get_node(node)
   	node_fieldvalues = cmiss.field.new_float_array(components) # carrays method
    	field_id.evaluate_at_node(node_id, 0, components, node_fieldvalues) #returns a c array into node_fieldvalues object

    	for i in range(0,components):
            field_values[i] = cmiss.field.float_array_getitem(node_fieldvalues,i) # retrieve individual elements from the node_fielvalue object

    	print field, "field values at node", node, ":", field_values

    	# write node number and values to file
    	writelist = map(lambda(x): str(x), field_values)
    	f.writelines(node+"\t"+" ".join(writelist)+"\n")

    f.close()


#==================================#
#= write out the coordinate field =#
#==================================#
field = "coordinates"
field_id = region_id.find_field_by_name(field)
field_number_of_components = field_id.get_number_of_components()
print "Number of components in field \"", field, "\":", field_number_of_components

write_field(field_id, region_id, region_number_of_nodes, field_number_of_components, "coordinate_field.txt")


#==================================================================#
#= use field arithmetics to calculate a new field: multiple_field =#
#==================================================================#
field = "general"
field_id = region_id.find_field_by_name(field)
field_number_of_components = field_id.get_number_of_components()
print "Number of components in field \"", field, "\":", field_number_of_components

multiple_field_id = cmiss.field.Cmiss_field_create_multiply(field_id, field_id)
write_field(multiple_field_id, region_id, region_number_of_nodes, field_number_of_components,  "multiple_field.txt")

a.main_loop_run()
	 

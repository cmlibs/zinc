#*******************************************************************************
#FILE : pythoncmiss_field_tools.py

#LAST MODIFIED : 30 January 2008

#DESCRIPTION :
#Commonly used python functions for field operation with the CMGUI API
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

import cmiss.region
import cmiss.field

###########################################################################
# extracts one component (1,2,3...) of field values at nodes from a field
# inputs: field - PyObject Cmiss_field_id, field from which a component will 
#		  be extracted
#         region - PyObject Cmiss_region_id, region in which the field is in
#         component - the component to be extracted from the field
# outputs: component_values - a python list of field values for the component
#			      specified
#			      returns 1 if specified component does not exist

def get_component_values(field, region, component):

    number_of_nodes = region.get_number_of_nodes_in_region()
    print number_of_nodes, "nodes in region"
    components = field.get_number_of_components()
    node_field_values = cmiss.field.new_float_array(components) # initialise c array
    component_values = [0.0]*(number_of_nodes)

    if component > components:
	print "ERROR in get_component_values: Specified component out of range"
	return 0
	
    for node in range(1, number_of_nodes+1):
    	node_id = region.get_node(str(node))
    	field.evaluate_at_node(node_id, 0, components, node_field_values) 
        component_values[node-1] = cmiss.field.float_array_getitem(node_field_values, component-1)

    return component_values
###########################################################################

###########################################################################
# extracts all components of a field at a given node and returns values as 
# a list
# inputs: node - PyObject Cmiss_node_id of the specified node
#	  field - PyObject Cmiss_field_id of the specified field
#         components - integer, number of components in the field
#	  node_array - carray object with length = components, stores output
#		       from field.evaluate_at_node
#         t - float specifying time, default is 0.0
# outputs: field_values - list of field values at node, length = number of
#                         components 
#	   node_array - stored with new values

def get_field_values_at_node(node, field, components, node_array, t=0.0):
    
    if field.is_defined_at_node(node):
    	field_values = [0.0]*components 
    	field.evaluate_at_node(node, t, components, node_array)
    
   	for i in range(0,components):
    	    field_values[i] = cmiss.field.float_array_getitem(node_array,i)

    	return field_values
    else:
	print "ERROR in get_field_values_at_node: field is undefined at node"
	return 0

###########################################################################

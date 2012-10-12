#*******************************************************************************
#FILE : python_example_surf.py

#LAST MODIFIED : 30 January 2008

#DESCRIPTION :
#python script for drawing and modifying a surf plot
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

import cmiss.command_data
import cmiss.region
import cmiss.field
import pythoncmiss_field_tools as Pyfield
from Numeric import *

# create Cmiss_command_data
print "creating cmiss_command_data..."
a = cmiss.command_data.Cmiss_command_data()
root_region = a.get_root_region()

# draw axes
a.execute_command("gfx create axes length 1.2")
a.execute_command("gfx draw axes")

# read mesh
print "loading mesh..."
filename = "grid"
root_region.read_file(filename+".exnode")
root_region.read_file(filename+".exelem")

a.execute_command("gfx define field surf composite coordinates.x coordinates.y general")

# initialise fields
data_field_name = "general"
coord_field_name = "coordinates"
surf_field_name = "surf"

data_field = root_region.find_field_by_name(data_field_name)
coord_field = root_region.find_field_by_name(coord_field_name)
surf_field = root_region.find_field_by_name(surf_field_name)

print "initial surf values"
print "x\n", Pyfield.get_component_values(surf_field, root_region, 1), "\n"
print "y\n", Pyfield.get_component_values(surf_field, root_region, 2), "\n"
print "z\n", Pyfield.get_component_values(surf_field, root_region, 3), "\n"

a.execute_command("gfx modify g_element grid surfaces coordinate surf select_on material default data general spectrum default selected_material default_selected render_shaded;")
a.execute_command("gfx cre win 1")

print "PAUSE: viewing initial surf..."
a.main_loop_run()

# modify z values
nodes = root_region.get_number_of_nodes_in_region()
node_field_values = cmiss.field.new_float_array(3)
components = surf_field.get_number_of_components()
field_values = [0.0]*components

for node in range(1, nodes+1):
    node_id = root_region.get_node(str(node))
    print "node", str(node)

    # get initial values
    field_values = Pyfield.get_field_values_at_node(node_id, surf_field, components, node_field_values)
    print "initial", field_values

    # modify(double) field(z) values in float_array
    cmiss.field.float_array_setitem(node_field_values, 2, float(field_values[2]*2.0))
    for i in range(0,components):
    	field_values[i] = cmiss.field.float_array_getitem(node_field_values,i)
    print "new unset", field_values
    # set field(z) values
    surf_field.set_values_at_node(node_id, 0, components, node_field_values)

    # get new values
    field_values = Pyfield.get_field_values_at_node(node_id, surf_field, components, node_field_values)
    print "new set", field_values, "\n"

print "new surf values"
print "x\n", Pyfield.get_component_values(surf_field, root_region, 1), "\n"
print "y\n", Pyfield.get_component_values(surf_field, root_region, 2), "\n"
print "z\n", Pyfield.get_component_values(surf_field, root_region, 3), "\n"

print "surf modified\nEND: going to main_loop"
a.main_loop_run()


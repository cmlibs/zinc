#*******************************************************************************
#FILE : python_example_a2.py

#LAST MODIFIED : 23 January 2008

#DESCRIPTION :
#python script for cmgui example a2
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
import cmiss.scene_viewer

# create Cmiss_command_data
print "creating cmiss_command_data..."
a = cmiss.command_data.Cmiss_command_data()

# create materials
a.execute_command("gfx create material kermit ambient 0 0.7 0.2 diffuse 0 0.7 0.2 specular 1 1 1 shininess 0.5;")
a.execute_command("gfx create material trans_purple ambient 0.4 0 0.9 diffuse 0.4 0 0.9 alpha 0.5;")
a.execute_command("gfx create material gold ambient 1 0.7 0 diffuse 1 0.7 0 specular 1 1 0.8 shininess 0.8;")

# draw axes
a.execute_command("gfx create axes length 1.2")
a.execute_command("gfx draw axes")


# read mesh
print "loading mesh..."
filename = "cube"
region_id = cmiss.region.Cmiss_region()
region_id.read_file(filename+".exnode")
region_id.read_file(filename+".exelem")

region_name = "cube"
root_region_id = a.get_root_region()
root_region_id.add_child_region(region_id, region_name, -1)

# adjust visibility
a.execute_command("gfx set visibility cube off")
a.execute_command("gfx set visibility cube on")
a.execute_command("gfx modify g_element cube lines invisible")

# modify graphics
a.execute_command("gfx modify g_element cube general circle_discretization 12")
a.execute_command("gfx modify g_element cube cylinders constant_radius 0.02 material gold")
a.execute_command("gfx modify g_element cube surfaces material trans_purple render_shaded")
a.execute_command("gfx modify g_element cube surfaces face xi1_0 material kermit render_shaded position 3")

#a.execute_command("gfx cre win 1")
a.main_loop_run()

#=====================#
#= create isosurface =#
#=====================#

# debug: write x_coord_field to file
#field_number_of_components = cmiss.field.Cmiss_field_get_number_of_components(x_coord_field_region_id)
#write_field(x_coord_field_region_id, region_id, 8, field_number_of_components, "x_coord_field_region")

# add isosurface
print "adding isosurface..."

field_name = "coordinates"
coord_field_id = region_id.find_field_by_name(field_name)

#=====================================================================
def add_isosurface(field_id, region_id, component, value):

    # create a field from one of the coordinate field components
    coord_field_id = cmiss.field.Cmiss_field_create_component(field_id, component);
    coord_field_region_id = region_id.add_field(coord_field_id)

    coord_field_name = "coordinates"+str(component)
    coord_field_region_id.set_name(coord_field_name)

    # add isosurface to scene
    a.execute_command("gfx modify g_element cube iso_surfaces iso_scalar " + coord_field_name + " iso_value " + str(value) + " use_elements material red render_shaded position 4")

#=====================================================================

go = 1
while go:
    component = input("enter coordinate component (0,1,2) or 'q' to move on: ")
    if component == 'q':
	go = 0
    else:
        value = input("enter component value: ")
	add_isosurface(coord_field_id, region_id, component, value)
	
a.main_loop_run()


#==================#
#= get screenshot =#
#==================#
get_screenshot = input("open 3D window and enter 1 if you want a screenshot, otherwise enter 0: ")
if get_screenshot:
    print "getting screenshot..."
    #a.execute_command("gfx cre win 1")
    scene_viewer_id = a.get_graphics_window_pane_by_name("1", 0)

    force_onscreen = 1
    cmiss.scene_viewer.Cmiss_scene_viewer_write_image_to_file(scene_viewer_id, "isosurf.tiff", force_onscreen, 0, 0, 0, 0);


run = input("continue? (0 or 1): ")
if run:
   print("...")
else:
   print("EXITING")
   quit()


#======================#
#= node modifications =#
#======================#

# label nodes with cmiss_numbers
a.execute_command("gfx modify g_element cube node_points glyph point label cmiss_number select_on material default selected_material default_selected")

#========================================================================
# function for viewing and changing node values
def set_node_value(node, field_id, region_id):

    node_id = region_id.get_node(str(node))
    field_number_of_components = field_id.get_number_of_components()
    node_values = [0.0]*field_number_of_components

    # create c array of node values
    node_value_array = cmiss.field.new_float_array(field_number_of_components) # carrays method

    # get current values
    cmiss.field.Cmiss_field_evaluate_at_node(field_id, node_id, 0, field_number_of_components, node_value_array)
    for i in range(0,field_number_of_components):
	node_values[i] = cmiss.field.float_array_getitem(node_value_array, i) 
    
    print "current values at node", node, ":", node_values
    
    # set new node values
    for i in range(0,field_number_of_components):
	node_value = input("enter value for component "+str(i+1)+": ")
	cmiss.field.float_array_setitem(node_value_array, i, node_value)
    
    cmiss.field.Cmiss_field_set_values_at_node(field_id, node_id, 0, field_number_of_components, node_value_array)

#========================================================================

# call function to change node values
selected_node = 1
while selected_node > 0:
    selected_node = input("enter node number (or 0 to exit): ")
    if selected_node > 0:
    	set_node_value(selected_node, coord_field_id, region_id)


# adjust discretisation
a.execute_command('gfx modify g_element cube general element_discretization "16*16*16"')

a.main_loop_run()

run = input("continue? (0 or 1): ")
if run:
   print("...")
else:
   print("EXITING")
   quit()




print ("END OF EXAMPLE")
a.main_loop_run()



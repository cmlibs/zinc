#*******************************************************************************
#FILE : cmgui_api_header_image_field_test.py

#LAST MODIFIED : 20 January 2008

#DESCRIPTION :
#python script for testing image field reading and writing
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

#===========#
#=Initiate =#
#===========#
print "loading modules..."
import cmiss.command_data
import cmiss.region
import cmiss.field

print "creating cmiss_command_data..."
a = cmiss.command_data.Cmiss_command_data()

#=============#
#= read mesh =#
#=============#
print "loading mesh..."
filename = "square"
region_id = cmiss.region.Cmiss_region()
region_id.read_file(filename+".exnode")
region_id.read_file(filename+".exelem")

# add mesh region to root region
region_name = "image"
root_region_id = a.get_root_region()
if not root_region_id.add_child_region(region_id, region_name, -1):
    print "cannot add", region_name, "to root region"
    quit()

#==================================#
#= Read image into an image field =#
#==================================#
# create domain field
print "creating domain field..."
domain_double_array = cmiss.field.new_double_array(1)
cmiss.field.double_array_setitem(domain_double_array,1,1.0)
domain_field_id = cmiss.field.Cmiss_field_create_constant(1, domain_double_array)

# create image field
print "creating cmiss image field..."
new_field_id = cmiss.field.Cmiss_field_create_image(domain_field_id)
# cast field to a cmiss_field_image
image_field_id = cmiss.field.Cmiss_field_image_cast(new_field_id)

# read image
print "reading image file..."
image_filename = "BrainProtonDensitySlice.png"
if not cmiss.field.Cmiss_field_image_read_file(image_field_id, image_filename):
    print "cannot read image", image_filename
    quit()

#=================================#
#= write out a 100x100 thumbnail =#
#=================================#
print "creating storage information..."
output_image_filename = "thumbnail_output.png"
storage_information = cmiss.field.Cmiss_field_image_storage_information_create()
cmiss.field.Cmiss_field_image_storage_information_add_file_name(storage_information, output_image_filename)
cmiss.field.Cmiss_field_image_storage_information_set_width(storage_information, 100)
cmiss.field.Cmiss_field_image_storage_information_set_height(storage_information, 100)

print "writing to file..."
if not cmiss.field.Cmiss_field_image_write(image_field_id, storage_information):
    print "cannot write", output_image_filename
    quit()
else:
   print "file written"

#================================#
#= write binary threshold image =#
#================================#
# perform binary thresholding
print "thresholding image"
binary_threshold_field_id = cmiss.field.Cmiss_field_create_binary_threshold_image_filter(new_field_id,
	0.1, 0.5)
# add field to region for display in 3D window
binary_threshold_region_field_id = region_id.add_field(binary_threshold_field_id)


a.main_loop_run()
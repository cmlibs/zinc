function cmgui_surf(x,y,X,Y,Z)

makegrid(x,y,X,Y,Z);
cmiss_command_data;
cmiss_region;
cmiss_field;
cmiss_element;

a = cmiss_command_data.new_Cmiss_command_data();
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx re elem grid generate_faces_and_lines");

%***************************************************************
%obtain cmiss_region objects to query number of nodes in cube
%***************************************************************
region = "grid";
root_region_id = cmiss_command_data.Cmiss_command_data_get_root_region(a);
region_1_id = cmiss_region.Cmiss_region_get_sub_region(root_region_id, region);
region_1_number_of_nodes = cmiss_region.Cmiss_region_get_number_of_nodes_in_region(region_1_id);
%printf("Number of nodes in region %s: %d\n", region, region_1_number_of_nodes)

%***************************************************************
%get cmiss_field objects from a region to query number of components of a
%coordinate field  
%***************************************************************
field = "coordinates";
field_1_id = cmiss_region.Cmiss_region_find_field_by_name(region_1_id, field);
field_1_number_of_components = cmiss_field.Cmiss_field_get_number_of_components(field_1_id);
%printf("Number of components in field %s: %d\n", field, field_1_number_of_components)

% To list scene editor commands: gfx lis g_e grid comm
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx cre win");
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx define field surf composite coordinates.x coordinates.y general");
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx modify g_element grid general clear circle_discretization 6 default_coordinate coordinates element_discretization \"25*25*25\" native_discretization none;");
cmiss_command_data.Cmiss_command_data_execute_command(a, "gfx modify g_element grid surfaces coordinate surf select_on material default data general spectrum default selected_material default_selected render_shaded;");

cmiss_command_data.Cmiss_main_loop_run(a);

endfunction;

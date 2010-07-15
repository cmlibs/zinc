
# Defines API_INTERFACE_SRCS, CHOOSE_INTERFACE_SRCS, COMFILE_INTERFACE_SRCS, COLOUR_INTERFACE_SRCS, COMMAND_INTERFACE_SRCS, COMPUTED_FIELD_INTERFACE_SRCS,
# CURVE_INTERFACE_SRCS, FINITE_ELEMENT_INTERFACE_SRCS, GENERAL_INTERFACE_SRCS, GTK_INTERFACE_SRCS, GRAPHICS_INTERFACE_SRCS, INTERACTION_INTERFACE_SRCS,
# IO_DEVICES_INTERFACE_SRCS, LINK_INTERFACE_SRCS, MATERIAL_INTERFACE_SRCS, MOTIF_INTERFACE_SRCS, NODE_INTERFACE_SRCS, REGION_INTERFACE_SRCS,
# NODE_INTERFACE_SRCS, SELECT_INTERFACE_SRCS, TRANSFORMATION_INTERFACE_SRCS, THREE_D_DRAWING_INTERFACE_SRCS, TIME_INTERFACE_SRCS, USER_INTERFACE_SRCS, 
# USER_INTERFACE_INTERFACE_SRCS, VIEW_INTERFACE_SRCS

# Interface sources are ordered by ui dependence
# All interfaces first
# Most second
# Wx third
# Gtk fourth
# Motif fifth

# All
SET( USER_INTERFACE_SRCS
	source/user_interface/confirmation.cpp
	source/user_interface/event_dispatcher.cpp
	source/user_interface/filedir.cpp
	source/user_interface/message.c
	source/user_interface/user_interface.cpp
	source/user_interface/idle.c
	source/user_interface/timer.c )
SET( USER_INTERFACE_HDRS
	source/user_interface/confirmation.h
	source/user_interface/event_dispatcher.h
	source/user_interface/fd_io.h
	source/user_interface/filedir.h
	source/user_interface/gui_button.h
	source/user_interface/gui_declaration.h
	source/user_interface/gui_dialog_macros.h
	source/user_interface/gui_list.h
	source/user_interface/gui_prototype.h
	source/user_interface/gui_scroll.h
	source/user_interface/idle.h
	source/user_interface/message.h
	source/user_interface/process_list_or_write_command.hpp
	source/user_interface/timer.h
	source/user_interface/user_interface.h )

# Most
IF( WX_USER_INTERFACE OR MOTIF_USER_INTERFACE OR GTK_USER_INTERFACE OR
	CARBON_USER_INTERFACE OR WIN32_USER_INTERFACE )
	SET( API_INTERFACE_SRCS source/api/cmiss_graphics_window.c )

	SET( COMMAND_INTERFACE_SRCS source/command/command_window.cpp )
	SET( COMMAND_INTERFACE_HDRS source/command/command_window.h )

	SET( COMPUTED_FIELD_INTERFACE_SRCS
		source/computed_field/computed_field_window_projection.cpp )

	SET( GRAPHICS_INTERFACE_SRCS source/graphics/graphics_window.cpp )
ENDIF( WX_USER_INTERFACE OR	MOTIF_USER_INTERFACE OR	GTK_USER_INTERFACE OR
	CARBON_USER_INTERFACE OR WIN32_USER_INTERFACE )

# Wx
IF( WX_USER_INTERFACE )
	SET( COMFILE_INTERFACE_SRCS source/comfile/comfile_window_wx.cpp )
	SET( COMFILE_INTERFACE_HDRS
		source/comfile/comfile_window_wx.h
		source/comfile/comfile_window_wx.hpp )
		
	SET( COLOUR_INTERFACE_SRCS source/colour/colour_editor_wx.cpp )
	SET( COLOUR_INTERFACE_HDRS source/colour/colour_editor_wx.hpp )
	
	IF(  ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
		SET( MATERIAL_INTERFACE_SRCS
			source/material/material_editor_wx.cpp )
		SET( MATERIAL_INTERFACE_HDRS
			source/material/material_editor_wx.h )
	ENDIF(  ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

	SET( NODE_INTERFACE_SRCS source/node/node_viewer_wx.cpp )
	SET( NODE_INTERFACE_HDRS source/node/node_viewer_wx.h )
	
	SET( REGION_INTERFACE_SRCS source/region/cmiss_region_chooser_wx.cpp )
	
	SET( TRANSFORMATION_INTERFACE_SRCS source/transformation/transformation_editor_wx.cpp )
	SET( TRANSFORMATION_INTERFACE_HDRS source/transformation/transformation_editor_wx.hpp )
ENDIF( WX_USER_INTERFACE )

# Gtk
IF( GTK_USER_INTERFACE )
	SET( GTK_INTERFACE_SRCS source/gtk/gtk_cmiss_scene_viewer.c )
	SET( GTK_INTERFACE_HDRS source/gtk/gtk_cmiss_scene_viewer.h )
ENDIF( GTK_USER_INTERFACE )

# Motif
IF( MOTIF_USER_INTERFACE )
	SET( CHOOSE_INTERFACE_SRCS source/choose/choose_computed_field.c
		source/choose/choose_curve.c
		source/choose/choose_enumerator.c
		source/choose/choose_fe_field.c
		source/choose/choose_field_component.c
		source/choose/choose_graphical_material.c
		source/choose/choose_spectrum.c
		source/choose/choose_texture.c
		source/choose/choose_volume_texture.c
		source/choose/chooser.c
		source/choose/text_choose_fe_element.c
		source/choose/text_choose_fe_node.c )

	IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
		SET( CHOOSE_INTERFACE_SRCS ${CHOOSE_INTERFACE_SRCS}
			source/choose/choose_gt_object.c
			source/choose/choose_scene.cpp )
	ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

	SET( CHOOSE_INTERFACE_HDRS
		source/choose/chooser.h
		source/choose/choose_class.hpp
		source/choose/choose_computed_field.h
		source/choose/choose_curve.h
		source/choose/choose_element_group.h
		source/choose/choose_enumerator.h
		source/choose/choose_enumerator_class.hpp
		source/choose/choose_fe_field.h
		source/choose/choose_field_component.h
		source/choose/choose_graphical_material.h
		source/choose/choose_listbox_class.hpp
		source/choose/choose_list_class.hpp
		source/choose/choose_manager_class.hpp
		source/choose/choose_manager_listbox_class.hpp
		source/choose/choose_node_group.h
		source/choose/choose_object.h
		source/choose/choose_object_list.h
		source/choose/choose_object_list_private.h
		source/choose/choose_object_private.h
		source/choose/choose_spectrum.h
		source/choose/choose_texture.h
		source/choose/choose_volume_texture.h
		source/choose/fe_region_choose_object.h
		source/choose/fe_region_choose_object_private.h
		source/choose/text_choose_class.hpp
		source/choose/text_choose_fe_element.h
		source/choose/text_choose_fe_node.h
		source/choose/text_choose_from_fe_element.hpp
		source/choose/text_choose_from_fe_region.h
		source/choose/text_choose_from_fe_region_private.h
		source/choose/text_choose_object.h
		source/choose/text_choose_object_private.h
		source/choose/text_FE_choose_class.hpp )

	IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
		SET( CHOOSE_INTERFACE_HDRS ${CHOOSE_INTERFACE_HDRS}
			source/choose/choose_gt_object.h
			source/choose/choose_scene.h )
	ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

	SET( COLOUR_INTERFACE_SRCS
		source/colour/colour_editor.c 
		source/colour/edit_var.c )
	SET( COLOUR_INTERFACE_HDRS
		source/colour/colour_editor.h
		source/colour/edit_var.h )

	SET( COMFILE_SRCS ${COMFILE_SRCS} source/comfile/comfile_window.c )
	SET( COMFILE_HDRS ${COMFILE_HDRS} source/comfile/comfile_window.h )

	SET( CURVE_INTERFACE_SRCS source/curve/curve_editor.c source/curve/curve_editor_dialog.c )
	SET( CURVE_INTERFACE_HDRS source/curve/curve_editor.h source/curve/curve_editor_dialog.h )

	SET( FINITE_ELEMENT_INTERFACE_SRCS source/finite_element/grid_field_calculator.c )

	SET( GENERAL_INTERFACE_SRCS source/general/postscript.c )

	SET( GRAPHICS_INTERFACE_SRCS source/graphics/graphical_element_editor.c graphics/movie_graphics.c )

	IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
		SET( GRAPHICS_INTERFACE_SRCS ${GRAPHICS_INTERFACE_SRCS}
			source/graphics/scene_editor.cpp
			source/graphics/settings_editor.c
			source/graphics/spectrum_editor.c
			source/graphics/spectrum_editor_dialog.c
			source/graphics/spectrum_editor_settings.c )
	ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

	SET( INTERACTION_INTERFACE_SRCS
		source/interaction/interactive_toolbar_widget.c 
		source/interaction/select_tool.c )
	SET( INTERACTION_INTERFACE_HDRS
		source/interaction/interactive_toolbar_widget.h
		source/interaction/select_tool.h )

	SET( IO_DEVICES_INTERFACE_SRCS 
		source/io_devices/haptic_input_module.cpp
		source/io_devices/input_module.c
		source/io_devices/input_module_dialog.c
		source/io_devices/input_module_widget.c )
	SET( IO_DEVICES_INTERFACE_HDRS
		source/io_devices/haptic_input_module.h
		source/io_devices/input_module.h
		source/io_devices/input_module_dialog.h
		source/io_devices/input_module_widget.h )

	IF( USE_LINK_CMISS )
		SET( LINK_INTERFACE_SRCS source/link/cmiss.c )
		SET( LINK_INTERFACE_HDRS source/link/cmiss.h )
	ENDIF( USE_LINK_CMISS )

	IF(  ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
		SET( MATERIAL_INTERFACE_SRCS
			source/material/material_editor_dialog.c
			source/material/material_editor.cpp )
		SET( MATERIAL_INTERFACE_HDRS
			source/material/material_editor_dialog.h
			source/material/material_editor.h )
	ENDIF(  ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

	SET( MOTIF_INTERFACE_SRCS source/motif/image_utilities.c )
	SET( MOTIF_INTERFACE_HDRS source/motif/image_utilities.h )
	
	SET( NODE_INTERFACE_SRCS source/node/node_field_viewer_widget.c
		source/node/node_viewer.c
		source/node/node_viewer_widget.c )
	SET( NODE_INTERFACE_HDRS source/node/node_field_viewer_widget.h
		source/node/node_viewer.h
		source/node/node_viewer_widget.h )
		
	SET( REGION_INTERFACE_SRCS source/region/cmiss_region_chooser.c )
	SET( REGION_INTERFACE_HDRS source/region/cmiss_region_chooser.h
		source/region/cmiss_region_chooser_wx.hpp )
		
	SET( TRANSFORMATION_INTERFACE_SRCS source/transformation/transformation_editor.c )
	SET( TRANSFORMATION_INTERFACE_HDRS source/transformation/transformation_editor.h )

	SET( SELECT_INTERFACE_SRCS
		source/select/select_curve.c
		source/select/select_environment_map.c
		source/select/select_graphical_material.c
		source/select/select_private.c
		source/select/select_spectrum.c )
	SET( SELECT_INTERFACE_HDRS
		source/select/select.h
		source/select/select_curve.h
		source/select/select_environment_map.h
		source/select/select_graphical_material.h
		source/select/select_private.h
		source/select/select_spectrum.h )

	SET( THREE_D_DRAWING_INTERFACE_SRCS
		source/three_d_drawing/movie_extensions.c
		source/three_d_drawing/ThreeDDraw.c )
	SET( THREE_D_DRAWING_INTERFACE_HDRS
		source/three_d_drawing/movie_extensions.h
		source/three_d_drawing/ThreeDDraP.h
		source/three_d_drawing/ThreeDDraw.h
		source/three_d_drawing/X3d.h )

	SET( TIME_INTERFACE_SRCS source/time/time_editor.c
		source/time/time_editor_dialog.c )
	SET( TIME_INTERFACE_HDRS source/time/time_editor.h
		source/time/time_editor_dialog.h )

	SET( USER_INTERFACE_INTERFACE_SRCS
		source/user_interface/call_work_procedures.c
		source/user_interface/printer.c )
	SET( USER_INTERFACE_INTERFACE_HDRS
		source/user_interface/call_work_procedures.h
		source/user_interface/printer.h )

	SET( VIEW_INTERFACE_SRCS
		source/view/camera.c
		source/view/coord.c
		source/view/coord_trans.c
		source/view/poi.c
		source/view/vector.c
		source/view/view.c
		source/view/view_control.c )	
	SET( VIEW_INTERFACE_HDRS
		source/view/camera.h
		source/view/coord.h
		source/view/coord_trans.h
		source/view/poi.h
		source/view/relative.h
		source/view/vector.h
		source/view/view.h
		source/view/view_control.h )
ENDIF( MOTIF_USER_INTERFACE )




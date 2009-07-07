
# Defines API_INTERFACE_SRCS, CHOOSE_INTERFACE_SRCS, COMFILE_SRCS, COLOUR_INTERFACE_SRCS, COMMAND_INTERFACE_SRCS, COMPUTED_FIELD_INTERFACE_SRCS,
# CURVE_INTERFACE_SRCS, FINITE_ELEMENT_INTERFACE_SRCS, GENERAL_INTERFACE_SRCS, GRAPHICS_INTERFACE_SRCS, INTERACTION_INTERFACE_SRCS,
# IO_DEVICES_INTERFACE_SRCS, LINK_INTERFACE_SRCS, MATERIAL_INTERFACE_SRCS, MOTIF_INTERFACE_SRCS, NODE_INTERFACE_SRCS, REGION_INTERFACE_SRCS,
# SELECT_INTERFACE_SRCS, TRANSFORMATION_INTERFACE_SRCS, THREE_D_DRAWING_INTERFACE_SRCS, TIME_INTERFACE_SRCS, USER_INTERFACE_SRCS, 
# USER_INTERFACE_INTERFACE_SRCS, VIEW_INTERFACE_SRCS

SET( API_INTERFACE_SRCS api/cmiss_graphics_window.c )

SET( CHOOSE_INTERFACE_SRCS choose/choose_computed_field.c
	choose/choose_curve.c
	choose/choose_enumerator.c
	choose/choose_fe_field.c
	choose/choose_field_component.c
	choose/choose_graphical_material.c
	choose/choose_spectrum.c
	choose/choose_texture.c
	choose/choose_volume_texture.c
	choose/chooser.c
	choose/text_choose_fe_element.c
	choose/text_choose_fe_node.c )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( CHOOSE_INTERFACE_SRCS ${CHOOSE_INTERFACE_SRCS}
		choose/choose_gt_object.c
		choose/choose_scene.cpp )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

SET( COMFILE_SRCS comfile/comfile.cpp )

IF( WX_USER_INTERFACE )
	SET( COLOUR_INTERFACE_SRCS colour/colour_editor_wx.cpp )
	SET( COMFILE_SRCS ${COMFILE_SRCS} comfile/comfile_window_wx.cpp )
ELSE( WX_USER_INTERFACE )
	SET( COLOUR_INTERFACE_SRCS colour/colour_editor.c colour/edit_var.c )
ENDIF( WX_USER_INTERFACE )

IF( MOTIF_USER_INTERFACE )
	SET( COMFILE_SRCS ${COMFILE_SRCS} comfile/comfile_window.c )
ENDIF( MOTIF_USER_INTERFACE )

SET( COMMAND_INTERFACE_SRCS command/command_window.cpp )

SET( COMPUTED_FIELD_INTERFACE_SRCS computed_field/computed_field_window_projection.cpp )

SET( CURVE_INTERFACE_SRCS curve/curve_editor.c curve/curve_editor_dialog.c )

SET( FINITE_ELEMENT_INTERFACE_SRCS finite_element/grid_field_calculator.c )

SET( GENERAL_INTERFACE_SRCS general/postscript.c )

SET( GRAPHICS_INTERFACE_SRCS graphics/graphical_element_editor.c graphics/movie_graphics.c )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GRAPHICS_INTERFACE_SRCS ${GRAPHICS_INTERFACE_SRCS}
		graphics/graphics_window.cpp
		graphics/scene_editor.cpp
		graphics/settings_editor.c
		graphics/spectrum_editor.c
		graphics/spectrum_editor_dialog.c
		graphics/spectrum_editor_settings.c )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

SET( INTERACTION_INTERFACE_SRCS interaction/interactive_toolbar_widget.c interaction/select_tool.c )

SET( IO_DEVICES_INTERFACE_SRCS 
	io_devices/haptic_input_module.cpp
	io_devices/input_module.c
	io_devices/input_module_dialog.c
	io_devices/input_module_widget.c )

IF( USE_LINK_CMISS )
  SET( LINK_INTERFACE_SRCS link/cmiss.c )
ENDIF( USE_LINK_CMISS )

IF(  ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	IF( WX_USER_INTERFACE )
		SET( MATERIAL_INTERFACE_SRCS material/material_editor_dialog_wx.cpp material/material_editor_wx.cpp )
	ELSE( WX_USER_INTERFACE )
		SET( MATERIAL_INTERFACE_SRCS material/material_editor_dialog.c material/material_editor.cpp )
	ENDIF( WX_USER_INTERFACE )
ENDIF(  ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

IF( MOTIF_USER_INTERFACE )
	SET( MOTIF_INTERFACE_SRCS motif/image_utilities.c )
	SET( NODE_INTERFACE_SRCS node/node_field_viewer_widget.c
		node/node_viewer.c
		node/node_viewer_widget.c )
ENDIF( MOTIF_USER_INTERFACE )

IF( WX_USER_INTERFACE )
	SET( REGION_INTERFACE_SRCS region/cmiss_region_chooser_wx.cpp )
	SET( TRANSFORMATION_INTERFACE_SRCS transformation/transformation_editor_wx.cpp )
ELSE( WX_USER_INTERFACE )
	SET( REGION_INTERFACE_SRCS region/cmiss_region_chooser.c )
	SET( TRANSFORMATION_INTERFACE_SRCS transformation/transformation_editor.c )
ENDIF( WX_USER_INTERFACE )

SET( SELECT_INTERFACE_SRCS
	select/select_curve.c
	select/select_environment_map.c
	select/select_graphical_material.c
	select/select_private.c
	select/select_spectrum.c )

SET( THREE_D_DRAWING_INTERFACE_SRCS three_d_drawing/movie_extensions.c three_d_drawing/ThreeDDraw.c )

SET( TIME_INTERFACE_SRCS time/time_editor.c time/time_editor_dialog.c )

SET( USER_INTERFACE_SRCS
	user_interface/confirmation.cpp
	user_interface/event_dispatcher.cpp
	user_interface/filedir.cpp
	user_interface/message.c
	user_interface/user_interface.cpp
	user_interface/idle.c
	user_interface/timer.c )

SET( USER_INTERFACE_INTERFACE_SRCS
	user_interface/call_work_procedures.c
	user_interface/printer.c )

SET( VIEW_INTERFACE_SRCS
	view/camera.c
	view/coord.c
	view/coord_trans.c
	view/poi.c
	view/vector.c
	view/view.c
	view/view_control.c )	





# Defines CURVE_SRCS, ELEMENT_SRCS, EMOTER_SRCS, FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS, 
# FINITE_ELEMENT_SRCS (definition includes the previous two), HELP_SRCS, INTERACTION_SRCS, IO_DEVICES_SRCS, NODE_SRCS, 
# REGION_SRCS, SELECTION_SRCS, THREE_D_DRAWING_SRCS, TIME_SRCS

SET( CURVE_SRCS curve/curve.c )

SET( ELEMENT_SRCS element/element_operations.c
	element/element_point_tool.cpp
	element/element_tool.cpp )

IF( WX_USER_INTERFACE )
	SET( ELEMENT_SRCS ${ELEMENT_SRCS} element/element_point_viewer_wx.cpp )
ENDIF( WX_USER_INTERFACE )

SET( EMOTER_SRCS emoter/em_cmgui.c emoter/emoter_dialog.cpp )

SET( FINITE_ELEMENT_CORE_SRCS
  finite_element/export_finite_element.cpp
	finite_element/finite_element.c
	finite_element/finite_element_discretization.c
	finite_element/finite_element_helper.cpp
	finite_element/finite_element_region.c
	finite_element/finite_element_time.c
	finite_element/import_finite_element.cpp )

SET( FINITE_ELEMENT_GRAPHICS_SRCS
	finite_element/finite_element_to_graphics_object.cpp
	finite_element/finite_element_to_iso_lines.cpp
	finite_element/finite_element_to_streamlines.cpp )

SET( FINITE_ELEMENT_SRCS
	${FINITE_ELEMENT_CORE_SRCS}
	${FINITE_ELEMENT_GRAPHICS_SRCS}
	finite_element/export_cm_files.c
	finite_element/finite_element_adjacent_elements.c
	finite_element/finite_element_conversion.c
	finite_element/finite_element_to_iges.c
	finite_element/read_fieldml.c
	finite_element/snake.c
	finite_element/write_fieldml.c )

SET( NETSCAPE_HELP TRUE )
SET( HELP_SRCS help/help_interface.c )

SET( INTERACTION_SRCS
	interaction/interaction_graphics.c
	interaction/interaction_volume.c
	interaction/interactive_event.cpp
	interaction/interactive_tool.c )

SET( IO_DEVICES_SRCS
	io_devices/conversion.c
	io_devices/io_device.c
	io_devices/matrix.c )


SET( NODE_SRCS node/node_operations.c
	node/node_tool.cpp )
IF( WX_USER_INTERFACE )
	SET( NODE_SRCS ${NODE_SRCS} node/node_viewer_wx.cpp )
ENDIF( WX_USER_INTERFACE )

SET( REGION_SRCS region/cmiss_region.cpp
	region/cmiss_region_write_info.c )

SET( SELECTION_SRCS selection/any_object_selection.c
	selection/element_point_ranges_selection.c
	selection/element_selection.c
	selection/node_selection.c )
	
IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( THREE_D_DRAWING_SRCS
		three_d_drawing/graphics_buffer.cpp
		three_d_drawing/window_system_extensions.c )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

SET( TIME_SRCS time/time.c time/time_keeper.c )


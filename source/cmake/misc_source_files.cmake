
# Defines COMFILE_SRCS, CURVE_SRCS, ELEMENT_SRCS, EMOTER_SRCS, FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS, 
# FINITE_ELEMENT_SRCS (definition includes the previous two), HELP_SRCS, INTERACTION_SRCS, IO_DEVICES_SRCS, NODE_SRCS, 
# REGION_SRCS, SELECTION_SRCS, THREE_D_DRAWING_SRCS, TIME_SRCS

SET( COMFILE_SRCS source/comfile/comfile.cpp )
SET( COMFILE_HDRS source/comfile/comfile.h )

SET( CURVE_SRCS source/curve/curve.c )
SET( CURVE_HDRS source/curve/curve.h )

SET( ELEMENT_SRCS
	source/element/element_operations.cpp
	source/element/element_point_tool.cpp
	source/element/element_tool.cpp )
SET( ELEMENT_HDRS
	source/element/element_operations.h
	source/element/element_point_tool.h
	source/element/element_tool.h )

IF( WX_USER_INTERFACE )
	SET( ELEMENT_SRCS ${ELEMENT_SRCS}
		source/element/element_point_viewer_wx.cpp )
	SET( ELEMENT_HDRS ${ELEMENT_HDRS}
		source/element/element_point_viewer_wx.h )
ENDIF( WX_USER_INTERFACE )
IF( MOTIF_USER_INTERFACE )
	SET( ELEMENT_HDRS ${ELEMENT_HDRS}
		source/element/element_point_viewer_wx.cpp )
#	source/element/element_creator.h
#	source/element/element_point_viewer.h
ENDIF( MOTIF_USER_INTERFACE )

SET( EMOTER_SRCS
	source/emoter/em_cmgui.c
	source/emoter/emoter_dialog.cpp )
SET( EMOTER_HDRS
	source/emoter/emoter_dialog.h 
	source/emoter/em_cmgui.h )

SET( NETSCAPE_HELP TRUE )
SET( HELP_SRCS source/help/help_interface.c )
SET( HELP_HDRS source/help/help.h
	source/help/help_interface.h
	source/help/help_window.h
	source/help/help_work.h )

SET( INTERACTION_SRCS
	source/interaction/interaction_graphics.c
	source/interaction/interaction_volume.c
	source/interaction/interactive_event.cpp
	source/interaction/interactive_tool.c )
SET( INTERACTION_HDRS
	source/interaction/interaction_graphics.h
	source/interaction/interaction_volume.h
	source/interaction/interactive_event.h
	source/interaction/interactive_tool.h
	source/interaction/interactive_tool_private.h )


SET( IO_DEVICES_SRCS
	source/io_devices/conversion.c
	source/io_devices/io_device.c
	source/io_devices/matrix.c )
SET( IO_DEVICES_HDRS
	source/io_devices/conversion.h
	source/io_devices/gstScene.h
	source/io_devices/gstTransform.h
	source/io_devices/io_device.h
	source/io_devices/matrix.h )

SET( FIELD_IO_SRCS
	source/field_io/read_fieldml_02.cpp )
SET( FIELD_IO_HDRS
	source/field_io/read_fieldml_02.h )

SET( MAIN_SRCS source/cmgui.c )

SET( NODE_SRCS source/node/node_operations.c
	source/node/node_tool.cpp )
SET( NODE_HDRS source/node/node_operations.h
	source/node/node_tool.h )

SET( REGION_SRCS source/region/cmiss_region.cpp
	source/region/cmiss_region_write_info.c )
SET( REGION_HDRS source/region/cmiss_region.h
	source/region/cmiss_region_private.h
	source/region/cmiss_region_write_info.h )

SET( SELECTION_SRCS source/selection/any_object_selection.c
	source/selection/element_point_ranges_selection.c
	source/selection/element_selection.c
	source/selection/node_selection.c )
SET( SELECTION_HDRS source/selection/any_object_selection.h
	source/selection/element_point_ranges_selection.h
	source/selection/element_selection.h
	source/selection/node_selection.h )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( THREE_D_DRAWING_SRCS
		source/three_d_drawing/graphics_buffer.cpp
		source/three_d_drawing/window_system_extensions.c )
	SET( THREE_D_DRAWING_HDRS
		source/three_d_drawing/graphics_buffer.h
		source/three_d_drawing/window_system_extensions.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

SET( TIME_SRCS source/time/time.c source/time/time_keeper.c )
SET( TIME_HDRS
	source/time/time.h
	source/time/time_keeper.h
	source/time/time_private.h )


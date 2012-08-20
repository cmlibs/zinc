
# Defines API_INTERFACE_SRCS, CHOOSE_INTERFACE_SRCS, COMFILE_INTERFACE_SRCS, COLOUR_INTERFACE_SRCS, COMMAND_INTERFACE_SRCS, COMPUTED_FIELD_INTERFACE_SRCS,
# CURVE_INTERFACE_SRCS, FINITE_ELEMENT_INTERFACE_SRCS, GENERAL_INTERFACE_SRCS, GTK_INTERFACE_SRCS, GRAPHICS_INTERFACE_SRCS, INTERACTION_INTERFACE_SRCS,
# IO_DEVICES_INTERFACE_SRCS, LINK_INTERFACE_SRCS, MATERIAL_INTERFACE_SRCS, NODE_INTERFACE_SRCS, REGION_INTERFACE_SRCS,
# NODE_INTERFACE_SRCS, SELECT_INTERFACE_SRCS, TRANSFORMATION_INTERFACE_SRCS, THREE_D_DRAWING_INTERFACE_SRCS, TIME_INTERFACE_SRCS, USER_INTERFACE_SRCS, 
# USER_INTERFACE_INTERFACE_SRCS, VIEW_INTERFACE_SRCS

# Interface sources are ordered by ui dependence
# All interfaces first
# Most second
# Wx third
# Gtk fourth

# All
SET( USER_INTERFACE_SRCS
	source/ui/confirmation.cpp
	source/ui/event_dispatcher.cpp
	source/ui/filedir.cpp
	source/ui/message.c
	source/ui/user_interface.cpp
	source/ui/idle.c )
SET( USER_INTERFACE_HDRS
	source/ui/confirmation.h
	source/ui/event_dispatcher.h
	source/ui/fd_io.h
	source/ui/filedir.h
	source/ui/idle.h
	source/ui/message.h
	source/ui/process_list_or_write_command.hpp
	source/ui/user_interface.h )

# Most
IF( WX_USER_INTERFACE OR GTK_USER_INTERFACE OR
	CARBON_USER_INTERFACE OR WIN32_USER_INTERFACE )
	SET( API_INTERFACE_SRCS )

	SET( COMMAND_INTERFACE_SRCS source/ui/command_window.cpp )
	SET( COMMAND_INTERFACE_HDRS source/ui/command_window.h )

	SET( COMPUTED_FIELD_INTERFACE_SRCS
		source/ui/computed_field_scene_viewer_projection.cpp )

	SET( GRAPHICS_INTERFACE_SRCS source/ui/graphics_window.cpp )
	SET( GRAPHICS_INTERFACE_HDRS source/ui/graphics_window.h
		source/ui/graphics_window_private.hpp )
ENDIF( WX_USER_INTERFACE OR GTK_USER_INTERFACE OR
	CARBON_USER_INTERFACE OR WIN32_USER_INTERFACE )

# Wx
IF( WX_USER_INTERFACE )
	SET( USER_INTERFACE_HDRS ${USER_INTERFACE_HDRS}
		source/ui/user_interface_wx.hpp )
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




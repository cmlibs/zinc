
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
	#source/ui/confirmation.cpp
	source/user_interface/event_dispatcher.cpp
	source/user_interface/filedir.cpp
	#source/ui/message.c
	source/user_interface/user_interface.cpp
	source/user_interface/idle.c )
SET( USER_INTERFACE_HDRS
#	source/ui/confirmation.h
	source/user_interface/event_dispatcher.h
	source/user_interface/filedir.h
	source/user_interface/idle.h
#	source/ui/message.h
	source/user_interface/process_list_or_write_command.hpp
	source/user_interface/user_interface.h )

# Most
IF( WX_USER_INTERFACE OR GTK_USER_INTERFACE OR
	CARBON_USER_INTERFACE OR WIN32_USER_INTERFACE )
	SET( API_INTERFACE_SRCS )

	SET( COMPUTED_FIELD_INTERFACE_SRCS
		source/user_interface/computed_field_scene_viewer_projection.cpp )
ENDIF( WX_USER_INTERFACE OR GTK_USER_INTERFACE OR
	CARBON_USER_INTERFACE OR WIN32_USER_INTERFACE )

# Wx
IF( WX_USER_INTERFACE )
ENDIF( WX_USER_INTERFACE )

# Gtk
IF( GTK_USER_INTERFACE )
ENDIF( GTK_USER_INTERFACE )




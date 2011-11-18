
# Defines COMMAND_SRCS, CONTEXT_SRCS

SET( CONTEXT_SRCS source/context/context.cpp
	source/context/user_interface_module.cpp )

SET( CONTEXT_HDRS source/context/context.h
	source/context/user_interface_module.h )

SET( COMMAND_SRCS source/command/command.c
	source/command/console.c
	source/command/example_path.c
	source/command/parser.c )
  
 SET( COMMAND_HDRS
 	source/command/command.h
	source/command/console.h
	source/command/example_path.h
	source/command/parser.h	)

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( COMMAND_SRCS ${COMMAND_SRCS} source/command/cmiss.cpp )
	SET( COMMAND_HDRS ${COMMAND_HDRS}
		source/command/cmiss.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )


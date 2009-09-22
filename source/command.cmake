
# Defines COMMAND_SRCS

SET( COMMAND_SRCS source/command/command.c
	source/command/console.c
	source/command/example_path.c
	source/command/parser.c )
  
 SET( COMMAND_HDRS
 	source/command/command.h
	source/command/console.h
	source/command/example_path.h
	source/command/parser.h
	source/command/pass_through_interpreter.h )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( COMMAND_SRCS ${COMMAND_SRCS} source/command/cmiss.cpp )
	SET( COMMAND_HDRS ${COMMAND_HDRS}
		source/command/cmiss.h
		source/command/cmiss_unemap_link.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )


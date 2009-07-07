
# Defines COMMAND_SRCS

SET( COMMAND_SRCS command/command.c
  command/console.c
  command/example_path.c
  command/parser.c )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( COMMAND_SRCS ${COMMAND_SRCS} command/cmiss.cpp )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )


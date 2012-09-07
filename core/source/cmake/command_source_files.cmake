
# Defines COMMAND_SRCS, CONTEXT_SRCS

SET( CONTEXT_SRCS source/context/context.cpp )
SET( CONTEXT_HDRS source/context/context.h )

SET( COMMAND_SRCS source/command/command.c )
SET( COMMAND_HDRS source/command/command.h )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( COMMAND_SRCS ${COMMAND_SRCS} source/command/cmiss.cpp )
	SET( COMMAND_HDRS ${COMMAND_HDRS} source/command/cmiss.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )


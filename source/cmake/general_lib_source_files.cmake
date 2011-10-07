
# Defines GENERAL_LIB_SRCS

SET( GENERAL_LIB_SRCS 
	source/command/parser.c
	source/general/any_object.c
	source/general/compare.c
	source/general/debug.c
	source/general/geometry.cpp
	source/general/heapsort.c
	source/general/matrix_vector.c
	source/general/multi_range.c
	source/general/myio.c
	source/general/mystring.c
	source/general/value.c
	source/user_interface/message.c )

SET( GENERAL_LIB_HDRS 
	source/api/zn_core.h
	source/command/parser.h
	source/general/any_object.h
	source/general/compare.h
	source/general/debug.h
	source/general/geometry.h
	source/general/heapsort.h
	source/general/matrix_vector.h
	source/general/multi_range.h
	source/general/myio.h
	source/general/mystring.h
	source/general/value.h
	source/user_interface/message.h )


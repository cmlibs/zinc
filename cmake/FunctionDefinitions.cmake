
FUNCTION( WXWIDGETS_HDRS XRC_SRCS _OUTFILES )

	FOREACH( XRC_SRC ${XRC_SRCS} )
		STRING( REGEX MATCH "(.*)/([a-z_A-Z]*)\\.xrc$" DUMMY "${XRC_SRC}" )
		IF( NOT EXISTS ${PROJECT_BINARY_DIR}/${CMAKE_MATCH_1} )
			FILE( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_MATCH_1} )
		ENDIF( NOT EXISTS ${PROJECT_BINARY_DIR}/${CMAKE_MATCH_1} )
		SET( XRC_FCN ${CMAKE_MATCH_2} )
		WXWIDGETS_ADD_RESOURCES( XRC_OUTPUT ${XRC_SRC} OPTIONS --cpp-code
			--function=wxXmlInit_${XRC_FCN} --output=${PROJECT_BINARY_DIR}/${XRC_SRC}h )
	ENDFOREACH( XRC_SRC ${XRC_SRCS} )

	SET( ${_OUTFILES} ${XRC_OUTPUT} PARENT_SCOPE )
ENDFUNCTION( WXWIDGETS_HDRS _OUTFILES )

FUNCTION( TEST_FOR_VFSCANF HAVE_VFSCANF )

	INCLUDE( CheckCSourceCompiles )
	CHECK_C_SOURCE_COMPILES( 
		"#include <stdio.h>
		#include <stdarg.h>

		int vread(FILE *stream, char *fmt, ...)
		{
			int rc;
			va_list arg_ptr;
			va_start(arg_ptr, fmt);
			rc = vfscanf(stream, fmt, arg_ptr);
			va_end(arg_ptr);
			return(rc);
		}
 
		#define MAX_LEN 80
		int main(void)
		{
			FILE *stream;
			char s[MAX_LEN + 1];
			/* Put in various data. */
			stream = fopen( \"CMakeCache.txt\", \"r\" );
			return vread(stream, \"%s\", &s[0]);
		}" VFSCANF )
			SET( ${HAVE_VFSCANF} ${VFSCANF} PARENT_SCOPE )
ENDFUNCTION( TEST_FOR_VFSCANF HAVE_VFSCANF )

FUNCTION(GROUP_SOURCE_FILES)
	FOREACH(F ${ARGV})
		STRING(REGEX REPLACE ".*source/" "" SRC_RELATIVE_F ${F})
		STRING(REGEX MATCHALL "([^/]+/)" RESULT ${SRC_RELATIVE_F})
		SET(FOLDER "${RESULT}")
		LIST(LENGTH RESULT RESULT_LEN)
		IF(${RESULT_LEN} GREATER 0)
			STRING(REGEX REPLACE "/;|/" "\\\\" FOLDER_MSVC ${FOLDER}) 
			# If the file doesn't end in a 'c' or 'cpp' then it's a header file
			STRING(REGEX MATCH "c[p]*$" SOURCE_FILE ${F})
			IF(SOURCE_FILE)
				SET(F_LOCATION "Source Files\\${FOLDER_MSVC}")
			ELSE()
				SET(F_LOCATION "Header Files\\${FOLDER_MSVC}")
			ENDIF()
		ELSE()
			STRING(REGEX MATCH "c[p]*$" SOURCE_FILE ${F})
			IF(SOURCE_FILE)
				SET(F_LOCATION "Source Files")
			ELSE()
				SET(F_LOCATION "Header Files")
			ENDIF()
		ENDIF()
		SOURCE_GROUP(${F_LOCATION} FILES ${F})
	ENDFOREACH()
ENDFUNCTION()

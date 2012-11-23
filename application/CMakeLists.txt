
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.1 )

PROJECT(CmguiApplication)

FIND_PACKAGE( perl_interpreter QUIET HINTS ${CMGUI_INSTALL_PREFIX}/lib/cmake )

OPTION( USE_PERL_INTERPRETER "Do you want to use the perl interpreter?" FALSE )
OPTION( WX_USER_INTERFACE "use wx for interface." )
OPTION( GTK_USER_INTERFACE "use gtk for interface." )
OPTION( WIN32_USER_INTERFACE "use win32 for interface." )

IF(WIN32)
    SET( WIN32_SYSTEM TRUE )
ENDIF()

IF( WX_USER_INTERFACE )
    SET( CMAKE_PREFIX_PATH ${wxWidgets_DIR} )
    FIND_PACKAGE( wxWidgets QUIET COMPONENTS xrc adv html gl xml core base )
    SET( CAN_USE_USER_INTERFACE ${wxWidgets_FOUND} )
ENDIF( WX_USER_INTERFACE )
IF( GTK_USER_INTERFACE )
    FIND_PACKAGE( GTK2 REQUIRED gtk)
    FIND_PACKAGE( GTKGLext REQUIRED )
    SET( CAN_USE_USER_INTERFACE $(GTK2_FOUND) )
ENDIF( GTK_USER_INTERFACE )
IF( WIN32_USER_INTERFACE AND WIN32 )
    SET( CAN_USE_USER_INTERFACE TRUE )
ENDIF( WIN32_USER_INTERFACE AND WIN32 )

# User interfaces
# wxwidgets - is a config file on win32 and not on configure enabled platforms
IF( WX_USER_INTERFACE )
    IF( WIN32 AND NOT CYGWIN AND NOT MSYS )
        # Using a config file so add this to the dependent configs
        SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nINCLUDE(${wxWidgets_DIR}/wxWidgets-config.cmake)" )
    ELSE( WIN32 AND NOT CYGWIN AND NOT MSYS )
        IF( UNIX OR MSYS )
            IF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
                SET( wxWidgets_USE_DEBUG TRUE )
            ELSE( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
                SET( wxWidgets_USE_DEBUG FALSE )
            ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
            SET( wxWidgets_USE_UNICODE FALSE )
            SET( wxWidgets_USE_UNIVERSAL FALSE )
            SET( wxWidgets_USE_STATIC TRUE )
            FIND_PACKAGE( wxWidgets QUIET COMPONENTS xrc adv html
                gl xml core base )
        ENDIF( UNIX OR MSYS )
    ENDIF( WIN32 AND NOT CYGWIN AND NOT MSYS )
    IF( wxWidgets_FOUND )
        # Order of components here is important when linking static libraries
        SET( USE_WXWIDGETS TRUE )
        MARK_AS_ADVANCED( wxWidgets_DIR )
        DBG_MSG( "C FLAGS: ${wxWidgets_CXX_FLAGS}" )
        DBG_MSG( "LIBS: ${wxWidgets_LIBRARIES}" )
        SET( WXWIDGETS_LIBRARIES ${wxWidgets_LIBRARIES} )
        DBG_MSG( "DEFS: ${wxWidgets_DEFINITIONS}" )
        DBG_MSG( "INC_DIRS: ${wxWidgets_INCLUDE_DIRS}" )
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}" )
        SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${wxWidgets_LIBRARIES} )
        SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} )
        #SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} ${wxWidgets_DEFINITIONS} )
        SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} ${wxWidgets_DEFINITIONS} )
        SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nSET( wxWidgets_wxrc_EXECUTABLE ${wxWidgets_wxrc_EXECUTABLE} )" )
        IF( MSVC )
            SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} wxUSE_NO_MANIFEST=1 )
            SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} wxUSE_NO_MANIFEST=1 )
        ENDIF( MSVC )
        IF( UNIX AND NOT APPLE ) # A not very good way of getting wxWidgets to compile with gcc 4.6.2
            SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} /usr/lib/libX11.so )
        ENDIF()
        INCLUDE( source/cmake/xrc_source_files.cmake )
        DBG_MSG( "XRC SRCS: ${XRC_SRCS}" )
        WXWIDGETS_HDRS( "${XRC_SRCS}" wxWidgets_GENERATED_HDRS )
    ELSE( wxWidgets_FOUND )
        MESSAGE( FATAL_ERROR "wxWidgets was requested but not found." )
    ENDIF( wxWidgets_FOUND )
ENDIF( WX_USER_INTERFACE )

IF( GTK_USER_INTERFACE )
    IF( GTK2_FOUND )
        SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${GTK2_LIBRARIES} )
        SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS} )
        IF( GTKGLEXT_FOUND )
            SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${GTKGLEXT_LIBRARIES} )
            SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${GTKGLEXT_INCLUDE_DIRS} )
        ELSE( GTKGLEXT_FOUND )
            MESSAGE( FATAL_ERROR "gtkglext was requested but not found." )
        ENDIF( GTKGLEXT_FOUND )
    ELSE( GTK2_FOUND )
        MESSAGE( FATAL_ERROR "gtk2 was requested but not found." )
    ENDIF( GTK2_FOUND )
ENDIF( GTK_USER_INTERFACE )

INCLUDE(source/cmake/app_srcs.cmake)
INCLUDE(source/cmake/app_hdrs.cmake)

SET( CMISS_VERSION_STRING "\"${ZINC_VERSION}\"" )
SET( CMISS_SVN_REVISION_STRING "\"xxxx\"" )
SET( CMISS_BUILD_STRING "\"wx os\"" )
SET( CMGUI_CONFIGURE_HDR ${PROJECT_BINARY_DIR}/source/configure/cmgui_configure.h )
SET( CMGUI_VERSION_HDR ${PROJECT_BINARY_DIR}/source/configure/version.h )
CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/source/configure/cmgui_configure.h.cmake ${CMGUI_CONFIGURE_HDR} )
CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/source/configure/version.h.cmake ${CMGUI_VERSION_HDR} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/source ${CMAKE_CURRENT_BINARY_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/source ${CMAKE_CURRENT_BINARY_DIR}/../core/source
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/source/api ${wxWidgets_INCLUDE_DIRS} ${FIELDML_INCLUDE_DIRS}
    ${ITK_INCLUDE_DIRS} ${PERL_INTERPRETER_INCLUDE_DIRS} )

FOREACH( DEF ${EXTRA_COMPILER_DEFINITIONS} ${DEPENDENT_DEFINITIONS} )
    ADD_DEFINITIONS( -D${DEF} )
ENDFOREACH( DEF ${EXTRA_COMPILER_DEFINITIONS} ${DEPENDENT_DEFINITIONS} )

ADD_EXECUTABLE( cmgui WIN32 ${APP_SRCS} ${APP_HDRS} ${CMGUI_CONFIGURE_HDR} ${CMGUI_VERSION_HDR} ${wxWidgets_GENERATED_HDRS} )

TARGET_LINK_LIBRARIES( cmgui zinc-static ${PERL_INTERPRETER_LIBRARIES} ${WXWIDGETS_LIBRARIES} )


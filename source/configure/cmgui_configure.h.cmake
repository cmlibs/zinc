
#ifndef CMGUI_CONFIGURE_H
#define CMGUI_CONFIGURE_H

// Platform specific defines
#cmakedefine UNIX
#cmakedefine APPLE
#cmakedefine CYGWIN
#cmakedefine WIN32_SYSTEM

// Operating specific defines
#cmakedefine SGI
#cmakedefine GENERIC_PC

// User interface specific defines
#cmakedefine MOTIF_USER_INTERFACE
#cmakedefine WIN32_USER_INTERFACE
#cmakedefine GTK_USER_INTERFACE
#cmakedefine WX_USER_INTERFACE
#cmakedefine CARBON_USER_INTERFACE
#cmakedefine CONSOLE_USER_INTERFACE
#cmakedefine USE_GTK_MAIN_STEP
#cmakedefine TARGET_API_MAC_CARBON

// Graphics specific defines
#cmakedefine OPENGL_API
#cmakedefine DM_BUFFERS
#cmakedefine SELECT_DESCRIPTORS
#cmakedefine REPORT_GL_ERRORS
#cmakedefine USE_PARAMETER_ON

// Extension specific defines
#cmakedefine USE_OPENCASCADE
#cmakedefine USE_IMAGEMAGICK
#cmakedefine USE_ITK
#cmakedefine USE_PERL_INTERPRETER
#cmakedefine USE_XML2
#cmakedefine USE_NETGEN
#cmakedefine USE_GLEW
#cmakedefine USE_PNG
#cmakedefine USE_TIFF

// Miscellaneous defines
#cmakedefine NETSCAPE_HELP
#cmakedefine FE_VALUE_IS_DOUBLE
#cmakedefine HAVE_VFSCANF
#cmakedefine GLEW_STATIC
#cmakedefine USE_MSAA
#cmakedefine OPTIMISED
#cmakedefine DEBUG
#cmakedefine MEMORY_CHECKING

#endif


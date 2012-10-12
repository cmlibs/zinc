
#ifndef CMISS_ZINC_CONFIGURE_H
#define CMISS_ZINC_CONFIGURE_H

// Platform specific defines
#cmakedefine UNIX
#cmakedefine DARWIN
#cmakedefine CYGWIN
#cmakedefine WIN32_SYSTEM

// Operating specific defines
#cmakedefine SGI
#cmakedefine GENERIC_PC

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
#cmakedefine USE_NETGEN
#cmakedefine USE_GLEW
#cmakedefine USE_PNG
#cmakedefine USE_TIFF

// Miscellaneous defines
#cmakedefine HAVE_VFSCANF
#cmakedefine GLEW_STATIC
#cmakedefine USE_MSAA
#cmakedefine OPTIMISED
#cmakedefine MEMORY_CHECKING
#cmakedefine ZINC_NO_STDOUT

typedef @FE_value@ FE_value;
#cmakedefine FE_VALUE_INPUT_STRING @FE_VALUE_INPUT_STRING@
#cmakedefine FE_VALUE_STRING @FE_VALUE_STRING@
typedef @ZnReal@ ZnReal;
typedef @COLOUR_PRECISION@ COLOUR_PRECISION;
typedef @MATERIAL_PRECISION@ MATERIAL_PRECISION;

#endif


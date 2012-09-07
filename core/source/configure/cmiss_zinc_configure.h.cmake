
#ifndef ZINC_CONFIGURE_H
#define ZINC_CONFIGURE_H

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
#cmakedefine FE_VALUE_IS_DOUBLE
#cmakedefine HAVE_VFSCANF
#cmakedefine GLEW_STATIC
#cmakedefine USE_MSAA
#cmakedefine OPTIMISED
#cmakedefine MEMORY_CHECKING
#cmakedefine ZINC_NO_STDOUT

// Shared object export defines
#cmakedefine ZINC_SHARED_OBJECT
#cmakedefine ZINC_EXPORTS

#if defined _WIN32 || defined __CYGWIN__
  #define ZINC_DLL_IMPORT __declspec(dllimport)
  #define ZINC_DLL_EXPORT __declspec(dllexport)
#else
  #if __GNUC__ >= 4
    #define ZINC_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define ZINC_DLL_EXPORT __attribute__ ((visibility ("default")))
  #else
    #define ZINC_DLL_IMPORT
    #define ZINC_DLL_EXPORT
  #endif
#endif

// Now we use the generic helper definitions above to define ZINC_API.
// ZINC_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)

#ifdef ZINC_SHARED_OBJECT // defined if Zinc is compiled as a shared object
  #ifdef ZINC_EXPORTS // defined if we are building the Zinc DLL (instead of using it)
    #define ZINC_API ZINC_DLL_EXPORT
  #else
    #define ZINC_API ZINC_DLL_IMPORT
  #endif // ZINC_EXPORTS
#else // ZINC_SHARED_OBJECT is not defined: this means Zinc is a static lib.
  #define ZINC_API
#endif // ZINC_SHARED_OBJECT


#endif


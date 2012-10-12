
#ifndef CMISS_SHARED_OBJECT_H
#define CMISS_SHARED_OBJECT_H

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


#ifndef _TUTK_GLOBAL_DEFS_H_
#define _TUTK_GLOBAL_DEFS_H_

#ifdef _WIN32
    /** @cond */
    #ifdef IOTC_STATIC_LIB
    #define TUTK_GLOBAL_APIs
    #elif defined P2PAPI_EXPORTS
    #define TUTK_GLOBAL_APIs __declspec(dllexport)
    #else
    #define TUTK_GLOBAL_APIs __declspec(dllimport)
    #endif // #ifdef P2PAPI_EXPORTS
    /** @endcond */
#else // #ifdef _WIN32
    #define TUTK_GLOBAL_APIs
#endif //#ifdef _WIN32

#if defined(__GNUC__) || defined(__clang__)
    #define TUTK_GLOBAL_APIs_DEPRECATED __attribute__((deprecated))
    #elif defined(_MSC_VER)
    #ifdef IOTC_STATIC_LIB
    #define TUTK_GLOBAL_APIs_DEPRECATED __declspec(deprecated)
    #elif defined P2PAPI_EXPORTS
    #define TUTK_GLOBAL_APIs_DEPRECATED __declspec(deprecated, dllexport)
    #else
    #define TUTK_GLOBAL_APIs_DEPRECATED __declspec(deprecated, dllimport)
    #endif
#else
    #define TUTK_GLOBAL_APIs_DEPRECATED
#endif

#ifndef _WIN32
#define __stdcall
#endif // #ifndef _WIN32

#endif /* _TUTK_GLOBAL_DEFS_H_ */

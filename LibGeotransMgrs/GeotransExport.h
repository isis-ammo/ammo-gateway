
#if defined _WIN32 || defined __CYGWIN__
  #ifdef GEOTRANS_DLL_BUILD
    #ifdef __GNUC__
      #define GEOTRANS_API __attribute__((dllexport))
    #else
      #define GEOTRANS_API __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define GEOTRANS_API __attribute__((dllimport))
    #else
      #define GEOTRANS_API __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define GEOTRANS_LOCAL
#else
  #if __GNUC__ >= 4
    #define GEOTRANS_API __attribute__ ((visibility("default")))
    #define GEOTRANS_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define GEOTRANS_API
    #define GEOTRANS_LOCAL
  #endif
#endif
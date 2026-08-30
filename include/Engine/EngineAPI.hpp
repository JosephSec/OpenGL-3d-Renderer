#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(ZENGINE_DLL_BUILD)
    #define ZENGINE_API __declspec(dllexport)
  #elif defined(ZENGINE_DLL)
    #define ZENGINE_API __declspec(dllimport)
  #else
    #define ZENGINE_API
  #endif
#else
  #define ZENGINE_API
#endif
#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef UI_DLL_BUILD
    #define UI_API __declspec(dllexport)
  #elif UI_DLL
    #define UI_API __declspec(dllimport)
  #else
    #define UI_API
  #endif
#else
  #define UI_API
#endif
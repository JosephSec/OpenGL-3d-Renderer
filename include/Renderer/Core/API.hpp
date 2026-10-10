#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(RENDER3D_DLL_BUILD)
    #define RENDER3D_API __declspec(dllexport)
  #elif defined(RENDER3D_DLL)
    #define RENDER3D_API __declspec(dllimport)
  #else
    #define RENDER3D_API
  #endif
#else
  #define RENDER3D_API
#endif
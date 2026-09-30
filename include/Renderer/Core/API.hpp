#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(HORDE3D_DLL_BUILD)
    #define HORDE3D_API __declspec(dllexport)
  #elif defined(HORDE3D_DLL)
    #define HORDE3D_API __declspec(dllimport)
  #else
    #define HORDE3D_API
  #endif
#else
  #define HORDE3D_API
#endif
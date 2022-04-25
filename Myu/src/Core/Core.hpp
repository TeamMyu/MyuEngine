#pragma once

#ifdef MYU_BUILD_DLL
#define MYU_API __declspec(dllexport)
#else
#define MYU_API __declspec(dllimport)
#endif

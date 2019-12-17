#pragma once

#ifdef INKCPP_API
#define INK_ENABLE_UNREAL
#else
#define INK_ENABLE_STL
#define INK_ENABLE_CSTD
#endif

// Only turn on if you have json.hpp and you want to use it with the compiler
// #define INK_EXPOSE_JSON
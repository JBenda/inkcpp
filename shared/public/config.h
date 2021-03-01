#pragma once

#ifdef INKCPP_API
#define INK_ENABLE_UNREAL
#else
#define INK_ENABLE_STL
#define INK_ENABLE_CSTD
#endif

// Only turn on if you have json.hpp and you want to use it with the compiler
// #define INK_EXPOSE_JSON

namespace ink::config {
	static constexpr unsigned limitGlobalVariables = 50;
	static constexpr unsigned limitThreadDepth = 20;
	static constexpr unsigned limitEvalStackDepth = 20;
	static constexpr unsigned limitContainerDepth = 20;
	// temporary variables and callstack;
	static constexpr unsigned limitRuntimeStack = 50;
	// max number of elements in one output (a string is one element)
	static constexpr unsigned limitOutputSize = 200;
	// max number of choices per choice
	static constexpr unsigned maxChoices = 10;
}

/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

// The UE build process will define INKCPP_API
#ifdef INKCPP_API
#	define INK_ENABLE_UNREAL
#	define INKCPP_NO_RTTI
#	define INKCPP_NO_EXCEPTIONS
#elif defined(INKCPP_BUILD_CLIB)
#	ifndef INKCPP_NO_STD
#		define INK_ENABLE_CSTD
#	endif
#else
#	ifndef INKCPP_NO_STD
#		define INK_ENABLE_STL
#		define INK_ENABLE_CSTD
#	endif
#endif

#ifndef INKCPP_NO_RTTI
#	define INK_ENABLE_RTTI
#endif

#ifndef INKCPP_NO_EXCEPTIONS
#	define INK_ENABLE_EXCEPTIONS
#endif

// Only turn on if you have json.hpp and you want to use it with the compiler
// #define INK_EXPOSE_JSON

namespace ink::config
{
/// set limitations which are required to minimize heap allocations.
/// if required you can set them to -x then, the system will use dynamic
/// allocation for this type, with an initial size of x.
static constexpr int limitGlobalVariables         = -50;
static constexpr int limitGlobalVariableObservers = -10;
static constexpr int limitThreadDepth             = -10;
static constexpr int limitEvalStackDepth          = -20;
static constexpr int limitContainerDepth          = -20;
/** number of lists which can be accessed with get_var
 *  before the story must continue
 * @attention list vars are only valid until the story continuous!
 */
static constexpr int limitEditableLists           = -5;
/// number of simultaneous active tags
static constexpr int limitActiveTags              = -10;
// temporary variables and call stack;

static constexpr int limitRuntimeStack   = -20;
// references and call stack
static constexpr int limitReferenceStack = -20;
// max number of elements in one output (a string is one element)
static constexpr int limitOutputSize     = -100;
// maximum number of text fragments between choices
static constexpr int limitStringTable    = -100;
// max number of choices per choice
static constexpr int maxChoices          = -10;
// max number of list types, and there total amount of flags
static constexpr int maxListTypes        = -20;
static constexpr int maxFlags            = -200;
// number of max initialized lists
static constexpr int maxLists            = -50;
// max number of arguments for external functions (dynamic not possible)
static constexpr int maxArrayCallArity   = 10;

namespace statistics
{
	struct container {
		int capacity;
		int size;
	};

	struct list_table {
		container editable_lists; /** based on @ref limitEditableLists */
		container list_types;     /** based on @ref maxListTypes */
		container flags;          /** based on @ref maxFlags */
		container lists;          /** based on @ref maxLists */
	};

	struct string_table {
		container string_refs; /** based on @ref limitStringTable */
	};

	struct global {
		container    variables;           /** based on @ref limitGlobalVariables */
		container    variables_observers; /** based on @ref limitGlobalVariableObservers */
		list_table   lists;
		string_table strings;
	};

	struct runner {
		container threads;           /** based on @ref limitThreadDepth */
		container evaluation_stack;  /** based on @ref limitEvalStackDepth */
		container container_stack;   /** based on @ref limitContainerDepth */
		container active_tags;       /** based on @ref limitActiveTags */
		container runtime_stack;     /** based on @ref limitContainerDepth  */
		container runtime_ref_stack; /** based on @ref limitReferenceStack */
		container output;            /** based on @ref limitOutputSize */
		container choices;           /** based on @ref limitContainerDepth */
	};
} // namespace statistics
} // namespace ink::config

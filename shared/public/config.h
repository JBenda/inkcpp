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
/**
 * set limitations which are required to minimize heap allocations.
 * if required you can set them to -x then, the system will use dynamic
 * allocation for this type, with an initial size of x.
 */
namespace ink::config
{
/** amount of global variables in the script. */
constexpr int limitGlobalVariables         = -50;
/** amount of simustanly registerd variable observers. */
constexpr int limitGlobalVariableObservers = -10;
/** maximum amount of tunnel/choice inception. */
constexpr int limitThreadDepth             = -10;
/** maximum size of the evaluation stack.
 * Each operation inside an expression needs at least 3 slots.
 * Also string building for choices with @c [] syntax will use the stack.
 */
constexpr int limitEvalStackDepth          = -20;
/** maximum number of cascaded nodes.
 * beside stitches and knots, choices are also containers.
 */
constexpr int limitContainerDepth          = -20;
/** number of lists which can be accessed with get_var
 *  before the story must continue
 * @attention list vars are only valid until the story continuous!
 */
constexpr int limitEditableLists           = -5;
/** number of simultaneous active tags. */
constexpr int limitActiveTags              = -10;
/** temporary variables and call stack. */
constexpr int limitRuntimeStack            = -20;
/** references and call stack. */
constexpr int limitReferenceStack          = -20;
/** max number of elements in one output (a string is one element). */
constexpr int limitOutputSize              = -100;
/** maximum number of text fragments between choices. */
constexpr int limitStringTable             = -100;
/** max number of choices per choice. */
constexpr int maxChoices                   = -10;
/** max number of list types, and there total amount of flags. */
constexpr int maxListTypes                 = -20;
/** maximum number of defined list flags. */
constexpr int maxFlags                     = -200;
/** number of max initialized lists. */
constexpr int maxLists                     = -50;
/** max number of arguments for external functions (dynamic not possible). */
constexpr int maxArrayCallArity            = 10;

/** Staistiac data for different game elements.
 * use this to set you config settings appropriate to your scenario or just to get some insight.
 */
namespace statistics
{
	/** Statistic data for an container data type. */
	struct container {
		int capacity; /**< current capacity of the container. This does typicall only inceares over the
		                 runtime. */
		int size;     /**< current size aka activly used elements inside the container. */
	};

	/** Statistics for managed lists, including static and dynamic enties. */
	struct list_table {
		container editable_lists; /**< based on @ref ink::config::limitEditableLists */
		container list_types;     /**< based on @ref ink::config::maxListTypes */
		container flags;          /**< based on @ref ink::config::maxFlags */
		container lists;          /**< based on @ref ink::config::maxLists */
	};

	/** Statistiacs to managed strings, which are build at runtime. */
	struct string_table {
		container string_refs; /**< based on @ref limitStringTable */
	};

	/** Stastics for state managed for one runtime. */
	struct global {
		container  variables;           /**< based on @ref ink::config::limitGlobalVariables */
		container  variables_observers; /**< based on @ref ink::config::limitGlobalVariableObservers */
		list_table lists;               /**< Staistics for all lists associated with this runtime. */
		string_table strings;           /**< Staistics for all strings associtade with this runtime. */
	};

	/** Stastics for state managed for one thread inside a runtime. */
	struct runner {
		container threads;           /**< based on @ref ink::config::limitThreadDepth */
		container evaluation_stack;  /**< based on @ref ink::config::limitEvalStackDepth */
		container container_stack;   /**< based on @ref ink::config::limitContainerDepth */
		container active_tags;       /**< based on @ref ink::config::limitActiveTags */
		container runtime_stack;     /**< based on @ref ink::config::limitContainerDepth  */
		container runtime_ref_stack; /**< based on @ref ink::config::limitReferenceStack */
		container output;            /**< based on @ref ink::config::limitOutputSize */
		container choices;           /**< based on @ref ink::config::limitContainerDepth */
	};
} // namespace statistics
} // namespace ink::config

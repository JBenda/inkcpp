#pragma once

#ifdef INKCPP_API
#	define INK_ENABLE_UNREAL
#else
#	define INK_ENABLE_STL
#	define INK_ENABLE_CSTD
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
 * @attention list vars are only valid until the story continous!
 */
static constexpr int limitEditableLists           = -5;
/// number of simultaneous active tags
static constexpr int limitActiveTags              = -10;
// temporary variables and callstack;
static constexpr int limitRuntimeStack            = -20;
// references  and callstack
static constexpr int limitReferenceStack          = -20;
// max number of elements in one output (a string is one element)
// no dynamic support now! (FIXME)
static constexpr int limitOutputSize              = 200;
// max number of choices per choice
static constexpr int maxChoices                   = -10;
// max number of list types, and there total amount of flags
static constexpr int maxListTypes                 = -20;
static constexpr int maxFlags                     = -200;
// number of max initelized lists
static constexpr int maxLists                     = -50;
static constexpr int maxArrayCallArity            = 10;
} // namespace ink::config

/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "config.h"
#include "system.h"
#include "functional.h"
#include "types.h"

#ifdef INK_ENABLE_UNREAL
#	include "Containers/UnrealString.h"
#endif

namespace ink::runtime
{
class choice;

/**
 * A runner to execute ink script from a story.
 *
 * An independent runner which can execute ink script from a
 * story independent of other runners. Think of the ink story
 * object like an executable file and the runners as 'threads'
 * (not to be confused with ink threads, which are a language
 * feature). Runners track their own instruction pointer, choice
 * list, temporary variables, callstack, etc. They can either
 * be started with their own globals store, or can share
 * one with other runner instances.
 * @see globals
 * @see story
 */
class runner_interface
{
public:
	virtual ~runner_interface(){};

	// String type to simplify interfaces working with strings
#ifdef INK_ENABLE_STL
	using line_type = std::string;
#elif defined(INK_ENABLE_UNREAL)
	using line_type = FString;
#endif

#pragma region Interface Methods
	/**
	 * Sets seed for PRNG used in runner.
	 * Else runner is started with the current time as seed.
	 * @param seed seed to use for PRNG
	 */
	virtual void set_rng_seed(uint32_t seed) = 0;

	/**
	 * Moves the runner to the specified path
	 *
	 * Clears any execution context and moves the runner
	 *  to the content at the specified path.
	 *
	 * @param path path to search and move execution to
	 * @return If the path was found
	 */
	virtual bool move_to(hash_t path) = 0;

	/**
	 * Can the runner continue?
	 *
	 * Checks if the runner can continue execution. If it
	 * can't, we are either at a choice or are out of content.
	 * @see continue
	 * @see has_choices
	 *
	 * @return Can continue be called
	 */
	virtual bool can_continue() const = 0;

	/**
	 * @brief creates a snapshot containing the runner, globals and all other runners connected to the
	 * globals.
	 * @sa story::new_runner_from_snapshot, story::new_globals_from_snapshot
	 */
	virtual snapshot* create_snapshot() const = 0;

#ifdef INK_ENABLE_CSTD
	/**
	 * Continue execution until the next newline, then allocate a c-style
	 * string with the output. This allocated string is managed by the runtime
	 * and will be deleted at the next @ref ink::runtime::runner_interface::choose() "choose()" or
	 * @ref ink::runtime::runner_interface::getline() "getline()"
	 *
	 * @return allocated c-style string with the output of a single line of execution
	 */
	virtual const char* getline_alloc() = 0;
#endif

#if defined(INK_ENABLE_STL) || defined(INK_ENABLE_UNREAL)
	/**
	 * Execute the next line of the script.
	 *
	 * Continue execution until the next newline, then returns the output as an STL C++
	 *`std::string` or Unreal's `FString`.
	 *
	 * @return string with the next line of output
	 */
	virtual line_type getline() = 0;

	/**
	 * Execute the script until the next choice or the end of the script.
	 *
	 * Continue execution until the next choice or the story ends, then returns the output as an STL
	 * C++ `std::string` or Unreal's `FString`.
	 *
	 * @return string with the next line of output
	 */
	virtual line_type getall() = 0;
#endif

#ifdef INK_ENABLE_STL
	/**
	 * Gets the next line of output using C++ STL string.
	 *
	 * Continue execution until the next newline, then return the output to
	 * an STL C++ std::ostream. Requires INK_ENABLE_STL
	 */
	virtual void getline(std::ostream&) = 0;

	/**
	 * Gets all the text until the next choice or end
	 *
	 * Continue execution until the next choice or the story ends,
	 * then return the output to an STL C++ std::ostream.
	 * Requires INK_ENABLE_STL
	 */
	virtual void getall(std::ostream&) = 0;
#endif

	/**
	 * Choice iterator.
	 *
	 * Iterates over choices the runner is currently facing.
	 *
	 * @see end
	 * @return constant iterator to the first choice
	 */
	virtual const choice* begin() const = 0;

	/**
	 * Terminal choice iterator.
	 *
	 * Pointer past the last choice the runner is currently facing.
	 *
	 * @see begin
	 * @return end iterator
	 */
	virtual const choice* end() const = 0;

	/**
	 * Make a choice.
	 *
	 * Takes the choice at the given index and moves the instruction
	 * pointer to that branch.
	 *
	 * @param index index of the choice to make
	 */
	virtual void choose(size_t index) = 0;

	/**
	 * Check if the current line has any tags. Excludes global tags.
	 *
	 * @return true if the line has tags, false if not
	 *
	 * @sa num_tags get_tag has_global_tags has_knot_tags
	 */
	virtual bool has_tags() const = 0;

	/**
	 * Returns the number of tags on the current line. Excludes global tags.
	 *
	 * @sa get_tag has_tags num_global_tags num_knot_tags
	 */
	virtual size_t num_tags() const = 0;

	/**
	 * Access a tag by index.
	 *
	 * Note that this method only allows the retrieval of tags attached to the current line. For
	 * tags at the top of the script, use get_global_tag()
	 *
	 * @param index tag id to fetch [0;@ref ink::runtime::runner_interface::num_tags() "num_tags()")
	 *
	 * @return pointer to the tag string memory or nullptr if the index is invalid
	 *
	 * @sa has_tags num_tags get_global_tag get_knot_tag
	 */
	virtual const char* get_tag(size_t index) const = 0;

	/**
	 * Check if the there are global tags.
	 *
	 * @return ture if there are global tags.
	 * @info global tags  are also assoziated to the first line in the knot/stitch
	 * @sa num_global_tags get_global_tags  has_tags has_knot_tags
	 */
	virtual bool has_global_tags() const = 0;

	/**
	 * Get Number of global tags.
	 * @info global tags  are also assoziated to the first line in the knot/stitch
	 * @sa has_global_tags get_global_tags num_knot_tags num_tags
	 * @return the number of tags at the top of the document.
	 */
	virtual size_t num_global_tags() const = 0;

	/**
	 * Access a global tag by index.
	 *
	 * Global tags are placed at the top of the script instead of above or next to the current line.
	 * For tags related to the current line, use @sa get_tag
	 *
	 * @param index tag id to fetch [0;@ref ink::runtime::runner_interface::num_global_tags()
	 * "num_global_tags()")
	 *
	 * @return pointer to the tag string memory or nullptr if the index is invalid
	 * @sa has_global_tags num_global_tags get_tag get_knot_tag
	 */
	virtual const char* get_global_tag(size_t index) const = 0;

	/**
	 * Check if there are knot/stitch tags.
	 *
	 * @info knot/stitch tags  are also assoziated to the first line in the knot/stitch
	 * @return true if there are knot/stitch tags.
	 * @sa num_knot_tags get_knot_tag has_global_tags has_tags
	 */
	virtual bool has_knot_tags() const = 0;

	/**
	 * Get Number of knot/stitch tags.
	 * @info knot/stitch tags are also assoziated to the first line in the knot/stitch
	 * @return number of tags at the top of a knot/stitch
	 * @sa has_knot_tags get_knot_tag num_global_tags num_tags
	 */
	virtual size_t num_knot_tags() const = 0;

	/**
	 * Access a knot/stitch tag by index.
	 *
	 * Knot stitch tags are placed at the top of a knot/stitch.
	 * @param index tag id to fetch [0;@ref ink::runtime::runner_interface::num_knot_tags()
	 * "num_knot_tags()")
	 * @return pointor to tag string memory or nullptr if the index is invalid
	 * @sa has_knot_tag num_knot_tags get_global_tag get_tag
	 */
	virtual const char* get_knot_tag(size_t index) const = 0;

	/**
	 * Get hash of current knot/stitch name.
	 * @return hash of current knot/stitch name
	 * @sa ink::hash_string()
	 */
	virtual hash_t get_current_knot() const = 0;

	/** Get usage statistics for the runner. */
	virtual config::statistics::runner statistics() const = 0;

protected:
	/** internal bind implementation. not for calling.
	 * @private */
	virtual void internal_bind(hash_t name, internal::function_base* function) = 0;

public:
	/**
	 * Binds an external callable to the runtime
	 *
	 * Given a name and a callable object, register this function
	 *  to be called back from the ink runtime.
	 *
	 * beside an exact signature match, the function can also have one of the following signatures:
	 * + void(size_t argl, const ink::runtime::value* argv)
	 * + ink::runtime::value(size_t argl, const ink::runtime::value* argv)
	 * this provides a generic way to bind functions with abitrary length
	 * @param name name hash
	 * @param function callable
	 * @param lookaheadSafe if false stop glue lookahead if encounter this function
	 *                      this prevents double execution of external functions but can lead to
	 *                      missing glues
	 */
	template<typename F>
	inline void bind(hash_t name, F function, bool lookaheadSafe = false)
	{
		internal_bind(name, new internal::function(function, lookaheadSafe));
	}

	/**
	 * Binds an external callable to the runtime
	 *
	 * Given a name and a callable object, register this function
	 *  to be called back from the ink runtime.
	 *
	 * @param name name string
	 * @param function callable
	 * @param lookaheadSafe if false stop glue lookahead if encounter this function
	 *                      this prevents double execution of external functions but can lead to
	 *                      missing glues
	 */
	template<typename F>
	inline void bind(const char* name, F function, bool lookaheadSafe = false)
	{
		bind(ink::hash_string(name), function, lookaheadSafe);
	}

#ifdef INK_ENABLE_UNREAL
	/** bind and unreal delegate
	 * @param name hash of external function name in ink script
	 * @param functionDelegate
	 * @param lookaheadSafe @ref #bind()
	 */
	template<typename D>
	void bind_delegate(hash_t name, D functionDelegate, bool lookaheadSafe)
	{
		internal_bind(name, new internal::function_array_delegate(functionDelegate, lookaheadSafe));
	}
#endif

#pragma endregion

#pragma region Convenience Methods

	/**
	 * Shortcut for checking if the runner can continue.
	 *
	 * @see can_continue
	 */
	inline operator bool() const { return can_continue(); }

	/**
	 * Checks if we're currently facing any choices
	 *
	 * @return are there any choices available
	 */
	inline bool has_choices() const { return begin() != end(); }

	/**
	 * Returns the number of choices currently available
	 *
	 * @return number of choices
	 */
	size_t num_choices() const;

	/**
	 * Gets a choice
	 *
	 * Returns the choice object at a given index
	 *
	 * @see num_choices
	 * @param index index of the choice to access
	 * @return choice object with info on the choice
	 */
	const choice* get_choice(size_t index) const;

	/**
	 * Shorcut for accessing a choice
	 *
	 * @see get_choice
	 * @param index index of the choice to access
	 * @return choice object with info on the choice
	 */
	inline const choice* operator[](size_t index) { return get_choice(index); }

#pragma endregion
};
} // namespace ink::runtime

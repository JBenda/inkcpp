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
 * An independant runner which can execute ink script from a
 * story independant of other runners. Think of the ink story
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

#ifdef INK_ENABLE_CSTD
	/**
	 * Continue execution until the next newline, then allocate a c-style
	 * string with the output. This allocated string is now the callers
	 * responsibility and it should be deleted.
	 *
	 * @return allocated c-style string with the output of a single line of execution
	 */
	virtual char* getline_alloc() = 0;
#endif

	/**
	 * @brief creates a snapshot containing the runner, globals and all other runners connected to the
	 * globals.
	 * @sa story::new_runner_from_snapshot, story::new_globals_from_snapshot
	 */
	virtual snapshot* create_snapshot() const = 0;

#ifdef INK_ENABLE_STL
	/**
	 * Gets the next line of output using C++ STL string.
	 *
	 * Continue execution until the next newline, then return the output as
	 * an STL C++ std::string. Requires INK_ENABLE_STL
	 *
	 * @return std::string with the next line of output
	 */
	virtual std::string getline() = 0;

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
	 * then return the output as an STL C++ std::string.
	 * Requires INK_ENABLE_STL
	 *
	 * @return std::string with the next line of output
	 */
	virtual std::string getall() = 0;

	/**
	 * Gets all the text until the next choice or end
	 *
	 * Continue execution until the next choice or the story ends,
	 * then return the output to an STL C++ std::ostream.
	 * Requires INK_ENABLE_STL
	 */
	virtual void getall(std::ostream&) = 0;
#endif

#ifdef INK_ENABLE_UNREAL
	/**
	 * Gets the next line of output using unreal string allocation
	 *
	 * Continue execution until the next newline, then return the output as
	 * an Unreal FString. Requires INK_ENABLE_UNREAL
	 *
	 * @return FString with the next line of output
	 */
	virtual FString getline() = 0;
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

	/** check if since last choice selection tags have been added */
	virtual bool        has_tags() const            = 0;
	/** return the number of tags accumulated since last choice
	 * order of tags wont change, and new are added at the end */
	virtual size_t      num_tags() const            = 0;
	virtual const char* get_tag(size_t index) const = 0;

protected:
	// internal bind implementation. not for calling.
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
	 */
	template<typename F>
	inline void bind(hash_t name, F function)
	{
		internal_bind(name, new internal::function(function));
	}

	/**
	 * Binds an external callable to the runtime
	 *
	 * Given a name and a callable object, register this function
	 *  to be called back from the ink runtime.
	 *
	 * @param name name string
	 * @param function callable
	 */
	template<typename F>
	inline void bind(const char* name, F function)
	{
		bind(ink::hash_string(name), function);
	}

#ifdef INK_ENABLE_UNREAL
	template<typename D>
	void bind_delegate(hash_t name, D functionDelegate)
	{
		internal_bind(name, new internal::function_array_delegate(functionDelegate));
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

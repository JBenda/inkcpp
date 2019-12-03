#pragma once

#include "types.h"

namespace ink::runtime
{
	/**
	 * A loaded ink story.
	 *
	 * Created by loading a binary ink story into memory. Once loaded,
	 * the story class can create "runners" which execute story code.
	 * A story can have any number of runners, which can optionally
	 * share globals (variables, visit counts, etc). through the
	 * globals object. By default, each runner gets its own newly
	 * created globals store.
	 * @see runner_interface
	 * @see globals
	*/
	class story
	{
	public:
#pragma region Interface Methods
		/**
		 * Creates a new global store
		 *
		 * Creates a new global store that can be passed in when
		 * creating new runners for this story. Note: Can not be
		 * used for other stories. It is tied to this story.
		 *
		 * @return managed pointer to a new global store
		*/
		virtual globals new_globals() = 0;

		/**
		 * Creates a new runner
		 *
		 * Creates a new runner whose initial instruction pointer
		 * is the first instruction in this story. If no global
		 * store is passed, a new one will be created for the runner.
		 *
		 * @return managed pointer to a new runner
		*/
		virtual runner new_runner(globals store = nullptr) = 0;
#pragma endregion

#pragma region Factory Methods
		/**
		 * Creates a new story object from a file.
		 *
		 * Requires STL or other extension which allows files
		 * to be loaded and read. Will allocate all the data
		 * necessary to load the file and close it.
		 *
		 * @param filename filename of the binary ink data
		 * @return new story object
		*/
		static story* from_file(const char* filename);

		/**
		 * Create a new story object from binary buffer
		 *
		 * No extensions required. Creates the story from binary
		 * data already loaded into memory. By default, the story
		 * will free this buffer when it is destroyed.
		 *
		 * @param data binary data
		 * @param length of the binary data in bytes
		 * @param freeOnDestroy if true, free this buffer once the story is destroyed
		 * @return new story object
		*/
		static story* from_binary(unsigned char* data, size_t length, bool freeOnDestroy = true);
#pragma endregion
	};
}
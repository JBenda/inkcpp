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
		class input {
		public:
			/**
			 * @brief Create an input object from a file.
			 *
			 * Requires STL or other extension which allows files
			 * to be loaded and read. Will allocate all the data
			 * necessary to load the file and close it.
			 * @param filename of file to load data from
			 */
			input(const char*const filename);

			/**
			 * @brief Create input object from memory-segment
			 * @param data pointer to begin of segment
			 * @param length of the segment
			 * @param freeOnDestroy wither or not free memory when no longer needed.
			 *        on story destruction or when destroy input before usage.
			 */
			input(unsigned char* data, size_t length, bool freeOnDestroy = false)
				: _data{data}, _length{length}, _freeOnDestroy{freeOnDestroy}{}

			unsigned char* data() const { return _data; }
			size_t length() const { return _length; }
			/// @brief claim ownership for memory segment in input.
			/// @return false when memory segment is not owned by input
			/// @return true otherwise
			bool get_ctrl() {
				bool res = _freeOnDestroy;
				_freeOnDestroy = false;
				return _freeOnDestroy;
			}
			~input() {
				if (_data && _freeOnDestroy) {
					free(_data);
					_data = nullptr;
				}
			}
			input(input&& o)
				: _data{o._data},
				_length{o._length},
				_freeOnDestroy{o.get_ctrl()}
			{}

		private:
			input(const input&) = delete;
			input& operator=(const input&) = delete;
			unsigned char* _data;
			size_t _length;
			bool _freeOnDestroy;
		};
		/**
		 * Create a new story object from two inputs
		 * @param inkbin input containing InkBin file data
		 * @param text input containing strings/translated strings
		*/
		static story* create(input&& inkbin, input&& text);
#pragma endregion
	};
}

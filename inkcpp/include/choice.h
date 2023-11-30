#pragma once
#include "system.h"

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			class basic_stream;
			class runner_impl;
			class string_table;
			class list_table;
		}

		/**
		 * An ink choice that is being presented to the user
		 *
		 * Contains all the data about a single choice being faced
		 * by an ink runner. Of primary concern is the index and the
		 * text.
		 *
		 * @see runner
		*/
		class choice
		{
		public:
			/**
			 * Choice index
			 *
			 * Pass this to the runner to choose this choice and
			 * have it follow its branch.
			 *
			 * @returns index of the choice. 0 is the first, etc.
			*/
			int index() const { return _index; }

			/**
			 * Choice text
			 *
			 * Text to display to the user for choosing this choice.
			 *
			 * @returns choice text as a string
			 */
			const char* text() const { return _text; }

			choice() : choice(0) {}
			choice(int) : _tags{nullptr}, _text{nullptr}, _index{~0}, _path{~0u}, _thread{~0u} {}

			bool has_tags() const { return _tags != nullptr; }
			size_t num_tags() const
			{
				size_t i = 0;
				if (has_tags()) while ( _tags[i] != nullptr )
				{
					++i;
				};
				return i;
			}
			const char* get_tag(size_t index) const {
				return _tags[index];
			}

		private:
			friend class internal::runner_impl;

			uint32_t path() const { return _path; }
			choice&  setup( internal::basic_stream&, internal::string_table& strings, internal::list_table& lists, int index, uint32_t path, thread_t thread, const char* const* tags );

		protected:
			const char* const* _tags;
			const char* _text;
			int _index;
			uint32_t _path;
			thread_t _thread;
		};
	}
}

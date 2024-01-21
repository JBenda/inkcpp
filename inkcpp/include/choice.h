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
	} // namespace internal

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

		/** @private */
		choice()
		    : choice(0)
		{
		}

		/** @private */
		choice(int)
		    : _tags{nullptr}
		    , _text{nullptr}
		    , _index{~0}
		    , _path{~0u}
		    , _thread{~0u}
		{
		}

		/** does this choice has tags? */
		bool has_tags() const { return _tags != nullptr; }

		/** number of tags assoziated with this choice
		 * @see @ref ink::runtime::choice::has_tags() "has_tags()"
		 */
		size_t num_tags() const
		{
			size_t i = 0;
			if (has_tags())
				while (_tags[i] != nullptr) {
					++i;
				};
			return i;
		}

		/** @copydoc ink::runtime::runner_interface::get_tag() */
		const char* get_tag(size_t index) const { return _tags[index]; }

	private:
		friend class internal::runner_impl;

		uint32_t path() const { return _path; }

		choice& setup(
		    internal::basic_stream&, internal::string_table& strings, internal::list_table& lists,
		    int index, uint32_t path, thread_t thread, const char* const* tags
		);

	protected:
		const char* const* _tags;   ///< @private
		const char*        _text;   ///< @private
		int                _index;  ///< @private
		uint32_t           _path;   ///< @private
		thread_t           _thread; ///< @private
	};
} // namespace runtime
} // namespace ink

/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
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
		class snap_tag;
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

		/** does this choice has tags? */
		bool has_tags() const { return _tags_start != _tags_end; }

		/** number of tags associated with this choice
		 * @see @ref ink::runtime::choice::has_tags() "has_tags()"
		 */
		size_t num_tags() const;

		/** @copydoc ink::runtime::runner_interface::get_tag() */
		const char* get_tag(size_t index) const;

	private:
		friend class internal::runner_impl;

		uint32_t path() const { return _path; }

		choice& setup(
		    internal::basic_stream&, internal::string_table& strings, internal::list_table& lists,
		    int index, uint32_t path, thread_t thread, const internal::snap_tag* tags_start,
		    const internal::snap_tag* tags_end
		);

	protected:
		const char*               _text       = nullptr; ///< @private
		const internal::snap_tag* _tags_start = nullptr; ///< @private
		const internal::snap_tag* _tags_end   = nullptr; ///< @private
		uint32_t                  _path       = ~0U;     ///< @private
		thread_t                  _thread     = ~0U;     ///< @private
		int                       _index      = -1;      ///< @private
	};

} // namespace runtime
} // namespace ink

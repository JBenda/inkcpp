#pragma once

#include "../snapshot_impl.h"

#include <system.h>
#include <traits.h>

namespace ink::runtime::internal
{
	struct entry;
	template<typename ElementType>
	constexpr auto EmptyNullPredicate = [](const ElementType&) { return false; };

	// Iterator type used with restorable
	template<typename ElementType>
	class restorable_iter
	{
	public:
		// Create an iterator moving from start (inclusive) to end (exclusive)
		restorable_iter(ElementType* start, ElementType* end)
			: _current(start), _end(end) { }

		// Move to the next non-null element
		template<typename IsNullPredicate = decltype(EmptyNullPredicate<ElementType>)>
		bool next(IsNullPredicate isNull = EmptyNullPredicate<ElementType>)
		{
			if (_current != _end)
			{
				// Determine direction of iteration
				int dir = _end - _current > 0 ? 1 : -1;

				// Move pointer
				_current += dir;

				// Make sure to skip over null items
				while (isNull(*_current) && _current != _end) {
					_current += dir;
				}
			}

			// If we've hit the end, return false
			if (_current == _end)
				return false;

			// Otherwise, iteration is valid
			return true;
		}

		// Get current element
		inline ElementType* get() { return _current; }

		// Get current element (const)
		inline const ElementType* get() const { return _current;  }

		// Is iteration complete (opposite of is valid)
		inline bool done() const { return _current == _end; }

	private:
		// Current point of iteration
		ElementType* _current;

		// End point (non-valid)
		ElementType* _end;
	};

	/**
	 * A special base class for collections which have save/restore/forget functionality
	 * 
	 * In order to properly handle Ink's glue system, we need to be able to execute beyond
	 * the end of the line and "peek" to see if a glue command is executed. If one is, we keep
	 * executing (as the next line will be glued to the current). If we don't, or find more content
	 * before finding glue, then in actuality, we never should have executed beyond the newline. We
	 * need to *restore* back to our state before we moved past the end of line. Collections inheriting
	 * from this class gain this functionality.
	 */
	template<typename ElementType>
	class restorable : public snapshot_interface
	{
	public:
		restorable(ElementType* buffer, size_t size)
			: _buffer(buffer), _size(size), _pos(0), _save(~0), _jump(~0)
		{ }

		// Checks if we have a save state
		bool is_saved() const { return _save != ~0; }

		// Creates a save point which can later be restored to or forgotten
		void save()
		{
			inkAssert(_save == ~0, "Collection is already saved. You should never call save twice. Ignoring.");
			if (_save != ~0) {
				return;
			}

			// Set the save and jump points to the current position.
			_save = _jump = _pos;
		}

		// Restore to the last save point
		void restore()
		{
			inkAssert(_save != ~0, "Collection can't be restored because it's not saved. Ignoring.");
			if (_save == ~0) {
				return;
			}

			// Move our position back to the saved position
			_pos = _save;

			// Clear save point
			_save = _jump = ~0;
		}

		// Forget the save point and continue with the current data
		template<typename NullifyMethod>
		void forget(NullifyMethod nullify)
		{
			inkAssert(_save != ~0, "Can't forget save point because there is none. Ignoring.");
			if (_save == ~0) {
				return;
			}

			// If we're behind the save point but past the jump point
			if (_save != _jump && _pos > _jump)
			{
				// Nullify everything between the jump point and the save point
				for (size_t i = _jump; i < _save; ++i)
					nullify(_buffer[i]);
			}

			// Reset save position
			_save = _jump = ~0;
		}

		using iterator = restorable_iter<ElementType>;
		using const_iterator = restorable_iter<const ElementType>;

		// Iterator that begins at the end of the stack
		iterator begin() { return iterator(&_buffer[_pos - 1], _buffer - 1); }
		const_iterator begin() const { return iterator(&_buffer[_pos - 1], _buffer - 1); }

		// Iterator that points to the element past the beginning of the stack
		iterator end() { return iterator(_buffer - 1, _buffer - 1); }
		iterator end() const { return const_iterator(_buffer - 1, _buffer - 1); }

		// Push element onto the top of collection
		ElementType& push(const ElementType& elem)
		{
			// Don't destroy saved data. Jump over it
			if (_pos < _save && _save != ~0)
			{
				_jump = _pos;
				_pos = _save;
			}

			// Overflow check
			if (_pos >= _size)
				overflow(_buffer, _size);

			// Push onto the top
			_buffer[_pos++] = elem;

			// Return reference
			return _buffer[_pos - 1];
		}

		// Pop an element off the top of the collection
		template<typename IsNullPredicate>
		const ElementType& pop(IsNullPredicate isNull)
		{
			// Make sure we have something to pop
			inkAssert(_pos > 0, "Can not pop. No elements to pop!");

			// Jump over save data
			if (_pos == _save)
				_pos = _jump;

			// Move over empty data
			while (isNull(_buffer[_pos - 1]))
				_pos--;

			// Decrement and return
			_pos--;
			return _buffer[_pos];
		}

		template<typename IsNullPredicate>
		const ElementType& top(IsNullPredicate isNull) const
		{
			inkAssert(_pos > 0, "Can not top. No elememnts to show!");
			auto pos = _pos;
			if (_pos == _save)
				pos = _jump;
			while(isNull(_buffer[pos-1]))
				--pos;
			return _buffer[pos-1];
		}

		bool is_empty() const { return _pos == 0; }

		void clear()
		{
			_pos = 0;
			_save = _jump = ~0;
		}

		// Forward iterate
		template<typename CallbackMethod, typename IsNullPredicate>
		void for_each(CallbackMethod callback, IsNullPredicate isNull) const
		{
			if (_pos == 0) {
				return;
			}

			// Start at the beginning
			size_t i = 0;
			do
			{
				// Jump over saved data
				if (i == _jump)
					i = _save;

				// Run callback
				if(!isNull(_buffer[i]))
					callback(_buffer[i]);

				// Move forward one element
				i++;
			} while (i < _pos);
		}

		template<typename Predicate>
		const ElementType* find(Predicate predicate) const
		{
			if (_pos == 0) {
				return nullptr;
			}

			// Start at the beginning
			size_t i = 0;
			do
			{
				// Jump over saved data
				if (i == _jump)
					i = _save;

				// Run callback
				if (!isNull(_buffer[i]) && predicate(_buffer[i]))
					return &_buffer[i];

				// Move forward one element
				i++;
			} while (i < _pos);

			return nullptr;
		}

		template<typename CallbackMethod>
		void for_each_all(CallbackMethod callback) const
		{
			// no matter if we're saved or not, we iterate everything
			int len = (_save == ~0 || _pos > _save) ? _pos : _save;

			// Iterate
			for (int i = 0; i < len; i++)
				callback(_buffer[i]);
		}

		// Reverse iterate
		template<typename CallbackMethod, typename IsNullPredicate>
		void reverse_for_each(CallbackMethod callback, IsNullPredicate isNull) const
		{
			if (_pos == 0) {
				return;
			}

			// Start at the end
			size_t i = _pos;
			do
			{
				// Move back one element
				i--;

				// Run callback
				if (!isNull(_buffer[i]))
					callback(_buffer[i]);

				// Jump over saved data
				if (i == _save)
					i = _jump;

			} while (i > 0);
		}

		// Reverse find
		template<typename Predicate>
		ElementType* reverse_find(Predicate predicate) {
			return reverse_find_impl(predicate);
		}

		template<typename Predicate>
		const ElementType* reverse_find(Predicate predicate) const {
			return reverse_find_impl(predicate);
		}

		template<typename IsNullPredicate>
		size_t size(IsNullPredicate isNull) const
		{
			if (_pos == 0) {
				return 0;
			}

			size_t count = 0;

			// Start at the end
			size_t i = _pos;
			do
			{
				// Move back one element
				i--;

				// Run callback
				if(!isNull(_buffer[i]))
					count++;

				// Jump over saved data
				if (i == _save)
					i = _jump;

			} while (i > 0);

			return count;
		}

		// snapshot interface
		virtual size_t snap(unsigned char* data, const snapper&) const override;
		const unsigned char* snap_load(const unsigned char* data, const loader&) override;

	protected:
		// Called when we run out of space in buffer. 
		virtual void overflow(ElementType*& buffer, size_t& size) {
			throw ink_exception("Restorable run out of memory!");
		}

	private:

		template<typename Predicate>
		ElementType* reverse_find_impl(Predicate predicate) const
		{
			if (_pos == 0) {
				return nullptr;
			}

			// Start at the end
			size_t i = _pos;
			do
			{
				// Move back one element
				i--;

				// Run callback
				if (predicate(_buffer[i]))
					return &_buffer[i];

				// Jump over saved data
				if (i == _save)
					i = _jump;

			} while (i > 0);

			return nullptr;
		}

		// Data buffer. Collection is stored here
		ElementType* _buffer;

		// Size of the _buffer array
		size_t _size;

		// Set to the next empty position in the buffer.
		size_t _pos;

		// Jump and save points. Used when we've been saved.
		size_t _jump;
		size_t _save;
	};
	template<>
	size_t restorable<value>::snap(unsigned char* data, const snapper& snapper) const;
	template<>
	size_t restorable<entry>::snap(unsigned char* data, const snapper& snapper) const;
	template<>
	size_t restorable<int>::snap(unsigned char* data, const snapper&) const;

	template<>
	const unsigned char* restorable<value>::snap_load(const unsigned char* data, const loader&);
	template<>
	const unsigned char* restorable<entry>::snap_load(const unsigned char* data, const loader&);
	template<>
	const unsigned char* restorable<int>::snap_load(const unsigned char* data, const loader&);
}

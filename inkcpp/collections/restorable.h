#include <system.h>

namespace ink::runtime::internal
{
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
	class restorable
	{
	public:
		restorable(ElementType* buffer, size_t size)
			: _buffer(buffer), _size(size), _pos(0), _save(~0), _jump(~0)
		{ }

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

		// Push element onto the top of collection
		void push(const ElementType& elem)
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
		}

		// Pop an element off the top of the collection
		template<typename IsNullPredicate>
		const ElementType& pop(IsNullPredicate isNull)
		{
			// Make sure we have something to pop
			inkAssert(_pos > 0, "Can not pop. No elements to pop!");
			if (_pos <= 0) {
				throw 0; // TODO
			}

			// Jump over save data
			if (_pos == _save)
				_pos = _jump;

			// Move over empty data
			while (isNull(_buffer[_pos]))
				_pos--;

			// Decrement and return
			_pos--;
			return _buffer[_pos];
		}

		const ElementType& top() const
		{
			if (_pos == _save)
				return _buffer[_jump - 1];

			return _buffer[_pos - 1];
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

	protected:
		// Called when we run out of space in buffer. 
		virtual void overflow(ElementType*& buffer, size_t& size) { throw 0; /* TODO: What to do here? Throw something more useful? */ }

	private:
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
}
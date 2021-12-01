#pragma once

#include "system.h"
#include "array.h"

namespace ink::runtime::internal
{
	template<typename T>
	class simple_restorable_stack
	{
	public:
		simple_restorable_stack(T* buffer, size_t size, const T& null)
			: _buffer(buffer), _size(size), _null(null) { }

		void push(const T& value);
		T pop();
		const T& top() const;

		size_t size() const;
		bool empty() const;
		void clear();

		bool iter(const T*& iterator) const;

		// == Save/Restore ==
		void save();
		void restore();
		void forget();

	protected:
		virtual void overflow(T*& buffer, size_t& size) {
			throw ink_exception("Stack overflow!");
		}

		void initialize_data(T* buffer, size_t size) {
			inkAssert(_buffer == nullptr && _size == 0, "Try to double initialize a restorable stack."
					"To extend the size use overflow()");
			_buffer = buffer;
			_size = size;
		}
	private:
		T* _buffer;
		size_t _size;
		const T _null;

		const static size_t InvalidIndex = ~0;

		size_t _pos = 0;
		size_t _save = InvalidIndex, _jump = InvalidIndex;
	};

	template<typename T, bool dynamic, size_t N>
	class managed_restorable_stack : public simple_restorable_stack<T>
	{
		using base = simple_restorable_stack<T>;
	public:
		template<bool ... D, bool con = dynamic, enable_if_t<con, bool> = true>
		managed_restorable_stack(const T& null) : simple_restorable_stack<T>(nullptr, 0, null) { }
		template<bool ... D, bool con = dynamic, enable_if_t<!con, bool> = true>
		managed_restorable_stack(const T& null) :
			simple_restorable_stack<T>(nullptr, 0, null), _stack{}
		{ base::initialize_data(_stack.data(), N); }
		virtual void overflow(T*& buffer, size_t& size) override final {
			if constexpr (dynamic) {
				if (buffer) {
					_stack.extend();
				}
				buffer = _stack.data();
				size = _stack.capacity();
			} else {
				base::overflow(buffer, size);
			}
		}
	private:
		managed_array<T, dynamic, N> _stack;
	};


	template<typename T>
	inline void simple_restorable_stack<T>::push(const T& value)
	{
		inkAssert(value != _null, "Can not push a 'null' value onto the stack.");

		// Don't overwrite saved data. Jump over it and record where we jumped from
		if (_save != InvalidIndex && _pos < _save)
		{
			_jump = _pos;
			_pos = _save;
		}

		if (_pos >= _size) {
			overflow(_buffer, _size);
		}

		// Push onto the top of the stack
		_buffer[_pos++] = value;
	}

	template<typename T>
	inline T simple_restorable_stack<T>::pop()
	{
		inkAssert(_pos > 0, "Nothing left to pop!");

		// Move over jump area
		if (_pos == _save) {
			_pos = _jump;
		}

		// Decrement and return
		return _buffer[--_pos];
	}

	template<typename T>
	inline const T& simple_restorable_stack<T>::top() const
	{
		if (_pos == _save)
		{
			inkAssert(_jump > 0, "Stack is empty! No top()");
			return _buffer[_jump - 1];
		}

		inkAssert(_pos > 0, "Stack is empty! No top()");
		return _buffer[_pos - 1];
	}

	template<typename T>
	inline size_t simple_restorable_stack<T>::size() const
	{
		// If we're past the save point, ignore anything in the jump region
		if (_pos >= _save)
			return _pos - (_save - _jump);

		// Otherwise, pos == size
		return _pos;
	}

	template<typename T>
	inline bool simple_restorable_stack<T>::empty() const
	{
		return size() == 0;
	}

	template<typename T>
	inline void simple_restorable_stack<T>::clear()
	{
		// Reset to start
		// TODO: Support save!
		_save = _jump = InvalidIndex;
		_pos = 0;
	}

	template<typename T>
	inline bool simple_restorable_stack<T>::iter(const T*& iterator) const
	{
		// If empty, nothing to iterate
		if (_pos == 0)
			return false;

		// Begin at the top of the stack
		if (iterator == nullptr || iterator < _buffer || iterator > _buffer + _pos)
		{
			iterator = _buffer + _pos - 1;
			return true;
		}

		// Move over stored data
		if (iterator == _buffer + _save)
			iterator = _buffer + _jump;

		// Run backwards
		iterator--;

		// Skip nulls
		while (*iterator == _null)
			iterator--;

		// End
		if (iterator < _buffer)
		{
			iterator = nullptr;
			return false;
		}

		return true;
	}

	template<typename T>
	inline void simple_restorable_stack<T>::save()
	{
		inkAssert(_save == InvalidIndex, "Can not save stack twice! restore() or forget() first");

		// Save current stack position
		_save = _jump = _pos;
	}

	template<typename T>
	inline void simple_restorable_stack<T>::restore()
	{
		inkAssert(_save != InvalidIndex, "Can not restore() when there is no save!");

		// Move position back to saved position
		_pos = _save;
		_save = _jump = InvalidIndex;
	}

	template<typename T>
	inline void simple_restorable_stack<T>::forget()
	{
		inkAssert(_save != InvalidIndex, "Can not forget when the stack has never been saved!");

		/*// If we have moven to a point earlier than the save point but we have a jump point
		if (_pos < _save && _pos > _jump)
		{*/
			// If we're at the save point, move us instead
			if (_pos == _save)
				_pos = _jump;
			// Everything between the jump point and the save point needs to be nullified
			else for (size_t i = _jump; i < _save; i++)
				_buffer[i] = _null;
		/*}*/

		// Just reset save position
		_save = InvalidIndex;
	}
}

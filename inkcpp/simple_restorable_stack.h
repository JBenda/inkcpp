#pragma once

#include "system.h"
#include "array.h"
#include "snapshot_impl.h"

namespace ink::runtime::internal
{
	/// only use this type for simple objects with simple copy operator and no heap references
	/// because they will may be serialized, stored and loaded in a different instance
	template<typename T>
	class simple_restorable_stack : public snapshot_interface
	{
	public:
		simple_restorable_stack(T* buffer, size_t size, const T& null)
			: _buffer(buffer), _size(size), _null(null) { }
		virtual ~simple_restorable_stack() = default;
			

		void push(const T& value);
		T pop();
		const T& top() const;

		size_t size() const;
		bool empty() const;
		void clear();

		bool iter(const T*& iterator) const;
		bool rev_iter(const T*& iterator) const;

		// == Save/Restore ==
		void save();
		void restore();
		void forget();

		virtual size_t snap(unsigned char* data, const snapper&) const;
		virtual const unsigned char* snap_load(const unsigned char* data, const loader&);

	protected:
		virtual void overflow(T*& buffer, size_t& size) {
			inkFail("Stack overflow!");
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
		virtual void overflow(T*& buffer, size_t& size) override {
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
			if (_pos == _save) {
				if(_jump == 0) {
					iterator = nullptr;
					return false;
				}
				iterator = _buffer + _jump -1;
			} else {
				iterator = _buffer + _pos - 1;
			}
			return true;
		}

		// Move over stored data
		if (iterator == _buffer + _save)
			iterator = _buffer + _jump;

		// Run backwards
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
	inline bool simple_restorable_stack<T>::rev_iter(const T*& iterator) const
	{
		if (_pos == 0)
			return false;
		if (iterator == nullptr || iterator < _buffer || iterator > _buffer + _pos) {
			if (_jump == 0) {
				if (_save == _pos) {
					iterator = nullptr;
					return false;
				}
				iterator = _buffer + _save;
			} else {
				iterator = _buffer;
			}
			return true;
		}
		++iterator;
		if (iterator == _buffer + _jump) {
			iterator = _buffer + _save;
		}

		if(iterator == _buffer + _pos) {
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

		inkAssert(_pos >= _save || _pos < _jump, "Pos is in backup areal! (should be impossible)");
		// if we are below the backup areal, no changes are needed
		// if we above the backup areal, we need to collpse it
		if (_pos >= _save) {
			size_t delta = _save - _jump;
			for(size_t i = _save; i < _pos; ++i) {
				_buffer[i - delta] = _buffer[i];
			}
			_pos -= delta;
		}

		// Just reset save position
		_save = _jump = InvalidIndex;
	}
	template<typename T>
	size_t simple_restorable_stack<T>::snap(unsigned char* data, const snapper&) const
	{
		unsigned char* ptr = data;
		bool should_write = data != nullptr;
		ptr = snap_write(ptr, _null, should_write);
		ptr = snap_write(ptr, _pos, should_write );
		ptr = snap_write(ptr, _save, should_write );
		ptr = snap_write(ptr, _jump, should_write );
		size_t max = _pos;
		if (_save > max) { max = _save; }
		if (_jump > max) { max = _jump; }
		for(size_t i = 0; i < max; ++i)
		{
			ptr = snap_write(ptr, _buffer[i], should_write );
		}
		return ptr - data;
	}

	template<typename T>
	const unsigned char* simple_restorable_stack<T>::snap_load(const unsigned char* ptr, const loader& loader)
	{
		T null;
		ptr = snap_read(ptr, null);
		inkAssert(null == _null, "different null value compared to snapshot!");
		ptr = snap_read(ptr, _pos);
		ptr = snap_read(ptr, _save);
		ptr = snap_read(ptr, _jump);
		size_t max = _pos;
		if(_save > max) { max = _save; }
		if(_jump > max) { max = _jump; }
		while(_size < max) { overflow(_buffer, _size); }
		for(size_t i = 0; i < max; ++i)
		{
			ptr = snap_read(ptr, _buffer[i]);
		}
		return ptr;
	}
}

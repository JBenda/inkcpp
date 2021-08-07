#pragma once

#include "system.h"

namespace ink::runtime::internal
{
	template<typename T, bool dynamic, size_t initialCapacity>
	class managed_array {
	public:
		managed_array() : _capacity{initialCapacity}, _size{0}{
			if constexpr (dynamic) {
				_dynamic_data = new T[initialCapacity];
			}
		}

		const T& operator[](size_t i) const { return data()[i]; }
		T& operator[](size_t i) { return data()[i]; }
		const T* data() const {
			if constexpr (dynamic) {
				return _dynamic_data;
			} else {
				return _static_data;
			}
		}
		T* data() {
			if constexpr (dynamic) {
				return _dynamic_data;
			} else {
				return _static_data;
			}
		}
		const T* begin() const { return data(); }
		T* begin() { return data(); }
		const T* end() const { return data() + _size; }
		T* end() { return data() + _size; }
		const T& back() const { return end()[-1]; }
		T& back() { return end()[-1]; }

		const size_t size() const { return _size; }
		const size_t capacity() const { return _capacity; }
		T& push() {
			if constexpr (dynamic) {
				if (_size == _capacity) { extend(); }
			} else {
				ink_assert(_size <= _capacity, "Stack Overflow!");
			}
			return data()[_size++];
		}
		void clear() { _size = 0; }
		void resize(size_t size) {
			ink_assert(size <= _size, "Only allow to reduce size");
			_size = size;
		}

		void extend();
	private:

		if_t<dynamic, char, T> _static_data[dynamic ? 1 : initialCapacity];
		T* _dynamic_data = nullptr;
		size_t _capacity;
		size_t _size;
	};

	template<typename T, bool dynamic, size_t initialCapacity>
	void managed_array<T, dynamic, initialCapacity>::extend()
	{
		static_assert(dynamic, "Can only extend if array is dynamic!");
		size_t new_capacity = 1.5f * _capacity;
		if (new_capacity < 5) { new_capacity = 5; }
		T* new_data = new T[new_capacity];

		for(size_t i = 0; i < _capacity; ++i) {
			new_data[i] = _dynamic_data[i];
		}

		delete[] _dynamic_data;
		_dynamic_data = new_data;
		_capacity = new_capacity;
	}

	template<typename T>
	class basic_restorable_array
	{
	public:
		basic_restorable_array(T* array, size_t capacity, T nullValue)
			: _saved(false), _array(array), _temp(array + capacity/2), _capacity(capacity/2), _null(nullValue)
		{
			inkAssert(capacity % 2 == 0, "basic_restorable_array requires a datablock of even length to split into two arrays");

			// zero out main array and put 'nulls' in the clear_temp()
			inkZeroMemory(_array, _capacity * sizeof(T));
			clear_temp();
		}

		// == Non-Copyable ==
		basic_restorable_array(const basic_restorable_array<T>&) = delete;
		basic_restorable_array<T>& operator=(const basic_restorable_array<T>&) = delete;

		// set value by index
		void set(size_t index, const T& value);

		// get value by index
		const T& get(size_t index) const;

		// size of the array
		inline size_t capacity() const { return _capacity; }

		// only const indexing is supported due to save/restore system
		inline const T& operator[](size_t index) const { return get(index); }

		// == Save/Restore ==
		void save();
		void restore();
		void forget();

		// Resets all values and clears any save points
		void clear(const T& value);

	protected:
		inline T* buffer() { return _array; }
		void set_new_buffer(T* buffer, size_t capacity) {
			_array = buffer;
			_temp = buffer + capacity/2;
			_capacity = capacity/2;
		}

	private:
		inline void check_index(size_t index) const { inkAssert(index < capacity(), "Index out of range!"); }
		void clear_temp();
	private:
		bool _saved;

		// real values live here
		T* _array;

		// we store values here when we're in save mode
		//  they're copied on a call to forget()
		T* _temp;

		// size of both _array and _temp
		size_t _capacity;

		// null
		const T _null;
	};

	template<typename T>
	inline void basic_restorable_array<T>::set(size_t index, const T& value)
	{
		check_index(index);
		inkAssert(value != _null, "Can not add a value considered a 'null' to a restorable_array");

		// If we're saved, store in second half of the array
		if (_saved)
			_temp[index] = value;
		else
			// Otherwise, store in the main array
			_array[index] = value;
	}

	template<typename T>
	inline const T& basic_restorable_array<T>::get(size_t index) const
	{
		check_index(index);

		// If we're in save mode and we have a value at that index, return that instead
		if (_saved && _temp[index] != _null)
			return _temp[index];

		// Otherwise, fall back on the real array
		return _array[index];
	}

	template<typename T>
	inline void basic_restorable_array<T>::save()
	{
		// Put us into save/restore mode
		_saved = true;
	}

	template<typename T>
	inline void basic_restorable_array<T>::restore()
	{
		clear_temp();

		// Clear saved flag
		_saved = false;
	}

	template<typename T>
	inline void basic_restorable_array<T>::forget()
	{
		// Run through the _temp array
		for (size_t i = 0; i < _capacity; i++)
		{
			// Copy if there's values
			if (_temp[i] != _null)
				_array[i] = _temp[i];

			// Clear
			_temp[i] = _null;
		}
	}

	template<typename T>
	inline void basic_restorable_array<T>::clear_temp()
	{
		// Run through the _temp array
		for (size_t i = 0; i < _capacity; i++)
		{
			// Clear
			_temp[i] = _null;
		}
	}

	template<typename T>
	inline void basic_restorable_array<T>::clear(const T& value)
	{
		_saved = false;
		for (size_t i = 0; i < _capacity; i++)
		{
			_temp[i] = _null;
			_array[i] = value;
		}
	}

	template<typename T, size_t SIZE>
	class fixed_restorable_array : public basic_restorable_array<T>
	{
	public:
		fixed_restorable_array(const T& initial, const T &nullValue) : basic_restorable_array<T>(_buffer, SIZE * 2, nullValue) 
		{ 
			basic_restorable_array<T>::clear(initial);
		}

	private:
		T _buffer[SIZE * 2];
	};

	template<typename T>
	class allocated_restorable_array : public basic_restorable_array<T>
	{
		using base = basic_restorable_array<T>;
	public:
		allocated_restorable_array(const T& initial, const T& nullValue)
			: basic_restorable_array<T>(0, 0, nullValue), _initialValue{initial}, _nullValue{nullValue},
				_buffer{nullptr}
		{}
		allocated_restorable_array(size_t capacity, const T& initial, const T &nullValue)
			: basic_restorable_array<T>(new T[capacity * 2], capacity * 2, nullValue),
			_initialValue{initial},
			_nullValue{nullValue}
		{
			_buffer = this->buffer();
			this->clear(_initialValue);
		}

		void resize(size_t n) {
			size_t new_capacity = 2 * n;
			T* new_buffer = new T[new_capacity];
			if (_buffer) {
				for(size_t i = 0; i < base::capacity(); ++i) {
					new_buffer[i] = _buffer[i];
					// copy temp
					new_buffer[i + base::capacity()] = _buffer[i + base::capacity()];
				}
				delete[] _buffer;
			}
			for(size_t i = base::capacity(); i < new_capacity; ++i) {
				new_buffer[i] = _initialValue;
				new_buffer[i+base::capacity()] = _nullValue;
			}

			_buffer = new_buffer;
			this->set_new_buffer(_buffer, new_capacity);
		}

		~allocated_restorable_array()
		{
			if(_buffer) {
				delete[] _buffer;
				_buffer = nullptr;
			}
		}

	private:
		T _initialValue;
		T _nullValue;
		T* _buffer;
	};
}

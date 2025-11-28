/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "config.h"
#include "snapshot_interface.h"
#include "system.h"
#include "traits.h"

#include <limits>

namespace ink::runtime::internal
{
/** Managed array of objects.
 *
 * @tparam simple if the object has a trivial destructor, so delete[](char*) can be used instead of
 * calling the constructor.
 * @tparam dynamic if the memory should be allocated on the heap and grow if needed
 * @tparam initialCapacitiy number of elements to allocate at construction, if !dynamic, this is
 * allocated in place and can not be changed.
 */
template<typename T, bool dynamic, size_t initialCapacity, bool simple>
class managed_array : public snapshot_interface
{
public:
	managed_array()
	    : _capacity{initialCapacity}
	    , _size{0}
	    , _static_data{}
	{
		if constexpr (dynamic) {
			if constexpr (simple) {
				_dynamic_data = reinterpret_cast<T*>(new char[sizeof(T) * initialCapacity]);
			} else {
				_dynamic_data = new T[initialCapacity];
			}
		}
	}

	config::statistics::container statistics() const
	{
		return {static_cast<int>(_capacity), static_cast<int>(_size)};
	}

	virtual ~managed_array()
	{
		if constexpr (dynamic) {
			if constexpr (simple) {
				delete[] reinterpret_cast<char*>(_dynamic_data);
			} else {
				delete[] _dynamic_data;
			}
		}
	}

	const T& operator[](size_t i) const
	{
		inkAssert(i < _size, "Access array out of bounds, index %u in array of size %u", i, _size);
		return data()[i];
	}

	T& operator[](size_t i)
	{
		inkAssert(i < _size, "Access array out of bounds, index %u in array of size %u", i, _size);
		return data()[i];
	}

	const T* data() const
	{
		if constexpr (dynamic) {
			return _dynamic_data;
		} else {
			return _static_data;
		}
	}

	T* data()
	{
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

	size_t size() const { return _size; }

	size_t capacity() const { return _capacity; }

	T& push()
	{
		if constexpr (dynamic) {
			if (_size == _capacity) {
				extend();
			}
		} else {
			inkAssert(_size <= _capacity, "Try to append to a full array!");
			// TODO(JBenda): Silent fail?
		}
		return data()[_size++];
	}

	virtual T& insert(size_t position)
	{
		inkAssert(
		    position <= _size,
		    "Array must be dense, cannot insert value at position larger then array.size."
		);
		push();
		if (_size >= 2) {
			for (size_t i = _size - 2; i >= position && i < std::numeric_limits<size_t>::max(); --i) {
				data()[i + 1] = data()[i];
			}
		}
		return data()[position];
	}

	virtual void remove(size_t begin, size_t end)
	{
		inkAssert(end <= _size, "can not delete behind end of array.");
		inkAssert(begin <= end, "can not remove negative range.");
		for (size_t i = 0; i < (end - begin) && end + i < _size; ++i) {
			data()[begin + i] = data()[end + i];
		}
		_size -= end - begin;
	}

	void clear() { _size = 0; }

	void resize(size_t size)
	{
		if constexpr (dynamic) {
			if (size > _capacity) {
				extend(size);
			}
		} else {
			inkAssert(size <= _size, "Only allow to reduce size");
		}
		_size = size;
	}

	void extend(size_t capacity = 0);

	size_t snap(unsigned char* data, const snapper& snapper) const
	{
		inkAssert(! is_pointer<T>{}(), "here is a special case oversight");
		unsigned char* ptr          = data;
		bool           should_write = data != nullptr;
		ptr                         = snap_write(ptr, _size, should_write);
		for (const T& e : *this) {
			if constexpr (is_base_of<snapshot_interface, T>::value) {
				ptr += e.snap(data == nullptr ? nullptr : ptr, snapper);
			} else {
				ptr = snap_write(ptr, e, should_write);
			}
		}
		return static_cast<size_t>(ptr - data);
	}

	const unsigned char* snap_load(const unsigned char* ptr, const loader& loader)
	{
		decltype(_size) size;
		ptr = snap_read(ptr, size);
		if constexpr (dynamic) {
			resize(size);
		} else {
			inkAssert(size <= initialCapacity, "capacity of non dynamic array is to small vor snapshot!");
			_size = size;
		}
		for (T& e : *this) {
			if constexpr (is_base_of<snapshot_interface, T>::value) {
				ptr = e.snap_load(ptr, loader);
			} else {
				ptr = snap_read(ptr, e);
			}
		}
		return ptr;
	}

private:
	T*                     _dynamic_data = nullptr;
	size_t                 _capacity;
	size_t                 _size;
	if_t<dynamic, char, T> _static_data[dynamic ? 1 : initialCapacity];
};

template<typename T, bool dynamic, size_t initialCapacity>
class managed_restorable_array : public managed_array<T, dynamic, initialCapacity>
{
	using base = managed_array<T, dynamic, initialCapacity>;

public:
	managed_restorable_array()
	    : base()
	{
	}

	virtual T& insert(size_t position) override
	{
		inkAssert(position >= _last_size, "Cannot insert data before last save point.");
		return base::insert(position);
	}

	virtual void remove(size_t begin, size_t end) override
	{
		inkAssert(begin >= _last_size, "Cannot delete data before last save point.");
		base::remove(begin, end);
	}

	void restore()
	{
		base::resize(_last_size);
		_last_size = 0;
	}

	void save() { _last_size = this->size(); }

	void forgett() { _last_size = 0; }

	bool has_changed() const { return base::size() != _last_size; }

	size_t last_size() const { return _last_size; }

	size_t snap(unsigned char* data, const snapshot_interface::snapper& snapper) const
	{
		unsigned char* ptr          = data;
		bool           should_write = data != nullptr;
		ptr += base::snap(ptr, snapper);
		ptr = base::snap_write(ptr, _last_size, should_write);
		return static_cast<size_t>(ptr - data);
	}

	const unsigned char* snap_load(const unsigned char* ptr, const snapshot_interface::loader& loader)
	{
		ptr = base::snap_load(ptr, loader);
		ptr = base::snap_read(ptr, _last_size);
		return ptr;
	}

private:
	size_t _last_size = 0;
};

template<typename T, bool dynamic, size_t initialCapacity, bool simple>
void managed_array<T, dynamic, initialCapacity, simple>::extend(size_t capacity)
{
	static_assert(dynamic, "Can only extend if array is dynamic!");
	size_t new_capacity = capacity > _capacity ? capacity : _capacity + _capacity / 2U;
	if (new_capacity < 5) {
		new_capacity = 5;
	}
	T* new_data = nullptr;
	if constexpr (simple) {
		new_data = reinterpret_cast<T*>(new char[sizeof(T) * new_capacity]);
	} else {
		new_data = new T[new_capacity];
	}

	for (size_t i = 0; i < _capacity; ++i) {
		new_data[i] = _dynamic_data[i];
	}

	if constexpr (simple) {
		delete[] reinterpret_cast<char*>(_dynamic_data);
	} else {
		delete[] _dynamic_data;
	}
	_dynamic_data = new_data;
	_capacity     = new_capacity;
}

template<typename T>
class basic_restorable_array : public snapshot_interface
{
public:
	basic_restorable_array(T* array, size_t capacity, T nullValue)
	    : _saved(false)
	    , _array(array)
	    , _temp(array + capacity / 2)
	    , _capacity(capacity / 2)
	    , _null(nullValue)
	{
		inkAssert(
		    capacity % 2 == 0,
		    "basic_restorable_array requires a datablock of even length to split into two arrays"
		);

		// zero out main array and put 'nulls' in the clear_temp()
		inkZeroMemory(_array, _capacity * sizeof(T));
		clear_temp();
	}

	// == Non-Copyable ==
	basic_restorable_array(const basic_restorable_array<T>&)               = delete;
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

	// snapshot interface
	virtual size_t               snap(unsigned char* data, const snapper&) const;
	virtual const unsigned char* snap_load(const unsigned char* data, const loader&);

protected:
	inline T* buffer() { return _array; }

	void set_new_buffer(T* buffer, size_t capacity)
	{
		_array    = buffer;
		_temp     = buffer + capacity / 2;
		_capacity = capacity / 2;
	}

private:
	inline void check_index(size_t index) const
	{
		inkAssert(index < capacity(), "Index out of range!");
	}

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
	if (_saved) {
		_temp[index] = value;
	} else {
		// Otherwise, store in the main array
		_array[index] = value;
	}
}

template<typename T>
inline const T& basic_restorable_array<T>::get(size_t index) const
{
	check_index(index);

	// If we're in save mode and we have a value at that index, return that instead
	if (_saved && _temp[index] != _null) {
		return _temp[index];
	}

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
	for (size_t i = 0; i < _capacity; i++) {
		// Copy if there's values
		if (_temp[i] != _null) {
			_array[i] = _temp[i];
		}

		// Clear
		_temp[i] = _null;
	}
}

template<typename T>
inline void basic_restorable_array<T>::clear_temp()
{
	// Run through the _temp array
	for (size_t i = 0; i < _capacity; i++) {
		// Clear
		_temp[i] = _null;
	}
}

template<typename T>
inline void basic_restorable_array<T>::clear(const T& value)
{
	_saved = false;
	for (size_t i = 0; i < _capacity; i++) {
		_temp[i]  = _null;
		_array[i] = value;
	}
}

template<typename T, size_t SIZE>
class fixed_restorable_array : public basic_restorable_array<T>
{
public:
	fixed_restorable_array(const T& initial, const T& nullValue)
	    : basic_restorable_array<T>(_buffer, SIZE * 2, nullValue)
	{
		basic_restorable_array<T>::clear(initial);
	}

private:
	T _buffer[SIZE * 2];
};

template<typename T>
class allocated_restorable_array final : public basic_restorable_array<T>
{
	using base = basic_restorable_array<T>;

public:
	allocated_restorable_array(const T& initial, const T& nullValue)
	    : basic_restorable_array<T>(0, 0, nullValue)
	    , _initialValue{initial}
	    , _nullValue{nullValue}
	    , _buffer{nullptr}
	{
	}

	allocated_restorable_array(size_t capacity, const T& initial, const T& nullValue)
	    : basic_restorable_array<T>(new T[capacity * 2], capacity * 2, nullValue)
	    , _initialValue{initial}
	    , _nullValue{nullValue}
	{
		_buffer = this->buffer();
		this->clear(_initialValue);
	}

	void resize(size_t n)
	{
		size_t new_capacity = 2 * n;
		T*     new_buffer   = new T[new_capacity];
		if (_buffer) {
			for (size_t i = 0; i < base::capacity(); ++i) {
				new_buffer[i]     = _buffer[i];
				// copy temp
				new_buffer[i + n] = _buffer[i + base::capacity()];
			}
			delete[] _buffer;
		}
		for (size_t i = base::capacity(); i < n; ++i) {
			new_buffer[i]     = _initialValue;
			new_buffer[i + n] = _nullValue;
		}

		_buffer = new_buffer;
		this->set_new_buffer(_buffer, new_capacity);
	}

	virtual ~allocated_restorable_array()
	{
		if (_buffer) {
			delete[] _buffer;
			_buffer = nullptr;
		}
	}

private:
	T  _initialValue;
	T  _nullValue;
	T* _buffer;
};

template<typename T>
inline size_t basic_restorable_array<T>::snap(unsigned char* data, const snapper&) const
{
	unsigned char* ptr          = data;
	bool           should_write = data != nullptr;
	ptr                         = snap_write(ptr, _saved, should_write);
	ptr                         = snap_write(ptr, _capacity, should_write);
	ptr                         = snap_write(ptr, _null, should_write);
	for (size_t i = 0; i < _capacity; ++i) {
		ptr = snap_write(ptr, _array[i], should_write);
		ptr = snap_write(ptr, _temp[i], should_write);
	}
	return static_cast<size_t>(ptr - data);
}

template<typename T>
inline const unsigned char*
    basic_restorable_array<T>::snap_load(const unsigned char* data, const loader&)
{
	auto ptr = data;
	ptr      = snap_read(ptr, _saved);
	decltype(_capacity) capacity;
	ptr = snap_read(ptr, capacity);
	if (buffer() == nullptr) {
		static_cast<allocated_restorable_array<T>&>(*this).resize(capacity);
	}
	inkAssert(
	    _capacity >= capacity, "New config does not allow for necessary size used by this snapshot!"
	);
	T null;
	ptr = snap_read(ptr, null);
	inkAssert(null == _null, "null value is different to snapshot!");
	for (size_t i = 0; i < _capacity; ++i) {
		ptr = snap_read(ptr, _array[i]);
		ptr = snap_read(ptr, _temp[i]);
	}
	return ptr;
}
} // namespace ink::runtime::internal

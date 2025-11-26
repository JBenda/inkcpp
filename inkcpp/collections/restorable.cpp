/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "restorable.h"
#include "../stack.h"

namespace ink::runtime::internal
{
unsigned char*
    snap_base(unsigned char* ptr, bool write, size_t pos, size_t jump, size_t save, size_t& max)
{
	ptr = snapshot_interface::snap_write(ptr, pos, write);
	ptr = snapshot_interface::snap_write(ptr, jump, write);
	ptr = snapshot_interface::snap_write(ptr, save, write);
	max = pos;
	if (jump != ~0U && jump > max) {
		max = jump;
	}
	if (save != ~0U && save > max) {
		max = save;
	}
	return ptr;
}

const unsigned char*
    snap_load_base(const unsigned char* ptr, size_t& pos, size_t& jump, size_t& save, size_t& max)
{
	ptr = snapshot_interface::snap_read(ptr, pos);
	ptr = snapshot_interface::snap_read(ptr, jump);
	ptr = snapshot_interface::snap_read(ptr, save);
	max = pos;
	if (jump != ~0U && jump > max) {
		max = jump;
	}
	if (save != ~0U && save > max) {
		max = save;
	}
	return ptr;
}

template<>
size_t restorable<entry>::snap(unsigned char* data, const snapper& snapper) const
{
	unsigned char* ptr = data;
	size_t         max;
	ptr = snap_base(ptr, data != nullptr, _pos, _jump, _save, max);
	for (size_t i = 0; i < max; ++i) {
		ptr = snap_write(ptr, _buffer[i].name, data != nullptr);
		ptr += _buffer[i].data.snap(data ? ptr : nullptr, snapper);
	}
	return static_cast<size_t>(ptr - data);
}

template<>
size_t restorable<value>::snap(unsigned char* data, const snapper& snapper) const
{
	unsigned char* ptr = data;
	size_t         max;
	ptr = snap_base(ptr, data != nullptr, _pos, _jump, _save, max);
	for (size_t i = 0; i < max; ++i) {
		ptr += _buffer[i].snap(data ? ptr : nullptr, snapper);
	}
	return static_cast<size_t>(ptr - data);
}

template<>
size_t restorable<int>::snap(unsigned char* data, const snapper&) const
{
	unsigned char* ptr = data;
	size_t         max;
	ptr = snap_base(ptr, data != nullptr, _pos, _jump, _save, max);
	for (size_t i = 0; i < max; ++i) {
		ptr = snap_write(ptr, _buffer[i], data != nullptr);
	}
	return static_cast<size_t>(ptr - data);
}

template<>
const unsigned char* restorable<entry>::snap_load(const unsigned char* ptr, const loader& loader)
{
	size_t max;
	ptr = snap_load_base(ptr, _pos, _jump, _save, max);
	while (_size < max) {
		overflow(_buffer, _size);
	}
	for (size_t i = 0; i < max; ++i) {
		ptr = snap_read(ptr, _buffer[i].name);
		ptr = _buffer[i].data.snap_load(ptr, loader);
	}
	return ptr;
}

template<>
const unsigned char* restorable<value>::snap_load(const unsigned char* ptr, const loader& loader)
{
	size_t max;
	ptr = snap_load_base(ptr, _pos, _jump, _save, max);
	while (_size < max) {
		overflow(_buffer, _size);
	}
	for (size_t i = 0; i < max; ++i) {
		ptr = _buffer[i].snap_load(ptr, loader);
	}
	return ptr;
}

template<>
const unsigned char* restorable<int>::snap_load(const unsigned char* ptr, const loader&)
{
	size_t max;
	ptr = snap_load_base(ptr, _pos, _jump, _save, max);
	while (_size < max) {
		overflow(_buffer, _size);
	}
	for (size_t i = 0; i < max; ++i) {
		ptr = snap_read(ptr, _buffer[i]);
	}
	return ptr;
}

} // namespace ink::runtime::internal

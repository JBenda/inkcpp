#include "restorable.h"
#include "../stack.h"

namespace ink::runtime::internal {
	template<>
	size_t restorable<entry>::snap(unsigned char* data, const snapper& snapper) const
	{
		unsigned char* ptr = data;
		ptr = snap_write(ptr, _pos, data);
		ptr = snap_write(ptr, _jump, data);
		ptr = snap_write(ptr, _save, data);
		size_t max = _pos;
		if (_jump > max) { max = _jump; }
		if (_save > max) { max = _save; }
		for(size_t i = 0; i < max; ++i) {
			ptr = snap_write(ptr, _buffer[i].name, data);
			ptr += _buffer[i].data.snap(data ? ptr : nullptr, snapper);
		}
		return ptr - data;
	}
	template<>
	size_t restorable<value>::snap(unsigned char* data, const snapper& snapper) const
	{
		unsigned char* ptr = data;
		ptr = snap_write(ptr, _pos, data);
		ptr = snap_write(ptr, _jump, data);
		ptr = snap_write(ptr, _save, data);
		size_t max = _pos;
		if (_jump > max) { max = _jump; }
		if (_save > max) { max = _save; }
		for(size_t i = 0; i < max; ++i) {
			ptr += _buffer[i].snap(data ? ptr : nullptr, snapper);
		}
		return ptr - data;
	}
}

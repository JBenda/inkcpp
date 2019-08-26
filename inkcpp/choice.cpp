#include "pch.h"
#include "choice.h"

namespace binary
{
	namespace runtime
	{
		choice::choice(const std::string& text, int index, uint32_t path)
			: _text(text), _index(index), _path(path)
		{
		}
	}
}
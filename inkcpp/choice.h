#pragma once

namespace binary
{
	namespace runtime
	{
		class choice
		{
		public:
			choice(const std::string& text, int index, uint32_t path);

			int index() const { return _index; }
			const std::string& text() const { return _text; }
			uint32_t path() const { return _path; }
		private:
			std::string _text;
			int _index;
			uint32_t _path;
		};
	}
}

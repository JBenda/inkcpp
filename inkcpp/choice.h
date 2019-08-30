#pragma once

namespace ink
{
	namespace runtime
	{
		namespace internal
		{
			class basic_stream;
		}

		class choice
		{
		public:
			int index() const { return _index; }
			const char* text() const { return _text; }
			uint32_t path() const { return _path; }
		private:
			friend class runner;

			void setup(internal::basic_stream&, int index, uint32_t path);
		private:
			const char* _text;
			int _index;
			uint32_t _path;
		};
	}
}

#pragma once

#include "value.h"
#include "system.h"

namespace binary
{
	namespace runtime
	{
		class ink
		{
		public:
			ink();
			~ink();
			void load(const char* filename);

			const char* string(uint32_t index) const;
			const unsigned char* instructions() const { return instruction_data; }
			const unsigned char* end() const { return file + length; }

		private:
			unsigned char* file;
			size_t length;
			const char* string_table;
			const unsigned char* instruction_data;
		};

		typedef const unsigned char* IP;
		class choice;

		class runner
		{
		public:
			runner(const ink* story);

			void run();

			const std::vector<choice*>& choices() const { return _choices; }
			void choose(int index);

			std::string getline();


		private:
			void step();

		private:
			const ink* story;

			// == State ==

			// IP
			IP ptr;

			// Evaluation (NOTE: Will later need to be per-callstack entry)
			bool bEvaluationMode;
			std::stack<value> evalStack;

			bool bStringMode;
			std::stringstream outputStream;

			// Temporary variables (NOTE: Will be later moved into per-callstack entry)
			std::map<system::NameHash, value> temporary;

			std::vector<choice*> _choices;

			value pop();

		};
	}
}
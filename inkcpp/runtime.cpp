#include "pch.h"
#include "runtime.h"
#include "command.h"
#include "operators.h"
#include "choice.h"

namespace binary
{
	namespace runtime
	{
		unsigned char* read_file_into_memory(const char* filename, size_t* read)
		{
			using namespace std;

			ifstream ifs(filename, ios::binary | ios::ate);
			ifstream::pos_type pos = ifs.tellg();
			size_t length = (size_t)pos;
			unsigned char* data = new unsigned char[length];
			ifs.seekg(0, ios::beg);
			ifs.read((char*)data, length);
			ifs.close();

			*read = (size_t)length;
			return data;
		}

		ink::ink() 
			: file(nullptr)
			, string_table(nullptr)
			, instruction_data(nullptr)
		{ }

		ink::~ink()
		{
			if (file != nullptr)
				delete[] file;

			file = nullptr;
			instruction_data = nullptr;
			string_table = nullptr;
		}

		void ink::load(const char* filename)
		{
			// Load file into memory
			file = read_file_into_memory(filename, &length);

			// String table is after the version information
			string_table = (char*)file + sizeof(int);

			// Pass over strings
			const char* ptr = string_table;
			while (true)
			{
				// Read until null terminator
				while (*ptr != 0)
					ptr++;

				// Check next character
				ptr++;

				// Second null. Strings are done.
				if (*ptr == 0)
					break;
			}

			// After strings comes instruction data
			instruction_data = (const unsigned char*)ptr + 1;
		}

		const char* ink::string(uint32_t index) const
		{
			return string_table + index;
		}

		template<typename T>
		inline T read(IP& ip)
		{
			T val = *(const T*)ip;
			ip += sizeof(T);
			return val;
		}

		inline const char* read(IP& ip, const ink* story)
		{
			uint32_t str = read<uint32_t>(ip);
			return story->string(str);
		}

		runner::runner(const ink* data)
			: story(data)
		{
			ptr = story->instructions();
			bEvaluationMode = false;
		}

		void runner::run()
		{
			while (true) {
				// Load current command
				Command cmd = read<Command>(ptr);
				CommandFlag flag = read<CommandFlag>(ptr);

				// Binary operators
				if (cmd >= BINARY_OPERATORS_START && cmd < BINARY_OPERATORS_END)
				{
					value a = pop(), b = pop();
					value c = evaluate(cmd, a, b);
					evalStack.push(c);
					continue;
				}

				switch (cmd)
				{
				case Command::STR:
				{
					const char* str = read(ptr, story);

					if (!bEvaluationMode)
						std::cout << str;
					else if (bStringMode)
						outputStream << str;
					else
						evalStack.push(value(str));
				}
				break;
				case Command::DIVERT_VAL:
				{
					// TODO: Error if not in eval mode?

					// Push the divert target onto the stack
					uint32_t target = read<uint32_t>(ptr);
					evalStack.push(value(target));
				}
				break;
				case Command::INT:
				{
					int val = read<int>(ptr);
					if (!bEvaluationMode)
						std::cout << val;
					else if (bStringMode)
						outputStream << val;
					else
						evalStack.push(value(val));
				}
				break;
				case Command::DIVERT:
				{
					uint32_t target = read<uint32_t>(ptr);
					ptr = story->instructions() + target;
				}
				break;
				case DIVERT_TO_VARIABLE:
				{
					// Get variable value
					system::NameHash variable = read<system::NameHash>(ptr);
					const value& val = temporary.at(variable);

					// Move to location
					ptr = story->instructions() + val.as_divert();
				}
				break;
				case CHOICE:
				{
					std::stringstream choiceText;

					if (flag & CHOICE_HAS_CONDITION) {} // TODO
					if (flag & CHOICE_HAS_START_CONTENT) {
						choiceText << pop();
					} 
					if (flag & CHOICE_HAS_CHOICE_ONLY_CONTENT) {
						choiceText << pop();
					}
					if (flag & CHOICE_IS_INVISIBLE_DEFAULT) {} // TODO
					if (flag & CHOICE_IS_ONCE_ONLY) {} // TODO

					// Read path
					uint32_t path = read<uint32_t>(ptr);

					// Create choice and record it
					auto c = new choice(choiceText.str(), _choices.size(), path);
					_choices.push_back(c);
				} break;
				case START_STR:
				{
					bStringMode = true;
				} break;
				case END_STR:
				{
					bStringMode = false;

					// Create value
					value val = outputStream.str();
					outputStream.str("");

					// Push onto stack
					evalStack.push(val);
				} break;
				case START_EVAL:
					bEvaluationMode = true;
					break;
				case END_EVAL:
					bEvaluationMode = false;
					break;
				case OUTPUT:
				{
					value v = pop();
					std::cout << v;
				}
				break;
				case DEFINE_TEMP:
				{
					system::NameHash variableName = read<system::NameHash>(ptr);

					// Get the top value and put it into the variable
					value v = pop();
					temporary[variableName] = v;
				}
					break;
				case END:
				case DONE:
					return;
					break;
				}
			}
		}

		void runner::choose(int index)
		{
			ptr = story->instructions() + _choices[index]->path();
		}

		value runner::pop()
		{
			value v = evalStack.top();
			evalStack.pop();
			return v;
		}

		std::ostream& operator<<(std::ostream& out, const value& v)
		{
			switch (v.type())
			{
			case FLOAT:
				out << v.as_float();
				break;
			case INTEGER:
				out << v.as_int();
				break;
			case STRING:
				out << v.as_string();
				break;
			default:
				throw std::exception();
			}

			return out;
		}

	}
}
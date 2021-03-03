#pragma once

#include "config.h"
#ifdef INK_EXPOSE_JSON
#include "../json.hpp"
#endif
#include "compilation_results.h"
#include <iostream>

namespace ink
{
	namespace compiler
	{
		struct json;
		class output {
		public:
			output(const char*const& filename): _name{&filename}{}
			output(std::ostream& out) : _stream{&out}{}
			std::ostream& get(std::ofstream& out) const;
		private:
			std::ostream* _stream = nullptr;
			const char*const* _name = nullptr;
		};
		class input{
		public:
			input(const char*const& filename) : _name{&filename} {}
			input(std::istream& in) : _stream{&in} {}
#ifdef INK_EXPOSE_JSON
			input(const json& j) : _json{&j} {}
#endif
			const json& get(json& in) const;
		private:
			std::istream* _stream = nullptr;
			const char*const* _name = nullptr;
			const json* _json = nullptr;
		};

		void run(const input&, const output& bin_out, const output& str_out, compilation_results* results = nullptr);

	}
}

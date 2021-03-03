#include "compiler.h"

#include "json_compiler.h"
#include "binary_emitter.h"

#include <fstream>

namespace ink::compiler
{
	struct json : public nlohmann::json { };
	const json& input::get(json& j) const {
		if(_name) {
			std::ifstream(*_name, std::ios::binary | std::ios::in) >> j;
			return j;
		} else if (_stream) {
			*_stream >> j;
			return j;
		} else if (_json) {
			return *_json;
		} else {
			throw ink_exception("no input defined!");
		}
	}
	std::ostream& output::get(std::ofstream& out) const {
		if (_name) {
			out.open(*_name);
			return out;
		} else if (_stream) {
			return *_stream;
		} else {
			throw ink_exception("no output defined!");
		}
	}
	void run(const nlohmann::json& src, std::ostream& out, std::ostream& strings, compilation_results* results)
	{
		using namespace internal;

		// Create compiler and emitter
		json_compiler compiler;
		binary_emitter emitter;

		// Compile into emitter
		compiler.compile(src, &emitter, results);

		// write emitter's results into the stream
		emitter.output(out, strings);
	}

	void run(const input& in, const output& bin_out, const output& str_out, compilation_results* results)
	{
		json j;
		std::ofstream bout, sout;
		run(in.get(j), bin_out.get(bout), str_out.get(sout), results);
	}
}

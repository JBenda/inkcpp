#include "compilation_results.h"
#include "story_ptr.h"
#include "types.h"
#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace py = pybind11;

#include <story.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>
#include <globals.h>
#include <snapshot.h>

#include <sstream>
#include <functional>

using runner      = ink::runtime::runner_interface;
using runner_ptr  = ink::runtime::runner;
using globals     = ink::runtime::globals_interface;
using globals_ptr = ink::runtime::globals;
using story       = ink::runtime::story;
using choice      = ink::runtime::choice;
using value       = ink::runtime::value;
using list        = ink::runtime::list_interface;
using snapshot    = ink::runtime::snapshot;

PYBIND11_DECLARE_HOLDER_TYPE(T, ink::runtime::story_ptr<T>);

struct StringValueWrap : public value {
	StringValueWrap(const std::string& s)
	    : str{s}
	{
		value::type     = value::Type::String;
		value::v_string = str.c_str();
	}

	~StringValueWrap() { std::cout << "death" << std::endl; }

	std::string str;
};

std::string list_to_str(const list& list)
{
	std::stringstream out;
	out << "[";
	bool first = true;
	for (const auto& flag : list) {
		if (first) {
			first = false;
		} else {
			out << ", ";
		}
		out << flag;
	}
	out << "]";
	return out.str();
}

PYBIND11_MODULE(inkcpp_py, m)
{
	m.doc()
	    = "Python bindings for InkCPP https://github.com/JBenda/inkcpp"; // optional module docstring

	py::class_<story>(m, "Story")
	    .def("from_file", &story::from_file, "Creates a new story from a .bin file")
	    .def("new_globals", &story::new_globals, "creates new globals store for the current story")
	    .def("new_runner", [](story& self) { return self.new_runner(); })
	    .def("new_runner", &story::new_runner, "creates a new runner for the current story")
	    .def("new_globals_from_snapshot", &story::new_globals_from_snapshot)
	    .def("new_runner_from_snapshot", &story::new_runner_from_snapshot)
	    .def(
	        "new_runner_from_snapshot", [](story& self, const snapshot& snap, globals_ptr store
	                                    ) { return self.new_runner_from_snapshot(snap, store); }
	    )
	    .def(
	        "new_runner_from_snapshot",
	        [](story& self, const snapshot& snap, globals_ptr store, unsigned idx) {
		        return self.new_runner_from_snapshot(snap, store, idx);
	        }
	    );

	py::class_<globals, globals_ptr>(m, "Globals")
	    .def(
	        "create_snapshot", &globals::create_snapshot,
	        "Creates a snapshot from the current state for later usage"
	    )
	    .def(
	        "observe_ping",
	        [](globals& self, const char* name, std::function<void()> f) { self.observe(name, f); },
	        "Get a ping each time the observed variable changes"
	    )
	    .def(
	        "observe_value",
	        [](globals& self, const char* name, std::function<void(const value&)> f) {
		        self.observe(name, f);
	        },
	        "Get a call with new value each time the variable changes"
	    )
	    .def(
	        "observe_delta",
	        [](globals& self, const char* name,
	           std::function<void(const value&, ink::optional<const value>)> f) {
		        self.observe(name, f);
	        },
	        "Get a call with the new and old value (this could be None) each time the variable "
	        "changes"
	    )
	    .def(
	        "__getattr__",
	        [](const globals& self, const std::string& key) {
		        auto res = self.get<value>(key.c_str());
		        if (! res.has_value()) {
			        throw py::key_error(std::string("No global variable with name '") + key + "' found");
		        }
		        return res.value();
	        },
	        "Access global varible if exists, if not throws an key_error"
	    )
	    .def("__setattr__", [](globals& self, const std::string& key, const value& val) {
		    if (! self.set<value>(key.c_str(), val)) {
			    throw py::key_error(
			        std::string("No global variable with name '") + key
			        + "' found you are trying to override a non string variable with a string"
			    );
		    }
	    });

	py::class_<list, std::unique_ptr<list, py::nodelete>>(
	    m, "List",
	    "Allows reading and editing inkcpp lists. !Only valid until next choose ore getline a runner "
	    "referncing the corresponding global"
	)
	    .def("add", &list::add, "Add flag to list")
	    .def("remove", &list::remove, "Remove flag from list")
	    .def("contains", &list::contains, "Check if list contains the given flag")
	    .def("__str__", &list_to_str);


	py::class_<value> py_value(m, "Value", "A Value of a Ink Variable");
	py_value.def_readonly("type", &value::type, "Type contained in value");
	py_value.def(py::init<>());
	py_value.def(py::init<bool>());
	py_value.def(py::init<uint32_t>());
	py_value.def(py::init<int32_t>());
	py_value.def(py::init<float>());
	py_value.def(py::init<list*>());
	py_value.def(py::init([](const std::string& str) { return new StringValueWrap(str); }));
	py_value.def(
	    "as_list",
	    [](const value& self) {
		    if (self.type != value::Type::List) {
			    throw py::attribute_error("Try to access list of non list value");
		    }
		    return self.v_list;
	    },
	    py::return_value_policy::reference
	);
	py_value.def("__str__", [](const value& self) {
		switch (self.type) {
			case value::Type::Bool: return std::string(self.v_bool ? "true" : "false");
			case value::Type::Uint32: return std::to_string(self.v_uint32);
			case value::Type::Int32: return std::to_string(self.v_int32);
			case value::Type::String: return std::string(self.v_string);
			case value::Type::Float: return std::to_string(self.v_float);
			case value::Type::List: {
				return list_to_str(*self.v_list);
			}
		}
		throw py::attribute_error("value is in an invalid state");
	});

	py::enum_<value::Type>(m, "Type")
	    .value("Bool", value::Type::Bool)
	    .value("Uint32", value::Type::Uint32)
	    .value("Int32", value::Type::Int32)
	    .value("String", value::Type::String)
	    .value("Float", value::Type::Float)
	    .value("List", value::Type::List)
	    .export_values();

	py::class_<runner, runner_ptr>(m, "Runner")
	    .def(
	        "create_snapshot", &runner::create_snapshot,
	        "Creates a snapshot from the current state for later usage"
	    )
	    .def("can_continue", &runner::can_continue, "check if there is content left in story")
	    .def("getline", static_cast<std::string (runner::*)()>(&runner::getline))
	    .def("has_tags", &runner::has_tags, "Where there tags since last getline")
	    .def("num_tags", &runner::num_tags, "Number of tags currently stored")
	    .def(
	        "get_tag", &runner::get_tag, "Get Tag currently stored at position i",
	        py::return_value_policy::reference_internal
	    )
	    .def(
	        "tags",
	        [](const runner& self) {
		        std::vector<const char*> tags(self.num_tags());
		        for (size_t i = 0; i < self.num_tags(); ++i) {
			        tags[i] = self.get_tag(i);
		        }
		        return tags;
	        },
	        "Get all current assigned tags"
	    )
	    .def(
	        "has_choices", &runner::has_choices,
	        "Check if there is at least one open choice at the moment."
	    )
	    .def(
	        "get_choice", &runner::get_choice, "Get current choice at index",
	        py::return_value_policy::reference_internal
	    )
	    .def("num_choices", &runner::num_choices, "Number of current open choices")
	    .def(
	        "__iter__",
	        [](const runner& self) { return py::make_iterator(self.begin(), self.end()); },
	        py::keep_alive<0, 1>()
	    )
	    .def("choose", &runner::choose, "Select a choice to continue")
	    .def(
	        "bind_void",
	        [](runner& self, const char* function_name, std::function<void(std::vector<value>)> f) {
		        self.bind(function_name, [f](size_t len, const value* vals) {
			        std::vector args(vals, vals + len);
			        f(args);
		        });
	        },
	        "Bind function which void result"
	    )
	    .def(
	        "bind",
	        [](runner& self, const char* function_name, std::function<value(std::vector<value>)> f) {
		        self.bind(function_name, [f](size_t len, const value* vals) {
			        std::vector args(vals, vals + len);
			        return f(args);
		        });
	        },
	        "Bind a function with return value"
	    );
	py::class_<choice>(m, "Choice")
	    .def("text", &choice::text, "Get choice printable content")
	    .def("has_tags", &choice::has_tags, "if choices is tagged?")
	    .def("num_tags", &choice::num_tags, "Number of tags assigned to choice")
	    .def(
	        "get_tag", &choice::get_tag, "Get tag at index",
	        py::return_value_policy::reference_internal
	    )
	    .def(
	        "tags",
	        [](const choice& self) {
		        std::vector<const char*> tags(self.num_tags());
		        for (size_t i = 0; i < self.num_tags(); ++i) {
			        tags[i] = self.get_tag(i);
		        }
		        return tags;
	        },
	        "Get all current assinged tags"
	    );
	py::class_<snapshot>(
	    m, "Snapshot", "Globals and all assoziatet runner stored for later restoration"
	)
	    .def("num_runners", &snapshot::num_runners, "Number of different runners stored in snapshot")
	    .def("write_to_file", &snapshot::write_to_file, "Store snapshot in file")
	    .def("from_file", &snapshot::from_file, "Load snapshot from file");
	m.def(
	    "compile_json",
	    [](const char* input_file_name, const char* output_filen_ame) {
		    ink::compiler::compilation_results results;
		    ink::compiler::run(input_file_name, output_filen_ame, &results);
		    if (! results.errors.empty()) {
			    std::string str;
			    for (auto& error : results.errors) {
				    str += error;
				    str += '\n';
			    }
			    throw py::value_error(str);
		    }
	    },
	    "Converts a story.json file to a story.bin file used by inkcpp"
	);
}

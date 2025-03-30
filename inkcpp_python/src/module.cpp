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
using namespace pybind11::literals;

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
using ilist       = ink::runtime::list_interface;
using snapshot    = ink::runtime::snapshot;

PYBIND11_DECLARE_HOLDER_TYPE(T, ink::runtime::story_ptr<T>);

struct StringValueWrap : public value {
	StringValueWrap(const std::string& s)
	    : value()
	    , str{s}
	{
		static_cast<value&>(*this) = value(str.c_str());
	}

	~StringValueWrap() {}

	std::string str;
};

std::string list_to_str(const ilist& list)
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
	py::options options;
	options.disable_enum_members_docstring();
	m.doc()
	    = "Python bindings for InkCPP https://github.com/JBenda/inkcpp"; // optional module docstring

	m.def(
	    "hash_string", &ink::hash_string,
	    "Converts string into hash used inside inkcpp as representation.",
	    py::arg("str").none("false")
	);
	py::class_<ilist, std::unique_ptr<ilist, py::nodelete>> py_list(
	    m, "IList",
	    "Allows reading and editing inkcpp lists. !Only valid until next choose ore getline a runner "
	    "referncing the corresponding global"
	);
	py::class_<ilist::iterator::Flag>(
	    py_list, "Flag", "A list flag containing the name of the flag and the corresponding list"
	)
	    .def_readonly("name", &ilist::iterator::Flag::flag_name, "The flag")
	    .def_readonly(
	        "list_name", &ilist::iterator::Flag::list_name, "Name of the corresponding list"
	    );
	py_list.def("add", &ilist::add, "Add flag to list.", py::arg("flag").none(false))
	    .def("remove", &ilist::remove, "Remove flag from list", py::arg("flag").none(false))
	    .def(
	        "contains", &ilist::contains, "Check if list contains the given flag",
	        py::arg("flag").none(false)
	    )
	    .def(
	        "flags_from",
	        [](const ilist& self, const char* list_name) {
		        return py::make_iterator(self.begin(list_name), self.end());
	        },
	        R"(Rerutrns all flags contained in this list from a list of name list_name.
	      
Use iter(List) to iterate over all flags.)",
	        py::keep_alive<0, 1>(), py::arg("list_name").none(false)
	    )
	    .def(
	        "__iter__", [](const ilist& self) { return py::make_iterator(self.begin(), self.end()); },
	        py::keep_alive<0, 1>()
	    )
	    .def("__str__", &list_to_str);
	py::class_<value> py_value(m, "Value", "A Value of a Ink Variable");
	py::enum_<value::Type>(py_value, "Type")
	    .value("Bool", value::Type::Bool)
	    .value("Uint32", value::Type::Uint32)
	    .value("Int32", value::Type::Int32)
	    .value("String", value::Type::String)
	    .value("Float", value::Type::Float)
	    .value("List", value::Type::List)
	    .export_values();
	py_value.def_readonly("type", &value::type, "Type contained in value");
	// py_value.def(py::init<>());
	py_value.def(py::init<bool>(), py::arg("value").none(false));
	py_value.def(
	    "__init__",
	    [](value& self, uint32_t v, value::Type type) {
		    if (type != value::Type::Uint32) {
			    throw py::key_error("only use this signture if you want to explicit pass a uint");
		    }
		    self = value(v);
	    },
	    "Used to explicit set a Uint32 value. Type must be inkcpp_py.Value.Type.Uint32!",
	    py::arg("value").none(false), py::arg("type").none(false)
	);
	py_value.def(py::init<int32_t>(), py::arg("value").none(false));
	py_value.def(py::init<float>(), py::arg("value").none(false));
	py_value.def(py::init<ilist*>(), py::arg("value").none(false));
	py_value.def(
	    py::init([](const std::string& str) { return new StringValueWrap(str); }),
	    py::arg("value").none(false)
	);
	py_value.def(
	    "as_list",
	    [](const value& self) {
		    if (self.type != value::Type::List) {
			    throw py::attribute_error("Try to access list of non list value");
		    }
		    return self.get<value::Type::List>();
	    },
	    R"(If it contains a inkcpp_py.Value.Type.List, return it. Else throws AttributeError.)",
	    py::return_value_policy::reference
	);
	py_value.def(
	    "as_int",
	    [](const value& self) {
		    if (self.type == value::Type::Int32) {
			    return static_cast<int64_t>(self.get<value::Type::Int32>());
		    } else if (self.type == value::Type::Uint32) {
			    return static_cast<int64_t>(self.get<value::Type::Uint32>());
		    }
		    throw py::attribute_error("Try to access a int of a non int32 nor uint32 value.");
	    },
	    "If value contains a inkcpp_py.Value.Type.Int32 or inkcpp_py.Value.Type.Uint32 return the "
	    "int value. "
	    "Else throws AttributeError"
	);
	py_value.def(
	    "as_bool",
	    [](const value& self) {
		    if (self.type != value::Type::Bool) {
			    throw py::attribute_error("Try to access a bool of a non bool value.");
		    }
		    return self.get<value::Type::Bool>();
	    },
	    "If value contains a inkcpp_py.Value.Type.Bool, return it. Else throws a AttributeError."
	);
	py_value.def(
	    "as_str",
	    [](const value& self) {
		    if (self.type != value::Type::String) {
			    throw py::attribute_error("Try to access a string of a non string value.");
		    }
		    return self.get<value::Type::String>();
	    },
	    R"(If value contains a inkcpp_py.Value.Type.String, return it. Else throws an AttributeError.
  
If you want convert it to a string use: `str(value)`.)"
	);
	py_value.def(
	    "as_float",
	    [](const value& self) {
		    if (self.type != value::Type::Float) {
			    throw py::attribute_error("Try to access a float of a non float value.");
		    }
		    return self.get<value::Type::Float>();
	    },
	    "If value contains a inkcpp_py.Value.Type.Float, return it. Else throws an AttributeError."
	);
	py_value.def("__str__", [](const value& self) {
		switch (self.type) {
			case value::Type::Bool: return std::string(self.get<value::Type::Bool>() ? "true" : "false");
			case value::Type::Uint32: return std::to_string(self.get<value::Type::Uint32>());
			case value::Type::Int32: return std::to_string(self.get<value::Type::Int32>());
			case value::Type::String: return std::string(self.get<value::Type::String>());
			case value::Type::Float: return std::to_string(self.get<value::Type::Float>());
			case value::Type::List: {
				return list_to_str(*self.get<value::Type::List>());
			}
		}
		throw py::attribute_error("value is in an invalid state");
	});


	py::class_<snapshot>(
	    m, "Snapshot", "Globals and all assoziatet runner stored for later restoration"
	)
	    .def("num_runners", &snapshot::num_runners, "Number of different runners stored in snapshot")
	    .def(
	        "write_to_file", &snapshot::write_to_file, "Store snapshot in file.",
	        py::arg("filename").none(false)
	    )
	    .def_static(
	        "from_file", &snapshot::from_file, "Load snapshot from file",
	        py::arg("filename").none(false)
	    );
	m.def(
	    "compile_json",
	    [](const char* input_file_name, const char* output_file_name) {
		    ink::compiler::compilation_results results;
		    ink::compiler::run(input_file_name, output_file_name, &results);
		    if (! results.errors.empty()) {
			    std::string str;
			    for (auto& error : results.errors) {
				    str += error;
				    str += '\n';
			    }
			    throw py::value_error(str);
		    }
	    },
	    py::arg("input_file_name").none(false), py::arg("output_file_name").none(false),
	    "Converts a story.json file to a story.bin file used by inkcpp"
	);
	py::class_<choice>(m, "Choice")
	    .def("text", &choice::text, "Get choice printable content")
	    .def("has_tags", &choice::has_tags, "if choices is tagged?")
	    .def("num_tags", &choice::num_tags, "Number of tags assigned to choice")
	    .def(
	        "get_tag", &choice::get_tag, "Get tag at index", py::arg("index").none(false),
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

	py::class_<globals, globals_ptr>(
	    m, "Globals", "Global variable store. Use `globals[var_name]` to read/write them."
	)
	    .def(
	        "create_snapshot", &globals::create_snapshot,
	        "Creates a snapshot from the current state for later usage"
	    )
	    .def(
	        "observe",
	        [](globals& self, const char* name,
	           std::function<void(const value&, ink::optional<const value>)> f) {
		        self.observe(name, f);
	        },
	        R"(
Start observing a variable.

Args:
     name: name of the (global) variable to observe
     callback: called when varibale changes with `(new_value, old_value)`.
               `old_value` will be `None` when variable is initelized
	        )",
	        py::arg("name").none(false), py::arg("callback").none(false)
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
	        "Access global varible if exists, if not throws an KeyError"
	    )
	    .def("__setattr__", [](globals& self, const std::string& key, const value& val) {
		    if (! self.set<value>(key.c_str(), val)) {
			    throw py::key_error(
			        std::string("No global variable with name '") + key
			        + "' found or you are trying to override a non string variable with a string"
			    );
		    }
	    });
	py::class_<runner, runner_ptr>(m, "Runner", "Runtime logic for a story.")
	    .def(
	        "create_snapshot", &runner::create_snapshot,
	        R"(Creates a snapshot from the current state for later usage.

This snapshot will also contain the global state.
To reload:

>>> snapshot = old_runner.create_snapshot()
>>> story = Story.from_file("story.bin")
>>> runner = story.new_runner_from_snapshot(snapshot)
	        )"
	    )
	    .def("can_continue", &runner::can_continue, "check if there is content left in story")
	    .def(
	        "getline", static_cast<std::string (runner::*)()>(&runner::getline),
	        "Get content of the next output line"
	    )
	    .def(
	        "getall", static_cast<std::string (runner::*)()>(&runner::getall),
	        "execute getline and append until inkcp_py.Runner.can_continue is false"
	    )
	    .def("has_tags", &runner::has_tags, "Where there tags assoziated with the last line.")
	    .def("num_tags", &runner::num_tags, "Number of tags assoziated with last line.")
	    .def(
	        "get_tag", &runner::get_tag, "Get Tag currently stored at index",
	        py::arg("index").none(false), py::return_value_policy::reference_internal
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
	        "has_knot_tags", &runner::has_knot_tags,
	        "Are there tags assoziated with current knot/stitch."
	    )
	    .def(
	        "num_knot_tags", &runner::num_knot_tags,
	        "Number of tags assoziated with current knot/stitch."
	    )
	    .def(
	        "get_knot_tag", &runner::get_knot_tag, "Get knot/stitch tag stored at index.",
	        py::arg("index").none(false), py::return_value_policy::reference_internal
	    )
	    .def(
	        "knot_tags",
	        [](const runner& self) {
		        std::vector<const char*> tags(self.num_knot_tags());
		        for (size_t i = 0; i < self.num_knot_tags(); ++i) {
			        tags[i] = self.get_knot_tag(i);
		        }
		        return tags;
	        },
	        "Get all tags assoziated with current knot/stitch."
	    )
	    .def("current_knot", &runner::get_current_knot, "Get hash of current knot/tag path.")
	    .def(
	        "has_global_tags", &runner::has_global_tags,
	        "Are there tags assoziated with current global."
	    )
	    .def(
	        "num_global_tags", &runner::num_global_tags,
	        "Number of tags assoziated with current global."
	    )
	    .def(
	        "get_global_tag", &runner::get_global_tag, "Get global tag stored at index.",
	        py::arg("index").none(false), py::return_value_policy::reference_internal
	    )
	    .def(
	        "global_tags",
	        [](const runner& self) {
		        std::vector<const char*> tags(self.num_global_tags());
		        for (size_t i = 0; i < self.num_global_tags(); ++i) {
			        tags[i] = self.get_global_tag(i);
		        }
		        return tags;
	        },
	        "Get all tags assoziated with current global."
	    )
	    .def(
	        "all_tags",
	        [](const runner& self) {
		        std::vector<const char*> line_tags(self.num_tags());
		        for (size_t i = 0; i < self.num_tags(); ++i) {
			        line_tags[i] = self.get_tag(i);
		        }
		        std::vector<const char*> knot_tags(self.num_knot_tags());
		        for (size_t i = 0; i < self.num_knot_tags(); ++i) {
			        knot_tags[i] = self.get_knot_tag(i);
		        }
		        std::vector<const char*> global_tags(self.num_global_tags());
		        for (size_t i = 0; i < self.num_global_tags(); ++i) {
			        global_tags[i] = self.get_global_tag(i);
		        }
		        return py::dict("line"_a = line_tags, "knot"_a = knot_tags, "global"_a = global_tags);
	        },
	        "Get a dictionary with tags for different levels. Be aware that global and knot tags are "
	        "also part of the next output line after their definition."
	    )
	    .def(
	        "has_choices", &runner::has_choices,
	        "Check if there is at least one open choice at the moment."
	    )
	    .def(
	        "get_choice", &runner::get_choice, R"(
Get current choice at index.

iter(inkcpp_py.Runner) returns a iterator over all current choices.)",
	        py::arg("index").none(false), py::return_value_policy::reference_internal
	    )
	    .def("num_choices", &runner::num_choices, "Number of current open choices")
	    .def(
	        "__iter__",
	        [](const runner& self) { return py::make_iterator(self.begin(), self.end()); },
	        py::keep_alive<0, 1>()
	    )
	    .def(
	        "choose", &runner::choose, "Select choice at index and continue.",
	        py::arg("index").none(false)
	    )
	    .def(
	        "bind_void",
	        [](runner& self, const char* function_name, std::function<void(std::vector<value>)> f,
	           bool lookaheadSafe) {
		        self.bind(
		            function_name,
		            [f](size_t len, const value* vals) {
			            std::vector args(vals, vals + len);
			            f(args);
		            },
		            lookaheadSafe
		        );
	        },
	        py::arg("function_name").none(false), py::arg("function").none(false),
	        py::arg_v("lookaheadSafe", false).none(false), "Bind function with void result"
	    )
	    .def(
	        "bind",
	        [](runner& self, const char* function_name, std::function<value(std::vector<value>)> f,
	           bool lookaheadSafe) {
		        self.bind(function_name, [f](size_t len, const value* vals) {
			        std::vector args(vals, vals + len);
			        return f(args);
		        });
	        },
	        py::arg("function_name").none(false), py::arg("function").none(false),
	        py::arg_v("lookaheadSafe", false).none(false), "Bind a function with return value"
	    )
	    .def(
	        "move_to",
	        [](runner& self, const char* path) -> bool {
		        return self.move_to(ink::hash_string(path));
	        },
	        "Moves execution pointer to start of container desrcipet by the path",
	        py::arg("path").none(false)
	    );
	py::class_<story>(m, "Story")
	    .def_static(
	        "from_file", &story::from_file, R"(
Creates a new story from a .bin file.

Returns:
    inkcpp_py.Story: a new story
)",
	        py::arg("filename").none(false)
	    )
	    .def("new_globals", &story::new_globals, R"(
Creates new globals store for the current story.
)")
	    .def(
	        "new_runner", &story::new_runner, R"(
Ceates a new runner.

Args:
    globals: pass a global to use, else use a new inetrnal one.

Returns:
    inkcpp_py.Runner: a newly created runner, initelized at start of story
	    )",
	        py::arg("globals").none(true) = py::none()
	    )
	    .def(
	        "new_globals_from_snapshot", &story::new_globals_from_snapshot, R"(
Loads a global store from a snapshot.

Returns:
    inkcpp_py.Globals: a new global store with the same state then stored in the snapshot.
	    )",
	        py::arg("snapshot").none(false)
	    )
	    .def(
	        "new_runner_from_snapshot", &story::new_runner_from_snapshot, R"(
Reconstructs a runner from a snapshot.

Args:
    snapshot: snapshot to load runner from.
    globals: store used by this runner, else load the store from the file and use it.
                                           (created with inkcpp_py.Story.new_runner_from_snapshot) 
    runner_id: if multiple runners are stored in the snapshot, id of runner to reconstruct. (ids start at 0 and are dense)

Returns:
    inkcpp_py.Runner: at same state as before
)",
	        py::arg("snapshot").none(false), py::arg("globals").none(true) = py::none(),
	        py::arg("runner_id").none(false) = 0
	    );
}

#include "story_ptr.h"
#include "types.h"
#include <pybind11/attr.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <story.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>
#include <globals.h>
#include <snapshot.h>

using runner      = ink::runtime::runner_interface;
using runner_ptr  = ink::runtime::runner;
using globals     = ink::runtime::globals_interface;
using globals_ptr = ink::runtime::globals;
using story       = ink::runtime::story;
using choice      = ink::runtime::choice;

PYBIND11_DECLARE_HOLDER_TYPE(T, ink::runtime::story_ptr<T>);

PYBIND11_MODULE(inkcpp_py, m)
{
	m.doc()
	    = "Python bindings for InkCPP https://github.com/JBenda/inkcpp"; // optional module docstring

	py::class_<story>(m, "Story")
	    .def("from_file", &story::from_file, "Creates a new story from a .bin file")
	    .def("new_globals", &story::new_globals, "creates new globals store for the current story")
	    .def("new_runner", [](story& self) { return self.new_runner(); })
	    .def("new_runner", &story::new_runner, "creates a new runner for the current story");

	py::class_<globals, globals_ptr>(m, "Globals");

	py::class_<runner, runner_ptr>(m, "Runner")
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
	    .def("choose", &runner::choose, "Select a choice to continue");

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
}

#pragma once

#include <ostream>

namespace ink::compiler
{
	struct compilation_results;
}

namespace ink::compiler::internal
{
	class reporter
	{
	protected:
		reporter();
		virtual ~reporter() { }

		// sets the results pointer for this reporter
		void set_results(compilation_results*);

		// clears the results pointer
		void clear_results();

		// report warning
		std::ostream& warn();

		// report error
		std::ostream& err();
		
		// report critical error
		std::ostream& crit();
	private:
		compilation_results* _results;
	};
}
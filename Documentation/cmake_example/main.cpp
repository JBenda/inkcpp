#include <ink/system.h>
#include <ink/choice.h>
#include <ink/runner.h>
#include <ink/story.h>
#include <ink/compiler.h>

#include <iostream>

using namespace ink::runtime;

int MyInkFunction(int a, int b) { return a + b; }

int main()
{
	ink::compiler::run("test.ink.json", "test.bin");
	// Load ink binary story, generated from the inkCPP compiler
	story* myInk = story::from_file("test.bin");

	// Create a new thread
	runner thread = myInk->new_runner();

	// Register external functions (glue automatically generated via templates)
	thread->bind("my_ink_function", &MyInkFunction);

	// Write to cout
	while (thread->can_continue())
		std::cout << thread->getline();

	// Iterate choices
	int id = 0;
	for (const choice& c : *thread) {
		std::cout << (id++) << ". " << c.text() << std::endl;
	}
	std::cin >> id;
	thread->choose(id);

	// Write to cout
	while (thread->can_continue())
		std::cout << thread->getline();
}

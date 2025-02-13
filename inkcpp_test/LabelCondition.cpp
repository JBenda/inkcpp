#include "catch.hpp"

#include <choice.h>
#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO( "run story with hidden choice" )
{
	GIVEN( "a story with choice visible by second visit" )
	{
		auto    ink     = story::from_file(INK_TEST_RESOURCE_DIR "LabelConditionStory.bin");
		globals globals = ink->new_globals();
		runner  thread  = ink->new_runner( globals );

		WHEN( "normal run" )
		{
			std::string out = thread->getall();
			REQUIRE( thread->num_choices() == 1 );
			std::string choice1a = thread->get_choice( 0 )->text();
			thread->choose( 0 );
			std::string out2 = thread->getall();
			REQUIRE( thread->num_choices() == 1 );
			std::string choice2a = thread->get_choice( 0 )->text();

			THEN( "second choice keeps hidden" )
			{
				REQUIRE( out == "" );
				REQUIRE( choice1a == "labeled choice" );
				REQUIRE( out2 == "" );
				REQUIRE( choice2a == "labeled choice" );
			}
		}

		WHEN( "set global variable to enable hidden choice" )
		{
			globals->set<bool>( "bool_variable", false );
			std::string out = thread->getall();
			REQUIRE( thread->num_choices() == 1 );
			std::string choice1a = thread->get_choice( 0 )->text();
			thread->choose( 0 );
			std::string out2 = thread->getall();
			REQUIRE( thread->num_choices() == 2 );
			std::string choice2a = thread->get_choice( 0 )->text();
			std::string choice2b = thread->get_choice( 1 )->text();

			THEN( "second choice is visible" )
			{
				REQUIRE( out == "" );
				REQUIRE( choice1a == "labeled choice" );
				REQUIRE( out2 == "" );
				REQUIRE( choice2a == "labeled choice" );
				REQUIRE( choice2b == "hidden choice" );
			}
		}
	}
}

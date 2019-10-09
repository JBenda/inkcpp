#include "catch.hpp"

#include "..//inkcpp/story_ptr.h"

using namespace ink::runtime;
using ink::runtime::internal::ref_block;

class test_object
{
public:
	test_object() { num_objects++; }
	~test_object() { num_objects--; }

	int myData = 5;

	static int num_objects;
};

int test_object::num_objects = 0;

typedef story_ptr<test_object> test_object_p;

SCENARIO("pointers can be created and destroyed", "[pointer]")
{
	// ref_block normally held by the story
	ref_block* story = new ref_block();
	story->references = 1;

	GIVEN("a story pointer")
	{
		{
			// create object and pointer
			test_object* myObject = new test_object();
			test_object_p pointer = test_object_p(myObject, story);

			THEN("it should be valid")
			{
				REQUIRE(pointer);
				REQUIRE(test_object::num_objects == 1);
				REQUIRE(pointer->myData == 5);
			}

			THEN("it should increment references to the story object")
			{
				REQUIRE(story->references == 2);
			}
		}

		WHEN("it goes out of scope")
		{
			THEN("the object should be destroyed")
			{
				REQUIRE(test_object::num_objects == 0);
			}

			THEN("the story references should decrease")
			{
				REQUIRE(story->references == 1);
			}
		}
	}

	delete story;
	story = nullptr;
}

SCENARIO("pointers can be copied and assigned", "[pointer]")
{
	// ref_block normally held by the story
	ref_block* story = new ref_block();
	story->references = 1;

	GIVEN("a pointer")
	{
		// create object and pointer
		test_object* myObject = new test_object();
		test_object_p pointer = test_object_p(myObject, story);

		WHEN("we create a copy")
		{
			{
				test_object_p ref = pointer;

				THEN("there should still only be one object")
				{
					REQUIRE(test_object::num_objects == 1);
				}
			}

			WHEN("that copy goes out of scope")
			{
				THEN("we should still have one object")
				{
					REQUIRE(test_object::num_objects == 1);
				}
			}
		}
	}

	GIVEN("two pointers to the same object created by assignment")
	{
		{
			// create object and two pointers
			test_object* myObject = new test_object();
			test_object_p pointer = test_object_p(myObject, story);
			test_object_p ref = pointer;
		}

		WHEN("both go out of scope")
		{
			THEN("we should have zero instances")
			{
				REQUIRE(test_object::num_objects == 0);
			}
		}
	}

	GIVEN("two pointers to the same object created by copy-constructors")
	{
		{
			// create object and two pointers
			test_object* myObject = new test_object();
			test_object_p pointer = test_object_p(myObject, story);
			test_object_p ref = test_object_p(pointer);
		}

		WHEN("both go out of scope")
		{
			THEN("we should have zero instances")
			{
				REQUIRE(test_object::num_objects == 0);
			}
		}
	}

	GIVEN("two pointers to different objects")
	{
		test_object* myObject = new test_object();
		test_object* myOtherObject = new test_object();
		test_object_p pointerA = test_object_p(myObject, story);
		test_object_p pointerB = test_object_p(myOtherObject, story);

		THEN("we should have two instances")
		{
			REQUIRE(test_object::num_objects == 2);
		}

		WHEN("we assign one pointer to the other")
		{
			pointerB = pointerA;

			THEN("we should only have one object")
			{
				REQUIRE(test_object::num_objects == 1);
			}
		}
	}

	delete story;
	story = nullptr;
}

SCENARIO("pointers become invalid when the story dies", "[pointer]")
{
	// ref_block normally held by the story
	ref_block* story = new ref_block();
	story->references = 1;

	GIVEN("a pointer")
	{
		test_object* myObject = new test_object();
		test_object_p pointer = test_object_p(myObject, story);

		THEN("it should be valid")
		{
			REQUIRE(pointer);
		}

		WHEN("the story becomes invalid")
		{
			story->valid = false;

			THEN("the pointer should be invalid")
			{
				REQUIRE(!pointer);
			}
		}
	}

	delete story;
}
#include "catch.hpp"

#include "../inkcpp/stack.h"

using ink::hash_t;
using ink::thread_t;
using ink::runtime::internal::value;
using ink::runtime::internal::value_type;
using ink::runtime::internal::frame_type;

const hash_t X = ink::hash_string("X");
const hash_t Y = ink::hash_string("Y");
const hash_t Z = ink::hash_string("Z");

value operator "" _v(unsigned long long i) {
	return value{}.set<value_type::int32>(static_cast<int32_t>(i));
}

SCENARIO("threading with the callstack", "[callstack]")
{
	GIVEN("a callstack with a few temporary variables")
	{
		// Create the stack
		auto stack = ink::runtime::internal::stack<50>();

		// Set X and Y temporary variables
		stack.set(X, 100_v);
		stack.set(Y, 200_v);

		WHEN("there is a fork and more is pushed")
		{
			thread_t thread = stack.fork_thread();

			THEN("old variables aren't accessible")
			{
				REQUIRE(stack.get(X) == nullptr);
				REQUIRE(stack.get(Y) == nullptr);
			}

			// Push something onto the thread
			stack.set(X, 200_v);
			REQUIRE(stack.get(X)->get<value_type::int32>() == 200);

			WHEN("when that thread ends")
			{
				stack.complete_thread(thread);

				WHEN("we collapse to the thread")
				{
					stack.collapse_to_thread(thread);

					THEN("we should have the value from that thread")
					{
						REQUIRE(stack.get(X)->get<value_type::int32>() == 200);
					}
				}

				WHEN("we collapse to the main thraed")
				{
					stack.collapse_to_thread(~0);

					THEN("we should have the value from the original thread")
					{
						REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
						REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
					}
				}

				THEN("we should be able to access original thread values")
				{
					REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
					REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
				}

				WHEN("we push more on the main thread")
				{
					stack.set(Z, 500_v);

					THEN("collapsing to the thread shouldn't have the values anymore")
					{
						stack.collapse_to_thread(thread);
						REQUIRE(stack.get(Z) == nullptr);
					}

					WHEN("we start a second thread that closes")
					{
						thread_t thread2 = stack.fork_thread();
						stack.set(X, 999_v);
						stack.complete_thread(thread2);

						THEN("we can still collapse to the main thread")
						{
							stack.collapse_to_thread(~0);
							REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
							REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
						}

						THEN("we can still collapse to the first thread")
						{
							stack.collapse_to_thread(thread);
							REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
							REQUIRE(stack.get(Z) == nullptr);
						}
					}
				}
			}

			WHEN("that thread also forks a thread")
			{
				thread_t thread2 = stack.fork_thread();

				// Put something on this thread
				stack.set(X, 999_v);

				WHEN("that inner thread and outer thread complete")
				{
					stack.complete_thread(thread2);
					stack.complete_thread(thread);

					WHEN("we collapse to the inner thread")
					{
						stack.collapse_to_thread(thread2);

						THEN("we should have the value from that thread")
						{
							REQUIRE(stack.get(X)->get<value_type::int32>() == 999);
						}
					}

					WHEN("we collapse to the outer thread")
					{
						stack.collapse_to_thread(thread);

						THEN("we should have the value from that thread")
						{
							REQUIRE(stack.get(X)->get<value_type::int32>() == 200);
						}
					}

					WHEN("we collapse to the main thraed")
					{
						stack.collapse_to_thread(~0);

						THEN("we should have the value from the original thread")
						{
							REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
							REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
						}
					}
				}
			}
		}

		WHEN("there is a fork with a tunnel that finishes")
		{
			thread_t thread = stack.fork_thread();
			stack.push_frame<frame_type::tunnel>(555, false);
			stack.complete_thread(thread);

			THEN("there should be no frames on the stack")
			{
				REQUIRE(stack.has_frame() == false);
			}
		}
	}

	GIVEN("a callstack with a single tunnel pushed")
	{
		// Create the stack
		auto stack = ink::runtime::internal::stack<50>();

		// Set X and Y temporary variables
		stack.set(X, 100_v);
		stack.set(Y, 200_v);

		// Push a tunnel
		stack.push_frame<frame_type::tunnel>(505, false);

		// Push some more temps
		stack.set(X, 101_v);
		stack.set(Y, 201_v);

		bool eval_mode;

		WHEN("a thread is forked")
		{
			thread_t thread = stack.fork_thread();

			WHEN("that thread does a tunnel return")
			{
				frame_type type;
				auto offset = stack.pop_frame(&type, eval_mode);
				
				THEN("that thread should be outside the tunnel")
				{
					REQUIRE(type == frame_type::tunnel);
					REQUIRE(offset == 505);

					REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
					REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
				}

				stack.complete_thread(thread);

				WHEN("we collapse to that thread")
				{
					stack.collapse_to_thread(thread);
					THEN("the stack should be outside the tunnel")
					{
						REQUIRE(stack.get(X)->type() == value_type::int32);
						REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
						REQUIRE(stack.get(Y)->type() == value_type::int32);
						REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
					}
				}

				WHEN("we collapse back to the main thread")
				{
					stack.collapse_to_thread(~0);
					THEN("the stack should be inside the tunnel")
					{
						REQUIRE(stack.get(X)->type() == value_type::int32);
						REQUIRE(stack.get(X)->get<value_type::int32>() == 101);
						REQUIRE(stack.get(Y)->type() == value_type::int32);
						REQUIRE(stack.get(Y)->get<value_type::int32>() == 201);
					}

					WHEN("we do a tunnel return")
					{
						frame_type type;
						auto offset = stack.pop_frame(&type, eval_mode);

						THEN("we should be back outside")
						{
							REQUIRE(type == frame_type::tunnel);
							REQUIRE(offset == 505);
							REQUIRE(stack.get(X)->type() == value_type::int32);
							REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
							REQUIRE(stack.get(Y)->type() == value_type::int32);
							REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
						}
					}
				}
			}

			WHEN("that thread completes and we pop off the main thread")
			{
				stack.complete_thread(thread);
				frame_type type;
				auto offset = stack.pop_frame(&type, eval_mode);

				THEN("we should be outside the tunnel")
				{
					REQUIRE(stack.get(X)->type() == value_type::int32);
					REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
					REQUIRE(stack.get(Y)->type() == value_type::int32);
					REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
				}

				THEN("collapsing to the thread will have the correct callstack")
				{
					stack.collapse_to_thread(thread);

					REQUIRE(stack.get(X)->type() == value_type::int32);
					REQUIRE(stack.get(X)->get<value_type::int32>() == 101);
					REQUIRE(stack.get(Y)->type() == value_type::int32);
					REQUIRE(stack.get(Y)->get<value_type::int32>() == 201);
				}
			}
		}
	}

	GIVEN("A thread with a frame pushed")
	{
		// Create the stack
		auto stack = ink::runtime::internal::stack<50>();
		
		// Create the thread
		thread_t thread = stack.fork_thread();

		// Set X and Y temporary variables
		stack.set(X, 100_v);
		stack.set(Y, 200_v);

		// Push a tunnel
		stack.push_frame<frame_type::tunnel>(505, false);

		// Push some more temps
		stack.set(X, 101_v);
		stack.set(Y, 201_v);

		bool eval_mode;

		WHEN("a second thread is forked off the first")
		{
			thread_t thread2 = stack.fork_thread();

			WHEN("the second thread ends")
			{
				stack.complete_thread(thread2);

				WHEN("the first thread does a pop")
				{
					frame_type _ignore;
					stack.pop_frame(&_ignore, eval_mode);

					THEN("accessing the variable should return the original")
					{
						REQUIRE(stack.get(X)->type() == value_type::int32);
						REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
						REQUIRE(stack.get(Y)->type() == value_type::int32);
						REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
					}

					WHEN("the first thread ends")
					{
						stack.complete_thread(thread);

						THEN("collapsing to the first thread should return the correct variables")
						{
							stack.collapse_to_thread(thread);

							REQUIRE(stack.get(X)->type() == value_type::int32);
							REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
							REQUIRE(stack.get(Y)->type() == value_type::int32);
							REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
						}
					}
				}
			}
		}
	}

	GIVEN("A thread with two frames pushed")
	{
		// Create the stack
		auto stack = ink::runtime::internal::stack<50>();

		// Create the thread
		thread_t thread = stack.fork_thread();

		// Set X and Y temporary variables
		stack.set(X, 100_v);
		stack.set(Y, 200_v);

		// Push a tunnel
		stack.push_frame<frame_type::tunnel>(505, false);

		// Push some more temps
		stack.set(X, 101_v);
		stack.set(Y, 201_v);

		// Push another tunnel
		stack.push_frame<frame_type::tunnel>(505, false);

		// Push some more temps
		stack.set(X, 102_v);
		stack.set(Y, 202_v);

		WHEN("another thread is started and completed on top of it")
		{
			thread_t thread2 = stack.fork_thread();
			stack.complete_thread(thread2);

			WHEN("we then try to pop both frames")
			{
				frame_type _ignore;
				bool eval_mode;
				stack.pop_frame(&_ignore, eval_mode);
				stack.pop_frame(&_ignore, eval_mode);

				THEN("we should have access to the original variables")
				{
					REQUIRE(stack.get(X)->type() == value_type::int32);
					REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
					REQUIRE(stack.get(Y)->type() == value_type::int32);
					REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
				}

				THEN("collapsing to the thread should also get us the original variables")
				{
					stack.complete_thread(thread);
					stack.collapse_to_thread(thread);
					REQUIRE(stack.get(X)->type() == value_type::int32);
					REQUIRE(stack.get(X)->get<value_type::int32>() == 100);
					REQUIRE(stack.get(Y)->type() == value_type::int32);
					REQUIRE(stack.get(Y)->get<value_type::int32>() == 200);
				}
			}
		}
	}
}

#pragma once

#include "value.h"
#include "system.h"
#include "output.h"
#include "stack.h"
#include "choice.h"
#include "config.h"
#include "simple_restorable_stack.h"
#include "types.h"
#include "functions.h"
#include "string_table.h"
#include "list_table.h"
#include "array.h"
#include "random.h"
#include "snapshot_impl.h"

#include "runner.h"
#include "choice.h"

#include "executioner.h"

namespace ink::runtime::internal
{
	class story_impl;
	class globals_impl;
	class snapshot_impl;


	class runner_impl : public runner_interface, public snapshot_interface
	{
	public:
		// Creates a new runner at the start of a loaded ink story
		runner_impl(const story_impl*, globals);
		virtual ~runner_impl();

		// used by the globals object to do garbage collection
		void mark_used(string_table&, list_table&) const;

#pragma region runner Implementation
		// sets seed for prng in runner
		virtual void set_rng_seed(uint32_t seed) override { _rng.srand(seed); }

		// Checks that the runner can continue
		virtual bool can_continue() const override;

		// Begin iterating choices
		virtual const choice* begin() const override { return _choices.begin(); }

		// End iterating choices
		virtual const choice* end() const override { return _choices.end(); }

		// Chooses a choice by index
		virtual void choose(size_t index) override;

		// runs silently
		void getline_silent();

		virtual bool has_tags() const override;
		virtual size_t num_tags() const override;
		virtual const char* get_tag(size_t index) const override;

		snapshot* create_snapshot() const override;	

		size_t snap(unsigned char* data, snapper&) const;
		const unsigned char* snap_load(const unsigned char* data, loader&);


#ifdef INK_ENABLE_CSTD
		// c-style getline
		virtual char* getline_alloc() override;
#endif

		// move to path
		virtual bool move_to(hash_t path) override;

#ifdef INK_ENABLE_STL
		// Gets a single line of output and stores it in a C++ std::string
		virtual std::string getline() override;

		// Reads a line into a std::ostream
		virtual void getline(std::ostream&) override;

		// get all into string
		virtual std::string getall() override;

		// get all into stream
		virtual void getall(std::ostream&) override;
#endif
#ifdef INK_ENABLE_UNREAL
		// Reads a line into an Unreal FString
		virtual FString getline() override;
#endif
#pragma endregion
	protected:
		// bind external
		virtual void internal_bind(hash_t name, internal::function_base* function) override;
	private:
		// Advances the interpreter by a line. This fills the output buffer
		void advance_line();

		// Steps the interpreter a single instruction and returns
		//  when it has hit a new line
		bool line_step();

		// Steps the interpreter a single instruction
		void step();

		// Resets the runtime
		void reset();

		// == Save/Restore
		void save();
		void restore();
		void forget();

		enum class Scope { NONE, GLOBAL, LOCAL};
		template<Scope Hint = Scope::NONE>	
		value* get_var(hash_t variableName);
		template<Scope Hint = Scope::NONE>	
		const value* get_var(hash_t variableName) const;
		template<Scope Hint = Scope::NONE>
		void set_var(hash_t variableName, const value& val, bool is_redef);
		const value* dereference(const value& val);

		enum class change_type
		{
			no_change,
			extended_past_newline,
			newline_removed
		};

		change_type detect_change() const;

	private:
		template<typename T>
		inline T read();

		choice& add_choice();
		void clear_choices();

		void clear_tags();

		// Special code for jumping from the current IP to another
		void jump(ip_t, bool record_visits);

		void run_binary_operator(unsigned char cmd);
		void run_unary_operator(unsigned char cmd);

		frame_type execute_return();
		template<frame_type type>
		void start_frame(uint32_t target);

		void on_done(bool setDone);
		void set_done_ptr(ip_t ptr);

		inline thread_t current_thread() const { return _threads.empty() ? ~0 : _threads.top(); }

	public:
		template<bool dynamic, size_t N>
		class threads : public internal::managed_restorable_stack<thread_t, dynamic, N> {
			using base = internal::managed_restorable_stack<thread_t, dynamic, N>;
		public:
			template<bool ... D, bool con = dynamic,  enable_if_t<con, bool> = true >
			threads()
				: base(~0),
				_threadDone(nullptr, reinterpret_cast<ip_t>(~0)) {
					static_assert(sizeof...(D) == 0, "Don't use explicit template arguments!");
			}
			template<bool ... D, bool con = dynamic, enable_if_t<!con, bool> = true >
			threads()
				: base(~0),
				_threadDone(nullptr, reinterpret_cast<ip_t>(~0)) {
					static_assert(sizeof...(D) == 0, "Don't use explicit template arguments");
					_threadDone.clear(nullptr);
				}
			void clear() {
				base::clear();
				_threadDone.clear(nullptr);
			}
			void save() {
				base::save();
				_threadDone.save();
			}
			void restore() {
				base::restore();
				_threadDone.restore();
			}
			void forget() {
				base::forget();
				_threadDone.forget();
			}

			void set(size_t index, const ip_t& value) {
				_threadDone.set(index, value);
			}
			const ip_t& get(size_t index) const {
				return _threadDone.get(index);
			}
			const ip_t& operator[](size_t index) const { return get(index); }

			// snapshot interface
			size_t snap(unsigned char* data, const snapper&) const override;
			const unsigned char* snap_load(const unsigned char* data, const loader&) override;

		protected:
			virtual void overflow(thread_t*& buffer, size_t& size) override final;
		private:
			using array_type = if_t<dynamic,
				internal::allocated_restorable_array<ip_t>,
				internal::fixed_restorable_array<ip_t, N>>;

			void resize(size_t size, int) { _threadDone.resize(size); }


			array_type _threadDone;
		};

	private:
		const story_impl* const _story;
		story_ptr<globals_impl> _globals;
		executer _operations;

		// == State ==

		// Instruction pointer
		ip_t _ptr;
		ip_t _backup; // backup pointer
		ip_t _done; // when we last hit a done

		// Output stream
		internal::stream<config::limitOutputSize> _output;

		// Runtime stack. Used to store temporary variables and callstack
		internal::stack<abs(config::limitRuntimeStack), config::limitRuntimeStack < 0> _stack;
		internal::stack<abs(config::limitReferenceStack), config::limitReferenceStack < 0> _ref_stack;

		// Evaluation stack
		bool _evaluation_mode = false;
		bool _string_mode = false;
		internal::eval_stack<abs(config::limitEvalStackDepth), config::limitEvalStackDepth < 0> _eval;
		bool _saved_evaluation_mode = false;

		// Keeps track of what threads we're inside
		threads <config::limitContainerDepth < 0, abs( config::limitThreadDepth )> _threads;

		// Choice list
		managed_restorable_array<snap_choice, config::maxChoices < 0, abs(config::maxChoices)> _choices;
		optional<snap_choice> _fallback_choice;

		// Tag list
		managed_restorable_array<snap_tag, config::limitActiveTags < 0, abs(config::limitActiveTags)> _tags;
		int _choice_tags_begin;

		// TODO: Move to story? Both?
		functions _functions;

		// Container set
		struct ContainerData {
			container_t id = ~0u;
			ip_t offset = 0;
			bool operator==(const ContainerData& oth) const {
				return oth.id == id && oth.offset == offset;
			}
			bool operator!=(const ContainerData& oth) const { return !(*this == oth); }
		};
		internal::managed_restorable_stack<ContainerData, config::limitContainerDepth < 0,abs(config::limitContainerDepth)> _container;
		bool _is_falling = false;

		bool _saved = false;

		prng _rng;
	};

	template<bool dynamic, size_t N>
	void runner_impl::threads<dynamic, N>::overflow(thread_t*& buffer, size_t& size) {
		base::overflow(buffer, size);
		if constexpr (dynamic) {
			resize(size, 0);
		}
	}

	template<bool dynamic, size_t N>
	size_t runner_impl::threads<dynamic, N>::snap( unsigned char* data, const snapper& snapper ) const {
		unsigned char* ptr = data;
		ptr += base::snap( data ? ptr : nullptr, snapper );
		ptr += _threadDone.snap( data ? ptr : nullptr, snapper );
		return ptr - data;
	}

	template<bool dynamic, size_t N>
	const unsigned char* runner_impl::threads<dynamic, N>::snap_load( const unsigned char* ptr, const loader& loader ) {
		ptr = base::snap_load( ptr, loader );
		ptr = _threadDone.snap_load( ptr, loader );
		return ptr;
	}

	template<>
	inline const char* runner_impl::read();

#ifdef INK_ENABLE_STL
	std::ostream& operator<<(std::ostream&, runner_impl&);
#endif
}

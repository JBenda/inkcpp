#pragma once

#include "system.h"

namespace ink::runtime
{
	namespace internal
	{
		struct ref_block
		{
			ref_block()
				: references(0)
				, valid(true)
			{ }

			size_t references;
			bool valid;

			static void remove_reference(ref_block*&);
		};

		class story_ptr_base
		{
		protected:
			story_ptr_base(internal::ref_block* story);
			story_ptr_base(internal::ref_block* story, internal::ref_block* instance);
			story_ptr_base(const story_ptr_base&);
			
			story_ptr_base& operator=(const story_ptr_base&) = delete;

			void add_reference();
			bool remove_reference();

			void set(const story_ptr_base& other);

			inline bool is_valid() const { 
				return 
					_story_block != nullptr && _instance_block != nullptr 
					&&_story_block->valid && _instance_block->valid; 
			}

			inline bool is_story_valid() const {
				return _story_block != nullptr && _story_block->valid;
			}

		private:
			// reference block for the parent story
			ref_block* _story_block;

			// reference block for this pointer
			ref_block* _instance_block;
		};
	}

	/**
	* Pointer wrapper to an object whose lifetime is tied to a story object.
	*
	* A shared pointer whose lifetime is also tied to the lifetime of the parent
	* story object. The referenced object will live until either
	* 1) There are no more story_ptr's pointing to this object
	* 2) The story object which owns this object dies
	*
	* @see story_interface
	* @see runner_interface
	* @see globals_interface
	*/
	template<typename T>
	class story_ptr : public internal::story_ptr_base
	{
	public:
		// constructor. internal use only.
		story_ptr(T* ptr, internal::ref_block* story)
			: story_ptr_base(story)
			, _ptr(ptr)
		{ 
			add_reference();
		}

		// casting constructor. internal use only
		template<typename U>
		story_ptr(T* ptr, const story_ptr<U>& other)
			: story_ptr_base(other)
			, _ptr(ptr)
		{
			add_reference();
		}

		// pointer constructor. for nullptr only.
		story_ptr(T* ptr)
			: story_ptr_base(nullptr, nullptr)
			, _ptr(nullptr)
		{
			inkAssert(ptr == nullptr, "can not create story_ptr from existing pointer!");
		}

		// null constructor
		story_ptr()
			: story_ptr_base(nullptr, nullptr)
			, _ptr(nullptr)
		{
		}

		// destructor
		~story_ptr();

		// == copy methods ==
		story_ptr(const story_ptr<T>&);
		story_ptr<T>& operator=(const story_ptr<T>&);

		// == casting ==
		template<typename U>
		story_ptr<U> cast()
		{
			// if cast fails, return null
#ifdef INK_ENABLE_UNREAL
			// Unreal disables RTTI
			U* casted = reinterpret_cast<U*>(_ptr);
#else
			U* casted = dynamic_cast<U*>(_ptr);
#endif
			if (casted == nullptr)
				return nullptr;

			// create new pointer with casted value but same instance blocks
			return story_ptr<U>(casted, *this);
		}

		// == equality ==
		inline bool operator==(const story_ptr<T>& other) { return _ptr == other._ptr; }

		// == validity ==
		bool is_valid() const { return story_ptr_base::is_valid() && _ptr; }
		inline operator bool() const { return is_valid(); }

		// === dereference operators ==
		inline T* get() { return is_valid() ? _ptr : nullptr; }
		inline const T* get() const { return is_valid() ? _ptr : nullptr; }
		inline T* operator->() { return get(); }
		inline const T* operator->() const { return get(); }
		inline T& operator*() { return *get(); }
		inline const T& operator*() const { return *get(); }
	private:
		T* _ptr;
	};

	template<typename T>
	story_ptr<T>::~story_ptr()
	{
		if (remove_reference())
		{
			delete _ptr;
			_ptr = nullptr;
		}
	}

	template<typename T>
	story_ptr<T>::story_ptr(const story_ptr<T>& other)
		: story_ptr_base(other)
		, _ptr(other._ptr)
	{
		add_reference();
	}

	template<typename T>
	story_ptr<T>& story_ptr<T>::operator=(const story_ptr<T>& other)
	{
		// Clear out any old data
		if (remove_reference())
		{
			delete _ptr;
			_ptr = nullptr;
		}

		// Set pointers
		set(other);
		_ptr = other._ptr;

		// initialize
		add_reference();

		// return reference to self
		return *this;
	}
}
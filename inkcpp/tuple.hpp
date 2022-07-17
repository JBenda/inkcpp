#pragma once

/// very basic flat tuple implementation, only use for trivial data types.

#include "traits.h"

namespace ink::runtime::internal {
	namespace tuple_internal {
		/// data member of tuple
		template<size_t I, typename T>
		class tuple_leaf{
		public:
			tuple_leaf() : _value() {};
			template<typename U>
			explicit tuple_leaf(U&& u) : _value(std::forward<U>(u)) {}
			T& get() { return _value; }
			const T& get() const { return _value; }
		private:
			T _value;
			tuple_leaf(const tuple_leaf& tl) = delete;
			tuple_leaf& operator=(const tuple_leaf&) = delete;
		};

		// handle indexing
		template<size_t... Is>
		struct tuple_indexes {};

		// create tuple_indexes in [Start,End[
		template<size_t End, size_t Start = 0, size_t... Is>
		struct make_tuple_indexes {
			using type = typename make_tuple_indexes<End, Start+1, Is..., Start>::type;
		};
		template<size_t End, size_t... Is>
		struct make_tuple_indexes<End, End, Is...> {
			using type = tuple_indexes<Is...>;
		};

		/// get the index of first appearance of an type in tuple
		template<typename T, size_t I, typename U, typename ...Tys>
		struct type_index_imp : type_index_imp<T,I+1,Tys...> {};
		template<typename T, size_t I, typename ...Tys>
		struct type_index_imp<T,I,T,Tys...> {
			static constexpr size_t value = I;
		};
		template<typename T, typename ...Tys>
		constexpr size_t type_index = type_index_imp<T,0,Tys...>::value;


		/// implementation class to extract indices
		template<typename Indexes, typename ...Tys>
		struct tuple_imp;
		template<size_t ...Is,  typename ...Tys>
		struct tuple_imp<tuple_indexes<Is...>, Tys...>
			: public tuple_leaf<Is, Tys>...
		{
			template<typename ...Us>
			tuple_imp(Us&& ... us) : tuple_leaf<Is,Tys>(std::forward<Us>(us))... {
				static_assert(sizeof...(Us) == sizeof...(Tys),
						"Tuple must be initialized with same amount of arguments"
						", then types!");
			}
		};
	}

	/// minimal tuple class, only for simple data types!
	/// flat tuple implementation
	template<typename ...Tys>
	class tuple
		: public tuple_internal::tuple_imp<
		  	typename tuple_internal::make_tuple_indexes<sizeof...(Tys)>::type,
		  	Tys... >
	{
		using base = tuple_internal::tuple_imp<
			typename tuple_internal::make_tuple_indexes<sizeof...(Tys)>::type,
			Tys...>;
		using this_type = tuple<Tys...>;
	public:
		template<size_t I>
		using element_type = typename get_ith_type<I, Tys...>::type;
		template<typename T>
		static constexpr size_t type_index = tuple_internal::type_index<T, Tys...>;

		template<typename ...Us>
		tuple(Us&& ... us) : base(std::forward<Us>(us)...) {}
	};

	/// access tuple element by index
	template<size_t I, typename T>
	constexpr auto const&
	get(const T& t) {
		return static_cast<tuple_internal::tuple_leaf<I, typename T::template element_type<I>>const&>(t).get();
	};

	/// access tuple element by type. First of this type
	template<typename T, typename Tuple>
 	constexpr const T&
	get(const Tuple& t) {
		return get<Tuple::template type_index<T>, Tuple>(t);
	};

	template<>
	class tuple<> {
	public:
		tuple() {}
	};

	/// check if tuple contains type
	template<typename T, typename Tuple>
	struct has_type;
	template<typename T>
	struct has_type<T,tuple<>> : false_type {};
	template<typename T, typename U, typename ... Tys>
	struct has_type<T, tuple<U, Tys...>> : has_type<T,tuple<Tys...>> {};
	template<typename T, typename ... Tys>
	struct has_type<T, tuple<T, Tys...>> : true_type {};
}

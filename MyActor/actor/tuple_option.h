#ifndef __TUPLE_OPTION_H
#define __TUPLE_OPTION_H

#include <tuple>
#include "try_move.h"

template <typename... TYPES>
struct types_pck
{
	enum { number = sizeof...(TYPES) };
};

template <typename... TYPES>
struct merge_types;

template <typename... LTypes, typename... RTypes>
struct merge_types<types_pck<LTypes...>, types_pck<RTypes...>>
{
	typedef types_pck<LTypes..., RTypes...> types;
};

template <typename... TYPES>
struct types_to_tuple;

template <typename... TYPES>
struct types_to_tuple<types_pck<TYPES...>>
{
	typedef std::tuple<TYPES...> tuple_type;
};
//////////////////////////////////////////////////////////////////////////

template <size_t I, typename FIRST, typename... TYPES>
struct SingleElement_ : public SingleElement_<I - 1, TYPES...> {};

template <typename FIRST, typename... TYPES>
struct SingleElement_<0, FIRST, TYPES...>
{
	typedef FIRST type;
};

/*!
@brief 取第I个类型
*/
template <size_t I, typename FIRST, typename... TYPES>
struct single_element : public SingleElement_<I, FIRST, TYPES...> {};

template <size_t I, typename... TYPES>
struct single_element<I, types_pck<TYPES...>>: public single_element<I, TYPES...>{};

 template <size_t I, typename... TYPES>
 struct single_element<I, std::tuple<TYPES...>>: public single_element<I, TYPES...>{};
 
 template <size_t I, typename... TYPES>
 struct single_element<I, const std::tuple<TYPES...>>: public single_element<I, TYPES...>{};
//////////////////////////////////////////////////////////////////////////
 
 template <typename... Types>
 struct eliminate_first;

 template <typename First, typename... Types>
 struct eliminate_first<First, Types...>
 {
	 typedef types_pck<Types...> types;
 };

 template <typename... Types>
 struct eliminate_first<types_pck<Types...>>: public eliminate_first<Types...>{};

 template <size_t N, typename... Types>
 struct prefix_types
 {
	 typedef typename single_element<0, Types...>::type _frist;
	 typedef typename eliminate_first<Types...>::types _eliminate_first;
	 typedef typename merge_types<types_pck<_frist>, typename prefix_types<N - 1, _eliminate_first>::types>::types types;
 };

 template <typename... Types>
 struct prefix_types<0, types_pck<Types...>>
 {
	 typedef types_pck<> types;
 };

 template <size_t N, typename... Types>
 struct prefix_types<N, types_pck<Types...>>: public prefix_types<N, Types...>{};
 //////////////////////////////////////////////////////////////////////////

template <typename T>
struct CheckRef0_
{
	typedef T& type;
};

template <typename T>
struct CheckRef0_<T&&>
{
	typedef T&& type;
};

/*!
@brief 尝试移动tuple中的第I个参数
*/
template <size_t I, typename TUPLE>
struct tuple_move
{
	typedef typename single_element<I, RM_REF(TUPLE)>::type type;

	template <typename Tuple>
	static auto get(Tuple&& tup)->typename CheckRef0_<type>::type
	{
		return (typename CheckRef0_<type>::type)std::get<I>(tup);
	}
};

template <typename T>
struct CheckRef1_
{
	typedef const T& type;
};

template <typename T>
struct CheckRef1_<T&&>
{
	typedef const T& type;
};

template <typename T>
struct CheckRef1_<T&>
{
	typedef T& type;
};

template <size_t I, typename TUPLE>
struct tuple_move<I, const TUPLE&>
{
	typedef typename single_element<I, RM_REF(TUPLE)>::type type;

	template <typename Tuple>
	static auto get(Tuple&& tup)->typename CheckRef1_<type>::type
	{
		return (typename CheckRef1_<type>::type)std::get<I>(tup);
	}
};

template <typename T>
struct CheckRef2_
{
	typedef T&& type;
};

template <typename T>
struct CheckRef2_<T&>
{
	typedef T& type;
};

template <size_t I, typename TUPLE>
struct tuple_move<I, TUPLE&&>
{
	typedef typename single_element<I, RM_REF(TUPLE)>::type type;

	template <typename Tuple>
	static auto get(Tuple&& tup)->typename CheckRef2_<type>::type
	{
		return (typename CheckRef2_<type>::type)std::get<I>(tup);
	}
};
 
 template <size_t I, typename TUPLE>
 struct tuple_move<I, const TUPLE&&>: public tuple_move<I, const TUPLE&>{};

template <typename R, size_t N>
struct ApplyArg_
{
	template <typename Handler, typename Tuple, typename... Args>
	static inline R append(Handler& h, Tuple&& tup, Args&&... args)
	{
		return ApplyArg_<R, N - 1>::append(h, std::forward<Tuple>(tup), tuple_move<N - 1, Tuple&&>::get(std::forward<Tuple>(tup)), std::forward<Args>(args)...);
	}
};

template <typename R>
struct ApplyArg_<R, 0>
{
	template <typename Handler, typename Tuple, typename... Args>
	static inline R append(Handler& h, Tuple&&, Args&&... args)
	{
		return h(std::forward<Args>(args)...);
	}
};

template <typename R, typename Handler, typename... TYPES>
struct TupleInvokeWrap_;

template <typename R, typename Handler, typename... TYPES>
struct TupleInvokeWrap_<R, Handler, std::tuple<TYPES...>&&>
{
	template <typename... Args>
	inline R operator ()(Args&&... args) const
	{
		return ApplyArg_<R, sizeof...(TYPES)>::append(_h, std::move(_tuple), std::forward<Args>(args)...);
	}

	Handler& _h;
	std::tuple<TYPES...>& _tuple;
};

template <typename R, typename Handler, typename... TYPES>
struct TupleInvokeWrap_<R, Handler, std::tuple<TYPES...>&>
{
	template <typename... Args>
	inline R operator ()(Args&&... args) const
	{
		return ApplyArg_<R, sizeof...(TYPES)>::append(_h, _tuple, std::forward<Args>(args)...);
	}

	Handler& _h;
	std::tuple<TYPES...>& _tuple;
};

template <typename R, typename Handler, typename... TYPES>
struct TupleInvokeWrap_<R, Handler, const std::tuple<TYPES...>&>
{
	template <typename... Args>
	inline R operator ()(Args&&... args) const
	{
		return ApplyArg_<R, sizeof...(TYPES)>::append(_h, _tuple, std::forward<Args>(args)...);
	}

	Handler& _h;
	const std::tuple<TYPES...>& _tuple;
};

template <typename R, typename Handler, typename... TYPES>
struct TupleInvokeWrap_<R, Handler, const std::tuple<TYPES...>&&>
{
	template <typename... Args>
	inline R operator ()(Args&&... args) const
	{
		return ApplyArg_<R, sizeof...(TYPES)>::append(_h, _tuple, std::forward<Args>(args)...);
	}

	Handler& _h;
	const std::tuple<TYPES...>& _tuple;
};

template <typename R, typename Handler, typename TYPE>
struct TupleInvokeWrap_<R, Handler, TYPE>
{
	template <typename... Args>
	inline R operator ()(Args&&... args) const
	{
		return _h((TYPE)_type, std::forward<Args>(args)...);
	}

	Handler& _h;
	TYPE& _type;
};

template <typename R, typename F, typename C>
struct InvokeWrapObj_
{
	template <typename... Args>
	inline R operator ()(Args&&... args) const
	{
		return (obj->*pf)(std::forward<Args>(args)...);
	}

	F pf;
	C* obj;
};

template <typename R, bool>
struct TupleInvoke_ 
{
	template <typename Handler>
	static inline R tuple_invoke(Handler& h)
	{
		return h();
	}

	template <typename Handler, typename First, typename... Tuple>
	static inline R tuple_invoke(Handler& h, First&& fst, Tuple&&... tups)
	{
		TupleInvokeWrap_<R, Handler, First&&> wrap = { h, fst };
		return tuple_invoke(wrap, std::forward<Tuple>(tups)...);
	}
};

template <typename R>
struct TupleInvoke_<R, true>
{
	template <typename F, typename C, typename... Tuple>
	static inline R tuple_invoke(F pf, C* obj, Tuple&&... tups)
	{
		InvokeWrapObj_<R, F, C> wrap = { pf, obj };
		return TupleInvoke_<R, false>::tuple_invoke(wrap, std::forward<Tuple>(tups)...);
	}
};

template <typename T>
struct CheckClassFunc_
{
	enum { value = false };
};

template <typename R, typename C>
struct CheckClassFunc_<R(C::*)>
{
	enum { value = true };
};

template <typename R, typename C, typename... Types>
struct CheckClassFunc_<R(C::*)(Types...) const>
{
	enum { value = true };
};

/*!
@brief 从tuple中取参数调用某个函数
*/
template <typename R = void, typename Handler, typename Unknown, typename... Tuple>
inline R tuple_invoke(Handler&& h, Unknown&& unkown, Tuple&&... tups)
{
	return TupleInvoke_<R, CheckClassFunc_<RM_REF(Handler)>::value>::tuple_invoke(h, std::forward<Unknown>(unkown), std::forward<Tuple>(tups)...);
}

template <typename R = void, typename Handler>
inline R tuple_invoke(Handler&& h)
{
	static_assert(!CheckClassFunc_<RM_REF(Handler)>::value, "");
	return h();
}

template <typename R, size_t N>
struct ApplyRval_ 
{
	template <typename Handler, typename Arg>
	struct wrap_handler
	{
		template <typename... Args>
		R operator ()(Args&&... args)
		{
			bool rval = 0 != try_move<Arg>::can_move;
			return _h(_arg, rval, std::forward<Args>(args)...);
		}

		Handler& _h;
		Arg& _arg;
	};

	template <typename Handler, typename First, typename... Args>
	static inline R append(Handler& h, First&& fst, Args&&... args)
	{
		wrap_handler<Handler, First&&> wrap = { h, fst };
		return ApplyRval_<R, N - 1>::append(wrap, std::forward<Args>(args)...);
	}
};

template <typename R>
struct ApplyRval_<R, 0>
{
	template <typename Handler>
	static inline R append(Handler& h)
	{
		return h();
	}
};

template <typename R, bool>
struct RvalInvoke_
{
	template <typename Handler, typename... Args>
	static inline R invoke(Handler& h, Args&&... args)
	{
		return ApplyRval_<R, sizeof...(Args)>::append(h, std::forward<Args>(args)...);
	}
};

template <typename R>
struct RvalInvoke_<R, true>
{
	template <typename F, typename C, typename... Args>
	static inline R invoke(F pf, C* obj, Args&&... args)
	{
		InvokeWrapObj_<R, F, C> wrap = { pf, obj };
		return RvalInvoke_<R, false>::invoke(wrap, std::forward<Args>(args)...);
	}
};

/*!
@brief 动态传入当前调用是否是右值
*/
template <typename R = void, typename Handler, typename Unknown, typename... Args>
inline R try_rval_invoke(Handler&& h, Unknown&& unkown, Args&&... args)
{
	return RvalInvoke_<R, CheckClassFunc_<RM_REF(Handler)>::value>::invoke(h, std::forward<Unknown>(unkown), std::forward<Args>(args)...);
}

template <typename R = void, typename Handler>
inline R try_rval_invoke(Handler&& h)
{
	static_assert(!CheckClassFunc_<RM_REF(Handler)>::value, "");
	return h();
}

template <typename... Type>
std::tuple<Type&&...> wrap_tuple(Type&&... args)
{
	return std::tuple<Type&&...>(std::forward<Type>(args)...);
}

template <size_t N, size_t I>
struct SameCopyToTuple_
{
	template <typename Head, typename... Dst, typename... Src>
	static void same_copy_to_tuple(const std::tuple<Dst&...>& dst, Head&& head, Src&&... src)
	{
		std::get<N-I>(dst) = std::forward<Head>(head);
		SameCopyToTuple_<N, I - 1>::same_copy_to_tuple(dst, std::forward<Src>(src)...);
	}
};

template <size_t N>
struct SameCopyToTuple_<N, 0>
{
	template <typename... Dst, typename... Src>
	static void same_copy_to_tuple(const std::tuple<Dst&...>& dst, Src&&... src) {}
};

template <size_t N, size_t I>
struct SameCopyTupleToTuple_
{
	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, const std::tuple<Src...>& src)
	{
		std::get<N - I>(dst) = (typename std::tuple_element<N - I, std::tuple<Src...>>::type)std::get<N - I>(src);
		SameCopyTupleToTuple_<N, I - 1>::same_copy_tuple_to_tuple(dst, src);
	}

	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, std::tuple<Src...>&& src)
	{
		std::get<N - I>(dst) = (typename std::tuple_element<N - I, std::tuple<Src...>>::type&&)std::get<N - I>(src);
		SameCopyTupleToTuple_<N, I - 1>::same_copy_tuple_to_tuple(dst, std::move(src));
	}
};

template <size_t N>
struct SameCopyTupleToTuple_<N, 0>
{
	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, const std::tuple<Src...>& src) {}
};

template <size_t A, size_t B, bool lt>
struct SameCopyToTuple__
{
	template <typename... Dst, typename... Src>
	static void same_copy_to_tuple(const std::tuple<Dst&...>& dst, Src&&... src)
	{
		SameCopyToTuple_<A, A>::same_copy_to_tuple(dst, std::forward<Src>(src)...);
	}
};

template <size_t A, size_t B>
struct SameCopyToTuple__<A, B, false>
{
	template <typename... Dst, typename... Src>
	static void same_copy_to_tuple(const std::tuple<Dst&...>& dst, Src&&... src)
	{
		SameCopyToTuple_<B, B>::same_copy_to_tuple(dst, std::forward<Src>(src)...);
	}
};

template <size_t A, size_t B, bool lt>
struct SameCopyTupleToTuple__
{
	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, const std::tuple<Src...>& src)
	{
		SameCopyTupleToTuple_<A, A>::same_copy_tuple_to_tuple(dst, src);
	}

	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, std::tuple<Src...>&& src)
	{
		SameCopyTupleToTuple_<A, A>::same_copy_tuple_to_tuple(dst, std::move(src));
	}
};

template <size_t A, size_t B>
struct SameCopyTupleToTuple__<A, B, false>
{
	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, const std::tuple<Src...>& src)
	{
		SameCopyTupleToTuple_<B, B>::same_copy_tuple_to_tuple(dst, src);
	}

	template <typename... Dst, typename... Src>
	static void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, std::tuple<Src...>&& src)
	{
		SameCopyTupleToTuple_<B, B>::same_copy_tuple_to_tuple(dst, std::move(src));
	}
};

template <typename... Dst, typename... Src>
void same_copy_to_tuple(const std::tuple<Dst&...>& dst, Src&&... src)
{
	bool const lt = sizeof...(Dst) < sizeof...(Src);
	SameCopyToTuple__<sizeof...(Dst), sizeof...(Src), lt>::same_copy_to_tuple(dst, std::forward<Src>(src)...);
}

template <typename... Dst, typename... Src>
void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, const std::tuple<Src...>& src)
{
	bool const lt = sizeof...(Dst) < sizeof...(Src);
	SameCopyTupleToTuple__<sizeof...(Dst), sizeof...(Src), lt>::same_copy_tuple_to_tuple(dst, src);
}

template <typename... Dst, typename... Src>
void same_copy_tuple_to_tuple(const std::tuple<Dst&...>& dst, std::tuple<Src...>&& src)
{
	bool const lt = sizeof...(Dst) < sizeof...(Src);
	SameCopyTupleToTuple__<sizeof...(Dst), sizeof...(Src), lt>::same_copy_tuple_to_tuple(dst, std::move(src));
}

#endif
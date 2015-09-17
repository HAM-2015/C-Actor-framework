#ifndef __TUPLE_OPTION_H
#define __TUPLE_OPTION_H

#include <tuple>
#include "try_move.h"

template <typename... ARGS>
struct types_pck;

/*!
@brief �ƶ�tuple�еĵ�I������
*/
template <typename TUPLE, size_t I>
struct tuple_move
{
	typedef typename std::tuple_element<I, RM_REF(TUPLE)>::type type;

	static RM_REF(type)& get(TUPLE& tup)
	{
		return std::get<I>(tup);
	}
};

template <typename TUPLE, size_t I>
struct tuple_move<TUPLE&&, I>
{
	typedef typename std::tuple_element<I, RM_REF(TUPLE)>::type type;

	static RM_REF(type) && get(TUPLE&& tup)
	{
		return std::move(std::get<I>(tup));
	}
};

template <typename R, size_t N>
struct ApplyArg_
{
	template <typename H, typename TUPLE, typename... Args>
	static inline R append(H&& h, TUPLE&& tp, Args&&... args)
	{
		return ApplyArg_<R, N - 1>::append(TRY_MOVE(h), TRY_MOVE(tp), tuple_move<TUPLE&&, N - 1>::get(TRY_MOVE(tp)), TRY_MOVE(args)...);
	}

	template <typename F, typename TUPLE, typename... Args>
	static inline R append(RM_REF(F) pf, TUPLE&& tp, Args&&... args)
	{
		return ApplyArg_<R, N - 1>::append(pf, TRY_MOVE(tp), tuple_move<TUPLE&&, N - 1>::get(TRY_MOVE(tp)), TRY_MOVE(args)...);
	}

	template <typename F, typename C, typename TUPLE, typename... Args>
	static inline R append(F pf, C* obj, TUPLE&& tp, Args&&... args)
	{
		return ApplyArg_<R, N - 1>::append(pf, obj, TRY_MOVE(tp), tuple_move<TUPLE&&, N - 1>::get(TRY_MOVE(tp)), TRY_MOVE(args)...);
	}

	template <typename TUPLE, typename FIRST, typename... Args>
	static inline void to_args(TUPLE&& tp, FIRST& fst, Args&... dsts)
	{
		fst = tuple_move<TUPLE&&, std::tuple_size<RM_REF(TUPLE)>::value - N>::get(TRY_MOVE(tp));
		ApplyArg_<R, N - 1>::to_args(TRY_MOVE(tp), dsts...);
	}

	template <typename TUPLE, typename FIRST, typename... Args>
	static inline void to_tuple(TUPLE& dst, FIRST&& fst, Args&&... args)
	{
		std::get<std::tuple_size<TUPLE>::value - N>(dst) = TRY_MOVE(fst);
		ApplyArg_<R, N - 1>::to_tuple(dst, TRY_MOVE(args)...);
	}
};

template <typename R>
struct ApplyArg_<R, 0>
{
	template <typename H, typename TUPLE, typename... Args>
	static inline R append(H&& h, TUPLE&&, Args&&... args)
	{
		return h(TRY_MOVE(args)...);
	}

	template <typename F, typename TUPLE, typename... Args>
	static inline R append(RM_REF(F) pf, TUPLE&&, Args&&... args)
	{
		return pf(TRY_MOVE(args)...);
	}

	template <typename F, typename C, typename TUPLE, typename... Args>
	static inline R append(F pf, C* obj, TUPLE&&, Args&&... args)
	{
		return (obj->*pf)(TRY_MOVE(args)...);
	}

	template <typename TUPLE>
	static inline void to_args(TUPLE&&)
	{

	}

	template <typename TUPLE>
	static inline void to_tuple(TUPLE&)
	{

	}
};

/*!
@brief �Ƚ�����ȡ����ֵ����Сֵ
*/
template <long long A, long long B = 0, long long... NUMS>
struct static_cmp
{
	template <long long A, long long B>
	struct cmp
	{
		template <bool = true>
		struct result
		{
			enum { max = A, min = B };
		};

		template <>
		struct result<false>
		{
			enum { max = B, min = A };
		};

		enum { max = result<(A > B)>::max, min = result<(A > B) >::min };
	};

	enum
	{
		max = cmp<A, static_cmp<B, NUMS...>::max>::max,
		min = cmp<A, static_cmp<B, NUMS...>::min>::min
	};
};

template <long long N>
struct static_cmp<N>
{
	enum { min = N, max = N };
};

/*!
@brief �Ƚ����ʹ�С��ȡ����ֵ����Сֵ
*/
template <typename... TYPES>
struct static_cmp_type_size: public static_cmp<sizeof(TYPES)...>
{
};

/*!
@brief ȡ��I������
*/
template <size_t I, typename FIRST, typename... ARGS>
struct types_element
{
	typedef typename types_element<I - 1, ARGS...>::type type;
};

template <typename FIRST, typename... ARGS>
struct types_element<0, FIRST, ARGS...>
{
	typedef FIRST type;
};

/*!
@brief ȡ��I������
*/
template <size_t I>
struct static_args_get
{
	template <typename Head, typename... Args>
	static inline auto get(Head&&, Args&&... args)->typename types_element<I - 1, Args&&...>::type
	{
		return static_args_get<I - 1>::get(TRY_MOVE(args)...);
	}
};

template <>
struct static_args_get<0>
{
	template <typename Head, typename... Args>
	static inline auto get(Head&& head, Args&&...)->Head&&
	{
		return (Head&&)head;
	}
};

template <size_t N>
struct ReverseArgs_
{
	template <typename Handler, typename Tail>
	struct reverse_depth;

	template <typename Handler, typename Tail>
	struct reverse_depth<Handler, Tail&>
	{
		template <typename... Args>
		inline void operator ()(Args&&... args)
		{
			handler(TRY_MOVE(args)..., tail);
		}

		Handler& handler;
		Tail& tail;
	};

	template <typename Handler, typename Tail>
	struct reverse_depth<Handler, Tail&&>
	{
		template <typename... Args>
		inline void operator ()(Args&&... args)
		{
			handler(TRY_MOVE(args)..., (Tail&&)tail);
		}

		Handler& handler;
		Tail& tail;
	};

	template <typename Handler, typename Head, typename... Args>
	static inline void reverse(Handler& h, Head&& head, Args&&... args)
	{
		reverse_depth<Handler, Head&&> rd = { h, head };
		ReverseArgs_<N - 1>::reverse(rd, TRY_MOVE(args)...);
	}
};

template <>
struct ReverseArgs_<0>
{
	template <typename Handler>
	static inline void reverse(Handler& h)
	{
		h();
	}
};

/*!
@brief �ߵ�����ǰ��˳�����ĳ������
*/
template <typename Handler, typename... Args>
inline void reverse_invoke(Handler& h, Args&&... args)
{
	ReverseArgs_<sizeof...(Args)>::reverse(h, TRY_MOVE(args)...);
}

/*!
@brief ȡ�ұ�N����������ĳ������
*/
template <size_t N>
struct right_invoke
{
	template <size_t I>
	struct right
	{
		template <typename Handler, typename Head, typename... Args>
		static inline void invoke(Handler& h, Head&& head, Args&&... args)
		{
			right<I - 1>::invoke(h, TRY_MOVE(args)...);
		}

	};

	template <>
	struct right<0>
	{
		template <typename Handler, typename... Args>
		static inline void invoke(Handler& h, Args&&... args)
		{
			h(TRY_MOVE(args)...);
		}
	};

	template <typename Handler, typename... Args>
	static inline void invoke(Handler& h, Args&&... args)
	{
		right<sizeof...(Args)-N>::invoke(h, TRY_MOVE(args)...);
	}
};

/*!
@brief ȡ���N����������ĳ������
*/
template <size_t N>
struct left_invoke
{
	template <typename Handler>
	struct reverse;

	template <typename Handler>
	struct left
	{
		template <typename... Args>
		inline void operator ()(Args&&... args)
		{
			reverse_invoke(h, TRY_MOVE(args)...);
		}

		Handler& h;
	};

	template <typename Handler>
	struct reverse
	{
		template <typename... Args>
		inline void operator ()(Args&&... args)
		{
			left<Handler> lh = { h };
			right_invoke<N>::invoke(lh, TRY_MOVE(args)...);
		}

		Handler& h;
	};

	template <typename Handler, typename... Args>
	static inline void invoke(Handler& h, Args&&... args)
	{
		reverse<Handler> rh = { h };
		reverse_invoke(rh, TRY_MOVE(args)...);
	}
};

/*!
@brief ��tuple��ȡ��������ĳ������
*/
template <typename R = void, typename H, typename TUPLE>
inline R tuple_invoke(H&& h, TUPLE&& t)
{
	return ApplyArg_<R, std::tuple_size<RM_REF(TUPLE)>::value>::append(TRY_MOVE(h), TRY_MOVE(t));
}

template <typename R = void, typename F, typename TUPLE>
inline R tuple_invoke(RM_REF(F) pf, TUPLE&& t)
{
	return ApplyArg_<R, std::tuple_size<RM_REF(TUPLE)>::value>::append(pf, TRY_MOVE(t));
}

template <typename R = void, typename F, typename C, typename TUPLE>
inline R tuple_invoke(F pf, C* obj, TUPLE&& t)
{
	return ApplyArg_<R, std::tuple_size<RM_REF(TUPLE)>::value>::append(pf, obj, TRY_MOVE(t));
}

/*!
@brief ��tuple�Ƶ���ɢ������
*/
template <typename TUPLE, typename... Args>
inline void tuple_to_args(TUPLE&& t, Args&... dsts)
{
	static_assert(std::tuple_size<RM_REF(TUPLE)>::value == sizeof...(Args), "src size != dst size");
	ApplyArg_<void, std::tuple_size<RM_REF(TUPLE)>::value>::to_args(TRY_MOVE(t), dsts...);
}

/*!
@brief ����ɢ�����Ƶ�tuple��
*/
template <typename TUPLE, typename... Args>
inline void args_to_tuple(TUPLE& dst, Args&&... args)
{
	static_assert(std::tuple_size<RM_REF(TUPLE)>::value == sizeof...(Args), "src size != dst size");
	ApplyArg_<void, std::tuple_size<RM_REF(TUPLE)>::value>::to_tuple(dst, TRY_MOVE(args)...);
}

/*!
@brief ȡ��i�����������в�������һ����
*/
template <typename First>
inline typename First& args_get(const size_t i, First& fst)
{
	assert(0 == i);
	return fst;
}

template <typename First, typename... Args>
inline typename First& args_get(const size_t i, First& fst, Args&... args)
{
	if (i)
	{
		return args_get(i - 1, args...);
	} 
	else
	{
		return fst;
	}
}

#endif
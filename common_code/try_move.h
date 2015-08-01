#ifndef __TRY_MOVE_H
#define __TRY_MOVE_H

#include <type_traits>
#include <assert.h>

template <typename T>
struct try_move
{
	//不正确参数，无法检测是否可移动

	template <typename Arg>
	static inline Arg&& move(Arg&& p)
	{
		return (Arg&&)p;
	}
};

template <typename T>
struct try_move<const T&>
{
	enum { can_move = false };

	static inline const T& move(const T& p0)
	{
		return p0;
	}
};

template <typename T>
struct try_move<T&>
{
	enum { can_move = false };

	static inline T& move(T& p0)
	{
		return p0;
	}
};

template <typename T>
struct try_move<const T&&>
{
	enum { can_move = false };

	static inline const T& move(const T& p0)
	{
		return p0;
	}
};

template <typename T>
struct try_move<T&&>
{
	enum { can_move = true };

	static inline T&& move(T& p0)
	{
		return (T&&)p0;
	}
};

//移除类型本身的引用属性，然后移除 const
#define RM_CREF(__T__) typename std::remove_const<typename std::remove_reference<__T__>::type>::type
//移除引用属性
#define RM_REF(__T__) typename std::remove_reference<__T__>::type
//移除类型本身的 const 属性
#define RM_CONST(__T__) typename std::remove_const<__T__>::type


//检测一个参数是否是右值传递，是就继续进行右值传递
#define TRY_MOVE(__P__) try_move<decltype(__P__)>::move(__P__)
//检测一个参数是否是右值传递
#define CAN_MOVE(__P__) try_move<decltype(__P__)>::can_move
//const属性强制转换为右值
#define FORCE_MOVE(__P__) (RM_CREF(decltype(__P__))&&)__P__
//////////////////////////////////////////////////////////////////////////

/*!
@brief  检测一个参数是否是为引用，是就转为引用包装，否则尝试右值传递
*/
template <typename ARG>
struct try_ref_move
{
	template <typename Arg>
	static inline Arg&& move(Arg&& p)
	{
		return (Arg&&)p;
	}
};

template <typename ARG>
struct try_ref_move<ARG&>
{
	static inline std::reference_wrapper<ARG> move(ARG& p)
	{
		return std::reference_wrapper<ARG>(p);
	}
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
struct type_move_pipe
{
	typedef T type;
};

template <typename T>
struct type_move_pipe<const T&>
{
	typedef std::reference_wrapper<const T> type;
};

template <typename T>
struct type_move_pipe<T&>
{
	typedef std::reference_wrapper<T> type;
};

template <typename T>
struct type_move_pipe<const T&&>
{
	typedef const T&& type;
};

template <typename T>
struct type_move_pipe<T&&>
{
	typedef T&& type;
};

#define TYPE_PIPE(__T__) typename type_move_pipe<__T__>::type
//////////////////////////////////////////////////////////////////////////
/*!
@brief 对象拷贝链测试
*/
struct copy_chain_test
{
#ifdef _DEBUG
	copy_chain_test()
	:_copied(false) {}

	copy_chain_test(const copy_chain_test& s)
		: _copied(false)
	{
		assert(!s._copied);
		s._copied = true;
	}

	void operator =(const copy_chain_test& s)
	{
		assert(!_copied);
		assert(!s._copied);
		s._copied = true;
	}

	mutable bool _copied;
#endif
};

/*!
@brief 取最大的值
*/
template <long long A, long long B = 0, long long... C>
struct static_get_max
{
	template <long long A, long long B>
	struct cmp 
	{
		template <bool = true>
		struct gt 
		{
			enum { value = A };
		};

		template <>
		struct gt<false>
		{
			enum { value = B };
		};

		enum { value = gt<(A > B)>::value };
	};

	enum { value = cmp<A, static_get_max<B, C...>::value>::value };
};

template <long long A>
struct static_get_max<A>
{
	enum { value = A };
};

/*!
@brief 取最小的值
*/
template <long long A, long long B = 0, long long... C>
struct static_get_min
{
	template <long long A, long long B>
	struct cmp
	{
		template <bool = true>
		struct lt
		{
			enum { value = A };
		};

		template <>
		struct lt<false>
		{
			enum { value = B };
		};

		enum { value = lt<(A < B)> ::value };
	};

	enum { value = cmp<A, static_get_min<B, C...>::value>::value };
};

template <long long A>
struct static_get_min<A>
{
	enum { value = A };
};

#endif
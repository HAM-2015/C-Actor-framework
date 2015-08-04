#ifndef __TRY_MOVE_H
#define __TRY_MOVE_H

#include <type_traits>
#include <assert.h>

template <typename T>
struct try_move
{
	//����ȷ�������޷�����Ƿ���ƶ�

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

//�Ƴ����ͱ�����������ԣ�Ȼ���Ƴ� const
#define RM_CREF(__T__) typename std::remove_const<typename std::remove_reference<__T__>::type>::type
//�Ƴ���������
#define RM_REF(__T__) typename std::remove_reference<__T__>::type
//�Ƴ����ͱ���� const ����
#define RM_CONST(__T__) typename std::remove_const<__T__>::type


//���һ�������Ƿ�����ֵ���ݣ��Ǿͼ���������ֵ����
#define TRY_MOVE(__P__) try_move<decltype(__P__)>::move(__P__)
//���һ�������Ƿ�����ֵ����
#define CAN_MOVE(__P__) try_move<decltype(__P__)>::can_move
//const����ǿ��ת��Ϊ��ֵ
#define FORCE_MOVE(__P__) (RM_CREF(decltype(__P__))&&)__P__
//////////////////////////////////////////////////////////////////////////

/*!
@brief  ���һ�������Ƿ���Ϊ���ã��Ǿ�תΪ���ð�װ����������ֵ����
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
@brief ���󿽱�������
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

#endif
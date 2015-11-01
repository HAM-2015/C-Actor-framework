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

//�Ƴ���������
#define RM_REF(__T__) typename std::remove_reference<__T__>::type
//�Ƴ����ͱ���� const ����
#define RM_CONST(__T__) typename std::remove_const<__T__>::type
//�Ƴ����ͱ�����������ԣ�Ȼ���Ƴ� const
#define RM_CREF(__T__) RM_CONST(RM_REF(__T__))

//�Ƴ�ָ��
#define RM_PTR(__T__) typename std::remove_pointer<__T__>::type
//�Ƴ����ͱ����ָ�룬Ȼ���Ƴ� const
#define RM_CPTR(__T__) RM_CONST(RM_PTR(__T__))
//�Ƴ����ͱ�����������ԣ����Ƴ�ָ�룬Ȼ���Ƴ� const
#define RM_CPTR_REF(__T__) RM_CONST(RM_PTR(RM_REF(__T__)))


//���һ�������Ƿ�����ֵ���ݣ��Ǿͼ���������ֵ����
#define TRY_MOVE(__P__) (decltype(__P__))(__P__)
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

#endif
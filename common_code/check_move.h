#ifndef __CHECK_MOVE_H
#define __CHECK_MOVE_H

#include <type_traits>

template <typename T>
struct check_move
{
	template <typename PT>
	static PT& move(PT& p0)
	{
		return p0;
	}
};

template <typename T>
struct check_move<T&&>
{
	template <typename PT>
	static auto move(PT&& p0)->decltype(std::move(p0))
	{
		return std::move(p0);
	}
};

//��⵱ǰ�����Ƿ�����ֵ���Ǿ�ִ��std::move
#define CHECK_MOVE(__P__) check_move<decltype(__P__)>::move(__P__)

#endif
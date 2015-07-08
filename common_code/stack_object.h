#ifndef __STACK_OBJECT_H
#define __STACK_OBJECT_H

#include "scattered.h"

/*!
@brief ��ջ�Ϸ���һ����ʱ�ռ����ڹ�����ʱ����
*/
template <typename OBJ>
class stack_obj
{
public:
	stack_obj()
	{
		DEBUG_OPERATION(_null = true);
	}

	~stack_obj()
	{
		assert(_null);
	}
private:
	stack_obj(const stack_obj&){};
	void operator =(const stack_obj&){};
	void* operator new(size_t){ return 0; };
	void operator delete(void*){};
public:
	/*!
	@brief ����һ����ʱ����
	*/
	template <typename PT0, typename PT1, typename PT2, typename PT3>
	void create(PT0&& p0, PT1&& p1, PT2&& p2, PT3&& p3)
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)OBJ(TRY_MOVE(p0), TRY_MOVE(p1), TRY_MOVE(p2), TRY_MOVE(p3));
	}

	template <typename PT0, typename PT1, typename PT2>
	void create(PT0&& p0, PT1&& p1, PT2&& p2)
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)OBJ(TRY_MOVE(p0), TRY_MOVE(p1), TRY_MOVE(p2));
	}

	template <typename PT0, typename PT1>
	void create(PT0&& p0, PT1&& p1)
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)OBJ(TRY_MOVE(p0), TRY_MOVE(p1));
	}

	template <typename PT0>
	void create(PT0&& p0)
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)OBJ(TRY_MOVE(p0));
	}

	void create()
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)OBJ();
	}

	/*!
	@brief ������ʱ����
	*/
	void destroy()
	{
		assert(!_null);
		DEBUG_OPERATION(_null = true);
		((OBJ*)_buff)->~OBJ();
	}

	OBJ& get()
	{
		assert(!_null);
		return *(OBJ*)_buff;
	}

	/*!
	@brief ���ø���ʱ���󷽷�
	*/
	OBJ* operator -> () const
	{
		assert(!_null);
		return (OBJ*)_buff;
	}
private:
	unsigned char _buff[sizeof(OBJ)];
	DEBUG_OPERATION(bool _null);
};

#endif
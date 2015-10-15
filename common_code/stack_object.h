#ifndef __STACK_OBJECT_H
#define __STACK_OBJECT_H

#include "scattered.h"

/*!
@brief ��ջ�Ϸ���һ����ʱ�ռ����ڹ�����ʱ����
*/
template <typename OBJ = void, bool AUTO = true>
class stack_obj
{
	typedef TYPE_PIPE(OBJ) type;
public:
	stack_obj()
	{
		_null = true;
	}

	~stack_obj()
	{
		destroy();
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
	template <typename... Args>
	void create(Args&&... args)
	{
		if (!_null)
		{
			free();
		}
		_null = false;
		new(_buff)type(TRY_MOVE(args)...);
	}

	void create()
	{
		if (!_null)
		{
			free();
		}
		_null = false;
		new(_buff)type();
	}

	template <typename PT0>
	void operator =(PT0&& p0)
	{
		if (!_null)
		{
			free();
		}
		_null = false;
		new(_buff)type(TRY_MOVE(p0));
	}

	bool has() const
	{
		return !_null;
	}

	/*!
	@brief ������ʱ����
	*/
	void destroy()
	{
		if (!_null)
		{
			free();
			_null = true;
		}
	}

	RM_REF(OBJ)& get()
	{
		assert(!_null);
		return *(type*)_buff;
	}

	RM_REF(OBJ)& operator *()
	{
		assert(!_null);
		return *(type*)_buff;
	}

	/*!
	@brief ���ø���ʱ���󷽷�
	*/
	RM_REF(OBJ)* operator -> ()
	{
		assert(!_null);
		return &get();
	}
private:
	void free()
	{
		assert(!_null);
		((type*)_buff)->~type();
#ifdef _DEBUG
		memset(_buff, 0xcf, sizeof(_buff));
#endif
	}
private:
	unsigned char _buff[sizeof(type)];
	bool _null;
};

template <typename OBJ>
class stack_obj<OBJ, false>
{
	typedef TYPE_PIPE(OBJ) type;
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
	template <typename... Args>
	void create(Args&&... args)
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)type(TRY_MOVE(args)...);
	}

	void create()
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)type();
	}

	template <typename PT0>
	void operator =(PT0&& p0)
	{
		assert(_null);
		DEBUG_OPERATION(_null = false);
		new(_buff)type(TRY_MOVE(p0));
	}

	/*!
	@brief ������ʱ����
	*/
	void destroy()
	{
		assert(!_null);
		DEBUG_OPERATION(_null = true);
		((type*)_buff)->~type();
#ifdef _DEBUG
		memset(_buff, 0xcf, sizeof(_buff));
#endif
	}

	RM_REF(OBJ)& get()
	{
		assert(!_null);
		return *(type*)_buff;
	}

	RM_REF(OBJ)& operator *()
	{
		assert(!_null);
		return *(type*)_buff;
	}

	/*!
	@brief ���ø���ʱ���󷽷�
	*/
	RM_REF(OBJ)* operator -> ()
	{
		assert(!_null);
		return &get();
	}
private:
	unsigned char _buff[sizeof(type)];
	DEBUG_OPERATION(bool _null);
};

template <bool AUTO>
class stack_obj<void, AUTO>
{
public:
	stack_obj()
	{
		_null = true;
	}

	~stack_obj()
	{
		destroy();
	}

	void create()
	{
		_null = false;
	}

	bool has() const
	{
		return !_null;
	}
	void destroy()
	{
		_null = true;
	}

	void get()
	{
	}

	bool _null;
};

#endif
/*!
 @header     actor_framework.h
 @abstract   �����߼����ƿ��(Actor Model)��ʹ��"Э��(coroutine)"����������boost_1.55�����;
 @discussion һ��Actor����(actor_handle)����һ��shared_strand(����������������������io_service)�����Actor���Թ�ͬ����ͬһ��shared_strand;
             ֧��ǿ�ƽ���������/�ָ�����ʱ����������(��������);
             ��Actor�л���������io_service�н��г�ʱ�������Ĳ��������������㣬������Ӱ������ͬһ��io_service��Actor��Ӧ�ٶ�;
             Ĭ��Actorջ�ռ�64k�ֽڣ�Զ���߳�ջС��ע��ֲ�����ռ�õĿռ��Լ����ò��(ע��ݹ�).
 @copyright  Copyright (c) 2015 HAM, E-Mail:591170887@qq.com
 */

#ifndef __ACTOR_FRAMEWORK_H
#define __ACTOR_FRAMEWORK_H

#include <list>
#include <xutility>
#include <functional>
#include "ios_proxy.h"
#include "shared_strand.h"
#include "ref_ex.h"
#include "function_type.h"
#include "msg_queue.h"
#include "actor_mutex.h"
#include "scattered.h"

class my_actor;
typedef std::shared_ptr<my_actor> actor_handle;//Actor���

class mutex_trig_notifer;
class mutex_trig_handle;
class _actor_mutex;

using namespace std;

//�˺��������Actor�жϱ�ǣ�ʹ��ʱע���߼��ġ������ԡ����ܻᱻ����
#define __yield_interrupt

/*!
@brief ���ڼ����Actor�ڵ��õĺ����Ƿ񴥷���ǿ���˳�
*/
#ifdef _DEBUG
#define BEGIN_CHECK_FORCE_QUIT try {
#define END_CHECK_FORCE_QUIT } catch (my_actor::force_quit_exception&) {assert(false);}
#else
#define BEGIN_CHECK_FORCE_QUIT
#define END_CHECK_FORCE_QUIT
#endif

// Actor��ʹ�ã���ʹ����Actor�������쳣��׽ catch (...) ֮ǰ���ڹ���Actor�˳��쳣�������׳�����Ȼ���ܵ��³������
#define CATCH_ACTOR_QUIT()\
catch (my_actor::force_quit_exception& e)\
{\
	throw e;\
}

//Ĭ�϶�ջ��С64k
#define kB	*1024
#define DEFAULT_STACKSIZE	64 kB

//��� pump_msg �Ƿ��� pump_disconnected_exception �쳣�׳�����Ϊ�� catch �ڲ����ܰ�ȫ�Ľ���coro�л�
#define CATCH_PUMP_DISCONNECTED CATCH_FOR(my_actor::pump_disconnected_exception)

template <typename T0, typename T1 = void, typename T2 = void, typename T3 = void>
struct msg_param
{
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef const_ref_ex<T0, T1, T2, T3> const_ref_type;

	msg_param()
	{

	}

	msg_param(const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		:_res0(p0), _res1(p1), _res2(p2), _res3(p3)
	{

	}

	msg_param(const const_ref_type& rp)
		:_res0(rp._p0), _res1(rp._p1), _res2(rp._p2), _res3(rp._p3)
	{

	}

	msg_param(ref_type& s)
	{
		move_from(s);
	}

	msg_param(msg_param&& s)
	{
		_res0 = std::move(s._res0);
		_res1 = std::move(s._res1);
		_res2 = std::move(s._res2);
		_res3 = std::move(s._res3);
	}

	void operator =(msg_param&& s)
	{
		_res0 = std::move(s._res0);
		_res1 = std::move(s._res1);
		_res2 = std::move(s._res2);
		_res3 = std::move(s._res3);
	}

	void move_out(ref_type& dst)
	{
		dst._p0 = std::move(_res0);
		dst._p1 = std::move(_res1);
		dst._p2 = std::move(_res2);
		dst._p3 = std::move(_res3);
	}

	void save_from(const const_ref_type& rp)
	{
		_res0 = rp._p0;
		_res1 = rp._p1;
		_res2 = rp._p2;
		_res3 = rp._p3;
	}

	void move_from(ref_type& src)
	{
		_res0 = std::move(src._p0);
		_res1 = std::move(src._p1);
		_res2 = std::move(src._p2);
		_res3 = std::move(src._p3);
	}

	T0 _res0;
	T1 _res1;
	T2 _res2;
	T3 _res3;
};

template <typename T0, typename T1, typename T2>
struct msg_param<T0, T1, T2, void>
{
	typedef ref_ex<T0, T1, T2> ref_type;
	typedef const_ref_ex<T0, T1, T2> const_ref_type;

	msg_param()
	{

	}

	msg_param(const T0& p0, const T1& p1, const T2& p2)
		:_res0(p0), _res1(p1), _res2(p2)
	{

	}

	msg_param(const const_ref_type& rp)
		:_res0(rp._p0), _res1(rp._p1), _res2(rp._p2)
	{

	}

	msg_param(msg_param&& s)
	{
		_res0 = std::move(s._res0);
		_res1 = std::move(s._res1);
		_res2 = std::move(s._res2);
	}

	msg_param(ref_type& s)
	{
		move_from(s);
	}

	void operator =(msg_param&& s)
	{
		_res0 = std::move(s._res0);
		_res1 = std::move(s._res1);
		_res2 = std::move(s._res2);
	}

	void move_out(ref_type& dst)
	{
		dst._p0 = std::move(_res0);
		dst._p1 = std::move(_res1);
		dst._p2 = std::move(_res2);
	}

	void save_from(const const_ref_type& rp)
	{
		_res0 = rp._p0;
		_res1 = rp._p1;
		_res2 = rp._p2;
	}

	void move_from(ref_type& src)
	{
		_res0 = std::move(src._p0);
		_res1 = std::move(src._p1);
		_res2 = std::move(src._p2);
	}

	T0 _res0;
	T1 _res1;
	T2 _res2;
};

template <typename T0, typename T1>
struct msg_param<T0, T1, void, void>
{
	typedef ref_ex<T0, T1> ref_type;
	typedef const_ref_ex<T0, T1> const_ref_type;

	msg_param()
	{

	}

	msg_param(const T0& p0, const T1& p1)
		:_res0(p0), _res1(p1)
	{

	}

	msg_param(const const_ref_type& rp)
		:_res0(rp._p0), _res1(rp._p1)
	{

	}

	msg_param(msg_param&& s)
	{
		_res0 = std::move(s._res0);
		_res1 = std::move(s._res1);
	}

	msg_param(ref_type& s)
	{
		move_from(s);
	}

	void operator =(msg_param&& s)
	{
		_res0 = std::move(s._res0);
		_res1 = std::move(s._res1);
	}

	void move_out(ref_type& dst)
	{
		dst._p0 = std::move(_res0);
		dst._p1 = std::move(_res1);
	}

	void save_from(const const_ref_type& rp)
	{
		_res0 = rp._p0;
		_res1 = rp._p1;
	}

	void move_from(ref_type& src)
	{
		_res0 = std::move(src._p0);
		_res1 = std::move(src._p1);
	}

	T0 _res0;
	T1 _res1;
};

template <typename T0>
struct msg_param<T0, void, void, void>
{
	typedef ref_ex<T0> ref_type;
	typedef const_ref_ex<T0> const_ref_type;

	msg_param()
	{

	}

	msg_param(const T0& p0)
		:_res0(p0)
	{

	}

	msg_param(const const_ref_type& rp)
		:_res0(rp._p0)
	{

	}

	msg_param(msg_param&& s)
	{
		_res0 = std::move(s._res0);
	}

	msg_param(ref_type& s)
	{
		move_from(s);
	}

	void operator =(msg_param&& s)
	{
		_res0 = std::move(s._res0);
	}

	void move_out(ref_type& dst)
	{
		dst._p0 = std::move(_res0);
	}

	void save_from(const const_ref_type& rp)
	{
		_res0 = rp._p0;
	}

	void move_from(ref_type& src)
	{
		_res0 = std::move(src._p0);
	}

	T0 _res0;
};
//////////////////////////////////////////////////////////////////////////

template <typename T0, typename T1 = void, typename T2 = void, typename T3 = void>
struct dst_receiver_base
{
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef msg_param<T0, T1, T2, T3> param_type;
	virtual void move_from(ref_type& s) = 0;
	virtual void move_from(param_type& s) = 0;
};

template <typename T0, typename T1 = void, typename T2 = void, typename T3 = void>
struct dst_receiver_ref : public dst_receiver_base<T0, T1, T2, T3>
{
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef msg_param<T0, T1, T2, T3> param_type;

	dst_receiver_ref(ref_type& dst)
		:_dstRef(dst){}

	void move_from(ref_type& s)
	{
		_dstRef.move_from(s);
	}

	void move_from(param_type& s)
	{
		s.move_out(_dstRef);
	}

	ref_type& _dstRef;
};

template <typename T0, typename T1 = void, typename T2 = void, typename T3 = void>
struct dst_receiver_buff : public dst_receiver_base<T0, T1, T2, T3>
{
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef msg_param<T0, T1, T2, T3> param_type;

	struct dst_buff 
	{
		dst_buff()
		:_has(false){}

		~dst_buff()
		{
			if (_has)
			{
				((param_type*)_buff)->~param_type();
			}
		}

		param_type& get()
		{
			assert(_has);
			return *(param_type*)_buff;
		}

		bool _has;
		BYTE _buff[sizeof(param_type)];
	};

	dst_receiver_buff(dst_buff& dstBuff)
		:_dstBuff(dstBuff){}

	void move_from(ref_type& s)
	{
		assert(!_dstBuff._has);
		_dstBuff._has = true;
		new(_dstBuff._buff)param_type(s);
	}

	void move_from(param_type& s)
	{
		assert(!_dstBuff._has);
		_dstBuff._has = true;
		new(_dstBuff._buff)param_type(std::move(s));
	}

	dst_buff& _dstBuff;
};

//////////////////////////////////////////////////////////////////////////

class actor_msg_handle_base
{
protected:
	actor_msg_handle_base();
	virtual ~actor_msg_handle_base(){};
public:
	virtual void close() = 0;
protected:
	void run_one();
	bool is_quited();
	void set_actor(const actor_handle& hostActor);
	static std::shared_ptr<bool> new_bool();
protected:
	bool _waiting;
	my_actor* _hostActor;
	std::shared_ptr<bool> _closed;
	DEBUG_OPERATION(shared_strand _strand);
};

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class actor_msg_handle;

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class actor_trig_handle;

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class actor_msg_notifer
{
	typedef actor_msg_handle<T0, T1, T2, T3> msg_handle;

	friend msg_handle;
public:
	actor_msg_notifer()
		:_msgHandle(NULL){}
private:
	actor_msg_notifer(msg_handle* msgHandle)
		:_msgHandle(msgHandle),
		_hostActor(_msgHandle->_hostActor->shared_from_this()),
		_closed(msgHandle->_closed)
	{
		assert(msgHandle->_strand == _hostActor->self_strand());
	}
public:
	template <typename PT0, typename PT1, typename PT2, typename PT3>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2, const PT3& p3) const
	{
		auto& msgHandle_ = _msgHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				msgHandle_->push_msg(ref_ex<PT0, PT1, PT2, PT3>((PT0&)p0, (PT1&)p1, (PT2&)p2, (PT3&)p3));
			}
		});
	}

	template <typename PT0, typename PT1, typename PT2>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2) const
	{
		auto& msgHandle_ = _msgHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				msgHandle_->push_msg(ref_ex<PT0, PT1, PT2>((PT0&)p0, (PT1&)p1, (PT2&)p2));
			}
		});
	}

	template <typename PT0, typename PT1>
	void operator()(const PT0& p0, const PT1& p1) const
	{
		auto& msgHandle_ = _msgHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				msgHandle_->push_msg(ref_ex<PT0, PT1>((PT0&)p0, (PT1&)p1));
			}
		});
	}

	template <typename PT0>
	void operator()(const PT0& p0) const
	{
		auto& msgHandle_ = _msgHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				msgHandle_->push_msg(ref_ex<PT0>((PT0&)p0));
			}
		});
	}

	void operator()() const
	{
		auto& msgHandle_ = _msgHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				msgHandle_->push_msg();
			}
		});
	}

	typename func_type<T0, T1, T2, T3>::result case_func()
	{
		return typename func_type<T0, T1, T2, T3>::result(*this);
	}

	actor_handle host_actor() const
	{
		return _hostActor;
	}

	bool empty() const
	{
		return !_msgHandle;
	}

	void clear()
	{
		_msgHandle = NULL;
		_hostActor.reset();
		_closed.reset();
	}

	operator bool() const
	{
		return !empty();
	}
private:
	msg_handle* _msgHandle;
	actor_handle _hostActor;
	std::shared_ptr<bool> _closed;
};

template <typename T0, typename T1, typename T2, typename T3>
class actor_msg_handle: public actor_msg_handle_base
{
	typedef msg_param<T0, T1, T2, T3> msg_type;
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef dst_receiver_base<T0, T1, T2, T3> dst_receiver;
	typedef actor_msg_notifer<T0, T1, T2, T3> msg_notifer;

	friend msg_notifer;
	friend my_actor;
public:
	actor_msg_handle(size_t fixedSize = 16)
		:_msgBuff(fixedSize), _dstRec(NULL) {}

	~actor_msg_handle()
	{
		close();
	}
private:
	msg_notifer make_notifer(const actor_handle& hostActor)
	{
		close();
		set_actor(hostActor);
		_closed = new_bool();
		_waiting = false;
		return msg_notifer(this);
	}

	void push_msg(ref_type& msg)
	{
		assert(_strand->running_in_this_thread());
		if (!is_quited())
		{
			if (_waiting)
			{
				_waiting = false;
				assert(_msgBuff.empty());
				assert(_dstRec);
				_dstRec->move_from(msg);
				_dstRec = NULL;
				run_one();
				return;
			}
			_msgBuff.push_back(std::move(msg_type(msg)));
		}
	}

	bool read_msg(dst_receiver& dst)
	{
		assert(_strand->running_in_this_thread());
		if (!_msgBuff.empty())
		{
			dst.move_from(_msgBuff.front());
			_msgBuff.pop_front();
			return true;
		}
		_dstRec = &dst;
		_waiting = true;
		return false;
	}

	void close()
	{
		if (_closed)
		{
			assert(_strand->running_in_this_thread());
			*_closed = true;
			_closed.reset();
		}
		_dstRec = NULL;
		_waiting = false;
		_msgBuff.clear();
		_hostActor = NULL;
	}

	size_t size()
	{
		assert(_strand->running_in_this_thread());
		return _msgBuff.size();
	}
private:
	dst_receiver* _dstRec;
	msg_queue<msg_type> _msgBuff;
};


template <>
class actor_msg_handle<void, void, void, void> : public actor_msg_handle_base
{
	typedef actor_msg_notifer<> msg_notifer;

	friend msg_notifer;
	friend my_actor;
public:
	~actor_msg_handle()
	{
		close();
	}
private:
	msg_notifer make_notifer(const actor_handle& hostActor)
	{
		close();
		set_actor(hostActor);
		_closed = new_bool();
		_waiting = false;
		return msg_notifer(this);
	}

	void push_msg()
	{
		assert(_strand->running_in_this_thread());
		if (!is_quited())
		{
			if (_waiting)
			{
				_waiting = false;
				run_one();
				return;
			}
			_msgCount++;
		}
	}

	bool read_msg()
	{
		assert(_strand->running_in_this_thread());
		if (_msgCount)
		{
			_msgCount--;
			return true;
		}
		_waiting = true;
		return false;
	}

	void close()
	{
		if (_closed)
		{
			assert(_strand->running_in_this_thread());
			*_closed = true;
			_closed.reset();
		}
		_msgCount = 0;
		_waiting = false;
		_hostActor = NULL;
	}

	size_t size()
	{
		assert(_strand->running_in_this_thread());
		return _msgCount;
	}
private:
	size_t _msgCount;
};
//////////////////////////////////////////////////////////////////////////

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class actor_trig_notifer
{
	typedef actor_trig_handle<T0, T1, T2, T3> trig_handle;

	friend trig_handle;
public:
	actor_trig_notifer()
		:_trigHandle(NULL){}
private:
	actor_trig_notifer(trig_handle* trigHandle)
		:_trigHandle(trigHandle),
		_hostActor(trigHandle->_hostActor->shared_from_this()),
		_closed(trigHandle->_closed)
	{
		assert(_trigHandle->_strand == _hostActor->self_strand());
	}
public:
	template <typename PT0, typename PT1, typename PT2, typename PT3>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2, const PT3& p3) const
	{
		auto& trigHandle_ = _trigHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_strand->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				trigHandle_->push_msg(ref_ex<PT0, PT1, PT2, PT3>((PT0&)p0, (PT1&)p1, (PT2&)p2, (PT3&)p3));
			}
		});
	}

	template <typename PT0, typename PT1, typename PT2>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2) const
	{
		auto& trigHandle_ = _trigHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				trigHandle_->push_msg(ref_ex<PT0, PT1, PT2>((PT0&)p0, (PT1&)p1, (PT2&)p2));
			}
		});
	}

	template <typename PT0, typename PT1>
	void operator()(const PT0& p0, const PT1& p1) const
	{
		auto& trigHandle_ = _trigHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				trigHandle_->push_msg(ref_ex<PT0, PT1>((PT0&)p0, (PT1&)p1));
			}
		});
	}

	template <typename PT0>
	void operator()(const PT0& p0) const
	{
		auto& trigHandle_ = _trigHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				trigHandle_->push_msg(ref_ex<PT0>((PT0&)p0));
			}
		});
	}

	void operator()() const
	{
		auto& trigHandle_ = _trigHandle;
		auto& hostActor_ = _hostActor;
		auto& closed_ = _closed;
		_hostActor->self_strand()->post([=]
		{
			if (!(*closed_))
			{
				auto lockActor = hostActor_;
				trigHandle_->push_msg();
			}
		});
	}

	typename func_type<T0, T1, T2, T3>::result case_func()
	{
		return typename func_type<T0, T1, T2, T3>::result(*this);
	}

	bool empty() const
	{
		return !_trigHandle;
	}

	void clear()
	{
		_trigHandle = NULL;
		_hostActor.reset();
		_closed.reset();
	}

	operator bool() const
	{
		return !empty();
	}
private:
	trig_handle* _trigHandle;
	actor_handle _hostActor;
	std::shared_ptr<bool> _closed;
};

template <typename T0, typename T1, typename T2, typename T3>
class actor_trig_handle : public actor_msg_handle_base
{
	typedef msg_param<T0, T1, T2, T3> msg_type;
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef dst_receiver_base<T0, T1, T2, T3> dst_receiver;
	typedef actor_trig_notifer<T0, T1, T2, T3> msg_notifer;

	friend msg_notifer;
	friend my_actor;
public:
	actor_trig_handle()
		:_hasMsg(false), _dstRec(NULL) {}

	~actor_trig_handle()
	{
		close();
	}
private:
	msg_notifer make_notifer(const actor_handle& hostActor)
	{
		close();
		set_actor(hostActor);
		_closed = new_bool();
		_waiting = false;
		_hasMsg = false;
		return msg_notifer(this);
	}

	void push_msg(ref_type& msg)
	{
		assert(_strand->running_in_this_thread());
		*_closed = true;
		if (!is_quited())
		{
			if (_waiting)
			{
				_waiting = false;
				assert(_dstRec);
				_dstRec->move_from(msg);
				_dstRec = NULL;
				run_one();
				return;
			}
			_hasMsg = true;
			new (_msgBuff)msg_type(msg);
		}
	}

	bool read_msg(dst_receiver& dst)
	{
		assert(_strand->running_in_this_thread());
		if (_hasMsg)
		{
			_hasMsg = false;
			dst.move_from(*(msg_type*)_msgBuff);
			((msg_type*)_msgBuff)->~msg_type();
			return true;
		}
		_dstRec = &dst;
		_waiting = true;
		return false;
	}

	void close()
	{
		if (_closed)
		{
			assert(_strand->running_in_this_thread());
			*_closed = true;
			_closed.reset();
		}
		if (_hasMsg)
		{
			_hasMsg = false;
			((msg_type*)_msgBuff)->~msg_type();
		}
		_dstRec = NULL;
		_waiting = false;
		_hostActor = NULL;
	}
public:
	bool has()
	{
		assert(_strand->running_in_this_thread());
		return _hasMsg;
	}
private:
	dst_receiver* _dstRec;
	bool _hasMsg;
	BYTE _msgBuff[sizeof(msg_type)];
};

template <>
class actor_trig_handle<void, void, void, void> : public actor_msg_handle_base
{
	typedef actor_trig_notifer<> msg_notifer;

	friend msg_notifer;
	friend my_actor;
public:
	actor_trig_handle()
		:_hasMsg(false){}

	~actor_trig_handle()
	{
		close();
	}
private:
	msg_notifer make_notifer(const actor_handle& hostActor)
	{
		close();
		set_actor(hostActor);
		_closed = new_bool();
		_waiting = false;
		_hasMsg = false;
		return msg_notifer(this);
	}

	void push_msg()
	{
		assert(_strand->running_in_this_thread());
		*_closed = true;
		if (!is_quited())
		{
			if (_waiting)
			{
				_waiting = false;
				run_one();
				return;
			}
			_hasMsg = true;
		}
	}

	bool read_msg()
	{
		assert(_strand->running_in_this_thread());
		if (_hasMsg)
		{
			_hasMsg = false;
			return true;
		}
		_waiting = true;
		return false;
	}

	void close()
	{
		if (_closed)
		{
			assert(_strand->running_in_this_thread());
			*_closed = true;
			_closed.reset();
		}
		_hasMsg = false;
		_waiting = false;
		_hostActor = NULL;
	}
public:
	bool has()
	{
		assert(_strand->running_in_this_thread());
		return _hasMsg;
	}
private:
	bool _hasMsg;
};

//////////////////////////////////////////////////////////////////////////
template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class msg_pump;

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class msg_pool;

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class post_actor_msg;

class msg_pump_base
{
	friend my_actor;
public:
	virtual ~msg_pump_base() {};
	virtual void clear() = 0;
	virtual void close() = 0;
protected:
	bool is_quited();
	void run_one();
protected:
	my_actor* _hostActor;
};

class msg_pool_base
{
	friend msg_pump<>;
	friend my_actor;
public:
	virtual ~msg_pool_base() {};
};

template <typename T0, typename T1, typename T2, typename T3>
class msg_pump : public msg_pump_base
{
	typedef msg_param<T0, T1, T2, T3> msg_type;
	typedef const_ref_ex<T0, T1, T2, T3> const_ref_type;
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef msg_pool<T0, T1, T2, T3> msg_pool_type;
	typedef dst_receiver_base<T0, T1, T2, T3> dst_receiver;
	typedef typename msg_pool_type::pump_handler pump_handler;

	friend my_actor;
	friend msg_pool<T0, T1, T2, T3>;
	friend pump_handler;
public:
	typedef msg_pump* handle;
private:
	msg_pump(){}
	~msg_pump(){}
private:
	static std::shared_ptr<msg_pump> make(const actor_handle& hostActor)
	{
		std::shared_ptr<msg_pump> res(new msg_pump(), [](msg_pump* p){delete p; });
		res->_weakThis = res;
		res->_hasMsg = false;
		res->_waiting = false;
		res->_checkDis = false;
		res->_pumpCount = 0;
		res->_dstRec = NULL;
		res->_hostActor = hostActor.get();
		res->_strand = hostActor->self_strand();
		return res;
	}

	void receiver(msg_type&& msg)
	{
		if (!is_quited())
		{
			assert(!_hasMsg);
			_pumpCount++;
			if (_dstRec)
			{
				_dstRec->move_from(msg);
				_dstRec = NULL;
				if (_waiting)
				{
					_waiting = false;
					_checkDis = false;
					run_one();
				}
			}
			else
			{//pump_msg��ʱ������Ž��ܵ���Ϣ
				assert(!_waiting);
				_hasMsg = true;
				new (_msgSpace)msg_type(std::move(msg));
			}
		}
	}

	void receive_msg_post(msg_type&& msg, const actor_handle& hostActor)
	{
		auto shared_this = _weakThis.lock();
		_strand->post([=]
		{
			actor_handle lockActor = hostActor;
			shared_this->receiver(std::move((msg_type&)msg));
		});
	}

	void receive_msg(msg_type&& msg, const actor_handle& hostActor)
	{
		if (_strand->running_in_this_thread())
		{
			receiver(std::move((msg_type&)msg));
		} 
		else
		{
			receive_msg_post(std::move(msg), hostActor);
		}
	}

	bool read_msg(dst_receiver& dst)
	{
		assert(_strand->running_in_this_thread());
		assert(!_dstRec);
		assert(!_waiting);
		if (_hasMsg)
		{
			_hasMsg = false;
			dst.move_from(*(msg_type*)_msgSpace);
			((msg_type*)_msgSpace)->~msg_type();
			return true;
		}
		_dstRec = &dst;
		if (!_pumpHandler.empty())
		{
			_pumpHandler(_pumpCount);
			_waiting = !!_dstRec;
			return !_dstRec;
		}
		_waiting = true;
		return false;
	}

	void connect(const pump_handler& pumpHandler)
	{
		assert(_strand->running_in_this_thread());
		assert(_hostActor);
		_pumpHandler = pumpHandler;
		_pumpCount = 0;
		if (_waiting)
		{
			_pumpHandler.post_pump(_pumpCount);
		}
	}

	void clear()
	{
		assert(_strand->running_in_this_thread());
		assert(_hostActor);
		_pumpHandler.clear();
		if (!is_quited() && _checkDis)
		{
			assert(_waiting);
			_waiting = false;
			_dstRec = NULL;
			run_one();
		}
	}

	void close()
	{
		if (_hasMsg)
		{
			((msg_type*)_msgSpace)->~msg_type();
		}
		_hasMsg = false;
		_dstRec = NULL;
		_pumpCount = 0;
		_waiting = false;
		_checkDis = false;
		_pumpHandler.clear();
		_hostActor = NULL;
	}

	bool isDisconnected()
	{
		return _pumpHandler.empty();
	}
private:
	std::weak_ptr<msg_pump> _weakThis;
	BYTE _msgSpace[sizeof(msg_type)];
	pump_handler _pumpHandler;
	shared_strand _strand;
	dst_receiver* _dstRec;
	BYTE _pumpCount;
	bool _hasMsg;
	bool _waiting;
	bool _checkDis;
};

template <typename T0, typename T1, typename T2, typename T3>
class msg_pool : public msg_pool_base
{
	typedef msg_param<T0, T1, T2, T3> msg_type;
	typedef const_ref_ex<T0, T1, T2, T3> const_ref_type;
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef msg_pump<T0, T1, T2, T3> msg_pump_type;
	typedef post_actor_msg<T0, T1, T2, T3> post_type;

	struct pump_handler
	{
		void operator()(BYTE pumpID)
		{
			assert(_thisPool);
			if (_thisPool->_strand->running_in_this_thread())
			{
				if (_msgPump == _thisPool->_msgPump)
				{
					auto& msgBuff = _thisPool->_msgBuff;
					if (pumpID == _thisPool->_sendCount)
					{
						if (!msgBuff.empty())
						{
							msg_type mt_ = std::move(msgBuff.front());
							msgBuff.pop_front();
							_thisPool->_sendCount++;
							_thisPool->_msgPump->receive_msg(std::move(mt_), _msgPump->_hostActor->shared_from_this());
						}
						else
						{
							_thisPool->_waiting = true;
						}
					}
					else
					{//�ϴ���Ϣûȡ��������ȡ����ʵ���м��Ѿ�post��ȥ��
						assert(!_thisPool->_waiting);
						assert(pumpID + 1 == _thisPool->_sendCount);
					}
				}
			}
			else
			{
				post_pump(pumpID);
			}
		}

		void post_pump(BYTE pumpID)
		{
			assert(!empty());
			auto& refThis_ = *this;
			auto hostActor = _msgPump->_hostActor->shared_from_this();
			_thisPool->_strand->post([=]
			{
				actor_handle lockActor = hostActor;
				((pump_handler&)refThis_)(pumpID);
			});
		}

		bool empty()
		{
			return !_thisPool;
		}

		void clear()
		{
			_thisPool.reset();
			_msgPump.reset();
		}

		std::shared_ptr<msg_pool> _thisPool;
		std::shared_ptr<msg_pump_type> _msgPump;
	};

	friend my_actor;
	friend msg_pump_type;
	friend post_type;
private:
	msg_pool(size_t fixedSize)
		:_msgBuff(fixedSize)
	{

	}
	~msg_pool()
	{

	}
private:
	static std::shared_ptr<msg_pool> make(shared_strand strand, size_t fixedSize)
	{
		std::shared_ptr<msg_pool> res(new msg_pool(fixedSize), [](msg_pool* p){delete p; });
		res->_weakThis = res;
		res->_strand = strand;
		res->_waiting = false;
		res->_sendCount = 0;
		return res;
	}

	void send_msg(msg_type&& mt, const actor_handle& hostActor)
	{
		if (_waiting)
		{
			_waiting = false;
			assert(_msgPump);
			_sendCount++;
			if (_msgBuff.empty())
			{
				_msgPump->receive_msg(std::move(mt), hostActor);
			}
			else
			{
				_msgBuff.push_back(std::move(mt));
				msg_type mt_ = std::move(_msgBuff.front());
				_msgBuff.pop_front();
				_msgPump->receive_msg(std::move(mt_), hostActor);
			}
		}
		else
		{
			_msgBuff.push_back(std::move(mt));
		}
	}

	void post_msg(msg_type&& mt, const actor_handle& hostActor)
	{
		if (_waiting)
		{
			_waiting = false;
			assert(_msgPump);
			_sendCount++;
			if (_msgBuff.empty())
			{
				_msgPump->receive_msg_post(std::move(mt), hostActor);
			}
			else
			{
				_msgBuff.push_back(std::move(mt));
				_msgPump->receive_msg_post(std::move(_msgBuff.front()), hostActor);
				_msgBuff.pop_front();
			}
		}
		else
		{
			_msgBuff.push_back(std::move(mt));
		}
	}

	void push_msg(msg_type&& mt, const actor_handle& hostActor)
	{
		if (_strand->running_in_this_thread())
		{
			post_msg(std::move(mt), hostActor);
		}
		else
		{
			auto shared_this = _weakThis.lock();
			_strand->post([=]
			{
				shared_this->send_msg(std::move((msg_type&)mt), hostActor);
			});
		}
	}

	pump_handler connect_pump(const std::shared_ptr<msg_pump_type>& msgPump)
	{
		assert(msgPump);
		assert(_strand->running_in_this_thread());
		_msgPump = msgPump;
		pump_handler compHandler;
		compHandler._thisPool = _weakThis.lock();
		compHandler._msgPump = msgPump;
		_sendCount = 0;
		_waiting = false;
		return compHandler;
	}

	void disconnect()
	{
		assert(_strand->running_in_this_thread());
		_msgPump.reset();
		_waiting = false;
	}

	void expand_fixed(size_t fixedSize)
	{
		assert(_strand->running_in_this_thread());
		_msgBuff.expand_fixed(fixedSize);
	}
private:
	std::weak_ptr<msg_pool> _weakThis;
	std::shared_ptr<msg_pump_type> _msgPump;
	msg_queue<msg_type> _msgBuff;
	shared_strand _strand;
	BYTE _sendCount;
	bool _waiting;
};

class msg_pump_void;

class msg_pool_void : public msg_pool_base
{
	typedef post_actor_msg<> post_type;
	typedef msg_pump_void msg_pump_type;

	struct pump_handler
	{
		void operator()(BYTE pumpID);
		void post_pump(BYTE pumpID);
		bool empty();
		bool same_strand();
		void clear();

		std::shared_ptr<msg_pool_void> _thisPool;
		std::shared_ptr<msg_pump_type> _msgPump;
	};

	friend my_actor;
	friend msg_pump_void;
	friend post_type;
protected:
	msg_pool_void(shared_strand strand);
public:
	virtual ~msg_pool_void();
protected:
	void send_msg(const actor_handle& hostActor);
	void post_msg(const actor_handle& hostActor);
	void push_msg(const actor_handle& hostActor);
	pump_handler connect_pump(const std::shared_ptr<msg_pump_type>& msgPump);
	void disconnect();
	void expand_fixed(size_t fixedSize){};
protected:
	std::weak_ptr<msg_pool_void> _weakThis;
	std::shared_ptr<msg_pump_type> _msgPump;
	size_t _msgBuff;
	shared_strand _strand;
	BYTE _sendCount;
	bool _waiting;
};

class msg_pump_void : public msg_pump_base
{
	typedef msg_pool_void msg_pool_type;
	typedef msg_pool_void::pump_handler pump_handler;

	friend my_actor;
	friend msg_pool_void;
	friend pump_handler;
protected:
	msg_pump_void(const actor_handle& hostActor);
public:
	virtual ~msg_pump_void();
protected:
	void receiver();
	void receive_msg_post(const actor_handle& hostActor);
	void receive_msg(const actor_handle& hostActor);
	bool read_msg();
	void connect(const pump_handler& pumpHandler);
	void clear();
	void close();
	bool isDisconnected();
protected:
	std::weak_ptr<msg_pump_void> _weakThis;
	pump_handler _pumpHandler;
	shared_strand _strand;
	BYTE _pumpCount;
	bool _waiting;
	bool _hasMsg;
	bool _checkDis;
};

template <>
class msg_pool<void, void, void, void> : public msg_pool_void
{
	friend my_actor;
private:
	typedef std::shared_ptr<msg_pool> handle;

	msg_pool(shared_strand strand)
		:msg_pool_void(strand){}

	~msg_pool(){}

	static handle make(shared_strand strand, size_t fixedSize)
	{
		handle res(new msg_pool(strand), [](msg_pool* p){delete p; });
		res->_weakThis = res;
		return res;
	}
};

template <>
class msg_pump<void, void, void, void> : public msg_pump_void
{
	friend my_actor;
public:
	typedef msg_pump* handle;
private:
	msg_pump(const actor_handle& hostActor)
		:msg_pump_void(hostActor){}

	~msg_pump(){}

	static std::shared_ptr<msg_pump> make(const actor_handle& hostActor)
	{
		std::shared_ptr<msg_pump> res(new msg_pump(hostActor), [](msg_pump* p){delete p; });
		res->_weakThis = res;
		return res;
	}
};

template <typename T0, typename T1, typename T2, typename T3>
class post_actor_msg
{
	typedef msg_pool<T0, T1, T2, T3> msg_pool_type;
public:
	post_actor_msg(){}
	post_actor_msg(const std::shared_ptr<msg_pool_type>& msgPool, const actor_handle& hostActor)
		:_msgPool(msgPool), _hostActor(hostActor){}
public:
	template <typename PT0, typename PT1, typename PT2, typename PT3>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2, const PT3& p3) const
	{
		assert(!empty());
		_msgPool->push_msg(std::move(msg_param<PT0, PT1, PT2, PT3>(p0, p1, p2, p3)), _hostActor);
	}

	template <typename PT0, typename PT1, typename PT2>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2) const
	{
		assert(!empty());
		_msgPool->push_msg(std::move(msg_param<PT0, PT1, PT2>(p0, p1, p2)), _hostActor);
	}

	template <typename PT0, typename PT1>
	void operator()(const PT0& p0, const PT1& p1) const
	{
		assert(!empty());
		_msgPool->push_msg(std::move(msg_param<PT0, PT1>(p0, p1)), _hostActor);
	}

	template <typename PT0>
	void operator()(const PT0& p0) const
	{
		assert(!empty());
		_msgPool->push_msg(std::move(msg_param<PT0>(p0)), _hostActor);
	}

	void operator()() const
	{
		assert(!empty());
		_msgPool->push_msg(_hostActor);
	}

	typename func_type<T0, T1, T2, T3>::result case_func()
	{
		return typename func_type<T0, T1, T2, T3>::result(*this);
	}

	bool empty() const
	{
		return !_msgPool;
	}

	void clear()
	{
		_hostActor.reset();
		_msgPool.reset();
	}

	operator bool() const
	{
		return !empty();
	}
private:
	actor_handle _hostActor;
	std::shared_ptr<msg_pool_type> _msgPool;
};
//////////////////////////////////////////////////////////////////////////

class trig_once_base
{
protected:
	DEBUG_OPERATION(trig_once_base() :_pIsTrig(new boost::atomic<bool>(false)){})
public:
	virtual ~trig_once_base(){};
protected:
	template <typename DST /*dst_receiver*/, typename SRC /*msg_param*/>
	void _trig_handler(DST& dstRec, SRC&& src) const
	{
#ifdef _DEBUG
		if (!_pIsTrig->exchange(true))
		{
			assert(_hostActor);
			_hostActor->_trig_handler(dstRec, std::move(src));
		}
		else
		{
			assert(false);
		}
#else
		assert(_hostActor);
		_hostActor->_trig_handler(dstRec, std::move(src));
#endif
	}

	void trig_handler() const;
protected:
	actor_handle _hostActor;
	DEBUG_OPERATION(std::shared_ptr<boost::atomic<bool> > _pIsTrig);
};

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class trig_once_notifer: public trig_once_base
{
	typedef dst_receiver_base<T0, T1, T2, T3> dst_receiver;

	friend my_actor;
public:
	trig_once_notifer() :_dstRec(0) {};
private:
	trig_once_notifer(const actor_handle& hostActor, dst_receiver* dstRec)
		:_dstRec(dstRec) {
		_hostActor = hostActor;
	}
public:
	template <typename PT0, typename PT1, typename PT2, typename PT3>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2, const PT3& p3) const
	{
		_trig_handler(*_dstRec, std::move(msg_param<PT0, PT1, PT2, PT3>(p0, p1, p2, p3)));
	}

	template <typename PT0, typename PT1, typename PT2>
	void operator()(const PT0& p0, const PT1& p1, const PT2& p2) const
	{
		_trig_handler(*_dstRec, std::move(msg_param<PT0, PT1, PT2>(p0, p1, p2)));
	}

	template <typename PT0, typename PT1>
	void operator()(const PT0& p0, const PT1& p1) const
	{
		_trig_handler(*_dstRec, std::move(msg_param<PT0, PT1>(p0, p1)));
	}

	template <typename PT0>
	void operator()(const PT0& p0) const
	{
		_trig_handler(*_dstRec, std::move(msg_param<PT0>(p0)));
	}

	void operator()() const
	{
		trig_handler();
	}
private:
	dst_receiver* _dstRec;
};

//////////////////////////////////////////////////////////////////////////
/*!
@brief ��Actor��������ɿ���
*/
class child_actor_handle 
{
public:
	typedef std::shared_ptr<child_actor_handle> ptr;
private:
	friend my_actor;
	/*!
	@brief ��Actor���������child_actor_handle��ʹ��
	*/
	struct child_actor_param
	{
#ifdef _DEBUG
		child_actor_param();
		child_actor_param(child_actor_param& s);
		~child_actor_param();
		child_actor_param& operator =(child_actor_param& s);
		bool _isCopy;
#endif
		actor_handle _actor;///<��Actor
		list<actor_handle>::iterator _actorIt;///<�����ڸ�Actor�����еĽڵ�
	};
private:
	child_actor_handle(child_actor_handle&);
	child_actor_handle& operator =(child_actor_handle&);
public:
	child_actor_handle();
	child_actor_handle(child_actor_param& s);
	~child_actor_handle();
	child_actor_handle& operator =(child_actor_param& s);
	actor_handle get_actor();
	static ptr make_ptr();
	bool empty();
private:
	actor_handle peel();
	void* operator new(size_t s);
public:
	void operator delete(void* p);
private:
	DEBUG_OPERATION(list<std::function<void ()> >::iterator _qh);
	bool _norQuit;///<�Ƿ������˳�
	bool _quited;///<����Ƿ��Ѿ��ر�
	child_actor_param _param;
};
//////////////////////////////////////////////////////////////////////////

class my_actor
{
	struct suspend_resume_option 
	{
		bool _isSuspend;
		std::function<void ()> _h;
	};

	template <typename T = void>
	struct my_size_of
	{
		enum{ size = sizeof(T) };
	};

	template <>
	struct my_size_of<void>
	{
		enum{ size = -1 };
	};

	struct msg_pool_status 
	{
		struct pck_base 
		{
			pck_base(my_actor* hostActor)
			:_strand(hostActor->_strand), _amutex(_strand), _isHead(true), _hostActor(hostActor){}

			virtual ~pck_base(){};

			virtual void close() = 0;

			virtual bool cmp_size(const size_t s0, const size_t s1, const size_t s2, const size_t s3) = 0;

			bool is_closed()
			{
				return !_hostActor;
			}

			void lock(my_actor* self)
			{
				_amutex.lock(self);
			}

			void unlock(my_actor* self)
			{
				_amutex.unlock(self);
			}

			shared_strand _strand;
			actor_mutex _amutex;
			bool _isHead;
			my_actor* _hostActor;
		};

		template <typename T0, typename T1, typename T2, typename T3>
		struct pck: public pck_base
		{
			pck(my_actor* hostActor)
			:pck_base(hostActor){}

			void close()
			{
				if (_msgPump)
				{
					_msgPump->close();
				}
				_hostActor = NULL;
			}

			bool cmp_size(const size_t s0, const size_t s1, const size_t s2, const size_t s3)
			{
				return 0 == (((my_size_of<T0>::size ^ s0) | (my_size_of<T1>::size ^ s1)) | ((my_size_of<T2>::size ^ s2) | (my_size_of<T3>::size ^ s3)));
			}

			std::shared_ptr<msg_pool<T0, T1, T2, T3> > _msgPool;
			std::shared_ptr<msg_pump<T0, T1, T2, T3> > _msgPump;
			std::shared_ptr<pck> _next;
		};

		void clear(my_actor* self)
		{
			for (int i = 0; i < 5; i++)
			{
				for_each(_msgPumpList[i].begin(), _msgPumpList[i].end(), [&](const std::shared_ptr<pck_base>& p){p->_amutex.quited_lock(self); });
				for_each(_msgPumpList[i].begin(), _msgPumpList[i].end(), [](const std::shared_ptr<pck_base>& p){p->close(); });
				for_each(_msgPumpList[i].begin(), _msgPumpList[i].end(), [&](const std::shared_ptr<pck_base>& p){p->_amutex.quited_unlock(self); });
			}
		}

		list<std::shared_ptr<pck_base> > _msgPumpList[5];
	};

	struct timer_pck;
	class boost_actor_run;
	friend boost_actor_run;
	friend child_actor_handle;
	friend msg_pump_base;
	friend actor_msg_handle_base;
	friend trig_once_base;
	friend mutex_trig_notifer;
	friend mutex_trig_handle;
	friend _actor_mutex;
public:
	/*!
	@brief ��{}һ����Χ��������ǰActor����ǿ���˳�
	*/
	class quit_guard
	{
	public:
		quit_guard(my_actor* self)
		:_self(self)
		{
			_locked = true;
			_self->lock_quit();
		}

		~quit_guard()
		{
			if (_locked)
			{
				_self->unlock_quit();
			}
		}

		void lock()
		{
			assert(!_locked);
			_locked = true;
			_self->lock_quit();
		}

		void unlock()
		{
			assert(_locked);
			_locked = false;
			_self->unlock_quit();
		}
	private:
		quit_guard(const quit_guard&){};
		void operator=(const quit_guard&){};
		my_actor* _self;
		bool _locked;
	};

	/*!
	@brief Actor��ǿ���˳����쳣����
	*/
	struct force_quit_exception { };

	/*!
	@brief ��Ϣ�ñ��Ͽ�
	*/
	struct pump_disconnected_exception { };

	/*!
	@brief Actor��ں�����
	*/
	typedef std::function<void (my_actor*)> main_func;

	/*!
	@brief actor id
	*/
	typedef long long id;
private:
	my_actor();
	my_actor(const my_actor&);
	my_actor& operator =(const my_actor&);
	~my_actor();
public:
	/*!
	@brief ����һ��Actor
	@param actorStrand Actor��������strand
	@param mainFunc Actorִ�����
	@param stackSize Actorջ��С��Ĭ��64k�ֽڣ�������4k������������С4k�����1M
	*/
	static actor_handle create(shared_strand actorStrand, const main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);

	/*!
	@brief ���ö�ջ�ڴ��
	*/
	static void enable_stack_pool();

	/*!
	@brief ���ô���Actorʱ�Զ����춨ʱ��
	*/
	static void disable_auto_make_timer();
public:
	/*!
	@brief ����һ����Actor����Actor��ֹʱ����ActorҲ��ֹ������Actor����ȫ�˳��󣬸�Actor�Ž�����
	@param actorStrand ��Actor������strand
	@param mainFunc ��Actor��ں���
	@param stackSize Actorջ��С��4k�������������1MB��
	@return ��Actor�����ʹ�� child_actor_handle ���շ���ֵ
	*/
	child_actor_handle::child_actor_param create_child_actor(shared_strand actorStrand, const main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);
	child_actor_handle::child_actor_param create_child_actor(const main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);

	/*!
	@brief ��ʼ������Actor��ֻ�ܵ���һ��
	*/
	void child_actor_run(child_actor_handle& actorHandle);
	void child_actor_run(const list<child_actor_handle::ptr>& actorHandles);

	/*!
	@brief ǿ����ֹһ����Actor
	*/
	__yield_interrupt bool child_actor_force_quit(child_actor_handle& actorHandle);
	__yield_interrupt void child_actors_force_quit(const list<child_actor_handle::ptr>& actorHandles);

	/*!
	@brief �ȴ�һ����Actor��ɺ󷵻�
	@return �����˳��ķ���true������false
	*/
	__yield_interrupt bool child_actor_wait_quit(child_actor_handle& actorHandle);

	__yield_interrupt bool timed_child_actor_wait_quit(int tm, child_actor_handle& actorHandle);

	/*!
	@brief �ȴ�һ����Actor��ɺ󷵻�
	@return �������˳��ķ���true������false
	*/
	__yield_interrupt void child_actors_wait_quit(const list<child_actor_handle::ptr>& actorHandles);

	/*!
	@brief ������Actor
	*/
	__yield_interrupt void child_actor_suspend(child_actor_handle& actorHandle);
	
	/*!
	@brief ����һ����Actor
	*/
	__yield_interrupt void child_actors_suspend(const list<child_actor_handle::ptr>& actorHandles);

	/*!
	@brief �ָ���Actor
	*/
	__yield_interrupt void child_actor_resume(child_actor_handle& actorHandle);
	
	/*!
	@brief �ָ�һ����Actor
	*/
	__yield_interrupt void child_actors_resume(const list<child_actor_handle::ptr>& actorHandles);

	/*!
	@brief ������һ��Actor��Actorִ����ɺ󷵻�
	*/
	__yield_interrupt bool run_child_actor_complete(shared_strand actorStrand, const main_func& h, size_t stackSize = DEFAULT_STACKSIZE);
	__yield_interrupt bool run_child_actor_complete(const main_func& h, size_t stackSize = DEFAULT_STACKSIZE);

	/*!
	@brief ��ʱ�ȴ���Actor�ڲ���ֹʹ�ò���ϵͳAPI Sleep()
	@param ms �ȴ�������������0ʱ��ʱ����Actorִ�У�ֱ���´α�����������
	*/
	__yield_interrupt void sleep(int ms);

	/*!
	@brief �жϵ�ǰʱ��Ƭ���ȵ��´α�����(��ΪActor�Ƿ���ռʽ���ȣ�����ռ��ʱ��Ƭ�ϳ����߼�ʱ���ʵ�ʹ��yield�ָ�ʱ��Ƭ)
	*/
	__yield_interrupt void yield();

	/*!
	@brief ����disable_auto_make_timer��ʹ������򿪵�ǰActor��ʱ��
	*/
	void open_timer();

	/*!
	@brief �ر��ڲ���ʱ��
	*/
	void close_timer();

	/*!
	@brief ��ȡ��Actor
	*/
	actor_handle parent_actor();

	/*!
	@brief ��ȡ��Actor
	*/
	const list<actor_handle>& child_actors();
public:
	typedef list<std::function<void ()> >::iterator quit_iterator;

	/*!
	@brief ע��һ����Դ�ͷź�������ǿ���˳�Actorʱ����
	*/
	quit_iterator regist_quit_handler(const std::function<void ()>& quitHandler);

	/*!
	@brief ע����Դ�ͷź���
	*/
	void cancel_quit_handler(quit_iterator qh);
public:
	/*!
	@brief ʹ���ڲ���ʱ����ʱ����ĳ���������ڴ������֮ǰ���ܶ�ε���
	@param ms ������ʱ(����)
	@param h ��������
	*/
	template <typename H>
	void delay_trig(int ms, const H& h)
	{
		assert_enter();
		if (ms > 0)
		{
			assert(_timer);
			time_out(ms, h);
		} 
		else if (0 == ms)
		{
			_strand->post(h);
		}
		else
		{
			assert(false);
		}
	}

	/*!
	@brief ȡ���ڲ���ʱ������
	*/
	void cancel_delay_trig();
public:
	/*!
	@brief ����һ���첽������shared_strand��ִ�У���ɺ󷵻�
	*/
	template <typename H>
	__yield_interrupt void send(shared_strand exeStrand, const H& h)
	{
		assert_enter();
		if (exeStrand != _strand)
		{
			actor_handle shared_this = shared_from_this();
			exeStrand->asyncInvokeVoid(h, [shared_this]{shared_this->trig_handler(); });
			push_yield();
			return;
		}
		h();
	}

	template <typename T0, typename H>
	__yield_interrupt T0 send(shared_strand exeStrand, const H& h)
	{
		assert_enter();
		if (exeStrand != _strand)
		{
			dst_receiver_buff<T0>::dst_buff dstBuff;
			dst_receiver_buff<T0> dstRec(dstBuff);
			actor_handle shared_this = shared_from_this();
			exeStrand->asyncInvoke(h, [shared_this, &dstRec](const T0& p0){shared_this->_trig_handler(dstRec, std::move(msg_param<T0>(p0))); });
			push_yield();
			return dstBuff.get()._res0;
		} 
		return h();
	}

	template <typename H>
	__yield_interrupt void async_send(shared_strand exeStrand, const H& h)
	{
		assert_enter();
		actor_handle shared_this = shared_from_this();
		exeStrand->asyncInvokeVoid(h, [shared_this]{shared_this->trig_handler(); });
		push_yield();
	}

	template <typename T0, typename H>
	__yield_interrupt T0 async_send(shared_strand exeStrand, const H& h)
	{
		assert_enter();
		dst_receiver_buff<T0>::dst_buff dstBuff;
		dst_receiver_buff<T0> dstRec(dstBuff);
		actor_handle shared_this = shared_from_this();
		exeStrand->asyncInvoke(h, [shared_this, &dstRec](const T0& p0){shared_this->_trig_handler(dstRec, std::move(msg_param<T0>(p0))); });
		push_yield();
		return dstBuff.get()._res0;
	}

	/*!
	@brief ����һ���첽�������첽�ص���ɺ󷵻�
	*/
	template <typename H>
	__yield_interrupt void trig(const H& h)
	{
		assert_enter();
		h(trig_once_notifer<>(shared_from_this(), NULL));
		push_yield();
	}

	template <typename T0, typename H>
	__yield_interrupt void trig(__out T0& r0, const H& h)
	{
		assert_enter();
		ref_ex<T0> dstRef(r0);
		dst_receiver_ref<T0> dstRec(dstRef);
		h(trig_once_notifer<T0>(shared_from_this(), &dstRec));
		push_yield();
	}

	template <typename T0, typename H>
	__yield_interrupt T0 trig(const H& h)
	{
		T0 r0;
		trig(r0, h);
		return r0;
	}

	template <typename T0, typename T1, typename H>
	__yield_interrupt void trig(__out T0& r0, __out T1& r1, const H& h)
	{
		assert_enter();
		ref_ex<T0, T1> dstRef(r0, r1);
		dst_receiver_ref<T0, T1> dstRec(dstRef);
		h(trig_once_notifer<T0, T1>(shared_from_this(), &dstRec));
		push_yield();
	}

	template <typename T0, typename T1, typename T2, typename H>
	__yield_interrupt void trig(__out T0& r0, __out T1& r1, __out T2& r2, const H& h)
	{
		assert_enter();
		ref_ex<T0, T1, T2> dstRef(r0, r1, r2);
		dst_receiver_ref<T0, T1, T2> dstRec(dstRef);
		h(trig_once_notifer<T0, T1, T2>(shared_from_this(), &dstRec));
		push_yield();
	}

	template <typename T0, typename T1, typename T2, typename T3, typename H>
	__yield_interrupt void trig(__out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3, const H& h)
	{
		assert_enter();
		ref_ex<T0, T1, T2, T3> dstRef(r0, r1, r2, r3);
		dst_receiver_ref<T0, T1, T2, T3> dstRec(dstRef);
		h(trig_once_notifer<T0, T1, T2, T3>(shared_from_this(), &dstRec));
		push_yield();
	}
private:
	void trig_handler();

	template <typename DST /*dst_receiver*/, typename SRC /*msg_param*/>
	void _trig_handler(DST& dstRec, SRC&& src)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited)
			{
				dstRec.move_from(src);
				trig_handler();
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &dstRec]
			{
				if (!shared_this->_quited)
				{
					dstRec.move_from((SRC&)src);
					shared_this->pull_yield();
				}
			});
		}
	}
public:
	/*!
	@brief ����һ����Ϣ֪ͨ����
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	actor_msg_notifer<T0, T1, T2, T3> make_msg_notifer(actor_msg_handle<T0, T1, T2, T3>& amh)
	{
		return amh.make_notifer(shared_from_this());
	}

	template <typename T0, typename T1, typename T2>
	actor_msg_notifer<T0, T1, T2> make_msg_notifer(actor_msg_handle<T0, T1, T2>& amh)
	{
		return amh.make_notifer(shared_from_this());
	}

	template <typename T0, typename T1>
	actor_msg_notifer<T0, T1> make_msg_notifer(actor_msg_handle<T0, T1>& amh)
	{
		return amh.make_notifer(shared_from_this());
	}

	template <typename T0>
	actor_msg_notifer<T0> make_msg_notifer(actor_msg_handle<T0>& amh)
	{
		return amh.make_notifer(shared_from_this());
	}

	actor_msg_notifer<> make_msg_notifer(actor_msg_handle<>& amh);

	/*!
	@brief �ر���Ϣ֪ͨ���
	*/
	void close_msg_notifer(actor_msg_handle_base& amh);
public:
	/*!
	@brief ʹ���ڲ���ʱ����ʱ����ĳ�������֮ǰ������ù�make_trig_notifer
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	void delay_trig(int ms, actor_trig_handle<T0, T1, T2, T3>& ath, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		assert_enter();
		assert(ath._hostActor && ath._hostActor->self_id() == self_id());
		assert(ath._closed && !(*ath._closed));
		auto& closed_ = ath._closed;
		delay_trig(ms, [=, &ath]
		{
			if (!*(closed_))
			{
				ath.push_msg(ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3));
			}
		});
	}

	template <typename T0, typename T1, typename T2>
	void delay_trig(int ms, actor_trig_handle<T0, T1, T2>& ath, const T0& p0, const T1& p1, const T2& p2)
	{
		assert_enter();
		assert(ath._hostActor && ath._hostActor->self_id() == self_id());
		assert(ath._closed && !(*ath._closed));
		auto& closed_ = ath._closed;
		delay_trig(ms, [=, &ath]
		{
			if (!*(closed_))
			{
				ath.push_msg(ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2));
			}
		});
	}

	template <typename T0, typename T1>
	void delay_trig(int ms, actor_trig_handle<T0, T1>& ath, const T0& p0, const T1& p1)
	{
		assert_enter();
		assert(ath._hostActor && ath._hostActor->self_id() == self_id());
		assert(ath._closed && !(*ath._closed));
		auto& closed_ = ath._closed;
		delay_trig(ms, [=, &ath]
		{
			if (!*(closed_))
			{
				ath.push_msg(ref_ex<T0, T1>((T0&)p0, (T1&)p1));
			}
		});
	}

	template <typename T0>
	void delay_trig(int ms, actor_trig_handle<T0>& ath, const T0& p0)
	{
		assert_enter();
		assert(ath._hostActor && ath._hostActor->self_id() == self_id());
		assert(ath._closed && !(*ath._closed));
		auto& closed_ = ath._closed;
		delay_trig(ms, [=, &ath]
		{
			if (!*(closed_))
			{
				ath.push_msg(ref_ex<T0>((T0&)p0));
			}
		});
	}
private:
	template <typename AMH, typename DST>
	bool _timed_wait_msg(AMH& amh, DST& dstRec, int tm)
	{
		assert(amh._hostActor && amh._hostActor->self_id() == self_id());
		if (!amh.read_msg(dstRec))
		{
			bool timeout = false;
			if (tm >= 0)
			{
				delay_trig(tm, [this, &timeout]
				{
					if (!_quited)
					{
						timeout = true;
						pull_yield();
					}
				});
			}
			push_yield();
			if (!timeout)
			{
				if (tm >= 0)
				{
					cancel_delay_trig();
				}
				return true;
			}
			amh._dstRec = NULL;
			amh._waiting = false;
			return false;
		}
		return true;
	}
public:
	/*!
	@brief ����Ϣ�������ȡ��Ϣ
	@param tm ��ʱʱ��
	@return ��ʱ��ɷ���false���ɹ���ȡ��Ϣ����true
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0, T1, T2, T3>& amh, T0& r0, T1& r1, T2& r2, T3& r3)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		ref_ex<T0, T1, T2, T3> dstRef(r0, r1, r2, r3);
		dst_receiver_ref<T0, T1, T2, T3> dstRec(dstRef);
		return _timed_wait_msg(amh, dstRec, tm);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0, T1, T2>& amh, T0& r0, T1& r1, T2& r2)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		ref_ex<T0, T1, T2> dstRef(r0, r1, r2);
		dst_receiver_ref<T0, T1, T2> dstRec(dstRef);
		return _timed_wait_msg(amh, dstRec, tm);
	}

	template <typename T0, typename T1>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0, T1>& amh, T0& r0, T1& r1)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		ref_ex<T0, T1> dstRef(r0, r1);
		dst_receiver_ref<T0, T1> dstRec(dstRef);
		return _timed_wait_msg(amh, dstRec, tm);
	}

	template <typename T0>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0>& amh, T0& r0)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		ref_ex<T0> dstRef(r0);
		dst_receiver_ref<T0> dstRec(dstRef);
		return _timed_wait_msg(amh, dstRec, tm);
	}

	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<>& amh);

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0, T1, T2, T3>& amh, const Handler& h)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		dst_receiver_buff<T0, T1, T2, T3>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1, T2, T3> dstRec(dstBuff);
		if (_timed_wait_msg(amh, dstRec, tm))
		{
			msg_param<T0, T1, T2, T3>& msg = dstBuff.get();
			h(msg._res0, msg._res1, msg._res2, msg._res3);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0, T1, T2>& amh, const Handler& h)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		dst_receiver_buff<T0, T1, T2>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1, T2> dstRec(dstBuff);
		if (_timed_wait_msg(amh, dstRec, tm))
		{
			msg_param<T0, T1, T2>& msg = dstBuff.get();
			h(msg._res0, msg._res1, msg._res2);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0, T1>& amh, const Handler& h)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		dst_receiver_buff<T0, T1>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1> dstRec(dstBuff);
		if (_timed_wait_msg(amh, dstRec, tm))
		{
			msg_param<T0, T1>& msg = dstBuff.get();
			h(msg._res0, msg._res1);
			return true;
		}
		return false;
	}

	template <typename T0, typename Handler>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<T0>& amh, const Handler& h)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		dst_receiver_buff<T0>::dst_buff dstBuff;
		dst_receiver_buff<T0> dstRec(dstBuff);
		if (_timed_wait_msg(amh, dstRec, tm))
		{
			msg_param<T0>& msg = dstBuff.get();
			h(msg._res0);
			return true;
		}
		return false;
	}

	template <typename Handler>
	__yield_interrupt bool timed_wait_msg(int tm, actor_msg_handle<>& amh, const Handler& h)
	{
		assert_enter();
		assert(amh._closed && !(*amh._closed));
		if (timed_wait_msg(tm, amh))
		{
			h();
			return true;
		}
		return false;
	}


	/*!
	@brief ����Ϣ�������ȡ��Ϣ
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void wait_msg(actor_msg_handle<T0, T1, T2, T3>& amh, T0& r0, T1& r1, T2& r2, T3& r3)
	{
		timed_wait_msg(-1, amh, r0, r1, r2, r3);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void wait_msg(actor_msg_handle<T0, T1, T2>& amh, T0& r0, T1& r1, T2& r2)
	{
		timed_wait_msg(-1, amh, r0, r1, r2);
	}

	template <typename T0, typename T1>
	__yield_interrupt void wait_msg(actor_msg_handle<T0, T1>& amh, T0& r0, T1& r1)
	{
		timed_wait_msg(-1, amh, r0, r1);
	}

	template <typename T0>
	__yield_interrupt void wait_msg(actor_msg_handle<T0>& amh, T0& r0)
	{
		timed_wait_msg(-1, amh, r0);
	}

	template <typename T0>
	__yield_interrupt T0 wait_msg(actor_msg_handle<T0>& amh)
	{
		T0 r0;
		timed_wait_msg(-1, amh, r0);
		return r0;
	}

	__yield_interrupt void wait_msg(actor_msg_handle<>& amh);

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt void wait_msg(actor_msg_handle<T0, T1, T2, T3>& amh, const Handler& h)
	{
		timed_wait_msg<T0, T1, T2, T3>(-1, amh, h);
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt void wait_msg(actor_msg_handle<T0, T1, T2>& amh, const Handler& h)
	{
		timed_wait_msg<T0, T1, T2>(-1, amh, h);
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt void wait_msg(actor_msg_handle<T0, T1>& amh, const Handler& h)
	{
		timed_wait_msg<T0, T1>(-1, amh, h);
	}

	template <typename T0, typename Handler>
	__yield_interrupt void wait_msg(actor_msg_handle<T0>& amh, const Handler& h)
	{
		timed_wait_msg<T0>(-1, amh, h);
	}

	template <typename Handler>
	__yield_interrupt void wait_msg(actor_msg_handle<>& amh, const Handler& h)
	{
		timed_wait_msg(-1, amh, h);
	}
public:
	/*!
	@brief ����һ����Ϣ����������ֻ��һ�δ�����Ч
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	actor_trig_notifer<T0, T1, T2, T3> make_trig_notifer(actor_trig_handle<T0, T1, T2, T3>& ath)
	{
		return ath.make_notifer(shared_from_this());
	}

	template <typename T0, typename T1, typename T2>
	actor_trig_notifer<T0, T1, T2> make_trig_notifer(actor_trig_handle<T0, T1, T2>& ath)
	{
		return ath.make_notifer(shared_from_this());
	}

	template <typename T0, typename T1>
	actor_trig_notifer<T0, T1> make_trig_notifer(actor_trig_handle<T0, T1>& ath)
	{
		return ath.make_notifer(shared_from_this());
	}

	template <typename T0>
	actor_trig_notifer<T0> make_trig_notifer(actor_trig_handle<T0>& ath)
	{
		return ath.make_notifer(shared_from_this());
	}

	actor_trig_notifer<> make_trig_notifer(actor_trig_handle<>& ath);

	/*!
	@brief �ر���Ϣ�������
	*/
	void close_trig_notifer(actor_msg_handle_base& ath);
public:
	/*!
	@brief �Ӵ����������ȡ��Ϣ
	@param tm ��ʱʱ��
	@return ��ʱ��ɷ���false���ɹ���ȡ��Ϣ����true
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0, T1, T2, T3>& ath, T0& r0, T1& r1, T2& r2, T3& r3)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		ref_ex<T0, T1, T2, T3> dstRef(r0, r1, r2, r3);
		dst_receiver_ref<T0, T1, T2, T3> dstRec(dstRef);
		return _timed_wait_msg(ath, dstRec, tm);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0, T1, T2>& ath, T0& r0, T1& r1, T2& r2)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		ref_ex<T0, T1, T2> dstRef(r0, r1, r2);
		dst_receiver_ref<T0, T1, T2> dstRec(dstRef);
		return _timed_wait_msg(ath, dstRec, tm);
	}

	template <typename T0, typename T1>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0, T1>& ath, T0& r0, T1& r1)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		ref_ex<T0, T1> dstRef(r0, r1);
		dst_receiver_ref<T0, T1> dstRec(dstRef);
		return _timed_wait_msg(ath, dstRec, tm);
	}

	template <typename T0>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0>& ath, T0& r0)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		ref_ex<T0> dstRef(r0);
		dst_receiver_ref<T0> dstRec(dstRef);
		return _timed_wait_msg(ath, dstRec, tm);
	}

	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<>& ath);

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0, T1, T2, T3>& ath, const Handler& h)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		dst_receiver_buff<T0, T1, T2, T3>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1, T2, T3> dstRec(dstBuff);
		if (_timed_wait_msg(ath, dstRec, tm))
		{
			msg_param<T0, T1, T2, T3>& msg = dstBuff.get();
			h(msg._res0, msg._res1, msg._res2, msg._res3);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0, T1, T2>& ath, const Handler& h)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		dst_receiver_buff<T0, T1, T2>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1, T2> dstRec(dstBuff);
		if (_timed_wait_msg(ath, dstRec, tm))
		{
			msg_param<T0, T1, T2>& msg = dstBuff.get();
			h(msg._res0, msg._res1, msg._res2);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0, T1>& ath, const Handler& h)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		dst_receiver_buff<T0, T1>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1> dstRec(dstBuff);
		if (_timed_wait_msg(ath, dstRec, tm))
		{
			msg_param<T0, T1>& msg = dstBuff.get();
			h(msg._res0, msg._res1);
			return true;
		}
		return false;
	}

	template <typename T0, typename Handler>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<T0>& ath, const Handler& h)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		dst_receiver_buff<T0>::dst_buff dstBuff;
		dst_receiver_buff<T0> dstRec(dstBuff);
		if (_timed_wait_msg(ath, dstRec, tm))
		{
			msg_param<T0>& msg = dstBuff.get();
			h(msg._res0);
			return true;
		}
		return false;
	}

	template <typename Handler>
	__yield_interrupt bool timed_wait_trig(int tm, actor_trig_handle<>& ath, const Handler& h)
	{
		assert_enter();
		assert(ath._closed && !(*ath._closed));
		if (timed_wait_trig(tm, ath))
		{
			h();
			return true;
		}
		return false;
	}

	/*!
	@brief �Ӵ����������ȡ��Ϣ
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void wait_trig(actor_trig_handle<T0, T1, T2, T3>& ath, T0& r0, T1& r1, T2& r2, T3& r3)
	{
		timed_wait_trig(-1, ath, r0, r1, r2, r3);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void wait_trig(actor_trig_handle<T0, T1, T2>& ath, T0& r0, T1& r1, T2& r2)
	{
		timed_wait_trig(-1, ath, r0, r1, r2);
	}

	template <typename T0, typename T1>
	__yield_interrupt void wait_trig(actor_trig_handle<T0, T1>& ath, T0& r0, T1& r1)
	{
		timed_wait_trig(-1, ath, r0, r1);
	}

	template <typename T0>
	__yield_interrupt void wait_trig(actor_trig_handle<T0>& ath, T0& r0)
	{
		timed_wait_trig(-1, ath, r0);
	}

	template <typename T0>
	__yield_interrupt T0 wait_trig(actor_trig_handle<T0>& ath)
	{
		T0 r0;
		timed_wait_trig(-1, ath, r0);
		return r0;
	}

	__yield_interrupt void wait_trig(actor_trig_handle<>& ath);

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt void wait_trig(actor_trig_handle<T0, T1, T2, T3>& ath, const Handler& h)
	{
		timed_wait_trig<T0, T1, T2, T3>(-1, ath, h);
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt void wait_trig(actor_trig_handle<T0, T1, T2>& ath, const Handler& h)
	{
		timed_wait_trig<T0, T1, T2>(-1, ath, h);
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt void wait_trig(actor_trig_handle<T0, T1>& ath, const Handler& h)
	{
		timed_wait_trig<T0, T1>(-1, ath, h);
	}

	template <typename T0, typename Handler>
	__yield_interrupt void wait_trig(actor_trig_handle<T0>& ath, const Handler& h)
	{
		timed_wait_trig<T0>(-1, ath, h);
	}

	template <typename Handler>
	__yield_interrupt void wait_trig(actor_trig_handle<>& ath, const Handler& h)
	{
		timed_wait_trig(-1, ath, h);
	}
private:
	/*!
	@brief Ѱ�ҳ���ģ���������ƥ�����Ϣ��
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3> > msg_pool_pck(bool make = true)
	{
		typedef msg_pool_status::pck<T0, T1, T2, T3> pck_type;

		auto& msgPumpList = _msgPoolStatus._msgPumpList[func_type<T0, T1, T2, T2>::number];
		if (0 == func_type<T0, T1, T2, T2>::number)
		{
			if (!msgPumpList.empty())
			{
				//û�в�������Ϣ������Ҫƥ������
				return std::static_pointer_cast<pck_type>(msgPumpList.front());
			}
		}
		else
		{
			for (auto it = msgPumpList.begin(); it != msgPumpList.end(); it++)
			{
				//��ƥ��������ʹ�С�Ƿ�һ��
				if ((*it)->cmp_size(my_size_of<T0>::size, my_size_of<T1>::size, my_size_of<T2>::size, my_size_of<T3>::size))
				{
					//Ȼ��̬���Ͳ���
					auto pool = std::dynamic_pointer_cast<pck_type>(*it);
					if (pool)
					{
						return pool;
					}
				}
			}
		}
		if (make)
		{
			std::shared_ptr<pck_type> newPck(new pck_type(this));
			msgPumpList.push_back(newPck);
			return newPck;
		}
		return std::shared_ptr<pck_type>();
	}

	/*!
	@brief �����Ϣ������
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	void clear_msg_list(const std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>>& msgPck, bool poolDis = true)
	{
		std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>> uStack[16];
		size_t stackl = 0;
		auto pckIt = msgPck;
		while (true)
		{
			if (pckIt->_next)
			{
				pckIt->_msgPool.reset();
				pckIt = pckIt->_next;
				pckIt->lock(this);
				assert(stackl < 15);
				uStack[stackl++] = pckIt;
			}
			else
			{
				if (!pckIt->is_closed())
				{
					if (pckIt->_msgPool)
					{
						if (poolDis)
						{
							auto& msgPool_ = pckIt->_msgPool;
							auto& msgPump_ = pckIt->_msgPump;
							send(msgPool_->_strand, [&msgPool_, &msgPump_]
							{
								assert(msgPool_->_msgPump == msgPump_);
								msgPool_->disconnect();
							});
						}
						pckIt->_msgPool.reset();
					}
					if (pckIt->_msgPump)
					{
						auto& msgPump_ = pckIt->_msgPump;
						send(msgPump_->_strand, [&msgPump_]
						{
							msgPump_->clear();
						});
					}
				}
				while (stackl)
				{
					uStack[--stackl]->unlock(this);
				}
				return;
			}
		}
	}

	/*!
	@brief ������Ϣ������
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	actor_handle update_msg_list(const std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>>& msgPck, const std::shared_ptr<msg_pool<T0, T1, T2, T3>>& newPool)
	{
		typedef typename msg_pool<T0, T1, T2, T3>::pump_handler pump_handler;

		std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>> uStack[16];
		size_t stackl = 0;
		auto pckIt = msgPck;
		while (true)
		{
			if (pckIt->_next)
			{
				pckIt->_msgPool = newPool;
				pckIt = pckIt->_next;
				pckIt->lock(this);
				assert(stackl < 15);
				uStack[stackl++] = pckIt;
			} 
			else
			{
				if (!pckIt->is_closed())
				{
					if (pckIt->_msgPool)
					{
						auto& msgPool_ = pckIt->_msgPool;
						send(msgPool_->_strand, [&msgPool_]
						{
							msgPool_->disconnect();
						});
					}
					pckIt->_msgPool = newPool;
					if (pckIt->_msgPump)
					{
						auto& msgPump_ = pckIt->_msgPump;
						if (newPool)
						{
							auto ph = send<pump_handler>(newPool->_strand, [&newPool, &msgPump_]()->pump_handler
							{
								return newPool->connect_pump(msgPump_);
							});
							send(msgPump_->_strand, [&msgPump_, &ph]
							{
								msgPump_->connect(ph);
							});
						}
						else
						{
							send(msgPump_->_strand, [&msgPump_]
							{
								msgPump_->clear();
							});
						}
					}
					while (stackl)
					{
						uStack[--stackl]->unlock(this);
					}
					return pckIt->_hostActor->shared_from_this();
				}
				while (stackl)
				{
					uStack[--stackl]->unlock(this);
				}
				return actor_handle();
			}
		}
	}
private:
	/*!
	@brief �ѱ�Actor����Ϣ�ɻ��Actor������
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool msg_agent_to(actor_handle childActor)
	{
		typedef std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>> pck_type;

		assert_enter();
		assert(childActor);
		{
			auto isSelf = childActor->parent_actor();
			if (!isSelf || isSelf->self_id() != self_id())
			{
				assert(false);
				return false;
			}
		}
		quit_guard qg(this);
		auto childPck = send<pck_type>(childActor->self_strand(), [&childActor]()->pck_type
		{
			if (!childActor->is_quited())
			{
				return childActor->msg_pool_pck<T0, T1, T2, T3>();
			}
			return pck_type();
		});
		if (childPck)
		{
			auto msgPck = msg_pool_pck<T0, T1, T2, T3>();
			msgPck->lock(this);
			childPck->lock(this);
			childPck->_isHead = false;
			actor_handle hostActor = update_msg_list<T0, T1, T2, T3>(childPck, msgPck->_msgPool);
			if (hostActor)
			{
				if (msgPck->_next && msgPck->_next != childPck)
				{
					msgPck->_next->lock(this);
					clear_msg_list<T0, T1, T2, T3>(msgPck->_next, false);
					msgPck->_next->unlock(this);
				}
				msgPck->_next = childPck;
				childPck->unlock(this);
				msgPck->unlock(this);
				return true;
			}
			childPck->unlock(this);
			msgPck->unlock(this);
		}
		return false;
	}
public:
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool msg_agent_to(child_actor_handle& childActor)
	{
		return msg_agent_to<T0, T1, T2, T3>(childActor.get_actor());
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool msg_agent_to(child_actor_handle& childActor)
	{
		return msg_agent_to<T0, T1, T2, void>(childActor.get_actor());
	}

	template <typename T0, typename T1>
	__yield_interrupt bool msg_agent_to(child_actor_handle& childActor)
	{
		return msg_agent_to<T0, T1, void, void>(childActor.get_actor());
	}

	template <typename T0>
	__yield_interrupt bool msg_agent_to(child_actor_handle& childActor)
	{
		return msg_agent_to<T0, void, void, void>(childActor.get_actor());
	}

	__yield_interrupt bool msg_agent_to(child_actor_handle& childActor);

public:
	/*!
	@brief ����Ϣָ����һ���ض�Actor������ȥ����
	@return ���ش������Ϣ����Actor���
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(shared_strand strand, bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		child_actor_handle::child_actor_param childActor = create_child_actor(strand, [agentActor](my_actor* self)
		{
			agentActor(self, self->connect_msg_pump<T0, T1, T2, T3>());
		}, stackSize);
		msg_agent_to<T0, T1, T2, T3>(childActor._actor);
		if (autoRun)
		{
			childActor._actor->notify_run();
		}
		return childActor;
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(shared_strand strand, bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, T1, T2, void>(strand, autoRun, agentActor, stackSize);
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(shared_strand strand, bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, T1, void, void>(strand, autoRun, agentActor, stackSize);
	}

	template <typename T0, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(shared_strand strand, bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, void, void, void>(strand, autoRun, agentActor, stackSize);
	}

	template <typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(shared_strand strand, bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<void, void, void, void>(strand, autoRun, agentActor, stackSize);
	}

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, T1, T2, T3>(self_strand(), autoRun, agentActor, stackSize);
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, T1, T2, void>(self_strand(), autoRun, agentActor, stackSize);
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, T1, void, void>(self_strand(), autoRun, agentActor, stackSize);
	}

	template <typename T0, typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<T0, void, void, void>(self_strand(), autoRun, agentActor, stackSize);
	}

	template <typename Handler>
	__yield_interrupt child_actor_handle::child_actor_param msg_agent_to_actor(bool autoRun, const Handler& agentActor, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return msg_agent_to_actor<void, void, void, void>(self_strand(), autoRun, agentActor, stackSize);
	}
public:
	/*!
	@brief �Ͽ����������Ϣ
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void msg_agent_off()
	{
		assert_enter();
		auto msgPck = msg_pool_pck<T0, T1, T2, T3>(false);
		if (msgPck)
		{
			quit_guard qg(this);
			msgPck->lock(this);
			if (msgPck->_next)
			{
				msgPck->_next->lock(this);
				clear_msg_list<T0, T1, T2, T3>(msgPck->_next);
				msgPck->_next->_isHead = true;
				msgPck->_next->unlock(this);
				msgPck->_next.reset();
			}
			msgPck->unlock(this);
		}
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void msg_agent_off()
	{
		msg_agent_off<T0, T1, T2, void>();
	}

	template <typename T0, typename T1>
	__yield_interrupt void msg_agent_off()
	{
		msg_agent_off<T0, T1, void, void>();
	}

	template <typename T0>
	__yield_interrupt void msg_agent_off()
	{
		msg_agent_off<T0, void, void, void>();
	}

	__yield_interrupt void msg_agent_off();
public:
	/*!
	@brief ������Ϣ֪ͨ��һ�����Actor����Actor��������Actor��û�и�Actor
	@param makeNew false ������ڷ���֮ǰ�����򴴽��µ�֪ͨ��true ǿ�ƴ����µ�֪ͨ��֮ǰ�Ľ�ʧЧ���ҶϿ���buddyActor�Ĺ���
	@param fixedSize ��Ϣ�����ڴ�س���
	@warning ��� makeNew = false �Ҹýڵ�Ϊ���Ĵ���������ʧ��
	@return ��Ϣ֪ͨ����
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt post_actor_msg<T0, T1, T2, T3> connect_msg_notifer_to(actor_handle buddyActor, bool makeNew = false, size_t fixedSize = 16)
	{
		typedef msg_pool<T0, T1, T2, T3> pool_type;
		typedef typename pool_type::pump_handler pump_handler;
		typedef std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>> pck_type;

		assert_enter();
		{
			auto buddyParent = buddyActor->parent_actor();
			if (!(buddyActor && (!buddyParent || buddyParent->self_id() == self_id())))
			{
				assert(false);
				return post_actor_msg<T0, T1, T2, T3>();
			}
		}
#ifdef _DEBUG
		{
			auto pa = parent_actor();
			while (pa)
			{
				assert(pa->self_id() != buddyActor->self_id());
				pa = pa->parent_actor();
			}
		}
#endif
		auto msgPck = msg_pool_pck<T0, T1, T2, T3>();
		quit_guard qg(this);
		msgPck->lock(this);
		auto childPck = send<pck_type>(buddyActor->self_strand(), [&buddyActor]()->pck_type
		{
			if (!buddyActor->is_quited())
			{
				return buddyActor->msg_pool_pck<T0, T1, T2, T3>();
			}
			return pck_type();
		});
		if (!childPck)
		{
			msgPck->unlock(this);
			return post_actor_msg<T0, T1, T2, T3>();
		}
		if (makeNew)
		{
			auto newPool = pool_type::make(buddyActor->self_strand(), fixedSize);
			childPck->lock(this);
			childPck->_isHead = true;
			actor_handle hostActor = update_msg_list<T0, T1, T2, T3>(childPck, newPool);
			childPck->unlock(this);
			if (hostActor)
			{
				//�����ǰ��Ϣ�ڵ��ڸ��µ���Ϣ���У���Ͽ���Ȼ�����ӵ�������Ϣ��
				if (msgPck->_next == childPck)
				{
					msgPck->_next.reset();
					if (msgPck->_msgPump)
					{
						auto& msgPump_ = msgPck->_msgPump;
						if (msgPck->_msgPool)
						{
							auto& msgPool_ = msgPck->_msgPool;
							msgPump_->connect(this->send<pump_handler>(msgPool_->_strand, [&msgPool_, &msgPump_]()->pump_handler
							{
								return msgPool_->connect_pump(msgPump_);
							}));
						}
						else
						{
							msgPump_->clear();
						}
					}
				}
				msgPck->unlock(this);
				return post_actor_msg<T0, T1, T2, T3>(newPool, hostActor);
			}
			msgPck->unlock(this);
			//��Ϣ���Ĵ������Ѿ��˳���ʧ��
			return post_actor_msg<T0, T1, T2, T3>();
		}
		childPck->lock(this);
		if (childPck->_isHead)
		{
			assert(msgPck->_next != childPck);
			auto childPool = childPck->_msgPool;
			if (!childPool)
			{
				childPool = pool_type::make(buddyActor->self_strand(), fixedSize);
			}
			actor_handle hostActor = update_msg_list<T0, T1, T2, T3>(childPck, childPool);
			if (hostActor)
			{
				childPck->unlock(this);
				msgPck->unlock(this);
				return post_actor_msg<T0, T1, T2, T3>(childPool, hostActor);
			}
			//��Ϣ���Ĵ������Ѿ��˳���ʧ��
		}
		childPck->unlock(this);
		msgPck->unlock(this);
		return post_actor_msg<T0, T1, T2, T3>();
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt post_actor_msg<T0, T1, T2, T3> connect_msg_notifer_to(child_actor_handle& childActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, T1, T2, T3>(childActor.get_actor(), makeNew, fixedSize);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt post_actor_msg<T0, T1, T2> connect_msg_notifer_to(const actor_handle& buddyActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, T1, T2, void>(buddyActor, makeNew, fixedSize);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt post_actor_msg<T0, T1, T2> connect_msg_notifer_to(child_actor_handle& childActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, T1, T2, void>(childActor.get_actor(), makeNew, fixedSize);
	}

	template <typename T0, typename T1>
	__yield_interrupt post_actor_msg<T0, T1> connect_msg_notifer_to(const actor_handle& buddyActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, T1, void, void>(buddyActor, makeNew, fixedSize);
	}

	template <typename T0, typename T1>
	__yield_interrupt post_actor_msg<T0, T1> connect_msg_notifer_to(child_actor_handle& childActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, T1, void, void>(childActor.get_actor(), makeNew, fixedSize);
	}

	template <typename T0>
	__yield_interrupt post_actor_msg<T0> connect_msg_notifer_to(const actor_handle& buddyActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, void, void, void>(buddyActor, makeNew, fixedSize);
	}

	template <typename T0>
	__yield_interrupt post_actor_msg<T0> connect_msg_notifer_to(child_actor_handle& childActor, bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to<T0, void, void, void>(childActor.get_actor(), makeNew, fixedSize);
	}

	__yield_interrupt post_actor_msg<> connect_msg_notifer_to(const actor_handle& buddyActor, bool makeNew = false);
	__yield_interrupt post_actor_msg<> connect_msg_notifer_to(child_actor_handle& childActor, bool makeNew = false);

	/*!
	@brief ������Ϣ֪ͨ���Լ���Actor
	@param makeNew false ������ڷ���֮ǰ�����򴴽��µ�֪ͨ��true ǿ�ƴ����µ�֪ͨ��֮ǰ�Ľ�ʧЧ���ҶϿ���buddyActor�Ĺ���
	@param fixedSize ��Ϣ�����ڴ�س���
	@warning ����ýڵ�Ϊ���Ĵ�����ô������ʧ��
	@return ��Ϣ֪ͨ����
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt post_actor_msg<T0, T1, T2, T3> connect_msg_notifer_to_self(bool makeNew = false, size_t fixedSize = 16)
	{
		typedef msg_pool<T0, T1, T2, T3> pool_type;

		assert_enter();
		auto msgPck = msg_pool_pck<T0, T1, T2, T3>();
		quit_guard qg(this);
		msgPck->lock(this);
		if (msgPck->_isHead)
		{
			std::shared_ptr<pool_type> msgPool;
			if (makeNew || !msgPck->_msgPool)
			{
				msgPool = pool_type::make(self_strand(), fixedSize);
			}
			else
			{
				msgPool = msgPck->_msgPool;
			}
			actor_handle hostActor = update_msg_list<T0, T1, T2, T3>(msgPck, msgPool);
			if (hostActor)
			{
				msgPck->unlock(this);
				return post_actor_msg<T0, T1, T2, T3>(msgPool, hostActor);
			}
		}
		msgPck->unlock(this);
		return post_actor_msg<T0, T1, T2, T3>();
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt post_actor_msg<T0, T1, T2> connect_msg_notifer_to_self(bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to_self<T0, T1, T2, void>(makeNew, fixedSize);
	}

	template <typename T0, typename T1>
	__yield_interrupt post_actor_msg<T0, T1> connect_msg_notifer_to_self(bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to_self<T0, T1, void, void>(makeNew, fixedSize);
	}

	template <typename T0>
	__yield_interrupt post_actor_msg<T0> connect_msg_notifer_to_self(bool makeNew = false, size_t fixedSize = 16)
	{
		return connect_msg_notifer_to_self<T0, void, void, void>(makeNew, fixedSize);
	}

	__yield_interrupt post_actor_msg<> connect_msg_notifer_to_self(bool makeNew = false);

	/*!
	@brief ����һ����Ϣ֪ͨ�������ڸ�Actor��������ios�޹��߳���ʹ�ã����ڸ�Actor���� notify_run() ֮ǰ
	@param fixedSize ��Ϣ�����ڴ�س���
	@return ��Ϣ֪ͨ����
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	post_actor_msg<T0, T1, T2, T3> connect_msg_notifer(size_t fixedSize = 16)
	{
		typedef post_actor_msg<T0, T1, T2, T3> post_type;

		return _strand->syncInvoke<post_type>([this, fixedSize]()->post_type
		{
			typedef msg_pool<T0, T1, T2, T3> pool_type;
			if (!this->parent_actor() && !this->is_started() && !this->is_quited())
			{
				auto msgPck = this->msg_pool_pck<T0, T1, T2, T3>();
				msgPck->_msgPool = pool_type::make(this->self_strand(), fixedSize);
				return post_type(msgPck->_msgPool, shared_from_this());
			}
			assert(false);
			return post_type();
		});
	}

	template <typename T0, typename T1, typename T2>
	post_actor_msg<T0, T1, T2> connect_msg_notifer(size_t fixedSize = 16)
	{
		return connect_msg_notifer<T0, T1, T2, void>(fixedSize);
	}

	template <typename T0, typename T1>
	post_actor_msg<T0, T1> connect_msg_notifer(size_t fixedSize = 16)
	{
		return connect_msg_notifer<T0, T1, void, void>(fixedSize);
	}

	template <typename T0>
	post_actor_msg<T0> connect_msg_notifer(size_t fixedSize = 16)
	{
		return connect_msg_notifer<T0, void, void, void>(fixedSize);
	}

	post_actor_msg<> connect_msg_notifer();
	//////////////////////////////////////////////////////////////////////////

	/*!
	@brief ������Ϣ�õ���Ϣ��
	@return ������Ϣ�þ��
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	typename msg_pump<T0, T1, T2, T3>::handle connect_msg_pump()
	{
		typedef msg_pump<T0, T1, T2, T3> pump_type;
		typedef msg_pool<T0, T1, T2, T3> pool_type;
		typedef typename pool_type::pump_handler pump_handler;

		assert_enter();
		auto msgPck = msg_pool_pck<T0, T1, T2, T3>();
		quit_guard qg(this);
		msgPck->lock(this);
		if (msgPck->_next)
		{
			msgPck->_next->lock(this);
			clear_msg_list<T0, T1, T2, T3>(msgPck->_next);
			msgPck->_next->unlock(this);
			msgPck->_next.reset();
		}
		if (!msgPck->_msgPump)
		{
			msgPck->_msgPump = pump_type::make(shared_from_this());
		}
		auto msgPump = msgPck->_msgPump;
		auto msgPool = msgPck->_msgPool;
		if (msgPool)
		{
			msgPump->connect(send<pump_handler>(msgPool->_strand, [&msgPck]()->pump_handler
			{
				return msgPck->_msgPool->connect_pump(msgPck->_msgPump);
			}));
		}
		else
		{
			msgPump->clear();
		}
		msgPck->unlock(this);
		return msgPump.get();
	}

	template <typename T0, typename T1, typename T2>
	typename msg_pump<T0, T1, T2>::handle connect_msg_pump()
	{
		return connect_msg_pump<T0, T1, T2, void>();
	}

	template <typename T0, typename T1>
	typename msg_pump<T0, T1>::handle connect_msg_pump()
	{
		return connect_msg_pump<T0, T1, void, void>();
	}

	template <typename T0>
	typename msg_pump<T0>::handle connect_msg_pump()
	{
		return connect_msg_pump<T0, void, void, void>();
	}

	msg_pump<>::handle connect_msg_pump();
private:
	template <typename PUMP, typename DST>
	bool _timed_pump_msg(const PUMP& pump, DST& dstRec, int tm, bool checkDis)
	{
		assert(pump->_hostActor && pump->_hostActor->self_id() == self_id());
		if (!pump->read_msg(dstRec))
		{
			if (checkDis && pump->isDisconnected())
			{
				pump->_waiting = false;
				pump->_dstRec = NULL;
				throw pump_disconnected_exception();
			}
			pump->_checkDis = checkDis;
			bool timeout = false;
			if (tm >= 0)
			{
				delay_trig(tm, [this, &timeout]
				{
					if (!_quited)
					{
						timeout = true;
						pull_yield();
					}
				});
			}
			push_yield();
			if (!timeout)
			{
				if (tm >= 0)
				{
					cancel_delay_trig();
				}
				if (pump->_checkDis)
				{
					assert(checkDis);
					pump->_checkDis = false;
					throw pump_disconnected_exception();
				}
				return true;
			}
			pump->_checkDis = false;
			pump->_waiting = false;
			pump->_dstRec = NULL;
			return false;
		}
		return true;
	}
public:

	/*!
	@brief ����Ϣ������ȡ��Ϣ
	@param tm ��ʱʱ��
	@param checkDis ����Ƿ񱻶Ͽ����ӣ��Ǿ��׳� pump_disconnected_exception �쳣
	@return ��ʱ��ɷ���false���ɹ�ȡ����Ϣ����true
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0, T1, T2, T3>::handle& pump, T0& r0, T1& r1, T2& r2, T3& r3, bool checkDis = false)
	{
		assert_enter();
		ref_ex<T0, T1, T2, T3> dstRef(r0, r1, r2, r3);
		dst_receiver_ref<T0, T1, T2, T3> dstRec(dstRef);
		return _timed_pump_msg(pump, dstRec, tm, checkDis);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0, T1, T2>::handle& pump, T0& r0, T1& r1, T2& r2, bool checkDis = false)
	{
		assert_enter();
		ref_ex<T0, T1, T2> dstRef(r0, r1, r2);
		dst_receiver_ref<T0, T1, T2> dstRec(dstRef);
		return _timed_pump_msg(pump, dstRec, tm, checkDis);
	}

	template <typename T0, typename T1>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0, T1>::handle& pump, T0& r0, T1& r1, bool checkDis = false)
	{
		assert_enter();
		ref_ex<T0, T1> dstRef(r0, r1);
		dst_receiver_ref<T0, T1> dstRec(dstRef);
		return _timed_pump_msg(pump, dstRec, tm, checkDis);
	}

	template <typename T0>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0>::handle& pump, T0& r0, bool checkDis = false)
	{
		assert_enter();
		ref_ex<T0> dstRef(r0);
		dst_receiver_ref<T0> dstRec(dstRef);
		return _timed_pump_msg(pump, dstRec, tm, checkDis);
	}

	__yield_interrupt bool timed_pump_msg(int tm, const msg_pump<>::handle& pump, bool checkDis = false);

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0, T1, T2, T3>::handle& pump, const Handler& h, bool checkDis = false)
	{
		assert_enter();
		dst_receiver_buff<T0, T1, T2, T3>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1, T2, T3> dstRec(dstBuff);
		if (_timed_pump_msg(pump, dstRec, tm, checkDis))
		{
			msg_param<T0, T1, T2, T3>& msg = dstBuff.get();
			h(msg._res0, msg._res1, msg._res2, msg._res3);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0, T1, T2>::handle& pump, const Handler& h, bool checkDis = false)
	{
		assert_enter();
		dst_receiver_buff<T0, T1, T2>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1, T2> dstRec(dstBuff);
		if (_timed_pump_msg(pump, dstRec, tm, checkDis))
		{
			msg_param<T0, T1, T2>& msg = dstBuff.get();
			h(msg._res0, msg._res1, msg._res2);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0, T1>::handle& pump, const Handler& h, bool checkDis = false)
	{
		assert_enter();
		dst_receiver_buff<T0, T1>::dst_buff dstBuff;
		dst_receiver_buff<T0, T1> dstRec(dstBuff);
		if (_timed_pump_msg(pump, dstRec, tm, checkDis))
		{
			msg_param<T0, T1>& msg = dstBuff.get();
			h(msg._res0, msg._res1);
			return true;
		}
		return false;
	}

	template <typename T0, typename Handler>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<T0>::handle& pump, const Handler& h, bool checkDis = false)
	{
		assert_enter();
		dst_receiver_buff<T0>::dst_buff dstBuff;
		dst_receiver_buff<T0> dstRec(dstBuff);
		if (_timed_pump_msg(pump, dstRec, tm, checkDis))
		{
			msg_param<T0>& msg = dstBuff.get();
			h(msg._res0);
			return true;
		}
		return false;
	}

	template <typename Handler>
	__yield_interrupt bool timed_pump_msg(int tm, const typename msg_pump<>::handle& pump, const Handler& h, bool checkDis = false)
	{
		assert_enter();
		if (timed_pump_msg(tm, pump, checkDis))
		{
			h();
			return true;
		}
		return false;
	}

	/*!
	@brief ����Ϣ������ȡ��Ϣ
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void pump_msg(const typename msg_pump<T0, T1, T2, T3>::handle& pump, T0& r0, T1& r1, T2& r2, T3& r3, bool checkDis = false)
	{
		timed_pump_msg(-1, pump, r0, r1, r2, r3, checkDis);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void pump_msg(const typename msg_pump<T0, T1, T2>::handle& pump, T0& r0, T1& r1, T2& r2, bool checkDis = false)
	{
		timed_pump_msg(-1, pump, r0, r1, r2, checkDis);
	}

	template <typename T0, typename T1>
	__yield_interrupt void pump_msg(const typename msg_pump<T0, T1>::handle& pump, T0& r0, T1& r1, bool checkDis = false)
	{
		timed_pump_msg(-1, pump, r0, r1, checkDis);
	}

	template <typename T0>
	__yield_interrupt void pump_msg(const typename msg_pump<T0>::handle& pump, T0& r0, bool checkDis = false)
	{
		timed_pump_msg(-1, pump, r0, checkDis);
	}

	template <typename T0>
	__yield_interrupt T0 pump_msg(msg_pump<T0>& pump, bool checkDis = false)
	{
		T0 r0;
		timed_pump_msg(-1, &pump, r0, checkDis);
		return r0;
	}

	__yield_interrupt void pump_msg(const msg_pump<>::handle& pump, bool checkDis = false);

	template <typename T0, typename T1, typename T2, typename T3, typename Handler>
	__yield_interrupt void pump_msg(const typename msg_pump<T0, T1, T2, T3>::handle& pump, const Handler& h, bool checkDis = false)
	{
		timed_pump_msg<T0, T1, T2, T3>(-1, pump, h, checkDis);
	}

	template <typename T0, typename T1, typename T2, typename Handler>
	__yield_interrupt void pump_msg(const typename msg_pump<T0, T1, T2>::handle& pump, const Handler& h, bool checkDis = false)
	{
		timed_pump_msg<T0, T1, T2>(-1, pump, h, checkDis);
	}

	template <typename T0, typename T1, typename Handler>
	__yield_interrupt void pump_msg(const typename msg_pump<T0, T1>::handle& pump, const Handler& h, bool checkDis = false)
	{
		timed_pump_msg<T0, T1>(-1, pump, h, checkDis);
	}

	template <typename T0, typename Handler>
	__yield_interrupt void pump_msg(const typename msg_pump<T0>::handle& pump, const Handler& h, bool checkDis = false)
	{
		timed_pump_msg<T0>(-1, pump, h, checkDis);
	}

	template <typename Handler>
	__yield_interrupt void pump_msg(const typename msg_pump<>::handle& pump, const Handler& h, bool checkDis = false)
	{
		timed_pump_msg(-1, pump, h, checkDis);
	}
public:
	/*!
	@brief ��ѯ��ǰ��Ϣ��˭����
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	actor_handle msg_agent_handle(actor_handle buddyActor)
	{
		typedef std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>> pck_type;

		quit_guard qg(this);
		auto msgPck = send<pck_type>(buddyActor->self_strand(), [&buddyActor]()->pck_type
		{
			if (!buddyActor->is_quited())
			{
				return buddyActor->msg_pool_pck<T0, T1, T2, T3>(false);
			}
			return pck_type();
		});
		if (msgPck)
		{
			std::shared_ptr<msg_pool_status::pck<T0, T1, T2, T3>> uStack[16];
			size_t stackl = 0;
			msgPck->lock(this);
			auto pckIt = msgPck;
			while (true)
			{
				if (pckIt->_next)
				{
					assert(stackl < 15);
					pckIt = pckIt->_next;
					uStack[stackl++] = pckIt;
					pckIt->lock(this);
				}
				else
				{
					actor_handle r;
					if (!pckIt->is_closed())
					{
						r = pckIt->_hostActor->shared_from_this();
					}
					while (stackl)
					{
						uStack[--stackl]->unlock(this);
					}
					msgPck->unlock(this);
					return r;
				}
			}
		}
		return actor_handle();
	}

	template <typename T0, typename T1, typename T2>
	actor_handle msg_agent_handle(const actor_handle& buddyActor)
	{
		return msg_agent_handle<T0, T1, T2, void>(buddyActor);
	}

	template <typename T0, typename T1>
	actor_handle msg_agent_handle(const actor_handle& buddyActor)
	{
		return msg_agent_handle<T0, T1, void, void>(buddyActor);
	}

	template <typename T0>
	actor_handle msg_agent_handle(const actor_handle& buddyActor)
	{
		return msg_agent_handle<T0, void, void, void>(buddyActor);
	}

	actor_handle msg_agent_handle(const actor_handle& buddyActor);
public:
	/*!
	@brief ��ȡ��ǰ�����������ĸ�Actor��
	@return ��������Actor�»�û���� CHECK_SELF ����NULL
	*/
	static my_actor* self_actor();

	/*!
	@brief ��⵱ǰ�Ƿ��ڸ�Actor������
	*/
	void check_self();

	/*!
	@brief ���Ե�ǰ�µ�Actorջ�Ƿ�ȫ
	*/
	void check_stack();

	/*!
	@brief ��ȡ��ǰActorʣ�లȫջ�ռ�
	*/
	size_t stack_free_space();

	/*!
	@brief ��ȡ��ǰActor������
	*/
	shared_strand self_strand();

	/*!
	@brief ���ر����������ָ��
	*/
	actor_handle shared_from_this();

	/*!
	@brief ��ȡ��ǰActorID��
	*/
	id self_id();

	/*!
	@brief ��ȡActor�л�����
	*/
	size_t yield_count();

	/*!
	@brief Actor�л���������
	*/
	void reset_yield();

	/*!
	@brief ��ʼ���н����õ�Actor
	*/
	void notify_run();

	/*!
	@brief ǿ���˳���Actor���������ã��п��ܻ������Դй©
	*/
	void notify_quit();

	/*!
	@brief ǿ���˳���Actor����ɺ�ص�
	*/
	void notify_quit(const std::function<void (bool)>& h);

	/*!
	@brief Actor�Ƿ��Ѿ���ʼ����
	*/
	bool is_started();

	/*!
	@brief Actor�Ƿ��Ѿ��˳�
	*/
	bool is_quited();

	/*!
	@brief �Ƿ���Actor��
	*/
	bool in_actor();

	/*!
	@brief lock_quit�����Ƿ��յ��˳���Ϣ
	*/
	bool quit_msg();

	/*!
	@brief ������ǰActor����ʱ����ǿ���˳�
	*/
	void lock_quit();

	/*!
	@brief ����˳�����
	*/
	void unlock_quit();

	/*!
	@brief ��ͣActor
	*/
	void notify_suspend();
	void notify_suspend(const std::function<void ()>& h);

	/*!
	@brief �ָ�����ͣActor
	*/
	void notify_resume();
	void notify_resume(const std::function<void ()>& h);

	/*!
	@brief �л�����/�ǹ���״̬
	*/
	void switch_pause_play();
	void switch_pause_play(const std::function<void (bool isPaused)>& h);

	/*!
	@brief �ȴ�Actor�˳�����Actor��������ios�޹��߳���ʹ��
	*/
	bool outside_wait_quit();

	/*!
	@brief ���һ��Actor�����ص�
	*/
	void append_quit_callback(const std::function<void (bool)>& h);

	/*!
	@brief ����һ��Actor
	*/
	void actors_start_run(const list<actor_handle>& anotherActors);

	/*!
	@brief ǿ���˳���һ��Actor�����ҵȴ����
	*/
	__yield_interrupt bool actor_force_quit(actor_handle anotherActor);
	__yield_interrupt void actors_force_quit(const list<actor_handle>& anotherActors);

	/*!
	@brief �ȴ���һ��Actor�����󷵻�
	*/
	__yield_interrupt bool actor_wait_quit(actor_handle anotherActor);
	__yield_interrupt void actors_wait_quit(const list<actor_handle>& anotherActors);
	__yield_interrupt bool timed_actor_wait_quit(int tm, actor_handle anotherActor);

	/*!
	@brief ������һ��Actor���ȴ���������Actor�����ú�ŷ���
	*/
	__yield_interrupt void actor_suspend(actor_handle anotherActor);
	__yield_interrupt void actors_suspend(const list<actor_handle>& anotherActors);

	/*!
	@brief �ָ���һ��Actor���ȴ���������Actor�����ú�ŷ���
	*/
	__yield_interrupt void actor_resume(actor_handle anotherActor);
	__yield_interrupt void actors_resume(const list<actor_handle>& anotherActors);

	/*!
	@brief ����һ��Actor���й���/�ָ�״̬�л�
	@return ���ѹ��𷵻�true������false
	*/
	__yield_interrupt bool actor_switch(actor_handle anotherActor);
	__yield_interrupt bool actors_switch(const list<actor_handle>& anotherActors);

	void assert_enter();
private:
	void time_out(int ms, const std::function<void ()>& h);
	void expires_timer();
	void cancel_timer();
	void suspend_timer();
	void resume_timer();
	void start_run();
	void force_quit(const std::function<void (bool)>& h);
	void suspend(const std::function<void ()>& h);
	void resume(const std::function<void ()>& h);
	void suspend();
	void resume();
	void run_one();
	void pull_yield();
	void pull_yield_as_mutex();
	void push_yield();
	void push_yield_as_mutex();
	void force_quit_cb_handler();
	void exit_callback();
	void child_suspend_cb_handler();
	void child_resume_cb_handler();
private:
#ifdef _DEBUG
	list<stack_line_info> _createStack;//��ǰActor����ʱ�ĵ��ö�ջ
#endif
	void* _actorPull;///<Actor�жϵ�ָ�
	void* _actorPush;///<Actor�жϵ�
	void* _stackTop;///<Actorջ��
	id _selfID;///<ActorID
	size_t _stackSize;///<Actorջ��С
	shared_strand _strand;///<Actor������
	bool _inActor;///<��ǰ����Actor�ڲ�ִ�б��
	bool _started;///<�Ѿ���ʼ���еı��
	bool _quited;///<_mainFunc�Ѿ�ִ�����
	bool _exited;///<��ȫ�˳�
	bool _suspended;///<Actor������
	bool _hasNotify;///<��ǰActor�������ⲿ����׼������Actor���
	bool _isForce;///<�Ƿ���ǿ���˳��ı�ǣ��ɹ�������force_quit
	bool _notifyQuited;///<��ǰActor���������յ��˳���Ϣ
	size_t _lockQuit;///<������ǰActor�������ǰ���յ��˳���Ϣ����ʱ���ˣ��ȵ��������˳�
	size_t _yieldCount;//yield����
	size_t _childOverCount;///<��Actor�˳�ʱ����
	size_t _childSuspendResumeCount;///<��Actor����/�ָ�����
	size_t _timerCount;//��ʱ������
	std::weak_ptr<my_actor> _parentActor;///<��Actor
	main_func _mainFunc;///<Actor���
	list<suspend_resume_option> _suspendResumeQueue;///<����/�ָ���������
	list<actor_handle> _childActorList;///<��Actor����
	list<std::function<void (bool)> > _exitCallback;///<Actor������Ļص�������ǿ���˳�����false�������˳�����true
	list<std::function<void ()> > _quitHandlerList;///<Actor�˳�ʱǿ�Ƶ��õĺ�������ע�����ִ��
	msg_pool_status _msgPoolStatus;//��Ϣ���б�
	timer_pck* _timer;///<��ʱ��
	std::weak_ptr<my_actor> _weakThis;
	static std::shared_ptr<shared_obj_pool_base<bool>> _sharedBoolPool;
#ifdef CHECK_SELF
	map<void*, my_actor*>::iterator _btIt;
	map<void*, my_actor*>::iterator _topIt;
#endif
};

#endif
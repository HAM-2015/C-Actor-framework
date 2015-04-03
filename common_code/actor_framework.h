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

#include <boost/circular_buffer.hpp>
#include <list>
#include <xutility>
#include <functional>
#include "ios_proxy.h"
#include "shared_strand.h"
#include "wrapped_trig_handler.h"
#include "ref_ex.h"
#include "function_type.h"

class my_actor;
typedef std::shared_ptr<my_actor> actor_handle;//Actor���

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

#ifdef _DEBUG
#define DEBUG_OPERATION(__exp__)	__exp__
#else
#define DEBUG_OPERATION(__exp__)
#endif

//Ĭ�϶�ջ��С64k
#define kB	*1024
#define DEFAULT_STACKSIZE	64 kB

template <typename T0, typename T1 = void, typename T2 = void, typename T3 = void>
struct msg_param
{
	typedef std::function<void (T0, T1, T2, T3)> overflow_notify;
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

	static void proxy_notify(const overflow_notify& nfy, const msg_param& p)
	{
		nfy(p._res0, p._res1, p._res2, p._res3);
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
	typedef std::function<void (T0, T1, T2)> overflow_notify;
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

	static void proxy_notify(const overflow_notify& nfy, const msg_param& p)
	{
		nfy(p._res0, p._res1, p._res2);
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
	typedef std::function<void (T0, T1)> overflow_notify;
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

	static void proxy_notify(const overflow_notify& nfy, const msg_param& p)
	{
		nfy(p._res0, p._res1);
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
	typedef std::function<void (T0)> overflow_notify;
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

	static void proxy_notify(const overflow_notify& nfy, const msg_param& p)
	{
		nfy(p._res0);
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

class param_list_base
{
	friend my_actor;
private:
	param_list_base(const param_list_base&);
	param_list_base& operator =(const param_list_base&);
protected:
	param_list_base();
	virtual ~param_list_base();
	bool closed();
	bool empty();
	void begin(long long actorID);
	virtual size_t length() = 0;
	virtual void close() = 0;
	virtual void clear() = 0;
protected:
	std::shared_ptr<bool> _pIsClosed;
	bool _waiting;
	bool _timeout;
	bool _hasTm;
	DEBUG_OPERATION(long long _actorID);
};

template <typename T /*msg_param*/>
class param_list: public param_list_base
{
	friend my_actor;
	typedef typename T::ref_type ref_type;
	typedef typename T::const_ref_type const_ref_type;
public:
	virtual ~param_list() {}
protected:
	param_list(): _dstRefPt(NULL) {}
	virtual size_t move_to_back(T& src) = 0;
	virtual T* front() = 0;
	virtual size_t pop_front() = 0;

	void move_from(ref_type& src)
	{
		assert(!(*_pIsClosed) && _dstRefPt);
		_dstRefPt->move_from(src);
		DEBUG_OPERATION(_dstRefPt = NULL);
	}

	void save_from(const_ref_type& src)
	{
		assert(!(*_pIsClosed) && _dstRefPt);
		_dstRefPt->save_from(src);
		DEBUG_OPERATION(_dstRefPt = NULL);
	}

	void set_dst_ref(ref_type& dstRef)
	{
		_dstRefPt = &dstRef;
	}
protected:
	ref_type* _dstRefPt;
};

template <typename T /*msg_param*/>
class param_list_no_bounded: public param_list<T>
{
	friend my_actor;
public:
	size_t length()
	{
		return _params.size();
	}

	void clear()
	{
		_params.clear();
	}
protected:
	void close()
	{
		_params.clear();
		if (_pIsClosed)
		{
			assert(!(*_pIsClosed));
			(*_pIsClosed) = true;
			_waiting = false;
			_pIsClosed.reset();
		}
	}

	size_t move_to_back(T& src)
	{
		assert(!(*_pIsClosed));
		_params.push_back(std::move(src));
		return _params.size();
	}

	T* front()
	{
		assert(!(*_pIsClosed));
		if (!_params.empty())
		{
			return &_params.front();
		}
		return NULL;
	}

	size_t pop_front()
	{
		assert(!(*_pIsClosed));
		assert(!_params.empty());
		_params.pop_front();
		return _params.size();
	}
private:
	list<T> _params;
};

template <typename T /*msg_param*/>
class param_list_bounded: public param_list<T>
{
	friend my_actor;
public:
	size_t length()
	{
		return _params.size();
	}

	void clear()
	{
		_params.clear();
	}
protected:
	typedef typename T::overflow_notify overflow_notify;

	param_list_bounded(size_t maxBuff, const overflow_notify& ofh)
		:_params(maxBuff)
	{
		_ofh = ofh;
	}

	void close()
	{
		_params.clear();
		if (_pIsClosed)
		{
			assert(!(*_pIsClosed));
			(*_pIsClosed) = true;
			_waiting = false;
			_pIsClosed.reset();
			_ofh.clear();
		}
	}

	size_t move_to_back(T& src)
	{
		assert(!(*_pIsClosed));
		if (_params.size() < _params.capacity())
		{
			_params.push_back(std::move(src));
			return _params.size();
		}
		if (!_ofh)
		{
			_params.pop_front();
			_params.push_back(std::move(src));
			return _params.size();
		} 
		T::proxy_notify(_ofh, src);
		return 0;
	}

	T* front()
	{
		assert(!(*_pIsClosed));
		if (!_params.empty())
		{
			return &_params.front();
		}
		return NULL;
	}

	size_t pop_front()
	{
		assert(!(*_pIsClosed));
		assert(!_params.empty());
		_params.pop_front();
		return _params.size();
	}
private:
	overflow_notify _ofh;
	boost::circular_buffer<T> _params;
};

class null_param_list: public param_list_base
{
	friend my_actor;
public:
	size_t length();

	void clear();
protected:
	null_param_list();

	void close();

	size_t count;
};

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class actor_msg_handle: public param_list_no_bounded<msg_param<T0, T1, T2, T3> >
{
	friend my_actor;
public:
	typedef std::shared_ptr<actor_msg_handle<T0, T1, T2, T3> > ptr;

	static std::shared_ptr<actor_msg_handle<T0, T1, T2, T3> > make_ptr()
	{
		return std::shared_ptr<actor_msg_handle<T0, T1, T2, T3> >(new actor_msg_handle<T0, T1, T2, T3>);
	}
};

template <typename T0, typename T1, typename T2>
class actor_msg_handle<T0, T1, T2, void>: public param_list_no_bounded<msg_param<T0, T1, T2> >
{
	friend my_actor;
public:
	typedef std::shared_ptr<actor_msg_handle<T0, T1, T2> > ptr;

	static std::shared_ptr<actor_msg_handle<T0, T1, T2> > make_ptr()
	{
		return std::shared_ptr<actor_msg_handle<T0, T1, T2> >(new actor_msg_handle<T0, T1, T2>);
	}
};

template <typename T0, typename T1>
class actor_msg_handle<T0, T1, void, void>: public param_list_no_bounded<msg_param<T0, T1> >
{
	friend my_actor;
public:
	typedef std::shared_ptr<actor_msg_handle<T0, T1> > ptr;

	static std::shared_ptr<actor_msg_handle<T0, T1> > make_ptr()
	{
		return std::shared_ptr<actor_msg_handle<T0, T1> >(new actor_msg_handle<T0, T1>);
	}
};

template <typename T0>
class actor_msg_handle<T0, void, void, void>: public param_list_no_bounded<msg_param<T0> >
{
	friend my_actor;
public:
	typedef std::shared_ptr<actor_msg_handle<T0> > ptr;

	static std::shared_ptr<actor_msg_handle<T0> > make_ptr()
	{
		return std::shared_ptr<actor_msg_handle<T0> >(new actor_msg_handle<T0>);
	}
};

template <>
class actor_msg_handle<void, void, void, void>: public null_param_list
{
	friend my_actor;
public:
	typedef std::shared_ptr<actor_msg_handle<> > ptr;

	static std::shared_ptr<actor_msg_handle<> > make_ptr()
	{
		return std::shared_ptr<actor_msg_handle<> >(new actor_msg_handle<>);
	}
};
//////////////////////////////////////////////////////////////////////////
template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class actor_msg_bounded_handle: public param_list_bounded<msg_param<T0, T1, T2, T3> >
{
	friend my_actor;
public:
	typedef typename msg_param<T0, T1, T2, T3>::overflow_notify overflow_notify;
	typedef std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2, T3> > ptr;

	/*!
	@param maxBuff ��󻺴����
	@param ofh �����������
	*/
	actor_msg_bounded_handle(int maxBuff,  const overflow_notify& ofh)
		:param_list_bounded<msg_param<T0, T1, T2, T3> >(maxBuff, ofh)
	{

	}

	static std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2, T3> > make_ptr(int maxBuff,  const overflow_notify& ofh)
	{
		return std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2, T3> >(new actor_msg_bounded_handle<T0, T1, T2, T3>(maxBuff, ofh));
	}
};

template <typename T0, typename T1, typename T2>
class actor_msg_bounded_handle<T0, T1, T2, void>: public param_list_bounded<msg_param<T0, T1, T2> >
{
	friend my_actor;
public:
	typedef typename msg_param<T0, T1, T2>::overflow_notify overflow_notify;
	typedef std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2> > ptr;

	actor_msg_bounded_handle(int maxBuff,  const overflow_notify& ofh)
		:param_list_bounded<msg_param<T0, T1, T2> >(maxBuff, ofh)
	{

	}

	static std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2> > make_ptr(int maxBuff,  const overflow_notify& ofh)
	{
		return std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2> >(new actor_msg_bounded_handle<T0, T1, T2>(maxBuff, ofh));
	}
};

template <typename T0, typename T1>
class actor_msg_bounded_handle<T0, T1, void, void>: public param_list_bounded<msg_param<T0, T1> >
{
	friend my_actor;
public:
	typedef typename msg_param<T0, T1>::overflow_notify overflow_notify;
	typedef std::shared_ptr<actor_msg_bounded_handle<T0, T1> > ptr;

	actor_msg_bounded_handle(int maxBuff,  const overflow_notify& ofh)
		:param_list_bounded<msg_param<T0, T1> >(maxBuff, ofh)
	{

	}

	static std::shared_ptr<actor_msg_bounded_handle<T0, T1> > make_ptr(int maxBuff,  const overflow_notify& ofh)
	{
		return std::shared_ptr<actor_msg_bounded_handle<T0, T1> >(new actor_msg_bounded_handle<T0, T1>(maxBuff, ofh));
	}
};

template <typename T0>
class actor_msg_bounded_handle<T0, void, void, void>: public param_list_bounded<msg_param<T0> >
{
	friend my_actor;
public:
	typedef typename msg_param<T0>::overflow_notify overflow_notify;
	typedef std::shared_ptr<actor_msg_bounded_handle<T0> > ptr;

	actor_msg_bounded_handle(int maxBuff,  const overflow_notify& ofh)
		:param_list_bounded<msg_param<T0> >(maxBuff, ofh)
	{

	}

	static std::shared_ptr<actor_msg_bounded_handle<T0> > make_ptr(int maxBuff,  const overflow_notify& ofh)
	{
		return std::shared_ptr<actor_msg_bounded_handle<T0> >(new actor_msg_bounded_handle<T0>(maxBuff, ofh));
	}
};

class async_trig_base
{
	friend my_actor;
public:
	async_trig_base();
	virtual ~async_trig_base();
public:
	bool has_trig();
private:
	void begin(long long actorID);
	void close();
	virtual void move_out(void* dst) = 0;
	virtual void save_from(void* src) = 0;
	virtual void move_from(void* src) = 0;
	virtual void save_to_temp(void* src) = 0;
	virtual void move_to_temp(void* src) = 0;
	async_trig_base(const async_trig_base&);
	async_trig_base& operator=(const async_trig_base&);
protected:
	std::shared_ptr<bool> _pIsClosed;
	bool _notify;
	bool _waiting;
	bool _timeout;
	bool _hasTm;
	void* _dstRefPt;
	DEBUG_OPERATION(long long _actorID);
};

template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class async_trig_handle: public async_trig_base
{
	friend my_actor;
	typedef ref_ex<T0, T1, T2, T3> ref_type;
	typedef const_ref_ex<T0, T1, T2, T3> const_ref_type;
	typedef msg_param<T0, T1, T2, T3> msg_type;
public:
	typedef std::shared_ptr<async_trig_handle<T0, T1, T2, T3> > ptr;

	static std::shared_ptr<async_trig_handle<T0, T1, T2, T3> > make_ptr()
	{
		return std::shared_ptr<async_trig_handle<T0, T1, T2, T3> >(new async_trig_handle<T0, T1, T2, T3>);
	}

	void move_out(void* dst)
	{
		_temp.move_out(*(ref_type*)dst);
	}

	void save_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->save_from(*(const_ref_type*)src);
	}

	void move_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->move_from(*(ref_type*)src);
	}

	void save_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.save_from(*(const_ref_type*)src);
	}

	void move_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.move_from(*(ref_type*)src);
	}
private:
	msg_type _temp;
};

template <typename T0, typename T1, typename T2>
class async_trig_handle<T0, T1, T2, void>: public async_trig_base
{
	friend my_actor;
	typedef ref_ex<T0, T1, T2> ref_type;
	typedef const_ref_ex<T0, T1, T2> const_ref_type;
	typedef msg_param<T0, T1, T2> msg_type;
public:
	typedef std::shared_ptr<async_trig_handle<T0, T1, T2> > ptr;

	static std::shared_ptr<async_trig_handle<T0, T1, T2> > make_ptr()
	{
		return std::shared_ptr<async_trig_handle<T0, T1, T2> >(new async_trig_handle<T0, T1, T2>);
	}

	void move_out(void* dst)
	{
		_temp.move_out(*(ref_type*)dst);
	}

	void save_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->save_from(*(const_ref_type*)src);
	}

	void move_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->move_from(*(ref_type*)src);
	}

	void save_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.save_from(*(const_ref_type*)src);
	}

	void move_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.move_from(*(ref_type*)src);
	}
private:
	msg_type _temp;
};

template <typename T0, typename T1>
class async_trig_handle<T0, T1, void, void>: public async_trig_base
{
	friend my_actor;
	typedef ref_ex<T0, T1> ref_type;
	typedef const_ref_ex<T0, T1> const_ref_type;
	typedef msg_param<T0, T1> msg_type;
public:
	typedef std::shared_ptr<async_trig_handle<T0, T1> > ptr;

	static std::shared_ptr<async_trig_handle<T0, T1> > make_ptr()
	{
		return std::shared_ptr<async_trig_handle<T0, T1> >(new async_trig_handle<T0, T1>);
	}

	void move_out(void* dst)
	{
		_temp.move_out(*(ref_type*)dst);
	}

	void save_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->save_from(*(const_ref_type*)src);
	}

	void move_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->move_from(*(ref_type*)src);
	}

	void save_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.save_from(*(const_ref_type*)src);
	}

	void move_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.move_from(*(ref_type*)src);
	}
private:
	msg_type _temp;
};

template <typename T0>
class async_trig_handle<T0, void, void, void>: public async_trig_base
{
	friend my_actor;
	typedef ref_ex<T0> ref_type;
	typedef const_ref_ex<T0> const_ref_type;
	typedef msg_param<T0> msg_type;
public:
	typedef std::shared_ptr<async_trig_handle<T0> > ptr;

	static std::shared_ptr<async_trig_handle<T0> > make_ptr()
	{
		return std::shared_ptr<async_trig_handle<T0> >(new async_trig_handle<T0>);
	}

	void move_out(void* dst)
	{
		_temp.move_out(*(ref_type*)dst);
	}

	void save_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->save_from(*(const_ref_type*)src);
	}

	void move_from(void* src)
	{
		assert(_dstRefPt);
		((ref_type*)_dstRefPt)->move_from(*(ref_type*)src);
	}

	void save_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.save_from(*(const_ref_type*)src);
	}

	void move_to_temp(void* src)
	{
		assert(!_waiting);
		_temp.move_from(*(ref_type*)src);
	}
private:
	msg_type _temp;
};

template <>
class async_trig_handle<void, void, void, void>: public async_trig_base
{
	friend my_actor;
public:
	typedef std::shared_ptr<async_trig_handle<> > ptr;

	static std::shared_ptr<async_trig_handle<> > make_ptr()
	{
		return std::shared_ptr<async_trig_handle<> >(new async_trig_handle<>);
	}

	void move_out(void* dst)
	{
		assert(!dst);
	}

	void save_from(void* src)
	{
		assert(!src);
	}

	void move_from(void* src)
	{
		assert(!src);
	}

	void save_to_temp(void* src)
	{
		assert(!_waiting);
		assert(!src);
	}

	void move_to_temp(void* src)
	{
		assert(!_waiting);
		assert(!src);
	}
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

	struct msg_pump_status 
	{
		struct pck_base
		{
			size_t _size;
			virtual ~pck_base() {};
		};

		template <typename T0, typename T1, typename T2, typename T3>
		struct pck: public pck_base
		{
			typedef typename func_type<T0, T1, T2, T3>::result notifer_type;

			pck()
			{
				_size = sizeof(pck<T0, T1, T2, T3>);
			}

			actor_msg_handle<T0, T1, T2, T3> _handle;
			notifer_type _notifer;
		};

		void clear()
		{
			for (int i = 0; i < 5; i++)
			{
				_msgPumpList[i].clear();
			}
		}

		list<std::shared_ptr<pck_base> > _msgPumpList[5];
	};

	struct timer_pck;
	class boost_actor_run;
	friend boost_actor_run;
	friend child_actor_handle;
public:
	/*!
	@brief Actor��ǿ���˳����쳣����
	*/
	struct force_quit_exception { };

	/*!
	@brief Actor��ں�����
	*/
	typedef std::function<void (my_actor*)> main_func;
private:
	my_actor();
	my_actor(const my_actor&);
	my_actor& operator =(const my_actor&);
public:
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
	@brief ͬ�ϣ������Actor��Ļص�֪ͨ
	@param cb Actor��ɺ�Ĵ���������falseǿ�ƽ����ģ�true��������
	*/
	static actor_handle create(shared_strand actorStrand, const main_func& mainFunc, 
		const std::function<void (bool)>& cb, size_t stackSize = DEFAULT_STACKSIZE);

	/*!
	@brief �첽����һ��Actor�������ɹ���ͨ���ص�����֪ͨ
	*/
	static void async_create(shared_strand actorStrand, const main_func& mainFunc, 
		const std::function<void (actor_handle)>& ch, size_t stackSize = DEFAULT_STACKSIZE);

	/*!
	@brief ͬ�ϣ������Actor��Ļص�֪ͨ
	*/
	static void async_create(shared_strand actorStrand, const main_func& mainFunc, 
		const std::function<void (actor_handle)>& ch, const std::function<void (bool)>& cb, size_t stackSize = DEFAULT_STACKSIZE);

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
	@brief �������Actor
	*/
	actor_handle child_actor_peel(child_actor_handle& actorHandle);

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
	@brief ����disable_auto_make_timer��ʹ������򿪵�ǰActor��ʱ��
	*/
	void open_timer();

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
	@brief ����һ���첽����������ʹ��timed_wait_trig()�ȴ�
	@param th �������
	@return �����������󣬿��Զ�ε���(�̰߳�ȫ)����ֻ��timed_wait_trig()����һ�ε��õ�ֵ
	*/
	std::function<void ()> begin_trig(async_trig_handle<>& th);
	std::function<void ()> begin_trig(const std::shared_ptr<async_trig_handle<> >& th);

	template <typename T0>
	std::function<void (T0)> begin_trig(async_trig_handle<T0>& th)
	{
		th.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		return [=, &th](const T0& p0)
		{shared_this->async_trig_handler(pIsClosed_, th, p0); };
	}

	template <typename T0>
	std::function<void (T0)> begin_trig(const std::shared_ptr<async_trig_handle<T0> >& th)
	{
		th->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		return [=](const T0& p0)
		{shared_this->async_trig_handler_ptr(pIsClosed_, th, p0); };
	}

	template <typename T0, typename T1>
	std::function<void (T0, T1)> begin_trig(async_trig_handle<T0, T1>& th)
	{
		th.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		return [=, &th](const T0& p0, const T1& p1)
		{shared_this->async_trig_handler(pIsClosed_, th, p0, p1); };
	}

	template <typename T0, typename T1>
	std::function<void (T0, T1)> begin_trig(const std::shared_ptr<async_trig_handle<T0, T1> >& th)
	{
		th->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		return [=](const T0& p0, const T1& p1)
		{shared_this->async_trig_handler_ptr(pIsClosed_, th, p0, p1); };
	}

	template <typename T0, typename T1, typename T2>
	std::function<void (T0, T1, T2)> begin_trig(async_trig_handle<T0, T1, T2>& th)
	{
		th.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		return [=, &th](const T0& p0, const T1& p1, const T2& p2)
		{shared_this->async_trig_handler(pIsClosed_, th, p0, p1, p2); };
	}

	template <typename T0, typename T1, typename T2>
	std::function<void (T0, T1, T2)> begin_trig(const std::shared_ptr<async_trig_handle<T0, T1, T2> >& th)
	{
		th->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		return [=](const T0& p0, const T1& p1, const T2& p2)
		{shared_this->async_trig_handler_ptr(pIsClosed_, th, p0, p1, p2); };
	}

	template <typename T0, typename T1, typename T2, typename T3>
	std::function<void (T0, T1, T2, T3)> begin_trig(async_trig_handle<T0, T1, T2, T3>& th)
	{
		th.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		return [=, &th](const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		{shared_this->async_trig_handler(pIsClosed_, th, p0, p1, p2, p3); };
	}

	template <typename T0, typename T1, typename T2, typename T3>
	std::function<void (T0, T1, T2, T3)> begin_trig(const std::shared_ptr<async_trig_handle<T0, T1, T2, T3> >& th)
	{
		th->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		return [=](const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		{shared_this->async_trig_handler_ptr(pIsClosed_, th, p0, p1, p2, p3); };
	}

	/*!
	@brief �ȴ�begin_trig�����Ļص����
	@param th �첽���
	@param tm �첽�ȴ���ʱms����ʱ�󷵻�false
	@return ��ʱ�󷵻�false
	*/
	__yield_interrupt bool timed_wait_trig(async_trig_handle<>& th, int tm);
	__yield_interrupt void wait_trig(async_trig_handle<>& th);
	__yield_interrupt bool timed_wait_trig(const std::shared_ptr<async_trig_handle<> >& th, int tm);
	__yield_interrupt void wait_trig(const std::shared_ptr<async_trig_handle<> >& th);
	//////////////////////////////////////////////////////////////////////////

	template <typename T0>
	__yield_interrupt bool timed_wait_trig(async_trig_handle<T0>& th, T0& r0, int tm)
	{
		assert(th._actorID == _actorID);
		assert_enter();
		ref_ex<T0> dst(r0);
		if (async_trig_push(th, tm, &dst))
		{
			close_trig(th);
			return true;
		}
		return false;
	}

	template <typename T0>
	__yield_interrupt void wait_trig(async_trig_handle<T0>& th, T0& r0)
	{
		timed_wait_trig(th, r0, -1);
	}

	template <typename T0>
	__yield_interrupt T0 wait_trig(async_trig_handle<T0>& th)
	{
		T0 r;
		timed_wait_trig(th, r, -1);
		return r;
	}

	template <typename T0>
	__yield_interrupt bool timed_wait_trig(const std::shared_ptr<async_trig_handle<T0> >& th, T0& r0, int tm)
	{
		assert(th);
		auto lockTh = th;
		return timed_wait_trig(*th, r0, tm);
	}

	template <typename T0>
	__yield_interrupt void wait_trig(const std::shared_ptr<async_trig_handle<T0> >& th, T0& r0)
	{
		assert(th);
		auto lockTh = th;
		timed_wait_trig(*th, r0, -1);
	}

	template <typename T0>
	__yield_interrupt T0 wait_trig(const std::shared_ptr<async_trig_handle<T0> >& th)
	{
		assert(th);
		auto lockTh = th;
		return wait_trig(*th);
	}
	//////////////////////////////////////////////////////////////////////////
	template <typename T0, typename T1>
	__yield_interrupt bool timed_wait_trig(async_trig_handle<T0, T1>& th, T0& r0, T1& r1, int tm)
	{
		assert(th._actorID == _actorID);
		assert_enter();
		ref_ex<T0, T1> dst(r0, r1);
		if (async_trig_push(th, tm, &dst))
		{
			close_trig(th);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1>
	__yield_interrupt void wait_trig(async_trig_handle<T0, T1>& th, T0& r0, T1& r1)
	{
		timed_wait_trig(th, r0, r1, -1);
	}

	template <typename T0, typename T1>
	__yield_interrupt bool timed_wait_trig(const std::shared_ptr<async_trig_handle<T0, T1> >& th, T0& r0, T1& r1, int tm)
	{
		assert(th);
		auto lockTh = th;
		return timed_wait_trig(*th, r0, r1, tm);
	}

	template <typename T0, typename T1>
	__yield_interrupt void wait_trig(const std::shared_ptr<async_trig_handle<T0, T1> >& th, T0& r0, T1& r1)
	{
		assert(th);
		auto lockTh = th;
		timed_wait_trig(*th, r0, r1, -1);
	}
	//////////////////////////////////////////////////////////////////////////
	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_wait_trig(async_trig_handle<T0, T1, T2>& th, T0& r0, T1& r1, T2& r2, int tm)
	{
		assert(th._actorID == _actorID);
		assert_enter();
		ref_ex<T0, T1, T2> dst(r0, r1, r2);
		if (async_trig_push(th, tm, &dst))
		{
			close_trig(th);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void wait_trig(async_trig_handle<T0, T1, T2>& th, T0& r0, T1& r1, T2& r2)
	{
		timed_wait_trig(th, r0, r1, r2, -1);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_wait_trig(const std::shared_ptr<async_trig_handle<T0, T1, T2> >& th, T0& r0, T1& r1, T2& r2, int tm)
	{
		assert(th);
		auto lockTh = th;
		return timed_wait_trig(*th, r0, r1, r2, tm);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void wait_trig(const std::shared_ptr<async_trig_handle<T0, T1, T2> >& th, T0& r0, T1& r1, T2& r2)
	{
		assert(th);
		auto lockTh = th;
		timed_wait_trig(*th, r0, r1, r2, -1);
	}
	//////////////////////////////////////////////////////////////////////////
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_wait_trig(async_trig_handle<T0, T1, T2, T3>& th, T0& r0, T1& r1, T2& r2, T3& r3, int tm)
	{
		assert(th._actorID == _actorID);
		assert_enter();
		ref_ex<T0, T1, T2, T3> dst(r0, r1, r2, r3);
		if (async_trig_push(th, tm, &dst))
		{
			close_trig(th);
			return true;
		}
		return false;
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void wait_trig(async_trig_handle<T0, T1, T2, T3>& th, T0& r0, T1& r1, T2& r2, T3& r3)
	{
		timed_wait_trig(th, r0, r1, r2, r3, -1);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_wait_trig(const std::shared_ptr<async_trig_handle<T0, T1, T2, T3> >& th, T0& r0, T1& r1, T2& r2, T3& r3, int tm)
	{
		assert(th);
		auto lockTh = th;
		return timed_wait_trig(*th, r0, r1, r2, r3, tm);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void wait_trig(const std::shared_ptr<async_trig_handle<T0, T1, T2, T3> >& th, T0& r0, T1& r1, T2& r2, T3& r3)
	{
		assert(th);
		auto lockTh = th;
		timed_wait_trig(*th, r0, r1, r2, r3, -1);
	}

	/*!
	@brief �رմ������
	*/
	void close_trig(async_trig_base& th);

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
	@brief ʹ���ڲ���ʱ����ʱ�����첽�����ʹ��֮ǰ�����Ѿ�������begin_trig(async_trig_handle)���ڴ������֮ǰ���ܶ�ε���
	@param ms ������ʱ(����)
	@param th �첽�������
	*/
	void delay_trig(int ms, async_trig_handle<>& th);
	void delay_trig(int ms, std::shared_ptr<async_trig_handle<> >& th);

	template <typename T0>
	void delay_trig(int ms, async_trig_handle<T0>& th, const T0& p0)
	{
		assert_enter();
		assert(th._pIsClosed);
		assert(_timer);
		assert(th._actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		delay_trig(ms, [=, &th]()
		{shared_this->_async_trig_handler(pIsClosed_, th, ref_ex<T0>((T0&)p0)); });
	}

	template <typename T0>
	void delay_trig(int ms, const std::shared_ptr<async_trig_handle<T0> >& th, const T0& p0)
	{
		assert_enter();
		assert(th->_pIsClosed);
		assert(_timer);
		assert(th->_actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		delay_trig(ms, [=]()
		{shared_this->_async_trig_handler(pIsClosed_, *th, ref_ex<T0>((T0&)p0)); });
	}

	template <typename T0, typename T1>
	void delay_trig(int ms, async_trig_handle<T0, T1>& th, const T0& p0, const T1& p1)
	{
		assert_enter();
		assert(th._pIsClosed);
		assert(_timer);
		assert(th._actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		delay_trig(ms, [=, &th]()
		{shared_this->_async_trig_handler(pIsClosed_, th, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
	}

	template <typename T0, typename T1>
	void delay_trig(int ms, const std::shared_ptr<async_trig_handle<T0, T1> >& th, const T0& p0, const T1& p1)
	{
		assert_enter();
		assert(th->_pIsClosed);
		assert(_timer);
		assert(th->_actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		delay_trig(ms, [=]()
		{shared_this->_async_trig_handler(pIsClosed_, *th, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
	}

	template <typename T0, typename T1, typename T2>
	void delay_trig(int ms, async_trig_handle<T0, T1, T2>& th, const T0& p0, const T1& p1, const T2& p2)
	{
		assert_enter();
		assert(th._pIsClosed);
		assert(_timer);
		assert(th._actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		delay_trig(ms, [=, &th]()
		{shared_this->_async_trig_handler(pIsClosed_, th, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
	}

	template <typename T0, typename T1, typename T2>
	void delay_trig(int ms, const std::shared_ptr<async_trig_handle<T0, T1, T2> >& th, const T0& p0, const T1& p1, const T2& p2)
	{
		assert_enter();
		assert(th->_pIsClosed);
		assert(_timer);
		assert(th->_actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		delay_trig(ms, [=]()
		{shared_this->_async_trig_handler(pIsClosed_, *th, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void delay_trig(int ms, async_trig_handle<T0, T1, T2, T3>& th, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		assert_enter();
		assert(th._pIsClosed);
		assert(_timer);
		assert(th._actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th._pIsClosed;
		delay_trig(ms, [=, &th]()
		{shared_this->_async_trig_handler(pIsClosed_, th, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void delay_trig(int ms, const std::shared_ptr<async_trig_handle<T0, T1, T2, T3> >& th, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		assert_enter();
		assert(th->_pIsClosed);
		assert(_timer);
		assert(th->_actorID == _actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = th->_pIsClosed;
		delay_trig(ms, [=]()
		{shared_this->_async_trig_handler(pIsClosed_, *th, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
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
			trig([=](const std::function<void()>& cb){exeStrand->asyncInvokeVoid(h, cb); });
		}
		else
		{
			h();
		}
	}

	template <typename T0, typename H>
	__yield_interrupt T0 send(shared_strand exeStrand, const H& h)
	{
		assert_enter();
		if (exeStrand != _strand)
		{
			return trig<T0>([=](const std::function<void(T0)>& cb){exeStrand->asyncInvoke(h, cb); });
		} 
		return h();
	}

	/*!
	@brief ����һ���첽�������첽�ص���ɺ󷵻�
	*/
	template <typename H>
	__yield_interrupt void trig(const H& h)
	{
		assert_enter();
		actor_handle shared_this = shared_from_this();
#ifdef _DEBUG
		h(wrapped_trig_handler<>::wrap([shared_this](){shared_this->trig_handler(); }));
#else
		h([shared_this](){shared_this->trig_handler(); });
#endif
		push_yield();
	}

	template <typename T0, typename H>
	__yield_interrupt void trig(__out T0& r0, const H& h)
	{
		assert_enter();
		ref_ex<T0> mulRef(r0);
		actor_handle shared_this = shared_from_this();
#ifdef _DEBUG
		h(wrapped_trig_handler<>::wrap([shared_this, &mulRef](const T0& p0){shared_this->trig_handler(mulRef, p0); }));
#else
		h([shared_this, &mulRef](const T0& p0){shared_this->trig_handler(mulRef, p0); });
#endif
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
		ref_ex<T0, T1> mulRef(r0, r1);
		actor_handle shared_this = shared_from_this();
#ifdef _DEBUG
		h(wrapped_trig_handler<>::wrap([shared_this, &mulRef](const T0& p0, const T1& p1){shared_this->trig_handler(mulRef, p0, p1); }));
#else
		h([shared_this, &mulRef](const T0& p0, const T1& p1){shared_this->trig_handler(mulRef, p0, p1); });
#endif
		push_yield();
	}

	template <typename T0, typename T1, typename T2, typename H>
	__yield_interrupt void trig(__out T0& r0, __out T1& r1, __out T2& r2, const H& h)
	{
		assert_enter();
		ref_ex<T0, T1, T2> mulRef(r0, r1, r2);
		actor_handle shared_this = shared_from_this();
#ifdef _DEBUG
		h(wrapped_trig_handler<>::wrap(
			[shared_this, &mulRef](const T0& p0, const T1& p1, const T2& p2){shared_this->trig_handler(mulRef, p0, p1, p2); }));
#else
		h([shared_this, &mulRef](const T0& p0, const T1& p1, const T2& p2){shared_this->trig_handler(mulRef, p0, p1, p2); });
#endif
		push_yield();
	}

	template <typename T0, typename T1, typename T2, typename T3, typename H>
	__yield_interrupt void trig(__out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3, const H& h)
	{
		assert_enter();
		ref_ex<T0, T1, T2, T3> mulRef(r0, r1, r2, r3);
		actor_handle shared_this = shared_from_this();
#ifdef _DEBUG
		h(wrapped_trig_handler<>::wrap(
			[shared_this, &mulRef](const T0& p0, const T1& p1, const T2& p2, const T3& p3){shared_this->trig_handler(mulRef, p0, p1, p2, p3); }));
#else
		h([shared_this, &mulRef](const T0& p0, const T1& p1, const T2& p2, const T3& p3){shared_this->trig_handler(mulRef, p0, p1, p2, p3); });
#endif
		push_yield();
	}
	//////////////////////////////////////////////////////////////////////////
	/*!
	@brief ����һ��"������"������pump_msg"������"ȡ���ص����ݣ�T0-T3�ǻص���������
	@param amh �첽֪ͨ����
	@return �첽��������
	*/
	std::function<void()> make_msg_notifer(actor_msg_handle<>& amh);
	std::function<void()> make_msg_notifer(const std::shared_ptr<actor_msg_handle<> >& amh);

	template <typename T0>
	std::function<void(T0)> make_msg_notifer(param_list<msg_param<T0> >& amh)
	{
		amh.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh._pIsClosed;
		return [=, &amh](const T0& p0)
		{shared_this->notify_handler(pIsClosed_, amh, p0); };
	}

	template <typename T0>
	std::function<void(T0)> make_msg_notifer(const std::shared_ptr<actor_msg_handle<T0> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0> >>(amh), p0); };
	}

	template <typename T0>
	std::function<void(T0)> make_msg_notifer(const std::shared_ptr<actor_msg_bounded_handle<T0> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0> >>(amh), p0); };
	}

	template <typename T0, typename T1>
	std::function<void(T0, T1)> make_msg_notifer(param_list<msg_param<T0, T1> >& amh)
	{
		amh.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh._pIsClosed;
		return [=, &amh](const T0& p0, const T1& p1)
		{shared_this->notify_handler(pIsClosed_, amh, p0, p1); };
	}

	template <typename T0, typename T1>
	std::function<void(T0, T1)> make_msg_notifer(const std::shared_ptr<actor_msg_handle<T0, T1> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0, const T1& p1)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0, T1> >>(amh), p0, p1); };
	}

	template <typename T0, typename T1>
	std::function<void(T0, T1)> make_msg_notifer(const std::shared_ptr<actor_msg_bounded_handle<T0, T1> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0, const T1& p1)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0, T1> >>(amh), p0, p1); };
	}

	template <typename T0, typename T1, typename T2>
	std::function<void(T0, T1, T2)> make_msg_notifer(param_list<msg_param<T0, T1, T2> >& amh)
	{
		amh.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh._pIsClosed;
		return [=, &amh](const T0& p0, const T1& p1, const T2& p2)
		{shared_this->notify_handler(pIsClosed_, amh, p0, p1, p2); };
	}

	template <typename T0, typename T1, typename T2>
	std::function<void(T0, T1, T2)> make_msg_notifer(const std::shared_ptr<actor_msg_handle<T0, T1, T2> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0, const T1& p1, const T2& p2)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0, T1, T2> >>(amh), p0, p1, p2); };
	}

	template <typename T0, typename T1, typename T2>
	std::function<void(T0, T1, T2)> make_msg_notifer(const std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0, const T1& p1, const T2& p2)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0, T1, T2> >>(amh), p0, p1, p2); };
	}

	template <typename T0, typename T1, typename T2, typename T3>
	std::function<void(T0, T1, T2, T3)> make_msg_notifer(param_list<msg_param<T0, T1, T2, T3> >& amh)
	{
		amh.begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh._pIsClosed;
		return [=, &amh](const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		{shared_this->notify_handler(pIsClosed_, amh, p0, p1, p2, p3); };
	}

	template <typename T0, typename T1, typename T2, typename T3>
	std::function<void(T0, T1, T2, T3)> make_msg_notifer(const std::shared_ptr<actor_msg_handle<T0, T1, T2, T3> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0, T1, T2, T3> >>(amh), p0, p1, p2, p3); };
	}

	template <typename T0, typename T1, typename T2, typename T3>
	std::function<void(T0, T1, T2, T3)> make_msg_notifer(const std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2, T3> >& amh)
	{
		amh->begin(_actorID);
		actor_handle shared_this = shared_from_this();
		auto& pIsClosed_ = amh->_pIsClosed;
		return [=](const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		{shared_this->notify_handler_ptr(pIsClosed_, std::static_pointer_cast<param_list<msg_param<T0, T1, T2, T3> >>(amh), p0, p1, p2, p3); };
	}

	/*!
	@brief �ر�make_msg_notifer�����ľ����֮�󽫲��ٽ����κ���Ϣ
	*/
	void close_msg_notifer(param_list_base& amh);
	//////////////////////////////////////////////////////////////////////////
	/*!
	@brief ȡ��make_msg_notifer������Ļص�����
	@param amh ��Ϣ���
	@param tm ��Ϣ�ȴ���ʱms����ʱ�󷵻�false
	@return ��ʱ�󷵻�false
	*/
	__yield_interrupt bool timed_pump_msg(actor_msg_handle<>& amh, int tm);
	__yield_interrupt void pump_msg(actor_msg_handle<>& amh);
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_handle<> >& amh, int tm);
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_handle<> >& amh);
	//////////////////////////////////////////////////////////////////////////
	template <typename T0>
	__yield_interrupt bool timed_pump_msg(param_list<msg_param<T0> >& amh, __out T0& r0, int tm)
	{
		assert(amh._actorID == _actorID);
		assert_enter();
		assert(amh._pIsClosed);
		ref_ex<T0> ref(r0);
		msg_param<T0>* param = amh.front();
		if (param)
		{
			param->move_out(ref);
			amh.pop_front();
			return true;
		}
		amh._waiting = true;
		amh.set_dst_ref(ref);
		return pump_msg_push(amh, tm);
	}

	template <typename T0>
	__yield_interrupt void pump_msg(param_list<msg_param<T0> >& amh, __out T0& r0)
	{
		timed_pump_msg(amh, r0, -1);
	}

	template <typename T0>
	__yield_interrupt T0 pump_msg(param_list<msg_param<T0> >& amh)
	{
		T0 r0;
		timed_pump_msg(amh, r0, -1);
		return r0;
	}

	template <typename T0>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_handle<T0> >& amh, __out T0& r0, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, tm);
	}

	template <typename T0>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_handle<T0> >& amh, __out T0& r0)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, -1);
	}

	template <typename T0>
	__yield_interrupt T0 pump_msg(const std::shared_ptr<actor_msg_handle<T0> >& amh)
	{
		T0 r0;
		timed_pump_msg(*amh, r0, -1);
		return r0;
	}

	template <typename T0>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0> >& amh, __out T0& r0, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, tm);
	}

	template <typename T0>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0> >& amh, __out T0& r0)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, -1);
	}

	template <typename T0>
	__yield_interrupt T0 pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0> >& amh)
	{
		T0 r0;
		timed_pump_msg(*amh, r0, -1);
		return r0;
	}
	//////////////////////////////////////////////////////////////////////////
	template <typename T0, typename T1>
	__yield_interrupt bool timed_pump_msg(param_list<msg_param<T0, T1> >& amh, __out T0& r0, __out T1& r1, int tm)
	{
		assert(amh._actorID == _actorID);
		assert_enter();
		assert(amh._pIsClosed);
		ref_ex<T0, T1> ref(r0, r1);
		msg_param<T0, T1>* param = amh.front();
		if (param)
		{
			param->move_out(ref);
			amh.pop_front();
			return true;
		}
		amh._waiting = true;
		amh.set_dst_ref(ref);
		return pump_msg_push(amh, tm);
	}

	template <typename T0, typename T1>
	__yield_interrupt void pump_msg(param_list<msg_param<T0, T1> >& amh, __out T0& r0, __out T1& r1)
	{
		timed_pump_msg(amh, r0, r1, -1);
	}

	template <typename T0, typename T1>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_handle<T0, T1> >& amh, __out T0& r0, __out T1& r1, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, r1, tm);
	}

	template <typename T0, typename T1>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_handle<T0, T1> >& amh, __out T0& r0, __out T1& r1)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, r1, -1);
	}

	template <typename T0, typename T1>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0, T1> >& amh, __out T0& r0, __out T1& r1, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, r1, tm);
	}

	template <typename T0, typename T1>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0, T1> >& amh, __out T0& r0, __out T1& r1)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, r1, -1);
	}
	//////////////////////////////////////////////////////////////////////////
	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_pump_msg(param_list<msg_param<T0, T1, T2> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, int tm)
	{
		assert(amh._actorID == _actorID);
		assert_enter();
		assert(amh._pIsClosed);
		ref_ex<T0, T1, T2> ref(r0, r1, r2);
		msg_param<T0, T1, T2>* param = amh.front();
		if (param)
		{
			param->move_out(ref);
			amh.pop_front();
			return true;
		}
		amh._waiting = true;
		amh.set_dst_ref(ref);
		return pump_msg_push(amh, tm);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void pump_msg(param_list<msg_param<T0, T1, T2> >& amh, __out T0& r0, __out T1& r1, __out T2& r2)
	{
		timed_pump_msg(amh, r0, r1, r2, -1);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_handle<T0, T1, T2> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, r1, r2, tm);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_handle<T0, T1, T2> >& amh, __out T0& r0, __out T1& r1, __out T2& r2)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, r1, r2, -1);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, r1, r2, tm);
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2> >& amh, __out T0& r0, __out T1& r1, __out T2& r2)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, r1, r2, -1);
	}
	//////////////////////////////////////////////////////////////////////////
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_pump_msg(param_list<msg_param<T0, T1, T2, T3> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3, int tm)
	{
		assert(amh._actorID == _actorID);
		assert_enter();
		assert(amh._pIsClosed);
		ref_ex<T0, T1, T2, T3> ref(r0, r1, r2, r3);
		msg_param<T0, T1, T2, T3>* param = amh.front();
		if (param)
		{
			param->move_out(ref);
			amh.pop_front();
			return true;
		}
		amh._waiting = true;
		amh.set_dst_ref(ref);
		return pump_msg_push(amh, tm);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void pump_msg(param_list<msg_param<T0, T1, T2, T3> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3)
	{
		timed_pump_msg(amh, r0, r1, r2, r3, -1);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_handle<T0, T1, T2, T3> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, r1, r2, r3, tm);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_handle<T0, T1, T2, T3> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, r1, r2, r3, -1);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt bool timed_pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2, T3> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3, int tm)
	{
		assert(amh);
		auto lockAmh = amh;
		return timed_pump_msg(*amh, r0, r1, r2, r3, tm);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt void pump_msg(const std::shared_ptr<actor_msg_bounded_handle<T0, T1, T2, T3> >& amh, __out T0& r0, __out T1& r1, __out T2& r2, __out T3& r3)
	{
		assert(amh);
		auto lockAmh = amh;
		timed_pump_msg(*amh, r0, r1, r2, r3, -1);
	}
private:
	bool pump_msg_push(param_list_base& pm, int tm);

	void trig_handler();

	template <typename REF /*ref_ex*/>
	void _trig_handler(REF& dst, REF& src)
	{
		if (!_quited)
		{
			dst.move_from(src);
			pull_yield();
		}
	}

	template <typename T0>
	void trig_handler(ref_ex<T0>& r, const T0& p0)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited)
			{
				r.save_from(const_ref_ex<T0>(p0));//�����ڴ�(_strand�ڲ�)������������Actor���˳�(_quited == true)��������rʧЧ
				trig_handler();
			}
		} 
		else
		{//��ʱ���ܴ����������Ϊ r ������ʧЧ
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &r](){shared_this->_trig_handler(r, ref_ex<T0>((T0&)p0)); });
		}
	}

	template <typename T0, typename T1>
	void trig_handler(ref_ex<T0, T1>& r, const T0& p0, const T1& p1)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited)
			{
				r.save_from(const_ref_ex<T0, T1>(p0, p1));
				trig_handler();
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &r](){shared_this->_trig_handler(r, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
		}
	}

	template <typename T0, typename T1, typename T2>
	void trig_handler(ref_ex<T0, T1, T2>& r, const T0& p0, const T1& p1, const T2& p2)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited)
			{
				r.save_from(const_ref_ex<T0, T1, T2>(p0, p1, p2));
				trig_handler();
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &r](){shared_this->_trig_handler(r, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
		}
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void trig_handler(ref_ex<T0, T1, T2, T3>& r, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited)
			{
				r.save_from(const_ref_ex<T0, T1, T2, T3>(p0, p1, p2, p3));
				trig_handler();
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &r](){shared_this->_trig_handler(r, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
		}
	}

	bool async_trig_push(async_trig_base& th, int tm, void* dst/*ref_ex*/);

	void async_trig_post_yield(async_trig_base& th, void* src/*const_ref_ex*/);

	void async_trig_pull_yield(async_trig_base& th, void* src/*ref_ex*/);

	void _async_trig_handler(const std::shared_ptr<bool>& pIsClosed, async_trig_handle<>& th);

	template <typename TH /*async_trig_handle*/, typename REF /*ref_ex*/>
	void _async_trig_handler(const std::shared_ptr<bool>& pIsClosed, TH& th, REF& src)
	{
		assert(_strand->running_in_this_thread());
		if (!_quited && !(*pIsClosed) && !th._notify)
		{
			async_trig_pull_yield(th, &src);
		}
	}

	template <typename T0>
	void async_trig_handler(const std::shared_ptr<bool>& pIsClosed, async_trig_handle<T0>& th, const T0& p0)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th._notify)
			{
				const_ref_ex<T0> cref(p0);
				async_trig_post_yield(th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &th](){shared_this->_async_trig_handler(pIsClosed, th, ref_ex<T0>((T0&)p0)); });
		}
	}

	template <typename T0>
	void async_trig_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<async_trig_handle<T0> >& th, const T0& p0)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th->_notify)
			{
				const_ref_ex<T0> cref(p0);
				async_trig_post_yield(*th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->_async_trig_handler(pIsClosed, *th, ref_ex<T0>((T0&)p0)); });
		}
	}

	template <typename T0, typename T1>
	void async_trig_handler(const std::shared_ptr<bool>& pIsClosed, async_trig_handle<T0, T1>& th, const T0& p0, const T1& p1)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th._notify)
			{
				const_ref_ex<T0, T1> cref(p0, p1);
				async_trig_post_yield(th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &th](){shared_this->_async_trig_handler(pIsClosed, th, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
		}
	}

	template <typename T0, typename T1>
	void async_trig_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<async_trig_handle<T0, T1> >& th, const T0& p0, const T1& p1)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th->_notify)
			{
				const_ref_ex<T0, T1> cref(p0, p1);
				async_trig_post_yield(*th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->_async_trig_handler(pIsClosed, *th, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
		}
	}

	template <typename T0, typename T1, typename T2>
	void async_trig_handler(const std::shared_ptr<bool>& pIsClosed, async_trig_handle<T0, T1, T2>& th, const T0& p0, const T1& p1, const T2& p2)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th._notify)
			{
				const_ref_ex<T0, T1, T2> cref(p0, p1, p2);
				async_trig_post_yield(th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &th](){shared_this->_async_trig_handler(pIsClosed, th, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
		}
	}

	template <typename T0, typename T1, typename T2>
	void async_trig_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<async_trig_handle<T0, T1, T2> >& th, const T0& p0, const T1& p1, const T2& p2)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th->_notify)
			{
				const_ref_ex<T0, T1, T2> cref(p0, p1, p2);
				async_trig_post_yield(*th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->_async_trig_handler(pIsClosed, *th, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
		}
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void async_trig_handler(const std::shared_ptr<bool>& pIsClosed, async_trig_handle<T0, T1, T2, T3>& th, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th._notify)
			{
				const_ref_ex<T0, T1, T2, T3> cref(p0, p1, p2, p3);
				async_trig_post_yield(th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &th](){shared_this->_async_trig_handler(pIsClosed, th, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
		}
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void async_trig_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<async_trig_handle<T0, T1, T2, T3> >& th, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed) && !th->_notify)
			{
				const_ref_ex<T0, T1, T2, T3> cref(p0, p1, p2, p3);
				async_trig_post_yield(*th, &cref);
			}
		}
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->_async_trig_handler(pIsClosed, *th, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
		}
	}
private:
	template <typename T0>
	void notify_handler(const std::shared_ptr<bool>& pIsClosed, param_list<msg_param<T0> >& amh, const T0& p0)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(amh, const_ref_ex<T0>(p0));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &amh](){shared_this->check_run2(pIsClosed, amh, ref_ex<T0>((T0&)p0)); });
		}
	}

	template <typename T0>
	void notify_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<param_list<msg_param<T0> > >& amh, const T0& p0)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(*amh, const_ref_ex<T0>(p0));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->check_run2(pIsClosed, *amh, ref_ex<T0>((T0&)p0)); });
		}
	}

	template <typename T0, typename T1>
	void notify_handler(const std::shared_ptr<bool>& pIsClosed, param_list<msg_param<T0, T1> >& amh, const T0& p0, const T1& p1)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(amh, const_ref_ex<T0, T1>(p0, p1));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &amh](){shared_this->check_run2(pIsClosed, amh, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
		}
	}

	template <typename T0, typename T1>
	void notify_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<param_list<msg_param<T0, T1> > >& amh, const T0& p0, const T1& p1)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(*amh, const_ref_ex<T0, T1>(p0, p1));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->check_run2(pIsClosed, *amh, ref_ex<T0, T1>((T0&)p0, (T1&)p1)); });
		}
	}

	template <typename T0, typename T1, typename T2>
	void notify_handler(const std::shared_ptr<bool>& pIsClosed, param_list<msg_param<T0, T1, T2> >& amh, const T0& p0, const T1& p1, const T2& p2)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(amh, const_ref_ex<T0, T1, T2>(p0, p1, p2));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &amh](){shared_this->check_run2(pIsClosed, amh, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
		}
	}

	template <typename T0, typename T1, typename T2>
	void notify_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<param_list<msg_param<T0, T1, T2> > >& amh, const T0& p0, const T1& p1, const T2& p2)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(*amh, const_ref_ex<T0, T1, T2>(p0, p1, p2));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->check_run2(pIsClosed, *amh, ref_ex<T0, T1, T2>((T0&)p0, (T1&)p1, (T2&)p2)); });
		}
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void notify_handler(const std::shared_ptr<bool>& pIsClosed, param_list<msg_param<T0, T1, T2, T3> >& amh, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(amh, const_ref_ex<T0, T1, T2, T3>(p0, p1, p2, p3));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=, &amh](){shared_this->check_run2(pIsClosed, amh, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
		}
	}

	template <typename T0, typename T1, typename T2, typename T3>
	void notify_handler_ptr(const std::shared_ptr<bool>& pIsClosed, const std::shared_ptr<param_list<msg_param<T0, T1, T2, T3> > >& amh, const T0& p0, const T1& p1, const T2& p2, const T3& p3)
	{
		if (_strand->running_in_this_thread())
		{
			if (!_quited && !(*pIsClosed))
			{
				check_run3(*amh, const_ref_ex<T0, T1, T2, T3>(p0, p1, p2, p3));
			}
		} 
		else
		{
			actor_handle shared_this = shared_from_this();
			_strand->post([=](){shared_this->check_run2(pIsClosed, *amh, ref_ex<T0, T1, T2, T3>((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3)); });
		}
	}
	//////////////////////////////////////////////////////////////////////////

	void check_run1(std::shared_ptr<bool>& pIsClosed, actor_msg_handle<>& amh);

	template <typename MT /*msg_param*/, typename REF /*ref_ex*/>
	void check_run2(const std::shared_ptr<bool>& pIsClosed, param_list<MT>& amh, REF& src)
	{
		if (!_quited && !(*pIsClosed))
		{
			if (amh._waiting)
			{
				amh._waiting = false;
				if (amh._hasTm)
				{
					amh._hasTm = false;
					cancel_timer();
				}
				assert(amh.empty());
				amh.move_from(src);
				pull_yield();
			} 
			else
			{
				amh.move_to_back(MT(src));
			}
		}
	}

	template <typename MT /*msg_param*/, typename CREF /*msg_param::const_ref_type*/>
	void check_run3(param_list<MT>& amh, CREF& srcRef)
	{
		if (amh._waiting)
		{
			amh._waiting = false;
			if (amh._hasTm)
			{
				amh._hasTm = false;
				cancel_timer();
			}
			assert(amh.empty());
			amh.save_from(srcRef);
			trig_handler();
		} 
		else
		{
			amh.move_to_back(MT(srcRef));
		}
	}
private:
	template <typename T0, typename T1, typename T2, typename T3>
	my_actor::msg_pump_status::pck<T0, T1, T2, T3>& get_pck(msg_pump_status& msgPumpStatus)
	{
		typedef my_actor::msg_pump_status::pck<T0, T1, T2, T3> msg_type;
		msg_type* res = NULL;
		auto& msgPumpList = msgPumpStatus._msgPumpList[func_type<T0, T1, T2, T3>::number];
		if (0 == func_type<T0, T1, T2, T3>::number)
		{
			if (!msgPumpList.empty())
			{
				res = (msg_type*)msgPumpList.front().get();
			}
		}
		else
		{
			for (auto it = msgPumpList.begin(); it != msgPumpList.end() && !res; it++)
			{
				if (sizeof(msg_type) == (*it)->_size && dynamic_cast<msg_type*>(it->get()))
				{
					res = (msg_type*)it->get();
				}
			}
		}

		if (!res)
		{
			msgPumpList.push_front(std::shared_ptr<msg_type>(new msg_type));
			res = (msg_type*)msgPumpList.front().get();
			res->_notifer = make_msg_notifer(res->_handle);
		}
		return *res;
	}
public:
	/*!
	@@brief ��ȡһ����Ϣ�þ��
	*/
	actor_msg_handle<>& get_msg_handle();

	template <typename T0>
	actor_msg_handle<T0>& get_msg_handle()
	{
		assert_enter();
		return get_pck<T0, void, void, void>(_msgPumpStatus)._handle;
	}

	template <typename T0, typename T1>
	actor_msg_handle<T0, T1>& get_msg_handle()
	{
		assert_enter();
		return get_pck<T0, T1, void, void>(_msgPumpStatus)._handle;
	}

	template <typename T0, typename T1, typename T2>
	actor_msg_handle<T0, T1, T2>& get_msg_handle()
	{
		assert_enter();
		return get_pck<T0, T1, T2, void>(_msgPumpStatus)._handle;
	}

	template <typename T0, typename T1, typename T2, typename T3>
	actor_msg_handle<T0, T1, T2, T3>& get_msg_handle()
	{
		assert_enter();
		return get_pck<T0, T1, T2, T3>(_msgPumpStatus)._handle;
	}

	/*!
	@@brief ��ȡ��һ��Actor����Ϣ���ݺ���
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	__yield_interrupt typename func_type<T0, T1, T2, T3>::result get_buddy_notifer(actor_handle buddyActor)
	{
		typedef typename func_type<T0, T1, T2, T3>::result notifer_type;
		return send<notifer_type>(buddyActor->_strand, [&buddyActor]()->notifer_type
		{
			return buddyActor->get_pck<T0, T1, T2, T3>(buddyActor->_msgPumpStatus)._notifer;
		});
	}

	template <typename T0, typename T1, typename T2>
	__yield_interrupt std::function<void(T0, T1, T2)> get_buddy_notifer(actor_handle buddyActor)
	{
		return get_buddy_notifer<T0, T1, T2, void>(buddyActor);
	}

	template <typename T0, typename T1>
	__yield_interrupt std::function<void(T0, T1)> get_buddy_notifer(actor_handle buddyActor)
	{
		return get_buddy_notifer<T0, T1, void, void>(buddyActor);
	}

	template <typename T0>
	__yield_interrupt std::function<void(T0)> get_buddy_notifer(actor_handle buddyActor)
	{
		return get_buddy_notifer<T0, void, void, void>(buddyActor);
	}

	__yield_interrupt std::function<void()> get_buddy_notifer(actor_handle buddyActor);
	//////////////////////////////////////////////////////////////////////////

	/*!
	@@brief ��ȡ��Actor����Ϣ���ݺ�������Actor��������ios�޹��߳���ʹ��
	*/
	template <typename T0, typename T1, typename T2, typename T3>
	typename func_type<T0, T1, T2, T3>::result outside_get_notifer()
	{
		typedef typename func_type<T0, T1, T2, T3>::result notifer_type;
		return _strand->syncInvoke<notifer_type>([this]()->notifer_type
		{
			return this->get_pck<T0, T1, T2, T3>(_msgPumpStatus)._notifer;
		});
	}

	template <typename T0, typename T1, typename T2>
	std::function<void(T0, T1, T2)> outside_get_notifer()
	{
		return outside_get_notifer<T0, T1, T2, void>();
	}

	template <typename T0, typename T1>
	std::function<void(T0, T1)> outside_get_notifer()
	{
		return outside_get_notifer<T0, T1, void, void>();
	}

	template <typename T0>
	std::function<void(T0)> outside_get_notifer()
	{
		return outside_get_notifer<T0, void, void, void>();
	}

	std::function<void()> outside_get_notifer();
public:
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
	shared_strand this_strand();

	/*!
	@brief ���ر����������ָ��
	*/
	actor_handle shared_from_this();

	/*!
	@brief ��ȡ��ǰActorID��
	*/
	long long this_id();

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
private:
	void assert_enter();
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
	void push_yield();
	void force_quit_cb_handler();
	void exit_callback();
	void child_suspend_cb_handler();
	void child_resume_cb_handler();
private:
	void* _actorPull;///<Actor�жϵ�ָ�
	void* _actorPush;///<Actor�жϵ�
	void* _stackTop;///<Actorջ��
	long long _actorID;///<ActorID
	size_t _stackSize;///<Actorջ��С
	shared_strand _strand;///<Actor������
	DEBUG_OPERATION(bool _inActor);///<��ǰ����Actor�ڲ�ִ�б��
	bool _started;///<�Ѿ���ʼ���еı��
	bool _quited;///<�Ѿ�׼���˳����
	bool _suspended;///<Actor������
	bool _hasNotify;///<��ǰActor�������ⲿ����׼������Actor���
	bool _isForce;///<�Ƿ���ǿ���˳��ı�ǣ��ɹ�������force_quit
	size_t _yieldCount;//yield����
	size_t _childOverCount;///<��Actor�˳�ʱ����
	size_t _childSuspendResumeCount;///<��Actor����/�ָ�����
	std::weak_ptr<my_actor> _parentActor;///<��Actor
	main_func _mainFunc;///<Actor���
	list<suspend_resume_option> _suspendResumeQueue;///<����/�ָ���������
	list<actor_handle> _childActorList;///<��Actor����
	list<std::function<void (bool)> > _exitCallback;///<Actor������Ļص�������ǿ���˳�����false�������˳�����true
	list<std::function<void ()> > _quitHandlerList;///<Actor�˳�ʱǿ�Ƶ��õĺ�������ע�����ִ��
	msg_pump_status _msgPumpStatus;///<�Է����͹�������Ϣ��
	timer_pck* _timer;///<�ṩ��ʱ����
	std::weak_ptr<my_actor> _weakThis;
};

#endif
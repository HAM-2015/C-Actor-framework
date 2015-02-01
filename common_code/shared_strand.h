#ifndef __SHARED_STRAND_H
#define __SHARED_STRAND_H


#ifdef ENABLE_STRAND_IMPL_POOL
#include "strand_ex.h"
#else
//#define BOOST_ASIO_ENABLE_SEQUENTIAL_STRAND_ALLOCATION
//#define BOOST_ASIO_STRAND_IMPLEMENTATIONS	11//����ios��strand�����Ŀ���������ص���
#include <boost/asio/strand.hpp>
#endif //end ENABLE_SHARE_STRAND_IMPL

#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "ios_proxy.h"
#include "wrapped_post_handler.h"
#include "wrapped_dispatch_handler.h"

class boost_strand;
typedef boost::shared_ptr<boost_strand> shared_strand;

/*!
@brief ���¶���dispatchʵ�֣����в�ͬstrand������Ϣ��ʽ���к�������
*/
#ifdef ENABLE_STRAND_IMPL_POOL
class boost_strand: public strand_ex
{
private:
	boost_strand(ios_proxy& iosProxy)
		: strand_ex(iosProxy), _iosProxy(iosProxy)

#else //defined ENABLE_SHARE_STRAND_IMPL

class boost_strand: public boost::asio::strand
{
private:
	boost_strand(ios_proxy& iosProxy)
		: boost::asio::strand(iosProxy), _iosProxy(iosProxy)

#endif //end ENABLE_SHARE_STRAND_IMPL

	{

	}
public:
	static shared_strand create(ios_proxy& iosProxy)
	{
		return shared_strand(new boost_strand(iosProxy));
	}
public:
	/*!
	@brief �ض���ԭdispatchʵ�֣�����ڱ�strand�е�����ֱ��ִ�У�������ӵ������еȴ�ִ��
	@param handler �����ú���
	*/
	template <typename Handler>
	void dispatch(const Handler& handler)
	{
		if (running_in_this_thread())
		{
			handler();
		} 
		else
		{
			post(handler);
		}
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, Handler> wrap(const Handler& handler)
	{
		return wrapped_dispatch_handler<boost_strand, Handler>(this, handler);
	}
	
	/*!
	@brief �ѱ����ú�����װ��post��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, Handler> wrap_post(const Handler& handler)
	{
		return wrapped_post_handler<boost_strand, Handler>(this, handler);
	}

	/*!
	@brief ����һ������ͬһ��ios��strand
	*/
	shared_strand clone()
	{
		return create(_iosProxy);
	}

	/*!
	@brief ����Ƿ���������ios�߳���ִ��
	@return true ��, false ����
	*/
	bool in_this_ios()
	{
		return _iosProxy.runningInThisIos();
	}

	/*!
	@brief ������ios�����������߳���
	*/
	size_t ios_thread_number()
	{
		return _iosProxy.threadNumber();
	}

	/*!
	@brief ��ȡ��ǰ����������
	*/
	ios_proxy& get_ios_proxy()
	{
		return _iosProxy;
	}
private:
	ios_proxy& _iosProxy;
public:
	/*!
	@brief ��һ��strand�е���ĳ��������ֱ�����������ִ����ɺ�ŷ���
	@warning �˺����п���ʹ������������������ֻ������strand��������ios�޹��߳��е���
	*/
	static void syncInvoke(shared_strand strand, const boost::function<void ()>& h)
	{
		assert(!strand->in_this_ios());
		boost::mutex mutex;
		boost::condition_variable con;
		boost::unique_lock<boost::mutex> ul(mutex);
		strand->post(boost::bind(&syncInvoke_proxy, boost::ref(h), boost::ref(mutex), boost::ref(con)));
		con.wait(ul);
	}

	/*!
	@brief ͬ�ϣ�������ֵ
	*/
	template <class R>
	static R syncInvoke(shared_strand strand, const boost::function<R ()>& h)
	{
		assert(!strand->in_this_ios());
		R r;
		boost::mutex mutex;
		boost::condition_variable con;
		boost::unique_lock<boost::mutex> ul(mutex);
		strand->post(boost::bind(&syncInvoke_proxy_ret<R>, boost::ref(r), boost::ref(h), boost::ref(mutex), boost::ref(con)));
		con.wait(ul);
		return r;
	}

	/*!
	@brief ��strand�е���ĳ��������ֵ������Ȼ��ͨ��һ���ص������ѷ���ֵ����
	@param cb ���������ĺ���
	*/
	template <class R>
	static void asyncInvoke(shared_strand strand, const boost::function<R ()>& h, const boost::function<void (R)>& cb)
	{
		strand->post(boost::bind(&asyncInvoke_proxy_ret<R>, h, cb));
	}

	static void asyncInvokeVoid(shared_strand strand, const boost::function<void ()>& h, const boost::function<void ()>& cb)
	{
		strand->post(boost::bind(&asyncInvoke_proxy_ret_void, h, cb));
	}
private:
	static void syncInvoke_proxy(const boost::function<void ()>& h, boost::mutex& mutex, boost::condition_variable& con)
	{
		h();
		mutex.lock();
		con.notify_one();
		mutex.unlock();
	}

	template <class R>
	static void syncInvoke_proxy_ret(R& r, const boost::function<R ()>& h, boost::mutex& mutex, boost::condition_variable& con)
	{
		r = h();
		mutex.lock();
		con.notify_one();
		mutex.unlock();
	}

	template <class R>
	static void asyncInvoke_proxy_ret(const boost::function<R ()>& h, const boost::function<void (R)>& cb)
	{
		cb(h());
	}

	static void asyncInvoke_proxy_ret_void(const boost::function<void ()>& h, const boost::function<void ()>& cb)
	{
		h();
		cb();
	}
};

#endif
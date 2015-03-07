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
class boost_strand
{
#ifdef ENABLE_STRAND_IMPL_POOL
	typedef strand_ex strand_type;
#else
	typedef boost::asio::strand strand_type;
#endif
protected:
	boost_strand();
public:
	virtual ~boost_strand();
	static shared_strand create(ios_proxy& iosProxy);
public:
	/*!
	@brief ����ڱ�strand�е�����ֱ��ִ�У�������ӵ������еȴ�ִ��
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
	@brief ���һ������strand����
	*/
	template <typename Handler>
	void post(const Handler& handler)
	{
#ifndef ENABLE_MFC_ACTOR
		_strand->post(handler);
#else
		if (_strand)
		{
			_strand->post(handler);
		}
		else
		{
			_post(handler);
		}
#endif
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
	virtual shared_strand clone();

	/*!
	@brief ����Ƿ���������ios�߳���ִ��
	@return true ��, false ����
	*/
	virtual bool in_this_ios();

	/*!
	@brief ������ios�����������߳���
	*/
	size_t ios_thread_number();

	/*!
	@brief �ж��Ƿ��ڱ�strand������
	*/
	virtual bool running_in_this_thread();

	/*!
	@brief ��ȡ��ǰ����������
	*/
	ios_proxy& get_ios_proxy();

	/*!
	@brief ��ȡ��ǰ������
	*/
	boost::asio::io_service& get_io_service();

#ifdef ENABLE_MFC_ACTOR
	virtual void _post(const boost::function<void ()>& h);
#endif
protected:
	ios_proxy* _iosProxy;
	strand_type* _strand;
public:
	/*!
	@brief ��һ��strand�е���ĳ��������ֱ�����������ִ����ɺ�ŷ���
	@warning �˺����п���ʹ������������������ֻ������strand��������ios�޹��߳��е���
	*/
	static void syncInvoke(shared_strand strand, const boost::function<void ()>& h);

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

	static void asyncInvokeVoid(shared_strand strand, const boost::function<void ()>& h, const boost::function<void ()>& cb);
private:
	static void syncInvoke_proxy(const boost::function<void ()>& h, boost::mutex& mutex, boost::condition_variable& con);

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

	static void asyncInvoke_proxy_ret_void(const boost::function<void ()>& h, const boost::function<void ()>& cb);
};

#endif
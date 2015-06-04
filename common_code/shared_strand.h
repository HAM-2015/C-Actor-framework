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
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <functional>
#include <memory>
#include "ios_proxy.h"
#include "wrapped_post_handler.h"
#include "wrapped_dispatch_handler.h"

class boost_strand;
typedef std::shared_ptr<boost_strand> shared_strand;

#define UI_STRAND()\
if (_strand)\
{\
	_strand->post(CHECK_MOVE(handler)); \
}\
else\
{\
	_post(handler);\
};

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
	virtual ~boost_strand();
public:
	static shared_strand create(ios_proxy& iosProxy);
public:
	/*!
	@brief ����ڱ�strand�е�����ֱ��ִ�У�������ӵ������еȴ�ִ��
	@param handler �����ú���
	*/
	template <typename Handler>
	void dispatch(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		if (running_in_this_thread())
		{
			handler();
		} 
		else
		{
			post(CHECK_MOVE(handler));
		}
	}

	/*!
	@brief ���һ������strand����
	*/
	template <typename Handler>
	void post(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
#ifdef ENABLE_MFC_ACTOR
		UI_STRAND()
#elif ENABLE_WX_ACTOR
		UI_STRAND()
#else
		_strand->post(CHECK_MOVE(handler));
#endif
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, Handler> wrap(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		return wrapped_dispatch_handler<boost_strand, Handler>(this, CHECK_MOVE(handler));
	}
	
	/*!
	@brief �ѱ����ú�����װ��post��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, Handler> wrap_post(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		return wrapped_post_handler<boost_strand, Handler>(this, CHECK_MOVE(handler));
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

#if (defined ENABLE_MFC_ACTOR || defined ENABLE_WX_ACTOR)
	virtual void _post(const std::function<void ()>& h);
#endif
protected:
	ios_proxy* _iosProxy;
	strand_type* _strand;
public:
	/*!
	@brief ��һ��strand�е���ĳ��������ֱ�����������ִ����ɺ�ŷ���
	@warning �˺����п���ʹ������������������ֻ������strand��������ios�޹��߳��е���
	*/
	template <typename H>
	void syncInvoke(H&& h)
	{
		assert(!in_this_ios());
		boost::mutex mutex;
		boost::condition_variable con;
		boost::unique_lock<boost::mutex> ul(mutex);
		post([&]
		{
			h();
			mutex.lock();
			con.notify_one();
			mutex.unlock();
		});
		con.wait(ul);
	}

	/*!
	@brief ͬ�ϣ�������ֵ
	*/
	template <typename R, typename H>
	R syncInvoke(H&& h)
	{
		assert(!in_this_ios());
		R r;
		boost::mutex mutex;
		boost::condition_variable con;
		boost::unique_lock<boost::mutex> ul(mutex);
		post([&]
		{
			r = h();
			mutex.lock();
			con.notify_one();
			mutex.unlock();
		});
		con.wait(ul);
		return r;
	}

	/*!
	@brief ��strand�е���ĳ��������ֵ������Ȼ��ͨ��һ���ص������ѷ���ֵ����
	@param cb ���������ĺ���
	*/
	template <typename H, typename CB>
	void asyncInvoke(H&& h, CB&& cb)
	{
		post([=]
		{
			cb(h());
		});
	}

	template <typename H, typename CB>
	void asyncInvokeVoid(H&& h, CB&& cb)
	{
		post([=]
		{
			h();
			cb();
		});
	}
};

#endif
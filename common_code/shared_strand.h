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
#include "msg_queue.h"
#include "wrapped_post_handler.h"
#include "wrapped_tick_handler.h"
#include "wrapped_dispatch_handler.h"
#include "wrapped_next_tick_handler.h"

class actor_timer;

class boost_strand;
typedef std::shared_ptr<boost_strand> shared_strand;

#define SPACE_SIZE		(sizeof(void*)*16)

#define RUN_HANDLER [this, handler]\
{\
	bool checkDestroy = false;\
	_pCheckDestroy = &checkDestroy;\
	handler();\
	if (!checkDestroy)\
	{\
		_pCheckDestroy = NULL;\
		if (!_nextTickQueue.empty())\
		{\
			run_tick(); \
		}\
	}\
}

#define UI_POST()\
if (_strand)\
{\
	_strand->post(RUN_HANDLER); \
}\
else\
{\
	_post(handler); \
};

#define UI_NEXT_TICK()\
if (_strand)\
{\
	_nextTickQueue.push_back(wrap_tick_type::create(_nextTickAlloc, TRY_MOVE(handler)));\
}\
else\
{\
	_post(handler); \
}

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

	struct wrap_next_tick_base
	{
		virtual void invoke() = 0;
		virtual void* destroy() = 0;
	};

	struct wrap_next_tick_space : public wrap_next_tick_base
	{
		unsigned char space[SPACE_SIZE];
	};

	template <typename H, bool = true>
	struct wrap_next_tick_handler : public wrap_next_tick_space
	{
		wrap_next_tick_handler(const H& h)
		{
			assert(sizeof(H) <= sizeof(space));
			new(space)H(h);
		}

		wrap_next_tick_handler(H&& h)
		{
			assert(sizeof(H) <= sizeof(space));
			new(space)H((H&&)h);
		}

		template <typename Handler>
		static inline wrap_next_tick_base* create(mem_alloc<wrap_next_tick_space>& alloc, BOOST_ASIO_MOVE_ARG(Handler) handler)
		{
			return new(alloc.allocate())wrap_next_tick_handler<RM_CREF(handler)>(TRY_MOVE(handler));
		}

		void invoke()
		{
			(*(H*)space)();
		}

		void* destroy()
		{
			((H*)space)->~H();
			return this;
		}
	};

	template <typename H>
	struct wrap_next_tick_handler<H, false> : public wrap_next_tick_base
	{
		wrap_next_tick_handler(const H& h)
			:_h(h) {}

		wrap_next_tick_handler(H&& h)
			:_h((H&&)h) {}

		template <typename Handler>
		static inline wrap_next_tick_base* create(mem_alloc<wrap_next_tick_space>& alloc, BOOST_ASIO_MOVE_ARG(Handler) handler)
		{
			return new wrap_next_tick_handler<RM_CREF(handler), false>(TRY_MOVE(handler));
		}

		void invoke()
		{
			_h();
		}

		void* destroy()
		{
			delete this;
			return NULL;
		}

		H _h;
	};
protected:
	boost_strand();
	virtual ~boost_strand();
public:
	static shared_strand create(ios_proxy& iosProxy, bool makeTimer = true);
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
			post(TRY_MOVE(handler));
		}
	}

	/*!
	@brief ���һ������ strand ����
	*/
	template <typename Handler>
	void post(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
#ifdef ENABLE_MFC_ACTOR
		UI_POST();
#elif ENABLE_WX_ACTOR
		UI_POST();
#else
		_strand->post(RUN_HANDLER);
#endif
	}

	/*!
	@brief ���һ������ next_tick ����
	*/
	template <typename Handler>
	void next_tick(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		assert(running_in_this_thread());
		assert(sizeof(wrap_next_tick_space) == sizeof(wrap_next_tick_handler<RM_CREF(handler)>));
		typedef wrap_next_tick_handler<RM_CREF(handler), sizeof(RM_CREF(handler)) <= SPACE_SIZE> wrap_tick_type;
#ifdef ENABLE_MFC_ACTOR
		UI_NEXT_TICK();
#elif ENABLE_WX_ACTOR
		UI_NEXT_TICK();
#else
		_nextTickQueue.push_back(wrap_tick_type::create(_nextTickAlloc, TRY_MOVE(handler)));
#endif
	}

	/*!
	@brief �������һ������ next_tick ����
	*/
	template <typename Handler>
	void try_tick(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		if (running_in_this_thread())
		{
			next_tick(TRY_MOVE(handler));
		} 
		else
		{
			post(TRY_MOVE(handler));
		}
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, Handler> wrap(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		return wrapped_dispatch_handler<boost_strand, Handler>(this, TRY_MOVE(handler));
	}
	
	/*!
	@brief �ѱ����ú�����װ��post��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, Handler> wrap_post(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		return wrapped_post_handler<boost_strand, Handler>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��next_tick��
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, Handler> wrap_next_tick(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		return wrapped_next_tick_handler<boost_strand, Handler>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��tick��
	*/
	template <typename Handler>
	wrapped_tick_handler<boost_strand, Handler> wrap_tick(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		return wrapped_tick_handler<boost_strand, Handler>(this, TRY_MOVE(handler));
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

	/*!
	@brief ��ȡ��ʱ��
	*/
	actor_timer* get_timer();
private:
	void run_tick();
protected:
#if (defined ENABLE_MFC_ACTOR || defined ENABLE_WX_ACTOR)
	virtual void _post(const std::function<void ()>& h);
#endif
	bool* _pCheckDestroy;
	actor_timer* _timer;
	ios_proxy* _iosProxy;
	strand_type* _strand;
	std::weak_ptr<boost_strand> _weakThis;
	mem_alloc<wrap_next_tick_space> _nextTickAlloc;
	msg_queue<wrap_next_tick_base*> _nextTickQueue;
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
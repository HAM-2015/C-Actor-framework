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
#include "wrapped_try_tick_handler.h"
#include "wrapped_dispatch_handler.h"
#include "wrapped_next_tick_handler.h"

class ActorTimer_;

class boost_strand;
typedef std::shared_ptr<boost_strand> shared_strand;

#define SPACE_SIZE		(sizeof(void*)*16)

#ifdef ENABLE_NEXT_TICK

#define RUN_HANDLER handler_capture<RM_REF(Handler)>(TRY_MOVE(handler), this)

#else //ENABLE_NEXT_TICK

#define RUN_HANDLER TRY_MOVE(handler)

#endif //ENABLE_NEXT_TICK


#define UI_POST()\
if (_strand)\
{\
	_strand->post(RUN_HANDLER); \
}\
else\
{\
	_post(TRY_MOVE(handler)); \
};


#ifdef ENABLE_NEXT_TICK

#define APPEND_TICK()	\
	static_assert(sizeof(wrap_next_tick_space) == sizeof(wrap_next_tick_handler<RM_REF(Handler)>), "next tick wrap error");\
	typedef wrap_next_tick_handler<RM_REF(Handler), sizeof(RM_REF(Handler)) <= SPACE_SIZE> wrap_tick_type;\
	_backTickQueue->push_back(wrap_tick_type::create(TRY_MOVE(handler), _nextTickAlloc, _reuMemAlloc));

#else //ENABLE_NEXT_TICK

#define APPEND_TICK()	post(TRY_MOVE(handler));

#endif //ENABLE_NEXT_TICK


#define UI_NEXT_TICK()\
if (_strand)\
{\
	APPEND_TICK(); \
}\
else\
{\
	_post(TRY_MOVE(handler)); \
}

/*!
@brief ���¶���dispatchʵ�֣����в�ͬstrand������Ϣ��ʽ���к�������
*/
class boost_strand
{
#ifdef ENABLE_STRAND_IMPL_POOL
	typedef StrandEx_ strand_type;
#else
	typedef boost::asio::strand strand_type;
#endif

#ifdef ENABLE_NEXT_TICK

	template <typename H>
	struct handler_capture
	{
		template <typename Handler>
		handler_capture(Handler&& handler, boost_strand* strand)
			:_handler(TRY_MOVE(handler)), _strand(strand) {}

		handler_capture(const handler_capture& s)
			:_handler(std::move(s._handler)), _strand(s._strand) {}

		handler_capture(handler_capture&& s)
			:_handler(std::move(s._handler)), _strand(s._strand) {}

		void operator =(const handler_capture& s)
		{
			static_assert(false, "no copy");
			_handler = std::move(s._handler);
			_strand = s._strand;
		}

		void operator =(handler_capture&& s)
		{
			_handler = std::move(s._handler);
			_strand = s._strand;
		}

		void operator ()()
		{
			if (_strand->_isReset)
			{
				_strand->run_tick_front();
			}
			bool checkDestroy = false;
			_strand->_pCheckDestroy = &checkDestroy;
			_handler();
			if (!checkDestroy)
			{
				_strand->_pCheckDestroy = NULL;
				_strand->_isReset = _strand->ready_empty();
				if (_strand->_isReset)
				{
					_strand->run_tick_back();
				}
			}
		}

		mutable H _handler;
		boost_strand* _strand;
	};

	struct wrap_next_tick_base
	{
		struct result 
		{
			void* _ptr;
			unsigned _size;
		};

		virtual result invoke() = 0;
	};

	struct wrap_next_tick_space : public wrap_next_tick_base
	{
		unsigned char space[SPACE_SIZE];
	};

	template <typename H, bool = true>
	struct wrap_next_tick_handler : public wrap_next_tick_space
	{
		template <typename Handler>
		wrap_next_tick_handler(Handler&& h)
		{
			assert(sizeof(H) <= sizeof(space));
			new(space)H(TRY_MOVE(h));
		}

		~wrap_next_tick_handler()
		{
			((H*)space)->~H();
		}

		template <typename Handler>
		static inline wrap_next_tick_base* create(Handler&& handler, mem_alloc_base& alloc, reusable_mem& reuAlloc)
		{
			if (!alloc.full())
			{
				return new(alloc.allocate())wrap_next_tick_handler<RM_REF(Handler)>(TRY_MOVE(handler));
			}
			else
			{
				return wrap_next_tick_handler<RM_REF(Handler), false>::create(TRY_MOVE(handler), alloc, reuAlloc);
			}
		}

		result invoke()
		{
			(*(H*)space)();
			this->~wrap_next_tick_handler();
			result res = { this, -1 };
			return res;
		}
	};

	template <typename H>
	struct wrap_next_tick_handler<H, false> : public wrap_next_tick_base
	{
		template <typename Handler>
		wrap_next_tick_handler(Handler&& h)
			:_h(TRY_MOVE(h)) {}

		~wrap_next_tick_handler()
		{
		}

		template <typename Handler>
		static inline wrap_next_tick_base* create(Handler&& handler, mem_alloc_base& alloc, reusable_mem& reuAlloc)
		{
			return new(reuAlloc.allocate(sizeof(wrap_next_tick_handler<RM_REF(Handler), false>)))wrap_next_tick_handler<RM_REF(Handler), false>(TRY_MOVE(handler));
		}

		result invoke()
		{
			_h();
			this->~wrap_next_tick_handler();
			result res = { this, sizeof(wrap_next_tick_handler) };
			return res;
		}

		H _h;
	};
#endif //ENABLE_NEXT_TICK

	template <typename H, typename CB>
	struct wrap_async_invoke
	{
		template <typename H1, typename CB1>
		wrap_async_invoke(H1&& h, CB1&& cb)
			:_h(TRY_MOVE(h)), _cb(TRY_MOVE(cb)) {}

		wrap_async_invoke(wrap_async_invoke&& s)
			:_h(std::move(s._h)), _cb(std::move(s._cb)) {}

		wrap_async_invoke(const wrap_async_invoke& s)
			:_h(s._h), _cb(s._cb) {}

		void operator=(wrap_async_invoke&& s)
		{
			_h = std::move(s._h);
			_cb = std::move(s._cb);
		}

		void operator=(const wrap_async_invoke&)
		{
			static_assert(false, "no copy");
		}

		void operator()()
		{
			_cb(_h());
		}

		H _h;
		CB _cb;
	};

	template <typename H, typename CB>
	struct wrap_async_invoke_void
	{
		template <typename H1, typename CB1>
		wrap_async_invoke_void(H1&& h, CB1&& cb)
			:_h(TRY_MOVE(h)), _cb(TRY_MOVE(cb)) {}

		wrap_async_invoke_void(wrap_async_invoke_void&& s)
			:_h(std::move(s._h)), _cb(std::move(s._cb)) {}

		wrap_async_invoke_void(const wrap_async_invoke_void& s)
			:_h(s._h), _cb(s._cb) {}

		void operator=(wrap_async_invoke_void&& s)
		{
			_h = std::move(s._h);
			_cb = std::move(s._cb);
		}

		void operator=(const wrap_async_invoke_void&)
		{
			static_assert(false, "no copy");
		}

		void operator()()
		{
			_h();
			_cb();
		}

		H _h;
		CB _cb;
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
	void dispatch(Handler&& handler)
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
	void post(Handler&& handler)
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
	void next_tick(Handler&& handler)
	{
		assert(running_in_this_thread());
#ifdef ENABLE_MFC_ACTOR
		UI_NEXT_TICK();
#elif ENABLE_WX_ACTOR
		UI_NEXT_TICK();
#else
		APPEND_TICK();
#endif
	}

	/*!
	@brief �������һ������ next_tick ����
	*/
	template <typename Handler>
	void try_tick(Handler&& handler)
	{
#ifdef ENABLE_NEXT_TICK
		if (running_in_this_thread())
		{
			next_tick(TRY_MOVE(handler));
		} 
		else
#endif //ENABLE_NEXT_TICK
		{
			post(TRY_MOVE(handler));
		}
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, RM_CREF(Handler)> wrap(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, RM_CREF(Handler), true> suck_wrap(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}
	
	/*!
	@brief �ѱ����ú�����װ��post��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, RM_CREF(Handler)> wrap_post(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��post�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_post(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��next_tick��
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, RM_CREF(Handler)> wrap_next_tick(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��next_tick�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_next_tick(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��tick��
	*/
	template <typename Handler>
	wrapped_try_tick_handler<boost_strand, RM_CREF(Handler)> wrap_try_tick(Handler&& handler)
	{
		return wrapped_try_tick_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��tick�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_try_tick_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_try_tick(Handler&& handler)
	{
		return wrapped_try_tick_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
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
	@brief ��ǰ��������Ƿ�Ϊ��(���㵱ǰ)
	@param checkTick �Ƿ����tick����
	*/
	virtual bool empty(bool checkTick = true);

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
	ActorTimer_* get_timer();
private:
#ifdef ENABLE_NEXT_TICK
	bool ready_empty();
	bool waiting_empty();
	void run_tick_front();
	void run_tick_back();
	bool* _pCheckDestroy;
	bool _isReset;
	reusable_mem _reuMemAlloc;
	mem_alloc<wrap_next_tick_space> _nextTickAlloc;
	msg_queue<wrap_next_tick_base*> _nextTickQueue1;
	msg_queue<wrap_next_tick_base*> _nextTickQueue2;
	msg_queue<wrap_next_tick_base*>* _backTickQueue;
	msg_queue<wrap_next_tick_base*>* _frontTickQueue;
#endif //ENABLE_NEXT_TICK
protected:
#if (defined ENABLE_MFC_ACTOR || defined ENABLE_WX_ACTOR)
	virtual void _post(const std::function<void ()>& h);
#endif
	ActorTimer_* _timer;
	ios_proxy* _iosProxy;
	strand_type* _strand;
	std::weak_ptr<boost_strand> _weakThis;
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
		unsigned char r[sizeof(R)];
		boost::mutex mutex;
		boost::condition_variable con;
		boost::unique_lock<boost::mutex> ul(mutex);
		post([&]
		{
			new(r)R(h());
			mutex.lock();
			con.notify_one();
			mutex.unlock();
		});
		con.wait(ul);
		AUTO_CALL(
		{
			typedef R T_;
			((T_*)r)->~R();
		});
		return (R&&)(*(R*)r);
	}

	/*!
	@brief ��strand�е���ĳ��������ֵ������Ȼ��ͨ��һ���ص������ѷ���ֵ����
	@param cb ���������ĺ���
	*/
	template <typename H, typename CB>
	void asyncInvoke(H&& h, CB&& cb)
	{
		try_tick(wrap_async_invoke<RM_CREF(H), RM_CREF(CB)>(TRY_MOVE(h), TRY_MOVE(cb)));
	}

	template <typename H, typename CB>
	void asyncInvokeVoid(H&& h, CB&& cb)
	{
		try_tick(wrap_async_invoke_void<RM_CREF(H), RM_CREF(CB)>(TRY_MOVE(h), TRY_MOVE(cb)));
	}
};

#endif
#ifndef __SHARED_STRAND_H
#define __SHARED_STRAND_H

#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <functional>
#include <memory>
#include "wrapped_post_handler.h"
#include "wrapped_try_tick_handler.h"
#include "wrapped_dispatch_handler.h"
#include "wrapped_next_tick_handler.h"
#include "wrapped_distribute_handler.h"
#include "strand_ex.h"
#include "io_engine.h"
#include "msg_queue.h"
#include "scattered.h"

class ActorTimer_;
class AsyncTimer_;
class my_actor;
class generator;
class overlap_timer;

class boost_strand;
typedef std::shared_ptr<boost_strand> shared_strand;

#ifdef ENABLE_NEXT_TICK

#define RUN_HANDLER handler_capture<Handler>(handler, this)

#else //ENABLE_NEXT_TICK

#define RUN_HANDLER std::forward<Handler>(handler)

#endif //ENABLE_NEXT_TICK

#define CHOOSE_POST()\
if (_strand)\
{\
	_strand->post(RUN_HANDLER); \
}\
else\
{\
	post_choose(std::forward<Handler>(handler)); \
};

#define CHOOSE_DISPATCH()\
if (_strand)\
{\
	_strand->dispatch(RUN_HANDLER); \
}\
else\
{\
	dispatch_choose(std::forward<Handler>(handler)); \
};

#ifdef ENABLE_NEXT_TICK

#define APPEND_TICK()\
	typedef wrap_next_tick_handler<Handler, true> wrap_tick_type1; \
	typedef wrap_next_tick_handler<Handler, false> wrap_tick_type2; \
	void* const space = alloc_space(sizeof(wrap_tick_type1)); \
	push_next_tick(space ? static_cast<wrap_next_tick_face*>(new(space)wrap_tick_type1(handler)) : \
	static_cast<wrap_next_tick_face*>(new(_reuMemAlloc->allocate(sizeof(wrap_tick_type2)))wrap_tick_type2(handler)));

#else //ENABLE_NEXT_TICK

#define APPEND_TICK()	post(std::forward<Handler>(handler));

#endif //ENABLE_NEXT_TICK


#define CHOOSE_TICK()\
if (_strand)\
{\
	APPEND_TICK(); \
}\
else\
{\
	post_choose(std::forward<Handler>(handler)); \
}

#ifdef DISABLE_BOOST_TIMER
struct TimerBoostCompletedEventFace_
{
	virtual void post_event(int tc) = 0;
};
#endif

class boost_strand
{
	typedef StrandEx_ strand_type;

#ifdef ENABLE_NEXT_TICK
	template <typename Handler>
	struct handler_capture
	{
		typedef RM_CREF(Handler) handler_type;

		handler_capture(Handler& handler, boost_strand* strand)
			:_strand(strand), _handler(std::forward<Handler>(handler)) {}

		void operator ()()
		{
			_strand->run_tick_front();
			CHECK_EXCEPTION(_handler);
			_strand->run_tick_back();
		}

		boost_strand* _strand;
		handler_type _handler;
	private:
		void operator =(const handler_capture&) = delete;
		COPY_CONSTRUCT2(handler_capture, _strand, _handler);
	};

	struct wrap_next_tick_face : public op_queue::face
	{
		virtual size_t invoke() = 0;
	};

	template <typename Handler, bool NtSpace>
	struct wrap_next_tick_handler : public wrap_next_tick_face
	{
		typedef RM_CREF(Handler) handler_type;

		wrap_next_tick_handler(Handler& handler)
			:_handler(std::forward<Handler>(handler)) {}

		size_t invoke()
		{
			CHECK_EXCEPTION(_handler);
			this->~wrap_next_tick_handler();
			return sizeof(wrap_next_tick_handler);
		}

		handler_type _handler;
		NONE_COPY(wrap_next_tick_handler);
	};

	template <typename Handler>
	struct wrap_next_tick_handler<Handler, false> : public wrap_next_tick_face
	{
		typedef RM_CREF(Handler) handler_type;

		wrap_next_tick_handler(Handler& handler)
			:_handler(std::forward<Handler>(handler)) {}

		size_t invoke()
		{
			CHECK_EXCEPTION(_handler);
			this->~wrap_next_tick_handler();
			return (size_t)-1 >> 1;
		}

		handler_type _handler;
		NONE_COPY(wrap_next_tick_handler);
	};
#endif //ENABLE_NEXT_TICK

	template <typename Handler, typename Callback>
	struct wrap_async_invoke
	{
		typedef RM_CREF(Handler) handler_type;
		typedef RM_CREF(Callback) callback_type;

		wrap_async_invoke(Handler& handler, Callback& callback)
			:_handler(std::forward<Handler>(handler)), _callback(std::forward<Callback>(callback)) {}

		void operator()()
		{
			BEGIN_CHECK_EXCEPTION;
			_callback(_handler());
			END_CHECK_EXCEPTION;
		}

		handler_type _handler;
		callback_type _callback;
	private:
		void operator =(const wrap_async_invoke&) = delete;
		COPY_CONSTRUCT2(wrap_async_invoke, _handler, _callback);
	};

	template <typename Handler, typename Callback>
	struct wrap_async_invoke_void
	{
		typedef RM_CREF(Handler) handler_type;
		typedef RM_CREF(Callback) callback_type;

		wrap_async_invoke_void(Handler& handler, Callback& callback)
			:_handler(std::forward<Handler>(handler)), _callback(std::forward<Callback>(callback)) {}

		void operator()()
		{
			BEGIN_CHECK_EXCEPTION;
			_handler();
			_callback();
			END_CHECK_EXCEPTION;
		}

		handler_type _handler;
		callback_type _callback;
	private:
		void operator =(const wrap_async_invoke_void&) = delete;
		COPY_CONSTRUCT2(wrap_async_invoke_void, _handler, _callback);
	};

	friend my_actor;
	friend generator;
	friend overlap_timer;
	friend io_engine;
	friend ActorTimer_;
	friend AsyncTimer_;
protected:
	enum strand_choose
	{
		strand_default,
		strand_ui,
		strand_uv
	};
protected:
	boost_strand();
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
	virtual ~boost_strand();
#else
	~boost_strand();
#endif
public:
	static shared_strand create(io_engine& ioEngine);
	static std::vector<shared_strand> create_multi(size_t n, io_engine& ioEngine);
	static void create_multi(shared_strand* res, size_t n, io_engine& ioEngine);
	static void create_multi(std::vector<shared_strand>& res, size_t n, io_engine& ioEngine);
	static void create_multi(std::list<shared_strand>& res, size_t n, io_engine& ioEngine);
public:
	/*!
	@brief ����ڱ�strand�е�����ֱ��ִ�У�������ӵ������еȴ�ִ��
	@param handler �����ú���
	*/
	template <typename Handler>
	void distribute(Handler&& handler)
	{
		if (running_in_this_thread())
		{
			handler();
		}
		else
		{
			post(std::forward<Handler>(handler));
		}
	}

	/*!
	@brief �����ǰstrandû�����ֱ��ִ�У�������ӵ������еȴ�ִ��
	*/
	template <typename Handler>
	void dispatch(Handler&&  handler)
	{
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
		CHOOSE_DISPATCH();
#else
		_strand->dispatch(RUN_HANDLER);
#endif
	}

	/*!
	@brief ���һ������ strand ����
	*/
	template <typename Handler>
	void post(Handler&& handler)
	{
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
		CHOOSE_POST();
#else
		_strand->post(RUN_HANDLER);
#endif
	}

	/*!
	@brief ���һ������ tick ����
	*/
	template <typename Handler>
	void next_tick(Handler&& handler)
	{
		assert(running_in_this_thread());
		assert(is_running());//����, strand��û��ʼ��һ��post���Ѿ���Ͷ��tick
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
		CHOOSE_TICK();
#else
		APPEND_TICK();
#endif
	}

	/*!
	@brief �������һ������ boost_tick ����
	*/
	template <typename Handler>
	void try_tick(Handler&& handler)
	{
#ifdef ENABLE_NEXT_TICK
		if (running_in_this_thread())
		{
			next_tick(std::forward<Handler>(handler));
		}
		else
#endif //ENABLE_NEXT_TICK
		{
			post(std::forward<Handler>(handler));
		}
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, Handler> wrap_asio(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, Handler>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, Handler, true> wrap_asio_once(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, Handler, true>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��distribute�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_distribute_handler<boost_strand, Handler> wrap(Handler&& handler)
	{
		return wrapped_distribute_handler<boost_strand, Handler>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��distribute�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_distribute_handler<boost_strand, Handler, true> wrap_once(Handler&& handler)
	{
		return wrapped_distribute_handler<boost_strand, Handler, true>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��post��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, Handler> wrap_post(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, Handler>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��post�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, Handler, true> wrap_post_once(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, Handler, true>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��tick��
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, Handler> wrap_next_tick(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, Handler>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��tick�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, Handler, true> wrap_next_tick_once(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, Handler, true>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��tick��
	*/
	template <typename Handler>
	wrapped_try_tick_handler<boost_strand, Handler> wrap_try_tick(Handler&& handler)
	{
		return wrapped_try_tick_handler<boost_strand, Handler>(this, handler);
	}

	/*!
	@brief �ѱ����ú�����װ��tick�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_try_tick_handler<boost_strand, Handler, true> wrap_try_tick_once(Handler&& handler)
	{
		return wrapped_try_tick_handler<boost_strand, Handler, true>(this, handler);
	}

	/*!
	@brief ����һ������ͬһ��ios��strand
	*/
	shared_strand clone();

	/*!
	@brief ����Ƿ���������ios�߳���ִ��
	@return true ��, false ����
	*/
	virtual bool in_this_ios();

	/*!
	@brief ��⵱ǰ������ios�Ƿ��ǵ��߳�
	*/
	virtual bool sync_safe();

	/*!
	@brief ������ios�����������߳���
	*/
	size_t io_threads();

	/*!
	@brief ����ջ��ֻ�е�ǰstrand
	*/
	virtual bool only_self();

	/*!
	@brief �ж��Ƿ��ڱ�strand������
	*/
	virtual bool running_in_this_thread();

	/*!
	@brief ��ǰ��������Ƿ�Ϊ��(���㵱ǰ)
	@param checkTick �Ƿ����tick����
	*/
	bool empty(bool checkTick = true);

	/*!
	@brief �Ƿ�������
	*/
	virtual bool is_running();

	/*!
	@brief �Ƿ�������
	*/
	bool safe_is_running();

	/*!
	@brief ��ȡ��ǰ����������
	*/
	io_engine& get_io_engine();

	/*!
	@brief ��ȡ��ǰ������
	*/
	boost::asio::io_service& get_io_service();

	/*!
	@brief ����һ����ʱ��
	*/
	std::shared_ptr<AsyncTimer_> make_timer();

	/*!
	@brief ��ȡ�ص���ʱ��
	*/
	overlap_timer* over_timer();
private:
	/*!
	@brief ��ȡActor��ʱ��
	*/
	ActorTimer_* actor_timer();
#ifdef ENABLE_QT_ACTOR
	template <typename Handler>
	void post_ui(Handler&& handler);

	template <typename Handler>
	void dispatch_ui(Handler&& handler);
#endif
#ifdef ENABLE_UV_ACTOR
	template <typename Handler>
	void post_uv(Handler&& handler);

	template <typename Handler>
	void dispatch_uv(Handler&& handler);
#endif
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
	template <typename Handler>
	void post_choose(Handler&& handler)
	{
#if (ENABLE_QT_ACTOR && ENABLE_UV_ACTOR)
		if (strand_ui == _strandChoose)
		{
			post_ui(std::forward<Handler>(handler));
		}
		else
		{
			post_uv(std::forward<Handler>(handler));
		}
#elif ENABLE_QT_ACTOR
		post_ui(std::forward<Handler>(handler));
#elif ENABLE_UV_ACTOR
		post_uv(std::forward<Handler>(handler));
#endif
	}

	template <typename Handler>
	void dispatch_choose(Handler&& handler)
	{
#if (ENABLE_QT_ACTOR && ENABLE_UV_ACTOR)
		if (strand_ui == _strandChoose)
		{
			dispatch_ui(std::forward<Handler>(handler));
		}
		else
		{
			dispatch_uv(std::forward<Handler>(handler));
		}
#elif ENABLE_QT_ACTOR
		dispatch_ui(std::forward<Handler>(handler));
#elif ENABLE_UV_ACTOR
		dispatch_uv(std::forward<Handler>(handler));
#endif
	}
#endif
	void* alloc_space(size_t size);
protected:
#ifdef ENABLE_NEXT_TICK
	bool ready_empty();
	bool waiting_empty();
	void push_next_tick(wrap_next_tick_face* handler);
	void run_tick_front();
	void run_tick_back();
	size_t _thisRoundCount;
	reusable_mem* _reuMemAlloc;
	mem_alloc_base* _nextTickAlloc[3];
	op_queue _backTickQueue;
	op_queue _frontTickQueue;
#endif //ENABLE_NEXT_TICK
protected:
#if (ENABLE_QT_ACTOR && ENABLE_UV_ACTOR)
	strand_choose _strandChoose;
#endif
protected:
	ActorTimer_* _actorTimer;
	overlap_timer* _overTimer;
	io_engine* _ioEngine;
	strand_type* _strand;
	std::weak_ptr<boost_strand> _weakThis;
	NONE_COPY(boost_strand);
public:
	/*!
	@brief ��һ��strand�е���ĳ��������ֱ�����������ִ����ɺ�ŷ���
	@warning �˺����п���ʹ������������������ֻ������strand��������ios�޹��߳��е���
	*/
	template <typename H>
	void syncInvoke(H&& h)
	{
		assert(!in_this_ios());
		std::mutex mutex;
		std::condition_variable con;
		std::unique_lock<std::mutex> ul(mutex);
		post([&]
		{
			BEGIN_CHECK_EXCEPTION;
			h();
			END_CHECK_EXCEPTION;
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
		__space_align char space[sizeof(R)];
		std::mutex mutex;
		std::condition_variable con;
		std::unique_lock<std::mutex> ul(mutex);
		post([&]
		{
			BEGIN_CHECK_EXCEPTION;
			new(space)R(h());
			END_CHECK_EXCEPTION;
			mutex.lock();
			con.notify_one();
			mutex.unlock();
		});
		con.wait(ul);
		BREAK_OF_SCOPE(
		{
			as_ptype<R>(space)->~R();
		});
		return std::forward<R>(*as_ptype<R>(space));
	}

	/*!
	@brief ��strand�е���ĳ��������ֵ������Ȼ��ͨ��һ���ص������ѷ���ֵ����
	@param cb ���������ĺ���
	*/
	template <typename Handler, typename Callback>
	void asyncInvoke(Handler&& h, Callback&& cb)
	{
		try_tick(wrap_async_invoke<Handler, Callback>(h, cb));
	}

	template <typename Handler, typename Callback>
	void asyncInvokeVoid(Handler&& h, Callback&& cb)
	{
		try_tick(wrap_async_invoke_void<Handler, Callback>(h, cb));
	}
};

#undef RUN_HANDLER
#undef CHOOSE_TICK
#undef CHOOSE_POST
#undef CHOOSE_DISPATCH
#undef CHOOSE_POST_FRONT
#undef CHOOSE_DISPATCH_FRONT
#undef APPEND_TICK

#endif
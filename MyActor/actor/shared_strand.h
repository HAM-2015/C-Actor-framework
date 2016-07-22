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
class TimerBoost_;
class AsyncTimer_;
class my_actor;

class boost_strand;
typedef std::shared_ptr<boost_strand> shared_strand;

#define SPACE_SIZE		(sizeof(void*)*16)

#ifdef ENABLE_NEXT_TICK

#define RUN_HANDLER handler_capture<RM_CREF(Handler)>(TRY_MOVE(handler), this)

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
	post_ui(TRY_MOVE(handler)); \
};

#define UI_DISPATCH()\
if (_strand)\
{\
	_strand->dispatch(RUN_HANDLER); \
}\
else\
{\
	dispatch_ui(TRY_MOVE(handler)); \
};

#ifdef ENABLE_POST_FRONT
#define UI_POST_FRONT()\
if (_strand)\
{\
	_strand->post_front(RUN_HANDLER); \
}\
else\
{\
	post_ui(TRY_MOVE(handler)); \
};

#define UI_DISPATCH_FRONT()\
if (_strand)\
{\
	_strand->dispatch_front(RUN_HANDLER); \
}\
else\
{\
	dispatch_ui(TRY_MOVE(handler)); \
};
#endif //ENABLE_POST_FRONT

#ifdef ENABLE_NEXT_TICK

#define APPEND_TICK()	\
	static_assert(sizeof(wrap_next_tick_space) == sizeof(wrap_next_tick_handler<RM_CREF(Handler)>), "next tick wrap error"); \
	typedef wrap_next_tick_handler<RM_CREF(Handler), sizeof(RM_CREF(Handler)) <= SPACE_SIZE> wrap_tick_type; \
	_backTickQueue->push_back(wrap_tick_type::create(TRY_MOVE(handler), _nextTickAlloc, _reuMemAlloc));
#else //ENABLE_NEXT_TICK

#define APPEND_TICK()	post(TRY_MOVE(handler));

#endif //ENABLE_NEXT_TICK


#define UI_TICK()\
if (_strand)\
{\
	APPEND_TICK(); \
}\
else\
{\
	post_ui(TRY_MOVE(handler)); \
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

	struct capture_base
	{
		capture_base(boost_strand* strand);
		void begin_run();
		void end_run();
		boost_strand* _strand;
	};

	template <typename H>
	struct handler_capture : public capture_base
	{
		template <typename Handler>
		handler_capture(Handler&& handler, boost_strand* strand)
			:capture_base(strand), _handler(TRY_MOVE(handler)) {}

		handler_capture(const handler_capture& s)
			:capture_base(s._strand), _handler(std::move(s._handler)) {}

		handler_capture(handler_capture&& s)
			:capture_base(s._strand), _handler(std::move(s._handler)) {}

		void operator ()()
		{
			begin_run();
			_handler();
			end_run();
		}

		mutable H _handler;
	private:
		void operator =(const handler_capture&);
	};

	struct wrap_next_tick_base
	{
		struct result
		{
			void* _ptr;
			int _size;
		};

		virtual result invoke() = 0;
	};

	struct wrap_next_tick_space : public wrap_next_tick_base
	{
		__space_align char space[SPACE_SIZE];
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
		static inline wrap_next_tick_base* create(Handler&& handler, mem_alloc_base* alloc, reusable_mem* reuAlloc)
		{
			if (!alloc->overflow())
			{
				return new(alloc->allocate())wrap_next_tick_handler<RM_CREF(Handler)>(TRY_MOVE(handler));
			}
			else
			{
				return wrap_next_tick_handler<RM_CREF(Handler), false>::create(TRY_MOVE(handler), alloc, reuAlloc);
			}
		}

		result invoke()
		{
			(*(H*)space)();
			this->~wrap_next_tick_handler();
			return { this, -1 };
		}
		NONE_COPY(wrap_next_tick_handler);
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
		static inline wrap_next_tick_base* create(Handler&& handler, mem_alloc_base* alloc, reusable_mem* reuAlloc)
		{
			return new(reuAlloc->allocate(sizeof(wrap_next_tick_handler<RM_CREF(Handler), false>)))wrap_next_tick_handler<RM_CREF(Handler), false>(TRY_MOVE(handler));
		}

		result invoke()
		{
			_h();
			this->~wrap_next_tick_handler();
			return { this, sizeof(wrap_next_tick_handler) };
		}

		H _h;
		NONE_COPY(wrap_next_tick_handler);
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
			:_h(std::move(s._h)), _cb(std::move(s._cb)) {}

		void operator()()
		{
			BEGIN_CHECK_EXCEPTION;
			_cb(_h());
			END_CHECK_EXCEPTION;
		}

		mutable H _h;
		mutable CB _cb;
	private:
		void operator=(const wrap_async_invoke&);
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
			:_h(std::move(s._h)), _cb(std::move(s._cb)) {}

		void operator()()
		{
			BEGIN_CHECK_EXCEPTION;
			_h();
			_cb();
			END_CHECK_EXCEPTION;
		}

		mutable H _h;
		mutable CB _cb;
	private:
		void operator=(const wrap_async_invoke_void&);
	};

	friend my_actor;
	friend io_engine;
	friend ActorTimer_;
	friend TimerBoost_;
protected:
	boost_strand();
#ifdef ENABLE_QT_ACTOR
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
	@brief 如果在本strand中调用则直接执行，否则添加到队列中等待执行
	@param handler 被调用函数
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
			post(TRY_MOVE(handler));
		}
	}

	/*!
	@brief 如果当前strand没任务就直接执行，否则添加到队列中等待执行
	*/
	template <typename Handler>
	void dispatch(Handler&&  handler)
	{
#ifdef ENABLE_QT_ACTOR
		UI_DISPATCH();
#else
		_strand->dispatch(RUN_HANDLER);
#endif
	}

	/*!
	@brief 添加一个任务到 strand 队列
	*/
	template <typename Handler>
	void post(Handler&& handler)
	{
#ifdef ENABLE_QT_ACTOR
		UI_POST();
#else
		_strand->post(RUN_HANDLER);
#endif
	}

#ifdef ENABLE_POST_FRONT
	/*!
	@brief 如果在本strand中调用则直接执行，否则添加到队列头部等待执行
	@param handler 被调用函数
	*/
	template <typename Handler>
	void distribute_front(Handler&& handler)
	{
		if (running_in_this_thread())
		{
			handler();
		}
		else
		{
			post_front(TRY_MOVE(handler));
		}
	}

	/*!
	@brief 如果当前strand没任务就直接执行，否则添加到队列头部等待执行
	*/
	template <typename Handler>
	void dispatch_front(Handler&&  handler)
	{
#ifdef ENABLE_QT_ACTOR
		UI_DISPATCH_FRONT();
#else
		_strand->dispatch_front(RUN_HANDLER);
#endif
	}

	/*!
	@brief 添加一个任务到 strand 等待队列头部
	*/
	template <typename Handler>
	void post_front(Handler&& handler)
	{
#ifdef ENABLE_QT_ACTOR
		UI_POST_FRONT();
#else
		_strand->post_front(RUN_HANDLER);
#endif
	}
#endif //ENABLE_POST_FRONT

	/*!
	@brief 添加一个任务到 tick 队列
	*/
	template <typename Handler>
	void next_tick(Handler&& handler)
	{
		assert(running_in_this_thread());
		assert(is_running());//错误, strand还没开始第一个post就已经再投递tick
#ifdef ENABLE_QT_ACTOR
		UI_TICK();
#else
		APPEND_TICK();
#endif
	}

	/*!
	@brief 尝试添加一个任务到 boost_tick 队列
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
	@brief 把被调用函数包装到dispatch中，用于不同strand间消息传递
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, RM_CREF(Handler)> wrap_asio(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到dispatch中，用于不同strand间消息传递，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_asio(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到distribute中，用于不同strand间消息传递
	*/
	template <typename Handler>
	wrapped_distribute_handler<boost_strand, RM_CREF(Handler)> wrap(Handler&& handler)
	{
		return wrapped_distribute_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到distribute中，用于不同strand间消息传递，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_distribute_handler<boost_strand, RM_CREF(Handler), true> suck_wrap(Handler&& handler)
	{
		return wrapped_distribute_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到post中
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, RM_CREF(Handler)> wrap_post(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到post中，并且锁定ios，直到释放通知函数包
	*/
	template <typename Handler>
	wrapped_hold_work_post_handler<boost_strand, RM_CREF(Handler)> wrap_hold_post(Handler&& handler)
	{
		return wrapped_hold_work_post_handler<boost_strand, RM_CREF(Handler)>(get_io_service(), this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到post中，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_post(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

#ifdef ENABLE_POST_FRONT
	/*!
	@brief 把被调用函数包装到dispatch_front中，用于不同strand间消息传递
	*/
	template <typename Handler>
	wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler)> wrap_asio_front(Handler&& handler)
	{
		return wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到dispatch_front中，用于不同strand间消息传递，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_asio_front(Handler&& handler)
	{
		return wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到distribute_front中，用于不同strand间消息传递
	*/
	template <typename Handler>
	wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler)> wrap_front(Handler&& handler)
	{
		return wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到distribute_front中，用于不同strand间消息传递，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_front(Handler&& handler)
	{
		return wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到post_front中
	*/
	template <typename Handler>
	wrapped_post_front_handler<boost_strand, RM_CREF(Handler)> wrap_post_front(Handler&& handler)
	{
		return wrapped_post_front_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到post_front中，并且锁定ios，直到释放通知函数包
	*/
	template <typename Handler>
	wrapped_hold_work_post_front_handler<boost_strand, RM_CREF(Handler)> wrap_hold_post_front(Handler&& handler)
	{
		return wrapped_hold_work_post_front_handler<boost_strand, RM_CREF(Handler)>(get_io_service(), this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到post_front中，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_post_front_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_post_front(Handler&& handler)
	{
		return wrapped_post_front_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}
#endif //ENABLE_POST_FRONT

	/*!
	@brief 把被调用函数包装到tick中
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, RM_CREF(Handler)> wrap_next_tick(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到tick中，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_next_tick(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到tick中
	*/
	template <typename Handler>
	wrapped_try_tick_handler<boost_strand, RM_CREF(Handler)> wrap_try_tick(Handler&& handler)
	{
		return wrapped_try_tick_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 把被调用函数包装到tick中，调用后参数将强制被右值引用，且只能调用一次
	*/
	template <typename Handler>
	wrapped_try_tick_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_try_tick(Handler&& handler)
	{
		return wrapped_try_tick_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief 复制一个依赖同一个ios的strand
	*/
	shared_strand clone();

	/*!
	@brief 检查是否在所依赖ios线程中执行
	@return true 在, false 不在
	*/
	virtual bool in_this_ios();

	/*!
	@brief 检测当前依赖的ios是否是单线程
	*/
	virtual bool sync_safe();

	/*!
	@brief 依赖的ios调度器运行线程数
	*/
	size_t ios_thread_number();

	/*!
	@brief 判断是否在本strand中运行
	*/
	virtual bool running_in_this_thread();

	/*!
	@brief 当前任务队列是否为空(不算当前)
	@param checkTick 是否计算tick任务
	*/
	bool empty(bool checkTick = true);

	/*!
	@brief 是否在运行
	*/
	virtual bool is_running();

	/*!
	@brief 获取当前调度器代理
	*/
	io_engine& get_io_engine();

	/*!
	@brief 获取当前调度器
	*/
	boost::asio::io_service& get_io_service();

	/*!
	@brief 创建一个定时器
	*/
	std::shared_ptr<AsyncTimer_> make_timer();
private:
	/*!
	@brief 获取Actor定时器
	*/
	ActorTimer_* actor_timer();
#ifdef ENABLE_QT_ACTOR
	template <typename Handler>
	void post_ui(Handler&& handler);

	template <typename Handler>
	void dispatch_ui(Handler&& handler);
#endif
protected:
#ifdef ENABLE_NEXT_TICK
	bool ready_empty();
	bool waiting_empty();
	void run_tick_front();
	void run_tick_back();
	size_t _thisRoundCount;
	reusable_mem* _reuMemAlloc;
	mem_alloc2<wrap_next_tick_space>* _nextTickAlloc;
	msg_queue<wrap_next_tick_base*, mem_alloc2<>>* _backTickQueue;
	msg_queue<wrap_next_tick_base*, mem_alloc2<>>* _frontTickQueue;
#endif //ENABLE_NEXT_TICK
protected:
	ActorTimer_* _actorTimer;
	TimerBoost_* _timerBoost;
	io_engine* _ioEngine;
	strand_type* _strand;
	std::weak_ptr<boost_strand> _weakThis;
	NONE_COPY(boost_strand);
public:
	/*!
	@brief 在一个strand中调用某个函数，直到这个函数被执行完成后才返回
	@warning 此函数有可能使整个程序陷入死锁，只能在与strand所依赖的ios无关线程中调用
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
	@brief 同上，带返回值
	*/
	template <typename R, typename H>
	R syncInvoke(H&& h)
	{
		assert(!in_this_ios());
		__space_align char r[sizeof(R)];
		std::mutex mutex;
		std::condition_variable con;
		std::unique_lock<std::mutex> ul(mutex);
		post([&]
		{
			BEGIN_CHECK_EXCEPTION;
			new(r)R(h());
			END_CHECK_EXCEPTION;
			mutex.lock();
			con.notify_one();
			mutex.unlock();
		});
		con.wait(ul);
		BREAK_OF_SCOPE(
		{
			typedef R T_;
			((T_*)r)->~R();
		});
		return (R&&)(*(R*)r);
	}

	/*!
	@brief 在strand中调用某个带返回值函数，然后通过一个回调函数把返回值传出
	@param cb 传出参数的函数
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

#undef SPACE_SIZE
#undef RUN_HANDLER
#undef UI_POST
#undef UI_DISPATCH
#undef APPEND_TICK
#undef UI_NEXT_TICK

#endif
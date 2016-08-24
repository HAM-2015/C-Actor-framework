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

#define NEXT_TICK_SPACE_SIZE (sizeof(void*)*8)

#ifdef ENABLE_NEXT_TICK

#define RUN_HANDLER handler_capture<RM_CREF(Handler)>(TRY_MOVE(handler), this)

#else //ENABLE_NEXT_TICK

#define RUN_HANDLER TRY_MOVE(handler)

#endif //ENABLE_NEXT_TICK

#define CHOOSE_POST()\
if (_strand)\
{\
	_strand->post(RUN_HANDLER); \
}\
else\
{\
	post_choose(TRY_MOVE(handler)); \
};

#define CHOOSE_DISPATCH()\
if (_strand)\
{\
	_strand->dispatch(RUN_HANDLER); \
}\
else\
{\
	dispatch_choose(TRY_MOVE(handler)); \
};

#ifdef ENABLE_POST_FRONT
#define CHOOSE_POST_FRONT()\
if (_strand)\
{\
	_strand->post_front(RUN_HANDLER); \
}\
else\
{\
	post_choose(TRY_MOVE(handler)); \
};

#define CHOOSE_DISPATCH_FRONT()\
if (_strand)\
{\
	_strand->dispatch_front(RUN_HANDLER); \
}\
else\
{\
	dispatch_choose(TRY_MOVE(handler)); \
};
#endif //ENABLE_POST_FRONT

#ifdef ENABLE_NEXT_TICK

#define APPEND_TICK()\
	typedef wrap_next_tick_handler<RM_CREF(Handler), true> wrap_tick_type1; \
	typedef wrap_next_tick_handler<RM_CREF(Handler), false> wrap_tick_type2; \
	void* const space = alloc_space(sizeof(wrap_tick_type1)); \
	if (space) _backTickQueue->push_back(new(space)wrap_tick_type1(TRY_MOVE(handler))); \
	else _backTickQueue->push_back(new(_reuMemAlloc->allocate(sizeof(wrap_tick_type2)))wrap_tick_type2(TRY_MOVE(handler)));

#else //ENABLE_NEXT_TICK

#define APPEND_TICK()	post(TRY_MOVE(handler));

#endif //ENABLE_NEXT_TICK


#define CHOOSE_TICK()\
if (_strand)\
{\
	APPEND_TICK(); \
}\
else\
{\
	post_choose(TRY_MOVE(handler)); \
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
		void operator =(const handler_capture&) = delete;
	};

	struct wrap_next_tick_face
	{
		virtual size_t invoke() = 0;
	};

	template <typename Handler, bool NtSpace>
	struct wrap_next_tick_handler : public wrap_next_tick_face
	{
		template <typename H>
		wrap_next_tick_handler(H&& h)
			:_handler(TRY_MOVE(h)) {}

		size_t invoke()
		{
			CHECK_EXCEPTION(_handler);
			this->~wrap_next_tick_handler();
			return sizeof(*this);
		}

		Handler _handler;
		NONE_COPY(wrap_next_tick_handler);
	};

	template <typename Handler>
	struct wrap_next_tick_handler<Handler, false> : public wrap_next_tick_face
	{
		template <typename H>
		wrap_next_tick_handler(H&& h)
			:_handler(TRY_MOVE(h)) {}

		size_t invoke()
		{
			CHECK_EXCEPTION(_handler);
			this->~wrap_next_tick_handler();
			return (size_t)-1 >> 1;
		}

		Handler _handler;
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
		void operator =(const wrap_async_invoke&) = delete;
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
		void operator =(const wrap_async_invoke_void&) = delete;
	};

	friend my_actor;
	friend io_engine;
	friend ActorTimer_;
	friend TimerBoost_;
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
			post(TRY_MOVE(handler));
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

#ifdef ENABLE_POST_FRONT
	/*!
	@brief ����ڱ�strand�е�����ֱ��ִ�У�������ӵ�����ͷ���ȴ�ִ��
	@param handler �����ú���
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
	@brief �����ǰstrandû�����ֱ��ִ�У�������ӵ�����ͷ���ȴ�ִ��
	*/
	template <typename Handler>
	void dispatch_front(Handler&&  handler)
	{
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
		CHOOSE_DISPATCH_FRONT();
#else
		_strand->dispatch_front(RUN_HANDLER);
#endif
	}

	/*!
	@brief ���һ������ strand �ȴ�����ͷ��
	*/
	template <typename Handler>
	void post_front(Handler&& handler)
	{
#if (ENABLE_QT_ACTOR || ENABLE_UV_ACTOR)
		CHOOSE_POST_FRONT();
#else
		_strand->post_front(RUN_HANDLER);
#endif
	}
#endif //ENABLE_POST_FRONT

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
	wrapped_dispatch_handler<boost_strand, RM_CREF(Handler)> wrap_asio(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_dispatch_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_asio(Handler&& handler)
	{
		return wrapped_dispatch_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��distribute�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_distribute_handler<boost_strand, RM_CREF(Handler)> wrap(Handler&& handler)
	{
		return wrapped_distribute_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��distribute�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_distribute_handler<boost_strand, RM_CREF(Handler), true> suck_wrap(Handler&& handler)
	{
		return wrapped_distribute_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
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
	@brief �ѱ����ú�����װ��post�У���������ios��ֱ���ͷ�֪ͨ������
	*/
	template <typename Handler>
	wrapped_hold_work_post_handler<boost_strand, RM_CREF(Handler)> wrap_hold_post(Handler&& handler)
	{
		return wrapped_hold_work_post_handler<boost_strand, RM_CREF(Handler)>(get_io_service(), this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��post�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_post_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_post(Handler&& handler)
	{
		return wrapped_post_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

#ifdef ENABLE_POST_FRONT
	/*!
	@brief �ѱ����ú�����װ��dispatch_front�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler)> wrap_asio_front(Handler&& handler)
	{
		return wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��dispatch_front�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_asio_front(Handler&& handler)
	{
		return wrapped_dispatch_front_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��distribute_front�У����ڲ�ͬstrand����Ϣ����
	*/
	template <typename Handler>
	wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler)> wrap_front(Handler&& handler)
	{
		return wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��distribute_front�У����ڲ�ͬstrand����Ϣ���ݣ����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_front(Handler&& handler)
	{
		return wrapped_distribute_front_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��post_front��
	*/
	template <typename Handler>
	wrapped_post_front_handler<boost_strand, RM_CREF(Handler)> wrap_post_front(Handler&& handler)
	{
		return wrapped_post_front_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��post_front�У���������ios��ֱ���ͷ�֪ͨ������
	*/
	template <typename Handler>
	wrapped_hold_work_post_front_handler<boost_strand, RM_CREF(Handler)> wrap_hold_post_front(Handler&& handler)
	{
		return wrapped_hold_work_post_front_handler<boost_strand, RM_CREF(Handler)>(get_io_service(), this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��post_front�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
	*/
	template <typename Handler>
	wrapped_post_front_handler<boost_strand, RM_CREF(Handler), true> suck_wrap_post_front(Handler&& handler)
	{
		return wrapped_post_front_handler<boost_strand, RM_CREF(Handler), true>(this, TRY_MOVE(handler));
	}
#endif //ENABLE_POST_FRONT

	/*!
	@brief �ѱ����ú�����װ��tick��
	*/
	template <typename Handler>
	wrapped_next_tick_handler<boost_strand, RM_CREF(Handler)> wrap_next_tick(Handler&& handler)
	{
		return wrapped_next_tick_handler<boost_strand, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief �ѱ����ú�����װ��tick�У����ú������ǿ�Ʊ���ֵ���ã���ֻ�ܵ���һ��
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
	size_t ios_thread_number();

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
			post_ui(TRY_MOVE(handler));
		}
		else
		{
			post_uv(TRY_MOVE(handler));
		}
#elif ENABLE_QT_ACTOR
		post_ui(TRY_MOVE(handler));
#elif ENABLE_UV_ACTOR
		post_uv(TRY_MOVE(handler));
#endif
	}

	template <typename Handler>
	void dispatch_choose(Handler&& handler)
	{
#if (ENABLE_QT_ACTOR && ENABLE_UV_ACTOR)
		if (strand_ui == _strandChoose)
		{
			dispatch_ui(TRY_MOVE(handler));
		}
		else
		{
			dispatch_uv(TRY_MOVE(handler));
		}
#elif ENABLE_QT_ACTOR
		dispatch_ui(TRY_MOVE(handler));
#elif ENABLE_UV_ACTOR
		dispatch_uv(TRY_MOVE(handler));
#endif
	}
#endif
	void* alloc_space(size_t size);
protected:
#ifdef ENABLE_NEXT_TICK
	bool ready_empty();
	bool waiting_empty();
	void run_tick_front();
	void run_tick_back();
	size_t _thisRoundCount;
	reusable_mem* _reuMemAlloc;
	mem_alloc_base* _nextTickAlloc[3];
	msg_queue<wrap_next_tick_face*, mem_alloc2<>>* _backTickQueue;
	msg_queue<wrap_next_tick_face*, mem_alloc2<>>* _frontTickQueue;
#endif //ENABLE_NEXT_TICK
protected:
#if (ENABLE_QT_ACTOR && ENABLE_UV_ACTOR)
	strand_choose _strandChoose;
#endif
protected:
	ActorTimer_* _actorTimer;
	TimerBoost_* _timerBoost;
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

#undef RUN_HANDLER
#undef CHOOSE_TICK
#undef CHOOSE_POST
#undef CHOOSE_DISPATCH
#undef CHOOSE_POST_FRONT
#undef CHOOSE_DISPATCH_FRONT
#undef APPEND_TICK

#endif
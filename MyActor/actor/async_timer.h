#ifndef __ASYNC_TIMER_H
#define __ASYNC_TIMER_H

#include <algorithm>
#include "run_strand.h"
#include "msg_queue.h"
#include "mem_pool.h"
#include "stack_object.h"

class ActorTimer_;
class overlap_timer;
class qt_strand;
class uv_strand;
class boost_strand;
typedef std::shared_ptr<AsyncTimer_> async_timer;

/*!
@brief �첽��ʱ����һ����ʱѭ��һ��AsyncTimer_
*/
class AsyncTimer_ : public ActorTimerFace_
{
	friend boost_strand;
	friend overlap_timer;
	FRIEND_SHARED_PTR(AsyncTimer_);

	struct wrap_base
	{
		virtual void invoke(bool isAdvance = false) = 0;
		virtual void destroy(reusable_mem& reuMem) = 0;
		virtual void set_sign(bool* sign) = 0;
		virtual bool is_top_call() = 0;
		virtual wrap_base* interval_handler() = 0;
	};

	template <typename Handler>
	struct wrap_handler : public wrap_base
	{
		template <typename H>
		wrap_handler(H&& h)
			:_h(std::forward<H>(h)) {}

		void invoke(bool isAdvance = false)
		{
			CHECK_EXCEPTION(_h);
		}

		void destroy(reusable_mem& reuMem)
		{
			this->~wrap_handler();
			reuMem.deallocate(this);
		}

		void set_sign(bool* sign) {}
		bool is_top_call() { return false; }
		wrap_base* interval_handler() { return NULL; }

		Handler _h;
	};

	template <typename Handler>
	struct wrap_advance_handler : public wrap_base
	{
		template <typename H>
		wrap_advance_handler(H&& h)
			:_h(std::forward<H>(h)) {}

		void invoke(bool isAdvance = false)
		{
			CHECK_EXCEPTION(_h, isAdvance);
		}

		void destroy(reusable_mem& reuMem)
		{
			this->~wrap_advance_handler();
			reuMem.deallocate(this);
		}

		void set_sign(bool* sign) {}
		bool is_top_call() { return false; }
		wrap_base* interval_handler() { return NULL; }

		Handler _h;
	};

	template <typename Handler>
	struct wrap_interval_handler : public wrap_base
	{
		template <typename H>
		wrap_interval_handler(H&& h, wrap_base* intervalHandler)
			:_h(std::forward<H>(h)), _sign(NULL), _intervalHandler(intervalHandler) {}

		void invoke(bool isAdvance = false)
		{
			CHECK_EXCEPTION(_h, this);
		}

		void destroy(reusable_mem& reuMem)
		{
			if (!_sign)
			{
				_intervalHandler->destroy(reuMem);
			}
			else
			{
				*_sign = true;
			}
			this->~wrap_interval_handler();
			reuMem.deallocate(this);
		}

		void set_sign(bool* sign)
		{
			_sign = sign;
		}

		bool is_top_call()
		{
			return NULL != _sign;
		}

		wrap_base* interval_handler()
		{
			return _intervalHandler;
		}

		bool* _sign;
		wrap_base* _intervalHandler;
		Handler _h;
	};

	template <typename Handler>
	struct wrap_advance_interval_handler : public wrap_base
	{
		template <typename H>
		wrap_advance_interval_handler(H&& h, wrap_base* intervalHandler)
			:_h(std::forward<H>(h)), _sign(NULL), _intervalHandler(intervalHandler) {}

		void invoke(bool isAdvance = false)
		{
			CHECK_EXCEPTION(_h, isAdvance, this);
		}

		void destroy(reusable_mem& reuMem)
		{
			if (!_sign)
			{
				_intervalHandler->destroy(reuMem);
			}
			else
			{
				*_sign = true;
			}
			this->~wrap_advance_interval_handler();
			reuMem.deallocate(this);
		}

		void set_sign(bool* sign)
		{
			_sign = sign;
		}

		bool is_top_call()
		{
			return NULL != _sign;
		}

		wrap_base* interval_handler()
		{
			return _intervalHandler;
		}

		bool* _sign;
		wrap_base* _intervalHandler;
		Handler _h;
	};
private:
	AsyncTimer_(ActorTimer_* actorTimer);
	~AsyncTimer_();
public:
	/*!
	@brief ����һ����ʱ����������strand�߳��е���
	*/
	template <typename Handler>
	long long timeout(int ms, Handler&& handler)
	{
		return timeout_us((long long)ms * 1000, std::forward<Handler>(handler));
	}

	template <typename Handler>
	long long timeout_us(long long us, Handler&& handler)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		_isInterval = false;
		_handler = wrap_timer_handler(_reuMem, std::forward<Handler>(handler));
		_timerHandle = _actorTimer->timeout(us, _weakThis.lock(), false);
		return _timerHandle._beginStamp;
	}

	template <typename Handler>
	long long timeout2(int ms, Handler&& handler)
	{
		return timeout_us2((long long)ms * 1000, std::forward<Handler>(handler));
	}

	template <typename Handler>
	long long timeout_us2(long long us, Handler&& handler)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		_isInterval = false;
		_handler = wrap_advance_timer_handler(_reuMem, std::forward<Handler>(handler));
		_timerHandle = _actorTimer->timeout(us, _weakThis.lock(), false);
		return _timerHandle._beginStamp;
	}

	/*!
	@brief ����һ�����Զ�ʱ����������strand�߳��е���
	*/
	template <typename Handler>
	long long deadline(long long us, Handler&& handler)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		_isInterval = false;
		_handler = wrap_timer_handler(_reuMem, std::forward<Handler>(handler));
		_timerHandle = _actorTimer->timeout(us, _weakThis.lock(), true);
		return _timerHandle._beginStamp;
	}

	template <typename Handler>
	long long deadline2(long long us, Handler&& handler)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		_isInterval = false;
		_handler = wrap_advance_timer_handler(_reuMem, std::forward<Handler>(handler));
		_timerHandle = _actorTimer->timeout(us, _weakThis.lock(), true);
		return _timerHandle._beginStamp;
	}

	/*!
	@brief ѭ����ʱ����һ��handler����������strand�߳��е���
	*/
	template <typename Handler>
	void interval(int ms, Handler&& handler, bool immed = false)
	{
		interval_us((long long)ms * 1000, std::forward<Handler>(handler), immed);
	}

	template <typename Handler>
	void interval2(int ms, Handler&& handler, bool immed = false)
	{
		interval_us2((long long)ms * 1000, std::forward<Handler>(handler), immed);
	}

	template <typename Handler>
	void interval_us(long long intervalus, Handler&& handler, bool immed = false)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		_isInterval = true;
		long long const deadtime = get_tick_us() + intervalus;
		_handler = wrap_interval_timer_handler(_reuMem, std::bind([this, intervalus](wrap_base* const thisHandler, long long& deadtime)
		{
			bool sign = false;
			AsyncTimer_* const this_ = this;
			wrap_base* const intervalHandler = thisHandler->interval_handler();
			thisHandler->set_sign(&sign);
			intervalHandler->invoke();
			if (!sign)
			{
				thisHandler->set_sign(NULL);
				deadtime += intervalus;
				_timerHandle = _actorTimer->timeout(deadtime, _weakThis.lock(), true);
			}
			else
			{
				intervalHandler->destroy(this_->_reuMem);
			}
		}, __1, deadtime), wrap_timer_handler(_reuMem, std::forward<Handler>(handler)));
		_timerHandle = _actorTimer->timeout(deadtime, _weakThis.lock(), true);
		if (immed)
		{
			_handler->invoke();
		}
	}

	template <typename Handler>
	void interval_us2(long long intervalus, Handler&& handler, bool immed = false)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		_isInterval = true;
		long long const deadtime = get_tick_us() + intervalus;
		_handler = wrap_advance_interval_timer_handler(_reuMem, std::bind([this, intervalus](bool isAdvance, wrap_base* const thisHandler, long long& deadtime)
		{
			bool sign = false;
			AsyncTimer_* const this_ = this;
			wrap_base* const intervalHandler = thisHandler->interval_handler();
			thisHandler->set_sign(&sign);
			intervalHandler->invoke(isAdvance);
			if (!sign)
			{
				thisHandler->set_sign(NULL);
				deadtime += intervalus;
				_timerHandle = _actorTimer->timeout(deadtime, _weakThis.lock(), true);
			}
			else
			{
				intervalHandler->destroy(this_->_reuMem);
			}
		}, __1, __2, deadtime), wrap_advance_timer_handler(_reuMem, std::forward<Handler>(handler)));
		_timerHandle = _actorTimer->timeout(deadtime, _weakThis.lock(), true);
		if (immed)
		{
			_handler->invoke();
		}
	}

	/*!
	@brief ȡ�����μ�ʱ����������strand�߳��е���
	*/
	void cancel();

	/*!
	@brief ��ǰ���μ�ʱ��������������strand�߳��е���
	*/
	bool advance();

	/*!
	@brief 
	*/
	shared_strand self_strand();

	/*!
	@brief ����һ������ͬһ��shared_strand�Ķ�ʱ��
	*/
	async_timer clone();
private:
	void timeout_handler();

	template <typename Handler>
	static wrap_base* wrap_timer_handler(reusable_mem& reuMem, Handler&& handler)
	{
		typedef wrap_handler<RM_CREF(Handler)> wrap_type;
		return new(reuMem.allocate(sizeof(wrap_type)))wrap_type(std::forward<Handler>(handler));
	}

	template <typename Handler>
	static wrap_base* wrap_advance_timer_handler(reusable_mem& reuMem, Handler&& handler)
	{
		typedef wrap_advance_handler<RM_CREF(Handler)> wrap_type;
		return new(reuMem.allocate(sizeof(wrap_type)))wrap_type(std::forward<Handler>(handler));
	}

	template <typename Handler>
	static wrap_base* wrap_interval_timer_handler(reusable_mem& reuMem, Handler&& handler, wrap_base* intervalHandler)
	{
		typedef wrap_interval_handler<RM_CREF(Handler)> wrap_type;
		return new(reuMem.allocate(sizeof(wrap_type)))wrap_type(std::forward<Handler>(handler), intervalHandler);
	}

	template <typename Handler>
	static wrap_base* wrap_advance_interval_timer_handler(reusable_mem& reuMem, Handler&& handler, wrap_base* intervalHandler)
	{
		typedef wrap_advance_interval_handler<RM_CREF(Handler)> wrap_type;
		return new(reuMem.allocate(sizeof(wrap_type)))wrap_type(std::forward<Handler>(handler), intervalHandler);
	}
private:
	wrap_base* _handler;
	reusable_mem _reuMem;
	ActorTimer_* _actorTimer;
	std::weak_ptr<AsyncTimer_> _weakThis;
	ActorTimer_::timer_handle _timerHandle;
	bool _isInterval;
	NONE_COPY(AsyncTimer_);
};

/*!
@brief ���ص�ʹ�õĶ�ʱ��
*/
class overlap_timer
#ifdef DISABLE_BOOST_TIMER
	: public TimerBoostCompletedEventFace_
#endif
{
public:
	class timer_handle;
private:
	typedef msg_multimap<long long, timer_handle*> handler_queue;

	friend boost_strand;
	friend qt_strand;
	friend uv_strand;
public:
	class timer_handle
	{
		friend overlap_timer;
	public:
		timer_handle()
			:_timestamp(0), _handler(NULL), _isInterval(false) {}

		~timer_handle()
		{
			assert(0 == _handler);
		}

		long long timestamp()
		{
			return _timestamp;
		}
	private:
		bool is_null() const
		{
			return !_handler;
		}

		void reset()
		{
			_timestamp = 0;
			_handler = NULL;
		}
	private:
		long long _timestamp;
		handler_queue::iterator _queueNode;
		AsyncTimer_::wrap_base* _handler;
		bool _isInterval;
		NONE_COPY(timer_handle);
	};
private:
	overlap_timer(const shared_strand& strand);
	~overlap_timer();
public:
	/*!
	@brief ����һ����ʱ����������strand�߳��е���
	*/
	template <typename Handler>
	void timeout(int ms, timer_handle& timerHandle, Handler&& handler)
	{
		timeout_us((long long)ms * 1000, timerHandle, std::forward<Handler>(handler));
	}

	template <typename Handler>
	void timeout2(int ms, timer_handle& timerHandle, Handler&& handler)
	{
		timeout_us2((long long)ms * 1000, timerHandle, std::forward<Handler>(handler));
	}

	template <typename Handler>
	void timeout_us(long long us, timer_handle& timerHandle, Handler&& handler)
	{
		assert(_weakStrand.lock()->running_in_this_thread());
		assert(timerHandle.is_null());
		timerHandle._isInterval = false;
		timerHandle._handler = AsyncTimer_::wrap_timer_handler(_reuMem, std::bind([&timerHandle](Handler& handler)
		{
			timerHandle.reset();
			CHECK_EXCEPTION(handler);
		}, std::forward<Handler>(handler)));
		_timeout(us, timerHandle);
	}

	template <typename Handler>
	void timeout_us2(long long us, timer_handle& timerHandle, Handler&& handler)
	{
		assert(_weakStrand.lock()->running_in_this_thread());
		assert(timerHandle.is_null());
		timerHandle._isInterval = false;
		timerHandle._handler = AsyncTimer_::wrap_advance_timer_handler(_reuMem, std::bind([&timerHandle](bool isAdvance, Handler& handler)
		{
			timerHandle.reset();
			CHECK_EXCEPTION(handler, isAdvance);
		}, __1, std::forward<Handler>(handler)));
		_timeout(us, timerHandle);
	}

	/*!
	@brief ����һ�����Զ�ʱ����������strand�߳��е���
	*/
	template <typename Handler>
	void deadline(long long us, timer_handle& timerHandle, Handler&& handler)
	{
		assert(_weakStrand.lock()->running_in_this_thread());
		assert(timerHandle.is_null());
		timerHandle._isInterval = false;
		timerHandle._handler = AsyncTimer_::wrap_timer_handler(_reuMem, std::bind([&timerHandle](Handler& handler)
		{
			timerHandle.reset();
			CHECK_EXCEPTION(handler);
		}, std::forward<Handler>(handler)));
		_timeout(us, timerHandle, true);
	}

	template <typename Handler>
	void deadline2(long long us, timer_handle& timerHandle, Handler&& handler)
	{
		assert(_weakStrand.lock()->running_in_this_thread());
		assert(timerHandle.is_null());
		timerHandle._isInterval = false;
		timerHandle._handler = AsyncTimer_::wrap_advance_timer_handler(_reuMem, std::bind([&timerHandle](bool isAdvance, Handler& handler)
		{
			timerHandle.reset();
			CHECK_EXCEPTION(handler, isAdvance);
		}, __1, std::forward<Handler>(handler)));
		_timeout(us, timerHandle, true);
	}

	/*!
	@brief ѭ����ʱ����һ��handler����������strand�߳��е���
	*/
	template <typename Handler>
	void interval(int ms, timer_handle& timerHandle, Handler&& handler, bool immed = false)
	{
		interval_us((long long)ms * 1000, timerHandle, std::forward<Handler>(handler), immed);
	}

	template <typename Handler>
	void interval2(int ms, timer_handle& timerHandle, Handler&& handler, bool immed = false)
	{
		interval_us2((long long)ms * 1000, timerHandle, std::forward<Handler>(handler), immed);
	}

	template <typename Handler>
	void interval_us(long long intervalus, timer_handle& timerHandle, Handler&& handler, bool immed = false)
	{
		assert(self_strand()->running_in_this_thread());
		assert(timerHandle.is_null());
		timerHandle._isInterval = true;
		long long const deadtime = get_tick_us() + intervalus;
		timerHandle._handler = AsyncTimer_::wrap_interval_timer_handler(_reuMem, std::bind([this, &timerHandle, intervalus](AsyncTimer_::wrap_base* const thisHandler, long long& deadtime)
		{
			bool sign = false;
			overlap_timer* const this_ = this;
			AsyncTimer_::wrap_base* const intervalHandler = thisHandler->interval_handler();
			thisHandler->set_sign(&sign);
			intervalHandler->invoke();
			if (!sign)
			{
				thisHandler->set_sign(NULL);
				deadtime += intervalus;
				_timeout(deadtime, timerHandle, true);
			}
			else
			{
				intervalHandler->destroy(this_->_reuMem);
			}
		}, __1, deadtime), AsyncTimer_::wrap_timer_handler(_reuMem, std::forward<Handler>(handler)));
		_timeout(deadtime, timerHandle, true);
		if (immed)
		{
			timerHandle._handler->invoke();
		}
	}

	template <typename Handler>
	void interval_us2(long long intervalus, timer_handle& timerHandle, Handler&& handler, bool immed = false)
	{
		assert(self_strand()->running_in_this_thread());
		assert(timerHandle.is_null());
		timerHandle._isInterval = true;
		long long const deadtime = get_tick_us() + intervalus;
		timerHandle._handler = AsyncTimer_::wrap_advance_interval_timer_handler(_reuMem, std::bind([this, &timerHandle, intervalus](bool isAdvance, AsyncTimer_::wrap_base* const thisHandler, long long& deadtime)
		{
			bool sign = false;
			overlap_timer* const this_ = this;
			AsyncTimer_::wrap_base* const intervalHandler = thisHandler->interval_handler();
			thisHandler->set_sign(&sign);
			intervalHandler->invoke(isAdvance);
			if (!sign)
			{
				thisHandler->set_sign(NULL);
				deadtime += intervalus;
				_timeout(deadtime, timerHandle, true);
			}
			else
			{
				intervalHandler->destroy(this_->_reuMem);
			}
		}, __1, __2, deadtime), AsyncTimer_::wrap_advance_timer_handler(_reuMem, std::forward<Handler>(handler)));
		_timeout(deadtime, timerHandle, true);
		if (immed)
		{
			timerHandle._handler->invoke();
		}
	}

	/*!
	@brief ȡ��һ�μ�ʱ����������strand�߳��е���
	*/
	void cancel(timer_handle& timerHandle);

	/*!
	@brief ��ǰ���μ�ʱ��������������strand�߳��е���
	*/
	bool advance(timer_handle& timerHandle);

	/*!
	@brief
	*/
	shared_strand self_strand();
private:
	void timer_loop(long long abs, long long rel);
	void event_handler(int tc);
#ifdef DISABLE_BOOST_TIMER
	void post_event(int tc);
#endif
private:
	void _cancel(timer_handle& timerHandle);
	void _timeout(long long us, timer_handle& timerHandle, bool deadline = false);
private:
	void* _timer;
	std::weak_ptr<boost_strand>& _weakStrand;
	shared_strand _lockStrand;
	handler_queue _handlerQueue;
	reusable_mem _reuMem;
	long long _extMaxTick;
	long long _extFinishTime;
#ifdef DISABLE_BOOST_TIMER
	stack_obj<boost::asio::io_service::work, false> _lockIos;
#endif
	int _timerCount;
	bool _looping;
	NONE_COPY(overlap_timer);
};

#endif
#include "async_timer.h"
#include "scattered.h"
#include "io_engine.h"
#include "actor_timer.h"
#ifndef DISABLE_BOOST_TIMER
#include <boost/chrono/system_clocks.hpp>
#include <boost/asio/high_resolution_timer.hpp>
typedef boost::asio::basic_waitable_timer<boost::chrono::high_resolution_clock> timer_type;
typedef boost::chrono::microseconds micseconds;
#else
#include "waitable_timer.h"
typedef WaitableTimerEvent_ timer_type;
typedef long long micseconds;
#endif

AsyncTimer_::AsyncTimer_(ActorTimer_* actorTimer)
:_actorTimer(actorTimer), _handler(NULL), _currTimeout(0), _isInterval(false) {}

AsyncTimer_::~AsyncTimer_()
{
	assert(!_handler);
}

void AsyncTimer_::cancel()
{
	assert(self_strand()->running_in_this_thread());
	if (_handler)
	{
		_actorTimer->cancel(_timerHandle);
		_handler->destroy(_reuMem);
		_handler = NULL;
		_currTimeout = 0;
	}
}

bool AsyncTimer_::advance()
{
	assert(self_strand()->running_in_this_thread());
	if (_handler)
	{
		if (!_isInterval)
		{
			_actorTimer->cancel(_timerHandle);
			wrap_base* const cb = _handler;
			_handler = NULL;
			_currTimeout = 0;
			cb->invoke(true);
			cb->destroy(_reuMem);
			return true;
		}
		else if (!_handler->is_top_call())
		{
			_handler->invoke(true);
			return true;
		}
	}
	return false;
}

long long AsyncTimer_::restart()
{
	assert(self_strand()->running_in_this_thread());
	if (_handler && _currTimeout)
	{
		if (!_isInterval)
		{
			_actorTimer->cancel(_timerHandle);
			_timerHandle = _actorTimer->timeout(_currTimeout, _weakThis.lock());
		}
		else if (!_handler->is_top_call())
		{
			_actorTimer->cancel(_timerHandle);
			_timerHandle = _actorTimer->timeout(_currTimeout, _weakThis.lock());
			_handler->set_deadtime(_timerHandle._beginStamp + _currTimeout);
		}
		else
		{
			_timerHandle._beginStamp = get_tick_us();
			_handler->set_deadtime(_timerHandle._beginStamp + _currTimeout);
		}
		return _timerHandle._beginStamp;
	}
	return 0;
}

bool AsyncTimer_::completed()
{
	assert(self_strand()->running_in_this_thread());
	return !_handler;
}

shared_strand AsyncTimer_::self_strand()
{
	return _actorTimer->_weakStrand.lock();
}

async_timer AsyncTimer_::clone()
{
	return self_strand()->make_timer();
}

void AsyncTimer_::timeout_handler()
{
	_timerHandle.reset();
	if (!_isInterval)
	{
		wrap_base* const cb = _handler;
		_handler = NULL;
		_currTimeout = 0;
		cb->invoke();
		cb->destroy(_reuMem);
	}
	else
	{
		_handler->invoke();
	}
}

long long AsyncTimer_::_utimeout(long long us, wrap_base* handler)
{
	assert(self_strand()->running_in_this_thread());
	assert(!_handler);
	_isInterval = false;
	_currTimeout = us;
	_handler = handler;
	_timerHandle = _actorTimer->timeout(us, _weakThis.lock());
	return _timerHandle._beginStamp;
}

long long AsyncTimer_::_deadline(long long us, wrap_base* handler)
{
	assert(self_strand()->running_in_this_thread());
	assert(!_handler);
	_isInterval = false;
	_handler = handler;
	_timerHandle = _actorTimer->timeout(us, _weakThis.lock(), true);
	return _timerHandle._beginStamp;
}

void AsyncTimer_::_uinterval(long long intervalus, wrap_base* handler, bool immed)
{
	assert(self_strand()->running_in_this_thread());
	assert(!_handler);
	_isInterval = true;
	_currTimeout = intervalus;
	_handler = wrap_interval_timer_handler(_reuMem, [this, intervalus](bool isAdvance, wrap_base* const thisHandler)
	{
		bool sign = false;
		AsyncTimer_* const this_ = this;
		wrap_base* const intervalHandler = thisHandler->interval_handler();
		thisHandler->set_sign(&sign);
		intervalHandler->invoke(isAdvance);
		if (!sign)
		{
			thisHandler->set_sign(NULL);
			if (!isAdvance)
			{
				long long& deadtime = thisHandler->deadtime_ref();
				deadtime += intervalus;
				_timerHandle = _actorTimer->timeout(deadtime, _weakThis.lock(), true);
			}
		}
		else
		{
			intervalHandler->destroy(this_->_reuMem);
		}
	}, handler);
	if (immed)
	{
		_handler->set_deadtime(get_tick_us());
		_handler->invoke();
	}
	else
	{
		_timerHandle = _actorTimer->timeout(intervalus, _weakThis.lock());
		_handler->set_deadtime(_timerHandle._beginStamp + intervalus);
	}
}
//////////////////////////////////////////////////////////////////////////

overlap_timer::overlap_timer(const shared_strand& strand)
:_weakStrand(strand->_weakThis), _looping(false), _timerCount(0),
_extMaxTick(0), _extFinishTime(-1), _handlerQueue(MEM_POOL_LENGTH)
{
#ifdef DISABLE_BOOST_TIMER
	_timer = new timer_type(strand->get_io_engine(), this);
#else
	_timer = new timer_type(strand->get_io_engine());
#endif
}

overlap_timer::~overlap_timer()
{
	assert(_handlerQueue.empty());
	delete (timer_type*)_timer;
}

void overlap_timer::_timeout(long long us, timer_handle& timerHandle, bool deadline)
{
	if (!_lockStrand)
	{
		_lockStrand = _weakStrand.lock();
#ifdef DISABLE_BOOST_TIMER
		_lockIos.create(_lockStrand->get_io_engine());
#endif
	}
	assert(_lockStrand->running_in_this_thread());
	timerHandle._timestamp = get_tick_us();
	long long et = deadline ? us : (timerHandle._timestamp + us);
	if (et >= _extMaxTick)
	{
		_extMaxTick = et;
		timerHandle._queueNode = _handlerQueue.insert(_handlerQueue.end(), std::make_pair(et, &timerHandle));
	}
	else
	{
		timerHandle._queueNode = _handlerQueue.insert(std::make_pair(et, &timerHandle));
	}

	if (!_looping)
	{//定时器已经退出循环，重新启动定时器
		_looping = true;
		assert(_handlerQueue.size() == 1);
		_extFinishTime = et;
		timer_loop(et, et - timerHandle._timestamp);
	}
	else if ((unsigned long long)et < (unsigned long long)_extFinishTime)
	{//定时期限前于当前定时器期限，取消后重新计时
		_timerCount++;
		_extFinishTime = et;
		boost::system::error_code ec;
		as_ptype<timer_type>(_timer)->cancel(ec);
		timer_loop(et, et - timerHandle._timestamp);
	}
}

void overlap_timer::_cancel(timer_handle& timerHandle)
{
	if (timerHandle._timestamp)
	{
		assert(_lockStrand);
		handler_queue::iterator itNode = timerHandle._queueNode;
		if (_handlerQueue.size() == 1)
		{
			_timerCount++;
			_extMaxTick = 0;
			_looping = false;
			_handlerQueue.erase(itNode);
			//如果没有定时任务就退出定时循环
			boost::system::error_code ec;
			as_ptype<timer_type>(_timer)->cancel(ec);
		}
		else if (itNode->first == _extMaxTick)
		{
			_handlerQueue.erase(itNode++);
			if (_handlerQueue.end() == itNode)
			{
				_extMaxTick = (--itNode)->first;
			}
			else
			{
				assert(itNode->first == _extMaxTick);
			}
		}
		else
		{
			_handlerQueue.erase(itNode);
		}
	}
}

void overlap_timer::cancel(timer_handle& timerHandle)
{
	assert(self_strand()->running_in_this_thread());
	if (!timerHandle.completed())
	{//删除当前定时器节点
		_cancel(timerHandle);
		timerHandle._handler->destroy(_reuMem);
		timerHandle.reset();
	}
}

bool overlap_timer::advance(timer_handle& timerHandle)
{
	assert(self_strand()->running_in_this_thread());
	if (!timerHandle.completed())
	{
		if (!timerHandle._isInterval)
		{
			_cancel(timerHandle);
			AsyncTimer_::wrap_base* const cb = timerHandle._handler;
			timerHandle.reset();
			cb->invoke(true);
			cb->destroy(_reuMem);
			return true;
		}
		else if (!timerHandle._handler->is_top_call())
		{
			timerHandle._handler->invoke(true);
			return true;
		}
	}
	return false;
}

long long overlap_timer::restart(timer_handle& timerHandle)
{
	assert(self_strand()->running_in_this_thread());
	if (!timerHandle.completed() && timerHandle._currTimeout)
	{
		if (!timerHandle._isInterval)
		{
			_cancel(timerHandle);
			_timeout(timerHandle._currTimeout, timerHandle, false);
		}
		else if (!timerHandle._handler->is_top_call())
		{
			_cancel(timerHandle);
			_timeout(timerHandle._currTimeout, timerHandle, false);
			timerHandle._handler->set_deadtime(timerHandle._timestamp + timerHandle._currTimeout);
		}
		else
		{
			timerHandle._timestamp = get_tick_us();
			timerHandle._handler->set_deadtime(timerHandle._timestamp + timerHandle._currTimeout);
		}
		return timerHandle._timestamp;
	}
	return 0;
}

shared_strand overlap_timer::self_strand()
{
	return _weakStrand.lock();
}

void overlap_timer::timer_loop(long long abs, long long rel)
{
	int tc = ++_timerCount;
#ifdef DISABLE_BOOST_TIMER
	as_ptype<timer_type>(_timer)->async_wait(micseconds(abs), micseconds(rel), tc);
#else
	boost::system::error_code ec;
	as_ptype<timer_type>(_timer)->expires_at(timer_type::time_point(micseconds(abs)), ec);
	as_ptype<timer_type>(_timer)->async_wait(_lockStrand->wrap_asio_once([this, tc](const boost::system::error_code&)
	{
		event_handler(tc);
	}));
#endif
}

#ifdef DISABLE_BOOST_TIMER
void overlap_timer::post_event(int tc)
{
	assert(_lockStrand);
	_lockStrand->post([this, tc]
	{
		event_handler(tc);
		if (!_lockStrand)
		{
			_lockIos.destroy();
		}
	});
}

void overlap_timer::cancel_event()
{
	assert(_lockStrand && _lockStrand->running_in_this_thread());
	if (!_looping)
	{
		_lockStrand.reset();
		_lockIos.destroy();
	}
}
#endif

void overlap_timer::event_handler(int tc)
{
	assert(_lockStrand->running_in_this_thread());
	if (tc == _timerCount)
	{
		_extFinishTime = 0;
		while (!_handlerQueue.empty())
		{
			handler_queue::iterator iter = _handlerQueue.begin();
			long long ct = get_tick_us();
			if (iter->first > ct)
			{
				_extFinishTime = iter->first;
				timer_loop(_extFinishTime, _extFinishTime - ct);
				return;
			}
			else
			{
				timer_handle* const timerHandle = iter->second;
				AsyncTimer_::wrap_base* const cb = timerHandle->_handler;
				if (!timerHandle->_isInterval)
				{
					timerHandle->reset();
					cb->invoke();
					cb->destroy(_reuMem);
				}
				else
				{
					timerHandle->_timestamp = 0;
					cb->invoke();
				}
				_handlerQueue.erase(iter);
			}
		}
		_looping = false;
		_lockStrand.reset();
	}
	else if (tc == _timerCount - 1)
	{
		_lockStrand.reset();
	}
}

void overlap_timer::_utimeout(long long us, timer_handle& timerHandle, AsyncTimer_::wrap_base* handler)
{
	assert(_weakStrand.lock()->running_in_this_thread());
	assert(timerHandle.completed());
	timerHandle._isInterval = false;
	timerHandle._currTimeout = us;
	timerHandle._handler = handler;
	_timeout(us, timerHandle, false);
}

void overlap_timer::_deadline(long long us, timer_handle& timerHandle, AsyncTimer_::wrap_base* handler)
{
	assert(_weakStrand.lock()->running_in_this_thread());
	assert(timerHandle.completed());
	timerHandle._isInterval = false;
	timerHandle._handler = handler;
	_timeout(us, timerHandle, true);
}

void overlap_timer::_uinterval(long long intervalus, timer_handle& timerHandle, AsyncTimer_::wrap_base* handler, bool immed)
{
	assert(self_strand()->running_in_this_thread());
	assert(timerHandle.completed());
	timerHandle._isInterval = true;
	timerHandle._currTimeout = intervalus;
	timerHandle._handler = AsyncTimer_::wrap_interval_timer_handler(_reuMem, [this, &timerHandle, intervalus](bool isAdvance, AsyncTimer_::wrap_base* const thisHandler)
	{
		bool sign = false;
		overlap_timer* const this_ = this;
		AsyncTimer_::wrap_base* const intervalHandler = thisHandler->interval_handler();
		thisHandler->set_sign(&sign);
		intervalHandler->invoke(isAdvance);
		if (!sign)
		{
			thisHandler->set_sign(NULL);
			if (!isAdvance)
			{
				long long& deadtime = thisHandler->deadtime_ref();
				deadtime += intervalus;
				_timeout(deadtime, timerHandle, true);
			}
		}
		else
		{
			intervalHandler->destroy(this_->_reuMem);
		}
	}, handler);
	if (immed)
	{
		timerHandle._handler->set_deadtime(get_tick_us());
		timerHandle._handler->invoke();
	}
	else
	{
		_timeout(intervalus, timerHandle, false);
		timerHandle._handler->set_deadtime(timerHandle._timestamp + intervalus);
	}
}
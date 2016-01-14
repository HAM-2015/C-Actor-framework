#include "shared_strand.h"
#include "actor_timer.h"
#include "async_timer.h"

boost_strand::boost_strand()
#ifdef ENABLE_NEXT_TICK
:_pCheckDestroy(NULL),
_beginNextRound(false),
_thisRoundCount(64),
_nextTickAlloc(NULL),
_reuMemAlloc(NULL),
_frontTickQueue(NULL),
_backTickQueue(NULL)
#endif //ENABLE_NEXT_TICK
{
	_ioEngine = NULL;
	_strand = NULL;
	_actorTimer = NULL;
	_timerBoost = NULL;
}

boost_strand::~boost_strand()
{
#ifdef ENABLE_NEXT_TICK
	assert(_frontTickQueue->empty());
	assert(_backTickQueue->empty());
	delete _nextTickAlloc;
	delete _reuMemAlloc;
	delete _frontTickQueue;
	delete _backTickQueue;
	if (_pCheckDestroy)
	{
		*_pCheckDestroy = true;
	}
#endif //ENABLE_NEXT_TICK
	delete _actorTimer;
	delete _timerBoost;
	if (_strand)
	{
		_ioEngine->_strandPool->recycle(_strand);
	}
}

shared_strand boost_strand::create(io_engine& ioEngine, bool makeTimer)
{
	shared_strand res(new boost_strand);
	res->_weakThis = res;
	res->_ioEngine = &ioEngine;
	res->_strand = ioEngine._strandPool->pick();
#ifdef ENABLE_NEXT_TICK
	res->_reuMemAlloc = new reusable_mem();
	res->_nextTickAlloc = new mem_alloc2<wrap_next_tick_space>(8192);
	res->_frontTickQueue = new msg_queue<wrap_next_tick_base*, mem_alloc2<>>(8192);
	res->_backTickQueue = new msg_queue<wrap_next_tick_base*, mem_alloc2<>>(8192);
#endif
	if (makeTimer)
	{
		res->_actorTimer = new ActorTimer_(res);
		res->_timerBoost = new TimerBoost_(res);
	}
	assert(!res->running_in_this_thread());
	return res;
}

vector<shared_strand> boost_strand::create_multi(size_t n, io_engine& ioEngine, bool makeTimer)
{
	assert(0 != n);
	vector<shared_strand> res(n);
	for (size_t i = 0; i < n; i++)
	{
		res[i] = boost_strand::create(ioEngine, makeTimer);
	}
	return res;
}

void boost_strand::create_multi(shared_strand* res, size_t n, io_engine& ioEngine, bool makeTimer)
{
	assert(0 != n);
	for (size_t i = 0; i < n; i++)
	{
		res[i] = boost_strand::create(ioEngine, makeTimer);
	}
}

void boost_strand::create_multi(vector<shared_strand>& res, size_t n, io_engine& ioEngine, bool makeTimer)
{
	assert(0 != n);
	res.resize(n);
	for (size_t i = 0; i < n; i++)
	{
		res[i] = boost_strand::create(ioEngine, makeTimer);
	}
}

void boost_strand::create_multi(list<shared_strand>& res, size_t n, io_engine& ioEngine, bool makeTimer)
{
	assert(0 != n);
	res.clear();
	for (size_t i = 0; i < n; i++)
	{
		res.push_front(boost_strand::create(ioEngine, makeTimer));
	}
}

shared_strand boost_strand::clone()
{
	return create(*_ioEngine, !!_actorTimer);
}

bool boost_strand::in_this_ios()
{
	assert(_ioEngine);
	return _ioEngine->runningInThisIos();
}

bool boost_strand::running_in_this_thread()
{
	assert(_strand);
	return _strand->running_in_this_thread();
}

bool boost_strand::empty(bool checkTick)
{
	assert(_strand);
	assert(running_in_this_thread());
#ifdef ENABLE_NEXT_TICK
	return _strand->empty() && (checkTick ? _frontTickQueue->empty() && _backTickQueue->empty() : true);
#else
	return _strand->empty();
#endif
}

size_t boost_strand::ios_thread_number()
{
	assert(_ioEngine);
	return _ioEngine->threadNumber();
}

io_engine& boost_strand::get_io_engine()
{
	assert(_ioEngine);
	return *_ioEngine;
}

boost::asio::io_service& boost_strand::get_io_service()
{
	assert(_ioEngine);
	return *_ioEngine;
}

ActorTimer_* boost_strand::actor_timer()
{
	return _actorTimer;
}

std::shared_ptr<AsyncTimer_> boost_strand::make_timer()
{
	std::shared_ptr<AsyncTimer_> res(new AsyncTimer_(*_timerBoost));
	res->_weakThis = res;
	return res;
}

#ifdef ENABLE_NEXT_TICK
bool boost_strand::ready_empty()
{
	return _strand->ready_empty();
}

bool boost_strand::waiting_empty()
{
	return _strand->waiting_empty();
}

void boost_strand::run_tick_front()
{
	assert(_beginNextRound);
	_thisRoundCount = 64;
	while (!_frontTickQueue->empty())
	{
		wrap_next_tick_base* tick = _frontTickQueue->front();
		_frontTickQueue->pop_front();
		auto res = tick->invoke();
		if (-1 == res._size)
		{
			_nextTickAlloc->deallocate(res._ptr);
		}
		else
		{
			_reuMemAlloc->deallocate(res._ptr);
		}
	}
}

void boost_strand::run_tick_back()
{
	assert(ready_empty());
	if (!_backTickQueue->empty())
	{
		shared_strand lockThis = _weakThis.lock();
		size_t tickCount = 0;
		do
		{
			wrap_next_tick_base* tick = _backTickQueue->front();
			_backTickQueue->pop_front();
			auto res = tick->invoke();
			if (-1 == res._size)
			{
				_nextTickAlloc->deallocate(res._ptr);
			}
			else
			{
				_reuMemAlloc->deallocate(res._ptr);
			}
		} while (!_backTickQueue->empty() && ++tickCount <= _thisRoundCount);
		std::swap(_frontTickQueue, _backTickQueue);
		if (!_frontTickQueue->empty() && waiting_empty())
		{
			post([lockThis]{});
		}
	}
}
#endif //ENABLE_NEXT_TICK

#ifdef ENABLE_QT_ACTOR
void boost_strand::_post(const std::function<void()>& h)
{
	assert(false);
}

void boost_strand::_post(std::function<void()>&& h)
{
	assert(false);
}
#endif
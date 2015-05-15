#define WIN32_LEAN_AND_MEAN

#include <boost/coroutine/all.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include "actor_framework.h"
#include "actor_stack.h"
#include "wrapped_no_params_handler.h"

typedef boost::coroutines::coroutine<void>::pull_type actor_pull_type;
typedef boost::coroutines::coroutine<void>::push_type actor_push_type;
typedef boost::asio::basic_waitable_timer<boost::chrono::high_resolution_clock> timer_type;

#ifdef _DEBUG

#define CHECK_EXCEPTION(__h) try { (__h)(); } catch (...) { assert(false); }
#define CHECK_EXCEPTION1(__h, __p0) try { (__h)(__p0); } catch (...) { assert(false); }
#define CHECK_EXCEPTION2(__h, __p0, __p1) try { (__h)(__p0, __p1); } catch (...) { assert(false); }
#define CHECK_EXCEPTION3(__h, __p0, __p1, __p2) try { (__h)(__p0, __p1, __p2); } catch (...) { assert(false); }

#else

#define CHECK_EXCEPTION(__h) (__h)()
#define CHECK_EXCEPTION1(__h, __p0) (__h)(__p0)
#define CHECK_EXCEPTION2(__h, __p0, __p1) (__h)(__p0, __p1)
#define CHECK_EXCEPTION3(__h, __p0, __p1, __p2) (__h)(__p0, __p1, __p2)

#endif //end _DEBUG

//��ջ��Ԥ���ռ䣬��ֹ��ջ
#define STACK_RESERVED_SPACE_SIZE		64
//�ڴ�߽����
#define MEM_ALIGN(__o, __a) (((__o) + ((__a)-1)) & (((__a)-1) ^ -1))

/*!
@brief Actorջ������
*/
struct actor_stack_pool_allocate
{
	actor_stack_pool_allocate() {}

	actor_stack_pool_allocate(void* sp, size_t size)
	{
		_stack.sp = sp;
		_stack.size = size;
	}

	void allocate( boost::coroutines::stack_context & stackCon, size_t size)
	{
		stackCon = _stack;
	}

	void deallocate( boost::coroutines::stack_context & stackCon)
	{

	}

	boost::coroutines::stack_context _stack;
};

struct actor_stack_allocate 
{
	actor_stack_allocate() {}

	actor_stack_allocate(void** sp, size_t* size)
	{
		_sp = sp;
		_size = size;
	}

	void allocate( boost::coroutines::stack_context & stackCon, size_t size)
	{
		boost::coroutines::stack_allocator all;
		all.allocate(stackCon, size);
		*_sp = stackCon.sp;
		*_size = stackCon.size;
	}

	void deallocate( boost::coroutines::stack_context & stackCon)
	{
		boost::coroutines::stack_allocator all;
		all.deallocate(stackCon);
	}

	void** _sp;
	size_t* _size;
};
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
child_actor_handle::child_actor_param& child_actor_handle::child_actor_param::operator=( child_actor_handle::child_actor_param& s )
{
	_actor = s._actor;
	_actorIt = s._actorIt;
	s._isCopy = true;
	_isCopy = false;
	return *this;
}

child_actor_handle::child_actor_param::~child_actor_param()
{
	assert(_isCopy);//��ⴴ������û�н�����Actor���
}

child_actor_handle::child_actor_param::child_actor_param( child_actor_handle::child_actor_param& s )
{
	*this = s;
}

child_actor_handle::child_actor_param::child_actor_param()
{
	_isCopy = true;
}
#endif

child_actor_handle::child_actor_handle( child_actor_handle& )
{
	assert(false);
}

child_actor_handle::child_actor_handle()
{
	_quited = true;
	_norQuit = false;
}

child_actor_handle::child_actor_handle( child_actor_param& s )
{
	_quited = true;
	_norQuit = false;
	*this = s;
}

child_actor_handle& child_actor_handle::operator=( child_actor_handle& )
{
	assert(false);
	return *this;
}

child_actor_handle& child_actor_handle::operator=( child_actor_param& s )
{
	assert(_quited);
	_quited = false;
	_norQuit = false;
	_param = s;
	DEBUG_OPERATION(_param._isCopy = true);
	DEBUG_OPERATION(_qh = s._actor->parent_actor()->regist_quit_handler([this]{_quited = true;}));//�ڸ�Actor�˳�ʱ��_quited=true
	return *this;
}

child_actor_handle::~child_actor_handle()
{
	assert(_quited);
}

actor_handle child_actor_handle::get_actor()
{
	return _param._actor;
}

actor_handle child_actor_handle::peel()
{
	actor_handle r = _param._actor;
	if (_param._actor)
	{
		assert(_param._actor->parent_actor()->_strand->running_in_this_thread());
		assert(!_param._actor->parent_actor()->_quited);
		_quited = true;
		DEBUG_OPERATION(_param._actor->parent_actor()->_quitHandlerList.erase(_qh));
		_param._actor->parent_actor()->_childActorList.erase(_param._actorIt);
		_param._actor->_parentActor.reset();
		_param._actor.reset();
	}
	return r;
}

child_actor_handle::ptr child_actor_handle::make_ptr()
{
	return ptr(new child_actor_handle);
}

bool child_actor_handle::empty()
{
	return !_param._actor;
}

void* child_actor_handle::operator new(size_t s)
{
	void* p = malloc(s);
	assert(p);
	return p;
}

void child_actor_handle::operator delete(void* p)
{
	if (p)
	{
		free(p);
	}
}

//////////////////////////////////////////////////////////////////////////

void actor_msg_handle_base::run_one()
{
	assert(!_hostActor->is_quited());
	_hostActor->pull_yield();
}

actor_msg_handle_base::actor_msg_handle_base()
:_waiting(false)
{

}

void actor_msg_handle_base::set_actor(const actor_handle& hostActor)
{
	_hostActor = hostActor.get();
	DEBUG_OPERATION(_strand = hostActor->self_strand());
}

bool actor_msg_handle_base::is_quited()
{
	return _hostActor->is_quited();
}

std::shared_ptr<bool> actor_msg_handle_base::new_bool()
{
	if (my_actor::_sharedBoolPool)
	{
		std::shared_ptr<bool> r = my_actor::_sharedBoolPool->new_();
		*r = false;
		return r;
	} 
	return std::shared_ptr<bool>(new bool(false));
}

//////////////////////////////////////////////////////////////////////////

void msg_pump_base::run_one()
{
	assert(!_hostActor->is_quited());
	_hostActor->pull_yield();
}

bool msg_pump_base::is_quited()
{
	return !_hostActor || _hostActor->is_quited();
}

//////////////////////////////////////////////////////////////////////////

msg_pool_void::msg_pool_void(shared_strand strand)
{
	_strand = strand;
	_waiting = false;
	_sendCount = 0;
	_msgBuff = 0;
}

msg_pool_void::~msg_pool_void()
{

}

msg_pool_void::pump_handler msg_pool_void::connect_pump(const std::shared_ptr<msg_pump_type>& msgPump)
{
	assert(msgPump);
	assert(_strand->running_in_this_thread());
	_msgPump = msgPump;
	pump_handler compHandler;
	compHandler._thisPool = _weakThis.lock();
	compHandler._msgPump = msgPump;
	_waiting = false;
	_sendCount = 0;
	return compHandler;
}

void msg_pool_void::push_msg(const actor_handle& hostActor)
{
	if (_strand->running_in_this_thread())
	{
		post_msg(hostActor);
	}
	else
	{
		auto shared_this = _weakThis.lock();
		_strand->post([=]
		{
			shared_this->send_msg(hostActor);
		});
	}
}

void msg_pool_void::send_msg(const actor_handle& hostActor)
{
	if (_waiting)
	{
		_waiting = false;
		assert(_msgPump);
		_sendCount++;
		_msgPump->receive_msg(hostActor);
	}
	else
	{
		_msgBuff++;
	}
}

void msg_pool_void::post_msg(const actor_handle& hostActor)
{
	if (_waiting)
	{
		_waiting = false;
		assert(_msgPump);
		_sendCount++;
		_msgPump->receive_msg_post(hostActor);
	}
	else
	{
		_msgBuff++;
	}
}

void msg_pool_void::disconnect()
{
	assert(_strand->running_in_this_thread());
	_msgPump.reset();
	_waiting = false;
}

void msg_pool_void::pump_handler::clear()
{
	_thisPool.reset();
	_msgPump.reset();
}

bool msg_pool_void::pump_handler::empty()
{
	return !_thisPool;
}

void msg_pool_void::pump_handler::post_pump(BYTE pumpID)
{
	assert(!empty());
	auto& refThis_ = *this;
	auto hostActor = _msgPump->_hostActor->shared_from_this();
	_thisPool->_strand->post([=]
	{
		actor_handle lockActor = hostActor;
		((pump_handler&)refThis_)(pumpID);
	});
}

void msg_pool_void::pump_handler::operator()(BYTE pumpID)
{
	assert(_thisPool);
	if (_thisPool->_strand->running_in_this_thread())
	{
		if (_msgPump == _thisPool->_msgPump)
		{
			assert(!_thisPool->_waiting);
			if (pumpID == _thisPool->_sendCount)
			{
				if (_thisPool->_msgBuff)
				{
					_thisPool->_msgBuff--;
					_thisPool->_sendCount++;
					_thisPool->_msgPump->receive_msg(_msgPump->_hostActor->shared_from_this());
				} 
				else
				{
					_thisPool->_waiting = true;
				}
			}
			else
			{//�ϴ���Ϣûȡ��������ȡ����ʵ���м��Ѿ�post��ȥ��
				assert(!_thisPool->_waiting);
				assert(pumpID + 1 == _thisPool->_sendCount);
			}
		}
	}
	else
	{
		post_pump(pumpID);
	}
}
//////////////////////////////////////////////////////////////////////////

msg_pump_void::msg_pump_void(const actor_handle& hostActor)
{
	_waiting = false;
	_hasMsg = false;
	_checkDis = false;
	_pumpCount = 0;
	_hostActor = hostActor.get();
	_strand = hostActor->self_strand();
}

msg_pump_void::~msg_pump_void()
{

}

void msg_pump_void::clear()
{
	assert(_strand->running_in_this_thread());
	_pumpHandler.clear();
	if (!is_quited() && _checkDis)
	{
		assert(_waiting);
		_waiting = false;
		run_one();
	}
}

void msg_pump_void::close()
{
	_hasMsg = false;
	_waiting = false;
	_checkDis = false;
	_pumpCount = 0;
	_pumpHandler.clear();
	_hostActor = NULL;
}

void msg_pump_void::connect(const pump_handler& pumpHandler)
{
	assert(_strand->running_in_this_thread());
	assert(_hostActor);
	_pumpHandler = pumpHandler;
	_pumpCount = 0;
	if (_waiting)
	{
		_pumpHandler.post_pump(_pumpCount);
	}
}

void msg_pump_void::receiver()
{
	if (!is_quited())
	{
		_pumpCount++;
		assert(!_hasMsg);
		if (_waiting)
		{
			_waiting = false;
			_checkDis = false;
			run_one();
		}
		else
		{
			_hasMsg = true;
		}
	}
}

bool msg_pump_void::read_msg()
{
	assert(_strand->running_in_this_thread());
	assert(!_waiting);
	if (_hasMsg)
	{
		_hasMsg = false;
		return true;
	}
	if (!_pumpHandler.empty())
	{
		_pumpHandler(_pumpCount);
		_waiting = !_hasMsg;
		_hasMsg = false;
		return !_waiting;
	}
	_waiting = true;
	return false;
}

void msg_pump_void::receive_msg(const actor_handle& hostActor)
{
	if (_strand->running_in_this_thread())
	{
		receiver();
	}
	else
	{
		receive_msg_post(hostActor);
	}
}

void msg_pump_void::receive_msg_post(const actor_handle& hostActor)
{
	auto shared_this = _weakThis.lock();
	_strand->post([=]
	{
		actor_handle lockActor = hostActor;
		shared_this->receiver();
	});
}

bool msg_pump_void::isDisconnected()
{
	return _pumpHandler.empty();
}
//////////////////////////////////////////////////////////////////////////

void trig_once_base::trig_handler() const
{
#ifdef _DEBUG
	if (!_pIsTrig->exchange(true))
	{
		assert(_hostActor);
		_hostActor->trig_handler();
	}
	else
	{
		assert(false);
	}
#else
	assert(_hostActor);
	_hostActor->trig_handler();
#endif
}
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct actor_ref_count_alloc
{
	template<class Other>
	struct rebind
	{
		typedef actor_ref_count_alloc<Other> other;
	};

	actor_ref_count_alloc(stack_pck& stackMem, void** ptr)
		:_stackMem(stackMem), _ptr(ptr)
	{
	}

	actor_ref_count_alloc(const actor_ref_count_alloc<T>& s)
		:_stackMem(s._stackMem), _ptr(s._ptr)
	{
	}

	template<class Other>
	actor_ref_count_alloc(const actor_ref_count_alloc<Other>& s)
		: _stackMem(s._stackMem), _ptr(s._ptr)
	{
	}

	T* allocate(size_t count)
	{
		assert(1 == count);
		*_ptr = (BYTE*)*_ptr - MEM_ALIGN(sizeof(T), sizeof(void*));
		return (T*)*_ptr;
	}

	void deallocate(T* ptr, size_t count)
	{
		assert(1 == count);
		actor_stack_pool::recovery(_stackMem);
	}

	void destroy(T* ptr)
	{
		ptr->~T();
	}

	stack_pck _stackMem;
	void** _ptr;
};
//////////////////////////////////////////////////////////////////////////

struct my_actor::timer_pck
{
	timer_pck(ios_proxy& ios)
		:_ios(ios), _timer((timer_type*)ios.getTimer()), _timerTime(0), _timerSuspend(false), _timerCompleted(true) {}

	~timer_pck()
	{
		boost::system::error_code ec;
		_timer->cancel(ec);
		_ios.freeTimer(_timer);
	}

	ios_proxy& _ios;
	timer_type* _timer;
	bool _timerSuspend;
	bool _timerCompleted;
	boost::posix_time::microsec _timerTime;
	boost::posix_time::ptime _timerStampBegin;
	boost::posix_time::ptime _timerStampEnd;
	std::function<void ()> _h;
};

boost::atomic<long long> _actorIDCount(0);//ID����
bool _autoMakeTimer = true;
#ifdef CHECK_ACTOR
map<void*, my_actor*> _stackLine;
boost::mutex _stackLineMutex;
#endif

std::shared_ptr<shared_obj_pool_base<bool>> my_actor::_sharedBoolPool;

class my_actor::boost_actor_run
{
public:
	boost_actor_run(my_actor& actor)
		: _actor(actor)
	{

	}

	void operator ()( actor_push_type& actorPush )
	{
		assert(!_actor._quited);
		assert(_actor._mainFunc);
		_actor._actorPush = &actorPush;
		try
		{
			_actor._inActor = true;
			_actor.push_yield();
			assert(_actor._strand->running_in_this_thread());
			_actor._mainFunc(&_actor);
			_actor._quited = true;
			assert(_actor._inActor);
			assert(_actor._childActorList.empty());
			assert(_actor._quitHandlerList.empty());
			assert(_actor._suspendResumeQueue.empty());
		}
		catch (my_actor::force_quit_exception&)
		{//����Actor��ǿ���˳��쳣
			assert(!_actor._inActor);
			_actor._inActor = true;
		}
#ifdef _DEBUG
		catch (my_actor::pump_disconnected_exception&)
		{
			assert(false);
		}
		catch (...)
		{
			assert(false);
		}
#endif
		assert(_actor._quited);
		clear_function(_actor._mainFunc);
		if (_actor._timer)
		{
			_actor._timerCount++;
			_actor.cancel_timer();
		}
		_actor._msgPoolStatus.clear(&_actor);
		_actor._inActor = false;
		_actor._exited = true;
		DEBUG_OPERATION(size_t yc = _actor.yield_count());
		while (!_actor._exitCallback.empty())
		{
			assert(_actor._exitCallback.front());
			CHECK_EXCEPTION1(_actor._exitCallback.front(), !_actor._isForce);
			_actor._exitCallback.pop_front();
		}
		assert(_actor.yield_count() == yc);
	}
private:
	my_actor& _actor;
};

my_actor::my_actor()
{
	_actorPull = NULL;
	_actorPush = NULL;
	_timer = NULL;
	_quited = false;
	_exited = false;
	_started = false;
	_inActor = false;
	_suspended = false;
	_hasNotify = false;
	_isForce = false;
	_notifyQuited = false;
	_lockQuit = 0;
	_stackTop = NULL;
	_stackSize = 0;
	_yieldCount = 0;
	_timerCount = 0;
	_childOverCount = 0;
	_childSuspendResumeCount = 0;
	_selfID = ++_actorIDCount;
}

my_actor::my_actor(const my_actor&)
{

}

my_actor::~my_actor()
{
	assert(_quited);
	assert(_exited);
	assert(!_mainFunc);
	assert(!_childOverCount);
	assert(!_lockQuit);
	assert(!_childSuspendResumeCount);
	assert(_suspendResumeQueue.empty());
	assert(_quitHandlerList.empty());
	assert(_suspendResumeQueue.empty());
	assert(_exitCallback.empty());
	assert(_childActorList.empty());
	if (_timer)
	{
		if (actor_stack_pool::isEnable())
		{
			_timer->~timer_pck();
		}
		else
		{
			delete _timer;
		}
	}
#ifdef CHECK_ACTOR
	_stackLineMutex.lock();
	_stackLine.erase(_btIt);
	_stackLine.erase(_topIt);
	_stackLineMutex.unlock();
#endif
#if (CHECK_ACTOR_STACK) || (_DEBUG)
	if (*(long long*)((BYTE*)_stackTop-_stackSize+STACK_RESERVED_SPACE_SIZE-sizeof(long long)) != 0xFEFEFEFEFEFEFEFE)
	{
		assert(false);
		char buf[48];
		sprintf_s(buf, "%llu Actor��ջ���", _selfID);
		throw std::shared_ptr<string>(new string(buf));
	}
#endif
	delete (actor_pull_type*)_actorPull;
}

my_actor& my_actor::operator =(const my_actor&)
{
	return *this;
}

actor_handle my_actor::create( shared_strand actorStrand, const main_func& mainFunc, size_t stackSize )
{
	return create(actorStrand, mainFunc, std::function<void (bool)>(), stackSize);
}

actor_handle my_actor::create( shared_strand actorStrand, const main_func& mainFunc,
	const std::function<void (bool)>& cb, size_t stackSize )
{
	assert(stackSize && stackSize <= 1024 kB && 0 == stackSize % (4 kB));
	actor_handle newActor;
	if (actor_stack_pool::isEnable())
	{
		/*�ڴ�ṹ:|------Actor Stack------|----timer_type----|---shared_ptr_ref_count---|--Actor Obj--|*/
		const size_t actorSize = MEM_ALIGN(sizeof(my_actor), sizeof(void*));
		const size_t timerSize = MEM_ALIGN(sizeof(timer_pck), sizeof(void*));
		stack_pck stackMem = actor_stack_pool::getStack(stackSize);
		BYTE* refTop = (BYTE*)stackMem._stack.sp - actorSize;
		newActor = actor_handle(new(refTop)my_actor, [stackMem](my_actor* p){p->~my_actor(); },
			actor_ref_count_alloc<void>(stackMem, (void**)&refTop));
		newActor->_stackTop = refTop - timerSize;
		newActor->_stackSize = stackMem._stack.size - ((size_t)stackMem._stack.sp - (size_t)newActor->_stackTop);
		newActor->_strand = actorStrand;
		newActor->_mainFunc = mainFunc;
		if (_autoMakeTimer)
		{
			newActor->_timer = new(newActor->_stackTop) timer_pck(actorStrand->get_ios_proxy());
		}
		if (cb) newActor->_exitCallback.push_back(cb);
		newActor->_actorPull = new actor_pull_type(boost_actor_run(*newActor),
			boost::coroutines::attributes(newActor->_stackSize), actor_stack_pool_allocate(newActor->_stackTop, newActor->_stackSize));
	} 
	else
	{
		newActor = actor_handle(new my_actor, [](my_actor* p){delete p; });
		if (_autoMakeTimer)
		{
			newActor->_timer = new timer_pck(actorStrand->get_ios_proxy());
		}
		newActor->_strand = actorStrand;
		newActor->_mainFunc = mainFunc;
		if (cb) newActor->_exitCallback.push_back(cb);
		newActor->_actorPull = new actor_pull_type(boost_actor_run(*newActor),
			boost::coroutines::attributes(stackSize), actor_stack_allocate(&newActor->_stackTop, &newActor->_stackSize));
	}
	newActor->_weakThis = newActor;
#ifdef CHECK_ACTOR
	_stackLineMutex.lock();
	newActor->_btIt = _stackLine.insert(make_pair((BYTE*)newActor->_stackTop-newActor->_stackSize, newActor.get())).first;
	newActor->_topIt = _stackLine.insert(make_pair(newActor->_stackTop, (my_actor*)NULL)).first;
	_stackLineMutex.unlock();
#endif
#if (CHECK_ACTOR_STACK) || (_DEBUG)
	*(long long*)((BYTE*)newActor->_stackTop-newActor->_stackSize+STACK_RESERVED_SPACE_SIZE-sizeof(long long)) = 0xFEFEFEFEFEFEFEFE;
#endif
	return newActor;
}

void my_actor::async_create( shared_strand actorStrand, const main_func& mainFunc,
	const std::function<void (actor_handle)>& ch, size_t stackSize )
{
	actorStrand->asyncInvoke([actorStrand, mainFunc, stackSize]()->actor_handle
	{
		return my_actor::create(actorStrand, mainFunc, stackSize);
	}, ch);
}

void my_actor::async_create( shared_strand actorStrand, const main_func& mainFunc,
	const std::function<void (actor_handle)>& ch, const std::function<void (bool)>& cb, size_t stackSize )
{
	actorStrand->asyncInvoke([actorStrand, mainFunc, cb, stackSize]()->actor_handle
	{
		return my_actor::create(actorStrand, mainFunc, cb, stackSize);
	}, ch);
}

child_actor_handle::child_actor_param my_actor::create_child_actor( shared_strand actorStrand, const main_func& mainFunc, size_t stackSize )
{
	assert_enter();
	child_actor_handle::child_actor_param actorHandle;
	actorHandle._actor = my_actor::create(actorStrand, mainFunc, stackSize);
	actorHandle._actor->_parentActor = shared_from_this();
	_childActorList.push_front(actorHandle._actor);
	actorHandle._actorIt = _childActorList.begin();
	DEBUG_OPERATION(actorHandle._isCopy = false);
	return actorHandle;
}

child_actor_handle::child_actor_param my_actor::create_child_actor(const main_func& mainFunc, size_t stackSize)
{
	return create_child_actor(self_strand(), mainFunc, stackSize);
}

void my_actor::child_actor_run( child_actor_handle& actorHandle )
{
	assert_enter();
	assert(!actorHandle._quited);
	assert(actorHandle.get_actor());
	assert(actorHandle.get_actor()->parent_actor()->self_id() == self_id());
	actorHandle._param._actor->notify_run();
}

void my_actor::child_actor_run(const list<std::shared_ptr<child_actor_handle> >& actorHandles)
{
	assert_enter();
	for (auto it = actorHandles.begin(); it != actorHandles.end(); it++)
	{
		child_actor_run(**it);
	}
}

bool my_actor::child_actor_force_quit( child_actor_handle& actorHandle )
{
	assert_enter();
	if (!actorHandle._quited)
	{
		assert(actorHandle.get_actor());
		assert(actorHandle.get_actor()->parent_actor()->self_id() == self_id());

		actor_handle actor = actorHandle.peel();
		if (actor->self_strand() == _strand)
		{
			actorHandle._norQuit = trig<bool>([actor](const trig_once_notifer<bool>& h){actor->force_quit(h); });
		} 
		else
		{
			actorHandle._norQuit = trig<bool>([actor](const trig_once_notifer<bool>& h){actor->notify_quit(h); });
		}
	}
	return actorHandle._norQuit;
}

void my_actor::child_actors_force_quit(const list<child_actor_handle::ptr>& actorHandles)
{
	assert_enter();
	actor_msg_handle<> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = actorHandles.begin(); it != actorHandles.end(); it++)
	{
		child_actor_handle::ptr actorHandle = *it;
		assert(actorHandle->get_actor());
		assert(actorHandle->get_actor()->parent_actor()->self_id() == self_id());
		actor_handle actor = actorHandle->peel();
		if (actor->self_strand() == _strand)
		{
			actor->force_quit(wrap_no_params(h));
		} 
		else
		{
			actor->notify_quit(wrap_no_params(h));
		}
	}
	for (size_t i = actorHandles.size(); i > 0; i--)
	{
		wait_msg(amh);
	}
	close_msg_notifer(amh);
}

bool my_actor::child_actor_wait_quit( child_actor_handle& actorHandle )
{
	assert_enter();
	if (!actorHandle._quited)
	{
		assert(actorHandle.get_actor());
		assert(actorHandle.get_actor()->parent_actor()->self_id() == self_id());
		actorHandle._norQuit = actor_wait_quit(actorHandle.get_actor());
		actorHandle.peel();
	}
	return actorHandle._norQuit;
}

bool my_actor::timed_child_actor_wait_quit(int tm, child_actor_handle& actorHandle)
{
	assert_enter();
	if (!actorHandle._quited)
	{
		assert(actorHandle.get_actor());
		assert(actorHandle.get_actor()->parent_actor()->self_id() == self_id());
		if (timed_actor_wait_quit(tm, actorHandle.get_actor()))
		{
			actorHandle._norQuit = !actorHandle.get_actor()->_isForce;
			actorHandle.peel();
			return true;
		} 
		return false;
	}
	return true;
}

void my_actor::child_actors_wait_quit(const list<child_actor_handle::ptr>& actorHandles)
{
	assert_enter();
	for (auto it = actorHandles.begin(); it != actorHandles.end(); it++)
	{
		assert((*it)->get_actor()->parent_actor()->self_id() == self_id());
		child_actor_wait_quit(**it);
	}
}

void my_actor::child_actor_suspend(child_actor_handle& actorHandle)
{
	assert_enter();
	assert(actorHandle.get_actor());
	assert(actorHandle.get_actor()->parent_actor()->self_id() == self_id());
	actor_handle actor = actorHandle.get_actor();
	if (actorHandle.get_actor()->self_strand() == _strand)
	{
		trig([actor](const trig_once_notifer<>& h){actor->suspend(h); });
	} 
	else
	{
		trig([actor](const trig_once_notifer<>& h){actor->notify_suspend(h); });
	}
}

void my_actor::child_actors_suspend(const list<child_actor_handle::ptr>& actorHandles)
{
	assert_enter();
	actor_msg_handle<> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = actorHandles.begin(); it != actorHandles.end(); it++)
	{
		child_actor_handle::ptr actorHandle = *it;
		assert(actorHandle->get_actor());
		assert(actorHandle->get_actor()->parent_actor()->self_id() == self_id());
		actor_handle actor = actorHandle->get_actor();
		if (actor->self_strand() == _strand)
		{
			actor->suspend(h);
		}
		else
		{
			actor->notify_suspend(h);
		}
	}
	for (size_t i = actorHandles.size(); i > 0; i--)
	{
		wait_msg(amh);
	}
	close_msg_notifer(amh);
}

void my_actor::child_actor_resume(child_actor_handle& actorHandle)
{
	assert_enter();
	assert(actorHandle.get_actor());
	assert(actorHandle.get_actor()->parent_actor()->self_id() == self_id());
	actor_handle actor = actorHandle.get_actor();
	if (actorHandle.get_actor()->self_strand() == _strand)
	{
		trig([actor](const trig_once_notifer<>& h){actor->resume(h); });
	}
	else
	{
		trig([actor](const trig_once_notifer<>& h){actor->notify_resume(h); });
	}
}

void my_actor::child_actors_resume(const list<child_actor_handle::ptr>& actorHandles)
{
	assert_enter();
	actor_msg_handle<> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = actorHandles.begin(); it != actorHandles.end(); it++)
	{
		child_actor_handle::ptr actorHandle = *it;
		assert(actorHandle->get_actor());
		assert(actorHandle->get_actor()->parent_actor()->self_id() == self_id());
		actor_handle actor = actorHandle->get_actor();
		if (actor->self_strand() == _strand)
		{
			actor->resume(h);
		}
		else
		{
			actor->notify_resume(h);
		}
	}
	for (size_t i = actorHandles.size(); i > 0; i--)
	{
		wait_msg(amh);
	}
	close_msg_notifer(amh);
}

bool my_actor::run_child_actor_complete( shared_strand actorStrand, const main_func& h, size_t stackSize )
{
	assert_enter();
	child_actor_handle actorHandle = create_child_actor(actorStrand, h, stackSize);
	child_actor_run(actorHandle);
	return child_actor_wait_quit(actorHandle);
}

bool my_actor::run_child_actor_complete(const main_func& h, size_t stackSize)
{
	return run_child_actor_complete(self_strand(), h, stackSize);
}

void my_actor::open_timer()
{
	assert_enter();
	if (!_timer)
	{
		if (actor_stack_pool::isEnable())
		{
			size_t timerSize = MEM_ALIGN(sizeof(timer_pck), sizeof(void*));
			_timer = new(_stackTop) timer_pck(_strand->get_ios_proxy());
		}
		else
		{
			_timer = new timer_pck(_strand->get_ios_proxy());
		}
	}
}

void my_actor::close_timer()
{
	assert_enter();
	if (_timer)
	{
		_timerCount++;
		if (actor_stack_pool::isEnable())
		{
			_timer->~timer_pck();
		} 
		else
		{
			delete _timer;
		}
		_timer = NULL;
	}
}

void my_actor::sleep( int ms )
{
	assert_enter();
	actor_handle shared_this = shared_from_this();
	delay_trig(ms, [shared_this]{shared_this->run_one(); });
	push_yield();
}

void my_actor::yield()
{
	assert_enter();
	actor_handle shared_this = shared_from_this();
	_strand->post([shared_this]{shared_this->run_one(); });
	push_yield();
}

actor_handle my_actor::parent_actor()
{
	return _parentActor.lock();
}

const list<actor_handle>& my_actor::child_actors()
{
	return _childActorList;
}

my_actor::quit_iterator my_actor::regist_quit_handler( const std::function<void ()>& quitHandler )
{
	assert_enter();
	_quitHandlerList.push_front(quitHandler);//��ע�����ִ��
	return _quitHandlerList.begin();
}

void my_actor::cancel_quit_handler( quit_iterator qh )
{
	assert_enter();
	_quitHandlerList.erase(qh);
}

void my_actor::cancel_delay_trig()
{
	assert_enter();
	cancel_timer();
}

void my_actor::trig_handler()
{
	actor_handle shared_this = shared_from_this();
	_strand->post([shared_this]{shared_this->run_one(); });
}

shared_strand my_actor::self_strand()
{
	return _strand;
}

actor_handle my_actor::shared_from_this()
{
	return _weakThis.lock();
}

long long my_actor::self_id()
{
	return _selfID;
}

size_t my_actor::yield_count()
{
	assert(_strand->running_in_this_thread());
	return _yieldCount;
}

void my_actor::reset_yield()
{
	assert_enter();
	_yieldCount = 0;
}

void my_actor::notify_run()
{
	actor_handle shared_this = shared_from_this();
	_strand->post([shared_this]{shared_this->start_run(); });
}

void my_actor::assert_enter()
{
	assert(_strand->running_in_this_thread());
	assert(!_quited);
	assert(_inActor);
	assert((size_t)get_sp() >= (size_t)_stackTop-_stackSize+1024);
}

void my_actor::start_run()
{
	if (_strand->running_in_this_thread())
	{
		assert(!_inActor);
		assert(!_started);
		if (!_quited && !_started)
		{
			_started = true;
			pull_yield();
		}
	} 
	else
	{
		notify_run();
	}
}

void my_actor::notify_quit()
{
	actor_handle shared_this = shared_from_this();
	_strand->post([shared_this]{shared_this->force_quit(std::function<void(bool)>()); });
}

void my_actor::notify_quit(const std::function<void (bool)>& h)
{
	actor_handle shared_this = shared_from_this();
	_strand->post([=]{shared_this->force_quit(h); });
}

void my_actor::force_quit( const std::function<void (bool)>& h )
{
	assert(_strand->running_in_this_thread());
	assert(!_inActor);
	if (!_quited)
	{
		if (!_lockQuit)
		{
			_quited = true;
			_isForce = true;
			if (h) _exitCallback.push_back(h);
			if (!_childActorList.empty())
			{
				_childOverCount = _childActorList.size();
				while (!_childActorList.empty())
				{
					auto cc = _childActorList.front();
					_childActorList.pop_front();
					actor_handle shared_this = shared_from_this();
					if (cc->_strand == _strand)
					{
						cc->force_quit([shared_this](bool){shared_this->force_quit_cb_handler(); });
					}
					else
					{
						cc->notify_quit(_strand->wrap_post((std::function<void(bool)>)[shared_this](bool){shared_this->force_quit_cb_handler(); }));
					}
				}
			}
			else
			{
				exit_callback();
			}
			return;
		} 
		else if (h)
		{
			_exitCallback.push_back(h);
		}
		_notifyQuited = true;
	}
	else if (h)
	{
		if (_exited)
		{
			DEBUG_OPERATION(size_t yc = yield_count());
			CHECK_EXCEPTION1(h, !_isForce);
			assert(yield_count() == yc);
		} 
		else
		{
			_exitCallback.push_back(h);
		}
	}
}

bool my_actor::is_started()
{
	assert(_strand->running_in_this_thread());
	return _started;
}

bool my_actor::is_quited()
{
	assert(_strand->running_in_this_thread());
	return _quited;
}

bool my_actor::in_actor()
{
	assert(_strand->running_in_this_thread());
	return _inActor;
}

bool my_actor::quit_msg()
{
	assert_enter();
	return _notifyQuited;
}

void my_actor::lock_quit()
{
	assert_enter();
	_lockQuit++;
}

void my_actor::unlock_quit()
{
	assert_enter();
	assert(_lockQuit);
	if (0 == --_lockQuit && _notifyQuited)
	{
		_notifyQuited = false;
		notify_quit();
		push_yield();
	}
}

void my_actor::notify_suspend()
{
	notify_suspend(std::function<void ()>());
}

void my_actor::notify_suspend(const std::function<void ()>& h)
{
	actor_handle shared_this = shared_from_this();
	_strand->post([=]{shared_this->suspend(h); });
}

void my_actor::suspend(const std::function<void ()>& h)
{
	assert(_strand->running_in_this_thread());
	assert(!_inActor);

	_suspendResumeQueue.push_back(suspend_resume_option());
	suspend_resume_option& tmp = _suspendResumeQueue.back();
	tmp._isSuspend = true;
	tmp._h = h;
	if (_suspendResumeQueue.size() == 1)
	{
		suspend();
	}
}

void my_actor::suspend()
{
	assert(_strand->running_in_this_thread());
	assert(!_inActor);

	assert(!_childSuspendResumeCount);
	if (!_quited)
	{
		if (!_suspended)
		{
			_suspended = true;
			if (_timer)
			{
				suspend_timer();
			}
			if (!_childActorList.empty())
			{
				_childSuspendResumeCount = _childActorList.size();
				for (auto it = _childActorList.begin(); it != _childActorList.end(); it++)
				{
					actor_handle shared_this = shared_from_this();
					if ((*it)->_strand == _strand)
					{
						(*it)->suspend([shared_this]{shared_this->child_suspend_cb_handler(); });
					}
					else
					{
						(*it)->notify_suspend(_strand->wrap_post([shared_this]{shared_this->child_suspend_cb_handler(); }));
					}
				}
				return;
			} 
		}
	}
	_childSuspendResumeCount = 1;
	child_suspend_cb_handler();
}

void my_actor::notify_resume()
{
	notify_resume(std::function<void ()>());
}

void my_actor::notify_resume(const std::function<void ()>& h)
{
	actor_handle shared_this = shared_from_this();
	_strand->post([=]{shared_this->resume(h); });
}

void my_actor::resume(const std::function<void ()>& h)
{
	assert(_strand->running_in_this_thread());
	assert(!_inActor);

	_suspendResumeQueue.push_back(suspend_resume_option());
	suspend_resume_option& tmp = _suspendResumeQueue.back();
	tmp._isSuspend = false;
	tmp._h = h;
	if (_suspendResumeQueue.size() == 1)
	{
		resume();
	}
}

void my_actor::resume()
{
	assert(_strand->running_in_this_thread());
	assert(!_inActor);

	assert(!_childSuspendResumeCount);
	if (!_quited)
	{
		if (_suspended)
		{
			_suspended = false;
			if (_timer)
			{
				resume_timer();
			}
			if (!_childActorList.empty())
			{
				_childSuspendResumeCount = _childActorList.size();
				for (auto it = _childActorList.begin(); it != _childActorList.end(); it++)
				{
					actor_handle shared_this = shared_from_this();
					if ((*it)->_strand == _strand)
					{
						(*it)->resume(_strand->wrap_post([shared_this]{shared_this->child_resume_cb_handler(); }));
					} 
					else
					{
						(*it)->notify_resume(_strand->wrap_post([shared_this]{shared_this->child_resume_cb_handler(); }));
					}
				}
				return;
			} 
		}
	}
	_childSuspendResumeCount = 1;
	child_resume_cb_handler();
}

void my_actor::switch_pause_play()
{
	switch_pause_play(std::function<void (bool isPaused)>());
}

void my_actor::switch_pause_play(const std::function<void (bool isPaused)>& h)
{
	actor_handle shared_this = shared_from_this();
	_strand->post([shared_this, h]
	{
		assert(shared_this->_strand->running_in_this_thread());
		if (!shared_this->_quited)
		{
			if (shared_this->_suspended)
			{
				if (h)
				{
					auto& h_ = h;
					shared_this->resume([h_]{h_(false); });
				} 
				else
				{
					shared_this->resume(std::function<void()>());
				}
			} 
			else
			{
				if (h)
				{
					auto& h_ = h;
					shared_this->suspend([h_]{h_(true); });
				} 
				else
				{
					shared_this->suspend(std::function<void()>());
				}
			}
		}
		else if (h)
		{
			DEBUG_OPERATION(size_t yc = shared_this->yield_count());
			CHECK_EXCEPTION1(h, true);
			assert(shared_this->yield_count() == yc);
		}
	});
}

bool my_actor::outside_wait_quit()
{
	assert(!_strand->in_this_ios());
	bool rt = false;
	boost::condition_variable conVar;
	boost::mutex mutex;
	boost::unique_lock<boost::mutex> ul(mutex);
	_strand->post([&]
	{
		assert(_strand->running_in_this_thread());
		if (_quited && !_childOverCount)
		{
			rt = !_isForce;
			boost::lock_guard<boost::mutex> lg(mutex);
			conVar.notify_one();
		}
		else
		{
			bool& rt_ = rt;
			auto& mutex_ = mutex;
			auto& conVar_ = conVar;
			_exitCallback.push_back([&](bool ok)
			{
				rt_ = ok;
				boost::lock_guard<boost::mutex> lg(mutex_);
				conVar_.notify_one();
			});
		}
	});
	conVar.wait(ul);
	return rt;
}

void my_actor::append_quit_callback(const std::function<void (bool)>& h)
{
	if (_strand->running_in_this_thread())
	{
		if (_quited && !_childOverCount)
		{
			DEBUG_OPERATION(size_t yc = yield_count());
			CHECK_EXCEPTION1(h, !_isForce);
			assert(yield_count() == yc);
		} 
		else
		{
			_exitCallback.push_back(h);
		}
	} 
	else
	{
		actor_handle shared_this = shared_from_this();
		_strand->post([=]{shared_this->append_quit_callback(h); });
	}
}

void my_actor::actors_start_run(const list<actor_handle>& anotherActors)
{
	assert_enter();
	for (auto it = anotherActors.begin(); it != anotherActors.end(); it++)
	{
		(*it)->notify_run();
	}
}

bool my_actor::actor_force_quit( actor_handle anotherActor )
{
	assert_enter();
	assert(anotherActor);
	return trig<bool>([anotherActor](const trig_once_notifer<bool>& h){anotherActor->notify_quit(h); });
}

void my_actor::actors_force_quit(const list<actor_handle>& anotherActors)
{
	assert_enter();
	actor_msg_handle<> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = anotherActors.begin(); it != anotherActors.end(); it++)
	{
		(*it)->notify_quit(wrap_no_params(h));
	}
	for (size_t i = anotherActors.size(); i > 0; i--)
	{
		wait_msg(amh);
	}
	close_msg_notifer(amh);
}

bool my_actor::actor_wait_quit( actor_handle anotherActor )
{
	assert_enter();
	assert(anotherActor);
	return trig<bool>([anotherActor](const trig_once_notifer<bool>& h){anotherActor->append_quit_callback(h); });
}

void my_actor::actors_wait_quit(const list<actor_handle>& anotherActors)
{
	assert_enter();
	for (auto it = anotherActors.begin(); it != anotherActors.end(); it++)
	{
		actor_wait_quit(*it);
	}
}

bool my_actor::timed_actor_wait_quit(int tm, actor_handle anotherActor)
{
	assert_enter();
	assert(anotherActor);
	actor_trig_handle<> ath;
	anotherActor->append_quit_callback(wrap_no_params(make_trig_notifer(ath)));
	return timed_wait_trig(tm, ath);
}

void my_actor::actor_suspend(actor_handle anotherActor)
{
	assert_enter();
	assert(anotherActor);
	trig([anotherActor](const trig_once_notifer<>& h){anotherActor->notify_suspend(h); });
}

void my_actor::actors_suspend(const list<actor_handle>& anotherActors)
{
	assert_enter();
	actor_msg_handle<> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = anotherActors.begin(); it != anotherActors.end(); it++)
	{
		(*it)->notify_suspend(h);
	}
	for (size_t i = anotherActors.size(); i > 0; i--)
	{
		wait_msg(amh);
	}
	close_msg_notifer(amh);
}

void my_actor::actor_resume(actor_handle anotherActor)
{
	assert_enter();
	assert(anotherActor);
	trig([anotherActor](const trig_once_notifer<>& h){anotherActor->notify_resume(h); });
}

void my_actor::actors_resume(const list<actor_handle>& anotherActors)
{
	assert_enter();
	actor_msg_handle<> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = anotherActors.begin(); it != anotherActors.end(); it++)
	{
		(*it)->notify_resume(h);
	}
	for (size_t i = anotherActors.size(); i > 0; i--)
	{
		wait_msg(amh);
	}
	close_msg_notifer(amh);
}

bool my_actor::actor_switch(actor_handle anotherActor)
{
	assert_enter();
	assert(anotherActor);
	return trig<bool>([anotherActor](const trig_once_notifer<bool>& h){anotherActor->switch_pause_play(h); });
}

bool my_actor::actors_switch(const list<actor_handle>& anotherActors)
{
	assert_enter();
	bool isPause = true;
	actor_msg_handle<bool> amh;
	auto h = make_msg_notifer(amh);
	for (auto it = anotherActors.begin(); it != anotherActors.end(); it++)
	{
		(*it)->switch_pause_play(h);
	}
	for (size_t i = anotherActors.size(); i > 0; i--)
	{
		isPause &= wait_msg(amh);
	}
	close_msg_notifer(amh);
	return isPause;
}

void my_actor::run_one()
{
	assert(_strand->running_in_this_thread());
	assert(!_inActor);
	if (!_quited)
	{
		pull_yield();
	}
}

void my_actor::pull_yield()
{
	assert(!_quited);
	if (!_suspended)
	{
		(*(actor_pull_type*)_actorPull)();
	}
	else
	{
		assert(!_hasNotify);
		_hasNotify = true;
	}
}

void my_actor::pull_yield_as_mutex()
{
	(*(actor_pull_type*)_actorPull)();
}

void my_actor::push_yield()
{
	assert(!_quited);
	assert(_inActor);
	_yieldCount++;
	_inActor = false;
	(*(actor_push_type*)_actorPush)();
	if (!_quited)
	{
		_inActor = true;
		return;
	}
	throw force_quit_exception();
}

void my_actor::push_yield_as_mutex()
{
	(*(actor_push_type*)_actorPush)();
}

void my_actor::force_quit_cb_handler()
{
	assert(_quited);
	assert(_isForce);
	assert(!_inActor);
	assert((int)_childOverCount > 0);
	if (0 == --_childOverCount)
	{
		exit_callback();
	}
}

void my_actor::child_suspend_cb_handler()
{
	assert(_strand->running_in_this_thread());
	assert((int)_childSuspendResumeCount > 0);
	if (0 == --_childSuspendResumeCount)
	{
		assert(_suspendResumeQueue.front()._isSuspend);
		while (!_suspendResumeQueue.empty())
		{
			suspend_resume_option& tmp = _suspendResumeQueue.front();
			if (tmp._isSuspend)
			{
				if (tmp._h)
				{
					DEBUG_OPERATION(size_t yc = yield_count());
					CHECK_EXCEPTION(tmp._h);
					assert(yield_count() == yc);
				}
				_suspendResumeQueue.pop_front();
			} 
			else
			{
				actor_handle shared_this = shared_from_this();
				_strand->post([shared_this]{shared_this->resume(); });
				return;
			}
		}
	}
}

void my_actor::child_resume_cb_handler()
{
	assert(_strand->running_in_this_thread());
	assert((int)_childSuspendResumeCount > 0);
	if (0 == --_childSuspendResumeCount)
	{
		assert(!_suspendResumeQueue.front()._isSuspend);
		while (!_suspendResumeQueue.empty())
		{
			suspend_resume_option& tmp = _suspendResumeQueue.front();
			if (!tmp._isSuspend)
			{
				if (tmp._h)
				{
					DEBUG_OPERATION(size_t yc = yield_count());
					CHECK_EXCEPTION(tmp._h);
					assert(yield_count() == yc);
				}
				_suspendResumeQueue.pop_front();
			} 
			else
			{
				actor_handle shared_this = shared_from_this();
				_strand->post([shared_this]{shared_this->suspend(); });
				return;
			}
		}
		if (_hasNotify)
		{
			_hasNotify = false;
			run_one();
		}
	}
}

void my_actor::exit_callback()
{
	assert(_quited);
	assert(!_childOverCount);
	DEBUG_OPERATION(size_t yc = yield_count());
	while (!_quitHandlerList.empty())
	{
		CHECK_EXCEPTION(_quitHandlerList.front());
		_quitHandlerList.pop_front();
	}
	assert(yield_count() == yc);
	(*(actor_pull_type*)_actorPull)();
}

void my_actor::enable_stack_pool()
{
	assert(0 == _actorIDCount);
	actor_stack_pool::enable();
	_sharedBoolPool = std::shared_ptr<shared_obj_pool_base<bool>>(create_shared_pool<bool>(4096, [](void*){}));
}

void my_actor::expires_timer()
{
	size_t tid = ++_timerCount;
	actor_handle shared_this = shared_from_this();
	boost::system::error_code ec;
	_timer->_timer->expires_from_now(boost::chrono::microseconds(_timer->_timerTime.total_microseconds()), ec);
	_timer->_timer->async_wait(_strand->wrap_post([shared_this, tid](const boost::system::error_code& err)
	{
		if (tid == shared_this->_timerCount)
		{
			timer_pck* timer = shared_this->_timer;
			assert(!err);
			assert(timer);
			assert(!timer->_timerSuspend && !timer->_timerCompleted);
			timer->_timerCompleted = true;
			std::function<void()> h;
			timer->_h.swap(h);
			h();
		}
	}));
}

void my_actor::time_out(int ms, const std::function<void ()>& h)
{
	assert_enter();
	assert(_timer);
	assert(_timer->_timerCompleted);
	assert(!_timer->_timerSuspend);
	assert(_timer->_h._Empty());
	assert(ms > 0);
	_timer->_timerCompleted = false;
	_timer->_h = h;
	_timer->_timerTime = boost::posix_time::microsec((unsigned long long)ms * 1000);
	_timer->_timerStampBegin = boost::posix_time::microsec_clock::universal_time();
	expires_timer();
}

void my_actor::cancel_timer()
{
	assert(_timer);
	if (!_timer->_timerCompleted)
	{
		_timerCount++;
		_timer->_timerCompleted = true;
		clear_function(_timer->_h);
		boost::system::error_code ec;
		_timer->_timer->cancel(ec);
	}
}

void my_actor::suspend_timer()
{
	assert(_timer);
	if (!_timer->_timerSuspend)
	{
		_timer->_timerSuspend = true;
		if (!_timer->_timerCompleted)
		{
			_timerCount++;
			boost::system::error_code ec;
			_timer->_timer->cancel(ec);
			_timer->_timerStampEnd = boost::posix_time::microsec_clock::universal_time();
			auto tt = _timer->_timerStampBegin+_timer->_timerTime;
			if (_timer->_timerStampEnd > tt)
			{
				_timer->_timerStampEnd = tt;
			}
		}
	}
}

void my_actor::resume_timer()
{
	assert(_timer);
	if (_timer->_timerSuspend)
	{
		_timer->_timerSuspend = false;
		if (!_timer->_timerCompleted)
		{
			assert(_timer->_timerTime >= _timer->_timerStampEnd-_timer->_timerStampBegin);
			_timer->_timerTime -= _timer->_timerStampEnd-_timer->_timerStampBegin;
			_timer->_timerStampBegin = boost::posix_time::microsec_clock::universal_time();
			expires_timer();
		}
	}
}

void my_actor::disable_auto_make_timer()
{
	assert(0 == _actorIDCount);
	_autoMakeTimer = false;
}

void my_actor::check_stack()
{
#if (CHECK_ACTOR_STACK) || (_DEBUG)
	if ((size_t)get_sp() < (size_t)_stackTop-_stackSize+STACK_RESERVED_SPACE_SIZE)
	{
		assert(false);
		throw std::shared_ptr<string>(new string("Actor��ջ�쳣"));
	}
#endif
	check_actor();
}

my_actor* my_actor::self_actor()
{
#ifdef CHECK_ACTOR
	boost::lock_guard<boost::mutex> lg(_stackLineMutex);
	auto it = _stackLine.insert(make_pair(get_sp(), (my_actor*)NULL)).first;
	if (it != _stackLine.end())
	{
		_stackLine.erase(it--);
		if (it != _stackLine.end())
		{
			return it->second;
		}
	}
#endif
	return NULL;
}

void my_actor::check_actor()
{
#ifdef CHECK_ACTOR
	assert(this == self_actor());
#endif
}

size_t my_actor::stack_free_space()
{
	int s = (int)((size_t)get_sp() - ((size_t)_stackTop-_stackSize+STACK_RESERVED_SPACE_SIZE));
	if (s < 0)
	{
		return 0;
	}
	return (size_t)s;
}

bool my_actor::msg_agent_to(child_actor_handle& childActor)
{
	return msg_agent_to<void, void, void, void>(childActor.get_actor());
}

void my_actor::msg_agent_off()
{
	msg_agent_off<void, void, void, void>();
}

post_actor_msg<> my_actor::connect_msg_notifer_to(const actor_handle& buddyActor, bool makeNew)
{
	return connect_msg_notifer_to<void, void, void, void>(buddyActor, makeNew, 0);
}

post_actor_msg<> my_actor::connect_msg_notifer_to(child_actor_handle& childActor, bool makeNew)
{
	return connect_msg_notifer_to<void, void, void, void>(childActor.get_actor(), makeNew, 0);
}

post_actor_msg<> my_actor::connect_msg_notifer()
{
	return connect_msg_notifer<void, void, void, void>(0);
}

post_actor_msg<> my_actor::connect_msg_notifer_to_self(bool makeNew)
{
	return connect_msg_notifer_to_self<void, void, void, void>(makeNew, 0);
}

msg_pump<>::handle my_actor::connect_msg_pump()
{
	return connect_msg_pump<void, void, void, void>();
}

actor_msg_notifer<> my_actor::make_msg_notifer(actor_msg_handle<>& amh)
{
	return amh.make_notifer(shared_from_this());
}

void my_actor::close_msg_notifer(actor_msg_handle_base& amh)
{
	assert_enter();
	amh.close();
}

bool my_actor::timed_wait_msg(int tm, actor_msg_handle<>& amh)
{
	assert_enter();
	assert(amh._closed && !(*amh._closed));
	if (!amh.read_msg())
	{
		bool timeout = false;
		if (tm >= 0)
		{
			delay_trig(tm, [this, &timeout]
			{
				if (!_quited)
				{
					timeout = true;
					pull_yield();
				}
			});
		}
		push_yield();
		if (!timeout)
		{
			if (tm >= 0)
			{
				cancel_delay_trig();
			}
			return true;
		}
		amh._waiting = false;
		return false;
	}
	return true;
}

bool my_actor::timed_pump_msg(int tm, const msg_pump<>::handle& pump, bool checkDis)
{
	assert_enter();
	if (!pump->read_msg())
	{
		if (checkDis && pump->isDisconnected())
		{
			pump->_waiting = false;
			throw pump_disconnected_exception();
		}
		pump->_checkDis = checkDis;
		bool timeout = false;
		if (tm >= 0)
		{
			delay_trig(tm, [this, &timeout]
			{
				if (!_quited)
				{
					timeout = true;
					pull_yield();
				}
			});
		}
		push_yield();
		if (!timeout)
		{
			if (tm >= 0)
			{
				cancel_delay_trig();
			}
			if (pump->_checkDis)
			{
				assert(checkDis);
				pump->_checkDis = false;
				throw pump_disconnected_exception();
			}
			return true;
		}
		pump->_checkDis = false;
		pump->_waiting = false;
		return false;
	}
	return true;
}

void my_actor::wait_msg(actor_msg_handle<>& amh)
{
	timed_wait_msg(-1, amh);
}

void my_actor::pump_msg(const msg_pump<>::handle& pump, bool checkDis)
{
	timed_pump_msg(-1, pump, checkDis);
}

actor_trig_notifer<> my_actor::make_trig_notifer(actor_trig_handle<>& ath)
{
	return ath.make_notifer(shared_from_this());
}

void my_actor::close_trig_notifer(actor_msg_handle_base& ath)
{
	assert_enter();
	ath.close();
}

bool my_actor::timed_wait_trig(int tm, actor_trig_handle<>& ath)
{
	assert_enter();
	assert(ath._closed && !(*ath._closed));
	if (!ath.read_msg())
	{
		bool timeout = false;
		if (tm >= 0)
		{
			delay_trig(tm, [this, &timeout]
			{
				if (!_quited)
				{
					timeout = true;
					pull_yield();
				}
			});
		}
		push_yield();
		if (!timeout)
		{
			if (tm >= 0)
			{
				cancel_delay_trig();
			}
			return true;
		}
		ath._waiting = false;
		return false;
	}
	return true;
}

void my_actor::wait_trig(actor_trig_handle<>& ath)
{
	timed_wait_trig(-1, ath);
}

actor_handle my_actor::msg_agent_handle(const actor_handle& buddyActor)
{
	return msg_agent_handle<void, void, void, void>(buddyActor);
}

#ifndef __ACTOR_MUTEX_H
#define __ACTOR_MUTEX_H

#include "run_strand.h"
#include "msg_queue.h"
#include "scattered.h"
#include "lambda_ref.h"
#include "generator.h"

class my_actor;

/*!
@brief Actor�����ɵݹ�
*/
class actor_mutex
{
	friend my_actor;
public:
	actor_mutex(const shared_strand& strand);
	~actor_mutex();
public:
	/*!
	@brief ������Դ����������Actor���У��ȴ���ֱ�������ȣ��� unlock ������У��ɵݹ���ã�lock�ڼ�Actor������ǿ���˳�
	@warning ����Actor�����ǿ���˳����п������ lock ���޷� unlock
	*/
	void lock(my_actor* host);

	/*!
	@brief ������Դ������Ѿ�����������lockNtf
	*/
	template <typename Ntf>
	void lock(my_actor* host, Ntf&& lockNtf)
	{
		lock(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	/*!
	@brief ���Ե�ǰ�Ƿ��Ѿ���ռ�ã�����Ѿ�����ĳ��У����Լ����в��㣩�ͷ��������û���У����Լ����У��� unlock �������
	@return �Ѿ������Actor���� false���ɹ����з���true
	*/
	bool try_lock(my_actor* host);

	/*!
	@brief ��һ��ʱ���ڳ���������Դ
	@return �ɹ�����true����ʱʧ�ܷ���false
	*/
	bool timed_lock(int ms, my_actor* host);

	/*!
	@brief ��һ��ʱ���ڳ���������Դ������Ѿ�����������lockNtf
	*/
	template <typename Ntf>
	bool timed_lock(int ms, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock(ms, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	/*!
	@brief �����ǰActor������У��ݹ� lock ���Σ�����Ҫ unlock ����
	*/
	void unlock(my_actor* host);

	/*!
	@brief ��ǰ������strand
	*/
	const shared_strand& self_strand();

	/*!
	@brief 
	*/
	co_mutex& co_ref();
private:
	void quited_lock(my_actor* host);
	void quited_unlock(my_actor* host);
	void lock(my_actor*, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock(int ms, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
private:
	co_mutex _mutex;
	NONE_COPY(actor_mutex)
};
//////////////////////////////////////////////////////////////////////////

class actor_condition_variable;
/*!
@brief ��һ����Χ������mutex��ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_lock_guard
{
	friend actor_condition_variable;
public:
	actor_lock_guard(actor_mutex& amutex, my_actor* host);
	~actor_lock_guard();
public:
	void unlock();
	void lock();
private:
	actor_mutex& _amutex;
	my_actor* _host;
	bool _isUnlock;
	NONE_COPY(actor_lock_guard)
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��Actor�����еĶ�д�������ɵݹ�
*/
class actor_shared_mutex
{
public:
	actor_shared_mutex(const shared_strand& strand);
	~actor_shared_mutex();
public:
	/*!
	@brief ��ռ��
	*/
	void lock(my_actor* host);
	bool try_lock(my_actor* host);
	bool timed_lock(int ms, my_actor* host);

	template <typename Ntf>
	void lock(my_actor* host, Ntf&& lockNtf)
	{
		lock(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	template <typename Ntf>
	bool timed_lock(int ms, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock(ms, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	/*!
	@brief ������
	*/
	void lock_shared(my_actor* host);
	bool try_lock_shared(my_actor* host);
	bool timed_lock_shared(int ms, my_actor* host);
	void lock_pess_shared(my_actor* host);

	template <typename Ntf>
	void lock_shared(my_actor* host, Ntf&& lockNtf)
	{
		lock_shared(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	template <typename Ntf>
	bool timed_lock_shared(int ms, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock_shared(ms, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	template <typename Ntf>
	void lock_pess_shared(my_actor* host, Ntf&& lockNtf)
	{
		lock_pess_shared(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	/*!
	@brief ����������Ϊ��ռ��
	*/
	void lock_upgrade(my_actor* host);
	bool try_lock_upgrade(my_actor* host);

	template <typename Ntf>
	void lock_upgrade(my_actor* host, Ntf&& lockNtf)
	{
		lock_upgrade(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(lockNtf));
	}

	/*!
	@brief �����ռ����
	*/
	void unlock(my_actor* host);

	/*!
	@brief �����������
	*/
	void unlock_shared(my_actor* host);

	/*!
	@brief �����ռ�������ָ�Ϊ��������
	*/
	void unlock_upgrade(my_actor* host);

	/*!
	@brief ��ǰ������strand
	*/
	const shared_strand& self_strand();

	/*!
	@brief
	*/
	co_shared_mutex& co_ref();
private:
	void lock(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock(int ms, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	void lock_shared(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock_shared(int ms, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	void lock_pess_shared(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	void lock_upgrade(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
private:
	co_shared_mutex _mutex;
	NONE_COPY(actor_shared_mutex)
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��һ����Χ������shared_mutex˽������ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_unique_lock
{
public:
	actor_unique_lock(actor_shared_mutex& amutex, my_actor* host);
	~actor_unique_lock();
public:
	void unlock();
	void lock();
private:
	actor_shared_mutex& _amutex;
	my_actor* _host;
	bool _isUnlock;
	NONE_COPY(actor_unique_lock)
};

/*!
@brief ��һ����Χ������shared_mutex��������ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_shared_lock
{
public:
	actor_shared_lock(actor_shared_mutex& amutex, my_actor* host);
	~actor_shared_lock();
public:
	void unlock_shared();
	void lock_shared();
private:
	actor_shared_mutex& _amutex;
	my_actor* _host;
	bool _isUnlock;
	NONE_COPY(actor_shared_lock)
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��������
*/
class actor_condition_variable
{
public:
	actor_condition_variable(const shared_strand& strand);
	~actor_condition_variable();
public:
	/*!
	@brief �ȴ�֪ͨ
	*/
	void wait(my_actor* host, actor_lock_guard& lg);
	void wait(my_actor* host, actor_mutex& mtx);
	void wait(my_actor* host, co_mutex& mtx);

	/*!
	@brief ��ʱ�ȴ�֪ͨ
	*/
	bool timed_wait(my_actor* host, actor_lock_guard& lg, int ms);
	bool timed_wait(my_actor* host, actor_mutex& mtx, int ms);
	bool timed_wait(my_actor* host, co_mutex& mtx, int ms);

	/*!
	@brief ֪ͨһ���ȴ�
	*/
	void notify_one();

	/*!
	@brief ֪ͨ�����ڵȴ���
	*/
	void notify_all();

	/*!
	@brief 
	*/
	co_condition_variable& co_ref();
private:
	co_condition_variable _conVar;
};
#endif
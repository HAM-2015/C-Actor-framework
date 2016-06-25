#ifndef __ACTOR_MUTEX_H
#define __ACTOR_MUTEX_H

#include "shared_strand.h"
#include "msg_queue.h"
#include "scattered.h"
#include "lambda_ref.h"

class my_actor;
class MutexTrigNotifer_;
class actor_condition_variable;

/*!
@brief Actor�����ɵݹ�
*/
class actor_mutex
{
	friend my_actor;

	struct wait_node
	{
		MutexTrigNotifer_& ntf;
		long long _waitHostID;
	};
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
		lock(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
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
	bool timed_lock(int tm, my_actor* host);

	/*!
	@brief ��һ��ʱ���ڳ���������Դ������Ѿ�����������lockNtf
	*/
	template <typename Ntf>
	bool timed_lock(int tm, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock(tm, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
	}

	/*!
	@brief �����ǰActor������У��ݹ� lock ���Σ�����Ҫ unlock ����
	*/
	void unlock(my_actor* host);

	/*!
	@brief ��ǰ������strand
	*/
	const shared_strand& self_strand();
private:
	void quited_lock(my_actor* host);
	void quited_unlock(my_actor* host);
	void lock(my_actor*, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock(int tm, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
private:
	shared_strand _strand;
	msg_list<wait_node> _waitQueue;
	long long _lockActorID;
	size_t _recCount;
	NONE_COPY(actor_mutex)
};
//////////////////////////////////////////////////////////////////////////

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
@brief ��Actor���е��������������actor_lock_guardʹ��
*/
class actor_condition_variable
{
	struct wait_node
	{
		MutexTrigNotifer_& ntf;
	};
public:
	actor_condition_variable(const shared_strand& strand);
	~actor_condition_variable();
public:
	/*!
	@brief �ȴ�һ��֪ͨ
	*/
	void wait(my_actor* host, actor_lock_guard& mutex);

	/*!
	@brief ��ʱ�ȴ�Ҫ��֪ͨ
	*/
	bool timed_wait(int tm, my_actor* host, actor_lock_guard& mutex);

	/*!
	@brief ֪ͨһ���ȴ�
	*/
	bool notify_one(my_actor* host);

	/*!
	@brief ֪ͨ���еȴ�
	*/
	size_t notify_all(my_actor* host);

	/*!
	@brief ��ǰ������strand
	*/
	const shared_strand& self_strand();
private:
	shared_strand _strand;
	msg_list<wait_node> _waitQueue;
	NONE_COPY(actor_condition_variable)
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��Actor�����еĶ�д�������ɵݹ�
*/
class actor_shared_mutex
{
	enum lock_status
	{
		st_shared,
		st_unique,
		st_upgrade
	};

	struct wait_node
	{
		MutexTrigNotifer_& ntf;
		long long _waitHostID;
		lock_status _status;
	};
public:
	actor_shared_mutex(const shared_strand& strand);
	~actor_shared_mutex();
public:
	/*!
	@brief ��ռ��
	*/
	void lock(my_actor* host);
	bool try_lock(my_actor* host);
	bool timed_lock(int tm, my_actor* host);

	template <typename Ntf>
	void lock(my_actor* host, Ntf&& lockNtf)
	{
		lock(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
	}

	template <typename Ntf>
	bool timed_lock(int tm, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock(tm, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
	}

	/*!
	@brief ������
	*/
	void lock_shared(my_actor* host);
	bool try_lock_shared(my_actor* host);
	bool timed_lock_shared(int tm, my_actor* host);

	template <typename Ntf>
	void lock_shared(my_actor* host, Ntf&& lockNtf)
	{
		lock_shared(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
	}

	template <typename Ntf>
	bool timed_lock_shared(int tm, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock_shared(tm, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
	}

	/*!
	@brief ����������Ϊ��ռ��
	*/
	void lock_upgrade(my_actor* host);
	bool try_lock_upgrade(my_actor* host);
	bool timed_lock_upgrade(int tm, my_actor* host);

	template <typename Ntf>
	void lock_upgrade(my_actor* host, Ntf&& lockNtf)
	{
		lock_upgrade(host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
	}

	template <typename Ntf>
	bool timed_lock_upgrade(int tm, my_actor* host, Ntf&& lockNtf)
	{
		return timed_lock_upgrade(tm, host, (wrap_local_handler_face<void()>&&)wrap_local_handler(TRY_MOVE(lockNtf)));
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
private:
	void lock(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock(int tm, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	void lock_shared(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock_shared(int tm, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	void lock_upgrade(my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
	bool timed_lock_upgrade(int tm, my_actor* host, wrap_local_handler_face<void()>&& lockNtf);
private:
	lock_status _status;
	shared_strand _strand;
	msg_list<wait_node> _waitQueue;
	msg_map<long long, lock_status> _inSet;
	NONE_COPY(actor_shared_mutex)
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��һ����Χ������shared_mutex˽������ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_unique_lock
{
	friend actor_condition_variable;
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
	friend actor_condition_variable;
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
#endif
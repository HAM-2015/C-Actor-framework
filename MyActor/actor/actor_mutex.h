#ifndef __ACTOR_MUTEX_H
#define __ACTOR_MUTEX_H

#include "shared_strand.h"

class ActorMutex_;
class ActorConditionVariable_;
class ActorSharedMutex;
class my_actor;

/*!
@brief Actor�����ɵݹ�
*/
class actor_mutex
{
	friend my_actor;
public:
	actor_mutex(const shared_strand& strand);
	actor_mutex(const actor_mutex& s);
	actor_mutex(actor_mutex&& s);
	actor_mutex();
	~actor_mutex();
public:
	void operator=(const actor_mutex& s);
	void operator=(actor_mutex&& s);
public:
	/*!
	@brief ������Դ����������Actor���У��ȴ���ֱ�������ȣ��� unlock ������У��ɵݹ���ã�lock�ڼ�Actor������ǿ���˳�
	@warning ����Actor�����ǿ���˳����п������ lock ���޷� unlock
	*/
	void lock(my_actor* host) const;

	/*!
	@brief ���Ե�ǰ�Ƿ��Ѿ���ռ�ã�����Ѿ�����ĳ��У����Լ����в��㣩�ͷ��������û���У����Լ����У��� unlock �������
	@return �Ѿ������Actor���� false���ɹ����з���true
	*/
	bool try_lock(my_actor* host) const;

	/*!
	@brief ��һ��ʱ���ڳ���������Դ
	@return �ɹ�����true����ʱʧ�ܷ���false
	*/
	bool timed_lock(int tm, my_actor* host) const;

	/*!
	@brief �����ǰActor������У��ݹ� lock ���Σ�����Ҫ unlock ����
	*/
	void unlock(my_actor* host) const;
private:
	void quited_lock(my_actor* host) const;
	void quited_unlock(my_actor* host) const;
private:
	std::shared_ptr<ActorMutex_> _amutex;
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��һ����Χ������mutex��ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_lock_guard
{
	friend ActorConditionVariable_;
public:
	actor_lock_guard(const actor_mutex& amutex, my_actor* host);
	~actor_lock_guard();
private:
	actor_lock_guard(const actor_lock_guard&);
	void operator=(const actor_lock_guard&);
public:
	void unlock();
	void lock();
private:
	actor_mutex _amutex;
	my_actor* _host;
	bool _isUnlock;
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��Actor���е��������������actor_lock_guardʹ��
*/
class actor_condition_variable
{
public:
	actor_condition_variable(const shared_strand& strand);
	actor_condition_variable(const actor_condition_variable& s);
	actor_condition_variable(actor_condition_variable&& s);
	actor_condition_variable();
	~actor_condition_variable();
public:
	void operator=(const actor_condition_variable& s);
	void operator=(actor_condition_variable&& s);
public:
	/*!
	@brief �ȴ�һ��֪ͨ
	*/
	void wait(my_actor* host, actor_lock_guard& mutex) const;

	/*!
	@brief ��ʱ�ȴ�Ҫ��֪ͨ
	*/
	bool timed_wait(int tm, my_actor* host, actor_lock_guard& mutex) const;

	/*!
	@brief ֪ͨһ���ȴ�
	*/
	bool notify_one(my_actor* host) const;

	/*!
	@brief ֪ͨ���еȴ�
	*/
	size_t notify_all(my_actor* host) const;
private:
	std::shared_ptr<ActorConditionVariable_> _aconVar;
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��Actor�����еĶ�д�������ɵݹ�
*/
class actor_shared_mutex
{
public:
	actor_shared_mutex(const shared_strand& strand);
	actor_shared_mutex(const actor_shared_mutex& s);
	actor_shared_mutex(actor_shared_mutex&& s);
	actor_shared_mutex();
	~actor_shared_mutex();
public:
	void operator=(const actor_shared_mutex& s);
	void operator=(actor_shared_mutex&& s);
public:
	/*!
	@brief ��ռ��
	*/
	void lock(my_actor* host) const;
	bool try_lock(my_actor* host) const;
	bool timed_lock(int tm, my_actor* host) const;

	/*!
	@brief ������
	*/
	void lock_shared(my_actor* host) const;
	bool try_lock_shared(my_actor* host) const;
	bool timed_lock_shared(int tm, my_actor* host) const;

	/*!
	@brief ����������Ϊ��ռ��
	*/
	void lock_upgrade(my_actor* host) const;
	bool try_lock_upgrade(my_actor* host) const;
	bool timed_lock_upgrade(int tm, my_actor* host) const;

	/*!
	@brief �����ռ����
	*/
	void unlock(my_actor* host) const;

	/*!
	@brief �����������
	*/
	void unlock_shared(my_actor* host) const;

	/*!
	@brief �����ռ�������ָ�Ϊ��������
	*/
	void unlock_upgrade(my_actor* host) const;
private:
	std::shared_ptr<ActorSharedMutex> _amutex;
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��һ����Χ������shared_mutex˽������ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_unique_lock
{
	friend ActorConditionVariable_;
public:
	actor_unique_lock(const actor_shared_mutex& amutex, my_actor* host);
	~actor_unique_lock();
private:
	actor_unique_lock(const actor_unique_lock&);
	void operator=(const actor_unique_lock&);
public:
	void unlock();
	void lock();
private:
	actor_shared_mutex _amutex;
	my_actor* _host;
	bool _isUnlock;
};

/*!
@brief ��һ����Χ������shared_mutex��������ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_shared_lock
{
	friend ActorConditionVariable_;
public:
	actor_shared_lock(const actor_shared_mutex& amutex, my_actor* host);
	~actor_shared_lock();
private:
	actor_shared_lock(const actor_shared_lock&);
	void operator=(const actor_shared_lock&);
public:
	void unlock_shared();
	void lock_shared();
private:
	actor_shared_mutex _amutex;
	my_actor* _host;
	bool _isUnlock;
};
#endif
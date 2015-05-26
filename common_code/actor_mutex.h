#ifndef __ACTOR_MUTEX_H
#define __ACTOR_MUTEX_H

#include "shared_strand.h"

class _actor_mutex;
class _actor_condition_variable;
class _actor_shared_mutex;
class my_actor;

/*!
@brief Actor�����ɵݹ�
*/
class actor_mutex
{
	friend my_actor;
public:
	/*!
	@brief mutex���رպ� lock �����׳����쳣
	*/
	struct close_exception {};
public:
	actor_mutex(shared_strand strand);
	~actor_mutex();
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

	/*!
	@brief �ر�mutex�����еȴ� lock ���׳�close_exception
	*/
	void close(my_actor* host) const;

	/*!
	@brief close�����ã�ȷ��û���κ�Actorռ�ú��ٵ���
	*/
	void reset() const;
private:
	void quited_lock(my_actor* host) const;
	void quited_unlock(my_actor* host) const;
private:
	std::shared_ptr<_actor_mutex> _amutex;
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��һ����Χ������mutex��ͬʱ���е�ActorҲ�ᱻ����ǿ���˳�
*/
class actor_lock_guard
{
	friend _actor_condition_variable;
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
	struct close_exception 
	{
	};
public:
	actor_condition_variable(shared_strand strand);
	~actor_condition_variable();
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

	/*!
	@brief �رն�������wait�Ľ��׳�close_exception�쳣
	*/
	void close(my_actor* host) const;

	/*!
	@brief close�����ã�ȷ��û���κ�Actorռ�ú��ٵ���
	*/
	void reset() const;
private:
	std::shared_ptr<_actor_condition_variable> _aconVar;
};
//////////////////////////////////////////////////////////////////////////

/*!
@brief ��Actor�����еĶ�д�������ɵݹ�
*/
class actor_shared_mutex
{
public:
	struct close_exception {};
public:
	actor_shared_mutex(shared_strand strand);
	~actor_shared_mutex();
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
	bool try_lock_shared(my_actor* host);
	bool timed_lock_shared(int tm, my_actor* host) const;

	/*!
	@brief ����������Ϊ��ռ���������ǰ��������Actor������ȴ�����Actor�����ռ�ã�
	������Actor�ڹ�������ͬʱ���ã�����������(��Actor�½�����try_lock_upgrade��timed_lock_upgrade)
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

	/*!
	@brief �رն�������wait�Ľ��׳�close_exception�쳣
	*/
	void close(my_actor* host) const;

	/*!
	@brief close�����ã�ȷ��û���κ�Actorռ�ú��ٵ���
	*/
	void reset() const;
private:
	std::shared_ptr<_actor_shared_mutex> _amutex;
};
#endif
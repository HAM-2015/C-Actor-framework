#ifndef __ACTOR_MUTEX_H
#define __ACTOR_MUTEX_H

#include "shared_strand.h"

class _actor_mutex;
class my_actor;

/*!
@brief Actor�����ɵݹ�
*/
class actor_mutex
{
	friend my_actor;
public:
	actor_mutex(shared_strand strand);
	~actor_mutex();
public:
	/*!
	@brief ������Դ����������Actor���У��ȴ���ֱ�������ȣ��� unlock ������У��ɵݹ���ã�lock�ڼ�Actor������ǿ���˳�
	@warning ����Actor�����ǿ���˳����п������ lock ���޷� unlock
	*/
	void lock(my_actor* self) const;

	/*!
	@brief ���Ե�ǰ�Ƿ��Ѿ���ռ�ã�����Ѿ�����ĳ��У����Լ����в��㣩�ͷ��������û���У����Լ����У��� unlock �������
	@return �Ѿ������Actor���� false���ɹ����з���true
	*/
	bool try_lock(my_actor* self) const;

	/*!
	@brief ��һ��ʱ���ڳ���������Դ
	@return �ɹ�����true����ʱʧ�ܷ���false
	*/
	bool timed_lock(int tm, my_actor* self) const;

	/*!
	@brief �����ǰActor������У��ݹ� lock ���Σ�����Ҫ unlock ����
	*/
	void unlock(my_actor* self) const;
	void unlock() const;

	/*!
	@brief ����mutex��ʹ��ǰȷ����ǰû��һ��Actor������mutex��
	*/
	//void reset_mutex(my_actor* self);
private:
	void quited_lock(my_actor* self) const;
	void quited_unlock(my_actor* self) const;
private:
	std::shared_ptr<_actor_mutex> _amutex;
};
//////////////////////////////////////////////////////////////////////////

class actor_lock_guard
{
public:
	actor_lock_guard(const actor_mutex& amutex, my_actor* self);
	~actor_lock_guard();
private:
	actor_lock_guard(const actor_lock_guard&);
	void operator=(const actor_lock_guard&);
public:
	void unlock();
	void lock();
private:
	actor_mutex _amutex;
	my_actor* _self;
	bool _isUnlock;
};

#endif
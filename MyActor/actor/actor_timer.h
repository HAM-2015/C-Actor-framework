#ifndef __ACTOR_TIMER_H
#define __ACTOR_TIMER_H

#include <algorithm>
#include "shared_strand.h"
#include "msg_queue.h"

class boost_strand;
class mfc_strand;
class wx_strand;
class qt_strand;
class my_actor;

/*!
@brief Actor �ڲ�ʹ�õĶ�ʱ��
*/
class ActorTimer_
{
	typedef std::shared_ptr<my_actor> actor_handle;
	typedef msg_multimap<unsigned long long, actor_handle> handler_table;

	friend boost_strand;
	friend mfc_strand;
	friend wx_strand;
	friend qt_strand;
	friend my_actor;

	class timer_handle 
	{
		friend ActorTimer_;
	public:
		void reset()
		{
			_null = true;
		}
	private:
		bool _null : true;
		handler_table::iterator _tableNode;
	};
private:
	ActorTimer_(const shared_strand& strand);
	~ActorTimer_();
private:
	/*!
	@brief ��ʼ��ʱ
	@param us ΢��
	@param host ׼����ʱ��Actor
	@return ��ʱ���������cancel
	*/
	timer_handle timeout(unsigned long long us, const actor_handle& host);

	/*!
	@brief ȡ����ʱ
	*/
	void cancel(timer_handle& th);

	/*!
	@brief timerѭ��
	*/
	void timer_loop(unsigned long long us);
private:
	io_engine& _ios;
	void* _timer;
	bool _looping;
	int _timerCount;
	shared_strand _strand;
	handler_table _handlerTable;
	unsigned long long _extMaxTick;
	unsigned long long _extFinishTime;
	std::weak_ptr<boost_strand> _weakStrand;
};

#endif
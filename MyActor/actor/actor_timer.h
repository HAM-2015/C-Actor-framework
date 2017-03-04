#ifndef __ACTOR_TIMER_H
#define __ACTOR_TIMER_H

#include <algorithm>
#include "run_strand.h"
#include "msg_queue.h"
#include "stack_object.h"

class boost_strand;
class qt_strand;
class uv_strand;
class my_actor;
class generator;
class AsyncTimer_;

/*!
@brief Actor �ڲ�ʹ�õĶ�ʱ��
*/
class ActorTimer_
#ifdef DISABLE_BOOST_TIMER
	: public TimerBoostCompletedEventFace_
#endif
{
	typedef std::shared_ptr<ActorTimerFace_> actor_face_handle;
	typedef msg_multimap<long long, actor_face_handle> handler_queue;

	friend boost_strand;
	friend qt_strand;
	friend uv_strand;
	friend my_actor;
	friend generator;
	friend AsyncTimer_;

	class timer_handle 
	{
		friend ActorTimer_;
	public:
		bool is_null() const
		{
			return 0 == _beginStamp;
		}

		void reset()
		{
			_beginStamp = 0;
		}
		long long _beginStamp = 0;
	private:
		handler_queue::iterator _queueNode;
	};
private:
	ActorTimer_(const shared_strand& strand);
	~ActorTimer_();
private:
	/*!
	@brief ��ʼ��ʱ
	@param us ΢��
	@param host ׼����ʱ��Actor
	@param deadline �Ƿ�Ϊ����ʱ��
	@return ��ʱ���������cancel
	*/
	timer_handle timeout(long long us, actor_face_handle&& host, bool deadline = false);

	/*!
	@brief ȡ����ʱ
	*/
	void cancel(timer_handle& th);

	/*!
	@brief timerѭ��
	*/
	void timer_loop(long long abs, long long rel);

	/*!
	@brief timer�¼�
	*/
	void event_handler(int tc);
#ifdef DISABLE_BOOST_TIMER
	void post_event(int tc);
	void cancel_event();
#endif
private:
	void* _timer;
	std::weak_ptr<boost_strand>& _weakStrand;
	shared_strand _lockStrand;
	handler_queue _handlerQueue;
	long long _extMaxTick;
	long long _extFinishTime;
#ifdef DISABLE_BOOST_TIMER
	stack_obj<io_work, false> _lockIos;
#endif
	int _timerCount;
	bool _looping;
	NONE_COPY(ActorTimer_);
};

#endif
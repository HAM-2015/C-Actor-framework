#ifndef __ASYNC_TIMER_H
#define __ASYNC_TIMER_H

#include <algorithm>
#include "shared_strand.h"
#include "msg_queue.h"
#include "mem_pool.h"
#include "stack_object.h"

class AsyncTimer_;
class qt_strand;
class boost_strand;
typedef std::shared_ptr<AsyncTimer_> async_timer;

/*!
@brief ��ʱ�����ܼ�����
*/
class TimerBoost_
#ifdef DISABLE_BOOST_TIMER
	: public TimerBoostCompletedEventFace_
#endif
{
	typedef msg_multimap<unsigned long long, async_timer> handler_queue;

	friend AsyncTimer_;
	friend qt_strand;
	friend boost_strand;

	class timer_handle
	{
		friend TimerBoost_;
	public:
		void reset()
		{
			_null = true;
			_beginStamp = 0;
		}
		long long _beginStamp = 0;
	private:
		handler_queue::iterator _queueNode;
		bool _null = true;
	};
private:
	TimerBoost_(const shared_strand& strand);
	~TimerBoost_(); 
private:
	/*!
	@brief ��ʼ��ʱ
	@param us ΢��
	@param host ׼����ʱ��AsyncTimer_
	@return ��ʱ���������cancel
	*/
	timer_handle timeout(unsigned long long us, async_timer&& host);

	/*!
	@brief ȡ����ʱ
	*/
	void cancel(timer_handle& th);

	/*!
	@brief timerѭ��
	*/
	void timer_loop(unsigned long long us);

	/*!
	@brief timer�¼�
	*/
	void event_handler(int tc);
#ifdef DISABLE_BOOST_TIMER
	void post_event(int tc);
#endif
private:
	void* _timer;
	std::weak_ptr<boost_strand>& _weakStrand;
	shared_strand _lockStrand;
	handler_queue _handlerQueue;
	unsigned long long _extMaxTick;
	unsigned long long _extFinishTime;
#ifdef DISABLE_BOOST_TIMER
	stack_obj<boost::asio::io_service::work, false> _lockIos;
#endif
	int _timerCount;
	bool _looping;
	NONE_COPY(TimerBoost_);
};

/*!
@brief �첽��ʱ����������TimerBoost_��һ����ʱѭ��һ��AsyncTimer_
*/
class AsyncTimer_
{
	friend TimerBoost_;
	friend boost_strand;
	FRIEND_SHARED_PTR(AsyncTimer_);

	struct wrap_base
	{
		virtual void invoke() = 0;
		virtual void destroy() = 0;
	};

	template <typename Handler>
	struct wrap_handler : public wrap_base
	{
		template <typename H>
		wrap_handler(H&& h)
			:_h(TRY_MOVE(h)) {}

		void invoke()
		{
			_h();
			destroy();
		}

		void destroy()
		{
			this->~wrap_handler();
		}

		Handler _h;
	};
private:
	AsyncTimer_(TimerBoost_& timerBoost);
	~AsyncTimer_();
public:
	/*!
	@brief ����һ����ʱѭ������������strand�߳��е���
	*/
	template <typename Handler>
	long long timeout(int tm, Handler&& handler)
	{
		assert(self_strand()->running_in_this_thread());
		assert(!_handler);
		typedef wrap_handler<RM_REF(Handler)> wrap_type;
		_handler = new(_reuMem.allocate(sizeof(wrap_type)))wrap_type(TRY_MOVE(handler));
		_timerHandle = _timerBoost.timeout(tm * 1000, _weakThis.lock());
		return _timerHandle._beginStamp;
	}

	/*!
	@brief ȡ�����μ�ʱ����������strand�߳��е���
	*/
	void cancel();

	/*!
	@brief 
	*/
	shared_strand self_strand();
private:
	void timeout_handler();
private:
	wrap_base* _handler;
	reusable_mem _reuMem;
	TimerBoost_& _timerBoost;
	std::weak_ptr<AsyncTimer_> _weakThis;
	TimerBoost_::timer_handle _timerHandle;
	NONE_COPY(AsyncTimer_);
};

#endif
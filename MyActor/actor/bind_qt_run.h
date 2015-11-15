#ifndef __BIND_QT_RUN_H
#define __BIND_QT_RUN_H

#ifdef ENABLE_QT_ACTOR
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <functional>
#include <mutex>
#include <QtGui/qevent.h>
#include "wrapped_post_handler.h"
#include "actor_framework.h"
#include "qt_strand.h"
#include "msg_queue.h"

#define QT_POST_TASK	(QEvent::MaxUser-1)

//��ʼ��Actor�У�Ƕ��һ����qt�߳���ִ�е������߼�
#define begin_RUN_IN_QT_FOR(__qt__, __host__) (__qt__)->send(__host__, [&]() {
#define begin_RUN_IN_QT() begin_RUN_IN_QT_FOR(this, self)
//������qt�߳���ִ�е�һ�������߼���ֻ�е�����߼�ִ����Ϻ�Ż�ִ��END��������
#define end_RUN_IN_QT() })
//////////////////////////////////////////////////////////////////////////
//��Actor�У�Ƕ��һ����qt�߳���ִ�е����
#define RUN_IN_QT_FOR(__qt__, __host__, __exp__) (__qt__)->send(__host__, [&]() {__exp__;})
#define RUN_IN_QT(__exp__) RUN_IN_QT_FOR(this, self, __exp__)
//////////////////////////////////////////////////////////////////////////
//��Actor�У�Ƕ��һ����qt�߳���ִ�е�Actor�߼��������߼��а����첽����ʱʹ�ã���������begin_RUN_IN_QT_FOR��
#define begin_ACTOR_RUN_IN_QT_FOR(__qt__, __host__, __ios__) {\
	auto ___host = __host__; \
	my_actor::quit_guard ___qg(__host__); \
	auto ___tactor = (__qt__)->create_qt_actor(__ios__, [&](my_actor* __host__) {

//��Actor�У�Ƕ��һ����qt�߳���ִ�е�Actor�߼��������߼��а����첽����ʱʹ�ã���������begin_RUN_IN_QT��
#define begin_ACTOR_RUN_IN_QT(__ios__) begin_ACTOR_RUN_IN_QT_FOR(this, self, __ios__)

//������qt�߳���ִ�е�Actor��ֻ�е�Actor���߼�ִ����Ϻ�Ż�ִ��END��������
#define end_ACTOR_RUN_IN_QT()\
	}); \
	___tactor->notify_run(); \
	___host->actor_wait_quit(___tactor); \
}

struct qt_ui_closed_exception {};

class bind_qt_run_base
{
protected:
	bind_qt_run_base();
	virtual ~bind_qt_run_base();
public:
	shared_qt_strand make_qt_strand();
	shared_qt_strand make_qt_strand(io_engine& ios);

	/*!
	@brief ��ȡ���߳�ID
	*/
	boost::thread::id thread_id();

	/*!
	@brief ������гس���
	*/
	void post_queue_size(size_t fixedSize);

	/*!
	@brief ����һ��ִ�к�����UI��Ϣ������ִ��
	*/
	void post(const std::function<void()>& h);
	void post(std::function<void()>&& h);

	/*!
	@brief ����һ��ִ�к�����UI��Ϣ������ִ�У���ɺ󷵻�
	*/
	template <typename Handler>
	void send(my_actor* host, Handler&& h)
	{
		host->lock_quit();
		bool closed = false;
		host->trig([&](const trig_once_notifer<>& cb)
		{
			boost::shared_lock<boost::shared_mutex> sl(_postMutex);
			if (!_isClosed)
			{
				_mutex.lock();
				_tasksQueue.push_back([&h, cb]()
				{
					h();
					cb();
				});
				_mutex.unlock();
				sl.unlock();
				postTaskEvent();
			}
			else
			{
				sl.unlock();
				closed = true;
				cb();
			}
		});
		host->unlock_quit();
		if (closed)
		{
			throw qt_ui_closed_exception();
		}
	}

	/*!
	@brief ��һ��������UI����ִ��
	*/
	template <typename Handler>
	wrapped_post_handler<bind_qt_run_base, Handler> wrap(Handler&& handler)
	{
		return wrapped_post_handler<bind_qt_run_base, RM_CREF(Handler)>(this, TRY_MOVE(handler));
	}

	/*!
	@brief ��UI�߳��д���һ��Actor
	@param ios Actor�ڲ�timerʹ�õĵ�������û�оͲ�����timer
	*/
	actor_handle create_qt_actor(io_engine& ios, const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);
	actor_handle create_qt_actor(const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);
protected:
	virtual void postTaskEvent() = 0;
	virtual void runOneTask();

	virtual void userEvent(QEvent* e);
	void notifyUiClosed();
private:
	msg_queue<std::function<void()> > _tasksQueue;
	boost::shared_mutex _postMutex;
	boost::thread::id _threadID;
	std::mutex _mutex;
protected:
	bool _isClosed;
};

template <typename FRAME>
class bind_qt_run : public FRAME, public bind_qt_run_base
{
	struct task_event : public QEvent
	{
		template <typename... Args>
		task_event(Args&&... args)
			:QEvent(TRY_MOVE(args)...) {}
	};
protected:
	template <typename... Args>
	bind_qt_run(Args&&... args)
		: FRAME(TRY_MOVE(args)...)
	{
	}

	virtual ~bind_qt_run()
	{

	}
private:
	void postTaskEvent() override final
	{
		QCoreApplication::postEvent(this, new task_event(QEvent::Type(QT_POST_TASK)));
	}

	void runOneTask() override final
	{

	}

	void customEvent(QEvent* e) override final
	{
		if (e->type() == QEvent::Type(QT_POST_TASK))
		{
			if (!_isClosed)
			{
				assert(dynamic_cast<task_event*>(e));
				bind_qt_run_base::runOneTask();
			}
		}
		else
		{
			userEvent(e);
		}
	}
};
#endif

#endif
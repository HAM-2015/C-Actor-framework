#ifndef __BIND_MODAL_RUN_H
#define __BIND_MODAL_RUN_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <functional>
#include <memory>
#include "wrapped_post_handler.h"
#include "actor_framework.h"
#include "mfc_strand.h"
#include "msg_queue.h"

using namespace std;

#define 	WM_USER_BEGIN		(WM_USER+0x8000)
#define 	WM_USER_POST		(WM_USER+0x8001)
#define		WM_USER_END			(WM_USER+0x8002)

#define		BIND_MFC_RUN(__base__) \
private:\
void post_message(int id)\
{\
	PostMessage(id);\
}\
\
void peek_message()\
{\
	MSG msg;\
	while (PeekMessage(&msg, m_hWnd, WM_USER_BEGIN, WM_USER_END, PM_REMOVE))\
	{}\
}\
void cancel()\
{\
	__base__::OnCancel();\
}\
\
LRESULT _postRun(WPARAM wp, LPARAM lp)\
{\
	return bind_mfc_run::_postRun(wp, lp);\
}\
\
afx_msg void OnCancel();

#define REGIEST_MFC_RUN(__dlg__) \
ON_WM_CLOSE()\
ON_MESSAGE(WM_USER_POST, &__dlg__::_postRun)

class bind_mfc_run
{
protected:
	bind_mfc_run(): _isClosed(false), _postOptions(16) {};
public:
	virtual ~bind_mfc_run() {};
public:
	/*!
	@brief ��һ��������MFC����ִ��
	*/
	template <typename Handler>
	wrapped_post_handler<bind_mfc_run, Handler> wrap(const Handler& handler)
	{
		return wrapped_post_handler<bind_mfc_run, Handler>(this, handler);
	}
protected:
	virtual void post_message(int id) = 0;
	virtual void peek_message() = 0;
	virtual void cancel() = 0;

	void clear_message()
	{
		boost::unique_lock<boost::shared_mutex> ul(_postMutex);
		peek_message();
		_postOptions.clear();
	}

	/*!
	@brief �رջ��࣬������ֹ��Ϣ��Ͷ��
	*/
	void mfc_close()
	{
		_isClosed = true;
		clear_message();
		cancel();
	}
public:
	/*!
	@brief ��MFC���̳߳�ʼ��ʱ����
	*/
	void set_thread_id()
	{
		_threadID = boost::this_thread::get_id();
	}

	/*!
	@brief ��ȡ���߳�ID
	*/
	boost::thread::id thread_id()
	{
		assert(boost::thread::id() != _threadID);
		return _threadID;
	}

	/*!
	@brief ����һ��ִ�к�����MFC��Ϣ������ִ��
	*/
	template <typename H>
	void post(const H& h)
	{
		boost::shared_lock<boost::shared_mutex> sl(_postMutex);
		if (!_isClosed)
		{
			_mutex.lock();
			_postOptions.push_back(h);
			_mutex.unlock();
			post_message(WM_USER_POST);
		}
	}
	
	/*!
	@brief ����һ��ִ�к�����MFC��Ϣ������ִ�У���ɺ󷵻�
	*/
	template <typename H>
	void send(my_actor* self, const H& h)
	{
		assert(boost::this_thread::get_id() != thread_id());
		my_actor::quit_guard qg(self);
		self->trig([&, this](const trig_once_notifer<>& cb)
		{
			boost::shared_lock<boost::shared_mutex> sl(_postMutex);
			if (!_isClosed)
			{
				auto& h_ = h;
				_mutex.lock();
				_postOptions.push_back([&h_, cb]()
				{
					h_();
					cb();
				});
				_mutex.unlock();
				this->post_message(WM_USER_POST);
			}
		});
	}

	/*!
	@brief ����һ��������ֵ������MFC��Ϣ������ִ�У���ɺ󷵻�
	*/
	template <typename T, typename H>
	T send(my_actor* self, const H& h)
	{
		assert(boost::this_thread::get_id() != thread_id());
		my_actor::quit_guard qg(self);
		return self->trig<T>([&, this](const trig_once_notifer<T>& cb)
		{
			boost::shared_lock<boost::shared_mutex> sl(_postMutex);
			if (!_isClosed)
			{
				auto& h_ = h;
				_mutex.lock();
				_postOptions.push_back([&h_, cb]()
				{
					cb(h_());
				});
				_mutex.unlock();
				this->post_message(WM_USER_POST);
			}
		});
	}

	void post_queue_size(size_t fixedSize)
	{
		_postOptions.expand_fixed(fixedSize);
	}
#ifdef ENABLE_MFC_ACTOR
	/*!
	@brief ��MFC�߳��д���һ��Actor
	@param ios Actor�ڲ�timerʹ�õĵ�������û�оͲ�����timer
	*/
	actor_handle create_mfc_actor(ios_proxy& ios, const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return my_actor::create(mfc_strand::create(ios, this), mainFunc, stackSize);
	}

	actor_handle create_mfc_actor(const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE)
	{
		return my_actor::create(mfc_strand::create(this), mainFunc, stackSize);
	}
#endif
protected:
	LRESULT _postRun(WPARAM wp, LPARAM lp)
	{
		_mutex.lock();
		assert(!_postOptions.empty());
		auto h = _postOptions.front();
		_postOptions.pop_front();
		_mutex.unlock();
		assert(h);
		h();
		return 0;
	}
private:
	msg_queue<std::function<void ()> > _postOptions;
	boost::mutex _mutex;
	boost::shared_mutex _postMutex;
	boost::thread::id _threadID;
	bool _isClosed;
};

#endif
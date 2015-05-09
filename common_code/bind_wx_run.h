#ifndef __BIND_WX_RUN_H
#define __BIND_WX_RUN_H

#include "wrapped_post_handler.h"
#include "actor_framework.h"
#include "wx_strand.h"
#include "msg_queue.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <functional>
#include <memory>
#include <WinSock2.h>
#include <wx/event.h>

using namespace std;

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#ifdef _DEBUG
#pragma comment(lib, "wxbase30ud.lib")
#pragma comment(lib, "wxbase30ud_net.lib")
#pragma comment(lib, "wxbase30ud_xml.lib")
#pragma comment(lib, "wxexpatd.lib")
#pragma comment(lib, "wxjpegd.lib")
#pragma comment(lib, "wxmsw30ud_adv.lib")
#pragma comment(lib, "wxmsw30ud_aui.lib")
#pragma comment(lib, "wxmsw30ud_core.lib")
#pragma comment(lib, "wxmsw30ud_gl.lib")
#pragma comment(lib, "wxmsw30ud_html.lib")
#pragma comment(lib, "wxmsw30ud_media.lib")
#pragma comment(lib, "wxmsw30ud_propgrid.lib")
#pragma comment(lib, "wxmsw30ud_qa.lib")
#pragma comment(lib, "wxmsw30ud_ribbon.lib")
#pragma comment(lib, "wxmsw30ud_richtext.lib")
#pragma comment(lib, "wxmsw30ud_stc.lib")
#pragma comment(lib, "wxmsw30ud_webview.lib")
#pragma comment(lib, "wxmsw30ud_xrc.lib")
#pragma comment(lib, "wxpngd.lib")
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxscintillad.lib")
#pragma comment(lib, "wxtiffd.lib")
#pragma comment(lib, "wxzlibd.lib")
#else
#pragma comment(lib, "wxbase30u.lib")
#pragma comment(lib, "wxbase30u_net.lib")
#pragma comment(lib, "wxbase30u_xml.lib")
#pragma comment(lib, "wxexpat.lib")
#pragma comment(lib, "wxjpeg.lib")
#pragma comment(lib, "wxmsw30u_adv.lib")
#pragma comment(lib, "wxmsw30u_aui.lib")
#pragma comment(lib, "wxmsw30u_core.lib")
#pragma comment(lib, "wxmsw30u_gl.lib")
#pragma comment(lib, "wxmsw30u_html.lib")
#pragma comment(lib, "wxmsw30u_media.lib")
#pragma comment(lib, "wxmsw30u_propgrid.lib")
#pragma comment(lib, "wxmsw30u_qa.lib")
#pragma comment(lib, "wxmsw30u_ribbon.lib")
#pragma comment(lib, "wxmsw30u_richtext.lib")
#pragma comment(lib, "wxmsw30u_stc.lib")
#pragma comment(lib, "wxmsw30u_webview.lib")
#pragma comment(lib, "wxmsw30u_xrc.lib")
#pragma comment(lib, "wxpng.lib")
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxscintilla.lib")
#pragma comment(lib, "wxtiff.lib")
#pragma comment(lib, "wxzlib.lib")
#endif

#define 	WX_USER_BEGIN		(0x8000)
#define 	WX_USER_POST		(0x8001)
#define		WX_USER_END			(0x8002)

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxEVT_POST, WX_USER_POST)
END_DECLARE_EVENT_TYPES()

#define		BIND_WX_RUN(__frame__, __base__) \
private:\
	void __postMessage()\
{\
	wxPostEvent(this, wxCommandEvent(wxEVT_POST)); \
}\
	void __cancel()\
{\
	__base__::Close(); \
}\
	void __closeEventHandler(wxCloseEvent& event)\
{\
	bind_wx_run::__closeEventHandler(event); \
}\
	\
	void __connectCloseEvent()\
{\
	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(__frame__::__closeEventHandler)); \
}\
	\
	void __disconnectCloseEvent()\
{\
	Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(__frame__::__closeEventHandler)); \
}\
	\
	void __bindPostEvent()\
{\
	Bind(wxEVT_POST, &__frame__::__postRun, (bind_wx_run*)this); \
}\
	\
	void __unbindPostEvent()\
{\
	Unbind(wxEVT_POST, &__frame__::__postRun, (bind_wx_run*)this); \
}
//////////////////////////////////////////////////////////////////////////
#define BEGIN_RUN_IN_WX_FOR(__self__) send(__self__, [&, this]() {
#define BEGIN_RUN_IN_WX() BEGIN_RUN_IN_WX_FOR(self)
#define END_RUN_IN_WX() })
//////////////////////////////////////////////////////////////////////////
#define RUN_IN_WX_FOR(__self__, __exp__) send(__self__, [&, this]() {__exp__;})
#define RUN_IN_WX(__exp__) RUN_IN_WX_FOR(self, __exp__)
//////////////////////////////////////////////////////////////////////////
#define BEGIN_ACTOR_RUN_IN_WX_FOR(__self__, __ios__) {\
	my_actor::quit_guard ___qg(__self__); \
	auto ___tactor = create_wx_actor(__ios__, [&, this](my_actor* __self__) {

#define END_ACTOR_RUN_IN_WX_FOR(__self__)\
}); \
	___tactor->notify_run(); \
	__self__->actor_wait_quit(___tactor); \
}

#define BEGIN_ACTOR_RUN_IN_WX(__ios__) BEGIN_ACTOR_RUN_IN_WX_FOR(self, __ios__)
#define END_ACTOR_RUN_IN_WX() END_ACTOR_RUN_IN_WX_FOR(self)

/*!
@brief ��Ϊwx�������ʹ�ã���ʹwx����֧��Actor������������������������������ڱ�wx���������߳���ִ��
*/
class bind_wx_run
{
protected:
	bind_wx_run();;
public:
	virtual ~bind_wx_run();;
public:
	/*!
	@brief ��һ��������UI����ִ��
	*/
	template <typename Handler>
	wrapped_post_handler<bind_wx_run, Handler> wrap(const Handler& handler)
	{
		return wrapped_post_handler<bind_wx_run, Handler>(this, handler);
	}
protected:
	void __closeEventHandler(wxCloseEvent& event);
	void __postRun(wxEvent& ue);
	virtual void __cancel() = 0;
	virtual void __connectCloseEvent() = 0;
	virtual void __disconnectCloseEvent() = 0;
	virtual void __bindPostEvent() = 0;
	virtual void __unbindPostEvent() = 0;
	virtual void __postMessage() = 0;
protected:
	/*!
	@brief ��wx������ʱ���ȵ��ã�Ȼ���������������Actor
	*/
	void start_wx_actor();

	/*!
	@brief ��wx�߳��е��ã����رոö���֮�󽫲��ܽ����κ�UI����
	*/
	void wx_close();

	/*!
	@brief �̳иú��������յ�wx����ر���Ϣ
	*/
	virtual void OnClose();
public:
	/*!
	@brief ��UI���̳߳�ʼ��ʱ����
	*/
	void set_thread_id();

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
	template <typename H>
	void post(const H& h)
	{
		boost::shared_lock<boost::shared_mutex> sl(_postMutex);
		if (!_isClosed)
		{
			_mutex.lock();
			_postOptions.push_back(h);
			_mutex.unlock();
			__postMessage();
		}
	}

	/*!
	@brief ����һ��ִ�к�����UI��Ϣ������ִ�У���ɺ󷵻�
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
				this->__postMessage();
			}
		});
	}

	/*!
	@brief ����һ��������ֵ������UI��Ϣ������ִ�У���ɺ󷵻�
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
				this->__postMessage();
			}
		});
	}
#ifdef ENABLE_WX_ACTOR
	/*!
	@brief ��UI�߳��д���һ��Actor
	@param ios Actor�ڲ�timerʹ�õĵ�������û�оͲ�����timer
	*/
	actor_handle create_wx_actor(ios_proxy& ios, const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);

	actor_handle create_wx_actor(const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE);
#endif
private:
	msg_queue<std::function<void()> > _postOptions;
	boost::mutex _mutex;
	boost::shared_mutex _postMutex;
	boost::thread::id _threadID;
	bool _isClosed;
};

#endif
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

//////////////////////////////////////////////////////////////////////////
//��ʼ��Actor�У�Ƕ��һ����wx�߳���ִ�е������߼�
#define begin_RUN_IN_WX_FOR(__self__) send(__self__, [&, this]() {
#define begin_RUN_IN_WX() begin_RUN_IN_WX_FOR(self)
//������wx�߳���ִ�е�һ�������߼���ֻ�е�����߼�ִ����Ϻ�Ż�ִ��END��������
#define end_RUN_IN_WX() })
//////////////////////////////////////////////////////////////////////////
//��Actor�У�Ƕ��һ����wx�߳���ִ�е����
#define RUN_IN_WX_FOR(__self__, __exp__) send(__self__, [&, this]() {__exp__;})
//��Actor�У�Ƕ��һ����wx�߳���ִ�е����
#define RUN_IN_WX(__exp__) RUN_IN_WX_FOR(self, __exp__)
//////////////////////////////////////////////////////////////////////////
//��Actor�У�Ƕ��һ����wx�߳���ִ�е�Actor�߼��������߼��а����첽����ʱʹ�ã���������begin_RUN_IN_WX_FOR��
#define begin_ACTOR_RUN_IN_WX_FOR(__self__, __ios__) {\
	my_actor::quit_guard ___qg(__self__); \
	auto ___tactor = create_wx_actor(__ios__, [&, this](my_actor* __self__) {

//������wx�߳���ִ�е�Actor��ֻ�е�Actor���߼�ִ����Ϻ�Ż�ִ��END��������
#define end_ACTOR_RUN_IN_WX_FOR(__self__)\
}); \
	___tactor->notify_run(); \
	__self__->actor_wait_quit(___tactor); \
}

//��Actor�У�Ƕ��һ����wx�߳���ִ�е�Actor�߼��������߼��а����첽����ʱʹ�ã���������begin_RUN_IN_WX��
#define begin_ACTOR_RUN_IN_WX(__ios__) begin_ACTOR_RUN_IN_WX_FOR(self, __ios__)
//������wx�߳���ִ�е�Actor��ֻ�е�Actor���߼�ִ����Ϻ�Ż�ִ��END��������
#define end_ACTOR_RUN_IN_WX() end_ACTOR_RUN_IN_WX_FOR(self)

//����һ��ģ̬�Ի��򣨵�ǰ�������Disable����Enable��������
#define SHOW_MODAL(__dlg__)\
{\
	bool __isEnabled = IsEnabled();\
	if (!__isEnabled)\
	{\
		Enable(); \
	}\
	__dlg__; \
	if (!__isEnabled)\
	{\
		Disable();\
	}\
}

//////////////////////////////////////////////////////////////////////////
class bind_wx_run_base
{
public:
	virtual boost::thread::id thread_id() = 0;
	virtual void post(const std::function<void()>& h) = 0;
	shared_strand make_wx_strand();
	shared_strand make_wx_strand(ios_proxy& ios);
};

/*!
@brief ��Ϊwx�������ʹ�ã���ʹwx����֧��Actor������������������������������ڱ�wx���������߳���ִ��
*/
template <typename FRAME>
class bind_wx_run: public FRAME, public bind_wx_run_base
{
protected:
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
	bind_wx_run(const T0& p0, const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5)
		: FRAME((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3, (T4&)p4, (T5&)p5), _isClosed(false), _postOptions(16)
	{
		init();
	}

	template <typename T0, typename T1, typename T2, typename T3, typename T4>
	bind_wx_run(const T0& p0, const T1& p1, const T2& p2, const T3& p3, const T4& p4)
		: FRAME((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3, (T4&)p4), _isClosed(false), _postOptions(16)
	{
		init();
	}

	template <typename T0, typename T1, typename T2, typename T3>
	bind_wx_run(const T0& p0, const T1& p1, const T2& p2, const T3& p3)
		: FRAME((T0&)p0, (T1&)p1, (T2&)p2, (T3&)p3), _isClosed(false), _postOptions(16)
	{
		init();
	}

	template <typename T0, typename T1, typename T2>
	bind_wx_run(const T0& p0, const T1& p1, const T2& p2)
		: FRAME((T0&)p0, (T1&)p1, (T2&)p2), _isClosed(false), _postOptions(16)
	{
		init();
	}

	template <typename T0, typename T1>
	bind_wx_run(const T0& p0, const T1& p1)
		: FRAME((T0&)p0, (T1&)p1), _isClosed(false), _postOptions(16)
	{
		init();
	}

	template <typename T0>
	bind_wx_run(const T0& p0)
		: FRAME((T0&)p0), _isClosed(false), _postOptions(16)
	{
		init();
	}

	bind_wx_run()
		: _isClosed(false), _postOptions(16)
	{
		init();
	}

public:
	virtual ~bind_wx_run()
	{
		assert(boost::this_thread::get_id() == _threadID);
	}
public:
	/*!
	@brief ��һ��������UI����ִ��
	*/
	template <typename Handler>
	wrapped_post_handler<bind_wx_run, Handler> wrap(const Handler& handler)
	{
		return wrapped_post_handler<bind_wx_run, Handler>(this, handler);
	}
private:
	void init()
	{
		_threadID = boost::this_thread::get_id();
		Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(bind_wx_run::closeEventHandler));
		Bind(wxEVT_POST, &bind_wx_run::postRun, this);
	}

	void closeEventHandler(wxCloseEvent& event)
	{
		OnClose();
	}

	void postRun(wxEvent& ue)
	{
		_mutex.lock();
		assert(!_postOptions.empty());
		auto h = std::move(_postOptions.front());
		_postOptions.pop_front();
		_mutex.unlock();
		assert(h);
		h();
	}
protected:
	/*!
	@brief ��wx�߳��е��ã����رոö���֮��ö��󽫲��ܽ����κβ���(����post�����Բ�Ҫ��wx_actor�ڵ���)
	*/
	void wx_close()
	{
		assert(boost::this_thread::get_id() == _threadID);
		boost::unique_lock<boost::shared_mutex> ul(_postMutex);
		_isClosed = true;
		Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(bind_wx_run::closeEventHandler));
		Unbind(wxEVT_POST, &bind_wx_run::postRun, this);
		Close();
		while (!_closedCallback.empty())
		{
			_closedCallback.front()();
			_closedCallback.pop_front();
		}
	}

	/*!
	@brief �̳иú��������յ�wx����ر���Ϣ
	*/
	virtual void OnClose()
	{

	}
public:
	/*!
	@brief �ȴ�����ر�
	*/
	void wait_closed(my_actor* host)
	{
		boost::unique_lock<boost::shared_mutex> ul(_postMutex);
		if (!_isClosed)
		{
			actor_trig_handle<> ath;
			_closedCallback.push_back(host->make_trig_notifer(ath));
			ul.unlock();
			host->wait_trig(ath);
		}
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
	@brief ���õ�ǰ����Ϊ����
	*/
	void SetFocus()
	{
		assert(boost::this_thread::get_id() == _threadID);
		if (IsEnabled())
		{
			FRAME::SetFocus();
		}
		else
		{
			Enable();
			FRAME::SetFocus();
			Disable();
		}
	}

	/*!
	@brief ������гس���
	*/
	void post_queue_size(size_t fixedSize)
	{
		assert(boost::this_thread::get_id() == _threadID);
		_mutex.lock();
		_postOptions.expand_fixed(fixedSize);
		_mutex.unlock();
	}

	/*!
	@brief ����һ��ִ�к�����UI��Ϣ������ִ��
	*/
	void post(const std::function<void()>& h)
	{
		boost::shared_lock<boost::shared_mutex> sl(_postMutex);
		if (!_isClosed)
		{
			_mutex.lock();
			_postOptions.push_back(h);
			_mutex.unlock();
			wxPostEvent(this, wxCommandEvent(wxEVT_POST));
		}
	}

	/*!
	@brief ����һ��ִ�к�����UI��Ϣ������ִ�У���ɺ󷵻�
	*/
	template <typename H>
	void send(my_actor* self, const H& h)
	{
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
				wxPostEvent(this, wxCommandEvent(wxEVT_POST));
			}
		});
	}

	/*!
	@brief ����һ��������ֵ������UI��Ϣ������ִ�У���ɺ󷵻�
	*/
	template <typename T, typename H>
	T send(my_actor* self, const H& h)
	{
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
				wxPostEvent(this, wxCommandEvent(wxEVT_POST));
			}
		});
	}
#ifdef ENABLE_WX_ACTOR
	/*!
	@brief ��UI�߳��д���һ��Actor
	@param ios Actor�ڲ�timerʹ�õĵ�������û�оͲ�����timer
	*/
	actor_handle create_wx_actor(ios_proxy& ios, const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE)
	{
		assert(!_isClosed);
		return my_actor::create(make_wx_strand(ios), mainFunc, stackSize);
	}

	actor_handle create_wx_actor(const my_actor::main_func& mainFunc, size_t stackSize = DEFAULT_STACKSIZE)
	{
		assert(!_isClosed);
		return my_actor::create(make_wx_strand(), mainFunc, stackSize);
	}
#endif
private:
	msg_queue<std::function<void()> > _postOptions;
	list<actor_trig_notifer<>> _closedCallback;
	boost::mutex _mutex;
	boost::shared_mutex _postMutex;
	boost::thread::id _threadID;
	bool _isClosed;
};

#endif
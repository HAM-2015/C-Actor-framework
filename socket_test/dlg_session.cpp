// dlg_session.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "socket_test.h"
#include "dlg_session.h"
#include "afxdialogex.h"


// dlg_session �Ի���

IMPLEMENT_DYNAMIC(dlg_session, CDialogEx)

dlg_session::dlg_session(CWnd* pParent /*=NULL*/)
	: CDialogEx(dlg_session::IDD, pParent)
{
	_exit = false;
}

dlg_session::~dlg_session()
{
}

void dlg_session::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT5, _msgEdit);
	DDX_Control(pDX, IDC_EDIT4, _outputEdit);
}


BEGIN_MESSAGE_MAP(dlg_session, CDialogEx)
	REGIEST_MFC_RUN(dlg_session)
	ON_BN_CLICKED(IDC_BUTTON3, &dlg_session::OnBnClickedSendMsg)
	ON_BN_CLICKED(IDC_BUTTON1, &dlg_session::OnBnClickedClear)
END_MESSAGE_MAP()

BOOL dlg_session::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	_editFont.CreatePointFont(90, "����");
	_msgEdit.SetFont(&_editFont);
	_outputEdit.SetFont(&_editFont);

	_sendPump = msg_pipe<shared_data>::make(_sendPipe);
	_sessionActor = boost_actor::create(_strand, boost::bind(&dlg_session::sessionActor, this, _1));
	_sessionActor->notify_start_run();
	_lstCloseActor = boost_actor::create(_strand, boost::bind(&dlg_session::lstClose, this, _1));
	_lstCloseActor->notify_start_run();
	return TRUE;
}

// dlg_session ��Ϣ�������

void dlg_session::OnCancel()
{
	if (_lstCloseActor)
	{
		_lstCloseActor->notify_force_quit();
		_lstCloseActor->outside_wait_quit();
		_socket->close();
		_sessionActor->outside_wait_quit();
		_lstCloseActor.reset();
		_socket.reset();
		_sessionActor.reset();
		_isClosed = true;
		CLEAR_MSG();
	}
	CDialogEx::OnCancel();
}

void dlg_session::showClientMsg(shared_data msg)
{
	if (!_exit)
	{
		int nLength = _outputEdit.GetWindowTextLength();
		_outputEdit.SetSel(nLength, nLength);
		_outputEdit.ReplaceSel(msg->c_str());
		_outputEdit.ReplaceSel("\n");
	}
	else
	{
		CLEAR_MSG();
		PostMessage(WM_CLOSE);
	}
}

void dlg_session::lstClose(boost_actor* actor)
{
	actor_msg_handle<> cmh;
	_lstClose(actor, cmh);
	//��������Ի���ر���Ϣ��Ȼ��֪ͨ�Ի���ر�
	actor->pump_msg(cmh);
	_exit = true;
	PostMessage(WM_CLOSE);
	actor->close_msg_notify(cmh);
}

void dlg_session::sessionActor(boost_actor* actor)
{
	post(boost::bind(&dlg_session::showClientMsg, this, msg_data::create(_socket->ip())));
	actor_msg_handle<shared_data> cmh;
	boost::shared_ptr<text_stream_io> textio = text_stream_io::create(_strand, _socket, actor->make_msg_notify(cmh));
	child_actor_handle wd = actor->create_child_actor([this, &textio](boost_actor* actor)
	{
		actor_msg_handle<shared_data> cmh;
		_sendPump(actor, cmh);
		while (true)
		{
			auto msg = actor->pump_msg(cmh);
			if (!msg)
			{
				break;
			}
			textio->write(msg);
		}
		actor->close_msg_notify(cmh);
	});
	actor->child_actor_run(wd);
	while (true)
	{
		auto msg = actor->pump_msg(cmh);
		if (!msg)
		{
			break;
		}
		post(boost::bind(&dlg_session::showClientMsg, this, msg));
	}
	actor->child_actor_quit(wd);
	actor->close_msg_notify(cmh);
	post(boost::bind(&dlg_session::showClientMsg, this, msg_data::create("���ӶϿ�")));
}

void dlg_session::OnBnClickedSendMsg()
{
	CString cs;
	_msgEdit.GetWindowText(cs);
	if (cs.GetLength())
	{
		_msgEdit.SetWindowText("");
		_sendPipe(msg_data::create(cs.GetBuffer()));
	}
	_msgEdit.SetFocus();
}


void dlg_session::OnBnClickedClear()
{
	_outputEdit.SetWindowText("");
}

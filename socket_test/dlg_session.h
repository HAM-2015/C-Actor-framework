#pragma once
#include "afxwin.h"
#include "bind_mfc_run.h"
#include "actor_framework.h"
#include "msg_pipe.h"
#include "socket_io.h"
#include "text_stream_io.h"


// dlg_session �Ի���

class dlg_session : public CDialogEx, bind_mfc_run
{
	DECLARE_DYNAMIC(dlg_session)

public:
	dlg_session(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~dlg_session();

// �Ի�������
	enum { IDD = IDD_SESSION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedSendMsg();
	afx_msg void OnBnClickedClear();
	void showClientMsg(shared_data msg);
	DECLARE_MESSAGE_MAP()
	BIND_MFC_RUN()
private:
	void sessionActor(boost_actor* actor);
	void lstClose(boost_actor* actor);
public:
	shared_strand _strand;
	boost::shared_ptr<socket_io> _socket;
	msg_pipe<>::regist_reader _lstClose;
	boost::function<void ()> _closeNtf;
private:
	bool _exit;
	msg_pipe<shared_data>::writer_type _sendPipe;
	msg_pipe<shared_data>::regist_reader _sendPump;
	actor_handle _sessionActor;
	actor_handle _lstCloseActor;
	CFont _editFont;
public:
	CEdit _msgEdit;
	CEdit _outputEdit;
};

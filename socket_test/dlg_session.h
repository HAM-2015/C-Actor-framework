#pragma once
#include "afxwin.h"
#include "bind_mfc_run.h"
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
	BIND_MFC_RUN(CDialogEx)
	DECLARE_MESSAGE_MAP()
private:
	void sessionActor(my_actor* self);
public:
	shared_strand _strand;
	socket_handle _socket;
	std::function<void ()> _closeCallback;
	msg_pipe<>::writer_type _closeNtf;
	msg_pipe<>::regist_reader _lstClose;
private:
	bool _exit;
	msg_pipe<shared_data>::writer_type _sendPipe;
	msg_pipe<shared_data>::regist_reader _sendPump;
	CFont _editFont;
public:
	CEdit _msgEdit;
	CEdit _outputEdit;
};

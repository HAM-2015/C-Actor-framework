
// socket_testDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "bind_mfc_run.h"
#include "msg_pipe.h"
#include "shared_data.h"
#include "socket_io.h"
#include "text_stream_io.h"
#include "acceptor_socket.h"
#include "dlg_session.h"


// Csocket_testDlg �Ի���
class Csocket_testDlg : public CDialogEx, bind_mfc_run
{
// ����
public:
	Csocket_testDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~Csocket_testDlg();

// �Ի�������
	enum { IDD = IDD_SOCKET_TEST_DIALOG };

	enum ui_cmd
	{
		ui_connect,
		ui_disconnect,
		ui_listen,
		ui_stopListen,
		ui_close,
		ui_postMsg
	};

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	struct client_param 
	{
		socket_handle _clientSocket;
		string _ip;
		int _port;
		int _tm;
	};

	struct server_param 
	{
		int _port;
		int _maxSessionNum;
	};

	struct session_pck 
	{
		socket_handle _socket;
		msg_pipe<>::writer_type _closeNtf;
		msg_pipe<>::regist_reader _lstClose;
		actor_handle _sessionDlg;
	};
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedClientConnect();
	afx_msg void OnBnClickedClientDisconnect();
	afx_msg void OnBnClickedSendClientMsg();
	afx_msg void OnBnClickedListen();
	afx_msg void OnBnClickedStopLst();
	afx_msg void OnBnClickedClear();
	void showClientMsg(shared_data msg);
	void showSessionNum(int n);
	BIND_MFC_RUN(CDialogEx);
	DECLARE_MESSAGE_MAP()
private:
	void connectActor(my_actor* self, std::shared_ptr<client_param> param);
	void newSession(my_actor* self, std::shared_ptr<session_pck> sess);
	void serverActor(my_actor* self, std::shared_ptr<server_param> param);
	void mainActor(my_actor* self);
private:
	ios_proxy _ios;
	shared_strand _strand;
	CFont _editFont;
	std::function<void (ui_cmd)> _uiCMD;
public:
	CEdit _outputEdit;
	CEdit _msgEdit;
	CIPAddressCtrl _clientIpEdit;
	CEdit _clientPortEdit;
	CEdit _clientTimeoutEdit;
	CEdit _serverPortEdit;
	CEdit _maxSessionEdit;
	CEdit _extSessNumEdit;
};


// socket_testDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "bind_modal_run.h"
#include "boost_coroutine.h"
#include "msg_pipe.h"
#include "shared_data.h"
#include "socket_io.h"
#include "text_stream_io.h"
#include "acceptor_socket.h"
#include "dlg_session.h"


// Csocket_testDlg �Ի���
class Csocket_testDlg : public CDialogEx, bind_modal_run
{
// ����
public:
	Csocket_testDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SOCKET_TEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	struct client_param 
	{
		boost::shared_ptr<socket_io> _clientSocket;
		string _ip;
		int _port;
		int _tm;
		msg_pipe<shared_data>::regist_reader _msgPump;
	};

	struct server_param 
	{
		int _port;
		int _maxSessionNum;
		msg_pipe<>::regist_reader _closePump;
	};

	struct session_pck 
	{
		boost::thread _thread;
		boost::shared_ptr<socket_io> _socket;
		msg_pipe<>::writer_type _closeNtf;
	};
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedClientConnect();
	afx_msg void OnBnClickedClientDisconnect();
	afx_msg void OnBnClickedSendClientMsg();
	afx_msg void OnBnClickedListen();
	afx_msg void OnBnClickedStopLst();
	afx_msg void OnBnClickedClear();
	void showClientMsg(shared_data msg);
	void showSessionNum(int n);
	DECLARE_MESSAGE_MAP()
	BIND_MODAL_RUN()
private:
	void connectCoro(boost_coro* coro, boost::shared_ptr<client_param> param);

	void sessionMng(boost_coro* coro, 
		const msg_pipe<list<boost::shared_ptr<session_pck> >::iterator>::regist_reader& sessClose);
	void newSession(boost::shared_ptr<socket_io> socket, msg_pipe<>::regist_reader closePump, boost::function<void ()> cb);
	void lstClose(boost_coro* coro, boost::shared_ptr<acceptor_socket> accept, const msg_pipe<>::regist_reader& lst);
	void serverCoro(boost_coro* coro, boost::shared_ptr<server_param> param);
private:
	ios_proxy _ios;
	shared_strand _strand;
	boost::shared_ptr<socket_io> _clientSocket;
	boost::shared_ptr<client_param> _extClient;
	list<boost::shared_ptr<session_pck> > _sessList;
	msg_pipe<shared_data>::writer_type _clientPostPipe;
	msg_pipe<>::writer_type _serverNtfClose;
	coro_handle _clientCoro;
	coro_handle _serverCoro;
	CFont _editFont;
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

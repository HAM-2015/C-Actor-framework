// socket_testDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "socket_test.h"
#include "socket_testDlg.h"
#include "afxdialogex.h"
#include <boost/lexical_cast.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Csocket_testDlg �Ի���




Csocket_testDlg::Csocket_testDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Csocket_testDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

Csocket_testDlg::~Csocket_testDlg()
{
	_ios.stop();
}

void Csocket_testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT4, _outputEdit);
	DDX_Control(pDX, IDC_EDIT5, _msgEdit);
	DDX_Control(pDX, IDC_IPADDRESS1, _clientIpEdit);
	DDX_Control(pDX, IDC_EDIT2, _clientPortEdit);
	DDX_Control(pDX, IDC_EDIT3, _clientTimeoutEdit);
	DDX_Control(pDX, IDC_EDIT6, _serverPortEdit);
	DDX_Control(pDX, IDC_EDIT7, _maxSessionEdit);
	DDX_Control(pDX, IDC_EDIT8, _extSessNumEdit);
}

BEGIN_MESSAGE_MAP(Csocket_testDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	REGIEST_MFC_RUN(Csocket_testDlg)
	ON_BN_CLICKED(IDC_BUTTON1, &Csocket_testDlg::OnBnClickedClientConnect)
	ON_BN_CLICKED(IDC_BUTTON2, &Csocket_testDlg::OnBnClickedClientDisconnect)
	ON_BN_CLICKED(IDC_BUTTON3, &Csocket_testDlg::OnBnClickedSendClientMsg)
	ON_BN_CLICKED(IDC_BUTTON4, &Csocket_testDlg::OnBnClickedListen)
	ON_BN_CLICKED(IDC_BUTTON5, &Csocket_testDlg::OnBnClickedStopLst)
	ON_BN_CLICKED(IDC_BUTTON6, &Csocket_testDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// Csocket_testDlg ��Ϣ�������

BOOL Csocket_testDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	_editFont.CreatePointFont(90, "����");
	_clientIpEdit.SetFont(&_editFont);
	_clientPortEdit.SetFont(&_editFont);
	_clientTimeoutEdit.SetFont(&_editFont);
	_serverPortEdit.SetFont(&_editFont);
	_msgEdit.SetFont(&_editFont);
	_outputEdit.SetFont(&_editFont);
	_maxSessionEdit.SetFont(&_editFont);
	_extSessNumEdit.SetFont(&_editFont);

	_clientIpEdit.SetAddress(127, 0, 0, 1);
	_clientPortEdit.SetWindowText("1000");
	_clientTimeoutEdit.SetWindowText("500");
	_serverPortEdit.SetWindowText("1000");
	_maxSessionEdit.SetWindowText("3");

	boost_actor::enable_stack_pool();
	boost_actor::disable_auto_make_timer();
	_ios.run();
	_strand = boost_strand::create(_ios);
	actor_msg_handle<ui_cmd>::ptr lstCmd = actor_msg_handle<ui_cmd>::make_ptr();
	actor_handle mainActor = boost_actor::create(_strand, boost::bind(&Csocket_testDlg::mainActor, this, _1, lstCmd));
	_uiCMD = mainActor->make_msg_notify(*lstCmd);
	mainActor->notify_start_run();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void Csocket_testDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void Csocket_testDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR Csocket_testDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Csocket_testDlg::showClientMsg(shared_data msg)
{
	int nLength = _outputEdit.GetWindowTextLength();
	_outputEdit.SetSel(nLength, nLength);
	_outputEdit.ReplaceSel(msg->c_str());
	_outputEdit.ReplaceSel("\n");
}

void Csocket_testDlg::showSessionNum(int n)
{
	_extSessNumEdit.SetWindowText(boost::lexical_cast<string>(n).c_str());
}

void Csocket_testDlg::OnCancel()
{
	_uiCMD(ui_close);
}

void Csocket_testDlg::OnBnClickedClientConnect()
{
	_uiCMD(ui_connect);
}

void Csocket_testDlg::OnBnClickedClientDisconnect()
{
	_uiCMD(ui_disconnect);
}

void Csocket_testDlg::OnBnClickedListen()
{
	_uiCMD(ui_listen);
}

void Csocket_testDlg::OnBnClickedStopLst()
{
	_uiCMD(ui_stopListen);
}

void Csocket_testDlg::OnBnClickedSendClientMsg()
{
	_uiCMD(ui_postMsg);
}

void Csocket_testDlg::OnBnClickedClear()
{
	_outputEdit.SetWindowText("");
}

void Csocket_testDlg::connectActor(boost_actor* actor, boost::shared_ptr<client_param> param)
{
	async_trig_handle<boost::system::error_code> ath;
	post(boost::bind(&Csocket_testDlg::showClientMsg, this, msg_data::create("������...")));
	param->_clientSocket->async_connect(param->_ip.c_str(), param->_port, actor->begin_trig(ath));
	actor->open_timer();
	boost::system::error_code err;
	if (actor->timed_wait_trig(ath, err, param->_tm) && !err && param->_clientSocket->no_delay())
	{
		post(boost::bind(&Csocket_testDlg::showClientMsg, this, msg_data::create("���ӳɹ�")));
		actor_msg_handle<shared_data> cmh;
		child_actor_handle readActor = actor->create_child_actor([this, &cmh](boost_actor* actor)
		{
			while (true)
			{
				//���շ���������Ϣ��Ȼ���͸��Ի���
				shared_data msg = actor->pump_msg(cmh);
				if (!msg)
				{
					break;
				}
				post(boost::bind(&Csocket_testDlg::showClientMsg, this, msg));
			}
			actor->close_msg_notify(cmh);
		});
		auto textio = text_stream_io::create(actor->this_strand(), param->_clientSocket, readActor.get_actor()->make_msg_notify(cmh));
		child_actor_handle writerActor = actor->create_child_actor([&textio, &param](boost_actor* actor)
		{
			actor_msg_handle<shared_data> cmh;
			param->_msgPump(actor, cmh);
			while (true)
			{
				//���նԻ��������Ϣ��Ȼ���͸�������
				textio->write(actor->pump_msg(cmh));
			}
			actor->close_msg_notify(cmh);
		});
		actor->child_actor_run(readActor);
		actor->child_actor_run(writerActor);
		actor->child_actor_wait_quit(readActor);
		actor->child_actor_force_quit(writerActor);
		textio->close();
		post(boost::bind(&Csocket_testDlg::showClientMsg, this, msg_data::create("�Ͽ�����")));
	} 
	else
	{
		post(boost::bind(&Csocket_testDlg::showClientMsg, this, msg_data::create("����ʧ��")));
		param->_clientSocket->close();
	}
	_uiCMD(ui_disconnect);
}

void Csocket_testDlg::newSession(boost_actor* actor, boost::shared_ptr<socket_io> socket, msg_pipe<>::regist_reader closePump)
{//���Actor�����ڶԻ����߳���
	async_trig_handle<> lstClose;
	dlg_session dlg;
	dlg._strand = _strand;
	dlg._socket = socket;
	dlg._lstClose = closePump;
	dlg._closeNtf = actor->begin_trig(lstClose);
	dlg.Create(IDD_SESSION, GetDesktopWindow());
	dlg.ShowWindow(SW_SHOW);
	actor->wait_trig(lstClose);
	dlg.DestroyWindow();
}

void Csocket_testDlg::serverActor(boost_actor* actor, boost::shared_ptr<server_param> param)
{
	actor_msg_handle<boost::shared_ptr<socket_io> > cmh;
	auto accept = acceptor_socket::create(actor->this_strand(), param->_port, actor->make_msg_notify(cmh), false);//��������������
	if (!accept)
	{
		actor->close_msg_notify(cmh);
		post(boost::bind(&Csocket_testDlg::OnBnClickedStopLst, this));
		return;
	}
	//���������������ر�
	child_actor_handle lstCloseProxyActor = actor->create_child_actor([&param, &accept](boost_actor* actor)//�����������ر�
	{
		actor_msg_handle<> cmh;
		param->_closePump(actor, cmh);
		actor->pump_msg(cmh);
		//�õ��������ر���Ϣ���ر�����������
		accept->close();
		actor->close_msg_notify(cmh);
	});
	//�����Ự�ر���Ӧ��
	list<boost::shared_ptr<session_pck> > sessList;
	actor_msg_handle<list<boost::shared_ptr<session_pck> >::iterator> sessDissonnLst;
	child_actor_handle sessMngActor = actor->create_child_actor([&](boost_actor* actor)
	{
		while (true)
		{
			auto it = actor->pump_msg(sessDissonnLst);
			//�õ�һ���Ự�ر���Ϣ���ȵȴ��Ի����̹߳رգ��ٴ��б���ɾ��
			actor->another_actor_wait_quit((*it)->_sessionDlg);
			sessList.erase(it);
			post(boost::bind(&Csocket_testDlg::showSessionNum, this, sessList.size()));
		}
		actor->close_msg_notify(sessDissonnLst);
	});
	auto sessDissonnNtf = sessMngActor.get_actor()->make_msg_notify(sessDissonnLst);
	actor->child_actor_run(lstCloseProxyActor);
	actor->child_actor_run(sessMngActor);
	while (true)
	{
		auto newSocket = actor->pump_msg(cmh);
		if (!newSocket)
		{
			break;
		}
		if (sessList.size() < (size_t)param->_maxSessionNum && newSocket->no_delay())
		{
			boost::shared_ptr<session_pck> newSess(new session_pck);
			post(boost::bind(&Csocket_testDlg::showSessionNum, this, sessList.size()));
			newSess->_socket = newSocket;
			sessList.push_front(newSess);
			newSess->_sessionDlg = create_mfc_actor(boost::bind(&Csocket_testDlg::newSession, this, _1, newSocket,
				msg_pipe<>::make(newSess->_closeNtf)));
			newSess->_sessionDlg->notify_start_run();
		}
		else
		{
			newSocket->close();
		}
	}
	actor->child_actor_force_quit(lstCloseProxyActor);
	actor->child_actor_force_quit(sessMngActor);
	actor->close_msg_notify(cmh);
	//֪ͨ���д��ڵĶԻ���ر�
	for (auto it = sessList.begin(); it != sessList.end(); it++)
	{
		(*it)->_closeNtf();
		actor->another_actor_wait_quit((*it)->_sessionDlg);
	}
}

void Csocket_testDlg::mainActor(boost_actor* actor, actor_msg_handle<ui_cmd>::ptr lstCMD)
{
	boost::shared_ptr<client_param> extClient;
	boost::function<void ()> serverNtfClose;
	boost::function<void (shared_data)> clientPostPipe;
	actor_handle clientActorHandle;
	actor_handle serverActorHandle;
	auto disconnectHandler = [&, this]()
	{
		if (clientActorHandle)
		{
			extClient->_clientSocket->close();
			actor->another_actor_wait_quit(clientActorHandle);
			extClient.reset();
			clientActorHandle.reset();
			clientPostPipe.clear();
			auto _this = this;
			this->send(actor, [&, _this]()
			{
				_this->GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
				_this->GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
			});
		}
	};
	auto stopListenHandler = [&, this]()
	{
		if (serverActorHandle)
		{
			serverNtfClose();
			actor->another_actor_wait_quit(serverActorHandle);
			serverActorHandle.reset();
			auto _this = this;
			this->send(actor, [&, _this]()
			{
				_extSessNumEdit.SetWindowText("");
				_this->GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
				_this->GetDlgItem(IDC_BUTTON5)->EnableWindow(FALSE);
			});
		}
	};

	while (true)
	{
		switch (actor->pump_msg(*lstCMD))
		{
		case ui_connect:
			{
				if (!clientActorHandle)
				{
					send(actor, [&, this]()
					{
						try
						{
							BYTE bip[4];
							char sip[32];
							CString sport;
							CString stm;
							_clientIpEdit.GetAddress(bip[0], bip[1], bip[2], bip[3]);
							sprintf_s(sip, "%d.%d.%d.%d", bip[0], bip[1], bip[2], bip[3]);
							_clientPortEdit.GetWindowText(sport);
							_clientTimeoutEdit.GetWindowText(stm);
							extClient = boost::shared_ptr<client_param>(new client_param);
							extClient->_ip = sip;
							extClient->_port = boost::lexical_cast<int>(sport.GetBuffer());
							extClient->_tm = boost::lexical_cast<int>(stm.GetBuffer());
							extClient->_clientSocket = socket_io::create(_ios);
							extClient->_msgPump = msg_pipe<shared_data>::make(clientPostPipe);
							clientActorHandle = boost_actor::create(_strand, 
								boost::bind(&Csocket_testDlg::connectActor, this, _1, extClient));
							clientActorHandle->notify_start_run();
							this->GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
							this->GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
						}
						catch (...)
						{
							extClient.reset();
							clientActorHandle.reset();
						}
					});
				}
			}
			break;
		case ui_listen:
			{
				if (!serverActorHandle)
				{
					send(actor, [&, this]()
					{
						CString sport;
						CString snum;
						_serverPortEdit.GetWindowText(sport);
						_maxSessionEdit.GetWindowText(snum);
						try
						{
							boost::shared_ptr<server_param> param(new server_param);
							param->_port = boost::lexical_cast<int>(sport.GetBuffer());
							param->_maxSessionNum = boost::lexical_cast<int>(snum.GetBuffer());
							param->_closePump = msg_pipe<>::make(serverNtfClose);
							serverActorHandle = boost_actor::create(_strand, boost::bind(&Csocket_testDlg::serverActor, this, _1, param));
							serverActorHandle->notify_start_run();
							_extSessNumEdit.SetWindowText("");
							this->GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
							this->GetDlgItem(IDC_BUTTON5)->EnableWindow(TRUE);
						}
						catch (...)
						{
							serverActorHandle.reset();
						}
					});
				}
			}
			break;
		case ui_postMsg:
			{
				if (clientPostPipe)
				{
					send(actor, [&]()
					{
						CString cs;
						_msgEdit.GetWindowText(cs);
						if (cs.GetLength())
						{
							_msgEdit.SetWindowText("");
							clientPostPipe(msg_data::create(cs.GetBuffer()));
						}
						_msgEdit.SetFocus();
					});
				}
			}
			break;
		case ui_disconnect:
			{
				disconnectHandler();
			}
			break;
		case ui_stopListen:
			{
				stopListenHandler();
			}
			break;
		case ui_close:
			{
				disconnectHandler();
				stopListenHandler();
				actor->append_quit_callback(wrap<boost::function<void (bool)> >([this](bool)
				{
					_isClosed = true;
					this->baseCancel();
				}));
				actor->close_msg_notify(*lstCMD);
				return;
			}
			break;
		default:
			break;
		}
	}
}


// mfc_coroDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "mfc_coro.h"
#include "mfc_coroDlg.h"
#include "afxdialogex.h"

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


// Cmfc_coroDlg �Ի���




Cmfc_coroDlg::Cmfc_coroDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cmfc_coroDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

Cmfc_coroDlg::~Cmfc_coroDlg()
{
	_ios.stop();
}

void Cmfc_coroDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, _dataEdit1);
	DDX_Control(pDX, IDC_EDIT2, _dataEdit2);
	DDX_Control(pDX, IDC_EDIT3, _dataEdit3);
	DDX_Control(pDX, IDC_EDIT4, _dataEdit4);
	DDX_Control(pDX, IDC_EDIT5, _dataEdit5);
	DDX_Control(pDX, IDC_EDIT6, _dataEdit6);
	DDX_Control(pDX, IDC_EDIT7, _dataEdit7);
	DDX_Control(pDX, IDC_EDIT8, _dataEdit8);
	DDX_Control(pDX, IDC_EDIT9, _dataEdit9);
	DDX_Control(pDX, IDC_EDIT10, _dataEdit10);
	DDX_Control(pDX, IDC_EDIT11, _dataEdit11);
	DDX_Control(pDX, IDC_EDIT12, _dataEdit12);
}

BEGIN_MESSAGE_MAP(Cmfc_coroDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	REGIEST_MFC_RUN(Cmfc_coroDlg)
END_MESSAGE_MAP()


// Cmfc_coroDlg ��Ϣ�������

BOOL Cmfc_coroDlg::OnInitDialog()
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
	_ios.run();
	_mfcCoro1 = create_mfc_coro(_ios, boost::bind(&Cmfc_coroDlg::mfc_coro1, this, _1));
	_mfcCoro2 = create_mfc_coro(_ios, boost::bind(&Cmfc_coroDlg::mfc_coro2, this, _1));
	_boostCoro1 = boost_coro::create(boost_strand::create(_ios), boost::bind(&Cmfc_coroDlg::boost_coro1, this, _1));
	_boostCoro2 = boost_coro::create(boost_strand::create(_ios), boost::bind(&Cmfc_coroDlg::boost_coro2, this, _1));
	_mfcCoro1->notify_start_run();
	_mfcCoro2->notify_start_run();
	_boostCoro1->notify_start_run();
	_boostCoro2->notify_start_run();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void Cmfc_coroDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cmfc_coroDlg::OnPaint()
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
HCURSOR Cmfc_coroDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cmfc_coroDlg::mfc_coro1(boost_coro* coro)
{
	while (true)
	{
		_dataEdit1.SetWindowText("+++");
		_dataEdit2.SetWindowText("");
		_dataEdit3.SetWindowText("");
		coro->sleep(1000);
		_dataEdit1.SetWindowText("");
		_dataEdit2.SetWindowText("+++");
		_dataEdit3.SetWindowText("");
		coro->sleep(1000);
		_dataEdit1.SetWindowText("");
		_dataEdit2.SetWindowText("");
		_dataEdit3.SetWindowText("+++");
		coro->sleep(1000);
	}
}

void Cmfc_coroDlg::mfc_coro2(boost_coro* coro)
{
	while (true)
	{
		_dataEdit4.SetWindowText("---");
		_dataEdit5.SetWindowText("");
		_dataEdit6.SetWindowText("");
		coro->sleep(300);
		_dataEdit4.SetWindowText("");
		_dataEdit5.SetWindowText("---");
		_dataEdit6.SetWindowText("");
		coro->sleep(300);
		_dataEdit4.SetWindowText("");
		_dataEdit5.SetWindowText("");
		_dataEdit6.SetWindowText("---");
		coro->sleep(300);
	}
}

void Cmfc_coroDlg::boost_coro1(boost_coro* coro)
{
	while (true)
	{
		send(coro, [this]()
		{
			_dataEdit7.SetWindowText("***");
			_dataEdit8.SetWindowText("");
			_dataEdit9.SetWindowText("");
		});
		coro->sleep(200);
		send(coro, [this]()
		{
			_dataEdit7.SetWindowText("");
			_dataEdit8.SetWindowText("***");
			_dataEdit9.SetWindowText("");
		});
		coro->sleep(200);
		send(coro, [this]()
		{
			_dataEdit7.SetWindowText("");
			_dataEdit8.SetWindowText("");
			_dataEdit9.SetWindowText("***");
		});
		coro->sleep(200);
	}
}

void Cmfc_coroDlg::boost_coro2(boost_coro* coro)
{
	while (true)
	{
		send(coro, [this]()
		{
			_dataEdit10.SetWindowText("///");
			_dataEdit11.SetWindowText("");
			_dataEdit12.SetWindowText("");
		});
		coro->sleep(100);
		send(coro, [this]()
		{
			_dataEdit10.SetWindowText("");
			_dataEdit11.SetWindowText("///");
			_dataEdit12.SetWindowText("");
		});
		coro->sleep(100);
		send(coro, [this]()
		{
			_dataEdit10.SetWindowText("");
			_dataEdit11.SetWindowText("");
			_dataEdit12.SetWindowText("///");
		});
		coro->sleep(100);
	}
}

void Cmfc_coroDlg::OnCancel()
{
	_mfcCoro1->notify_force_quit();
	_mfcCoro2->notify_force_quit();
	_boostCoro1->notify_force_quit();
	_boostCoro2->notify_force_quit();
	_mfcCoro1->append_quit_callback(close_mfc_handler());
	_mfcCoro2->append_quit_callback(close_mfc_handler());
	_boostCoro1->append_quit_callback(close_mfc_handler());
	_boostCoro2->append_quit_callback(close_mfc_handler());
}
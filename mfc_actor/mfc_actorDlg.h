
// mfc_actorDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "bind_mfc_run.h"
#include "mfc_strand.h"
#include "actor_framework.h"


// Cmfc_actorDlg �Ի���
class Cmfc_actorDlg : public CDialogEx, bind_mfc_run
{
	// ����
public:
	Cmfc_actorDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~Cmfc_actorDlg();

	// �Ի�������
	enum { IDD = IDD_MFC_ACTOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


	// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedAdd();
	DECLARE_MESSAGE_MAP()
	BIND_MFC_RUN()
	BIND_ACTOR_SEND()
	BIND_MFC_ACTOR(Cmfc_actorDlg, CDialogEx)
private:
	void mfc_actor1(boost_actor* actor);
	void mfc_actor2(boost_actor* actor);
	void boost_actor1(boost_actor* actor);
	void boost_actor2(boost_actor* actor);
	void calc_add(boost_actor* actor, actor_msg_handle<>::ptr lstTask);
private:
	ios_proxy _ios;
	shared_strand _strand;
	actor_handle _mfcActor1;
	actor_handle _mfcActor2;
	actor_handle _boostActor1;
	actor_handle _boostActor2;
	actor_handle _calcActor;
	boost::function<void ()> _calcNotify;
public:
	CEdit _dataEdit1;
	CEdit _dataEdit2;
	CEdit _dataEdit3;
	CEdit _dataEdit4;
	CEdit _dataEdit5;
	CEdit _dataEdit6;
	CEdit _dataEdit7;
	CEdit _dataEdit8;
	CEdit _dataEdit9;
	CEdit _dataEdit10;
	CEdit _dataEdit11;
	CEdit _dataEdit12;
};
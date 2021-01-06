#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "LouFaceDlg.h"

extern CLouFaceDlg *pLouFaceDlg;
#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF", "adoEOF") //����ADO���ӵ�ϵͳ�ļ���ͬʱ���������


// FaceManDlg �Ի���
class FaceManDlg : public CDialog
{
	DECLARE_DYNAMIC(FaceManDlg)

public:
	FaceManDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~FaceManDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FACEMAN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedOk();
	CBrush m_brush;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	_ConnectionPtr m_pConnection;//����access���ݿ�����Ӷ���
	_RecordsetPtr m_pRecordset;//���������
	CListCtrl m_list;
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedQuery();
	afx_msg void OnBnClickedDel();
	CEdit m_faceid;
	CEdit m_address;
	CEdit m_name;
	afx_msg void OnBnClickedMod();//�޸�����
	afx_msg void OnClose();//�رմ���
	afx_msg void OnBnClickedExit();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);//�ػ��������ġ�
};

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "LouFaceDlg.h"

extern CLouFaceDlg *pLouFaceDlg;
#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF", "adoEOF") //导入ADO连接的系统文件，同时声明结果集


// FaceManDlg 对话框
class FaceManDlg : public CDialog
{
	DECLARE_DYNAMIC(FaceManDlg)

public:
	FaceManDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~FaceManDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FACEMAN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedOk();
	CBrush m_brush;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	_ConnectionPtr m_pConnection;//连接access数据库的链接对象
	_RecordsetPtr m_pRecordset;//结果集对象
	CListCtrl m_list;
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedQuery();
	afx_msg void OnBnClickedDel();
	CEdit m_faceid;
	CEdit m_address;
	CEdit m_name;
	afx_msg void OnBnClickedMod();//修改数据
	afx_msg void OnClose();//关闭窗口
	afx_msg void OnBnClickedExit();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);//截获控制命令的。
};

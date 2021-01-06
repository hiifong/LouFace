#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "AVPlayer.h"
#include "LouFaceDlg.h"
extern CLouFaceDlg *pLouFaceDlg;
// CVideoPlayerDlg �Ի���

class CVideoPlayerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoPlayerDlg)

public:
	CVideoPlayerDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CVideoPlayerDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEOPLAYER_DIALOG};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	HICON m_hIcon;
	DECLARE_MESSAGE_MAP()
private:
	CAVPlayer   m_cAVPlayer;        // ��������
public:

	void ShowPlaylist(BOOL bShow);  // ��ʾ�����ز����б�
	afx_msg void OnBnClickedButtonPlay();
	CListBox m_listPlay;
	CSliderCtrl m_slider;
	afx_msg void OnBnClickedButtonPlayPrevious();
	afx_msg void OnDropFiles(HDROP hDropInfo);
//	afx_msg LRESULT OnPosChanged(WPARAM wParam, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnBnClickedButtonPlayNext();
	afx_msg void OnBnClickedButtonPlayFastBackforward();
	afx_msg void OnBnClickedButtonPlayFastForward();
	afx_msg void OnBnClickedButtonVolumeDecrease();
	afx_msg void OnBnClickedButtonVolumeIncrease();
	afx_msg void OnBnClickedButtonShowPlayList();
//	afx_msg void OnLbnDblclkListPlay();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnDblclkListPlay();

//	afx_msg LRESULT OnPosChanged(WPARAM wParam, LPARAM lParam);
protected:
	afx_msg LRESULT OnUserPosChanged(WPARAM wParam, LPARAM lParam);
public:
	CProgressCtrl m_Voiceprocess;
//	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};

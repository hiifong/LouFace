
// LouFaceDlg.h : ͷ�ļ�
//���ƣ����ں�������IP����ͷ������ʶ��ϵͳ
//���ߣ�¦��
//ʱ�䣺2020.04.01
//��ϵQQ��406960301
//��飺���ں�������+����SDK
//�汾��Version 1.0

#pragma once
#include "afxcmn.h"
#include "ArcFaceEngine.h"

#include <memory>
#include <string>

#include "HCNetSDK.h"
#include "PlayM4.h"
#include "afxcmn.h"
//����GDI+
#include <GdiPlus.h>
#include "afxwin.h"
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

#include "resource.h"

// CLouFaceDlg �Ի���
class CLouFaceDlg : public CDialogEx
{
	// ����
public:
	CLouFaceDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CLouFaceDlg();
	// �Ի�������
	enum { IDD = IDD_LouFace_DIALOG};

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
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnRegister();
	afx_msg void OnBnClickedBtnRecognition();
	afx_msg void OnBnClickedBtnCompare();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnCamera();
	afx_msg void OnEnChangeEditThreshold();

	void EditOut(CString str, bool add_endl);
	void IplDrawToHDC(BOOL isVideoMode, IplImage* rgbImage, CRect& showRect, UINT ID);

private:
	void LoadThumbnailImages();
	CString SelectFolder();
	BOOL TerminateLoadThread();
	BOOL ClearRegisterImages();
	BOOL CalculateShowPositon(IplImage*curSelectImage, Gdiplus::Rect& showRect);

	void ClearShowWindow();
public:
	MRESULT StaticImageFaceOp(IplImage* image);
	CListCtrl m_ImageListCtrl;//ͼƬ�б�ؼ�
	CImageList m_IconImageList;
	CEdit m_editLog;

	CString m_folderPath;
	std::vector<ASF_FaceFeature> m_featuresVec;//����ֵ����

	BOOL m_bLoadIconThreadRunning;
	DWORD m_dwLoadIconThreadID;
	HANDLE m_hLoadIconThread;

	BOOL m_bClearFeatureThreadRunning;
	DWORD m_dwClearFeatureThreadID;
	HANDLE m_hClearFeatureThread;

	BOOL m_bFDThreadRunning;
	DWORD m_dwFDThreadID;
	HANDLE m_hFDThread;

	BOOL m_bFRThreadRunning;
	DWORD m_dwFRThreadID;
	HANDLE m_hFRThread;

	ArcFaceEngine m_imageFaceEngine;
	ArcFaceEngine m_videoFaceEngine;

	IplImage* m_curStaticImage;					//��ǰѡ�е�ͼƬ
	ASF_FaceFeature m_curStaticImageFeature;	//��ǰͼƬ����������
	BOOL m_curStaticImageFRSucceed;
	Gdiplus::Rect m_curFaceShowRect;
	Gdiplus::Rect m_curImageShowRect;//ͼƬ�ڴ�����ʾ����

	CString m_curStaticShowAgeGenderString;

	CString m_curStaticShowCmpString;

	IplImage* m_curVideoImage;
	IplImage* m_curIrVideoImage;
	ASF_SingleFaceInfo m_curFaceInfo;
	bool m_dataValid;
	bool m_irDataValid;

	bool m_videoOpened;
	CString m_strEditThreshold;

	Gdiplus::PointF m_curStringShowPosition;	//��ǰ�ַ�����ʾ��λ��
	CString m_curVideoShowString;
	CString m_curIRVideoShowString;
	CFont* m_Font;
private:

public:
	CRect m_windowViewRect;						//չʾ�ؼ��ĳߴ�
	afx_msg void OnClose();
	LONG lUserID;
	LONG llRealHandle;
	// IP��ַ�ؼ�����
	CIPAddressCtrl m_ctrlDeviceIP;

	// �û�������
	CString m_csPassword;
	// ��¼�˿ں�
	short m_nLoginPort;
	// �û���
	CString m_csUserName;
	// IP��ַ�༭������
	short iPChannel;
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedStartreview();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedRecord();
	BOOL m_bIsRecording;
	CBrush m_brush;
	void StartRecord();
	void StopRecord();

	void SaveCDC2BMP(CWnd * pWnd, TCHAR * path, Gdiplus::Rect rect);
	afx_msg void OnBnClickedFaceman();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	
	afx_msg void OnBnClickedVideoPlayer();
	afx_msg void OnBnClickedFaceOpen();
	afx_msg void OnBnClickedFaceOpen2();
//	afx_msg void OnBnClickedButtonTest();

//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonTest();
};

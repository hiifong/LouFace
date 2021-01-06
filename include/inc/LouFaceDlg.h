
// LouFaceDlg.h : 头文件
//名称：基于海康威视IP摄像头的人脸识别系统
//作者：娄瑞
//时间：2020.04.01
//联系QQ：406960301
//简介：基于海康威视+虹软SDK
//版本：Version 1.0

#pragma once
#include "afxcmn.h"
#include "ArcFaceEngine.h"

#include <memory>
#include <string>

#include "HCNetSDK.h"
#include "PlayM4.h"
#include "afxcmn.h"
//调用GDI+
#include <GdiPlus.h>
#include "afxwin.h"
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

#include "resource.h"

// CLouFaceDlg 对话框
class CLouFaceDlg : public CDialogEx
{
	// 构造
public:
	CLouFaceDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CLouFaceDlg();
	// 对话框数据
	enum { IDD = IDD_LouFace_DIALOG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
	CListCtrl m_ImageListCtrl;//图片列表控件
	CImageList m_IconImageList;
	CEdit m_editLog;

	CString m_folderPath;
	std::vector<ASF_FaceFeature> m_featuresVec;//特征值队列

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

	IplImage* m_curStaticImage;					//当前选中的图片
	ASF_FaceFeature m_curStaticImageFeature;	//当前图片的人脸特征
	BOOL m_curStaticImageFRSucceed;
	Gdiplus::Rect m_curFaceShowRect;
	Gdiplus::Rect m_curImageShowRect;//图片在窗口显示区域

	CString m_curStaticShowAgeGenderString;

	CString m_curStaticShowCmpString;

	IplImage* m_curVideoImage;
	IplImage* m_curIrVideoImage;
	ASF_SingleFaceInfo m_curFaceInfo;
	bool m_dataValid;
	bool m_irDataValid;

	bool m_videoOpened;
	CString m_strEditThreshold;

	Gdiplus::PointF m_curStringShowPosition;	//当前字符串显示的位置
	CString m_curVideoShowString;
	CString m_curIRVideoShowString;
	CFont* m_Font;
private:

public:
	CRect m_windowViewRect;						//展示控件的尺寸
	afx_msg void OnClose();
	LONG lUserID;
	LONG llRealHandle;
	// IP地址控件变量
	CIPAddressCtrl m_ctrlDeviceIP;

	// 用户名密码
	CString m_csPassword;
	// 登录端口号
	short m_nLoginPort;
	// 用户名
	CString m_csUserName;
	// IP地址编辑器变量
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

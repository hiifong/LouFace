
// LouFaceDlg.cpp : 实现文件
//名称：基于海康威视IP摄像头的人脸识别系统
//作者：娄瑞
//时间：2020.04.01
//联系QQ：406960301
//简介：基于海康威视+虹软SDK
//版本：Version 1.0
#include "stdafx.h"

#include "LouFace.h"
#include "LouFaceDlg.h"
#include "FaceManDlg.h"
#include "afxdialogex.h"
#include <afx.h>
#include <vector>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <mutex>
#include <strmif.h>
#include <initguid.h>
#include <string>
#include "VideoPlayerDlg.h"
#include<windows.h>
#include<Mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <sapi.h>    // 导入语音头文件
#include <sphelper.h>// 导入语音识别头文件
#pragma comment(lib,"sapi.lib") //语音库

#pragma comment(lib, "setupapi.lib")

extern FaceManDlg *pFaceManDlg;
CString CurrentFaceID("");
using namespace std;
using namespace Gdiplus;
ULONG_PTR m_gdiplusToken;

#define VIDEO_FRAME_DEFAULT_WIDTH 1024
#define VIDEO_FRAME_DEFAULT_HEIGHT 1024

#define FACE_FEATURE_SIZE 1032

#define THUMBNAIL_WIDTH  55
#define THUMBNAIL_HEIGHT  55
#define Threshold 0.80

#define VI_MAX_CAMERAS 20
DEFINE_GUID(CLSID_SystemDeviceEnum, 0x62be5d10, 0x60eb, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(CLSID_VideoInputDeviceCategory, 0x860bb310, 0x5d01, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(IID_ICreateDevEnum, 0x29840822, 0x5b84, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; } 

mutex g_mutex;

vector<string> g_cameraName;
static int g_cameraNum = 0;
static int g_rgbCameraId = -1;
static int g_irCameraId = -1;
static float g_rgbLiveThreshold = 0.0;
static float g_irLiveThreshold = 0.0;

//////////////////////////////////////////////////////////////////////////
int iPicNum = 0;//Set channel NO.
LONG nPort = -1;
FILE *Videofile = NULL;
FILE *Audiofile = NULL;
char filename[100];
HWND hPlayWnd = NULL;
CLouFaceDlg *pLouFaceDlg = NULL;
vector<CString> m_vFileName;//文件名称数组
//语音库
ISpVoice* pVoice = NULL; //初始化COM
HRESULT hr;
ISpObjectToken* pSpObjectToken = NULL;


CString ErrorNum;//错误号
////////////////////////////////////////////////////////////////////////
unsigned long _stdcall RunLoadThumbnailThread(LPVOID lpParam);
unsigned long _stdcall RunFaceFeatureOperation(LPVOID lpParam);
unsigned long _stdcall RunFaceDetectOperation(LPVOID lpParam);
unsigned long _stdcall ClearFaceFeatureOperation(LPVOID lpParam);


Bitmap* IplImage2Bitmap(const IplImage* pIplImg);
IplImage* Bitmap2IplImage(Bitmap* pBitmap);
CBitmap* IplImage2CBitmap(const IplImage *img);
BOOL SetTextFont(CFont* font, int fontHeight, int fontWidth, string fontStyle);

int listDevices(vector<string>& list);			//获取摄像头
//读取配置文件
void ReadSetting(char* appID, char* sdkKey, char* activeKey, char* tag,
char* rgbLiveThreshold, char* irLiveThreshold, char* rgbCameraId, char* irCameraId);

// IP to String
CString IPToStr(DWORD dwIP)
{
	CString strIP = _T("");
	WORD add1, add2, add3, add4;

	add1 = (WORD)(dwIP & 255);
	add2 = (WORD)((dwIP >> 8) & 255);
	add3 = (WORD)((dwIP >> 16) & 255);
	add4 = (WORD)((dwIP >> 24) & 255);
	strIP.Format("%d.%d.%d.%d", add4, add3, add2, add1);
	return strIP;
}
//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void yv12toYUV(char *outYuv, char *inYv12, int width, int height, int widthStep)
{
	int col, row;
	unsigned int Y, U, V;
	int tmp;
	int idx;

	//printf("widthStep=%d.\n",widthStep);

	for (row = 0; row<height; row++)
	{
		idx = row * widthStep;
		int rowptr = row*width;

		for (col = 0; col<width; col++)
		{
			//int colhalf=col>>1;
			tmp = (row / 2)*(width / 2) + (col / 2);
			//         if((row==1)&&( col>=1400 &&col<=1600))
			//         { 
			//          printf("col=%d,row=%d,width=%d,tmp=%d.\n",col,row,width,tmp);
			//          printf("row*width+col=%d,width*height+width*height/4+tmp=%d,width*height+tmp=%d.\n",row*width+col,width*height+width*height/4+tmp,width*height+tmp);
			//         } 
			Y = (unsigned int)inYv12[row*width + col];
			U = (unsigned int)inYv12[width*height + width*height / 4 + tmp];
			V = (unsigned int)inYv12[width*height + tmp];
			//         if ((col==200))
			//         { 
			//         printf("col=%d,row=%d,width=%d,tmp=%d.\n",col,row,width,tmp);
			//         printf("width*height+width*height/4+tmp=%d.\n",width*height+width*height/4+tmp);
			//         return ;
			//         }
			if ((idx + col * 3 + 2)> (1200 * widthStep))
			{
				//printf("row * widthStep=%d,idx+col*3+2=%d.\n",1200 * widthStep,idx+col*3+2);
			}
			outYuv[idx + col * 3] = Y;
			outYuv[idx + col * 3 + 1] = U;
			outYuv[idx + col * 3 + 2] = V;
		}
	}
	//printf("col=%d,row=%d.\n",col,row);
}
unsigned long _stdcall Voice_ThreadProc(LPVOID lpParameter)
{

	pVoice = NULL; //初始化COM 
	if (FAILED(CoInitialize(NULL)))
	{
		AfxMessageBox(_T("Error to intiliaze COM"));
		//	return;
	}
	// 初始化SAPI 
	hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	pSpObjectToken = NULL;
	if (SUCCEEDED(SpFindBestToken(SPCAT_VOICES, L"language=804", NULL, &pSpObjectToken)))// 804代表中文 
	{
		pVoice->SetVoice(pSpObjectToken); // 声音大小
		pVoice->SetRate(-1);// 语速
		pSpObjectToken->Release();
	}
	else
	{
		AfxMessageBox(_T("没有安装微软语音库!"));
		//		return;
	}
	if (SUCCEEDED(hr))
	{
		CString strText = _T("欢迎进入北京交通大学海滨学院娄瑞制作的人脸识别系统");
		hr = pVoice->Speak(strText.AllocSysString(), 0, NULL);

		pVoice->Release();
		pVoice = NULL;
		CoUninitialize();

	}

	return 0;
}
//对于pImg的RGB图像进行人脸检测
BOOL FaceDetect_flag = FALSE;
void FaceDetect(IplImage* pImg)
{
	//停止预览时此段代码会出现卡死，所以打算采取互斥量

	ASF_SingleFaceInfo faceInfo = { 0 };
	MRESULT detectRes;
	detectRes = pLouFaceDlg->m_videoFaceEngine.PreDetectFace(pImg, faceInfo, true);
	if (MOK == detectRes)
	{
		cvRectangle(pImg, cvPoint(faceInfo.faceRect.left, faceInfo.faceRect.top), cvPoint(faceInfo.faceRect.right, faceInfo.faceRect.bottom), cvScalar(0, 0, 255), 2);

		pLouFaceDlg->m_curFaceInfo = faceInfo;
		pLouFaceDlg->m_dataValid = true;
		pLouFaceDlg->EditOut("识别到人脸\n", TRUE);
		CString   str(".//notice.wav");
		PlaySound(str, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
	}
	else
	{
		//没有人脸不要显示信息
		pLouFaceDlg->m_curVideoShowString = "";
		pLouFaceDlg->m_dataValid = false;
		pLouFaceDlg->EditOut("无人脸\n", TRUE);
		CString   str(".//scan.wav");
		PlaySound(str, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
	}
	//重新拷贝
	{
		lock_guard<mutex> lock(g_mutex);
		cvReleaseImage(&pLouFaceDlg->m_curVideoImage);
		pLouFaceDlg->m_curVideoImage = cvCloneImage(pImg);
	}
	pLouFaceDlg->m_videoOpened = true;
	pLouFaceDlg->SendMessage(WM_PAINT);

 }

BOOL FaceFeature_flag = FALSE;
FaceManDlg *pFaceManDlg = NULL;

UINT __cdecl ThreadProc(LPVOID lpParameter)
{


	return 0;
}
void FaceFeature(IplImage* pImg)
{

	/***********************************************/
	//检测人脸
	/***********************************************/
	ASF_SingleFaceInfo faceInfo = { 0 };


	MRESULT detectRes = pLouFaceDlg->m_videoFaceEngine.PreDetectFace(pImg, faceInfo, true);
	if (MOK == detectRes)
	{
		cvRectangle(pImg, cvPoint(faceInfo.faceRect.left, faceInfo.faceRect.top),
			cvPoint(faceInfo.faceRect.right, faceInfo.faceRect.bottom), cvScalar(0, 0, 255), 2);

		pLouFaceDlg->m_curFaceInfo = faceInfo;
		pLouFaceDlg->m_dataValid = true;
	}
	else
	{
		//没有人脸不要显示信息
		pLouFaceDlg->m_curVideoShowString = "";
		pLouFaceDlg->m_dataValid = false;
	}

	
	//重新拷贝
	{
		lock_guard<mutex> lock(g_mutex);
		cvReleaseImage(&pLouFaceDlg->m_curVideoImage);
		pLouFaceDlg->m_curVideoImage = cvCloneImage(pImg);
	}
	pLouFaceDlg->SendMessage(WM_PAINT);

	/***********************************************/
	//人脸特征提取
	/***********************************************/
	pLouFaceDlg->m_bFDThreadRunning = TRUE;
	//设置活体检测阈值，sdk内部默认RGB:0.5 IR:0.7,可选择是否调用该接口
	pLouFaceDlg->m_imageFaceEngine.SetLivenessThreshold(g_rgbLiveThreshold, g_irLiveThreshold);

	//初始化特征
	ASF_FaceFeature faceFeature = { 0 };
	faceFeature.featureSize = FACE_FEATURE_SIZE;
	faceFeature.feature = (MByte *)malloc(faceFeature.featureSize * sizeof(MByte));

	ASF_MultiFaceInfo multiFaceInfo = { 0 };
	multiFaceInfo.faceOrient = (MInt32*)malloc(sizeof(MInt32));
	multiFaceInfo.faceRect = (MRECT*)malloc(sizeof(MRECT));


	//先拷贝一份，防止读写冲突
	IplImage* tempImage = NULL;
	{
		lock_guard<mutex> lock(g_mutex);
		if (pLouFaceDlg->m_curVideoImage)
		{
			tempImage = cvCloneImage(pLouFaceDlg->m_curVideoImage);
		}
	}

	//发送一份到活体
	multiFaceInfo.faceNum = 1;
	multiFaceInfo.faceOrient[0] = pLouFaceDlg->m_curFaceInfo.faceOrient;
	multiFaceInfo.faceRect[0] = pLouFaceDlg->m_curFaceInfo.faceRect;

	ASF_AgeInfo ageInfo = { 0 };
	ASF_GenderInfo genderInfo = { 0 };
	ASF_Face3DAngle angleInfo = { 0 };
	ASF_LivenessInfo liveNessInfo = { 0 };

	//IR活体检测
	bool isIRAlive = false;
	isIRAlive = true;

	//RGB属性检测
	MRESULT detectRes2 = pLouFaceDlg->m_imageFaceEngine.FaceASFProcess(multiFaceInfo, tempImage,
		ageInfo, genderInfo, angleInfo, liveNessInfo);

	bool isRGBAlive = false;
	if (detectRes2 == 0 && liveNessInfo.num > 0)
	{
		if (liveNessInfo.isLive[0] == 1)
		{
			isRGBAlive = true;
		}
		else if (liveNessInfo.isLive[0] == 0)
		{
			pLouFaceDlg->m_curVideoShowString = "RGB假体";
		}
		else if(liveNessInfo.isLive[0] == -3)
		{
			pLouFaceDlg->m_curVideoShowString = "人脸过小";
		}
		else if (liveNessInfo.isLive[0] == -4)
		{
			pLouFaceDlg->m_curVideoShowString = "角度过大";
		}
		else if (liveNessInfo.isLive[0] == -5)
		{
			pLouFaceDlg->m_curVideoShowString = "人脸超出边界";
		}
		else 
		{
			//-1：不确定；-2:传入人脸数>1； -3: 人脸过小；-4: 角度过大；-5: 人脸超出边界 
			pLouFaceDlg->m_curVideoShowString = "unknown";
		}
	}
	else
	{
		pLouFaceDlg->m_curVideoShowString = "";
	}

	if (!(isRGBAlive && isIRAlive))
	{
		if (isRGBAlive && !isIRAlive)
		{
			pLouFaceDlg->m_curVideoShowString = "RGB活体,这是活的";
		}
		cvReleaseImage(&tempImage);
		//continue;
	}

	//特征提取
	detectRes = pLouFaceDlg->m_videoFaceEngine.PreExtractFeature(tempImage,
		faceFeature, pLouFaceDlg->m_curFaceInfo);

	cvReleaseImage(&tempImage);

	if (MOK != detectRes)
	{
		//continue;
	}

	int maxIndex = 0;
	MFloat maxThreshold = 0.0;
	int curIndex = 0;


	for each (auto regisFeature in pLouFaceDlg->m_featuresVec)
	{
		curIndex++;
		MFloat confidenceLevel = 0;
		MRESULT pairRes = pLouFaceDlg->m_videoFaceEngine.FacePairMatching(confidenceLevel, faceFeature, regisFeature);

		if (MOK == pairRes && confidenceLevel > maxThreshold)
		{
			maxThreshold = confidenceLevel;
			maxIndex = curIndex;
		}
	}

	if (atof(pLouFaceDlg->m_strEditThreshold) >= 0 &&
		maxThreshold >= atof(pLouFaceDlg->m_strEditThreshold) &&
		isRGBAlive && isIRAlive)
	{
		CString resStr;
		vector<CString>::const_iterator iter;
		CString actualFilename;
		UINT num = 0;
		for (iter = m_vFileName.begin();
		iter != m_vFileName.end();
			iter++)
		{
			//提取文件名
			if (maxIndex == num)
			{
				actualFilename = (*iter);
				actualFilename = actualFilename.Left(actualFilename.Find("."));
			}
			num++;
		}
		CurrentFaceID = actualFilename;
		resStr.Format("文件名：%s\n匹配度：%.4f\n", actualFilename.GetBuffer(actualFilename.GetLength()), maxThreshold);
		pLouFaceDlg->m_curVideoShowString = resStr + ",RGB活体";
		pLouFaceDlg->EditOut(pLouFaceDlg->m_curVideoShowString, TRUE);
		BOOL FindFlag = FALSE;

		//pDlg->ShowWindow(SW_SHOW);//显示对话框
		try
		{
			_variant_t RecordsAffected;
			pFaceManDlg->m_pRecordset.CreateInstance(__uuidof(Recordset));
			CString search_sql;
			search_sql = "SELECT * FROM cmdchoose";
			pFaceManDlg->m_pRecordset = pFaceManDlg->m_pConnection->Execute(search_sql.AllocSysString(), NULL, adCmdText);
			while (!pFaceManDlg->m_pRecordset->adoEOF)
			{
				CString id;
				CString chooseid;
				CString choosevalue;
				id = pFaceManDlg->m_pRecordset->GetCollect("id").bstrVal;
				//pDlg->m_list.InsertItem(0, id);
				chooseid = pFaceManDlg->m_pRecordset->GetCollect("chooseid").bstrVal;
				//pDlg->m_list.SetItemText(0, 1, chooseid);
				choosevalue = pFaceManDlg->m_pRecordset->GetCollect("choosevalue").bstrVal;
				//pDlg->m_list.SetItemText(0, 2, choosevalue);
				if (!id.CompareNoCase(CurrentFaceID))
				{
					pLouFaceDlg->EditOut("数据库内的信息姓名", TRUE);
					pLouFaceDlg->EditOut(" 姓名：" + chooseid + " 地址：" + choosevalue, TRUE);
					FindFlag = TRUE;
				}
				pFaceManDlg->m_pRecordset->MoveNext();
			}
			pFaceManDlg->m_pRecordset->Close();
		}
		catch (_com_error e)
		{
			//AfxMessageBox(_T("搜索失败！"));
			return ;
		}


	}
	else if (isRGBAlive)
	{
		pLouFaceDlg->m_curVideoShowString = "RGB活体";
		CurrentFaceID = "";
	}
	//}

	SafeFree(multiFaceInfo.faceOrient);
	SafeFree(multiFaceInfo.faceRect);
	SafeFree(faceFeature.feature);

}


//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	long lFrameType = pFrameInfo->nType;

	if (lFrameType == T_YV12)
	{
#if 1
		//int start = clock();
		IplImage* pImgYCrCb = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);//得到图像的Y分量  
		yv12toYUV(pImgYCrCb->imageData, pBuf, pFrameInfo->nWidth, pFrameInfo->nHeight, pImgYCrCb->widthStep);//得到全部RGB图像
		IplImage* pImg = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
		cvCvtColor(pImgYCrCb, pImg, CV_YCrCb2RGB);
		//人脸检测部分
		if(FaceDetect_flag)
			FaceDetect(pImg);
	
		//获取人脸特征
		if (FaceFeature_flag)
			FaceFeature(pImg);

#else
		IplImage* pImg = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 1);
		memcpy(pImg->imageData, pBuf, pFrameInfo->nWidth*pFrameInfo->nHeight);
#endif
		//printf("%d\n",end-start);
		//VideoStreamProcessing(pImg);
		//cvShowImage("IPCamera", pImg);
		//cvWaitKey(1);
#if 1
		cvReleaseImage(&pImgYCrCb);
			cvReleaseImage(&pImg);
#else
		cvReleaseImage(&pImg);
#endif
		//此时是YV12格式的视频数据，保存在pBuf中，可以fwrite(pBuf,nSize,1,Videofile);
		//fwrite(pBuf,nSize,1,fp);
	}
	/***************
	else if (lFrameType ==T_AUDIO16)
	{
	//此时是音频数据，数据保存在pBuf中，可以fwrite(pBuf,nSize,1,Audiofile);

	}
	else
	{
	}
	*******************/

}
//////////////////////////////////////////////////////////////////////////
///实时流回调
void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		if (nPort >= 0)
		{
			break; //同一路码流不需要多次调用开流接口
		}

		if (!PlayM4_GetPort(&nPort))
		{
			break;
		}
		if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}

		//设置解码回调函数 只解码不显示
		//if (!PlayM4_SetDecCallBack(nPort,DecCBFun))
		//{
		// 	dRet=PlayM4_GetLastError(nPort);
		// 	break;
		//}

		////设置解码回调函数 解码且显示
		if (!PlayM4_SetDecCallBackEx(nPort, DecCBFun, NULL, NULL))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}
		
		//打开视频解码
		if (!PlayM4_Play(nPort, hPlayWnd))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}

		//打开音频解码, 需要码流是复合流
		/*if (!PlayM4_PlaySound(nPort))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}*/
		break;

	case NET_DVR_STREAMDATA://视频流数据
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	default:
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	}
}
wchar_t * char2wchar(const char* cchar)
{
	wchar_t *m_wchar;
	int len = MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
	m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
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


// CLouFaceDlg 对话框

CLouFaceDlg::CLouFaceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLouFaceDlg::IDD, pParent),
	m_strEditThreshold(_T("")),
	m_curStaticImageFRSucceed(FALSE)
	, m_csPassword(_T(""))
	, m_nLoginPort(0)
	, m_csUserName(_T(""))
	, iPChannel(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_COMERA);
	pLouFaceDlg = this;
}

CLouFaceDlg::~CLouFaceDlg()//析构函数
{

}

void CLouFaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_IMAGE, m_ImageListCtrl);
	DDX_Control(pDX, IDC_EDIT_LOG, m_editLog);
	DDX_Text(pDX, IDC_EDIT_THRESHOLD, m_strEditThreshold);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctrlDeviceIP);
	DDX_Text(pDX, IDC_EDIT_Password, m_csPassword);
	DDX_Text(pDX, IDC_EDIT_Port, m_nLoginPort);
	DDX_Text(pDX, IDC_EDIT_Username, m_csUserName);
	DDX_Text(pDX, IDC_EDIT_IPCH, iPChannel);
}

BEGIN_MESSAGE_MAP(CLouFaceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_REGISTER, &CLouFaceDlg::OnBnClickedBtnRegister)
	ON_BN_CLICKED(IDC_BTN_RECOGNITION, &CLouFaceDlg::OnBnClickedBtnRecognition)
	ON_BN_CLICKED(IDC_BTN_COMPARE, &CLouFaceDlg::OnBnClickedBtnCompare)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CLouFaceDlg::OnBnClickedBtnClear)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CAMERA, &CLouFaceDlg::OnBnClickedBtnCamera)
	ON_EN_CHANGE(IDC_EDIT_THRESHOLD, &CLouFaceDlg::OnEnChangeEditThreshold)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK_Login, &CLouFaceDlg::OnBnClickedLogin)
	ON_BN_CLICKED(IDCANCEL, &CLouFaceDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_StartReview, &CLouFaceDlg::OnBnClickedStartreview)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CLouFaceDlg::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_Record, &CLouFaceDlg::OnBnClickedRecord)
	ON_BN_CLICKED(IDC_FACEMAN, &CLouFaceDlg::OnBnClickedFaceman)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_VIDEO_PLAYER, &CLouFaceDlg::OnBnClickedVideoPlayer)
	ON_BN_CLICKED(IDC_FACE_OPEN, &CLouFaceDlg::OnBnClickedFaceOpen)
	ON_BN_CLICKED(IDC_FACE_OPEN2, &CLouFaceDlg::OnBnClickedFaceOpen2)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CLouFaceDlg::OnBnClickedButtonTest)
END_MESSAGE_MAP()


// CLouFaceDlg 消息处理程序
BOOL CLouFaceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	m_IconImageList.Create(THUMBNAIL_WIDTH,
		THUMBNAIL_HEIGHT,
		ILC_COLOR32,
		0,
		1);

	m_ImageListCtrl.SetImageList(&m_IconImageList, LVSIL_NORMAL);

	char tag[MAX_PATH] = "x86_free";
	char appID[MAX_PATH] = "2XN3vCyhSUkKEBkz3URsFDqdNX2Q3wjh9Z8K5UTBebPg";
	char  sdkKey[MAX_PATH] = "GLbuo9Rv45fvQCzsREqbAqjGp6afWKosFVMhp2QL119h";
	char  activeKey[MAX_PATH] = "";
	char rgbLiveThreshold[MAX_PATH] = "0.5";
	char irLiveThreshold[MAX_PATH] = "0.7";
	char rgbCameraId[MAX_PATH] = "1";
	char irCameraId[MAX_PATH] = "0";

	//ReadSetting(appID, sdkKey, activeKey, tag, rgbLiveThreshold, irLiveThreshold, rgbCameraId, irCameraId);

	g_rgbCameraId = atoi(rgbCameraId);
	g_irCameraId = atoi(irCameraId);
	g_rgbLiveThreshold = atof(rgbLiveThreshold);
	g_irLiveThreshold = atof(irLiveThreshold);

	CString resStr = "";;

	MRESULT faceRes = m_imageFaceEngine.ActiveSDK(appID, sdkKey, activeKey);
	resStr.Format("激活结果: %d\n", faceRes);
	EditOut(resStr, TRUE);

	//获取激活文件信息
	ASF_ActiveFileInfo activeFileInfo = { 0 };
	m_imageFaceEngine.GetActiveFileInfo(activeFileInfo);

	if (faceRes == MOK)
	{
		resStr.Format("老铁，激活成功啦！");
		EditOut(resStr, TRUE);
		resStr = "";
		faceRes = m_imageFaceEngine.InitEngine(ASF_DETECT_MODE_IMAGE);//Image
		resStr.Format("IMAGE模式下初始化结果: %d", faceRes);
		EditOut(resStr, TRUE);

		resStr = "";
		faceRes = m_videoFaceEngine.InitEngine(ASF_DETECT_MODE_VIDEO);//Video
		resStr.Format("VIDEO模式下初始化结果: %d", faceRes);
		EditOut(resStr, TRUE);
	}
	else
	{
		resStr = "";
		resStr.Format("老铁，激活失败！");
		EditOut(resStr, TRUE);
	}

	//设置输入框位数
	((CEdit*)GetDlgItem(IDC_EDIT_THRESHOLD))->SetLimitText(4);
	m_strEditThreshold.Format("%.2f", Threshold);
	UpdateData(FALSE);

	GetDlgItem(IDC_STATIC_VIEW)->GetWindowRect(&m_windowViewRect);//获取窗口Rect矩形

	//人脸库按钮置灰
	GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CAMERA)->EnableWindow(FALSE);

	//编辑阈值置灰
	GetDlgItem(IDC_EDIT_THRESHOLD)->EnableWindow(FALSE);

	m_curStaticImageFeature.featureSize = FACE_FEATURE_SIZE;
	m_curStaticImageFeature.feature = (MByte *)malloc(m_curStaticImageFeature.featureSize * sizeof(MByte));


	m_Font = new CFont;

	SetTextFont(m_Font, 20, 20, "微软雅黑");
	UpdateData(TRUE);
	NET_DVR_Init();
	llRealHandle = -1;//获取实时预览信息的句柄
	iPChannel = 1;

	//用户注册信息
	m_ctrlDeviceIP.SetAddress(192, 168, 1, 107);
	m_csUserName = "admin";
	m_csPassword = "23456987l";
	m_nLoginPort = 8000;

	hPlayWnd = GetDlgItem(IDC_STATIC_VIEW3)->m_hWnd;//获取窗口的播放器句柄
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	UpdateData(FALSE);
	// TODO: 在此添加控件通知处理程序代码

	pLouFaceDlg->SetTimer(1, 100, NULL);
	//CBitmap bmp;
	//bmp.LoadBitmap(IDB_BACKGROUD);   //IDB_BACKGROUD是图片资源ID
	//m_brush.CreatePatternBrush(&bmp);

	//m_curVideoShowString = "欢迎进入娄瑞制作的人脸识别系统";
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread(NULL, 0, Voice_ThreadProc, (LPVOID)456, 0, &dwThreadID);
	CloseHandle(hThread);		CDC* pCDC = GetDlgItem(IDC_STATIC_VIEW)->GetDC();
	HDC hDC = pCDC->m_hDC;
	HBRUSH hBrush = ::CreateSolidBrush(RGB(240, 240, 240));
	::FillRect(hDC, CRect(0, 0, m_windowViewRect.Width(), m_windowViewRect.Height()), hBrush);
	DeleteObject(hBrush);

	pFaceManDlg = new FaceManDlg();
	pFaceManDlg->Create(IDD_FACEMAN, NULL);//设置对话框的资源ID和父窗口
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLouFaceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。


void CLouFaceDlg::OnPaint()
{
	if (IsIconic())//判断窗口是否处于最小化状态（点击了最小化按钮之后）。
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if (m_videoOpened)
		{
			lock_guard<mutex> lock(g_mutex);
			//文字显示框
			CRect rect(m_curFaceInfo.faceRect.left - 10, m_curFaceInfo.faceRect.top - 50,m_curFaceInfo.faceRect.right, m_curFaceInfo.faceRect.bottom);
			IplDrawToHDC(TRUE, m_curVideoImage, rect, IDC_STATIC_VIEW);
		}
		else
		{
			if (m_curStaticImage)
			{
				CRect rect((int)m_curStringShowPosition.X + 10, (int)m_curStringShowPosition.Y + 10, 40,40);//左上角，右下角
				IplDrawToHDC(FALSE, m_curStaticImage, rect, IDC_STATIC_VIEW4);
			}
		}

		CDialogEx::OnPaint();
	}


}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLouFaceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL IsImageGDIPLUSValid(CString filePath)
{
	Bitmap image(filePath.AllocSysString());

	if (image.GetFlags() == ImageFlagsNone)
		return FALSE;
	else
		return TRUE;
}


//加载缩略图片
void CLouFaceDlg::LoadThumbnailImages()
{
	m_bLoadIconThreadRunning = TRUE;

	GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CAMERA)->EnableWindow(FALSE);

	m_hLoadIconThread = CreateThread(
		NULL,
		0,
		RunLoadThumbnailThread,
		this,
		0,
		&m_dwLoadIconThreadID);

	if (m_hLoadIconThread == NULL)
	{
		::CloseHandle(m_hLoadIconThread);
	}
}
//注册照片
void CLouFaceDlg::OnBnClickedBtnRegister()
{
	// TODO:  在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(FALSE);
	m_folderPath = SelectFolder();
	if (m_folderPath == "")
	{
		GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(TRUE);
		return;
	}
	LoadThumbnailImages();//加载缩略图
}
//加载缩略图
unsigned long _stdcall RunLoadThumbnailThread(LPVOID lpParam)
{
	CLouFaceDlg* dialog = (CLouFaceDlg*)(lpParam);//获取对话框

	if (dialog == nullptr)
	{
		dialog->m_bLoadIconThreadRunning = FALSE;
		return 1;
	}

	if (dialog->m_folderPath == "")
	{
		vector<CString> m_vFileName;//文件名称数组->m_bLoadIconThreadRunning = FALSE;
		return 1;
	}

	int iExistFeatureSize = (int)dialog->m_featuresVec.size();//特征值数组大小

	CString resStr;
	resStr.Format("开始注册人脸库");
	dialog->EditOut(resStr, TRUE);

	CFileFind finder;

	CString m_strCurrentDirectory(dialog->m_folderPath);//最近选择的路径
	CString strWildCard(m_strCurrentDirectory);//通配符

	strWildCard += "\\*.*";//找路径中的所有类型的文件

	BOOL bWorking = finder.FindFile(strWildCard);//如果成功返回0

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots() || finder.IsDirectory())//如果是目录跳出本次循环
		{
			continue;
		}

		CString filePath = finder.GetFileName();

		if (IsImageGDIPLUSValid(m_strCurrentDirectory + _T("\\") + filePath))//是否是图片
		{
			m_vFileName.push_back(filePath);//Adds a new last element.
		}
	}
	
	resStr.Format("已选择图片张数: %d", m_vFileName.size());
	dialog->EditOut(resStr, TRUE);

	dialog->GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(FALSE);
	dialog->GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(FALSE);
	dialog->GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(FALSE);

	if (dialog->GetDlgItem(IDC_BTN_RECOGNITION)->IsWindowEnabled())
	{
		dialog->GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(FALSE);
	}

	vector<CString>::const_iterator iter;

	int actualIndex = iExistFeatureSize;

	for (iter = m_vFileName.begin();
		iter != m_vFileName.end();
		iter++)
	{
		if (!dialog->m_bLoadIconThreadRunning)
		{
			dialog->m_bLoadIconThreadRunning = FALSE;
			return 1;
		}

		CString imagePath;
		imagePath.Empty();
		imagePath.Format("%s\\%s", m_strCurrentDirectory, *iter);

		USES_CONVERSION;
		IplImage* originImage = cvLoadImage(T2A(imagePath.GetBuffer(0)));
		imagePath.ReleaseBuffer();

		if (!originImage)
		{
			cvReleaseImage(&originImage);
			continue;
		}

		//FD 
		ASF_SingleFaceInfo faceInfo = { 0 };
		MRESULT detectRes = dialog->m_imageFaceEngine.PreDetectFace(originImage, faceInfo, true);
		if (MOK != detectRes)
		{
			cvReleaseImage(&originImage);
			continue;
		}

		//FR
		ASF_FaceFeature faceFeature = { 0 };
		faceFeature.featureSize = FACE_FEATURE_SIZE;
		faceFeature.feature = (MByte *)malloc(faceFeature.featureSize * sizeof(MByte));
		detectRes = dialog->m_imageFaceEngine.PreExtractFeature(originImage, faceFeature, faceInfo);

		if (MOK != detectRes)
		{
			free(faceFeature.feature);
			cvReleaseImage(&originImage);
			continue;
		}

		Bitmap* image = IplImage2Bitmap(originImage);
		dialog->m_featuresVec.push_back(faceFeature);

		//计算缩略图显示位置
		int sourceWidth = image->GetWidth();
		int sourceHeight = image->GetHeight();

		int destX = 0;
		int destY = 0;

		float nPercent = 0;
		float nPercentW = ((float)THUMBNAIL_WIDTH / (float)sourceWidth);;
		float nPercentH = ((float)THUMBNAIL_HEIGHT / (float)sourceHeight);

		if (nPercentH < nPercentW)
		{
			nPercent = nPercentH;
			destX = (int)((THUMBNAIL_WIDTH - (sourceWidth * nPercent)) / 2);
		}
		else
		{
			nPercent = nPercentW;
			destY = (int)((THUMBNAIL_HEIGHT - (sourceHeight * nPercent)) / 2);
		}

		int destWidth = (int)(sourceWidth * nPercent);
		int destHeight = (int)(sourceHeight * nPercent);
		//提取文件名
		CString actualFilename = (*iter);
		actualFilename=actualFilename.Left(actualFilename.Find("."));
		dialog->m_ImageListCtrl.InsertItem(actualIndex, (LPCTSTR)(actualFilename), actualIndex);

		actualIndex++;

		Bitmap* bmPhoto = new Bitmap(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, PixelFormat24bppRGB);

		bmPhoto->SetResolution(image->GetHorizontalResolution(), image->GetVerticalResolution());

		Graphics *grPhoto = Graphics::FromImage(bmPhoto);
		Gdiplus::Color colorW(255, 255, 255, 255);
		grPhoto->Clear(colorW);
		grPhoto->SetInterpolationMode(InterpolationModeHighQualityBicubic);
		grPhoto->DrawImage(image, Gdiplus::Rect(destX, destY, destWidth, destHeight));

		HBITMAP hbmReturn = NULL;
		bmPhoto->GetHBITMAP(colorW, &hbmReturn);

		CBitmap Bmp1;
		Bmp1.Attach(hbmReturn);

		dialog->m_IconImageList.Add(&Bmp1, RGB(0, 0, 0));

		delete grPhoto;
		delete bmPhoto;
		Bmp1.Detach();
		DeleteObject(hbmReturn);

		dialog->m_ImageListCtrl.RedrawItems(actualIndex, actualIndex);

		//重绘
		if (actualIndex % 10 == 0)
		{
			dialog->m_ImageListCtrl.SetRedraw(TRUE);
			dialog->m_ImageListCtrl.Invalidate();
			dialog->m_ImageListCtrl.EnsureVisible(actualIndex - 1, FALSE);
		}

		cvReleaseImage(&originImage);
		delete image;
	}

	resStr.Format("成功注册图片张数: %d", actualIndex - iExistFeatureSize);
	dialog->EditOut(resStr, TRUE);

	dialog->m_ImageListCtrl.SetRedraw(TRUE);
	dialog->m_ImageListCtrl.Invalidate();
	dialog->m_ImageListCtrl.EnsureVisible(actualIndex - 1, FALSE);

	if (dialog->m_featuresVec.empty())
	{

	}

	//注册人脸库后按钮重置
	dialog->GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(TRUE);

	if (!dialog->m_videoOpened)
	{
		dialog->GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(TRUE);
		dialog->GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(TRUE);
	}
	else
	{
		dialog->GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(FALSE);
	}

	dialog->GetDlgItem(IDC_BTN_CAMERA)->EnableWindow(TRUE);
	dialog->GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(TRUE);
	dialog->GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(TRUE);

	dialog->m_bLoadIconThreadRunning = FALSE;

	return 0;
}
//选择文件夹
CString CLouFaceDlg::SelectFolder()
{
	TCHAR           szFolderPath[MAX_PATH] = { 0 };
	CString         strFolderPath = TEXT("");

	BROWSEINFO      sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = _T("请选择一个文件夹：");
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	sInfo.lpfn = NULL;

	// 显示文件夹选择对话框  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// 取得文件夹名  
		if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
		{
			strFolderPath = szFolderPath;
		}
	}
	if (lpidlBrowse != NULL)
	{
		::CoTaskMemFree(lpidlBrowse);
	}

	return strFolderPath;
}
//选择识别图片并显示到窗口
void CLouFaceDlg::OnBnClickedBtnRecognition()
{
	//获取打开图片的全路径
	CFileDialog fileDlg(TRUE, _T("bmp"), NULL, 0, _T("Picture Files|*.jpg;*.jpeg;*.png;*.bmp;||"), NULL);
	fileDlg.DoModal();//显示一个Windows常用文件对话框
	CString strFilePath;//文件路径
	strFilePath = fileDlg.GetPathName();

	if (strFilePath == _T(""))
		return;
	//从路径加载图像
	USES_CONVERSION;
	IplImage* image = cvLoadImage(T2A(strFilePath.GetBuffer(0)));//用来表示图像，其中Ipl是Intel Image Processing Library的简写。
	strFilePath.ReleaseBuffer();//结束对由GetBuffer分配的缓冲区的使用。
	if (!image)
	{
		cvReleaseImage(&image);
		return;
	}
	if (m_curStaticImage)
	{
		cvReleaseImage(&m_curStaticImage);
		m_curStaticImage = NULL;
	}
	m_curStaticImage = cvCloneImage(image);//制作图像的完整拷贝
	cvReleaseImage(&image);
	StaticImageFaceOp(m_curStaticImage);//显示图片
	GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(TRUE);
}
//对于图片的人脸识别处理
MRESULT CLouFaceDlg::StaticImageFaceOp(IplImage* image)  
{
	Gdiplus::Rect showRect;//图片在窗口显示区域
	CalculateShowPositon(image, showRect);//根据图片计算显示区域(符合控件大小）
	m_curImageShowRect = showRect;//选中图片的显示区域
	//FD
	ASF_SingleFaceInfo faceInfo = { 0 };//单一人脸信息
	MRESULT detectRes = m_imageFaceEngine.PreDetectFace(image, faceInfo, true);//人脸的检测

	//初始化
	m_curStaticShowAgeGenderString = "";//选中图片的年龄字符串
	m_curStaticShowCmpString = "";//选中图片对比字符串
	m_curFaceShowRect = Rect(0, 0, 0, 0);//脸部显示区域初始化

	SendMessage(WM_PAINT);//加载图片到窗口

	if (MOK == detectRes)//检测到人脸
	{
		//show rect
		int n_top = showRect.Height*faceInfo.faceRect.top / image->height;
		int n_bottom = showRect.Height*faceInfo.faceRect.bottom / image->height;
		int n_left = showRect.Width*faceInfo.faceRect.left / image->width;
		int n_right = showRect.Width*faceInfo.faceRect.right / image->width;

		m_curFaceShowRect.X = n_left + showRect.X;
		m_curFaceShowRect.Y = n_top + showRect.Y;
		m_curFaceShowRect.Width = n_right - n_left;
		m_curFaceShowRect.Height = n_bottom - n_top;

		//显示文字在图片左上角
		m_curStringShowPosition.X = (REAL)(showRect.X);
		m_curStringShowPosition.Y = (REAL)(showRect.Y);

		//age gender
		ASF_MultiFaceInfo multiFaceInfo = { 0 };
		multiFaceInfo.faceOrient = (MInt32*)malloc(sizeof(MInt32));
		multiFaceInfo.faceRect = (MRECT*)malloc(sizeof(MRECT));

		multiFaceInfo.faceNum = 1;
		multiFaceInfo.faceOrient[0] = faceInfo.faceOrient;
		multiFaceInfo.faceRect[0] = faceInfo.faceRect;

		ASF_AgeInfo ageInfo = { 0 };
		ASF_GenderInfo genderInfo = { 0 };
		ASF_Face3DAngle angleInfo = { 0 };
		ASF_LivenessInfo liveNessInfo = { 0 };

		//age 、gender 、3d angle 信息
		detectRes = m_imageFaceEngine.FaceASFProcess(multiFaceInfo, image,
			ageInfo, genderInfo, angleInfo, liveNessInfo);

		if (MOK == detectRes)
		{
			CString showStr;
			showStr.Format("年龄:%d,性别:%s,活体:%s", ageInfo.ageArray[0], genderInfo.genderArray[0] == 0 ? "男" : "女",
				liveNessInfo.isLive[0] == 1 ? "是" : "否");
			m_curStaticShowAgeGenderString = showStr;
			EditOut(showStr, TRUE);
		}
		else
		{
			m_curStaticShowAgeGenderString = "";
		}

		SendMessage(WM_PAINT);

		CWnd *pWnd = GetDlgItem(IDC_STATIC_VIEW4);
		//SaveDC2BMP(pWnd->GetSafeHwnd(), _T("static_bg.bmp"));
		SaveCDC2BMP(pWnd, _T("static_bg.bmp"), m_curFaceShowRect);
		EditOut("保存完成！",TRUE);



		free(multiFaceInfo.faceRect);
		free(multiFaceInfo.faceOrient);

		//FR
		detectRes = m_imageFaceEngine.PreExtractFeature(image, m_curStaticImageFeature, faceInfo);

		if (MOK == detectRes)
		{
			m_curStaticImageFRSucceed = TRUE;
		}
		else//提取特征不成功
		{
			m_curStaticImageFRSucceed = FALSE;
			CString resStr;
			resStr.Format("特征提取失败");
			EditOut(resStr, TRUE);
			return -1;
		}
		return MOK;
	}
	else
	{
		m_curStaticImageFRSucceed = FALSE;

		CString resStr;
		resStr.Format("未检测到人脸");
		EditOut(resStr, TRUE);
		return -1;
	}
}
//编辑输出
void CLouFaceDlg::EditOut(CString str, bool add_endl)
{
	if (add_endl)
		str += "\r\n";
	int iLen = m_editLog.GetWindowTextLength();
	m_editLog.SetSel(iLen, iLen, TRUE);
	m_editLog.ReplaceSel(str, FALSE);
}
//bitmap转iplimage
IplImage* Bitmap2IplImage(Bitmap* pBitmap)
{
	if (!pBitmap)
		return NULL;

	int w = pBitmap->GetWidth();
	int h = pBitmap->GetHeight();

	BitmapData bmpData;
	Gdiplus::Rect rect(0, 0, w, h);
	pBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bmpData);
	BYTE* temp = (bmpData.Stride > 0) ? ((BYTE*)bmpData.Scan0) : ((BYTE*)bmpData.Scan0 + bmpData.Stride*(h - 1));

	IplImage* pIplImg = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	if (!pIplImg)
	{
		pBitmap->UnlockBits(&bmpData);
		return NULL;
	}

	memcpy(pIplImg->imageData, temp, abs(bmpData.Stride)*bmpData.Height);
	pBitmap->UnlockBits(&bmpData);

	//判断Top-Down or Bottom-Up
	if (bmpData.Stride < 0)
		cvFlip(pIplImg, pIplImg);

	return pIplImg;
}
// pBitmap 同样需要外部释放
Bitmap* IplImage2Bitmap(const IplImage* pIplImg)
{
	if (!pIplImg)
		return NULL;

	Bitmap *pBitmap = new Bitmap(pIplImg->width, pIplImg->height, PixelFormat24bppRGB);
	if (!pBitmap)
		return NULL;

	BitmapData bmpData;
	Gdiplus::Rect rect(0, 0, pIplImg->width, pIplImg->height);
	pBitmap->LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bmpData);
	//BYTE *pByte = (BYTE*)bmpData.Scan0;

	if (pIplImg->widthStep == bmpData.Stride) //likely
		memcpy(bmpData.Scan0, pIplImg->imageDataOrigin, pIplImg->imageSize);

	pBitmap->UnlockBits(&bmpData);
	return pBitmap;
}
//人脸比对
void CLouFaceDlg::OnBnClickedBtnCompare()
{
	// TODO:  在此添加控件通知处理程序代码

	if (!m_curStaticImageFRSucceed)
	{
		AfxMessageBox(_T("人脸比对失败，请重新选择识别照!"));
		return;
	}

	if (m_featuresVec.size() == 0)
	{
		AfxMessageBox(_T("还未选择注册图片！"));
		return;
	}

	int maxIndex = 1;//默认1号开始
	MFloat maxThreshold = 0.0;
	int curIndex = 0;

	//FR 比对
	for each (auto regisFeature in m_featuresVec)
	{
		curIndex++;
		MFloat confidenceLevel = 0;//匹配度
		// 可以选择比对模型，人证模型推荐阈值：0.82 生活照模型推荐阈值：0.80
		MRESULT pairRes = m_imageFaceEngine.FacePairMatching(confidenceLevel, m_curStaticImageFeature, regisFeature);
		if (MOK == pairRes &&confidenceLevel > maxThreshold)//寻找匹配度最高的
		{
			maxThreshold = confidenceLevel;
			maxIndex = curIndex;
		}
	}
	//获取第maxIndex个图片的文件名
	CFileFind finder;

	CString m_strCurrentDirectory(m_folderPath);//最近选择的路径
	CString strWildCard(m_strCurrentDirectory);//通配符
	//vector<CString> m_vFileName;//文件名称数组
	strWildCard += "\\*.*";//找路径中的所有类型的文件

	BOOL bWorking = finder.FindFile(strWildCard);//如果成功返回0

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots() || finder.IsDirectory())//如果是目录跳出本次循环
		{
			continue;
		}

		CString filePath = finder.GetFileName();

		if (IsImageGDIPLUSValid(m_strCurrentDirectory + _T("\\") + filePath))//是否是图片
		{
			m_vFileName.push_back(filePath);//Adds a new last element.
		}
	}
	vector<CString>::const_iterator iter;
	CString actualFilename;
	UINT num = 0;
	for (iter =m_vFileName.begin();
	iter != m_vFileName.end();
		iter++)
	{
		//提取文件名
		if (maxIndex == num+1)
		{
			actualFilename = (*iter);
			actualFilename = actualFilename.Left(actualFilename.Find("."));
		}
		num++;
	}
	CString resStr;

	//显示结果		
	resStr.Format("文件名：%s\n匹配度：%.4f\n",actualFilename.GetBuffer(actualFilename.GetLength()), maxThreshold);
	actualFilename.ReleaseBuffer();
	EditOut(resStr, TRUE);
	m_curStaticShowCmpString = resStr;
	SendMessage(WM_PAINT);

	resStr.Format("比对结束");
	EditOut(resStr, TRUE);
}
//还原窗口
void CLouFaceDlg::OnBnClickedBtnClear()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_videoOpened)
	{
		AfxMessageBox(_T("请先关闭摄像头！"));
		return;
	}

	// 注册人脸库按钮置灰
	GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CAMERA)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(FALSE);

	//清理原有的图片以及特征
	ClearRegisterImages();

	GetDlgItem(IDC_BTN_REGISTER)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CAMERA)->EnableWindow(TRUE);

}

BOOL CLouFaceDlg::TerminateLoadThread()
{
	while (m_bLoadIconThreadRunning)
	{
		MSG message;
		while (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	}

	::CloseHandle(m_hLoadIconThread);

	return TRUE;
}
//注册照片
BOOL CLouFaceDlg::ClearRegisterImages()
{
	if (m_bLoadIconThreadRunning)
	{
		TerminateLoadThread();
	}
	else
	{
		m_bClearFeatureThreadRunning = TRUE;

		m_hClearFeatureThread = CreateThread(
			NULL,
			0,
			ClearFaceFeatureOperation,//
			this,
			0,
			&m_dwClearFeatureThreadID);

		if (m_dwClearFeatureThreadID == NULL)
		{
			::CloseHandle(m_hClearFeatureThread);
		}
	}
	return 0;
}
//计算显示的Position
BOOL CLouFaceDlg::CalculateShowPositon(IplImage*curSelectImage, Gdiplus::Rect& showRect)
{
	//计算实际显示宽高
	int actualWidth = 0;//实际宽
	int actualHeight = 0;//实际高

	int imageWidth = curSelectImage->width;//原始图像宽
	int imageHeight = curSelectImage->height;//原始图像高

	int windowWidth = m_windowViewRect.Width();//窗口宽
	int windowHeight = m_windowViewRect.Height();//窗口高

	int paddingLeft = 0;//左边边距
	int paddingTop = 0;//顶部边距

	//以宽为基准的高
	actualHeight = windowWidth*imageHeight / imageWidth;
	if (actualHeight > windowHeight)
	{
		//以高为基准的宽
		actualWidth = windowHeight*imageWidth / imageHeight;
		actualHeight = windowHeight;
	}
	else
	{
		actualWidth = windowWidth;
	}

	paddingLeft = (windowWidth - actualWidth) / 2;
	paddingTop = (windowHeight - actualHeight) / 2;

	showRect.X = paddingLeft;
	showRect.Y = paddingTop;
	showRect.Width = actualWidth;
	showRect.Height = actualHeight;

	return 0;
}

void CLouFaceDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}


void CLouFaceDlg::OnBnClickedBtnCamera()
{
	// TODO: 在此添加控件通知处理程序代码

	CString btnLabel;

	GetDlgItem(IDC_BTN_CAMERA)->GetWindowText(btnLabel);

	//获取摄像头数量以及名称
	g_cameraNum = listDevices(g_cameraName);

	//防止太频繁点击按钮
	Sleep(3000);

	if (btnLabel == "启用摄像头")
	{

		GetDlgItem(IDC_EDIT_THRESHOLD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CAMERA)->SetWindowText("关闭摄像头");

		//FD 线程
		m_hFDThread = CreateThread(
			NULL,
			0,
			RunFaceDetectOperation,
			this,
			0,
			&m_dwFDThreadID);

		if (m_hFDThread == NULL)
		{
			::CloseHandle(m_hFDThread);
		}

		m_bFDThreadRunning = TRUE;

		//FR 线程
		m_hFRThread = CreateThread(
			NULL,
			0,
			RunFaceFeatureOperation,
			this,
			0,
			&m_dwFRThreadID);

		if (m_hFRThread == NULL)
		{
			::CloseHandle(m_hFRThread);
		}
		GetDlgItem(IDC_FACE_OPEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_FACE_OPEN2)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_THRESHOLD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(TRUE);
		GetDlgItem(IDC_FACE_OPEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_FACE_OPEN2)->EnableWindow(TRUE);
		//将之前存储的信息清除
		m_curFaceInfo = { 0 };
		m_curVideoShowString = "";
		{
			lock_guard<mutex> lock(g_mutex);
			if (m_curVideoImage)
			{
				cvReleaseImage(&m_curVideoImage);
				m_curVideoImage = NULL;
			}

			if (m_curIrVideoImage)
			{
				cvReleaseImage(&m_curIrVideoImage);
				m_curIrVideoImage = NULL;
			}
		}

		m_dataValid = false;
		m_videoOpened = false;

		Sleep(600);

		ClearShowWindow();

		if (m_hFDThread == NULL)
		{
			BOOL res = ::CloseHandle(m_hFDThread);
			if (!res)
			{
				GetLastError();
			}
		}

		m_bFDThreadRunning = FALSE;

		if (m_hFRThread == NULL)
		{
			::CloseHandle(m_hFRThread);
		}

		GetDlgItem(IDC_BTN_CAMERA)->SetWindowText("启用摄像头");

	}
}
//人脸检测操作
unsigned long _stdcall RunFaceDetectOperation(LPVOID lpParam)
{
	CLouFaceDlg* dialog = (CLouFaceDlg*)(lpParam);

	if (dialog == nullptr)
	{
		return 1;
	}

	cv::Mat irFrame;
	cv::VideoCapture irCapture;

	cv::Mat rgbFrame;
	cv::VideoCapture rgbCapture;
	if (g_cameraNum == 2)
	{
		if (!irCapture.isOpened())
		{
			if (rgbCapture.open(g_rgbCameraId) && irCapture.open(g_irCameraId))
				dialog->m_videoOpened = true;
		}

		if (!(rgbCapture.set(CV_CAP_PROP_FRAME_WIDTH, VIDEO_FRAME_DEFAULT_WIDTH) &&
			rgbCapture.set(CV_CAP_PROP_FRAME_HEIGHT, VIDEO_FRAME_DEFAULT_HEIGHT)))
		{
			AfxMessageBox(_T("RGB摄像头初始化失败！"));
			return 1;
		}

		if (!(irCapture.set(CV_CAP_PROP_FRAME_WIDTH, VIDEO_FRAME_DEFAULT_WIDTH) &&
			irCapture.set(CV_CAP_PROP_FRAME_HEIGHT, VIDEO_FRAME_DEFAULT_HEIGHT)))
		{
			AfxMessageBox(_T("IR摄像头初始化失败！"));
			return 1;
		}
	}
	else if (g_cameraNum == 1)
	{
		if (!rgbCapture.isOpened())//如果RGB摄像头没打开
		{
			bool res = rgbCapture.open(0);//打开RGB摄像头
			if (res)
				dialog->m_videoOpened = true;
		}

		if (!(rgbCapture.set(CV_CAP_PROP_FRAME_WIDTH, VIDEO_FRAME_DEFAULT_WIDTH) &&
			rgbCapture.set(CV_CAP_PROP_FRAME_HEIGHT, VIDEO_FRAME_DEFAULT_HEIGHT)))
		{
			dialog->EditOut("RGB摄像头初始化失败！",TRUE);
			//AfxMessageBox(_T("RGB摄像头初始化失败！"));
			return 1;
		}
	}
	else
	{
		dialog->EditOut("摄像头数量不支持！", TRUE);
		//AfxMessageBox(_T("摄像头数量不支持！"));
		return 1;
	}

	while (dialog->m_videoOpened)
	{
		if (g_cameraNum == 2)
		{
			irCapture >> irFrame;

			rgbCapture >> rgbFrame;

			ASF_SingleFaceInfo faceInfo = { 0 };

			IplImage rgbImage(rgbFrame);
			IplImage irImage(irFrame);

			MRESULT detectRes = dialog->m_videoFaceEngine.PreDetectFace(&rgbImage, faceInfo, true);
			/*FILE *fp = NULL;
			fp = fopen("rect.txt", "a+");
			if (fp)
			{
				fprintf(fp, "RGB: (%d %d %d %d)\n",
					faceInfo.faceRect.left, faceInfo.faceRect.top, faceInfo.faceRect.right, faceInfo.faceRect.bottom);
				fflush(fp);
				fclose(fp);
			}*/
			if (MOK == detectRes)
			{
				cvRectangle(&rgbImage, cvPoint(faceInfo.faceRect.left, faceInfo.faceRect.top),
					cvPoint(faceInfo.faceRect.right, faceInfo.faceRect.bottom), cvScalar(0, 0, 255), 2);

				cvRectangle(&irImage, cvPoint(faceInfo.faceRect.left, faceInfo.faceRect.top),
					cvPoint(faceInfo.faceRect.right, faceInfo.faceRect.bottom), cvScalar(0, 0, 255), 2);

				dialog->m_curFaceInfo = faceInfo;
				dialog->m_dataValid = true;
			}
			else
			{
				//没有人脸不要显示信息
				dialog->m_curVideoShowString = "";
				dialog->m_curIRVideoShowString = "";
				dialog->m_dataValid = false;
			}

			ASF_SingleFaceInfo irFaceInfo = { 0 };
			MRESULT irRes = dialog->m_videoFaceEngine.PreDetectFace(&irImage, irFaceInfo, false);
			if (irRes == MOK)
			{
				if (abs(faceInfo.faceRect.left - irFaceInfo.faceRect.left) < 20 &&
					abs(faceInfo.faceRect.top - irFaceInfo.faceRect.top) < 20 &&
					abs(faceInfo.faceRect.right - irFaceInfo.faceRect.right) < 20 &&
					abs(faceInfo.faceRect.bottom - irFaceInfo.faceRect.bottom) < 20)
				{
					dialog->m_irDataValid = true;
				}
				else
				{
					dialog->m_irDataValid = false;
				}
			}
			else
			{
				dialog->m_irDataValid = false;
			}

			//重新拷贝
			{
				lock_guard<mutex> lock(g_mutex);
				cvReleaseImage(&dialog->m_curVideoImage);
				dialog->m_curVideoImage = cvCloneImage(&rgbImage);

				cvReleaseImage(&dialog->m_curIrVideoImage);
				dialog->m_curIrVideoImage = cvCloneImage(&irImage);
			}
		}
		else if (g_cameraNum == 1)
		{
			rgbCapture >> rgbFrame;//获取一帧

			ASF_SingleFaceInfo faceInfo = { 0 };

			IplImage rgbImage(rgbFrame);

			MRESULT detectRes = dialog->m_videoFaceEngine.PreDetectFace(&rgbImage, faceInfo, true);
			if (MOK == detectRes)
			{
				cvRectangle(&rgbImage, cvPoint(faceInfo.faceRect.left, faceInfo.faceRect.top),
					cvPoint(faceInfo.faceRect.right, faceInfo.faceRect.bottom), cvScalar(0, 0, 255), 2);

				dialog->m_curFaceInfo = faceInfo;
				dialog->m_dataValid = true;
			}
			else
			{
				//没有人脸不要显示信息
				dialog->m_curVideoShowString = "";
				dialog->m_dataValid = false;
			}


			//重新拷贝
			{
				lock_guard<mutex> lock(g_mutex);
				cvReleaseImage(&dialog->m_curVideoImage);
				dialog->m_curVideoImage = cvCloneImage(&rgbImage);
			}
		}
		else
		{
			AfxMessageBox(_T("摄像头数量不支持！"));
		}
		
		dialog->SendMessage(WM_PAINT);
	}

	rgbCapture.release();
	irCapture.release();

	return 0;
}
//获取人脸特征操作
unsigned long _stdcall RunFaceFeatureOperation(LPVOID lpParam)
{
	CLouFaceDlg* dialog = (CLouFaceDlg*)(lpParam);

	if (dialog == nullptr)
	{
		return 1;
	}

	//设置活体检测阈值，sdk内部默认RGB:0.5 IR:0.7,可选择是否调用该接口
	dialog->m_imageFaceEngine.SetLivenessThreshold(g_rgbLiveThreshold, g_irLiveThreshold);

	//初始化特征
	ASF_FaceFeature faceFeature = { 0 };
	faceFeature.featureSize = FACE_FEATURE_SIZE;
	faceFeature.feature = (MByte *)malloc(faceFeature.featureSize * sizeof(MByte));

	ASF_MultiFaceInfo multiFaceInfo = { 0 };
	multiFaceInfo.faceOrient = (MInt32*)malloc(sizeof(MInt32));
	multiFaceInfo.faceRect = (MRECT*)malloc(sizeof(MRECT));

	while (dialog->m_bFDThreadRunning)
	{
		if (dialog->m_bLoadIconThreadRunning ||
			dialog->m_bClearFeatureThreadRunning)
		{
			//加载和清除注册库的过程中 不要显示信息
			dialog->m_curVideoShowString = "";
			continue;
		}

		if (!dialog->m_dataValid)
		{
			continue;
		}

		//先拷贝一份，防止读写冲突
		IplImage* tempImage = NULL;
		{
			lock_guard<mutex> lock(g_mutex);
			if (dialog->m_curVideoImage)
			{
				tempImage = cvCloneImage(dialog->m_curVideoImage);
			}
		}

		//发送一份到活体
		multiFaceInfo.faceNum = 1;
		multiFaceInfo.faceOrient[0] = dialog->m_curFaceInfo.faceOrient;
		multiFaceInfo.faceRect[0] = dialog->m_curFaceInfo.faceRect;

		ASF_AgeInfo ageInfo = { 0 };
		ASF_GenderInfo genderInfo = { 0 };
		ASF_Face3DAngle angleInfo = { 0 };
		ASF_LivenessInfo liveNessInfo = { 0 };

		//IR活体检测
		bool isIRAlive = false;
		if (g_cameraNum == 2)
		{
			IplImage* tempIRImage = NULL;
			lock_guard<mutex> lock(g_mutex);
			{
				if (dialog->m_curIrVideoImage)
				{
					tempIRImage = cvCloneImage(dialog->m_curIrVideoImage);
				}
			}
			
			if (dialog->m_irDataValid)
			{
				ASF_LivenessInfo irLiveNessInfo = { 0 };
				MRESULT irRes = dialog->m_imageFaceEngine.FaceASFProcess_IR(multiFaceInfo, tempIRImage, irLiveNessInfo);
				if (irRes == 0 && irLiveNessInfo.num > 0)
				{
					if (irLiveNessInfo.isLive[0] == 1)
					{
						dialog->m_curIRVideoShowString = "IR活体";
						isIRAlive = true;
					}
					else if (irLiveNessInfo.isLive[0] == 0)
					{
						dialog->m_curIRVideoShowString = "IR假体";
					}
					else
					{
						//-1：不确定；-2:传入人脸数>1； -3: 人脸过小；-4: 角度过大；-5: 人脸超出边界 
						dialog->m_curIRVideoShowString = "unknown";
					}
				}
				else
				{
					dialog->m_curIRVideoShowString = "";
				}
			}
			else
			{
				dialog->m_curIRVideoShowString = "";
			}

			cvReleaseImage(&tempIRImage);
		}
		else if (g_cameraNum == 1)
		{
			isIRAlive = true;
		}
		else
		{
			break;
		}

		//RGB属性检测
		MRESULT detectRes = dialog->m_imageFaceEngine.FaceASFProcess(multiFaceInfo, tempImage,
			ageInfo, genderInfo, angleInfo, liveNessInfo);

		bool isRGBAlive = false;
		if (detectRes == 0 && liveNessInfo.num > 0)
		{
			if (liveNessInfo.isLive[0] == 1)
			{
				isRGBAlive = true;
			}
			else if (liveNessInfo.isLive[0] == 0)
			{
				dialog->m_curVideoShowString = "RGB假体";
			}
			else
			{
				//-1：不确定；-2:传入人脸数>1； -3: 人脸过小；-4: 角度过大；-5: 人脸超出边界 
				dialog->m_curVideoShowString = "unknown";  
			}
		}
		else
		{
			dialog->m_curVideoShowString = "";
		}

		if (!(isRGBAlive && isIRAlive))
		{
			if (isRGBAlive && !isIRAlive)
			{
				dialog->m_curVideoShowString = "RGB活体";
			}
			cvReleaseImage(&tempImage);
			continue;
		}

		//特征提取
		detectRes = dialog->m_videoFaceEngine.PreExtractFeature(tempImage,
			faceFeature, dialog->m_curFaceInfo);

		cvReleaseImage(&tempImage);

		if (MOK != detectRes)
		{
			continue;
		}

		int maxIndex = 0;
		MFloat maxThreshold = 0.0;
		int curIndex = 0;

		if (dialog->m_bLoadIconThreadRunning ||
			dialog->m_bClearFeatureThreadRunning)
		{
			continue;
		}

		for each (auto regisFeature in dialog->m_featuresVec)
		{
			curIndex++;
			MFloat confidenceLevel = 0;
			MRESULT pairRes = dialog->m_videoFaceEngine.FacePairMatching(confidenceLevel, faceFeature, regisFeature);

			if (MOK == pairRes && confidenceLevel > maxThreshold)
			{
				maxThreshold = confidenceLevel;
				maxIndex = curIndex;
			}
		}

		if (atof(dialog->m_strEditThreshold) >= 0 &&
			maxThreshold >= atof(dialog->m_strEditThreshold) &&
			isRGBAlive && isIRAlive)
		{
			CString resStr;
			resStr.Format("%d号 :%.2f", maxIndex, maxThreshold);
			dialog->m_curVideoShowString = resStr + ",RGB活体";
		}
		else if (isRGBAlive)
		{
			dialog->m_curVideoShowString = "RGB活体";
		}
	}
	
	SafeFree(multiFaceInfo.faceOrient);
	SafeFree(multiFaceInfo.faceRect);
	SafeFree(faceFeature.feature);
	return 0;
}

//双缓存画图
void CLouFaceDlg::IplDrawToHDC(BOOL isVideoMode, IplImage* rgbImage, CRect& strShowRect, UINT ID)
{
	if (!rgbImage)
	{
		return;
	}

	CDC MemDC;//内存DC

	CClientDC pDc(GetDlgItem(ID));//获取的控件的ID

	//创建与窗口DC兼容的内存DC（MyDC）
	MemDC.CreateCompatibleDC(&pDc);


	IplImage* cutImg;//裁剪图片
	//裁剪视频/或图片
	if (m_curIrVideoImage)//红外视频
	{
		//红外图像的缩放并拷贝
		IplImage* shrinkIrImage = cvCreateImage(cvSize(m_curIrVideoImage->width / 3, m_curIrVideoImage->height / 3), m_curIrVideoImage->depth, m_curIrVideoImage->nChannels);
		cvResize(m_curIrVideoImage, shrinkIrImage, CV_INTER_AREA);

		//将IR图像融合到RGB图像上
		cv::Mat matRGBImage = cv::cvarrToMat(rgbImage);
		cv::Mat matIRImage = cv::cvarrToMat(shrinkIrImage);
		cv::Mat imageROI = matRGBImage(cv::Rect(10, 10, matIRImage.cols, matIRImage.rows));
		matIRImage.copyTo(imageROI);
		IplImage* roiImage = &IplImage(matRGBImage);	//浅拷贝

		//裁剪图片
		cutImg = cvCreateImage(cvSize(roiImage->width - (roiImage->width % 4), roiImage->height), IPL_DEPTH_8U, roiImage->nChannels);
		PicCutOut(roiImage, cutImg, 0, 0);
		cvReleaseImage(&shrinkIrImage);
	}
	else
	{
		cutImg = cvCreateImage(cvSize(rgbImage->width - (rgbImage->width % 4), rgbImage->height), IPL_DEPTH_8U, rgbImage->nChannels);
		//位深度：IPL_DEPTH_8U - 8位无符号整数
		PicCutOut(rgbImage, cutImg, 0, 0);//cutImg为目标
	}

	CBitmap* bmp = IplImage2CBitmap(cutImg);
	//把内存位图选进内存DC中用来保存在内存DC中绘制的图形
	CBitmap *oldbmp = MemDC.SelectObject(bmp);

	CPen pen(PS_SOLID, 4, RGB(240,240,240));
	pDc.SelectStockObject(NULL_BRUSH);

	pDc.SetBkMode(TRANSPARENT);
	pDc.SetTextColor(RGB(255, 255,255));

	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);

	//把内存DC中的图形粘贴到窗口中；
	pDc.SetStretchBltMode(HALFTONE);
	
	strShowRect.left = strShowRect.left < 0 ? 0 : strShowRect.left;
	strShowRect.top = strShowRect.top < 0 ? 0 : strShowRect.top;
	strShowRect.right = strShowRect.right > rect.right ? 0 : strShowRect.right;
	strShowRect.bottom = strShowRect.bottom > rect.bottom ? rect.bottom : strShowRect.bottom;




	if (isVideoMode)
	{
		pDc.StretchBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, rgbImage->width, rgbImage->height, SRCCOPY);

		//为了让文字不贴边
		strShowRect.left += 4;
		strShowRect.top += 4;

		//让文字不超出视频框
		GetDlgItem(ID)->SetFont(m_Font);

		SIZE size;
		GetTextExtentPoint32A(pDc, m_curVideoShowString, (int)strlen(m_curVideoShowString), &size);

		if (strShowRect.left + size.cx > rect.Width())
		{
			strShowRect.left = rect.Width() - size.cx;
		}
		if (strShowRect.top + size.cy > rect.Height())
		{
			strShowRect.top = rect.Height() - size.cy;
		}

		//画比对信息
		if (m_curVideoShowString == "RGB假体")
		{
			pDc.SetTextColor(RGB(255, 242, 0));
		}
		pDc.DrawText(m_curVideoShowString, &strShowRect, DT_TOP | DT_LEFT | DT_NOCLIP);

		if (m_curIRVideoShowString == "IR假体")
		{
			pDc.SetTextColor(RGB(255, 242, 0));
		}
		pDc.DrawText(m_curIRVideoShowString, CRect(20,20,100,100), DT_TOP | DT_LEFT | DT_NOCLIP);
	}
	else//图片部分
	{
		//图片由于尺寸不一致 ，需要重绘背景
		HBRUSH hBrush = ::CreateSolidBrush(RGB(240,240,240));
		::FillRect(pDc.m_hDC, CRect(0, 0, m_windowViewRect.Width(), m_windowViewRect.Height()), hBrush);
		/* hDC[in] Handle to the device context.    
		   lprc[in] Long pointer to a RECT structure that contains the logical coordinates of the rectangle to be filled.
		   hbr[in] Handle to the brush used to fill the rectangle.
		*/
		pDc.StretchBlt(m_curImageShowRect.X + 2, m_curImageShowRect.Y + 2,
			m_curImageShowRect.Width - 2, m_curImageShowRect.Height - 5, &MemDC, 0, 0, cutImg->width, cutImg->height, SRCCOPY);
		//将位图从源矩形复制到目标矩形中，在必要时拉伸或压缩位图以适应目标矩形的尺寸。
		Gdiplus::Graphics graphics(pDc.m_hDC);
		Gdiplus::Pen pen(Gdiplus::Color::Red, 2);
		graphics.DrawRectangle(&pen, m_curFaceShowRect);

		//画age gender信息
		pDc.DrawText(m_curStaticShowAgeGenderString, &strShowRect, DT_TOP | DT_LEFT | DT_NOCLIP);

		//将比对信息放在age gender信息下
		strShowRect.top += 20;
		strShowRect.bottom += 20;

		//让文字不超出视频框
		GetDlgItem(ID)->SetFont(m_Font);

		SIZE size;
		GetTextExtentPoint32A(pDc, m_curVideoShowString, (int)strlen(m_curVideoShowString), &size);

		if (strShowRect.left + size.cx > rect.Width())
		{
			strShowRect.left = rect.Width() - size.cx;
		}
		if (strShowRect.top + size.cy > rect.Height())
		{
			strShowRect.top = rect.Height() - size.cy;
		}

		//画比对信息
		pDc.DrawText(m_curStaticShowCmpString, &strShowRect, DT_TOP | DT_LEFT | DT_NOCLIP);
	}

	cvReleaseImage(&cutImg);

	//选进原来的位图，删除内存位图对象和内存DC
	MemDC.SelectObject(oldbmp);
	bmp->DeleteObject();
	MemDC.DeleteDC();

}


//图片格式转换
CBitmap* IplImage2CBitmap(const IplImage *img)
{
	if (!img)
	{
		return NULL;
	}

	CBitmap* bitmap = new CBitmap;//new一个CWnd对象
	BITMAPINFO bmpInfo;  //创建位图        
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = img->width;
	bmpInfo.bmiHeader.biHeight = img->origin ? abs(img->height) : -abs(img->height);//img->height;//高度
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	void *pArray = NULL;
	HBITMAP hbmp = CreateDIBSection(NULL, &bmpInfo, DIB_RGB_COLORS, &pArray, NULL, 0);//创建DIB，可直接写入、与设备无关，相当于创建一个位图窗口
	ASSERT(hbmp != NULL);
	UINT uiTotalBytes = img->width * img->height * 3;
	memcpy(pArray, img->imageData, uiTotalBytes);

	bitmap->Attach(hbmp);// 一个CWnd对象的HWND成员指向这个窗口句柄

	return bitmap;
}

void CLouFaceDlg::OnEnChangeEditThreshold()
{
	//更新阈值
	UpdateData(TRUE);
	if (atof(m_strEditThreshold) < 0)
	{
		AfxMessageBox(_T("阈值必须大于0！"));
		SetDlgItemTextA(IDC_EDIT_THRESHOLD, "0.80");
	}

}


void CLouFaceDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_videoOpened)
	{
		AfxMessageBox(_T("请先关闭摄像头！"));
		return;
	}

	CDialogEx::OnClose();

	m_bLoadIconThreadRunning = FALSE;
	TerminateLoadThread();
	m_bClearFeatureThreadRunning = FALSE;
	ClearRegisterImages();

	m_videoOpened = false;
	Sleep(500);

	m_bFDThreadRunning = FALSE;

	::CloseHandle(m_hFDThread);
	::CloseHandle(m_hFRThread);

	Sleep(500);

	m_imageFaceEngine.UnInitEngine();
	m_videoFaceEngine.UnInitEngine();
}

void CLouFaceDlg::ClearShowWindow()
{
	//清空背景
	CDC* pCDC = GetDlgItem(IDC_STATIC_VIEW)->GetDC();
	HDC hDC = pCDC->m_hDC;
	HBRUSH hBrush = ::CreateSolidBrush(RGB(240, 240, 240));
	::FillRect(hDC, CRect(0, 0, m_windowViewRect.Width(), m_windowViewRect.Height()), hBrush);
	pCDC = GetDlgItem(IDC_STATIC_VIEW3)->GetDC();
	hDC = pCDC->m_hDC;
	::FillRect(hDC, CRect(0, 0, m_windowViewRect.Width(), m_windowViewRect.Height()), hBrush);
	DeleteObject(hBrush);


}


unsigned long _stdcall ClearFaceFeatureOperation(LPVOID lpParam)
{
	CLouFaceDlg* dialog = (CLouFaceDlg*)(lpParam);

	if (dialog == nullptr)
	{
		return 1;
	}

	int iImageCount = dialog->m_IconImageList.GetImageCount();

	dialog->m_IconImageList.Remove(-1);

	dialog->m_ImageListCtrl.DeleteAllItems();

	iImageCount = dialog->m_IconImageList.SetImageCount(0);

	//清除特征
	for (auto feature : dialog->m_featuresVec)
	{
		free(feature.feature);
	}

	dialog->m_featuresVec.clear();

	dialog->m_bClearFeatureThreadRunning = FALSE;

	return 0;
}


BOOL SetTextFont(CFont* font, int fontHeight, int fontWidth, string fontStyle)
{
	return font->CreateFont(
		fontHeight,					// nHeight
		fontWidth,					// nWidth
		0,							// nEscapement
		0,							// nOrientation
		FW_BOLD,					// nWeight
		FALSE,						// bItalic
		FALSE,						// bUnderline
		0,							// cStrikeOut
		DEFAULT_CHARSET,				// nCharSet
		OUT_DEFAULT_PRECIS,			// nOutPrecision
		CLIP_DEFAULT_PRECIS,			// nClipPrecision
		DEFAULT_QUALITY,				// nQuality
		DEFAULT_PITCH | FF_SWISS,		// nPitchAndFamily
		fontStyle.c_str());			// lpszFacename
}

//列出硬件设备
int listDevices(vector<string>& list)
{
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(
		CLSID_SystemDeviceEnum,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum)
	);

	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
		if (hr == S_OK) {

			IMoniker *pMoniker = NULL;
			while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
			{
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue; // Skip this one, maybe the next one will work.
				}

				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);
				if (FAILED(hr))
				{
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				}

				if (SUCCEEDED(hr))
				{
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					int count = 0;
					char tmp[255] = { 0 };
					while (varName.bstrVal[count] != 0x00 && count < 255)
					{
						tmp[count] = (char)varName.bstrVal[count];
						count++;
					}
					list.push_back(tmp);
				}

				pPropBag->Release();
				pPropBag = NULL;
				pMoniker->Release();
				pMoniker = NULL;

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;
			pEnum->Release();
			pEnum = NULL;
		}
	}
	return deviceCounter;
}

void ReadSetting(char* appID, char* sdkKey, char* activeKey, char* tag, 
	char* rgbLiveThreshold, char* irLiveThreshold, char* rgbCameraId, char* irCameraId)
{
	CString iniPath = _T(".\\setting.ini");

	char resultStr[MAX_PATH] = "";

	GetPrivateProfileStringA("tag", _T("tag"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(tag, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("APPID"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(appID, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("SDKKEY"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(sdkKey, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("ACTIVE_KEY"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(activeKey, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("rgbLiveThreshold"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(rgbLiveThreshold, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("irLiveThreshold"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(irLiveThreshold, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("rgbCameraId"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(rgbCameraId, resultStr, MAX_PATH);

	GetPrivateProfileStringA(tag, _T("irCameraId"), NULL, resultStr, MAX_PATH, iniPath);
	memcpy(irCameraId, resultStr, MAX_PATH);
}


void CLouFaceDlg::OnBnClickedLogin()
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(TRUE);
	DWORD dwDeviceIP;
	char DeviceIP[16] = { 0 };
	char cUserName[100] = { 0 };
	char cPassword[100] = { 0 };
	CString csTemp;

	m_ctrlDeviceIP.GetAddress(dwDeviceIP);//将此时IP地址控件里面的值赋值给dwDeviceIP
	csTemp = IPToStr(dwDeviceIP);
	sprintf(DeviceIP, "%s", csTemp.GetBuffer(0));
	strncpy(cUserName, m_csUserName, MAX_NAMELEN);
	strncpy(cPassword, m_csPassword, PASSWD_LEN);

	//登录设备，需要设备IP、端口、用户名、密码 Login the device
	NET_DVR_DEVICEINFO_V30 devInfo;
	lUserID = NET_DVR_Login_V30(DeviceIP, m_nLoginPort, cUserName, cPassword, &devInfo);

	DWORD dwReturned = 0;

	if (lUserID<0)
		AfxMessageBox("糟糕，登录失败!");
	else
	{
	//	AfxMessageBox("恭喜，恭喜，登录成功啦!");
		EditOut("恭喜，恭喜，登录成功啦!\n", FALSE);
		GetDlgItem(IDOK_Login)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}


void CLouFaceDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (llRealHandle >= 0)
	{
		AfxMessageBox("亲，请停止预览啊!");
		return;
	}

	NET_DVR_Logout_V30(lUserID);
	NET_DVR_Cleanup();
	CDialogEx::OnCancel();
}


void CLouFaceDlg::OnBnClickedStartreview()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet1;
	UpdateData(TRUE);
	if (llRealHandle<0)
	{
		UpdateData(TRUE);

		if (lUserID<0)
		{
			ErrorNum.Format("Login failed Error number ：%d\n", NET_DVR_GetLastError());
			OutputDebugString(ErrorNum);
		}

		//////////////////////////////////////////////////////////////////////////
		NET_DVR_CLIENTINFO ClientInfo;
		ClientInfo.lChannel = iPChannel; //Channel number 设备通道号
		ClientInfo.hPlayWnd = NULL;  //窗口为空，设备SDK不解码只取流
		ClientInfo.lLinkMode = 0;    //Main Stream
		ClientInfo.sMultiCastIP = NULL;

		//预览取流 
		llRealHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack, NULL, TRUE);

		if (llRealHandle<0)
		{
			ErrorNum.Format("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
			AfxMessageBox("请先登录啊！");
			return;
		}
		//m_bFDThreadRunning = TRUE;
		GetDlgItem(IDC_StartReview)->SetWindowText("停止预览");
		EditOut("已开启IP摄像头预览", FALSE);
	
	}
	else
	{
		
		//停止预览
		if (NET_DVR_StopRealPlay(llRealHandle))
		{
			bRet1 = NET_DVR_GetLastError();
		}
		llRealHandle = -1;

		//停止解码
		if (nPort>-1)
		{
			if (!PlayM4_StopSound())
			{
				bRet1 = PlayM4_GetLastError(nPort);
			}
			if (!PlayM4_Stop(nPort))
			{
				bRet1 = PlayM4_GetLastError(nPort);
			}
			if (!PlayM4_CloseStream(nPort))
			{
				bRet1 = PlayM4_GetLastError(nPort);
			}
			PlayM4_FreePort(nPort);
			nPort = -1;
		}

		//关闭保存解码后数据的音视频文件
		if (Audiofile != NULL)
		{
			fclose(Audiofile);
			Audiofile = NULL;
		}

		if (Videofile != NULL)
		{
			fclose(Videofile);
			Videofile = NULL;
		}

		GetDlgItem(IDOK_Login)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_THRESHOLD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(TRUE);

		//将之前存储的信息清除
		m_curFaceInfo = { 0 };
		m_curVideoShowString = "";
		{
			lock_guard<mutex> lock(g_mutex);
			if (m_curVideoImage)
			{
				cvReleaseImage(&m_curVideoImage);
				m_curVideoImage = NULL;
			}

			if (m_curIrVideoImage)
			{
				cvReleaseImage(&m_curIrVideoImage);
				m_curIrVideoImage = NULL;
			}
		}

		m_dataValid = false;
		m_videoOpened = false;

		Sleep(600);

		ClearShowWindow();

		//m_bFDThreadRunning = FALSE;
		GetDlgItem(IDC_StartReview)->SetWindowText("开始预览");
		EditOut("已关闭IP摄像头预览", FALSE);

	}
	

	UpdateData(FALSE);
}

UINT __cdecl RunIPCameraFaceDetectOperation(LPVOID lpParameter)
{
	//从路径加载图像
	USES_CONVERSION;
	CString strFilePath;//文件路径
	strFilePath = A2T((char*)lpParameter);

	if (strFilePath == _T(""))
		return 0;
	IplImage* image = cvLoadImage(T2A(strFilePath.GetBuffer(0)));//用来表示图像，其中Ipl是Intel Image Processing Library的简写。
	strFilePath.ReleaseBuffer();//结束对由GetBuffer分配的缓冲区的使用。
	if (!image)
	{
		cvReleaseImage(&image);
		return 0;
	}

	if (pLouFaceDlg->m_curStaticImage)
	{
		cvReleaseImage(&pLouFaceDlg->m_curStaticImage);
		pLouFaceDlg->m_curStaticImage = NULL;
	}

	pLouFaceDlg->m_curStaticImage = cvCloneImage(image);//制作图像的完整拷贝
	cvReleaseImage(&image);

	pLouFaceDlg->StaticImageFaceOp(pLouFaceDlg->m_curStaticImage);//显示图片

	pLouFaceDlg->GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(TRUE);
	return 0;
}

void CLouFaceDlg::OnBnClickedButtonCapture()
{
	// TODO: 在此添加控件通知处理程序代码
	FILE *file = NULL;

	NET_DVR_JPEGPARA JpegPara;
	JpegPara.wPicQuality = 0;
	JpegPara.wPicSize = 0xff;

	char *JpegPicBuffer = new char[352 * 288 * 2];
	//这里的缓冲区大小需要根据抓图的分辨率大小自己调节，建议设置成2*图片的分辨率宽*图片的分辨率高

	DWORD  SizeReturned = 0;
	BOOL bRet = NET_DVR_CaptureJPEGPicture_NEW(lUserID, iPChannel, &JpegPara, JpegPicBuffer, 352 * 288 * 2, &SizeReturned);

	if (!bRet)
	{
		ErrorNum.Format("NET_DVR_CaptureJPEGPicture_NEW failed! Error number: %d\n", NET_DVR_GetLastError());
		AfxMessageBox(ErrorNum);

		return;
	}
	else
	{
		EditOut("NET_DVR_CaptureJPEGPicture_NEW successful!", TRUE);
		//AfxMessageBox("NET_DVR_CaptureJPEGPicture_NEW successful");
	}

	if (file == NULL)
	{
		sprintf(filename, "..\\bin\\JPEGCAPTest_%d.jpg", iPicNum);
		file = fopen(filename, "wb");
	}
	fwrite(JpegPicBuffer, SizeReturned, 1, file);
	iPicNum++;

	delete JpegPicBuffer;
	fclose(file);
	file = NULL;

	CWnd* pic = GetDlgItem(IDC_STATIC_VIEW4);    // 用此句，得到图片控件的CWnd，图片将被绘制在控件上，IDC_PIC_VIEW为picture control的控件ID  
	Graphics graphics(pic->GetDC()->m_hDC);
	Image  Image(char2wchar(filename));          //加载图片  
	CRect rect;
	GetDlgItem(IDC_STATIC_VIEW4)->GetClientRect(&rect);
	graphics.DrawImage(&Image, 0, 0, rect.Width(), rect.Height());

	CWinThread *pFaceDetectThread = AfxBeginThread(RunIPCameraFaceDetectOperation, (LPVOID)filename);
	
	return;
}


void CLouFaceDlg::OnBnClickedRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	if (llRealHandle == -1)
	{
		MessageBox("请先选择一个通道播放");
		return;
	}
	if (!m_bIsRecording)
	{
		StartRecord();
	}
	else
	{
		StopRecord();
	}
}


void CLouFaceDlg::StartRecord()
{
	char RecName[256] = { 0 };

	CTime CurTime = CTime::GetCurrentTime();;
	sprintf(RecName, "%04d年%02d月%02d日%02d时%02d分%02d秒.mp4", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
		CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());

	if (!NET_DVR_SaveRealData_V30(llRealHandle,STREAM_3GPP, RecName))
	{
		MessageBox("启动录像失败");
		return;
	}
	m_bIsRecording = TRUE;
	GetDlgItem(IDC_Record)->SetWindowText("停止录像");
}


void CLouFaceDlg::StopRecord()
{
	if (!NET_DVR_StopSaveRealData(llRealHandle))
	{
		MessageBox("停止录像失败");
		return;
	}
	m_bIsRecording = FALSE;
	GetDlgItem(IDC_Record)->SetWindowText("录像");
}


void CLouFaceDlg::SaveCDC2BMP(CWnd * pWnd, TCHAR * path, Gdiplus::Rect rect)
{
	CDC tempDC;
	CBitmap bitmp;
	CDC *pDC = pWnd->GetDC();

	tempDC.CreateCompatibleDC(pDC);
	bitmp.CreateCompatibleBitmap(pDC, rect.Width, rect.Height);
	tempDC.SelectObject(&bitmp);

	tempDC.BitBlt(0, 0, rect.Width, rect.Height, pDC, rect.X, rect.Y, SRCCOPY);
	//rect.bottom - rect.top;
	//rect.right - rect.left;
	CImage image;
	image.Attach(bitmp.operator HBITMAP());
	image.Save(path);

	//  SaveBmp(bitmp.operator HBITMAP(), path);

	bitmp.DeleteObject();
	tempDC.DeleteDC();
	pDC->DeleteDC();

	CWnd* pic = GetDlgItem(IDC_STATIC_VIEW2);    // 用此句，得到图片控件的CWnd，图片将被绘制在控件上，IDC_PIC_VIEW为picture control的控件ID  
	Graphics graph(pic->GetDC()->m_hDC);
	Image  im(L"static_bg.bmp");          //加载图片  
	CRect rect1;
	GetDlgItem(IDC_STATIC_VIEW2)->GetClientRect(&rect1);
	graph.DrawImage(&im, 0, 0, rect1.Width(), rect1.Height());
}


void CLouFaceDlg::OnBnClickedFaceman()
{
	// TODO: 在此添加控件通知处理程序代码
	//pLouFaceDlg->ShowWindow(SW_HIDE);
	FaceManDlg dlg;
	dlg.DoModal();

}


HBRUSH CLouFaceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return (HBRUSH)m_brush;
}


void CLouFaceDlg::OnBnClickedVideoPlayer()
{
	// TODO: 在此添加控件通知处理程序代码
	//pLouFaceDlg->ShowWindow(SW_HIDE);
	CVideoPlayerDlg Dlg;
	Dlg.DoModal();
}


void CLouFaceDlg::OnBnClickedFaceOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!FaceDetect_flag)
	{
		FaceDetect_flag = TRUE;
		GetDlgItem(IDC_FACE_OPEN)->SetWindowTextA("关闭检测人脸");

	}
	else
	{
		FaceDetect_flag = FALSE;
		GetDlgItem(IDC_FACE_OPEN)->SetWindowTextA("开启检测人脸");
		CDC* pCDC = GetDlgItem(IDC_STATIC_VIEW)->GetDC();
		HDC hDC = pCDC->m_hDC;
		HBRUSH hBrush = ::CreateSolidBrush(RGB(240, 240, 240));
		::FillRect(hDC, CRect(0, 0, m_windowViewRect.Width(), m_windowViewRect.Height()), hBrush);
		DeleteObject(hBrush);
	}
}


void CLouFaceDlg::OnBnClickedFaceOpen2()
{
	// TODO: 在此添加控件通知处理程序代码

	if (!FaceFeature_flag)
	{
		FaceFeature_flag = TRUE;
		pLouFaceDlg->m_videoOpened = true;
		GetDlgItem(IDC_FACE_OPEN2)->SetWindowTextA("关闭检测特征");
		GetDlgItem(IDC_StartReview)->EnableWindow(FALSE);
	}
	else
	{
		FaceFeature_flag = FALSE;
		GetDlgItem(IDC_FACE_OPEN2)->SetWindowTextA("开启检测特征");
		CDC* pCDC = GetDlgItem(IDC_STATIC_VIEW)->GetDC();
		HDC hDC = pCDC->m_hDC;
		HBRUSH hBrush = ::CreateSolidBrush(RGB(240, 240, 240));
		::FillRect(hDC, CRect(0, 0, m_windowViewRect.Width(), m_windowViewRect.Height()), hBrush);
		DeleteObject(hBrush);

		GetDlgItem(IDC_BTN_COMPARE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_THRESHOLD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOGNITION)->EnableWindow(TRUE);
		GetDlgItem(IDC_StartReview)->EnableWindow(TRUE);
		//将之前存储的信息清除
		m_curFaceInfo = { 0 };
		m_curVideoShowString = "";
		{
			lock_guard<mutex> lock(g_mutex);
			if (m_curVideoImage)
			{
				cvReleaseImage(&m_curVideoImage);
				m_curVideoImage = NULL;
			}

			if (m_curIrVideoImage)
			{
				cvReleaseImage(&m_curIrVideoImage);
				m_curIrVideoImage = NULL;
			}
		}

		m_dataValid = false;
		m_videoOpened = false;

		Sleep(600);
		m_bFDThreadRunning = FALSE;

	}
}



void CLouFaceDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CWinThread *pThread = AfxBeginThread(ThreadProc, (LPVOID)125);

}

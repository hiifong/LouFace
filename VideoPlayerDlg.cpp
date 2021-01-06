// VideoPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LouFace.h"
#include "VideoPlayerDlg.h"
#include "afxdialogex.h"
#include "AVPlayer.h"
#include "UTN.h"
#define WM_USER_POS_CHANGED WM_USER + 1




// CVideoPlayerDlg 对话框

IMPLEMENT_DYNAMIC(CVideoPlayerDlg, CDialogEx)

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_XML_DIALOG, pParent)
{
}

CVideoPlayerDlg::~CVideoPlayerDlg()
{
}

void CVideoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PLAY, m_listPlay);
	DDX_Control(pDX, IDC_SLIDER_PLAY, m_slider);
	DDX_Control(pDX, IDC_VOICE_PROGRESS, m_Voiceprocess);
}


BEGIN_MESSAGE_MAP(CVideoPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CVideoPlayerDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_PREVIOUS, &CVideoPlayerDlg::OnBnClickedButtonPlayPrevious)
	ON_WM_DROPFILES()
	ON_WM_QUERYDRAGICON()
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_BUTTON_PLAY_NEXT, &CVideoPlayerDlg::OnBnClickedButtonPlayNext)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_FAST_BACKFORWARD, &CVideoPlayerDlg::OnBnClickedButtonPlayFastBackforward)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_FAST_FORWARD, &CVideoPlayerDlg::OnBnClickedButtonPlayFastForward)
	ON_BN_CLICKED(IDC_BUTTON_VOLUME_DECREASE, &CVideoPlayerDlg::OnBnClickedButtonVolumeDecrease)
	ON_BN_CLICKED(IDC_BUTTON_VOLUME_INCREASE, &CVideoPlayerDlg::OnBnClickedButtonVolumeIncrease)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_PLAY_LIST, &CVideoPlayerDlg::OnBnClickedButtonShowPlayList)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_LBN_DBLCLK(IDC_LIST_PLAY, &CVideoPlayerDlg::OnLbnDblclkListPlay)
	ON_MESSAGE(WM_USER_POS_CHANGED, &CVideoPlayerDlg::OnUserPosChanged)
//	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CVideoPlayerDlg 消息处理程序


void CVideoPlayerDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_cAVPlayer.IsOpen())
	{
		return;
	}

	if (m_cAVPlayer.IsPlaying())
	{
		m_cAVPlayer.Pause();
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(_T("播放"));
	}
	else
	{
		m_cAVPlayer.Play();
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(_T("暂停"));
	}
}


void CVideoPlayerDlg::OnBnClickedButtonPlayPrevious()
{
	// TODO: 在此添加控件通知处理程序代码
	int nIndex = m_listPlay.GetCurSel();
	if (LB_ERR == nIndex)
	{
		return;
	}

	if (nIndex - 1 < 0)
	{
		nIndex = m_listPlay.GetCount();
	}

	m_listPlay.SetCurSel(nIndex - 1);
	OnLbnDblclkListPlay();
}
void CVideoPlayerDlg::ShowPlaylist(BOOL bShow)
{
	CRect rcWnd, rcPlaylist;

	GetWindowRect(&rcWnd);
	m_listPlay.GetClientRect(&rcPlaylist);

	if (m_listPlay.IsWindowVisible() != bShow) // 只有显示状态发生变化时，才调整窗口大小
	{
		if (bShow)
		{
			SetWindowPos(NULL, 0, 0, rcWnd.Width() + rcPlaylist.Width(), rcWnd.Height(), SWP_NOMOVE);
		}
		else
		{
			SetWindowPos(NULL, 0, 0, rcWnd.Width() - rcPlaylist.Width(), rcWnd.Height(), SWP_NOMOVE);
		}
	}

	m_listPlay.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}
void EnumerateFiles(CStringArray &strArray)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(_T("*.*"), &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// 如果为目录
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 如果不为当前目录.或上一层目录..
				if (_tcscmp(fd.cFileName, _T(".")) && _tcscmp(fd.cFileName, _T("..")))
				{
					::SetCurrentDirectory(fd.cFileName);    // 进入该目录
					EnumerateFiles(strArray);               // 递归枚举
					::SetCurrentDirectory(_T(".."));        // 返回该目录上一层目录
				}
			}
			// 如果为文件
			else
			{
				CString strDir;
				TCHAR   lpDir[MAX_PATH];

				::GetCurrentDirectory(MAX_PATH, lpDir);
				strDir = lpDir;
				if (strDir.Right(1) != _T("\\"))
				{
					strDir += _T("\\");
				}
				strDir += fd.cFileName;

				strArray.Add(strDir);
			}
		} while (::FindNextFile(hFind, &fd));

		::FindClose(hFind);
	}
}
BOOL IsWantedFile(const CString &str)
{
	CString strLower;
	TCHAR   szExt[_MAX_EXT] = _T("");

	// 这里偷懒直接用了之前的过滤字符串，由于文件名不能含有【*】，所以可以在后缀名后面加上【;*】来判断是否完全匹配
	const   CString STR_FileFilter = 
		_T("*.rm;*.rmvb;*.flv;*.f4v;*.avi;*.3gp;*.mp4;*.wmv;*.mpeg;*.mpga;*.asf;*.mov;*");
		//_T("*.mp3;*.wma;*.wav;*.mid;*.rmi;*.aac;*.ac3;*.aiff;*.m4a;*.mka;*.mp2;*.ogg;*");

	_tsplitpath_s(str, NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);   // 获取后缀名
	strLower = szExt;
	strLower.MakeLower();

	if (!strLower.IsEmpty())    // 没有后缀名的不符合条件
	{
		strLower += _T(";*");   // .mo不符合条件，由于会匹配到.mov，所以在后面加上【;*】来判断是否完全匹配
		return -1 != STR_FileFilter.Find(strLower);
	}

	return FALSE;
}
//拖入文件
void CVideoPlayerDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	TCHAR        szFileName[_MAX_PATH] = _T("");
	CFileStatus  fStatus;
	CStringArray strArray;

	ShowPlaylist(TRUE);

	// 获取拖拽进来文件和文件夹
	for (UINT i = 0; i < nFileCount; i++)
	{
		// 获取文件路径
		::DragQueryFile(hDropInfo, i, szFileName, sizeof(szFileName));
		CFile::GetStatus(szFileName, fStatus);

		// 是否为文件夹
		if (fStatus.m_attribute & FILE_ATTRIBUTE_DIRECTORY)
		{
			::SetCurrentDirectory(szFileName);  // 将当前目录设置在此           
			EnumerateFiles(strArray);	        // 枚举目录中所有文件
		}
		else
		{
			strArray.Add(szFileName);
		}
	}

	// 过滤需要的文件
	m_listPlay.SetRedraw(FALSE);
	m_listPlay.SetHorizontalExtent(1500); // 这里只是简单的设置为500宽度，若要刚好匹配路径宽度，请参见msdn里的例子

	for (int i = 0; i < strArray.GetCount(); i++)
	{
		if (IsWantedFile(strArray[i]))
		{
			m_listPlay.AddString(strArray[i]);
		}
	}

	m_listPlay.SetRedraw(TRUE);
	::DragFinish(hDropInfo);
	CDialog::OnDropFiles(hDropInfo);
}
HCURSOR CVideoPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CVideoPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{

	CDialog::OnSysCommand(nID, lParam);

}

void CVideoPlayerDlg::OnBnClickedButtonPlayNext()
{
	int nIndex = m_listPlay.GetCurSel();
	if (LB_ERR == nIndex)
	{
		return;
	}

	if (nIndex + 1 >= m_listPlay.GetCount())
	{
		nIndex = -1;
	}

	m_listPlay.SetCurSel(nIndex + 1);
	OnLbnDblclkListPlay();
}


void CVideoPlayerDlg::OnBnClickedButtonPlayFastBackforward()
{
	m_cAVPlayer.SeekBackward();
}


void CVideoPlayerDlg::OnBnClickedButtonPlayFastForward()
{
	m_cAVPlayer.SeekForward();
}


void CVideoPlayerDlg::OnBnClickedButtonVolumeDecrease()
{
	m_cAVPlayer.VolumeReduce();
	m_Voiceprocess.SetPos(m_cAVPlayer.m_VoiceVolume);
}


void CVideoPlayerDlg::OnBnClickedButtonVolumeIncrease()
{
	m_cAVPlayer.VolumeIncrease();
	m_Voiceprocess.SetPos(m_cAVPlayer.m_VoiceVolume);
}
//获取当前可执行程序的路径
CString GetProgramPath()
{
	CString  strProgramPath;
	GetModuleFileName(NULL, strProgramPath.GetBuffer(MAX_PATH), MAX_PATH);
	strProgramPath.ReleaseBuffer(MAX_PATH);
	int nPathPos = strProgramPath.ReverseFind('\\');
	strProgramPath = strProgramPath.Left(nPathPos + 1);
	return strProgramPath;
}
void CVideoPlayerDlg::OnBnClickedButtonShowPlayList()
{
	ShowPlaylist(!m_listPlay.IsWindowVisible());
}

void CVideoPlayerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	pLouFaceDlg->ShowWindow(SW_SHOW);
	CDialogEx::OnClose();
}

void CallbackPosChanged(void *data, int iPos)
{
	CAVPlayer *pAVPlayer = (CAVPlayer *)data;

	if (pAVPlayer)
	{
		HWND hWnd = pAVPlayer->GetHWND();

		if (::IsWindow(hWnd) && ::IsWindow(::GetParent(hWnd)))
		{
			::PostMessage(::GetParent(hWnd), WM_USER_POS_CHANGED, (WPARAM)data, iPos);
		}
	}
}
// 为了让大家更方便的编译工程，所以没用三方库，实际项目中，推荐用三方库，简洁安全~
CStringA UnicodeToUTF8(const CStringW& strWide)
{
	CStringA strUTF8;
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, strWide, -1, NULL, 0, NULL, NULL);

	if (nLen > 1)
	{
		::WideCharToMultiByte(CP_UTF8, 0, strWide, -1, strUTF8.GetBuffer(nLen - 1), nLen, NULL, NULL);
		strUTF8.ReleaseBuffer();
	}

	return strUTF8;
}
//初始化
BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

									// TODO: 在此添加额外的初始化代码  
	ModifyStyleEx(NULL, WS_EX_ACCEPTFILES);                             // 接收文件拖拽
	m_cAVPlayer.SetHWND(GetDlgItem(IDC_STATIC_VIDEO)->GetSafeHwnd());   // 设置播放器的窗口句柄
	m_cAVPlayer.SetCallback(CallbackPosChanged);

	// 隐藏播放列表，并调整窗口大小
	/*CRect rcWnd, rcPlaylist;*/
	//GetWindowRect(&rcWnd);
	//m_listPlay.GetClientRect(&rcPlaylist);
	//SetWindowPos(NULL, 0, 0, rcWnd.Width() - rcPlaylist.Width() - GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXBORDER), rcWnd.Height(), SWP_NOMOVE);
	//GetDlgItem(IDC_LIST_PLAY)->ShowWindow(SW_HIDE);
	m_Voiceprocess.SetRange32(0, 1000);
	//m_Voiceprocess.SetPos(m_cAVPlayer.GetVoiceVolume());
	//m_Voiceprocess.SetBarColor(RGB(255, 0, 0));

	//自动加载当前程序目录所有的视频
	CFileFind finder;
	CString strPath("");
	BOOL bWorking = finder.FindFile(GetProgramPath() + _T("\\*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		strPath = finder.GetFilePath();
		//strPath就是所要获取Test目录下的文件夹和文件（包括路径）
		//m_List.AddString(strPath);
		CString firstFileName = "";

		firstFileName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);//从路径中截取文件名
		if (IsWantedFile(firstFileName))
		{
			m_listPlay.AddString(firstFileName);
		}
	}
	finder.Close();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
				  // 异常: OCX 属性页应返回 FALSE
}
//双击列表中的文件名函数
void CVideoPlayerDlg::OnLbnDblclkListPlay()
{
	int nIndex = m_listPlay.GetCurSel();
	if (LB_ERR == nIndex)
	{
		return;
	}

	CString strPath;
	CRect   rcVideo;

	GetDlgItem(IDC_STATIC_VIDEO)->GetClientRect(&rcVideo);
	InvalidateRect(rcVideo);
	m_listPlay.GetText(nIndex, strPath);

	if (m_cAVPlayer.Play((LPCSTR)AsciiToUtf8_CSTR(strPath)))
	{
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(_T("暂停"));
	}
}

afx_msg LRESULT CVideoPlayerDlg::OnUserPosChanged(WPARAM wParam, LPARAM lParam)
{
	m_slider.SetPos(m_cAVPlayer.GetPos());
	return 0;
}



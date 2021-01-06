// VideoPlayerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "LouFace.h"
#include "VideoPlayerDlg.h"
#include "afxdialogex.h"
#include "AVPlayer.h"
#include "UTN.h"
#define WM_USER_POS_CHANGED WM_USER + 1




// CVideoPlayerDlg �Ի���

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


// CVideoPlayerDlg ��Ϣ�������


void CVideoPlayerDlg::OnBnClickedButtonPlay()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_cAVPlayer.IsOpen())
	{
		return;
	}

	if (m_cAVPlayer.IsPlaying())
	{
		m_cAVPlayer.Pause();
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(_T("����"));
	}
	else
	{
		m_cAVPlayer.Play();
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(_T("��ͣ"));
	}
}


void CVideoPlayerDlg::OnBnClickedButtonPlayPrevious()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

	if (m_listPlay.IsWindowVisible() != bShow) // ֻ����ʾ״̬�����仯ʱ���ŵ������ڴ�С
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
			// ���ΪĿ¼
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// �����Ϊ��ǰĿ¼.����һ��Ŀ¼..
				if (_tcscmp(fd.cFileName, _T(".")) && _tcscmp(fd.cFileName, _T("..")))
				{
					::SetCurrentDirectory(fd.cFileName);    // �����Ŀ¼
					EnumerateFiles(strArray);               // �ݹ�ö��
					::SetCurrentDirectory(_T(".."));        // ���ظ�Ŀ¼��һ��Ŀ¼
				}
			}
			// ���Ϊ�ļ�
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

	// ����͵��ֱ������֮ǰ�Ĺ����ַ����������ļ������ܺ��С�*�������Կ����ں�׺��������ϡ�;*�����ж��Ƿ���ȫƥ��
	const   CString STR_FileFilter = 
		_T("*.rm;*.rmvb;*.flv;*.f4v;*.avi;*.3gp;*.mp4;*.wmv;*.mpeg;*.mpga;*.asf;*.mov;*");
		//_T("*.mp3;*.wma;*.wav;*.mid;*.rmi;*.aac;*.ac3;*.aiff;*.m4a;*.mka;*.mp2;*.ogg;*");

	_tsplitpath_s(str, NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);   // ��ȡ��׺��
	strLower = szExt;
	strLower.MakeLower();

	if (!strLower.IsEmpty())    // û�к�׺���Ĳ���������
	{
		strLower += _T(";*");   // .mo���������������ڻ�ƥ�䵽.mov�������ں�����ϡ�;*�����ж��Ƿ���ȫƥ��
		return -1 != STR_FileFilter.Find(strLower);
	}

	return FALSE;
}
//�����ļ�
void CVideoPlayerDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	TCHAR        szFileName[_MAX_PATH] = _T("");
	CFileStatus  fStatus;
	CStringArray strArray;

	ShowPlaylist(TRUE);

	// ��ȡ��ק�����ļ����ļ���
	for (UINT i = 0; i < nFileCount; i++)
	{
		// ��ȡ�ļ�·��
		::DragQueryFile(hDropInfo, i, szFileName, sizeof(szFileName));
		CFile::GetStatus(szFileName, fStatus);

		// �Ƿ�Ϊ�ļ���
		if (fStatus.m_attribute & FILE_ATTRIBUTE_DIRECTORY)
		{
			::SetCurrentDirectory(szFileName);  // ����ǰĿ¼�����ڴ�           
			EnumerateFiles(strArray);	        // ö��Ŀ¼�������ļ�
		}
		else
		{
			strArray.Add(szFileName);
		}
	}

	// ������Ҫ���ļ�
	m_listPlay.SetRedraw(FALSE);
	m_listPlay.SetHorizontalExtent(1500); // ����ֻ�Ǽ򵥵�����Ϊ500��ȣ���Ҫ�պ�ƥ��·����ȣ���μ�msdn�������

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
//��ȡ��ǰ��ִ�г����·��
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
// Ϊ���ô�Ҹ�����ı��빤�̣�����û�������⣬ʵ����Ŀ�У��Ƽ��������⣬��లȫ~
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
//��ʼ��
BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	ModifyStyleEx(NULL, WS_EX_ACCEPTFILES);                             // �����ļ���ק
	m_cAVPlayer.SetHWND(GetDlgItem(IDC_STATIC_VIDEO)->GetSafeHwnd());   // ���ò������Ĵ��ھ��
	m_cAVPlayer.SetCallback(CallbackPosChanged);

	// ���ز����б����������ڴ�С
	/*CRect rcWnd, rcPlaylist;*/
	//GetWindowRect(&rcWnd);
	//m_listPlay.GetClientRect(&rcPlaylist);
	//SetWindowPos(NULL, 0, 0, rcWnd.Width() - rcPlaylist.Width() - GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXBORDER), rcWnd.Height(), SWP_NOMOVE);
	//GetDlgItem(IDC_LIST_PLAY)->ShowWindow(SW_HIDE);
	m_Voiceprocess.SetRange32(0, 1000);
	//m_Voiceprocess.SetPos(m_cAVPlayer.GetVoiceVolume());
	//m_Voiceprocess.SetBarColor(RGB(255, 0, 0));

	//�Զ����ص�ǰ����Ŀ¼���е���Ƶ
	CFileFind finder;
	CString strPath("");
	BOOL bWorking = finder.FindFile(GetProgramPath() + _T("\\*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		strPath = finder.GetFilePath();
		//strPath������Ҫ��ȡTestĿ¼�µ��ļ��к��ļ�������·����
		//m_List.AddString(strPath);
		CString firstFileName = "";

		firstFileName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);//��·���н�ȡ�ļ���
		if (IsWantedFile(firstFileName))
		{
			m_listPlay.AddString(firstFileName);
		}
	}
	finder.Close();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
				  // �쳣: OCX ����ҳӦ���� FALSE
}
//˫���б��е��ļ�������
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
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(_T("��ͣ"));
	}
}

afx_msg LRESULT CVideoPlayerDlg::OnUserPosChanged(WPARAM wParam, LPARAM lParam)
{
	m_slider.SetPos(m_cAVPlayer.GetPos());
	return 0;
}



// FaceManDlg.cpp : 人脸信息管理系统实现文件
//

#include "stdafx.h"
#include "LouFace.h"
#include "FaceManDlg.h"
#include "afxdialogex.h"

//ADO(ActiveX Data Object)是Microsoft数据库应用程序开发的新接口，是建立在OLE DB之上的高层数据库访问技术，请不必为此担心，即使你对OLE DB，COM不了解也能轻松对付ADO, 因为它非常简单易用，甚至比你以往所接触的ODBC API、DAO、RDO都要容易使用，并不失灵活性。本文将详细地介绍在VC下如何使用ADO来进行数据库应用程序开发，并给出示例代码。
//ADO中最重要的对象有三个：Connection、Recordset和Command，分别表示连接对象、记录集对象和命令对象。
//三个对象对应的智能指针分别是：_ConnectionPtr、_RecordsetPtr、_CommandPtr。

// FaceManDlg 对话框
extern CLouFaceDlg *pLouFaceDlg;
IMPLEMENT_DYNAMIC(FaceManDlg, CDialog)

FaceManDlg::FaceManDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FACEMAN, pParent)

{

}

FaceManDlg::~FaceManDlg()
{
}
//为每个控件定义一个变量，这段代码就是让变量和控件绑定在一起.
void FaceManDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_INFO_FACEID, m_faceid);
	DDX_Control(pDX, IDC_INFO_ADDRESS, m_address);
	DDX_Control(pDX, IDC_INFO_NAME, m_name);
}

//消息映射
BEGIN_MESSAGE_MAP(FaceManDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADD, &FaceManDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_QUERY, &FaceManDlg::OnBnClickedQuery)
	ON_BN_CLICKED(IDC_DEL, &FaceManDlg::OnBnClickedDel)
	ON_BN_CLICKED(IDC_MOD, &FaceManDlg::OnBnClickedMod)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_EXIT, &FaceManDlg::OnBnClickedExit)
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// FaceManDlg 消息处理程序


BOOL FaceManDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CBitmap bmp;
	bmp.LoadBitmap(IDB_BACKGROUD);   //IDB_BACKGROUD是图片资源ID
	m_brush.CreatePatternBrush(&bmp);

	//连接数据库代码
	try {

		CoInitialize(NULL);//用来告诉 Windows以单线程的方式创建com对象
		m_pConnection = _ConnectionPtr(__uuidof(Connection));//_ConnectionPtr这个指针来操纵Connection对象
		m_pConnection->ConnectionString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=mfc_access.mdb;";
		m_pConnection->Open("", "", "", adConnectUnspecified);
	}
	catch (_com_error e) {

		pLouFaceDlg->EditOut("数据库连接失败！", TRUE);
		return TRUE;
	}

	UpdateData(true);
	CRect rect;
	// 获取编程语言列表视图控件的位置和大小   
	m_list.GetClientRect(&rect);
	// 为列表视图控件添加全行选中和栅格风格   
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("人脸ID"), LVCFMT_LEFT, rect.Width() / 5);
	m_list.InsertColumn(1, _T("姓名"), LVCFMT_LEFT, rect.Width() / 5);
	m_list.InsertColumn(2, _T("地址"), LVCFMT_LEFT, rect.Width() / 2);

	UpdateData(FALSE);
	OnBnClickedQuery();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


HBRUSH FaceManDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return (HBRUSH)m_brush;
}

//添加一个数据
void FaceManDlg::OnBnClickedAdd()
{
	_variant_t RecordsAffected;
	CString AddSql;
	CString cmdchooseID;
	CString cmdchoose;
	CString cmdchoosevalue;
	m_faceid.GetWindowText(cmdchooseID);
	m_name.GetWindowText(cmdchoose);
	m_address.GetWindowText(cmdchoosevalue);
	if (cmdchooseID.IsEmpty() || cmdchoose.IsEmpty() || cmdchoosevalue.IsEmpty())
		return;
	AddSql.Format("INSERT INTO cmdchoose(id,chooseid,choosevalue) VALUES('" + cmdchooseID + "','" + cmdchoose + "','" + cmdchoosevalue + "')");
	try {
		m_pConnection->Execute((_bstr_t)AddSql, &RecordsAffected, adCmdText);
		//AfxMessageBox(_T("添加命令成功！"));
		OnBnClickedQuery();
	}
	catch (_com_error*e) {
		AfxMessageBox(_T("添加命令失败！"));
	}
	m_faceid.SetWindowText("");
	m_name.SetWindowText("");
	m_address.SetWindowText("");
}

//查询数据库中的内容
void FaceManDlg::OnBnClickedQuery()
{
	m_list.DeleteAllItems();
	UpdateData(true);
	try
	{
		_variant_t RecordsAffected;
		m_pRecordset.CreateInstance(__uuidof(Recordset));
		CString search_sql;
		search_sql = "SELECT * FROM cmdchoose";
		m_pRecordset = m_pConnection->Execute(search_sql.AllocSysString(), NULL, adCmdText);
		while (!m_pRecordset->adoEOF)
		{
			CString id;
			CString chooseid;
			CString choosevalue;
			id = m_pRecordset->GetCollect("id").bstrVal;
			m_list.InsertItem(0, id);
			chooseid = m_pRecordset->GetCollect("chooseid").bstrVal;
			m_list.SetItemText(0, 1, chooseid);
			choosevalue = m_pRecordset->GetCollect("choosevalue").bstrVal;
			m_list.SetItemText(0, 2, choosevalue);
			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
	}
	catch (_com_error e)
	{
		AfxMessageBox(_T("搜索失败！"));
		return;
	}
}

//删除指定的一条数据
void FaceManDlg::OnBnClickedDel()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	CString choose_del;
	//m_faceid.GetWindowText(choose_del);
	int nItem = -1;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		while (pos)
		{
			nItem = m_list.GetNextSelectedItem(pos);
			// nItem即是选中行的序号
		}
	}
	choose_del = m_list.GetItemText(nItem, 0);
	if (AfxMessageBox(_T("确认要删除") + str + _T("这条记录吗？"), MB_YESNO) == IDNO)
		return;
	try
	{
		str.Format("DELETE from cmdchoose where id='" + choose_del + "'");
		_variant_t RecordsAffected;
		m_pConnection->Execute((_bstr_t)str, &RecordsAffected, adCmdText);
		OnBnClickedQuery();
	}
	catch (_com_error*e)
	{
		AfxMessageBox(e->ErrorMessage());
	}
	OnBnClickedQuery();
}


void FaceManDlg::OnBnClickedMod()
{
	int nSel = m_list.GetSelectionMark();
	if (nSel < 0)
	{
		AfxMessageBox(_T("请选中一条再修改"));
	}
	CString szSQL;
	CString str;
	str = m_list.GetItemText(nSel, 0);
	CString cmdchooseID;
	CString cmdchoose;
	CString cmdchoosevalue;
	m_faceid.GetWindowText(cmdchooseID);
	m_name.GetWindowText(cmdchoose);
	m_address.GetWindowText(cmdchoosevalue);
	if (cmdchooseID.IsEmpty() || cmdchoose.IsEmpty() || cmdchoosevalue.IsEmpty())
		return;
	if (AfxMessageBox(_T("确认要修改 ") + str + _T(" 这条记录吗？"), MB_YESNO) == IDNO)
		return;
	szSQL = "UPDATE cmdchoose SET id='" + cmdchooseID + "',chooseid='" + cmdchoose + "',choosevalue='" + cmdchoosevalue+"' where id='" + str+"'";
	CString m_szLastError="";
	try
	{
		_variant_t RecordsAffected;
		m_pConnection->Execute((_bstr_t)szSQL, &RecordsAffected, adCmdText);
	}
	catch (_com_error &e)
	{
		m_szLastError = (LPCTSTR)e.Description();
		e.Error();
	}
	m_faceid.SetWindowText("");
	m_name.SetWindowText("");
	m_address.SetWindowText("");
	OnBnClickedQuery();
}


void FaceManDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnClose();
}


void FaceManDlg::OnBnClickedExit()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
	pLouFaceDlg->ShowWindow(SW_SHOW);
}


void FaceManDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void FaceManDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nID == SC_CLOSE)
	{
		pLouFaceDlg->ShowWindow(SW_SHOW);
		CDialog::OnCancel();
	}

	CDialog::OnSysCommand(nID, lParam);
}

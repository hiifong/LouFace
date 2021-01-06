// FaceManDlg.cpp : ������Ϣ����ϵͳʵ���ļ�
//

#include "stdafx.h"
#include "LouFace.h"
#include "FaceManDlg.h"
#include "afxdialogex.h"

//ADO(ActiveX Data Object)��Microsoft���ݿ�Ӧ�ó��򿪷����½ӿڣ��ǽ�����OLE DB֮�ϵĸ߲����ݿ���ʼ������벻��Ϊ�˵��ģ���ʹ���OLE DB��COM���˽�Ҳ�����ɶԸ�ADO, ��Ϊ���ǳ������ã����������������Ӵ���ODBC API��DAO��RDO��Ҫ����ʹ�ã�����ʧ����ԡ����Ľ���ϸ�ؽ�����VC�����ʹ��ADO���������ݿ�Ӧ�ó��򿪷���������ʾ�����롣
//ADO������Ҫ�Ķ�����������Connection��Recordset��Command���ֱ��ʾ���Ӷ��󡢼�¼��������������
//���������Ӧ������ָ��ֱ��ǣ�_ConnectionPtr��_RecordsetPtr��_CommandPtr��

// FaceManDlg �Ի���
extern CLouFaceDlg *pLouFaceDlg;
IMPLEMENT_DYNAMIC(FaceManDlg, CDialog)

FaceManDlg::FaceManDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FACEMAN, pParent)

{

}

FaceManDlg::~FaceManDlg()
{
}
//Ϊÿ���ؼ�����һ����������δ�������ñ����Ϳؼ�����һ��.
void FaceManDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_INFO_FACEID, m_faceid);
	DDX_Control(pDX, IDC_INFO_ADDRESS, m_address);
	DDX_Control(pDX, IDC_INFO_NAME, m_name);
}

//��Ϣӳ��
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


// FaceManDlg ��Ϣ�������


BOOL FaceManDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CBitmap bmp;
	bmp.LoadBitmap(IDB_BACKGROUD);   //IDB_BACKGROUD��ͼƬ��ԴID
	m_brush.CreatePatternBrush(&bmp);

	//�������ݿ����
	try {

		CoInitialize(NULL);//�������� Windows�Ե��̵߳ķ�ʽ����com����
		m_pConnection = _ConnectionPtr(__uuidof(Connection));//_ConnectionPtr���ָ��������Connection����
		m_pConnection->ConnectionString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=mfc_access.mdb;";
		m_pConnection->Open("", "", "", adConnectUnspecified);
	}
	catch (_com_error e) {

		pLouFaceDlg->EditOut("���ݿ�����ʧ�ܣ�", TRUE);
		return TRUE;
	}

	UpdateData(true);
	CRect rect;
	// ��ȡ��������б���ͼ�ؼ���λ�úʹ�С   
	m_list.GetClientRect(&rect);
	// Ϊ�б���ͼ�ؼ����ȫ��ѡ�к�դ����   
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("����ID"), LVCFMT_LEFT, rect.Width() / 5);
	m_list.InsertColumn(1, _T("����"), LVCFMT_LEFT, rect.Width() / 5);
	m_list.InsertColumn(2, _T("��ַ"), LVCFMT_LEFT, rect.Width() / 2);

	UpdateData(FALSE);
	OnBnClickedQuery();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


HBRUSH FaceManDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return (HBRUSH)m_brush;
}

//���һ������
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
		//AfxMessageBox(_T("�������ɹ���"));
		OnBnClickedQuery();
	}
	catch (_com_error*e) {
		AfxMessageBox(_T("�������ʧ�ܣ�"));
	}
	m_faceid.SetWindowText("");
	m_name.SetWindowText("");
	m_address.SetWindowText("");
}

//��ѯ���ݿ��е�����
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
		AfxMessageBox(_T("����ʧ�ܣ�"));
		return;
	}
}

//ɾ��ָ����һ������
void FaceManDlg::OnBnClickedDel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
			// nItem����ѡ���е����
		}
	}
	choose_del = m_list.GetItemText(nItem, 0);
	if (AfxMessageBox(_T("ȷ��Ҫɾ��") + str + _T("������¼��"), MB_YESNO) == IDNO)
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
		AfxMessageBox(_T("��ѡ��һ�����޸�"));
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
	if (AfxMessageBox(_T("ȷ��Ҫ�޸� ") + str + _T(" ������¼��"), MB_YESNO) == IDNO)
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialog::OnClose();
}


void FaceManDlg::OnBnClickedExit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnCancel();
	pLouFaceDlg->ShowWindow(SW_SHOW);
}


void FaceManDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
}


void FaceManDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (nID == SC_CLOSE)
	{
		pLouFaceDlg->ShowWindow(SW_SHOW);
		CDialog::OnCancel();
	}

	CDialog::OnSysCommand(nID, lParam);
}

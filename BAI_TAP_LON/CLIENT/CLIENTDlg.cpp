
// CLIENTDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CLIENT.h"
#include "CLIENTDlg.h"
#include "afxdialogex.h"
#include "CONST.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCLIENTDlg::CCLIENTDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCLIENTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCLIENTDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(buttonBrowse, &CCLIENTDlg::OnBnClickedBrowse)
END_MESSAGE_MAP()


// CCLIENTDlg message handlers

BOOL CCLIENTDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	this->s_file.Create(_T("File"), WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(100, 110, 130, 140), this, staticFile);
	this->s_fileName.Create(_T("File Name"), WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(50, 200, 130, 230), this, staticFileName);
	this->e_forward.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(150, 105, 700, 135), this, editForward);
	this->e_search.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(150, 195, 700, 225), this, editSearch);
	this->b_connect.Create(_T("Connect"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(400, 20, 500, 80), this, buttonConnect);
	this->b_borwse.Create(_T("Browse"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(720, 105, 820, 135), this, buttonBrowse);
	this->b_forward.Create(_T("Forward"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(840, 105, 940, 135), this, buttonForward);
	this->b_search.Create(_T("Search"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(720, 195, 820, 225), this, buttonSearch);

	return TRUE;
}

void CCLIENTDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CCLIENTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCLIENTDlg::OnBnClickedBrowse() {
	CFileDialog l_fDlg(TRUE);
	int iRet = l_fDlg.DoModal();
	CString l_strFileName;
	l_strFileName = l_fDlg.GetPathName();

	if (iRet == IDOK) {
		try
		{
			CStdioFile file(l_strFileName, CFile::modeRead);
			CString str, contentstr = _T("");

			while (file.ReadString(str))
			{
				contentstr += str;
				contentstr += _T("\n");
			}
			CString cPathToFile = l_fDlg.GetPathName();
			SetDlgItemText(editForward, cPathToFile);
			pathToFile = CW2A(cPathToFile.GetString());
		}
		catch (CException* e)
		{
			MessageBox(_T("Error"));
			e->Delete();
		}
	}
}

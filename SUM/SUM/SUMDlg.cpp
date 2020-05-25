
// SUMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SUM.h"
#include "SUMDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSUMDlg dialog

CSUMDlg::CSUMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SUM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSUMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, edit_a, a);
	DDX_Control(pDX, edit_b, b);
}

BEGIN_MESSAGE_MAP(CSUMDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(btn_sum, &CSUMDlg::OnBnClickedsum)
END_MESSAGE_MAP()


// CSUMDlg message handlers

BOOL CSUMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSUMDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSUMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSUMDlg::OnBnClickedsum()
{
	CString aStr, bStr;
	GetDlgItemText(edit_a, aStr);
	GetDlgItemText(edit_b, bStr);
	int aInt = _wtoi(aStr);
	int bInt = _wtoi(bStr);
	if (aInt == 0 && bInt == 0) {
		MessageBox(_T("a và b không thể cùng bằng 0", _T("Error"), MB_ICONERROR | MB_OKCANCEL));
		return;
	}
	CString result;
	result.Format(_T("%d"), aInt + bInt);
	SetDlgItemText(edit_sum, result);
}

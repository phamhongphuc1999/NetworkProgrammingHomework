
// CLIENTDlg.h : header file
//
#include <String>
using namespace std;
#pragma once


// CCLIENTDlg dialog
class CCLIENTDlg : public CDialogEx
{
// Construction
public:
	CCLIENTDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//controller
	CStatic s_file, s_fileName;
	CEdit e_forward, e_search;
	CButton b_connect, b_borwse, b_forward, b_search;

	string pathToFile;

	//event
	afx_msg void OnBnClickedBrowse();
};

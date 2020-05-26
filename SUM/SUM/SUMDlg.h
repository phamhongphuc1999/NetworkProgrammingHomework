
// SUMDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "SUM_class.h"

// CSUMDlg dialog
class CSUMDlg : public CDialogEx
{
// Construction
public:
	CSUMDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SUM_DIALOG };
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
	CEdit a;
	CEdit b;
	CEdit c;
	afx_msg void OnBnClickedsum();
};

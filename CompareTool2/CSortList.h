#pragma once
#include <afxcmn.h>
#include <afxwin.h>



typedef struct SORT_PARAMS {
	// HWND hWnd;		// ����Ʈ��Ʈ�� �ڵ�
	int nCol;		// ���ı����� �÷�
	BOOL bAscend;	// ���������ΰ� ���������ΰ�...
	CListCtrl *pList;
}SORT_PARAMS;


// int IsDigit(CString str);

class CSortList : public CListCtrl
{
	DECLARE_DYNAMIC(CSortList)

public:
	CSortList();
	virtual ~CSortList();

private:
	// CSortList *pThis;
	// int IsDigit(CString str);
protected:
	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_IsRealtime;
	BOOL m_bAscend;
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	static int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};



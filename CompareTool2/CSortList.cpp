#include "stdafx.h"

#include "CSortList.h"

IMPLEMENT_DYNAMIC(CSortList, CListCtrl)


BEGIN_MESSAGE_MAP(CSortList, CListCtrl)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CSortList::OnLvnColumnclick)
END_MESSAGE_MAP()

//
//int IsDigit(CString str)
//{
//	int count = str.GetLength();
//
//	int i;
//	for (i = 0; i < count; i++)
//
//	{
//		char temp = str.GetAt(i);
//
//		if (i == 0 && temp == '-') continue; // ���� ó��.
//		// �Էµ� Ű�� 0 ~ 9 �����ΰ��� üũ.
//
//		if (temp >= '0' && temp <= '9')
//
//			continue;
//
//		else
//
//			break;
//	}
//
//	if (i == count)
//		return 1;
//	else
//		return 0;
//}



CSortList::CSortList()
{
}


CSortList::~CSortList()
{
}





void CSortList::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult) //����Ʈ�� col�� ����Ŭ���Ҷ� �߻��ϴ� �޼����Լ�
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// int nColumn = pNMLV->iSubItem; // Ŭ���� Į���� �ε���
	int nColumn = pNMLV->iItem;
	
	
	MessageBox(_T("CLICKED"), _T("HELLO"), MB_OK);


	for (int i = 0; i < m_IsRealtime.GetItemCount(); i++) // ���� ����Ʈ ��Ʈ�ѿ� �ִ� ������ �� �ڷ� ������ŭ ����
	{
		m_IsRealtime.SetItemData(i, i);		// �ι�°���ڴ� DWORD_PTR �ε� �׳� i�� �ᵵ �Ǵ� ����̴�?
	}

	m_bAscend = !m_bAscend; // ���� ��� ����(Ascend, Descend) 

	// ���� ���õ� ����ü ���� ���� �� ������ �ʱ�ȭ
	SORT_PARAMS sort_params;
	//sort_params.hWnd = GetSafeHwnd();
	sort_params.pList = &m_IsRealtime;
	sort_params.nCol = nColumn;
	sort_params.bAscend = m_bAscend;



	// ���� �Լ� ȣ��
	m_IsRealtime.SortItems(&SortFunc, (LPARAM)&sort_params);

	*pResult = 0;
}







int CALLBACK CSortList::SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{

	//SORT_PARAMS *pSortParams = (SORT_PARAMS *)lParamSort;

	//CListCtrl *pListCtrl = (CListCtrl *)CWnd::FromHandle(pSortParams->hWnd);
	//BOOL bAscend = pSortParams->bAscend;
	//int nCol = pSortParams->nCol;

	CListCtrl *pList = ((SORT_PARAMS *)lParamSort)->pList;
	BOOL bAscend = ((SORT_PARAMS *)lParamSort)->bAscend;
	int nCol = ((SORT_PARAMS *)lParamSort)->nCol;

	LVFINDINFO info1, info2;
	info1.flags = LVFI_PARAM;
	info1.lParam = lParam1;
	info2.flags = LVFI_PARAM;
	info2.lParam = lParam2;
	int irow1 = pList->FindItem(&info1, -1);
	int irow2 = pList->FindItem(&info2, -1);

	CString strItem1 = pList->GetItemText(irow1, nCol);
	CString strItem2 = pList->GetItemText(irow2, nCol);

	return bAscend ? strcmp(strItem1, strItem2) : -strcmp(strItem1, strItem2);



	//CString strItem1 = pListCtrl->GetItemText((int)lParam1, nCol);
	//CString strItem2 = pListCtrl->GetItemText((int)lParam2, nCol);

	//if (IsDigit(strItem1))
	//{
	//	INT nVal1 = _tstoi(strItem1);
	//	INT nVal2 = _tstoi(strItem2);
	//	if (bAscend)
	//		return nVal1 - nVal2;
	//	else
	//		return nVal2 - nVal1;

	//}
	//else
	//{
	//	strItem1.MakeLower();
	//	strItem2.MakeLower();

	//	if (bAscend)
	//		return strItem1.Compare(strItem2);
	//	else
	//		return strItem2.Compare(strItem1);
	//}
}
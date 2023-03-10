// CListBoxFileDrop.cpp: 구현 파일
//

#include "stdafx.h"
#include "CompareTool2.h"
#include "CListBoxFileDrop.h"


// CListBoxFileDrop

IMPLEMENT_DYNAMIC(CListBoxFileDrop, CListBox)

CListBoxFileDrop::CListBoxFileDrop()
{

}

CListBoxFileDrop::~CListBoxFileDrop()
{
}


BEGIN_MESSAGE_MAP(CListBoxFileDrop, CListBox)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()



// CListBoxFileDrop 메시지 처리기




void CListBoxFileDrop::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if (gv_bDropFile) {

		// 파일 드랍 - 글로벌 변수에 저장된 콤보박스 1,2 내용으로 그에 맞는 확장자 혹은 폴더만 드랍할 수 있도록 함
		TCHAR szPathName[MAX_PATH] = { 0, };
		UINT count = DragQueryFile(hDropInfo, 0xFFFFFFFF, szPathName, MAX_PATH);

		for (UINT i = 0; i < count; i++) {
			DragQueryFile(hDropInfo, i, szPathName, MAX_PATH);

			CString fileExtension;
			fileExtension = ExtractFileExtension(szPathName);

			if (gv_strFormat1 == '1') {
				if (gv_strFormat2 == 'd') {
					// dbf 파일
					if (fileExtension == "dbf")
						AddString(szPathName);
				}
				else if (gv_strFormat2 == 's') {
					// shp 폴더 - 확장자가 없는 경우만 들어감
					if (fileExtension == "")
						AddString(szPathName);
						//InsertItem(i, szPathName);
				}
			}
			if (gv_strFormat1 == '2') {
				if (gv_strFormat2 == 't') {
					// txt 파일
					if (fileExtension == "txt") {
						AddString(szPathName);
					}
				}
				else if (gv_strFormat2 == 'm') {
					// mdb 파일
					if (fileExtension == "mdb") {
						AddString(szPathName);
					}
				}
				else if (gv_strFormat2 == 'c') {
					// csv 파일
					if (fileExtension == "csv" || fileExtension == "csl")
						AddString(szPathName);
				}
			}
			if (gv_strFormat1 == '3') {
				if (gv_strFormat2 == 'x') {
					// xlsx 파일
					if (fileExtension == "xlsx" || fileExtension == "xls")
						AddString(szPathName);
				}
				else if (gv_strFormat2 == 'd') {
					// doc 파일
					if (fileExtension == "doc" || fileExtension == "docx")
						AddString(szPathName);
				}
			}
			if (gv_strFormat1 == '4') {
				if (gv_strFormat2 == 'j') {
					// json 파일
					if (fileExtension == "json")
						AddString(szPathName);
				}
				else if (gv_strFormat2 == 'x') {
					// xml 파일
					if (fileExtension == "xml")
						AddString(szPathName);
				}
			}
		}

		::DragFinish(hDropInfo);

		CListBox::OnDropFiles(hDropInfo);
	}
}


CString CListBoxFileDrop::ExtractFileExtension(TCHAR *pathName)
{
	// TODO: 여기에 구현 코드 추가.

	// 파일확장자 추출하기
	CString szFull = (LPCTSTR)pathName;
	CString fileExtension;
	AfxExtractSubString(fileExtension, szFull, 1, '.');

	return fileExtension;
}

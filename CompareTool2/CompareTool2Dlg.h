
// CompareTool2Dlg.h: 헤더 파일
//

#pragma once
#include "CListBoxFileDrop.h"
#include "CSortList.h"



#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"




class CCompareTool2DlgAutoProxy;


// CCompareTool2Dlg 대화 상자
class CCompareTool2Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCompareTool2Dlg);
	friend class CCompareTool2DlgAutoProxy;

// 생성입니다.
public:
	CCompareTool2Dlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	virtual ~CCompareTool2Dlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMPARETOOL2_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	CCompareTool2DlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	CListBoxFileDrop m_lbOld;
	CListBoxFileDrop m_lbNew;
	CComboBox m_cbFormat1;
	CComboBox m_cbFormat2;
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnSelchangeCombo2();
	afx_msg void OnClickedButtonOldPath();
	afx_msg void OnClickedButtonNewPath();
	int m_nClicked;
	void SelectDrawPath();
	CString m_strOldPath;
	CString m_strNewPath;

	afx_msg void OnClickedButtonDel();
	afx_msg void OnClickedButtonCompare();
	CSortList m_lcResult;
	void FileSearch(CString file_path, const char oldornew[]);
	void CCompareDrawListView(vector<vector<CString>> v1, vector<vector<CString>> v2, bool reverse);
	afx_msg void OnClickedButtonExcel();
	void ExportExcel();
	CString ConvertByteTo(double dTempSize);
	void GetDbfFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex);
	afx_msg void OnClickedButtonFieldCheck();
	CCheckListBox m_check_list;
	CTabCtrl m_tab;
	CSortList m_lcDbfResult;
	afx_msg void OnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);

	// shpResult 결과창 전체 저장해 두기위한 변수와 함수
	vector<SHPRESULT> shpResults;
	void ShpResultSaveToGV();
	
	// 체크박스 눌렀을 때
	CButton m_cbNewFile;
	CButton m_cbDeleteFile;
	CButton m_cbChngFile;
	afx_msg void OnClickedCheckBox(UINT value);
	
	afx_msg void OnCustomdrawListResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomdrawListDbfResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomdrawTreeCtrlOld(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawTreeCtrlNew(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawListDbfResult2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomdrawListCsvResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomdrawListTxtResult(NMHDR *pNMHDR, LRESULT *pResult);

	CProgressCtrl m_progressBar;
	CSortList m_lcDbfResult2;
	void OpenMDB();
	void GetCsvFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex);
	CSortList m_lcCsvResult;
	void GetTxtFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex);
	CSortList m_lcTxtResult;
	void GetJsonFile(CString oldPath, bool bOld);
	CEdit m_ecJson;

	void doRapidJson(const rapidjson::Value& oRoot, CString sKey, bool bOld, HTREEITEM hNode);
	CTreeCtrl m_ctrTreeOld;
	CTreeCtrl m_ctrTreeNew;
	void GetXlsxFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex);
	
};



// DBF에 필요한 class 2개

class Field {
public:
	Field();
	~Field();

	// 기록항의 데이터타입으로 아스키코드값을 enum으로 관리
	/*
	B: binary, a string
	C: Character
	D: Date
	N: Numeric
	L: Logical
	M: Memo, a string

	*/
	enum _eRecordItemDataType { B, C, D, G, L, M, N };

	// member various
	char _cTitle[11];
	char _cDataType;
	unsigned char _ucFieldLength;
	char _cFieldContent[100];
	vector<bool> compareExist;
	vector<char*> _vField;
	// vector<string> _vField;
	// vector<CString> _vField;

};


// dbf 파일 정보 불러오는 함수 및 저장 변수 클래스
class DBF {
	/*
	*
	* brief DBF 파일을 load / read / save 하기 위한 클래스이다.
	* details 파일의 디테일을 클래스 멤버 변수에 넣어둔다
	* date 2021-10-27
	*
	*/

public:
	DBF(CString sFilename);
	~DBF();

	// member various
	int _iRecCount;
	int _iFieldCount;
	short _BytesOfFileHead;
	short _BytesOfEachRecord;

	Field* _pField;
	CString _sFilename;
	vector<Field*> _vTable;

	// member func
	void loadFile(CString sFileName);
	void readFile_(CString sFileName);

protected:
	void readFileHead(ifstream& inFile);
	void readFileRecord(ifstream& inFile);
	bool isReadFileOK(CString sFilename);
};

// CSV 에 필요한 class
class Field_csv {
public:
	Field_csv();
	~Field_csv();

	string _cTitle;
	vector<bool> compareExist;
	vector<string> _vField;

};

class CSV {
public:
	CSV(CString sFilename);
	~CSV();

	int _iRecCount;
	int _iFieldCount;
	int _iHeadCount;

	Field_csv* _pField;
	CString _sFilename;
	vector<Field_csv*> _vTable;

	void readFile(CString sFilename);

protected:

	void readFileRecord(ifstream& inFile);
	bool isReadFileOK(CString sfilename);

};


class TXT {
public:
	TXT(CString sFilename);
	~TXT();

	int _iRecCount;
	int _iFieldCount;

	CString _sFilename;
	vector<vector<string>> _vTable;
	vector<vector<bool>> compareExist;

	void readFile();

protected:

	void readFileRecord(ifstream& inFile);
	bool isReadFileOK();
};


// Rapid Json
enum Type {
	kNullType = 0, kFalseType = 1, kTrueType = 2, kObjectType = 3,
	kArrayType = 4, kStringType = 5, kNumberType = 6
};


class Field_xlsx {
public:
	Field_xlsx();
	~Field_xlsx();

	CString _cTitle;
	vector<bool> compareExist;
	vector<CString> _vField;

};

class XLSX {
public:
	XLSX(CString sFilename);
	~XLSX();

	unsigned int _iRecCount;
	unsigned int _iFieldCount;

	CString _sFilename;
	Field_xlsx* _pField;
	vector<Field_xlsx*> _vTable;

	void readHead();
	void readFile();

};
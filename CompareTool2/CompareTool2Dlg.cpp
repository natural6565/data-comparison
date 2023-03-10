
// CompareTool2Dlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "CompareTool2.h"
#include "CompareTool2Dlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"


// excel control 
#include "CApplication.h"
#include "CRange.h"
#include "CStyle.h"
#include "CStyles.h"
#include "CWorkbook.h"
#include "CWorkbooks.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "CFont0.h"
#include "CBorder.h"
#include "CBorders.h"

// #include "json.h"
// #pragma comment(lib, "jsoncpp.lib")
// #pragma warning(disable: 4996)

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// 글씨 깨짐 현상
#include "locale.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// DBF
Field::Field() {}
Field::~Field() {}


DBF::DBF(CString sFilename) { _sFilename = sFilename; }
DBF::~DBF() {}

void DBF::loadFile(CString sFilename) {
	ifstream inFile(sFilename, ios::binary | ios::in);

	if (!isReadFileOK(sFilename))    return;

	readFileHead(inFile);
	// readFileRecord(inFile);
	inFile.close();
}
void DBF::readFile_(CString sFileName) {
	ifstream inFile(sFileName, ios::binary | ios::in);

	if (!isReadFileOK(sFileName))    return;

	readFileHead(inFile);
	readFileRecord(inFile);
	inFile.close();
}

void DBF::readFileHead(ifstream& inFile) {
	if (!isReadFileOK(_sFilename))  return;

	inFile.seekg(4, ios::beg);  // 처음부터 4떨어진 곳부터 읽는 이유 : dbf file 속성에서 4-7에 문서상 기록건수가 저장되어있으므로
	/*  seekg 함수 - 두 번째 매개변수
		ios::cur은 현재 위치로부터
		ios::beg는 시작 위치로부터
		ios::end는 끝 위치로부터 이동 음수값도 가능*/
	inFile.read((char*)&_iRecCount, sizeof(int));			// 4-7 이므로 4바이트. int로 읽음
	inFile.read((char*)&_BytesOfFileHead, sizeof(short));   // 8-9 2바이트. short으로 읽음
	inFile.read((char*)&_BytesOfEachRecord, sizeof(short)); // 10-11 2바이트 short
	_iFieldCount = (_BytesOfFileHead - 33) / 32;			// (n개*32바이트) 기록항정보를 나눔. 총 몇개의 컬럼이 있는지 확인

	for (int i = 0; i < _iFieldCount; i++) {
		// head 레코드값부터 읽기 - Field Descriptor Array
		inFile.seekg(32 + 32 * i, ios::beg);

		_pField = new Field;
		for (int j = 0; j < 11; j++) {
			inFile.read(_pField->_cTitle + j, sizeof(char));
		}

		inFile.read(&_pField->_cDataType, sizeof(char));
		inFile.seekg(4, ios::cur);
		inFile.read((char*)&_pField->_ucFieldLength, sizeof(char));


		_vTable.push_back(_pField);
		// _pField = nullptr;
	}
}

void DBF::readFileRecord(ifstream& inFile) {
	// 헤더 길이만큼 건너뛰고
	inFile.seekg(_BytesOfFileHead, ios::beg);

	char cDeleteTag;

	char* cRecord;

	for (int i = 0; i < _iRecCount; i++) {
		inFile.read(&cDeleteTag, sizeof(char));

		for (unsigned int j = 0; j < _vTable.size(); j++) {

			// 마지막 하나는 \0을 넣어 끝을 알게 하려고 +1 함
			int temp_len = (int)_vTable[j]->_ucFieldLength + 1;
			cRecord = new char[temp_len];							// 데이터 길이 + 1만큼

			inFile.read(cRecord, sizeof(char)*(temp_len-1));		// 길이보다 +1 이므로 -1만큼 파일을 읽고

			// for (int k = 0; k < temp_len; k++) {
			// 	inFile.read(cRecord + k, sizeof(char));
			// }

			cRecord[temp_len - 1] = '\0';							// 마지막에 \0으로 마지막을 알림

			// 각각의 필드의 값을 저장하고 비교시 사용할 boolean 도 false 기본값으로 정해줌
			(_vTable[j]->_vField).push_back(cRecord);
			_vTable[j]->compareExist.push_back(false);

			// delete[] cRecord;
			cRecord = nullptr;
		}
	}
}

bool DBF::isReadFileOK(CString sFilename) {
	ifstream inFile(sFilename, ios::binary | ios::in);
	if (inFile.good()) {
		inFile.close();
		return true;
	}
	else {
		return false;
	}
}






// CSV 

Field_csv::Field_csv() {};
Field_csv::~Field_csv() {};

CSV::CSV(CString sFilename) { _sFilename = sFilename; }
CSV::~CSV() {}

bool CSV::isReadFileOK(CString sFilename) {
	ifstream inFile(sFilename);
	if (inFile.good()) {
		inFile.close();
		return true;
	}
	else {
		return false;
	}
}


void CSV::readFile(CString sFilename) {
	ifstream inFile(sFilename);

	// 파일을 열지 못하면 끝내기
	if (!isReadFileOK(sFilename))
		return;

	readFileRecord(inFile);
	inFile.close();
}

void CSV::readFileRecord(ifstream& inFile) {
	if (!isReadFileOK(_sFilename))   return;

	stringstream ss;
	bool inquotes = false;

	int field_cnt = 0;
	int record_cnt = 0;

	_pField = new Field_csv;

	while (inFile.good()) {
		char c = inFile.get();

		if (c == ',') {
			field_cnt++;

			_pField->_cTitle = ss.str();
			_vTable.push_back(_pField);
			ss.str("");

			_pField = new Field_csv;
		}
		else if (c == '\r' || c == '\n') {

			_pField->_cTitle = ss.str();
			_vTable.push_back(_pField);
			ss.str("");

			field_cnt++;                // ',' 는 필드값보다 하나 적게 있으므로 '\n'을 찾았을 때 1 더해주어야 한다.
			_iFieldCount = field_cnt;   // 그 값을 _iField 에 저장(csv member 변수)

			break;
		}
		else {
			ss << c;
		}
	}
	// 헤더까지 몇자있는지 확인 - 굳이 필요없음
	// _iHeadCount = inFile.tellg();

	field_cnt = 0;

	while (!inFile.eof()) {
		char c = inFile.get();


		if (!inquotes && c == '"') {
			inquotes = true;
		}
		else if (inquotes && c == '"') {
			if (inFile.peek() == '"')
			{
				inFile.get(); //ss << (char)inFile.get(); <- 만약 " 를 넣고 싶으면
			}
			else
			{
				inquotes = false;
			}
		}

		else if (!inquotes &&  c == ',') {
			string temp = ss.str();

			// iFieldCount를 field_cnt로 나눈 나머지로 필드 구분
			const int cur_field = field_cnt % _iFieldCount;
			(_vTable[cur_field]->_vField).push_back(temp.c_str());
			(_vTable[cur_field]->compareExist).push_back(false);
			ss.str("");

			// field 몇번째인지 확인하기 위한 변수
			field_cnt++;
		}

		// row 끝나는 조건문이므로 record_cnt++
		else if (!inquotes && (c == '\r' || c == '\n')) {
			if (inFile.peek() == '\n') { inFile.get(); }
			string temp = ss.str();

			const int cur_field = _iFieldCount - 1;
			(_vTable[cur_field]->_vField).push_back(temp.c_str());
			(_vTable[cur_field]->compareExist).push_back(false);      // 각 셀마다 false를 디폴트로함
			ss.str("");

			field_cnt = 0;
			record_cnt++;
		}
		else {
			ss << c;
		}
	}

	// 레코드 갯수
	_iRecCount = record_cnt;

}






// TXT

TXT::TXT(CString filename) {
	_sFilename = filename;
	// 6개로 fix 되어 있음
	_iFieldCount = 1;
}
TXT::~TXT() {}

bool TXT::isReadFileOK() {
	ifstream inFile(_sFilename);
	if (inFile.good()) {
		inFile.close();
		return true;
	}
	else {
		return false;
	}
}

void TXT::readFile() {
	ifstream inFile(_sFilename);

	// 파일을 열지 못하면 끝내기
	if (!isReadFileOK())
		return;

	readFileRecord(inFile);
	inFile.close();
}


void TXT::readFileRecord(ifstream& inFile) {
	if (!isReadFileOK())   return;

	stringstream ss;

	int record_cnt = 0;
	bool header_cnt = false;

	vector<string> temp_data;
	vector<bool> temp_bool;
	
	while (inFile.good()) {
		char c = inFile.get();

		if (c == ',' || c == ';' || c == ':') {

			if (!header_cnt) {
				_iFieldCount++;
			}
			temp_bool.push_back(false);
			temp_data.push_back(ss.str());
			ss.str("");

		}
		else if (c == '\r' || c == '\n' || inFile.eof()) {

			if (!header_cnt) {
				// _iFieldCount++;
				header_cnt = true;
			}
			temp_bool.push_back(false);
			temp_data.push_back(ss.str());
			ss.str("");

			compareExist.push_back(temp_bool);
			_vTable.push_back(temp_data);
			temp_data.clear();
			
			// 레코드가 몇개인지 확인
			record_cnt++;
		}
		else {
			ss << c;
		}
	}

	// 레코드 갯수
	_iRecCount = record_cnt;
}






// XLSX
Field_xlsx::Field_xlsx() {};
Field_xlsx::~Field_xlsx() {};

XLSX::XLSX(CString sFilename) { _sFilename = sFilename; };
XLSX::~XLSX() {};

void XLSX::readHead() {
	CApplication objApp;
	CWorkbook objBook;
	CWorkbooks objBooks;
	CWorksheet  objSheet;
	CWorksheets objSheets;
	CRange  objRange;

	COleVariant VOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant VFalse((short)FALSE);
	COleVariant VTrue((short)TRUE);
	COleException* pe = new COleException;

	if (!objApp.CreateDispatch(_T("Excel.Application")))
		throw pe;


	objApp.put_Visible(FALSE);
	objApp.put_UserControl(TRUE);

	// 파일 오픈하기 위한 형변환
	LPCTSTR strOpenFile = _sFilename;

	// 파일 오픈해서 sheets 를 가져오는 과정
	objBooks = objApp.get_Workbooks();
	objBook = objBooks.Open(strOpenFile, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional);
	objSheets = objBook.get_Worksheets();

	// 시트 열기
	objSheet = objSheets.get_Item(COleVariant((short)1));
	objSheet.Activate();


	CString strdata;
	CString strcel = 0;


	// VARIANT ret;

	// 범위 알아내기
	objRange = objSheet.get_UsedRange();

	COleSafeArray saRet(objRange.get_Value(VOptional));


	long numelementsrow = 0;
	long numelementcol = 0;

	saRet.GetUBound(1, &numelementsrow);
	saRet.GetUBound(2, &numelementcol);

	// printf("[ %ld , %ld ]\n", numelementsrow, numelementcol);

	// 전체 row 에서 Column Name 은 하나 빼준다.
	_iRecCount = numelementsrow - 1;
	_iFieldCount = numelementcol;

	long index[2];
	VARIANT val;


	_pField = new Field_xlsx;

	

	for (int iCol = 1; iCol <= numelementcol; iCol++)
	{
		index[0] = 1;
		index[1] = iCol;

		saRet.GetElement(index, &val);

		// 현재 열을 확인하기 위한 cur_field
		const int cur_field = (iCol - 1) % _iFieldCount;

		// 셀 값을 저장할 변수
		CString tempValue;

		switch (val.vt)
		{
		case VT_R8:

			// printf("INT TYPE\t");
			
			tempValue.Format(_T("%f"), val.dblVal);

			_pField->_cTitle = tempValue;
			_vTable.push_back(_pField);

			_pField = new Field_xlsx;
			
			break;


		case VT_BSTR:

			// printf("STRING TYPE\t");
			
			tempValue.Format(_T("%s"), (CString)val.bstrVal);

			_pField->_cTitle = tempValue;
			_vTable.push_back(_pField);

			_pField = new Field_xlsx;
			
			break;

			
		case VT_EMPTY:

			// printf("NULL\t");
			
			tempValue.Format(_T("NULL"));

			_pField->_cTitle = tempValue;
			_vTable.push_back(_pField);

			_pField = new Field_xlsx;
			

			break;
		}
	}
	



	objRange.ReleaseDispatch();
	objSheet.ReleaseDispatch();
	objSheets.ReleaseDispatch();
	objBook.ReleaseDispatch();
	objBooks.ReleaseDispatch();
	// objRanges.ReleaseDispatch();
	objApp.Quit();
	objApp.ReleaseDispatch();

}



void XLSX::readFile() {
	CApplication objApp;
	CWorkbook objBook;
	CWorkbooks objBooks;
	CWorksheet  objSheet;
	CWorksheets objSheets;
	CRange  objRange;

	COleVariant VOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant VFalse((short)FALSE);
	COleVariant VTrue((short)TRUE);
	COleException* pe = new COleException;

	if (!objApp.CreateDispatch(_T("Excel.Application")))
		throw pe;


	objApp.put_Visible(FALSE);
	objApp.put_UserControl(TRUE);

	// 파일 오픈하기 위한 형변환
	LPCTSTR strOpenFile = _sFilename;

	// 파일 오픈해서 sheets 를 가져오는 과정
	objBooks = objApp.get_Workbooks();
	objBook = objBooks.Open(strOpenFile, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional, VOptional);
	objSheets = objBook.get_Worksheets();

	// 시트 열기
	objSheet = objSheets.get_Item(COleVariant((short)1));
	objSheet.Activate();


	CString strdata;
	CString strcel = 0;


	// VARIANT ret;

	// 범위 알아내기
	objRange = objSheet.get_UsedRange();

	COleSafeArray saRet(objRange.get_Value(VOptional));


	long numelementsrow = 0;
	long numelementcol = 0;

	saRet.GetUBound(1, &numelementsrow);
	saRet.GetUBound(2, &numelementcol);

	// printf("[ %ld , %ld ]\n", numelementsrow, numelementcol);

	// 전체 row 에서 헤더 Column Name 은 하나 빼준다.
	_iRecCount = numelementsrow - 1;
	_iFieldCount = numelementcol;

	long index[2];
	VARIANT val;


	_pField = new Field_xlsx;

	for (int iRow = 1; iRow <= numelementsrow; iRow++)
	{

		for (int iCol = 1; iCol <= numelementcol; iCol++)
		{
			index[0] = iRow;
			index[1] = iCol;

			saRet.GetElement(index, &val);

			// 현재 열을 확인하기 위한 cur_field
			const int cur_field = (iCol-1) % _iFieldCount;

			// 셀 값을 저장할 변수
			CString tempValue;
			
			COleDateTime DateInfo(val);
			

			switch (val.vt)
			{
			case VT_R8:
				// TRACE("\t\t%1.2f", val.dblVal);

				// printf("\t\t%1.2f", val.dblVal);

				
				tempValue.Format(_T("%.0f"), val.dblVal);

				// 헤더 - 필드명 가져오기
				if (iRow == 1) {
					
					_pField->_cTitle = tempValue;
					_vTable.push_back(_pField);

					_pField = new Field_xlsx;
				}
				else {

					(_vTable[cur_field]->_vField).push_back(tempValue);
					(_vTable[cur_field]->compareExist).push_back(false);

				}


				break;



			case VT_BSTR:
				// TRACE("\t\t%s", (CString)val.bstrVal);

				// printf("<STRING TYPE>\t%s", (CString)val.bstrVal);

				saRet.PutElement(index, COleVariant((CString)val.bstrVal));

				tempValue.Format(_T("%s"), (CString)val.bstrVal);

				if (iRow == 1) {
					
					_pField->_cTitle = tempValue;
					_vTable.push_back(_pField);

					_pField = new Field_xlsx;
				}
				else {
					(_vTable[cur_field]->_vField).push_back(tempValue);
					(_vTable[cur_field]->compareExist).push_back(false);
					// printf("%s\t", tempValue);
				}

				break;

			case VT_DATE:
				//TRACE("\t\t<empty");

				//tempValue.Format(_T("%s"), (CString)val.date);

				// printf("\t<DATE TYPE>\n");
				
				tempValue = DateInfo.Format(_T("%Y-%m-%d")); //(_T("%d-%d-%d", COleDateTime::GetYear, COleDateTime::GetMonth, COleDateTime::GetDay));//(_T("%Y-%m-%d"));//DateInfo.GetYear(), DateInfo.GetMonth(), DateInfo.GetDay()));


				


				if (iRow == 1) {

					_pField->_cTitle = tempValue;
					_vTable.push_back(_pField);

					_pField = new Field_xlsx;
				}
				else {
					(_vTable[cur_field]->_vField).push_back(tempValue);
					(_vTable[cur_field]->compareExist).push_back(false);
				}

				break;



			case VT_EMPTY:
				//TRACE("\t\t<empty");

				// printf("\t\t<empty");

				tempValue.Format(_T(""));

				if (iRow == 1) {
					
					_pField->_cTitle = tempValue;
					_vTable.push_back(_pField);

					_pField = new Field_xlsx;
				}
				else {
					(_vTable[cur_field]->_vField).push_back(tempValue);
					(_vTable[cur_field]->compareExist).push_back(false);
				}

				break;
			}
		}
	}



	objRange.ReleaseDispatch();
	objSheet.ReleaseDispatch();
	objSheets.ReleaseDispatch();
	objBook.ReleaseDispatch();
	objBooks.ReleaseDispatch();
	// objRanges.ReleaseDispatch();

	objApp.Quit();
	objApp.ReleaseDispatch();

}





// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCompareTool2Dlg 대화 상자


IMPLEMENT_DYNAMIC(CCompareTool2Dlg, CDialogEx);

CCompareTool2Dlg::CCompareTool2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMPARETOOL2_DIALOG, pParent)
	, m_strOldPath(_T(""))
	, m_strNewPath(_T(""))
	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	m_pAutoProxy = nullptr;
}

CCompareTool2Dlg::~CCompareTool2Dlg()
{
	// 이 대화 상자에 대한 자동화 프록시가 있을 경우 이 대화 상자에 대한
	//  후방 포인터를 null로 설정하여
	//  대화 상자가 삭제되었음을 알 수 있게 합니다.
	if (m_pAutoProxy != nullptr)
		m_pAutoProxy->m_pDialog = nullptr;
}

void CCompareTool2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_OLD, m_lbOld);
	DDX_Control(pDX, IDC_LIST_NEW, m_lbNew);
	DDX_Control(pDX, IDC_COMBO1, m_cbFormat1);
	DDX_Control(pDX, IDC_COMBO2, m_cbFormat2);
	DDX_Text(pDX, IDC_EDIT_OLD_PATH, m_strOldPath);
	DDX_Text(pDX, IDC_EDIT_NEW_PATH, m_strNewPath);
	DDX_Control(pDX, IDC_LIST_RESULT, m_lcResult);
	DDX_Control(pDX, IDC_LIST_FIELD, m_check_list);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST_DBF_RESULT, m_lcDbfResult);
	DDX_Control(pDX, IDC_CHECK_NEW_FILE, m_cbNewFile);
	DDX_Control(pDX, IDC_CHECK_DELETE_FILE, m_cbDeleteFile);
	DDX_Control(pDX, IDC_CHECK_CHNG_FILE, m_cbChngFile);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progressBar);
	DDX_Control(pDX, IDC_LIST_DBF_RESULT2, m_lcDbfResult2);
	DDX_Control(pDX, IDC_LIST_CSV_RESULT, m_lcCsvResult);
	DDX_Control(pDX, IDC_LIST_TXT_RESULT, m_lcTxtResult);
	DDX_Control(pDX, IDC_JSON_EDIT, m_ecJson);
	DDX_Control(pDX, IDC_TREE_OLD, m_ctrTreeOld);
	DDX_Control(pDX, IDC_TREE_NEW, m_ctrTreeNew);
}

BEGIN_MESSAGE_MAP(CCompareTool2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CCompareTool2Dlg::OnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CCompareTool2Dlg::OnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON_OLD_PATH, &CCompareTool2Dlg::OnClickedButtonOldPath)

	ON_BN_CLICKED(IDC_BUTTON_NEW_PATH, &CCompareTool2Dlg::OnClickedButtonNewPath)
	ON_BN_CLICKED(IDC_BUTTON_DEL, &CCompareTool2Dlg::OnClickedButtonDel)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE, &CCompareTool2Dlg::OnClickedButtonCompare)
	ON_BN_CLICKED(IDC_BUTTON_EXCEL, &CCompareTool2Dlg::OnClickedButtonExcel)
	ON_BN_CLICKED(IDC_BUTTON_FIELD_CHECK, &CCompareTool2Dlg::OnClickedButtonFieldCheck)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CCompareTool2Dlg::OnSelchangeTab1)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_NEW_FILE,IDC_CHECK_CHNG_FILE, &CCompareTool2Dlg::OnClickedCheckBox)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_RESULT, &CCompareTool2Dlg::OnCustomdrawListResult)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DBF_RESULT, &CCompareTool2Dlg::OnCustomdrawListDbfResult)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TREE_OLD, &CCompareTool2Dlg::OnCustomdrawTreeCtrlOld)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TREE_NEW, &CCompareTool2Dlg::OnCustomdrawTreeCtrlNew)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DBF_RESULT2, &CCompareTool2Dlg::OnCustomdrawListDbfResult2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_CSV_RESULT, &CCompareTool2Dlg::OnCustomdrawListCsvResult)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_TXT_RESULT, &CCompareTool2Dlg::OnCustomdrawListTxtResult)

END_MESSAGE_MAP()


// CCompareTool2Dlg 메시지 처리기

BOOL CCompareTool2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	// 한글 깨짐 방지 - 현재 문제점은 해결되지 않음. ㅠㅠ...
	// setlocale(LC_ALL, "ko_KR.UTF-8");

	// File Drop
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	DragAcceptFiles();

	// 콤보박스1 
	m_cbFormat1.AddString(_T("1. 원도 데이터 비교"));
	m_cbFormat1.AddString(_T("2. 변환 메타 파일 비교"));
	m_cbFormat1.AddString(_T("3. 검수 리스트 파일 비교"));
	m_cbFormat1.AddString(_T("4. 기타 파일 비교"));

	// 결과창 셋팅
	char *szText[9] = { "파일명", "사이즈", "날짜", "<=>", "날짜", "사이즈", "파일명", "용량증감율", "증감바이트수" };
	int nWidth[9] = { 250,90,100,40,100,90,250,80,90 };

	LV_COLUMN iCol;
	iCol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iCol.fmt = LVCFMT_LEFT;
	m_lcResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	for (int i = 0; i < 9; i++) {
		iCol.pszText = (LPTSTR)szText[i];
		iCol.iSubItem = i;
		iCol.cx = nWidth[i];
		m_lcResult.InsertColumn(i, &iCol);
	}

	// DBF 결과창 셋팅
	char *szTextDbf[6] = { "파일명", "[New기준] 삭제된 필드", "[New기준] 추가된 필드", "일치여부", "Old 레코드 수", "New 레코드 수" };
	int nWidthDbf[9] = { 300,200,200,80,100,100 };

	LV_COLUMN iColDbf;
	iColDbf.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iColDbf.fmt = LVCFMT_LEFT;
	m_lcDbfResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	for (int i = 0; i < 6; i++) {
		iColDbf.pszText = (LPTSTR)szTextDbf[i];
		iColDbf.iSubItem = i;
		iColDbf.cx = nWidthDbf[i];
		m_lcDbfResult.InsertColumn(i, &iColDbf);
	}

	// Tab 첫화면 설정
	TC_ITEM tabItem;
	const int tabCnt = 10;
	LPSTR tabName[tabCnt] = { _T("   Shp 폴더 결과창   "), _T("   DBF 요약 결과창   "), _T("   DBF 상세 결과창   "), _T("   Mdb 결과창   "), _T("   Txt 결과창   "), _T("   CSV 결과창   "), _T("   Doc 결과창   "), _T("   Xlsx 결과창   "), _T("   Json 결과창   "), _T("   Xml 결과창   ")};
	for (int nIndex = 0; nIndex < tabCnt; nIndex++) {
		tabItem.mask = TCIF_TEXT;
		tabItem.pszText = tabName[nIndex];
		m_tab.InsertItem(nIndex, &tabItem);
	}


	// JSON EDIT 숨기기
	GetDlgItem(IDC_JSON_EDIT)->ShowWindow(FALSE);

	// Tab 화면 보이기 설정
	GetDlgItem(IDC_LIST_RESULT)->ShowWindow(TRUE);
	GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
	GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);
	GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);
	GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
	GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
	GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);


	// 필터 체크 넣어 두기
	((CButton*)GetDlgItem(IDC_CHECK_NEW_FILE))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_CHECK_DELETE_FILE))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_CHECK_CHNG_FILE))->SetCheck(TRUE);

	// 프로그래스바
	m_progressBar.SetRange(0, 100);


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CCompareTool2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCompareTool2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCompareTool2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 컨트롤러에서 해당 개체 중 하나를 계속 사용하고 있을 경우
//  사용자가 UI를 닫을 때 자동화 서버를 종료하면 안 됩니다.  이들
//  메시지 처리기는 프록시가 아직 사용 중인 경우 UI는 숨기지만,
//  UI가 표시되지 않아도 대화 상자는
//  남겨 둡니다.

void CCompareTool2Dlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void CCompareTool2Dlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CCompareTool2Dlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CCompareTool2Dlg::CanExit()
{
	// 프록시 개체가 계속 남아 있으면 자동화 컨트롤러에서는
	//  이 응용 프로그램을 계속 사용합니다.  대화 상자는 남겨 두지만
	//  해당 UI는 숨깁니다.
	if (m_pAutoProxy != nullptr)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}



void CCompareTool2Dlg::OnSelchangeCombo1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	// 프로그래스 바 0% 만들기
	m_progressBar.SetPos(0);


	// 콤보박스2 내용 지우기
	m_cbFormat2.ResetContent();

	// 콤보박스1 셀렉트 내용 가져오기
	CString selected_text;

	m_cbFormat1.GetLBText(m_cbFormat1.GetCurSel(), selected_text);

	if (selected_text[0] == '1') {
		m_cbFormat2.AddString(_T("shp 폴더 비교"));
		m_cbFormat2.AddString(_T("dbf 파일 비교"));
	}
	else if (selected_text[0] == '2') {
		m_cbFormat2.AddString(_T("mdb 파일 비교"));
		m_cbFormat2.AddString(_T("txt 파일 비교"));
		m_cbFormat2.AddString(_T("cvs 파일 비교"));
	}
	else if (selected_text[0] == '3') {
		m_cbFormat2.AddString(_T("xlsx 파일 비교"));
		m_cbFormat2.AddString(_T("doc 파일 비교"));
	}
	else if (selected_text[0] == '4') {
		m_cbFormat2.AddString(_T("json 파일 비교"));
		m_cbFormat2.AddString(_T("xml 파일 비교"));
	}

	GetDlgItem(IDC_COMBO2)->EnableWindow(TRUE);

}


void CCompareTool2Dlg::OnSelchangeCombo2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 콤보박스 1과 2의 내용 글로벌 변수에 저장하기
	CString selected_text;
	CString selected_text2;

	m_cbFormat1.GetLBText(m_cbFormat1.GetCurSel(), selected_text);
	m_cbFormat2.GetLBText(m_cbFormat2.GetCurSel(), selected_text2);


	gv_strFormat1 = selected_text.GetAt(0);			// = selected_text[0];
	gv_strFormat2 = selected_text2.GetAt(0);		// = selected_text2[0];


	// gv_bDropFile 글로벌 변수 True 로 파일 드랍 가능
	gv_bDropFile = true;

	// File Dir OLD & NEW 경로 지우기
	int cnt1 = m_lbOld.GetCount();
	int cnt2 = m_lbNew.GetCount();

	for (int i = cnt1 - 1; i >= 0; i--) {
		m_lbOld.DeleteString(i);
	}
	for (int i = cnt2 - 1; i >= 0; i--) {
		m_lbNew.DeleteString(i);
	}

	// 필드값확인 or 비교하기 버튼 활성화하기
	// shp 폴더 / json 는 비교하기 버튼 바로 활성화
	if (gv_strFormat2 == 's' || gv_strFormat2 == 'j') {
		GetDlgItem(IDC_BUTTON_COMPARE)->EnableWindow(TRUE);
	}
	// shp / json 제외한 나머지 form은 필드값 체크
	else {
		GetDlgItem(IDC_BUTTON_FIELD_CHECK)->EnableWindow(TRUE);
	}

	if (gv_strFormat2 == 'j') {
		GetDlgItem(IDC_JSON_EDIT)->ShowWindow(TRUE);
		GetDlgItem(IDC_LIST_FIELD)->ShowWindow(FALSE);


	}
	else {
		GetDlgItem(IDC_LIST_FIELD)->ShowWindow(TRUE);
		GetDlgItem(IDC_JSON_EDIT)->ShowWindow(FALSE);
	}

	// 폴더 경로 지우기
	Old.folder_name.Format("");
	New.folder_name.Format("");
}


void CCompareTool2Dlg::OnClickedButtonOldPath()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nClicked = 0;
	SelectDrawPath();
}


void CCompareTool2Dlg::OnClickedButtonNewPath()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nClicked = 1;
	SelectDrawPath();
}


// 파일 혹은 폴더 선택하여 경로를 리스트박스에 그리기
void CCompareTool2Dlg::SelectDrawPath()
{
	// TODO: 여기에 구현 코드 추가.

	// shp 일때 - 폴더 열리기
	if (gv_strFormat1 == '1' && gv_strFormat2 == 's') {

		BROWSEINFO BrInfo;
		TCHAR szBuffer[512];

		::ZeroMemory(&BrInfo, sizeof(BROWSEINFO));
		::ZeroMemory(szBuffer, 512);

		BrInfo.hwndOwner = GetSafeHwnd();
		BrInfo.lpszTitle = _T("폴더를 선택하세요");
		BrInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
		LPITEMIDLIST pItemIdList = ::SHBrowseForFolder(&BrInfo);
		::SHGetPathFromIDList(pItemIdList, szBuffer);               // 파일경로 읽어오기

		// 경로를 가져와 사용
		CString str;
		str.Format(_T("%s"), szBuffer);

		// 파일 경로가 아무것도 안나오는 것 방지
		if (str.GetLength() == 0) {
			return;
		}
		
		if (!m_nClicked) {
			m_lbOld.AddString(str);
		}
		else if (m_nClicked) {
			m_lbNew.AddString(str);
		}

	}
	else {
		CString str = _T("All files(*.*)|*.*|"); // 모든 파일 표시
		// _T("Excel 파일 (*.xls, *.xlsx) |*.xls; *.xlsx|"); 와 같이 확장자를 제한하여 표시할 수 있음
		CFileDialog dlg(TRUE, _T("*.dat"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str, this);

		if (dlg.DoModal() == IDOK)
		{
			CString strPathName = dlg.GetPathName();
			// 파일 경로를 가져와 사용


			// 파일 경로가 아무것도 안나오는 것 방지
			if (strPathName.GetLength() == 0) {
				return;
			}

			if (!m_nClicked) {
				m_lbOld.AddString(strPathName);
			}
			else if (m_nClicked) {
				m_lbNew.AddString(strPathName);
			}
		}
	}
}


void CCompareTool2Dlg::OnClickedButtonDel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int cnt = m_lbOld.GetCurSel();
	if (cnt != LB_ERR) {
		m_lbOld.DeleteString(cnt);
	}

	int cnt2 = m_lbNew.GetCurSel();
	if (cnt2 != LB_ERR) {
		m_lbNew.DeleteString(cnt2);
	}

}


// 비교하기 버튼을 눌렀을 때 
void CCompareTool2Dlg::OnClickedButtonCompare()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 필터 체크버튼 체크
	((CButton*)GetDlgItem(IDC_CHECK_NEW_FILE))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_CHECK_DELETE_FILE))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_CHECK_CHNG_FILE))->SetCheck(TRUE);


	// OLD NEW 경로 edit control에 그리기
	m_lbOld.GetText(0, m_strOldPath);
	m_lbNew.GetText(0, m_strNewPath);
	UpdateData(FALSE);

	// list control item 삭제
	m_lcResult.DeleteAllItems();
	m_lcDbfResult.DeleteAllItems();
	m_lcDbfResult2.DeleteAllItems();
	m_lcCsvResult.DeleteAllItems();
	m_lcTxtResult.DeleteAllItems();

	// tree ctrl 아이템 전체 삭제
	m_ctrTreeOld.DeleteAllItems();
	m_ctrTreeNew.DeleteAllItems();
	

	// File Drop  못하도록
	gv_bDropFile = false;

	// 전역변수 비우기
	dbf_result_save.clear();


	// Compare 처리
	CString tempPathOld, tempPathNew;
	CListBox *p_list = (CListBox *)GetDlgItem(IDC_LIST_OLD);
	CListBox *p_list2 = (CListBox *)GetDlgItem(IDC_LIST_NEW);
	p_list->GetText(0, tempPathOld);
	p_list2->GetText(0, tempPathNew);


	// 경로가 입력되지 않았을 때 경고 메세지 날리기
	if (tempPathNew.IsEmpty() || tempPathOld.IsEmpty()) {
		MessageBox(_T("경로를 넣어주세요"), _T("경로 입력 누락"), MB_ICONERROR);
		return;
	}


	// 전역변수 벡터 값 지우기
	Old.v.clear();
	New.v.clear();


	// CCheckListBox 에서 선택되어진 값 확인하기 (Field값 확인)
	vector<int> selectedFieldIndex;
	
	// CCheckListBox 에서 선택되어진 값이 selectedFieldIndex 벡터에 저장된다. 인덱스 값으로 사용하면 됨
	if (m_check_list.GetCount() != 0) {
		for (int i = 0; i < m_check_list.GetCount(); i++) {
			if (m_check_list.GetCheck(i) == BST_CHECKED) {
				// 체크된 필드의 인덱스를 저장
				selectedFieldIndex.push_back(i);
				// printf("selectedFieldIndex [%d] ", i);
			}
		}
	}
	   	 

	// 원도 데이터 비교
	if (gv_strFormat1 == '1') {
		// shp 일때
		if (gv_strFormat2 == 's') {

			// tab 바꾸기, - nmhdr - sendmessage.. TCN_SELCHANGE
			m_tab.SetCurSel(0);

			// 파일을 찾아서 Old.v or New.v - gv_save 구조체에 저장
			FileSearch(tempPathOld, "old");
			FileSearch(tempPathNew, "new");

		}
		// dbf 일때
		else if (gv_strFormat2 == 'd') {

			// tab 바꾸기
			m_tab.SetCurSel(2);

			GetDbfFile(tempPathOld, tempPathNew, selectedFieldIndex);
		}
	}
	else if (gv_strFormat1 == '2') {
		// mdb 일때
		if (gv_strFormat2 == 'm') {
					   			 				
			// tab 바꾸기
			m_tab.SetCurSel(3);

			// OpenMDB();

		}
		else if (gv_strFormat2 == 't') {

			// tab 바꾸기
			m_tab.SetCurSel(4);

			GetTxtFile(tempPathOld, tempPathNew, selectedFieldIndex);


		}
		else if (gv_strFormat2 == 'c') {

			// tab 바꾸기
			m_tab.SetCurSel(5);

			GetCsvFile(tempPathOld, tempPathNew, selectedFieldIndex);

		}
	}
	else if (gv_strFormat1 == '3') {
		// doc 일때
		if (gv_strFormat2 == 'd') {
			
			// tab 바꾸기
			m_tab.SetCurSel(6);
		}
		// xlsx 일 때
		else if (gv_strFormat2 == 'x') {

			// tab 바꾸기
			m_tab.SetCurSel(7);


			GetXlsxFile(tempPathOld, tempPathNew, selectedFieldIndex);


		}
	}
	else if (gv_strFormat1 == '4') {
		// json 일때
		if (gv_strFormat2 == 'j') {

			// tab 바꾸기
			m_tab.SetCurSel(8);

			// IDC_JSON_EDIT
			CString inputValue;
			m_ecJson.GetWindowTextA(inputValue);

			gv_jsonInputValue.Format(_T("%s"), inputValue);

			// old 파일 그리기
			GetJsonFile(tempPathOld, true);
			// new 파일 그리기
			GetJsonFile(tempPathNew, false);

		}
		else if (gv_strFormat2 == 'x') {

			// tab 바꾸기
			m_tab.SetCurSel(9);
		}
	}
	// File Dir OLD & NEW 경로 지우기
	int cnt1 = m_lbOld.GetCount();
	int cnt2 = m_lbNew.GetCount();

	for (int i = cnt1 - 1; i >= 0; i--) {
		m_lbOld.DeleteString(i);
	}
	for (int i = cnt2 - 1; i >= 0; i--) {
		m_lbNew.DeleteString(i);
	}

	if (gv_strFormat1 == '1' && gv_strFormat2 == 's') {
		// old와 new 비교 후 list view 에 결과 그리기
		CCompareDrawListView(Old.v, New.v, false);
		CCompareDrawListView(New.v, Old.v, true);
	}

	// shp 결과창 데이터 저장해두기
	ShpResultSaveToGV();

	// 콤보박스2 / 필드확인 / 비교하기 버튼 비활성화
	GetDlgItem(IDC_COMBO2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_FIELD_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_COMPARE)->EnableWindow(FALSE);


	// listview 체크박스 전부 지우기
	m_check_list.ResetContent();


	if (gv_strFormat1 == '1') {
		// shp 일때
		if (gv_strFormat2 == 's') {

			// shp_result_save 구조체에 들어있는 값 중 - old/new에 모두 존재하는 dbf 파일만 걸러내서 확인
			int shpResultsCnt = shpResults.size();

			int nIndex = 0;
			UpdateData(TRUE);
			LVITEM iItem;
			iItem.mask = LVIF_TEXT;
			iItem.iItem = nIndex;

			double dForRate = 100.0 / (double)shpResultsCnt;


			for (int i = 0; i < shpResultsCnt; i++) {

				m_progressBar.SetPos(dForRate);

				if (shpResults[i].compare == "변경" || shpResults[i].compare == "<=>") {

					// dbf 파일 열어서 필드값 확인
					CString oldPath(shpResults[i].oldFileName);
					CString newPath(shpResults[i].newFileName);

					CString extension;

					AfxExtractSubString(extension, oldPath, 1, '.');

					if (extension != "dbf") {
						continue;
					}

					CString baseOldPath = m_strOldPath.GetString();
					CString baseNewPath = m_strNewPath.GetString();

					DBF oldFile(baseOldPath+_T("/")+oldPath);
					oldFile.loadFile(baseOldPath + _T("/") + oldPath);

					DBF newFile(baseNewPath + _T("/") + newPath);
					newFile.loadFile(baseNewPath + _T("/") + newPath);

					// 레코드 값 구하기
					CString strOldRecordCnt;
					CString strNewRecordCnt;

					int oldRecCount = oldFile._iRecCount;
					int newRecCount = newFile._iRecCount;

					strOldRecordCnt.Format("%d", oldRecCount);
					strNewRecordCnt.Format("%d", newRecCount);

					
					// 필드 값 구하기
					int oldFileCnt = oldFile._iFieldCount;
					int newFileCnt = newFile._iFieldCount;


					// old에만 존재하는 필드명 구하기
					bool bOnlyOld = false;
					bool bOnlyNew = false;

					CString strOldExist = _T("");
					CString strNewExist = _T("");

					// old 기준 비교
					for (int j = 0; j < oldFileCnt; j++) {
						CString oldFieldName = oldFile._vTable[j]->_cTitle;

						for (int k = 0; k < newFileCnt; k++) {
							CString newFieldName = newFile._vTable[k]->_cTitle;

							// 비교해서 둘 다 있다면 
							if (newFieldName.Compare(oldFieldName) == 0) {
								bOnlyOld = false;
								break;
							}
							// 비교해서 필드명이 old 에만 있다면
							else {
								bOnlyOld = true;
							}

						}

						// 필드명이 old에만 있다면 필드명 저장
						if (bOnlyOld) {
							strOldExist = oldFieldName + _T(",") + strOldExist;
						}
						
					}

					// new 기준 비교
					for (int j = 0; j < newFileCnt; j++) {
						CString newFieldName = newFile._vTable[j]->_cTitle;

						for (int k = 0; k < oldFileCnt; k++) {
							CString oldFieldName = oldFile._vTable[k]->_cTitle;

							// 비교해서 둘 다 있다면 
							if (oldFieldName.Compare(newFieldName) == 0) {
								bOnlyNew = false;
								break;
							}
							// 비교해서 필드명이 new 에만 있다면
							else {
								bOnlyNew = true;
							}

						}

						// 필드명이 new에만 있다면 필드명 저장
						if (bOnlyNew) {
							strNewExist = newFieldName + _T(",") + strNewExist;
						}

					}


					// dbf 요약결과창

					// 3번째 컬럼 일치여부
					CString strCompareResult;
					if (strOldExist.Compare(strNewExist) == 0) {
						strCompareResult = _T("YES");
					}
					else {
						strCompareResult = _T("NO");
					}

					// 4,5번째 컬럼 레코드 수
					/*CString strOldRecordCnt;
					CString strNewRecordCnt;

					strOldRecordCnt.Format("%d", oldFile._iRecCount);
					strNewRecordCnt.Format("%d", newFile._iRecCount);*/


					// dbf 요약결과창에 그려주기
					iItem.iSubItem = 0;
					iItem.pszText = newPath.GetBuffer();
					m_lcDbfResult.InsertItem(&iItem);

					iItem.iSubItem = 1;
					iItem.pszText = strOldExist.GetBuffer();
					m_lcDbfResult.SetItem(&iItem);

					iItem.iSubItem = 2;
					iItem.pszText = strNewExist.GetBuffer();
					m_lcDbfResult.SetItem(&iItem);

					iItem.iSubItem = 3;
					iItem.pszText = strCompareResult.GetBuffer();
					m_lcDbfResult.SetItem(&iItem);

					iItem.iSubItem = 4;
					iItem.pszText = strOldRecordCnt.GetBuffer();
					m_lcDbfResult.SetItem(&iItem);

					iItem.iSubItem = 5;
					iItem.pszText = strNewRecordCnt.GetBuffer();
					m_lcDbfResult.SetItem(&iItem);
					
				}
			}

			m_progressBar.SetPos(100);

		}

		if (gv_strFormat2 == 'd') {


		}
	}

	NMHDR nmhdr;
	nmhdr.code = TCN_SELCHANGE;
	nmhdr.idFrom = IDC_TAB1;
	nmhdr.hwndFrom = m_tab.m_hWnd;
	SendMessage(WM_NOTIFY, IDC_TAB1, (LPARAM)&nmhdr);


	// json edit line 값 삭제
	m_ecJson.SetSel(0, -1, TRUE);
	m_ecJson.Clear();

}


void CCompareTool2Dlg::FileSearch(CString file_path, const char oldornew[])
{
	// TODO: 여기에 구현 코드 추가.

	if (file_path.IsEmpty()) {
		return;
	}

	CFileFind finder;
	file_path += "\\*.*";

	BOOL bWorking = finder.FindFile(file_path);

	if (bWorking == 0) {
		return;
	}

	while (bWorking) {

		bWorking = finder.FindNextFile();

		// 상위 폴더 뛰어넘기
		if (finder.IsDots())
			continue;

		// 폴더일 경우에
		if (finder.IsDirectory()) {

			CString str_path = finder.GetFilePath();

			// 폴더 이름 연속해서 저장해 두기
			CString str_temp_folderName = finder.GetFileName();
			if (oldornew == "old") {
				Old.folder_name += str_temp_folderName;
				Old.folder_name += "/";
			}
			else if (oldornew == "new") {
				New.folder_name += str_temp_folderName;
				New.folder_name += "/";
			}
			
			// 파일을 찾을 때까지 재귀
			FileSearch(str_path, oldornew);

		}

		// 파일일 경우에
		else if (!finder.IsDirectory()) {

			// file 수정날짜
			CTime tempTime;
			CString str_temp_date;

			if (finder.GetLastWriteTime(tempTime)) {

				int year = tempTime.GetYear();
				int month = tempTime.GetMonth();
				int day = tempTime.GetDay();
				int hour = tempTime.GetHour();
				int minute = tempTime.GetMinute();
				int second = tempTime.GetSecond();
				
				str_temp_date.Format("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
			}

			// file 사이즈
			int temp_size = 0;
			CString str_temp_size;

			// temp vector - 폴더이름/파일이름, 파일사이즈, 파일수정날짜 를 저장하는 임시 벡터
			vector<CString> temp_3;

			if (temp_size = (int)(finder.GetLength())) {
				str_temp_size.Format(_T("%d"), temp_size);
			}

			// file 이름
			CString str_temp_fileName = finder.GetFileName();
			str_temp_fileName.MakeLower();							// 파일이름 전부 소문자로 만들기

			if (oldornew == "old") {
				CString temp = Old.folder_name + str_temp_fileName;
				temp_3.push_back(temp);
				temp_3.push_back(str_temp_size);
				temp_3.push_back(str_temp_date);

				Old.v.push_back(temp_3);
				// v_old.push_back(temp_3);
			}
			else if (oldornew == "new") {
				CString temp = New.folder_name + str_temp_fileName;
				temp_3.push_back(temp);
				temp_3.push_back(str_temp_size);
				temp_3.push_back(str_temp_date);

				New.v.push_back(temp_3);
			}
		}
	}

	finder.Close();

}


void CCompareTool2Dlg::CCompareDrawListView(vector<vector<CString>> v1, vector<vector<CString>> v2, bool reverse)
{
	// TODO: 여기에 구현 코드 추가.

	// list Control
	int nIndex = 0;
	UpdateData(TRUE);
	LVITEM iItem;
	iItem.mask = LVIF_TEXT;
	iItem.iItem = nIndex;

	// 결과창에 그려넣기 위한 변수
	bool exist = false;

	vector<vector<CString>> temp_save1;

	// old 와 new 에 있는 파일 이름을 비교해서 old에는 있었지만 new 에 없는 파일명&사이즈&날짜 저장

	for (int i = 0; i < static_cast<int>(v1.size()); i++) {

		// v 벡터 안에는 폴더명/파일명, 사이즈, 날짜 순으로 들어있음
		CString tempOld = v1[i][0];			// 폴더명/파일명

		for (int j = 0; j < static_cast<int>(v2.size()); j++) {
			CString tempNew = v2[j][0];		// 폴더명/파일명

			if (tempOld.Compare(tempNew) == 0) {

				// 사이즈와 날짜 비교해야함
				double nTempSize1, nTempSize2;

# ifdef _UNICODE
				nTempSize1 = _wtof(v1[i][1]);
				nTempSize2 = _wtof(v2[j][1]);

# else
				nTempSize1 = atof(v1[i][1]);
				nTempSize2 = atof(v2[j][1]);

# endif

				CString strTempSize1 = ConvertByteTo(nTempSize1);			// v1[i][1] 바이트 변환 표기 후
				CString strTempSize2 = ConvertByteTo(nTempSize2);			// v2[j][1] 바이트 변환 표기 후


				// 기존보다 플마5 이상 차이나면 - 변경 
				double nTempGap = nTempSize1 * 0.05;

				// 비교해서 같은 이름의 파일이 있을 때
				bool existTwoFile;
				bool moreGap;

				/// check box 가 변경에 체크되어있으면
				if ((nTempSize2 > nTempSize1 + nTempGap || nTempSize2 < nTempSize1 - nTempGap)){	/// && ((CButton*)GetDlgItem(IDC_CHECK_CHNG_FILE))->GetCheck()) { // 무조건 체크되어있는 것으로 변경함
					existTwoFile = true;
					moreGap = true;
				}
				else if ((nTempSize2 <= nTempSize1 + nTempGap && nTempSize2 >= nTempSize1 - nTempGap)) {
					existTwoFile = true;
					moreGap = false;
				}

				if (existTwoFile) {
					CString tempTitle, tempStr;
					tempStr.Format(_T("%f"), nTempSize1);

					// 함수 두번 들어오지만 한번만 그리기
					if (!reverse) {
						// list View  변경 결과창 그리기
						iItem.iSubItem = 0;
						iItem.pszText = tempOld.GetBuffer();
						m_lcResult.InsertItem(&iItem);

						iItem.iSubItem = 1;
						iItem.pszText = (strTempSize1).GetBuffer();
						m_lcResult.SetItem(&iItem);

						iItem.iSubItem = 2;
						iItem.pszText = (v1[i][2]).GetBuffer();
						m_lcResult.SetItem(&iItem);

						if (moreGap) {
							iItem.iSubItem = 3;
							iItem.pszText = _T("변경");
							m_lcResult.SetItem(&iItem);
						}
						else {
							iItem.iSubItem = 3;
							iItem.pszText = _T("<=>");
							m_lcResult.SetItem(&iItem);
						}

						iItem.iSubItem = 4;
						iItem.pszText = (v2[j][2]).GetBuffer();
						m_lcResult.SetItem(&iItem);

						iItem.iSubItem = 5;
						iItem.pszText = (strTempSize2).GetBuffer();
						m_lcResult.SetItem(&iItem);

						iItem.iSubItem = 6;
						iItem.pszText = tempNew.GetBuffer();
						m_lcResult.SetItem(&iItem);


						// 증감율 계산 
						CString strTempSizeGap;
						double dTempRate;
						dTempRate = (nTempSize2 - nTempSize1) / nTempSize1 * 100;
						strTempSizeGap.Format(_T("%7.2lf"), dTempRate);
						strTempSizeGap += " %";

						iItem.iSubItem = 7;
						iItem.pszText = (strTempSizeGap).GetBuffer();
						m_lcResult.SetItem(&iItem);

						// 용량 증감바이트 수 -  바이트 변환하여 그리기
						double dTempSize = nTempSize2 - nTempSize1;

						CString strByteGap = ConvertByteTo(dTempSize);

						iItem.iSubItem = 8;
						iItem.pszText = (strByteGap).GetBuffer();
						m_lcResult.SetItem(&iItem);

					}
				}

				exist = true;
				break;
			}
		}

		// v2 사이즈만큼 돌고 난 후에 exist가 없었다면
		if (!exist){
			
			CString tempName = tempOld;
			CString tempDate = v1[i][2];


			// 사이즈와 날짜 비교해야함
			double nTempSize;

# ifdef _UNICODE
			nTempSize = _wtof(v1[i][1]);

# else
			nTempSize = atof(v1[i][1]);

# endif

			CString tempSize = ConvertByteTo(nTempSize);

			// Old가 v1 일때
			if (!reverse && ((CButton*)GetDlgItem(IDC_CHECK_DELETE_FILE))->GetCheck()) {

				// list View
				iItem.iSubItem = 0;
				iItem.pszText = tempName.GetBuffer();
				m_lcResult.InsertItem(&iItem);

				iItem.iSubItem = 1;
				iItem.pszText = tempSize.GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 2;
				iItem.pszText = tempDate.GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 3;
				iItem.pszText = _T("삭제");
				m_lcResult.SetItem(&iItem);
			}
			// New 가 v1일때
			else if (reverse && ((CButton*)GetDlgItem(IDC_CHECK_NEW_FILE))->GetCheck()) {
				iItem.iSubItem = 0;
				iItem.pszText = _T(" ");
				m_lcResult.InsertItem(&iItem);

				iItem.iSubItem = 3;
				iItem.pszText = _T("추가");
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 4;
				iItem.pszText = tempDate.GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 5;
				iItem.pszText = tempSize.GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 6;
				iItem.pszText = tempName.GetBuffer();
				m_lcResult.SetItem(&iItem);

			}

			nIndex++;

		}
		exist = false;
	}


	// 처음 들어왔다면 전부 일치 부분이 지나가게 만듬
	// 비교한 값이 전부 같을 때
	int itemCnt = m_lcResult.GetItemCount();
	if (!itemCnt && reverse) {
		iItem.iSubItem = 0;
		iItem.pszText = _T("전부 일치");
		m_lcResult.InsertItem(&iItem); 

		iItem.iSubItem = 6;
		iItem.pszText = _T("이전 데이터와 현재 데이터는 완전히 같습니다");
		m_lcResult.SetItem(&iItem);
	}

}


void CCompareTool2Dlg::OnClickedButtonExcel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ExportExcel();
	
}


void CCompareTool2Dlg::ExportExcel()
{
	// TODO: 여기에 구현 코드 추가.

	int form_num = 0;
	// { _T("Shp 폴더"), _T("DBF 요약"), _T("DBF 상세"), _T("Mdb"), _T("Txt"), _T("CSV"), _T("Doc"), _T("Xlsx"), _T("Json"), _T("Xml")};

	if (gv_strFormat1 == '1' && gv_strFormat2 == 's') {
		form_num = 1;
	}
	else if (gv_strFormat1 == '1' && gv_strFormat2 == 'd') {
		form_num = 2;
	}
	else if (gv_strFormat1 == '2' && gv_strFormat2 == 'm') {
		form_num = 3;
	}
	else if (gv_strFormat1 == '2' && gv_strFormat2 == 't') {
		form_num = 4;
	}
	else if (gv_strFormat1 == '2' && gv_strFormat2 == 'c') {
		form_num = 5;
	}
	else if (gv_strFormat1 == '3' && gv_strFormat2 == 'd') {
		form_num = 6;
	}
	else if (gv_strFormat1 == '3' && gv_strFormat2 == 'x') {
		form_num = 7;
	}
	else if (gv_strFormat1 == '4' && gv_strFormat2 == 'j') {
		form_num = 8;
	}
	else if (gv_strFormat1 == '4' && gv_strFormat2 == 'x') {
		form_num = 9;
	}


	CApplication app;

	// 엑셀 시작
	if (!app.CreateDispatch(_T("Excel.Application")))
	{
		AfxMessageBox(_T("Couldn't start Excel."));
	}
	else
	{
		//열리는 과정이 보이게
		app.put_Visible(true);
	}

	// workbook 생성
	app.put_SheetsInNewWorkbook(2);

	CWorkbooks wbs;
	CWorkbook wb;

	// COleVariant covTrue((short)TRUE);
	// COleVariant covFalse((short)FALSE);
	COleVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

	// books 연결
	wbs.AttachDispatch(app.get_Workbooks());
	// book 연결
	wb.AttachDispatch(wbs.Add(covOptional));

	// sheets 생성, 연결
	CWorksheets wss;
	wss = wb.get_Sheets();

	// sheet 생성, 연결 (1번 시트)
	CWorksheet ws;
	ws = wss.get_Item(COleVariant((short)1));

	// 2번 시트
	CWorksheet ws2;

	// range 생성, 연결
	CRange range;

	CString file_name = _T("");

	if (form_num == 1) {

		
		// sheet 이름 변경
		ws.put_Name(_T("Shp folder"));

		// range 생성, 연결
		// CRange range;
		range.AttachDispatch(ws.get_Cells(), true);

		// range 값 쓰기
		int recordCnt = m_lcResult.GetItemCount();

		for (int i = 0; i < recordCnt; i++) {
			for (int j = 0; j < 9; j++)
			{
				CString strRow;
				//strRow.Format(_T("ROW%d %d"), i, j);
				strRow = m_lcResult.GetItemText(i, j);
				range.put_Item(COleVariant(long(i + 1)), COleVariant(long(j + 1)), COleVariant(strRow));		// 엑셀은 시작 1부터.. 0 아님!
			}
		}
		
		ws2 = wss.get_Item(COleVariant((short)2));		
		ws2.put_Name(_T("Dbf Summary"));

		range.AttachDispatch(ws2.get_Cells(), true);

		recordCnt = m_lcDbfResult.GetItemCount();

		for (int i = 0; i < recordCnt; i++) {
			for (int j = 0; j < 6; j++)
			{
				CString strRow;
				//strRow.Format(_T("ROW%d %d"), i, j);
				strRow = m_lcDbfResult.GetItemText(i, j);
				range.put_Item(COleVariant(long(i + 1)), COleVariant(long(j + 1)), COleVariant(strRow));		// 엑셀은 시작 1부터.. 0 아님!
			}
		}

		file_name = _T(".\\Shp_Dbf_result.xlsx");
	}

	else if (form_num == 2) {

		CHeaderCtrl* pHeader = (CHeaderCtrl*)m_lcDbfResult2.GetDlgItem(0);
		int nColumnCount = pHeader->GetItemCount();

		// sheet 이름 변경
		ws.put_Name(_T("Dbf Detail"));

		// range 생성, 연결
		range.AttachDispatch(ws.get_Cells(), true);

		// range 값 쓰기
		int recordCnt = m_lcDbfResult2.GetItemCount();

		for (int i = 0; i < recordCnt; i++) {
			for (int j = 0; j < nColumnCount; j++)
			{
				CString strRow;
				//strRow.Format(_T("ROW%d %d"), i, j);
				strRow = m_lcDbfResult2.GetItemText(i, j);
				range.put_Item(COleVariant(long(i + 1)), COleVariant(long(j + 1)), COleVariant(strRow));		// 엑셀은 시작 1부터.. 0 아님!
			}
		}

		file_name = _T(".\\Dbf_Detail_result.xlsx");
	}

	else if (form_num == 4) {

		CHeaderCtrl* pHeader = (CHeaderCtrl*)m_lcTxtResult.GetDlgItem(0);
		int nColumnCount = pHeader->GetItemCount();

		// sheet 이름 변경
		ws.put_Name(_T("Txt Result"));

		// range 생성, 연결
		range.AttachDispatch(ws.get_Cells(), true);

		// range 값 쓰기
		int recordCnt = m_lcTxtResult.GetItemCount();

		for (int i = 0; i < recordCnt; i++) {
			for (int j = 0; j < nColumnCount; j++)
			{
				CString strRow;
				strRow = m_lcTxtResult.GetItemText(i, j);
				range.put_Item(COleVariant(long(i + 1)), COleVariant(long(j + 1)), COleVariant(strRow));		// 엑셀은 시작 1부터.. 0 아님!
			}
		}

		file_name = _T(".\\Txt_result.xlsx");
	}

	else if (form_num == 5) {

		CHeaderCtrl* pHeader = (CHeaderCtrl*)m_lcCsvResult.GetDlgItem(0);
		int nColumnCount = pHeader->GetItemCount();

		// sheet 이름 변경
		ws.put_Name(_T("CSV Result"));

		// range 생성, 연결
		range.AttachDispatch(ws.get_Cells(), true);

		// range 값 쓰기
		int recordCnt = m_lcCsvResult.GetItemCount();

		for (int i = 0; i < recordCnt; i++) {
			for (int j = 0; j < nColumnCount; j++)
			{
				CString strRow;
				strRow = m_lcCsvResult.GetItemText(i, j);
				range.put_Item(COleVariant(long(i + 1)), COleVariant(long(j + 1)), COleVariant(strRow));		// 엑셀은 시작 1부터.. 0 아님!
			}
		}

		file_name = _T(".\\CSV_result.xlsx");
	}
	
	else if (form_num == 7) {

	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_lcCsvResult.GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();

	// sheet 이름 변경
	ws.put_Name(_T("XLSX Result"));

	// range 생성, 연결
	range.AttachDispatch(ws.get_Cells(), true);

	// range 값 쓰기
	int recordCnt = m_lcCsvResult.GetItemCount();

	for (int i = 0; i < recordCnt; i++) {
		for (int j = 0; j < nColumnCount; j++)
		{
			CString strRow;
			strRow = m_lcCsvResult.GetItemText(i, j);
			range.put_Item(COleVariant(long(i + 1)), COleVariant(long(j + 1)), COleVariant(strRow));		// 엑셀은 시작 1부터.. 0 아님!
		}
	}

		file_name = _T(".\\XLSX_result.xlsx");
	}



	// -- 저장하기
	COleVariant vtOpt((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	wb.SaveAs(COleVariant(file_name), vtOpt, vtOpt, vtOpt, vtOpt, vtOpt, 0, vtOpt, vtOpt, vtOpt, vtOpt, vtOpt);		// 문서 폴더에 저장이 됨 ./파일이름

	// 연결 끊기
	range.ReleaseDispatch();
	ws.ReleaseDispatch();
	wss.ReleaseDispatch();
	wb.ReleaseDispatch();
	wbs.ReleaseDispatch();
	app.ReleaseDispatch();
}

// 사이즈 int 값에서 단위 붙여서 변환하기
CString CCompareTool2Dlg::ConvertByteTo(double dTempSize)
{
	// TODO: 여기에 구현 코드 추가.
	CString strTempSize;

	if (dTempSize >= 0 && dTempSize < pow(2, 10)) {
		strTempSize.Format(_T("%9.0lf Byte"), dTempSize);
	}
	else if (dTempSize >= pow(2, 10) && dTempSize < pow(2, 20)) {
		strTempSize.Format(_T("%9.2lf KB"), dTempSize / pow(2, 10));
	}
	else if (dTempSize >= pow(2, 20) && dTempSize < pow(2, 30)) {
		strTempSize.Format(_T("%9.2lf MB"), dTempSize / pow(2, 20));
	}
	else if (dTempSize >= pow(2, 30) && dTempSize < pow(2, 40)) {
		strTempSize.Format(_T("%9.2lf GB"), dTempSize / pow(2, 30));
	}
	else if (dTempSize >= pow(2, 40) && dTempSize < pow(2, 50)) {
		strTempSize.Format(_T("%9.2lf TB"), dTempSize / pow(2, 40));
	}
	// Minus
	else if (dTempSize >= -pow(2, 10) && dTempSize < 0) {
		strTempSize.Format(_T("%9.0lf Byte"), dTempSize);
	}
	else if (dTempSize >= -pow(2, 20) && dTempSize < -pow(2, 10)) {
		strTempSize.Format(_T("%9.2lf KB"), dTempSize / pow(2, 10));
	}
	else if (dTempSize < -pow(2, 20) && dTempSize >= -pow(2, 30)) {
		strTempSize.Format(_T("%9.2lf MB"), dTempSize / pow(2, 20));
	}
	else if (dTempSize < -pow(2, 30) && dTempSize >= -pow(2, 40)) {
		strTempSize.Format(_T("%9.2lf GB"), dTempSize / pow(2, 30));
	}
	else if (dTempSize < -pow(2, 40) && dTempSize >= -pow(2, 50)) {
		strTempSize.Format(_T("%9.2lf TB"), dTempSize / pow(2, 40));
	}

	return strTempSize;
}


void CCompareTool2Dlg::GetDbfFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex)
{
	// TODO: 여기에 구현 코드 추가.
	
	// 값이 있다면 지우기
	m_lcDbfResult2.DeleteAllItems();

	// int cnt = m_lcDbfResult2.GetItemCount();

	// 전체 헤더컬럼 지우기
	while (m_lcDbfResult2.DeleteColumn(0)) {}

	m_progressBar.SetRange(0, 100);
	m_progressBar.SetPos(0);

	DBF oldFile(oldPath);
	oldFile.readFile_(oldPath);

	DBF newFile(newPath);
	newFile.readFile_(newPath);


	// 결과창 컬럼 그리기
	const int fieldCnt = oldFile._vTable.size();

	LV_COLUMN iColDbf;
	iColDbf.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iColDbf.fmt = LVCFMT_LEFT;
	m_lcDbfResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);


	if (selectedFieldIndex.size() == 0)
		return;


	int idx;

	// DBF 파일 헤더 읽어서 결과창 헤더 컬럼 그리기

	for (idx = 0; idx < fieldCnt * 2 + 1; idx++) {
		if (idx < fieldCnt) {
			char *szText = oldFile._vTable[idx]->_cTitle;
			iColDbf.pszText = (LPTSTR)szText;
			iColDbf.iSubItem = idx;
			iColDbf.cx = 80;
			m_lcDbfResult2.InsertColumn(idx, &iColDbf);
		}
		else if (idx == fieldCnt) {
			iColDbf.pszText = (LPTSTR)"일치여부";
			iColDbf.iSubItem = idx + 1;
			iColDbf.cx = 55;
			m_lcDbfResult2.InsertColumn(idx + 1, &iColDbf);
		}
		else {

			char *szText = oldFile._vTable[(idx - 1) % fieldCnt]->_cTitle;
			iColDbf.pszText = (LPTSTR)szText;
			iColDbf.iSubItem = idx;
			iColDbf.cx = 80;
			m_lcDbfResult2.InsertColumn(idx, &iColDbf);
		}
	}



	// 필드값 확인 후 체크박스에서 체크된 갯수	 -  함수 가변인자로 가져온 것 
	const unsigned int selectedFieldCnt = selectedFieldIndex.size();

	LVITEM iItem;
	iItem.mask = LVIF_TEXT;
	iItem.iItem = 0;

	// 프로그래스 바를 위한 것 - 전부 읽어들이면 100을 만들기 위함
	double file_count = (double)100 / oldFile._iRecCount;
	double start_count = 0;


	// old - new 비교 시작
	// vector<vector<CString>> forDrawing;  // dbf_result_save

	// old 에 있는 레코드 수만큼 하나씩 확인
	for (int i = 0; i < oldFile._iRecCount; i++) {

		// 프로그래스 바를 위한 변수
		start_count += file_count;
		m_progressBar.SetPos(start_count);


		// new 에 있는 레코드 만큼 확인
		for (int j = 0; j < newFile._iRecCount; j++) {

			int same_cnt = 0;

			for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {
				
				CString strTempold = "";
				strTempold.Format("%s", oldFile._vTable[selectedFieldIndex[sel]]->_vField[i]);

				CString strTempnew = "";
				strTempnew.Format("%s", newFile._vTable[selectedFieldIndex[sel]]->_vField[j]);


				if (strTempold.Compare(strTempnew) == 0) {
					oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] = true;
					newFile._vTable[selectedFieldIndex[sel]]->compareExist[j] = true;

					same_cnt++;
				}
			}

			// 모두 true 라면
			if (same_cnt == selectedFieldCnt) {
				// 완전 일치
				vector<CString> tempDrawing;
				for (int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back(oldFile._vTable[d]->_vField[i]);
				}

				tempDrawing.push_back(_T("일치"));

				for (int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back(newFile._vTable[d]->_vField[j]);
				}

				dbf_result_save.push_back(tempDrawing);

				break;
			}
			else if (same_cnt == 0) {
				// 일치하는 부분 없음

			}
			else {
				// 부분 일치 - 변경
				vector<CString> tempDrawing;
				for (int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back(oldFile._vTable[d]->_vField[i]);
				}

				tempDrawing.push_back(_T("변경"));

				for (int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back(newFile._vTable[d]->_vField[j]);
				}
				dbf_result_save.push_back(tempDrawing);
			}
		}


		// 프로그래스 바 (응답없음) 해결 방법
		MSG msg;
		while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}




	for (int i = 0; i < oldFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
				diff_cnt++;
			}
		}

		// false 가 전부라면 old file 에서는 삭제 데이터 / new file에서는 신규 데이터
		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (int d = 0; d < oldFile._iFieldCount; d++) {
				tempDrawing.push_back(oldFile._vTable[d]->_vField[i]);
			}

			tempDrawing.push_back(_T("삭제된데이터"));

			for (int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back(_T(" "));
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}


	// new 데이터 그리기
	for (int i = 0; i < newFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (newFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
				diff_cnt++;
			}
		}

		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (int d = 0; d < oldFile._iFieldCount; d++) {

				tempDrawing.push_back(_T(" "));
				
			}

			tempDrawing.push_back(_T("새로운데이터"));

			for (int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back(newFile._vTable[d]->_vField[i]);
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}

	int drawingSize = dbf_result_save.size();			// 행 row
	int drawingFieldSize = dbf_result_save[0].size();	// 열 column

	for (int i=0; i<drawingSize; i++){

		for (int j = 0; j < drawingFieldSize; j++) {

			if (j == 0) {

				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = 0;
				iItem.pszText = tempL;
				m_lcDbfResult2.InsertItem(&iItem);
			}
			else {
				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = j;
				iItem.pszText = tempL;
				m_lcDbfResult2.SetItem(&iItem);

			}
		}
	}




	//// old 삭제 데이터 그리기
	//for (int i = 0; i < oldFile._iRecCount; i++) {

	//	vector<CString> temp_result;
	//	temp_result.clear();

	//	int diff_cnt = 0;
	//	for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

	//		if (oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
	//			diff_cnt++;
	//		}
	//	}

	//	// false 가 전부라면 old file 에서는 삭제 데이터 / new file에서는 신규 데이터
	//	if (diff_cnt == selectedFieldCnt) {

	//		for (int k = 0; k < oldFile._iFieldCount; k++) {

	//			if (k == 0) {
	//				iItem.iSubItem = 0;
	//				iItem.pszText = oldFile._vTable[k]->_vField[i];//(LPTSTR)((oldFile._vTable[i][k]).c_str());
	//				m_lcDbfResult2.InsertItem(&iItem);

	//				temp_result.push_back(oldFile._vTable[k]->_vField[i]);
	//			}
	//			else if (k == oldFile._iFieldCount - 1) {
	//				iItem.iSubItem = k;
	//				iItem.pszText = oldFile._vTable[k]->_vField[i];
	//				m_lcDbfResult2.SetItem(&iItem);

	//				temp_result.push_back(oldFile._vTable[k]->_vField[i]);


	//				iItem.iSubItem = k + 1;
	//				iItem.pszText = TEXT("삭제된데이터");
	//				m_lcDbfResult2.SetItem(&iItem);

	//				temp_result.push_back("삭제된데이터");
	//			}
	//			else {
	//				iItem.iSubItem = k;
	//				iItem.pszText = oldFile._vTable[k]->_vField[i];
	//				m_lcDbfResult2.SetItem(&iItem);

	//				temp_result.push_back(oldFile._vTable[k]->_vField[i]);
	//			}
	//		}

	//		// 전역에 저장
	//		dbf_result_save.push_back(temp_result);
	//	}

	//}

	//// new 데이터 그리기
	//for (int i = 0; i < newFile._iRecCount; i++) {

	//	vector<CString> temp_result;
	//	temp_result.clear();

	//	int diff_cnt = 0;
	//	for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

	//		if (newFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
	//			diff_cnt++;
	//		}
	//	}


	//	for (int k = 0; k < newFile._iFieldCount; k++) {

	//		if (k == 0) {
	//			iItem.iSubItem = 0;
	//			iItem.pszText = newFile._vTable[k]->_vField[i];
	//			m_lcDbfResult2.InsertItem(&iItem);

	//			temp_result.push_back(newFile._vTable[k]->_vField[i]);
	//		}
	//		else if (k == newFile._iFieldCount - 1) {
	//			iItem.iSubItem = k;
	//			iItem.pszText = newFile._vTable[k]->_vField[i];
	//			m_lcDbfResult2.SetItem(&iItem);

	//			temp_result.push_back(newFile._vTable[k]->_vField[i]);


	//			// 체크박스에서 체크한 인덱스 모두 false 라면
	//			// new 전부 false 는 새로운 데이터
	//			if (diff_cnt == selectedFieldCnt) {
	//				iItem.iSubItem = k + 1;
	//				iItem.pszText = TEXT("새로운데이터");
	//				m_lcDbfResult2.SetItem(&iItem);

	//				temp_result.push_back("새로운데이터");

	//			}
	//			else if (diff_cnt == 0) {
	//				// 전체 true는 변경 없는 데이터
	//				iItem.iSubItem = k + 1;
	//				iItem.pszText = TEXT("일치");
	//				m_lcDbfResult2.SetItem(&iItem);

	//				temp_result.push_back("일치");

	//			}
	//			else {
	//				// 부분 일치
	//				iItem.iSubItem = k + 1;
	//				iItem.pszText = TEXT("변경");
	//				m_lcDbfResult2.SetItem(&iItem);

	//				temp_result.push_back("변경");
	//			}

	//		}
	//		else {
	//			iItem.iSubItem = k;
	//			iItem.pszText = newFile._vTable[k]->_vField[i];
	//			m_lcDbfResult2.SetItem(&iItem);

	//			temp_result.push_back(newFile._vTable[k]->_vField[i]);
	//		}
	//	}

	//	// 전역에 저장
	//	dbf_result_save.push_back(temp_result);

	//}

	m_progressBar.SetPos(100);
	return;
}


// 필드값 읽기
void CCompareTool2Dlg::OnClickedButtonFieldCheck()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// listview 체크박스 전부 지우기
	m_check_list.ResetContent();


	CString oldPath, newPath;
	m_lbOld.GetText(0, oldPath);	// IDC_LIST_OLD의 첫번째 값을 path에 복사 후 읽어서 저장
	m_lbNew.GetText(0, newPath);	// IDC_LIST_NEW의 첫번째 값을 path에 복사 후 읽어서 저장


	if (gv_strFormat1 == '1') {

		if (gv_strFormat2 == 'd') {

			// DBF 확인용 -- old 와 new 의 비교
			

			// dbf header 정보 읽어서 저장해두기
			DBF oldFile(oldPath);
			DBF newFile(newPath);
			oldFile.loadFile(oldPath);
			newFile.loadFile(newPath);


			int oldFileCnt = oldFile._iFieldCount;
			int newFileCnt = newFile._iFieldCount;

			// 필드의 갯수가 다를때는 무조건 변경된 필드가 존재한다는 것이기 때문에 함수를 벗어난다
			if (oldFileCnt != newFileCnt) {
				MessageBox(_T("변경된 필드가 존재합니다\n확인해주세요"), _T("변경된 필드가 존재"), MB_ICONWARNING);
				return;
			}

			// 필드명의 순서에 변화가 없는지 확인 
			for (int i = 0; i < oldFileCnt; i++) {
				CString oldTableTitle(oldFile._vTable[i]->_cTitle);
				CString newTableTitle(newFile._vTable[i]->_cTitle);
				
				// 두 필드명을 compare 하여 다르다면 return
				if (oldTableTitle.Compare(newTableTitle) != 0) {
					MessageBox(_T(oldTableTitle + "\t" + newTableTitle + "\n변경된 필드가 하나이상 존재합니다\n확인해주세요"), _T("변경된 필드가 존재"), MB_ICONWARNING);
					return;
				}
			}

			// 필드값 check box 넣기
			for (int i = 0; i < oldFileCnt; i++) {
				m_check_list.InsertString(i, oldFile._vTable[i]->_cTitle);
			}
		}
	}
	else if (gv_strFormat1 == '2') {
		if (gv_strFormat2 == 'c') {

			// csv 확인용 -- old 와 new 의 비교

			// csv 전체 정보 읽어서 저장해두기
			CSV oldFile(oldPath);
			CSV newFile(newPath);
			oldFile.readFile(oldPath);
			newFile.readFile(newPath);


			int oldFileCnt = oldFile._iFieldCount;
			int newFileCnt = newFile._iFieldCount;

			// 필드의 갯수가 다를때는 무조건 변경된 필드가 존재한다는 것이기 때문에 함수를 벗어난다
			if (oldFileCnt != newFileCnt) {
				MessageBox(_T("변경된 필드가 존재합니다\n확인해주세요"), _T("변경된 필드가 존재"), MB_ICONWARNING);
				return;
			}

			// 필드명의 순서에 변화가 없는지 확인 
			for (int i = 0; i < oldFileCnt; i++) {
				CString oldTableTitle((oldFile._vTable[i]->_cTitle).c_str());
				CString newTableTitle((newFile._vTable[i]->_cTitle).c_str());

				// 두 필드명을 compare 하여 다르다면 return
				if (oldTableTitle.Compare(newTableTitle) != 0) {
					MessageBox(_T(oldTableTitle + "\t" + newTableTitle + "\n변경된 필드가 하나이상 존재합니다\n확인해주세요"), _T("변경된 필드가 존재"), MB_ICONWARNING);
					return;
				}
			}

			// 필드값 check box 넣기
			for (int i = 0; i < oldFileCnt; i++) {
				m_check_list.InsertString(i, (oldFile._vTable[i]->_cTitle).c_str());
			}
		}
		else if (gv_strFormat2 == 't') {
			

			TXT headerold(oldPath);
			headerold.readFile();
			int old_cnt = headerold._iFieldCount;

			TXT headernew(newPath);
			headernew.readFile();
			int new_cnt = headernew._iFieldCount;

			if (old_cnt != new_cnt) {
				MessageBox(_T("두 파일의 필드의 갯수가 다릅니다.\n확인해주세요"), _T("필드 갯수가 상이함"), MB_ICONWARNING);
				return;
			}
			
			// 필드값 check box 넣기
			for (int i = 0; i < old_cnt; i++) {
				string temp_str = to_string(i+1) + " 번째";
				m_check_list.InsertString(i, temp_str.c_str());
			}
		}
	}
	else if (gv_strFormat1 == '3') {
		if (gv_strFormat2 == 'x') {
			

			XLSX oldFile(oldPath);
			XLSX newFile(newPath);

			oldFile.readHead();
			newFile.readHead();

			int oldFileCnt = oldFile._iFieldCount;
			int newFileCnt = newFile._iFieldCount;

			if (oldFileCnt != newFileCnt) {
				MessageBox(_T("변경된 필드가 존재합니다\n확인해주세요"), _T("변경된 필드가 존재"), MB_ICONWARNING);
				return;
			}
			
			// 필드명의 순서에 변화가 없는지 확인 
			for (int i = 0; i < oldFileCnt; i++) {
				CString oldTableTitle(oldFile._vTable[i]->_cTitle);
				CString newTableTitle(newFile._vTable[i]->_cTitle);

				// 두 필드명을 compare 하여 다르다면 return
				if (oldTableTitle.Compare(newTableTitle) != 0) {
					MessageBox(_T(oldTableTitle + "\t" + newTableTitle + "\n변경된 필드가 하나이상 존재합니다\n확인해주세요"), _T("변경된 필드가 존재"), MB_ICONWARNING);
					return;
				}
			}

			// 필드값 check box 넣기
			for (int i = 0; i < oldFileCnt; i++) {
				m_check_list.InsertString(i, (oldFile._vTable[i]->_cTitle));
			}
		}

	}

	// 비교하기 버튼 활성화
	GetDlgItem(IDC_BUTTON_COMPARE)->EnableWindow(TRUE);
	
}

// TAB 눌렀을 때 포맷에 맞는 결과창 보여주기
void CCompareTool2Dlg::OnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSelect = m_tab.GetCurSel();

	switch (nSelect) {
	case 0:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(TRUE);			// 0
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);		// 1
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);	// 2
		
		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);		// 4
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);		// 5, 7

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);			// 8
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);			// 8
		break;
	case 1:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(TRUE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);
		
		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);
		break;
	case 2:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(TRUE);

		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	case 3:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);
		
		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);
		
		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	case 4:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);

		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(TRUE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	case 5:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);

		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(TRUE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	case 6:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);

		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	case 7:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);
		
		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(TRUE);		// CSV / XLSX 같이 결과창 공유

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	case 8:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);
		
		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(TRUE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(TRUE);

		break;
	case 9:
		GetDlgItem(IDC_LIST_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_DBF_RESULT2)->ShowWindow(FALSE);
		
		GetDlgItem(IDC_LIST_TXT_RESULT)->ShowWindow(FALSE);
		GetDlgItem(IDC_LIST_CSV_RESULT)->ShowWindow(FALSE);

		GetDlgItem(IDC_TREE_OLD)->ShowWindow(FALSE);
		GetDlgItem(IDC_TREE_NEW)->ShowWindow(FALSE);

		break;
	}

	*pResult = 0;
}

// SHP 폴더 비교 후 결과창 자료를 전역변수에 저장하기
void CCompareTool2Dlg::ShpResultSaveToGV()
{
	// TODO: 여기에 구현 코드 추가.
	
	shpResults.clear();

	int nRow = m_lcResult.GetItemCount();

	CString text;
	for (int i = 0; i < nRow; i++) {
		SHPRESULT shpResult;
		shpResult.oldFileName = m_lcResult.GetItemText(i, 0);
		shpResult.oldFileSize = m_lcResult.GetItemText(i, 1);
		shpResult.oldFileDate = m_lcResult.GetItemText(i, 2);
		shpResult.compare = m_lcResult.GetItemText(i, 3);
		shpResult.newFileDate = m_lcResult.GetItemText(i, 4);
		shpResult.newFileSize = m_lcResult.GetItemText(i, 5);
		shpResult.newFileName = m_lcResult.GetItemText(i, 6);
		shpResult.rate = m_lcResult.GetItemText(i, 7);
		shpResult.gap = m_lcResult.GetItemText(i, 8);

		shpResults.push_back(shpResult);
	}
}


void CCompareTool2Dlg::OnClickedCheckBox(UINT value)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.


	// shp 선택되어있을 때
	if (gv_strFormat1 == '1' && gv_strFormat2 == 's') {

		int nIndex = 0;
		UpdateData(TRUE);
		LVITEM iItem;
		iItem.mask = LVIF_TEXT;
		iItem.iItem = nIndex;
		
		// shp 폴더 전부 지우기
		m_lcResult.DeleteAllItems();

		for (int i = shpResults.size() - 1; i >= 0; i--) {

			CString judge(shpResults[i].compare);
			bool add_file = false;
			bool delete_file = false;
			bool change_file = false;
			bool nothing = false;

			if (m_cbNewFile.GetCheck() == 1 && judge.Compare("추가") == 0) {
				add_file = true;
			}

			if (m_cbDeleteFile.GetCheck() == 1 && judge.Compare("삭제") == 0) {
				delete_file = true;
			}

			if (m_cbChngFile.GetCheck() == 1 && judge.Compare("변경") == 0) {
				change_file = true;
			}

			// 필터가 아무것도 없다면 <=> 추가삭제변경이 없는 데이터만 보여주기
			if (m_cbNewFile.GetCheck() == 0 && m_cbDeleteFile.GetCheck() == 0 && m_cbChngFile.GetCheck() == 0 && judge.Compare("<=>") == 0) {
				nothing = true;
			}

			if (add_file || delete_file || change_file || nothing) {

				iItem.iSubItem = 0;
				iItem.pszText = (shpResults[i].oldFileName).GetBuffer();
				m_lcResult.InsertItem(&iItem);

				iItem.iSubItem = 1;
				iItem.pszText = (shpResults[i].oldFileSize).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 2;
				iItem.pszText = (shpResults[i].oldFileDate).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 3;
				iItem.pszText = (shpResults[i].compare).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 4;
				iItem.pszText = (shpResults[i].newFileDate).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 5;
				iItem.pszText = (shpResults[i].newFileSize).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 6;
				iItem.pszText = (shpResults[i].newFileName).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 7;
				iItem.pszText = (shpResults[i].rate).GetBuffer();
				m_lcResult.SetItem(&iItem);

				iItem.iSubItem = 8;
				iItem.pszText = (shpResults[i].gap).GetBuffer();
				m_lcResult.SetItem(&iItem);
			}
		}
	}
	else if ((gv_strFormat1 == '1' && gv_strFormat2 == 'd') || (gv_strFormat1 == '2' && gv_strFormat2 == 'c') || (gv_strFormat1 == '2' && gv_strFormat2 == 't') || (gv_strFormat1 == '3' && gv_strFormat2 == 'x')){

		CString str_new_data;
		CString str_delete_data;
		CString str_change_data;
		CString str_same_data;
			
		if (gv_strFormat1 == '1' && gv_strFormat2 == 'd') {
			// dbf 폴더 전부 지우기
			m_lcDbfResult2.DeleteAllItems();

			str_new_data.Format(_T("새로운데이터"));
			str_delete_data.Format(_T("삭제된데이터"));
			str_change_data.Format(_T("변경"));
			str_same_data.Format(_T("일치"));

		}
		// 결과창 csv / xlsx 함께씀
		else if ((gv_strFormat1 == '2' && gv_strFormat2 == 'c') || (gv_strFormat1 == '3' && gv_strFormat2 == 'x')) {
			// csv 결과창 전부 지우기
			m_lcCsvResult.DeleteAllItems();

			str_new_data.Format(_T("새로운데이터"));
			str_delete_data.Format(_T("삭제된데이터"));
			str_change_data.Format(_T("변경"));
			str_same_data.Format(_T("일치"));
		}
		else if (gv_strFormat1 == '2' && gv_strFormat2 == 't') {
			// txt 결과창 전부 지우기
			m_lcTxtResult.DeleteAllItems();

			str_new_data.Format(_T("추가"));
			str_delete_data.Format(_T("삭제"));
			str_change_data.Format(_T("변경"));
			str_same_data.Format(_T("일치"));
		}



		int nIndex = 0;
		UpdateData(TRUE);
		LVITEM iItem;
		iItem.mask = LVIF_TEXT;
		iItem.iItem = nIndex;

		// 전역변수 똑같음(dbf, csv, txt dbf_result_save 여기에 저장되어 있음)
		int result_size = dbf_result_save.size();

		for (int i = 0; i < result_size; i++) {

			vector<CString> temp_results;
			temp_results.clear();
			temp_results.assign((dbf_result_save[i]).begin(), (dbf_result_save[i]).end()); // row 전체 복사

			/// 마지막 값의 새로운 데이터, 일치하지않음을 판별하기 위한 변수
			// 정 가운데 값으로 일치 여부 판별
			int temp_result_size = (int)(temp_results.size()/2);
			CString judge = temp_results[temp_result_size];

			

			bool add_file = false;
			bool delete_file = false;
			bool change_file = false;
			bool nothing = false;

			// 신규 버튼 눌려져있을 때
			if (m_cbNewFile.GetCheck() == 1 && judge.Compare(str_new_data) == 0) {
				add_file = true;
			}
			// 변경 버튼 눌려져있을 때
			if (m_cbChngFile.GetCheck() == 1 && judge.Compare(str_change_data) == 0) {
				change_file = true;
			}
			// 삭제 버튼 눌려져있을 때
			if (m_cbDeleteFile.GetCheck() == 1 && judge.Compare(str_delete_data) == 0) {
				delete_file = true;
			}
			// 필터가 하나도 없을 때
			if (m_cbDeleteFile.GetCheck() == 0 && m_cbNewFile.GetCheck() == 0 && m_cbChngFile.GetCheck() == 0 && judge.Compare(str_same_data) == 0) {
				nothing = true;
			}


			int temp_result_2 = temp_results.size();


			if (add_file || change_file || delete_file || nothing) {

				for (int j = 0; j < temp_result_2; j++) {

					if (j == 0) {
						iItem.iSubItem = 0;
						iItem.pszText = (temp_results[j]).GetBuffer();

						if (gv_strFormat1 == '1' && gv_strFormat2 == 'd') {
							m_lcDbfResult2.InsertItem(&iItem);
						}
						else if ((gv_strFormat1 == '2' && gv_strFormat2 == 'c') || (gv_strFormat1 == '3' && gv_strFormat2 == 'x')) {
							m_lcCsvResult.InsertItem(&iItem);
						}
						else if (gv_strFormat1 == '2' && gv_strFormat2 == 't') {
							m_lcTxtResult.InsertItem(&iItem);
						}


					}
					else {
						iItem.iSubItem = j;
						iItem.pszText = (temp_results[j]).GetBuffer();

						if (gv_strFormat1 == '1' && gv_strFormat2 == 'd') {
							m_lcDbfResult2.SetItem(&iItem);
						}
						else if ((gv_strFormat1 == '2' && gv_strFormat2 == 'c') || (gv_strFormat1 == '3' && gv_strFormat2 == 'x')) {
							m_lcCsvResult.SetItem(&iItem);
						}
						else if (gv_strFormat1 == '2' && gv_strFormat2 == 't') {
							m_lcTxtResult.SetItem(&iItem);
						}
					}

				}
			}
		}
	}
	
}

// 색 입히기
void CCompareTool2Dlg::OnCustomdrawListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = (LRESULT)CDRF_NOTIFYITEMDRAW;
		return; // 여기서 함수를 빠져 나가야 *pResult 값이 유지된다.
	}

	if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {

		// 한 줄 (row) 가 그려질 때. 여기서만 설정하면 한 줄이 모두 동일하게 설정이 된다.
		if (m_lcResult.GetItemData(pNMCD->dwItemSpec) == 0 ) {//dwItemSpec 이 현재 그려지는 row index
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0xffffff;
			pDraw->clrTextBk = 0x0;
			//*pResult = (LRESULT)CDRF_NEWFONT;//여기서 나가면 sub-item 이 변경되지 않는다.
			*pResult = (LRESULT)CDRF_NOTIFYSUBITEMDRAW;//sub-item 을 변경하기 위해서. 
			return;//여기서 중단해야 *pResult 값이 유지된다.
		}
		else { // When all matrices are already made. 
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = RGB(255, 255, 196);
			*pResult = (LRESULT)CDRF_NEWFONT;
			return;
		}
	}
	else if (pNMCD->dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT)) {
		// sub-item 이 그려지는 순간. 위에서 *pResult 에 CDRF_NOTIFYSUBITEMDRAW 를 해서 여기로 올 수 있었던 것이다.
		// 색상 참고 사이트 : https://www.htmlcsscolor.com

		NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
		CString text = m_lcResult.GetItemText(pNMCD->dwItemSpec, pDraw->iSubItem);
		
		if (text == _T("추가")) {
			pDraw->clrText = RGB(0, 0, 139);//0xC3CFDA;
			// pDraw->clrTextBk = 0xf0fff0;
		}
		else if (text == _T("삭제")) {
			pDraw->clrText = RGB(139, 0, 0); //0xDAC3CF; // 0xff;
		}
		else if (text == _T("변경")) {
			pDraw->clrText = RGB(0, 100, 0);//0x17B07A;
		}
		else {
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = 0xffffff;
		}

		*pResult = (LRESULT)CDRF_NEWFONT; // 이렇게 해야 설정한 대로 그려진다.
		return;
	}
	*pResult = 0;
}

void CCompareTool2Dlg::OnCustomdrawListDbfResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = (LRESULT)CDRF_NOTIFYITEMDRAW;
		return; // 여기서 함수를 빠져 나가야 *pResult 값이 유지된다.
	}

	if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {

		// 한 줄 (row) 가 그려질 때. 여기서만 설정하면 한 줄이 모두 동일하게 설정이 된다.
		if (m_lcDbfResult.GetItemData(pNMCD->dwItemSpec) == 0) {	//dwItemSpec 이 현재 그려지는 row index


			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = RGB(139, 0, 0); //0xffffff;
			pDraw->clrTextBk = 0x0;
			//*pResult = (LRESULT)CDRF_NEWFONT;//여기서 나가면 sub-item 이 변경되지 않는다.
			*pResult = (LRESULT)CDRF_NOTIFYSUBITEMDRAW;//sub-item 을 변경하기 위해서. 
			return;//여기서 중단해야 *pResult 값이 유지된다.
		
		}
		else { // When all matrices are already made. 
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = RGB(255, 255, 196);
			*pResult = (LRESULT)CDRF_NEWFONT;
			return;
		}
		
	}
	else if (pNMCD->dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT)) {
		// sub-item 이 그려지는 순간. 위에서 *pResult 에 CDRF_NOTIFYSUBITEMDRAW 를 해서 여기로 올 수 있었던 것이다.
		// 색상 참고 사이트 : https://www.htmlcsscolor.com

		NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
		CString textDbf = m_lcDbfResult.GetItemText(pNMCD->dwItemSpec, pDraw->iSubItem);

		if (textDbf == _T("NO")) {
			pDraw->clrText = 0xffffff;
			pDraw->clrTextBk = RGB(139, 0, 0);
		}
		else {
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = 0xffffff;
		}

		*pResult = (LRESULT)CDRF_NEWFONT; // 이렇게 해야 설정한 대로 그려진다.
		return;
	}

	*pResult = 0;

}

void CCompareTool2Dlg::OpenMDB()
{
	// TODO: 여기에 구현 코드 추가.
	CDatabase database;
	CString sDsn;
	CString sFile = _T("D:/example/20210522_통합데이터비교TOOL/02_MDB/MDB_OLD/01_UniqueSymbolList - 20Q2.mdb");

	// Build ODBC connection string
	sDsn.Format(_T("ODBC;DRIVER={%s};DSN='';DBQ=%s"), _T("MICROSOFT ACCESS DRIVER (*.mdb)"), sFile);
	try
	{
		// Open the database
		database.Open(NULL, FALSE, FALSE, sDsn);

		// Allocate the recordset
		CRecordset recset(&database);

		// Build the SQL statement
		CString sSqlString = _T("SELECT * FROM  MSysObjects where FLAGS = 0 and type = 1;");	// 현재 권한이 없음

		// Execute the query
		recset.Open(CRecordset::forwardOnly, sSqlString, CRecordset::readOnly);

		// Loop through each record
		while (!recset.IsEOF())
		{
			// Copy each column into a variable
			CString sField1;
			CString sField2;
			//recset.GetFieldValue(_T("Index"), sField1);
			//recset.GetFieldValue(_T("MULTIMEDIA_NAME"), sField2);

			recset.GetFieldValue(1, sField1);
			recset.GetFieldValue(2, sField2);

			MessageBox(_T(sField1 + " " + sField2), _T(sField1), MB_OK);

			// goto next record
			recset.MoveNext();
		}
		// Close the database
		database.Close();

		// AfxMessageBox(_T("열었다 닫았다"));
	}
	catch (CDBException* e)
	{
		// If a database exception occured, show error msg
		AfxMessageBox(_T("Database error: ") + e->m_strError);
	}


}


void CCompareTool2Dlg::GetCsvFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex)
{
	// TODO: 여기에 구현 코드 추가.
	// 값이 있다면 지우기
	m_lcCsvResult.DeleteAllItems();

	// 전체 헤더컬럼 지우기
	while (m_lcCsvResult.DeleteColumn(0)) {}

	m_progressBar.SetRange(0, 100);
	m_progressBar.SetPos(0);


	CSV oldFile(oldPath);
	oldFile.readFile(oldPath);


	CSV newFile(newPath);
	newFile.readFile(newPath);

	// 신규/변경/삭제 등 확인하기 위한 Path 저장 글로벌 변수
	gv_dbfPath = newPath;

	m_progressBar.OffsetPos(10);


	// 결과창 컬럼 그리기
	const int fieldCnt = oldFile._vTable.size();

	LV_COLUMN iColCsv;
	iColCsv.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iColCsv.fmt = LVCFMT_LEFT;
	m_lcCsvResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if (selectedFieldIndex.size() == 0)
		return;


	int idx;

	// 결과창 헤더 컬럼 그리기

	for (idx = 0; idx < fieldCnt * 2 + 1; idx++) {
		if (idx < fieldCnt) {
			const char* szText = (oldFile._vTable[idx]->_cTitle).c_str();
			iColCsv.pszText = (LPTSTR)szText;
			iColCsv.iSubItem = idx;
			iColCsv.cx = 100;
			m_lcCsvResult.InsertColumn(idx, &iColCsv);
		}
		else if (idx == fieldCnt) {
			iColCsv.pszText = (LPTSTR)"일치여부";
			iColCsv.iSubItem = idx + 1;
			iColCsv.cx = 100;
			m_lcCsvResult.InsertColumn(idx + 1, &iColCsv);
		}
		else {
			const char* szText = (oldFile._vTable[(idx - 1) % fieldCnt]->_cTitle).c_str();
			iColCsv.pszText = (LPTSTR)szText;
			iColCsv.iSubItem = idx;
			iColCsv.cx = 100;
			m_lcCsvResult.InsertColumn(idx, &iColCsv);
		}
	}


	// 필드값 확인 후 체크박스에서 체크된 갯수	 -  함수 가변인자로 가져온 것 
	const unsigned int selectedFieldCnt = selectedFieldIndex.size();

	LVITEM iItem;
	iItem.mask = LVIF_TEXT;
	iItem.iItem = 0;

	// 프로그래스 바를 위한 것 - 전부 읽어들이면 100을 만들기 위함
	double file_count = (double)90 / oldFile._iRecCount;
	double start_count = 10;


	// old 에 있는 레코드 수만큼 하나씩 확인
	for (int i = 0; i < oldFile._iRecCount; i++) {

		// 프로그래스 바를 위한 변수
		start_count += file_count;
		m_progressBar.SetPos(start_count);


		// new 에 있는 레코드 만큼 확인
		for (int j = 0; j < newFile._iRecCount; j++) {

			int same_cnt = 0;

			for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

				CString strTempold = (oldFile._vTable[selectedFieldIndex[sel]]->_vField[i]).c_str();
				CString strTempnew = (newFile._vTable[selectedFieldIndex[sel]]->_vField[j]).c_str();
				
				if (strTempold.Compare(strTempnew) == 0) {
					oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] = true;
					newFile._vTable[selectedFieldIndex[sel]]->compareExist[j] = true;

					same_cnt++;
				}
			}

			// 모두 true 라면
			if (same_cnt == selectedFieldCnt) {
				// 완전 일치
				vector<CString> tempDrawing;
				for (int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back((oldFile._vTable[d]->_vField[i]).c_str());
				}
				tempDrawing.push_back(_T("일치"));

				for (int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back((newFile._vTable[d]->_vField[j]).c_str());
				}

				dbf_result_save.push_back(tempDrawing);

				break;
			}
			else if (same_cnt == 0) {
				// 일치하는 부분 없음

			}
			else {
				// 부분 일치 - 변경
				vector<CString> tempDrawing;
				for (int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back((oldFile._vTable[d]->_vField[i]).c_str());
				}

				tempDrawing.push_back(_T("변경"));

				for (int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back((newFile._vTable[d]->_vField[j]).c_str());
				}
				dbf_result_save.push_back(tempDrawing);
			}
		}


		// 프로그래스 바 (응답없음) 해결 방법
		MSG msg;
		while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}


	for (int i = 0; i < oldFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
				diff_cnt++;
			}
		}

		// false 가 전부라면 old file 에서는 삭제 데이터 / new file에서는 신규 데이터
		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (int d = 0; d < oldFile._iFieldCount; d++) {
				tempDrawing.push_back((oldFile._vTable[d]->_vField[i]).c_str());
			}

			tempDrawing.push_back(_T("삭제된데이터"));

			for (int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back(_T(" "));
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}


	// new 데이터 그리기
	for (int i = 0; i < newFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (newFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
				diff_cnt++;
			}
		}

		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (int d = 0; d < oldFile._iFieldCount; d++) {

				tempDrawing.push_back(_T(" "));

			}

			tempDrawing.push_back(_T("새로운데이터"));

			for (int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back((newFile._vTable[d]->_vField[i]).c_str());
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}

	int drawingSize = dbf_result_save.size();			// 행 row
	int drawingFieldSize = dbf_result_save[0].size();	// 열 column

	for (int i = 0; i < drawingSize; i++) {

		for (int j = 0; j < drawingFieldSize; j++) {

			if (j == 0) {

				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = 0;
				iItem.pszText = tempL;
				m_lcCsvResult.InsertItem(&iItem);
			}
			else {
				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = j;
				iItem.pszText = tempL;
				m_lcCsvResult.SetItem(&iItem);

			}
		}
	}

	m_progressBar.SetPos(100);
	return;
}


void CCompareTool2Dlg::GetTxtFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex)
{
	// TODO: 여기에 구현 코드 추가.

	// 값이 있다면 지우기
	m_lcTxtResult.DeleteAllItems();

	// 전체 헤더컬럼 지우기
	while (m_lcTxtResult.DeleteColumn(0)) {}

	m_progressBar.SetRange(0, 100);
	m_progressBar.SetPos(0);

	TXT oldFile(oldPath);
	oldFile.readFile();

	TXT newFile(newPath);
	newFile.readFile();
	
	// 결과창 컬럼 그리기
	const int fieldCnt = oldFile._iFieldCount;

	LV_COLUMN iColTxt;
	iColTxt.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iColTxt.fmt = LVCFMT_LEFT;
	m_lcTxtResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if (selectedFieldIndex.size() == 0)
		return;


	int idx;

	// 결과창 헤더 컬럼 그리기

	
	for (idx = 0; idx < fieldCnt*2 + 1; idx++) {
		if (idx < fieldCnt) {
			string str = "Num" + to_string(idx + 1);
			const char* szText = str.c_str();
			iColTxt.pszText = (LPTSTR)szText;
			iColTxt.iSubItem = idx;
			iColTxt.cx = 80;
			m_lcTxtResult.InsertColumn(idx, &iColTxt);
		}
		else if (idx == fieldCnt) {
			iColTxt.pszText = (LPTSTR)"일치여부";
			iColTxt.iSubItem = idx + 1;
			iColTxt.cx = 55;
			m_lcTxtResult.InsertColumn(idx + 1, &iColTxt);
		}
		else {
			string str = "Num" + to_string(((idx-1)%fieldCnt)+1);
			const char* szText = str.c_str();
			iColTxt.pszText = (LPTSTR)szText;
			iColTxt.iSubItem = idx;
			iColTxt.cx = 80;
			m_lcTxtResult.InsertColumn(idx, &iColTxt);
		}
	}

	// 필드값 확인 후 체크박스에서 체크된 갯수	 -  함수 가변인자로 가져온 것 
	const unsigned int selectedFieldCnt = selectedFieldIndex.size();

	LVITEM iItem;
	iItem.mask = LVIF_TEXT;
	iItem.iItem = 0;

	// 프로그래스 바를 위한 것 - 전부 읽어들이면 100을 만들기 위함
	double file_count = (double)100 / oldFile._iRecCount;
	double start_count = 0;


	// old - new 비교 시작
	// printf("Start to compare\n[old] %d [new] %d\n", oldFile._iRecCount, newFile._iRecCount);

	// old 에 있는 레코드 수만큼 하나씩 확인
	for (int i = 0; i < oldFile._iRecCount; i++) {

		// 프로그래스 바를 위한 변수
		start_count += file_count;

		// int start_cnt_int = round(start_count);
		m_progressBar.SetPos(start_count);


		// new 에 있는 레코드 만큼 확인
		for (int j = 0; j < newFile._iRecCount; j++) {


			int same_cnt = 0;

			for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

				// 두개 값이 같을 때
				string strTempold = oldFile._vTable[i][selectedFieldIndex[sel]];
				string strTempnew = newFile._vTable[j][selectedFieldIndex[sel]];

				//printf("[OLD] %s  [NEW] %s\n", strTempold, strTempnew);

				if (strTempold.compare(strTempnew) == 0) {
					oldFile.compareExist[i][selectedFieldIndex[sel]] = true;
					newFile.compareExist[j][selectedFieldIndex[sel]] = true;

					same_cnt++;
				}
			}

			// 모두 true 라면
			if (same_cnt == selectedFieldCnt) {
				// 완전 일치
				vector<CString> tempDrawing;
				for (int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back((oldFile._vTable[i][d]).c_str());
				}

				tempDrawing.push_back(_T("일치"));

				for (int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back((newFile._vTable[j][d]).c_str());
				}

				dbf_result_save.push_back(tempDrawing);


				break;
			}
			else if (same_cnt == 0) {
				// 일치하는 부분 없음

			}
			else {
				// 부분 일치 - 변경
				vector<CString> tempDrawing;
				for (int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back((oldFile._vTable[i][d]).c_str());
				}

				tempDrawing.push_back(_T("변경"));

				for (int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back((newFile._vTable[j][d]).c_str());
				}
				dbf_result_save.push_back(tempDrawing);
			}
		}


		// 프로그래스 바 (응답없음) 해결 방법
		MSG msg;
		while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	for (int i = 0; i < oldFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (oldFile.compareExist[i][selectedFieldIndex[sel]] == false) {
				diff_cnt++;
			}
		}

		// false 가 전부라면 old file 에서는 삭제 데이터 / new file에서는 신규 데이터
		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (int d = 0; d < oldFile._iFieldCount; d++) {
				tempDrawing.push_back((oldFile._vTable[i][d]).c_str());
			}

			tempDrawing.push_back(_T("삭제"));

			for (int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back(_T(" "));
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}


	// new 데이터 그리기
	for (int i = 0; i < newFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (newFile.compareExist[i][selectedFieldIndex[sel]] == false) {
				diff_cnt++;
			}
		}

		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (int d = 0; d < oldFile._iFieldCount; d++) {

				tempDrawing.push_back(_T(" "));

			}

			tempDrawing.push_back(_T("추가"));

			for (int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back((newFile._vTable[i][d]).c_str());
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}

	int drawingSize = dbf_result_save.size();			// 행 row
	int drawingFieldSize = dbf_result_save[0].size();	// 열 column

	for (int i = 0; i < drawingSize; i++) {

		for (int j = 0; j < drawingFieldSize; j++) {

			if (j == 0) {

				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = 0;
				iItem.pszText = tempL;
				m_lcTxtResult.InsertItem(&iItem);
			}
			else {
				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = j;
				iItem.pszText = tempL;
				m_lcTxtResult.SetItem(&iItem);

			}
		}
	}



	m_progressBar.SetPos(100);
	return;
	
}


void CCompareTool2Dlg::GetJsonFile(CString oldPath, bool bOld)
{
	// TODO: 여기에 구현 코드 추가.
	rapidjson::Document dOld;


	// CString -> const char* 변경
	const char* cOldPath = (char*)(LPCTSTR)oldPath;


	// 파일 읽기
	CFile cOldFile;
	if (cOldFile.Open(cOldPath, CFile::modeRead)) {

		unsigned int cFileSize = (unsigned int)(cOldFile.GetLength());

		LPBYTE lpTmp = new BYTE[cFileSize];
		memset(lpTmp, 0x00, cOldFile.GetLength());

		// json file 을 전체 읽고 lpTmp 에 잠시 저장한다
		unsigned long long nReadSize = cOldFile.Read(lpTmp, cOldFile.GetLength());
	
		if (nReadSize > 0) {
			CString sOldRead((LPSTR)lpTmp, cOldFile.GetLength());
	
			dOld.Parse(sOldRead);
			if (dOld.HasParseError()) {

				CString messageStr;
				messageStr.Format(_T("%s\n\n유효하지 않은 JSON 파일입니다."), cOldPath);

				// 인터넷 창 열어주기
				::ShellExecute(NULL, _T("open"), _T("http://json-validator.com"), NULL, NULL, SW_SHOW);

				MessageBox(_T(messageStr), _T("inside parse - ERROR"));


			}
			else {
				// MessageBox(_T("Read Success"), _T("inside parse"));
								
				TVINSERTSTRUCT tInsert;
				HTREEITEM hNode;
				
				for (rapidjson::Value::ConstMemberIterator itr = dOld.MemberBegin(); itr != dOld.MemberEnd(); ++itr) {

					CString sKey((LPSTR)itr->name.GetString(), strlen((LPSTR)itr->name.GetString()));

					const char* szText = (char*)(LPCTSTR)sKey;

					tInsert.hParent = 0;
					tInsert.hInsertAfter = TVI_LAST;
					tInsert.item.mask = TVIF_TEXT;
					tInsert.item.pszText = (LPTSTR)szText;
					if (bOld) {
						hNode = m_ctrTreeOld.InsertItem(&tInsert);
					}
					else {
						hNode = m_ctrTreeNew.InsertItem(&tInsert);
					}

					

					// 3번째 인자 true : Tree Ctrl - Parent 0 번째, 
					doRapidJson(dOld[itr->name.GetString()], sKey, bOld, hNode);

				}		
			}
		}
	}
}

// JSON key - value 확인해서 Tree Ctrl 에 그리는 함수
void CCompareTool2Dlg::doRapidJson(const rapidjson::Value& oRoot, CString sKey, bool bOld, HTREEITEM hNode)
{
	// TODO: 여기에 구현 코드 추가.
	
	CString sDebugStr;

	switch (oRoot.GetType()) {

		TVINSERTSTRUCT tInsert;
		HTREEITEM hSubNode;

		case kNullType: {

			sDebugStr.Format(_T("[%s] : null"), (LPCTSTR)sKey);
			// MessageBox(sDebugStr, _T("Null Type"));

			const char* szText = (char*)(LPCTSTR)sDebugStr;

			// 첫번째 노드
			
			tInsert.hParent = hNode;
			tInsert.hInsertAfter = TVI_LAST;
			tInsert.item.mask = TVIF_TEXT;
			tInsert.item.pszText = (LPTSTR)szText;
			// hSubNode = m_ctrTreeOld.InsertItem(&tInsert);

			if (bOld) {
				hSubNode = m_ctrTreeOld.InsertItem(&tInsert);
			}
			else {
				hSubNode = m_ctrTreeNew.InsertItem(&tInsert);
			}
			

		}break;
	

		case kFalseType:
		case kTrueType: {

			sDebugStr.Format(_T("[%s] : %s"), (LPCTSTR)sKey, oRoot.GetBool() ? _T("True") : _T("False"));
			// MessageBox(sDebugStr, _T("Bool Type"));

			const char* szText = (char*)(LPCTSTR)sDebugStr;

			
			tInsert.hParent = hNode;
			tInsert.hInsertAfter = TVI_LAST;
			tInsert.item.mask = TVIF_TEXT;
			tInsert.item.pszText = (LPTSTR)szText;
			
			if (bOld) {
				hSubNode = m_ctrTreeOld.InsertItem(&tInsert);
			}
			else {
				hSubNode = m_ctrTreeNew.InsertItem(&tInsert);
			}


		}break;

		case kStringType: {

			CString sValue;
			sValue = oRoot.GetString();
			sDebugStr.Format(_T("[%s] : %s"), (LPCTSTR)sKey, (LPCTSTR)sValue);
			// MessageBox(sDebugStr, _T("String Type"));

			const char* szText = (char*)(LPCTSTR)sDebugStr;

			tInsert.hParent = hNode;
			tInsert.hInsertAfter = TVI_LAST;
			tInsert.item.mask = TVIF_TEXT;
			tInsert.item.pszText = (LPTSTR)szText;
			
			if (bOld) {
				hSubNode = m_ctrTreeOld.InsertItem(&tInsert);
			}
			else {
				hSubNode = m_ctrTreeNew.InsertItem(&tInsert);
			}


		}break;

		case kNumberType: {

			if (oRoot.IsInt()) {
				sDebugStr.Format(_T("[%s] : %d"), (LPCTSTR)sKey, oRoot.GetInt());
				// MessageBox(sDebugStr, _T("Int Type"));
			}
			else if (oRoot.IsUint()) {
				sDebugStr.Format(_T("[%s] : %d"), (LPCTSTR)sKey, oRoot.GetUint());
				// MessageBox(sDebugStr, _T("Uint Type"));
			}
			else if (oRoot.IsDouble()) {
				sDebugStr.Format(_T("[%s] : %f"), (LPCTSTR)sKey, oRoot.GetDouble());
				// MessageBox(sDebugStr, _T("Double Type"));
			}

			const char* szText = (char*)(LPCTSTR)sDebugStr;

			// 첫번째 노드
			
			tInsert.hParent = hNode;
			tInsert.hInsertAfter = TVI_FIRST;
			tInsert.item.mask = TVIF_TEXT;
			tInsert.item.pszText = (LPTSTR)szText;
			
			if (bOld) {
				hSubNode = m_ctrTreeOld.InsertItem(&tInsert);
			}
			else {
				hSubNode = m_ctrTreeNew.InsertItem(&tInsert);
			}
			

		}break;

		case kObjectType: {

			sDebugStr.Format(_T("%s"), (LPCTSTR)sKey);
			
			const char* szText = (char*)(LPCTSTR)sDebugStr;

			tInsert.hParent = hNode;
			tInsert.hInsertAfter = TVI_LAST;
			tInsert.item.mask = TVIF_TEXT;
			tInsert.item.pszText = (LPTSTR)szText;

			if (bOld) {
				hSubNode = m_ctrTreeOld.InsertItem(&tInsert);
			}
			else {
				hSubNode = m_ctrTreeNew.InsertItem(&tInsert);
			}
			
			
			CString sPath;
			
			for (rapidjson::Value::ConstMemberIterator itr = oRoot.MemberBegin(); itr != oRoot.MemberEnd(); ++itr) {
				CString sName;
				sName = itr->name.GetString();
				
				doRapidJson(oRoot[itr->name.GetString()], sName, bOld, hSubNode);

			}
		}break;

		case kArrayType: {
			sDebugStr.Format(_T("%s"), (LPCTSTR)sKey);
			// MessageBox(sDebugStr, _T("Array Type"));

			const char* szText = (char*)(LPCTSTR)sDebugStr;
			
			tInsert.hParent = hNode;
			tInsert.hInsertAfter = TVI_LAST;
			tInsert.item.mask = TVIF_TEXT;
			tInsert.item.pszText = (LPTSTR)szText;
			
			if (bOld) {
				hSubNode = m_ctrTreeOld.InsertItem(&tInsert);
			}
			else {
				hSubNode = m_ctrTreeNew.InsertItem(&tInsert);
			}
			 

			unsigned int nArrCnt = oRoot.Size();
			CString sPath;
			for (unsigned int index = 0; index < nArrCnt; ++index) {
				// sDebugStr.Format(_T("[%s][%d]"), (LPCTSTR)sKey, index);

				sPath.Format(_T("[%d]"), index + 1);// %s"), index+1, (LPCTSTR)sKey);

				doRapidJson(oRoot[index], sPath, bOld, hSubNode);
			}

		}break;
	}
}

// Tree Ctrl 색 바꾸기
void CCompareTool2Dlg::OnCustomdrawTreeCtrlOld(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTVCUSTOMDRAW* pLVCD = reinterpret_cast<NMTVCUSTOMDRAW*>(pNMHDR);
	HTREEITEM hItemTemp;

	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		hItemTemp = (HTREEITEM)pLVCD->nmcd.dwItemSpec;

		CString currentText = m_ctrTreeOld.GetItemText(hItemTemp);


		// if (currentText.Compare(gv_jsonInputValue) == 0) {		// 완전 일치
		if (currentText.Find(gv_jsonInputValue) != -1) {			// 키워드 포함
			pLVCD->clrTextBk = RGB(153, 204, 255);
		}

		*pResult = CDRF_DODEFAULT;
	}
}

void CCompareTool2Dlg::OnCustomdrawTreeCtrlNew(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTVCUSTOMDRAW* pLVCD = reinterpret_cast<NMTVCUSTOMDRAW*>(pNMHDR);
	HTREEITEM hItemTemp;

	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		hItemTemp = (HTREEITEM)pLVCD->nmcd.dwItemSpec;

		CString currentText = m_ctrTreeNew.GetItemText(hItemTemp);


		// if (currentText.Compare(gv_jsonInputValue) == 0) {		// 완전 일치
		if (currentText.Find(gv_jsonInputValue) != -1) {			// 키워드 포함
			pLVCD->clrTextBk = RGB(153, 204, 255);
		}

		*pResult = CDRF_DODEFAULT;
	}
}


void CCompareTool2Dlg::GetXlsxFile(CString oldPath, CString newPath, vector<int> selectedFieldIndex)
{
	// TODO: 여기에 구현 코드 추가.

	XLSX oldFile(oldPath);
	XLSX newFile(newPath);

	oldFile.readFile();
	newFile.readFile();

	
	// 결과창 컬럼 그리기
	const int fieldCnt = oldFile._vTable.size();

	LV_COLUMN iColCsv;
	iColCsv.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iColCsv.fmt = LVCFMT_LEFT;
	m_lcCsvResult.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	if (selectedFieldIndex.size() == 0)
		return;

	int idx;

	// 결과창 헤더 컬럼 그리기

	for (idx = 0; idx < fieldCnt * 2 + 1; idx++) {
		if (idx < fieldCnt) {
			LPTSTR szText;
			szText = (oldFile._vTable[idx]->_cTitle).GetBuffer((oldFile._vTable[idx]->_cTitle).GetLength());

			iColCsv.pszText = szText;
			iColCsv.iSubItem = idx;
			iColCsv.cx = 80;
			m_lcCsvResult.InsertColumn(idx, &iColCsv);
		}
		else if (idx == fieldCnt) {
			iColCsv.pszText = (LPTSTR)"일치여부";
			iColCsv.iSubItem = idx + 1;
			iColCsv.cx = 55;
			m_lcCsvResult.InsertColumn(idx + 1, &iColCsv);
		}
		else {

			LPTSTR szText;
			szText = (oldFile._vTable[(idx - 1) % fieldCnt]->_cTitle).GetBuffer((oldFile._vTable[(idx - 1) % fieldCnt]->_cTitle).GetLength());

			iColCsv.pszText = szText;
			iColCsv.iSubItem = idx;
			iColCsv.cx = 80;
			m_lcCsvResult.InsertColumn(idx, &iColCsv);

		}
	}



	// 필드값 확인 후 체크박스에서 체크된 갯수	 -  함수 가변인자로 가져온 것 
	const unsigned int selectedFieldCnt = selectedFieldIndex.size();

	LVITEM iItem;
	iItem.mask = LVIF_TEXT;
	iItem.iItem = 0;

	// 프로그래스 바를 위한 것 - 전부 읽어들이면 100을 만들기 위함
	double file_count = (double)100 / oldFile._iRecCount;
	double start_count = 0;

	// old 에 있는 레코드 수만큼 하나씩 확인
	for (unsigned int i = 0; i < oldFile._iRecCount; i++) {

		// 프로그래스 바를 위한 변수
		start_count += file_count;
		m_progressBar.SetPos(start_count);


		// new 에 있는 레코드 만큼 확인
		for (unsigned int j = 0; j < newFile._iRecCount; j++) {

			int same_cnt = 0;

			for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

				CString strTempold = oldFile._vTable[selectedFieldIndex[sel]]->_vField[i];
				CString strTempnew = newFile._vTable[selectedFieldIndex[sel]]->_vField[j];

				if (strTempold.Compare(strTempnew) == 0) {
					oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] = true;
					newFile._vTable[selectedFieldIndex[sel]]->compareExist[j] = true;

					same_cnt++;
				}
			}

			// 모두 true 라면
			if (same_cnt == selectedFieldCnt) {
				// 완전 일치
				vector<CString> tempDrawing;
				for (unsigned int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back(oldFile._vTable[d]->_vField[i]);
				}

				tempDrawing.push_back(_T("일치"));

				for (unsigned int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back(newFile._vTable[d]->_vField[j]);
				}

				dbf_result_save.push_back(tempDrawing);

				break;
			}
			else if (same_cnt == 0) {
				// 일치하는 부분 없음

			}
			else {
				// 부분 일치
				vector<CString> tempDrawing;
				for (unsigned int d = 0; d < oldFile._iFieldCount; d++) {
					tempDrawing.push_back(oldFile._vTable[d]->_vField[i]);
				}

				tempDrawing.push_back(_T("변경"));

				for (unsigned int d = 0; d < newFile._iFieldCount; d++) {
					tempDrawing.push_back(newFile._vTable[d]->_vField[j]);
				}
				dbf_result_save.push_back(tempDrawing);
			}
		}


		// 프로그래스 바 (응답없음) 해결 방법
		MSG msg;
		while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	
	for (unsigned int i = 0; i < oldFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (oldFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
				diff_cnt++;
			}
		}

		// false 가 전부라면 old file 에서는 삭제 데이터 / new file에서는 신규 데이터
		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (unsigned int d = 0; d < oldFile._iFieldCount; d++) {
				tempDrawing.push_back(oldFile._vTable[d]->_vField[i]);
			}

			tempDrawing.push_back(_T("삭제된데이터"));

			for (unsigned int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back(_T(" "));
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}


	// new 데이터 그리기
	for (unsigned int i = 0; i < newFile._iRecCount; i++) {

		vector<CString> temp_result;
		temp_result.clear();

		int diff_cnt = 0;
		for (unsigned int sel = 0; sel < selectedFieldCnt; sel++) {

			if (newFile._vTable[selectedFieldIndex[sel]]->compareExist[i] == false) {
				diff_cnt++;
			}
		}

		if (diff_cnt == selectedFieldCnt) {

			vector<CString> tempDrawing;
			for (unsigned int d = 0; d < oldFile._iFieldCount; d++) {

				tempDrawing.push_back(_T(" "));

			}

			tempDrawing.push_back(_T("새로운데이터"));

			for (unsigned int d = 0; d < newFile._iFieldCount; d++) {
				tempDrawing.push_back(newFile._vTable[d]->_vField[i]);
			}
			dbf_result_save.push_back(tempDrawing);

		}

	}

	int drawingSize = dbf_result_save.size();			// 행 row
	int drawingFieldSize = dbf_result_save[0].size();	// 열 column

	for (int i = 0; i < drawingSize; i++) {

		for (int j = 0; j < drawingFieldSize; j++) {

			if (j == 0) {

				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = 0;
				iItem.pszText = tempL;
				m_lcCsvResult.InsertItem(&iItem);
			}
			else {
				LPTSTR tempL = (dbf_result_save[i][j]).GetBuffer((dbf_result_save[i][j]).GetLength());

				iItem.iSubItem = j;
				iItem.pszText = tempL;
				m_lcCsvResult.SetItem(&iItem);

			}
		}
	}


	m_progressBar.SetPos(100);
	return;

	
}


void CCompareTool2Dlg::OnCustomdrawListDbfResult2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 구현 코드 추가.
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = (LRESULT)CDRF_NOTIFYITEMDRAW;
		return; // 여기서 함수를 빠져 나가야 *pResult 값이 유지된다.
	}

	if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {

		// 한 줄 (row) 가 그려질 때. 여기서만 설정하면 한 줄이 모두 동일하게 설정이 된다.
		if (m_lcDbfResult2.GetItemData(pNMCD->dwItemSpec) == 0) {//dwItemSpec 이 현재 그려지는 row index
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0xffffff;
			pDraw->clrTextBk = 0x0;
			//*pResult = (LRESULT)CDRF_NEWFONT;//여기서 나가면 sub-item 이 변경되지 않는다.
			*pResult = (LRESULT)CDRF_NOTIFYSUBITEMDRAW;//sub-item 을 변경하기 위해서. 
			return;//여기서 중단해야 *pResult 값이 유지된다.
		}
		else { // When all matrices are already made. 
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = RGB(255, 255, 196);
			*pResult = (LRESULT)CDRF_NEWFONT;
			return;
		}
	}
	else if (pNMCD->dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT)) {
		// sub-item 이 그려지는 순간. 위에서 *pResult 에 CDRF_NOTIFYSUBITEMDRAW 를 해서 여기로 올 수 있었던 것이다.
		// 색상 참고 사이트 : https://www.htmlcsscolor.com

		NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
		CString text = m_lcDbfResult2.GetItemText(pNMCD->dwItemSpec, pDraw->iSubItem);

		if (text == _T("새로운데이터")) {
			pDraw->clrText = RGB(0, 0, 139);//0xC3CFDA;
			// pDraw->clrTextBk = 0xf0fff0;
		}
		else if (text == _T("삭제된데이터")) {
			pDraw->clrText = RGB(139, 0, 0); //0xDAC3CF; // 0xff;
		}
		else if (text == _T("변경")) {
			pDraw->clrText = RGB(0, 100, 0);//0x17B07A;
		}
		else {
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = 0xffffff;
		}

		*pResult = (LRESULT)CDRF_NEWFONT; // 이렇게 해야 설정한 대로 그려진다.
		return;
	}
	*pResult = 0;
}

void CCompareTool2Dlg::OnCustomdrawListCsvResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 구현 코드 추가.
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = (LRESULT)CDRF_NOTIFYITEMDRAW;
		return; // 여기서 함수를 빠져 나가야 *pResult 값이 유지된다.
	}

	if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {

		// 한 줄 (row) 가 그려질 때. 여기서만 설정하면 한 줄이 모두 동일하게 설정이 된다.
		if (m_lcCsvResult.GetItemData(pNMCD->dwItemSpec) == 0) {//dwItemSpec 이 현재 그려지는 row index
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0xffffff;
			pDraw->clrTextBk = 0x0;
			//*pResult = (LRESULT)CDRF_NEWFONT;//여기서 나가면 sub-item 이 변경되지 않는다.
			*pResult = (LRESULT)CDRF_NOTIFYSUBITEMDRAW;//sub-item 을 변경하기 위해서. 
			return;//여기서 중단해야 *pResult 값이 유지된다.
		}
		else { // When all matrices are already made. 
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = RGB(255, 255, 196);
			*pResult = (LRESULT)CDRF_NEWFONT;
			return;
		}
	}
	else if (pNMCD->dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT)) {
		// sub-item 이 그려지는 순간. 위에서 *pResult 에 CDRF_NOTIFYSUBITEMDRAW 를 해서 여기로 올 수 있었던 것이다.
		// 색상 참고 사이트 : https://www.htmlcsscolor.com

		NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
		CString text = m_lcCsvResult.GetItemText(pNMCD->dwItemSpec, pDraw->iSubItem);

		if (text == _T("새로운데이터")) {
			pDraw->clrText = RGB(0, 0, 139);//0xC3CFDA;
			// pDraw->clrTextBk = 0xf0fff0;
		}
		else if (text == _T("삭제된데이터")) {
			pDraw->clrText = RGB(139, 0, 0); //0xDAC3CF; // 0xff;
		}
		else if (text == _T("변경")) {
			pDraw->clrText = RGB(0, 100, 0);//0x17B07A;
		}
		else {
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = 0xffffff;
		}

		*pResult = (LRESULT)CDRF_NEWFONT; // 이렇게 해야 설정한 대로 그려진다.
		return;
	}
	*pResult = 0;
}

void CCompareTool2Dlg::OnCustomdrawListTxtResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 구현 코드 추가.
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMCD->dwDrawStage == CDDS_PREPAINT) {
		*pResult = (LRESULT)CDRF_NOTIFYITEMDRAW;
		return; // 여기서 함수를 빠져 나가야 *pResult 값이 유지된다.
	}

	if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT) {

		// 한 줄 (row) 가 그려질 때. 여기서만 설정하면 한 줄이 모두 동일하게 설정이 된다.
		if (m_lcTxtResult.GetItemData(pNMCD->dwItemSpec) == 0) {//dwItemSpec 이 현재 그려지는 row index
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0xffffff;
			pDraw->clrTextBk = 0x0;
			//*pResult = (LRESULT)CDRF_NEWFONT;//여기서 나가면 sub-item 이 변경되지 않는다.
			*pResult = (LRESULT)CDRF_NOTIFYSUBITEMDRAW;//sub-item 을 변경하기 위해서. 
			return;//여기서 중단해야 *pResult 값이 유지된다.
		}
		else { // When all matrices are already made. 
			NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = RGB(255, 255, 196);
			*pResult = (LRESULT)CDRF_NEWFONT;
			return;
		}
	}
	else if (pNMCD->dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT)) {
		// sub-item 이 그려지는 순간. 위에서 *pResult 에 CDRF_NOTIFYSUBITEMDRAW 를 해서 여기로 올 수 있었던 것이다.
		// 색상 참고 사이트 : https://www.htmlcsscolor.com

		NMLVCUSTOMDRAW *pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
		CString text = m_lcTxtResult.GetItemText(pNMCD->dwItemSpec, pDraw->iSubItem);

		if (text == _T("추가")) {
			pDraw->clrText = RGB(0, 0, 139);//0xC3CFDA;
			// pDraw->clrTextBk = 0xf0fff0;
		}
		else if (text == _T("삭제")) {
			pDraw->clrText = RGB(139, 0, 0); //0xDAC3CF; // 0xff;
		}
		else if (text == _T("변경")) {
			pDraw->clrText = RGB(0, 100, 0);//0x17B07A;
		}
		else {
			pDraw->clrText = 0x0;
			pDraw->clrTextBk = 0xffffff;
		}

		*pResult = (LRESULT)CDRF_NEWFONT; // 이렇게 해야 설정한 대로 그려진다.
		return;
	}
	*pResult = 0;
}


// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// CompareTool2.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"


// MFC 개발시 프로그램 실행의 정보를 콘솔 창에 로그로 출력하기 위한것 콘솔창 안나오게 하려면 주석처리하면 됌
// #pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")



char gv_strFormat1 = '1';
char gv_strFormat2 = 's';
bool gv_bDropFile = false;

GV Old;
GV New;
std::vector<vector<CString>> dbf_result_save;
CString gv_dbfPath;

// Json input value 색 바꾸기 위함
CString gv_jsonInputValue;

// SHPRESULT shpResult; // 전역으로 말고 지역변수로 쓸 예정. -- class 멤버변수로 만들어서 씀

// std::vector<vector<CString>> v_old;
// std::vector<vector<CString>> v_new;

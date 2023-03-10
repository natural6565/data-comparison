
// DlgProxy.cpp: 구현 파일
//

#include "stdafx.h"
#include "CompareTool2.h"
#include "DlgProxy.h"
#include "CompareTool2Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCompareTool2DlgAutoProxy

IMPLEMENT_DYNCREATE(CCompareTool2DlgAutoProxy, CCmdTarget)

CCompareTool2DlgAutoProxy::CCompareTool2DlgAutoProxy()
{
	EnableAutomation();

	// 자동화 개체가 활성화되어 있는 동안 계속 응용 프로그램을 실행하기 위해
	//	생성자에서 AfxOleLockApp를 호출합니다.
	AfxOleLockApp();

	// 응용 프로그램의 주 창 포인터를 통해 대화 상자에 대한
	//  액세스를 가져옵니다.  프록시의 내부 포인터를 설정하여
	//  대화 상자를 가리키고 대화 상자의 후방 포인터를 이 프록시로
	//  설정합니다.
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CCompareTool2Dlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CCompareTool2Dlg)))
		{
			m_pDialog = reinterpret_cast<CCompareTool2Dlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CCompareTool2DlgAutoProxy::~CCompareTool2DlgAutoProxy()
{
	// 모든 개체가 OLE 자동화로 만들어졌을 때 응용 프로그램을 종료하기 위해
	// 	소멸자가 AfxOleUnlockApp를 호출합니다.
	//  이러한 호출로 주 대화 상자가 삭제될 수 있습니다.
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CCompareTool2DlgAutoProxy::OnFinalRelease()
{
	// 자동화 개체에 대한 마지막 참조가 해제되면
	// OnFinalRelease가 호출됩니다.  기본 클래스에서 자동으로 개체를 삭제합니다.
	// 기본 클래스를 호출하기 전에 개체에 필요한 추가 정리 작업을
	// 추가하세요.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CCompareTool2DlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CCompareTool2DlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// 참고: IID_ICompareTool2에 대한 지원을 추가하여
//  VBA에서 형식 안전 바인딩을 지원합니다.
//  이 IID는 .IDL 파일에 있는 dispinterface의 GUID와 일치해야 합니다.

// {7016d84c-9b4c-43a7-9e01-98513f02b17e}
static const IID IID_ICompareTool2 =
{0x7016d84c,0x9b4c,0x43a7,{0x9e,0x01,0x98,0x51,0x3f,0x02,0xb1,0x7e}};

BEGIN_INTERFACE_MAP(CCompareTool2DlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CCompareTool2DlgAutoProxy, IID_ICompareTool2, Dispatch)
END_INTERFACE_MAP()

// IMPLEMENT_OLECREATE2 매크로가 이 프로젝트의 StdAfx.h에 정의됩니다.
// {b5f38c0c-b559-4b79-af02-5657a53350b3}
IMPLEMENT_OLECREATE2(CCompareTool2DlgAutoProxy, "CompareTool2.Application", 0xb5f38c0c,0xb559,0x4b79,0xaf,0x02,0x56,0x57,0xa5,0x33,0x50,0xb3)


// CCompareTool2DlgAutoProxy 메시지 처리기

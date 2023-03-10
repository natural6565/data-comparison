#pragma once


// CListBoxFileDrop

class CListBoxFileDrop : public CListBox
{
	DECLARE_DYNAMIC(CListBoxFileDrop)

public:
	CListBoxFileDrop();
	virtual ~CListBoxFileDrop();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
	CString ExtractFileExtension(TCHAR *pathName);
};



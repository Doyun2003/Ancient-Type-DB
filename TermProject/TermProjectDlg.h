
// TermProjectDlg.h: 헤더 파일
//

#pragma once
#include <vector>

struct SCharInfo {
	CString m_char;
	int m_type;
	int m_sheet;
	int m_sx;
	int m_sy;
	int m_line;
	int m_order;
	int m_width;
	int m_height;
};

class CTypeDB {
public:
	std::vector<SCharInfo> m_Chars; // 모든 데이터를 담는 배열

	// CSV 파일 읽는 함수
	BOOL ReadCSVFile(CString strPath) {
		CStdioFile file;
		// 한글 포함 파일일 수 있으니 텍스트 모드로 열기
		if (!file.Open(strPath, CFile::modeRead | CFile::typeText))
			return FALSE;

		CString strLine;
		file.ReadString(strLine); // 첫 줄(헤더)은 건너뜀 [cite: 114]

		while (file.ReadString(strLine)) { // 두 번째 줄부터 끝까지
			SCharInfo info;
			CString strToken;

			// 쉼표(,)로 쪼개서 구조체에 넣기
			AfxExtractSubString(info.m_char, strLine, 0, ',');

			AfxExtractSubString(strToken, strLine, 1, ','); info.m_type = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 2, ','); info.m_sheet = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 3, ','); info.m_sx = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 4, ','); info.m_sy = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 5, ','); info.m_line = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 6, ','); info.m_order = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 7, ','); info.m_width = _ttoi(strToken);
			AfxExtractSubString(strToken, strLine, 8, ','); info.m_height = _ttoi(strToken);

			m_Chars.push_back(info); // 배열에 추가
		}
		file.Close();
		return TRUE;
	}
};

// CTermProjectDlg 대화 상자
class CTermProjectDlg : public CDialogEx
{
// 생성입니다.
public:
	CTermProjectDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.



// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TERMPROJECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedButtonCone();

public:
	vtkSmartPointer<vtkRenderWindow> m_vtkWindow;
	vtkSmartPointer<vtkRenderWindowInteractor> m_pInteractor;
	void InitVtkWindow(void* hWnd);
	void ResizeVtkWindow();
	CTypeDB m_TypeDB;
	void ViewSTL(CString strFilePath);
	afx_msg void OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl m_ListCtrl;
};

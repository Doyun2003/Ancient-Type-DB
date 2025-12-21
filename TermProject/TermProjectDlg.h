
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
    int m_nSheet = 3;

    BOOL ReadCSVFile(CString strPath) {
        CWaitCursor wait;

        m_Chars.clear();

        CStdioFile file;
        if (!file.Open(strPath, CFile::modeRead | CFile::typeText))
            return FALSE;

        CString strLine;
        file.ReadString(strLine); 

        while (file.ReadString(strLine)) {
            SCharInfo info;
            CString strToken;

            AfxExtractSubString(info.m_char, strLine, 0, ',');

            AfxExtractSubString(strToken, strLine, 1, ','); info.m_type = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 2, ','); info.m_sheet = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 3, ','); info.m_sx = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 4, ','); info.m_sy = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 5, ','); info.m_line = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 6, ','); info.m_order = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 7, ','); info.m_width = _ttoi(strToken);
            AfxExtractSubString(strToken, strLine, 8, ','); info.m_height = _ttoi(strToken);

            m_Chars.push_back(info);
        }
        file.Close();
        return TRUE;
    }
};

// CTermProjectDlg 대화 상자
class CTermProjectDlg : public CDialogEx
{
public:
    // 생성/기본
    CTermProjectDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TERMPROJECT_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    // MFC 기본 메시지 핸들러
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

public:
    // UI 이벤트(버튼/콤보/리스트/스핀 등)
    afx_msg void OnClickedButtonCone();
    afx_msg void OnClickedButtonButtonOpen();
    afx_msg void OnSelchangeEditBookname();
    afx_msg void OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMCustomdrawListChars(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnDeltaposSpinHistory(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelchangeListHistory();

protected:
    void LayoutSearchButton();  


    // Static 클릭(필요하면 유지)
    afx_msg void OnStnClickedStaticFrame();
    afx_msg void OnStnClickedStaticSheets();
    afx_msg void OnStnClickedStaticKindCount();
    afx_msg void OnStnClickedStaticTypeImage();
    afx_msg void OnStnClickedStaticCharUnicode();

public:
    // VTK/3D
    void InitVtkWindow(void* hWnd);
    void ResizeVtkWindow();
    void ViewSTL(CString strFilePath);

    vtkSmartPointer<vtkRenderWindow>           m_vtkWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> m_pInteractor;
    vtkSmartPointer<vtkRenderWindow>           m_vtkRenderWindow;
    vtkSmartPointer<vtkRenderer>              m_vtkRenderer;
    vtkSmartPointer<vtkRenderWindowInteractor> m_vtkInteractor;

protected:
    // 데이터/로딩/갱신 로직
    void LoadBookData(CString strBookName);
    void LoadSheetImage(int nSheet);
    void UpdateCharInfo(int nIndex);

    CTypeDB   m_TypeDB;
    int       m_nCurrentSheet;
    int       m_nSelectIndex = -1;
    CString   m_strRootPath;

    // 이미지(책/선택글자/활자)
    CImage m_imgBookPage;
    CImage m_imgTypeChar;
    CImage m_imgSelectedChar;
    CRect  m_rectBookImage;

public:
    // 컨트롤 핸들(DDX)
    CListCtrl  m_ListCtrl;
    CComboBox  m_editBookName;
    CImageList m_ImgList;

protected:
    // 리소스/기타
    HICON m_hIcon;
    std::vector<int> m_History;
    int m_HistoryPos = -1;
    bool m_bHistoryJump = false;
    CSpinButtonCtrl m_spinHistory;
    CListBox m_listHistory;

    void AddToHistory(int idx);
    void RefreshHistoryList();
    void SyncHistorySelection();
    CString MakeHistoryText(int dataIndex) const;

private:
    // 폰트 관련
    CFont m_fontUI;
    CFont m_fontBold;

    void CreateUiFonts();
    void ApplyFontsToAllControls();
    void ApplyFontToChildren(CWnd* pParent, CFont* pFont);
    void ApplyListHeaderFont(CListCtrl& list, CFont* pFont);
    void FixControlHeightsAfterFontChange();
public:
    afx_msg void OnStnClickedStaticMaxIdx();
    afx_msg void OnStnClickedStaticSheetType();
};


// TermProjectDlg.cpp

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "TermProject.h"
#include "TermProjectDlg.h"
#include "afxdialogex.h"

#include <set>
#include <vector>

#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ============================================================================
// CAboutDlg
// ============================================================================
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
void CAboutDlg::DoDataExchange(CDataExchange* pDX) { CDialogEx::DoDataExchange(pDX); }
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// ============================================================================
// CTermProjectDlg
// ============================================================================
CTermProjectDlg::CTermProjectDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_TERMPROJECT_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_nCurrentSheet = 1;
}

void CTermProjectDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHARS, m_ListCtrl);
    DDX_Control(pDX, IDC_EDIT_BOOKNAME, m_editBookName);
    DDX_Control(pDX, IDC_SPIN_HISTORY, m_spinHistory);
    DDX_Control(pDX, IDC_LIST_HISTORY, m_listHistory);
}

BEGIN_MESSAGE_MAP(CTermProjectDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_LBUTTONDOWN()
    ON_WM_ERASEBKGND()

    ON_BN_CLICKED(IDC_BUTTON_CONE, &CTermProjectDlg::OnClickedButtonCone)
    ON_BN_CLICKED(IDC_BUTTON_BUTTON_OPEN, &CTermProjectDlg::OnClickedButtonButtonOpen)

    ON_CBN_SELCHANGE(IDC_EDIT_BOOKNAME, &CTermProjectDlg::OnSelchangeEditBookname)

    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHEET, &CTermProjectDlg::OnDeltaposSpinSheet)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TYPE, &CTermProjectDlg::OnDeltaposSpinType)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HISTORY, &CTermProjectDlg::OnDeltaposSpinHistory)

    ON_NOTIFY(NM_CLICK, IDC_LIST_CHARS, &CTermProjectDlg::OnNMClickListChars)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_CHARS, &CTermProjectDlg::OnNMCustomdrawListChars)

    ON_LBN_SELCHANGE(IDC_LIST_HISTORY, &CTermProjectDlg::OnSelchangeListHistory)

    ON_STN_CLICKED(IDC_STATIC_FRAME, &CTermProjectDlg::OnStnClickedStaticFrame)
    ON_STN_CLICKED(IDC_STATIC_SHEETS, &CTermProjectDlg::OnStnClickedStaticSheets)
    ON_STN_CLICKED(IDC_STATIC_KIND_COUNT, &CTermProjectDlg::OnStnClickedStaticKindCount)
    ON_STN_CLICKED(IDC_STATIC_TYPE_IMAGE, &CTermProjectDlg::OnStnClickedStaticTypeImage)
    ON_STN_CLICKED(IDC_STATIC_CHAR_UNICODE, &CTermProjectDlg::OnStnClickedStaticCharUnicode)
    ON_STN_CLICKED(IDC_STATIC_MAX_IDX, &CTermProjectDlg::OnStnClickedStaticMaxIdx)
    ON_STN_CLICKED(IDC_STATIC_SHEET_TYPE, &CTermProjectDlg::OnStnClickedStaticSheetType)
END_MESSAGE_MAP()


// ============================================================================
// 1. 초기화 및 UI 배치
//    - 폰트/컨트롤 높이 보정
//    - 리스트 컬럼 구성
//    - 스핀 범위 설정
//    - VTK 렌더 윈도우 초기화
// ============================================================================
BOOL CTermProjectDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    FixControlHeightsAfterFontChange();

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        CString strAboutMenu;
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    TCHAR szPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);
    ::PathRemoveFileSpec(szPath);
    m_strRootPath = szPath;

    // 1) VTK 렌더링 기본 세팅
    m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkRenderWindow->AddRenderer(m_vtkRenderer);

    m_vtkRenderer->SetGradientBackground(true);
    m_vtkRenderer->SetBackground(0.96, 0.96, 0.97);
    m_vtkRenderer->SetBackground2(0.90, 0.91, 0.92);
    m_vtkRenderWindow->SetMultiSamples(8);

    m_vtkInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    m_vtkInteractor->SetRenderWindow(m_vtkRenderWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_vtkInteractor->SetInteractorStyle(style);

    if (CWnd* pWnd = GetDlgItem(IDC_STATIC_FRAME))
    {
        m_vtkRenderWindow->SetParentId(pWnd->GetSafeHwnd());
        CRect rect;
        pWnd->GetClientRect(&rect);
        m_vtkRenderWindow->SetSize(rect.Width(), rect.Height());
        m_vtkRenderWindow->SetPosition(0, 0);
    }

    // 2) 구성 글자 리스트(ListCtrl) 컬럼 세팅
    if (m_ListCtrl.GetSafeHwnd())
    {
        while (m_ListCtrl.DeleteColumn(0)) {}

        CRect rectList;
        m_ListCtrl.GetClientRect(&rectList);
        int nWidth = rectList.Width() / 3;

        m_ListCtrl.InsertColumn(0, _T("장"), LVCFMT_CENTER, nWidth);
        m_ListCtrl.InsertColumn(1, _T("행"), LVCFMT_CENTER, nWidth);
        m_ListCtrl.InsertColumn(2, _T("번"), LVCFMT_CENTER, rectList.Width() - nWidth * 2 - 1);

        m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    }

    // 3) 스핀 컨트롤 기본값
    if (CSpinButtonCtrl* pSpinSheet = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SHEET))
    {
        pSpinSheet->SetRange(1, 1000);
        pSpinSheet->SetPos(1);
    }

    if (CSpinButtonCtrl* pSpinType = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TYPE))
    {
        pSpinType->SetRange(0, 100);
        pSpinType->SetPos(0);
    }

    if (m_spinHistory.GetSafeHwnd())
    {
        m_spinHistory.SetRange32(0, 100);
        m_spinHistory.SetPos32(0);
    }

    // 4) 검색(폴더 선택) 아이콘 버튼 세팅 + 위치 맞춤
    HICON hSearchIcon = (HICON)::LoadImage(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_ICON_SEARCH),
        IMAGE_ICON,
        24, 24,
        LR_DEFAULTCOLOR);

    if (CButton* pBtnOpen = (CButton*)GetDlgItem(IDC_BUTTON_BUTTON_OPEN))
    {
        pBtnOpen->ModifyStyle(0, BS_ICON | BS_FLAT);
        pBtnOpen->SetWindowTextW(_T(""));
        if (hSearchIcon) pBtnOpen->SetIcon(hSearchIcon);
    }
    LayoutSearchButton();

    // 5) 폰트 적용
    if (m_fontUI.GetSafeHandle() == nullptr)
    {
        m_fontUI.CreatePointFont(112, _T("맑은 고딕"));
        m_fontBold.CreatePointFont(115, _T("맑은 고딕"));
    }

    SetFont(&m_fontUI, TRUE);
    for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetWindow(GW_HWNDNEXT))
        pChild->SetFont(&m_fontUI, TRUE);

    if (m_ListCtrl.GetSafeHwnd())     m_ListCtrl.SetFont(&m_fontUI, TRUE);
    if (m_editBookName.GetSafeHwnd()) m_editBookName.SetFont(&m_fontUI, TRUE);

    if (m_ListCtrl.GetSafeHwnd())
    {
        if (CHeaderCtrl* pHeader = m_ListCtrl.GetHeaderCtrl())
            pHeader->SetFont(&m_fontBold, TRUE);
    }

    // 6) (선택) 실행 폴더의 기본 typeDB.csv가 있으면 미리 로드
    CString dbPath = m_strRootPath + _T("\\typeDB.csv");
    if (m_TypeDB.ReadCSVFile(dbPath) && !m_TypeDB.m_Chars.empty())
        UpdateCharInfo(0);

    return TRUE;
}

void CTermProjectDlg::LayoutSearchButton()
{
    CWnd* pCombo = GetDlgItem(IDC_EDIT_BOOKNAME);
    CWnd* pBtn = GetDlgItem(IDC_BUTTON_BUTTON_OPEN);
    if (!pCombo || !pBtn) return;

    CRect rcC;
    pCombo->GetWindowRect(&rcC);
    ScreenToClient(&rcC);

    const int btnW = 24;
    const int btnH = 24;

    int x = rcC.right - 1;                         
    int y = rcC.top - (rcC.Height() - btnH) / 2;    
    pBtn->SetWindowPos(nullptr, x, y, btnW, btnH, SWP_NOZORDER | SWP_NOACTIVATE);
}


void CTermProjectDlg::FixControlHeightsAfterFontChange()
{
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&m_fontUI);

    TEXTMETRIC tm{};
    dc.GetTextMetrics(&tm);
    dc.SelectObject(pOldFont);

    const int hText = tm.tmHeight + tm.tmExternalLeading;
    const int hEdit = hText + 10;
    const int hStatic = hText + 6;

    for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetWindow(GW_HWNDNEXT))
    {
        if (!pChild->GetSafeHwnd()) continue;

        TCHAR cls[64]{};
        ::GetClassName(pChild->GetSafeHwnd(), cls, 63);
        CString c = cls;

        CRect r;
        pChild->GetWindowRect(&r);
        ScreenToClient(&r);

        if (c.CompareNoCase(_T("Edit")) == 0)
        {
            if (r.Height() < hEdit)
                pChild->SetWindowPos(nullptr, r.left, r.top, r.Width(), hEdit, SWP_NOZORDER);
            continue;
        }

        if (c.CompareNoCase(_T("Static")) == 0)
        {
            if (r.Height() <= 40 && r.Height() < hStatic)
                pChild->SetWindowPos(nullptr, r.left, r.top, r.Width(), hStatic, SWP_NOZORDER);
            continue;
        }

        if (c.CompareNoCase(_T("ComboBox")) == 0)
        {
            CComboBox* pCombo = (CComboBox*)pChild;
            int selH = hText + 8;
            pCombo->SetItemHeight(-1, selH);
            if (pCombo->GetCount() > 0) pCombo->SetItemHeight(0, selH);

            if (r.Height() < hEdit)
                pCombo->SetWindowPos(nullptr, r.left, r.top, r.Width(), hEdit, SWP_NOZORDER);
            continue;
        }

        if (c.CompareNoCase(_T("msctls_updown32")) == 0)
        {
            if (r.Height() < hEdit)
                pChild->SetWindowPos(nullptr, r.left, r.top, r.Width(), hEdit, SWP_NOZORDER);
            continue;
        }
    }
}


// ============================================================================
// 2. VTK(3D 모형) 처리
//    - STL 로드 후 렌더러 갱신
// ============================================================================
void CTermProjectDlg::ViewSTL(CString strFilePath)
{
    if (m_vtkRenderWindow == nullptr || m_vtkRenderWindow->GetGenericParentId() == nullptr)
        return;
    if (strFilePath.IsEmpty())
        return;

    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(CT2A(strFilePath, CP_UTF8));
    reader->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    actor->GetProperty()->SetInterpolationToPhong();
    actor->GetProperty()->SetColor(0.72, 0.74, 0.77);
    actor->GetProperty()->SetAmbient(0.22);
    actor->GetProperty()->SetDiffuse(0.70);
    actor->GetProperty()->SetSpecular(0.18);
    actor->GetProperty()->SetSpecularPower(18.0);

    if (m_vtkRenderer != nullptr)
    {
        m_vtkRenderer->SetGradientBackground(true);
        m_vtkRenderer->RemoveAllViewProps();
        m_vtkRenderer->AddActor(actor);
        m_vtkRenderer->SetBackground(0.96, 0.96, 0.97);
        m_vtkRenderer->SetBackground2(0.90, 0.91, 0.92);
        m_vtkRenderer->ResetCamera();
        m_vtkRenderWindow->Render();
    }
}

void CTermProjectDlg::InitVtkWindow(void* hWnd)
{
    m_pInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.08, 0.05);

    m_vtkWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_vtkWindow->SetParentId(hWnd);
    m_vtkWindow->SetInteractor(m_pInteractor);
    m_vtkWindow->AddRenderer(renderer);
    m_vtkWindow->Render();

    m_pInteractor->Initialize();
}

void CTermProjectDlg::ResizeVtkWindow()
{
    CWnd* pWnd = GetDlgItem(IDC_STATIC_FRAME);
    if (!pWnd || m_vtkWindow == nullptr) return;

    CRect rc;
    pWnd->GetClientRect(rc);
    m_vtkWindow->SetSize(rc.Width(), rc.Height());
}

void CTermProjectDlg::OnClickedButtonCone()
{
    AfxMessageBox(_T("버튼이 눌렸다."));
    CString path = _T(".\\Data\\월인천강지곡 권상(3)\\04_3d\\110A11A10000_1.stl");
    ViewSTL(path);
}


// ============================================================================
// 3. 데이터 로드(책 폴더/CSV/스캔 이미지) + 통계 표시
// ============================================================================
void CTermProjectDlg::OnClickedButtonButtonOpen()
{
    CWaitCursor wait;

    CFolderPickerDialog picker(NULL, 0, this, 0);
    picker.m_ofn.lpstrTitle = _T(".\\Data");

    if (picker.DoModal() != IDOK)
        return;

    m_strRootPath = picker.GetPathName();
    m_editBookName.ResetContent();

    CFileFind finder;
    CString strWildcard = m_strRootPath + _T("\\*.*");

    BOOL bWorking = finder.FindFile(strWildcard);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();
        if (finder.IsDots()) continue;

        if (finder.IsDirectory())
            m_editBookName.AddString(finder.GetFileName());
    }

    if (m_editBookName.GetCount() <= 0)
    {
        AfxMessageBox(_T("선택한 폴더 내에 책 폴더가 없습니다."));
        return;
    }

    m_editBookName.SetCurSel(0);

    CString strFirstBook;
    m_editBookName.GetLBText(0, strFirstBook);
    LoadBookData(strFirstBook);
}

void CTermProjectDlg::OnSelchangeEditBookname()
{
    int nIndex = m_editBookName.GetCurSel();
    if (nIndex == CB_ERR) return;

    CString strBookName;
    m_editBookName.GetLBText(nIndex, strBookName);

    LoadBookData(strBookName);
    Invalidate();
}

void CTermProjectDlg::LoadBookData(CString strBookName)
{
    CWaitCursor wait;

    CString strCSVPath;
    strCSVPath.Format(_T("%s\\%s\\typeDB.csv"), m_strRootPath, strBookName);

    if (!m_TypeDB.ReadCSVFile(strCSVPath))
    {
        AfxMessageBox(_T("선택한 책 폴더에 typeDB.csv 파일이 없습니다.\n") + strCSVPath);
        return;
    }

    m_ListCtrl.DeleteAllItems();

    std::set<CString> setUniqueChars;
    std::set<CString> setUniqueTypes;

    for (const auto& info : m_TypeDB.m_Chars)
    {
        setUniqueChars.insert(info.m_char);

        CString strTypeKey;
        strTypeKey.Format(_T("%s_%d"), info.m_char, info.m_type);
        setUniqueTypes.insert(strTypeKey);
    }

    CString strText;
    strText.Format(_T("%3d 개"), (int)m_TypeDB.m_Chars.size());
    SetDlgItemText(IDC_STATIC_TOTAL_COUNT, strText);

    strText.Format(_T("%3d 종"), (int)setUniqueChars.size());
    SetDlgItemText(IDC_STATIC_KIND_COUNT, strText);

    strText.Format(_T("%3d 개"), (int)setUniqueTypes.size());
    SetDlgItemText(IDC_STATIC_TYPE_COUNT, strText);

    m_nCurrentSheet = 1;
    LoadSheetImage(m_nCurrentSheet);

    if (!m_TypeDB.m_Chars.empty())
    {
        m_ListCtrl.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        m_ListCtrl.EnsureVisible(0, FALSE);

        UpdateCharInfo(0);

        m_nCurrentSheet = m_TypeDB.m_Chars[0].m_sheet;
        LoadSheetImage(m_nCurrentSheet);

        CString strSTLPath;
        strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"),
            m_strRootPath, strBookName, m_TypeDB.m_Chars[0].m_char, m_TypeDB.m_Chars[0].m_type);

        ViewSTL(strSTLPath);
    }

    Invalidate();
}

void CTermProjectDlg::LoadSheetImage(int nSheet)
{
    CWaitCursor wait;

    int nIndex = m_editBookName.GetCurSel();
    if (nIndex == CB_ERR) return;

    CString strBookName;
    m_editBookName.GetLBText(nIndex, strBookName);

    CString strImgPath;
    strImgPath.Format(_T("%s\\%s\\01_scan\\%03d.jpg"), m_strRootPath, strBookName, nSheet);

    if (!m_imgBookPage.IsNull())
        m_imgBookPage.Destroy();

    if (FAILED(m_imgBookPage.Load(strImgPath)))
        return;

    if (CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW))
    {
        pWnd->Invalidate();
        pWnd->UpdateWindow();
    }

    CString strSheetNum;
    strSheetNum.Format(_T(" / %d장"), m_TypeDB.m_nSheet);
    SetDlgItemText(IDC_STATIC_SHEETS, strSheetNum);

    int nSheetCharCount = 0;
    std::set<CString> setSheetChars;
    std::set<CString> setSheetTypes;

    for (const auto& info : m_TypeDB.m_Chars)
    {
        if (info.m_sheet != nSheet) continue;

        nSheetCharCount++;
        setSheetChars.insert(info.m_char);

        CString strTypeKey;
        strTypeKey.Format(_T("%s_%d"), info.m_char, info.m_type);
        setSheetTypes.insert(strTypeKey);
    }

    CString strText;
    strText.Format(_T("%d 개"), nSheetCharCount);
    SetDlgItemText(IDC_STATIC_SHEET_TOTAL, strText);

    strText.Format(_T("%d   종"), (int)setSheetChars.size());
    SetDlgItemText(IDC_STATIC_SHEET_KIND, strText);

    strText.Format(_T("%d 개"), (int)setSheetTypes.size());
    SetDlgItemText(IDC_STATIC_SHEET_TYPE, strText);

    CString strSheet;
    strSheet.Format(_T("%d"), nSheet);
    if (CWnd* pWndSheet = GetDlgItem(IDC_EDIT_CUR_SHEET))
        pWndSheet->SetWindowTextW(strSheet);

    Invalidate();
}

void CTermProjectDlg::OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    int nNewSheet = m_nCurrentSheet + pNMUpDown->iDelta;
    if (nNewSheet < 1) nNewSheet = 1;
    if (nNewSheet > m_TypeDB.m_nSheet) nNewSheet = m_TypeDB.m_nSheet;

    if (nNewSheet == m_nCurrentSheet)
    {
        *pResult = 0;
        return;
    }

    m_nCurrentSheet = nNewSheet;
    LoadSheetImage(m_nCurrentSheet);

    for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
    {
        if (m_TypeDB.m_Chars[i].m_sheet != m_nCurrentSheet) continue;

        UpdateCharInfo(i);

        CString strBookName;
        if (m_editBookName.GetCurSel() != CB_ERR)
        {
            m_editBookName.GetLBText(m_editBookName.GetCurSel(), strBookName);

            CString strSTLPath;
            strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"),
                m_strRootPath, strBookName, m_TypeDB.m_Chars[i].m_char, m_TypeDB.m_Chars[i].m_type);

            ViewSTL(strSTLPath);
        }
        break;
    }

    *pResult = 0;
}


// ============================================================================
// 4. 선택/갱신 로직(리스트 클릭/화면 표시 동기화)
//    - UpdateCharInfo: 오른쪽 정보, 구성 리스트, 이미지, 3D를 “현재 선택 인덱스” 기준으로 일괄 갱신
//    - OnNMClickListChars: 구성 글자 리스트 클릭 시, 해당 글자의 장으로 이동 후 갱신
// ============================================================================
void CTermProjectDlg::OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int nItem = pItemActivate->iItem;
    if (nItem == -1) { *pResult = 0; return; }

    int idx = (int)m_ListCtrl.GetItemData(nItem);

    //  리스트 항목이 다른 장이면, 현재 장을 먼저 이동(이미지 갱신) 후 선택 처리
    if (idx >= 0 && idx < (int)m_TypeDB.m_Chars.size())
    {
        int selSheet = m_TypeDB.m_Chars[idx].m_sheet;
        if (selSheet != m_nCurrentSheet)
        {
            m_nCurrentSheet = selSheet;
            LoadSheetImage(m_nCurrentSheet);

            if (CSpinButtonCtrl* pSpinSheet = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SHEET))
                pSpinSheet->SetPos(m_nCurrentSheet);
        }
    }

    UpdateCharInfo(idx);
    *pResult = 0;
}

void CTermProjectDlg::UpdateCharInfo(int nIndex)
{
    CWaitCursor wait;

    if (nIndex < 0 || nIndex >= (int)m_TypeDB.m_Chars.size())
        return;

    if (!m_bHistoryJump)
        AddToHistory(nIndex);

    m_nSelectIndex = nIndex;
    SCharInfo& curInfo = m_TypeDB.m_Chars[nIndex];

    CString strBookName;
    if (m_editBookName.GetCurSel() != CB_ERR)
        m_editBookName.GetLBText(m_editBookName.GetCurSel(), strBookName);

    SetDlgItemText(IDC_STATIC_CHAR_UNICODE, curInfo.m_char);

    CString pos;
    pos.Format(_T("%d장 %d행 %d번"), curInfo.m_sheet, curInfo.m_line, curInfo.m_order);
    SetDlgItemText(IDC_STATIC_CHAR_POS, pos);

    int nSameCharCount = 0;
    int nMyRank = 1;
    for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
    {
        if (m_TypeDB.m_Chars[i].m_char == curInfo.m_char)
        {
            nSameCharCount++;
            if (i == nIndex) nMyRank = nSameCharCount;
        }
    }

    CString strRank;
    strRank.Format(_T("%d"), nMyRank);
    SetDlgItemText(IDC_EDIT_CUR_TYPE_IDX, strRank);

    CString strTotal;
    strTotal.Format(_T("/ %d개"), nSameCharCount);
    SetDlgItemText(IDC_STATIC_MAX_IDX, strTotal);

    // 구성 글자 리스트 재구성(같은 글자의 등장 위치들을 장/행/번으로 나열)
    m_ListCtrl.DeleteAllItems();
    m_ListCtrl.SetRedraw(FALSE);

    int listIdx = 0;
    for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
    {
        if (m_TypeDB.m_Chars[i].m_char != curInfo.m_char)
            continue;

        bool bDuplicate = false;
        for (int j = 0; j < listIdx; j++)
        {
            if (m_ListCtrl.GetItemText(j, 0) == CString(std::to_wstring(m_TypeDB.m_Chars[i].m_sheet).c_str()) &&
                m_ListCtrl.GetItemText(j, 1) == CString(std::to_wstring(m_TypeDB.m_Chars[i].m_line).c_str()) &&
                m_ListCtrl.GetItemText(j, 2) == CString(std::to_wstring(m_TypeDB.m_Chars[i].m_order).c_str()))
            {
                bDuplicate = true;
                break;
            }
        }
        if (bDuplicate) continue;

        CString t;
        t.Format(_T("%d"), m_TypeDB.m_Chars[i].m_sheet);
        int item = m_ListCtrl.InsertItem(listIdx, t);

        t.Format(_T("%d"), m_TypeDB.m_Chars[i].m_line);
        m_ListCtrl.SetItemText(item, 1, t);

        t.Format(_T("%d"), m_TypeDB.m_Chars[i].m_order);
        m_ListCtrl.SetItemText(item, 2, t);

        m_ListCtrl.SetItemData(item, i);

        if (i == m_nSelectIndex)
            m_ListCtrl.SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

        listIdx++;
    }

    m_ListCtrl.SetRedraw(TRUE);

    // 선택 글자 이미지(선택 글자/모형 이미지)
    CString path;
    path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"),
        m_strRootPath, strBookName, curInfo.m_char, curInfo.m_type);

    CFileFind finder;
    if (finder.FindFile(path))
    {
        while (finder.FindNextFile())
        {
            if (finder.IsDots()) continue;

            if (!m_imgSelectedChar.IsNull()) m_imgSelectedChar.Destroy();
            if (!m_imgTypeChar.IsNull())     m_imgTypeChar.Destroy();

            m_imgSelectedChar.Load(finder.GetFilePath());
            m_imgTypeChar.Load(finder.GetFilePath());
            break;
        }
    }

    // 선택 글자의 3D 모형(STL)
    CString strSTL;
    strSTL.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"),
        m_strRootPath, strBookName, curInfo.m_char, curInfo.m_type);

    ViewSTL(strSTL);

    Invalidate();
}


// ============================================================================
// 5. 히스토리(이전 문자 목록) 처리
//    - 클릭/스핀 모두 “전체 갱신(UpdateCharInfo)”로 통일
// ============================================================================
CString CTermProjectDlg::MakeHistoryText(int dataIndex) const
{
    if (dataIndex < 0 || dataIndex >= (int)m_TypeDB.m_Chars.size())
        return _T("");

    const SCharInfo& info = m_TypeDB.m_Chars[dataIndex];

    CString s;
    s.Format(_T("%s  |  %d장 %d행 %d번"), info.m_char, info.m_sheet, info.m_line, info.m_order);
    return s;
}

void CTermProjectDlg::RefreshHistoryList()
{
    if (!m_listHistory.GetSafeHwnd()) return;

    m_listHistory.SetRedraw(FALSE);
    m_listHistory.ResetContent();

    for (int i = 0; i < (int)m_History.size(); ++i)
    {
        int dataIdx = m_History[i]; // 0이 최신
        int item = m_listHistory.AddString(MakeHistoryText(dataIdx));
        m_listHistory.SetItemData(item, (DWORD_PTR)dataIdx);
    }

    m_listHistory.SetRedraw(TRUE);
    m_listHistory.Invalidate(FALSE);
}

void CTermProjectDlg::SyncHistorySelection()
{
    if (!m_listHistory.GetSafeHwnd()) return;

    if (m_HistoryPos >= 0 && m_HistoryPos < (int)m_History.size())
        m_listHistory.SetCurSel(m_HistoryPos);
    else
        m_listHistory.SetCurSel(-1);
}

void CTermProjectDlg::AddToHistory(int idx)
{
    if (idx < 0 || idx >= (int)m_TypeDB.m_Chars.size())
        return;

    for (auto it = m_History.begin(); it != m_History.end(); ++it)
    {
        if (*it == idx)
        {
            m_History.erase(it);
            break;
        }
    }

    m_History.insert(m_History.begin(), idx);

    if (m_History.size() > 10)
        m_History.resize(10);

    m_HistoryPos = 0;

    RefreshHistoryList();
    SyncHistorySelection();
}

void CTermProjectDlg::OnSelchangeListHistory()
{
    int sel = m_listHistory.GetCurSel();
    if (sel == LB_ERR) return;
    if (sel < 0 || sel >= (int)m_History.size()) return;

    int dataIdx = (int)m_listHistory.GetItemData(sel);
    m_HistoryPos = sel;

    m_bHistoryJump = true;
    UpdateCharInfo(dataIdx);
    m_bHistoryJump = false;

    SyncHistorySelection();
}

void CTermProjectDlg::OnDeltaposSpinHistory(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN p = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    if (m_History.empty())
    {
        *pResult = 0;
        return;
    }

    if (m_HistoryPos < 0) m_HistoryPos = 0;

    int newPos = m_HistoryPos - p->iDelta;
    if (newPos < 0) newPos = 0;
    if (newPos >= (int)m_History.size()) newPos = (int)m_History.size() - 1;

    if (newPos != m_HistoryPos)
    {
        m_HistoryPos = newPos;
        int idx = m_History[m_HistoryPos];

        SyncHistorySelection();

        m_bHistoryJump = true;
        UpdateCharInfo(idx);
        m_bHistoryJump = false;
    }

    *pResult = 0;
}


// ============================================================================
// 6. 타입 스핀(같은 글자의 다른 모형/타입 이동)
// ============================================================================
void CTermProjectDlg::OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if (m_nSelectIndex < 0) { *pResult = 0; return; }

    CString curChar = m_TypeDB.m_Chars[m_nSelectIndex].m_char;

    std::vector<int> sameCharIndices;
    int currentRankIndex = -1;

    for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
    {
        if (m_TypeDB.m_Chars[i].m_char == curChar)
        {
            sameCharIndices.push_back(i);
            if (i == m_nSelectIndex)
                currentRankIndex = (int)sameCharIndices.size() - 1;
        }
    }

    int nextRankIndex = currentRankIndex + pNMUpDown->iDelta;
    if (nextRankIndex < 0) nextRankIndex = 0;
    if (nextRankIndex >= (int)sameCharIndices.size()) nextRankIndex = (int)sameCharIndices.size() - 1;

    if (nextRankIndex != currentRankIndex)
    {
        m_nSelectIndex = sameCharIndices[nextRankIndex];
        SCharInfo& info = m_TypeDB.m_Chars[m_nSelectIndex];

        CString strBookName;
        m_editBookName.GetLBText(m_editBookName.GetCurSel(), strBookName);

        CString strSTLPath;
        strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"),
            m_strRootPath, strBookName, info.m_char, info.m_type);
        ViewSTL(strSTLPath);

        CString path;
        path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"),
            m_strRootPath, strBookName, info.m_char, info.m_type);

        CFileFind finder;
        if (finder.FindFile(path) && finder.FindNextFile())
        {
            if (!m_imgTypeChar.IsNull()) m_imgTypeChar.Destroy();
            m_imgTypeChar.Load(finder.GetFilePath());
        }

        CString strRank;
        strRank.Format(_T("%d"), nextRankIndex + 1);
        SetDlgItemText(IDC_EDIT_CUR_TYPE_IDX, strRank);

        if (CWnd* pTypeWnd = GetDlgItem(IDC_STATIC_TYPE_IMAGE))
            pTypeWnd->Invalidate(FALSE);

        m_ListCtrl.Invalidate(FALSE);
    }

    *pResult = 0;
}


// ============================================================================
// 7. 리스트 색상(선택 강조) 처리
// ============================================================================
void CTermProjectDlg::OnNMCustomdrawListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
    *pResult = CDRF_DODEFAULT;

    switch (pLVCD->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_ITEMPREPAINT:
    {
        int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
        int nDataIndex = (int)m_ListCtrl.GetItemData(nItem);

        pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;

        if (nDataIndex == m_nSelectIndex)
        {
            pLVCD->clrTextBk = RGB(91, 155, 213);
            pLVCD->clrText = RGB(255, 255, 255);
        }
        else
        {
            pLVCD->clrText = RGB(0, 0, 0);
            pLVCD->clrTextBk = (nItem % 2 == 0) ? RGB(255, 255, 255) : RGB(237, 242, 250);
        }

        *pResult = CDRF_NEWFONT;
    }
    break;
    }
}


// ============================================================================
// 8. 그리기/마우스 입력
//    - OnPaint: 스캔 이미지 + 초록 박스(전체) + 빨간 박스(선택) 표시
//    - OnLButtonDown: 스캔 이미지 클릭 시 해당 글자 선택
// ============================================================================
void CTermProjectDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        return;
    }

    CDialogEx::OnPaint();
    CClientDC dc(this);

    if (!m_imgBookPage.IsNull())
    {
        if (CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW))
        {
            CRect rectView;
            pWnd->GetWindowRect(&rectView);
            ScreenToClient(&rectView);

            int nOldMode = ::SetStretchBltMode(dc.m_hDC, HALFTONE);
            ::SetBrushOrgEx(dc.m_hDC, 0, 0, NULL);

            m_imgBookPage.Draw(dc.m_hDC, rectView);
            ::SetStretchBltMode(dc.m_hDC, nOldMode);

            int nOrgWidth = m_imgBookPage.GetWidth();
            int nOrgHeight = m_imgBookPage.GetHeight();

            double dRatioX = (double)rectView.Width() / nOrgWidth;
            double dRatioY = (double)rectView.Height() / nOrgHeight;

            CPen penGreen(PS_SOLID, 1, RGB(0, 255, 0));
            CPen penRed(PS_SOLID, 2, RGB(255, 0, 0));

            dc.SelectStockObject(NULL_BRUSH);
            CPen* pOldPen = dc.SelectObject(&penGreen);

            for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
            {
                SCharInfo& info = m_TypeDB.m_Chars[i];
                if (info.m_sheet != m_nCurrentSheet) continue;

                int left = rectView.left + (int)(info.m_sx * dRatioX);
                int top = rectView.top + (int)(info.m_sy * dRatioY);
                int right = left + (int)(info.m_width * dRatioX);
                int bottom = top + (int)(info.m_height * dRatioY);

                if (i == m_nSelectIndex)
                {
                    dc.SelectObject(&penRed);
                    dc.Rectangle(left, top, right, bottom);
                    dc.SelectObject(&penGreen);
                }
                else
                {
                    dc.Rectangle(left, top, right, bottom);
                }
            }

            dc.SelectObject(pOldPen);
        }
    }

    if (!m_imgSelectedChar.IsNull())
    {
        if (CWnd* pWndChar = GetDlgItem(IDC_STATIC_SELECTED_CHAR_VIEW))
        {
            CRect rectChar;
            pWndChar->GetWindowRect(&rectChar);
            ScreenToClient(&rectChar);
            m_imgSelectedChar.Draw(dc.m_hDC, rectChar);
        }
    }

    if (!m_imgTypeChar.IsNull())
    {
        if (CWnd* pWndType = GetDlgItem(IDC_STATIC_TYPE_IMAGE))
        {
            CRect rectType;
            pWndType->GetWindowRect(&rectType);
            ScreenToClient(&rectType);
            m_imgTypeChar.Draw(dc.m_hDC, rectType);
        }
    }
}

void CTermProjectDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW);
    if (!pWnd) { CDialogEx::OnLButtonDown(nFlags, point); return; }

    CRect rectView;
    pWnd->GetWindowRect(&rectView);
    ScreenToClient(&rectView);

    if (!rectView.PtInRect(point) || m_imgBookPage.IsNull())
    {
        CDialogEx::OnLButtonDown(nFlags, point);
        return;
    }

    int nOrgWidth = m_imgBookPage.GetWidth();
    int nOrgHeight = m_imgBookPage.GetHeight();
    if (nOrgWidth == 0 || nOrgHeight == 0)
    {
        CDialogEx::OnLButtonDown(nFlags, point);
        return;
    }

    double dRatioX = (double)rectView.Width() / nOrgWidth;
    double dRatioY = (double)rectView.Height() / nOrgHeight;

    int nImgX = (int)((point.x - rectView.left) / dRatioX);
    int nImgY = (int)((point.y - rectView.top) / dRatioY);

    for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
    {
        SCharInfo& info = m_TypeDB.m_Chars[i];
        if (info.m_sheet != m_nCurrentSheet) continue;

        if (nImgX >= info.m_sx && nImgX <= (info.m_sx + info.m_width) &&
            nImgY >= info.m_sy && nImgY <= (info.m_sy + info.m_height))
        {
            UpdateCharInfo(i);
            break;
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

BOOL CTermProjectDlg::OnEraseBkgnd(CDC* pDC)
{
    return CDialogEx::OnEraseBkgnd(pDC);
}


// ============================================================================
// 9. 시스템/기타(기본 핸들러)
// ============================================================================
void CTermProjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
        return;
    }
    CDialogEx::OnSysCommand(nID, lParam);
}

HCURSOR CTermProjectDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CTermProjectDlg::OnStnClickedStaticFrame() {}
void CTermProjectDlg::OnStnClickedStaticSheets() {}
void CTermProjectDlg::OnStnClickedStaticKindCount() {}
void CTermProjectDlg::OnStnClickedStaticTypeImage() {}
void CTermProjectDlg::OnStnClickedStaticCharUnicode() {}
void CTermProjectDlg::OnStnClickedStaticMaxIdx() {}


void CTermProjectDlg::OnStnClickedStaticSheetType()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

// TermProjectDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "TermProject.h"
#include "TermProjectDlg.h"
#include "afxdialogex.h"

#include <set>

#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vector>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

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

// CTermProjectDlg 대화 상자

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
    ON_BN_CLICKED(IDC_BUTTON_CONE, &CTermProjectDlg::OnClickedButtonCone)
    ON_BN_CLICKED(IDC_BUTTON_BUTTON_OPEN, &CTermProjectDlg::OnClickedButtonButtonOpen)
    ON_CBN_SELCHANGE(IDC_EDIT_BOOKNAME, &CTermProjectDlg::OnSelchangeEditBookname)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHEET, &CTermProjectDlg::OnDeltaposSpinSheet)
    ON_NOTIFY(NM_CLICK, IDC_LIST_CHARS, &CTermProjectDlg::OnNMClickListChars)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TYPE, &CTermProjectDlg::OnDeltaposSpinType)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_CHARS, &CTermProjectDlg::OnNMCustomdrawListChars)
    ON_STN_CLICKED(IDC_STATIC_FRAME, &CTermProjectDlg::OnStnClickedStaticFrame)
    ON_STN_CLICKED(IDC_STATIC_SHEETS, &CTermProjectDlg::OnStnClickedStaticSheets)
    ON_WM_LBUTTONDOWN()
    ON_WM_ERASEBKGND()
    ON_STN_CLICKED(IDC_STATIC_KIND_COUNT, &CTermProjectDlg::OnStnClickedStaticKindCount)
    ON_STN_CLICKED(IDC_STATIC_TYPE_IMAGE, &CTermProjectDlg::OnStnClickedStaticTypeImage)
    ON_STN_CLICKED(IDC_STATIC_CHAR_UNICODE, &CTermProjectDlg::OnStnClickedStaticCharUnicode)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HISTORY, &CTermProjectDlg::OnDeltaposSpinHistory)
    ON_LBN_SELCHANGE(IDC_LIST_HISTORY, &CTermProjectDlg::OnSelchangeListHistory)

END_MESSAGE_MAP()

// CTermProjectDlg 메시지 처리기

BOOL CTermProjectDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    FixControlHeightsAfterFontChange();
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

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    TCHAR szPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);
    ::PathRemoveFileSpec(szPath);
    m_strRootPath = szPath;

    m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkRenderWindow->AddRenderer(m_vtkRenderer);
    m_vtkRenderer->SetGradientBackground(true);
    m_vtkRenderer->SetBackground(0.96, 0.96, 0.97);  // 위쪽(밝게)
    m_vtkRenderer->SetBackground2(0.90, 0.91, 0.92);  // 아래쪽(살짝 어둡게)
    m_vtkRenderWindow->SetMultiSamples(8);

    m_vtkInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    m_vtkInteractor->SetRenderWindow(m_vtkRenderWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_vtkInteractor->SetInteractorStyle(style);

    CWnd* pWnd = GetDlgItem(IDC_STATIC_FRAME);
    if (pWnd)
    {
        m_vtkRenderWindow->SetParentId(pWnd->GetSafeHwnd());
        CRect rect;
        pWnd->GetClientRect(&rect);
        m_vtkRenderWindow->SetSize(rect.Width(), rect.Height());
        m_vtkRenderWindow->SetPosition(0, 0);
    }

    if (m_ListCtrl.GetSafeHwnd())
    {
        while (m_ListCtrl.DeleteColumn(0))
        {
        }

        CRect rectList;
        m_ListCtrl.GetClientRect(&rectList);
        int nWidth = rectList.Width() / 3;

        m_ListCtrl.InsertColumn(0, _T("장"), LVCFMT_CENTER, nWidth);
        m_ListCtrl.InsertColumn(1, _T("행"), LVCFMT_CENTER, nWidth);
        m_ListCtrl.InsertColumn(2, _T("번"), LVCFMT_CENTER, rectList.Width() - nWidth * 2 - 1);

        m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    }

    CSpinButtonCtrl* pSpinSheet = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SHEET);
    if (pSpinSheet)
    {
        pSpinSheet->SetRange(1, 1000);
        pSpinSheet->SetPos(1);
    }

    CSpinButtonCtrl* pSpinType = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TYPE);
    if (pSpinType)
    {
        pSpinType->SetRange(0, 100);
        pSpinType->SetPos(0);
    }

    HICON hSearchIcon = (HICON)::LoadImage(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_ICON_SEARCH),
        IMAGE_ICON,
        32,
        32,
        LR_DEFAULTCOLOR);

    CButton* pBtnOpen = (CButton*)GetDlgItem(IDC_BUTTON_BUTTON_OPEN);
    if (pBtnOpen)
    {
        pBtnOpen->SetIcon(hSearchIcon);
    }

    m_vtkInteractor->Initialize();

    

    CString dbPath = m_strRootPath + _T("\\typeDB.csv");
    if (m_TypeDB.ReadCSVFile(dbPath))
    {
        if (!m_TypeDB.m_Chars.empty())
        {
            UpdateCharInfo(0);
        }
    }
    if (m_fontUI.GetSafeHandle() == nullptr)
    {
        m_fontUI.CreatePointFont(112, _T("맑은 고딕"));      
        m_fontBold.CreatePointFont(115, _T("맑은 고딕"));    
    }

   
    SetFont(&m_fontUI, TRUE);

    for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetWindow(GW_HWNDNEXT))
    {
        pChild->SetFont(&m_fontUI, TRUE);
    }

    if (m_ListCtrl.GetSafeHwnd()) m_ListCtrl.SetFont(&m_fontUI, TRUE);
    if (m_editBookName.GetSafeHwnd()) m_editBookName.SetFont(&m_fontUI, TRUE);

    if (m_ListCtrl.GetSafeHwnd())
    {
        CHeaderCtrl* pHeader = m_ListCtrl.GetHeaderCtrl();
        if (pHeader && pHeader->GetSafeHwnd())
            pHeader->SetFont(&m_fontBold, TRUE);
    }

    if (m_spinHistory.GetSafeHwnd())
    {
        m_spinHistory.SetRange32(0, 100);
        m_spinHistory.SetPos32(0);
    }

    return TRUE;
}


CString CTermProjectDlg::MakeHistoryText(int dataIndex) const
{
    if (dataIndex < 0 || dataIndex >= (int)m_TypeDB.m_Chars.size())
        return _T("");

    const SCharInfo& info = m_TypeDB.m_Chars[dataIndex];

    CString s;
    s.Format(_T("%s  |  %d장 %d행 %d번"),
        info.m_char, info.m_sheet, info.m_line, info.m_order);
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

        // Edit 높이 보정
        if (c.CompareNoCase(_T("Edit")) == 0)
        {
            if (r.Height() < hEdit)
                pChild->SetWindowPos(nullptr, r.left, r.top, r.Width(), hEdit, SWP_NOZORDER);
            continue;
        }

        // Static 높이 보정
        if (c.CompareNoCase(_T("Static")) == 0)
        {
            // 너무 큰 것(이미지 뷰 같은 것)은 건드리면 망가질 수 있어서
            // "작은 글자 라벨"만 올립니다.
            if (r.Height() <= 40 && r.Height() < hStatic)
                pChild->SetWindowPos(nullptr, r.left, r.top, r.Width(), hStatic, SWP_NOZORDER);
            continue;
        }

        // ComboBox는 높이만 SetWindowPos로 바꾸면 Dropdown 영역까지 꼬일 수 있어서
        // 선택영역 높이(ItemHeight)로 보정하는 방식이 안전합니다.
        if (c.CompareNoCase(_T("ComboBox")) == 0)
        {
            CComboBox* pCombo = (CComboBox*)pChild;
            int selH = hText + 8;
            pCombo->SetItemHeight(-1, selH); // 선택 영역
            // 드롭다운 리스트 항목 높이(있어도 없어도 무방)
            if (pCombo->GetCount() > 0) pCombo->SetItemHeight(0, selH);

            // 창 높이도 최소한은 확보
            if (r.Height() < hEdit)
                pCombo->SetWindowPos(nullptr, r.left, r.top, r.Width(), hEdit, SWP_NOZORDER);
            continue;
        }

        // UpDown(스핀) 컨트롤(클래스명: msctls_updown32)
        if (c.CompareNoCase(_T("msctls_updown32")) == 0)
        {
            if (r.Height() < hEdit)
                pChild->SetWindowPos(nullptr, r.left, r.top, r.Width(), hEdit, SWP_NOZORDER);
            continue;
        }
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
    CRect rc;
    CWnd* pWnd = GetDlgItem(IDC_STATIC_FRAME);

    if (pWnd)
    {
        pWnd->GetClientRect(rc);
        m_vtkWindow->SetSize(rc.Width(), rc.Height());
    }
}

void CTermProjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CTermProjectDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
    }
    else
    {
        CDialogEx::OnPaint();

        CClientDC dc(this);

        if (!m_imgBookPage.IsNull())
        {
            CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW);
            if (pWnd)
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
                    if (info.m_sheet == m_nCurrentSheet)
                    {
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
                }

                dc.SelectObject(pOldPen);
            }
        }

        if (!m_imgSelectedChar.IsNull())
        {
            CWnd* pWndChar = GetDlgItem(IDC_STATIC_SELECTED_CHAR_VIEW);
            if (pWndChar)
            {
                CRect rectChar;
                pWndChar->GetWindowRect(&rectChar);
                ScreenToClient(&rectChar);
                m_imgSelectedChar.Draw(dc.m_hDC, rectChar);
            }
        }

        if (!m_imgTypeChar.IsNull())
        {
            CWnd* pWndType = GetDlgItem(IDC_STATIC_TYPE_IMAGE);
            if (pWndType)
            {
                CRect rectType;
                pWndType->GetWindowRect(&rectType);
                ScreenToClient(&rectType);
                m_imgTypeChar.Draw(dc.m_hDC, rectType);
            }
        }
    }
}

void FixControlHeightsAfterFontChange();

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTermProjectDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CTermProjectDlg::OnClickedButtonCone()
{
    AfxMessageBox(_T("버튼이 눌렸다."));

    CString path = _T(".\\Data\\월인천강지곡 권상(3)\\04_3d\\110A11A10000_1.stl");
    ViewSTL(path);
}

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
        m_vtkRenderer->SetBackground(0.96, 0.96, 0.97);  // 위쪽(밝게)
        m_vtkRenderer->SetBackground2(0.90, 0.91, 0.92);  // 아래쪽(살짝 어둡게)
        m_vtkRenderer->ResetCamera();
        m_vtkRenderWindow->Render();
    }
}

void CTermProjectDlg::OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int nItem = pItemActivate->iItem;
    if (nItem == -1) { *pResult = 0; return; }

    int idx = (int)m_ListCtrl.GetItemData(nItem);
    UpdateCharInfo(idx);

    *pResult = 0;
}


void CTermProjectDlg::OnClickedButtonButtonOpen()
{
    CWaitCursor wait;

    CFolderPickerDialog picker(NULL, 0, this, 0);
    picker.m_ofn.lpstrTitle = _T(".\\Data");

    if (picker.DoModal() == IDOK)
    {
        m_strRootPath = picker.GetPathName();

        m_editBookName.ResetContent();

        CFileFind finder;
        CString strWildcard = m_strRootPath + _T("\\*.*");

        BOOL bWorking = finder.FindFile(strWildcard);
        while (bWorking)
        {
            bWorking = finder.FindNextFile();

            if (finder.IsDots())
                continue;

            if (finder.IsDirectory())
            {
                CString strBookName = finder.GetFileName();
                m_editBookName.AddString(strBookName);
            }
        }

        if (m_editBookName.GetCount() > 0)
        {
            m_editBookName.SetCurSel(0);

            CString strFirstBook;
            m_editBookName.GetLBText(0, strFirstBook);
            LoadBookData(strFirstBook);
        }
        else
        {
            AfxMessageBox(_T("선택한 폴더 내에 책 폴더가 없습니다."));
        }
    }
}

void CTermProjectDlg::LoadBookData(CString strBookName)
{
    CWaitCursor wait;

    CString strCSVPath;
    strCSVPath.Format(_T("%s\\%s\\typeDB.csv"), m_strRootPath, strBookName);

    if (m_TypeDB.ReadCSVFile(strCSVPath))
    {
        m_ListCtrl.DeleteAllItems();
    }
    else
    {
        AfxMessageBox(_T("선택한 책 폴더에 typeDB.csv 파일이 없습니다.\n") + strCSVPath);
    }

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
    strText.Format(_T("%d 개"), (int)m_TypeDB.m_Chars.size());
    SetDlgItemText(IDC_STATIC_TOTAL_COUNT, strText);

    strText.Format(_T("%d 종"), (int)setUniqueChars.size());
    SetDlgItemText(IDC_STATIC_KIND_COUNT, strText);

    strText.Format(_T("%d 개"), (int)setUniqueTypes.size());
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
        strSTLPath.Format(
            _T("%s\\%s\\04_3d\\%s_%d.stl"),
            m_strRootPath,
            strBookName,
            m_TypeDB.m_Chars[0].m_char,
            m_TypeDB.m_Chars[0].m_type);

        ViewSTL(strSTLPath);
    }

    Invalidate();
}

void CTermProjectDlg::OnSelchangeEditBookname()
{
    int nIndex = m_editBookName.GetCurSel();
    if (nIndex == CB_ERR)
        return;

    CString strBookName;
    m_editBookName.GetLBText(nIndex, strBookName);

    LoadBookData(strBookName);

    if (!m_TypeDB.m_Chars.empty())
    {
        m_ListCtrl.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        m_ListCtrl.EnsureVisible(0, FALSE);

        UpdateCharInfo(0);

        m_nCurrentSheet = m_TypeDB.m_Chars[0].m_sheet;
        LoadSheetImage(m_nCurrentSheet);
    }

    Invalidate();
}

void CTermProjectDlg::LoadSheetImage(int nSheet)
{
    CWaitCursor wait;

    CString strBookName;
    int nIndex = m_editBookName.GetCurSel();
    if (nIndex == CB_ERR)
        return;

    m_editBookName.GetLBText(nIndex, strBookName);

    CString strImgPath;
    strImgPath.Format(_T("%s\\%s\\01_scan\\%03d.jpg"), m_strRootPath, strBookName, nSheet);

    if (!m_imgBookPage.IsNull())
        m_imgBookPage.Destroy();

    HRESULT hResult = m_imgBookPage.Load(strImgPath);
    if (FAILED(hResult))
        return;

    CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW);
    if (pWnd)
    {
        pWnd->Invalidate();
        pWnd->UpdateWindow();
    }

    CString strSheetNum;
    strSheetNum.Format(_T(" / %d장") , m_TypeDB.m_nSheet);
    SetDlgItemText(IDC_STATIC_SHEETS, strSheetNum);

    int nSheetCharCount = 0;
    std::set<CString> setSheetChars;
    std::set<CString> setSheetTypes;

    for (const auto& info : m_TypeDB.m_Chars)
    {
        if (info.m_sheet == nSheet)
        {
            nSheetCharCount++;
            setSheetChars.insert(info.m_char);

            CString strTypeKey;
            strTypeKey.Format(_T("%s_%d"), info.m_char, info.m_type);
            setSheetTypes.insert(strTypeKey);
        }
    }

    CString strText;
    strText.Format(_T("%d 개"), nSheetCharCount);
    SetDlgItemText(IDC_STATIC_SHEET_TOTAL, strText);

    strText.Format(_T("%d 종"), (int)setSheetChars.size());
    SetDlgItemText(IDC_STATIC_SHEET_KIND, strText);

    strText.Format(_T("%d 개"), (int)setSheetTypes.size());
    SetDlgItemText(IDC_STATIC_SHEET_TYPE, strText);

    CString strSheet;
    strSheet.Format(_T("%d"), nSheet);

    CWnd* pWndSheet = GetDlgItem(IDC_EDIT_CUR_SHEET);
    if (pWndSheet != NULL && ::IsWindow(pWndSheet->m_hWnd))
    {
        pWndSheet->SetWindowTextW(strSheet);
    }

    Invalidate();
}

void CTermProjectDlg::OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    int nNewSheet = m_nCurrentSheet + pNMUpDown->iDelta;

    if (nNewSheet < 1) nNewSheet = 1;
    if (nNewSheet > m_TypeDB.m_nSheet) nNewSheet = m_TypeDB.m_nSheet;

    if (nNewSheet != m_nCurrentSheet)
    {
        m_nCurrentSheet = nNewSheet;
        LoadSheetImage(m_nCurrentSheet);

        for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
        {
            if (m_TypeDB.m_Chars[i].m_sheet == m_nCurrentSheet)
            {
                UpdateCharInfo(i);

                CString strBookName;
                int nComboIndex = m_editBookName.GetCurSel();
                if (nComboIndex != CB_ERR)
                {
                    m_editBookName.GetLBText(nComboIndex, strBookName);

                    CString strSTLPath;
                    strSTLPath.Format(
                        _T("%s\\%s\\04_3d\\%s_%d.stl"),
                        m_strRootPath,
                        strBookName,
                        m_TypeDB.m_Chars[i].m_char,
                        m_TypeDB.m_Chars[i].m_type);

                    ViewSTL(strSTLPath);
                }
                break;
            }
        }
    }

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
    int nComboIndex = m_editBookName.GetCurSel();
    if (nComboIndex != CB_ERR)
    {
        m_editBookName.GetLBText(nComboIndex, strBookName);
    }

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
            if (i == nIndex)
                nMyRank = nSameCharCount;
        }
    }

    CString strRank;
    strRank.Format(_T("%d"), nMyRank);
    SetDlgItemText(IDC_EDIT_CUR_TYPE_IDX, strRank);

    CString strTotal;
    strTotal.Format(_T("/ %d"), nSameCharCount);
    SetDlgItemText(IDC_STATIC_MAX_IDX, strTotal);

    m_ListCtrl.DeleteAllItems();
    m_ListCtrl.SetRedraw(FALSE);

    int listIdx = 0;
    for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
    {
        if (m_TypeDB.m_Chars[i].m_char == curInfo.m_char)
        {
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
            if (bDuplicate)
                continue;

            CString t;
            t.Format(_T("%d"), m_TypeDB.m_Chars[i].m_sheet);
            int item = m_ListCtrl.InsertItem(listIdx, t);

            t.Format(_T("%d"), m_TypeDB.m_Chars[i].m_line);
            m_ListCtrl.SetItemText(item, 1, t);

            t.Format(_T("%d"), m_TypeDB.m_Chars[i].m_order);
            m_ListCtrl.SetItemText(item, 2, t);

            m_ListCtrl.SetItemData(item, i);

            if (i == m_nSelectIndex)
            {
                m_ListCtrl.SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            }

            listIdx++;
        }
    }

    m_ListCtrl.SetRedraw(TRUE);

    CString path;
    path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"), m_strRootPath, strBookName, curInfo.m_char, curInfo.m_type);

    CFileFind finder;
    if (finder.FindFile(path))
    {
        while (finder.FindNextFile())
        {
            if (finder.IsDots())
                continue;

            if (!m_imgSelectedChar.IsNull()) m_imgSelectedChar.Destroy();
            if (!m_imgTypeChar.IsNull()) m_imgTypeChar.Destroy();

            m_imgSelectedChar.Load(finder.GetFilePath());
            m_imgTypeChar.Load(finder.GetFilePath());
            break;
        }
    }

    CString strSTL;
    strSTL.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"), m_strRootPath, strBookName, curInfo.m_char, curInfo.m_type);
    ViewSTL(strSTL);

    Invalidate();
}

void CTermProjectDlg::OnSelchangeListHistory()
{
    int sel = m_listHistory.GetCurSel();
    if (sel == LB_ERR) return;
    if (sel < 0 || sel >= (int)m_History.size()) return;

    int dataIdx = (int)m_listHistory.GetItemData(sel);

    m_HistoryPos = sel;          // ★ 순서 유지, 위치만 갱신
    m_bHistoryJump = true;       // ★ 히스토리 재정렬 금지 모드
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

    int step = (p->iDelta < 0) ? -1 : +1; // 위버튼(-1) => 위로, 아래버튼(+1) => 아래로

    int newPos = m_HistoryPos + step;

    if (newPos < 0) newPos = 0;
    if (newPos >= (int)m_History.size()) newPos = (int)m_History.size() - 1;

    if (newPos != m_HistoryPos)
    {
        m_HistoryPos = newPos;

        int idx = m_History[m_HistoryPos];

        m_bHistoryJump = true;
        UpdateCharInfo(idx);
        m_bHistoryJump = false;

        SyncHistorySelection();
    }

    *pResult = 0;
}


void CTermProjectDlg::OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if (m_nSelectIndex < 0)
        return;

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
        strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"), m_strRootPath, strBookName, info.m_char, info.m_type);
        ViewSTL(strSTLPath);

        CString path;
        path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"), m_strRootPath, strBookName, info.m_char, info.m_type);

        CFileFind finder;
        if (finder.FindFile(path) && finder.FindNextFile())
        {
            if (!m_imgTypeChar.IsNull()) m_imgTypeChar.Destroy();
            m_imgTypeChar.Load(finder.GetFilePath());
        }

        CString strRank;
        strRank.Format(_T("%d"), nextRankIndex + 1);
        SetDlgItemText(IDC_EDIT_CUR_TYPE_IDX, strRank);

        CWnd* pTypeWnd = GetDlgItem(IDC_STATIC_TYPE_IMAGE);
        if (pTypeWnd) pTypeWnd->Invalidate(FALSE);

        m_ListCtrl.Invalidate(FALSE);
    }

    *pResult = 0;
}

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


void CTermProjectDlg::OnStnClickedStaticFrame()
{
}

void CTermProjectDlg::OnStnClickedStaticSheets()
{
}

void CTermProjectDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW);
    if (pWnd)
    {
        CRect rectView;
        pWnd->GetWindowRect(&rectView);
        ScreenToClient(&rectView);

        if (rectView.PtInRect(point))
        {
            if (m_imgBookPage.IsNull())
                return;

            int nOrgWidth = m_imgBookPage.GetWidth();
            int nOrgHeight = m_imgBookPage.GetHeight();

            if (nOrgWidth == 0 || nOrgHeight == 0)
                return;

            double dRatioX = (double)rectView.Width() / nOrgWidth;
            double dRatioY = (double)rectView.Height() / nOrgHeight;

            int nImgX = (int)((point.x - rectView.left) / dRatioX);
            int nImgY = (int)((point.y - rectView.top) / dRatioY);

            for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
            {
                SCharInfo& info = m_TypeDB.m_Chars[i];

                if (info.m_sheet == m_nCurrentSheet)
                {
                    if (nImgX >= info.m_sx && nImgX <= (info.m_sx + info.m_width) &&
                        nImgY >= info.m_sy && nImgY <= (info.m_sy + info.m_height))
                    {
                        UpdateCharInfo(i);
                        break;
                    }
                }
            }
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

BOOL CTermProjectDlg::OnEraseBkgnd(CDC* pDC)
{
    return CDialogEx::OnEraseBkgnd(pDC);
}


void CTermProjectDlg::OnStnClickedStaticKindCount()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CTermProjectDlg::OnStnClickedStaticTypeImage()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CTermProjectDlg::OnStnClickedStaticCharUnicode()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

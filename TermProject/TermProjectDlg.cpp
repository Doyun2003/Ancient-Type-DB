
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
END_MESSAGE_MAP()


// CTermProjectDlg 메시지 처리기

BOOL CTermProjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 1. 기본 메뉴/아이콘 설정 (건드리지 않음)
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

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// 2. 실행 경로 확보
	TCHAR szPath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	::PathRemoveFileSpec(szPath);
	m_strRootPath = szPath;


	// =============================================================
	// [핵심 1] VTK 초기화 및 조작감 개선 (트랙볼 모드)
	// =============================================================
	m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
	m_vtkRenderWindow->AddRenderer(m_vtkRenderer);

	// 계단 현상 제거 (부드러운 화질)
	m_vtkRenderWindow->SetMultiSamples(8);

	m_vtkInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	m_vtkInteractor->SetRenderWindow(m_vtkRenderWindow);

	// ★★★ [조작감 개선] 드래그할 때만 움직이는 '트랙볼 모드' 적용 ★★★
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
		vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	m_vtkInteractor->SetInteractorStyle(style);


	// =============================================================
	// [중요] 3D 모형을 띄울 화면(Picture Control) 연결
	// =============================================================
	// ※ 주의: 리소스 편집기에서 3D 모형이 뜰 네모칸을 클릭하고 ID를 확인하세요.
	// 사용자님의 resource.h에는 명확한 3D용 ID가 안 보여서, 가장 유력한 곳이나
	// 새로 만드신 ID(예: IDC_STATIC_3D_VIEW)를 사용해야 합니다.

	// 아래 ID를 실제 사용자님의 리소스 ID로 변경해주세요!
	// (만약 ID를 모르겠으면 리소스 뷰 -> 3D칸 클릭 -> 속성 -> ID 확인)
	CWnd* pWnd = GetDlgItem(IDC_STATIC_FRAME); // <-- 여기 ID 확인 필수!

	if (pWnd)
	{
		m_vtkRenderWindow->SetParentId(pWnd->GetSafeHwnd());
		CRect rect;
		pWnd->GetClientRect(&rect);
		m_vtkRenderWindow->SetSize(rect.Width(), rect.Height());
		m_vtkRenderWindow->SetPosition(0, 0);
	}


	// =============================================================
	// [기타] 컨트롤 초기화
	// =============================================================

	// 리스트 컨트롤 헤더
	if (m_ListCtrl.GetSafeHwnd()) {
		// 1. 기존 열 깨끗이 삭제
		while (m_ListCtrl.DeleteColumn(0));

		// 2. 전체 너비 3등분
		CRect rectList;
		m_ListCtrl.GetClientRect(&rectList);
		int nWidth = rectList.Width() / 3;

		// 3. 열 생성 (LVCFMT_CENTER로 가운데 정렬)
		m_ListCtrl.InsertColumn(0, _T("장"), LVCFMT_CENTER, nWidth);
		m_ListCtrl.InsertColumn(1, _T("행"), LVCFMT_CENTER, nWidth);
		m_ListCtrl.InsertColumn(2, _T("번"), LVCFMT_CENTER, rectList.Width() - nWidth * 2 - 1);

		// 4. 스타일 적용
		m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	}

	// 스핀 컨트롤 초기화 (ID로 직접 제어)
	CSpinButtonCtrl* pSpinSheet = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SHEET);
	if (pSpinSheet) { pSpinSheet->SetRange(1, 1000); pSpinSheet->SetPos(1); }

	CSpinButtonCtrl* pSpinType = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_TYPE);
	if (pSpinType) { pSpinType->SetRange(0, 100); pSpinType->SetPos(0); }

	// 돋보기 아이콘
	HICON hSearchIcon = (HICON)::LoadImage(AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_ICON_SEARCH), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	CButton* pBtnOpen = (CButton*)GetDlgItem(IDC_BUTTON_BUTTON_OPEN);
	if (pBtnOpen) pBtnOpen->SetIcon(hSearchIcon);


	// =============================================================
	// [핵심 2] 데이터 로딩 및 자동 실행
	// =============================================================
	m_vtkInteractor->Initialize(); // 마우스 조작 시작

	CString dbPath = m_strRootPath + _T("\\typeDB.csv");
	if (m_TypeDB.ReadCSVFile(dbPath))
	{
		// ★★★ [자동 실행] 파일 읽자마자 0번째 글자를 화면에 띄움 ★★★
		if (m_TypeDB.m_Chars.size() > 0) {
			UpdateCharInfo(0);
		}
	}

	return TRUE;
}

void CTermProjectDlg::InitVtkWindow(void* hWnd) {
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

void CTermProjectDlg::ResizeVtkWindow() {
	CRect rc;
	CWnd* pWnd = GetDlgItem(IDC_STATIC_FRAME);

	if (pWnd) {
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
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		
	}
	else
	{
		CDialogEx::OnPaint();

		CClientDC dc(this);

		// [1] 왼쪽 메인 책 페이지 이미지 및 녹색/빨간 박스 그리기
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

				// 비율 계산 (이미지 좌표 -> 화면 좌표 변환용)
				int nOrgWidth = m_imgBookPage.GetWidth();
				int nOrgHeight = m_imgBookPage.GetHeight();
				double dRatioX = (double)rectView.Width() / nOrgWidth;
				double dRatioY = (double)rectView.Height() / nOrgHeight;

				// 펜 설정
				CPen penGreen(PS_SOLID, 1, RGB(0, 255, 0));
				CPen penRed(PS_SOLID, 2, RGB(255, 0, 0));
				CBrush* pNullBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
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

						// 현재 선택된 글자면 빨간색, 아니면 녹색
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

		// [2] 글자 정보 구역의 이미지 그리기 (기존 IDC_STATIC_SELECTED_CHAR_VIEW)
		if (!m_imgSelectedChar.IsNull()) {
			// ID 확인: 글자 정보 칸의 그림 영역 ID
			CWnd* pWndChar = GetDlgItem(IDC_STATIC_SELECTED_CHAR_VIEW);
			if (pWndChar) {
				CRect rectChar;
				pWndChar->GetWindowRect(&rectChar);
				ScreenToClient(&rectChar);
				m_imgSelectedChar.Draw(dc.m_hDC, rectChar);
			}
		}

		// [3] 활자 정보 구역 (우측 하단 작은 사진)
		if (!m_imgTypeChar.IsNull()) {
			// ID 확인: 활자 정보 칸 내부의 작은 사진 영역 ID
			CWnd* pWndType = GetDlgItem(IDC_STATIC_TYPE_IMAGE);
			if (pWndType) {
				CRect rectType;
				pWndType->GetWindowRect(&rectType);
				ScreenToClient(&rectType);
				m_imgTypeChar.Draw(dc.m_hDC, rectType);
			}
		}
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTermProjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTermProjectDlg::OnClickedButtonCone()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	AfxMessageBox(_T("버튼이 눌렸다."));
	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();

	CString path = _T(".\\Data\\월인천강지곡 권상(3)\\04_3d\\110A11A10000_1.stl");

	ViewSTL(path);
	//reader->Update();

	//// vtkSmartPointer<vtkConeSource> coneSource = vtkSmartPointer<vtkConeSource>::New();

	//vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	//mapper->SetInputConnection(reader->GetOutputPort());
	//vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	//actor->SetMapper(mapper);

	//vtkSmartPointer<vtkRenderer> renderer = m_vtkWindow->GetRenderers()->GetFirstRenderer();
	////renderer->AddActor(actor);
	////renderer->SetBackground(.1, .2, .3);
	////renderer->ResetCamera();

	//m_vtkWindow->AddRenderer(renderer);
	//m_vtkWindow->Render();
}

void CTermProjectDlg::ViewSTL(CString strFilePath) {
	// 1. VTK 윈도우 안전장치
	if (m_vtkRenderWindow == nullptr || m_vtkRenderWindow->GetGenericParentId() == nullptr)
		return;

	// 2. 파일 경로 확인
	if (strFilePath.IsEmpty()) return;

	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();

	// ★★★ [수정된 부분] CP_UTF8을 추가해서 한글 경로가 깨지지 않게 함 ★★★
	// 기존: reader->SetFileName(CT2A(strFilePath));
	// 변경:
	reader->SetFileName(CT2A(strFilePath, CP_UTF8));

	reader->Update();

	// 3. 매퍼 및 액터 설정
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// 디자인 설정
	actor->GetProperty()->SetInterpolationToPhong();
	actor->GetProperty()->SetColor(0.8, 0.65, 0.4);
	actor->GetProperty()->SetAmbient(0.15);
	actor->GetProperty()->SetDiffuse(0.7);
	actor->GetProperty()->SetSpecular(0.4);
	actor->GetProperty()->SetSpecularPower(40.0);

	// 4. 렌더링
	if (m_vtkRenderer != nullptr)
	{
		m_vtkRenderer->RemoveAllViewProps();
		m_vtkRenderer->AddActor(actor);
		m_vtkRenderer->SetBackground(0.15, 0.15, 0.2);
		m_vtkRenderer->ResetCamera();
		m_vtkRenderWindow->Render();
	}
}

void CTermProjectDlg::OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pItemActivate->iItem;

	if (nItem != -1)
	{
		// 1. 클릭한 줄에 연결된 전체 데이터 인덱스를 가져옵니다.
		int nDataIndex = (int)m_ListCtrl.GetItemData(nItem);
		m_nSelectIndex = nDataIndex;
		SCharInfo& info = m_TypeDB.m_Chars[nDataIndex];

		// 2. [수정] 장 이동 로직을 완전히 제거했습니다.
		// 이제 info.m_sheet가 현재 장과 달라도 LoadSheetImage를 호출하지 않습니다.
		// 따라서 왼쪽 책 이미지는 그대로 고정됩니다.

		// 3. 오른쪽 활자 정보 및 3D 모형만 업데이트합니다.
		CString strBookName;
		int nComboIndex = m_editBookName.GetCurSel();
		if (nComboIndex != CB_ERR)
		{
			m_editBookName.GetLBText(nComboIndex, strBookName);

			// STL 모형 갱신
			CString strSTL;
			strSTL.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"), m_strRootPath, strBookName, info.m_char, info.m_type);
			ViewSTL(strSTL);

			// 우측 하단 활자 사진 갱신 (선택 사항: 원하시면 추가하세요)
			CString path;
			path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"), m_strRootPath, strBookName, info.m_char, info.m_type);
			CFileFind finder;
			if (finder.FindFile(path) && finder.FindNextFile()) {
				if (!m_imgTypeChar.IsNull()) m_imgTypeChar.Destroy();
				m_imgTypeChar.Load(finder.GetFilePath());
			}
		}

		// 4. 리스트 컨트롤과 우측 정보창만 갱신 (왼쪽 책 영역은 건드리지 않음)
		m_ListCtrl.Invalidate(FALSE);

		// 우측 하단 이미지 영역만 다시 그리도록 요청
		CWnd* pTypeWnd = GetDlgItem(IDC_STATIC_TYPE_IMAGE);
		if (pTypeWnd) pTypeWnd->Invalidate(FALSE);
	}

	*pResult = 0;
}


void CTermProjectDlg::OnClickedButtonButtonOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 1. 폴더 선택 다이얼로그 띄우기 (Vista 스타일)
	CFolderPickerDialog picker(NULL, 0, this, 0);
	picker.m_ofn.lpstrTitle = _T(".\\Data");

	if (picker.DoModal() == IDOK)
	{
		m_strRootPath = picker.GetPathName(); // 선택된 경로

		// 2. ComboBox 초기화
		m_editBookName.ResetContent();

		// 3. 해당 경로 내의 하위 폴더(책 이름) 검색
		CFileFind finder;
		// 선택된 경로 안의 모든 파일/폴더 검색 (*.*)
		CString strWildcard = m_strRootPath + _T("\\*.*");

		BOOL bWorking = finder.FindFile(strWildcard);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// "."(현재폴더), ".."(상위폴더) 는 제외
			if (finder.IsDots())
				continue;

			// 디렉토리(폴더)인 경우에만 책으로 간주하고 추가
			if (finder.IsDirectory())
			{
				CString strBookName = finder.GetFileName();

				// 필요하다면 특정 조건 확인 (예: 내부에 typeDB.csv가 있는지 등)
				// 여기서는 단순히 폴더 이름을 책 제목으로 추가합니다.
				m_editBookName.AddString(strBookName);
			}
		}

		// 4. 검색된 책이 있다면 첫 번째 항목 자동 선택
		if (m_editBookName.GetCount() > 0)
		{
			m_editBookName.SetCurSel(0);

			// 첫 번째 책이 선택되었으므로, 해당 책 데이터를 로딩하는 함수 호출
			// OnCbnSelchangeBookName(); // 필요 시 강제 호출
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

void CTermProjectDlg::LoadBookData(CString strBookName) {
	// 1. 전체 경로 완성: RootPath \ 책이름 \ typeDB.csv
	CString strCSVPath;
	strCSVPath.Format(_T("%s\\%s\\typeDB.csv"), m_strRootPath, strBookName);

	// 2. CSV 파일 파싱
	if (m_TypeDB.ReadCSVFile(strCSVPath))
	{
		// 3. 리스트 컨트롤 초기화 (이전 책 데이터 지우기)
		m_ListCtrl.DeleteAllItems();

		// 4. 데이터 채우기
		

		// 5. 통계 정보 업데이트 (명세서 요구사항) [cite: 208-213]
		CString strCount;
		
		strCount.Format(_T("%d 개"), m_TypeDB.m_Chars.size());
		// IDC_STATIC_TOTAL_CHARS 같은 ID를 가진 스태틱 텍스트가 있다면 업데이트
		// GetDlgItem(IDC_STATIC_TOTAL_CHARS)->SetWindowText(strCount); 
	}
	else
	{
		AfxMessageBox(_T("선택한 책 폴더에 typeDB.csv 파일이 없습니다.\n") + strCSVPath);
	}

	// 1. 통계 계산용 변수 (set을 이용해 중복 제거)
	std::set<CString> setUniqueChars; // 글자 종류 (예: 가, 나, 다...)
	std::set<CString> setUniqueTypes; // 활자 종류 (예: 가_1, 가_2...)

	for (const auto& info : m_TypeDB.m_Chars) {
		setUniqueChars.insert(info.m_char);

		// 활자 고유 키 생성 (글자코드_타입번호)
		CString strTypeKey;
		strTypeKey.Format(_T("%s_%d"), info.m_char, info.m_type);
		setUniqueTypes.insert(strTypeKey);
	}

	// 2. 화면(Static Text)에 통계 표시
	// IDC_STATIC_TOTAL_COUNT, IDC_STATIC_KIND_COUNT, IDC_STATIC_TYPE_COUNT 등
	// 리소스 편집기에서 해당 Static Text들의 ID를 미리 지정해 두셔야 합니다.

	// 전체 글자 수
	CString strText;
	strText.Format(_T("%d 개"), (int)m_TypeDB.m_Chars.size());
	SetDlgItemText(IDC_STATIC_TOTAL_COUNT, strText); // ID를 본인 프로젝트에 맞게 수정하세요

	// 글자 종류 수
	strText.Format(_T("%d 종"), (int)setUniqueChars.size());
	SetDlgItemText(IDC_STATIC_KIND_COUNT, strText);

	// 활자 수
	strText.Format(_T("%d 개"), (int)setUniqueTypes.size());
	SetDlgItemText(IDC_STATIC_TYPE_COUNT, strText);

	// 3. 첫 페이지(1장) 로딩 시작
	m_nCurrentSheet = 1; // 현재 페이지를 기억할 멤버 변수 (헤더에 선언 필요)
	LoadSheetImage(m_nCurrentSheet);

	if (m_TypeDB.m_Chars.size() > 0)
	{
		// 1. 리스트 컨트롤의 첫 번째 항목(0번 인덱스)을 시각적으로 선택 상태로 만들기
		m_ListCtrl.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_ListCtrl.EnsureVisible(0, FALSE);

		// 2. [핵심] 첫 번째 글자의 유니코드/위치/이미지를 화면(Static Text)에 즉시 표시
		UpdateCharInfo(0);

		// 3. 첫 페이지 번호 설정 및 배경 이미지 로딩
		m_nCurrentSheet = m_TypeDB.m_Chars[0].m_sheet;
		LoadSheetImage(m_nCurrentSheet);

		// 4. 3D 모델도 1번 글자 것으로 초기화 (선택 사항)
		CString strSTLPath;
		strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"),
			m_strRootPath, strBookName, m_TypeDB.m_Chars[0].m_char, m_TypeDB.m_Chars[0].m_type);
		ViewSTL(strSTLPath);
	}

	

	Invalidate(); // 전체 화면 갱신
}

void CTermProjectDlg::OnSelchangeEditBookname()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 현재 선택된 인덱스 가져오기
	int nIndex = m_editBookName.GetCurSel();
	if (nIndex == CB_ERR) return;

	CString strBookName;
	m_editBookName.GetLBText(nIndex, strBookName); // 선택된 책 이름 가져오기
	// 데이터 로딩 함수 호출
	LoadBookData(strBookName);

	if (m_TypeDB.m_Chars.size() > 0) {
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

	// 1. 현재 선택된 책 이름 가져오기
	CString strBookName;
	int nIndex = m_editBookName.GetCurSel();
	if (nIndex == CB_ERR) return;
	m_editBookName.GetLBText(nIndex, strBookName);

	// 2. 이미지 파일 경로 완성 (예: ...\Data\책이름\01_scan\001.jpg)
	// 명세서 파일명 규칙: 3자리 숫자 (001, 002...)
	CString strImgPath;
	strImgPath.Format(_T("%s\\%s\\01_scan\\%03d.jpg"), m_strRootPath, strBookName, nSheet);

	// 3. 기존 이미지 제거 및 새 이미지 로드
	if (!m_imgBookPage.IsNull()) m_imgBookPage.Destroy();

	HRESULT hResult = m_imgBookPage.Load(strImgPath);

	if (FAILED(hResult)) {
		// 이미지가 없을 경우 처리 (필요 시 메시지 박스 등)
		return;
	}

	// 4. 화면 갱신 요청 (OnPaint 호출 유도)
	// 책 이미지가 들어갈 Picture Control의 ID를 IDC_STATIC_BOOK_VIEW라고 가정합니다.
	CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW);
	if (pWnd) {
		pWnd->Invalidate(); // 해당 영역만 다시 그리기
		pWnd->UpdateWindow();
	}

	// IDC_STATIC_SHEETS: "1 / 3장" 이라고 적힌 텍스트 컨트롤 ID
	CString strSheetNum;
	// m_TypeDB.m_nSheet는 전체 장 수 (CSV 읽을 때 저장해뒀다고 가정)
	// 만약 없으면 그냥 3이라고 하드코딩하거나 계산해야 함
	strSheetNum.Format(_T("%d / %d장"), nSheet, m_TypeDB.m_nSheet);
	SetDlgItemText(IDC_STATIC_SHEETS, strSheetNum);


	// [추가] 2. "장 내" 통계 계산
	int nSheetCharCount = 0;
	std::set<CString> setSheetChars;
	std::set<CString> setSheetTypes;

	for (const auto& info : m_TypeDB.m_Chars)
	{
		// 현재 보고 있는 장(nSheet)에 해당하는 글자만 필터링
		if (info.m_sheet == nSheet)
		{
			nSheetCharCount++;
			setSheetChars.insert(info.m_char);

			CString strTypeKey;
			strTypeKey.Format(_T("%s_%d"), info.m_char, info.m_type);
			setSheetTypes.insert(strTypeKey);
		}
	}

	

	// [추가] 3. 화면 업데이트 (ID는 리소스 뷰에서 만든 것과 맞춰주세요)
	// 예: IDC_STATIC_SHEET_TOTAL (장내 글자수)
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
	// IsWindow 체크를 추가하여 컨트롤이 물리적으로 생성된 상태인지 확인합니다.
	if (pWndSheet != NULL && ::IsWindow(pWndSheet->m_hWnd)) {
		pWndSheet->SetWindowTextW(strSheet);
	}

	// 화면 갱신 요청
	Invalidate();
}

void CTermProjectDlg::OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	int nNewSheet = m_nCurrentSheet + pNMUpDown->iDelta;

	// 1. 장 범위 체크
	if (nNewSheet < 1) nNewSheet = 1;
	if (nNewSheet > m_TypeDB.m_nSheet) nNewSheet = m_TypeDB.m_nSheet;

	if (nNewSheet != m_nCurrentSheet)
	{
		m_nCurrentSheet = nNewSheet;
		LoadSheetImage(m_nCurrentSheet); // 이미지 및 장 통계 갱신

		// [추가] 2. 바뀐 장의 "첫 번째 활자"를 찾아서 선택 초기화
		for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
		{
			if (m_TypeDB.m_Chars[i].m_sheet == m_nCurrentSheet)
			{
				// 해당 장의 첫 번째 글자를 찾으면 정보를 갱신하고 루프 종료
				UpdateCharInfo(i);

				// 3D 모델도 해당 활자의 것으로 초기화
				CString strBookName;
				int nComboIndex = m_editBookName.GetCurSel();
				if (nComboIndex != CB_ERR) {
					m_editBookName.GetLBText(nComboIndex, strBookName);
					CString strSTLPath;
					strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"),
						m_strRootPath, strBookName, m_TypeDB.m_Chars[i].m_char, m_TypeDB.m_Chars[i].m_type);
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

	// 1. 범위 및 유효성 체크
	if (nIndex < 0 || nIndex >= (int)m_TypeDB.m_Chars.size()) return;

	m_nSelectIndex = nIndex;
	SCharInfo& curInfo = m_TypeDB.m_Chars[nIndex];

	// 2. 책 이름 가져오기 (파일 경로 생성용)
	CString strBookName;
	int nComboIndex = m_editBookName.GetCurSel();
	if (nComboIndex != CB_ERR) {
		m_editBookName.GetLBText(nComboIndex, strBookName);
	}

	// =================================================================
	// [1] 텍스트 정보 표시 (유니코드, 위치)
	// =================================================================
	SetDlgItemText(IDC_STATIC_CHAR_UNICODE, curInfo.m_char);
	CString pos;
	pos.Format(_T("%d장 %d행 %d번"), curInfo.m_sheet, curInfo.m_line, curInfo.m_order);
	SetDlgItemText(IDC_STATIC_CHAR_POS, pos);


	// =================================================================
	// [2] 같은 글자 순위 계산 (예: 5개 중 1번째)
	// =================================================================
	int nSameCharCount = 0;
	int nMyRank = 1;

	for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
	{
		if (m_TypeDB.m_Chars[i].m_char == curInfo.m_char)
		{
			nSameCharCount++;
			if (i == nIndex) {
				nMyRank = nSameCharCount;
			}
		}
	}

	// 순번 표시 (에딧 컨트롤)
	CString strRank;
	strRank.Format(_T("%d"), nMyRank);
	SetDlgItemText(IDC_EDIT_CUR_TYPE_IDX, strRank);

	// 전체 개수 표시 (Static Text)
	CString strTotal;
	strTotal.Format(_T("/ %d"), nSameCharCount);
	SetDlgItemText(IDC_STATIC_MAX_IDX, strTotal);


	// =================================================================
	// [3] 리스트 컨트롤 갱신 (해당 글자들만 보여주기)
	// =================================================================
	m_ListCtrl.DeleteAllItems();
	m_ListCtrl.SetRedraw(FALSE);

	int listIdx = 0;
	for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
	{
		if (m_TypeDB.m_Chars[i].m_char == curInfo.m_char)
		{
			// 중복 방지 (장, 행, 번호가 같으면 패스)
			bool bDuplicate = false;
			for (int j = 0; j < listIdx; j++) {
				if (m_ListCtrl.GetItemText(j, 0) == CString(std::to_wstring(m_TypeDB.m_Chars[i].m_sheet).c_str()) &&
					m_ListCtrl.GetItemText(j, 1) == CString(std::to_wstring(m_TypeDB.m_Chars[i].m_line).c_str()) &&
					m_ListCtrl.GetItemText(j, 2) == CString(std::to_wstring(m_TypeDB.m_Chars[i].m_order).c_str())) {
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

			if (i == m_nSelectIndex) {
				m_ListCtrl.SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
			listIdx++;
		}
	}
	m_ListCtrl.SetRedraw(TRUE);


	// =================================================================
	// [4] 이미지 로드 (선택 글자 & 활자 정보 이미지)
	// =================================================================
	CString path;
	path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"), m_strRootPath, strBookName, curInfo.m_char, curInfo.m_type);

	CFileFind finder;
	if (finder.FindFile(path)) {
		while (finder.FindNextFile()) {
			if (finder.IsDots()) continue;

			// 두 이미지 컨트롤 모두 갱신
			if (!m_imgSelectedChar.IsNull()) m_imgSelectedChar.Destroy();
			if (!m_imgTypeChar.IsNull()) m_imgTypeChar.Destroy();

			m_imgSelectedChar.Load(finder.GetFilePath());
			m_imgTypeChar.Load(finder.GetFilePath());
			break;
		}
	}


	// =================================================================
	// [5] 3D 모델(STL) 갱신 (여기가 추가된 부분입니다!)
	// =================================================================
	CString strSTL;
	strSTL.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"), m_strRootPath, strBookName, curInfo.m_char, curInfo.m_type);
	ViewSTL(strSTL);


	// 화면 갱신 요청
	Invalidate();
}



void CTermProjectDlg::OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if (m_nSelectIndex < 0) return;

	// 1. 현재 글자와 동일한 활자 목록 추출
	CString curChar = m_TypeDB.m_Chars[m_nSelectIndex].m_char;
	std::vector<int> sameCharIndices;
	int currentRankIndex = -1;

	for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++) {
		if (m_TypeDB.m_Chars[i].m_char == curChar) {
			sameCharIndices.push_back(i);
			if (i == m_nSelectIndex) currentRankIndex = (int)sameCharIndices.size() - 1;
		}
	}

	// 2. 다음 활자 인덱스 계산
	int nextRankIndex = currentRankIndex + pNMUpDown->iDelta;
	if (nextRankIndex < 0) nextRankIndex = 0;
	if (nextRankIndex >= (int)sameCharIndices.size()) nextRankIndex = (int)sameCharIndices.size() - 1;

	if (nextRankIndex != currentRankIndex) {
		m_nSelectIndex = sameCharIndices[nextRankIndex];
		SCharInfo& info = m_TypeDB.m_Chars[m_nSelectIndex];

		CString strBookName;
		m_editBookName.GetLBText(m_editBookName.GetCurSel(), strBookName);

		// 3. STL 모형 갱신 (STL은 자체 Render를 호출하므로 깜빡임과 무관함)
		CString strSTLPath;
		strSTLPath.Format(_T("%s\\%s\\04_3d\\%s_%d.stl"), m_strRootPath, strBookName, info.m_char, info.m_type);
		ViewSTL(strSTLPath);

		// 4. 하단 활자 이미지 갱신
		CString path;
		path.Format(_T("%s\\%s\\03_type\\%s\\%d\\*.*"), m_strRootPath, strBookName, info.m_char, info.m_type);
		CFileFind finder;
		if (finder.FindFile(path) && finder.FindNextFile()) {
			if (!m_imgTypeChar.IsNull()) m_imgTypeChar.Destroy();
			m_imgTypeChar.Load(finder.GetFilePath());
		}

		// 5. [깜빡임 방지 핵심] 전체 Invalidate() 대신 필요한 영역만 갱신
		// (1) 순번 텍스트 업데이트 (텍스트는 깜빡임이 거의 없음)
		CString strRank;
		strRank.Format(_T("%d"), nextRankIndex + 1);
		SetDlgItemText(IDC_EDIT_CUR_TYPE_IDX, strRank);

		// (2) 우측 하단 이미지 영역만 '지우지 않고' 다시 그리기
		CWnd* pTypeWnd = GetDlgItem(IDC_STATIC_TYPE_IMAGE);
		if (pTypeWnd) pTypeWnd->Invalidate(FALSE);

		// (3) 리스트 컨트롤의 파란색 선택 바 위치 업데이트
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

		// ★★★ 이 줄이 핵심입니다! 기존 코드에 이 줄이 없으면 색이 안 변합니다. ★★★
		pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;

		// [아래는 기존에 드린 디자인 코드와 같습니다]
		if (nDataIndex == m_nSelectIndex)
		{
			pLVCD->clrTextBk = RGB(91, 155, 213); // 파란색 배경
			pLVCD->clrText = RGB(255, 255, 255);  // 흰색 글자
		}
		else
		{
			pLVCD->clrText = RGB(0, 0, 0); // 검정 글자
			if (nItem % 2 == 0)
				pLVCD->clrTextBk = RGB(255, 255, 255); // 흰색
			else
				pLVCD->clrTextBk = RGB(237, 242, 250); // 연한 하늘색
		}
		*pResult = CDRF_NEWFONT;
	}
	break;
	}
}

void CTermProjectDlg::OnStnClickedStaticFrame()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CTermProjectDlg::OnStnClickedStaticSheets()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


// TermProjectDlg.cpp 맨 아래에 추가

void CTermProjectDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 1. 책 이미지가 표시되는 영역(Static Control) 가져오기
	CWnd* pWnd = GetDlgItem(IDC_STATIC_BOOK_VIEW);
	if (pWnd)
	{
		CRect rectView;
		pWnd->GetWindowRect(&rectView);
		ScreenToClient(&rectView); // 다이얼로그 기준 좌표로 변환

		// 2. 클릭한 곳이 책 이미지 영역 안인지 확인
		if (rectView.PtInRect(point))
		{
			if (m_imgBookPage.IsNull()) return;

			// 3. 좌표 변환 (화면 클릭 좌표 -> 원본 이미지 좌표)
			// OnPaint에서 그릴 때 사용했던 비율 계산 공식을 그대로 역이용합니다.
			int nOrgWidth = m_imgBookPage.GetWidth();
			int nOrgHeight = m_imgBookPage.GetHeight();

			// 0으로 나누기 방지
			if (nOrgWidth == 0 || nOrgHeight == 0) return;

			double dRatioX = (double)rectView.Width() / nOrgWidth;
			double dRatioY = (double)rectView.Height() / nOrgHeight;

			// 클릭한 화면 좌표를 -> 원본 이미지 좌표로 변환
			int nImgX = (int)((point.x - rectView.left) / dRatioX);
			int nImgY = (int)((point.y - rectView.top) / dRatioY);

			// 4. 해당 좌표에 있는 글자 찾기 (현재 페이지 내에서만 검색)
			for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++)
			{
				SCharInfo& info = m_TypeDB.m_Chars[i];

				// 현재 보고 있는 장(Sheet)에 있는 글자인가?
				if (info.m_sheet == m_nCurrentSheet)
				{
					// 클릭한 좌표가 이 글자의 박스(rect) 안에 들어가는가?
					if (nImgX >= info.m_sx && nImgX <= (info.m_sx + info.m_width) &&
						nImgY >= info.m_sy && nImgY <= (info.m_sy + info.m_height))
					{
						// 찾았다! (i번째 글자)

						// 글자 선택 및 정보 갱신 (리스트, 이미지, 3D 등 모두 연동됨)
						UpdateCharInfo(i);

						// 찾았으면 루프 종료 (겹친 글자가 있어도 제일 먼저 잡히는 것 선택)
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
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return CDialogEx::OnEraseBkgnd(pDC);
}

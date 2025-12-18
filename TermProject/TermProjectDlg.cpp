
// TermProjectDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "TermProject.h"
#include "TermProjectDlg.h"
#include "afxdialogex.h"

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
}

void CTermProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CHARS, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CTermProjectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONE, &CTermProjectDlg::OnClickedButtonCone)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CTermProjectDlg::OnNMClickListChars)
END_MESSAGE_MAP()


// CTermProjectDlg 메시지 처리기

BOOL CTermProjectDlg::OnInitDialog()
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
	CWnd* pPC = GetDlgItem(IDC_STATIC_FRAME);
	if (pPC) {
		this->InitVtkWindow(pPC->GetSafeHwnd());
		this->ResizeVtkWindow();
	}

	m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ListCtrl.InsertColumn(0, _T("글자코드"), LVCFMT_LEFT, 120);
	m_ListCtrl.InsertColumn(1, _T("타입"), LVCFMT_CENTER, 50);
	m_ListCtrl.InsertColumn(2, _T("장"), LVCFMT_CENTER, 50);

	// 2. CSV 파일 읽기 (상대 경로 사용!)
	// .\\Data\\ 는 "현재 프로그램 위치의 Data 폴더"라는 뜻입니다.
	CString dbPath = _T(".\\Data\\typeDB.csv");

	if (m_TypeDB.ReadCSVFile(dbPath)) {
		// 3. 읽어온 데이터를 리스트에 한 줄씩 채우기
		for (int i = 0; i < (int)m_TypeDB.m_Chars.size(); i++) {
			SCharInfo& info = m_TypeDB.m_Chars[i];

			// 0번 열: 글자 코드
			int nIndex = m_ListCtrl.InsertItem(i, info.m_char);

			// 1번 열: 타입 번호
			CString strTemp;
			strTemp.Format(_T("%d"), info.m_type);
			m_ListCtrl.SetItemText(nIndex, 1, strTemp);

			// 2번 열: 장 번호
			strTemp.Format(_T("%d"), info.m_sheet);
			m_ListCtrl.SetItemText(nIndex, 2, strTemp);
		}
	}
	else {
		AfxMessageBox(_T("Data 폴더 안에 typeDB.csv 파일이 없습니다!"));
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CTermProjectDlg::InitVtkWindow(void* hWnd) {
	m_pInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(1.0, 0.0, 0.0);

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
HCURSOR CTermProjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTermProjectDlg::OnClickedButtonCone()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	AfxMessageBox(_T("버튼이 눌렸다."));
	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();

	CString path = _T(".\\Data\\04_3d\\110A11A10000_1.stl");

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
	CFileFind finder;
	if (!finder.FindFile(strFilePath)) {
		AfxMessageBox(_T("파일을 찾을 수 없습니다.\n경로를 다시 확인해주세요"));
		if (m_vtkWindow) {
			vtkSmartPointer<vtkRenderer> renderer = m_vtkWindow->GetRenderers()->GetFirstRenderer();
			if (renderer) {
				renderer->RemoveAllViewProps();
				m_vtkWindow->Render();
			}
		}
		return;
	}

	CT2A ascii(strFilePath, CP_UTF8);

	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();

	reader->SetFileName(ascii.m_psz);
	reader->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkRenderer> renderer = m_vtkWindow->GetRenderers()->GetFirstRenderer();
	renderer->RemoveAllViewProps();
	renderer->AddActor(actor);
	renderer->ResetCamera();

	m_vtkWindow->Render();
}

void CTermProjectDlg::OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// 클릭한 줄 번호 (Index)
	int nIndex = pNMItemActivate->iItem;

	if (nIndex != -1) { // 빈 곳을 클릭한 게 아니라면
		// 1. 클릭한 줄의 데이터 가져오기
		SCharInfo& info = m_TypeDB.m_Chars[nIndex];

		// 2. STL 파일 경로 만들기 (상대 경로)
		// 구조: .\Data\03_type\글자코드\글자코드_타입.stl
		CString strBasePath = _T(".\\Data\\04_3d\\");

		CString strFileName;
		// 예: 110A..._1.stl
		strFileName.Format(_T("%s_%d.stl"), info.m_char, info.m_type);

		CString strFullPath = strBasePath + strFileName;

		// 3. 3D 뷰어 호출! (경로 확인용 메시지 박스는 이제 지우셔도 됩니다)
		ViewSTL(strFullPath);
	}
	*pResult = 0;
}

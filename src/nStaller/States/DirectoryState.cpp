#include "DirectoryState.h"
#include "../Installer.h"
#include <iomanip>
#include <shlobj.h>
#include <shlwapi.h>
#include <sstream>
#include <vector>


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static std::string directory_and_package(const std::string & dir, const std::string & pack)
{
	std::string directory = dir;
	// Ensure last character is a backslash
	if (dir.size() && dir[dir.size() - 1ull] != '\\')
		directory += '\\' + pack;
	else
		directory += pack;
	return directory;
}

DirectoryState::~DirectoryState()
{
	UnregisterClass("DIRECTORY_STATE", m_hinstance);
	DestroyWindow(m_hwnd);
	DestroyWindow(m_directoryField);
	DestroyWindow(m_browseButton);
}

DirectoryState::DirectoryState(Installer * installer, const HINSTANCE hInstance, const HWND parent, const RECT & rc)
	: FrameState(installer) 
{
	// Create window class
	m_hinstance = hInstance;
	m_wcex.cbSize = sizeof(WNDCLASSEX);
	m_wcex.style = CS_HREDRAW | CS_VREDRAW;
	m_wcex.lpfnWndProc = WndProc;
	m_wcex.cbClsExtra = 0;
	m_wcex.cbWndExtra = 0;
	m_wcex.hInstance = hInstance;
	m_wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	m_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	m_wcex.lpszMenuName = NULL;
	m_wcex.lpszClassName = "DIRECTORY_STATE";
	m_wcex.hIconSm = LoadIcon(m_wcex.hInstance, IDI_APPLICATION);
	RegisterClassEx(&m_wcex);
	m_hwnd = CreateWindow("DIRECTORY_STATE", "", WS_OVERLAPPED | WS_VISIBLE | WS_CHILD, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, parent, NULL, hInstance, NULL);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
	setVisible(false);

	// Create directory lookup fields
	m_browseButton = CreateWindow("BUTTON", "Browse", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 510, 149, 100, 25, m_hwnd, NULL, hInstance, NULL);
	m_directoryField = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", directory_and_package(m_installer->getDirectory(), m_installer->getPackageName()).c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 10, 150, 490, 25, m_hwnd, NULL, hInstance, NULL);
}

void DirectoryState::enact()
{
	m_installer->showButtons(true, true, true);
	m_installer->enableButtons(true, true, true);
}

void DirectoryState::pressPrevious()
{
	m_installer->setState(Installer::AGREEMENT_STATE);
}

void DirectoryState::pressNext()
{
	auto directory = m_installer->getDirectory();
	
	if (directory == "" || directory == " " || directory.length() < 3)
		MessageBox(
			NULL,
			"Please enter a valid directory before proceeding.",
			"Invalid path!",
			MB_OK | MB_ICONERROR | MB_TASKMODAL
		);
	else
		m_installer->setState(Installer::INSTALL_STATE);
}

void DirectoryState::pressClose()
{
	// No new screen
	PostQuitMessage(0);
}

static HRESULT CreateDialogEventHandler(REFIID riid, void **ppv)
{
	/** File Dialog Event Handler */
	class DialogEventHandler : public IFileDialogEvents, public IFileDialogControlEvents {
	private:
		~DialogEventHandler() { };
		long _cRef;


	public:
		// Constructor
		DialogEventHandler() : _cRef(1) { };


		// IUnknown methods
		IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
			static const QITAB qit[] = {
				QITABENT(DialogEventHandler, IFileDialogEvents),
				QITABENT(DialogEventHandler, IFileDialogControlEvents),
				{ 0 },
			};
			return QISearch(this, qit, riid, ppv);
		}
		IFACEMETHODIMP_(ULONG) AddRef() {
			return InterlockedIncrement(&_cRef);
		}
		IFACEMETHODIMP_(ULONG) Release() {
			long cRef = InterlockedDecrement(&_cRef);
			if (!cRef)
				delete this;
			return cRef;
		}


		// IFileDialogEvents methods
		IFACEMETHODIMP OnFileOk(IFileDialog *) { return S_OK; };
		IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
		IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
		IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
		IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
		IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
		IFACEMETHODIMP OnTypeChange(IFileDialog *) { return S_OK; };
		IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };


		// IFileDialogControlEvents methods
		IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *, DWORD, DWORD) { return S_OK; };
		IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
		IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
		IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };
	};

	*ppv = NULL;
	DialogEventHandler *pDialogEventHandler = new (std::nothrow) DialogEventHandler();
	HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr)) {
		hr = pDialogEventHandler->QueryInterface(riid, ppv);
		pDialogEventHandler->Release();
	}
	return hr;
}

HRESULT OpenFileDialog(std::string & directory)
{
	// CoCreate the File Open Dialog object.
	IFileDialog *pfd = NULL;
	IFileDialogEvents *pfde = NULL;
	DWORD dwCookie, dwFlags;
	HRESULT hr = S_FALSE;
	if (
		SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))) &&
		SUCCEEDED(CreateDialogEventHandler(IID_PPV_ARGS(&pfde))) &&
		SUCCEEDED(pfd->Advise(pfde, &dwCookie)) &&
		SUCCEEDED(pfd->GetOptions(&dwFlags)) &&
		SUCCEEDED(pfd->SetOptions(dwFlags | FOS_PICKFOLDERS | FOS_OVERWRITEPROMPT | FOS_CREATEPROMPT)) &&
		SUCCEEDED(pfd->Show(NULL))
		)
	{
		// The result is an IShellItem object.
		IShellItem *psiResult;
		PWSTR pszFilePath = NULL;
		if (SUCCEEDED(pfd->GetResult(&psiResult)) && SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
			std::wstringstream ss;
			ss << pszFilePath;
			auto ws = ss.str();
			typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
			const std::locale locale("");
			const converter_type& converter = std::use_facet<converter_type>(locale);
			std::vector<char> to(ws.length() * converter.max_length());
			std::mbstate_t state;
			const wchar_t* from_next;
			char* to_next;
			const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);
			if (result == converter_type::ok || result == converter_type::noconv) {
				directory = std::string(&to[0], to_next);
				hr = S_OK;
			}
			CoTaskMemFree(pszFilePath);
			psiResult->Release();
		}

		// Unhook the event handler.
		pfd->Unadvise(dwCookie);
		pfde->Release();
		pfd->Release();
	}
	return hr;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto ptr = (DirectoryState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (message == WM_PAINT) {
		PAINTSTRUCT ps;
		Graphics graphics(BeginPaint(hWnd, &ps));

		// Draw Background
		SolidBrush solidWhiteBackground(Color(255, 255, 255, 255));
		graphics.FillRectangle(&solidWhiteBackground, 0, 0, 630, 450);
		LinearGradientBrush backgroundGradient(
			Point(0, 0),
			Point(0, 450),
			Color(50, 25, 125, 225),
			Color(255, 255, 255, 255)
		);
		graphics.FillRectangle(&backgroundGradient, 0, 0, 630, 450);

		// Preparing Fonts
		FontFamily  fontFamily(L"Segoe UI");
		Font        bigFont(&fontFamily, 25, FontStyleBold, UnitPixel);
		Font        regFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
		SolidBrush  blueBrush(Color(255, 25, 125, 225));
		SolidBrush  blackBrush(Color(255, 0, 0, 0));

		// Draw Text
		graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);
		graphics.DrawString(L"Where would you like to install to?", -1, &bigFont, PointF{ 10, 10 }, &blueBrush);
		graphics.DrawString(L"Choose a folder by pressing the 'Browse' button.", -1, &regFont, PointF{ 10, 100 }, &blackBrush);
		graphics.DrawString(L"Alternatively, type a specific directory into the box below.", -1, &regFont, PointF{ 10, 115 }, &blackBrush);

		constexpr static auto readableFileSize = [](const size_t & size) -> std::wstring {
			auto remainingSize = (double)size;
			constexpr static wchar_t * units[] = { L" B", L" KB", L" MB", L" GB", L" TB", L" PB" };
			int i = 0;
			while (remainingSize > 1024.00) {
				remainingSize /= 1024.00;
				++i;
			}
			std::wstringstream stream;
			stream << std::fixed << std::setprecision(2) << remainingSize;
			return stream.str() + units[i] + L"\t(" + std::to_wstring(size) + L" bytes )";
		};
		graphics.DrawString(L"Disk Space", -1, &regFont, PointF{ 10, 200 }, &blackBrush);
		graphics.DrawString((L"    Capacity:\t\t\t" + readableFileSize(ptr->m_installer->getDirectorySizeCapacity())).c_str(), -1, &regFont, PointF{ 10, 225 }, &blackBrush);
		graphics.DrawString((L"    Available:\t\t\t" + readableFileSize(ptr->m_installer->getDirectorySizeAvailable())).c_str(), -1, &regFont, PointF{ 10, 240 }, &blackBrush);
		graphics.DrawString((L"    Required:\t\t\t" + readableFileSize(ptr->m_installer->getDirectorySizeRequired())).c_str(), -1, &regFont, PointF{ 10, 255 }, &blackBrush);


		EndPaint(hWnd, &ps);
		return S_OK;
	}
	else if (message == WM_COMMAND) {
		const auto notification = HIWORD(wParam);
		auto controlHandle = HWND(lParam);
		if (notification == BN_CLICKED) {
			std::string directory("");
			if (SUCCEEDED(OpenFileDialog(directory))) {
				if (directory != "" && directory.length() > 2ull) {
					directory = directory_and_package(directory, ptr->m_installer->getPackageName());
					ptr->m_installer->setDirectory(directory);
					SetWindowTextA(ptr->m_directoryField, directory.c_str());
					RECT rc = { 10, 200, 600, 300 };
					RedrawWindow(hWnd, &rc, NULL, RDW_INVALIDATE);
					return S_OK;
				}
			}
		}
		else if (notification == EN_CHANGE) {
			std::vector<char> data(GetWindowTextLength(controlHandle) + 1ull);
			GetWindowTextA(controlHandle, &data[0], (int)data.size());
			ptr->m_installer->setDirectory(std::string(data.data()));
			RECT rc = { 10, 200, 600, 300 };
			RedrawWindow(hWnd, &rc, NULL, RDW_INVALIDATE);
			return S_OK;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
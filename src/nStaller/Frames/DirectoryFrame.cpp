#include "DirectoryFrame.h"
#include <sstream>
#include <vector>
#include <shlobj.h>
#include <shlwapi.h>


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DirectoryFrame::~DirectoryFrame()
{
	UnregisterClass("DIRECTORY_FRAME", m_hinstance);
	DestroyWindow(m_hwnd);
	DestroyWindow(m_directoryField);
	DestroyWindow(m_browseButton);
}

DirectoryFrame::DirectoryFrame(std::string * directory, const HINSTANCE hInstance, const HWND parent, const RECT & rc)
{
	// Create window class
	m_directory = directory;
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
	m_wcex.lpszClassName = "DIRECTORY_FRAME";
	m_wcex.hIconSm = LoadIcon(m_wcex.hInstance, IDI_APPLICATION);
	RegisterClassEx(&m_wcex);
	m_hwnd = CreateWindow("DIRECTORY_FRAME", "", WS_OVERLAPPED | WS_VISIBLE | WS_CHILD | WS_DLGFRAME, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, parent, NULL, hInstance, NULL);

	// Create directory lookup fields
	m_directoryField = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", directory->c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 10, 150, 490, 25, m_hwnd, NULL, hInstance, NULL);
	m_browseButton = CreateWindow("BUTTON", "Browse", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 510, 149, 100, 25, m_hwnd, NULL, hInstance, NULL);
	SetWindowLongPtr(m_browseButton, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtr(m_directoryField, GWLP_USERDATA, (LONG_PTR)m_directory);
	setVisible(false);
}

void DirectoryFrame::setDirectory(const std::string & dir)
{
	*m_directory = dir;
	SetWindowText(m_directoryField, dir.c_str());
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
		IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *, DWORD, DWORD ) { return S_OK; };
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
	if (message == WM_PAINT) {
		PAINTSTRUCT ps;
		auto hdc = BeginPaint(hWnd, &ps);
		auto big_font = CreateFont(35, 15, 0, 0, FW_ULTRABOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_ROMAN, "Segoe UI");
		auto reg_font = CreateFont(17, 7, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_ROMAN, "Segoe UI");

		// Draw Text
		constexpr static char* text[] = {
			"Where would you like to install to?",
			"Choose a folder by pressing the 'Browse' button.",
			"Alternatively, type a specific directory into the box below.",
			"Press the 'Next' button to begin installing . . ."
		};
		SelectObject(hdc, big_font);
		SetTextColor(hdc, RGB(25, 125, 225));
		TextOut(hdc, 10, 10, text[0], (int)strlen(text[0]));

		SelectObject(hdc, reg_font);
		SetTextColor(hdc, RGB(0, 0, 0));
		TextOut(hdc, 10, 100, text[1], (int)strlen(text[1]));
		TextOut(hdc, 10, 115, text[2], (int)strlen(text[2]));
		TextOut(hdc, 10, 420, text[3], (int)strlen(text[3]));

		// Cleanup
		DeleteObject(big_font);
		DeleteObject(reg_font);

		EndPaint(hWnd, &ps);
		return S_OK;
	}
	else if (message == WM_COMMAND) {
		const auto notification = HIWORD(wParam);
		auto controlHandle = HWND(lParam);
		if (notification == BN_CLICKED) {
			auto dirFrame = (DirectoryFrame*)GetWindowLongPtr(controlHandle, GWLP_USERDATA);
			if (dirFrame) {
				std::string directory("");
				if (SUCCEEDED(OpenFileDialog(directory))) {
					if (directory != "" && directory.length() > 2ull) {
						dirFrame->setDirectory(directory);
						return S_OK;
					}
				}
			}
		}
		else if (notification == EN_CHANGE) {
			auto dirPtr = (std::string*)GetWindowLongPtr(controlHandle, GWLP_USERDATA);
			if (dirPtr) {
				std::vector<char> data(GetWindowTextLength(controlHandle) + 1ull);
				GetWindowTextA(controlHandle, &data[0], (int)data.size());
				*dirPtr = std::string(data.data());
				return S_OK;
			}
		}
	}	
	return DefWindowProc(hWnd, message, wParam, lParam);
}
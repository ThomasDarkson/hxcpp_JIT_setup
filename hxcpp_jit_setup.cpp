#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <urlmon.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "hxcpp_jit_setup_version.h"

#define HXCPP_WIDEN2(x) L##x
#define HXCPP_WIDEN(x) HXCPP_WIDEN2(x)
#define HXCPP_JIT_SETUP_VERSION HXCPP_WIDEN(HXCPP_JIT_SETUP_VERSION_STR)

namespace {

const wchar_t* kVersion = HXCPP_JIT_SETUP_VERSION;
const wchar_t* kTitle = L"hxcpp_JIT setup " HXCPP_JIT_SETUP_VERSION;
const wchar_t* kSourceUrl = L"https://github.com/ThomasDarkson/hxcpp_JIT_setup";
const wchar_t* kLicenseUrl = L"https://visualstudio.microsoft.com/license-terms/";
const wchar_t* kSdkLicenseUrl = L"https://aka.ms/WinSDKLicenseURL";
const wchar_t* kBuildToolsUrl = L"https://aka.ms/vs/18/stable/vs_BuildTools.exe";

const wchar_t* kComponentMsvc = L"Microsoft.VisualStudio.Component.VC.Tools.x86.x64";
const wchar_t* kComponentSdk = L"Microsoft.VisualStudio.Component.Windows11SDK.28000";
const wchar_t* kToolsLabel = L"MSVC build tools and Windows 11 SDK";

enum Str {
  S_TITLE, S_BODY, S_LINK_LICENSE, S_LINK_SDK_LICENSE, S_LINK_SOURCE, S_BTN_CLOSE,
  S_PREPARING, S_DOWNLOADING, S_INSTALLING_TOOLS,
  S_DL_FAILED, S_FAILED, S_LOG_HINT, S_DONE, S_REBOOT, S_STILL_MISSING,
  S_WAIT_STEP, S_THIS_PROGRAM, S_FOOTER, S_COUNT
};

const wchar_t* kStrings[S_COUNT] = {
    L"Setting things up",
    L"{0} needs a few Microsoft components to compile C++ code while it runs. They are being installed now.\nBy using this program you accept Microsoft's license terms.",
    L"Build Tools license terms",
    L"Windows SDK license terms",
    L"Source code",
    L"Close",
    L"Checking your computer...",
    L"Downloading {0}...",
    L"Installing MSVC build tools and Windows SDK... This can take 10+ minutes.",
    L"Download failed: {0}. Please check your internet connection and try again.",
    L"Could not install {0} (exit code {1}). Close this window and try again.",
    L" Microsoft's installer log: {0}",
    L"All done! Everything is installed.",
    L"Almost done! Windows needs to restart once before this works. Please restart your PC.",
    L"Something is still missing after the installation. Close this window and try again.",
    L"Please wait until the installation has finished.",
    L"This program",
    L"You won't see this window again after the setup finishes.\nPlease do not close this window.",
};

void replaceAll(std::wstring& s, const std::wstring& from, const std::wstring& to) {
  for (size_t pos = 0; (pos = s.find(from, pos)) != std::wstring::npos; pos += to.size()) s.replace(pos, from.size(), to);
}

std::wstring T(int id, const std::wstring& a0 = L"", const std::wstring& a1 = L"") {
  std::wstring r = kStrings[id];
  replaceAll(r, L"{0}", a0);
  replaceAll(r, L"{1}", a1);
  return r;
}

std::wstring getEnv(const wchar_t* name) {
  DWORD len = GetEnvironmentVariableW(name, nullptr, 0);
  if (!len) return L"";
  std::wstring s(len, L'\0');
  len = GetEnvironmentVariableW(name, &s[0], len);
  s.resize(len);
  return s;
}

bool fileExists(const std::wstring& p) { return GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

std::wstring trim(const std::wstring& s) {
  size_t b = 0, e = s.size();
  while (b < e && (s[b] == L' ' || s[b] == L'\t' || s[b] == L'\r' || s[b] == L'\n')) ++b;
  while (e > b && (s[e - 1] == L' ' || s[e - 1] == L'\t' || s[e - 1] == L'\r' || s[e - 1] == L'\n')) --e;
  return s.substr(b, e - b);
}

std::wstring utf8ToWide(const std::string& s) {
  if (s.empty()) return L"";
  int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
  std::wstring w(n, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
  return w;
}

bool isElevated() {
  HANDLE token = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return false;
  TOKEN_ELEVATION e{};
  DWORD sz = 0;
  bool r = GetTokenInformation(token, TokenElevation, &e, sizeof(e), &sz) && e.TokenIsElevated;
  CloseHandle(token);
  return r;
}

bool hasArg(int argc, wchar_t** argv, const wchar_t* name) {
  for (int i = 1; i < argc; ++i)
    if (lstrcmpiW(argv[i], name) == 0) return true;
  return false;
}

int runCapture(std::wstring cmd, std::string& out) {
  STARTUPINFOW si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.wShowWindow = SW_HIDE;
  SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
  HANDLE rd = nullptr, wr = nullptr;
  if (!CreatePipe(&rd, &wr, &sa, 0)) return -1;
  SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
  HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);
  si.hStdInput = nul;
  si.hStdOutput = wr;
  si.hStdError = wr;
  PROCESS_INFORMATION pi{};
  BOOL ok = CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
  CloseHandle(wr);
  if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
  if (!ok) {
    CloseHandle(rd);
    return -1;
  }
  char buf[4096];
  DWORD n = 0;
  while (ReadFile(rd, buf, sizeof(buf), &n, nullptr) && n) out.append(buf, n);
  CloseHandle(rd);
  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 1;
  GetExitCodeProcess(pi.hProcess, &code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return (int)code;
}

std::wstring installerDir() { return getEnv(L"ProgramFiles(x86)") + L"\\Microsoft Visual Studio\\Installer"; }

bool toolsInstalled() {
  std::wstring vswhere = installerDir() + L"\\vswhere.exe";
  if (!fileExists(vswhere)) return false;
  std::wstring cmd = L"\"" + vswhere + L"\" -latest -products * -version [18.0,19.0) -nologo -utf8 -requires " +
                     kComponentMsvc + L" " + kComponentSdk + L" -format value -property installationPath";
  std::string out;
  if (runCapture(cmd, out) != 0) return false;
  return !trim(utf8ToWide(out)).empty();
}

bool allInstalled() { return toolsInstalled(); }

enum { WM_APP_STATUS = WM_APP + 1, WM_APP_PROGRESS, WM_APP_DONE };
const WPARAM PROG_MARQUEE = 1000;
enum { IDC_LICENSE = 101, IDC_SDK_LICENSE, IDC_SOURCE, IDC_CLOSE };
const UINT_PTR TIMER_AUTOCLOSE = 1;

HWND g_hwnd, g_linkLicense, g_linkSdk, g_linkSource, g_prog, g_status, g_btnClose;
HFONT g_font, g_fontTitle, g_fontLink;
bool g_busy = true, g_marquee = false;
std::wstring g_app;
int g_exit = 2;
int g_preview = 0;

double g_wTotal = 1, g_wDone = 0, g_wSpan = 0;
const double kToolsDownloadWeight = 1, kToolsInstallWeight = 12;

void post(UINT msg, WPARAM w = 0, LPARAM l = 0) { PostMessageW(g_hwnd, msg, w, l); }
void postStatus(const std::wstring& s) { PostMessageW(g_hwnd, WM_APP_STATUS, 0, (LPARAM) new std::wstring(s)); }

void reportOverall(double done) {
  double pct = g_wTotal > 0 ? done / g_wTotal * 100.0 : 0;
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  post(WM_APP_PROGRESS, (WPARAM)pct);
}

class DlProgress final : public IBindStatusCallback {
  LONG ref_ = 1;

public:
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
    if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
      *ppv = static_cast<IBindStatusCallback*>(this);
      AddRef();
      return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
  }
  STDMETHODIMP_(ULONG) AddRef() override { return (ULONG)InterlockedIncrement(&ref_); }
  STDMETHODIMP_(ULONG) Release() override {
    ULONG r = (ULONG)InterlockedDecrement(&ref_);
    if (!r) delete this;
    return r;
  }
  STDMETHODIMP OnStartBinding(DWORD, IBinding*) override { return S_OK; }
  STDMETHODIMP GetPriority(LONG*) override { return E_NOTIMPL; }
  STDMETHODIMP OnLowResource(DWORD) override { return S_OK; }
  STDMETHODIMP OnProgress(ULONG p, ULONG max, ULONG, LPCWSTR) override {
    if (max) reportOverall(g_wDone + g_wSpan * ((double)p / (double)max));
    return S_OK;
  }
  STDMETHODIMP OnStopBinding(HRESULT, LPCWSTR) override { return S_OK; }
  STDMETHODIMP GetBindInfo(DWORD* flags, BINDINFO* bi) override {
    if (flags) *flags = BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE;
    if (bi) {
      ULONG cb = bi->cbSize;
      memset(bi, 0, cb);
      bi->cbSize = cb;
    }
    return S_OK;
  }
  STDMETHODIMP OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) override { return S_OK; }
  STDMETHODIMP OnObjectAvailable(REFIID, IUnknown*) override { return S_OK; }
};

std::wstring tempDir() {
  wchar_t buf[MAX_PATH + 1];
  DWORD n = GetTempPathW(MAX_PATH, buf);
  std::wstring d(buf, n);
  d += L"hxcpp_JIT\\";
  CreateDirectoryW(d.c_str(), nullptr);
  return d;
}

bool download(const wchar_t* url, const std::wstring& dest, double span) {
  DeleteFileW(dest.c_str());
  g_wSpan = span;
  reportOverall(g_wDone);
  DlProgress* cb = new DlProgress();
  HRESULT hr = URLDownloadToFileW(nullptr, url, dest.c_str(), 0, cb);
  cb->Release();
  g_wDone += span;
  reportOverall(g_wDone);
  return hr == S_OK;
}

std::wstring mmss(DWORD seconds) {
  std::wstring s = std::to_wstring(seconds % 60);
  if (s.size() < 2) s = L"0" + s;
  return std::to_wstring(seconds / 60) + L":" + s;
}

int runSilent(std::wstring cmd, const std::wstring& status) {
  STARTUPINFOW si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
  PROCESS_INFORMATION pi{};
  if (!CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    return -1;
  DWORD start = GetTickCount();
  postStatus(status);
  while (WaitForSingleObject(pi.hProcess, 1000) == WAIT_TIMEOUT)
    postStatus(status + L" (" + mmss((GetTickCount() - start) / 1000) + L")");
  DWORD code = 1;
  GetExitCodeProcess(pi.hProcess, &code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return (int)code;
}

std::wstring newestVsLog() {
  wchar_t buf[MAX_PATH + 1];
  DWORD n = GetTempPathW(MAX_PATH, buf);
  std::wstring dir(buf, n), best;
  WIN32_FIND_DATAW fd;
  HANDLE h = FindFirstFileW((dir + L"dd_*.log").c_str(), &fd);
  if (h == INVALID_HANDLE_VALUE) return L"";
  FILETIME bestTime{};
  do {
    if (best.empty() || CompareFileTime(&fd.ftLastWriteTime, &bestTime) > 0) {
      best = dir + fd.cFileName;
      bestTime = fd.ftLastWriteTime;
    }
  } while (FindNextFileW(h, &fd));
  FindClose(h);
  return best;
}

bool installBuildTools(bool& reboot, std::wstring& error) {
  postStatus(T(S_DOWNLOADING, kToolsLabel));
  std::wstring exe = tempDir() + L"vs_BuildTools.exe";
  if (!download(kBuildToolsUrl, exe, kToolsDownloadWeight)) {
    error = T(S_DL_FAILED, kToolsLabel);
    return false;
  }
  post(WM_APP_PROGRESS, PROG_MARQUEE);
  std::wstring cmd = L"\"" + exe + L"\" --add " + kComponentMsvc + L" --add " + kComponentSdk +
                     L" --quiet --wait --norestart --nocache";
  int code = runSilent(cmd, T(S_INSTALLING_TOOLS));
  DeleteFileW(exe.c_str());
  g_wDone += kToolsInstallWeight;
  reportOverall(g_wDone);
  if (code == 3010) reboot = true;
  if (code == 0 || code == 3010) return true;
  error = T(S_FAILED, kToolsLabel, std::to_wstring(code));
  std::wstring log = newestVsLog();
  if (!log.empty()) error += T(S_LOG_HINT, log);
  return false;
}

void runPreview() {
  g_wTotal = kToolsDownloadWeight + kToolsInstallWeight;
  g_wDone = 0;
  g_wSpan = 0;
  postStatus(T(S_PREPARING));
  Sleep(1000);
  postStatus(T(S_DOWNLOADING, kToolsLabel));
  for (int i = 0; i <= 10; ++i) {
    reportOverall(kToolsDownloadWeight * i / 10.0);
    Sleep(200);
  }
  g_wDone = kToolsDownloadWeight;
  post(WM_APP_PROGRESS, PROG_MARQUEE);
  const std::wstring status = T(S_INSTALLING_TOOLS);
  for (DWORD sec = 0; sec <= 4; ++sec) {
    postStatus(status + L" (" + mmss(sec) + L")");
    Sleep(1000);
  }
  bool ok = g_preview == 1;
  postStatus(ok ? T(S_DONE) : T(S_FAILED, kToolsLabel, L"1603") + T(S_LOG_HINT, L"C:\\Users\\you\\AppData\\Local\\Temp\\dd_setup_example.log"));
  post(WM_APP_DONE, ok ? 1 : 0, 0);
}

DWORD WINAPI installThread(LPVOID) {
  if (g_preview) {
    runPreview();
    return 0;
  }
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  bool reboot = false;
  std::wstring error;

  postStatus(T(S_PREPARING));
  bool haveTools = toolsInstalled();

  g_wDone = 0;
  g_wSpan = 0;
  g_wTotal = haveTools ? 1 : kToolsDownloadWeight + kToolsInstallWeight;
  reportOverall(0);

  if (!haveTools) installBuildTools(reboot, error);
  CoUninitialize();

  bool ok = error.empty();
  if (ok && !allInstalled()) {
    ok = false;
    error = T(S_STILL_MISSING);
  }
  postStatus(ok ? T(reboot ? S_REBOOT : S_DONE) : error);
  post(WM_APP_DONE, ok ? 1 : 0, reboot ? 1 : 0);
  return 0;
}

void setProgress(bool marquee, int pct) {
  LONG style = GetWindowLongW(g_prog, GWL_STYLE);
  if (marquee && !g_marquee) {
    SetWindowLongW(g_prog, GWL_STYLE, style | PBS_MARQUEE);
    SendMessageW(g_prog, PBM_SETMARQUEE, TRUE, 30);
    g_marquee = true;
  } else if (!marquee) {
    if (g_marquee) {
      SendMessageW(g_prog, PBM_SETMARQUEE, FALSE, 0);
      SetWindowLongW(g_prog, GWL_STYLE, style & ~PBS_MARQUEE);
      g_marquee = false;
    }
    SendMessageW(g_prog, PBM_SETPOS, pct, 0);
  }
}

int textHeight(HWND hwnd, const std::wstring& text, int width, HFONT font) {
  HDC dc = GetDC(hwnd);
  HGDIOBJ old = SelectObject(dc, font);
  RECT r{0, 0, width, 0};
  DrawTextW(dc, text.c_str(), -1, &r, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
  SelectObject(dc, old);
  ReleaseDC(hwnd, dc);
  return r.bottom - r.top;
}

int textWidth(HWND hwnd, const std::wstring& text, HFONT font) {
  HDC dc = GetDC(hwnd);
  HGDIOBJ old = SelectObject(dc, font);
  RECT r{0, 0, 0, 0};
  DrawTextW(dc, text.c_str(), -1, &r, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
  SelectObject(dc, old);
  ReleaseDC(hwnd, dc);
  return r.right - r.left;
}

const DWORD kWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;

void layout(HWND hwnd) {
  const int X = 20, W = 420, CLIENT_W = 460;
  auto mk = [&](const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id = 0) {
    return CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h, hwnd, (HMENU)(INT_PTR)id,
                           GetModuleHandleW(nullptr), nullptr);
  };
  int y = 14;
  HWND title = mk(L"STATIC", T(S_TITLE).c_str(), 0, X, y, W, 26);
  y += 34;

  std::wstring body = T(S_BODY, g_app);
  int bodyH = textHeight(hwnd, body, W, g_font) + 4;
  mk(L"STATIC", body.c_str(), SS_NOPREFIX, X, y, W, bodyH);
  y += bodyH + 6;

  std::wstring l1 = T(S_LINK_LICENSE), l2 = T(S_LINK_SDK_LICENSE), l3 = T(S_LINK_SOURCE);
  int lh = textHeight(hwnd, l1, W, g_fontLink) + 2;
  const int GAP = 16;
  int w1 = textWidth(hwnd, l1, g_fontLink) + 4, w2 = textWidth(hwnd, l2, g_fontLink) + 4,
      w3 = textWidth(hwnd, l3, g_fontLink) + 4;
  int lx = X;
  g_linkLicense = mk(L"STATIC", l1.c_str(), SS_NOTIFY | SS_NOPREFIX, lx, y, w1, lh, IDC_LICENSE);
  lx += w1 + GAP;
  if (lx + w2 > X + W) { lx = X; y += lh + 2; }
  g_linkSdk = mk(L"STATIC", l2.c_str(), SS_NOTIFY | SS_NOPREFIX, lx, y, w2, lh, IDC_SDK_LICENSE);
  lx += w2 + GAP;
  if (lx + w3 > X + W) { lx = X; y += lh + 2; }
  g_linkSource = mk(L"STATIC", l3.c_str(), SS_NOTIFY | SS_NOPREFIX, lx, y, w3, lh, IDC_SOURCE);
  y += lh + 14;

  g_prog = mk(PROGRESS_CLASSW, L"", PBS_SMOOTH, X, y, W, 16);
  SendMessageW(g_prog, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  y += 24;

  int statusH = textHeight(hwnd, L"Ag", W, g_font) * 3 + 4;
  g_status = mk(L"STATIC", T(S_PREPARING).c_str(), SS_NOPREFIX, X, y, W, statusH);
  y += statusH + 8;

  g_btnClose = mk(L"BUTTON", T(S_BTN_CLOSE).c_str(), BS_DEFPUSHBUTTON | WS_TABSTOP, X + W - 90, y, 90, 28, IDC_CLOSE);
  ShowWindow(g_btnClose, SW_HIDE);
  y += 28 + 14;

  std::wstring footer = T(S_FOOTER);
  int footerH = textHeight(hwnd, footer, W, g_font) + 4;
  mk(L"STATIC", footer.c_str(), SS_NOPREFIX, X, y, W, footerH);
  y += footerH + 14;

  EnumChildWindows(hwnd, [](HWND c, LPARAM) -> BOOL {
    SendMessageW(c, WM_SETFONT, (WPARAM)g_font, TRUE);
    return TRUE;
  }, 0);
  SendMessageW(title, WM_SETFONT, (WPARAM)g_fontTitle, TRUE);
  SendMessageW(g_linkLicense, WM_SETFONT, (WPARAM)g_fontLink, TRUE);
  SendMessageW(g_linkSdk, WM_SETFONT, (WPARAM)g_fontLink, TRUE);
  SendMessageW(g_linkSource, WM_SETFONT, (WPARAM)g_fontLink, TRUE);

  RECT r{0, 0, CLIENT_W, y};
  AdjustWindowRect(&r, kWindowStyle, FALSE);
  int w = r.right - r.left, h = r.bottom - r.top;
  RECT work{};
  SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
  int px = work.left + ((work.right - work.left) - w) / 2, py = work.top + ((work.bottom - work.top) - h) / 2;
  if (py < work.top) py = work.top;
  SetWindowPos(hwnd, nullptr, px, py, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
  case WM_CREATE:
    g_hwnd = hwnd;
    layout(hwnd);
    return 0;

  case WM_CTLCOLORSTATIC: {
    HDC dc = (HDC)w;
    HWND c = (HWND)l;
    SetBkMode(dc, TRANSPARENT);
    if (c == g_linkLicense || c == g_linkSdk || c == g_linkSource) SetTextColor(dc, RGB(0, 102, 204));
    return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
  }

  case WM_COMMAND:
    switch (LOWORD(w)) {
    case IDC_LICENSE: ShellExecuteW(hwnd, L"open", kLicenseUrl, nullptr, nullptr, SW_SHOWNORMAL); break;
    case IDC_SDK_LICENSE: ShellExecuteW(hwnd, L"open", kSdkLicenseUrl, nullptr, nullptr, SW_SHOWNORMAL); break;
    case IDC_SOURCE: ShellExecuteW(hwnd, L"open", kSourceUrl, nullptr, nullptr, SW_SHOWNORMAL); break;
    case IDC_CLOSE: if (!g_busy) DestroyWindow(hwnd); break;
    }
    return 0;

  case WM_SETCURSOR:
    if ((HWND)w == g_linkLicense || (HWND)w == g_linkSdk || (HWND)w == g_linkSource) {
      SetCursor(LoadCursorW(nullptr, IDC_HAND));
      return TRUE;
    }
    break;

  case WM_APP_STATUS: {
    std::unique_ptr<std::wstring> s((std::wstring*)l);
    SetWindowTextW(g_status, s->c_str());
    return 0;
  }
  case WM_APP_PROGRESS:
    if (w == PROG_MARQUEE) setProgress(true, 0); else setProgress(false, (int)w);
    return 0;

  case WM_APP_DONE:
    g_busy = false;
    setProgress(false, w ? 100 : 0);
    if (w && !l) {
      g_exit = 0;
      SetTimer(hwnd, TIMER_AUTOCLOSE, 1500, nullptr);
    } else {
      g_exit = w ? 3 : 2;
      ShowWindow(g_btnClose, SW_SHOW);
    }
    return 0;

  case WM_TIMER:
    if (w == TIMER_AUTOCLOSE) { KillTimer(hwnd, TIMER_AUTOCLOSE); DestroyWindow(hwnd); }
    return 0;

  case WM_CLOSE:
    if (g_busy) {
      MessageBoxW(hwnd, T(S_WAIT_STEP).c_str(), kTitle, MB_OK | MB_ICONINFORMATION);
      return 0;
    }
    DestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    PostQuitMessage(g_exit);
    return 0;
  }
  return DefWindowProcW(hwnd, msg, w, l);
}

int runInstallerWindow(HINSTANCE inst) {
  INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES};
  InitCommonControlsEx(&icc);

  NONCLIENTMETRICSW ncm{};
  ncm.cbSize = sizeof(ncm);
  SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
  g_font = CreateFontIndirectW(&ncm.lfMessageFont);
  LOGFONTW big = ncm.lfMessageFont;
  big.lfHeight = big.lfHeight * 14 / 10;
  big.lfWeight = FW_SEMIBOLD;
  g_fontTitle = CreateFontIndirectW(&big);
  LOGFONTW ul = ncm.lfMessageFont;
  ul.lfUnderline = TRUE;
  g_fontLink = CreateFontIndirectW(&ul);

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = wndProc;
  wc.hInstance = inst;
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = L"hxcppJitSetup";
  RegisterClassExW(&wc);

  HWND hwnd = CreateWindowExW(0, wc.lpszClassName, kTitle, kWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, 460, 300,
                              nullptr, nullptr, inst, nullptr);
  if (!hwnd) return 2;
  ShowWindow(hwnd, SW_SHOW);
  SetForegroundWindow(hwnd);
  CloseHandle(CreateThread(nullptr, 0, installThread, nullptr, 0, nullptr));

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    if (!IsDialogMessageW(hwnd, &msg)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }
  return (int)msg.wParam;
}

int relaunchElevated() {
  wchar_t self[MAX_PATH];
  GetModuleFileNameW(nullptr, self, MAX_PATH);
  SHELLEXECUTEINFOW sei{};
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;
  sei.lpVerb = L"runas";
  sei.lpFile = self;
  std::wstring params = L"--elevated --app \"" + g_app + L"\"";
  sei.lpParameters = params.c_str();
  sei.nShow = SW_SHOWNORMAL;
  if (!ShellExecuteExW(&sei)) return 1;
  WaitForSingleObject(sei.hProcess, INFINITE);
  DWORD code = 2;
  GetExitCodeProcess(sei.hProcess, &code);
  CloseHandle(sei.hProcess);
  return (int)code;
}

void printToParentConsole(const std::wstring& text) {
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written = 0;
  if (out && out != INVALID_HANDLE_VALUE) WriteConsoleW(out, text.c_str(), (DWORD)text.size(), &written, nullptr);
  FreeConsole();
}

}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argv) return 2;

  if (hasArg(argc, argv, L"--version")) {
    printToParentConsole(std::wstring(L"hxcpp_jit_setup ") + kVersion + L"\r\n");
    return 0;
  }
  if (hasArg(argc, argv, L"--check")) return allInstalled() ? 0 : 10;
  if (hasArg(argc, argv, L"--quiet-unless-missing")) return allInstalled() ? 0 : 1;

  for (int i = 1; i + 1 < argc; ++i)
    if (lstrcmpiW(argv[i], L"--app") == 0 && argv[i + 1][0]) g_app = argv[i + 1];
  if (g_app.empty()) g_app = T(S_THIS_PROGRAM);

  if (hasArg(argc, argv, L"--preview-fail")) g_preview = 2;
  else if (hasArg(argc, argv, L"--preview")) g_preview = 1;
  if (g_preview) return runInstallerWindow(inst);

  if (allInstalled()) return 0;
  if (!isElevated() && !hasArg(argc, argv, L"--elevated")) return relaunchElevated();
  return runInstallerWindow(inst);
}
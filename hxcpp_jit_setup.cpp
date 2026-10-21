#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <urlmon.h>
#include <tlhelp32.h>

#include <string>
#include <memory>
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
const wchar_t* kSourceUrl = L"https://github.com/ThomasDarkson/hxcpp_JIT"; 
const wchar_t* kSetupSourceUrl = L"https://github.com/ThomasDarkson/hxcpp_JIT_setup";
const wchar_t* kLicenseUrl = L"https://visualstudio.microsoft.com/license-terms/";
const wchar_t* kLicenseRedistUrl = L"https://visualstudio.microsoft.com/license-terms/vs2026-ga-visualcpp-v14-redist-runtime/";
const wchar_t* kLicenseCommunityUrl = L"https://visualstudio.microsoft.com/license-terms/vs2022-ga-community/";
const wchar_t* kLicenseBuildToolsUrl = L"https://visualstudio.microsoft.com/license-terms/vs2022-ga-diagnosticbuildtools/";
const wchar_t* kLicenseSdkUrl = L"https://learn.microsoft.com/en-us/legal/windows-sdk/license";
const wchar_t* kVsCommunityUrl = L"https://aka.ms/vs/17/release/vs_community.exe";
const wchar_t* kBuildToolsUrl = L"https://aka.ms/vs/17/release/vs_BuildTools.exe";

enum Item {
  ITEM_VC2005, ITEM_VC2008, ITEM_VC2010, ITEM_VC2012, ITEM_VC2013, ITEM_VC14,
  ITEM_MSVC, ITEM_SDK, ITEM_ATL,
  ITEM_COUNT
};

const wchar_t* kItemNames[ITEM_COUNT] = {
    L"Visual C++ 2005 Redistributable (x86 + x64)",
    L"Visual C++ 2008 Redistributable (x86 + x64)",
    L"Visual C++ 2010 Redistributable (x86 + x64)",
    L"Visual C++ 2012 Redistributable (x86 + x64)",
    L"Visual C++ 2013 Redistributable (x86 + x64)",
    L"Visual C++ v14 Redistributable (2015-2026, x86 + x64)",
    L"MSVC v143 C++ build tools (x64)",
    L"Windows 11 SDK (10.0.22621.0)",
    L"C++ ATL for v143 build tools",
};
const wchar_t* kComponentIds[ITEM_COUNT] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    L"Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    L"Microsoft.VisualStudio.Component.Windows11SDK.22621",
    L"Microsoft.VisualStudio.Component.VC.ATL",
};
const wchar_t* kYears[ITEM_VC14 + 1] = {L"2005", L"2008", L"2010", L"2012", L"2013", nullptr};

struct Pkg {
  int item;
  bool x64;
  const wchar_t* label;
  const wchar_t* url;
  const wchar_t* args;
  const wchar_t* file;
};

const Pkg kPkgs[] = {
    {ITEM_VC2005, true, L"Visual C++ 2005 Redistributable (x64)",
     L"https://download.microsoft.com/download/8/B/4/8B42259F-5D70-43F4-AC2E-4B208FD8D66A/vcredist_x64.EXE",
     L"/Q", L"vcredist2005_x64.exe"},
    {ITEM_VC2005, false, L"Visual C++ 2005 Redistributable (x86)",
     L"https://download.microsoft.com/download/8/B/4/8B42259F-5D70-43F4-AC2E-4B208FD8D66A/vcredist_x86.EXE",
     L"/Q", L"vcredist2005_x86.exe"},

    {ITEM_VC2008, true, L"Visual C++ 2008 Redistributable (x64)",
     L"https://download.microsoft.com/download/5/D/8/5D8C65CB-C849-4025-8E95-C3966CAFD8AE/vcredist_x64.exe",
     L"/q /norestart", L"vcredist2008_x64.exe"},
    {ITEM_VC2008, false, L"Visual C++ 2008 Redistributable (x86)",
     L"https://download.microsoft.com/download/5/D/8/5D8C65CB-C849-4025-8E95-C3966CAFD8AE/vcredist_x86.exe",
     L"/q /norestart", L"vcredist2008_x86.exe"},

    {ITEM_VC2010, true, L"Visual C++ 2010 Redistributable (x64)",
     L"https://download.microsoft.com/download/E/E/0/EE05C9EF-A661-4D9E-BCE2-6961ECDF087F/vcredist_x64.exe",
     L"/q /norestart", L"vcredist2010_x64.exe"},
    {ITEM_VC2010, false, L"Visual C++ 2010 Redistributable (x86)",
     L"https://download.microsoft.com/download/E/E/0/EE05C9EF-A661-4D9E-BCE2-6961ECDF087F/vcredist_x86.exe",
     L"/q /norestart", L"vcredist2010_x86.exe"},

    {ITEM_VC2012, true, L"Visual C++ 2012 Redistributable (x64)",
     L"https://download.microsoft.com/download/1/6/B/16B06F60-3B20-4FF2-B699-5E9B7962F9AE/VSU_4/vcredist_x64.exe",
     L"/install /quiet /norestart", L"vcredist2012_x64.exe"},
    {ITEM_VC2012, false, L"Visual C++ 2012 Redistributable (x86)",
     L"https://download.microsoft.com/download/1/6/B/16B06F60-3B20-4FF2-B699-5E9B7962F9AE/VSU_4/vcredist_x86.exe",
     L"/install /quiet /norestart", L"vcredist2012_x86.exe"},

    {ITEM_VC2013, true, L"Visual C++ 2013 Redistributable (x64)", L"https://aka.ms/highdpimfc2013x64enu",
     L"/install /quiet /norestart", L"vcredist2013_x64.exe"},
    {ITEM_VC2013, false, L"Visual C++ 2013 Redistributable (x86)", L"https://aka.ms/highdpimfc2013x86enu",
     L"/install /quiet /norestart", L"vcredist2013_x86.exe"},

    {ITEM_VC14, true, L"Visual C++ v14 Redistributable (x64)", L"https://aka.ms/vc14/vc_redist.x64.exe",
     L"/install /quiet /norestart", L"vc_redist14_x64.exe"},
    {ITEM_VC14, false, L"Visual C++ v14 Redistributable (x86)", L"https://aka.ms/vc14/vc_redist.x86.exe",
     L"/install /quiet /norestart", L"vc_redist14_x86.exe"},
};

enum ItemState { ST_CHECKING, ST_OK, ST_MISSING, ST_INSTALLING };

enum {
  WM_APP_CHECKED = WM_APP + 1,
  WM_APP_LOG,
  WM_APP_STATUS,
  WM_APP_PROGRESS,
  WM_APP_ITEMSTATE,
  WM_APP_INSTALL_DONE,
  WM_APP_ELEVATED_DONE,
};
const WPARAM PROG_MARQUEE = 1000;

enum { IDC_INSTALL = 101, IDC_RECHECK, IDC_CLOSE, IDC_LICENSE, IDC_SOURCE, IDC_SOURCE_SETUP };
const UINT_PTR TIMER_AUTOCLOSE = 1;

enum Str {
  S_HEADLINE, S_BODY, S_LINK_JIT, S_LINK_SETUP, S_LICENSE_TEXT,
  S_BTN_LICENSE, S_BTN_RECHECK, S_BTN_INSTALL, S_BTN_ACCEPT, S_BTN_CANCEL, S_BTN_CLOSE,
  S_ST_OK, S_ST_MISSING, S_ST_INSTALLING, S_ST_CHECKING,
  S_CHECKING_PC, S_WAIT_ADMIN, S_STARTING, S_REBOOT, S_ALL_GOOD, S_STILL_MISSING, S_N_MISSING,
  S_INSTALL_INCOMPLETE, S_NO_ADMIN_LOG, S_NO_ADMIN_STATUS, S_WAIT_STEP, S_THIS_PROGRAM,
  S_DOWNLOADING, S_DL_LOG, S_DL_FAILED, S_DL_FAILED_GENERIC, S_INSTALLING_PKG, S_INSTALLER_EXIT,
  S_VS_INSTALLING, S_VS_EXISTING, S_VS_TRYING, S_VS_DL_INSTALLER, S_VS_DL_INSTALLER22,
  S_CANCELLED, S_MS_LOG, S_MANUAL, S_CLOSED, S_LICENSE_OPEN_FAILED, S_LIC_TITLE, S_LIC_INTRO,
  S_LIC_REDIST, S_LIC_COMMUNITY, S_LIC_COMMUNITY_NOTE, S_LIC_BUILDTOOLS, S_LIC_SDK, S_LIC_DECLINED,
  S_COUNT
};

const wchar_t* kStrings[S_COUNT] = {
    // S_HEADLINE
    L"Quick setup needed (nothing is wrong!)",
    // S_BODY
    L"Do not be alarmed. Your PC is simply missing some components that {0} needs. {0} will not run without them, because it uses hxcpp_JIT to compile C++ code while it is running.\nClick Install, review Microsoft's license terms, and this setup will take care of everything.\nWhen the installation is done, you will not see this window again.",
    // S_LINK_JIT
    L"View the hxcpp_JIT source code here.",
    // S_LINK_SETUP
    L"View this setup program's source code here.",
    // S_LICENSE_TEXT
    L"Install first shows the license terms of the components. After you accept them, the components are downloaded from Microsoft and installed silently in the background.",
    // S_BTN_LICENSE
    L"License terms",
    // S_BTN_RECHECK
    L"Check again",
    // S_BTN_INSTALL
    L"Install",
    // S_BTN_ACCEPT
    L"Accept and install",
    // S_BTN_CANCEL
    L"Cancel",
    // S_BTN_CLOSE
    L"Close",
    // S_ST_OK
    L"Installed",
    // S_ST_MISSING
    L"Missing",
    // S_ST_INSTALLING
    L"Installing...",
    // S_ST_CHECKING
    L"Checking...",
    // S_CHECKING_PC
    L"Checking your computer...",
    // S_WAIT_ADMIN
    L"Waiting for administrator permission...",
    // S_STARTING
    L"Starting installation...",
    // S_REBOOT
    L"Almost done! Windows needs to restart once before this works. Please restart your PC.",
    // S_ALL_GOOD
    L"All good! Everything is installed. You can close this window.",
    // S_STILL_MISSING
    L"Hmm, something is still missing. Check the log below, then try again.",
    // S_N_MISSING
    L"{0} component(s) missing. Click \"Install\" and we will set them up.",
    // S_INSTALL_INCOMPLETE
    L"Installation did not complete.",
    // S_NO_ADMIN_LOG
    L"Administrator permission was not granted, nothing was installed.",
    // S_NO_ADMIN_STATUS
    L"Windows needs administrator permission to install these components. Click \"Install\" to try again.",
    // S_WAIT_STEP
    L"Please wait until the current step has finished.",
    // S_THIS_PROGRAM
    L"This program",
    // S_DOWNLOADING
    L"Downloading {0}...",
    // S_DL_LOG
    L"Downloading {0}",
    // S_DL_FAILED
    L"Download failed: {0}. Please check your internet connection.",
    // S_DL_FAILED_GENERIC
    L"Download failed. Please check your internet connection.",
    // S_INSTALLING_PKG
    L"Installing {0}...",
    // S_INSTALLER_EXIT
    L"{0}: installer finished with exit code {1}",
    // S_VS_INSTALLING
    L"Installing Visual Studio C++ components... This can take 10+ minutes.",
    // S_VS_EXISTING
    L"Visual Studio 2022 is already installed, adding the missing components to it.",
    // S_VS_TRYING
    L"Trying the matching Visual Studio bootstrapper instead.",
    // S_VS_DL_INSTALLER
    L"Downloading the Visual Studio installer...",
    // S_VS_DL_INSTALLER22
    L"Downloading the Visual Studio 2022 installer...",
    // S_CANCELLED
    L"The installation was cancelled.",
    // S_MS_LOG
    L"Microsoft's installer log: {0}",
    // S_MANUAL
    L"You can also install these by hand: open \"Visual Studio Installer\", choose Modify on your Visual Studio 2022, and tick \"MSVC v143 - VS 2022 C++ x64/x86 build tools\", \"Windows 11 SDK (10.0.22621.0)\" and \"C++ ATL for latest v143 build tools (x86 & x64)\".",
    // S_CLOSED
    L"Closed {0}",
    // S_LICENSE_OPEN_FAILED
    L"Could not open Microsoft's license terms. Check your default browser and try again.",
    // S_LIC_TITLE
    L"License terms",
    // S_LIC_INTRO
    L"Open each of the license terms below. A few seconds after you have opened all of them, you can accept and install.\nBy clicking Accept and install you accept all of these terms.",
    // S_LIC_REDIST
    L"Visual C++ Redistributables",
    // S_LIC_COMMUNITY
    L"Visual Studio 2022 Community",
    // S_LIC_COMMUNITY_NOTE
    L"Visual Studio Community has eligibility conditions for larger organizations. Check that you qualify.",
    // S_LIC_BUILDTOOLS
    L"Visual Studio 2022 C++ build tools and ATL",
    // S_LIC_SDK
    L"Windows 11 SDK",
    // S_LIC_DECLINED
    L"The license terms were not accepted. Nothing was installed.",
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

int runProcess(std::wstring cmd, std::string* out, bool hidden) {
  STARTUPINFOW si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = hidden ? SW_HIDE : SW_SHOWNORMAL;

  HANDLE rd = nullptr, wr = nullptr, nul = nullptr;
  if (out) {
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    if (!CreatePipe(&rd, &wr, &sa, 0)) return -1;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = nul;
    si.hStdOutput = wr;
    si.hStdError = wr;
  }
  PROCESS_INFORMATION pi{};
  BOOL ok = CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, out ? TRUE : FALSE, hidden ? CREATE_NO_WINDOW : 0,
                           nullptr, nullptr, &si, &pi);
  if (wr) CloseHandle(wr);
  if (nul) CloseHandle(nul);
  if (!ok) {
    if (rd) CloseHandle(rd);
    return -1;
  }
  if (out) {
    char buf[4096];
    DWORD n = 0;
    while (ReadFile(rd, buf, sizeof(buf), &n, nullptr) && n) out->append(buf, n);
    CloseHandle(rd);
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 1;
  GetExitCodeProcess(pi.hProcess, &code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return (int)code;
}

bool readDword(HKEY key, const wchar_t* name, DWORD& v) {
  DWORD sz = sizeof(v), type = 0;
  return RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<LPBYTE>(&v), &sz) == ERROR_SUCCESS &&
         type == REG_DWORD;
}

std::vector<std::wstring> installedProgramNames() {
  std::vector<std::wstring> names;
  const REGSAM views[2] = {KEY_WOW64_64KEY, KEY_WOW64_32KEY};
  for (REGSAM view : views) {
    HKEY root = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0,
                      KEY_READ | view, &root) != ERROR_SUCCESS)
      continue;
    for (DWORD i = 0;; ++i) {
      wchar_t sub[256];
      DWORD len = 256;
      LONG r = RegEnumKeyExW(root, i, sub, &len, nullptr, nullptr, nullptr, nullptr);
      if (r == ERROR_NO_MORE_ITEMS) break;
      if (r != ERROR_SUCCESS) continue;
      HKEY k = nullptr;
      if (RegOpenKeyExW(root, sub, 0, KEY_READ | view, &k) != ERROR_SUCCESS) continue;
      wchar_t name[512];
      DWORD sz = sizeof(name) - sizeof(wchar_t), type = 0;
      if (RegQueryValueExW(k, L"DisplayName", nullptr, &type, reinterpret_cast<LPBYTE>(name), &sz) == ERROR_SUCCESS &&
          type == REG_SZ) {
        name[sz / sizeof(wchar_t)] = L'\0';
        names.push_back(name);
      }
      RegCloseKey(k);
    }
    RegCloseKey(root);
  }
  return names;
}

bool legacyRedistInstalled(const std::vector<std::wstring>& names, const wchar_t* year, bool x64) {
  std::wstring needle = std::wstring(L"Visual C++ ") + year;
  for (const std::wstring& n : names) {
    if (n.find(needle) == std::wstring::npos || n.find(L"Redistributable") == std::wstring::npos) continue;
    bool is64 = n.find(L"x64") != std::wstring::npos;
    if (is64 == x64) return true;
  }
  return false;
}

bool vc14Installed(bool x64) {
  const wchar_t* path = x64 ? L"SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64"
                            : L"SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x86";
  bool keyOk = false;
  const REGSAM views[2] = {KEY_WOW64_64KEY, KEY_WOW64_32KEY};
  for (REGSAM view : views) {
    HKEY k = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_READ | view, &k) != ERROR_SUCCESS) continue;
    DWORD installed = 0, major = 0, minor = 0;
    bool ok = readDword(k, L"Installed", installed) && readDword(k, L"Major", major) && readDword(k, L"Minor", minor);
    RegCloseKey(k);
    if (ok && installed == 1 && major == 14 && minor >= 30) {
      keyOk = true;
      break;
    }
  }
  if (!keyOk) return false;
  std::wstring root = getEnv(L"SystemRoot");
  if (x64) {
    std::wstring sys = root + L"\\System32\\";
    return fileExists(sys + L"vcruntime140.dll") && fileExists(sys + L"vcruntime140_1.dll") &&
           fileExists(sys + L"msvcp140.dll");
  }
  std::wstring sys = root + L"\\SysWOW64\\";
  return fileExists(sys + L"vcruntime140.dll") && fileExists(sys + L"msvcp140.dll");
}

bool pkgInstalled(const Pkg& p, const std::vector<std::wstring>& names) {
  if (p.item == ITEM_VC14) return vc14Installed(p.x64);
  return legacyRedistInstalled(names, kYears[p.item], p.x64);
}

std::wstring installerDir() { return getEnv(L"ProgramFiles(x86)") + L"\\Microsoft Visual Studio\\Installer"; }

std::wstring vsQuery(const std::vector<std::wstring>& required, const wchar_t* property) {
  std::wstring vswhere = installerDir() + L"\\vswhere.exe";
  if (!fileExists(vswhere)) return L"";
  std::wstring cmd = L"\"" + vswhere + L"\" -latest -products * -version [17.0,18.0) -nologo -utf8";
  if (!required.empty()) {
    cmd += L" -requires";
    for (auto& c : required) cmd += L" " + c;
  }
  cmd += std::wstring(L" -format value -property ") + property;
  std::string out;
  if (runProcess(cmd, &out, true) != 0) return L"";
  return trim(utf8ToWide(out));
}

std::wstring findVsInstance(const std::vector<std::wstring>& required) { return vsQuery(required, L"installationPath"); }

struct Status {
  bool ok[ITEM_COUNT] = {};
  bool all() const {
    for (bool b : ok)
      if (!b) return false;
    return true;
  }
  int missing() const {
    int n = 0;
    for (bool b : ok) n += b ? 0 : 1;
    return n;
  }
};

Status checkAll() {
  Status s;
  std::vector<std::wstring> names = installedProgramNames();
  for (int i = 0; i <= ITEM_VC14; ++i) s.ok[i] = true;
  for (const Pkg& p : kPkgs)
    if (!pkgInstalled(p, names)) s.ok[p.item] = false;
  for (int i = ITEM_MSVC; i < ITEM_COUNT; ++i) s.ok[i] = !findVsInstance({kComponentIds[i]}).empty();
  if (s.ok[ITEM_MSVC] && s.ok[ITEM_SDK] && s.ok[ITEM_ATL] &&
      findVsInstance({kComponentIds[ITEM_MSVC], kComponentIds[ITEM_SDK], kComponentIds[ITEM_ATL]}).empty())
    s.ok[ITEM_MSVC] = s.ok[ITEM_SDK] = s.ok[ITEM_ATL] = false;
  return s;
}

HWND g_link, g_linkSetup;
HWND g_hwnd, g_name[ITEM_COUNT], g_stat[ITEM_COUNT], g_prog, g_statusLine, g_log, g_btnInstall, g_btnRecheck, g_btnClose;
HFONT g_font, g_fontTitle, g_fontLink;
ItemState g_state[ITEM_COUNT];
Status g_st;
bool g_busy = false, g_marquee = false, g_visible = false, g_attempted = false, g_reboot = false;
std::wstring g_app;
bool g_quietUnlessMissing = false;
bool g_auto = false, g_installNow = false, g_startedInstallNow = false;
bool g_licenseAccepted = false;
int g_exit = 1;

double g_wTotal = 1, g_wDone = 0, g_wSpan = 0;
const double kVsDownloadWeight = 1, kVsInstallWeight = 12;

void post(UINT msg, WPARAM w = 0, LPARAM l = 0) { PostMessageW(g_hwnd, msg, w, l); }
void postText(UINT msg, const std::wstring& s) { PostMessageW(g_hwnd, msg, 0, (LPARAM) new std::wstring(s)); }
void postLog(const std::wstring& s) { postText(WM_APP_LOG, s); }
void postStatus(const std::wstring& s) { postText(WM_APP_STATUS, s); }

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

bool installPackage(const Pkg& pk, int n, int count, bool& reboot) {
  std::wstring step = L"[" + std::to_wstring(n) + L"/" + std::to_wstring(count) + L"] ";
  post(WM_APP_ITEMSTATE, pk.item, ST_INSTALLING);

  postStatus(step + T(S_DOWNLOADING, pk.label));
  postLog(T(S_DL_LOG, pk.url));
  std::wstring exe = tempDir() + pk.file;
  if (!download(pk.url, exe, 1.0)) {
    postLog(T(S_DL_FAILED, pk.label));
    g_wDone += 1.0;
    reportOverall(g_wDone);
    return false;
  }

  int code = runSilent(L"\"" + exe + L"\" " + pk.args, step + T(S_INSTALLING_PKG, pk.label));
  postLog(T(S_INSTALLER_EXIT, pk.label, std::to_wstring(code)));
  g_wDone += 1.0;
  reportOverall(g_wDone);
  DeleteFileW(exe.c_str());
  if (code == 3010 || code == 1641) reboot = true;
  return code == 0 || code == 3010 || code == 1641 || code == 1638;
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

bool vsCodeOk(int code) { return code == 0 || code == 3010; }

int runVsInstaller(const std::wstring& exe, const std::wstring& args, bool& reboot, const std::wstring& status) {
  std::wstring cmd = L"\"" + exe + L"\" " + args;
  postLog(L"Running: " + cmd);
  int code = runSilent(cmd, status);
  postLog(L"  -> exit code " + std::to_wstring(code));
  if (code == 3010) reboot = true;
  return code;
}

void logManualSteps() {
  std::wstring log = newestVsLog();
  if (!log.empty()) postLog(T(S_MS_LOG, log));
  postLog(T(S_MANUAL));
}

const wchar_t* bootstrapperUrlFor(const std::wstring& productId) {
  if (productId.find(L"Community") != std::wstring::npos) return kVsCommunityUrl;
  if (productId.find(L"Professional") != std::wstring::npos) return L"https://aka.ms/vs/17/release/vs_professional.exe";
  if (productId.find(L"Enterprise") != std::wstring::npos) return L"https://aka.ms/vs/17/release/vs_enterprise.exe";
  if (productId.find(L"BuildTools") != std::wstring::npos) return kBuildToolsUrl;
  return kVsCommunityUrl;
}

int closeVsInstaller() {
  static const wchar_t* kNames[] = {L"vs_installer.exe", L"vs_installershell.exe", L"vs_community.exe",
                                    L"vs_professional.exe", L"vs_enterprise.exe", L"vs_buildtools.exe",
                                    L"vs_bootstrapper.exe"};
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return 0;
  int closed = 0;
  PROCESSENTRY32W pe{};
  pe.dwSize = sizeof(pe);
  for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
    if (pe.th32ProcessID == GetCurrentProcessId()) continue;
    for (const wchar_t* n : kNames) {
      if (lstrcmpiW(pe.szExeFile, n) != 0) continue;
      HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
      if (h) {
        if (TerminateProcess(h, 1)) {
          ++closed;
          postLog(T(S_CLOSED, pe.szExeFile));
        }
        CloseHandle(h);
      }
      break;
    }
  }
  CloseHandle(snap);
  return closed;
}

bool installVsComponentsImpl(const Status& st, bool& reboot) {
  std::wstring addArgs;
  for (int i = ITEM_MSVC; i < ITEM_COUNT; ++i) addArgs += std::wstring(L" --add ") + kComponentIds[i];
  for (int i = ITEM_MSVC; i < ITEM_COUNT; ++i)
    if (!st.ok[i]) post(WM_APP_ITEMSTATE, i, ST_INSTALLING);

  const std::wstring installingText = T(S_VS_INSTALLING);
  int code = -1;
  std::wstring existing = findVsInstance({});

  if (!existing.empty()) {
    postLog(T(S_VS_EXISTING));
    std::wstring modifyArgs = L"modify --installPath \"" + existing + L"\"" + addArgs + L" --quiet --norestart";
    post(WM_APP_PROGRESS, PROG_MARQUEE);
    for (const std::wstring& exe : {installerDir() + L"\\vs_installer.exe", installerDir() + L"\\setup.exe"}) {
      if (!fileExists(exe)) continue;
      code = runVsInstaller(exe, modifyArgs, reboot, installingText);
      if (vsCodeOk(code) || code == 1602) break;
    }
    if (!vsCodeOk(code) && code != 1602) {
      std::wstring product = vsQuery({}, L"productId");
      const wchar_t* url = bootstrapperUrlFor(product);
      postLog(T(S_VS_TRYING));
      postStatus(T(S_VS_DL_INSTALLER));
      std::wstring boot = tempDir() + L"vs_bootstrapper.exe";
      if (download(url, boot, kVsDownloadWeight)) {
        post(WM_APP_PROGRESS, PROG_MARQUEE);
        code = runVsInstaller(boot, modifyArgs + L" --wait", reboot, installingText);
      } else {
        postLog(T(S_DL_FAILED_GENERIC));
      }
    }
    if (code == 1602) postLog(T(S_CANCELLED));
    if (!vsCodeOk(code)) logManualSteps();
    return vsCodeOk(code);
  }

  postStatus(T(S_VS_DL_INSTALLER22));
  postLog(T(S_DL_LOG, kVsCommunityUrl));
  std::wstring exe = tempDir() + L"vs_community.exe";
  if (!download(kVsCommunityUrl, exe, kVsDownloadWeight)) {
    postLog(T(S_DL_FAILED_GENERIC));
    return false;
  }
  post(WM_APP_PROGRESS, PROG_MARQUEE);
  code = runVsInstaller(exe, addArgs + L" --quiet --wait --norestart", reboot, installingText);
  if (code == 1602) postLog(T(S_CANCELLED));
  if (!vsCodeOk(code)) logManualSteps();
  return vsCodeOk(code);
}

bool installVsComponents(const Status& st, bool& reboot) {
  closeVsInstaller();
  bool ok = installVsComponentsImpl(st, reboot);
  closeVsInstaller();
  return ok;
}

struct InstallArgs {
  Status st;
};

DWORD WINAPI installThread(LPVOID p) {
  std::unique_ptr<InstallArgs> a(static_cast<InstallArgs*>(p));
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  bool reboot = false, ok = true;

  std::vector<std::wstring> names = installedProgramNames();
  std::vector<const Pkg*> todo;
  for (const Pkg& pk : kPkgs)
    if (!pkgInstalled(pk, names)) todo.push_back(&pk);
  bool needVs = !(a->st.ok[ITEM_MSVC] && a->st.ok[ITEM_SDK] && a->st.ok[ITEM_ATL]);

  g_wDone = 0;
  g_wSpan = 0;
  g_wTotal = (double)todo.size() * 2.0 + (needVs ? kVsDownloadWeight + kVsInstallWeight : 0);
  if (g_wTotal <= 0) g_wTotal = 1;
  reportOverall(0);

  int n = 0;
  for (const Pkg* pk : todo)
    if (!installPackage(*pk, ++n, (int)todo.size(), reboot)) ok = false;

  if (needVs) {
    if (!installVsComponents(a->st, reboot)) ok = false;
    g_wDone += kVsInstallWeight;
  }
  CoUninitialize();
  post(WM_APP_INSTALL_DONE, ok ? 1 : 0, reboot ? 1 : 0);
  return 0;
}

DWORD WINAPI checkThread(LPVOID) {
  Status s = checkAll();
  WPARAM mask = 0;
  for (int i = 0; i < ITEM_COUNT; ++i)
    if (s.ok[i]) mask |= (WPARAM)1 << i;
  post(WM_APP_CHECKED, mask);
  return 0;
}

DWORD WINAPI elevateThread(LPVOID) {
  wchar_t self[MAX_PATH];
  GetModuleFileNameW(nullptr, self, MAX_PATH);
  SHELLEXECUTEINFOW sei{};
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;
  sei.lpVerb = L"runas";
  sei.lpFile = self;
  std::wstring params = L"--install-now --license-accepted --app \"" + g_app + L"\"";
  sei.lpParameters = params.c_str();
  sei.nShow = SW_SHOWNORMAL;
  if (!ShellExecuteExW(&sei)) {
    post(WM_APP_ELEVATED_DONE, (WPARAM)-1);
    return 0;
  }
  WaitForSingleObject(sei.hProcess, INFINITE);
  DWORD code = 2;
  GetExitCodeProcess(sei.hProcess, &code);
  CloseHandle(sei.hProcess);
  post(WM_APP_ELEVATED_DONE, (WPARAM)code);
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

void setItemState(int i, ItemState s) {
  g_state[i] = s;
  std::wstring t = T(s == ST_OK ? S_ST_OK : s == ST_MISSING ? S_ST_MISSING : s == ST_INSTALLING ? S_ST_INSTALLING : S_ST_CHECKING);
  SetWindowTextW(g_stat[i], t.c_str());
  InvalidateRect(g_stat[i], nullptr, TRUE);
}

void appendLog(const std::wstring& s) {
  std::wstring line = s + L"\r\n";
  SendMessageW(g_log, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
  SendMessageW(g_log, EM_SCROLLCARET, 0, 0);
}

void updateButtons() {
  EnableWindow(g_btnInstall, !g_busy && g_st.missing() > 0);
  EnableWindow(g_btnRecheck, !g_busy);
  EnableWindow(g_btnClose, !g_busy);
}

void startCheck() {
  g_busy = true;
  for (int i = 0; i < ITEM_COUNT; ++i) setItemState(i, ST_CHECKING);
  SetWindowTextW(g_statusLine, T(S_CHECKING_PC).c_str());
  setProgress(true, 0);
  updateButtons();
  CloseHandle(CreateThread(nullptr, 0, checkThread, nullptr, 0, nullptr));
}

struct LicenseEntry {
  std::wstring title;
  std::wstring note;
  const wchar_t* url;
  bool visited;
  HWND link;
};

enum { IDC_LIC_ACCEPT = 201, IDC_LIC_CANCEL, IDC_LIC_LINK0 = 210 };
const UINT_PTR TIMER_LICENSE = 2;
const int kLicenseDelaySeconds = 5;
int g_licCountdown = 0;

std::vector<LicenseEntry> g_licEntries;
HWND g_licWnd = nullptr, g_licAcceptBtn = nullptr;
bool g_licResult = false, g_licDone = false;

bool allLicensesVisited() {
  for (const LicenseEntry& e : g_licEntries)
    if (!e.visited) return false;
  return true;
}

void startLicenseCountdown() {
  g_licCountdown = kLicenseDelaySeconds;
  std::wstring label = T(S_BTN_ACCEPT) + L" (" + std::to_wstring(g_licCountdown) + L")";
  SetWindowTextW(g_licAcceptBtn, label.c_str());
  SetTimer(g_licWnd, TIMER_LICENSE, 1000, nullptr);
}

LRESULT CALLBACK licenseProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
  case WM_CTLCOLORSTATIC: {
    HDC dc = (HDC)w;
    HWND c = (HWND)l;
    SetBkMode(dc, TRANSPARENT);
    for (const LicenseEntry& e : g_licEntries)
      if (c == e.link) {
        SetTextColor(dc, e.visited ? RGB(85, 26, 139) : RGB(0, 102, 204));
        break;
      }
    return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
  }

  case WM_COMMAND: {
    int id = LOWORD(w);
    if (id >= IDC_LIC_LINK0 && id < IDC_LIC_LINK0 + (int)g_licEntries.size()) {
      LicenseEntry& e = g_licEntries[id - IDC_LIC_LINK0];
      HINSTANCE r = ShellExecuteW(hwnd, L"open", e.url, nullptr, nullptr, SW_SHOWNORMAL);
      if ((INT_PTR)r <= 32) {
        MessageBoxW(hwnd, T(S_LICENSE_OPEN_FAILED).c_str(), kTitle, MB_OK | MB_ICONWARNING);
      } else {
        bool wasAll = allLicensesVisited();
        e.visited = true;
        InvalidateRect(e.link, nullptr, TRUE);
        if (!wasAll && allLicensesVisited()) startLicenseCountdown();
      }
    } else if (id == IDC_LIC_ACCEPT) {
      if (allLicensesVisited() && IsWindowEnabled(g_licAcceptBtn)) {
        g_licResult = true;
        DestroyWindow(hwnd);
      }
    } else if (id == IDC_LIC_CANCEL) {
      DestroyWindow(hwnd);
    }
    return 0;
  }

  case WM_SETCURSOR:
    for (const LicenseEntry& e : g_licEntries)
      if ((HWND)w == e.link) {
        SetCursor(LoadCursorW(nullptr, IDC_HAND));
        return TRUE;
      }
    break;

  case WM_TIMER:
    if (w == TIMER_LICENSE) {
      if (--g_licCountdown > 0) {
        std::wstring label = T(S_BTN_ACCEPT) + L" (" + std::to_wstring(g_licCountdown) + L")";
        SetWindowTextW(g_licAcceptBtn, label.c_str());
      } else {
        KillTimer(hwnd, TIMER_LICENSE);
        SetWindowTextW(g_licAcceptBtn, T(S_BTN_ACCEPT).c_str());
        EnableWindow(g_licAcceptBtn, TRUE);
      }
    }
    return 0;

  case WM_CLOSE:
    DestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    KillTimer(hwnd, TIMER_LICENSE);
    g_licDone = true;
    return 0;
  }
  return DefWindowProcW(hwnd, msg, w, l);
}

bool showLicenseDialog(HWND parent) {
  g_licEntries.clear();
  bool needVs = !(g_st.ok[ITEM_MSVC] && g_st.ok[ITEM_SDK] && g_st.ok[ITEM_ATL]);

  if (!g_st.ok[ITEM_VC14]) g_licEntries.push_back({T(S_LIC_REDIST), L"", kLicenseRedistUrl, false, nullptr});
  if (needVs) {
    if (findVsInstance({}).empty())
      g_licEntries.push_back({T(S_LIC_COMMUNITY), T(S_LIC_COMMUNITY_NOTE), kLicenseCommunityUrl, false, nullptr});
    g_licEntries.push_back({T(S_LIC_BUILDTOOLS), L"", kLicenseBuildToolsUrl, false, nullptr});
    g_licEntries.push_back({T(S_LIC_SDK), L"", kLicenseSdkUrl, false, nullptr});
  }
  if (g_licEntries.empty()) return true;

  static bool registered = false;
  HINSTANCE inst = GetModuleHandleW(nullptr);
  if (!registered) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = licenseProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"hxcppJitLicense";
    RegisterClassExW(&wc);
    registered = true;
  }

  const int X = 20, W = 480, CLIENT_W = 520;
  const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;

  int y = 16;
  int introH = 60;
  int rows = 0;
  for (const LicenseEntry& e : g_licEntries) rows += e.note.empty() ? 26 : 26 + 36;
  int clientH = y + introH + 10 + rows + 14 + 28 + 16;

  RECT r{0, 0, CLIENT_W, clientH};
  AdjustWindowRect(&r, style, FALSE);
  int w = r.right - r.left, h = r.bottom - r.top;
  RECT work{};
  SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
  int px = work.left + ((work.right - work.left) - w) / 2, py = work.top + ((work.bottom - work.top) - h) / 2;
  if (py < work.top) py = work.top;

  g_licDone = false;
  g_licResult = false;
  g_licWnd = CreateWindowExW(0, L"hxcppJitLicense", T(S_LIC_TITLE).c_str(), style, px, py, w, h, parent, nullptr, inst, nullptr);
  if (!g_licWnd) return false;

  auto mk = [&](const wchar_t* cls, const wchar_t* text, DWORD st, int x, int yy, int ww, int hh, int id) {
    return CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | st, x, yy, ww, hh, g_licWnd, (HMENU)(INT_PTR)id, inst, nullptr);
  };

  HWND intro = mk(L"STATIC", T(S_LIC_INTRO).c_str(), SS_NOPREFIX, X, y, W, introH, 0);
  SendMessageW(intro, WM_SETFONT, (WPARAM)g_font, TRUE);
  y += introH + 10;

  for (size_t i = 0; i < g_licEntries.size(); ++i) {
    LicenseEntry& e = g_licEntries[i];
    e.link = mk(L"STATIC", e.title.c_str(), SS_NOTIFY | SS_NOPREFIX, X + 10, y, W - 10, 20, IDC_LIC_LINK0 + (int)i);
    SendMessageW(e.link, WM_SETFONT, (WPARAM)g_fontLink, TRUE);
    y += 26;
    if (!e.note.empty()) {
      HWND note = mk(L"STATIC", e.note.c_str(), SS_NOPREFIX, X + 10, y - 4, W - 10, 32, 0);
      SendMessageW(note, WM_SETFONT, (WPARAM)g_font, TRUE);
      y += 36 - 4;
    }
  }
  y += 14;

  g_licAcceptBtn = mk(L"BUTTON", T(S_BTN_ACCEPT).c_str(), BS_DEFPUSHBUTTON | WS_TABSTOP | WS_DISABLED, X + W - 100 - 160 - 10, y, 160, 28, IDC_LIC_ACCEPT);
  HWND cancel = mk(L"BUTTON", T(S_BTN_CANCEL).c_str(), BS_PUSHBUTTON | WS_TABSTOP, X + W - 100, y, 100, 28, IDC_LIC_CANCEL);
  SendMessageW(g_licAcceptBtn, WM_SETFONT, (WPARAM)g_font, TRUE);
  SendMessageW(cancel, WM_SETFONT, (WPARAM)g_font, TRUE);

  EnableWindow(parent, FALSE);
  ShowWindow(g_licWnd, SW_SHOW);
  SetForegroundWindow(g_licWnd);

  MSG msg;
  while (!g_licDone && GetMessageW(&msg, nullptr, 0, 0) > 0) {
    if (!IsDialogMessageW(g_licWnd, &msg)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }
  EnableWindow(parent, TRUE);
  SetForegroundWindow(parent);
  g_licWnd = nullptr;
  return g_licResult;
}

void startInstall() {
  if (g_busy || g_st.all()) return;
  if (!g_licenseAccepted) {
    if (!showLicenseDialog(g_hwnd)) {
      SetWindowTextW(g_statusLine, T(S_LIC_DECLINED).c_str());
      return;
    }
    g_licenseAccepted = true;
  }
  g_attempted = true;
  g_busy = true;
  updateButtons();
  if (!isElevated()) {
    SetWindowTextW(g_statusLine, T(S_WAIT_ADMIN).c_str());
    ShowWindow(g_hwnd, SW_HIDE);
    g_visible = false;
    CloseHandle(CreateThread(nullptr, 0, elevateThread, nullptr, 0, nullptr));
    return;
  }
  setProgress(false, 0);
  SetWindowTextW(g_statusLine, T(S_STARTING).c_str());
  auto* a = new InstallArgs();
  a->st = g_st;
  CloseHandle(CreateThread(nullptr, 0, installThread, a, 0, nullptr));
}

void showWindow() {
  if (g_visible) return;
  g_visible = true;
  ShowWindow(g_hwnd, SW_SHOW);
  SetForegroundWindow(g_hwnd);
}

void onChecked(WPARAM mask) {
  for (int i = 0; i < ITEM_COUNT; ++i) {
    g_st.ok[i] = (mask >> i) & 1;
    setItemState(i, g_st.ok[i] ? ST_OK : ST_MISSING);
  }
  g_busy = false;
  setProgress(false, 0);

  if (g_st.all()) {
    g_exit = g_reboot ? 3 : 0;
    if (g_auto && !g_attempted && !g_visible) {
      DestroyWindow(g_hwnd);
      return;
    }
    showWindow();
    if (g_reboot) {
      SetWindowTextW(g_statusLine, T(S_REBOOT).c_str());
    } else {
      SetWindowTextW(g_statusLine, T(S_ALL_GOOD).c_str());
      if (g_auto || g_installNow) SetTimer(g_hwnd, TIMER_AUTOCLOSE, 1500, nullptr);
    }
  } else {
    g_exit = g_attempted ? 2 : 1;
    showWindow();
    std::wstring m = g_attempted ? T(S_STILL_MISSING) : T(S_N_MISSING, std::to_wstring(g_st.missing()));
    SetWindowTextW(g_statusLine, m.c_str());
  }
  updateButtons();
  if (g_installNow && !g_startedInstallNow && !g_st.all()) {
    g_startedInstallNow = true;
    startInstall();
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

void layout(HWND hwnd) {
  const int X = 20, W = 500, CLIENT_W = 540;
  auto mk = [&](const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id = 0, DWORD ex = 0) {
    return CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h, hwnd, (HMENU)(INT_PTR)id,
                           GetModuleHandleW(nullptr), nullptr);
  };
  int y = 14;

  HWND title = mk(L"STATIC", T(S_HEADLINE).c_str(), 0, X, y, W, 28);
  y += 36;

  std::wstring body = T(S_BODY, g_app);
  int bodyH = textHeight(hwnd, body, W, g_font) + 4;
  mk(L"STATIC", body.c_str(), SS_NOPREFIX, X, y, W, bodyH);
  y += bodyH + 6;

  std::wstring linkText = T(S_LINK_JIT);
  int linkH = textHeight(hwnd, linkText, W, g_fontLink) + 4;
  g_link = mk(L"STATIC", linkText.c_str(), SS_NOTIFY | SS_NOPREFIX, X, y, W, linkH, IDC_SOURCE);
  y += linkH + 2;

  std::wstring linkText2 = T(S_LINK_SETUP);
  int linkH2 = textHeight(hwnd, linkText2, W, g_fontLink) + 4;
  g_linkSetup = mk(L"STATIC", linkText2.c_str(), SS_NOTIFY | SS_NOPREFIX, X, y, W, linkH2, IDC_SOURCE_SETUP);
  y += linkH2 + 14;

  for (int i = 0; i < ITEM_COUNT; ++i) {
    g_name[i] = mk(L"STATIC", kItemNames[i], SS_NOPREFIX, X + 10, y, 350, 20);
    g_stat[i] = mk(L"STATIC", T(S_ST_CHECKING).c_str(), SS_RIGHT, 390, y, 130, 20);
    y += 23;
  }
  y += 8;

  std::wstring lic = T(S_LICENSE_TEXT);
  int licH = textHeight(hwnd, lic, 340, g_font) + 4;
  mk(L"STATIC", lic.c_str(), 0, X, y, 340, licH);
  mk(L"BUTTON", T(S_BTN_LICENSE).c_str(), BS_PUSHBUTTON | WS_TABSTOP, 370, y, 150, 26, IDC_LICENSE);
  y += (licH > 30 ? licH : 30) + 10;

  g_prog = mk(PROGRESS_CLASSW, L"", PBS_SMOOTH, X, y, W, 16);
  SendMessageW(g_prog, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  y += 24;

  int lineH = textHeight(hwnd, L"Ag", W, g_font);
  int statusH = lineH * 2 + 4;
  g_statusLine = mk(L"STATIC", T(S_CHECKING_PC).c_str(), SS_NOPREFIX, X, y, W, statusH);
  y += statusH + 8;

  int logH = 110;
  RECT work{};
  SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
  RECT frame{0, 0, CLIENT_W, 0};
  AdjustWindowRect(&frame, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
  int chrome = (frame.bottom - frame.top);
  int maxClient = (work.bottom - work.top) - chrome - 20;
  int needed = y + logH + 14 + 28 + 14;
  if (needed > maxClient) {
    logH -= needed - maxClient;
    if (logH < 40) logH = 40;
  }
  g_log = mk(L"EDIT", L"", ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, X, y, W, logH, 0, WS_EX_CLIENTEDGE);
  y += logH + 14;

  g_btnRecheck = mk(L"BUTTON", T(S_BTN_RECHECK).c_str(), BS_PUSHBUTTON | WS_TABSTOP, X, y, 140, 28, IDC_RECHECK);
  g_btnInstall = mk(L"BUTTON", T(S_BTN_INSTALL).c_str(), BS_DEFPUSHBUTTON | WS_TABSTOP, 330, y, 100, 28, IDC_INSTALL);
  g_btnClose = mk(L"BUTTON", T(S_BTN_CLOSE).c_str(), BS_PUSHBUTTON | WS_TABSTOP, 440, y, 80, 28, IDC_CLOSE);
  y += 28 + 14;

  EnumChildWindows(hwnd, [](HWND c, LPARAM) -> BOOL {
    SendMessageW(c, WM_SETFONT, (WPARAM)g_font, TRUE);
    return TRUE;
  }, 0);
  SendMessageW(title, WM_SETFONT, (WPARAM)g_fontTitle, TRUE);
  SendMessageW(g_link, WM_SETFONT, (WPARAM)g_fontLink, TRUE);
  SendMessageW(g_linkSetup, WM_SETFONT, (WPARAM)g_fontLink, TRUE);

  RECT r{0, 0, CLIENT_W, y};
  AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
  int w = r.right - r.left, h = r.bottom - r.top;
  int px = work.left + ((work.right - work.left) - w) / 2, py = work.top + ((work.bottom - work.top) - h) / 2;
  if (py < work.top) py = work.top;
  SetWindowPos(hwnd, nullptr, px, py, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
  case WM_CREATE:
    g_hwnd = hwnd;
    layout(hwnd);
    updateButtons();
    return 0;

  case WM_CTLCOLORSTATIC: {
    HDC dc = (HDC)w;
    HWND c = (HWND)l;
    if (c == g_log) {
      SetBkMode(dc, OPAQUE);
      SetBkColor(dc, GetSysColor(COLOR_WINDOW));
      SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
      return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    }
    SetBkMode(dc, TRANSPARENT);
    if (c == g_link || c == g_linkSetup) {
      SetTextColor(dc, RGB(0, 102, 204));
      return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    }
    for (int i = 0; i < ITEM_COUNT; ++i)
      if (c == g_stat[i]) {
        SetTextColor(dc, g_state[i] == ST_OK ? RGB(0, 128, 0) : g_state[i] == ST_MISSING ? RGB(200, 0, 0) : RGB(90, 90, 90));
        break;
      }
    return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
  }

  case WM_COMMAND:
    switch (LOWORD(w)) {
    case IDC_INSTALL: if (!g_busy) startInstall(); break;
    case IDC_RECHECK: if (!g_busy) startCheck(); break;
    case IDC_CLOSE: if (!g_busy) DestroyWindow(hwnd); break;
    case IDC_SOURCE: ShellExecuteW(hwnd, L"open", kSourceUrl, nullptr, nullptr, SW_SHOWNORMAL); break;
    case IDC_SOURCE_SETUP: ShellExecuteW(hwnd, L"open", kSetupSourceUrl, nullptr, nullptr, SW_SHOWNORMAL); break;
    case IDC_LICENSE: ShellExecuteW(hwnd, L"open", kLicenseUrl, nullptr, nullptr, SW_SHOWNORMAL); break;
    }
    return 0;

  case WM_SETCURSOR:
    if ((HWND)w == g_link || (HWND)w == g_linkSetup) {
      SetCursor(LoadCursorW(nullptr, IDC_HAND));
      return TRUE;
    }
    break;

  case WM_APP_CHECKED: onChecked(w); return 0;
  case WM_APP_LOG: { std::unique_ptr<std::wstring> s((std::wstring*)l); appendLog(*s); return 0; }
  case WM_APP_STATUS: { std::unique_ptr<std::wstring> s((std::wstring*)l); SetWindowTextW(g_statusLine, s->c_str()); return 0; }
  case WM_APP_PROGRESS:
    if (w == PROG_MARQUEE) setProgress(true, 0); else setProgress(false, (int)w);
    return 0;
  case WM_APP_ITEMSTATE: setItemState((int)w, (ItemState)l); return 0;

  case WM_APP_INSTALL_DONE:
    g_reboot = g_reboot || l != 0;
    setProgress(false, 100);
    if (!w) appendLog(T(S_INSTALL_INCOMPLETE));
    startCheck();
    return 0;

  case WM_APP_ELEVATED_DONE:
    if ((int)w == -1) {
      g_busy = false;
      showWindow();
      appendLog(T(S_NO_ADMIN_LOG));
      SetWindowTextW(g_statusLine, T(S_NO_ADMIN_STATUS).c_str());
      updateButtons();
    } else {
      g_exit = (int)w;
      DestroyWindow(hwnd);
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

void printToParentConsole(const std::wstring& text) {
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written = 0;
  if (out && out != INVALID_HANDLE_VALUE) WriteConsoleW(out, text.c_str(), (DWORD)text.size(), &written, nullptr);
  FreeConsole();
}

bool hasArg(int argc, wchar_t** argv, const wchar_t* name) {
  for (int i = 1; i < argc; ++i)
    if (lstrcmpiW(argv[i], name) == 0) return true;
  return false;
}

}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (hasArg(argc, argv, L"--version")) {
    printToParentConsole(std::wstring(L"hxcpp_jit_setup ") + kVersion + L"\r\n");
    return 0;
  }
  if (hasArg(argc, argv, L"--check")) return checkAll().all() ? 0 : 10;
  g_quietUnlessMissing = hasArg(argc, argv, L"--quiet-unless-missing");
  g_auto = g_quietUnlessMissing || hasArg(argc, argv, L"--auto");
  g_installNow = hasArg(argc, argv, L"--install-now");
  g_licenseAccepted = g_installNow && isElevated() && hasArg(argc, argv, L"--license-accepted");
  for (int i = 1; i + 1 < argc; ++i)
    if (lstrcmpiW(argv[i], L"--app") == 0 && argv[i + 1][0]) g_app = argv[i + 1];
  if (g_app.empty()) g_app = T(S_THIS_PROGRAM);

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

  const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
  RECT r{0, 0, 540, 464};
  AdjustWindowRect(&r, style, FALSE);
  int w = r.right - r.left, h = r.bottom - r.top;
  int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2, y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

  HWND hwnd = CreateWindowExW(0, wc.lpszClassName, kTitle, style, x, y, w, h, nullptr, nullptr, inst, nullptr);
  if (!hwnd) return 2;
  appendLog(std::wstring(L"hxcpp_JIT setup v") + kVersion);
  if (!g_auto) showWindow();
  startCheck();

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    if (!IsDialogMessageW(hwnd, &msg)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }
  int exitCode = (int)msg.wParam;
  if (g_quietUnlessMissing) exitCode = (exitCode == 0 || exitCode == 3) ? 0 : 1;
  return exitCode;
}
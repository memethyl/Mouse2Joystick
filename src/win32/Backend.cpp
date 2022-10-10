#define M2JWIN32DLL_EXPORTS
#include "Backend.hpp"

namespace m2j {

namespace win32 {

#pragma data_seg(".shared")
__declspec(allocate(".shared")) static MouseCtrl mouseCtrl = MouseCtrl{};
#pragma data_seg()
#pragma comment(linker, "/section:.shared,rws")

static inline std::string GetProcessNameFromHandle(HWND hWnd) {
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hWnd, &lpdwProcessId);
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, false, lpdwProcessId);
    LPSTR lpFilename = new CHAR[65535];
    GetModuleFileNameEx((HMODULE)hProc, NULL, lpFilename, 65535);
    CloseHandle(hProc);
    std::string filename{lpFilename};
    delete[] lpFilename;
    return filename;
}

static inline void Internal_ShowCursor() {
    while (ShowCursor(true) < 0) {}
}

static inline void Internal_HideCursor() {
    // have to call this to hide the cursor properly in fullscreen windows,
    // otherwise it briefly shows the cursor when the mouse stops moving
    SetCursor(NULL);
    while (ShowCursor(false) >= 0) {}
}

LRESULT CALLBACK Backend::MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if ((nCode < 0)) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    MSLLHOOKSTRUCT* param = (MSLLHOOKSTRUCT *)lParam;
    // mouse buttons
    switch (wParam & 0xFFFF) {
    case WM_MOUSEMOVE: {
        break;
    }
    case WM_LBUTTONDOWN: {
        mouse_l = true;
        goto backend_mouseproc_end;
    }
    case WM_LBUTTONUP: {
        mouse_l = false;
        goto backend_mouseproc_end;
    }
    case WM_MBUTTONDOWN: {
        mouse_m = true;
        goto backend_mouseproc_end;
    }
    case WM_MBUTTONUP: {
        mouse_m = false;
        goto backend_mouseproc_end;
    }
    case WM_RBUTTONDOWN: {
        mouse_r = true;
        goto backend_mouseproc_end;
    }
    case WM_RBUTTONUP: {
        mouse_r = false;
        goto backend_mouseproc_end;
    }
    case WM_XBUTTONDOWN: {
        (((wParam >> 16) == 0x0001) ? mouse_4 : mouse_5) = true;
        goto backend_mouseproc_end;
    }
    case WM_XBUTTONUP: {
        (((wParam >> 16) == 0x0001) ? mouse_4 : mouse_5) = false;
        goto backend_mouseproc_end;
    }
    default: {
        break;
    }
    }
    // mouse movement
    delta.x = param->pt.x - prev.x;
    deltasum.x += delta.x;
    prev.x = param->pt.x;
    delta.y = param->pt.y - prev.y;
    deltasum.y += delta.y;
    prev.y = param->pt.y;
    if (this->Locked()) {
        int x = -(delta.x - param->pt.x);
        int y = -(delta.y - param->pt.y);
        prev.x = x;
        prev.y = y;
        return 1;
    }
backend_mouseproc_end:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Backend::EventSystemMoveSizeEnd(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                     LONG idObject, LONG idChild, DWORD idEventThread,
                                     DWORD dwmsEventTime) {
    if (window_active) {
        GetWindowRect(hwnd, &window_rect);
    }
}

void Backend::EventSystemForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                    LONG idObject, LONG idChild, DWORD idEventThread,
                                    DWORD dwmsEventTime) {
    switch (event) {
    case EVENT_SYSTEM_FOREGROUND:
    case EVENT_SYSTEM_SWITCHEND:
    case EVENT_SYSTEM_MINIMIZEEND: {
        std::string processName = GetProcessNameFromHandle(hwnd);
        window_active = processName.ends_with(targetProcName);
        UnhookWindowsHookEx(getMessageHook);
        getMessageHook = NULL;
        if (window_active) {
            dwThreadId = GetWindowThreadProcessId(hwnd, NULL);
            GetWindowRect(hwnd, &window_rect);
            getMessageHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, dll, dwThreadId);
        }
        if (this->Locked() && lock_in_center) {
            int x = window_rect.left + ((window_rect.right - window_rect.left) / 2);
            int y = window_rect.top + ((window_rect.bottom - window_rect.top) / 2);
            prev.x = x;
            prev.y = y;
            SetCursorPos(x, y);
        }
        break;
    }
    default: { break; }
    }
}

void Backend::SetTargetProcName(std::string name) { targetProcName = std::move(name); }

bool Backend::GetWindowActive() { return window_active; }

bool Backend::Locked() { return window_active && lock_cursor; }

void Backend::LockCursor(bool value) { lock_cursor = value; }

void Backend::LockInCenter(bool value) { lock_in_center = value; }

extern "C" {

M2JWIN32DLL_API Backend backend = Backend{NULL};

M2JWIN32DLL_API BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        backend = Backend{hinstDLL};
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    default: {
        break;
    }
    }
    return TRUE;
}

M2JWIN32DLL_API LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    auto param = ((MSG *)lParam);
    switch (param->message & 0xFFFF) {
    case WM_MOUSEMOVE:
    case WM_SETCURSOR: {
        if (mouseCtrl.hideCursor) {
            param->message = WM_NULL;
        }
        break;
    }
    // don't block WM_MOUSEUP events or you won't be able to let go of the window
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN: {
        if (mouseCtrl.disableClicks) {
            param->message = WM_NULL;
        }
        break;
    }
    case WM_KEYDOWN: {
        if (mouseCtrl.disableClicks) {
            switch (param->wParam) {
            case VK_LBUTTON:
            case VK_MBUTTON:
            case VK_RBUTTON:
            case VK_XBUTTON1:
            case VK_XBUTTON2: {
                param->message = WM_NULL;
                break;
            }
            }
        }
        break;
    }
    }
    if (mouseCtrl.hideCursor) {
        Internal_HideCursor();
    } else {
        Internal_ShowCursor();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

M2JWIN32DLL_API void HideCursor(bool value) { mouseCtrl.hideCursor = value; }

M2JWIN32DLL_API void DisableClicks(bool value) { mouseCtrl.disableClicks = value; }

M2JWIN32DLL_API LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    return backend.MouseProc(nCode, wParam, lParam);
}

M2JWIN32DLL_API void EventSystemForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                           LONG idObject, LONG idChild, DWORD idEventThread,
                                           DWORD dwmsEventTime) {
    return backend.EventSystemForeground(hWinEventHook, event, hwnd, idObject, idChild,
                                         idEventThread, dwmsEventTime);
}

M2JWIN32DLL_API void EventSystemMoveSizeEnd(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                            LONG idObject, LONG idChild, DWORD idEventThread,
                                            DWORD dwmsEventTime) {
    return backend.EventSystemMoveSizeEnd(hWinEventHook, event, hwnd, idObject, idChild,
                                          idEventThread, dwmsEventTime);
}

} // extern "C"

} // namespace win32

} // namespace m2j
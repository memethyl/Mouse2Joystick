#include "Frontend.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

// https://stackoverflow.com/a/4609795
template <typename T>
int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

namespace m2j {

namespace win32 {

Frontend::Frontend() {
    dll = LoadLibrary("M2JWin32.dll");
    if (dll == NULL) {
        throw std::runtime_error{"Couldn't load M2JWin32.dll"};
    }

    client = vigem_alloc();
    if (client == nullptr) {
        FreeLibrary(dll);
        throw std::runtime_error{"Failed to allocate memory for PVIGEM_CLIENT"};
    }
    const auto retval = vigem_connect(client);
    if (!VIGEM_SUCCESS(retval)) {
        FreeLibrary(dll);
        std::stringstream string;
        string << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval;
        throw std::runtime_error{string.str()};
    }
    pad = vigem_target_x360_alloc();
    const auto pir = vigem_target_add(client, pad);
    if (!VIGEM_SUCCESS(pir)) {
        FreeLibrary(dll);
        std::stringstream string;
        string << "Target plugin failed with error code: 0x" << std::hex << pir;
        throw std::runtime_error{string.str()};
    }
    XInputGetState(0, &state);
    // initialize state to 0 so m2j doesn't send random button presses when it starts
    state.Gamepad.bLeftTrigger = 0;
    state.Gamepad.bRightTrigger = 0;
    state.Gamepad.sThumbLX = 0;
    state.Gamepad.sThumbLY = 0;
    state.Gamepad.sThumbRX = 0;
    state.Gamepad.sThumbRY = 0;
    state.Gamepad.wButtons = 0;

    hook_thread = std::thread{[&]() {
        globalMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, dll, NULL);
        // uses EVENT_SYSTEM_FOREGROUND (0x0003), EVENT_SYSTEM_SWITCHEND (0x0015),
        // EVENT_SYSTEM_MINIMIZEEND (0x0017)
        SystemForegroundHook =
            SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_MINIMIZEEND, dll,
                            EventSystemForeground, NULL, NULL, WINEVENT_OUTOFCONTEXT);
        SystemMoveSizeEndHook =
            SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MOVESIZEEND, dll,
                            EventSystemMoveSizeEnd, NULL, NULL, WINEVENT_OUTOFCONTEXT);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }};
    hook_thread.detach();
    reset_thread = std::thread{[&]() { this->Reset(); }};
    reset_thread.detach();
}

Frontend::~Frontend() {
    vigem_target_remove(client, pad);
    vigem_target_free(pad);
    vigem_disconnect(client);
    vigem_free(client);
    UnhookWindowsHookEx(globalMouseHook);
    UnhookWinEvent(SystemForegroundHook);
    UnhookWinEvent(SystemMoveSizeEndHook);
    FreeLibrary(dll);
}

void Frontend::Reset() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(input_delay_));
        // normalize mouse movement, converting it to the [0, 1) range
        double x = -std::exp((-1.0 / x_resist_) * std::abs(backend.deltasum.x)) + 1.0;
        double y = -std::exp((-1.0 / y_resist_) * std::abs(backend.deltasum.y)) + 1.0;
        // take the sign into account, expanding the range to (-1, 1)
        x *= sign(backend.deltasum.x);
        y *= -sign(backend.deltasum.y);
        // XInput joystick coordinates are signed shorts, so convert to (-32767, 32767)
        short leftX = !lock_x_axis_ * (short)(32767.0 * x);
        short rightX = leftX;
        short leftY = !lock_y_axis_ * (short)(32767.0 * y);
        short rightY = leftY;
        if ((joystick_ == Stick::Left) || (joystick_ == Stick::None)) {
            rightX = 0;
            rightY = 0;
        }
        if ((joystick_ == Stick::Right) || (joystick_ == Stick::None)) {
            leftX = 0;
            leftY = 0;
        }
        state.Gamepad.sThumbLX = leftX;
        state.Gamepad.sThumbLY = leftY;
        state.Gamepad.sThumbRX = rightX;
        state.Gamepad.sThumbRY = rightY;
        state.Gamepad.bLeftTrigger = 0;
        state.Gamepad.bRightTrigger = 0;
        state.Gamepad.wButtons = 0;
        for (const auto &[active, button] : {std::pair{backend.mouse_l, mouse.left},
                                             {backend.mouse_m, mouse.middle},
                                             {backend.mouse_r, mouse.right},
                                             {backend.mouse_4, mouse.four},
                                             {backend.mouse_5, mouse.five}}) {
            // |= is intentional, it allows two mouse buttons to be bound to the same trigger
            if (button == Button::LEFTTRIGGER) {
                state.Gamepad.bLeftTrigger |= (active ? 255 : 0);
            } else if (button == Button::RIGHTTRIGGER) {
                state.Gamepad.bRightTrigger |= (active ? 255 : 0);
            } else {
                state.Gamepad.wButtons |= (active ? button_map[button] : 0);
            }
        }
        vigem_target_x360_update(client, pad, *reinterpret_cast<XUSB_REPORT *>(&state.Gamepad));
        backend.deltasum.x = 0;
        backend.deltasum.y = 0;
    }
}

const char *Frontend::button_names[Button::MAX] = {
    "None",   "A",       "B",   "X",   "Y",   "Back", "N/A",  "Start",
    "L3",     "R3",      "L1",  "R1",  "L2",  "R2",   "D-Up", "D-Down",
    "D-Left", "D-Right", "N/A", "N/A", "N/A", "N/A",  "N/A",  "N/A"};

} // namespace win32

} // namespace m2j
#pragma once
#include <string>
#include <thread>
#include "Backend.hpp"
#include "FrontendData.hpp"
#include "ViGEm/Client.h"
#define WIN32_LEAN_AND_MEAN 1
#include "Windows.h"
#include "Xinput.h"

namespace m2j {

namespace win32 {

struct Frontend : public FrontendData {
  private:
    unsigned input_delay_ = 10;
    unsigned x_resist_ = 100;
    unsigned y_resist_ = 100;
    bool disable_clicks_ = false;
    bool lock_x_axis_ = false;
    bool lock_y_axis_ = false;
    Stick joystick_ = Stick::Right;
    bool hide_cursor_ = false;
    bool lock_cursor_ = false;
    std::string process_name_ = "";
    static constexpr unsigned short button_map[] = {0,
                                                    XINPUT_GAMEPAD_A,
                                                    XINPUT_GAMEPAD_B,
                                                    XINPUT_GAMEPAD_X,
                                                    XINPUT_GAMEPAD_Y,
                                                    XINPUT_GAMEPAD_BACK,
                                                    0,
                                                    XINPUT_GAMEPAD_START,
                                                    XINPUT_GAMEPAD_LEFT_SHOULDER,
                                                    XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                                    0,
                                                    0,
                                                    XINPUT_GAMEPAD_LEFT_THUMB,
                                                    XINPUT_GAMEPAD_RIGHT_THUMB,
                                                    XINPUT_GAMEPAD_DPAD_UP,
                                                    XINPUT_GAMEPAD_DPAD_DOWN,
                                                    XINPUT_GAMEPAD_DPAD_LEFT,
                                                    XINPUT_GAMEPAD_DPAD_RIGHT,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    0};

  public:
    unsigned input_delay() const override { return input_delay_; }
    unsigned x_resist() const override { return x_resist_; }
    unsigned y_resist() const override { return y_resist_; }
    bool disable_clicks() const override { return disable_clicks_; }
    bool lock_x_axis() const override { return lock_x_axis_; }
    bool lock_y_axis() const override { return lock_y_axis_; }
    Stick joystick() const override { return joystick_; }
    bool hide_cursor() const override { return hide_cursor_; }
    bool lock_cursor() const override { return lock_cursor_; }
    std::string process_name() const override { return process_name_; }
    void input_delay(unsigned value) override { input_delay_ = value; }
    void x_resist(unsigned value) override { x_resist_ = value; }
    void y_resist(unsigned value) override { y_resist_ = value; }
    void disable_clicks(bool value) override { DisableClicks(value); }
    void lock_x_axis(bool value) override { lock_x_axis_ = value; }
    void lock_y_axis(bool value) override { lock_y_axis_ = value; }
    void joystick(Stick value) override { joystick_ = value; }
    void hide_cursor(bool value) override {
        hide_cursor_ = value;
        HideCursor(hide_cursor_);
    }
    void lock_cursor(bool value) override {
        lock_cursor_ = value;
        backend.LockCursor(value);
    }
    void process_name(std::string value) override {
        process_name_ = std::move(value);
        backend.SetTargetProcName(process_name_);
    }

  private:
    HHOOK globalMouseHook;
    HWINEVENTHOOK SystemForegroundHook;
    HWINEVENTHOOK SystemMoveSizeEndHook;
    HMODULE dll;
    std::thread reset_thread;
    std::thread hook_thread;
    PVIGEM_CLIENT client;
    PVIGEM_TARGET pad;
    XINPUT_STATE state;

  public:
    static const char *button_names[Button::MAX];
    Frontend();
    virtual ~Frontend();
    void Reset();
};

} // namespace win32

} // namespace m2j
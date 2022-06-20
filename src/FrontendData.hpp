#pragma once
#include <string>

namespace m2j {

// this is the best way to create a scoped enum that can be implicitly converted to its underlying
// type
struct Stick {
  private:
    int value;

  public:
    Stick(int value_) : value{value_} {}
    operator int() const { return value; }
    enum : int { None = 0, Left, Right, Both, MAX };
};

struct Button {
  private:
    int value;

  public:
    Button(int value_) : value{value_} {}
    operator int() const { return value; }
    enum : int {
        INVALID = 0,
        A,
        B,
        X,
        Y,
        BACK,
        GUIDE,
        START,
        LEFTSHOULDER,
        RIGHTSHOULDER,
        LEFTTRIGGER,
        RIGHTTRIGGER,
        LEFTSTICK,
        RIGHTSTICK,
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        MISC1,
        PADDLE1,
        PADDLE2,
        PADDLE3,
        PADDLE4,
        TOUCHPAD,
        MAX
    };
};

struct FrontendData {
    static const char *button_names[Button::MAX];
    static const char *stick_names[Stick::MAX];
    struct MouseKeys {
        Button left = Button::INVALID;
        Button middle = Button::INVALID;
        Button right = Button::INVALID;
        Button four = Button::INVALID;
        Button five = Button::INVALID;
    };
    MouseKeys mouse{};
    virtual unsigned input_delay() const = 0;
    virtual unsigned x_resist() const = 0;
    virtual unsigned y_resist() const = 0;
    virtual bool disable_clicks() const = 0;
    virtual bool lock_x_axis() const = 0;
    virtual bool lock_y_axis() const = 0;
    virtual Stick joystick() const = 0;
    virtual bool hide_cursor() const = 0;
    virtual bool lock_cursor() const = 0;
    virtual std::string process_name() const = 0;
    virtual void input_delay(unsigned value) = 0;
    virtual void x_resist(unsigned value) = 0;
    virtual void y_resist(unsigned value) = 0;
    virtual void disable_clicks(bool value) = 0;
    virtual void lock_x_axis(bool value) = 0;
    virtual void lock_y_axis(bool value) = 0;
    virtual void joystick(Stick value) = 0;
    virtual void hide_cursor(bool value) = 0;
    virtual void lock_cursor(bool value) = 0;
    virtual void process_name(std::string value) = 0;
    virtual ~FrontendData() {}
};

} // namespace m2j
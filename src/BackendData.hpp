#pragma once
#include "Point.hpp"

namespace m2j {

struct BackendData {
    Point delta{0, 0};
    Point prev{0, 0};
    Point deltasum{0, 0};
    // mouse buttons
    bool mouse_l = false;
    bool mouse_m = false;
    bool mouse_r = false;
    bool mouse_4 = false;
    bool mouse_5 = false;
};

} // namespace m2j

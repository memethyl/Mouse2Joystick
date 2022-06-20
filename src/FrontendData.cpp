#include "FrontendData.hpp"

namespace m2j {

const char *FrontendData::button_names[Button::MAX] = {
    "None",   "A",       "B",     "X",        "Y",        "Back",     "Guide",    "Start",
    "L3",     "R3",      "L1",    "R1",       "L2",       "R2",       "D-Up",     "D-Down",
    "D-Left", "D-Right", "Misc1", "Paddle 1", "Paddle 2", "Paddle 3", "Paddle 4", "Touchpad"};
const char *FrontendData::stick_names[Stick::MAX] = {"None", "Left", "Right", "Both"};

} // namespace m2j
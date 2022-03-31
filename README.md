# Mouse2Joystick
This program converts mouse input to button and joystick input on a virtual controller. In addition, it can hide and lock the cursor over a specified target window. This allows the mouse to be used as input in programs that do not allow it.

This program requires ViGEmBus, which can be found here:

https://vigem.org/projects/ViGEm/How-to-Install/

## Usage
- Download the latest release
- Run Mouse2Joystick.exe
- Tweak the settings to your liking

You can now use the virtual controller in any running program. If you want to hide and lock the cursor, set the target process name to that of the desired window. The process name can be found in the "Processes" tab in Task Manager by right-clicking the column names and checking "Process name".

A setup guide for Cemu can be found in this repository's wiki.

## Building
Mouse2Joystick uses CMake, and can be built like so:
```
> git clone --recurse-submodules https://github.com/memethyl/Mouse2Joystick
> mkdir M2JBuild
> cd M2JBuild
> cmake ../Mouse2Joystick
> cmake --build .
```

## FAQ
> Which platforms are supported?

Mouse2Joystick is Windows-only at the moment. Support for other platforms is planned.

> When I click a key bind in the target window to change it, it immediately binds the key and moves to the next bind. How do I fix this?

Repeat the following for all binds:
1. Crank the input delay all the way up
2. Click the bind
3. Hold the desired button down until it binds

> Key binds in the target window keep being bound to the joystick. How do I fix this?

Enable "Lock X Axis" and "Lock Y Axis" while binding the keys. This will prevent the virtual joystick from moving.

## Libraries
- SDL (https://github.com/libsdl-org/SDL)
- GLAD (https://github.com/Dav1dde/glad)
- Nuklear (https://github.com/Immediate-Mode-UI/Nuklear)
- ViGEmClient (https://github.com/ViGEm/ViGEmClient)
- Cereal (https://github.com/USCiLab/cereal)
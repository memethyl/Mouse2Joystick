cmake_minimum_required(VERSION 3.21)

# borrowed from OpenCV source code
# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(FATAL_ERROR "M2J only supports Windows at the moment. Support for other platforms is planned.")
endif()

project(Mouse2Joystick
    LANGUAGES C CXX
    VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GLAD_API "gl=3.3")
add_subdirectory(submodules/glad)
# glad_add_library(glad_gl_core_33 STATIC LANGUAGE C API gl:core=3.3)
add_subdirectory(submodules/SDL)
include_directories(
    submodules/cereal/include
    ${GLAD_INCLUDE_DIRS}
    submodules/SDL/include
    submodules/Nuklear
    submodules
    src
)

set(ALL_LIBS
     ${OPENGL_LIBRARY}
     ${GLAD_LIBRARIES}
     SDL2main SDL2-static
)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
)

set(SOURCES src/main.cpp src/FrontendData.cpp)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # build win32 DLL
    add_subdirectory(src/win32)
    include_directories(submodules/ViGEmClient/include)
    list(APPEND ALL_LIBS M2JWin32 Xinput ViGEmClient)
    list(APPEND SOURCES src/win32/Frontend.cpp)
    # don't forget to copy M2JWin32.dll after the build succeeds
endif()

# build main.cpp
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
    # copy M2JWin32.dll
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

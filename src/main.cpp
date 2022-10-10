#include <fstream>
#include <string>
#include "CurrentFrontend.hpp"
#include "FrontendData.hpp"
#include "SDL.h"
#include "Version.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "glad/glad.h"
#include "nk_include.h"
#include "nuklear_sdl_gl3.h"

#define WINDOW_WIDTH 442
#define WINDOW_HEIGHT 390
#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

struct UIData {
    static const std::size_t PROCESS_NAME_MAX_SIZE = 256;
    const char *tooltip = "\0";
    // column 1
    struct {
        int left = 0;
        int middle = 0;
        int right = 0;
        int four = 0;
        int five = 0;

        template <class Archive>
        void serialize(Archive &archive) {
            archive(CEREAL_NVP(left), CEREAL_NVP(middle), CEREAL_NVP(right), CEREAL_NVP(four),
                    CEREAL_NVP(five));
        }
    } mouse;
    int joystick = m2j::Stick::Right;
    // column 2
    int input_delay = 10;
    int x_resist = 100;
    int y_resist = 100;
    int disable_clicks = 0;
    int lock_x_axis = 0;
    int lock_y_axis = 0;
    int hide_cursor = 0;
    int lock_cursor = 0;
    int lock_in_center = 0;
    char process_name[PROCESS_NAME_MAX_SIZE] = "Cemu.exe";

    template <class Archive>
    void save(Archive &archive) const {
        // process_name is converted to std::string because serializing raw pointers is not
        // supported in cereal
        archive(CEREAL_NVP(mouse), CEREAL_NVP(joystick), CEREAL_NVP(input_delay),
                CEREAL_NVP(x_resist), CEREAL_NVP(y_resist), CEREAL_NVP(disable_clicks),
                CEREAL_NVP(lock_x_axis), CEREAL_NVP(lock_y_axis), CEREAL_NVP(hide_cursor),
                CEREAL_NVP(lock_cursor), CEREAL_NVP(lock_in_center),
                cereal::make_nvp("process_name", std::string{process_name}));
    }

    template <class Archive>
    void load(Archive &archive) {
        std::string proc_name{};
        archive(CEREAL_NVP(mouse), CEREAL_NVP(joystick), CEREAL_NVP(input_delay),
                CEREAL_NVP(x_resist), CEREAL_NVP(y_resist), CEREAL_NVP(disable_clicks),
                CEREAL_NVP(lock_x_axis), CEREAL_NVP(lock_y_axis), CEREAL_NVP(hide_cursor),
                CEREAL_NVP(lock_cursor), CEREAL_NVP(lock_in_center),
                cereal::make_nvp("process_name", proc_name));
        std::memcpy(process_name, proc_name.data(), min(proc_name.size(), PROCESS_NAME_MAX_SIZE));
    }
};

inline void TooltipHover(nk_context *ctx, const char *&tooltip, const char *text) {
    if (nk_widget_is_hovered(ctx)) {
        tooltip = text;
    }
}

int main(int argc, char *argv[]) {
    /* Platform */
    SDL_Window *win;
    SDL_GLContext glContext;
    int win_width, win_height;

    /* GUI */
    struct nk_context *ctx;

    NK_UNUSED(argc);
    NK_UNUSED(argv);

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);
    win = SDL_CreateWindow("Mouse2Joystick " M2J_VERSION, SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI |
                               SDL_WINDOW_RESIZABLE);
    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &win_width, &win_height);

    /* OpenGL setup */
    if (gladLoadGLLoader(SDL_GL_GetProcAddress) != 1) {
        fprintf(stderr, "Failed to load GLAD");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    ctx = nk_sdl_init(win);
    /* Load Fonts */
    {
        struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        nk_sdl_font_stash_end();
    }

    m2j::CurrentFrontend frontend{};
    UIData data{};
    {
        std::ifstream config_file{"m2j_config.json"};
        if (config_file.is_open()) {
            cereal::JSONInputArchive archive{config_file};
            archive(data);
        }
    }
    while (true) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                goto cleanup;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        // can't use tooltips because they're buggy so i have to do this instead
        if (nk_begin(ctx, "Description", nk_rect(0, 0, WINDOW_WIDTH, 86),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 35, 1);
            nk_label_wrap(ctx, data.tooltip);
        }
        nk_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "Options", nk_rect(0, 86, WINDOW_WIDTH, WINDOW_HEIGHT - 86),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            data.tooltip = "\0";
            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT - 86 - 51, 2);
            if (nk_group_begin(ctx, "Key Binds", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 2);
                nk_label(ctx, "Left Mouse:", NK_TEXT_RIGHT);
                nk_combobox(ctx, frontend.button_names, NK_LEN(frontend.button_names),
                            &data.mouse.left, 20, {75, 75});
                frontend.mouse.left = data.mouse.left;
                nk_label(ctx, "Middle Mouse:", NK_TEXT_RIGHT);
                nk_combobox(ctx, frontend.button_names, NK_LEN(frontend.button_names),
                            &data.mouse.middle, 20, {75, 75});
                frontend.mouse.middle = data.mouse.middle;
                nk_label(ctx, "Right Mouse:", NK_TEXT_RIGHT);
                nk_combobox(ctx, frontend.button_names, NK_LEN(frontend.button_names),
                            &data.mouse.right, 20, {75, 75});
                frontend.mouse.right = data.mouse.right;
                nk_label(ctx, "Mouse 4:", NK_TEXT_RIGHT);
                nk_combobox(ctx, frontend.button_names, NK_LEN(frontend.button_names),
                            &data.mouse.four, 20, {75, 75});
                frontend.mouse.four = data.mouse.four;
                nk_label(ctx, "Mouse 5:", NK_TEXT_RIGHT);
                nk_combobox(ctx, frontend.button_names, NK_LEN(frontend.button_names),
                            &data.mouse.five, 20, {75, 75});
                frontend.mouse.five = data.mouse.five;
                TooltipHover(ctx, data.tooltip, "Joystick to send input to. Default is Right.");
                nk_label(ctx, "Joystick:", NK_TEXT_RIGHT);
                nk_combobox(ctx, frontend.stick_names, NK_LEN(frontend.stick_names), &data.joystick,
                            20, {77, 77});
                frontend.joystick(data.joystick);
                nk_group_end(ctx);
            }
            if (nk_group_begin(ctx, "Settings", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                TooltipHover(ctx, data.tooltip,
                             "Number of milliseconds between controller input "
                             "reports. Default is 10.");
                nk_property_int(ctx, "#Input Delay:", 1, &data.input_delay, 1000, 1, 1);
                frontend.input_delay(data.input_delay);
                TooltipHover(ctx, data.tooltip,
                             "Higher = less sensitive. Range is 1-1000, "
                             "default is 100.");
                nk_property_int(ctx, "#X Resistance:", 1, &data.x_resist, 1000, 1, 1);
                frontend.x_resist(data.x_resist);
                TooltipHover(ctx, data.tooltip,
                             "Higher = less sensitive. Range is 1-1000, "
                             "default is 100.");
                nk_property_int(ctx, "#Y Resistance:", 1, &data.y_resist, 1000, 1, 1);
                frontend.y_resist(data.y_resist);
                TooltipHover(ctx, data.tooltip,
                             "Process name of the target. Any window with this "
                             "process name will be targeted.");
                nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, data.process_name,
                                               data.PROCESS_NAME_MAX_SIZE, nk_filter_ascii);
                frontend.process_name(data.process_name);
                nk_layout_row_dynamic(ctx, 15, 1);
                TooltipHover(ctx, data.tooltip,
                             "Disable mouse clicks on the target window. This "
                             "will block all interaction with the window.");
                nk_checkbox_label(ctx, "Disable Mouse Clicks", &data.disable_clicks);
                frontend.disable_clicks(data.disable_clicks);
                TooltipHover(ctx, data.tooltip,
                             "Prevent the X axis from moving. Use this if "
                             "binding the stick is difficult.");
                nk_checkbox_label(ctx, "Lock X Axis", &data.lock_x_axis);
                frontend.lock_x_axis(data.lock_x_axis > 0);
                TooltipHover(ctx, data.tooltip,
                             "Prevent the Y axis from moving. Use this if "
                             "binding the stick is difficult.");
                nk_checkbox_label(ctx, "Lock Y Axis", &data.lock_y_axis);
                frontend.lock_y_axis(data.lock_y_axis > 0);
                TooltipHover(ctx, data.tooltip, "Hide the cursor over the target window.");
                nk_checkbox_label(ctx, "Hide Cursor", &data.hide_cursor);
                frontend.hide_cursor(data.hide_cursor > 0);
                TooltipHover(ctx, data.tooltip,
                             "Lock the cursor when the target window is focused.");
                nk_checkbox_label(ctx, "Lock Cursor", &data.lock_cursor);
                frontend.lock_cursor(data.lock_cursor > 0);
                TooltipHover(ctx, data.tooltip,
                             "If \"Lock Cursor\" is on, lock it in the center of the target "
                             "window. Does not work on some Windows installations.");
                nk_checkbox_label(ctx, "Lock In Center", &data.lock_in_center);
                frontend.lock_in_center(data.lock_in_center > 0);
                nk_group_end(ctx);
            }
        }
        nk_end(ctx);

        /* Draw */
        SDL_GetWindowSize(win, &win_width, &win_height);
        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.10f, 0.18f, 0.24f, 1.00f);
        /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state
         * after rendering the UI. */
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        SDL_GL_SwapWindow(win);
        SDL_Delay(16);
    }

cleanup:
    nk_sdl_shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    {
        std::ofstream config_file{"m2j_config.json", std::ofstream::out | std::ofstream::trunc};
        if (config_file.is_open()) {
            cereal::JSONOutputArchive archive{config_file};
            archive(CEREAL_NVP(data));
        }
    }
    return 0;
}
#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "menu.h"
#include "configs.h"

int main(int argc, char* argv[])
{
    // Initialize SDL2 Video Subsystem
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Double buffering & depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window with graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("lovemachine_css - Standalone ImGui Preview (Mac Test)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, window_flags);
    if (!window)
    {
        std::cerr << "Error: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui SDL2 & OpenGL3 backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup custom theme
    Menu::SetupStyle();
    Menu::show_menu = true; // Open menu by default in test mode

    // Main Loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode current_toggle_key = SDLK_INSERT;
                if (sets)
                {
                    switch (sets->menu.menu_key_idx)
                    {
                    case 0: current_toggle_key = SDLK_INSERT; break;
                    case 1: current_toggle_key = SDLK_DELETE; break;
                    case 2: current_toggle_key = SDLK_HOME; break;
                    case 3: current_toggle_key = SDLK_END; break;
                    case 4: current_toggle_key = SDLK_BACKQUOTE; break;
                    case 5: current_toggle_key = SDLK_F11; break;
                    case 6: current_toggle_key = SDLK_F12; break;
                    case 7: current_toggle_key = SDLK_RSHIFT; break;
                    default: current_toggle_key = SDLK_INSERT; break;
                    }
                }

                if (event.key.keysym.sym == current_toggle_key || event.key.keysym.sym == SDLK_INSERT)
                {
                    Menu::show_menu = !Menu::show_menu;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE && !Menu::show_menu)
                {
                    done = true;
                }
            }
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Render Background Preview Canvas (Simulated CS:S Game View)
        ImDrawList* bg_draw = ImGui::GetBackgroundDrawList();
        ImVec2 display_size = io.DisplaySize;

        // Dark modern background gradient
        bg_draw->AddRectFilledMultiColor(
            ImVec2(0, 0), display_size,
            IM_COL32(15, 15, 22, 255), IM_COL32(20, 18, 30, 255),
            IM_COL32(10, 10, 15, 255), IM_COL32(12, 12, 18, 255)
        );

        // Watermark Banner
        bg_draw->AddText(ImVec2(20, 20), IM_COL32(180, 120, 255, 200), "LOVEMACHINE CS:S - Standalone UI Test Preview");
        char fps_buf[128];
        snprintf(fps_buf, sizeof(fps_buf), "FPS: %.1f | Frame Time: %.2f ms | Press INSERT to toggle Menu", io.Framerate, 1000.0f / (io.Framerate > 0.0f ? io.Framerate : 1.0f));
        bg_draw->AddText(ImVec2(20, 40), IM_COL32(140, 140, 170, 180), fps_buf);

        // Render Main Menu UI
        Menu::Render();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.06f, 0.06f, 0.08f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

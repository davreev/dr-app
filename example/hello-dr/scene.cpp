#include "scene.hpp"

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

namespace dr
{
namespace
{

// clang-format off

struct {
    char const* name = "Scene Name";
    char const* author = "David Reeves";
    char const* year = "2023";
    struct {
        u16 major{0};
        u16 minor{1};
        u16 patch{0};
    } version;
} const scene_info;

struct {
    struct {
        bool space_down;
        // ...
    } input;
    // ...
} state;

// clang-format on

void draw_ui_tooltip()
{
    ImGui::BeginTooltip();
    ImGui::Text("42");
    ImGui::EndTooltip();
}

void draw_ui_about_tab()
{
    if (ImGui::BeginTabItem("About"))
    {
        ImGui::TextWrapped(
            "Press space bar to reveal the answer to the great question of life, the universe, and everything.");
        ImGui::Spacing();

        ImGui::Text(
            "Version %u.%u.%u",
            scene_info.version.major,
            scene_info.version.minor,
            scene_info.version.patch);

        ImGui::Text("Copyright %s %s", scene_info.author, scene_info.year);

        ImGui::EndTabItem();
    }
}

void draw_ui_main_window()
{
    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    constexpr int window_flags = ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin(scene_info.name, nullptr, window_flags);
    ImGui::PushItemWidth(200.0f);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        draw_ui_about_tab();
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void draw_ui()
{
    draw_ui_main_window();

    if (state.input.space_down)
        draw_ui_tooltip();
}

void draw()
{
    draw_ui();

    // ...
}

void handle_event(sapp_event const* const event)
{
    switch (event->type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            if (event->key_code == SAPP_KEYCODE_SPACE)
                state.input.space_down = true;

            break;
        }
        case SAPP_EVENTTYPE_KEY_UP:
        {
            if (event->key_code == SAPP_KEYCODE_SPACE)
                state.input.space_down = false;

            break;
        }
        default:
        {
            // ...
        }
    }
}

} // namespace

App::Scene const* scene()
{
    static App::Scene const scene{
        scene_info.name,
        nullptr,
        nullptr,
        nullptr,
        draw,
        handle_event,
    };

    return &scene;
}

} // namespace dr
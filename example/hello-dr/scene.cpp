#include "scene.hpp"

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

namespace dr
{
namespace
{

// clang-format off

struct {
    struct {
        bool space_down;
        // ...
    } input;
    // ...
} state{};

// clang-format on

void draw(void* /*context*/)
{
    ImGui::BeginTooltip();
    ImGui::Text(state.input.space_down ? "Hello!" : "Press spacebar");
    ImGui::EndTooltip();
}

void handle_event(void* /*context*/, App::Event const& event)
{
    switch (event.type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            if (event.key_code == SAPP_KEYCODE_SPACE)
                state.input.space_down = true;

            break;
        }
        case SAPP_EVENTTYPE_KEY_UP:
        {
            if (event.key_code == SAPP_KEYCODE_SPACE)
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

App::Scene scene()
{
    return {
        "Hello DR",
        nullptr,
        nullptr,
        nullptr,
        draw,
        handle_event,
        nullptr,
    };
}

} // namespace dr
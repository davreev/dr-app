#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

#include "default_scene.hpp"

namespace dr
{
namespace
{

void open()
{
    // ...
}

void update()
{
    // ...
}

void draw()
{
    ImGui::BeginTooltip();
    ImGui::Text("This is the default scene. If you're seeing this, you haven't assigned a scene.");
    ImGui::EndTooltip();
}

void close()
{
    // ...
}

void input(sapp_event const* /*event*/)
{
    // ...
}

} // namespace

Scene const* default_scene()
{
    static Scene const scene{
        "Default Scene",
        open,
        update,
        draw,
        close,
        input,
    };

    return &scene;
}

} // namespace dr
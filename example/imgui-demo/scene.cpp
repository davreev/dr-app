#include "scene.hpp"

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

#include <implot.h>

namespace dr
{
namespace
{

void draw(void* /*context*/)
{
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
}

} // namespace

App::Scene scene()
{
    App::Scene scene{};
    scene.name = "ImGui Demo";
    scene.draw = draw;
    return scene;
}

} // namespace dr
#include "scene.hpp"

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

#include <implot.h>
#include <implot3d.h>

namespace dr
{
namespace
{

void draw(void* /*context*/)
{
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
    ImPlot3D::ShowDemoWindow();
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
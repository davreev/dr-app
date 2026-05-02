#include <implot.h>
#include <implot3d.h>

#include <dr/app/app.hpp>

#include "scene.hpp"

int main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::run({
        .scene = scene(),
        .init_cb =
            []() {
                ImPlot::CreateContext();
                ImPlot3D::CreateContext();
            },
        .deinit_cb =
            []() {
                ImPlot3D::DestroyContext();
                ImPlot::DestroyContext();
            },
        .window{
            .title = "Example: ImGui Demo",
        },
        .sokol_config{
            .app = [](sapp_desc& desc) { desc.html5.canvas_selector = "#imgui-demo"; },
        },
    });

    return 0;
}

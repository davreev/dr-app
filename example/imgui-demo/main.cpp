#include <implot.h>
#include <implot3d.h>

#include <dr/app/app.hpp>

#include "scene.hpp"

dr::App::Desc DR_APP_MAIN(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::set_scene(scene());

    App::config().init.callback = [](void* /*context*/) {
        ImPlot::CreateContext();
        ImPlot3D::CreateContext();
    };

    App::config().cleanup.callback = [](void* /*context*/) {
        ImPlot::DestroyContext();
        ImPlot3D::DestroyContext();
    };

    App::Desc desc = App::desc();
    {
        desc.width = 1280;
        desc.height = 720;
        desc.window_title = "Example: ImGui Demo";
#if __EMSCRIPTEN__
        desc.html5_canvas_selector = "#imgui-demo";
#endif
    }
    return desc;
}

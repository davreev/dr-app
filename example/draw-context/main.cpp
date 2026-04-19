#include <dr/app/app.hpp>

#include "scene.hpp"

int main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::run({
        .scene = scene(),
        .sokol_config{
            .app =
                [](sapp_desc& desc) {
                    desc.window_title = "Example: Draw Context";
                    desc.html5_canvas_selector = "#draw-context";
                },
        },
    });

    return 0;
}

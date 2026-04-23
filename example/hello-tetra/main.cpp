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
                    desc.window_title = "Example: Hello Tetra";
                    desc.html5.canvas_selector = "#hello-tetra";
                },
        },
    });

    return 0;
}

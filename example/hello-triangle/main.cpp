#include <sokol_app.h>

#include "scene.hpp"

sapp_desc sokol_main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    app_set_scene(scene());

    sapp_desc desc = app_desc();
    desc.width = 1280;
    desc.height = 720;
    desc.window_title = "Hello Triangle";

    return desc;
}

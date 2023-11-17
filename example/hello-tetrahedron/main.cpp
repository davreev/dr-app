#include <sokol_app.h>

#include "scene.hpp"

sapp_desc sokol_main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::set_scene(scene());

    App::Desc desc = App::desc();
    {
        desc.width = 1280;
        desc.height = 720;
        desc.window_title = "App Name";
        // ...
    }
    return desc;
}

#pragma once

#include <dr/math_types.hpp>

#include <dr/app/app.hpp>

namespace dr
{

struct Orbit;
struct Zoom;
struct Pan;

/// True if the mouse is over the app window
bool is_mouse_over(App::Event const& event);

/// Scale factor for taking screen space displacements to view space (at z = -1).
f32 screen_to_view(f32 fov, f32 size);

/// Handles mouse events for camera control
void camera_handle_mouse_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* orbit,
    Pan* pan,
    f32 screen_to_view,
    bool mouse_down[3]);

/// Handles touch events for camera control
void camera_handle_touch_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* orbit,
    Pan* pan,
    f32 screen_to_view,
    Vec2<f32> prev_touch_points[2],
    i8& prev_num_touches);

} // namespace dr
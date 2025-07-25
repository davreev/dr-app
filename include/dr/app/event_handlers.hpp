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

/// Scale factor for taking screen space displacements to view space (at depth of 1).
f32 screen_to_view_scale(f32 fov, f32 size);
f32 screen_to_view_scale(f32 fov_y);

/// Creates an affine transform that takes points from screen space to view space (at depth of 1)
Mat3<f32> make_screen_to_view(f32 fov_y, bool flip_y = false);

/// Creates an affine transform that takes points from view space (at depth of 1) to screen space
Mat3<f32> make_view_to_screen(f32 fov_y, bool flip_y = false);

/// Handles mouse events for camera control
void camera_handle_mouse_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* orbit,
    Pan* pan,
    f32 drag_scale,
    f32 scroll_scale,
    bool mouse_down[3]);

/// Handles touch events for camera control
void camera_handle_touch_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* orbit,
    Pan* pan,
    f32 drag_scale,
    Vec2<f32> prev_touch_points[2],
    i8& prev_num_touches);

} // namespace dr
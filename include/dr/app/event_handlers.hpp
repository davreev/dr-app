#pragma once

#include <dr/math_types.hpp>

struct sapp_event;

namespace dr
{

struct Camera;
struct Orbit;
struct Zoom;
struct Pan;

/// True if the mouse is over the app window
bool is_mouse_over(sapp_event const* event);

/// Scale factor for taking input deltas from screen to view space (at z = -1).
f32 screen_to_view_scale(f32 fov, f32 size);

/// Handles mouse events for camera control
void camera_handle_mouse_event(
    Camera& camera,
    Orbit* orbit,
    Zoom* zoom,
    Pan* pan,
    bool mouse_down[3],
    f32 move_scale,
    sapp_event const& event);

/// Handles touch events for camera control
void camera_handle_touch_event(
    Camera& camera,
    Orbit* orbit,
    Zoom* zoom,
    Pan* pan,
    Vec2<f32> last_touch_points[2],
    i8& last_num_touches,
    f32 move_scale,
    sapp_event const& event);

} // namespace dr
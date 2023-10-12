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

/// Scale factor for taking screen space distances to view space (at z = -1).
f32 screen_to_view(f32 fov, f32 size);

/// Handles mouse events for camera control
void camera_handle_mouse_event(
    sapp_event const& event,
    f32 camera_distance,
    f32 screen_to_view,
    Orbit* orbit,
    Zoom* zoom,
    Pan* pan,
    bool mouse_down[3]);

/// Handles touch events for camera control
void camera_handle_touch_event(
    sapp_event const& event,
    f32 camera_distance,
    f32 screen_to_view,
    Orbit* orbit,
    Zoom* zoom,
    Pan* pan,
    Vec2<f32> last_touch_points[2],
    i8& last_num_touches);

} // namespace dr
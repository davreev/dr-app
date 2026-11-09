#pragma once

namespace dr
{

#ifdef __EMSCRIPTEN__
#define VERTEX_HEADER "#version 300 es"
#define FRAGMENT_HEADER "#version 300 es\nprecision highp float;"
#else
#define VERTEX_HEADER "#version 330 core"
#define FRAGMENT_HEADER "#version 330 core"
#endif

#define PASS_UNIFORMS_SRC                                                                          \
    R"(

uniform mat4 u_world_to_clip;
uniform vec3 u_viewport; // xy: size, z: dpi scale

)"

#define COMMON_SRC                                                                                 \
    R"(

// Assuming OpenGL NDC convention for now (z in [-1, 1], y up)
#define NDC_OPENGL

// Padding added to each quad so that antialiased edges aren't clipped
const float quad_aa_pad = 1.0;

// Returns the parameter at which mix(f0, f1, t) == f
float inv_mix(float f0, float f1, float f)
{
    return (f - f0) / (f1 - f0);
}

/*
    Narrows a segment to the half space where the signed distance d is non-negative, given d at
    each endpoint. Returns false if nothing remains.
*/
bool clip_to_half_space(float d0, float d1, inout vec4 p0, inout vec4 p1)
{
    if (d0 < 0.0 && d1 < 0.0)
        return false; // Entirely outside

    if (d0 < 0.0)
        p0 = mix(p0, p1, inv_mix(d0, d1, 0.0)); // Enters partway along
    else if (d1 < 0.0)
        p1 = mix(p0, p1, inv_mix(d0, d1, 0.0)); // Exits partway along

    return true;
}

/*
    Clips a segment in homogeneous space against near and side planes of the view frustum. Returns
    false if nothing remains.
*/
bool clip_segment(inout vec4 p0, inout vec4 p1)
{
#ifdef NDC_OPENGL
    // Clip z in [-1, 1]
    if (!clip_to_half_space(p0.z + p0.w, p1.z + p1.w, p0, p1)) // -z
        return false;
#else
    // Clip z in [0, 1]
    if (!clip_to_half_space(p0.z, p1.z, p0, p1)) // -z
        return false;
#endif

    // Pad x/y extent of frustum so that lines can overshoot side planes before expansion
    const float ndc_extent_xy = 2.0;
    float ext0 = ndc_extent_xy * p0.w;
    float ext1 = ndc_extent_xy * p1.w;

    if (!clip_to_half_space(p0.x + ext0, p1.x + ext1, p0, p1)) // -x
        return false;

    if (!clip_to_half_space(ext0 - p0.x, ext1 - p1.x, p0, p1)) // +x
        return false;

    if (!clip_to_half_space(p0.y + ext0, p1.y + ext1, p0, p1)) // -y
        return false;

    if (!clip_to_half_space(ext0 - p0.y, ext1 - p1.y, p0, p1)) // +y
        return false;

    return true;
}

float disc_dist(vec2 p, float radius)
{
    return length(p) - radius;
}

float box_dist(vec2 p, vec2 half_extent)
{
    vec2 q = abs(p) - half_extent;
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0)));
}

float ring_dist(vec2 p, float radius, float width)
{
    return abs(length(p) - (radius - 0.5 * width)) - 0.5 * width;
}

float cross_dist(vec2 p, float radius, float width)
{
    float half_w = 0.5 * width;
    return min(box_dist(p, vec2(radius, half_w)), box_dist(p, vec2(half_w, radius)));
}

/*
    Corner of a quad in [0, 1]^2 for the given vertex of a triangle strip.
*/
vec2 quad_corner(int vertex_id)
{
    return vec2(float(vertex_id >> 1), float(vertex_id & 1));
}

)"

constexpr char line_vertex_src[] = VERTEX_HEADER PASS_UNIFORMS_SRC COMMON_SRC R"( 

layout(location = 0) in vec3 a_start;
layout(location = 1) in vec3 a_end;
layout(location = 2) in vec2 a_trim;
layout(location = 3) in float a_width;
layout(location = 4) in vec4 a_color;
layout(location = 5) in uvec4 a_flags;

out vec4 v_color;
flat out vec2 v_start; // Segment start in framebuffer px
flat out vec2 v_dir; // Unit segment direction in framebuffer px
flat out vec2 v_extent; // x: segment length, y: half width (framebuffer px)
flat out int v_cap;

// Maps a clip-space point in front of the eye to screen space
vec4 clip_to_screen(vec4 p)
{
    float inv_w = 1.0 / p.w;
    return vec4(
        (p.xy * inv_w * 0.5 + 0.5) * u_viewport.xy,
        p.z * inv_w,
        inv_w
    );
}

// Distance in framebuffer pixels between two clip-space points in front of the eye
float screen_dist(vec4 a, vec4 b)
{
    return length((a.xy / a.w - b.xy / b.w) * (0.5 * u_viewport.xy));
}

// Offset in framebuffer pixels for line width, caps, and antialiasing
vec2 line_quad_offset(vec2 corner, vec2 dir, float half_width, int cap)
{
    // Square and round caps extend the quad past the segment's endpoints
    float cap_extent = (cap == 0) ? 0.0 : half_width;
    vec2 extent = vec2(cap_extent, half_width) + quad_aa_pad;
    vec2 offset = (corner * 2.0 - 1.0) * extent;
    return mat2(dir, vec2(-dir.y, dir.x)) * offset;
}

void main()
{
    // Transform and clip the segment to the frustum
    vec4[2] p_clip_raw = vec4[](
        u_world_to_clip * vec4(a_start, 1.0),
        u_world_to_clip * vec4(a_end, 1.0)
    );

    vec4[2] p_clip = p_clip_raw;
    if (!clip_segment(p_clip[0], p_clip[1]))
    {
        // Push the quad out of the clip volume (z = 2) and early out
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    vec4[2] p_screen = vec4[](
        clip_to_screen(p_clip[0]),
        clip_to_screen(p_clip[1])
    );

    // Resolve the trimmed interval in screen space
    vec2 d = p_screen[1].xy - p_screen[0].xy;
    float len = length(d);

    // Subtract the distance removed by clipping from trims. Endpoints at or behind the eye do not
    // use a trim.
    vec2 trim = a_trim * u_viewport.z;
    trim.x = p_clip_raw[0].w > 0.0 ? max(0.0, trim.x - screen_dist(p_clip_raw[0], p_clip[0])) : 0.0;
    trim.y = p_clip_raw[1].w > 0.0 ? max(0.0, trim.y - screen_dist(p_clip_raw[1], p_clip[1])) : 0.0;
    float trim_dist = trim.x + trim.y;

    // Hide fully trimmed lines. Keep untrimmed zero-length lines for their caps.
    if (trim_dist > 0.0 && trim_dist >= len)
    {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    vec2 t_range = vec2(0.0, 1.0);
    if (trim_dist > 0.0)
        t_range = vec2(trim.x, len - trim.y) / len;

    // Expand the interval into a quad
    const float screen_tol = 1e-6;
    vec2 dir = (len > screen_tol) ? d / len : vec2(1.0, 0.0);

    // Sub-pixel widths are drawn one pixel wide and faded instead of dropping out
    float w = a_width * u_viewport.z;
    float half_w = 0.5 * max(w, 1.0);
    float fade = clamp(w, 0.0, 1.0);

    int cap = int(a_flags.x);
    vec2 corner = quad_corner(gl_VertexID);
    float t = mix(t_range.x, t_range.y, corner.x);
    vec2 offset = line_quad_offset(corner, dir, half_w, cap);
    vec4 p = mix(p_screen[0], p_screen[1], t);
    p.xy += offset;

    // Write the quad position and fragment shader inputs
    vec2 ndc = p.xy / u_viewport.xy * 2.0 - 1.0;
    gl_Position = vec4(ndc, p.z, 1.0) / p.w;

    v_color = vec4(a_color.rgb, a_color.a * fade);
    v_start = mix(p_screen[0].xy, p_screen[1].xy, t_range.x);
    v_dir = dir;
    v_extent = vec2(len - trim_dist, half_w);
    v_cap = cap;
}

)";

constexpr char line_fragment_src[] = FRAGMENT_HEADER COMMON_SRC R"(

in vec4 v_color;
flat in vec2 v_start;
flat in vec2 v_dir;
flat in vec2 v_extent;
flat in int v_cap;

out vec4 f_color;

void main()
{
    /*
        Position within the segment is recovered from gl_FragCoord rather than interpolated. GLSL
        ES has no noperspective qualifier, so an interpolated distance along the segment wouldn't
        be linear in screen space.
    */
    vec2 p = gl_FragCoord.xy - v_start;
    float along = dot(p, v_dir);
    float across = dot(p, vec2(-v_dir.y, v_dir.x));

    float len = v_extent.x;
    float half_w = v_extent.y;

    float dist;
    if (v_cap == 2)
    {
        // Round
        dist = length(vec2(along - clamp(along, 0.0, len), across)) - half_w;
    }
    else
    {
        // Butt or square
        float half_len = 0.5 * len + ((v_cap == 1) ? half_w : 0.0);
        dist = box_dist(vec2(along - 0.5 * len, across), vec2(half_len, half_w));
    }

    float cov = clamp(0.5 - dist, 0.0, 1.0);

    if (cov <= 0.0)
        discard;

    f_color = vec4(v_color.rgb, v_color.a * cov);
}

)";

constexpr char point_vertex_src[] = VERTEX_HEADER PASS_UNIFORMS_SRC COMMON_SRC R"(

layout(location = 0) in vec3 a_position;
layout(location = 1) in float a_size;
layout(location = 2) in vec4 a_color;
layout(location = 3) in uvec4 a_flags;

out vec4 v_color;
flat out vec2 v_center; // Center in framebuffer px
flat out float v_radius; // Half size in framebuffer px
flat out int v_shape;

void main()
{
    vec4 cen_clip = u_world_to_clip * vec4(a_position, 1.0);
    vec2 cen_screen = (cen_clip.xy / cen_clip.w * 0.5 + 0.5) * u_viewport.xy;

    // Sub-pixel sizes are drawn one pixel wide and faded instead of dropping out
    float size = a_size * u_viewport.z;
    float radius = 0.5 * max(size, 1.0);

    vec2 offset = quad_corner(gl_VertexID) * 2.0 - 1.0;
    vec2 p_screen = cen_screen + offset * (radius + quad_aa_pad);

    vec2 p_ndc = p_screen / u_viewport.xy * 2.0 - 1.0;
    gl_Position = vec4(p_ndc * cen_clip.w, cen_clip.zw);

    float fade = clamp(size, 0.0, 1.0);

    v_color = vec4(a_color.rgb, a_color.a * fade);
    v_center = cen_screen;
    v_radius = radius;
    v_shape = int(a_flags.x);
}

)";

constexpr char point_fragment_src[] = FRAGMENT_HEADER COMMON_SRC R"(

in vec4 v_color;
flat in vec2 v_center;
flat in float v_radius;
flat in int v_shape;

out vec4 f_color;

void main()
{
    vec2 p = gl_FragCoord.xy - v_center;
    float r = v_radius;
    float w = max(1.0, 0.4 * r); // Stroke width of open shapes

    float dist;
    if (v_shape == 1)
        dist = box_dist(p, vec2(r));
    else if (v_shape == 2)
        dist = ring_dist(p, r, w);
    else if (v_shape == 3)
        dist = cross_dist(p, r, w);
    else
        dist = disc_dist(p, r);

    float cov = clamp(0.5 - dist, 0.0, 1.0);
    if (cov <= 0.0)
        discard;

    f_color = vec4(v_color.rgb, v_color.a * cov);
}

)";

#undef PASS_UNIFORMS_SRC
#undef COMMON_SRC
#undef VERTEX_HEADER
#undef FRAGMENT_HEADER

} // namespace dr

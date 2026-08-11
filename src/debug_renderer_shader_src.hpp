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
bool clip_segment(inout vec4 c0, inout vec4 c1)
{
#ifdef NDC_OPENGL
    // Clip z in [-1, 1]
    if (!clip_to_half_space(c0.z + c0.w, c1.z + c1.w, c0, c1)) // -z
        return false;
#else
    // Clip z in [0, 1]
    if (!clip_to_half_space(c0.z, c1.z, c0, c1)) // -z
        return false;
#endif

    // Pad x/y extent of frustum so that lines can overshoot side planes before expansion
    const float ndc_extent_xy = 2.0;
    float ext0 = ndc_extent_xy * c0.w;
    float ext1 = ndc_extent_xy * c1.w;

    if (!clip_to_half_space(c0.x + ext0, c1.x + ext1, c0, c1)) // -x
        return false;

    if (!clip_to_half_space(ext0 - c0.x, ext1 - c1.x, c0, c1)) // +x
        return false;

    if (!clip_to_half_space(c0.y + ext0, c1.y + ext1, c0, c1)) // -y
        return false;

    if (!clip_to_half_space(ext0 - c0.y, ext1 - c1.y, c0, c1)) // +y
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
layout(location = 2) in vec4 a_color;
layout(location = 3) in float a_width;
layout(location = 4) in uvec4 a_flags;

out vec4 v_color;
flat out vec2 v_start; // Segment start in framebuffer px
flat out vec2 v_dir; // Unit segment direction in framebuffer px
flat out vec2 v_extent; // x: segment length, y: half width (framebuffer px)
flat out int v_cap;

void main()
{
    vec4 c0 = u_world_to_clip * vec4(a_start, 1.0);
    vec4 c1 = u_world_to_clip * vec4(a_end, 1.0);

    if (!clip_segment(c0, c1))
    {
        // Push the quad out of the clip volume (z = 2) and early out
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    vec2 s0 = (c0.xy / c0.w * 0.5 + 0.5) * u_viewport.xy;
    vec2 s1 = (c1.xy / c1.w * 0.5 + 0.5) * u_viewport.xy;

    // Sub-pixel widths are drawn one pixel wide and faded instead of dropping out
    float w = a_width * u_viewport.z;
    float fade = clamp(w, 0.0, 1.0);
    float half_w = 0.5 * max(w, 1.0);

    vec2 d = s1 - s0;
    float len = length(d);

    const float screen_tol = 1e-6;
    vec2 dir = (len > screen_tol) ? d / len : vec2(1.0, 0.0);
    vec2 perp = vec2(-dir.y, dir.x);

    // Square and round caps extend the quad past the segment's endpoints
    int cap = int(a_flags.x);
    float ext = (cap == 0) ? 0.0 : half_w;

    vec2 corner = quad_corner(gl_VertexID);
    vec2 offset = (corner * 2.0 - 1.0) * (vec2(ext, half_w) + quad_aa_pad);

    vec2 p = mix(s0, s1, corner.x) + mat2(dir, perp) * offset;

    // Interpolating in clip space keeps depth correct along the segment
    vec4 c = mix(c0, c1, corner.x);
    vec2 ndc = p / u_viewport.xy * 2.0 - 1.0;
    gl_Position = vec4(ndc * c.w, c.z, c.w);

    v_color = vec4(a_color.rgb, a_color.a * fade);
    v_start = s0;
    v_dir = dir;
    v_extent = vec2(len, half_w);
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
layout(location = 1) in vec4 a_color;
layout(location = 2) in float a_size;
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
    float fade = clamp(size, 0.0, 1.0);
    float radius = 0.5 * max(size, 1.0);

    vec2 offset = quad_corner(gl_VertexID) * 2.0 - 1.0;
    vec2 p_screen = cen_screen + offset * (radius + quad_aa_pad);

    vec2 p_ndc = p_screen / u_viewport.xy * 2.0 - 1.0;
    gl_Position = vec4(p_ndc * cen_clip.w, cen_clip.zw);

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

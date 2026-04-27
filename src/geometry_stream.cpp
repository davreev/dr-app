#include <dr/app/geometry_stream.hpp>

#include <dr/memory.hpp>

namespace dr
{
namespace
{

void init_device_buffer(GfxBuffer& buf, GfxBuffer::Desc const& desc)
{
    if (buf.is_valid())
        buf.init(desc);
    else
        buf = GfxBuffer::make(desc);
}

} // namespace

bool GeometryStream::VertexKey::operator==(VertexKey const& other) const
{
    return src == other.src && slot == other.slot;
}

usize GeometryStream::VertexKey::Hash::operator()(VertexKey const& key) const
{
    return hash_mix(usize(key.src), usize(key.slot));
}

i32 GeometryStream::push_vertices(Span<u8 const> const& data) { return vertex_stage_.append(data); }

i32 GeometryStream::push_vertices_once(VertexKey const& key, Span<u8 const> const& data)
{
    auto const [it, ok] = vertex_offsets_.try_emplace(key);
    if (ok)
        it->second = push_vertices(data);

    return it->second;
}

i32 GeometryStream::push_indices(Span<u8 const> const& data) { return index_stage_.append(data); }

i32 GeometryStream::push_indices_once(void const* key, Span<u8 const> const& data)
{
    auto const [it, ok] = index_offsets_.try_emplace(key);
    if (ok)
        it->second = push_indices(data);

    return it->second;
}

void GeometryStream::update_device_buffers()
{
    vertex_stage_.update_device({.vertex_buffer = true});
    index_stage_.update_device({.index_buffer = true});
}

void GeometryStream::clear()
{
    vertex_stage_.host_buf.clear();
    index_stage_.host_buf.clear();
    vertex_offsets_.clear();
    index_offsets_.clear();
}

i32 GeometryStream::BufferStage::append(Span<u8 const> const& bytes)
{
    i32 const offset = host_buf.size();
    host_buf.insert(host_buf.end(), begin(bytes), end(bytes));
    return offset;
}

void GeometryStream::BufferStage::update_device(sg_buffer_usage usage)
{
    if (host_buf.empty())
        return;

    if (host_buf.size() > device_size)
    {
        device_size = host_buf.capacity();
        usage.stream_update = true;
        init_device_buffer(device_buf, {.size = device_size, .usage = usage});
    }

    sg_update_buffer(device_buf, {host_buf.data(), host_buf.size()});
}

} // namespace dr
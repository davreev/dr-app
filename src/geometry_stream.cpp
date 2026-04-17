#include <dr/app/geometry_stream.hpp>

#include <dr/memory.hpp>

namespace dr
{
namespace
{

void append_bytes(DynamicArray<u8>& buf, Span<u8 const> const& bytes)
{
    buf.insert(buf.end(), begin(bytes), end(bytes));
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

i32 GeometryStream::push_vertices(Span<u8 const> const& data)
{
    i32 const offset = vertex_.host.size();
    append_bytes(vertex_.host, data);
    return offset;
}

i32 GeometryStream::push_vertices_once(VertexKey const& key, Span<u8 const> const& data)
{
    auto const [it, ok] = vertex_.offsets.try_emplace(key);
    if (ok)
        it->second = push_vertices(data);

    return it->second;
}

i32 GeometryStream::push_indices(Span<u8 const> const& data)
{
    i32 const offset = index_.host.size();
    append_bytes(index_.host, data);
    return offset;
}

i32 GeometryStream::push_indices_once(void const* key, Span<u8 const> const& data)
{
    auto const [it, ok] = index_.offsets.try_emplace(key);
    if (ok)
        it->second = push_indices(data);

    return it->second;
}

void GeometryStream::clear()
{
    vertex_.host.clear();
    vertex_.offsets.clear();

    index_.host.clear();
    index_.offsets.clear();
}

void GeometryStream::DeviceBuffer::init(GfxBuffer::Desc const& desc)
{
    if (buffer.is_valid())
        buffer.init(desc);
    else
        buffer = GfxBuffer::make(desc);

    capacity = desc.size;
}

void GeometryStream::VertexStage::update_device()
{
    if (host.empty())
        return;

    if (host.size() > device.capacity)
    {
        device.init({
            .size = host.capacity(),
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .usage = SG_USAGE_STREAM,
        });
    }

    sg_update_buffer(device.buffer, {host.data(), host.size()});
}

void GeometryStream::IndexStage::update_device()
{
    if (host.empty())
        return;

    if (host.size() > device.capacity)
    {
        device.init({
            .size = host.capacity(),
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .usage = SG_USAGE_STREAM,
        });
    }

    sg_update_buffer(device.buffer, {host.data(), host.size()});
}

void GeometryStream::update_device_buffers()
{
    vertex_.update_device();
    index_.update_device();
}

} // namespace dr
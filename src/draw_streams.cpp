#include <dr/app/draw_streams.hpp>

#include <sokol_gfx.h>

#include <dr/bitwise.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/hash.hpp>
#include <dr/num_types.hpp>
#include <dr/span.hpp>

#include <dr/app/gfx_resource.hpp>

namespace dr
{
namespace
{

void align_end(DynamicArray<u8>& buffer, usize const align)
{
    buffer.resize(aligned_size(buffer.size(), align));
}

} // namespace

i32 StreamBuffer::push(Span<u8 const> const& bytes, usize const align)
{
    align_end(host_buffer, align);
    i32 const offset = host_buffer.size();
    host_buffer.insert(host_buffer.end(), begin(bytes), end(bytes));
    return offset;
}

void StreamBuffer::transfer(sg_buffer_usage usage)
{
    // Early out if nothing to transfer
    if (host_buffer.empty())
        return;

    // Resize the device buffer if necessary
    if (host_buffer.capacity() > device_size)
    {
        device_size = host_buffer.capacity();
        usage.stream_update = true;
        device_buffer.init({.size = device_size, .usage = usage});
    }

    sg_update_buffer(device_buffer, {host_buffer.data(), host_buffer.size()});
}

void StreamBuffer::reset() { host_buffer.clear(); }

bool VertexStream::Key::operator==(Key const& other) const
{
    return src == other.src && slot == other.slot;
}

usize VertexStream::Key::Hash::operator()(Key const& key) const
{
    return hash_mix(usize(key.src), usize(key.slot));
}

i32 VertexStream::push(Span<u8 const> const& bytes) { return buffer_.push(bytes, 4); }

i32 VertexStream::push_vertices_once(Key const& key, Span<u8 const> const& bytes)
{
    auto const [it, ok] = offsets_.try_emplace(key);
    if (ok)
        it->second = push(bytes);

    return it->second;
}

void VertexStream::transfer() { buffer_.transfer({.vertex_buffer = true}); }

void VertexStream::reset()
{
    buffer_.reset();
    offsets_.clear();
}

GfxBuffer::Handle VertexStream::device_buffer()
{
    if (!buffer_.device_buffer.is_valid())
        buffer_.device_buffer = GfxBuffer::alloc();

    return buffer_.device_buffer;
}

i32 IndexStream::push(Span<u8 const> const& bytes) { return buffer_.push(bytes, 4); }

i32 IndexStream::push_once(void const* key, Span<u8 const> const& bytes)
{
    auto const [it, ok] = offsets_.try_emplace(key);
    if (ok)
        it->second = push(bytes);

    return it->second;
}

void IndexStream::transfer() { buffer_.transfer({.index_buffer = true}); }

void IndexStream::reset()
{
    buffer_.reset();
    offsets_.clear();
}

GfxBuffer::Handle IndexStream::device_buffer()
{
    if (!buffer_.device_buffer.is_valid())
        buffer_.device_buffer = GfxBuffer::alloc();

    return buffer_.device_buffer;
}

i32 UniformStream::push(Span<u8 const> const& bytes)
{
    i32 const slice = buffer_.num_slices();
    buffer_.push_back(bytes);
    return slice;
}

i32 UniformStream::push_once(void const* key, Span<u8 const> const& bytes)
{
    auto const [it, ok] = slices_.try_emplace(key);
    if (ok)
        it->second = push(bytes);

    return it->second;
}

void UniformStream::reset()
{
    buffer_.clear();
    slices_.clear();
}

} // namespace dr

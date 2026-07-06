#include <dr/app/draw_streams.hpp>

#include <cassert>

#include <dr/memory.hpp>

namespace dr
{
namespace
{

usize aligned_size(usize const size, usize const align)
{
    return (size + (align - 1)) & ~(align - 1);
}

} // namespace

i32 StreamBuffer::push(Span<u8 const> const& bytes)
{
    i32 const offset = host_buffer.size();
    host_buffer.insert(host_buffer.end(), begin(bytes), end(bytes));
    return offset;
}

bool StreamBuffer::transfer(sg_buffer_usage usage)
{
    // Early out if nothing to transfer
    if (host_buffer.empty())
        return true;

    // NOTE(dr): Pass boundary is 4-byte aligned to match Sokol's append-cursor rounding. This
    // ensures host offsets match device offsets across passes.
    host_buffer.resize(aligned_size(host_buffer.size(), 4));

    // NOTE(dr): A resize recreates the device buffer, discarding anything already appended this
    // frame, so it's only safe to do before the frame's first transfer (host_offset == 0). Sizing
    // to the host buffer's capacity predicts this frame's needs from the previous frame's
    // high-water mark.
    if (host_offset == 0 && host_buffer.capacity() > device_size)
    {
        // First transfer; resize the device buffer
        device_size = host_buffer.capacity();
        usage.stream_update = true;
        device_buffer.init({.size = device_size, .usage = usage});
    }
    else if (host_buffer.size() > device_size)
    {
        // Overflow detected; can't resize device buffer on subsequent transfers
        host_offset = host_buffer.size();
        return false;
    }

    // Upload any data was pushed since the last transfer
    if (host_buffer.size() > host_offset)
    {
        i32 const offset = sg_append_buffer(
            device_buffer,
            {host_buffer.data() + host_offset, host_buffer.size() - host_offset});
        assert(usize(offset) == host_offset);
        host_offset = host_buffer.size();
    }

    return true;
}

void StreamBuffer::reset()
{
    host_buffer.clear();
    host_offset = 0;
}

bool VertexStream::Key::operator==(Key const& other) const
{
    return src == other.src && slot == other.slot;
}

usize VertexStream::Key::Hash::operator()(Key const& key) const
{
    return hash_mix(usize(key.src), usize(key.slot));
}

i32 VertexStream::push(Span<u8 const> const& bytes) { return buffer_.push(bytes); }

i32 VertexStream::push_vertices_once(Key const& key, Span<u8 const> const& bytes)
{
    auto const [it, ok] = offsets_.try_emplace(key);
    if (ok)
        it->second = push(bytes);

    return it->second;
}

bool VertexStream::transfer() { return buffer_.transfer({.vertex_buffer = true}); }

void VertexStream::reset()
{
    buffer_.reset();
    offsets_.clear();
}

template <typename Index>
i32 IndexStream<Index>::push(Span<Index const> const& indices)
{
    return buffer_.push(as<u8>(indices)) / sizeof(Index);
}

template <typename Index>
i32 IndexStream<Index>::push_once(void const* key, Span<Index const> const& indices)
{
    auto const [it, ok] = offsets_.try_emplace(key);
    if (ok)
        it->second = push(indices);

    return it->second;
}

template <typename Index>
bool IndexStream<Index>::transfer()
{
    return buffer_.transfer({.index_buffer = true});
}

template <typename Index>
void IndexStream<Index>::reset()
{
    buffer_.reset();
    offsets_.clear();
}

template struct IndexStream<u16>;
template struct IndexStream<u32>;
template struct IndexStream<i16>;
template struct IndexStream<i32>;

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
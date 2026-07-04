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

i32 BufferStage::push(Span<u8 const> const& bytes)
{
    i32 const offset = host_buf.size();
    host_buf.insert(host_buf.end(), begin(bytes), end(bytes));
    return offset;
}

bool BufferStage::transfer(sg_buffer_usage usage)
{
    // Early out if nothing to transfer
    if (host_buf.empty())
        return true;

    // NOTE(dr): Pass boundary is 4-byte aligned to match Sokol's append-cursor rounding. This
    // ensures host offsets match device offsets across passes.
    host_buf.resize(aligned_size(host_buf.size(), 4));

    // NOTE(dr): A resize recreates the device buffer, discarding anything already appended this
    // frame, so it's only safe to do before the frame's first transfer (host_offset == 0). Sizing
    // to the host buffer's capacity predicts this frame's needs from the previous frame's
    // high-water mark.
    if (host_offset == 0 && host_buf.capacity() > device_size)
    {
        // First transfer; resize the device buffer
        device_size = host_buf.capacity();
        usage.stream_update = true;
        device_buf.init({.size = device_size, .usage = usage});
    }
    else if (host_buf.size() > device_size)
    {
        // Overflow detected; can't resize device buffer on subsequent transfers
        host_offset = host_buf.size();
        return false;
    }

    // Upload any data was pushed since the last transfer
    if (host_buf.size() > host_offset)
    {
        i32 const offset = sg_append_buffer(
            device_buf,
            {host_buf.data() + host_offset, host_buf.size() - host_offset});
        assert(usize(offset) == host_offset);
        host_offset = host_buf.size();
    }

    return true;
}

void BufferStage::reset()
{
    host_buf.clear();
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

i32 VertexStream::push(Span<u8 const> const& bytes) { return stage_.push(bytes); }

i32 VertexStream::push_vertices_once(Key const& key, Span<u8 const> const& bytes)
{
    auto const [it, ok] = offsets_.try_emplace(key);
    if (ok)
        it->second = push(bytes);

    return it->second;
}

bool VertexStream::transfer() { return stage_.transfer({.vertex_buffer = true}); }

void VertexStream::reset()
{
    stage_.reset();
    offsets_.clear();
}

template <typename Index>
i32 IndexStream<Index>::push(Span<Index const> const& indices)
{
    return stage_.push(as<u8>(indices)) / sizeof(Index);
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
    return stage_.transfer({.index_buffer = true});
}

template <typename Index>
void IndexStream<Index>::reset()
{
    stage_.reset();
    offsets_.clear();
}

template struct IndexStream<u16>;
template struct IndexStream<u32>;
template struct IndexStream<i16>;
template struct IndexStream<i32>;

i32 UniformStream::push(Span<u8 const> const& bytes)
{
    i32 const slice = stage_.num_slices();
    stage_.push_back(bytes);
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
    stage_.clear();
    slices_.clear();
}

} // namespace dr
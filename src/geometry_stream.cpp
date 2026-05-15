#include <dr/app/geometry_stream.hpp>

#include <cassert>

#include <dr/memory.hpp>

namespace dr
{

i32 BufferStage::append(Span<u8 const> const& bytes)
{
    i32 const offset = host_buf.size();
    host_buf.insert(host_buf.end(), begin(bytes), end(bytes));
    return offset;
}

void BufferStage::update_device(sg_buffer_usage usage)
{
    if (host_buf.empty())
        return;

    if (host_buf.size() > device_size)
    {
        device_size = host_buf.capacity();
        usage.stream_update = true;
        device_buf.init({.size = device_size, .usage = usage});
    }

    sg_update_buffer(device_buf, {host_buf.data(), host_buf.size()});
}

bool VertexStream::Key::operator==(Key const& other) const
{
    return src == other.src && slot == other.slot;
}

usize VertexStream::Key::Hash::operator()(Key const& key) const
{
    return hash_mix(usize(key.src), usize(key.slot));
}

i32 VertexStream::push(Span<u8 const> const& bytes) { return stage_.append(bytes); }

i32 VertexStream::push_vertices_once(Key const& key, Span<u8 const> const& bytes)
{
    auto const [it, ok] = offsets_.try_emplace(key);
    if (ok)
        it->second = push(bytes);

    return it->second;
}

void VertexStream::update_device_buffer() { stage_.update_device({.vertex_buffer = true}); }

void VertexStream::clear()
{
    stage_.host_buf.clear();
    offsets_.clear();
}

template <typename Index>
i32 IndexStream<Index>::push(Span<Index const> const& indices)
{
    return stage_.append(as<u8>(indices)) / sizeof(Index);
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
void IndexStream<Index>::update_device_buffer()
{
    stage_.update_device({.index_buffer = true});
}

template <typename Index>
void IndexStream<Index>::clear()
{
    stage_.host_buf.clear();
    offsets_.clear();
}

template struct IndexStream<u16>;
template struct IndexStream<u32>;
template struct IndexStream<i16>;
template struct IndexStream<i32>;

} // namespace dr
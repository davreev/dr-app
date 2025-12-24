#pragma once

#include <dr/allocator.hpp>
#include <dr/hash_map.hpp>
#include <dr/string.hpp>

namespace dr
{

template <typename T>
struct AssetCache : AllocatorAware
{
    AssetCache(Allocator const alloc = {}) : assets_(alloc) {}

    AssetCache(AssetCache const& other, Allocator const alloc = {}) : assets_(other.assets_, alloc)
    {
    }

    AssetCache(AssetCache&& other) noexcept = default;
    AssetCache& operator=(AssetCache const& other) = default;
    AssetCache& operator=(AssetCache&& other) = default;

    /// Returns the allocator used by this container
    Allocator allocator() const { return assets_.get_allocator(); }

    /// Returns the asset at the given path if it's in the cache. Otherwise, returns a null pointer.
    T const* get(String const& path)
    {
        auto itr = assets_.find(path);
        return (itr == assets_.end()) ? nullptr : &itr->second;
    }

    /// Returns the asset at the given path. If the asset is not in the cache, it will be loaded by
    /// the given function object and cached.
    template <typename Loader>
    T const* get(String const& path, Loader&& load, bool const force_load = false)
    {
        static_assert(std::is_invocable_r_v<bool, Loader, String const&, T&>);

        // NOTE: The container's allocator must be explicitly passed to new keys when
        // using try_emplace
        auto const [itr, ok] = assets_.try_emplace({path, allocator()});

        // Load asset on cache miss
        if (ok || force_load)
        {
            if (!load(itr->first, itr->second))
            {
                assets_.erase(itr);
                return nullptr;
            }
        }

        return &itr->second;
    }

    /// Removes the asset at the given path from the cache
    void remove(String const& path) { assets_.erase(path); }

    /// Clears all assets from the cache
    void clear() { assets_.clear(); }

  private:
    StableHashMap<String, T> assets_;
};

} // namespace dr
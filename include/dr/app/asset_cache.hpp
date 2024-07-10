#pragma once

#include <string>

#include <dr/hash_map.hpp>

namespace dr
{

template <typename T>
struct AssetCache
{
    // TODO(dr): Make allocator-aware

    /// Returns the asset at the given path if it's in the cache. Otherwise, returns a null pointer.
    T const* get(std::string const& path)
    {
        auto itr = assets_.find(path);
        return (itr == assets_.end()) ? nullptr : &itr->second;
    }

    /// Returns the asset at the given path. If the asset is not in the cache, it will be loaded by
    /// the given function object and cached.
    template <typename Loader>
    T const* get(std::string path, Loader&& load, bool const force_load = false)
    {
        static_assert(std::is_invocable_r_v<bool, Loader, std::string const&, T&>);

        auto const [itr, ok] = assets_.try_emplace(std::move(path));

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
    void remove(std::string const& path) { assets_.erase(path); }

    /// Clears all assets from the cache
    void clear() { assets_.clear(); }

  private:
    HashMap<std::string, T> assets_{};
};

} // namespace dr
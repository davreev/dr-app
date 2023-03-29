#include <dr/app/thread_pool.hpp>

#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace dr
{
namespace
{

struct
{
    std::vector<std::thread> workers;
    std::deque<WorkItem> work_items;
    std::mutex mutex;
    std::condition_variable condition;
    bool started;
} state;

void do_work()
{
    while (true)
    {
        WorkItem item{};

        // Pop a work item off the queue when one becomes available
        {
            std::unique_lock lock{state.mutex};
            state.condition.wait(lock, []() { return !state.work_items.empty(); });

            item = state.work_items.front();
            state.work_items.pop_front();
        }

        // If work item is invalid, then stop has been requested
        if (item.func == nullptr)
            return;

        item.func(item.state);
    }
}

} // namespace

void thread_pool_queue_work(WorkItem const& item)
{
    assert(state.started);
    assert(item.func != nullptr);

    {
        std::scoped_lock lock{state.mutex};
        state.work_items.push_back(item);
    }
    state.condition.notify_one();
}

void thread_pool_start_workers(isize const count)
{
    assert(count > 0);

    // Wrap up any work in progress
    if (state.started)
        thread_pool_stop_workers();

    state.workers.resize(count);

    for (auto& worker : state.workers)
        worker = std::thread{do_work};

    state.started = true;
}

void thread_pool_stop_workers()
{
    if (!state.started)
        return;

    // Queue an invalid work item for each thread
    {
        std::scoped_lock lock{state.mutex};
        state.work_items.insert(state.work_items.end(), state.workers.size(), {});
    }
    state.condition.notify_all();

    // Wait on workers to finish up remaining work items
    for (auto& worker : state.workers)
        worker.join();

    state.started = false;
}

isize thread_pool_num_workers() { return (state.started) ? state.workers.size() : 0; }

} // namespace dr
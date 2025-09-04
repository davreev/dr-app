#include <dr/app/thread_pool.hpp>

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <dr/deque.hpp>
#include <dr/dynamic_array.hpp>

namespace dr
{
namespace
{

struct
{
    DynamicArray<std::thread> workers;
    Deque<TaskRef> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool is_active{false};
} state;

constexpr TaskRef invalid_task{};

void do_work()
{
    while (true)
    {
        TaskRef task{};

        // Pop a task off the queue when one becomes available
        {
            std::unique_lock lock{state.mutex};
            state.condition.wait(lock, []() { return !state.tasks.empty(); });

            task = state.tasks.front();
            state.tasks.pop_front();
        }

        // If task is invalid, then stop has been requested
        if (!task.is_valid())
            return;

        task();
    }
}

} // namespace

void ThreadPool::start(isize const num_workers)
{
    assert(num_workers > 0);

    // Wrap up any in progress tasks
    if (state.is_active)
        ThreadPool::stop();

    state.workers.resize(num_workers);

    for (auto& worker : state.workers)
        worker = std::thread{do_work};

    state.is_active = true;
}

void ThreadPool::stop()
{
    if (!state.is_active)
        return;

    // Queue an invalid task for each thread
    {
        std::scoped_lock const lock{state.mutex};
        state.tasks.insert(state.tasks.end(), state.workers.size(), invalid_task);
    }
    state.condition.notify_all();

    // Wait on workers to finish up remaining tasks
    for (auto& worker : state.workers)
        worker.join();

    state.is_active = false;
}

void ThreadPool::submit(TaskRef const& task)
{
    assert(state.is_active);
    assert(task.is_valid());

    {
        std::scoped_lock const lock{state.mutex};
        state.tasks.push_back(task);
    }
    state.condition.notify_one();
}

} // namespace dr
#include <dr/app/task_queue.hpp>

#include <algorithm>
#include <atomic>

#include <dr/app/thread_pool.hpp>

namespace dr
{
namespace
{

/// Partial backport of functionality offered by std::atomic_ref in C++20 limited to integral types
/// an enums. Likely UB but should work with most implementations. See links below for further
/// discussion (1) and precedent (2).
///
/// 1. https://stackoverflow.com/a/67620988
/// 2. https://github.com/facebook/folly/blob/main/folly/synchronization/AtomicRef.h#L86
template <typename T, std::enable_if_t<(std::is_integral_v<T> || std::is_enum_v<T>)>* = nullptr>
std::atomic<T>* as_atomic(T* t)
{
    static_assert(sizeof(std::atomic<T>) == sizeof(T));
    static_assert(alignof(std::atomic<T>) == alignof(T));
    return reinterpret_cast<std::atomic<T>*>(t);
}

} // namespace

void TaskQueue::poll()
{
    usize count = 0;

    // Poll tasks in the current generation
    for (auto& task : tasks_)
    {
        if (task.gen > poll_gen_)
            break;

        auto const status = as_atomic(&task.status)->load();
        switch (status)
        {
            case DeferredTask::Status_Queued:
            {
                if (task.raise_event(PollEvent::BeforeSubmit))
                {
                    task.status = DeferredTask::Status_Submitted;
                    thread_pool_submit(&task);
                }

                break;
            }
            case DeferredTask::Status_Completed:
            {
                if (task.raise_event(PollEvent::AfterComplete))
                    task.status = {};

                break;
            }
            default:
            {
            }
        }

        ++count;
    }

    // If no tasks left in the current generation, move on to the next. Otherwise, cull published
    // tasks from the current generation and continue.
    if (count == 0)
    {
        if (poll_gen_ < push_gen_)
            ++poll_gen_;
    }
    else
    {
        auto it =
            std::remove_if(tasks_.begin(), tasks_.begin() + count, [&](DeferredTask const& task) {
                return task.gen == poll_gen_ && task.status == DeferredTask::Status_Default;
            });

        tasks_.erase(it, tasks_.begin() + count);
    }
}

void TaskQueue::DeferredTask::operator()()
{
    task();
    as_atomic(&status)->store(Status_Completed);
};

} // namespace dr
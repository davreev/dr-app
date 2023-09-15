#include <dr/app/task_queue.hpp>

#include <algorithm>
#include <cassert>

#include <dr/container_utils.hpp>

#include <dr/app/thread_pool.hpp>

namespace dr
{

void TaskQueue::push(TaskRef const& task, void* context, PollCallback* poll_cb)
{
    assert(task.is_valid());
    tasks_.push_back({
        task,
        context,
        poll_cb,
        DeferredTask::Status_Queued,
        push_gen_,
    });
}

void TaskQueue::barrier()
{
    ++push_gen_;
}

void TaskQueue::poll()
{
    usize count = 0;

    // Poll tasks in the current generation
    for (auto& task : tasks_)
    {
        if (task.gen > poll_gen_)
            break;

        switch (task.status)
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
        auto it = std::remove_if(
            tasks_.begin(),
            tasks_.begin() + count,
            [&](DeferredTask const& task) {
                return task.gen == poll_gen_ && task.status == 0;
            });

        tasks_.erase(it, tasks_.begin() + count);
    }
}

isize TaskQueue::size() const { return dr::size(tasks_); }

} // namespace dr
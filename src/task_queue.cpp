#include <dr/app/task_queue.hpp>

#include <cassert>

#include <dr/app/thread_pool.hpp>

namespace dr
{

void TaskQueue::push(TaskRef const& ref, void* const context, PollCallback* const poll_cb)
{
    assert(ref.is_valid());
    queue_.push_back(pool_.make(ref, context, poll_cb));
}

void TaskQueue::poll()
{
    // Early out if queue is empty
    if (queue_.empty())
        return;

    isize batch_size{0};

    // Poll tasks in the current batch
    for (auto& task : queue_)
    {
        if (task == nullptr)
            break;

        Task::Status const status = task->status.load();
        switch (status)
        {
            case Task::Status_Queued:
            {
                if (task->raise_event(PollEvent::BeforeSubmit))
                {
                    task->status.store(Task::Status_Submitted);
                    ThreadPool::submit(task);
                }

                break;
            }
            case Task::Status_Completed:
            {
                if (task->raise_event(PollEvent::AfterComplete))
                {
                    pool_.release(task);
                    task = nullptr;
                }

                break;
            }
            default:
            {
            }
        }

        ++batch_size;
    }

    // Partition the batch, placing nulls at the front
    for (isize i = 0, j = 0; i < batch_size; ++i)
    {
        if (queue_[i] == nullptr)
        {
            queue_[i] = queue_[j];
            queue_[j] = nullptr;
            ++j;
        }
    }

    // Remove nulls from front of batch
    while (!queue_.empty() && queue_.front() == nullptr)
        queue_.pop_front();
}

TaskQueue::Task* TaskQueue::TaskPool::make(
    TaskRef const& ref,
    void* const context,
    PollCallback* const poll_cb)
{
    Task* task{};

    if (free_.empty())
    {
        pool_.emplace_back();
        task = &pool_.back();
    }
    else
    {
        task = free_.back();
        free_.pop_back();
    }

    task->ref = ref;
    task->context = context;
    task->poll_cb = poll_cb;
    return task;
}

void TaskQueue::TaskPool::release(Task* const task)
{
    task->ref = {};
    task->context = {};
    task->poll_cb = {};
    task->status.store({});
    free_.push_back(task);
}

} // namespace dr
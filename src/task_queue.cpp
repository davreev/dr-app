#include <dr/app/task_queue.hpp>

#include <algorithm>

#include <dr/app/thread_pool.hpp>

namespace dr
{

void TaskQueue::push(TaskRef const& ref, void* const context, PollCallback* const poll_cb)
{
    assert(ref.is_valid());
    Task* const task = tasks_.acquire(ref, context, poll_cb);
    batches_.back().push_back(task);
    ++size_;
}

void TaskQueue::poll()
{
    auto& batch = batches_.front();

    // Early out if batch is empty
    if (batch.empty())
        return;

    // Poll tasks in the current batch
    for (auto& task : batch)
    {
        Task::Status const status = task->status.load();

        switch (status)
        {
            case Task::Status_Queued:
            {
                if (task->raise_event(PollEvent::BeforeSubmit))
                {
                    task->status.store(Task::Status_Submitted);
                    thread_pool_submit(task);
                }

                break;
            }
            case Task::Status_Completed:
            {
                if (task->raise_event(PollEvent::AfterComplete))
                {
                    tasks_.release(task);
                    task = nullptr;
                }

                break;
            }
            default:
            {
            }
        }
    }

    // Cull cleared tasks from batch
    {
        auto it = std::remove_if(batch.begin(), batch.end(), [](Task const* task) {
            return task == nullptr;
        });
        size_ -= batch.end() - it;
        batch.erase(it, batch.end());
    }

    // Remove batch if empty and not the last one
    if (batch.empty() && batches_.size() > 1)
        batches_.pop_front();
}

TaskQueue::Task* TaskQueue::TaskPool::acquire(
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
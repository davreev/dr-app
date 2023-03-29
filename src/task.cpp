#include <dr/app/task.hpp>

#include <algorithm>
#include <cassert>

#include <dr/container_utils.hpp>

#include <dr/app/thread_pool.hpp>

namespace dr
{
namespace
{

WorkItem as_work_item(Task* const task)
{
    return {[](void* task) { static_cast<Task*>(task)->execute(); }, task};
}

} // namespace

void Task::execute()
{
    status = TaskStatus_Started;
    do_execute();
    status = TaskStatus_Finished;
}

bool TaskQueue::submit(
    Task* const task, 
    void* const context, 
    InitTask const init, 
    PublishTask const publish)
{
    if (task->status != TaskStatus_Default)
        return false;

    tasks_.push_back({task, context, init, publish, submit_gen_});
    task->status = TaskStatus_Submitted;
    return true;
}

void TaskQueue::barrier()
{
    ++submit_gen_;
}

void TaskQueue::poll()
{
    usize count = 0;

    // Poll tasks in current gen
    for (auto& task : tasks_)
    {
        if (task.gen > poll_gen_)
            break;

        ++count;

        switch (task.ptr->status)
        {
            case TaskStatus_Submitted:
            {
                if (task.init(task.ptr, task.context))
                {
                    task.ptr->status = TaskStatus_Initialized;
                    thread_pool_queue_work(as_work_item(task.ptr));
                }

                break;
            }
            case TaskStatus_Finished:
            {
                if (task.publish(task.ptr, task.context))
                    task.ptr->status = TaskStatus_Published;

                break;
            }
            default:
            {
            }
        }
    }

    // If no tasks left in the current generation, move on to the next. Otherwise, cull published
    // tasks from the current generation and continue.
    if (count == 0)
    {
        if (poll_gen_ < submit_gen_)
            ++poll_gen_;
    }
    else
    {
        auto it = std::remove_if(
            tasks_.begin(),
            tasks_.begin() + count,
            [&](DelayedTask const& task) {
                return task.gen == poll_gen_ && task.ptr->status == TaskStatus_Published;
            });

        tasks_.erase(it, tasks_.begin() + count);
    }
}

isize TaskQueue::count() const { return size<isize>(tasks_); }

} // namespace dr
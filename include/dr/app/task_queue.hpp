#pragma once

#include <vector>

#include <dr/basic_types.hpp>

#include <dr/app/task_ref.hpp>

namespace dr
{

struct TaskQueue
{
    struct PollEvent
    {
        enum Type : u8
        {
            Default = 0,
            BeforeSubmit,
            AfterComplete,
            _Count,
        };

        void* task;
        void* context;
        Type type;
    };

    /// Pushes a task onto the queue for deferred asynchronous execution. The calling context is
    /// responsible for keeping the task alive until its results have been published.
    void push(
        TaskRef const& task,
        void* context = nullptr,
        bool (*poll_cb)(PollEvent const& event) = nullptr);

    /// Inserts a barrier. Tasks queued after inserting a barrier won't start until all
    /// previously queued tasks have completed.
    void barrier();

    /// Polls tasks in the queue. This should be called at regular intervals (e.g. every frame).
    void poll();

    /// Returns the number of tasks in the queue
    isize size() const;

    /// Returns true if the queue is empty
    bool empty() const { return tasks_.size() == 0; }

  private:
    struct DeferredTask
    {
        enum Status : u8
        {
            Status_Default = 0,
            Status_Queued,
            Status_Submitted,
            Status_Completed,
            _Status_Count,
        };

        TaskRef task;
        void* context;
        bool (*poll_cb)(PollEvent const& event);
        Status status;
        usize gen;

        /// Invokes the referenced task
        constexpr void operator()()
        {
            task();
            status = Status_Completed;
        };

        constexpr bool raise_event(PollEvent::Type const type)
        {
            return (poll_cb) ? poll_cb({task.get(), context, type}) : true;
        }
    };

    std::vector<DeferredTask> tasks_;
    usize push_gen_;
    usize poll_gen_;
};

} // namespace dr
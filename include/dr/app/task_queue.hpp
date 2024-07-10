#pragma once

#include <atomic>
#include <deque>
#include <vector>

#include <dr/basic_types.hpp>

#include <dr/app/task_ref.hpp>

namespace dr
{

struct TaskQueue
{
    // TODO(dr): Make allocator-aware

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

    using PollCallback = bool(PollEvent const& event);

    /// Pushes a task onto the queue for deferred asynchronous execution. The calling context is
    /// responsible for keeping the task alive until its results have been published.
    void push(TaskRef const& ref, void* context = nullptr, PollCallback* poll_cb = nullptr);

    /// Inserts a barrier. Tasks queued after inserting a barrier won't start until all
    /// previously queued tasks have completed.
    void barrier() { batches_.push_back({}); }

    /// Polls tasks in the queue. This should be called at regular intervals (e.g. every frame).
    void poll();

    /// Returns the number of tasks in the queue
    isize size() const { return size_; }

  private:
    struct Task
    {
        enum Status : u8
        {
            Status_Queued = 0,
            Status_Submitted,
            Status_Completed,
            _Status_Count,
        };

        TaskRef ref;
        void* context;
        PollCallback* poll_cb;
        std::atomic<Status> status;

        /// Invokes the referenced task
        void operator()()
        {
            ref();
            status.store(Status_Completed);
        }

        /// Fires poll callback
        bool raise_event(PollEvent::Type const type)
        {
            return (poll_cb) ? poll_cb({ref.get(), context, type}) : true;
        }
    };

    struct TaskPool
    {
        Task* acquire(TaskRef const& ref, void* context, PollCallback* poll_cb);
        void release(Task* const task);

      private:
        std::deque<Task> pool_{};
        std::vector<Task*> free_{};
    };

    std::deque<std::vector<Task*>> batches_{1};
    TaskPool tasks_{};
    isize size_{};
};

} // namespace dr
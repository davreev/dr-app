#pragma once

#include <atomic>

#include <dr/allocator.hpp>
#include <dr/basic_types.hpp>
#include <dr/deque.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/string.hpp>

#include <dr/app/task_ref.hpp>

namespace dr
{

struct TaskQueue : AllocatorAware
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

    using PollCallback = bool(PollEvent const& event);

    TaskQueue(Allocator const alloc = {}) : queue_(alloc), pool_(alloc) {}

    TaskQueue(TaskQueue&& other) noexcept = default;
    TaskQueue& operator=(TaskQueue&& other) noexcept = default;

    /// Returns the allocator used by this container
    Allocator allocator() const { return queue_.get_allocator(); }

    /// Pushes a task onto the queue for deferred asynchronous execution. The calling context is
    /// responsible for keeping the task alive until completion.
    void push(TaskRef const& ref, void* context = nullptr, PollCallback* poll_cb = nullptr);

    /// Inserts a barrier. Tasks queued after inserting a barrier won't start until all
    /// previously queued tasks have completed.
    void barrier() { queue_.push_back(nullptr); }

    /// Polls tasks in the queue. This should be called at regular intervals (e.g. every frame).
    void poll();

    /// Returns the number of tasks in the queue
    isize size() const { return static_cast<isize>(queue_.size()); }

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

    struct TaskPool : AllocatorAware
    {
        TaskPool(Allocator const alloc = {}) : pool_(alloc), free_(alloc) {}

        TaskPool(TaskPool&& other) noexcept = default;
        TaskPool& operator=(TaskPool&& other) noexcept = default;

        Task* make(TaskRef const& ref, void* context, PollCallback* poll_cb);
        void release(Task* const task);

      private:
        Deque<Task> pool_;
        DynamicArray<Task*> free_;
    };

    Deque<Task*> queue_;
    TaskPool pool_;
};

} // namespace dr
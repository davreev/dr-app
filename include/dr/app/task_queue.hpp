#pragma once

#include <cassert>
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

    using PollCallback = bool(PollEvent const& event);

    /// Pushes a task onto the queue for deferred asynchronous execution. The calling context is
    /// responsible for keeping the task alive until its results have been published.
    void push(TaskRef const& task, void* context = nullptr, PollCallback* poll_cb = nullptr)
    {
        assert(task.is_valid());
        tasks_.push_back({task, context, poll_cb, DeferredTask::Status_Queued, push_gen_});
    }

    /// Inserts a barrier. Tasks queued after inserting a barrier won't start until all
    /// previously queued tasks have completed.
    void barrier() { ++push_gen_; }

    /// Polls tasks in the queue. This should be called at regular intervals (e.g. every frame).
    void poll();

    /// Returns the number of tasks in the queue
    isize size() const { return static_cast<isize>(tasks_.size()); }

    /// Returns true if the queue is empty
    bool empty() const { return tasks_.empty(); }

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
        PollCallback* poll_cb;
        Status status;
        usize gen;

        /// Invokes the referenced task
        void operator()();

        /// Fires poll callback
        bool raise_event(PollEvent::Type const type)
        {
            return (poll_cb) ? poll_cb({task.get(), context, type}) : true;
        }
    };

    /*
        TODO(dr): Could store a deque of vectors where each generation has its own list. Current
        generation is at the front and new tasks are pushed onto the back. Would avoid need for
        explicit generation tracking and make compaction more efficient.
    */
    std::vector<DeferredTask> tasks_;
    usize push_gen_;
    usize poll_gen_;
};



} // namespace dr
#pragma once

#include <vector>

#include <dr/app/task.hpp>

namespace dr
{

struct TaskQueue
{
    using InitTask = bool (*)(Task* task, void const* context);
    using PublishTask = bool (*)(Task const* task, void* context);

    // Submit a task for asynchronous execution. The calling context is responsible for keeping the
    // task alive until it has been published.
    bool submit(Task* task, void* context, InitTask init, PublishTask publish);

    // Inserts a barrier. Tasks submitted after inserting a barrier won't be initialized until
    // all previously submitted tasks have been published.
    void barrier();

    // Polls queued tasks. This should be called at regular intervals (e.g. every frame).
    void poll();

    // Returns the number of tasks in the queue
    isize size() const;

  private:
    struct DeferredTask
    {
        Task* ptr;
        void* context;
        InitTask init;
        PublishTask publish;
        usize gen;
    };

    std::vector<DeferredTask> tasks_;
    usize submit_gen_;
    usize poll_gen_;
};

} // namespace dr
#pragma once

/*
    Dead simple thread pool for doing work off the main thread
*/

#include <dr/basic_types.hpp>

namespace dr
{

struct WorkItem
{
    void (*func)(void*);
    void* state;
};

void thread_pool_start_workers(isize count);

void thread_pool_stop_workers();

isize thread_pool_num_workers();

void thread_pool_queue_work(WorkItem const& item);

} // namespace dr

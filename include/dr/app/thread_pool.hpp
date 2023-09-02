#pragma once

/*
    Simple global thread pool for doing work off the main thread
*/

#include <dr/basic_types.hpp>

#include <dr/app/task_ref.hpp>

namespace dr
{

void thread_pool_start(isize size);

void thread_pool_stop();

void thread_pool_submit(TaskRef const& task);

isize thread_pool_size();

} // namespace dr

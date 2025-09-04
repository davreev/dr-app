#pragma once

/*
    Simple global thread pool for doing work off the main thread
*/

#include <dr/basic_types.hpp>

#include <dr/app/task_ref.hpp>

namespace dr
{

struct ThreadPool
{
    static void start(isize num_workers);

    static void stop();

    static void submit(TaskRef const& task);
};

} // namespace dr

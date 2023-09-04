#include <utest.h>

#include <dr/defer.hpp>

#include <dr/app/task_queue.hpp>
#include <dr/app/thread_pool.hpp>

UTEST(task_queue, poll)
{
    using namespace dr;

    thread_pool_start(1);
    auto _ = defer([]() {
        thread_pool_stop();
    });

    isize x = 2;
    auto const square_x = [&]() -> void {
        x *= x;
    };

    isize y = 4;
    auto const negate_y = [&]() -> void {
        y = -y;
    };

    TaskQueue queue{};
    queue.push(&square_x);
    queue.push(&negate_y);

    while (!queue.empty())
        queue.poll();

    ASSERT_EQ(4, x);
    ASSERT_EQ(-4, y);
}

UTEST(task_queue, poll_barrier)
{
    using namespace dr;

    thread_pool_start(1);
    auto _ = defer([]() {
        thread_pool_stop();
    });

    isize x = 2;
    auto const square_x = [&]() -> void {
        x *= x;
    };
    auto const negate_x = [&]() -> void {
        x = -x;
    };
    auto const cube_x = [&]() -> void {
        x *= x * x;
    };

    TaskQueue queue{};
    queue.push(&square_x);
    queue.barrier();
    queue.push(&negate_x);
    queue.barrier();
    queue.push(&cube_x);

    while (!queue.empty())
        queue.poll();

    ASSERT_EQ(-64, x);
}
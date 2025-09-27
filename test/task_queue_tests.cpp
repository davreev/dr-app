#include <utest.h>

#include <dr/defer.hpp>
#include <dr/memory.hpp>

#include <dr/app/task_queue.hpp>
#include <dr/app/thread_pool.hpp>

UTEST(task_queue, poll)
{
    using namespace dr;

    ThreadPool::start(1);
    auto _ = defer([]() { ThreadPool::stop(); });

    isize x = 2;
    auto const square_x = [&]() -> void { x *= x; };

    isize y = 4;
    auto const negate_y = [&]() -> void { y = -y; };

    TaskQueue queue{};
    queue.push(&square_x);
    queue.push(&negate_y);

    while (queue.size() > 0)
        queue.poll();

    ASSERT_EQ(4, x);
    ASSERT_EQ(-4, y);
}

UTEST(task_queue, poll_barrier)
{
    using namespace dr;

    ThreadPool::start(1);
    auto _ = defer([]() { ThreadPool::stop(); });

    isize x = 2;
    auto const square_x = [&]() -> void { x *= x; };
    auto const negate_x = [&]() -> void { x = -x; };
    auto const cube_x = [&]() -> void { x *= x * x; };

    TaskQueue queue{};
    queue.push(&square_x);
    queue.barrier();
    queue.push(&negate_x);
    queue.barrier();
    queue.push(&cube_x);

    while (queue.size() > 0)
        queue.poll();

    ASSERT_EQ(-64, x);
}

UTEST(task_queue, allocator_propagation)
{
    using namespace dr;

    // Restore default memory resource after test is complete
    auto def_mem = std::pmr::get_default_resource();
    auto _ = defer([=]() { std::pmr::set_default_resource(def_mem); });

    DebugMemoryResource mem[2]{};
    std::pmr::set_default_resource(&mem[0]);

    {
        // Should use the current default resource
        TaskQueue src{};
        ASSERT_TRUE(src.allocator().resource()->is_equal(mem[0]));
    }

    {
        // Should use the given resource
        TaskQueue src{&mem[1]};
        ASSERT_TRUE(src.allocator().resource()->is_equal(mem[1]));

        // Should use the same memory resource as src
        TaskQueue src_move{std::move(src)};
        ASSERT_TRUE(src_move.allocator().resource()->is_equal(mem[1]));
    }
}

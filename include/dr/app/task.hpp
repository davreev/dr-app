#pragma once

#include <vector>

#include <dr/basic_types.hpp>

namespace dr
{

struct Task
{
    enum Status : u8
    {
        Status_Default = 0,
        Status_Submitted,
        Status_Initialized,
        Status_Started,
        Status_Finished,
        Status_Published,
        _Status_Count,
    };

    Status status{};

    void execute()
    {
        status = Status_Started;
        do_execute();
        status = Status_Finished;
    }

  protected:
    virtual void do_execute() = 0;
};

} // namespace dr
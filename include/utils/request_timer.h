#pragma once

#include "transport_logger.h"
#include <chrono>
#include <cpprest/http_listener.h>
#include <iostream>
#include <string>

namespace ts
{
class RequestTimer
{
  public:
    explicit RequestTimer(int status = 500) : start_(std::chrono::steady_clock::now()), status_(status)
    {
    }

    ~RequestTimer()
    {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();

        ts::TransportLogger::LogResponse(status_, duration);
    }

    void SetStatus(int status)
    {
        status_ = status;
    }

  private:
    std::chrono::steady_clock::time_point start_;
    int status_;
};
} // namespace ts
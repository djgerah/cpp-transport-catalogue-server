#pragma once

#include <chrono>
#include <cpprest/http_listener.h>
#include <iostream>
#include <string>

namespace ts
{
class TransportLogger
{
  public:
    static void LogRequest(const std::string &method, const std::string &address)
    {
        std::cout << "[REQUEST] " << method << " " << address << std::endl;
    }

    static void LogRequest(web::http::http_request &request)
    {
        auto method = request.method();
        auto path = request.request_uri().to_string();

        LogRequest(utility::conversions::to_utf8string(method), utility::conversions::to_utf8string(path));
    }

    static void LogResponse(int status, long long duration)
    {
        std::cout << "[RESPONSE] status=" << status << " time=" << duration << "ms" << std::endl;
    }

    static void LogError(const std::string &message)
    {
        std::cerr << "[ERROR] " << message << std::endl;
    }
};
} // namespace ts
#pragma once
#include <string>
#include <bits/chrono.h>
#include <iostream>

class ScopedLogTimer
{
public:
    explicit ScopedLogTimer(std::string label)
        : label_(std::move(label)),
            start_(std::chrono::steady_clock::now())
    {
    }

    ScopedLogTimer(const ScopedLogTimer&) = delete;
    ScopedLogTimer& operator=(const ScopedLogTimer&) = delete;

    ~ScopedLogTimer()
    {
        const auto elapsed = std::chrono::steady_clock::now() - start_;
        const double elapsed_ms =
            std::chrono::duration<double, std::milli>(elapsed).count();
        std::cout << label_ << ": " << elapsed_ms << " ms" << std::endl;
    }

private:
    std::string label_;
    std::chrono::steady_clock::time_point start_;
};